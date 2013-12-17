/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013-2014 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

#include "db-luna/MojDbLunaServiceDb.h"
#include "db/MojDbServiceDefs.h"
#include "core/MojApp.h"
#include "core/MojComp.h"
#include "core/MojUtil.h"
#include <cstring>
#include <stdlib.h>

//db-luna.dbservice

MojDbLunaServiceDb::MojDbLunaServiceDb(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher)
{
}

MojErr MojDbLunaServiceDb::init(MojReactor& reactor)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    m_handler.reset(new MojDbServiceHandler(m_db, reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::open(MojGmainReactor& reactor, MojDbEnv* env,
                                const MojChar* serviceName, const MojChar* dir, const MojObject& conf)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

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

    LOG_DEBUG("[MojDb] service name: %s", serviceName);

    // open db
    err = openDb(env, dir, conf);
    if (err != MojErrNone) {
        MojString msg;
        MojErrToString(err, msg);
        LOG_WARNING(MSGID_LUNA_SERVICE_DB_OPEN,
                      3,
                      PMLOGKS("dbdir", dir),
                      PMLOGKS("dbdata", msg.data()),
                      PMLOGKFV("dberror", "%d", err),
                      "Can't open database");

        err = recoverDb(env, dir, conf);
        MojErrCheck(err);
    }

    return err;
}

MojErr MojDbLunaServiceDb::recoverDb(MojDbEnv* env, const MojChar* dir, const MojObject& conf)
{
    MojErr err;

    if (m_recoveryScriptPath.empty()) {
        LOG_ERROR(MSGID_LUNA_SERVICE_DB_OPEN,0, "No recovery script, close database");
        return MojErrDbFatal;
    }

    int recoveryResult = system(m_recoveryScriptPath.data());
    if (recoveryResult != 0) {
        LOG_ERROR(MSGID_LUNA_SERVICE_DB_OPEN,
                  3,
                  PMLOGKS("recoveryScript", m_recoveryScriptPath.data()),
                  PMLOGKFV("scriptExecResult", "%i", recoveryResult),
                  PMLOGKS("dbdir", dir),
                  "Can't recovery database with help of recovery script");

        return MojErrDbFatal;   // close database and stop process
    }

    LOG_INFO(MSGID_LUNA_SERVICE_DB_OPEN, 0, "Recovery database success, reopen database" );
    err = openDb(env, dir, conf);

    if (err != MojErrNone) {
        MojString msg;
        MojErrToString(err, msg);
        LOG_ERROR(MSGID_LUNA_SERVICE_DB_OPEN, 3,
                  PMLOGKS("dbdir", dir),
                  PMLOGKS("dbdata", msg.data()),
                  PMLOGKFV("dberror", "%d", err),
                  "Error re-opening database after recovery");

        return err;
    }

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::openDb(MojDbEnv* env, const MojChar* dir, const MojObject& conf)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(env && dir);

    MojErr err;
    //  MojRefCountedPtr<MojDbBerkeleyEngine> engine(new MojDbBerkeleyEngine);
    MojRefCountedPtr<MojDbStorageEngine> engine;
    MojDbStorageEngine::engineFactory()->create(engine);
    MojAllocCheck(engine.get());
    err = engine->configure(conf);
    MojErrCheck(err);
    err = engine->open(dir, env);
    MojErrCheck(err);

    // open db
    err = m_db.open(dir, engine.get());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::configure(const MojObject& conf)
{
    MojErr err;

    err = conf.getRequired("recoveryScriptPath", m_recoveryScriptPath);
    MojErrCheck(err);

    // check if bash scipt exist and have exec rights
    if (access(m_recoveryScriptPath.data(), X_OK) != 0) {
        LOG_WARNING(MSGID_LUNA_SERVICE_DB_OPEN,
                  1,
                  PMLOGKS("recoveryScriptPath", m_recoveryScriptPath.data()),
                  "Can't find recovery script or no exec right");

        m_recoveryScriptPath.clear();
    }

    return MojErrNone;
}

MojErr MojDbLunaServiceDb::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojErr err = MojErrNone;

    m_handler.reset();
    MojErr errClose = m_service.close();
    MojErrAccumulate(err, errClose);
    errClose = m_db.close();
    MojErrAccumulate(err, errClose);

    return err;
}
