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


#ifndef MOJMAP_H_
#define MOJMAP_H_

#include "core/MojCoreDefs.h"
#include "core/MojComp.h"
#include "core/internal/MojRbTreeBase.h"

template<class KEY, class VAL, class LKEY = KEY,
		 class KCOMP = MojComp<LKEY>, class VCOMP = MojComp<VAL> >
class MojMap : private MojRbTreeBase
{
	friend class Iterator;
	friend class ConstIterator;
	class MapNode;
public:
	class Iterator;
	typedef KEY KeyType;
	typedef VAL ValueType;
	typedef LKEY LookupType;

	class ConstIterator
	{
		friend class Iterator;
		friend class MojMap;
	public:
		ConstIterator() : m_node(NULL) {}

		const KeyType& key() const { MojAssert(m_node); return node()->m_key; }
		const ValueType& value() const { MojAssert(m_node); return node()->m_val; }

		void operator++() { MojAssert(m_node); m_node = m_node->successor(); }
		const ConstIterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return m_node == rhs.m_node; }
		bool operator==(const Iterator& rhs) const;
		bool operator!=(const ConstIterator& rhs) const { return m_node != rhs.m_node; }
		bool operator!=(const Iterator& rhs) const;
		const ValueType& operator*() const { return value(); }
		const ValueType* operator->() const { return &value(); }

	private:
		ConstIterator(const Node* node) : m_node(node) {}
		const MapNode* node() const { return static_cast<const MapNode*>(m_node); }
		const Node* m_node;
	};

	class Iterator
	{
		friend class ConstIterator;
		friend class MojMap;
	public:
		Iterator() : m_node(NULL) {}

		const KeyType& key() const { MojAssert(m_node); return node()->m_key; }
		ValueType& value() const { MojAssert(m_node); return node()->m_val; }

		void operator++() { MojAssert(m_node); m_node = m_node->successor(); }
		const Iterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return m_node == rhs.m_node; }
		bool operator==(const Iterator& rhs) const { return m_node == rhs.m_node; }
		bool operator!=(const ConstIterator& rhs) const { return m_node != rhs.m_node; }
		bool operator!=(const Iterator& rhs) const { return m_node != rhs.m_node; }
		ValueType& operator*() const { return value(); }
		ValueType* operator->() const { return &value(); }

	private:
		Iterator(Node* node) : m_node(node) {}
		MapNode* node() const { return static_cast<MapNode*>(m_node); }
		Node* m_node;
	};

	MojMap() : MojRbTreeBase() {}
	MojMap(const MojMap& map) : MojRbTreeBase(map) {}
	~MojMap() { MojRbTreeBase::release(); }

	MojSize size() const { return MojRbTreeBase::size(); }
	bool empty() const { return MojRbTreeBase::empty(); }

	ConstIterator begin() const { return ConstIterator(MojRbTreeBase::begin()); }
	ConstIterator end() const { return ConstIterator(); }

	MojErr begin(Iterator& iter) { return MojRbTreeBase::begin(iter.m_node); }
	MojErr end(Iterator& iter) { return MojRbTreeBase::end(iter.m_node); }

	void clear() { MojRbTreeBase::clear(); }
	void swap(MojMap& map) { MojRbTreeBase::swap(map); }
	void assign(const MojMap& comp) { MojRbTreeBase::assign(comp); }
	int compare(const MojMap& comp) const { return MojRbTreeBase::compare(comp); }
	MojErr diff(const MojMap& comp, MojMap& diffOut) const { return MojRbTreeBase::diff(comp, diffOut); }

	bool contains(const LookupType& key) const { return find(key) != end(); }
	bool get(const LookupType& key, ValueType& valOut) const;
	MojErr del(const LookupType& key, bool& foundOut) { return MojRbTreeBase::del(&key, foundOut); }
	MojErr put(const KeyType& key, const ValueType& val);
	MojErr put(const MojMap& map) { return MojRbTreeBase::put(map); }

	ConstIterator find(const LookupType& key) const { return ConstIterator(MojRbTreeBase::find(&key)); }
	MojErr find(const LookupType& key, Iterator& iter) { return MojRbTreeBase::find(&key, iter.m_node); }
	ConstIterator lowerBound(const LookupType& key) const { return ConstIterator(MojRbTreeBase::lowerBound(&key)); }
	MojErr lowerBound(const LookupType& key, Iterator& iter) { return MojRbTreeBase::lowerBound(&key, iter.m_node); }
	ConstIterator upperBound(const LookupType& key) const { return ConstIterator(MojRbTreeBase::upperBound(&key)); }
	MojErr upperBound(const LookupType& key, Iterator& iter) { return MojRbTreeBase::upperBound(&key, iter.m_node); }

	MojMap& operator=(const MojMap& rhs) { assign(rhs); return *this; }
	bool operator==(const MojMap& rhs) const { return MojRbTreeBase::equals(rhs); }
	bool operator!=(const MojMap& rhs) const { return !operator==(rhs); }
	bool operator<(const MojMap& rhs) const { return compare(rhs) < 0; }
	bool operator<=(const MojMap& rhs) const { return compare(rhs) <= 0; }
	bool operator>(const MojMap& rhs) const { return compare(rhs) > 0; }
	bool operator>=(const MojMap& rhs) const { return compare(rhs) >= 0; }

