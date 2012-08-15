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


#ifndef MOJHASHMAP_H_
#define MOJHASHMAP_H_

#include "core/MojCoreDefs.h"
#include "core/MojComp.h"
#include "core/internal/MojHashBase.h"

template <class KEY, class VAL, class LKEY = KEY,
		  class HASH = MojHasher<LKEY>, class KEQ = MojEq<LKEY>, class VEQ = MojEq<VAL> >
class MojHashMap : private MojHashBase
{
	friend class ConstIterator;
	friend class Iterator;
	class MapNode;
public:
	typedef KEY KeyType;
	typedef VAL ValueType;
	typedef LKEY LookupType;

	class ConstIterator
	{
		friend class MojHashMap;
		friend class Iterator;
	public:
		ConstIterator() : m_node(NULL) {}

		const ConstIterator& operator++() { MojAssert(m_node); m_node = m_node->m_iterNext; return *this; }
		const ConstIterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return m_node == rhs.m_node; }
		bool operator==(const class Iterator& rhs) const { return operator==(static_cast<const ConstIterator&>(rhs)); }
		bool operator!=(const ConstIterator& rhs) const { return !operator==(rhs); }
		bool operator!=(const class Iterator& rhs) const { return !operator==(rhs); }
		const ValueType& operator*() const { return value(); }
		const ValueType* operator->() const { return &mapNode()->m_val; }

		const KeyType& key() const { MojAssert(m_node); return mapNode()->m_key; }
		const ValueType& value() const { MojAssert(m_node); return mapNode()->m_val; }

	private:
		ConstIterator(const Node* node) : m_node(const_cast<Node*>(node)) {}
		MapNode* mapNode() const { return static_cast<MapNode*>(m_node); }

		Node* m_node;
	};

	class Iterator : private ConstIterator
	{
		friend class MojHashMap;
	public:
		Iterator() : ConstIterator() {}

		const Iterator& operator++() { ConstIterator::operator++(); return *this; }
		const Iterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return ConstIterator::operator==(rhs); }
		bool operator==(const Iterator& rhs) const { return ConstIterator::operator==(rhs); }
		bool operator!=(const ConstIterator& rhs) const { return ConstIterator::operator!=(rhs); }
		bool operator!=(const Iterator& rhs) const { return ConstIterator::operator!=(rhs); }
		ValueType& operator*() const { return value; }
		ValueType* operator->() const { return const_cast<ValueType*>(ConstIterator::operator->()); }

		const KeyType& key() const { return ConstIterator::key(); }
		ValueType& value() const { return const_cast<ValueType&>(ConstIterator::value()); }

	private:
		Iterator(Node* node) : ConstIterator(node) {}
		Iterator(const ConstIterator& iter) : ConstIterator(iter) {}
	};

	MojHashMap() {}
	MojHashMap(const MojHashMap& map) : MojHashBase(map) {}
	~MojHashMap() { release(); }

	MojSize size() const { return MojHashBase::size(); }
	bool empty() const { return MojHashBase::empty(); }

	ConstIterator begin() const { return ConstIterator(MojHashBase::begin()); }
	const ConstIterator end() const { return ConstIterator(); }

	MojErr begin(Iterator& iter) { return MojHashBase::begin(iter.m_node); }
	MojErr end(Iterator& iter) { return MojHashBase::end(iter.m_node); }

	void clear() { MojHashBase::clear(); }
	void swap(MojHashMap& map) { MojHashBase::swap(map); }
	void assign(const MojHashMap& map) { MojHashBase::assign(map); }

	bool contains(const LookupType& key) const { return find(key) != end(); }
	bool get(const LookupType& key, ValueType& valOut) const;
	MojErr del(const LookupType& key, bool& foundOut) { return MojHashBase::del(&key, foundOut); }
	MojErr put(const KeyType& key, const ValueType& val);

	ConstIterator find(const LookupType& key) const { return ConstIterator(MojHashBase::find(&key)); }
	MojErr find(const LookupType& key, Iterator& iter) { return MojHashBase::find(&key, iter.m_node); }

	MojHashMap& operator=(const MojHashMap& rhs) { assign(rhs); return *this; }
	bool operator==(const MojHashMap& rhs) const { return MojHashBase::equals(rhs); }
	bool operator!=(const MojHashMap& rhs) const { return !operator==(rhs); }

private:
	struct MapNode : public Node
	{
		MapNode(const KeyType& key, const ValueType& val) : m_key(key), m_val(val) {}
		MapNode* next() const { return static_cast<MapNode*>(m_iterNext); }

		KeyType m_key;
		ValueType m_val;
	};

	virtual bool compareFull(const Node* node1, const Node* node2) const;
	virtual bool compareKey(const Node* node, const void* key) const;
	virtual void deleteNode(Node* node) const;
	virtual Node* cloneNode(const Node* node) const;
	virtual const void* key(const Node* node) const;
	virtual MojSize hash(const void* key) const;
};

#include "core/internal/MojHashMapInternal.h"

#endif /* MOJHASHMAP_H_ */
