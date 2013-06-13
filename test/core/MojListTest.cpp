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


#include "MojListTest.h"
#include "core/MojList.h"
#include "core/MojVector.h"

class TestItem : public MojRefCounted
{
public:
	TestItem(int val) : m_val(val) {}
	int m_val;
	MojListEntry m_entry1;
	MojListEntry m_entry2;
};

MojListTest::MojListTest()
: MojTestCase("MojList")
{
}

MojErr MojListTest::run()
{
	// create a bunch of TestItems
	typedef MojVector<MojRefCountedPtr<TestItem> > TestVec;
	TestVec vec;
	for (int i = 0; i < 100; ++i) {
		MojErr err = vec.push(new TestItem(i));
		MojTestErrCheck(err);
	}

	// empty test
	MojList<TestItem, &TestItem::m_entry1> list1;
	MojTestAssert(list1.empty());
	MojTestAssert(list1.size() == 0);
	TestItem item1(0);
	MojTestAssert(!list1.contains(&item1));
	MojTestAssert(list1.begin() == list1.end());
	const MojList<TestItem, &TestItem::m_entry1>& clist1 = list1;
	MojTestAssert(clist1.begin() == clist1.end());

	// push back
	TestVec::Iterator vecIter;
	MojErr err = vec.begin(vecIter);
	MojTestErrCheck(err);
	for (; vecIter != vec.end(); ++vecIter) {
		MojTestAssert(list1.size() == (MojSize) (*vecIter)->m_val);
		list1.pushBack(vecIter->get());
		MojTestAssert(!list1.empty());
		MojTestAssert(list1.contains(vecIter->get()));
		MojTestAssert(list1.back() == vecIter->get());
		MojTestAssert(list1.front() == clist1.front());
		MojTestAssert(list1.back() == clist1.back());
		if (list1.size() == 1)
			MojTestAssert(list1.front() == vecIter->get());
	}

	// const iterator
	int count = 0;
	for (MojList<TestItem, &TestItem::m_entry1>::ConstIterator i = list1.begin();
		 i != list1.end();
		 ++i, ++count) {
		MojAssert((*i)->m_val == count);
	}
	MojAssert(count == 100);

	// non-const iterator
	for (MojList<TestItem, &TestItem::m_entry1>::Iterator i = list1.begin();
		 i != list1.end();
		 ++i) {
		(*i)->m_val = -(*i)->m_val;
	}

	// iterator erase
	count = 0;
	MojList<TestItem, &TestItem::m_entry1>::Iterator listIter = list1.begin();
	while (listIter != list1.end()) {
		if (count++ % 2 == 0) {
			(listIter++).erase();
		} else {
			listIter++;
		}
	}
	MojTestAssert(list1.size() == 50);
	count = -1;
	listIter = list1.begin();
	for (MojList<TestItem, &TestItem::m_entry1>::ConstIterator i = list1.begin();
		 i != list1.end();
		 ++i) {
		MojTestAssert((*i)->m_val == count);
		count -=2;
	}
	MojTestAssert(count == -101);

	// list erase
	listIter = list1.begin();
	while (listIter != list1.end()) {
		list1.erase(*(listIter++));
	}
	MojTestAssert(list1.size() == 0);
	MojTestAssert(list1.empty());

	// add to two lists (front and back)
	MojList<TestItem, &TestItem::m_entry2> list2;
	err = vec.begin(vecIter);
	MojTestErrCheck(err);
	count = 0;
	for (; vecIter != vec.end(); ++vecIter) {
		(*vecIter)->m_val = count++;
		list1.pushBack(vecIter->get());
		list2.pushFront(vecIter->get());
	}
	// verify order
	count = 0;
	listIter = list1.begin();
	MojList<TestItem, &TestItem::m_entry2>::Iterator listIter2 = list2.begin();
	while (listIter != list1.end() && listIter2 != list2.end()) {
		int val1 = (*listIter)->m_val;
		int val2 = (*listIter2)->m_val;
		MojTestAssert(val1 == count);
		MojTestAssert(val2 == 99 - count);
		++count;
		++listIter;
		++listIter2;
	}
	MojTestAssert(count == 100);

	// popFront/popBack
	count = 0;
	while (!list1.empty() && !list2.empty()) {
		TestItem* item1 = list1.popFront();
		TestItem* item2 = list2.popBack();
		MojTestAssert(item1 == item2);
		++count;
	}
	MojTestAssert(count == 100);
	MojTestAssert(list1.empty() && list2.empty());

	// assignment
	MojList<TestItem, &TestItem::m_entry1> list3(list1);
	MojTestAssert(list1.begin() == list1.end() && list3.begin() == list3.end());
	list1.swap(list3);
	MojTestAssert(list1.begin() == list1.end() && list3.begin() == list3.end());
	list3 = list1;
	MojTestAssert(list1.begin() == list1.end() && list3.begin() == list3.end());
	MojList<TestItem, &TestItem::m_entry1> list4(list1);
	MojTestAssert(list1.begin() == list1.end() && list4.begin() == list4.end());

	err = vec.begin(vecIter);
	MojTestErrCheck(err);
	for (; vecIter != vec.end(); ++vecIter) {
		list1.pushBack(vecIter->get());
	};
	MojTestAssert(list1.size() == 100);
	MojList<TestItem, &TestItem::m_entry1> list5(list1);
	MojTestAssert(list1.size() == 0 && list5.size() == 100);
	MojTestAssert(list1.begin() == list1.end());
	count = 0;
	for (MojList<TestItem, &TestItem::m_entry1>::ConstIterator i = list5.begin();
		 i != list5.end();
		 ++i) {
		MojTestAssert((*i)->m_val == count);
		++count;
	}
	MojTestAssert(count == 100);
	list5 = list5;
	MojTestAssert(list5.size() == 100);
	list1 = list5;
	MojTestAssert(list5.empty() && list1.size() == 100);
	count = 0;
	for (MojList<TestItem, &TestItem::m_entry1>::ConstIterator i = list1.begin();
		 i != list1.end();
		 ++i) {
		MojTestAssert((*i)->m_val == count);
		++count;
	}
	MojTestAssert(count == 100);
	list5.swap(list1);
	MojTestAssert(list1.empty() && list5.size() == 100);
	TestItem* item = list5.popFront();
	list1.pushBack(item);
	MojTestAssert(list1.size() == 1 && list5.size() == 99);
	list1 = list5;
	MojTestAssert(list1.size() == 99 && list5.size() == 0);
	item = list1.popFront();
	list5.pushBack(item);
	MojTestAssert(list1.size() == 98 && list5.size() == 1);
	list5.swap(list1);
	MojTestAssert(list1.size() == 1 && list5.size() == 98);

	// clear
	list5.clear();

	return MojErrNone;
}
