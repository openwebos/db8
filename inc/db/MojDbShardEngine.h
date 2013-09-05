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

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    #include <boost/smart_ptr/scoped_ptr.hpp>
#else
    #include <auto_ptr.h>
#endif

#include <list>

class MojDb;
class MojDbReq;
class MojDbMediaLinkManager;

class MojDbShardEngine : private MojNoCopy
{
public:
    struct ShardInfo
    {
        bool active;
        bool transient;
        MojUInt32 id;
        MojString id_base64;
        MojString deviceId;
        MojString deviceUri;
        MojString mountPath;
        MojString deviceName;

        ShardInfo(MojUInt32 _id = 0, bool _active = false, bool _transient = false)
        {
            id = _id;
            active = _active;
            transient = _transient;
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
            return (*this);
        }
    };

    typedef MojSignal<ShardInfo> ShardInfoSignal;

    class Watcher : public MojSignalHandler
    {
    public:
        Watcher(MojDbShardEngine* shardEngine);
        MojErr handleShardInfoSlot(ShardInfo pdmShardInfo);

        MojDbShardEngine* m_shardEngine;
        ShardInfoSignal::Slot<Watcher> m_pdmSlot;
    };


    MojDbShardEngine(void);
    ~MojDbShardEngine(void);

    MojErr init (MojDb* ip_db, MojDbReq &req);
    MojErr put (const ShardInfo& shardInfo);
    MojErr get (MojUInt32 shardId, ShardInfo& shardInfo, bool& found);
    MojErr getByDeviceUuid (const MojString& deviceUuid, ShardInfo& shardInfo, bool& found);
    MojErr getAllActive (std::list<ShardInfo>& shardInfo, MojUInt32& count);
    MojErr update (const ShardInfo& i_shardInfo);
    MojErr getShardId (const MojString& deviceUuid, MojUInt32& shardId);
    MojErr isIdExist (MojUInt32 shardId, bool& found);

    static MojErr convertId (const MojUInt32 i_id, MojString& o_id_base64);
    static MojErr convertId (const MojString& i_id_base64, MojUInt32& o_id);
private:
    MojErr allocateId (const MojString& deviceUuid, MojUInt32& shardId);
    MojErr computeId (const MojString& mediaUuid, MojUInt32& sharId);

    std::auto_ptr<MojDbMediaLinkManager> m_mediaLinkManager;
    MojDb* mp_db;
    static MojLogger s_log;

public:
    Watcher m_pdmWatcher;
};

#endif /* MOJDBSHARDENGINE_H_ */
