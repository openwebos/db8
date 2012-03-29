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


#ifndef MOJLISTINTERNAL_H_
#define MOJLISTINTERNAL_H_

template <class T, MojListEntry T::* ENTRY>
inline MojList<T, ENTRY>::ConstIterator::ConstIterator(const Iterator& iter)
: m_pos(iter.m_pos)
{
}

template <class T, MojListEntry T::* ENTRY>
inline bool MojList<T, ENTRY>::ConstIterator::operator==(const Iterator& rhs) const
{
	return m_pos == rhs.m_pos;
}

template <class T, MojListEntry T::* ENTRY>
inline bool MojList<T, ENTRY>::ConstIterator::operator!=(const Iterator& rhs) const
{
	return !operator==(rhs);
}

template <class T, MojListEntry T::* ENTRY>
inline MojList<T, ENTRY>::~MojList()
{
	clear();
#ifdef MOJ_DEBUG
	m_entry.reset();
#endif
}

template <class T, MojListEntry T::* ENTRY>
bool MojList<T, ENTRY>::contains(const T* val) const
{
	MojAssert(val);
	for (ConstIterator i = begin(); i != end(); ++i) {
		if (*i == val)
			return true;
	}
	return false;
}

template <class T, MojListEntry T::* ENTRY>
void MojList<T, ENTRY>::swap(MojList& rhs)
{
	MojList tmp = *this;
	*this = rhs;
	rhs = tmp;
}

template <class T, MojListEntry T::* ENTRY>
void MojList<T, ENTRY>::clear()
{
	Iterator i = begin();
	while (i != end()) {
		(i++).pos()->reset();
	}
	init();
}

template <class T, MojListEntry T::* ENTRY>
inline void MojList<T, ENTRY>::erase(T* val)
{
	MojAssert(contains(val));
	(val->*ENTRY).erase();
	--m_size;
}

template <class T, MojListEntry T::* ENTRY>
inline void MojList<T, ENTRY>::pushFront(T* val)
{
	MojAssert(val);
	m_entry.m_next->insert(&(val->*ENTRY));
	++m_size;
}

template <class T, MojListEntry T::* ENTRY>
inline void MojList<T, ENTRY>::pushBack(T* val)
{
	MojAssert(val);
	m_entry.insert(&(val->*ENTRY));
	++m_size;
}

template <class T, MojListEntry T::* ENTRY>
T* MojList<T, ENTRY>::popFront()
{
	MojAssert(!empty());
	T* val = entryToVal(m_entry.m_next);
	m_entry.m_next->erase();
	--m_size;
	return val;
}

template <class T, MojListEntry T::* ENTRY>
T* MojList<T, ENTRY>::popBack()
{
	MojAssert(!empty());
	T* val = entryToVal(m_entry.m_prev);
	m_entry.m_prev->erase();
	--m_size;
	return val;
}

template <class T, MojListEntry T::* ENTRY>
MojList<T, ENTRY>& MojList<T, ENTRY>::operator=(MojList& list)
{
	if (&list != this) {
		clear();
		init(list);
	}
	return *this;
}

template <class T, MojListEntry T::* ENTRY>
void MojList<T, ENTRY>::init()
{
	m_size = 0;
	m_entry.m_prev = &m_entry;
	m_entry.m_next = &m_entry;
}

template <class T, MojListEntry T::* ENTRY>
void MojList<T, ENTRY>::init(MojList& list)
{
	if (list.empty()) {
		init();
	} else {
		m_size = list.m_size;
		m_entry.m_prev = list.m_entry.m_prev;
		m_entry.m_next = list.m_entry.m_next;
		m_entry.fixup();
		list.init();
	}
}

#endif /* MOJLISTINTERNAL_H_ */
