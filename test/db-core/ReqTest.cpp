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
 *  @file ReqTest.cpp
 ****************************************************************/

#include "db/MojDb.h"

#include "Runner.h"

namespace {

const char* const MojKindStr =
    _T("{\"id\":\"Test:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");
}

struct ReqSuite : public ::testing::Test
{
    MojDb db;
    std::string path;

    void SetUp()
    {
        const ::testing::TestInfo* const test_info =
          ::testing::UnitTest::GetInstance()->current_test_info();

        path = std::string(tempFolder) + '/'
             + test_info->test_case_name() + '-' + test_info->name();

        // open
        MojAssertNoErr( db.open(path.c_str()) );

        // add type
        MojObject obj;
        MojAssertNoErr( obj.fromJson(MojKindStr) );
        MojAssertNoErr( db.putKind(obj) );
    }

    void TearDown()
    {
        // TODO: clean DB
        MojExpectNoErr( db.close() );
    }

    void buildSample()
    {
        for (int i = 0; i < 100; ++i) {
            MojObject obj;
            MojAssertNoErr( obj.putString(MojDb::KindKey, _T("Test:1")) );
            MojAssertNoErr( obj.put(_T("foo"), (i + 25) % 100) );
            MojAssertNoErr( obj.put(_T("bar"), i % 3) );
            MojExpectNoErr( db.put(obj) ) << "Failed to put record #" << i;
        }

        // db: x0 = (25, 0), (26, 1), (27, 2), (28, 0) .. x74 = (99,2), x75 = (0,0) .. x99 = (24,0)
    }

    void mark1(MojDbReqRef req = MojDbReq())
    {
        // mark half with bar=-1
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("foo"), MojDbQuery::OpLessThan, 50) );

        MojObject update;
        MojAssertNoErr( update.put(_T("bar"), -1) );
        MojUInt32 count = 0xbaddcafe; // differentiat 0 from non-filled count
        MojAssertNoErr( db.merge(query, update, count) );
        EXPECT_EQ( 50ul, count );

        // db: x0 = (25, -1) .. x24 = (49,-1), x25 = (50,1)i .. x74 = (99,2), x75 = (0,-1) .. x99 = (24, -1)
    }

    void mark2(MojDbReqRef req = MojDbReq())
    {
        // re-mark half with bar=-1 to bar=-2
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpEq, -1) );

        MojObject update;
        MojAssertNoErr( update.put(_T("bar"), -2) );

        MojUInt32 count = 0xbaddcafe;
        MojAssertNoErr( db.merge(query, update, count, MojDb::FlagNone, req) );
        EXPECT_EQ( 50ul, count );

        // db: x0 = (25, -1) .. x24 = (49,-1), x25 = (50,1)i .. x74 = (99,2), x75 = (0,-1) .. x99 = (24, -1)
    }

    void deleteMark(MojUInt32 expect = 50ul, int mark = -1, MojDbReqRef req = MojDbReq())
    {
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpEq, mark) );

        MojUInt32 count = 0xbaddcafe;
        MojAssertNoErr( db.del(query, count, MojDb::FlagNone, req) );
        EXPECT_EQ( expect, count );
    }

    void mark3(MojDbReqRef req = MojDbReq())
    {
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpLessThan, 2) );

        MojObject update;
        MojAssertNoErr( update.put(_T("bar"), -3) );

        MojUInt32 count = 0xbaddcafe;
        MojAssertNoErr( db.merge(query, update, count, MojDb::FlagNone, req) );
        EXPECT_EQ( 33ul, count );
    }

    void checkMarkWithUpdate(MojUInt32 expect = 50ul, int mark = -1, MojDbReqRef req = MojDbReq())
    {
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpEq, mark) );

        MojObject update;

        MojUInt32 count = (MojUInt32)(-1);
        MojAssertNoErr( db.merge(query, MojObject(), count, MojDb::FlagNone, req) );
        EXPECT_EQ( expect, count );
    }
};

TEST_F(ReqSuite, verifySample)
{
    buildSample();
    mark1();
    checkMarkWithUpdate(50ul, -1);
    mark2();
    checkMarkWithUpdate(50ul, -2);
}

TEST_F(ReqSuite, visibility)
{
    buildSample();
    mark1();

    MojDbReq req;
    // start transaction
    req.begin(&db, false);

    checkMarkWithUpdate(50ul, -1, req);
    checkMarkWithUpdate(0ul, -2, req);

    mark2(req);

    checkMarkWithUpdate(50ul, -2, req);
}

TEST_F(ReqSuite, updateRollback)
{
    buildSample();
    mark1();

    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        checkMarkWithUpdate(50ul, -1, req);
        checkMarkWithUpdate(0ul, -2, req);

        mark2(req);

        checkMarkWithUpdate(50ul, -2, req);
    }
    checkMarkWithUpdate(50ul, -1);
    checkMarkWithUpdate(0ul, -2);
}

TEST_F(ReqSuite, deleteRollback)
{
    buildSample();
    mark1();

    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        checkMarkWithUpdate(50ul, -1, req);

        deleteMark(50ul, -1, req);

        checkMarkWithUpdate(0ul, -1, req);
    }
    checkMarkWithUpdate(50ul, -1);
}

TEST_F(ReqSuite, deleteUpdateRollback)
{
    buildSample();
    mark1();

    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        checkMarkWithUpdate(50ul, -1, req);
        checkMarkWithUpdate(0ul, -3, req);

        deleteMark(50ul, -1, req);

        checkMarkWithUpdate(0ul, -1, req);
        checkMarkWithUpdate(0ul, -3, req);

        mark3(req);

        checkMarkWithUpdate(0ul, -1, req);
        checkMarkWithUpdate(33ul, -3, req);

    }
    checkMarkWithUpdate(50ul, -1);
    checkMarkWithUpdate(0ul, -3);
}

TEST_F(ReqSuite, originalEq)
{
    buildSample();
    mark1();

    // test visibility with update
    {
        MojDbReq req;

        // start transaction
        req.begin(&db, false);
        mark2(req);

        // visible within transaction
        checkMarkWithUpdate(50ul, -2, req);
    }

    // invisible after aborted transaction
    checkMarkWithUpdate(0ul, -2);

    // test visibility with delete
    mark1();
    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        deleteMark(50ul, -1, req);
        // visible within transaction
        checkMarkWithUpdate(0ul, -1, req);
    }

    // invisible after aborted transaction
    checkMarkWithUpdate(50ul, -1);

}

TEST_F(ReqSuite, original)
{
    buildSample();
    mark1();

    // test visibility with update
    {
        MojDbReq req;

        // start transaction
        req.begin(&db, false);
        mark2(req);

        // visible within transaction
        checkMarkWithUpdate(50ul, -2, req);
    }

    // invisible after aborted transaction
    checkMarkWithUpdate(0ul, -2);

    // test visibility with delete
    mark1();
    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        deleteMark(50ul, -1, req);

        // visible within transaction
        {
            MojDbQuery query;
            MojAssertNoErr( query.from(_T("Test:1")) );
            MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpLessThan, 2) );

            MojObject update;

            MojUInt32 count = 0xbaddcafe;
            MojAssertNoErr( db.merge(query, update, count, MojDb::FlagNone, req) );
            EXPECT_EQ( 33ul, count);
        }
    }

    // invisible after aborted transaction
    {
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("Test:1")) );
        MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpLessThan, 2) );

        MojObject update;
        // Note: should not set value to something that will introduce double-update

        MojUInt32 count = 0xbaddcafe;
        MojAssertNoErr( db.merge(query, update, count) );

        EXPECT_EQ( 83ul, count);
    }
}
