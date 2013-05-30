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


#include "MojSetTest.h"
#include "core/MojString.h"

MojSetTest::MojSetTest()
: MojTestCase(_T("MojSet"))
{
}

MojErr MojSetTest::run()
{
	MojSet<int> intSet1;
	MojSet<int> intSet2(intSet1);
	MojSet<int> intSet3;
	MojSet<MojString> strSet1;
	MojSet<MojString> strSet2(strSet1);
	MojString str;
	MojSet<int>::ConstIterator i1, i2, i3;
	bool found = true;
	int c;

	// empty test;
	MojTestAssert(intSet1.empty());
	MojTestAssert(intSet2.empty());
	MojTestAssert(intSet1.size() == 0);
	MojTestAssert(intSet2.size() == 0);
	MojTestAssert(intSet1.begin() == intSet1.end());
	intSet1.assign(intSet2);
	MojTestAssert(intSet1.empty());
	MojTestAssert(intSet2.empty());
	MojTestAssert(intSet1 == intSet2);
	MojTestAssert(intSet1.compare(intSet2) == 0);
	MojTestAssert(!(intSet2 != intSet1));
	MojTestAssert(!intSet1.contains(5));
	MojTestAssert(intSet1.find(49) == intSet1.end());
	MojErr err = intSet1.del(52, found);
	MojTestErrCheck(err);
	MojTestAssert(!found);
	intSet1.clear();
	MojTestAssert(intSet1.empty());
	intSet1 = intSet1;
	err = intSet1.diff(intSet2, intSet3);
	MojTestErrCheck(err);
	MojTestAssert(intSet3.empty());
	intSet1.swap(intSet2);
	intSet1.swap(intSet1);
	err = intSet1.put(intSet2);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.empty());
	err = intSet1.del(intSet2);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.empty());
	err = intSet1.intersect(intSet2);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.empty());

	// basic test
	err = intSet1.put(30);
	MojTestErrCheck(err);
	MojTestAssert(!intSet1.empty());
	MojTestAssert(intSet1.size() == 1);
	MojTestAssert(count(intSet1) == 1);
	MojTestAssert(intSet1.contains(30));
	i1 = intSet1.begin();
	MojTestAssert(*i1 == 30);
	i2 = intSet1.find(30);
	MojTestAssert(i1 == i2);
	MojTestAssert(*i2 == 30);
	++i1; ++i2;
	MojTestAssert(i1 == intSet1.end() && i2 == intSet1.end());
	MojTestAssert(intSet1 != intSet2);
	intSet2.assign(intSet1);
	MojTestAssert(intSet2.size() == 1);
	MojTestAssert(intSet2.contains(30));
	MojTestAssert(intSet1 == intSet2);
	MojTestAssert(intSet1.compare(intSet2) == 0);
	err = intSet1.put(30);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.contains(30));
	MojTestAssert(intSet1 == intSet2);
	err = intSet1.del(30, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(intSet1.empty());
	for (int i = 0; i < 1000; ++i) {
		err = intSet1.put(i);
		MojTestErrCheck(err);
		MojTestAssert(intSet1.contains(i));
		MojTestAssert(intSet1.size() == (gsize) i + 1);
		MojTestAssert(intSet1 != intSet2);
		err = intSet1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = intSet1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(!found);
		err = intSet1.put(i);
		MojTestErrCheck(err);
		MojTestAssert(intSet1.contains(i));
		err = intSet1.put(i);
		MojTestErrCheck(err);
		MojTestAssert(intSet1.contains(i));

		err = str.format(_T("%d"), i);
		MojTestErrCheck(err);
		err = strSet1.put(str);
		MojTestErrCheck(err);
		MojTestAssert(strSet1.size() == (gsize) i + 1);
		MojTestAssert(strSet1.contains(str));
		err = strSet1.del(str, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = strSet1.del(str, found);
		MojTestErrCheck(err);
		MojTestAssert(!found);
		err = strSet1.put(str);
		MojTestErrCheck(err);
		MojTestAssert(strSet1.contains(str));
		err = strSet1.put(str);
		MojTestErrCheck(err);
		MojTestAssert(strSet1.contains(str));
	}
	c = 0;
	for (MojSet<int>::ConstIterator i = intSet1.begin(); i != intSet2.end(); ++i) {
		MojTestAssert(*i == c++);
	}
	MojTestAssert(c == 1000);
	MojTestAssert(*intSet1.begin() == 0);
	MojTestAssert(intSet1.size() == 1000);
	MojTestAssert(strSet1.size() == 1000);
	MojTestAssert(count(intSet1) == 1000);
	intSet1 = intSet1;
	MojTestAssert(intSet1.contains(30));
	intSet2 = intSet1;
	MojTestAssert(intSet2.size() == 1000);
	intSet1.clear();
	MojTestAssert(intSet1.empty());
	for (int i = 999; i >= 0; --i) {
		err = intSet1.put(i);
		MojTestErrCheck(err);
		MojTestAssert(intSet1.contains(i));
	}
	MojTestAssert(*intSet1.begin() == 0);
	MojTestAssert(intSet1.size() == 1000);
	MojTestAssert(count(intSet1) == 1000);
	MojTestAssert(intSet1 == intSet2);

	intSet1.clear();
	intSet2.clear();
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			err = intSet1.put(i);
			MojTestErrCheck(err);
		}
	}
	MojTestAssert(intSet1.size() == 5);
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 1) {
			err = intSet2.put(i);
			MojTestErrCheck(err);
		}
	}
	MojTestAssert(intSet2.size() == 5);
	err = intSet1.put(intSet2);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.size() == 10);
	for (int i = 0; i < 10; ++i) {
		MojTestAssert(intSet1.contains(i));
	}
	err = intSet1.diff(intSet2, intSet3);
	MojTestErrCheck(err);
	MojTestAssert(intSet3.size() == 5);
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			MojTestAssert(intSet3.contains(i));
		} else {
			MojTestAssert(!intSet3.contains(i));
		}
	}
	intSet1.clear();
	err = intSet2.diff(intSet1, intSet3);
	MojTestErrCheck(err);
	MojTestAssert(intSet3 == intSet2);
	err = intSet2.del(intSet3);
	MojTestErrCheck(err);
	MojTestAssert(intSet2.empty());

	// comparisons
	for (int i = 0; i < 10; ++i) {
		err = intSet1.put(i);
		MojTestErrCheck(err);
	}
	intSet2 = intSet1;
	err = intSet2.put(11);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.compare(intSet2) < 0);
	MojTestAssert(intSet1 < intSet2);
	MojTestAssert(intSet1 <= intSet2);
	MojTestAssert(intSet2 > intSet1);
	MojTestAssert(intSet2 >= intSet1);
	err = intSet1.put(12);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.compare(intSet2) > 0);
	MojTestAssert(intSet1 > intSet2);
	MojTestAssert(intSet1 >= intSet2);
	MojTestAssert(intSet2 < intSet1);
	MojTestAssert(intSet2 <= intSet1);

	// intersection
	intSet1.clear();
	intSet2.clear();
	for (int i = 0; i < 10; ++i) {
		err = intSet1.put(i);
		MojTestErrCheck(err);
		if (i % 2 == 0) {
			err = intSet2.put(i);
			MojTestErrCheck(err);
		}
	}
	err = intSet1.intersect(intSet2);
	MojTestErrCheck(err);
	MojTestAssert(intSet1.size() == 5);
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			MojTestAssert(intSet1.contains(i));
		} else {
			MojTestAssert(!intSet1.contains(i));
		}
	}

	return MojErrNone;
}

gsize MojSetTest::count(MojSet<int>& set)
{
	gsize size = 0;
	for (MojSet<int>::ConstIterator i = set.begin(); i != set.end(); ++i) {
		++size;
	}
	return size;
}
