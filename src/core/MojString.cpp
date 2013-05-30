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


#include "core/MojString.h"

#define MojStringAssertValid()  MojAssert(m_begin && m_end && m_endAlloc); \
								MojAssert(m_end >= m_begin && m_endAlloc >= m_end); \
								MojAssert(*m_end == _T('\0'))

static const gsize MojStringInitialSize = 8;
static const gsize MojStringFormatBufSize = 128;

const MojString MojString::Empty;
MojChar* MojString::s_emptyString = (MojChar*) _T("");

gsize MojString::find(const MojChar* str, gsize startIdx) const
{
	MojStringAssertValid();
	MojAssert(startIdx == 0 || startIdx < length());

	const MojChar* match = MojStrStr(m_begin + startIdx, str);
	if (match)
		return match - m_begin;
	return MojInvalidIndex;
}

gsize MojString::find(MojChar c, gsize startIdx) const
{
	MojStringAssertValid();
	MojAssert(startIdx == 0 || startIdx < length());

	const MojChar* match = MojMemChr(m_begin + startIdx, c, length() - startIdx);
	if (match)
		return match - m_begin;
	return MojInvalidIndex;
}

gsize MojString::rfind(MojChar c, gsize startIdx) const
{
	MojStringAssertValid();
	MojAssert(startIdx == MojInvalidIndex || startIdx < length());

	if (startIdx == MojInvalidIndex)
		startIdx = length() - 1;
	const MojChar* match = MojMemChrReverse(m_begin, c, startIdx  + 1);
	if (match)
		return match - m_begin;
	return MojInvalidIndex;
}

MojErr MojString::split(MojChar c, MojVector<MojString>& vecOut) const
{
	MojStringAssertValid();

	vecOut.clear();
	gsize idx = 0;
	gsize idxStart = 0;
	gsize len = length();
	MojVector<MojString> vec;
	do {
		idx = find(c, idx);
		if (idx == MojInvalidIndex)
			idx = len;
		gsize subLen = idx - idxStart;
		if (subLen > 0) {
			MojString str;
			MojErr err = substring(idxStart, subLen, str);
			MojErrCheck(err);
			err = vec.push(str);
			MojErrCheck(err);
		}
		idxStart = ++idx;
	} while (idx < len);

	vec.swap(vecOut);
	return MojErrNone;
}

