/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
#include "luna/MojFactoryReset.h"
#include <cstring>

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
    // open db
    err = openDb(env, dir, conf);
    if (err != MojErrNone) {
        LOG_DEBUG("[MojDb] service name: %s", serviceName);

        //com.palm.db
        if(strcmp(serviceName, MojDbServiceDefs::ServiceName) == 0)
        {
            LOG_INFO("MAINDB_RESET", 0, "[MojDb] do factory reset");
            m_factoryResetRequest.perform(m_service);
        }

        //com.palm.mediadb
        if(strcmp(serviceName, MojDbServiceDefs::MediaServiceName) == 0)
        {
            LOG_INFO("MEDIADB_RESET", 0, "[MojDb] clean mediadb folder '%s'", dir);
            MojRmDirContent(dir); // remove only content of base folder
            //reopen db
            LOG_INFO("MEDIADB_REOPEN", 0, "[MojDb] reopen mediadb");
            err = openDb(env, dir, conf);
        }

        if (err != MojErrNone)
        {
            MojString msg;
            MojErrToString(err, msg);
            LOG_ERROR(MSGID_LUNA_SERVICE_DB_OPEN, 3,
                PMLOGKS("dir", dir),
                PMLOGKS("data", msg.data()),
                PMLOGKFV("error", "%d", err),
                "Error opening 'dir' - 'data' ('error')");
        }
    }
    MojErrCheck(err);

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
