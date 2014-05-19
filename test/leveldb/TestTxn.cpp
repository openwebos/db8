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
 *  @file TestTxn.cpp
 ****************************************************************/

#include "db-luna/leveldb/MojDbLevelTxn.h"

#include "Runner.h"
#include "TestTxn.h"

TEST_F(TestTxn, visibility)
{
    initSample();

    ttxn.begin(*db);

    initTxnSampleA(ttxn);

    std::string val;

    EXPECT_TRUE( ttxn.Get("0", val).IsNotFound() );

    AssertLdbOk( ttxn.Get("a", val) );
    EXPECT_EQ( "txn-0", val );

    EXPECT_TRUE( ttxn.Get("b", val).IsNotFound() );

    AssertLdbOk( ttxn.Get("c", val) );
    EXPECT_EQ( "txn-1", val );

    EXPECT_TRUE( ttxn.Get("ca", val).IsNotFound() );

    AssertLdbOk( ttxn.Get("d", val) );
    EXPECT_EQ( "txn-2", val );

    EXPECT_TRUE( ttxn.Get("da", val).IsNotFound() );

    EXPECT_TRUE( ttxn.Get("e", val).IsNotFound() );

    AssertLdbOk( ttxn.Get("f", val) );
    EXPECT_EQ( "txn-3", val );

    AssertLdbOk( ttxn.Get("g", val) );
    EXPECT_EQ( "db-3", val );

    AssertLdbOk( ttxn.Get("h", val) );
    EXPECT_EQ( "txn-4", val );

    EXPECT_TRUE( ttxn.Get("i", val).IsNotFound() );
}

TEST_F(TestTxn, dbIsolation)
{
    initSample();

    ttxn.begin(*db);

    initTxnSampleA(ttxn);

    std::string val;
    leveldb::ReadOptions ro;

    EXPECT_TRUE( db->Get(ro, "a", &val).IsNotFound() );

    AssertLdbOk( db->Get(ro, "b", &val) );
    EXPECT_EQ( "db-0", val );

    EXPECT_TRUE( db->Get(ro, "c", &val).IsNotFound() );

    AssertLdbOk( db->Get(ro, "d", &val) );
    EXPECT_EQ( "db-1", val );

    AssertLdbOk( db->Get(ro, "e", &val) );
    EXPECT_EQ( "db-2", val );

    EXPECT_TRUE( db->Get(ro, "f", &val).IsNotFound() );

    AssertLdbOk( db->Get(ro, "g", &val) );
    EXPECT_EQ( "db-3", val );

    EXPECT_TRUE( db->Get(ro, "h", &val).IsNotFound() );
}

TEST_F(TestTxn, txnIsolation)
{
    initSample();

    ttxn.begin(*db);

    initTxnSampleA(ttxn);

    MojDbLevelTableTxn ttxn2;
    ttxn2.begin(*db);

    std::string val;

    EXPECT_TRUE( ttxn2.Get("a", val).IsNotFound() );

    AssertLdbOk( ttxn2.Get("b", val) );
    EXPECT_EQ( "db-0", val );

    EXPECT_TRUE( ttxn2.Get("c", val).IsNotFound() );

    AssertLdbOk( ttxn2.Get("d", val) );
    EXPECT_EQ( "db-1", val );

    AssertLdbOk( ttxn2.Get("e", val) );
    EXPECT_EQ( "db-2", val );

    EXPECT_TRUE( ttxn2.Get("f", val).IsNotFound() );

    AssertLdbOk( ttxn2.Get("g", val) );
    EXPECT_EQ( "db-3", val );

    EXPECT_TRUE( ttxn2.Get("h", val).IsNotFound() );
}

TEST_F(TestTxn, commitVisibility)
{
    initSample();
    ttxn.begin(*db);
    initTxnSampleA(ttxn);
    ttxn.commitImpl();

    std::string val;
    leveldb::ReadOptions ro;

    AssertLdbOk( db->Get(ro, "a", &val) );
    EXPECT_EQ( "txn-0", val );

    EXPECT_TRUE( db->Get(ro, "b", &val).IsNotFound() );

    AssertLdbOk( db->Get(ro, "d", &val) );
    EXPECT_EQ( "txn-2", val );
}

TEST_F(TestTxn, snapshotTransparency)
{
    std::string val;
    leveldb::ReadOptions ro;

    ASSERT_TRUE( db->Get(ro, "b", &val).IsNotFound() );

    ttxn.begin(*db);

    ASSERT_TRUE( ttxn.Get("b", val).IsNotFound() );

    initSample();

    AssertLdbOk( db->Get(ro, "b", &val) );
    EXPECT_EQ( "db-0", val );

    // XXX: do we actually shouldn't use snapshot here?
    AssertLdbOk( ttxn.Get("b", val) );
    EXPECT_EQ( "db-0", val );
}
