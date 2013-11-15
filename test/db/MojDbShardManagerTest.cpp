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
#include "db/MojDbKind.h"
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
 */

static const MojChar* const TestShardKind1Str =
    _T("{\"id\":\"TestShard1:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"RecId\",  \"props\":[ {\"name\":\"recId\"} ]}, \
                    ]}");

static const MojChar* const TestShardKind2Str =
    _T("{\"id\":\"TestShard2:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"RecId\",  \"props\":[ {\"name\":\"recId\"} ]}, \
                    ]}");

static const MojChar* const TestShardKind3Str =
    _T("{\"id\":\"TestShard3:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"RecId\",  \"props\":[ {\"name\":\"recId\"} ]}, \
                    ]}");


MojDbShardManagerTest::MojDbShardManagerTest()
    : MojTestCase(_T("MojDbShardEngine"))
{
}

/**
 * run
 */
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

    err = testShardCreateAndRemoveWithRecords(db);
    MojTestErrCheck(err);

    err = db.close();
    MojTestErrCheck(err);

    return err;
}

/**
 * testShardIdCacheIndexes
 *
 * compute a number of new id's (=SHARD_ITEMS_NUMBER);
 */
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

/**
 * testShardIdCacheOperations
 *
 * - verify id for root path;
 * - store sample shard info;
 * - try to get previously stored info;
 * - get id for path, verify;
 * - set activity, read and verify;
 * - check existance of id, even for wrong id
 */
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

/**
 * testShardEngine
 */
