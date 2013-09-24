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

/**
 *  @file ShardsTest.cpp
 *  Verify multi-shard logic
 */

#include <db/MojDbSearchCursor.h>
#include <core/MojObjectBuilder.h>

#include "MojDbCoreTest.h"

namespace {
    const MojChar* const MojTestKind1Str1 =
            _T("{\"id\":\"Test:1\",")
            _T("\"owner\":\"com.foo.bar\",")
            _T("\"indexes\":[")
                    _T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},")
                    _T("{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]}")
            _T("]}");

    const MojUInt32 MagicId = 42u;
}

/**
 * Test fixture for tests around multi-shard logic
 */
struct ShardsTest : public MojDbCoreTest
{
    MojDbIdGenerator idGen;
    MojDbShardEngine *shardEngine;

    void SetUp()
    {
        MojDbCoreTest::SetUp();

        shardEngine = db.shardEngine();

        // initialize own copy of _id generator
        idGen.init();

        // add kind
        MojObject obj;
        MojAssertNoErr( obj.fromJson(MojTestKind1Str1) );
        MojAssertNoErr( db.putKind(obj) );
    }

    MojErr put(MojObject &obj, MojUInt32 shardId = MojDbIdGenerator::MainShardId)
    {
        MojErr err;
        /* XXX: once shard id will be added to Put interface (GF-11469)
         *  MojAssertNoErr( db.put("_shardId", MagicId) );
         */

        // work-around with pre-generated _id
        MojObject id;
        err = idGen.id(id, shardId);
        MojErrCheck( err );

        err = obj.put("_id", id);
        MojErrCheck( err );

        return db.put(obj);
    }

    void registerShards()
    {
        // add shards
        MojDbShardEngine::ShardInfo shardInfo;
        shardInfo.id = MagicId;
        shardInfo.active = true;
        MojAssertNoErr( shardEngine->put( shardInfo ) );

        // verify shard put
        shardInfo = {};
        bool found;
        MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
        ASSERT_TRUE( found );
        ASSERT_EQ( MagicId, shardInfo.id );
        ASSERT_TRUE( shardInfo.active );
    }

    void fillData()
    {
        // add objects
        for(int i = 1; i <= 10; ++i)
        {
            MojObject obj;
            MojAssertNoErr( obj.putString("_kind", "Test:1") );
            MojAssertNoErr( obj.put("foo", i) );
            MojAssertNoErr( obj.put("bar", i/4) );

            if (i % 2) MojAssertNoErr( put(obj, MagicId) );
            else MojAssertNoErr( put(obj) );
        }
    }

    void expect(const MojChar* expectedJson, bool includeInactive = false)
    {
        MojDbQuery query;
        query.clear();
        MojAssertNoErr( query.from(_T("Test:1")) );
        if (includeInactive) query.setIgnoreInactiveShards( false );

        MojString str;
        MojDbSearchCursor cursor(str);
        MojAssertNoErr( db.find(query, cursor) );

        MojObjectBuilder builder;
        MojAssertNoErr( builder.beginArray() );
        MojAssertNoErr( cursor.visit(builder) );
        MojAssertNoErr( cursor.close() );
        MojAssertNoErr( builder.endArray() );
        MojObject results = builder.object();

        MojString json;
        MojAssertNoErr( results.toJson(json) );

        MojObject expected;
        MojAssertNoErr( expected.fromJson(expectedJson) );

        ASSERT_EQ( expected.size(), results.size() )
            << "Amount of expected records should match amount of records in result";
        MojObject::ConstArrayIterator j = results.arrayBegin();
        size_t n = 0;
        for (MojObject::ConstArrayIterator i = expected.arrayBegin();
             j != results.arrayEnd() && i != expected.arrayEnd();
             ++i, ++j, ++n)
        {
            MojString resultStr;
            MojAssertNoErr( j->toJson(resultStr) );
            fprintf(stderr, "result: %.*s\n", resultStr.length(), resultStr.begin());

            MojObject foo;
            MojAssertNoErr( j->getRequired("foo", foo) )
                << "\"foo\" should be present in item #" << n;
            ASSERT_EQ( *i, foo );
        }
    }
};

/**
 * @test Verify that put shardInfo works
 */
