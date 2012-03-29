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


#include "core/MojList.h"

MojListEntry::~MojListEntry()
{
	// if you are hitting this assert, you are destroying an object
	// before destroying the list that contains it
	MojAssert(!m_next && !m_prev);
}

void MojListEntry::insert(MojListEntry* entry)
{
	MojAssert(entry);
	MojAssert(entry->m_prev == NULL && entry->m_next == NULL);

	entry->m_next = this;
	entry->m_prev = m_prev;
	m_prev->m_next = entry;
	m_prev = entry;
}

void MojListEntry::erase()
{
   // just to double check we have valid pointers since we are getting crashes here from time to time
   // and it's extremely difficult to debug them.
   if(m_prev)
      m_prev->m_next = m_next;
   if(m_next)
      m_next->m_prev = m_prev;
   reset();
}

void MojListEntry::fixup()
{
	m_next->m_prev = this;
	m_prev->m_next= this;
}
