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


#ifndef MOJSTRING_H_
#define MOJSTRING_H_

#include "core/MojCoreDefs.h"
#include "core/MojAutoPtr.h"
#include "core/MojComp.h"
#include "core/MojHasher.h"
#include "core/MojVector.h"

class MojString
{
public:
	typedef MojChar* Iterator;
	typedef const MojChar* ConstIterator;

	static const MojString Empty;

	MojString() : m_begin(s_emptyString), m_end(s_emptyString), m_endAlloc(s_emptyString) {}
	MojString(const MojString& str) { init(str); }
	~MojString() { release(); }

	MojSize length() const { return m_end - m_begin; }
	MojSize capacity() const { return m_endAlloc - m_begin; }
	bool empty() const { return m_end == m_begin; }
	const MojChar* data() const { return m_begin; }
	MojChar at(MojSize idx) const { MojAssert(idx < length()); return data()[idx]; }
	MojChar first() const { MojAssert(!empty()); return *m_begin; }
	MojChar last() const { MojAssert(!empty()); return *(m_end - 1); }
	MojSize find(const MojChar* str, MojSize startIdx = 0) const;
	MojSize find(MojChar c, MojSize startIdx = 0) const;
	MojSize rfind(MojChar c, MojSize startIdx = MojInvalidIndex) const;
	MojErr split(MojChar c, MojVector<MojString>& vecOut) const;
	MojErr substring(MojSize idx, MojSize len, MojString& strOut) const;

	ConstIterator begin() const { return m_begin; }
	ConstIterator end() const { return m_end; }

	MojErr begin(Iterator& iterOut);
	MojErr end(Iterator& iterOut);

	int compare(const MojChar* str) const { return MojStrCmp(data(), str); }
	int compare(const MojChar* str, MojSize len) const { return MojStrNCmp(data(), str, len); }
	int compareCaseless(const MojChar* str) const { return MojStrCaseCmp(data(), str); }
	int compareCaseless(const MojChar* str, MojSize len) const { return MojStrNCaseCmp(data(), str, len); }
	bool startsWith(const MojChar* str) const;

	MojErr reserve(MojSize len);
	MojErr truncate(MojSize len);
	MojErr resize(MojSize len);
	MojErr toLower();
	MojErr toUpper();

	void clear();
	void swap(MojString& str) { MojSwap(*this, str); }
	void assign(const MojString& str);
	MojErr assign(const MojChar* str);
	MojErr assign(const MojChar* chars, MojSize len);
	MojErr setAt(MojSize idx, MojChar c);
	MojErr format(const MojChar* formatStr, ...) MOJ_FORMAT_ATTR((printf, 2, 3));
	MojErr vformat(const MojChar* formatStr, va_list args) MOJ_FORMAT_ATTR((printf, 2, 0));

	MojErr append(MojChar c);
	MojErr append(const MojChar* str);
	MojErr append(const MojChar* chars, MojSize len);
	MojErr append(const MojString& str) { return append(str, str.length()); }
	MojErr appendFormat(const MojChar* formatStr, ...) MOJ_FORMAT_ATTR((printf, 2, 3));
	MojErr appendVFormat(const MojChar* formatStr, va_list args) MOJ_FORMAT_ATTR((printf, 2, 0));

	MojErr base64Encode(const MojVector<MojByte>& vec, bool pad = true);
	MojErr base64Decode(MojVector<MojByte>& vecOut) const;

	MojString& operator=(const MojString& rhs) { assign(rhs); return *this; }
	bool operator==(const MojChar* rhs) const { return compare(rhs) == 0; }
	bool operator!=(const MojChar* rhs) const { return compare(rhs) != 0; }
	bool operator<(const MojChar* rhs) const { return compare(rhs) < 0; }
	bool operator<=(const MojChar* rhs) const { return compare(rhs) <= 0; }
	bool operator>(const MojChar* rhs) const { return compare(rhs) > 0; }
	bool operator>=(const MojChar* rhs) const { return compare(rhs) >= 0; }
	MojChar operator[](MojSize idx) const { return at(idx); }
	operator const MojChar*() const { return data(); }

private:
	void init(const MojString& str);
	void init(MojChar* begin, MojChar* end, MojChar* endAlloc);
	void release();
	void reset(MojChar* chars, MojSize len) { reset(chars, chars + len, chars + len); }
	void reset(MojChar* begin, MojChar* end, MojChar* endAlloc);
	bool isWritable() { return MojRefCountGet(m_begin) == 1; }
	static MojChar* alloc(MojSize len);
	MojErr realloc(MojSize allocLen);
	MojSize freeSpace() { return m_endAlloc - m_end; }
	MojErr ensureWritable();
	MojErr ensureSpace(MojSize len);
	void setEnd(MojSize len);

	MojChar* m_begin;
	MojChar* m_end;
	MojChar* m_endAlloc;
	static MojChar* s_emptyString;
};

template<>
struct MojComp<const MojString>
{
	int operator()(const MojString& val1, const MojString& val2)
	{
		return val1.compare(val2);
	}
};

template<>
struct MojComp<MojString> : public MojComp<const MojString> {};

template<>
struct MojHasher<MojString> : public MojHasher<const MojChar*> {};

#endif /* MOJSTRING_H_ */
