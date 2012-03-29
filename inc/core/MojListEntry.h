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


#ifndef MOJLISTENTRY_H_
#define MOJLISTENTRY_H_

#include "core/MojNoCopy.h"

class MojListEntry : private MojNoCopy
{
public:
	MojListEntry() { reset(); }
	~MojListEntry();

	bool inList() const { return m_next != NULL; }
	void insert(MojListEntry* elem);
	void erase();
	void fixup();
	void swap(MojListEntry& entry);
	void reset() { m_prev = NULL; m_next = NULL; }

	MojListEntry* m_prev;
	MojListEntry* m_next;
};

#endif /* MOJLISTENTRY_H_ */
