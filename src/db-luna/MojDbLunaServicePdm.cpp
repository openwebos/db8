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
#include "db-luna/MojDbLunaServicePdm.h"

#include "db/MojDbServiceDefs.h"
#include "core/MojApp.h"
#include <algorithm>

//db-luna.shard
const MojChar* const MojDbLunaServicePdm::ConfKey = _T("pdm");

MojDbLunaServicePdm::MojDbLunaServicePdm(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher),
  m_shardInfoSignal(0)
{
}

MojErr MojDbLunaServicePdm::configure(const MojObject& conf)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err;
    MojObject pdmConf;

    if (conf.get(ConfKey, pdmConf)) {
        bool found = pdmConf.get(_T("enabled"), m_isEnabled);
        if (!found) {
            m_isEnabled = true;
        }
    } else {
        m_isEnabled = true;
    }

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::init(MojReactor& reactor)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(!m_shardInfoSignal.get());
    m_shardInfoSignal.reset(new MojDbShardEngine::SignalPdm(this));
    MojAllocCheck(m_shardInfoSignal.get());

    m_handler.reset(new MojDbLunaServicePdmHandler(this, reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::open(MojGmainReactor& reactor, const MojChar* MojdbPDMClientName, const MojChar* pdmServiceName)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojAssert(MojdbPDMClientName);
    MojAssert(pdmServiceName);
    LOG_DEBUG("[db-luna_shard] Open connectoin to PDM");

    // open service
    MojErr err = m_service.open(MojdbPDMClientName);
    MojErrCheck(err);
    err = m_service.attach(reactor.impl());
    MojErrCheck(err);

    LOG_DEBUG("[db-luna_shard] Open PDM handler");
    // open handler
    err = m_handler->open(&m_service, pdmServiceName);
    MojErrCheck(err);
    //err = m_service.addCategory(MojDbServiceDefs::Category, m_handler.get());
    //MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(m_shardInfoSignal.get());

    MojErr err = MojErrNone;

    m_shardInfoSignal.release();
    m_handler.reset();
    MojErr errClose = m_service.close();
    MojErrAccumulate(err, errClose);

    return err;
}

MojErr MojDbLunaServicePdm::notifyShardEngine(const MojDbShardEngine::ShardInfo& shardInfo)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (m_shardInfoSignal.get()) {
        LOG_DEBUG("[db-luna_shard] Notify Shard Engine about new shard");
        return m_shardInfoSignal->call(shardInfo);
    } else {
        LOG_DEBUG("[db-luna_shard] Can't notify shard engine. Shard engine died");
    }

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::addShardEngine(MojDbShardEngine* shardEngine)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(m_shardInfoSignal.get());
    LOG_DEBUG("[db-luna_shard] Add shard engine to luna service pdm");

    MojErr err;
    err = shardEngine->connectPdmServiceSignal(m_shardInfoSignal.get());
    MojErrCheck(err);

    return MojErrNone;
}
