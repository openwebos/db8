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


#include "db/MojDbTextTokenizer.h"
#include "db/MojDbTextCollator.h"
#include "db/MojDbTextUtils.h"
#include "db/MojDbKey.h"
#include "core/MojObject.h"
#include "core/MojString.h"

MojDbTextTokenizer::MojDbTextTokenizer()
{
}

MojDbTextTokenizer::~MojDbTextTokenizer()
{
}

MojErr MojDbTextTokenizer::init(const MojChar* locale)
{
	MojAssert(locale);

	UErrorCode status = U_ZERO_ERROR;
	m_ubrk.reset(ubrk_open(UBRK_WORD, locale, NULL, 0, &status));
	MojUnicodeErrCheck(status);
	MojAssert(m_ubrk.get());

	return MojErrNone;
}

MojErr MojDbTextTokenizer::tokenize(const MojString& text, MojDbTextCollator* collator, KeySet& keysOut) const
{
	MojAssert(m_ubrk.get());

	// convert to UChar from str
	MojDbTextUtils::UnicodeVec unicodeStr;
	MojErr err = MojDbTextUtils::strToUnicode(text, unicodeStr);
	MojErrCheck(err);

	// clone break iterator and set text
	MojByte buf[U_BRK_SAFECLONE_BUFFERSIZE];
	UErrorCode status = U_ZERO_ERROR;
	MojInt32 size = sizeof(buf);
	IterPtr ubrk(ubrk_safeClone(m_ubrk.get(), buf, &size, &status));
	MojUnicodeErrCheck(status);
	MojAssert(ubrk.get());
	ubrk_setText(ubrk.get(), unicodeStr.begin(), (MojInt32) unicodeStr.size(), &status);
	MojUnicodeErrCheck(status);

	MojInt32 tokBegin = -1;
	MojInt32 pos = ubrk_first(ubrk.get());
	while (pos != UBRK_DONE) {
		UWordBreak status = (UWordBreak) ubrk_getRuleStatus(ubrk.get());
		if (status != UBRK_WORD_NONE) {
			MojAssert(tokBegin != -1);
			MojDbKey key;
			const UChar* tokChars = unicodeStr.begin() + tokBegin;
			MojSize tokSize = (MojSize) (pos - tokBegin);
			if (collator) {
				err = collator->sortKey(tokChars, tokSize, key);
				MojErrCheck(err);
			} else {
				MojString tok;
				err = MojDbTextUtils::unicodeToStr(tokChars, tokSize, tok);
				MojErrCheck(err);
				err = key.assign(tok);
				MojErrCheck(err);
			}
			err = keysOut.put(key);
			MojErrCheck(err);
		}
		tokBegin = pos;
		pos = ubrk_next(ubrk.get());
	}
	return MojErrNone;
}
