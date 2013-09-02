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


#include "MojDbShardManagerTest.h"
#include "db/MojDb.h"
#include "db/MojDbShardEngine.h"
#include "db/MojDbShardIdCache.h"
#include <stdarg.h>

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else
#error "Doesn't specified database type. See macro MOJ_USE_BDB and MOJ_USE_LDB"
#endif

#define SHARDID_CACHE_SIZE  100
#define SHARD_ITEMS_NUMBER  10

/**
 * Test description
 *
 * This test is cover functionality of two classes: MojDbShardIdCache and MojDbShardEngine.
 *
 * test scenario for MojDbShardIdCache:
 * ------------------------------------
 * - generate a new id's (=SHARDID_CASH_SIZE);
 * - put all new generated id's to cache;
 * - read id's from the cache and compaire them with local copies;
 * - check for existance of never been stored id.
 *
 * test scenario for MojDbShardEngine:
 * -----------------------------------
 * - compute a number of new id's (=SHARD_ITEMS_NUMBER);
 * - verify id for root path;
 * - store sample shard info;
 * - try to get previously stored info;
 * - get id for path, verify;
 * - set activity, read and verify;
 * - check existance of id, even for wrong id
 */

MojDbShardManagerTest::MojDbShardManagerTest()
    : MojTestCase(_T("MojDbShardManagerTest"))
{
}

MojErr MojDbShardManagerTest::run()
{
    //setup the test storage engine
#ifdef MOJ_USE_BDB
    MojDbStorageEngine::setEngineFactory (new MojDbBerkeleyFactory);
#elif MOJ_USE_LDB
    MojDbStorageEngine::setEngineFactory (new MojDbLevelFactory);
#else
#error "Not defined engine type"
#endif

    MojErr err = MojErrNone;
    MojDb db;

    cleanup();

    // open
    err = db.open(MojDbTestDir);
    MojTestErrCheck(err);

    MojDbShardIdCache* p_cache = db.shardIdCache();
    MojDbShardEngine* p_eng = db.shardEngine();

    err = _testShardIdCache(p_cache);
    MojTestErrCheck(err);
    err = _testShardManager(p_eng);
    MojTestErrCheck(err);
    err = db.close();
    MojTestErrCheck(err);

    return err;
}

MojErr MojDbShardManagerTest::_testShardIdCache (MojDbShardIdCache* ip_cache)
{
    MojErr err = MojErrNone;
    MojUInt32 arr[SHARDID_CACHE_SIZE];

    ip_cache->init(MojDbShardIdCache::TESTING);

    //ShardIdCache: Put id's..
    for (MojInt16 i = 0; i < SHARDID_CACHE_SIZE; i++)
    {
        arr[i] = i * 100L; //generate id and keep it
        ip_cache->put(arr[i]);
    }

    //compare..
    for (MojInt16 i = 0; i < SHARDID_CACHE_SIZE; i++)
    {
        if (!ip_cache->isExist(arr[i]))
        {
            err = MojErrDbVerificationFailed; //id %d is wrong ", i
        }
    }

    if (ip_cache->isExist(0xFFFFFFFF))
        err = MojErrDbVerificationFailed; //compare id6 is wrong

    //remove all id's
    for (MojInt16 i = 0; i < SHARDID_CACHE_SIZE; i++)
    {
        ip_cache->del(arr[i]);
    }

    //done

    return err;
}

MojErr MojDbShardManagerTest::_testShardManager (MojDbShardEngine* ip_eng)
{
    MojErr err;
    MojUInt32 id;
    MojString str;
    bool found;

    //compute a new shard id
    for(MojInt32 i = 0; i < SHARD_ITEMS_NUMBER; ++i)
    {
        //generate id
        str.format("MassStorageMedia%d",i);
        err = ip_eng->getShardId(str, id);
        MojTestErrCheck(err);
    }

    //store sample shard info
    MojDbShardEngine::ShardInfo shardInfo;
    shardInfo.id = 0xFF;
    shardInfo.mountPath.assign("/media/media01");
    err = ip_eng->put(shardInfo);
    MojTestErrCheck(err);
    //get info

    err = ip_eng->get(0xFF, shardInfo, found);
    MojTestErrCheck(err);
    MojAssert(found);

    if (shardInfo.mountPath.compare("/media/media01") != 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //set activity, read and verify
    shardInfo.active = true;
    err = ip_eng->update(shardInfo);
    MojTestErrCheck(err);

    err = ip_eng->get(0xFF, shardInfo, found);
    MojTestErrCheck(err);
    MojAssert(found);

    if (!shardInfo.active)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //check existance of id, even for wrong id
    err = ip_eng->isIdExist(0xFF, found);
    MojTestErrCheck(err);

    if (!found)
        err = MojErrDbVerificationFailed;
    MojTestErrCheck(err);

    err = ip_eng->isIdExist(0xFFFFFFFF, found);
    MojTestErrCheck(err);

    if (found)
        err = MojErrDbVerificationFailed;
    MojTestErrCheck(err);

    return err;
}

MojErr MojDbShardManagerTest::displayMessage(const MojChar* format, ...)
{
    va_list args;
    va_start (args, format);
    MojErr err = MojVPrintF(format, args);
    va_end(args);
    MojErrCheck(err);

    return MojErrNone;
}

void MojDbShardManagerTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
}

