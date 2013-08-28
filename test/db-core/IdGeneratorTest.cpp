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
 *  @file IdGeneratorTest.cpp
 *  Verify MojDbIdGenerator logic
 */

#include "Runner.h"

#include <core/MojObject.h>
#include <db/MojDbIdGenerator.h>

namespace {
    const size_t ShortIdChars = 11u;
    const size_t LongIdChars = 16u;
    const MojUInt32 MagicId = 42u;
}

/**
 * Test fixture for tests around MojDbIdGenerator
 */
struct IdGeneratorTest : public ::testing::Test
{
    MojDbIdGenerator gen;

    void SetUp()
    {
        ASSERT_NE( MojDbIdGenerator::MainShardId, MagicId );
        gen.init();
    }

    void genId(MojString &id)
    {
        MojObject idObj;
        MojAssertNoErr( gen.id(idObj) );

        ASSERT_EQ( MojObject::TypeString, idObj.type() );
        MojAssertNoErr( idObj.stringValue(id) );
    }

    template <typename T>
    void genId(MojString &id, T arg)
    {
        MojObject idObj;
        MojAssertNoErr( gen.id(idObj, arg) );

        ASSERT_EQ( MojObject::TypeString, idObj.type() );
        MojAssertNoErr( idObj.stringValue(id) );
    }
};

/**
 * @test Test generation of short _id when shard genId is set to main shard or
 * omitted In this case length should be ceil(64/8 * 4/3) = 11
 */
TEST_F(IdGeneratorTest, shortId_length)
{
    MojString x;

    genId(x);
    EXPECT_EQ( ShortIdChars, x.length() );
    x = MojString();

    genId(x, MojDbIdGenerator::MainShardId);
    EXPECT_EQ( ShortIdChars, x.length() );
    x = MojString();

    genId(x, MojString());
    EXPECT_EQ( ShortIdChars, x.length() );
    x = MojString();
}

/**
 * @test Test generation of _id for shards encoded incorrectly
 */
TEST_F(IdGeneratorTest, bad_shards)
{
    MojObject idObj;
    MojString shard;

    MojAssertNoErr( shard.assign("K.AK") );
    EXPECT_EQ( MojErrInvalidBase64Data, gen.id(idObj, shard) );

    MojAssertNoErr( shard.assign("KgA.") );
    EXPECT_EQ( MojErrInvalidBase64Data, gen.id(idObj, shard) );

    MojAssertNoErr( shard.assign("K") );
    EXPECT_EQ( MojErrInvalidBase64Data, gen.id(idObj, shard) );
}

/**
 * @test Test generation of short _id when shard genId is not a main shard In this
 * case length should be ceil((32+64)/8 * 4/3) = 16
 */
TEST_F(IdGeneratorTest, longId_length)
{
    MojString x;

    genId(x, MagicId);
    EXPECT_EQ( LongIdChars, x.length() );
    printf(">> %s\n", x.begin());
    x = MojString();

    MojString shard;
    MojAssertNoErr( shard.assign("KgAK") );
    genId(x, shard);
    EXPECT_EQ( LongIdChars, x.length() );
    x = MojString();
}

/**
 * @test Test extraction of shard from a short _id
 */
TEST_F(IdGeneratorTest, shortId_extractShard)
{
    MojString x;
    MojUInt32 shardId = MagicId;

    MojAssertNoErr( x.assign("IZsgY29QHSo") );
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( MojDbIdGenerator::MainShardId, shardId );
}

/**
 * @test Test extraction of shard from a long _id
 */
TEST_F(IdGeneratorTest, longId_extractShard)
{
    MojString x;
    MojUInt32 shardId = 0u;

    x.assign("++++9ZtDAOUFs+_U"); // contains 42 as shard id
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( MagicId, shardId );
}

/**
 * @test Test extraction of shard from a bad long _id
 */
TEST_F(IdGeneratorTest, longIdBad_extractShard)
{
    MojString x;
    MojUInt32 shardId;

    x.assign("+.++9ZtDAOUFs+_U");
    EXPECT_EQ( MojErrInvalidBase64Data, MojDbIdGenerator::extractShard(x, shardId) );

    x.assign("++++9.tDAOUFs+_U");
    EXPECT_EQ( MojErrInvalidBase64Data, MojDbIdGenerator::extractShard(x, shardId) );
}

/**
 * @test Test round-trip of shard through _id
 *    - check simple sample
 *    - check loosing the bits
 *    - check main shard id
 */
TEST_F(IdGeneratorTest, inOut_shard)
{
    MojString x;
    MojUInt32 shardId = 3; // no matches within cases

    genId(x, MagicId);
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( MagicId, shardId );
    x = MojString(), shardId = 3;

    genId(x, 0u); // all bits off
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( 0u, shardId );
    x = MojString(), shardId = 3;

    genId(x, 0xffffffffu); // all bits on
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( 0xffffffffu, shardId );
    x = MojString(), shardId = 3;

    genId(x, 0x7fffffffu); // all bits on but highest most is off
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( 0x7fffffffu, shardId );
    x = MojString(), shardId = 3;

    genId(x, MojDbIdGenerator::MainShardId);
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( MojDbIdGenerator::MainShardId, shardId );
    x = MojString(), shardId = 3;

    /* XXX: need this?
    MojString shard;
    MojAssertNoErr( shard.assign("KgAK") ); // 42
    genId(x, shard);
    MojAssertNoErr( MojDbIdGenerator::extractShard(x, shardId) );
    EXPECT_EQ( MagicId, shardId );
    */
}
