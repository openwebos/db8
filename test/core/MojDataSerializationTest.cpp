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


#include "MojDataSerializationTest.h"
#include "core/MojDataSerialization.h"
#include "core/MojDecimal.h"

MojDataSerializationTest::MojDataSerializationTest()
: MojTestCase(_T("MojDataSerialization"))
{
}

MojErr MojDataSerializationTest::run()
{
	MojBuffer buf;
	MojDataWriter writer(buf);
	MojErr err = writer.writeUInt8(0);
	MojTestErrCheck(err);
	err = writer.writeUInt8(5);
	MojTestErrCheck(err);
	err = writer.writeUInt8(0xFF);
	MojTestErrCheck(err);
	err = writer.writeUInt16(0);
	MojTestErrCheck(err);
	err = writer.writeUInt16(256);
	MojTestErrCheck(err);
	err = writer.writeUInt16(0xFFFF);
	MojTestErrCheck(err);
	err = writer.writeUInt32(0);
	MojTestErrCheck(err);
	err = writer.writeUInt32(0x10000);
	MojTestErrCheck(err);
	err = writer.writeUInt32(0xFFFFFFFF);
	MojTestErrCheck(err);
	err = writer.writeInt64(0);
	MojTestErrCheck(err);
	err = writer.writeInt64(-56);
	MojTestErrCheck(err);
	err = writer.writeInt64(9223372036854775807LL);
	MojTestErrCheck(err);
	err = writer.writeInt64(-9223372036854775807LL);
	MojTestErrCheck(err);
	err = writer.writeDecimal(MojDecimal(3, 14159));
	MojTestErrCheck(err);
	err = writer.writeDecimal(MojDecimal(-9999, 888888));
	MojTestErrCheck(err);

	const MojByte* data = NULL;
	MojSize size;
	err = writer.buf().data(data, size);
	MojTestErrCheck(err);
	MojDataReader reader(data, size);
	MojUInt8 ui8val = 98;
	err = reader.readUInt8(ui8val);
	MojTestErrCheck(err);
	MojTestAssert(ui8val == 0);
	err = reader.readUInt8(ui8val);
	MojTestErrCheck(err);
	MojTestAssert(ui8val == 5);
	err = reader.readUInt8(ui8val);
	MojTestErrCheck(err);
	MojTestAssert(ui8val == 0xFF);
	MojUInt16 ui16val = 9999;
	err = reader.readUInt16(ui16val);
	MojTestErrCheck(err);
	MojTestAssert(ui16val == 0);
	err = reader.readUInt16(ui16val);
	MojTestErrCheck(err);
	MojTestAssert(ui16val == 256);
	err = reader.readUInt16(ui16val);
	MojTestErrCheck(err);
	MojTestAssert(ui16val == 0xFFFF);
	MojUInt32 ui32val = 89765;
	err = reader.readUInt32(ui32val);
	MojTestErrCheck(err);
	MojTestAssert(ui32val == 0);
	err = reader.readUInt32(ui32val);
	MojTestErrCheck(err);
	MojTestAssert(ui32val == 0x10000);
	err = reader.readUInt32(ui32val);
	MojTestErrCheck(err);
	MojTestAssert(ui32val == 0xFFFFFFFF);
	MojInt64 i64val = -98765432;
	err = reader.readInt64(i64val);
	MojTestErrCheck(err);
	MojTestAssert(i64val == 0);
	err = reader.readInt64(i64val);
	MojTestErrCheck(err);
	MojTestAssert(i64val == -56);
	err = reader.readInt64(i64val);
	MojTestErrCheck(err);
	MojTestAssert(i64val == 9223372036854775807LL);
	err = reader.readInt64(i64val);
	MojTestErrCheck(err);
	MojTestAssert(i64val == -9223372036854775807LL);
	MojDecimal decVal;
	err = reader.readDecimal(decVal);
	MojTestErrCheck(err);
	MojTestAssert(decVal == MojDecimal(3, 14159));
	err = reader.readDecimal(decVal);
	MojTestErrCheck(err);
	MojTestAssert(decVal == MojDecimal(-9999, 888888));


	MojDataReader reader2(NULL, 0);
	err = reader.readUInt8(ui8val);
	MojTestErrExpected(err, MojErrUnexpectedEof);
	err = reader.readUInt16(ui16val);
	MojTestErrExpected(err, MojErrUnexpectedEof);
	err = reader.readUInt32(ui32val);
	MojTestErrExpected(err, MojErrUnexpectedEof);
	err = reader.readInt64(i64val);
	MojTestErrExpected(err, MojErrUnexpectedEof);
	err = reader.readDecimal(decVal);
	MojTestErrExpected(err, MojErrUnexpectedEof);

	return MojErrNone;
}
