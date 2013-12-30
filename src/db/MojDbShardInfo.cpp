/* @@@LICENSE
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
* LICENSE@@@ */

#include "db/MojDbShardInfo.h"

MojDbShardInfo::MojDbShardInfo(MojUInt32 _id, bool _active, bool _transient)
{
    id = _id;
    active = _active;
    transient = _transient;
    timestamp = 0;
}

MojDbShardInfo& MojDbShardInfo::operator=(const MojDbShardInfo& i_src)
{
    id = i_src.id;
    id_base64 = i_src.id_base64;
    active = i_src.active;
    transient = i_src.transient;
    deviceId = i_src.deviceId;
    deviceUri = i_src.deviceUri;
    mountPath = i_src.mountPath;
    deviceName = i_src.deviceName;
    kindIds = i_src.kindIds;

    return *this;
}