MojErr MojDbShardManagerTest::testShardEngine (MojDbShardEngine* ip_eng)
{
    MojAssert(ip_eng);

    MojErr err;
    MojUInt32 id;
    MojString str;
    bool found;

    //get shard by wrong id
    MojDbShardEngine::ShardInfo info;
    ip_eng->get(0xFFFFFFFE, info, found);
    MojTestAssert(!found);

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
    MojTestAssert(found);

    if (shardInfo.mountPath.compare("/tmp/db8-test/media01") != 0)
        err = MojErrDbVerificationFailed;

    MojTestErrCheck(err);

    //set activity flag, read and verify it
    shardInfo.active = true;
    err = ip_eng->update(shardInfo);
    MojTestErrCheck(err);

    err = ip_eng->get(id, shardInfo, found);
    MojTestErrCheck(err);
    MojTestAssert(found);

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

/**
 * testShardCreateAndRemoveWithRecords
 *
 * - create 3 new shards
 * - add records to 1st and 2nd shard
 * - remove shard 1
 * - verify existance of shard 1
 * - verify records of shard 2
 * - remove shard 2
 * - verify existance of shard 2
 * - verify records of shard 3 (== 0)
 * - remove shard 3
 * - verify existance of shard 3
 */
MojErr MojDbShardManagerTest::testShardCreateAndRemoveWithRecords (MojDb& db)
{
    MojDbShardEngine* p_eng = db.shardEngine();
    MojTestAssert(p_eng);

    MojDbShardEngine::ShardInfo shard1, shard2;
    MojVector<MojUInt32> arrShardIds;
    MojUInt32 count;

    //---------------------------------------------------------------
    // test shard 1
    //---------------------------------------------------------------

    MojErr err = createShardObjects1(db, shard1);
    MojErrCheck(err);

    //verify number of records for shard 1 (shard is active)
    err = verifyRecords(_T("TestShard1:1"), db, shard1, count);
    MojTestErrCheck(err);
    MojTestAssert(count == 1);

    //err = db.shardEngine()->purgeShardObjects(1);
    //MojTestErrCheck(err);

    //remove shard 1
    arrShardIds.push(shard1.id);
    err = p_eng->removeShardObjects(arrShardIds);
    MojErrCheck(err);

    //verify number of records for shard 1
    err = verifyRecords(_T("TestShard1:1"), db, shard1, count);
    MojErrCheck(err);
    MojTestAssert(count == 0);

    //verify existance of shard 1
    err = verifyShardExistance(db, shard1);
    MojErrCheck(err);

    //---------------------------------------------------------------
    // test shard 2
    //---------------------------------------------------------------

    err = createShardObjects2(db, shard2);
    MojErrCheck(err);

    //verify number of records for shard 1 (shard is active)
    err = verifyRecords(_T("TestShard1:1"), db, shard2, count);
    MojTestErrCheck(err);
    MojTestAssert(count == 1);

    err = verifyRecords(_T("TestShard2:1"), db, shard2, count);
    MojTestErrCheck(err);
    MojTestAssert(count == 1);

    err = verifyRecords(_T("TestShard3:1"), db, shard2, count);
    MojTestErrCheck(err);
    MojTestAssert(count == 1);

    //remove shard 1
    arrShardIds.push(shard2.id);
    err = p_eng->removeShardObjects(arrShardIds);
    MojErrCheck(err);

    //verify number of records for shard 1
    err = verifyRecords(_T("TestShard1:1"), db, shard2, count);
    MojErrCheck(err);
    MojTestAssert(count == 0);

    err = verifyRecords(_T("TestShard2:1"), db, shard2, count);
    MojErrCheck(err);
    MojTestAssert(count == 0);

    err = verifyRecords(_T("TestShard3:1"), db, shard2, count);
    MojErrCheck(err);
    MojTestAssert(count == 0);

    //verify existance of shard 1
    err = verifyShardExistance(db, shard2);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * Add records to first shard for a single Kind
 */
MojErr MojDbShardManagerTest::createShardObjects1 (MojDb& db, MojDbShardEngine::ShardInfo& shard)
{
    MojObject objKind;
    MojString kindId;

    MojErr err = kindId.assign(_T("TestShard1:1"));
    MojErrCheck(err);
    err = objKind.putString(_T("_kind"), kindId.data());
    MojErrCheck(err);

    //generate
    err = generateItem(shard);
    MojErrCheck(err);

    err = addKind(TestShardKind1Str, db);
    MojErrCheck(err);
    err = verifyKindExistance(kindId, db);
    MojErrCheck(err);

    //store shard info
    err = db.shardEngine()->put(shard);
    MojErrCheck(err);

    //add record
    MojObject record;
    err = record.putString(_T("_kind"), kindId.data());
    MojErrCheck(err);

    //add value
    MojObject objId(static_cast<MojInt32>(shard.id));
    err = record.put(_T("recId"), objId);
    MojErrCheck(err);

    //put
    MojString strShardId;
    MojDbShardEngine::convertId(shard.id, strShardId);
    err = db.put(record, MojDb::FlagNone, MojDbReq(), strShardId);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * Add records for second shard for three different Kinds
 */
MojErr MojDbShardManagerTest::createShardObjects2 (MojDb& db, MojDbShardEngine::ShardInfo& shard)
{
    MojObject objKind1;
    MojString kindId1;

    MojErr err = kindId1.assign(_T("TestShard1:1"));
    MojErrCheck(err);
    err = objKind1.putString(_T("_kind"), kindId1.data());
    MojErrCheck(err);

    MojObject objKind2;
    MojString kindId2;

    err = kindId2.assign(_T("TestShard2:1"));
    MojErrCheck(err);
    err = objKind2.putString(_T("_kind"), kindId2.data());
    MojErrCheck(err);

    MojObject objKind3;
    MojString kindId3;

    err = kindId3.assign(_T("TestShard3:1"));
    MojErrCheck(err);
    err = objKind3.putString(_T("_kind"), kindId3.data());
    MojErrCheck(err);

    //generate
    err = generateItem(shard);
    MojErrCheck(err);

    err = addKind(TestShardKind1Str, db);
    MojErrCheck(err);
    err = verifyKindExistance(kindId1, db);
    MojErrCheck(err);

    err = addKind(TestShardKind2Str, db);
    MojErrCheck(err);
    err = verifyKindExistance(kindId2, db);
    MojErrCheck(err);

    err = addKind(TestShardKind3Str, db);
    MojErrCheck(err);
    err = verifyKindExistance(kindId3, db);
    MojErrCheck(err);

    //store shard info
    err = db.shardEngine()->put(shard);
    MojErrCheck(err);

    //add record [1]
    MojObject record1;
    err = record1.putString(_T("_kind"), kindId1.data());
    MojErrCheck(err);

    //add value
    MojObject objId(static_cast<MojInt32>(shard.id));
    err = record1.put(_T("recId"), objId);
    MojErrCheck(err);

    //put [1]
    MojString strShardId;
    MojDbShardEngine::convertId(shard.id, strShardId);
    err = db.put(record1, MojDb::FlagNone, MojDbReq(), strShardId);
    MojErrCheck(err);

    //add record [2]
    MojObject record2;
    err = record2.putString(_T("_kind"), kindId2.data());
    MojErrCheck(err);

    //put [2]
    err = db.put(record2, MojDb::FlagNone, MojDbReq(), strShardId);
    MojErrCheck(err);

    //add record [3]
    MojObject record3;
    err = record3.putString(_T("_kind"), kindId3.data());
    MojErrCheck(err);

    //put [3]
    err = db.put(record3, MojDb::FlagNone, MojDbReq(), strShardId);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * addKind
 */
MojErr MojDbShardManagerTest::addKind (const MojChar* strKind, MojDb& db)
{
    MojObject kind;
    MojErr err = kind.fromJson(strKind);
    MojErrCheck(err);
    err = db.putKind(kind);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * is kind exist?
 */
MojErr MojDbShardManagerTest::verifyKindExistance (MojString kindId, MojDb& db)
{
    bool foundOurKind = false;
    MojString str; //for debug

    //kinds map
    MojDbKindEngine::KindMap& map = db.kindEngine()->kindMap();

    for (MojDbKindEngine::KindMap::ConstIterator it = map.begin();
         it != map.end();
         ++it)
    {
        str = it.key();
        if(kindId == str)
        {
            foundOurKind = true;
            break;
        }
    }

    if (!foundOurKind)
        MojErrThrowMsg(MojErrDbKindNotRegistered, "Kind %s not found in kindMap", kindId.data());

    return MojErrNone;
}

/**
 * verifyRecords
 */
MojErr MojDbShardManagerTest::verifyRecords (const MojChar* strKind, MojDb& db, const MojDbShardEngine::ShardInfo& shard, MojUInt32& count)
{
    MojDbQuery query;
    MojDbCursor cursor;
    MojObject dbObj;
    bool found;
    count = 0;

    MojErr err = query.from(strKind);
    MojErrCheck(err);

    err = db.find(query, cursor);
    MojErrCheck(err);

    while (true)
    {
        bool found;
        MojObject dbObj;

        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if (!found)
            break;

        ++count;
    }

    return MojErrNone;
}

/**
 * verifyExistance1
 */
MojErr MojDbShardManagerTest::verifyShardExistance (MojDb& db, const MojDbShardEngine::ShardInfo& shard)
{
    bool found;

    MojErr err = db.shardEngine()->isIdExist(shard.id, found);
    MojTestErrCheck(err);

    if (!found)
    {
        err = MojErrDbVerificationFailed;
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * generateItem
 */
MojErr MojDbShardManagerTest::generateItem (MojDbShardEngine::ShardInfo& o_shardInfo)
{
    static MojUInt32 id = 0xFF;
    o_shardInfo.id = ++id;
    MojDbShardEngine::convertId(o_shardInfo.id, o_shardInfo.id_base64);
    o_shardInfo.active = true;
    o_shardInfo.transient = false;
    o_shardInfo.deviceId.format("ID%x", o_shardInfo.id);
    o_shardInfo.deviceUri.format("URI%x", o_shardInfo.id);
    o_shardInfo.mountPath.format("/tmp/db8-test/media%x", o_shardInfo.id);
    o_shardInfo.deviceName.format("TEST-%x", o_shardInfo.id);

    return MojErrNone;
}

/**
 * displayMessage
 */
MojErr MojDbShardManagerTest::displayMessage(const MojChar* format, ...)
{
    va_list args;
    va_start (args, format);
    MojErr err = MojVPrintF(format, args);
    va_end(args);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * cleanup
 */
void MojDbShardManagerTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
}

