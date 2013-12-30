/* @@@LICENSE
*
*      Copyright (c) 2013 LG Electronics, Inc.
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

#ifndef MOJDBSHARDINFO_H
#define MOJDBSHARDINFO_H

#include "core/MojCoreDefs.h"
#include "core/MojString.h"
#include "db/MojDbKindIdList.h"

struct MojDbShardInfo
{
    bool active;
    bool transient;
    MojUInt32 id;
    MojInt64 timestamp;
    MojString id_base64;
    MojString deviceId;
    MojString deviceUri;
    MojString mountPath;
    MojString deviceName;

    MojDbKindIdList kindIds;

    MojDbShardInfo(MojUInt32 _id = 0, bool _active = false, bool _transient = false);
    MojDbShardInfo& operator=(const MojDbShardInfo& i_src);
};

#endif
