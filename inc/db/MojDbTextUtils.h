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


#ifndef MOJDBTEXTUTILS_H_
#define MOJDBTEXTUTILS_H_

#include "db/MojDbDefs.h"
#include "core/MojVector.h"
#include "unicode/utypes.h"

class MojDbTextUtils
{
public:
	typedef MojVector<UChar> UnicodeVec;

	static MojErr strToUnicode(const MojString& src, UnicodeVec& destOut);
	static MojErr unicodeToStr(const UChar* src, gsize len, MojString& destOut);
};

#define MojUnicodeErrCheck(STATUS) if (U_FAILURE(STATUS)) MojErrThrowMsg(MojErrDbUnicode, _T("icu: %s"), u_errorName(STATUS))

#endif /* MOJDBTEXTUTILS_H_ */
