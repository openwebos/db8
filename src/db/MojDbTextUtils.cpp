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


#include "db/MojDbTextUtils.h"
#include "core/MojString.h"
#include "unicode/ustring.h"

MojErr MojDbTextUtils::strToUnicode(const MojString& src, UnicodeVec& destOut)
{
	MojErr err = destOut.resize(src.length() * 2);
	MojErrCheck(err);
	MojInt32 destCapacity = 0;
	MojInt32 destLength = 0;
	do {
		UChar* dest = NULL;
		err = destOut.begin(dest);
		MojErrCheck(err);
		destCapacity = (MojInt32) destOut.size();
		UErrorCode status = U_ZERO_ERROR;
		u_strFromUTF8(dest, destCapacity, &destLength, src.data(), (MojInt32) src.length(), &status);
		if (status != U_BUFFER_OVERFLOW_ERROR)
			MojUnicodeErrCheck(status);
		err = destOut.resize(destLength);
		MojErrCheck(err);
	} while (destLength > destCapacity);

	return MojErrNone;
}

MojErr MojDbTextUtils::unicodeToStr(const UChar* src, MojSize len, MojString& destOut)
{
	MojAssert(src || len == 0);

	MojErr err = destOut.resize(len * 2);
	MojErrCheck(err);
	MojInt32 destCapacity = 0;
	MojInt32 destLength = 0;
	do {
		MojChar* dest = NULL;
		err = destOut.begin(dest);
		MojErrCheck(err);
		destCapacity = (MojInt32) destOut.length();
		UErrorCode status = U_ZERO_ERROR;
		u_strToUTF8(dest, destCapacity, &destLength, src, (MojInt32) len, &status);
		if (status != U_BUFFER_OVERFLOW_ERROR)
			MojUnicodeErrCheck(status);
		err = destOut.resize(destLength);
		MojErrCheck(err);
	} while (destLength > destCapacity);

	return MojErrNone;
}

