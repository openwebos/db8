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


/*
	Based on the json-c library:
	Copyright (c) 2004, 2005 Metaparadigm Pte Ltd

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	Done OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#include "core/MojJson.h"

static const MojChar* const MojJsonNullString = _T("null");
static const MojChar* const MojJsonTrueString = _T("true");
static const MojChar* const MojJsonFalseString = _T("false");

MojJsonWriter::MojJsonWriter()
: m_writeComma(false)
{
}

MojErr MojJsonWriter::reset()
{
	m_str.clear();
	m_writeComma = false;
	return MojErrNone;
}

MojErr MojJsonWriter::beginObject()
{
	MojErr err = m_str.reserve(InitialSize);
	MojErrCheck(err);
	err = writeComma();
	MojErrCheck(err);
	err = m_str.append(_T('{'));
	MojErrCheck(err);
	m_writeComma = false;
	return MojErrNone;
}

MojErr MojJsonWriter::endObject()
{
	MojErr err = m_str.append(_T('}'));
	MojErrCheck(err);
	m_writeComma = true;
	return MojErrNone;
}

MojErr MojJsonWriter::beginArray()
{
	MojErr err = writeComma();
	MojErrCheck(err);
	err = m_str.append(_T('['));
	MojErrCheck(err);
	m_writeComma = false;
	return MojErrNone;
}

MojErr MojJsonWriter::endArray()
{
	MojErr err = m_str.append(_T(']'));
	MojErrCheck(err);
	m_writeComma = true;
	return MojErrNone;
}

MojErr MojJsonWriter::propName(const MojChar* name, gsize len)
{
	MojAssert(name);

	MojErr err = writeComma();
	MojErrCheck(err);
	err = writeString(name, len);
	MojErrCheck(err);
	err = m_str.append(_T(':'));
	MojErrCheck(err);
	m_writeComma = false;
	return MojErrNone;
}

MojErr MojJsonWriter::nullValue()
{
	MojErr err = writeComma();
	MojErrCheck(err);
	err = m_str.append(MojJsonNullString);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojJsonWriter::boolValue(bool val)
{
	MojErr err = writeComma();
	MojErrCheck(err);
	err = m_str.append(val ? MojJsonTrueString : MojJsonFalseString);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojJsonWriter::intValue(gint64 val)
{
	MojErr err = writeComma();
	MojErrCheck(err);
	err = m_str.appendFormat(_T("%lld"), val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojJsonWriter::decimalValue(const MojDecimal& val)
{
	MojErr err = writeComma();
	MojErrCheck(err);
	MojChar buf[MojDecimal::MaxStringSize];
	err = val.stringValue(buf, MojDecimal::MaxStringSize);
	MojErrCheck(err);
	err = m_str.append(buf);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojJsonWriter::stringValue(const MojChar* val, gsize len)
{
	MojErr err = writeComma();
	MojErrCheck(err);
	err = writeString(val, len);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojJsonWriter::writeString(const MojChar* val, gsize len)
{
	MojAssert(val || len == 0);
	static const char escape[] = {
		/*       0,  1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
		/* 0 */  1,  1,  1,  1,  1,  1,  1,  1,'b','t','n',  1,'f','r',  1,  1,
		/* 1 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		/* 2 */  0,  0,'"',  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		/* 3 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		/* 4 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		/* 5 */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,'\\', 0,  0,  0
	};

	MojErr err = m_str.append(_T('"'));
	MojErrCheck(err);

	const MojChar* cur = val;
	const MojChar* end = val + len;
	while (cur < end) {
		unsigned int c = (unsigned int) *cur;
		if (c < (int) sizeof(escape) && escape[c]) {
			err = m_str.append(val, cur - val);
			MojErrCheck(err);
			val = cur + 1;
			if (escape[c] == 1) {
				err = m_str.appendFormat(_T("\\u%04X"), c);
				MojErrCheck(err);
			} else {
				err = m_str.append(_T('\\'));
				MojErrCheck(err);
				err = m_str.append(escape[c]);
				MojErrCheck(err);
			}
		}
		cur++;
	}
	err = m_str.append(val, cur - val);
	MojErrCheck(err);
	err = m_str.append(_T('"'));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojJsonWriter::writeComma()
{
	if (m_writeComma) {
		MojErr err = m_str.append(_T(','));
		MojErrCheck(err);
	} else {
		m_writeComma = true;
	}
	return MojErrNone;
}

MojJsonParser::MojJsonParser()
: m_line(0),
  m_col(0),
  m_strPos(0),
  m_depth(0),
  m_ucsChar(0),
  m_matchStr(NULL),
  m_isDecimal(false)
{
	MojZero(m_stack, sizeof(m_stack));
}

MojJsonParser::~MojJsonParser()
{
}

void MojJsonParser::begin()
{
	m_line = 1;
	m_col = 1;
	resetRec();
}

MojErr MojJsonParser::end(MojObjectVisitor& visitor)
{
	MojChar c = _T('\0');
	const MojChar* parseEnd = NULL;
	MojErr err = parseChunk(visitor, &c, 1, parseEnd);
	MojErrCheck(err);

	return MojErrNone;
}

