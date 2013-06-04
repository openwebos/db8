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


#ifndef MOJSET_H_
#define MOJSET_H_

#include "core/MojCoreDefs.h"
#include "core/MojComp.h"
#include "core/internal/MojRbTreeBase.h"

template<class T, class COMP = MojComp<T> >
class MojSet : private MojRbTreeBase
{
	friend class Iterator;
	friend class ConstIterator;
	class SetNode;
public:
	class Iterator;
	typedef T ValueType;

	class ConstIterator
	{
		friend class Iterator;
		friend class MojSet;
	public:
		ConstIterator() : m_node(NULL) {}

		void operator++() { MojAssert(m_node); m_node = m_node->successor(); }
		const ConstIterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return m_node == rhs.m_node; }
		bool operator==(const Iterator& rhs) const;
		bool operator!=(const ConstIterator& rhs) const { return m_node != rhs.m_node; }
		bool operator!=(const Iterator& rhs) const;
		const ValueType& operator*() const { MojAssert(m_node); return node()->m_val; }
		const ValueType* operator->() const { MojAssert(m_node); return &node()->m_val; }
		void operator=(const Iterator& rhs);

	private:
		ConstIterator(const Node* node) : m_node(static_cast<const SetNode*>(node)) {}
		const SetNode* node() const { return static_cast<const SetNode*>(m_node); }
		const Node* m_node;
	};

	class Iterator
	{
		friend class ConstIterator;
		friend class MojSet;
	public:
		Iterator() : m_node(NULL) {}

		void operator++() { MojAssert(m_node); m_node = m_node->successor(); }
		const Iterator operator++(int) { return MojPostIncrement(*this); }
		bool operator==(const ConstIterator& rhs) const { return m_node == rhs.m_node; }
		bool operator==(const Iterator& rhs) const { return m_node == rhs.m_node; }
		bool operator!=(const ConstIterator& rhs) const { return m_node != rhs.m_node; }
		bool operator!=(const Iterator& rhs) const { return m_node != rhs.m_node; }
		const ValueType& operator*() const { MojAssert(m_node); return node()->m_val; }
		const ValueType* operator->() const { MojAssert(m_node); return &node()->m_val; }

	private:
		Iterator(Node* node) : m_node(static_cast<SetNode*>(node)) {}
		SetNode* node() const { return static_cast<SetNode*>(m_node); }
		Node* m_node;
	};

	MojSet() : MojRbTreeBase() {}
	MojSet(const MojSet& set) : MojRbTreeBase(set) {}
	~MojSet() { MojRbTreeBase::release(); }

	MojSize size() const { return MojRbTreeBase::size(); }
	bool empty() const { return MojRbTreeBase::empty(); }

	ConstIterator begin() const { return ConstIterator(MojRbTreeBase::begin()); }
	ConstIterator end() const { return ConstIterator(); }

	MojErr begin(Iterator& iter) { return MojRbTreeBase::begin(iter.m_node); }
	MojErr end(Iterator& iter) { return MojRbTreeBase::end(iter.m_node); }

	void clear() { MojRbTreeBase::clear(); }
	void swap(MojSet& set) { MojRbTreeBase::swap(set); }
	void assign(const MojSet& set) { MojRbTreeBase::assign(set); }
	int compare(const MojSet& set) const { return MojRbTreeBase::compare(set); }
	MojErr diff(const MojSet& comp, MojSet& diffOut) const { return MojRbTreeBase::diff(comp, diffOut); }

	bool contains(const ValueType& val) const { return find(val) != end(); }
	MojErr del(const ValueType& val, bool& foundOut) { return MojRbTreeBase::del(&val, foundOut); }
	MojErr del(const MojSet& set) { return MojRbTreeBase::del(set); }
	MojErr put(const ValueType& val);
	MojErr put(const MojSet& set) { return MojRbTreeBase::put(set); }
	MojErr intersect(const MojSet& set);

	ConstIterator find(const ValueType& val) const { return ConstIterator(MojRbTreeBase::find(&val)); }
	MojErr find(const ValueType& val, Iterator& iter) { return MojRbTreeBase::find(&val, iter.m_node); }

	MojSet& operator=(const MojSet& rhs) { assign(rhs); return *this; }
	bool operator==(const MojSet& rhs) const { return MojRbTreeBase::equals(rhs); }
	bool operator!=(const MojSet& rhs) const { return !operator==(rhs); }
	bool operator<(const MojSet& rhs) const { return compare(rhs) < 0; }
	bool operator<=(const MojSet& rhs) const { return compare(rhs) <= 0; }
	bool operator>(const MojSet& rhs) const { return compare(rhs) > 0; }
	bool operator>=(const MojSet& rhs) const { return compare(rhs) >= 0; }

private:
	struct SetNode : public Node
	{
		SetNode(const ValueType& val) : m_val(val) {}
		SetNode* predecessor() const { return static_cast<SetNode*>(Node::predecessor()); }
		SetNode* successor() const { return static_cast<SetNode*>(Node::successor()); }
		ValueType m_val;
	};

	virtual int compareFull(const Node* node1, const Node* node2) const;
	virtual int compareKey(const Node* node, const void* key) const;
	virtual void deleteNode(Node* node) const;
	virtual Node* cloneNode(const Node* node) const;
	virtual const void* key(const Node* node) const;
};

#include "core/internal/MojSetInternal.h"

#endif /* MOJSET_H_ */
