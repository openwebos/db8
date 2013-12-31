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
 *  @file MojFactoryReset.h
 */

#ifndef __MOJFACTORYRESET_H
#define __MOJFACTORYRESET_H

#include "core/MojSignal.h"
#include "core/MojServiceRequest.h"
#include "luna/MojLunaService.h"

// Object to handle factory reset request
class MojFactoryReset : public MojSignalHandler
{
public:
    MojFactoryReset();

    MojErr perform(MojLunaService& service);

private:
    MojRefCountedPtr<MojServiceRequest> req;
    MojServiceRequest::ReplySignal::Slot<MojFactoryReset> slot;

    //handle factory reset command response
    MojErr handleResetReply (MojObject& obj, MojErr err);
};

#endif
