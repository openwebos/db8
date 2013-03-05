/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */


#include "MojDbLunaServiceApp.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbServiceHandler.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-luna/MojDbLevelFactory.h"
#else
#error "Set database type"
#endif

#ifndef MOJ_VERSION_STRING
#define MOJ_VERSION_STRING NULL
#endif

const MojChar* const MojDbLunaServiceApp::VersionString = MOJ_VERSION_STRING;
const MojChar* const MojDbLunaServiceApp::MainDir = _T("main");
const MojChar* const MojDbLunaServiceApp::TempDir = _T("temp");
const MojChar* const MojDbLunaServiceApp::TempStateDir = _T("/tmp/mojodb");
const MojChar* const MojDbLunaServiceApp::TempInitStateFile = _T("/tmp/mojodb/tempdb_init");

int main(int argc, char** argv)
{
   MojAutoPtr<MojDbLunaServiceApp> app(new MojDbLunaServiceApp);
   MojAllocCheck(app.get());
   return app->main(argc, argv);
}

MojDbLunaServiceApp::MojDbLunaServiceApp()
: MojReactorApp<MojGmainReactor>(MajorVersion, MinorVersion, VersionString),
  m_mainService(m_dispatcher),
  m_tempService(m_dispatcher)
{
   // set up db first
#ifdef MOJ_USE_BDB
   MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
   MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#else
  #error "Database not set"
#endif
   MojLogTrace(s_log);
}

MojDbLunaServiceApp::~MojDbLunaServiceApp()
{
    MojLogTrace(s_log);
}

