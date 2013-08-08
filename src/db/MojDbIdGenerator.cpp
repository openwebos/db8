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
#include "core/MojObjectSerialization.h"
#include "core/MojTime.h"

MojDbIdGenerator::MojDbIdGenerator()
{
}

MojErr MojDbIdGenerator::init()
{
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

    // if shard ID exists, add shard ID in front of _id
    if (!shardId.empty()) {
        // extract UInt32 shard info from shard Id
        MojVector<MojByte> shardIdVector;
        err = shardId.base64Decode(shardIdVector);
        MojErrCheck(err);
        MojUInt32 shard = 0;
        for (MojVector<MojByte>::ConstIterator byte = shardIdVector.begin(); byte != shardIdVector.end(); ++byte) {
            shard = (shard << 8) | (*byte);
        }
        // Revise "_id" field to include optional shard prefix ([32bit shard][50bit time][14bit random])
        err = writer.writeUInt32(shard);
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
