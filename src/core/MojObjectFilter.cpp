/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "core/MojObjectFilter.h"

MojObjectFilter::MojObjectFilter()
: m_visitor(NULL),
  m_nextNode(NULL)
{
}

MojObjectFilter::~MojObjectFilter()
{
}

MojErr MojObjectFilter::init(const StringSet& selectProps)
{
	MojAssert(m_root.get() == NULL);

	//create the root node
	m_root.reset(new Node(MojString()));
	MojAllocCheck(m_root.get());

	for (StringSet::ConstIterator i = selectProps.begin(); i != selectProps.end(); i++) {
		MojVector<MojString> parts;
		MojErr err = i->split(_T('.'), parts);
		MojErrCheck(err);

		Node* curNode = m_root.get();
		for (MojVector<MojString>::ConstIterator pi = parts.begin(); pi != parts.end(); ++pi) {
			Node* child = curNode->find(*pi);
			if (child) {
				if (child->m_children.empty()) {
					break;
				} else {
					curNode = child;
				}
			} else {
				MojAutoPtr<Node> node(new Node(*pi));
				MojAllocCheck(node.get());
				Node* nextNode = node.get();
				MojErr err = curNode->addChild(node);
				MojErrCheck(err);
				curNode = nextNode;
			}
		}
	}
	return MojErrNone;
}

void MojObjectFilter::setVisitor(MojObjectVisitor* visitor)
{
	MojAssert(visitor);
	MojAssert(m_root.get());

	m_visitor = visitor;
	m_stack.clear();
	m_state.reset(m_root.get());
	m_nextNode = m_state.m_node;
}

MojErr MojObjectFilter::reset()
{
	return MojErrNone;
}

MojErr MojObjectFilter::beginObject()
{
	return begin(false, &MojObjectVisitor::beginObject);
}

MojErr MojObjectFilter::endObject()
{
	return end(&MojObjectVisitor::endObject);
}

MojErr MojObjectFilter::beginArray()
{
	return begin(true, &MojObjectVisitor::beginArray);
}

MojErr MojObjectFilter::endArray()
{
	return end(&MojObjectVisitor::endArray);
}

MojErr MojObjectFilter::propName(const MojChar* name, MojSize len)
{
	MojAssert(m_visitor);
	MojAssert(name);

	// we only advance to the next node if we match a prop name below
	m_nextNode = NULL;
	// check for a node name match
	if (m_state.m_node) {
		bool end = false;
		do {
			// compare name of current child to prop name
			int comp = 1;
			end = (m_state.m_child == m_state.m_node->m_children.end());
			if (!end) {
				// TODO: proper n compare
				comp = (*m_state.m_child)->m_propName.compare(name);
			}

			if (comp == 0) {
				// names match, so include if the current child is childless
				if ((*m_state.m_child)->m_children.empty()) {
					m_state.m_includeCount = 1;
				} else {
					m_state.m_includeCount = 0;
					m_nextNode = (*m_state.m_child);
				}
				break;
			} else if (comp > 0) {
				// child name was greater than prop name, so skip the current sub-tree
				m_state.m_includeCount = 0;
				break;
			}
			// advance to next child and retry
			++(m_state.m_child);
		} while (!end);
	}

	if (m_state.m_includeCount) {
		if (!m_state.m_included) {
			MojErr err = includeParents();
			MojErrCheck(err);
		}
		MojErr err = m_visitor->propName(name, len);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::nullValue()
{
	MojAssert(m_visitor);

	if (m_state.m_includeCount) {
		m_state.m_includeCount--;
		MojErr err = m_visitor->nullValue();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::boolValue(bool val)
{
	MojAssert(m_visitor);

	if (m_state.m_includeCount) {
		m_state.m_includeCount--;
		MojErr err = m_visitor->boolValue(val);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::intValue(MojInt64 val)
{
	MojAssert(m_visitor);

	if (m_state.m_includeCount) {
		m_state.m_includeCount--;
		MojErr err = m_visitor->intValue(val);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::decimalValue(const MojDecimal& val)
{
	MojAssert(m_visitor);

	if (m_state.m_includeCount) {
		m_state.m_includeCount--;
		MojErr err = m_visitor->decimalValue(val);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::stringValue(const MojChar* val, MojSize len)
{
	MojAssert(m_visitor);

	if (m_state.m_includeCount) {
		m_state.m_includeCount--;
		MojErr err = m_visitor->stringValue(val, len);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::begin(bool array, VisitorMethod method)
{
	MojAssert(m_visitor);

	// push old state
	MojErr err = m_stack.push(m_state);
	MojErrCheck(err);

	// initialize new state (special-casing root object)
	m_state.m_included = m_state.m_includeCount || m_stack.size() == 1;
	m_state.m_array = array;
	m_state.m_node = m_nextNode;

	// reset child iter
	if (m_state.m_node) {
		m_state.m_child = m_state.m_node->m_children.begin();
	} else {
		m_state.m_child = NULL;
	}
	// bump include count if we are including entire subtree
	if (m_state.m_includeCount >= 1) {
		m_state.m_includeCount = MojSizeMax;
	}
	// visit
	if (m_state.m_included) {
		err = (m_visitor->*method)();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojObjectFilter::end(VisitorMethod method)
{
	MojAssert(m_visitor);
	MojAssert(!m_stack.empty());

	if (m_state.m_included) {
		MojErr err = (m_visitor->*method)();
		MojErrCheck(err);
	}
	// save node so we can re-enter it in the next array element
	m_nextNode = m_state.m_node;
	// pop state
	m_state = m_stack.back();
	MojErr err = m_stack.pop();
	MojErrCheck(err);
	// decrement include count
	if (m_state.m_includeCount)
		m_state.m_includeCount--;

	return MojErrNone;
}

MojErr MojObjectFilter::includeParents()
{
	MojAssert(!m_state.m_included);

	State* prevState = NULL;
	StateStack::Iterator iter;
	MojErr err = m_stack.begin(iter);
	MojErrCheck(err);
	for (; iter != m_stack.end(); ++iter) {
		if (!iter->m_included) {
			err = include(*iter, prevState);
			MojErrCheck(err);
		}
		prevState = iter;
	}
	err = include(m_state, prevState);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectFilter::include(State& state, State* prevState)
{
	MojAssert(prevState);
	MojAssert(state.m_node);

	state.m_included = true;
	if (prevState && !prevState->m_array) {
		MojString& name = state.m_node->m_propName;
		MojErr err = m_visitor->propName(name, name.length());
		MojErrCheck(err);
	}
	if (state.m_array) {
		MojErr err = m_visitor->beginArray();
		MojErrCheck(err);
	} else {
		MojErr err = m_visitor->beginObject();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojObjectFilter::Node::Node(const MojString& propName)
: m_propName(propName)
{
}

MojObjectFilter::Node::~Node()
{
	MojDeleteRange(m_children.begin(), m_children.end());
}

MojErr MojObjectFilter::Node::addChild(MojAutoPtr<Node> child)
{
	MojAssert(child.get());

	MojErr err = this->m_children.push(child.get());
	MojErrCheck(err);
	child.release();

	return MojErrNone;
}

MojObjectFilter::Node* MojObjectFilter::Node::find(const MojString& propName)
{
	for (MojVector<Node*>::ConstIterator ci = m_children.begin(); ci != m_children.end(); ++ci) {
		if (propName == (*ci)->m_propName) {
			return *ci;
		}
	}
	return NULL;
}

void MojObjectFilter::State::reset(Node* node)
{
	m_node = node;
	m_child = NULL;
	m_includeCount = 0;
	m_included = true;
	m_array = false;
}
