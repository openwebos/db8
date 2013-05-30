/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics
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


#include "core/MojObjectSerialization.h"

MojErr MojObjectWriter::reset()
{
    m_writer.clear();
    m_tokenSet = NULL;
    return MojErrNone;
}

MojErr MojObjectWriter::beginObject()
{
    MojErr err = m_writer.writeUInt8(MarkerObjectBegin);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojObjectWriter::endObject()
{
    MojErr err = m_writer.writeUInt8(MarkerObjectEnd);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojObjectWriter::beginArray()
{
    MojErr err = m_writer.writeUInt8(MarkerArrayBegin);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojObjectWriter::endArray()
{
    MojErr err = m_writer.writeUInt8(MarkerObjectEnd);
    MojErrCheck(err);
    return MojErrNone;
}

MojErr MojObjectWriter::propName(const MojChar* name, gsize len)
{
    MojErr err = writeString(name, len, true);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojObjectWriter::nullValue()
{
    MojErr err = m_writer.writeUInt8(MarkerNullValue);
    MojErrCheck(err);
    return MojErrNone;
}

gsize MojObjectWriter::nullSize()
{
    return 1;
}

MojErr MojObjectWriter::boolValue(bool val)
{
    MojErr err = m_writer.writeUInt8(val ? MarkerTrueValue : MarkerFalseValue);
    MojErrCheck(err);
    return MojErrNone;
}

gsize MojObjectWriter::boolSize()
{
    return 1;
}

MojErr MojObjectWriter::intValue(gint64 val)
{
    // TODO: simplify integer encoding
    if (val < 0) {
        MojErr err = m_writer.writeUInt8(MarkerNegativeIntValue);
        MojErrCheck(err);
        err = m_writer.writeInt64(val);
        MojErrCheck(err);
    } else if (val == 0) {
        MojErr err = m_writer.writeUInt8(MarkerZeroIntValue);
        MojErrCheck(err);
    } else if (val <= G_MAXUINT8) {
        MojErr err = m_writer.writeUInt8(MarkerUInt8Value);
        MojErrCheck(err);
        err = m_writer.writeUInt8((guint8) val);
        MojErrCheck(err);
    } else if (val <= G_MAXUINT16) {
        MojErr err = m_writer.writeUInt8(MarkerUInt16Value);
        MojErrCheck(err);
        err = m_writer.writeUInt16((guint16) val);
        MojErrCheck(err);
    } else if (val <= G_MAXUINT32) {
        MojErr err = m_writer.writeUInt8(MarkerUInt32Value);
        MojErrCheck(err);
        err = m_writer.writeUInt32((guint32) val);
        MojErrCheck(err);
    } else {
        MojErr err = m_writer.writeUInt8(MarkerInt64Value);
        MojErrCheck(err);
        err = m_writer.writeInt64(val);
        MojErrCheck(err);
    }
    return MojErrNone;
}

gsize MojObjectWriter::intSize(gint64 val)
{
    gsize size = 0;
    if (val < 0) {
        size = 1 + sizeof(gint64);
    } else if (val == 0) {
        size = 1;
    } else if (val <= G_MAXUINT8) {
        size = 1 + sizeof(guint8);
    } else if (val <= G_MAXUINT16) {
        size = 1 + sizeof(guint16);
    } else if (val <= G_MAXUINT32) {
        size = 1 + sizeof(guint32);
    } else {
        size = 1 + sizeof(gint64);
    }
    return size;
}

MojErr MojObjectWriter::decimalValue(const MojDecimal& val)
{
    Marker marker = val.rep() < 0 ? MarkerNegativeDecimalValue : MarkerPositiveDecimalValue;
    MojErr err = m_writer.writeUInt8(marker);
    MojErrCheck(err);
    err = m_writer.writeDecimal(val);
    MojErrCheck(err);
    return MojErrNone;
}

gsize MojObjectWriter::decimalSize(const MojDecimal& val)
{
    return 1 + MojDataWriter::sizeDecimal(val);
}

MojErr MojObjectWriter::stringValue(const MojChar* val, gsize len)
{
    MojErr err = writeString(val, len, false);
    MojErrCheck(err);

    return MojErrNone;
}

gsize MojObjectWriter::stringSize(const MojChar* val, gsize len)
{
    return 2 + MojDataWriter::sizeChars(val, len);
}

MojErr MojObjectWriter::writeString(const MojChar* val, gsize len, bool addToken)
{
    if (m_tokenSet) {
        guint8 token = MojTokenSet::InvalidToken;
        MojErr err = m_tokenSet->tokenFromString(val, token, addToken);
        MojErrCheck(err);
        if (token != MojTokenSet::InvalidToken) {
            err = m_writer.writeUInt8(token);
            MojErrCheck(err);
            return MojErrNone;
        }
    }
    MojErr err = m_writer.writeUInt8(MarkerStringValue);
    MojErrCheck(err);
    err = m_writer.writeChars(val, len);
    MojErrCheck(err);
    err = m_writer.writeUInt8(0);
    MojErrCheck(err);

    return MojErrNone;
}

MojObjectReader::MojObjectReader()
: m_skipBeginObj(false),
  m_tokenSet(NULL)
{

}

MojObjectReader::MojObjectReader(const guint8* data, gsize size)
: m_reader(data, size),
  m_skipBeginObj(false),
  m_tokenSet(NULL)
{
    MojAssert(data || size == 0);
}

void MojObjectReader::data(const guint8* data, gsize size)
{
    m_reader.data(data, size);
    m_skipBeginObj = false;
    m_tokenSet = NULL;
}

MojObjectWriter::Marker MojObjectReader::peek() const
{
    if (pos() == end())
        return MojObjectWriter::MarkerInvalid;
    return (MojObjectWriter::Marker) *pos();
}

MojErr MojObjectReader::nextObject(MojObjectVisitor& visitor)
{
    do {
        MojErr err = next(visitor);
        MojErrCheck(err);
    } while (!m_stack.empty());

    return MojErrNone;
}

MojErr MojObjectReader::next(MojObjectVisitor& visitor)
{
    const MojChar* str = NULL;
    gsize strLen = 0;
    MojErr err = MojErrNone;

    if (m_stack.empty()) {
        err = m_stack.push(StateValue);
        MojErrCheck(err);
    }

    guint8 marker;
    err = m_reader.readUInt8(marker);
    MojErrCheck(err);

Redo:
    State state = (State) m_stack.back();
    switch (state) {
    case StateValue:
        err = m_stack.pop();
        MojErrCheck(err);
        switch (marker) {
        case MojObjectWriter::MarkerObjectBegin: {
            err = m_stack.push(StateObject);
            MojErrCheck(err);
            if (!m_skipBeginObj) {
                err = visitor.beginObject();
                MojErrCheck(err);
            } else {
                m_skipBeginObj = false;
            }
            break;
        }
        case MojObjectWriter::MarkerArrayBegin: {
            err = m_stack.push(StateArray);
            MojErrCheck(err);
            err = visitor.beginArray();
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerStringValue: {
            err = readString(m_reader, str, strLen);
            MojErrCheck(err);
            err = visitor.stringValue(str, strLen);
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerNullValue: {
            err = visitor.nullValue();
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerTrueValue: {
            err = visitor.boolValue(true);
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerFalseValue: {
            err = visitor.boolValue(false);
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerNegativeDecimalValue:
        case MojObjectWriter::MarkerPositiveDecimalValue: {
            MojDecimal val;
            err = m_reader.readDecimal(val);
            MojErrCheck(err);
            err = visitor.decimalValue(val);
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerZeroIntValue:
        case MojObjectWriter::MarkerUInt8Value:
        case MojObjectWriter::MarkerUInt16Value:
        case MojObjectWriter::MarkerUInt32Value:
        case MojObjectWriter::MarkerNegativeIntValue:
        case MojObjectWriter::MarkerInt64Value: {
            gint64 val;
            err = readInt(m_reader, marker, val);
            MojErrCheck(err);
            err = visitor.intValue(val);
            MojErrCheck(err);
            break;
        }
        case MojObjectWriter::MarkerExtensionValue: {
            guint32 extSize;
            err = m_reader.readUInt32(extSize);
            MojErrCheck(err);
            err = m_reader.skip(extSize);
            MojErrCheck(err);
            break;
        }
        default: {
            if (m_tokenSet) {
                MojString val;
                err = m_tokenSet->stringFromToken(marker, val);
                MojErrCheck(err);
                err = visitor.stringValue(val);
                MojErrCheck(err);
                break;
            }
            MojErrThrow(MojErrObjectReaderUnexpectedMarker);
        }
        }
        break;
    case StateObject:
        switch (marker) {
        case MojObjectWriter::MarkerObjectEnd:
            err = visitor.endObject();
            MojErrCheck(err);
            err = m_stack.pop();
            MojErrCheck(err);
            break;
        case MojObjectWriter::MarkerStringValue:
            err = readString(m_reader, str, strLen);
            MojErrCheck(err);
            err = visitor.propName(str, strLen);
            MojErrCheck(err);
            err = m_stack.push(StateValue);
            MojErrCheck(err);
            break;
        default:
            // if a token set exists, look up this marker
            if (m_tokenSet) {
                MojString name;
                err = m_tokenSet->stringFromToken(marker, name);
                MojErrCheck(err);
                err = visitor.propName(name);
                MojErrCheck(err);
                err = m_stack.push(StateValue);
                MojErrCheck(err);
                break;
            }

            // TODO: Do not stop execution, retest it
            break;
            //MojErrThrow(MojErrObjectReaderUnexpectedMarker);
        }
        break;
    case StateArray:
        switch (marker) {
        case MojObjectWriter::MarkerObjectEnd:
            err = visitor.endArray();
            MojErrCheck(err);
            err = m_stack.pop();
            MojErrCheck(err);
            break;
        default:
            err = m_stack.push(StateValue);
            MojErrCheck(err);
            goto Redo;
        }
        break;
    default:
        MojAssertNotReached();
    }
    return MojErrNone;
}

MojErr MojObjectReader::read(MojObjectVisitor& visitor, const guint8* data, gsize size)
{
    MojObjectReader reader(data, size);
    MojErr err = reader.read(visitor);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojObjectReader::readInt(MojDataReader& dataReader, guint8 marker, gint64& valOut)
{
    MojErr err = MojErrNone;
    switch (marker) {
    case MojObjectWriter::MarkerZeroIntValue:
        valOut = 0;
        break;
    case MojObjectWriter::MarkerUInt8Value:
        guint8 val;
        err = dataReader.readUInt8(val);
        MojErrCheck(err);
        valOut = (gint64) val;
        break;
    case MojObjectWriter::MarkerUInt16Value:
        guint16 val16;
        err = dataReader.readUInt16(val16);
        MojErrCheck(err);
        valOut = (gint64) val16;
        break;
    case MojObjectWriter::MarkerUInt32Value:
        guint32 val32;
        err = dataReader.readUInt32(val32);
        MojErrCheck(err);
        valOut = (gint64) val32;
        break;
    case MojObjectWriter::MarkerNegativeIntValue:
    case MojObjectWriter::MarkerInt64Value:
        err = dataReader.readInt64(valOut);
        MojErrCheck(err);
        break;
    default:
        //throw error here
        MojErrThrow(MojErrObjectReaderUnexpectedMarker);
    }

    return MojErrNone;
}

MojErr MojObjectReader::readString(MojDataReader& reader, MojChar const*& str, gsize& strLen)
{
#ifdef MOJ_ENCODING_UTF8
    const MojChar* pos = (const MojChar*) reader.pos();
    str = pos;
    for (;;) {
        if (pos == (const MojChar*) reader.end())
            MojErrThrow(MojErrUnexpectedEof);
        if (*pos == 0)
            break;
        ++pos;
    }
    strLen = pos - str;
    MojErr err = reader.skip(strLen + 1);
    MojErrCheck(err);

    return MojErrNone;
#endif /* MOJ_ENCODING_UTF8 */
}

MojErr MojObjectReader::read(MojObjectVisitor& visitor)
{
    // TODO: remove this debug
    while (hasNext()) {
        MojErr err = next(visitor);
        MojErrCheck(err);
    }
    return MojErrNone;
}
