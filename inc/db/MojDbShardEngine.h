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
#include "core/MojSignal.h"
#include "core/MojObject.h"
#include "db/MojDbReq.h"

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    #include <boost/smart_ptr/scoped_ptr.hpp>
#else
    #include <auto_ptr.h>
#endif

#include <list>
#include <algorithm>
#include "db/MojDbShardIdCache.h"
#include "core/MojTime.h"
#include <boost/tokenizer.hpp>

class MojDb;
class MojDbReq;
class MojDbMediaLinkManager;

class MojDbShardEngine : private MojNoCopy
{
public:

    struct KindIdsList
    {
    private:
        std::list<MojString> listKindIds;

    public:

        std::list<MojString>::iterator begin (void)
        {
            return listKindIds.begin();
        }

        std::list<MojString>::iterator end (void)
        {
            return listKindIds.end();
        }

        bool isExist (const MojString& kindId)
        {
            std::list<MojString>::iterator it;
            it = std::find(listKindIds.begin(), listKindIds.end(), kindId);
            return (it != listKindIds.end());
        }

        void add (const MojString& kindId)
        {
            listKindIds.push_back(kindId);
        }

        void remove (const MojString& kindId)
        {
            std::list<MojString>::iterator it;
            it = std::find(listKindIds.begin(), listKindIds.end(), kindId);

            if(it != listKindIds.end())
            {
                listKindIds.erase(it);
            }
        }

        MojErr toString (MojString& str) const
        {
            std::list<MojString>::const_iterator it;

            for (it = listKindIds.begin(); it != listKindIds.end(); ++it)
            {
                if(it != listKindIds.begin())
                {
                    MojErrCheck(str.append(","));
                }

                MojErrCheck(str.append(*it));
            }

            return MojErrNone;
        }

        MojErr fromString (const MojString& str)
        {
            listKindIds.clear();

            if(str.empty())
            {
                return MojErrNone;
            }

            MojString item;
            std::string testStr = str.data();
            boost::char_separator<char> sep(",");
            boost::tokenizer<boost::char_separator<char> > tokens(testStr, sep);
            boost::tokenizer<boost::char_separator<char> >::iterator it;

            for(it = tokens.begin(); it != tokens.end(); ++it)
            {
                MojErrCheck(item.assign((*it).c_str()));
                listKindIds.push_back(item);
            }

            return MojErrNone;
        }
    };

    struct ShardInfo
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

        KindIdsList kindIds;

        ShardInfo(MojUInt32 _id = 0, bool _active = false, bool _transient = false)
        {
            id = _id;
            active = _active;
            transient = _transient;
            timestamp = 0;
        }

        ShardInfo& operator=(const ShardInfo& i_src)
        {
            this->active = i_src.active;
            this->transient = i_src.transient;
            this->id = i_src.id;
            this->id_base64 = i_src.id_base64;
            this->deviceId = i_src.deviceId;
            this->deviceUri = i_src.deviceUri;
            this->mountPath = i_src.mountPath;
            this->deviceName = i_src.deviceName;
            this->kindIds = i_src.kindIds;

            return (*this);
        }
    };

    class PDMSignalWatcher;

    typedef MojSignal<ShardInfo> SignalPdm;
    typedef SignalPdm::Slot<PDMSignalWatcher> SlotPdm;

    class PDMSignalWatcher : public MojSignalHandler
    {
    public:
        PDMSignalWatcher(MojDbShardEngine* shardEngine);
        SlotPdm& getSlot() { return  m_pdmSlot; }
    private:
        MojErr handleShardInfoSlot(ShardInfo pdmShardInfo);
        MojErr processShard(ShardInfo& shardInfo);
        void updateShard(const ShardInfo& from, ShardInfo& to);

        MojDbShardEngine* m_shardEngine;
        SlotPdm m_pdmSlot;
    };


    MojDbShardEngine(void);
    ~MojDbShardEngine(void);


    /**
     * initialize MojDbShardEngine
     */
    MojErr init (const MojObject& conf, MojDb* ip_db, MojDbReqRef req = MojDbReq());

    /**
    * put a new shard description to db
    */
    MojErr put (const ShardInfo& shardInfo, MojDbReqRef req = MojDbReq());

    /**
     * get shard description by id
     */
    MojErr get (MojUInt32 shardId, ShardInfo& shardInfo, bool& found);

    /**
     * get shard description by uuid
     */
    MojErr getByDeviceUuid (const MojString& deviceUuid, ShardInfo& shardInfo, bool& found);

    /**
     * get list of all active shards
     */
    MojErr getAllActive (std::list<ShardInfo>& shardInfo, MojUInt32& count, MojDbReqRef req = MojDbReq());

    /**
     * update shardInfo
     */
    MojErr update (const ShardInfo& i_shardInfo, MojDbReqRef req = MojDbReq());

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
    static MojErr convert (const ShardInfo& i_shardInfo, MojObject& o_obj);

    /**
     * convert MojObject to device info
     */
    static MojErr convert (const MojObject& i_obj, ShardInfo& o_shardInfo);

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

    MojErr connectPdmServiceSignal(SignalPdm* signal);

private:
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
     * update ShardInfo::timestamp with current time value
     */
    MojErr updateTimestamp (ShardInfo& shardInfo);

    /**
     * remove shard info record (from db + cache)
     */
    MojErr removeShardInfo (const MojUInt32 shardId);

    std::auto_ptr<MojDbMediaLinkManager> m_mediaLinkManager;
    MojDb* mp_db;
    MojDbShardIdCache m_cache;

public:
    PDMSignalWatcher m_pdmWatcher;
};

#endif /* MOJDBSHARDENGINE_H_ */
