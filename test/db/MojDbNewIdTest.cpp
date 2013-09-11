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

/**
****************************************************************************************************
* Filename              : MojDbNewIdTest.cpp
* Description           : Source file for testing id when putting objects.
****************************************************************************************************
**/

#include "MojDbNewIdTest.h"
#include "db/MojDb.h"
#include "db/MojDbSearchCursor.h"
#include "core/MojObjectBuilder.h"

// for PutTest
static const MojChar* const MojNewIdTestKindStr1 =
    _T("{\"id\":\"NewIdTest:1\",")
    _T("\"owner\":\"mojodb.admin\"")
    _T("}");

static const MojChar* const MojNewIdTestObjects1[] = {
    _T("{\"_kind\":\"NewIdTest:1\"}"),
    _T("{\"_kind\":\"NewIdTest:1\"}"),
    _T("{\"_kind\":\"NewIdTest:1\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"a123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"b1234567890123456\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"c123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"d123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"e1234567890123456\"}"),
};

static const MojChar* const MojNewIdTestShardIds1[] = {
    _T("zzzzzk"),
    _T("aaaaak"),
    _T("s123456"),
    _T("zzzzzk"),
    _T("zzzzzk"),
    _T("s123456"),
    _T(""),
    _T(""),
};

static const bool MojNewIdTestExpectResult1[] = {
    true,
    false, //"errorCode":-3968,"errorText":"db: Invalid shard ID","returnValue":false
    false, //"errorCode":-3968,"errorText":"db: Invalid shard ID","returnValue":false
    true,
    false, //"errorCode":-3968,"errorText":"db: Invalid _id","returnValue":false
    false, //"errorCode":-3968,"errorText":"db: Invalid shard ID","returnValue":false
    true,
    false, //"errorCode":-3968,"errorText":"db: Invalid _id","returnValue":false
};


// for duplicateTest
static const MojChar* const MojNewIdTestKindStr2 =
    _T("{\"id\":\"NewIdTest:2\",")
    _T("\"owner\":\"mojodb.admin\"")
    _T("}");

static const MojChar* const MojTestObjStr2 =
    _T("{\"_kind\":\"NewIdTest:2\",\"foo\":1,\"bar\":2}");

static const MojChar* const MojQueryStr2 =
    _T("NewIdTest:2");

// for adding shard id test
static const MojChar* const MojNewIdTestKindStr3 =
    _T("{\"id\":\"NewIdTest:3\",")
    _T("\"owner\":\"mojodb.admin\"")
    _T("}");

static const MojChar* const MojNewIdTestObjects3[] = {
    _T("{\"_kind\":\"NewIdTest:3\"}"),
    _T("{\"_kind\":\"NewIdTest:3\"}"),
    _T("{\"_kind\":\"NewIdTest:3\"}"),
    _T("{\"_kind\":\"NewIdTest:3\"}"),
    _T("{\"_kind\":\"NewIdTest:3\"}"),
};

static const MojChar* const MojNewIdTestShardIds3[] = {
    _T("zzzzzh"),
    _T("zzzzzi"),
    _T("zzzzzj"),
    _T("zzzzzj"),
    _T("zzzzzh"),
};

