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


#include "db/MojDbTextCollator.h"
#include "db/MojDbKey.h"
#include "core/MojObjectSerialization.h"
#include "core/MojString.h"
#include "unicode/ucol.h"

MojDbTextCollator::MojDbTextCollator()
: m_ucol(NULL)
{
}

MojDbTextCollator::~MojDbTextCollator()
{
	if (m_ucol)
		ucol_close(m_ucol);
}

MojErr MojDbTextCollator::init(const MojChar* locale, MojDbCollationStrength level)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(locale);
	MojAssert(!m_ucol);

	UCollationStrength strength = UCOL_PRIMARY;
	switch (level) {
	case MojDbCollationPrimary:
		strength = UCOL_PRIMARY;
		break;
	case MojDbCollationSecondary:
		strength = UCOL_SECONDARY;
		break;
	case MojDbCollationTertiary:
		strength = UCOL_TERTIARY;
		break;
    case MojDbCollationQuaternary:
        strength = UCOL_QUATERNARY;
        break;
	case MojDbCollationIdentical:
		strength = UCOL_IDENTICAL;
		break;
	default:
		MojAssertNotReached();
	}

	UErrorCode status = U_ZERO_ERROR;
	m_ucol = ucol_open(locale, &status);
	MojUnicodeErrCheck(status);
	MojAssert(m_ucol);
	ucol_setAttribute(m_ucol, UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
    if (level == MojDbCollationIdentical) {
        // Combination of IDENTICAL and NUMERIC option cover full-width comparison and ["001","01","1"] ordering.
        // NUMERIC option converts number charcter to numeric "a021" -> ["a",21]
        ucol_setAttribute(m_ucol, UCOL_NUMERIC_COLLATION, UCOL_ON, &status);
    }
	MojUnicodeErrCheck(status);
	ucol_setStrength(m_ucol, strength);

	return MojErrNone;
}

MojErr MojDbTextCollator::sortKey(const MojString& str, MojDbKey& keyOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// convert to UChar from utf8
	MojDbTextUtils::UnicodeVec chars;
	MojErr err = MojDbTextUtils::strToUnicode(str, chars);
	MojErrCheck(err);
	err = sortKey(chars.begin(), chars.size(), keyOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbTextCollator::sortKey(const UChar* chars, MojSize size, MojDbKey& keyOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	MojObjectWriter writer;

	if (size == 0) {
		err = writer.stringValue(_T(""), 0);
		MojErrCheck(err);
	} else {
		// get sort key
		MojInt32 destCapacity = 0;
		MojInt32 destLength = 0;
		MojDbKey::ByteVec vec;
		err = vec.resize(size * 3);
		MojErrCheck(err);
		do {
			MojByte* dest = NULL;
			err = vec.begin(dest);
			MojErrCheck(err);
			destCapacity = (MojInt32) vec.size();
			destLength = ucol_getSortKey(m_ucol, chars, (MojInt32) size, dest, destCapacity);
			if (destLength == 0) {
				MojErrThrow(MojErrDbUnicode);
			}
			err = vec.resize(destLength);
			MojErrCheck(err);
		} while (destLength > destCapacity);
		// write it
		MojAssert(vec.size() >= 1 && vec.back() == _T('\0'));
		err = writer.stringValue((const MojChar*) vec.begin(), vec.size() - 1);
		MojErrCheck(err);
	}
	err = keyOut.assign(writer.buf());
	MojErrCheck(err);

	return MojErrNone;
}
