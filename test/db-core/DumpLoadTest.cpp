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
 *  @file DumpLoadTest.cpp
 ****************************************************************/

#include "gtest/gtest.h"

#include "db/MojDb.h"
#include "core/MojUtil.h"

#include "Runner.h"

static const MojChar* const MojLoadTestFileName = _T("loadtest.json");
static const MojChar* const MojDumpTestFileName = _T("dumptest.json");
static const MojChar* const MojTestStr =
	_T("{\"_id\":\"_kinds/LoadTest:1\",\"_kind\":\"Kind:1\",\"id\":\"LoadTest:1\",\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}");

struct DatabaseSuite : public ::testing::Test
{
    MojDb db;
    MojString kindId;

    void SetUp()
    {
        // open
        MojAssertNoErr( db.open(tempFolder) );
        MojAssertNoErr( kindId.assign(_T("LoadTest:1")) );
    }

    void TearDown()
    {
        MojExpectNoErr( db.close() );

        MojUnlink(MojLoadTestFileName);
        MojUnlink(MojDumpTestFileName);
    }

    void checkCount()
    {
        MojDbQuery query;
        MojAssertNoErr( query.from(_T("LoadTest:1")) );

        MojDbCursor cursor;
        MojAssertNoErr( db.find(query, cursor) );

        MojUInt32 count = 0;
        MojAssertNoErr( cursor.count(count) );

        EXPECT_EQ( 10LL, count );
    }
};

TEST_F(DatabaseSuite, dump_and_load)
{

    // load
    MojAssertNoErr( MojFileFromString(MojLoadTestFileName, MojTestStr) );

    MojUInt32 count = 0;
    MojAssertNoErr( db.load(MojLoadTestFileName, count) );
    EXPECT_EQ( 11LL, count );

    checkCount();

    // dump
    count = 0;
    MojAssertNoErr( db.dump(MojDumpTestFileName, count) );
    EXPECT_EQ( 11LL, count );

    EXPECT_TRUE( db.kindEngine() );

    // verify that we can get this kind
    MojDbKind *pk = 0;
    MojExpectNoErr( db.kindEngine()->getKind(kindId.data(), pk) );
    EXPECT_TRUE( pk );

    // del
    bool found = false;
    MojAssertNoErr( db.delKind(kindId, found) );
    EXPECT_TRUE( found );

    // verify that we can NOT get this kind
    pk = 0;
    EXPECT_EQ( MojErrDbKindNotRegistered, db.kindEngine()->getKind(kindId.data(), pk) );
    EXPECT_FALSE( pk );

    // purge deleted kinds
    MojAssertNoErr( db.purge(count, 0) );
    EXPECT_EQ( 1LL, count ) << "Purged kinds marked for delete";

    // load again. why 12?
    MojAssertNoErr( db.load(MojDumpTestFileName, count) );
    EXPECT_EQ( 12LL, count );

    checkCount();

    // analyze
    MojObject analysis;
    MojAssertNoErr( db.stats(analysis) );
}
