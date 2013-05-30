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


#include "MojVectorTest.h"
#include "core/MojString.h"
#include "core/MojVector.h"

static int s_defaultConstructCount = 0;
static int s_copyConstructCount = 0;
static int s_destructCount = 0;
static int s_assignCount = 0;

class MojVectorTestObj
{
public:
	MojVectorTestObj() { s_defaultConstructCount++; }
	MojVectorTestObj(const MojVectorTestObj& obj) { s_copyConstructCount++; }
	~MojVectorTestObj() { s_destructCount++; }

	MojVectorTestObj& operator=(const MojVectorTestObj& obj) { s_assignCount++; return *this; }
};

MojVectorTest::MojVectorTest()
: MojTestCase(_T("MojVector"))
{
}

MojErr MojVectorTest::run()
{
	MojErr err = intTest();
	MojTestErrCheck(err);
	err = stringTest();
	MojTestErrCheck(err);
	err = objTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojVectorTest::intTest()
{
	MojVector<int> v1;
	MojVector<int> v2(v1);
	MojVector<int> v3;
	static const int array1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	static const int array2[] = {99, 99, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	static const int array3[] = {99, 99, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 88, 88, 9};
	static const int array4[] = {99, 99, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 88, 88, 9};
	static const int array5[] = {99, 99, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 88, 88, 9, 22, 22};
	static const int array6[] = {1, 0, 0, 0, 1, 1, 3, 1};
	static const int array7[] = {1, 2, 3, 4, 5, 6, 7, 8};
	static const int array8[] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	static const int array9[] = {8, 7, 6, 5, 4, 3, 2, 1};
	gsize idx;

	// empty test
	MojTestAssert(v1.size() == 0);
	MojTestAssert(v1.capacity() == 0);
	MojTestAssert(v1.empty());
	MojTestAssert(v1.begin() == v1.end());
	MojTestAssert(v1 == v1);
	MojTestAssert(v1 == v2);
	MojTestAssert(v1 <= v2);
	MojTestAssert(v1 >= v2);
	MojTestAssert(!(v1 != v2));
	MojTestAssert(!(v1 < v2));
	MojTestAssert(!(v1 > v2));
	MojTestAssert(v1.compare(v2) == 0);
	v1.clear();
	v1.swap(v1);
	v1.swap(v2);
	v1.assign(v1);
	v1.assign(v2);
	v1 = v1;
	v1 = v2;
	MojTestAssert(v1.find(1) == MojInvalidIndex);
	MojErr err = v1.assign(NULL, 0);
	MojErrCheck(err);
	MojTestAssert(v1.empty());
	err = v1.reverse();
	MojTestErrCheck(err);
	MojTestAssert(v1.empty());
	err = v1.sort();
	MojTestErrCheck(err);
	MojTestAssert(v1.empty());

	// size, capacity, empty, at, front, back
	err = v1.push(1);
	MojTestErrCheck(err);
	MojTestAssert(v1.size() == 1);
	MojTestAssert(v1.capacity() >= 1);
	MojTestAssert(!v1.empty());
	MojTestAssert(v1.at(0) == 1);
	MojTestAssert(v1.front() == 1);
	MojTestAssert(v1.back() == 1);
	err = v1.reverse();
	MojTestErrCheck(err);
	MojTestAssert(v1.front() == 1);
	v2 = v1;
	MojTestAssert(v2.size() == 1);
	MojTestAssert(v2.capacity() >= 1);
	MojTestAssert(v2.at(0) == 1);
	MojTestAssert(v1.begin() == v2.begin());
	err = v1.setAt(0, 0);
	MojTestErrCheck(err);
	MojTestAssert(v1.at(0) == 0);
	MojTestAssert(v1.front() == 0);
	MojTestAssert(v1.capacity() >= 1);
	MojTestAssert(v2.at(0) == 1);
	MojTestAssert(v1.begin() != v2.begin());
	// swap
	v1.swap(v2);
	MojTestAssert(v1.at(0) == 1);
	MojTestAssert(v2.at(0) == 0);
	// assign
	v1.assign(v2);
	MojTestAssert(v1.at(0) == 0);
	MojTestAssert(v2.at(0) == 0);
	// clear
	v1.clear();
	MojTestAssert(v1.empty() && v1.size() == 0);
	// reserve
	err = v1.reserve(50);
	MojTestErrCheck(err);
	MojTestAssert(v1.empty() && v1.size() == 0);
	MojTestAssert(v1.capacity() == 50);
	// push, pop
	for (int i = 0; i < 500; i++) {
		err = v1.push(i);
		MojTestErrCheck(err);
		MojTestAssert(v1.back() == i);
		for (int j = 0; j < i; j++) {
			MojTestAssert(v1.at(j) == j);
		}
	}
	MojTestAssert(v1.size() == 500);
	v2 = v1;
	for (int i = 499; i >= 0; i--) {
		MojTestAssert(v1.back() == i);
		err = v1.pop();
		MojTestErrCheck(err);
	}
	MojTestAssert(v1.empty());
	MojTestAssert(v2.size() == 500);
	// resize
	err = v2.resize(200);
	MojTestErrCheck(err);
	MojTestAssert(v2.size() == 200);
	err = v2.resize(300, 8);
	MojTestErrCheck(err);
	MojTestAssert(v2.size() == 300);
	MojTestAssert(v2.at(200) == 8 && v2.back() == 8);
	err = v2.resize(0);
	MojTestErrCheck(err);
	MojTestAssert(v2.empty());
	// insert
	err = v3.insert(0, array1, array1 + 10);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 10);
	MojTestAssert(MojMemCmp(&v3.front(), array1, 10 * sizeof(int)) == 0);
	err = v3.insert(0, array1, array1 + 10);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 20);
	MojTestAssert(MojMemCmp(&v3.front(), array1, 20 * sizeof(int)) == 0);
	err = v3.insert(0, 2, 99);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 22);
	MojTestAssert(MojMemCmp(&v3.front(), array2, 22 * sizeof(int)) == 0);
	err = v3.insert(21, 2, 88);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 24);
	MojTestAssert(MojMemCmp(&v3.front(), array3, 24 * sizeof(int)) == 0);
	err = v3.insert(7, array1, array1 + 5);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 29);
	MojTestAssert(MojMemCmp(&v3.front(), array4, 29 * sizeof(int)) == 0);
	err = v3.insert(v3.size(), 2, 22);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 31);
	MojTestAssert(MojMemCmp(&v3.front(), array5, 31 * sizeof(int)) == 0);
	// erase
	err = v3.erase(v3.size() - 2, 2);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 29);
	MojTestAssert(MojMemCmp(&v3.front(), array4, 29 * sizeof(int)) == 0);
	err = v3.erase(7, 5);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 24);
	MojTestAssert(MojMemCmp(&v3.front(), array3, 24 * sizeof(int)) == 0);
	err = v3.erase(0);
	MojTestErrCheck(err);
	MojTestAssert(v3.size() == 23);
	MojTestAssert(MojMemCmp(&v3.front(), array3 + 1, 23 * sizeof(int)) == 0);
	// equality
	v2 = v3;
	MojTestAssert(v2 == v3);
	MojTestAssert(!(v2 != v3));
	MojTestAssert(v2.compare(v3) == 0);
	err = v2.pop();
	MojTestErrCheck(err);
	MojTestAssert(v2 != v3);
	MojTestAssert(v2 < v3);
	MojTestAssert(v2 <= v3);
	MojTestAssert(v2.compare(v3) < 0);
	err = v3.pop();
	MojTestErrCheck(err);
	MojTestAssert(v2 == v3);
	err = v2.push(6);
	MojTestErrCheck(err);
	err = v3.push(5);
	MojTestErrCheck(err);
	MojTestAssert(v2 != v3);
	MojTestAssert(v2 > v3);
	MojTestAssert(v2 >= v3);
	MojTestAssert(v2.compare(v3) > 0);
	// find
	err = v1.assign(array6, array6 + 8);
	MojErrCheck(err);
	idx = v1.find(2);
	MojTestAssert(idx == MojInvalidIndex);
	idx = v1.find(1);
	MojTestAssert(idx == 0);
	idx = v1.find(1, idx + 1);
	MojTestAssert(idx == 4);
	idx = v1.find(1, idx + 1);
	MojTestAssert(idx == 5);
	idx = v1.find(1, idx + 1);
	MojTestAssert(idx == 7);
	// append
	v1.clear();
	err = v1.append(array7, array7 + 8);
	MojTestErrCheck(err);
	MojTestAssert(MojMemCmp(&v1.front(), array7, 8 * sizeof(int)) == 0);
	err = v1.append(array7, array7 + 8);
	MojTestErrCheck(err);
	MojTestAssert(MojMemCmp(&v1.front(), array8, 16 * sizeof(int)) == 0);
	// reverse
	v1.clear();
	err = v1.append(array7, array7 + 8);
	MojTestErrCheck(err);
	err = v1.reverse();
	MojTestErrCheck(err);
	MojTestAssert(MojMemCmp(&v1.front(), array9, 8 * sizeof(int)) == 0);
	// sort
	err = v1.sort();
	MojTestErrCheck(err);
	MojTestAssert(MojMemCmp(&v1.front(), array7, 8 * sizeof(int)) == 0);
	err = v1.sort();
	MojTestErrCheck(err);
	MojTestAssert(MojMemCmp(&v1.front(), array7, 8 * sizeof(int)) == 0);

	return MojErrNone;
}

