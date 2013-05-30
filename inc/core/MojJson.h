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


#ifndef MOJJSONPARSER_H_
#define MOJJSONPARSER_H_

#include "core/MojCoreDefs.h"
#include "core/MojObject.h"

class MojJsonWriter : public MojObjectVisitor
{
public:
	using MojObjectVisitor::propName;

	MojJsonWriter();

	virtual MojErr reset();
	virtual MojErr beginObject();
	virtual MojErr endObject();
	virtual MojErr beginArray();
	virtual MojErr endArray();
	virtual MojErr propName(const MojChar* name, gsize len);
	virtual MojErr nullValue();
	virtual MojErr boolValue(bool val);
	virtual MojErr intValue(gint64 val);
	virtual MojErr decimalValue(const MojDecimal& val);
	virtual MojErr stringValue(const MojChar* val, gsize len);

	const MojString& json() const { return m_str; }

private:
	static const gsize InitialSize = 1024;

	MojErr writeString(const MojChar* val, gsize len);
	MojErr writeComma();

	bool m_writeComma;
	MojString m_str;
};

class MojJsonParser
{
public:
	MojJsonParser();
	~MojJsonParser();

	void begin();
	MojErr end(MojObjectVisitor& visitor);
	bool finished();
	static MojErr parse(MojObjectVisitor& visitor, const MojChar* str) { return parse(visitor, str, G_MAXSIZE); }
	static MojErr parse(MojObjectVisitor& visitor, const MojChar* chars, gsize len);
	MojErr parseChunk(MojObjectVisitor& visitor, const MojChar* chars, gsize len, const MojChar*& parseEnd);

	guint32 line() { return m_line; }
	guint32 column() { return m_col; }

private:
	static const gsize MaxDepth = 32;

	typedef enum {
		StateEatWhitespace,
		StateStart,
		StateFinish,
		StateNull,
		StateCommentStart,
		StateComment,
		StateCommentEol,
		StateCommentEnd,
		StateString,
		StateStringEscape,
		StateEscapeUnicode,
		StateBool,
		StateNumber,
		StateArray,
		StateArraySep,
		StateObjFieldStart,
		StateObjField,
		StateObjFieldEnd,
		StateObjValue,
		StateObjSep
	} State;

	struct StackRec
	{
		State m_state;
		State m_savedState;
	};

	State& state() { return m_stack[m_depth].m_state; }
	State& savedState() { return m_stack[m_depth].m_savedState; }
	int hexDigit(MojChar c) { return (c <= _T('9')) ? c - _T('0') : (c & 7) + 9; }
	MojErr push();
	void resetRec();

	guint32 m_line;
	guint32 m_col;
	gsize m_strPos;
	gsize m_depth;
	guint32 m_ucsChar;
	const MojChar* m_matchStr;
	bool m_isDecimal;
	MojString m_str;
	StackRec m_stack[MaxDepth];
};

#endif /* MOJJSONPARSER_H_ */
