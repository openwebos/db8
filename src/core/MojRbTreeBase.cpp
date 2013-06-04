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


#include "core/internal/MojRbTreeBase.h"

// TODO: turn this off once we get more comfortable with the implementation
#ifdef MOJ_DEBUG
#	define MojRbTreeAssertValid()	validate()
#else
#	define MojRbTreeAssertValid()
#endif

MojRbTreeBase::Node::Node()
: m_color(ColorRed),
  m_parent(NULL)
{
	m_children[DirLeft] = NULL;
	m_children[DirRight] = NULL;
}

void MojRbTreeBase::Node::setChild(int dir, Node* node)
{
	MojAssert(node != this);
	m_children[dir] = node;
	if (node)
		node->m_parent = this;
}

MojRbTreeBase::Node* MojRbTreeBase::Node::next(int dir) const
{
	Node* node = child(dir);
	if (node) {
		while (node->child(!dir) != NULL)
			node = node->child(!dir);
	} else {
		const Node* last = this;
		node = m_parent;
		while (node && last == node->child(dir)) {
			last = node;
			node = node->m_parent;
		}
	}
	return node;
}

MojRbTreeBase::Node* MojRbTreeBase::Node::rotate(int dir)
{
	Node *save = child(!dir);
	setChild(!dir, save->child(dir));
	save->setChild(dir, this);
	m_color = ColorRed;
	save->m_color = ColorBlack;
	return save;
}

MojRbTreeBase::Node* MojRbTreeBase::Node::rotateDouble(int dir)
{
	setChild(!dir, child(!dir)->rotate(!dir));
	return rotate(dir);
}

void MojRbTreeBase::Node::replace(Node* node)
{
	MojAssert(node);

	node->setChild(DirLeft, child(DirLeft) == node ? NULL : child(DirLeft));
	node->setChild(DirRight, child(DirRight) == node ? NULL : child(DirRight));
	node->m_color = m_color;
	if (m_parent) {
		if (m_parent->child(DirLeft) == this) {
			m_parent->setChild(DirLeft, node);
		} else {
			MojAssert(m_parent->child(DirRight) == this);
			m_parent->setChild(DirRight, node);
		}
	}
}

MojRbTreeBase::Node* MojRbTreeBase::begin() const
{
	if (m_impl == NULL)
		return NULL;
	Node* node = m_impl->m_root;
	while (node->child(DirLeft) != NULL)
		node = node->child(DirLeft);
	return node;
}

MojErr MojRbTreeBase::begin(Node*& node)
{
	node = NULL;
	MojErr err = ensureWritable();
	MojErrCheck(err);
	node = begin();

	return MojErrNone;
}

void MojRbTreeBase::clear()
{
	release();
	m_impl = NULL;
}

void MojRbTreeBase::assign(const MojRbTreeBase& tree)
{
	if (&tree != this) {
		release();
		init(tree.m_impl);
	}
}

void MojRbTreeBase::swap(MojRbTreeBase& tree)
{
	Impl* tmp = m_impl;
	m_impl = tree.m_impl;
	tree.m_impl = tmp;
}

int MojRbTreeBase::compare(const MojRbTreeBase& comp) const
{
	if (m_impl == comp.m_impl)
		return 0;
	Node* node = begin();
	Node* compNode = comp.begin();
	for(;;) {
		if (node == NULL)
			return compNode == NULL ? 0 : -1;
		if (compNode == NULL)
			return 1;
		int comp = compareFull(node, compNode);
		if (comp != 0)
			return comp;
		node = node->successor();
		compNode = compNode->successor();
	}
	MojAssertNotReached();
	return 0;
}

bool MojRbTreeBase::equals(const MojRbTreeBase& comp) const
{
	if (size() != comp.size())
		return false;
	return compare(comp) == 0;
}

const MojRbTreeBase::Node* MojRbTreeBase::find(const void* key) const
{
	MojAssert(key);

	Node* node;
	int comp;
	search(key, node, comp);
	if (comp == 0)
		return node;
	return NULL;
}

