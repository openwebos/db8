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
#ifndef MOJDBLUNASERVICEPDMHANDLER_H
#define MOJDBLUNASERVICEPDMHANDLER_H

#include "core/MojString.h"
#include "core/MojObject.h"
#include "core/MojSignal.h"
#include "core/MojServiceRequest.h"
#include "core/MojReactor.h"

class MojLunaService;

class MojDbLunaServicePdmHandler : public MojSignalHandler
{
public:
    MojDbLunaServicePdmHandler(MojReactor& mojService);
    MojErr open(MojLunaService* service, const MojChar* pdmServiceName);

    virtual MojErr handleResult(MojObject& result, MojErr errCode);
    void reset();
    MojErr wait(MojService* service);

    // added for compatible
    MojService* service();

private:
    MojErr m_dbErr;
    MojString m_errTxt;
    MojObject m_result;
    bool m_callbackInvoked;

    MojReactor& m_reactor;
    MojServiceRequest::ReplySignal::Slot<MojDbLunaServicePdmHandler> m_slot;

    static MojLogger s_log;
};

#endif
