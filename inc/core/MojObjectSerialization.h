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


#ifndef MOJOBJECTSERIALIZATION_H_
#define MOJOBJECTSERIALIZATION_H_

#include "core/MojCoreDefs.h"
#include "core/MojDataSerialization.h"
#include "core/MojObject.h"
#include "core/MojTokenSet.h"

class MojObjectWriter : public MojObjectVisitor
{
public:
	// Note: Order of marker values is significant. It guarantees that a lexicographic comparison
	// of two serialized objects yields the same result as a comparison of the
	// objects themselves.
	enum Marker {
		MarkerInvalid = -1,
		MarkerObjectEnd = 0,
		MarkerNullValue = 1,
		MarkerObjectBegin = 2,
		MarkerArrayBegin = 3,
		MarkerStringValue = 4,
		MarkerFalseValue = 5,
		MarkerTrueValue = 6,
		MarkerNegativeDecimalValue = 7,
		MarkerPositiveDecimalValue = 8,
		MarkerNegativeIntValue = 9,
		MarkerZeroIntValue = 10,
		MarkerUInt8Value = 11,
		MarkerUInt16Value = 12,
		MarkerUInt32Value = 13,
		MarkerInt64Value = 14,
		MarkerExtensionValue = 15,
		MarkerHeaderEnd = 16
	};

	static const MojUInt8 Version = 1;
	static const MojUInt8 TokenStartMarker = 32;

	MojObjectWriter() : m_writer(m_buf), m_tokenSet(NULL) {}
	MojObjectWriter(MojBuffer& buf, MojTokenSet* tokenSet) : m_writer(buf), m_tokenSet(tokenSet) {}

	// MojObjectVisitor interface
	virtual MojErr reset();
	virtual MojErr beginObject();
	virtual MojErr endObject();
	virtual MojErr beginArray();
	virtual MojErr endArray();
	virtual MojErr propName(const MojChar* name, MojSize len);
	virtual MojErr nullValue();
	virtual MojErr boolValue(bool val);
	virtual MojErr intValue(MojInt64 val);
	virtual MojErr decimalValue(const MojDecimal& val);
	virtual MojErr stringValue(const MojChar* val, MojSize len);

	static MojSize nullSize();
	static MojSize boolSize();
	static MojSize intSize(MojInt64 val);
	static MojSize decimalSize(const MojDecimal& val);
	static MojSize stringSize(const MojChar* val, MojSize len);

	MojBuffer& buf() { return m_writer.buf(); }

private:
	MojErr writeString(const MojChar* name, MojSize len, bool addToken);

	MojBuffer m_buf;
	MojDataWriter m_writer;
	MojTokenSet* m_tokenSet;
};

class MojObjectReader : private MojNoCopy
{
public:
	MojObjectReader();
	MojObjectReader(const MojByte* data, MojSize size);

	const MojByte* begin() const { return m_reader.begin(); }
	const MojByte* end() const { return m_reader.end(); }
	const MojByte* pos() const { return m_reader.pos(); }

	MojDataReader& dataReader() { return m_reader; }
	bool hasNext() const { return m_reader.pos() < m_reader.end(); }
	MojObjectWriter::Marker peek() const;
	MojErr next(MojObjectVisitor& visitor);
	MojErr nextObject(MojObjectVisitor& visitor);

	void data(const MojByte* data, MojSize size);
	void skipBeginObj() { m_skipBeginObj = true; }
	void tokenSet(MojTokenSet* tokenSet) { m_tokenSet = tokenSet; }

	static MojErr read(MojObjectVisitor& visitor, const MojByte* data, MojSize size);
	static MojErr readInt(MojDataReader& dataReader, MojByte marker, MojInt64& valOut);
	static MojErr readString(MojDataReader& reader, MojChar const*& str, MojSize& strLen);

	MojErr read(MojObjectVisitor& visitor);

private:
	typedef enum {
		StateValue,
		StateObject,
		StatePropName,
		StateArray
	} State;

	MojDataReader m_reader;
	MojVector<MojByte> m_stack;
	bool m_skipBeginObj;
	MojTokenSet* m_tokenSet;
};

#endif /* MOJOBJECTSERIALIZATION_H_ */