MojErr MojDbLunaServiceApp::init()
{
    MojLogTrace(s_log);

    MojErr err = Base::init();
    MojErrCheck(err);

    MojDbStorageEngine::createEnv(m_env);
    MojAllocCheck(m_env.get());

    m_internalHandler.reset(new MojDbServiceHandlerInternal(m_mainService.db(), m_reactor, m_mainService.service()));
    MojAllocCheck(m_internalHandler.get());

    err = m_mainService.init(m_reactor);
    MojErrCheck(err);
    err = m_tempService.init(m_reactor);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::configure(const MojObject& conf)
{
    MojLogTrace(s_log);

    MojErr err = Base::configure(conf);
    MojErrCheck(err);

    MojObject dbConf;

    conf.get(MojDbStorageEngine::engineFactory()->name(), dbConf);
    err = m_env->configure(dbConf);
    MojErrCheck(err);
    err = m_mainService.db().configure(conf);
    MojErrCheck(err);
    err = m_tempService.db().configure(conf);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::open()
{
    MojLogTrace(s_log);
    MojLogNotice(s_log, _T("mojodb starting..."));

    MojErr err = Base::open();
    MojErrCheck(err);

    // start message queue thread pool
    err = m_dispatcher.start(NumThreads);
    MojErrCheck(err);

    // open db env
    bool dbOpenFailed = false;
    err = m_env->open(m_dbDir);
    MojErrCatchAll(err) {
        dbOpenFailed = true;
    }

    // open db services
    err = m_mainService.open(m_reactor, m_env.get(), MojDbServiceDefs::ServiceName, m_dbDir, MainDir);
    MojErrCatchAll(err) {
        dbOpenFailed = true;
    }
    err = m_tempService.open(m_reactor, m_env.get(), MojDbServiceDefs::TempServiceName, m_dbDir, TempDir);
    MojErrCatchAll(err) {
        dbOpenFailed = true;
    }
    if (!dbOpenFailed) {
        err = dropTemp();
        MojErrCheck(err);
    }

    // open internal handler
    err = m_internalHandler->open();
    MojErrCheck(err);
    err = m_mainService.service().addCategory(MojDbServiceDefs::InternalCategory, m_internalHandler.get());
    MojErrCheck(err);
    err = m_internalHandler->configure(dbOpenFailed);
    MojErrCheck(err);

    MojLogNotice(s_log, _T("mojodb started"));

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::close()
{
    MojLogTrace(s_log);
    MojLogNotice(s_log, _T("mojodb stopping..."));

    // stop dispatcher
    MojErr err = MojErrNone;
    MojErr errClose = m_dispatcher.stop();
    MojErrAccumulate(err, errClose);
    errClose = m_dispatcher.wait();
    MojErrAccumulate(err, errClose);
    // close services
    errClose = m_mainService.close();
    MojErrAccumulate(err, errClose);
    errClose = m_tempService.close();
    MojErrAccumulate(err, errClose);
    m_internalHandler->close();
    m_internalHandler.reset();

    errClose = Base::close();
    MojErrAccumulate(err, errClose);

    MojLogNotice(s_log, _T("mojodb stopped"));

    return err;
}

MojErr MojDbLunaServiceApp::dropTemp()
{
    MojStatT stat;
    MojErr err = MojStat(TempInitStateFile, &stat);
    MojErrCatch(err, MojErrNotFound) {
        err = m_tempService.db().drop(TempDir);
        MojErrCheck(err);
        err = m_tempService.openDb(m_env.get(), m_dbDir, TempDir);
        MojErrCheck(err);
        err = MojCreateDirIfNotPresent(TempStateDir);
        MojErrCheck(err);
        err = MojFileFromString(TempInitStateFile, _T(""));
        MojErrCheck(err);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::handleArgs(const StringVec& args)
{
    MojErr err = Base::handleArgs(args);
    MojErrCheck(err);

    if (args.size() != 1)
        MojErrThrow(MojErrInvalidArg);
    m_dbDir = args.front();

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::displayUsage()
{
    MojErr err = displayMessage(_T("Usage: %s [OPTION]... db-dir\n"), name().data());
    MojErrCheck(err);

    return MojErrNone;
}

MojDbLunaServiceApp::DbService::DbService(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher)
{
}

MojErr MojDbLunaServiceApp::DbService::init(MojReactor& reactor)
{
    m_handler.reset(new MojDbServiceHandler(m_db, reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::DbService::open(MojGmainReactor& reactor, MojDbEnv* env,
        const MojChar* serviceName, const MojChar* baseDir, const MojChar* subDir)
{
    MojAssert(serviceName && baseDir && subDir);

    // open service
    MojErr err = m_service.open(serviceName);
    MojErrCheck(err);
    err = m_service.attach(reactor.impl());
    MojErrCheck(err);
    // open handler
    err = m_handler->open();
    MojErrCheck(err);
    err = m_service.addCategory(MojDbServiceDefs::Category, m_handler.get());
    MojErrCheck(err);
    // open db
    err = openDb(env, baseDir, subDir);
    if (err != MojErrNone) {
        MojString msg;
        MojErrToString(err, msg);
        MojErrCheck(err);
        MojLogError(s_log, _T("Error opening %s/%s - %s (%d)"), baseDir, subDir, msg.data(), (int) err);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::DbService::openDb(MojDbEnv* env, const MojChar* baseDir, const MojChar* subDir)
{
    MojAssert(env && baseDir && subDir);

    // create engine with shared env
    MojString path;
    MojErr err = path.format(_T("%s/%s"), baseDir, subDir);
    MojErrCheck(err);
//  MojRefCountedPtr<MojDbBerkeleyEngine> engine(new MojDbBerkeleyEngine);
    MojRefCountedPtr<MojDbStorageEngine> engine;
    MojDbStorageEngine::engineFactory()->create(engine);
    MojAllocCheck(engine.get());
    err = engine->open(path, env);
    MojErrCheck(err);
    // open db
    err = m_db.open(path, engine.get());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::DbService::close()
{
    MojErr err = MojErrNone;

    m_handler.reset();
    MojErr errClose = m_service.close();
    MojErrAccumulate(err, errClose);
    errClose = m_db.close();
    MojErrAccumulate(err, errClose);

    return err;
}
