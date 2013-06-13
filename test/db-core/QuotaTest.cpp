/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

/****************************************************************
 *  @file QuotaTest.cpp
 *  Verify Quota. Based on MojDbQuotaTest.cpp
 ****************************************************************/

#include "db/MojDb.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else
#error "Specify database engine"
#endif
#include "MojDbTestStorageEngine.h"

#include "MojDbCoreTest.h"

namespace {
    const MojChar* const MojTestKind1Str1 =
            _T("{\"id\":\"Test:1\",")
            _T("\"owner\":\"com.foo.bar\"}");

    const MojChar* const MojTestKind1Str2 =
            _T("{\"id\":\"Test:1\",")
            _T("\"owner\":\"com.foo.bar\",")
            _T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"secondary\"}]}]}");

    const MojChar* const MojTestKind2Str1 =
            _T("{\"id\":\"Test2:1\",")
            _T("\"owner\":\"com.foo.bar\"}");

    const MojChar* const MojTestKind2Str2 =
            _T("{\"id\":\"Test2:1\",")
            _T("\"owner\":\"com.foo.baz\"}");

    const MojChar* const MojTestKind2Str3 =
            _T("{\"id\":\"Test2:1\",\"extends\":[\"Test:1\"],")
            _T("\"owner\":\"com.foo.baz\"}");

    const MojChar* const MojTestKind3Str1 =
            _T("{\"id\":\"Test3:1\",")
            _T("\"owner\":\"com.foo.boo\"}");

    const MojChar* const MojTestKind3Str2 =
            _T("{\"id\":\"Test3:1\",")
            _T("\"owner\":\"com.bar\"}");

    const MojChar* MojTestKind1Objects[] = {
            _T("{\"_id\":1,\"_kind\":\"Test:1\",\"foo\":\"cote\"}"),
            _T("{\"_id\":2,\"_kind\":\"Test:1\",\"foo\":\"coté\"}"),
            _T("{\"_id\":3,\"_kind\":\"Test:1\",\"foo\":\"côte\"}"),
            _T("{\"_id\":4,\"_kind\":\"Test:1\",\"foo\":\"côté\"}"),
            NULL
    };

    const MojChar* MojTestKind2Objects[] = {
            _T("{\"_id\":5,\"_kind\":\"Test2:1\",\"foo\":\"cote\"}"),
            _T("{\"_id\":6,\"_kind\":\"Test2:1\",\"foo\":\"coté\"}"),
            _T("{\"_id\":7,\"_kind\":\"Test2:1\",\"foo\":\"côte\"}"),
            _T("{\"_id\":8,\"_kind\":\"Test2:1\",\"foo\":\"côté\"}"),
            NULL
    };

    const MojChar* MojTestKind3Objects[] = {
            _T("{\"_id\":9,\"_kind\":\"Test3:1\",\"foo\":\"cote\"}"),
            _T("{\"_id\":10,\"_kind\":\"Test3:1\",\"foo\":\"coté\"}"),
            _T("{\"_id\":11,\"_kind\":\"Test3:1\",\"foo\":\"côte\"}"),
            _T("{\"_id\":12,\"_kind\":\"Test3:1\",\"foo\":\"côté\"}"),
            NULL
    };
}

struct QuotaTest : public MojDbCoreTest
{
    void SetUp()
    {
        MojDbCoreTest::SetUp();

        // add kind
        MojObject obj;
        MojAssertNoErr( obj.fromJson(MojTestKind1Str1) );
        MojAssertNoErr( db.putKind(obj) );
    }

    void put(MojDb& db, const MojChar* objJson)
    {
        MojObject obj;
        MojAssertNoErr( obj.fromJson(objJson) );
        MojAssertNoErr( db.put(obj) );
    }

    void getKindUsage(MojDb& db, const MojChar* kindId, MojInt64& usageOut)
    {
        MojRefCountedPtr<MojDbStorageTxn> txn;
        MojAssertNoErr( db.storageEngine()->beginTxn(txn) );
        MojAssertNoErr( db.quotaEngine()->kindUsage(kindId, usageOut, txn.get()) );
    }

    void getQuotaUsage(MojDb& db, const MojChar* owner, MojInt64& usageOut)
    {
        MojInt64 size;
        MojAssertNoErr( db.quotaEngine()->quotaUsage(owner, size, usageOut) );
    }

};

