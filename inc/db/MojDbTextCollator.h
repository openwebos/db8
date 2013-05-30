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


#ifndef MOJDBTEXTCOLLATOR_H_
#define MOJDBTEXTCOLLATOR_H_

#include "db/MojDbDefs.h"
#include "db/MojDbTextUtils.h"
#include "core/MojHashMap.h"
#include "core/MojRefCount.h"
#include "core/MojThread.h"

struct UCollator;

class MojDbTextCollator : public MojRefCounted
{
public:
	MojDbTextCollator();
	virtual ~MojDbTextCollator();

	MojErr init(const MojChar* locale, MojDbCollationStrength strength);
	MojErr sortKey(const MojString& str, MojDbKey& keyOut) const;
	MojErr sortKey(const UChar* chars, gsize size, MojDbKey& keyOut) const;

private:
	UCollator* m_ucol;
};

#endif /* MOJDBTEXTCOLLATOR_H_ */
