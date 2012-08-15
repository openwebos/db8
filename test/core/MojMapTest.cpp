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


#include "MojMapTest.h"
#include "core/MojString.h"

MojMapTest::MojMapTest()
: MojTestCase(_T("MojMap"))
{
}

MojErr MojMapTest::run()
{
	MojMap<int, MojString> intMap1;
	MojMap<int, MojString> intMap2(intMap1);
	MojMap<int, MojString> intMap3;
	MojMap<MojString, int> strMap1;
	MojMap<MojString, int> strMap2(strMap1);
	MojString str;
	MojMap<int, MojString>::ConstIterator ci1, ci2, ci3;
	MojMap<int, MojString>::Iterator i1, i2, i3;
	bool found = true;
	int c;

	// empty test;
	MojTestAssert(intMap1.empty());
	MojTestAssert(intMap2.empty());
	MojTestAssert(intMap1.size() == 0);
	MojTestAssert(intMap2.size() == 0);
	MojTestAssert(intMap1.begin() == intMap1.end());
	intMap1.assign(intMap2);
	MojTestAssert(intMap1.empty());
	MojTestAssert(intMap2.empty());
	MojTestAssert(intMap1 == intMap2);
	MojTestAssert(intMap1.compare(intMap2) == 0);
	MojTestAssert(!(intMap2 != intMap1));
	MojTestAssert(!intMap1.contains(5));
	MojTestAssert(intMap1.find(49) == intMap1.end());
	MojErr err = intMap1.find(8, i1);
	MojTestErrCheck(err);
	MojTestAssert(i1 == intMap1.end());
	MojTestAssert(intMap1.lowerBound(49) == intMap1.end());
	err = intMap1.lowerBound(8, i1);
	MojTestErrCheck(err);
	MojTestAssert(i1 == intMap1.end());
	MojTestAssert(intMap1.upperBound(49) == intMap1.end());
	err = intMap1.upperBound(8, i1);
	MojTestErrCheck(err);
	MojTestAssert(i1 == intMap1.end());
	err = intMap1.del(52, found);
	MojTestErrCheck(err);
	MojTestAssert(!found);
	intMap1.clear();
	MojTestAssert(intMap1.empty());
	intMap1 = intMap1;
	err = intMap1.diff(intMap2, intMap3);
	MojTestErrCheck(err);
	MojTestAssert(intMap3.empty());
	intMap1.swap(intMap2);
	intMap1.swap(intMap1);
	err = intMap1.put(intMap2);
	MojTestErrCheck(err);
	MojTestAssert(intMap1.empty());

	// basic test
	err = str.format(_T("%d"), 30);
	MojTestErrCheck(err);
	err = intMap1.put(30, str);
	MojTestErrCheck(err);
	MojTestAssert(!intMap1.empty());
	MojTestAssert(intMap1.size() == 1);
	MojTestAssert(count(intMap1) == 1);
	MojTestAssert(intMap1.contains(30));
	ci1 = intMap1.begin();
	MojTestAssert(ci1.key() == 30);
	MojTestAssert(ci1.value() == str);
	ci2 = intMap1.find(30);
	MojTestAssert(ci1 == ci2);
	MojTestAssert(*ci2 == str);
	++ci1; ++ci2;
	MojTestAssert(ci1 == intMap1.end() && ci2 == intMap1.end());
	MojTestAssert(intMap1 != intMap2);
	intMap2.assign(intMap1);
	MojTestAssert(intMap2.size() == 1);
	MojTestAssert(intMap2.contains(30));
	MojTestAssert(intMap1 == intMap2);
	MojTestAssert(intMap1.compare(intMap2) == 0);
	err = intMap1.put(30, str);
	MojTestErrCheck(err);
	MojTestAssert(intMap1.contains(30));
	MojTestAssert(intMap1 == intMap2);
	err = intMap1.find(30, i1);
	MojTestErrCheck(err);
	MojTestAssert(i1.key() == 30 && i1.value() == _T("30"));
	err = i1.value().append(_T('0'));
	MojTestErrCheck(err);
	MojTestAssert(*intMap1.find(30) == _T("300"));
	err = intMap1.del(30, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(intMap1.empty());
	for (int i = 0; i < 1000; ++i) {
		err = str.format(_T("%d"), i);
		MojTestErrCheck(err);
		err = intMap1.put(i, str);
		MojTestErrCheck(err);
		MojTestAssert(intMap1.contains(i));
		MojTestAssert(intMap1.size() == (MojSize) i + 1);
		MojTestAssert(intMap1 != intMap2);
		err = intMap1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = intMap1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(!found);
		err = intMap1.put(i, str);
		MojTestErrCheck(err);
		MojTestAssert(intMap1.contains(i));
		err = intMap1.put(i, str);
		MojTestErrCheck(err);
		MojTestAssert(intMap1.contains(i));

		err = strMap1.put(str, i);
		MojTestErrCheck(err);
		MojTestAssert(strMap1.size() == (MojSize) i + 1);
		MojTestAssert(strMap1.contains(str));
		err = strMap1.del(str, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = strMap1.del(str, found);
		MojTestErrCheck(err);
		MojTestAssert(!found);
		err = strMap1.put(str, i);
		MojTestErrCheck(err);
		MojTestAssert(strMap1.contains(str));
		err = strMap1.put(str, i);
		MojTestErrCheck(err);
		MojTestAssert(strMap1.contains(str));
	}
	c = 0;
	for (MojMap<int, MojString>::ConstIterator i = intMap1.begin(); i != intMap2.end(); ++i) {
		MojTestAssert(i.key() == c++);
	}
	MojTestAssert(c == 1000);
	MojTestAssert(intMap1.begin().key() == 0);
	MojTestAssert(intMap1.size() == 1000);
	MojTestAssert(strMap1.size() == 1000);
	MojTestAssert(count(intMap1) == 1000);
	intMap1 = intMap1;
	MojTestAssert(intMap1.contains(30));
	intMap2 = intMap1;
	MojTestAssert(intMap2.size() == 1000);
	intMap1.clear();
	MojTestAssert(intMap1.empty());
	for (int i = 999; i >= 0; --i) {
		err = str.format(_T("%d"), i);
		MojTestErrCheck(err);
		err = intMap1.put(i, str);
		MojTestErrCheck(err);
		MojTestAssert(intMap1.contains(i));
	}
	MojTestAssert(intMap1.begin().key() == 0);
	MojTestAssert(intMap1.size() == 1000);
	MojTestAssert(count(intMap1) == 1000);
	MojTestAssert(intMap1 == intMap2);

	intMap1.clear();
	intMap2.clear();
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			err = str.format(_T("%d"), i);
			MojTestErrCheck(err);
			err = intMap1.put(i, str);
			MojTestErrCheck(err);
		}
	}
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 1) {
			err = str.format(_T("%d"), i);
			MojTestErrCheck(err);
			err = intMap2.put(i, str);
			MojTestErrCheck(err);
		}
	}
	err = intMap1.put(intMap2);
	MojTestErrCheck(err);
	MojTestAssert(intMap1.size() == 10);
	for (int i = 0; i < 10; ++i) {
		MojTestAssert(intMap1.contains(i));
	}
	err = intMap1.diff(intMap2, intMap3);
	MojTestErrCheck(err);
	MojTestAssert(intMap3.size() == 5);
	for (int i = 0; i < 10; ++i) {
		if (i % 2 == 0) {
			MojTestAssert(intMap3.contains(i));
		} else {
			MojTestAssert(!intMap3.contains(i));
		}
	}
	intMap1.clear();
	err = intMap2.diff(intMap1, intMap3);
	MojTestErrCheck(err);
	MojTestAssert(intMap3 == intMap2);

	// lower/upper bound
	intMap1.clear();
	for (int i = 0; i < 40; ++i) {
		if (i % 2 == 1) {
			err = str.format(_T("%d"), i);
			MojTestErrCheck(err);
			err = intMap1.put(i, str);
			MojTestErrCheck(err);
		}
	}
	for (int i = 0; i < 38; ++i) {
		if (i % 2 == 1) {
			MojTestAssert(intMap1.lowerBound(i).key() == i);
			MojTestAssert(intMap1.upperBound(i).key() == i + 2);
		} else {
			int lb = intMap1.lowerBound(i).key();
			int ub = intMap1.upperBound(i).key();
			MojTestAssert(lb == i + 1);
			MojTestAssert(ub == i + 1);
		}
	}
	MojTestAssert(intMap1.lowerBound(39).key() == 39);
	MojTestAssert(intMap1.upperBound(39) == intMap1.end());
	MojTestAssert(intMap1.lowerBound(40) == intMap1.end());
	MojTestAssert(intMap1.upperBound(40) == intMap1.end());

	// comparisons
	intMap1.clear();
	for (int i = 0; i < 10; ++i) {
		err = str.format(_T("%d"), i);
		MojTestErrCheck(err);
		err = intMap1.put(i, str);
		MojTestErrCheck(err);
	}
	intMap2 = intMap1;
	err = intMap2.put(11, str);
	MojTestErrCheck(err);
	MojTestAssert(intMap1.compare(intMap2) < 0);
	MojTestAssert(intMap1 < intMap2);
	MojTestAssert(intMap1 <= intMap2);
	MojTestAssert(intMap2 > intMap1);
	MojTestAssert(intMap2 >= intMap1);
	err = intMap1.put(12, str);
	MojTestErrCheck(err);
	MojTestAssert(intMap1.compare(intMap2) > 0);
	MojTestAssert(intMap1 > intMap2);
	MojTestAssert(intMap1 >= intMap2);
	MojTestAssert(intMap2 < intMap1);
	MojTestAssert(intMap2 <= intMap1);
	strMap1.clear();
	strMap2.clear();
	err = strMap1.put(str, 2);
	MojTestErrCheck(err);
	strMap2 = strMap1;
	MojTestAssert(strMap1.compare(strMap2) == 0);
	err = strMap2.put(str, 3);
	MojTestErrCheck(err);
	MojTestAssert(strMap1.compare(strMap2) < 0);
	err = strMap1.put(str, 4);
	MojTestErrCheck(err);
	MojTestAssert(strMap1.compare(strMap2) > 0);

	return MojErrNone;
}

MojSize MojMapTest::count(MojMap<int, MojString>& map)
{
	MojSize size = 0;
	for (MojMap<int, MojString>::ConstIterator i = map.begin(); i != map.end(); ++i) {
		++size;
	}
	return size;
}
