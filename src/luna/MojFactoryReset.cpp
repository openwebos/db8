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
 *  @file MojFactoryReset.cpp
 */

#include "core/MojString.h"
#include "core/MojService.h"
#include "core/MojLogDb8.h"

#include "luna/MojLunaService.h"
#include "luna/MojFactoryReset.h"

MojFactoryReset::MojFactoryReset() :
    slot(this, &MojFactoryReset::handleResetReply)
{
}

//database files are corrupted: do factory reset
//luna-send -n 1 luna://com.webos.service.tv.systemproperty/doUserDefault ‘{}’
MojErr MojFactoryReset::perform(MojLunaService& service)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (req.get())
    {
        LOG_DEBUG("Already requested factory reset (ignoring)");
        return MojErrNone;
    }

    MojErr err = service.createRequest(req, false);
    MojErrCheck(err);

    MojObject payload;
    err = payload.fromJson("{}");
    MojErrCheck(err);

    err = req->send(slot, _T("com.webos.service.tv.systemproperty"), _T("doUserDefault"), payload);
    MojErrCheck(err);

    return MojErrNone;
}


MojErr MojFactoryReset::handleResetReply (MojObject& obj, MojErr err)
{
    MojString s;
    MojErr eerr = obj.toJson(s);
    LOG_INFO("FACTORYRESET_ANSWER", 0, "[MojDb] handle factory reset: (%d) %.*s", err, s.length(), s.data());
    return MojErrNone;
}