TEST_F(QuotaTest, usage)
{
    // put quota
    MojObject obj;
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.bar\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // empty
    MojInt64 kindUsage = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage) );
    EXPECT_EQ( 0, kindUsage )
        << "Kind without objects should have zero usage";
    MojInt64 quotaUsage = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage) );
    EXPECT_EQ( 0, quotaUsage )
        << "Quota without matching objects should have zero usage";

    // new obj
    EXPECT_NO_FATAL_FAILURE( put(db, MojTestKind1Objects[0]) );
    MojInt64 kindUsage1 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage1) );
    EXPECT_LT( 0, kindUsage1 )
        << "Adding new object into kind should increase kind usage";
    MojInt64 quotaUsage1 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1) );
    EXPECT_LT( 0, quotaUsage1 )
        << "Adding new object matching quota should increase quota usage";

    // add prop to existing obj
    MojAssertNoErr( obj.fromJson(MojTestKind1Objects[0]) );
    MojAssertNoErr( obj.put(_T("bar"), 2) );
    MojAssertNoErr( db.put(obj, MojDb::FlagForce) );
    MojInt64 kindUsage2 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage2) );
    EXPECT_LE( 0, kindUsage2 );
    EXPECT_LT( kindUsage1, kindUsage2 )
        << "Adding property to existing object should increase kind usage";
    MojInt64 quotaUsage2 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage2) );
    EXPECT_LE( 0, quotaUsage2 );
    EXPECT_LT( quotaUsage1, quotaUsage2 )
        << "Adding property to existing object that matches quota should increase usage";

    // add 2nd obj
    EXPECT_NO_FATAL_FAILURE( put(db, MojTestKind1Objects[1]) );
    MojInt64 kindUsage3 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage3) );
    EXPECT_LE( 0, kindUsage3 );
    EXPECT_LT( kindUsage2, kindUsage3 )
        << "Adding another object should increase kind usage";
    MojInt64 quotaUsage3 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage3) );
    EXPECT_LE( 0, quotaUsage3 );
    EXPECT_LT( quotaUsage2, quotaUsage3 )
        << "Adding another object matching to quota should increase usage";

    // del first obj
    bool found = false;
    MojExpectNoErr( db.del(1, found, MojDb::FlagPurge) );
    EXPECT_TRUE( found ) << "Object should be deleted";
    MojInt64 kindUsage4 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage4) );
    EXPECT_LE( 0, kindUsage4 );
    EXPECT_EQ( kindUsage3 - kindUsage2, kindUsage4 )
        << "Deletion of object should bring kind usage to expected value";
    MojInt64 quotaUsage4 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage4) );
    EXPECT_LE( 0, quotaUsage4 );
    EXPECT_EQ( quotaUsage3 - quotaUsage2, quotaUsage4 )
        << "Deletion of object should bring quota usage to expected value";

    // add index
    MojAssertNoErr( obj.fromJson(MojTestKind1Str2) );
    MojExpectNoErr( db.putKind(obj) );
    MojInt64 kindUsage5 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage5) );
    EXPECT_LE( 0, kindUsage5 );
    EXPECT_LT( kindUsage4, kindUsage5 )
        << "Adding new index should increase kind usage";
    MojInt64 quotaUsage5 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage5) );
    EXPECT_LE( 0, quotaUsage5 );
    EXPECT_LT( quotaUsage4, quotaUsage5 )
        << "Adding new index should increase quota usage";

    // update locale
    MojExpectNoErr( db.updateLocale(_T("FR_fr")) );
    MojExpectNoErr( db.updateLocale(_T("EN_us")) );
    MojInt64 kindUsage6 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage6) );
    EXPECT_LE( 0, kindUsage6 );
    EXPECT_EQ( kindUsage5, kindUsage6 )
        << "Switching locale forth and back shouldn't affect kind usage";
    MojInt64 quotaUsage6 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage6) );
    EXPECT_LE( 0, kindUsage6 );
    EXPECT_EQ( quotaUsage5, quotaUsage6 )
        << "Switching locale forth and back shouldn't affect quota usage";

    // drop index
    MojAssertNoErr( obj.fromJson(MojTestKind1Str1) );
    MojExpectNoErr( db.putKind(obj) );
    MojInt64 kindUsage7 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage7) );
    EXPECT_LE( 0, kindUsage7 );
    EXPECT_EQ( kindUsage4, kindUsage7 )
        << "Dropping of index should bring kind usage to expected value";
    MojInt64 quotaUsage7 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage7) );
    EXPECT_LE( 0, quotaUsage7 );
    EXPECT_EQ( quotaUsage4, quotaUsage7 )
        << "Dropping of index should bring quota usage to expected value";

    // drop kind
    MojString kindStr;
    MojAssertNoErr( kindStr.assign(_T("Test:1")) );
    MojExpectNoErr( db.delKind(kindStr, found) );
    EXPECT_TRUE( found ) << "Kind should be deleted";
    MojInt64 kindUsage8 = -1;
    EXPECT_NO_FATAL_FAILURE( getKindUsage(db, _T("Test:1"), kindUsage8) );
    EXPECT_EQ( 0, kindUsage8 )
        << "Dropping of kind should bring its usage to zero";
    MojInt64 quotaUsage8 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage8) );
    EXPECT_EQ( 0, quotaUsage8 )
        << "Dropping of kind that matches quota should bring quota usage to zero";
    MojExpectNoErr( db.quotaStats(obj) );

    MojString statStr;
    MojExpectNoErr( obj.toJson(statStr) );
    std::cerr << "quotaStats: " << statStr.data() << std::endl;
}

