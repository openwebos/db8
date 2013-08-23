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
#include <list>

class MojDb;

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

        ShardInfo()
        {
            id = 0;
            active = false;
            transient = false;
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

    MojDbShardEngine(void);
    ~MojDbShardEngine(void);

    //init
    MojErr init (MojDb* ip_db);

    //put a new shard description to db
    MojErr put (ShardInfo& i_info);

    //get shard description by id
    MojErr get (MojUInt32 i_id, ShardInfo& o_info);
    MojErr get (MojString& i_id_base64, ShardInfo& o_info);

    //get correspond shard id using path to device
    MojErr getIdForPath (MojString& i_path, MojUInt32& o_id);

    //get list of all active shards
    MojErr getAllActive (std::list<ShardInfo>& o_list, MojUInt32& o_count);

    //set shard activity
    MojErr setActivity (MojUInt32 i_id, bool i_isActive);

    //compute a new shard id
    MojErr computeShardId (MojString& i_media, MojUInt32& o_id);

    //return: MojErrExists         --> exist
    //        MojErrNotFound       --> not found
    //        MojErrNotInitialized --> db was not initialized
    MojErr isIdExist (MojUInt32 i_id);

    //return: MojErrExists         --> exist
    //        MojErrNotFound       --> not found
    //        MojErrNotInitialized --> db was not initialized
    MojErr isIdExist (MojString& i_id_base64);

private:
    MojDb* mp_db;
    static MojLogger s_log;

    bool _computeId (MojString& i_media, MojUInt32& o_id);
    MojErr _get (MojUInt32 i_id, MojString& i_id_base64, ShardInfo& o_info);
};

#endif /* MOJDBSHARDENGINE_H_ */
