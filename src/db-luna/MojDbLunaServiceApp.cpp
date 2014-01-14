/* @@@LICENSE
*
* Copyright (c) 2009-2014 LG Electronics, Inc.
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


#include "db-luna/MojDbLunaServiceApp.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbServiceHandler.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#else
#error "Set database type"
#endif

#ifndef MOJ_VERSION_STRING
#define MOJ_VERSION_STRING NULL
#endif

const MojChar* const MojDbLunaServiceApp::VersionString = MOJ_VERSION_STRING;

int main(int argc, char** argv)
{
   MojAutoPtr<MojDbLunaServiceApp> app(new MojDbLunaServiceApp);
   MojAllocCheck(app.get());
   return app->main(argc, argv);
}

MojDbLunaServiceApp::MojDbLunaServiceApp()
: MojReactorApp<MojGmainReactor>(MajorVersion, MinorVersion, VersionString)
, m_mainService(m_dispatcher)
{
   // set up db first
#ifdef MOJ_USE_BDB
   MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
   MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#else
  #error "Database not set"
#endif
}

MojDbLunaServiceApp::~MojDbLunaServiceApp()
{
}

MojErr MojDbLunaServiceApp::init()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = Base::init();
	MojErrCheck(err);

    MojDbStorageEngine::createEnv(m_env);
    MojAllocCheck(m_env.get());

    m_internalHandler.reset(new MojDbServiceHandlerInternal(m_mainService.db(), m_reactor, m_mainService.service()));
    MojAllocCheck(m_internalHandler.get());

    err = m_mainService.init(m_reactor);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::configure(const MojObject& conf)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err = Base::configure(conf);
    MojErrCheck(err);

    MojObject engineConf;

    conf.get(MojDbStorageEngine::engineFactory()->name(), engineConf);
    err = m_env->configure(engineConf);
    MojErrCheck(err);
    err = m_mainService.db().configure(conf);
    MojErrCheck(err);
    m_conf = engineConf;

    MojObject dbConf;
    err = conf.getRequired("db", dbConf);
    MojErrCheck(err);

    MojObject dbPath;
    err = dbConf.getRequired("path", dbPath);
    MojErrCheck(err);
    err = dbPath.stringValue(m_dbDir);
    MojErrCheck(err);

    MojObject serviceName;
    err = dbConf.getRequired("service_name", serviceName);
    MojErrCheck(err);
    err = serviceName.stringValue(m_serviceName);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceApp::open()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("[mojodb] starting...");

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
    err = m_mainService.open(m_reactor, m_env.get(), m_serviceName, m_dbDir, m_conf);
    MojErrCatchAll(err) {
		dbOpenFailed = true;
	}

	// open internal handler
	err = m_internalHandler->open();
	MojErrCheck(err);
	err = m_mainService.service().addCategory(MojDbServiceDefs::InternalCategory, m_internalHandler.get());
	MojErrCheck(err);
	err = m_internalHandler->configure(dbOpenFailed);
	MojErrCheck(err);

    err = m_internalHandler->subscribe();
    MojErrCheck(err);

    LOG_DEBUG("[mojodb] started");

	return MojErrNone;
}

MojErr MojDbLunaServiceApp::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("[mojodb] stopping...");

	// stop dispatcher
	MojErr err = MojErrNone;
	MojErr errClose = m_dispatcher.stop();
	MojErrAccumulate(err, errClose);
	errClose = m_dispatcher.wait();
	MojErrAccumulate(err, errClose);
	// close services
	errClose = m_mainService.close();
	MojErrAccumulate(err, errClose);

	m_internalHandler->close();
	m_internalHandler.reset();

	errClose = Base::close();
	MojErrAccumulate(err, errClose);

    LOG_DEBUG("[mojodb] stopped");

	return err;
}

MojErr MojDbLunaServiceApp::handleArgs(const StringVec& args)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = Base::handleArgs(args);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbLunaServiceApp::displayUsage()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err = displayMessage(_T("Usage: %s [OPTION]...\n"), name().data());
    MojErrCheck(err);

	return MojErrNone;
}
