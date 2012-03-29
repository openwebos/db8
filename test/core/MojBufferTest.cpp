/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#include "MojBufferTest.h"
#include "core/MojBuffer.h"

MojBufferTest::MojBufferTest()
: MojTestCase(_T("MojBuffer"))
{
}

MojErr MojBufferTest::run()
{
	// empty test
	MojBuffer buf1;
	MojTestAssert(buf1.empty());
	MojByte b;
	const MojByte* data = &b;
	MojSize size = 20;
	MojErr err = buf1.data(data, size);
	MojTestErrCheck(err);
	MojAutoPtr<MojBuffer::Chunk> chunk;
	err = buf1.release(chunk);
	MojTestErrCheck(err);
	MojTestAssert(chunk.get() == NULL);
	MojTestAssert(data == NULL && size == 0);
	MojIoVecT vec[10];
	size = 15;
	buf1.iovec(vec, 10, size);
	MojTestAssert(size == 0);
	buf1.iovec(vec, 0, size);
	MojTestAssert(size == 0);
	buf1.advance(0);
	buf1.clear();

	buf1.writeByte(4);
	MojTestAssert(!buf1.empty());
	MojBuffer buf2(buf1);
	MojTestAssert(!buf2.empty());
	MojTestAssert(buf1.empty());
	buf1 = buf2;
	MojTestAssert(!buf1.empty());
	MojTestAssert(buf2.empty());
	buf1.clear();
	MojTestAssert(buf1.empty());

	// write/read 1 byte at a time
	for (int i = 0; i < 10000; ++i) {
		b = (MojByte) i;
		if (i % 2) {
			err = buf1.writeByte(b);
			MojTestErrCheck(err);
		} else {
			err = buf1.write(&b, 1);
			MojTestErrCheck(err);
		}
		MojTestAssert(!buf1.empty());
		size = 53;
		buf1.iovec(vec, 10, size);
		MojTestAssert(size == 1 && vec[0].iov_len == 1 && *(MojByte*)(vec[0].iov_base) == b);
		buf1.advance(1);
		MojTestAssert(buf1.empty());
	}

	// write/read 2 bytes at a time
	for (int i = 0; i < 10000; ++i) {
		MojByte b2[2];
		b2[0] = (MojByte) i;
		b2[1] = (MojByte) (i >> 8);
		if (i % 2) {
			err = buf1.writeByte(b2[0]);
			MojTestErrCheck(err);
			err = buf1.writeByte(b2[1]);
			MojTestErrCheck(err);
		} else {
			err = buf1.write(b2, 2);
			MojTestErrCheck(err);
		}
		MojTestAssert(!buf1.empty());
		size = 53;
		buf1.iovec(vec, 10, size);
		MojTestAssert(size == 1 || size == 2);
		if (size == 1) {
			MojByte* base = (MojByte*)(vec[0].iov_base);
			MojTestAssert(vec[0].iov_len == 2 && base[0] == b2[0] && base[1] == b2[1]);
		} else {
			MojByte* base = (MojByte*)(vec[0].iov_base);
			MojTestAssert(vec[0].iov_len == 1 && base[0] == b2[0]);
			base = (MojByte*)(vec[1].iov_base);
			MojTestAssert(vec[1].iov_len == 1 && base[0] == b2[1]);
		}
		buf1.advance(2);
		MojTestAssert(buf1.empty());
	}

	// write/read 10000 bytes at a time
	MojAutoArrayPtr<MojByte> bytes(new MojByte[10000]);
	MojAllocCheck(bytes.get());
	MojZero(bytes.get(), 10000);
	for (int i = 0; i < 100; ++i) {
		err = buf1.write(bytes.get(), 10000);
		MojTestErrCheck(err);
		MojTestAssert(!buf1.empty());
		if (i % 2) {
			// read it all in one chunk
			size = 53;
			buf1.iovec(vec, 10, size);
			MojSize dataSize = 0;
			for (MojSize j = 0; j < size; ++j) {
				dataSize += vec[j].iov_len;
			}
			MojTestAssert(dataSize == 10000);
			buf1.advance(10000);
		} else {
			// read it one chunk at a time
			MojSize dataSize = 10000;
			while (dataSize > 0) {
				size = 53;
				buf1.iovec(vec, 1, size);
				MojTestAssert(size == 1);
				dataSize -= vec[0].iov_len;
				buf1.advance(vec[0].iov_len);
			}
		}
		MojTestAssert(buf1.empty());
	}
	return MojErrNone;
}

