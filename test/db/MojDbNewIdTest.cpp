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

static const MojChar* const MojNewIdTestObjects[] = {
    _T("{\"_kind\":\"NewIdTest:1\"}"),
    _T("{\"_kind\":\"NewIdTest:1\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"a123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"b1234567890123456\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"c123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"d123456789\"}"),
    _T("{\"_kind\":\"NewIdTest:1\",\"_id\":\"e1234567890123456\"}"),
};

static const MojChar* const MojNewIdTestShardIds[] = {
    _T("s12345"),
    _T("s123456"),
    _T("s12345"),
    _T("s12345"),
    _T("s123456"),
    _T(""),
    _T(""),
};

static const bool MojNewIdTestExpectResult[] = {
    true,
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

static const MojChar* const MojTestObjStr =
    _T("{\"_kind\":\"NewIdTest:2\",\"foo\":1,\"bar\":2}");

static const MojChar* const MojQueryStr =
    _T("NewIdTest:2");


MojDbNewIdTest::MojDbNewIdTest()
: MojTestCase(_T("MojDbNewId"))
{
}
/**
****************************************************************************************************
* @run       It runs 2 tests for testing id.
             1. check-invalid ids of inputed objects.
                (1) shardId:{"s12345"},  object : {"_kind":"NewIdTest:1"}
                    : normal
                (2) shardId:{"s123456"}, object : {"_kind":"NewIdTest:1"}
                    : Invalid shard ID, shard ID is too long.
                (3) shardId:{"s12345"},  object : {"_kind":"NewIdTest:1","_id":"a123456789"}
                    : normal
                (4) shardId:{"s12345"},  object : {"_kind":"NewIdTest:1","_id":"b1234567890123456"}
                    : Invalid _id, _id is too long.
                (5) shardId:{"s123456"}, object : {"_kind":"NewIdTest:1","_id":"c123456789"}
                    : Invalid shard ID, shard ID is too long.
                (6) shardId:{""},        object : {"_kind":"NewIdTest:1","_id":"d123456789"}
                    : normal
                (7) shardId:{""},        object : {"_kind":"NewIdTest:1","_id":"e1234567890123456"}
                    : Invalid _id, _id is too long.

                    2. check for duplicate id through looping. (1000 times)
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

    // put kinds
    err = putTestKind(db);
    MojTestErrCheck(err);

    // 1st Test : check-invalid ids of inputed objects.
    err = putTest(db);
    MojTestErrCheck(err);

    // 2nd Test : check for duplicate id through looping (1000 times)
    err = duplicateTest(db);
    MojTestErrCheck(err);

    err = db.close();
    MojTestErrCheck(err);

    return MojErrNone;
}

void MojDbNewIdTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
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

    return MojErrNone;
}


MojErr MojDbNewIdTest::putTest(MojDb& db)
{
    MojErr err;

    for (MojSize i = 0; i < sizeof(MojNewIdTestObjects) / sizeof(MojChar*); ++i) {
        MojObject obj;
        err = obj.fromJson(MojNewIdTestObjects[i]);
        MojTestErrCheck(err);

        MojString shardId;
        shardId.assign(MojNewIdTestShardIds[i]);
        db.shardId(shardId);

        err = db.put(obj);
        bool result = (err == 0);

        MojTestAssert(MojNewIdTestExpectResult[i] == result);
    }

    return MojErrNone;
}

MojErr MojDbNewIdTest::duplicateTest(MojDb& db)
{
    MojErr err;
    const MojUInt32 numObjects = 1000;

    for (MojUInt32 i = 0; i < numObjects; ++i) {
        MojObject objOrig;
        err = objOrig.fromJson(MojTestObjStr);
        MojTestErrCheck(err);
        MojObject obj1 = objOrig;
        err = db.put(obj1);
        MojTestErrCheck(err);
    }

    MojUInt32 count = getKindCount(MojQueryStr, db);
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