MojErr MojVectorTest::stringTest()
{
	MojVector<MojString> v1;
	MojVector<MojString> v2(v1);
	MojVector<MojString> v3;
	MojString str1;
	MojString str2;

	// empty test
	MojTestAssert(v1.size() == 0);
	MojTestAssert(v1.capacity() == 0);
	MojTestAssert(v1.empty());
	MojTestAssert(v1.begin() == v1.end());
	v1.clear();
	v1.swap(v1);
	v1.swap(v2);
	v1.assign(v1);
	v1.assign(v2);
	v1 = v1;
	v1 = v2;

	// size, capacity, empty, at, front, back
	MojErr err = str1.assign(_T("0"));
	MojTestErrCheck(err);
	err = str2.assign(_T("1"));
	MojTestErrCheck(err);
	err = v1.push(str2);
	MojTestErrCheck(err);
	MojTestAssert(v1.size() == 1);
	MojTestAssert(v1.capacity() >= 1);
	MojTestAssert(!v1.empty());
	MojTestAssert(v1.at(0) == str2);
	MojTestAssert(v1.front() == str2);
	MojTestAssert(v1.back() == str2);
	v2 = v1;
	MojTestAssert(v2.size() == 1);
	MojTestAssert(v2.capacity() >= 1);
	MojTestAssert(v2.at(0) == str2);
	MojTestAssert(v1.begin() == v2.begin());
	err = v1.setAt(0, str1);
	MojTestErrCheck(err);
	MojTestAssert(v1.at(0) == str1);
	MojTestAssert(v1.front() == str1);
	MojTestAssert(v1.capacity() >= 1);
	MojTestAssert(v2.at(0) == str2);
	MojTestAssert(v1.begin() != v2.begin());
	// swap
	v1.swap(v2);
	MojTestAssert(v1.at(0) == str2);
	MojTestAssert(v2.at(0) == str1);
	// assign
	v1.assign(v2);
	MojTestAssert(v1.at(0) == str1);
	MojTestAssert(v2.at(0) == str1);
	// clear
	v1.clear();
	MojTestAssert(v1.empty() && v1.size() == 0);
	// reserve
	err = v1.reserve(50);
	MojTestErrCheck(err);
	MojTestAssert(v1.empty() && v1.size() == 0);
	MojTestAssert(v1.capacity() == 50);
	// push, pop
	for (int i = 0; i < 500; i++) {
		err = str1.format(_T("%d"), i);
		MojTestErrCheck(err);
		err = v1.push(str1);
		MojTestErrCheck(err);
		MojTestAssert(v1.back() == str1);
	}
	MojTestAssert(v1.size() == 500);
	v2 = v1;
	for (int i = 499; i >= 0; --i) {
		err = str1.format(_T("%d"), i);
		MojTestErrCheck(err);
		MojTestAssert(v1.back() == str1);
		err = v1.pop();
		MojTestErrCheck(err);
	}
	MojTestAssert(v1.empty());
	MojTestAssert(v2.size() == 500);
	// resize
	err = v2.resize(200);
	MojTestErrCheck(err);
	MojTestAssert(v2.size() == 200);
	err = str2.assign(_T("howdy"));
	MojTestErrCheck(err);
	err = v2.resize(300, str2);
	MojTestErrCheck(err);
	MojTestAssert(v2.size() == 300);
	MojTestAssert(v2.at(200) == str2 && v2.back() == str2);
	v3 = v2;
	err = v2.resize(0);
	MojTestErrCheck(err);
	MojTestAssert(v2.empty());
	// insert
	err = v1.insert(0, v3.begin(), v3.begin() + 8);
	MojTestErrCheck(err);
	MojTestAssert(v1.size() == 8);
	for (gsize i = 0; i < v1.size(); ++i) {
		MojTestAssert(v1.at(i) == v3.at(i));
	}
	// erase
	err = v1.erase(0);
	MojTestErrCheck(err);
	for (gsize i = 0; i < v1.size(); ++i) {
		MojTestAssert(v1.at(i) == v3.at(i + 1));
	}

	return MojErrNone;
}

MojErr MojVectorTest::objTest()
{
	MojVector<MojVectorTestObj> v1;
	MojVector<MojVectorTestObj> v2;

	MojErr err = v1.insert(0, 100, MojVectorTestObj());
	MojTestErrCheck(err);
	err = v1.erase(6);
	MojTestErrCheck(err);
	v1 = v2;
	err = v2.push(MojVectorTestObj());
	MojTestErrCheck(err);
	{
		MojVector<MojVectorTestObj> v3(v2);
		v2.clear();
	}
	MojTestAssert(s_defaultConstructCount + s_copyConstructCount == s_destructCount);

	return MojErrNone;
}
