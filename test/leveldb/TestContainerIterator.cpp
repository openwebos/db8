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
 *  @file TestContainerIterator.cpp
 ****************************************************************/

#include "db-luna/leveldb/MojDbLevelContainerIterator.h"

#include "Runner.h"

struct TestContainerIterator: public ::testing::Test
{
    std::map<std::string, std::string> db;

    void SetUp()
    {
        initSample();
    }

    void TearDown()
    {
        db.clear();
    }

    void initSample()
    {
        db["b"] = "db-0";
        db["d"] = "db-1";
        db["e"] = "db-2";
        db["g"] = "db-3";
    }
};

TEST_F(TestContainerIterator, tails)
{
    MojDbLevelContainerIterator it(db);

    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );
    --it;
    ASSERT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );

    it.toLast();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
    ++it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );

    it.toBegin();
    EXPECT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );

    it.toEnd();
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
}

TEST_F(TestContainerIterator, walk)
{
    MojDbLevelContainerIterator it(db);

    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->first );
    EXPECT_EQ( "db-1", it->second );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "e", it->first );
    EXPECT_EQ( "db-2", it->second );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->first );
    EXPECT_EQ( "db-1", it->second );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "e", it->first );
    EXPECT_EQ( "db-2", it->second );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
    ++it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );
    --it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );
}

TEST_F(TestContainerIterator, seek)
{
    MojDbLevelContainerIterator it(db);

    // it.seek("b");
    it = db.lower_bound("b");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );

    // it.seek("c");
    it = db.lower_bound("c");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->first );
    EXPECT_EQ( "db-1", it->second );

    // it.seek("g");
    it = db.lower_bound("g");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
}

TEST_F(TestContainerIterator, seekReverse)
{
    MojDbLevelContainerIterator it(db);

    // it.seek("h");
    it = db.lower_bound("h");
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
}

TEST_F(TestContainerIterator, seekMissing)
{
    MojDbLevelContainerIterator it(db);

    // it.seek("d");
    it = db.lower_bound("d");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->first );
    EXPECT_EQ( "db-1", it->second );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->first );
    EXPECT_EQ( "db-0", it->second );
}

TEST_F(TestContainerIterator, seekOutside)
{
    MojDbLevelContainerIterator it(db);

    // it.seek("0");
    it = db.lower_bound("zzz");
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );

    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->first );
    EXPECT_EQ( "db-3", it->second );
}
