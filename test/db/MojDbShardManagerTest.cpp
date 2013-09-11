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

    MojDbShardEngine* p_eng = db.shardEngine();
    MojDbShardIdCache cache;
    err = testShardIdCacheIndexes(&cache);
    MojTestErrCheck(err);
    err = testShardIdCacheOperations(&cache);
    MojTestErrCheck(err);
    err = testShardEngine(p_eng);
    MojTestErrCheck(err);
    err = db.close();
    MojTestErrCheck(err);

    return err;
}

MojErr MojDbShardManagerTest::testShardIdCacheIndexes (MojDbShardIdCache* ip_cache)
{
    MojAssert(ip_cache);

    MojObject obj;
    MojErr err = MojErrNone;
    MojUInt32 arr[SHARDID_CACHE_SIZE];

    //ShardIdCache: Put id's..
    for (MojInt16 i = 0; i < SHARDID_CACHE_SIZE; i++)
    {
        arr[i] = i * 100L; //generate id and keep it
        ip_cache->put(arr[i], obj);
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
    ip_cache->clear();

    //done

    return err;
}

MojErr MojDbShardManagerTest::testShardIdCacheOperations (MojDbShardIdCache* ip_cache)
{
    MojAssert(ip_cache);

    MojErr err = MojErrNone;
    MojUInt32 id1 = 0xFF, id2 = 0xFFFF;
    MojObject obj1, obj2;
    MojObject v_obj1, v_obj2;
    //generate new object1
    MojObject d1_obj1(static_cast<MojInt32>(id1));
    err = obj1.put(_T("shardId"), d1_obj1);
    MojObject d1_obj2(true);
    err = obj1.put(_T("active"), d1_obj2);
    MojErrCheck(err);

    //generate new object2
    MojObject d2_obj1(static_cast<MojInt32>(id2));
    err = obj1.put(_T("shardId"), d2_obj1);
    MojObject d2_obj2(true);
    err = obj1.put(_T("active"), d2_obj2);
    MojErrCheck(err);

    //store
    ip_cache->put(id1, obj1);
    ip_cache->put(id2, obj2);

    //get
    if(!ip_cache->get(id1, v_obj1))
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    if(!ip_cache->get(id2, v_obj2))
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //verify
    if(v_obj1.compare(obj1) != 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    if(v_obj2.compare(obj1) == 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //update
    ip_cache->update(id1, obj2);

    if(!ip_cache->get(id1, v_obj1))
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //verify
    if(v_obj1.compare(obj2) != 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //del
    ip_cache->del(id2);

    if(ip_cache->get(id2, v_obj2))
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //clear
    ip_cache->clear();

    return err;
}

MojErr MojDbShardManagerTest::testShardEngine (MojDbShardEngine* ip_eng)
{
    MojAssert(ip_eng);

    MojErr err;
    MojUInt32 id;
    MojString str;
    bool found;

    //store sample shard info
    MojDbShardEngine::ShardInfo shardInfo;
    generateItem(shardInfo);
    shardInfo.mountPath.assign("/tmp/db8-test/media01");
    id = shardInfo.id;
    err = ip_eng->put(shardInfo);
    MojTestErrCheck(err);

    //get info
    err = ip_eng->get(id, shardInfo, found);
    MojTestErrCheck(err);
    MojAssert(found);

    if (shardInfo.mountPath.compare("/tmp/db8-test/media01") != 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //set activity flag, read and verify it
    shardInfo.active = true;
    err = ip_eng->update(shardInfo);
    MojTestErrCheck(err);

    err = ip_eng->get(id, shardInfo, found);
    MojTestErrCheck(err);
    MojAssert(found);

    if (!shardInfo.active)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //check existance of id, even for wrong id
    err = ip_eng->isIdExist(id, found);
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

MojErr MojDbShardManagerTest::generateItem (MojDbShardEngine::ShardInfo& o_shardInfo)
{
    static MojUInt32 id = 0xFF;
    o_shardInfo.id = ++id;
    MojDbShardEngine::convertId(o_shardInfo.id, o_shardInfo.id_base64);
    o_shardInfo.active = false;
    o_shardInfo.transient = false;
    o_shardInfo.deviceId.format("ID%x", o_shardInfo.id);
    o_shardInfo.deviceUri.format("URI%x", o_shardInfo.id);
    o_shardInfo.mountPath.format("/tmp/db8-test/media%x", o_shardInfo.id);
    o_shardInfo.deviceName.format("TEST-%x", o_shardInfo.id);

    return MojErrNone;
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

