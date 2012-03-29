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


#ifndef MOJRBTREEBASE_H_
#define MOJRBTREEBASE_H_

#include "core/MojCoreDefs.h"
#include "core/MojAutoPtr.h"

class MojRbTreeBase
{
protected:
	typedef enum {
		ColorBlack = 0,
		ColorRed
	} Color;

	typedef enum {
		DirLeft = 0,
		DirRight
	} Dir;

	class Node
	{
		friend class MojRbTreeBase;
	public:
		Node();
		Node* child(int dir) const { return m_children[dir]; }
		void setChild(int dir, Node* node);
		Node* next(int dir) const;
		Node* predecessor() const { return next(DirLeft); }
		Node* successor() const { return next(DirRight); }
		Node* rotate(int dir);
		Node* rotateDouble(int dir);
		void replace(Node* node);

	private:
		Color m_color;
		Node* m_parent;
		Node* m_children[2];
	};

	MojRbTreeBase() : m_impl(NULL) {}
	MojRbTreeBase(const MojRbTreeBase& tree) { init(tree.m_impl); }
	virtual ~MojRbTreeBase() {}
	void release();

	MojSize size() const { return m_impl ? m_impl->m_size : 0; }
	bool empty() const { return size() == 0; }
	Node* begin() const;
	MojErr begin(Node*& node);
	MojErr end(Node*& node) { node = NULL; return ensureWritable(); }

	void clear();
	void assign(const MojRbTreeBase& tree);
	void swap(MojRbTreeBase& tree);
	int compare(const MojRbTreeBase& comp) const;
	bool equals(const MojRbTreeBase& comp) const;
	MojErr diff(const MojRbTreeBase& tree, MojRbTreeBase& diffOut) const;

	const Node* find(const void* key) const;
	MojErr find(const void* key, Node*& nodeOut);
	const Node* lowerBound(const void* key) const;
	MojErr lowerBound(const void* key, Node*& nodeOut);
	const Node* upperBound(const void* key) const;
	MojErr upperBound(const void* key, Node*& nodeOut);

	MojErr put(MojAutoPtr<Node> node, const void* key);
	MojErr put(const MojRbTreeBase& tree);
	MojErr del(const void* key, bool& foundOut);
	MojErr del(const MojRbTreeBase& tree);

private:
	struct Impl
	{
		Impl(Node* root) : m_root(root), m_size(1), m_refcount(1) {}

		Node* m_root;
		MojSize m_size;
		MojAtomicInt m_refcount;
	};

	virtual int compareFull(const Node* node1, const Node* node2) const = 0;
	virtual int compareKey(const Node* node, const void* key) const = 0;
	virtual void deleteNode(Node* node) const = 0;
	virtual Node* cloneNode(const Node* node) const = 0;
	virtual const void* key(const Node* node) const = 0;

	static bool isRed(Node* n) { return n != NULL && n->m_color == ColorRed; }
	void search(const void* key, Node*& nodeOut, int& compOut) const;
	MojErr ensureWritable();
	MojErr clone();
	MojErr clone(Node*& dest, const Node* src);
	MojErr putClone(const Node* node);
	void init(Impl* impl);
	void release(Node* node);
	void validate() const;
	void validate(Node* n, MojSize& blackHeight, MojSize& size) const;

	Impl* m_impl;
};

#endif /* MOJRBTREEBASE_H_ */
