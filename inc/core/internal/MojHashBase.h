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


#ifndef MOJHASHBASE_H_
#define MOJHASHBASE_H_

#include "core/MojCoreDefs.h"
#include "core/MojAutoPtr.h"
#include "core/MojHasher.h"

class MojHashBase
{
	friend class ConstIterator;
	friend class Iterator;
	class Bucket;
protected:
	struct Node
	{
		Node();

		Node* m_bucketNext;
		Node* m_iterNext;
		Node** m_iterRef;
	};

	MojHashBase() : m_impl(NULL) {}
	MojHashBase(const MojHashBase& hb) { init(hb.m_impl); }
	virtual ~MojHashBase() {}
	void release();

	MojSize size() const { return m_impl ? m_impl->m_size : 0; }
	bool empty() const { return size() == 0; }
	const Node* begin() const { return m_impl ? m_impl->m_begin : NULL; }
	MojErr begin(Node*& node);
	MojErr end(Node*& node) { node = NULL; return ensureWritable(); }

	void clear();
	void swap(MojHashBase& hb);
	void assign(const MojHashBase& hb);
	bool equals(const MojHashBase& hb) const;

	const Node* find(const void* key, MojSize* idxOut = NULL) const;
	MojErr find(const void* key, Node*& nodeOut, MojSize* idxOut = NULL);
	MojErr put(MojAutoPtr<Node> node, MojSize idx, const void* key);
	MojErr del(const void* key, bool& foundOut);

private:
	static const MojSize FillFactor;
	static const MojSize InitialSize;

	struct Bucket
	{
		Bucket() : m_node(NULL) {}
		void insert(Node* node);
		void remove(Node* node);

		Node* m_node;
	};

	struct Impl
	{
		Impl();
		void put(Node* node, MojSize idx);

		MojAutoArrayPtr<Bucket> m_buckets;
		Node* m_begin;
		MojSize m_numBuckets;
		MojSize m_size;
		MojAtomicInt m_refcount;
	};

	virtual bool compareFull(const Node* node1, const Node* node2) const = 0;
	virtual bool compareKey(const Node* node, const void* key) const = 0;
	virtual void deleteNode(Node* node) const = 0;
	virtual Node* cloneNode(const Node* node) const = 0;
	virtual const void* key(const Node* node) const = 0;
	virtual MojSize hash(const void* key) const = 0;

	void init(Impl* impl);
	MojErr ensureWritable();
	MojErr clone();
	MojErr realloc(MojSize numBuckets);
	MojSize bucketForKey(const void* key, MojSize numBuckets) const;
	void deleteNodeList(Node* node);

	Impl* m_impl;
};


#endif /* MOJHASHBASE_H_ */
