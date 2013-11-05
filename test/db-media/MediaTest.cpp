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

/**
 * ***************************************************************************************************
 * @Filename             : MediaTest.cpp
 * @Description          : Source file for Media testcases.
 ****************************************************************************************************
 */

#include "gtest/gtest.h"

#include "db/MojDb.h"

#include "core/MojGmainReactor.h"
#include "luna/MojLunaService.h"
#include "core/MojGmainReactor.h"
#include "core/MojServiceRequest.h"
#include "core/MojErr.h"
#include "core/MojServiceMessage.h"

#include "Runner.h"

#include <iostream>
#include <list>

using namespace std;


const MojChar* ServiceName = _T("com.webos.service.attachedstoragemanager");    // PDM servicename

struct MediaSuite : public ::testing::Test
{
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};


/**
 * **************************************************************************************************
 * @class                   MojPdmClientHandler
 *                          Handle all luna request/response from PDM
 *
 ***************************************************************************************************
 */
class MojPdmClientHandler : public MojSignalHandler
{
public:
    MojPdmClientHandler(MojService* mojService)
    : m_dbErr(MojErrNone),
    m_callbackInvoked (false),
    m_slot(this, &MojPdmClientHandler::handleResult),
    m_service(mojService)
    {
        std::cout << "MojPdmClientHandler::MojPdmClientHandler" << std::endl;
    }

    virtual MojErr handleResult(MojObject& result, MojErr errCode)
    {
        std::cout << "MojPdmClientHandler::handleResult" << std::endl;

        m_callbackInvoked = true;
        m_result = result;
        m_dbErr = errCode;
        if (errCode != MojErrNone) {

            std::cout << "ERROR" << std::endl;

            bool found = false;
            MojErr err = result.get(MojServiceMessage::ErrorTextKey, m_errTxt, found);

            std::cout << m_errTxt << std::endl;

            //MojTestErrCheck(err);
            //MojTestAssert(found);
        } else {
            std::cout << "found" << std::endl;
            MojString resStr;
            result.toJson(resStr);
            std::cout << resStr << std::endl;
        }
        return MojErrNone;
    }

    void reset()
    {
        m_dbErr = MojErrNone;
        m_callbackInvoked = false;
        m_errTxt.clear();
        m_result.clear();
    }

    MojErr wait(MojService* service)
    {
        std::cout << "MojPdmClientHandler::wait" << std::endl;

        while (!m_callbackInvoked) {
            MojErr err = service->dispatch();
            MojErrCheck(err);
        }
        return MojErrNone;
    }

    MojErr m_dbErr;
    MojString m_errTxt;
    MojObject m_result;
    bool m_callbackInvoked;

    MojServiceRequest::ReplySignal::Slot<MojPdmClientHandler> m_slot;

    // added for compatible
    MojService* service() { return m_service; }

private:
    MojService* m_service;
};


/**
 * **************************************************************************************************
 * @run                   Test cases for ensuring the PDM attach/detach logic works well.
 *
 *                        This test subscribes for notifications from PDM about attaching/detaching of
 *                        medie devices.
 *
 *                        If PDM works, MojPdmClientHandler::handleResult will be called and MojObject& result
 *                        will have all information about media from PDM
 * @param                : None
 ***************************************************************************************************
 **/
TEST_F(MediaSuite, mediaSubscribe)
{
    MojAutoPtr<MojService> m_service;
    MojAutoPtr<MojPdmClientHandler> m_mediaClient;
    MojAutoPtr<MojReactor> m_reactor;

    MojErr err;

    MojAutoPtr<MojGmainReactor> reactor(new MojGmainReactor);
    ASSERT_TRUE(reactor.get());
    err = reactor->init();
    MojAssertNoErr(err);

    MojAutoPtr<MojLunaService> svc(new MojLunaService);
    ASSERT_TRUE(svc.get());
    err = svc->open(_T("mojodbmedia-test"));
    MojAssertNoErr(err);
    err = svc->attach(reactor->impl());
    MojAssertNoErr(err);

    m_reactor = reactor;
    m_service = svc;
    m_mediaClient.reset(new MojPdmClientHandler(m_service.get()));

    std::cout << _T("Create request") << std::endl;
    // send request
    MojRefCountedPtr<MojServiceRequest> req;
    err = m_service->createRequest(req);
    MojAssertNoErr(err);

    std::cout << _T("Send request") << std::endl;
    MojObject payload;
    err = payload.fromJson("{\"subscribe\":true}");

    MojAssertNoErr(err);
    err = req->send(m_mediaClient->m_slot, ServiceName, _T("listDevices"), payload);
    MojAssertNoErr(err);

    std::cout << _T("Wait") << std::endl;
    m_mediaClient->wait(m_service.get());

    std::cout << _T("End of test") << std::endl;

    m_service.release();
    m_mediaClient.release();
}
