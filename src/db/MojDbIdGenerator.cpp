/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include "db/MojDbIdGenerator.h"
#include "core/MojObject.h"
#include "core/MojDataSerialization.h"
#include "core/MojTime.h"
#include "core/MojLogDb8.h"

namespace {
    const size_t LongIdBytes = (32 + 64) / 8 /* bytes/bits */;
    const size_t LongIdChars = LongIdBytes * 4/3 /* base64/byte */;
}

const MojUInt32 MojDbIdGenerator::MainShardId = 0;

MojDbIdGenerator::MojDbIdGenerator()
{
}

MojErr MojDbIdGenerator::init()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);

	MojTime time;
	MojErr err = MojGetCurrentTime(time);
	MojErrCheck(err);
	MojZero(&m_randBuf, sizeof(m_randBuf));
	err = MojInitRandom((MojUInt32) time.microsecs(), m_randStateBuf, sizeof(m_randStateBuf), &m_randBuf);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIdGenerator::id(MojObject& idOut, MojString shardId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    // if shard ID exists, add shard ID in front of _id
    if (G_UNLIKELY(!shardId.empty())) {
        MojErr err;
        // extract UInt32 shard info from shard Id
        MojVector<MojByte> shardIdVector;
        err = shardId.base64Decode(shardIdVector);
        MojErrCheck(err);
        MojUInt32 shard = 0;
        for (MojVector<MojByte>::ConstIterator byte = shardIdVector.begin(); byte != shardIdVector.end(); ++byte) {
            shard = (shard << 8) | (*byte);
        }
        // Revise "_id" field to include optional shard prefix ([32bit shard][50bit time][14bit random])
        return id(idOut, shard);
    }
    else {
        return id(idOut);
    }
}

MojErr MojDbIdGenerator::id(MojObject& idOut, MojUInt32 shardId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// an id consists of  a timestamp (in microsecs) concatenated with a 32-bit random number.
	// The goal is to have a very low likelihood of id collision among multiple devices owned
	// by the same user (so they can can share an id-space) while still keeping them sequential
	// to minimize db fragmentation.

	MojThreadGuard guard(m_mutex);
	MojUInt32 randNum;
	MojErr err = MojRandom(&m_randBuf, &randNum);
	MojErrCheck(err);
	guard.unlock();

	MojTime time;
	err = MojGetCurrentTime(time);
	MojErrCheck(err);

	MojBuffer buf;
	MojDataWriter writer(buf);

    // if shard ID is different from main, add shard ID in front of _id
    if (G_UNLIKELY(shardId != MainShardId)) {
        // Note that shardId stored in native order inside of _id
        err = writer.writeUInt32(shardId);
        MojErrCheck(err);
    }

    // Generating a 50-bit timestamp based in microseconds,
    // but with a 2 bit downshift so the timestamp interval is in units of 4 microseconds,
    // which will normally produce a unique timestamp. Then concatenate a 14 bit random number to ensure uniqueness.
	MojUInt64 baseId = ((MojUInt64) (time.microsecs() & 0xFFFFFFFFFFFFC)) << 12;
    // Take the bottom 14 bits random number from 32 bits
    baseId +=  (randNum & 0x3FFF);
    err = writer.writeInt64(baseId);
	MojErrCheck(err);

	MojVector<MojByte> byteVec;
	err = buf.toByteVec(byteVec);
	MojErrCheck(err);
	MojString str;
	err = str.base64Encode(byteVec, false);
	MojErrCheck(err);
	idOut = str;

	return MojErrNone;
}

MojErr MojDbIdGenerator::extractShard(const MojObject &id, MojUInt32 &shardIdOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojAssert( id.type() == MojObject::TypeString );

    MojErr err;
    MojString idStr;

    err = id.stringValue(idStr);
    MojErrCheck(err);

    // in case long id (with shard prefix)
    if (G_UNLIKELY(idStr.length() == LongIdChars))
    {
        MojVector<MojByte> idVec;
        err = idStr.base64Decode(idVec);
        MojErrCheck(err);

        // extract first 32bits of _id as shard id in native order
        MojDataReader reader(idVec.begin(), idVec.size());

        MojUInt32 shardId;
        err = reader.readUInt32(shardId);
        MojErrCheck(err);

        // Note that shardId stored in native order inside of _id
        shardIdOut = shardId;
    }
    else
    {
        shardIdOut = MainShardId;
    }

    return MojErrNone;
}