bool MojJsonParser::finished()
{
	return (state() == StateFinish && m_depth == 0);
}

MojErr MojJsonParser::parse(MojObjectVisitor& visitor, const MojChar* chars, gsize len)
{
	MojAssert(chars || len == 0);

	MojJsonParser parser;
	MojErr err = MojErrNone;
	parser.begin();

	const MojChar* parseEnd = NULL;
	err = parser.parseChunk(visitor, chars, len, parseEnd);
	MojErrCheck(err);

	err = parser.end(visitor);
	MojErrCheck(err);

	if (!parser.finished())
		MojErrThrow(MojErrJsonParseEof);

	return MojErrNone;
}

MojErr MojJsonParser::parseChunk(MojObjectVisitor& visitor, const MojChar* chars, gsize len, const MojChar*& parseEnd)
{
	MojErr err = MojErrNone;
	const MojChar* end = (len == G_MAXSIZE) ? NULL : chars + len;

	MojChar c;
	while (chars != end) {
		c = *chars;
		if (c == '\n') {
			++m_line;
			m_col = 1;
		}

Redo:	switch (state()) {
		case StateEatWhitespace:
			if (MojIsSpace(c)) {
				/* okay */
			} else if (c == '/') {
				state() = StateCommentStart;
			} else {
				state() = savedState();
				goto Redo;
			}
			break;

		case StateStart:
			switch (c) {
			case '{':
				state() = StateEatWhitespace;
				savedState() = StateObjFieldStart;
				err = visitor.beginObject();
				MojErrCheck(err);
				break;
			case '[':
				state() = StateEatWhitespace;
				savedState() = StateArray;
				err = visitor.beginArray();
				MojErrCheck(err);
				break;
			case 'n':
				state() = StateNull;
				m_strPos = 0;
				goto Redo;
			case '"':
				state() = StateString;
				m_str.clear();
				break;
			case 't':
			case 'f':
				state() = StateBool;
				m_strPos = 0;
				m_matchStr = (c == 't') ? MojJsonTrueString : MojJsonFalseString;
				goto Redo;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '-':
				state() = StateNumber;
				m_str.clear();
				m_isDecimal = false;
				goto Redo;
			case '\0':
				break;
			default:
				MojErrThrowMsg(MojErrJsonParseUnexpected, _T("json: unexpected char at %d:%d"), m_line, m_col);
			}
			break;

		case StateFinish:
			if (m_depth == 0)
				goto Done;
			m_depth--;
			goto Redo;

		case StateNull:
			if (c != MojJsonNullString[m_strPos])
				MojErrThrowMsg(MojErrJsonParseNull, _T("json: error parsing null at %d:%d"), m_line, m_col);
			if (MojJsonNullString[++m_strPos] == _T('\0')) {
				err = visitor.nullValue();
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			}
			break;

		case StateCommentStart:
			if (c == '*') {
				state() = StateComment;
			} else if (c == '/') {
				state() = StateCommentEol;
			} else {
				MojErrThrow(MojErrJsonParseComment);
			}
			break;

		case StateComment:
			if (c == '*')
				state() = StateCommentEnd;
			break;

		case StateCommentEol:
			if (c == '\n') {
				state() = StateEatWhitespace;
			}
			break;

		case StateCommentEnd:
			if (c == '/') {
				state() = StateEatWhitespace;
			} else {
				state() = StateComment;
				goto Redo;
			}
			break;

		case StateString:
			if (c == '"') {
				err = visitor.stringValue(m_str, m_str.length());
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			} else if (c == '\\') {
				savedState() = StateString;
				state() = StateStringEscape;
			} else {
				err = m_str.append(c);
				MojErrCheck(err);
			}
			break;

		case StateStringEscape:
			if (c == 'u'){
				m_ucsChar = 0;
				m_strPos = 0;
				state() = StateEscapeUnicode;
			} else {
				MojChar escapeChar;
				switch (c) {
				case '"':
				case '\\':
				case '/':
					escapeChar = c;
					break;
				case 'b':
					escapeChar = _T('\b');
					break;
				case 'n':
					escapeChar = _T('\n');
					break;
				case 'r':
					escapeChar = _T('\r');
					break;
				case 't':
					escapeChar = _T('\t');
					break;
				case 'f':
					escapeChar = _T('\f');
					break;
				default:
					MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing string at %d:%d"), m_line, m_col);
				}
				err = m_str.append(escapeChar);
				MojErrCheck(err);
				state() = savedState();
			}
			break;

		case StateEscapeUnicode:
			if (MojIsHexDigit(c)) {
				m_ucsChar += ((guint32) hexDigit(c) << ((3 - m_strPos++) * 4));
				if (m_strPos == 4) {
					MojChar utfOut[3];
					gsize utfLen = 0;
					if (m_ucsChar == 0) {
						MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing escape sequence - null character not allowed at %d:%d"), m_line, m_col);
					}
					if (m_ucsChar < 0x80) {
						utfOut[0] = (MojChar) m_ucsChar;
						utfLen = 1;
					} else if (m_ucsChar < 0x800) {
						utfOut[0] = (MojChar) (0xc0 | (m_ucsChar >> 6));
						utfOut[1] = (MojChar) (0x80 | (m_ucsChar & 0x3f));
						utfLen = 2;
					} else {
						utfOut[0] = (MojChar) (0xe0 | (m_ucsChar >> 12));
						utfOut[1] = (MojChar) (0x80 | ((m_ucsChar >> 6) & 0x3f));
						utfOut[2] = (MojChar) (0x80 | (m_ucsChar & 0x3f));
						utfLen = 3;
					}
					err = m_str.append(utfOut, utfLen);
					MojErrCheck(err);
					state() = savedState();
				}
			} else {
				MojErrThrowMsg(MojErrJsonParseEscape, _T("json: error parsing escape sequence at %d:%d"), m_line, m_col);
			}
			break;

		case StateBool:
			if (c != m_matchStr[m_strPos])
				MojErrThrowMsg(MojErrJsonParseBool, _T("json: error parsing bool at %d:%d"), m_line, m_col);
			if (m_matchStr[++m_strPos] == _T('\0')) {
				err = visitor.boolValue(m_matchStr == MojJsonTrueString);
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			}
			break;

		case StateNumber:
			switch (c) {
			case '.':
			case 'e':
			case 'E':
				m_isDecimal = true;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '+':
			case '-': {
				err = m_str.append(c);
				MojErrCheck(err);
				break;
			}
			default: {
				const MojChar* numberEnd;
				if (m_isDecimal) {
					MojDecimal d;
					err = d.assign(m_str);
					MojErrCheck(err);
					err = visitor.decimalValue(d);
					MojErrCheck(err);
				} else {
					gint64 i = MojStrToInt64(m_str, &numberEnd, 0);
					err = visitor.intValue(i);
					MojErrCheck(err);
					if (numberEnd != m_str.end())
						MojErrThrowMsg(MojErrJsonParseInt, _T("json: error parsing int at %d:%d"), m_line, m_col);
				}
				savedState() = StateFinish;
				state() = StateEatWhitespace;
				goto Redo;
			}
			}
			break;

		case StateArray:
			if (c == ']') {
				err = visitor.endArray();
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			} else {
				state() = StateArraySep;
				err = push();
				MojErrCheck(err);
				goto Redo;
			}
			break;

		case StateArraySep:
			if (c == ']') {
				err = visitor.endArray();
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			} else if (c == ',') {
				savedState() = StateArray;
				state() = StateEatWhitespace;
			} else {
				MojErrThrowMsg(MojErrJsonParseArray, _T("json: error parsing array at %d:%d"), m_line, m_col);
			}
			break;

		case StateObjFieldStart:
			if (c == '}') {
				err = visitor.endObject();
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			} else if (c == '"') {
				m_str.clear();
				state() = StateObjField;
			} else {
				MojErrThrowMsg(MojErrJsonParsePropName, _T("json: error parsing prop name at %d:%d"), m_line, m_col);
			}
			break;

		case StateObjField:
			if (c == '"') {
				err = visitor.propName(m_str, m_str.length());
				MojErrCheck(err);
				savedState() = StateObjFieldEnd;
				state() = StateEatWhitespace;
			} else if (c == '\\') {
				savedState() = StateObjField;
				state() = StateStringEscape;
			} else {
				err = m_str.append(c);
				MojErrCheck(err);
			}
			break;

		case StateObjFieldEnd:
			if (c == ':') {
				savedState() = StateObjValue;
				state() = StateEatWhitespace;
			} else {
				MojErrThrowMsg(MojErrJsonParsePropName, _T("json: error parsing prop name at %d:%d"), m_line, m_col);
			}
			break;

		case StateObjValue:
			savedState() = StateObjSep;
			state() = StateEatWhitespace;
			err = push();
			MojErrCheck(err);
			goto Redo;

		case StateObjSep:
			if (c == '}') {
				err = visitor.endObject();
				MojErrCheck(err);
				savedState() = StateFinish;
				state() = StateEatWhitespace;
			} else if (c == ',') {
				savedState() = StateObjFieldStart;
				state() = StateEatWhitespace;
			} else {
				MojErrThrowMsg(MojErrJsonParseValueSep, _T("json: error parsing value separator at %d:%d"), m_line, m_col);
			}
			break;

		default:
			MojAssertNotReached();
		}
		++chars;
		++m_col;
	}

Done:
	parseEnd = chars;
	return MojErrNone;
}

MojErr MojJsonParser::push()
{
	if (m_depth >= MaxDepth - 1)
		MojErrThrowMsg(MojErrJsonParseMaxDepth, _T("json: max depth exceeded at %d:%d"), m_line, m_col);
	m_depth++;
	resetRec();
	return MojErrNone;
}

void MojJsonParser::resetRec()
{
	state() = StateEatWhitespace;
	savedState() = StateStart;
}
