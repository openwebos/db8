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

#ifndef MOJDBMEDIAHANDLER_H
#define MOJDBMEDIAHANDLER_H

#include "core/MojSignal.h"
#include "core/MojObject.h"
#include "db/MojDbDefs.h"
#include "db/MojDbShardEngine.h"
#include "luna/MojLunaService.h"
#include "luna/MojLunaRequest.h"

#include <map>
#include <set>

class MojDbMediaHandler : public MojSignalHandler
{
    typedef std::map<MojString, MojDbShardEngine::ShardInfo> shard_cache_t;

public:
    MojDbMediaHandler(MojService& service, MojDb& db);
    MojErr subscribe();

private:
    MojErr convert(const MojObject& object, MojDbShardEngine::ShardInfo& shardInfo);

    MojErr handleDeviceListResponse(MojObject& payload, MojErr errCode);

    bool existInCache(const MojString& id);
    void copyShardCache(std::set<MojString>*);

    MojServiceRequest::ReplySignal::Slot<MojDbMediaHandler> m_deviceListSlot;
    MojRefCountedPtr<MojServiceRequest> m_subscription;
    MojService& m_service;
    MojDb& m_db;

    shard_cache_t m_shardCache;
};

#endif