TEST_F(QuotaTest, multipleQuotas)
{
    MojObject obj;

    // put quota (from testUsage)
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.bar\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // quota for com.foo.baz
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.baz\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // register kinds
    MojAssertNoErr( obj.fromJson(MojTestKind1Str1) );
    MojExpectNoErr( db.putKind(obj) );
    MojAssertNoErr( obj.fromJson(MojTestKind2Str1) );
    MojExpectNoErr( db.putKind(obj) );

    // put object of kind1 and kind2
    EXPECT_NO_FATAL_FAILURE( put(db, MojTestKind1Objects[0]) );
    EXPECT_NO_FATAL_FAILURE( put(db, MojTestKind2Objects[0]) );
    MojInt64 quotaUsage1 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1) );
    EXPECT_LE( 0, quotaUsage1 );
    MojInt64 quotaUsage2 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.baz"), quotaUsage2) );
    EXPECT_LE( 0, quotaUsage2 );
    EXPECT_LT( 0, quotaUsage1 );
    EXPECT_EQ( 0, quotaUsage2 );

    // change owner of kind2 to com.foo.baz
    MojAssertNoErr( obj.fromJson(MojTestKind2Str2) );
    MojExpectNoErr( db.putKind(obj) );
    MojInt64 quotaUsage3 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage3) );
    EXPECT_LE( 0, quotaUsage3 );
    MojInt64 quotaUsage4 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.baz"), quotaUsage4) );
    EXPECT_LE( 0, quotaUsage4 );
    EXPECT_LT( 0, quotaUsage3 );
    EXPECT_EQ( quotaUsage3, quotaUsage4);

    // make kind2 inherit from kind1
    MojAssertNoErr( obj.fromJson(MojTestKind2Str3) );
    MojExpectNoErr( db.putKind(obj) );
    MojInt64 quotaUsage5 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage5) );
    MojInt64 quotaUsage6 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.baz"), quotaUsage6) );
    EXPECT_GT( quotaUsage5, quotaUsage1 );
    EXPECT_EQ( 0, quotaUsage6 );

    // kind3 and object
    MojAssertNoErr( obj.fromJson(MojTestKind3Str1) );
    MojExpectNoErr( db.putKind(obj) );
    EXPECT_NO_FATAL_FAILURE( put(db, MojTestKind3Objects[0]) );
    MojInt64 quotaUsage7 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage7) );
    EXPECT_EQ( quotaUsage7, quotaUsage5 );

    // wildcard
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.*\",\"size\":1000}")) );
    MojExpectNoErr( db.putQuotas(&obj, &obj + 1) );
    MojInt64 quotaUsage8 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage8) );
    MojInt64 quotaUsage9 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.*"), quotaUsage9) );
    EXPECT_EQ( quotaUsage5, quotaUsage8 );
    EXPECT_LT( 0, quotaUsage9 );
}

