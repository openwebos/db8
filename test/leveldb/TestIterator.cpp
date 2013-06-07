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
 *  @file TestIterator.cpp
 ****************************************************************/

#include "db-luna/leveldb/MojDbLevelIterator.h"

#include "Runner.h"
#include "TestLdb.h"

struct TestIterator: public TestLdb {};

TEST_F(TestIterator, tails)
{
    initSample();
    MojDbLevelIterator it(db);

    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
    --it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );

    it.toLast();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
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
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );

    it.toEnd();
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
}

TEST_F(TestIterator, walk)
{
    initSample();
    MojDbLevelIterator it(db);

    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->key().ToString() );
    EXPECT_EQ( "db-1", it->value().ToString() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "e", it->key().ToString() );
    EXPECT_EQ( "db-2", it->value().ToString() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->key().ToString() );
    EXPECT_EQ( "db-1", it->value().ToString() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "e", it->key().ToString() );
    EXPECT_EQ( "db-2", it->value().ToString() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
    ++it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_FALSE( it.isBegin() );
    EXPECT_TRUE( it.isEnd() );
    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
    it.toFirst();
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
    --it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
}

TEST_F(TestIterator, seek)
{
    initSample();
    MojDbLevelIterator it(db);

    it.seek("b");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );

    it.seek("c");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->key().ToString() );
    EXPECT_EQ( "db-1", it->value().ToString() );

    it.seek("g");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
}

TEST_F(TestIterator, seekReverse)
{
    initSample();
    MojDbLevelIterator it(db);

    it.seek("d");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->key().ToString() );
    EXPECT_EQ( "db-1", it->value().ToString() );

    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
}

TEST_F(TestIterator, seekMissing)
{
    initSample();
    MojDbLevelIterator it(db);

    it.seek("000");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
    --it;
    EXPECT_FALSE( it.isValid() );
    EXPECT_TRUE( it.isBegin() );
    EXPECT_FALSE( it.isEnd() );

    it.seek("000");
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "b", it->key().ToString() );
    EXPECT_EQ( "db-0", it->value().ToString() );
    ++it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "d", it->key().ToString() );
    EXPECT_EQ( "db-1", it->value().ToString() );

}

TEST_F(TestIterator, seekOutside)
{
    initSample();
    MojDbLevelIterator it(db);

    it.seek("zzz");
    EXPECT_FALSE( it.isValid() );

    --it;
    ASSERT_TRUE( it.isValid() );
    EXPECT_EQ( "g", it->key().ToString() );
    EXPECT_EQ( "db-3", it->value().ToString() );
}