MojErr MojRbTreeBase::find(const void* key, Node*& nodeOut)
{
	nodeOut = NULL;
	MojErr err = ensureWritable();
	MojErrCheck(err);
	nodeOut = const_cast<Node*>(find(key));

	return MojErrNone;
}

const MojRbTreeBase::Node* MojRbTreeBase::lowerBound(const void* key) const
{
	MojAssert(key);

	Node* node;
	int comp;
	search(key, node, comp);
	if (comp >= 0)
		return node;
	return node->successor();
}

MojErr MojRbTreeBase::lowerBound(const void* key, Node*& nodeOut)
{
	nodeOut = NULL;
	MojErr err = ensureWritable();
	MojErrCheck(err);
	nodeOut = const_cast<Node*>(lowerBound(key));

	return MojErrNone;
}

const MojRbTreeBase::Node* MojRbTreeBase::upperBound(const void* key) const
{
	MojAssert(key);

	Node* node;
	int comp;
	search(key, node, comp);
	if (comp > 0 || node == NULL)
		return node;
	return node->successor();
}

MojErr MojRbTreeBase::upperBound(const void* key, Node*& nodeOut)
{
	nodeOut = NULL;
	MojErr err = ensureWritable();
	MojErrCheck(err);
	nodeOut = const_cast<Node*>(upperBound(key));

	return MojErrNone;
}

MojErr MojRbTreeBase::diff(const MojRbTreeBase& tree, MojRbTreeBase& diffOut) const
{
	diffOut.clear();
	const Node* node = begin();
	const Node* compNode = tree.begin();
	while (node != NULL) {
		int comp = compNode ? compareFull(node, compNode) : -1;
		if (comp < 0) {
			MojErr err = diffOut.putClone(node);
			MojErrCheck(err);
		}
		if (comp <= 0)
			node = node->successor();
		if (comp >= 0)
			compNode = compNode->successor();
	}
	return MojErrNone;
}

MojErr MojRbTreeBase::put(MojAutoPtr<Node> node, const void* key)
{
	MojAssert(node.get());
	MojRbTreeAssertValid();

	MojErr err = ensureWritable();
	MojErrCheck(err);

	if (empty()) {
		m_impl = new Impl(node.get());
		MojAllocCheck(m_impl);
		node.release();
	} else {
		Node head; // dummy root
		Node *g, *t; // grandparent & parent
		Node *p, *q; // iterator & parent
		int dir = DirLeft, last = DirLeft;
		// tree up helpers
		t = &head;
		g = p = NULL;
		q = m_impl->m_root;
		t->setChild(DirRight, q);
		// search down the tree
		for (;;) {
			if (q == NULL) {
				// insert new node at the bottom
				q = node.release();
				p->setChild(dir, q);
				++(m_impl->m_size);
			} else if (isRed(q->child(DirLeft)) && isRed(q->child(DirRight))) {
				// color flip
				q->m_color = ColorRed;
				q->child(DirLeft)->m_color = ColorBlack;
				q->child(DirRight)->m_color = ColorBlack;
			}
			if (isRed(q) && isRed(p)) {
				int dir2 = (t->child(DirRight) == g);
				if (q == p->child(last))
					t->setChild(dir2, g->rotate(!last));
				else
					t->setChild(dir2, g->rotateDouble(!last));
			}
			// stop if inserted
			if (node.get() == NULL)
				break;
			// stop if found
			int comp = compareKey(q, key);
			if (comp == 0) {
				q->replace(node.release());
				deleteNode(q);
				break;
			}
			// update helpers
			last = dir;
			dir = comp < 0;
			if (g != NULL)
				t = g;
			g = p;
			p = q;
			q = q->child(dir);
		}
		// update root
		m_impl->m_root = head.child(DirRight);
		m_impl->m_root->m_parent = NULL;
	}
	// make root black
	m_impl->m_root->m_color = ColorBlack;

	MojRbTreeAssertValid();
	return MojErrNone;
}

