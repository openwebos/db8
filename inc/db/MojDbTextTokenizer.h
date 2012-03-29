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


#ifndef MOJDBTEXTTOKENIZER_H_
#define MOJDBTEXTTOKENIZER_H_

#include "db/MojDbDefs.h"
#include "core/MojRefCount.h"
#include "core/MojSet.h"
#include "unicode/ubrk.h"

class MojDbTextTokenizer : public MojRefCounted
{
public:
	typedef MojSet<MojDbKey> KeySet;

	MojDbTextTokenizer();
	~MojDbTextTokenizer();

	MojErr init(const MojChar* locale);
	MojErr tokenize(const MojString& text, MojDbTextCollator* collator, KeySet& keysOut) const;

private:
	struct IterCloser
	{
		void operator()(UBreakIterator* ubrk)
		{
			ubrk_close(ubrk);
		}
	};
	typedef MojAutoPtrBase<UBreakIterator, IterCloser> IterPtr;

	IterPtr m_ubrk;
};

#endif /* MOJDBTEXTTOKENIZER_H_ */
