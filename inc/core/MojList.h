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


#ifndef MOJLIST_H_
#define MOJLIST_H_

#include "core/MojCoreDefs.h"

template <class T, MojListEntry T::* ENTRY>
class MojList : private MojNoCopy
{
public:
	class Iterator;

	class ConstIterator
	{
	public:
		ConstIterator() : m_pos(NULL) {}
		ConstIterator(const Iterator& iter);

		void operator++() { MojAssert(m_pos && m_pos->m_next); m_pos = m_pos->m_next; }
		void operator--() { MojAssert(m_pos && m_pos->m_prev); m_pos = m_pos->m_prev; }
		const ConstIterator operator++(int) { return MojPostIncrement(*this); }
		const ConstIterator operator--(int) { return MojPreIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return rhs.m_pos == m_pos; }
		bool operator!=(const ConstIterator& rhs) const { return !operator==(rhs); }
		bool operator==(const Iterator& rhs) const;
		bool operator!=(const Iterator& rhs) const;
		ConstIterator& operator=(const Iterator& rhs);
		const T* operator*() const { return val(); }

	protected:
		friend class MojList;

		ConstIterator(const MojListEntry* entry) : m_pos(entry) {}
		const T* val() const { return entryToVal(m_pos); }

		const MojListEntry* m_pos;
	};

	class Iterator : private ConstIterator
	{
	public:
		Iterator() : m_list(NULL) {}
		void operator++() { ConstIterator::operator++(); }
		void operator--() { ConstIterator::operator--(); }
		Iterator operator++(int) { return MojPostIncrement(*this); }
		Iterator operator--(int) { return MojPreIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return rhs.m_pos == m_pos; }
		bool operator==(const Iterator& rhs) const { return rhs.m_pos == m_pos; }
		bool operator!=(const ConstIterator& rhs) const { return !operator==(rhs); }
		bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }
		T* operator*() const { return const_cast<T*>(ConstIterator::val()); }
		void erase() { MojAssert(m_pos); pos()->erase(); --(m_list->m_size); }

	private:
		friend class MojList;
		using ConstIterator::m_pos;

		Iterator(MojList* list, MojListEntry* entry) : ConstIterator(entry), m_list(list) {}
		MojListEntry* pos() const { return const_cast<MojListEntry*>(m_pos); }

		MojList* m_list;
	};

	MojList() { init(); }
	MojList(MojList& list) { init(list); }
	~MojList();

	MojSize size() const { return m_size; }
	bool empty() const { return m_size == 0; }
	bool contains(const T* val) const;

	ConstIterator begin() const { return ConstIterator(m_entry.m_next); }
	ConstIterator end() const { return ConstIterator(&m_entry); }
	Iterator begin() { return Iterator(this, m_entry.m_next); }
	Iterator end() { return Iterator(this, &m_entry); }

	const T* front() const { MojAssert(!empty()); return entryToVal(m_entry.m_next); }
	const T* back() const { MojAssert(!empty()); return entryToVal(m_entry.m_prev); }
	T* front() { MojAssert(!empty()); return entryToVal(m_entry.m_next); }
	T* back() { MojAssert(!empty()); return entryToVal(m_entry.m_prev); }

	void swap(MojList& list);
	void clear();
	void erase(T* val);
	void pushFront(T* val);
	void pushBack(T* val);
	T* popFront();
	T* popBack();

	MojList& operator=(MojList& rhs);

private:
	static T* entryToVal(const MojListEntry* entry) { return (T*)((MojByte*)entry - (MojSize)&(((T*)NULL)->*ENTRY)); }
	void init();
	void init(MojList& list);

	MojListEntry m_entry;
	MojSize m_size;
};

#include "core/internal/MojListInternal.h"

#endif /* MOJLIST_H_ */
