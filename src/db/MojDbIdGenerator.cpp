/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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
#include "core/MojString.h"

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

MojErr MojDbIdGenerator::id(MojObject& idOut)
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
	err = writer.writeInt64(time.microsecs());
	MojErrCheck(err);
	err = writer.writeUInt32(randNum);
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
