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


#ifndef MOJDBSHARDMANAGERTEST_H_
#define MOJDBSHARDMANAGERTEST_H_

#include "MojDbTestRunner.h"
#include "db/MojDbShardEngine.h"
#include "db/MojDbShardIdCache.h"

class MojDbShardManagerTest : public MojTestCase
{
public:
    MojDbShardManagerTest();

    virtual MojErr run(void);

private:
    /**
     * test cache indexes
     */
    MojErr testShardIdCacheIndexes (MojDbShardIdCache* ip_cache);

    /**
     * test cache operations
     */
    MojErr testShardIdCacheOperations (MojDbShardIdCache* ip_cache);

    /**
     * test engine
     */
    MojErr testShardEngine (MojDbShardEngine* ip_eng);

    /**
     * test operations with shards
     */
    MojErr testShardCreateAndRemoveWithRecords (MojDb& db);

    /**
     * create objects for shard 1
     */
    MojErr createShardObjects1 (MojDb& db, MojDbShardInfo& shard);

    /**
     * create objects for shard 2
     */
    MojErr createShardObjects2 (MojDb& db, MojDbShardInfo& shard);

    /**
     * add kind to db
     */
    MojErr addKind (const MojChar* strKind, MojDb& db);

    /**
     * is kind exist?
     */
    MojErr verifyKindExistance (MojString kindId, MojDb& db);

    /**
     * get number of records for kind
     */
    MojErr verifyRecords (const MojChar* strKind, MojDb& db, const MojDbShardInfo& shard, MojUInt32& count);

    /**
     * is shard exist?
     */
    MojErr verifyShardExistance (MojDb& db, const MojDbShardInfo& shard);

    /**
     * generate dummy shard info
     */
    MojErr generateItem (MojDbShardInfo& o_shardInfo);

    /**
     * display message
     */
    MojErr displayMessage(const MojChar* format, ...);

    /**
     * cleanup
     */
    void cleanup(void);
};

#endif /* MOJDBSHARDMANAGER_H_ */
