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


#ifndef MOJDATAWRITER_H_
#define MOJDATAWRITER_H_

#include "core/MojCoreDefs.h"
#include "core/MojBuffer.h"

class MojDataWriter
{
public:
	MojDataWriter(MojBuffer& buf) : m_buf(buf) {}

	void clear() { m_buf.clear(); }
	MojErr writeUInt8(MojByte val) { return m_buf.writeByte(val); }
	MojErr writeUInt16(MojUInt16 val);
	MojErr writeUInt32(MojUInt32 val);
	MojErr writeInt64(MojInt64 val);
	MojErr writeDecimal(const MojDecimal& val);
	MojErr writeChars(const MojChar* chars, MojSize len);

	static MojSize sizeChars(const MojChar* chars, MojSize len);
	static MojSize sizeDecimal(const MojDecimal& val);

	MojBuffer& buf() { return m_buf; }

private:
	MojBuffer& m_buf;
};

class MojDataReader
{
public:
	MojDataReader();
	MojDataReader(const MojByte* data, MojSize size);

	void data(const MojByte* data, MojSize size);
	MojErr readUInt8(MojByte& val);
	MojErr readUInt16(MojUInt16& val);
	MojErr readUInt32(MojUInt32& val);
	MojErr readInt64(MojInt64& val);
	MojErr readDecimal(MojDecimal& val);
	MojErr skip(MojSize len);

	const MojByte* begin() const { return m_begin; }
	const MojByte* end() const { return m_end; }
	const MojByte* pos() const { return m_pos; }

	MojSize available() const { return m_end - m_pos; }

private:

	const MojByte* m_begin;
	const MojByte* m_end;
	const MojByte* m_pos;
};

#endif /* MOJDATAWRITER_H_ */
