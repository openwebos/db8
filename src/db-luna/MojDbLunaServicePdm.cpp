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

MojLogger MojDbLunaServicePdm::s_log(_T("db-luna.shard"));
const MojChar* const MojDbLunaServicePdm::ConfKey = _T("pdm");

MojDbLunaServicePdm::MojDbLunaServicePdm(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher),
  m_shardInfoSignal(0)
{
    MojLogTrace(s_log);
}

MojErr MojDbLunaServicePdm::configure(const MojObject& conf)
{
    MojLogTrace(s_log);

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
    MojAssert(!m_shardInfoSignal.get());
    m_shardInfoSignal.reset(new MojDbShardEngine::SignalPdm(this));
    MojAllocCheck(m_shardInfoSignal.get());

    m_handler.reset(new MojDbLunaServicePdmHandler(this, reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::open(MojGmainReactor& reactor, const MojChar* MojdbPDMClientName, const MojChar* pdmServiceName)
{
    MojLogTrace(s_log);
    MojAssert(MojdbPDMClientName);
    MojAssert(pdmServiceName);
    MojLogDebug(s_log, "Open connectoin to PDM");

    // open service
    MojErr err = m_service.open(MojdbPDMClientName);
    MojErrCheck(err);
    err = m_service.attach(reactor.impl());
    MojErrCheck(err);

    MojLogDebug(s_log, "Open PDM handler");
    // open handler
    err = m_handler->open(&m_service, pdmServiceName);
    MojErrCheck(err);
    //err = m_service.addCategory(MojDbServiceDefs::Category, m_handler.get());
    //MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::close()
{
    MojLogTrace(s_log);
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
    MojLogTrace(s_log);
    if (m_shardInfoSignal.get()) {
        MojLogDebug(s_log, "Notify Shard Engine about new shard");
        return m_shardInfoSignal->call(shardInfo);
    } else {
        MojLogDebug(s_log, "Can't notify shard engine. Shard engine died");
    }

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::addShardEngine(MojDbShardEngine* shardEngine)
{
    MojLogTrace(s_log);
    MojAssert(m_shardInfoSignal.get());
    MojLogDebug(s_log, "Add shard engine to luna service pdm");

    MojErr err;
    err = shardEngine->connectPdmServiceSignal(m_shardInfoSignal.get());
    MojErrCheck(err);

    return MojErrNone;
}