MojErr MojRbTreeBase::put(const MojRbTreeBase& tree)
{
	for (const Node* node = tree.begin(); node != NULL; node = node->successor()) {
		MojErr err = putClone(node);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojRbTreeBase::del(const void* key, bool& foundOut)
{
	MojRbTreeAssertValid();

	foundOut = false;
	MojErr err = ensureWritable();
	MojErrCheck(err);

	if (!empty()) {
		Node head; // dummy root
		Node *q, *p, *g; // helpers
		Node *f = NULL; // found item
		int dir = 1;
		// tree up helpers
		q = &head;
		g = p = NULL;
		q->setChild(DirRight, m_impl->m_root);
		// search and push a red down
		while (q->child(dir)) {
			int last = dir;
			// update helpers
			g = p, p = q;
			q = q->child(dir);
			// save found node
			int comp = compareKey(q, key);
			if (comp == 0)
				f = q;
			// push the red node down
			dir = comp < 0;
			if (!isRed(q) && !isRed(q->child(dir))) {
				if (isRed(q->child(!dir))) {
					p->setChild(last, q->rotate(dir));
					p = p->child(last);
				} else if (!isRed(q->child(!dir))) {
					Node* s = p->child(!last);
					if (s) {
						if (!isRed(s->child(!last)) && !isRed(s->child(last))) {
							// color flip
							p->m_color = ColorBlack;
							s->m_color = ColorRed;
							q->m_color = ColorRed;
						} else {
							int dir2 = (g->child(DirRight) == p);
							if (isRed(s->child(last)))
								g->setChild(dir2, p->rotateDouble(last));
							else if (isRed(s->child(!last)))
								g->setChild(dir2, p->rotate(last));
							// ensure correct coloring
							q->m_color = g->child(dir2)->m_color = ColorRed;
							g->child(dir2)->child(DirLeft)->m_color = ColorBlack;
							g->child(dir2)->child(DirRight)->m_color = ColorBlack;
						}
					}
				}
			}
		}
		// replace and remove if found
		if (f) {
			Node* c = q->child(q->child(DirLeft) == NULL);
			f->replace(q);
			p->setChild(p->child(DirRight) == q, c);
			deleteNode(f);
			foundOut = true;
			--(m_impl->m_size);
		}
		// update root and make it black
		m_impl->m_root = head.child(DirRight);
		if (m_impl->m_root) {
			m_impl->m_root->m_parent = NULL;
			m_impl->m_root->m_color = ColorBlack;
		} else {
			delete m_impl;
			m_impl = NULL;
		}
	}
	MojRbTreeAssertValid();
	return MojErrNone;
}

MojErr MojRbTreeBase::del(const MojRbTreeBase& tree)
{
	bool found;
	for (Node* node = tree.begin(); node != NULL; node = node->successor()) {
		MojErr err = del(key(node), found);
		MojErrCheck(err);
	}

	return MojErrNone;
}

void MojRbTreeBase::search(const void* key, Node*& nodeOut, int& compOut) const
{
	MojAssert(key);

	if (m_impl == NULL || m_impl->m_root == NULL) {
		nodeOut = NULL;
		compOut = 0;
	} else {
		nodeOut = m_impl->m_root;
		for (;;) {
			compOut = compareKey(nodeOut, key);
			if (compOut == 0)
				break;
			Node* nextNode = nodeOut->child(compOut < 0);
			if (nextNode == NULL)
				break;
			nodeOut = nextNode;
		}
	}
}

MojErr MojRbTreeBase::ensureWritable()
{
	if (m_impl && m_impl->m_refcount > 1) {
		MojErr err = clone();
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojRbTreeBase::clone()
{
	MojAssert(m_impl);

	MojAutoPtr<Impl> impl(new Impl(NULL));
	MojAllocCheck(impl.get());

	Node* root = NULL;
	MojErr err = clone(root, m_impl->m_root);
	MojErrCheck(err);

	impl->m_root = root;
	impl->m_size = m_impl->m_size;
	release();
	m_impl = impl.release();

	MojRbTreeAssertValid();
	return MojErrNone;
}

MojErr MojRbTreeBase::clone(Node*& dest, const Node* src)
{
	MojAssert(src);

	dest = NULL;
	Node* newNode = cloneNode(src);
	MojAllocCheck(newNode);
	newNode->m_color = src->m_color;
	for (int i = DirLeft; i <= DirRight; ++i) {
		if (src->child(i)) {
			Node* newChild = NULL;
			MojErr err = clone(newChild, src->child(i));
			// TODO: make this exception safe
			if (err != MojErrNone)
				release(newNode);
			MojErrCheck(err);
			newNode->setChild(i, newChild);
		}
	}
	dest = newNode;
	return MojErrNone;
}

MojErr MojRbTreeBase::putClone(const Node* node)
{
	MojAssert(node);

	MojAutoPtr<Node> newNode(cloneNode(node));
	MojAllocCheck(newNode.get());
	const void* nodeKey = key(newNode.get());
	MojErr err = put(newNode, nodeKey);
	MojErrCheck(err);

	return MojErrNone;
}

void MojRbTreeBase::init(Impl* impl)
{
	m_impl = impl;
	if (m_impl)
		++(m_impl->m_refcount);
}

void MojRbTreeBase::release()
{
	MojRbTreeAssertValid();

	if (m_impl && --(m_impl->m_refcount) == 0) {
		release(m_impl->m_root);
		delete m_impl;
	}
}

void MojRbTreeBase::release(Node* node)
{
	MojAssert(node);
	Node** child;
	while (node != NULL) {
		if (*(child = &node->m_children[DirLeft]) != NULL
			|| *(child = &node->m_children[DirRight]) != NULL) {
			node = *child;
			*child = NULL;
		} else {
			Node* parent = node->m_parent;
			deleteNode(node);
			node = parent;
		}
	}
}

#ifdef MOJ_DEBUG
void MojRbTreeBase::validate() const
{
	if (m_impl == NULL)
		return;

	MojAssert(m_impl->m_root);
	MojAssert(m_impl->m_root->m_parent == NULL);
	MojSize blackHeight = 0;
	MojSize size = 0;
	validate(m_impl->m_root, blackHeight, size);
	MojAssert(blackHeight > 0);
	MojAssert(size == m_impl->m_size);
}

void MojRbTreeBase::validate(Node* node, MojSize& blackHeight, MojSize& size) const
{
	if (node == NULL) {
		blackHeight++;
		return;
	}
	++size;
	Node* leftNode = node->child(DirLeft);
	Node* rightNode = node->child(DirRight);
	MojAssert(node != leftNode && node != rightNode);
	MojAssert(leftNode == NULL || leftNode != rightNode);
	// invalid binary search tree
	MojAssert(leftNode == NULL || compareKey(leftNode, key(node)) < 0);
	MojAssert(rightNode == NULL || compareKey(rightNode, key(node)) > 0);
	// red violation
	if (isRed(node)) {
		MojAssert(!isRed(leftNode) && !isRed(rightNode));
	}
	// validate children
	MojSize leftHeight = 0;
	MojSize rightHeight = 0;
	validate(leftNode, leftHeight, size);
	validate(rightNode, rightHeight, size);
	MojAssert(leftNode == NULL || leftNode->m_parent == node);
	MojAssert(rightNode == NULL || rightNode->m_parent == node);
	// black height mismatch
	MojAssert(leftHeight > 0 && rightHeight > 0);
	MojAssert(leftHeight == rightHeight);
	// increment black height
	blackHeight += leftHeight;
	if (!isRed(node))
		++blackHeight;
}
#endif // MOJ_DEBUG