MojErr MojString::substring(gsize idx, gsize len, MojString& strOut) const
{
	MojStringAssertValid();
	MojAssert(idx <= length() && len <= (length() - idx));

	MojErr err = strOut.assign(m_begin + idx, len);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojString::begin(Iterator& iterOut)
{
	MojErr err = ensureWritable();
	MojErrCheck(err);
	iterOut = m_begin;

	return MojErrNone;
}

MojErr MojString::end(Iterator& iterOut)
{
	MojErr err = ensureWritable();
	MojErrCheck(err);
	iterOut = m_end;

	return MojErrNone;
}

bool MojString::startsWith(const MojChar* str) const
{
	MojAssert(str);

	ConstIterator i = begin();
	while (*str) {
		if (*i != *str)
			return false;
		++str;
		++i;
	}
	return true;
}

MojErr MojString::reserve(gsize len)
{
	MojStringAssertValid();

	if (len > capacity()) {
		MojErr err = realloc(len);
		MojErrCheck(err);
	}

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::truncate(gsize len)
{
	MojStringAssertValid();

	if (len < length()) {
		MojErr err = ensureWritable();
		MojErrCheck(err);
		setEnd(len);
	}

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::resize(gsize len)
{
	MojStringAssertValid();

	if (len != length()) {
		MojErr err = ensureWritable();
		MojErrCheck(err);
		err = reserve(len);
		MojErrCheck(err);
		setEnd(len);
	}

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::toUpper()
{
	MojStringAssertValid();
	MojErr err = ensureWritable();
	MojErrCheck(err);

	for (MojChar* i = m_begin; i != m_end; ++i) {
		*i = MojToUpper(*i);
	}

	return MojErrNone;
}

MojErr MojString::toLower()
{
	MojStringAssertValid();
	MojErr err = ensureWritable();
	MojErrCheck(err);

	for (MojChar* i = m_begin; i != m_end; ++i) {
		*i = MojToLower(*i);
	}

	return MojErrNone;
}

void MojString::assign(const MojString& str)
{
	MojStringAssertValid();
	if (&str != this) {
		release();
		init(str);
	}
	MojStringAssertValid();
}

MojErr MojString::assign(const MojChar* str)
{
	MojAssert(str);
	return assign(str, MojStrLen(str));
}

MojErr MojString::assign(const MojChar* chars, gsize len)
{
	MojAssert(chars || len == 0);
	MojStringAssertValid();

	if (len == 0) {
		clear();
	} else if (len <= capacity() && isWritable()) {
		MojMemMove(m_begin, chars, sizeof(MojChar) * len);
		setEnd(len);
	} else {
		MojChar* newData = alloc(len);
		MojAllocCheck(newData);
		MojMemCpy(newData, chars, sizeof(MojChar) * len);
		newData[len] = _T('\0');
		reset(newData, len);
	}

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::setAt(gsize idx, MojChar c)
{
	MojAssert(idx < length());
	MojStringAssertValid();

	MojErr err = ensureWritable();
	MojErrCheck(err);
	m_begin[idx] = c;

	MojStringAssertValid();
	return MojErrNone;
}

void MojString::clear()
{
	MojStringAssertValid();
	if (empty())
		return;
	if (isWritable()) {
		setEnd(0);
	} else {
		reset(s_emptyString, 0);
	}
	MojStringAssertValid();
}

MojErr MojString::format(const MojChar* formatStr, ...)
{
	MojStringAssertValid();
	MojAssert(formatStr);

	va_list args;
	va_start (args, formatStr);
	MojErr err = vformat(formatStr, args);
	va_end(args);
	MojErrCheck(err);

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::vformat(const MojChar* formatStr, va_list args)
{
	MojStringAssertValid();
	MojAssert(formatStr);

	clear();
	MojErr err = appendVFormat(formatStr, args);
	MojErrCheck(err);

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::append(MojChar c)
{
	MojStringAssertValid();

	MojErr err = ensureSpace(1);
	MojErrCheck(err);
	*(m_end++) = c;
	*m_end = _T('\0');

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::append(const MojChar* str)
{
	return append(str, MojStrLen(str));
}

MojErr MojString::append(const MojChar* chars, gsize len)
{
	MojAssert(chars || len == 0);
	MojStringAssertValid();

	if (len > 0) {
		MojErr err = ensureSpace(len);
		MojErrCheck(err);
		MojMemMove(m_end, chars, len);
		m_end += len;
		*m_end = _T('\0');
	}

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::appendFormat(const MojChar* formatStr, ...)
{
	MojStringAssertValid();
	MojAssert(formatStr);

	va_list args;
	va_start (args, formatStr);
	MojErr err = appendVFormat(formatStr, args);
	va_end(args);
	MojErrCheck(err);

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::appendVFormat(const MojChar* formatStr, va_list args)
{
	MojStringAssertValid();
	MojAssert(formatStr);

	// copy va_list since we have to use it twice
	va_list args2;
	va_copy(args2, args);
	// attempt to format into a small buffer
	gsize formattedLen = 0;
	MojChar buf[MojStringFormatBufSize];
	MojErr err = MojVsnPrintF(buf, MojStringFormatBufSize, formattedLen, formatStr, args);
	MojErrGoto(err, Done);
	if (formattedLen < MojStringFormatBufSize) {
		// string fit in buf
		err = append(buf, formattedLen);
		MojErrGoto(err, Done);
	} else {
		gsize newLen = 0;
		err = ensureSpace(formattedLen);
		MojErrGoto(err, Done);
		// format into new string
		err = MojVsnPrintF(m_end, formattedLen + 1, newLen, formatStr, args2);
		MojErrGoto(err, Done);
		MojAssert(formattedLen == newLen);
		m_end += formattedLen;
		*m_end = _T('\0');
	}
Done:
	va_end(args2);
	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::base64Encode(const MojVector<guint8>& vec, bool pad)
{
	MojString str;
	MojErr err = str.resize(MojBase64EncodedLenMax(vec.size()));
	MojErrCheck(err);
	gsize size;
	err = MojBase64Encode(vec.begin(), vec.size(), str.m_begin, str.length(), size, pad);
	MojErrCheck(err);
	err = str.truncate(size);
	MojErrCheck(err);
	swap(str);

	return MojErrNone;
}

MojErr MojString::base64Decode(MojVector<guint8>& vecOut) const
{
	MojVector<guint8> vec;
	MojErr err = vec.resize(MojBase64DecodedSizeMax(length()));
	MojErrCheck(err);
	MojVector<guint8>::Iterator i;
	err = vec.begin(i);
	MojErrCheck(err);
	gsize size;
	err = MojBase64Decode(m_begin, length(), i, vec.size(), size);
	MojErrCheck(err);
	err = vec.resize(size);
	MojErrCheck(err);
	vecOut.swap(vec);

	return MojErrNone;
}

void MojString::init(const MojString& str)
{
	if (str.empty()) {
		init(s_emptyString, s_emptyString, s_emptyString);
	} else {
		init(str.m_begin, str.m_end, str.m_endAlloc);
		MojRefCountRetain(m_begin);
	}
}

void MojString::init(MojChar* begin, MojChar* end, MojChar* endAlloc)
{
	m_begin = begin;
	m_end = end;
	m_endAlloc = endAlloc;
	MojStringAssertValid();
}

void MojString::release()
{
	MojStringAssertValid();
	if (m_begin != s_emptyString)
		MojRefCountRelease(m_begin);
}

void MojString::reset(MojChar* begin, MojChar* end, MojChar* endAlloc)
{
	release();
	init(begin, end, endAlloc);
}

MojChar* MojString::alloc(gsize len)
{
	return (MojChar*) MojRefCountAlloc(sizeof(MojChar) * (len + 1));
}

MojErr MojString::realloc(gsize allocLen)
{
	gsize len = length();
	MojChar* newData = alloc(allocLen);
	MojAllocCheck(newData);
	MojMemCpy(newData, m_begin, len + 1);
	reset(newData, newData + len, newData + allocLen);

	MojStringAssertValid();
	return MojErrNone;
}

MojErr MojString::ensureWritable()
{
	if (!isWritable()) {
		MojErr err = realloc(length());
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojString::ensureSpace(gsize len)
{
	MojAssert(len);

	if (len > freeSpace() || !isWritable()) {
		gsize allocLen = MojMax(capacity() * 2, length() + len);
		allocLen = MojMax(allocLen, MojStringInitialSize);
		MojErr err = realloc(allocLen);
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojString::setEnd(gsize len)
{
	MojAssert(len <= capacity());
	m_end = m_begin + len;
	*m_end = _T('\0');
}
