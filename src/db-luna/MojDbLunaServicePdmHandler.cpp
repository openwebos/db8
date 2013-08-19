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
#include "db-luna/MojDbLunaServicePdmHandler.h"

#include "luna/MojLunaService.h"
#include "core/MojServiceMessage.h"
#include "core/MojService.h"

MojLogger MojDbLunaServicePdmHandler::s_log(_T("db-luna.shard"));

MojDbLunaServicePdmHandler::MojDbLunaServicePdmHandler(MojReactor& reactor)
: m_dbErr(MojErrNone),
m_callbackInvoked (false),
m_slot(this, &MojDbLunaServicePdmHandler::handleResult),
m_reactor(reactor)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::MojDbLunaServicePdmHandler"));
}

MojErr MojDbLunaServicePdmHandler::open(MojLunaService* service, const MojChar* pdmServiceName)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, "Create subscribe request to PDM service");
    MojErr err;

    // send request
    MojRefCountedPtr<MojServiceRequest> req;
    err = service->createRequest(req);
    MojErrCheck(err);

    MojObject payload;
    err = payload.fromJson("{\"subscribe\":true}");
    MojErrCheck(err);

    MojLogDebug(s_log, "Send subscribe request to PDM service");
    err = req->send(m_slot, pdmServiceName, _T("listDevices"), payload, 777);   // @TODO: FIX THIS MAGIC NUMBER!!!!!!!
    MojErrCheck(err);

    /*std::cerr << "Wait" << std::endl;
     m_mediaClient->wait(m_service.get());*/


    return MojErrNone;
}

MojErr MojDbLunaServicePdmHandler::handleResult(MojObject& result, MojErr errCode)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::handleResult"));

    m_callbackInvoked = true;
    m_result = result;
    m_dbErr = errCode;
    if (errCode != MojErrNone) {
        bool found = false;
        MojErr err = result.get(MojServiceMessage::ErrorTextKey, m_errTxt, found);

        MojLogError(s_log, _T("Luna error response: %s"), m_errTxt.data());
        MojAssert(found);
    } else {
        MojString resStr;
        result.toJson(resStr);

        MojLogDebug(s_log, _T("Device list: %s"), resStr.data());
    }
    return MojErrNone;
}

void MojDbLunaServicePdmHandler::reset()
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::reset"));

    m_dbErr = MojErrNone;
    m_callbackInvoked = false;
    m_errTxt.clear();
    m_result.clear();
}

MojErr MojDbLunaServicePdmHandler::wait(MojService* service)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::wait"));

    while (!m_callbackInvoked) {
        MojErr err = service->dispatch();
        MojErrCheck(err);
    }
    return MojErrNone;
}

