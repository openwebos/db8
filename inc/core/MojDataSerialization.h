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


#ifndef MOJDATAWRITER_H_
#define MOJDATAWRITER_H_

#include "core/MojCoreDefs.h"
#include "core/MojBuffer.h"

class MojDataWriter
{
public:
	MojDataWriter(MojBuffer& buf) : m_buf(buf) {}

	void clear() { m_buf.clear(); }
	MojErr writeUInt8(guint8 val) { return m_buf.writeByte(val); }
	MojErr writeUInt16(guint16 val);
	MojErr writeUInt32(guint32 val);
	MojErr writeInt64(gint64 val);
	MojErr writeDecimal(const MojDecimal& val);
	MojErr writeChars(const MojChar* chars, gsize len);

	static gsize sizeChars(const MojChar* chars, gsize len);
	static gsize sizeDecimal(const MojDecimal& val);

	MojBuffer& buf() { return m_buf; }

private:
	MojBuffer& m_buf;
};

class MojDataReader
{
public:
	MojDataReader();
	MojDataReader(const guint8* data, gsize size);

	void data(const guint8* data, gsize size);
	MojErr readUInt8(guint8& val);
	MojErr readUInt16(guint16& val);
	MojErr readUInt32(guint32& val);
	MojErr readInt64(gint64& val);
	MojErr readDecimal(MojDecimal& val);
	MojErr skip(gsize len);

	const guint8* begin() const { return m_begin; }
	const guint8* end() const { return m_end; }
	const guint8* pos() const { return m_pos; }

	gsize available() const { return m_end - m_pos; }

private:

	const guint8* m_begin;
	const guint8* m_end;
	const guint8* m_pos;
};

#endif /* MOJDATAWRITER_H_ */