MojDbNewIdTest::MojDbNewIdTest()
: MojTestCase(_T("MojDbNewId"))
{
}
/**
****************************************************************************************************
* @run       It runs 3 tests for testing id.
             1. check-invalid ids of inputed objects.
                (1) shardId:{"zzzzzk"}, object : {"_kind":"NewIdTest:1"}
                    : normal
                (2) shardId:{"aaaaak"}, object : {"_kind":"NewIdTest:1"}
                    : Invalid shard ID, shard ID does not exist in shardIdCache.
                (3) shardId:{"s123456"}, object : {"_kind":"NewIdTest:1"}
                    : Invalid shard ID, shard ID is too long.
                (4) shardId:{"zzzzzk"},  object : {"_kind":"NewIdTest:1","_id":"a123456789"}
                    : normal
                (5) shardId:{"zzzzzk"},  object : {"_kind":"NewIdTest:1","_id":"b1234567890123456"}
                    : Invalid _id, _id is too long.
                (6) shardId:{"s123456"}, object : {"_kind":"NewIdTest:1","_id":"c123456789"}
                    : Invalid shard ID, shard ID is too long.
                (7) shardId:{""},        object : {"_kind":"NewIdTest:1","_id":"d123456789"}
                    : normal
                (8) shardId:{""},        object : {"_kind":"NewIdTest:1","_id":"e1234567890123456"}
                    : Invalid _id, _id is too long.

             2. check for duplicate id through looping. (1000 times)
             3. add shard Id into Kind:1
                and check if there are shard ids in Kind:1 without duplication of shard Id
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
MojErr MojDbNewIdTest::run()
{
    MojDb db;
    MojErr err;

    // db Open
    err = db.open(MojDbTestDir);
    MojTestErrCheck(err);

    // put shardIds into shardIdCache
    MojDbShardEngine* p_engine = db.shardEngine();
    err = initShardEngine(p_engine);
    MojTestErrCheck(err);

    // put kinds
    err = putTestKind(db);
    MojTestErrCheck(err);

    // 1st Test : check-invalid ids of inputed objects.
    err = putTest(db);
    MojTestErrCheck(err);

    // 2nd Test : check for duplicate id through looping (1000 times)
    err = duplicateTest(db);
    MojTestErrCheck(err);

    // 3rd Test : Check adding shardId into Kind:1
    err = addShardIdTest(db);
    MojTestErrCheck(err);

    err = db.close();
    MojTestErrCheck(err);

    return MojErrNone;
}

void MojDbNewIdTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbNewIdTest::initShardEngine (MojDbShardEngine* ip_engine)
{
    MojUInt32 arr[] = { 0xFFFFFFFA, 0xFFFFFFFB, 0xFFFFFFFC, 0xFFFFFFFD, 0xFFFFFFFE, 0xFFFFFFFF };
    MojDbShardEngine::ShardInfo info;

    for (MojSize i = 0; i < 6; i++)
    {
        generateItem(arr[i], info);
        ip_engine->put(info);
    }

    return MojErrNone;
}

MojErr MojDbNewIdTest::generateItem (MojUInt32 i_id, MojDbShardEngine::ShardInfo& o_shardInfo)
{
    static MojUInt32 id = 0xFF;

    if(i_id != 0)
        o_shardInfo.id = i_id;
    else
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

MojErr MojDbNewIdTest::putTestKind(MojDb& db)
{
    MojErr err;

    // add kind
    MojObject kindObj;
    err = kindObj.fromJson(MojNewIdTestKindStr1);
    MojTestErrCheck(err);
    err = db.putKind(kindObj);
    MojTestErrCheck(err);

    err = kindObj.fromJson(MojNewIdTestKindStr2);
    MojTestErrCheck(err);
    err = db.putKind(kindObj);
    MojTestErrCheck(err);

    err = kindObj.fromJson(MojNewIdTestKindStr3);
    MojTestErrCheck(err);
    err = db.putKind(kindObj);
    MojTestErrCheck(err);

    return MojErrNone;
}


MojErr MojDbNewIdTest::putTest(MojDb& db)
{
    MojErr err;

    for (MojSize i = 0; i < sizeof(MojNewIdTestObjects1) / sizeof(MojChar*); ++i) {
        MojObject obj;
        err = obj.fromJson(MojNewIdTestObjects1[i]);
        MojTestErrCheck(err);

        MojString shardId;
        shardId.assign(MojNewIdTestShardIds1[i]);

        err = db.put(obj, MojDb::FlagNone, MojDbReq(), shardId);
        bool result = (err == 0);

        MojTestAssert(MojNewIdTestExpectResult1[i] == result);
    }

    return MojErrNone;
}

MojErr MojDbNewIdTest::duplicateTest(MojDb& db)
{
    MojErr err;
    const MojUInt32 numObjects = 1000;

    for (MojUInt32 i = 0; i < numObjects; ++i) {
        MojObject objOrig;
        err = objOrig.fromJson(MojTestObjStr2);
        MojTestErrCheck(err);
        MojObject obj1 = objOrig;
        err = db.put(obj1);
        MojTestErrCheck(err);
    }

    MojUInt32 count = getKindCount(MojQueryStr2, db);
    MojTestAssert(numObjects == count);

    return MojErrNone;
}

MojUInt32 MojDbNewIdTest::getKindCount (const MojChar* MojQueryStr, MojDb& db)
{
    MojErr err;

    MojDbQuery query;
    query.clear();
    err = query.from(MojQueryStr);
    MojTestErrCheck(err);

    MojString str;
    MojDbSearchCursor cursor(str);
    err = db.find(query, cursor);
    MojTestErrCheck(err);

    MojUInt32 count;
    err = cursor.count(count);
    MojTestErrCheck(err);

    return count;
}

MojErr MojDbNewIdTest::addShardIdTest(MojDb& db)
{
    MojErr err;

    for (MojSize i = 0; i < sizeof(MojNewIdTestObjects3) / sizeof(MojChar*); ++i) {
        MojObject obj;
        err = obj.fromJson(MojNewIdTestObjects3[i]);
        MojTestErrCheck(err);

        MojString shardId;
        shardId.assign(MojNewIdTestShardIds3[i]);

        err = db.put(obj, MojDb::FlagNone, MojDbReq(), shardId);
    }

    // get object with _id from db
    MojObject resultObj;
    bool foundOut;
    MojString idStr;
    idStr.assign(_T("_kinds/NewIdTest:3"));
    MojObject id(idStr);
    err = db.get(id, resultObj, foundOut);
    MojErrCheck(err);
    // extract shardIds from result object
    MojObject shardIdsObj;
    resultObj.get(_T("shardId"), shardIdsObj);
    // check if size of shardIds is 3
    MojTestAssert(shardIdsObj.size() == 3);

    return MojErrNone;
}

