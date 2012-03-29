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


#include "core/internal/MojHashBase.h"


#define MojHashBaseAssertWritable() MojAssert(!m_impl || m_impl->m_refcount == 1)

const MojSize MojHashBase::FillFactor = 2;
const MojSize MojHashBase::InitialSize = 8;


MojHashBase::Node::Node()
: m_bucketNext(NULL),
  m_iterNext(NULL),
  m_iterRef(NULL)
{
}

void MojHashBase::Bucket::insert(Node* node)
{
	MojAssert(node && !node->m_bucketNext);
	// insert it at head of bucket list
	node->m_bucketNext = m_node;
	m_node = node;
}

void MojHashBase::Bucket::remove(Node* node)
{
	MojAssert(node && m_node);
	Node** nodePtr = &m_node;

	while (*nodePtr) {
		if (*nodePtr == node) {
			*nodePtr = node->m_bucketNext;
			node->m_bucketNext = NULL;
			break;
		}
		nodePtr = &(*nodePtr)->m_bucketNext;
		MojAssert(*nodePtr);
	}
}

MojHashBase::Impl::Impl()
: m_begin(NULL),
  m_numBuckets(0),
  m_size(0),
  m_refcount(1)
{
}

void MojHashBase::Impl::put(Node* node, MojSize idx)
{
	MojAssert(m_buckets.get() && idx < m_numBuckets);

	m_buckets[idx].insert(node);
	if (m_begin)
		m_begin->m_iterRef = &node->m_iterNext;
	node->m_iterNext = m_begin;
	node->m_iterRef = &m_begin;
	m_begin = node;
	++m_size;
}

MojErr MojHashBase::begin(Node*& node)
{
	MojErr err = ensureWritable();
	MojErrCheck(err);
	node = const_cast<Node*>(begin());

	return MojErrNone;
}

void MojHashBase::clear()
{
	release();
	m_impl = NULL;
}

void MojHashBase::swap(MojHashBase& hb)
{
	Impl* tmp = m_impl;
	m_impl = hb.m_impl;
	hb.m_impl = tmp;
}

void MojHashBase::assign(const MojHashBase& hb)
{
	if (&hb != this) {
		release();
		init(hb.m_impl);
	}
}

const MojHashBase::Node*
MojHashBase::find(const void* key, MojSize* idxOut) const
{
	if (empty())
		return NULL;

	MojSize idx = bucketForKey(key, m_impl->m_numBuckets);
	if (idxOut)
		*idxOut = idx;
	Bucket& bucket = m_impl->m_buckets[idx];
	for (Node* node = bucket.m_node;
		 node != NULL;
		 node = node->m_bucketNext) {
		if (compareKey(node, key))
			return node;
	}
	return NULL;
}

MojErr MojHashBase::find(const void* key, Node*& nodeOut, MojSize* idxOut)
{
	MojErr err = ensureWritable();
	MojErrCheck(err);
	nodeOut = const_cast<Node*>(find(key, idxOut));

	return MojErrNone;
}

MojErr MojHashBase::put(MojAutoPtr<Node> node, MojSize idx, const void* key)
{
	MojAssert(node.get());
	MojHashBaseAssertWritable();

	// grow if necessary
	MojSize numBuckets = empty() ? 0 : m_impl->m_numBuckets;
	if (size() >= (numBuckets * FillFactor)) {
		MojErr err = realloc(MojMax(InitialSize, numBuckets * 2));
		MojErrCheck(err);
		idx = bucketForKey(key, m_impl->m_numBuckets);
	}
	m_impl->put(node.release(), idx);

	return MojErrNone;
}

MojErr MojHashBase::del(const void* key, bool& foundOut)
{
	foundOut = false;
	Node* node = NULL;
	MojSize idx = 0;
	MojErr err = find(key, node, &idx);
	MojErrCheck(err);
	MojHashBaseAssertWritable();

	if (node) {
		// remove from bucket
		m_impl->m_buckets[idx].remove(node);
		// update iter list
		MojAssert(node->m_iterRef);
		*(node->m_iterRef) = node->m_iterNext;
		if (node->m_iterNext)
			node->m_iterNext->m_iterRef = node->m_iterRef;
		// and delete the node
		deleteNode(node);
		--(m_impl->m_size);
		foundOut = true;
	}

	return MojErrNone;
}

bool MojHashBase::equals(const MojHashBase& rhs) const
{
	if (size() != rhs.size())
		return false;
	if (m_impl == rhs.m_impl)
		return true;
	for (const Node* node = begin(); node != NULL; node = node->m_iterNext) {
		const Node* rhsNode = rhs.find(key(node));
		if (rhsNode == NULL)
			return false;
		if (!compareFull(node, rhsNode))
			return false;
	}
	return true;
}

MojErr MojHashBase::ensureWritable()
{
	if (m_impl && m_impl->m_refcount > 1) {
		MojErr err = clone();
		MojErrCheck(err);
	}
	MojHashBaseAssertWritable();

	return MojErrNone;
}

MojErr MojHashBase::clone()
{
	MojAssert(m_impl);

	// allocate new impl
	MojAutoPtr<Impl> impl(new Impl);
	MojAllocCheck(impl.get());
	impl->m_buckets.reset(new Bucket[m_impl->m_numBuckets]);
	MojAllocCheck(impl->m_buckets.get());
	impl->m_numBuckets = m_impl->m_numBuckets;

	// clone all the nodes
	for (const Node* node = begin();
		 node != NULL;
		 node = node->m_iterNext) {
		Node* newNode = cloneNode(node);
		// TODO: yuck, make this exception safe
		if (newNode == NULL)
			deleteNodeList(impl->m_begin);
		MojAllocCheck(newNode);
		MojSize idx = bucketForKey(key(node), impl->m_numBuckets);
		impl->put(newNode, idx);
	}

	release();
	m_impl = impl.release();

	return MojErrNone;
}

MojErr MojHashBase::realloc(MojSize numBuckets)
{
	MojHashBaseAssertWritable();

	// allocate new bucket array
	MojAutoArrayPtr<Bucket> newBuckets(new Bucket[numBuckets]);
	MojAllocCheck(newBuckets.get());
	if (m_impl == NULL) {
		m_impl = new Impl;
		MojAllocCheck(m_impl);
	}
	m_impl->m_buckets = newBuckets;
	m_impl->m_numBuckets = numBuckets;

	// move all the nodes to their new buckets
	for (Node* node = m_impl->m_begin;
		 node != NULL;
		 node = node->m_iterNext) {
		node->m_bucketNext = NULL;
		MojSize idx = bucketForKey(key(node), numBuckets);
		m_impl->m_buckets[idx].insert(node);
	}

	return MojErrNone;
}

MojSize MojHashBase::bucketForKey(const void* key, MojSize numBuckets) const
{
	MojSize hashCode = hash(key);
	return hashCode % numBuckets;
}

void MojHashBase::init(Impl* impl)
{
	m_impl = impl;
	if (m_impl)
		++(m_impl->m_refcount);
}

void MojHashBase::release()
{
	if (m_impl && --(m_impl->m_refcount) == 0) {
		deleteNodeList(m_impl->m_begin);
		delete m_impl;
	}
}

void MojHashBase::deleteNodeList(Node* node)
{
	while (node) {
		Node* next = node->m_iterNext;
		deleteNode(node);
		node = next;
	}
}


