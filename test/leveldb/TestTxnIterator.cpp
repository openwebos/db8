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
 *  @file TestTxnIterator.cpp
 ****************************************************************/

#include <memory>

#include "db-luna/leveldb/MojDbLevelTxnIterator.h"

#include "Runner.h"
#include "TestTxn.h"

struct TestTxnIterator : public TestTxn {};

TEST_F(TestTxnIterator, reversing)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->first();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );
}

TEST_F(TestTxnIterator, reversingThroughUpdate)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("d");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );
}

TEST_F(TestTxnIterator, backReversingThroughUpdate)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("d");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );
}

TEST_F(TestTxnIterator, tails)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->first();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );
    it->prev();
    EXPECT_FALSE( it->isValid() );
    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->last();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );
    it->next();
    EXPECT_FALSE( it->isValid() );
    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );
}

TEST_F(TestTxnIterator, visibility)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->first();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "g", it->getKey() );
    EXPECT_EQ( "db-3", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );

    it->next();
    EXPECT_FALSE( it->isValid() );
}

TEST_F(TestTxnIterator, visibilityReversed)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->last();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "g", it->getKey() );
    EXPECT_EQ( "db-3", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->prev();
    EXPECT_FALSE( it->isValid() );
}

TEST_F(TestTxnIterator, seekInsert)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("f");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "g", it->getKey() );
    EXPECT_EQ( "db-3", it->getValue() );
}

TEST_F(TestTxnIterator, seekDelete)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("b");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );
}

TEST_F(TestTxnIterator, seekUpdate)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("d");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "f", it->getKey() );
    EXPECT_EQ( "txn-3", it->getValue() );
}

TEST_F(TestTxnIterator, seekMissing)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("0");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );
}

TEST_F(TestTxnIterator, seekOutside)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("zzz");
    EXPECT_FALSE( it->isValid() );
}

TEST_F(TestTxnIterator, backAfterSeekOutside)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->seek("zzz");
    EXPECT_FALSE( it->isValid() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );
}

// case: Record inserted right in place where txn iterator currently points to (between db and txn)
TEST_F(TestTxnIterator, insertIntoNext)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    // g >> gi << h'
    it->seek("g");
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "g", it->getKey() );
    EXPECT_EQ( "db-3", it->getValue() );

    ttxn.Put("gi", "txn-5");

    // ensure that we didn't affected txn iterator
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "g", it->getKey() );
    EXPECT_EQ( "db-3", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "gi", it->getKey() );
    EXPECT_EQ( "txn-5", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "h", it->getKey() );
    EXPECT_EQ( "txn-4", it->getValue() );
}

// case: Record where txn iterator points to was deleted
TEST_F(TestTxnIterator, jumpOffDeleted)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->first();

    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    ttxn.Delete("a");
    // we should invalidate current record txn iterator points to
    EXPECT_FALSE( it->isValid() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "d", it->getKey() );
    EXPECT_EQ( "txn-2", it->getValue() );
}

TEST_F(TestTxnIterator, jumpBackOffDeleted)
{
    initSample();

    ttxn.begin(db);

    initTxnSampleA(ttxn);

    std::auto_ptr<MojDbLevelTxnIterator> it;
    it.reset(ttxn.createIterator());

    it->first();

    it->next();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "c", it->getKey() );
    EXPECT_EQ( "txn-1", it->getValue() );

    ttxn.Delete("c");
    // we should invalidate current record txn iterator points to
    EXPECT_FALSE( it->isValid() );

    it->prev();
    ASSERT_TRUE( it->isValid() );
    EXPECT_EQ( "a", it->getKey() );
    EXPECT_EQ( "txn-0", it->getValue() );

    it->prev();
    EXPECT_FALSE( it->isValid() );
    EXPECT_TRUE( it->isBegin() );
    EXPECT_FALSE( it->isEnd() );
}
