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

MojLogger MojDbLunaServicePdm::s_log(_T("db-luna.shard"));

MojDbLunaServicePdm::MojDbLunaServicePdm(MojMessageDispatcher& dispatcher)
: m_service(true, &dispatcher)
{
    MojLogTrace(s_log);
}

MojErr MojDbLunaServicePdm::init(MojReactor& reactor)
{
    m_handler.reset(new MojDbLunaServicePdmHandler(reactor));
    MojAllocCheck(m_handler.get());

    return MojErrNone;
}

MojErr MojDbLunaServicePdm::open(MojGmainReactor& reactor, const MojChar* MojdbPDMClientName, const MojChar* pdmServiceName)
{
    MojLogTrace(s_log);
    MojAssert(MojdbPDMClientName);
    MojAssert(pdmServiceName);
    MojLogInfo(s_log, "Open connectoin to PDM");

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
    MojErr err = MojErrNone;

    m_handler.reset();
    MojErr errClose = m_service.close();
    MojErrAccumulate(err, errClose);

    return err;
}