TEST_F(QuotaTest, enforce)
{
    MojObject obj;

    // put quota
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.bar\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // make kind2 inherit from kind1
    MojAssertNoErr( obj.fromJson(MojTestKind2Str3) );
    MojExpectNoErr( db.putKind(obj) );

    //// end of prerequisites form prev tests

    // set quota size to current usage
    MojInt64 quotaUsage1 = 0;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1) );
    MojAssertNoErr( obj.putString(_T("owner"), _T("com.foo.bar")) );
    MojAssertNoErr( obj.putInt(_T("size"), quotaUsage1) );
    MojExpectNoErr( db.putQuotas(&obj, &obj + 1) );

    MojAssertNoErr( obj.fromJson(MojTestKind1Objects[3]) );
    EXPECT_EQ( MojErrDbQuotaExceeded, db.put(obj) );

    // Try to delete the kind
    MojString kindStr;
    MojAssertNoErr( kindStr.assign(_T("Test:1")) );
    bool found = false;
    EXPECT_EQ( MojErrDbKindHasSubKinds, db.delKind(kindStr, found) )
        << "The delete should be failure, because it contain sub kind \"Test2:1\"";
    EXPECT_FALSE( found );
}

TEST_F(QuotaTest, error)
{
    MojObject obj;

    // put quota (from testUsage)
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.bar\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // quota for com.foo.baz (from multipleQuotas)
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.baz\",\"size\":1000}")) );
    MojAssertNoErr( db.putQuotas(&obj, &obj + 1) );

    // make kind2 inherit from kind1 (from multipleQuotas)
    MojAssertNoErr( obj.fromJson(MojTestKind2Str3) );
    MojExpectNoErr( db.putKind(obj) );

    // kind3 (from multipleQuotas)
    MojAssertNoErr( obj.fromJson(MojTestKind3Str1) );
    MojExpectNoErr( db.putKind(obj) );

    // wildcard (from multipleQuotas)
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.*\",\"size\":1000}")) );
    MojExpectNoErr( db.putQuotas(&obj, &obj + 1) );

#ifdef MOJ_USE_BDB
    MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbBerkeleyEngine());
#elif MOJ_USE_LDB
    MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbLevelEngine());
#endif
    EXPECT_TRUE( engine.get() ) << "Engine should be created successfully";
    MojRefCountedPtr<MojDbTestStorageEngine> testEngine(new MojDbTestStorageEngine(engine.get()));
    EXPECT_TRUE( testEngine.get() ) << "TestEngine should be created successfully";

    MojAssertNoErr( db.close() );
    MojAssertNoErr( testEngine->open(path.c_str()) );

    MojAssertNoErr( db.open(path.c_str(), testEngine.get()) );

    // test that failed put does not affect quota
    MojInt64 quotaUsage1 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.*"), quotaUsage1) );
    EXPECT_LE( 0, quotaUsage1 );

    MojAssertNoErr( testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock) );
    MojAssertNoErr( obj.fromJson(MojTestKind3Objects[1]) );
    EXPECT_EQ( MojErrDbDeadlock, db.put(obj) );

    MojInt64 quotaUsage2 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.*"), quotaUsage2) );
    EXPECT_LE( 0, quotaUsage2 );
    EXPECT_EQ( quotaUsage1, quotaUsage2 );

    // test that failed putQuota has no effect
    MojAssertNoErr( testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock) );
    MojAssertNoErr( obj.fromJson(_T("{\"owner\":\"com.foo.boo\",\"size\":1000}")) );
    EXPECT_EQ( MojErrDbDeadlock, db.putQuotas(&obj, &obj + 1) );

    MojInt64 quotaUsage3 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.*"), quotaUsage3) );
    EXPECT_LE( 0, quotaUsage3 );
    EXPECT_EQ( quotaUsage1, quotaUsage3 );

    // test that failed putKind has no effect
    MojAssertNoErr( testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock) );
    MojAssertNoErr( obj.fromJson(MojTestKind3Str2) );
    EXPECT_EQ( MojErrDbDeadlock, db.putKind(obj) );

    MojInt64 quotaUsage4 = -1;
    EXPECT_NO_FATAL_FAILURE( getQuotaUsage(db, _T("com.foo.*"), quotaUsage4) );
    EXPECT_LE( 0, quotaUsage4 );
    EXPECT_EQ( quotaUsage1, quotaUsage4 );
}
