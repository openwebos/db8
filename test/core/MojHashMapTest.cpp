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

/**
****************************************************************************************************
* Filename              : MojHashMapTest.cpp
* Description           : Source file for MojHashMap test.
****************************************************************************************************
**/

#include "MojHashMapTest.h"
#include "core/MojVector.h"

MojHashMapTest::MojHashMapTest()
: MojTestCase(_T("MojHashMap"))
{
}
/**
****************************************************************************************************
* @run              1. MojHashMap stores data in key-value pair. HashMap internally use Hashing
                       algorithm to store the data.
                    2. Hashing Algorithm:
                        * Key is given to the Hashing algorithm to generate the Hash_Key
                        * Hash_Key % No_Of_Buckets gives id of the bucket.
                        * Node having the information of key and value is inserted into the
                          corresponding bucket.
                       For all Insertion,Retrieval and Deletion, same algorithm is used to determine the
                       bucket id and to perform the required operation. Key and value can be of any
                       datatype.
                    3. Every key is mapped to only one value. Inserting a different value on the
                       existing key means modifying the previous value.
                       Ex: put(1,100) ==> key = 1 and value = 100
                           put(1,200) ==> key = 1 and value = 200 (100 is overwritten with 200)
                    4. MojHash provides utility functions like put/get/del/find for performing
                       various operations on HashMap.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
MojErr MojHashMapTest::run()
{
	MojHashMap<int, int> map1, map2;
	MojHashMap<int, int>::Iterator i1, i2;
	MojHashMap<int, int>::ConstIterator ci1, ci2;
	MojHashMap<MojString, int> map3;
	MojHashMap<MojString, int> map4(map3);
	MojHashMap<const MojChar*, int> map5;
	MojHashMap<MojString, int, const MojChar*> map7;
	MojString str1;
	int val1, count;
	bool found;

	// empty tests
	MojTestAssert(map1.size() == 0);
	MojTestAssert(map1.empty());
	MojTestAssert(map1.begin() == map1.end());
	map1.clear();
	map1.swap(map1);
	map1.swap(map2);
	map1.assign(map1);
	map1.assign(map2);
	map1 = map1;
	map1 = map2;
	MojErr err = map1.begin(i1);
	MojTestErrCheck(err);
	err = map1.end(i2);
	MojTestErrCheck(err);
	MojTestAssert(i1 == i2);
	MojTestAssert(!map1.contains(5));
	MojTestAssert(!map1.get(1, val1));
	err = map1.del(1, found);
	MojTestErrCheck(err);
	MojTestAssert(!found);
	MojTestAssert(map1.find(1) == map1.end());
	err = map1.find(1, i1);
	MojTestErrCheck(err);
	MojTestAssert(i1 == map1.end());

	// put/get/find/del
	for (int i = 0; i < 100; ++i) {
		MojTestAssert(!map1.contains(i));
		err = map1.put(i, i);
		MojTestErrCheck(err);
		MojTestAssert(map1.contains(i));
		MojTestAssert(map1.get(i, val1));
		MojTestAssert(val1 == i);
		ci1 = map1.find(i);
		MojTestAssert(ci1 != map1.end());
		MojTestAssert(ci1.key() == i);
		MojTestAssert(ci1.value() == i);
		err = map1.find(i, i1);
		MojTestErrCheck(err);
		MojTestAssert(i1 != map1.end());
		MojTestAssert(i1.key() == i);
		MojTestAssert(i1.value() == i);
		err = map1.put(i, -i);
		MojTestErrCheck(err);
		ci1 = map1.find(i);
		MojTestAssert(ci1 != map1.end());
		MojTestAssert(ci1.key() == i);
		MojTestAssert(ci1.value() == -i);
		err = map1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		MojTestAssert(!map1.contains(i));
		err = map1.del(i, found);
		MojTestErrCheck(err);
		MojTestAssert(!found);
		err = map1.put(i, i * 2);
		MojTestErrCheck(err);
		err = map1.find(i, i1);
		MojTestErrCheck(err);
		err = map1.del(i1.key(), found);
		MojTestErrCheck(err);
		MojTestAssert(!map1.contains(i));
		err = map1.put(i, i);
		MojTestErrCheck(err);
		MojTestAssert(map1.size() == (MojSize) i + 1);
		err = check(map1, i + 1, count);
		MojTestErrCheck(err);
		MojTestAssert(count == i + 1);
	}

	// iterate from find
	ci1 = map1.find(28);
	MojTestAssert(ci1 != map1.end());
	while (ci1 != map1.end())
		++ci1;

	// delete while iterating
	map1.swap(map2);
	err = map2.begin(i1);
	MojTestErrCheck(err);
	while (i1 != map2.end()) {
		err = map2.del(i1++.key(), found);
		MojTestErrCheck(err);
		--count;
	}

	// assign/refcounting
	MojTestAssert(count == 0);
	MojTestAssert(map1.empty());
	MojTestAssert(map2.empty());
	MojTestAssert(!map2.contains(4));
	err = map1.put(4, 3);
	MojTestErrCheck(err);
	map2.assign(map1);
	MojTestAssert(map1.size() == map2.size());
	ci1 = map1.find(4);
	ci2 = map2.find(4);
	MojTestAssert(ci1 != map1.end() && ci2 != map2.end());
	MojTestAssert(&ci1.value() == &ci2.value());
	err = map1.put(5, 6);
	MojTestErrCheck(err);
	MojTestAssert(map1.contains(5));
	MojTestAssert(!map2.contains(5));

	// strings
	err = str1.assign(_T("hello"));
	MojTestErrCheck(err);
	err = map3.put(str1, 8);
	MojTestErrCheck(err);
	MojTestAssert(map3.get(str1, val1));
	MojTestAssert(val1 == 8);
	err = map5.put(_T("howdy"), 27);
	MojTestErrCheck(err);
	MojTestAssert(map5.get(_T("howdy"), val1));
	MojTestAssert(val1 == 27);
	err = map7.put(str1, 89);
	MojTestErrCheck(err);
	MojTestAssert(map7.get(str1, val1));
	MojTestAssert(val1 == 89);
	MojTestAssert(map7.get(_T("hello"), val1));
	MojTestAssert(val1 == 89);

	return MojErrNone;
}

MojErr MojHashMapTest::check(MojHashMap<int, int> map, int maxExpected, int& numFound)
{
	MojVector<bool> v;

	numFound = 0;
	MojErr err = v.resize(maxExpected, false);
	MojTestErrCheck(err);
	for (MojHashMap<int, int>::ConstIterator i = map.begin(); i != map.end(); ++i) {
		int key = i.key();
		int val = i.value();
		MojTestAssert(key == val);
		MojTestAssert(key >= 0 && key <= maxExpected);
		MojTestAssert(!v.at(key));
		err = v.setAt(key, true);
		MojTestErrCheck(err);
		++numFound;
	}

	return MojErrNone;
}
