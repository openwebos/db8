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


#ifndef MOJOBJECTFILTER_H_
#define MOJOBJECTFILTER_H_

#include "MojObject.h"
#include "core/MojSet.h"
#include "core/MojString.h"

class MojObjectFilter: public MojObjectVisitor
{
public:
	typedef MojSet<MojString> StringSet;

	MojObjectFilter();
	~MojObjectFilter();

	MojErr init(const StringSet& selectProps);
	void setVisitor(MojObjectVisitor* visitor);

	virtual MojErr reset();
	virtual MojErr beginObject();
	virtual MojErr endObject();
	virtual MojErr beginArray();
	virtual MojErr endArray();
	virtual MojErr propName(const MojChar* name, MojSize len);
	virtual MojErr nullValue();
	virtual MojErr boolValue(bool val);
	virtual MojErr intValue(MojInt64 val);
	virtual MojErr decimalValue(const MojDecimal& val);
	virtual MojErr stringValue(const MojChar* val, MojSize len);

private:
	struct Node
	{
		typedef MojVector<Node*> NodeVec;

		Node(const MojString& propName);
		~Node();

		MojErr addChild(MojAutoPtr<Node> child);
		Node* find(const MojString& propName);

		MojString m_propName;
		NodeVec m_children;
	};

	struct State
	{
		typedef Node::NodeVec::ConstIterator NodeIter;

		State() { reset(NULL); }
		void reset(Node* node);

		Node* m_node;
		NodeIter m_child;
		MojSize m_includeCount;
		bool m_included;
		bool m_array;
	};

	typedef MojVector<State> StateStack;
	typedef MojErr (MojObjectVisitor::* VisitorMethod)();

	void init(MojObjectVisitor* visitor);
	MojErr begin(bool array, VisitorMethod method);
	MojErr end(VisitorMethod method);
	MojErr includeParents();
	MojErr include(State& state, State* prevState);

	MojAutoPtr<Node> m_root;
	MojObjectVisitor* m_visitor;
	StateStack m_stack;
	State m_state;
	Node* m_nextNode;
};

#endif /* MOJOBJECTFILTER_H_ */