TEST_F(ShardsTest, putShardInfo)
{
    // add shard
    shardEngine = db.shardEngine();
    MojDbShardEngine::ShardInfo shardInfo;
    shardInfo.id = MagicId;
    shardInfo.active = true;
    MojAssertNoErr( shardEngine->put( shardInfo ) );

    bool found;
    // verify shard put
    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    EXPECT_TRUE( found );
    EXPECT_EQ( MagicId, shardInfo.id );
    EXPECT_TRUE( shardInfo.active );
}

/**
 * @test Verify that setActivity actually changes state
 */
TEST_F(ShardsTest, setActivity)
{
    registerShards();

    MojDbShardEngine::ShardInfo shardInfo;
    bool found;

    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    EXPECT_TRUE( found );
    EXPECT_EQ( MagicId, shardInfo.id );
    EXPECT_TRUE( shardInfo.active );

    // verify active state change
    shardInfo.active = false;
    MojAssertNoErr( shardEngine->update(shardInfo) );
    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    ASSERT_TRUE( found );
    EXPECT_EQ( MagicId, shardInfo.id );
    EXPECT_FALSE( shardInfo.active );

    shardInfo.active = true;
    MojAssertNoErr( shardEngine->update(shardInfo) );
    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    ASSERT_TRUE( found );
    EXPECT_EQ( MagicId, shardInfo.id );
    EXPECT_TRUE( shardInfo.active );
}

/**
 * @test Verify behavior of different request with non-existing shard
 *   - getting info for non-existing shard
 *   - adding object to a non-existing shard
 */
TEST_F(ShardsTest, nonexistingShard)
{
    MojDbShardEngine::ShardInfo shardInfo = {};
    bool found;

    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    EXPECT_FALSE(found);

    shardInfo = {};
    shardInfo.id = MagicId;
    shardInfo.active = true;
    EXPECT_EQ( MojErrDbObjectNotFound, shardEngine->update(shardInfo) );

    MojObject obj;
    MojAssertNoErr( obj.putString("_kind", "Test:1") );
    MojAssertNoErr( obj.put("foo", 0) );

    // TODO: EXPECT_EQ( MojErr...,  put(obj, MagicId) );
    EXPECT_NE( MojErrNone, put(obj, MagicId) );
}

/**
 * @test Verify that after MojDb re-open all shards are inactive
 */
TEST_F(ShardsTest, initalInactivity)
{
    registerShards();

    MojDbShardEngine::ShardInfo shardInfo;
    bool found;

    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    ASSERT_TRUE( found );
    ASSERT_TRUE( shardInfo.active );

    MojAssertNoErr( db.close() );
    // re-open
    MojAssertNoErr( db.open(path.c_str()) );

    MojAssertNoErr( shardEngine->get( MagicId, shardInfo, found ) );
    ASSERT_TRUE(found);
    EXPECT_FALSE( shardInfo.active );
}

/**
 * @test Verify that all records available (including active shards)
 */
TEST_F(ShardsTest, queryActiveShard)
{
    registerShards();
    fillData();

    expect("[1,3,5,7,9,2,4,6,8,10]"); // re-order by shard prefix vs timestamp
}

/*
 * @test Verify that records for inactive shards are filtered out
 */
TEST_F(ShardsTest, queryInactiveShard)
{
    registerShards();
    fillData();

    MojDbShardEngine::ShardInfo shardInfo;
    bool found;
    MojAssertNoErr (shardEngine->get(MagicId, shardInfo, found));
    ASSERT_TRUE(found);
    EXPECT_EQ(MagicId, shardInfo.id);

    shardInfo.active = false;
    MojAssertNoErr (shardEngine->update(shardInfo));

    expect("[2,4,6,8,10]"); // no records from shard MagicId
}

/*
 * @test Verify that query.setIgnoreInactiveShards(false) results in all
 * records returned (even for inactive shards).
 */
TEST_F(ShardsTest, queryInactiveShardWithoutIgnore)
{
    registerShards();
    fillData();

    MojDbShardEngine::ShardInfo shardInfo;
    bool found;
    MojAssertNoErr (shardEngine->get(MagicId, shardInfo, found));
    ASSERT_TRUE(found);
    EXPECT_EQ(MagicId, shardInfo.id);

    shardInfo.active = false;
    MojAssertNoErr (shardEngine->update(shardInfo));

    expect("[1,3,5,7,9,2,4,6,8,10]", true); // re-order by shard prefix vs timestamp
}