private:
	struct MapNode : public Node
	{
		MapNode(const KeyType& key, const ValueType& val) : m_key(key), m_val(val) {}
		MapNode* predecessor() const { return static_cast<MapNode*>(Node::predecessor()); }
		MapNode* successor() const { return static_cast<MapNode*>(Node::successor()); }
		KeyType m_key;
		ValueType m_val;
	};

	virtual int compareFull(const Node* node1, const Node* node2) const;
	virtual int compareKey(const Node* node, const void* compData) const;
	virtual void deleteNode(Node* node) const;
	virtual Node* cloneNode(const Node* node) const;
	virtual const void* key(const Node* node) const;
};

template<>
struct MojComp<const MojMap<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> >
{
	int operator()(const MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>& val1, const MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>& val2)
	{
		return val1.compare(val2);
	}
};

template<>
struct MojComp<MojMap<KEY, VAL, LKEY, KCOMP, VCOMP> >
: public MojComp<const MojMap<KEY, VAL, LKEY, KCOMP, VCOMP> > {};

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> inline
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::ConstIterator::operator==(const Iterator& rhs) const
{
	return m_node == rhs.m_node;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> inline
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::ConstIterator::operator!=(const Iterator& rhs) const
{
	return m_node != rhs.m_node;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::get(const LookupType& key, ValueType& valOut) const
{
	ConstIterator iter = find(key);
	if (iter == end())
		return false;
	valOut = iter.value();
	return true;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
MojErr MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::put(const KeyType& key, const ValueType& val)
{
	MojAutoPtr<Node> node(new MapNode(key, val));
	MojAllocCheck(node.get());
	MojErr err = MojRbTreeBase::put(node, &key);
	MojErrCheck(err);

	return MojErrNone;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
int MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::compareFull(const Node* node1, const Node* node2) const
{
	MojAssert(node1 && node2);
	const MapNode* mapNode1 = static_cast<const MapNode*>(node1);
	const MapNode* mapNode2 = static_cast<const MapNode*>(node2);
	int comp = KCOMP()(mapNode1->m_key, mapNode2->m_key);
	if (comp == 0)
		comp = VCOMP()(mapNode1->m_val, mapNode2->m_val);
	return comp;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
int MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::compareKey(const Node* node, const void* key) const
{
	MojAssert(node && key);
	return KCOMP()(static_cast<const MapNode*>(node)->m_key, *static_cast<const LookupType*>(key));
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
void MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::deleteNode(Node* node) const
{
	MojAssert(node);
	delete static_cast<MapNode*>(node);
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
MojRbTreeBase::Node* MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::cloneNode(const Node* node) const
{
	MojAssert(node);
	const MapNode* mapNode = static_cast<const MapNode*>(node);
	return new MapNode(mapNode->m_key, mapNode->m_val);
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
const void* MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::key(const Node* node) const
{
	MojAssert(node);
	return &static_cast<const MapNode*>(node)->m_key;
}

#endif /* MOJMAP_H_ */
