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


#ifndef MOJDBSHARDENGINE_H_
#define MOJDBSHARDENGINE_H_

#include "core/MojCoreDefs.h"
#include "core/MojErr.h"
#include "core/MojString.h"
#include "core/MojObject.h"
#include "core/MojTime.h"
#include "db/MojDbDefs.h"
#include "db/MojDbReq.h"
#include "db/MojDbShardIdCache.h"
#include "db/MojDbShardInfo.h"
#include "db/MojDbMediaLinkManager.h"

class MojDbShardEngine : private MojNoCopy
{
public:
    MojDbShardEngine(MojDb& db);
    ~MojDbShardEngine();


    /**
     * initialize MojDbShardEngine
     */
    MojErr init (const MojObject& conf, MojDbReqRef req = MojDbReq());

    /**
    * put a new shard description to db
    */
    MojErr put (const MojDbShardInfo& shardInfo, MojDbReqRef req = MojDbReq());

    /**
     * get shard description by id
     */
    MojErr get (MojUInt32 shardId, MojDbShardInfo& shardInfo, bool& found);

    /**
     * get shard description by uuid
     */
    MojErr getByDeviceUuid (const MojString& deviceUuid, MojDbShardInfo& shardInfo, bool& found);

    /**
     * get list of all active shards
     */
    MojErr getAllActive (std::list<MojDbShardInfo>& shardInfo, MojUInt32& count, MojDbReqRef req = MojDbReq());

    /**
     * update shardInfo
     */
    MojErr update (const MojDbShardInfo& i_shardInfo, MojDbReqRef req = MojDbReq());

    /**
     * get device id by uuid
     *
     * search within db for i_deviceId, return id if found
     * else
     * allocate a new id
     */
    MojErr getShardId (const MojString& deviceUuid, MojUInt32& shardId);

    /**
     * is device id exist?
     */
    MojErr isIdExist (MojUInt32 shardId, bool& found);

    /**
     * convert device id to base64 string
     */
    static MojErr convertId (const MojUInt32 i_id, MojString& o_id_base64);

    /**
     * convert base64 string to device id
     */
    static MojErr convertId (const MojString& i_id_base64, MojUInt32& o_id);

    /**
     * convert device info to MojObject
     */
    static MojErr convert (const MojDbShardInfo& i_shardInfo, MojObject& o_obj);

    /**
     * convert MojObject to device info
     */
    static MojErr convert (const MojObject& i_obj, MojDbShardInfo& o_shardInfo);

    /**
     * Support garbage collection of obsolete shards
     * remove shard objects by vector of id's
     */
    MojErr removeShardObjects (const MojVector<MojUInt32>& arrShardIds, MojDbReqRef req = MojDbReq());

    /**
     * Support garbage collection of obsolete shards
     * remove shard objects by shard id
     */
    MojErr removeShardObjects (const MojString& strShardIdToRemove, MojDbReqRef req = MojDbReq());

    /**
     * Support garbage collection of obsolete shards
     * remove shard objects older <numDays> days
     */
    MojErr purgeShardObjects (MojInt64 numDays, MojDbReqRef req = MojDbReq());

    /**
     * Add kind id to shardInfo
     */
    MojErr linkShardAndKindId (const MojString& shardIdBase64, const MojString& kindId, MojDbReqRef req = MojDbReq());
    MojErr linkShardAndKindId (const MojUInt32 shardId, const MojString& kindId, MojDbReqRef req = MojDbReq());

    /**
     * Remove kind id from shardInfo
     */
    MojErr unlinkShardAndKindId (const MojString& shardIdBase64, const MojString& kindId, MojDbReqRef req = MojDbReq());
    MojErr unlinkShardAndKindId (const MojUInt32 shardId, const MojString& kindId, MojDbReqRef req = MojDbReq());

    MojErr processShardInfo(const MojDbShardInfo& shardInfo);

private:
    MojErr copyRequiredFields(const MojDbShardInfo& from, MojDbShardInfo& to);
    MojErr removeTransientShard(const MojDbShardInfo& shardInfo);

    /**
     * Configure shard engine after init
     */
    MojErr configure(const MojObject& conf);

    /**
     * compute a new shard id
     */
    MojErr allocateId (const MojString& deviceUuid, MojUInt32& shardId);

    /**
     * compute device id by media uuid
     */
    MojErr computeId (const MojString& mediaUuid, MojUInt32& sharId);

    /**
    * all devices should not be active at startup:
    * - reset 'active flag'
    * - reset 'mountPath'
    */
    MojErr resetShards (MojDbReq& io_req);

    /**
     * init cache
     */
    MojErr initCache (MojDbReq& io_req);

    /**
     * remove all records for shard within kind
     */
    MojErr removeShardKindObjects (const MojUInt32 shardId, const MojString& kindId, MojDbReq& req);

    /**
     * update MojDbShardInfo::timestamp with current time value
     */
    MojErr updateTimestamp (MojDbShardInfo& shardInfo);

    /**
     * remove shard info record (from db + cache)
     */
    MojErr removeShardInfo (const MojUInt32 shardId);

    MojDbMediaLinkManager m_mediaLinkManager;
    MojDb& m_db;
    MojDbShardIdCache m_cache;
};

#endif /* MOJDBSHARDENGINE_H_ */
