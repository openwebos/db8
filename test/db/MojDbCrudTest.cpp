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


#include "MojDbCrudTest.h"
#include "db/MojDb.h"
#include "core/MojObject.h"

static const MojChar* const MojKindStr =
	_T("{\"id\":\"Test:1\", \"owner\":\"mojodb.admin\"}");
static const MojChar* const MojTestObjStr =
	_T("{\"_kind\":\"Test:1\",\"a\":\"hello\",\"b\":null,\"c\":{\"d\":true,\"e\":false,\"f\":[86,75.4,[],{},\"foo\"]}}");
static const MojChar* const MojKindStrDefaultValues =
	_T("{\"id\":\"TestDefaultValues:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foobar\",\"props\":[{\"name\":\"foo\", \"default\":100},{\"name\":\"bar\"}]}]}");
static const MojChar* const MojKindStrNoDefaults =
	_T("{\"id\":\"TestNoDefaults:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foobar\",\"props\":[{\"name\":\"foo\"},{\"name\":\"bar\"}]}]}");
static const MojChar* const MojKindStrNestedDefaults =
	_T("{\"id\":\"TestNestedDefaults:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"bazfoobar\",\"props\":[{\"name\":\"baz\", \"default\":1}, {\"name\":\"foo.bar\", \"default\":100}]}]}");
static const MojChar* const MojKindStrArrayDefaults =
	_T("{\"id\":\"TestArrayDefaults:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"baz\",\"props\":[{\"name\":\"baz\", \"default\":[4,6,8]}]}]}");
/*static const MojChar* const MojTestObjNullCharStr =
	_T("{\"_kind\":\"Test:1\",\"contactBackupHash\":\"'aa3'**testLinkHash\",\"defaultPropertyHashes\":[{\"value\":")
	_T("\"\\u00d4\\u001d\\u008c\\u00d9\\u008f\\u0000\\u00b2\\u0004\\u00e9\\u0080\t\\u0098\\u00ec\\u00f8B~\",\"type\":\"PhoneNumber\"}]}");*/

MojDbCrudTest::MojDbCrudTest()
: MojTestCase(_T("MojDbCrud"))
{
}

MojErr MojDbCrudTest::run()
{
	MojDb db;

	// open
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	// add type
	MojObject obj;
	err = obj.fromJson(MojKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	// db lock
	err = lockTest();
	MojTestErrCheck(err);
	// simple test
	err = simpleTest(db);
	MojTestErrCheck(err);
	// big test
	err = bigTest(db);
	MojTestErrCheck(err);
	// merge test
	err = mergeTest(db);
	MojTestErrCheck(err);
	// merge array test
	err = mergeArrayTest(db);
	MojTestErrCheck(err);
	// array test
	err = arrayTest(db);
	MojTestErrCheck(err);
	// default values test
	err = defaultValuesTest(db);
	MojTestErrCheck(err);
	// stale update test
	err = staleUpdateTest(db);
	MojTestErrCheck(err);
	// large object test
	err = largeObjectTest(db);
	MojTestErrCheck(err);
	//err = objectWithNullCharTest(db);
	//MojTestErrCheck(err);
	// drop
	err = db.drop(MojDbTestDir);
	MojTestErrCheck(err);
	// persistence
	err = persistenceTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCrudTest::lockTest()
{
	MojDb db;

	MojErr err = db.open(MojDbTestDir);
	MojTestErrExpected(err, MojErrLocked);

	return MojErrNone;
}

MojErr MojDbCrudTest::simpleTest(MojDb& db)
{
	MojObject obj1;
	MojObject obj2;
	MojObject id1;
	MojObject id2;
	MojString val;
	bool found = false;

	// put
	MojErr err = obj1.putString(_T("hello"), _T("world"));
	MojTestErrCheck(err);
	err = obj1.putString(_T("_kind"), _T("Test:1"));
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj1.get(MojDb::IdKey, id1));
	err = obj1.del(MojDb::IdKey, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.put(obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj1.get(MojDb::IdKey, id2));
	MojTestAssert(id1 != id2);
	// get
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = obj2.get(_T("hello"), val, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(val == _T("world"));
	// del
	err = db.del(id1, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	bool deleted = false;
	MojTestAssert(found && obj2.get(MojDb::DelKey, deleted) && deleted);
	err = db.del(id1, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	deleted = false;
	MojTestAssert(found && obj2.get(MojDb::DelKey, deleted) && deleted);
	err = db.del(id1, found, MojDb::FlagPurge);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	MojTestAssert(!found);
	err = db.del(id1, found, MojDb::FlagPurge);
	MojTestErrCheck(err);
	MojTestAssert(!found);
	// reserveId
	MojObject idReserved;
	err = db.reserveId(idReserved);
	MojTestErrCheck(err);
	MojTestAssert(!idReserved.null());

	return MojErrNone;
}

MojErr MojDbCrudTest::bigTest(MojDb& db)
{
	MojObject obj1;
	MojObject obj2;
	MojObject objOrig;
	MojObject id1;
	MojString val;
	const int numObjects = 10000;
	bool found = false;

	MojErr err = objOrig.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	obj1 = objOrig;

	for (int i = 0; i < numObjects; ++i) {
		MojTestErrCheck(err);
		err = db.put(obj1);
		MojTestErrCheck(err);
		MojTestAssert(obj1.get(MojDb::IdKey, id1));
		err = db.get(id1, obj2, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = obj2.del(MojDb::RevKey, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		err = obj2.del(MojDb::IdKey, found);
		MojTestErrCheck(err);
		MojTestAssert(found);

		//delete the id that was assigned to obj2's array item
		MojObject::Iterator objIter;
		err = obj2.begin(objIter);
		MojTestErrCheck(err);
		for(; objIter != obj2.end(); ++objIter) {
			MojObject& value = objIter.value();
			if (objIter.key() == _T("c")) {
				MojObject::Iterator nestedIter;
				err = value.begin(nestedIter);
				MojTestErrCheck(err);
				for(; nestedIter != value.end(); ++nestedIter) {
					if (nestedIter.key() == _T("f")) {
						MojObject::ArrayIterator iter;
						MojObject& f = nestedIter.value();
						err = f.arrayBegin(iter);
						MojTestErrCheck(err);
						for (; iter != f.arrayEnd(); ++iter) {
							if (iter->type() == MojObject::TypeObject) {
								err = iter->del(MojDb::IdKey, found);
								MojTestErrCheck(err);
								MojTestAssert(found);
							}
						}
					}
				}
			}
		}

		MojTestAssert(obj2 == objOrig);

	}

	return MojErrNone;
}

MojErr MojDbCrudTest::mergeTest(MojDb& db)
{
	MojObject obj1;
	MojObject obj2;
	MojObject obj3;
	MojObject id1;
	MojString val;
	const MojChar* const json1 = _T("{\"_id\": \"merge1\", \"_kind\":\"Test:1\",\"f1\": 8, \"f2\": \"hello\", \"f3\": {\"n1\": 9}, \"f4\": {\"w\": false}}");
	const MojChar* const json2 = _T("{\"_id\": \"merge1\", \"f2\": \"bye\", \"f3\": {\"n2\": 99}, \"f4\": 6}");
	const MojChar* const json3 = _T("{\"_id\": \"merge1\", \"_kind\":\"Test:1\",\"f1\": 8, \"f2\": \"bye\", \"f3\": {\"n1\": 9, \"n2\": 99}, \"f4\": 6}");

	MojString str;
	MojErr err = str.assign(_T("merge1"));
	MojTestErrCheck(err);
	id1 = str;

	err = obj1.fromJson(json1);
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	err = obj1.fromJson(json2);
	MojTestErrCheck(err);
	err = db.merge(obj1);
	MojTestErrCheck(err);
	bool found = false;
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = obj3.fromJson(json3);
	MojTestErrCheck(err);
	err = obj2.del(_T("_rev"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(obj2 == obj3);

	// check that merging undeletes an object
	err = db.del(id1, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = obj1.fromJson(json1);
	MojTestErrCheck(err);
	err = db.merge(obj1);
	MojTestErrCheck(err);
	err = db.get(id1, obj1, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(!obj2.contains(MojDb::DelKey));

	return MojErrNone;
}

MojErr MojDbCrudTest::mergeArrayTest(MojDb& db)
{
	MojObject obj1;
	MojObject obj2;
	MojObject obj3;
	MojObject obj4;
	MojObject obj5;
	MojObject obj6;
	MojObject obj7;
	MojObject obj8;
	MojObject obj9;
	MojObject obj10;
	MojObject obj11;
	MojObject obj12;
	MojObject id1;
	MojObject id2;
	MojObject id3;
	const MojChar* const baseObj = _T("{\"_id\":\"arrayMerge1\",\"_kind\":\"Test:1\",\"array\":[1, 2, \"hi\", {\"a\":17}, {\"b\":\"test\"}, false, null]}");
	const MojChar* const baseObj2 = _T("{\"_id\":\"arrayMerge2\",\"_kind\":\"Test:1\",\"array\":[1, 2, [3,4,5]]}");
	const MojChar* const mergeObj = _T("{\"_id\":\"arrayMerge2\",\"_kind\":\"Test:1\",\"array\":[1, 2, [4,5,6]]}");
	const MojChar* const baseObj3 = _T("{\"_id\":\"objMerge\",\"_kind\":\"Test:1\",\"subObject\":{\"a\":\"hello\"}}");

	MojString str;
	MojErr err = str.assign(_T("arrayMerge1"));
	MojErrCheck(err);
	id1 = str;

	err = str.assign(_T("arrayMerge2"));
	MojErrCheck(err);
	id2 = str;

	err = str.assign(_T("objMerge"));
	MojErrCheck(err);
	id3 = str;

	err = obj1.fromJson(baseObj);
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	bool found = false;
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojObject array;
	found = obj2.get(_T("array"), array);
	MojTestAssert(found);

	MojObject aId;
	MojObject bId;
	MojSize count = 0;
	for (MojObject::ConstArrayIterator i = array.arrayBegin(); i != array.arrayEnd(); ++i, ++count) {
		if (i->type() == MojObject::TypeObject) {
			MojObject item;
			if (i->get(_T("a"), item)) {
				err = i->getRequired(MojDb::IdKey, aId);
				MojErrCheck(err);
			} else if (i->get(_T("b"), item)) {
				err = i->getRequired(MojDb::IdKey, bId);
				MojErrCheck(err);
			}
		}
	}
	MojTestAssert(!aId.undefined());
	MojTestAssert(!bId.undefined());
	MojTestAssert(count == 7);

	// try to merge the same object - with a and b's ids the same
	err = db.merge(obj1);
	MojTestErrCheck(err);

	err = db.get(id1, obj3, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj3.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	MojTestAssert(count == 7);

	// merge an object with missing array ids - generates dupes
	err = obj1.fromJson(baseObj);
	MojTestErrCheck(err);
	err = db.merge(obj1);
	MojTestErrCheck(err);

	err = db.get(id1, obj4, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj4.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	//MojTestAssert(count == 9); 2.3.10 - changing behavior of array merges to not be additive
	MojTestAssert(count == 7);

	// merge just the array subobjects
	MojObject array5;
	MojObject a;
	err = a.put(MojDb::IdKey, aId);
	MojTestErrCheck(err);
	err = a.put(_T("a"), 25);
	MojTestErrCheck(err);
	MojObject b;
	err = b.put(MojDb::IdKey, bId);
	MojTestErrCheck(err);
	err = b.putString(_T("b"), _T("test2"));
	MojTestErrCheck(err);
	err = b.putString(_T("c"), _T("new prop"));
	MojTestErrCheck(err);
	MojObject c;
	err = c.putString(_T("c"), _T("new array item"));
	MojTestErrCheck(err);
	err = array5.push(a);
	MojTestErrCheck(err);
	err = array5.push(b);
	MojTestErrCheck(err);
	err = array5.push(c);
	MojTestErrCheck(err);
	err = obj5.put(_T("array"), array5);
	MojTestErrCheck(err);
	err = obj5.putString(_T("_kind"), _T("Test:1"));
	MojTestErrCheck(err);
	err = obj5.putString(_T("_id"), _T("arrayMerge1"));
	MojTestErrCheck(err);

	err = db.merge(obj5);
	MojTestErrCheck(err);

	err = db.get(id1, obj6, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj6.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	//MojTestAssert(count == 10); 2.3.10 - changing behavior of array merges to not be additive
	MojTestAssert(count == 3);

	// merge an array subobject with fewer properties than are in the db
	// and make sure the merge is correct
	MojObject array6;
	err = b.del(_T("b"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = b.del(_T("c"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = b.put(_T("d"), 15);
	MojTestErrCheck(err);
	err = array6.push(b);
	MojTestErrCheck(err);
	err = obj5.put(_T("array"), array6);
	MojTestErrCheck(err);

	err = db.merge(obj5);
	MojTestErrCheck(err);
	err = db.get(id1, obj6, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj6.get(_T("array"), array);
	MojTestAssert(found);
	MojString bStr;
	err = bStr.assign(_T("test2"));
	MojTestErrCheck(err);
	MojString cStr;
	err = cStr.assign(_T("new prop"));
	MojTestErrCheck(err);
	count = 0;
	MojObject::ConstArrayIterator arrayEnd = array.arrayEnd();
	for (MojObject::ConstArrayIterator i = array.arrayBegin(); i != arrayEnd; ++i, ++count) {
		if (i->type() == MojObject::TypeObject) {
			MojObject id;
			err = i->getRequired(MojDb::IdKey, id);
			MojTestErrCheck(err);
			if (id == bId) {
				MojObject key;
				err = i->getRequired(_T("d"), key);
				MojTestErrCheck(err);
				MojTestAssert(key == 15);
				bool found = i->get(_T("b"), key);
				//MojTestAssert(key == bStr);
				MojTestAssert(!found);
				found = i->get(_T("c"), key);
				//MojTestAssert(key == cStr);
				MojTestAssert(!found);
			}
		}
	}
	MojTestAssert(count == 1);

	err = obj7.fromJson(baseObj2);
	MojTestErrCheck(err);
	err = db.put(obj7);
	MojTestErrCheck(err);
	err = db.get(id2, obj8, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj8.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	MojTestAssert(count == 3);

	err = obj7.fromJson(baseObj2);
	MojTestErrCheck(err);
	err = db.merge(obj7);
	MojTestErrCheck(err);
	err = db.get(id2, obj9, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj9.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	MojTestAssert(count == 3);

	err = obj7.fromJson(mergeObj);
	MojTestErrCheck(err);
	err = db.merge(obj7);
	MojTestErrCheck(err);
	err = db.get(id2, obj10, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	found = obj10.get(_T("array"), array);
	MojTestAssert(found);
	count = array.size();
	//MojTestAssert(count == 4); 2.3.10 - changing behavior of array merges to not be additive
	MojTestAssert(count == 3);

	err = obj11.fromJson(baseObj3);
	MojTestErrCheck(err);
	err = db.put(obj11);
	MojTestErrCheck(err);
	err = db.get(id3, obj12, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojObject subObject;
	found = obj12.get(_T("subObject"), subObject);
	MojTestAssert(found);

	MojObject nestedId;
	found = subObject.get(MojDb::IdKey, nestedId);
	MojTestAssert(!found);
	MojTestAssert(nestedId.undefined());

	return MojErrNone;
}

MojErr MojDbCrudTest::arrayTest(MojDb& db)
{
	MojObject array;
	for (int i = 0; i < 100; ++i) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, _T("Test:1"));
		MojTestErrCheck(err);
		err = obj.put(_T("foo"), i);
		MojTestErrCheck(err);
		err = obj.put(_T("bar"), i);
		MojTestErrCheck(err);
		err = array.push(obj);
		MojTestErrCheck(err);
	}
	MojObject::ArrayIterator begin;
	MojErr err = array.arrayBegin(begin);
	MojTestErrCheck(err);
	err = db.put(begin, array.arrayEnd());
	MojTestErrCheck(err);

	MojObject mergeObj;
	err = mergeObj.put(_T("bar"), -9);
	MojTestErrCheck(err);
	MojObject array2;
	int count = 0;
	for (MojObject::ConstArrayIterator i = array.arrayBegin();
		i != array.arrayEnd();
		++i, ++count) {
		MojObject id;
		err = i->getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		MojObject obj;
		bool found = false;
		err = db.get(id, obj, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		MojTestAssert(obj == *i);
		MojObject val;
		MojTestAssert(obj.get(_T("foo"), val) && val == count);
		MojTestAssert(obj.get(_T("bar"), val) && val == count);

		err = mergeObj.put(MojDb::IdKey, id);
		MojTestErrCheck(err);
		err = array2.push(mergeObj);
		MojTestErrCheck(err);
	}
	MojTestAssert(count == 100);

	err = array2.arrayBegin(begin);
	MojTestErrCheck(err);
	err = db.merge(begin, array2.arrayEnd());
	MojTestErrCheck(err);

	count = 0;
	for (MojObject::ConstArrayIterator i = array.arrayBegin();
		i != array.arrayEnd();
		++i, ++count) {
		MojObject id;
		err = i->getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		MojObject obj;
		bool found = false;
		err = db.get(id, obj, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
		MojObject val;
		MojTestAssert(obj.get(_T("foo"), val) && val == count);
		MojTestAssert(obj.get(_T("bar"), val) && val == -9);
	}
	MojTestAssert(count == 100);

	return MojErrNone;
}

MojErr MojDbCrudTest::defaultValuesTest(MojDb& db)
{
	// add new kinds
	MojObject obj;
	MojErr err = obj.fromJson(MojKindStrDefaultValues);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	MojObject obj2;
	err = obj2.fromJson(MojKindStrNoDefaults);
	MojTestErrCheck(err);
	err = db.putKind(obj2);
	MojTestErrCheck(err);

	MojObject obj3;
	err = obj3.fromJson(MojKindStrNestedDefaults);
	MojTestErrCheck(err);
	err = db.putKind(obj3);
	MojTestErrCheck(err);

	MojObject obj4;
	err = obj4.fromJson(MojKindStrArrayDefaults);
	MojTestErrCheck(err);
	err = db.putKind(obj4);
	MojTestErrCheck(err);

	MojString kind1Id;
	err = obj.getRequired(_T("id"), kind1Id);
	MojTestErrCheck(err);
	MojString kind2Id;
	err = obj2.getRequired(_T("id"), kind2Id);
	MojTestErrCheck(err);
	MojString kind3Id;
	err = obj3.getRequired(_T("id"), kind3Id);
	MojTestErrCheck(err);
	MojString kind4Id;
	err = obj4.getRequired(_T("id"), kind4Id);
	MojTestErrCheck(err);

	// put an object with only bar populated, shouldn't be found
	MojObject test;
	err = test.putString(_T("_kind"), kind2Id);
	MojTestErrCheck(err);
	err = test.putInt(_T("bar"), 400);
	MojTestErrCheck(err);
	err = db.put(test);
	MojTestErrCheck(err);

	MojDbQuery q;
	err = q.from(kind2Id);
	MojTestErrCheck(err);
	err = q.where(_T("foo"), MojDbQuery::OpEq, 100);
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(q, cursor);
	MojTestErrCheck(err);

	int count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 0);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an object with only foo populated, shouldn't be found
	MojObject test2;
	err = test2.putString(_T("_kind"), kind2Id);
	MojTestErrCheck(err);
	err = test2.putInt(_T("foo"), 100);
	MojTestErrCheck(err);
	err = db.put(test2);
	MojTestErrCheck(err);

	err = db.find(q, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 0);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an object of default kind with only bar populated.  Should be found
	MojObject test3;
	err = test3.putString(_T("_kind"), kind1Id);
	MojTestErrCheck(err);
	err = test3.putInt(_T("bar"), 400);
	MojTestErrCheck(err);
	err = db.put(test3);
	MojTestErrCheck(err);

	MojDbQuery q2;
	err = q2.from(kind1Id);
	MojTestErrCheck(err);
	err = q2.where(_T("foo"), MojDbQuery::OpEq, 100);
	MojTestErrCheck(err);

	err = db.find(q2, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an object of default kind with only foo populated.  Should not be found
	MojObject test4;
	err = test4.putString(_T("_kind"), kind1Id);
	MojTestErrCheck(err);
	err = test4.putInt(_T("foo"), 100);
	MojTestErrCheck(err);
	err = db.put(test4);
	MojTestErrCheck(err);

	err = db.find(q2, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an object of default kind with neither foo nor bar populated.  Should not be found
	MojObject test5;
	err = test5.putString(_T("_kind"), kind1Id);
	MojTestErrCheck(err);
	err = test5.putInt(_T("baz"), 300);
	MojTestErrCheck(err);
	err = db.put(test5);
	MojTestErrCheck(err);

	err = db.find(q2, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an object of default kind with foo set to a value other than the default. Should not be found
	MojObject test6;
	err = test6.putString(_T("_kind"), kind1Id);
	MojTestErrCheck(err);
	err = test6.putInt(_T("foo"), 300);
	MojTestErrCheck(err);
	err = db.put(test6);
	MojTestErrCheck(err);

	err = db.find(q2, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put a nested kind object with no defaults set. Should be found
	MojObject test7;
	err = test7.putString(_T("_kind"), kind3Id);
	MojTestErrCheck(err);
	err = db.put(test7);
	MojTestErrCheck(err);

	MojDbQuery q3;
	err = q3.from(kind3Id);
	MojTestErrCheck(err);
	err = q3.where(_T("baz"), MojDbQuery::OpEq, 1);
	MojTestErrCheck(err);
	err = q3.where(_T("foo.bar"), MojDbQuery::OpEq, 100);
	MojTestErrCheck(err);

	err = db.find(q3, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put a nested kind object with only foo set, not bar. Should be found
	MojObject test8;
	err = test8.putString(_T("_kind"), kind3Id);
	MojTestErrCheck(err);
	err = test8.putString(_T("foo"), _T("a"));
	err = db.put(test8);
	MojTestErrCheck(err);

	err = db.find(q3, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 2);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an array kind object with no default set. Should be found
	MojObject test9;
	err = test9.putString(_T("_kind"), kind4Id);
	MojTestErrCheck(err);
	err = db.put(test9);
	MojTestErrCheck(err);

	MojDbQuery q4;
	err = q4.from(kind4Id);
	MojTestErrCheck(err);
	MojObject array;
	err = array.fromJson(_T("[4,6,8]"));
	MojTestErrCheck(err);
	err = q4.where(_T("baz"), MojDbQuery::OpEq, array);
	MojTestErrCheck(err);

	err = db.find(q4, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 3);
	err = cursor.close();
	MojTestErrCheck(err);

	// put an array kind object with baz set to a different value. Should NOT be found
	MojObject test10;
	err = test10.putString(_T("_kind"), kind4Id);
	MojTestErrCheck(err);
	err = test10.putInt(_T("baz"), 1);
	MojTestErrCheck(err);
	err = db.put(test10);
	MojTestErrCheck(err);

	err = db.find(q4, cursor);
	MojTestErrCheck(err);

	count = 0;
	for (;;) {
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		count++;
	}
	MojTestAssert(count == 3);
	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCrudTest::persistenceTest()
{
	// open
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	// add type
	MojObject kind;
	err = kind.fromJson(MojKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);

	// put
	MojObject obj1;
	err = obj1.putString(_T("hello"), _T("world"));
	MojTestErrCheck(err);
	err = obj1.putString(_T("_kind"), _T("Test:1"));
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	MojObject id1;
	MojTestAssert(obj1.get(MojDb::IdKey, id1));
	// reopen
	err = db.close();
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	// get
	MojObject obj2;
	bool found = false;
	err = db.get(id1, obj2, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojString val;
	err = obj2.get(_T("hello"), val, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(val == _T("world"));
	// close
	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCrudTest::staleUpdateTest(MojDb& db)
{
	MojObject obj;
	MojErr err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);

	// put object
	err = db.put(obj);
	MojTestErrCheck(err);

	// now object in memory has a rev
	MojUInt64 rev;
	err = obj.getRequired(MojDb::RevKey, rev);
	MojTestErrCheck(err);

	// update one key and put it again - should work
	err = obj.putString(_T("a"), "hello3");
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	MojUInt64 rev2;
	err = obj.getRequired(MojDb::RevKey, rev2);
	MojTestErrCheck(err);
	MojTestAssert(rev2 > rev);

	// update one key and merge - also should work
	err = obj.putString(_T("a"), "hello4");
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrCheck(err);

	MojUInt64 rev3;
	err = obj.getRequired(MojDb::RevKey, rev3);
	MojTestErrCheck(err);
	MojTestAssert(rev3 > rev2);

	// delete - now the rev in the db doesn't match the one in memory
	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);
	bool found;
	err = db.del(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	MojObject objFromDb;
	err = db.get(id, objFromDb, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	// after the delete, the rev in memory did not change but rev in the db did
	MojUInt64 rev4;
	err = obj.getRequired(MojDb::RevKey, rev4);
	MojTestErrCheck(err);
	MojTestAssert(rev3 == rev4);
	MojUInt64 revFromDb;
	err = objFromDb.getRequired(MojDb::RevKey, revFromDb);
	MojTestErrCheck(err);
	MojTestAssert(rev3 < revFromDb);

	// now try to put and merge - errors expected
	err = db.put(obj);
	MojTestErrExpected(err, MojErrDbRevisionMismatch);
	err = obj.putString(_T("a"), "hello5");
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrExpected(err, MojErrDbRevisionMismatch);

	// put a new object and remove the rev key, merge will work but put will fail
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	err = obj.del(MojDb::RevKey, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = obj.putString(_T("a"), "hello6");
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrExpected(err, MojErrDbRevNotSpecified);

	err = obj.putString(_T("a"), "hello7");
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCrudTest::largeObjectTest(MojDb& db)
{
	MojObject obj;
	MojErr err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);

	// stuff a lot of properties into this object
	for(int i = 0; i < MojUInt8Max + 10; i++) {
		MojString str;
		err = str.format("%d", i);
		MojTestErrCheck(err);
		err = obj.putString(str, "hello");
		MojTestErrCheck(err);
	}

	// store and retrieve
	err = db.put(obj);
	MojTestErrCheck(err);

	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);

	MojObject dbObj;
	bool found;
	err = db.get(id, dbObj, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	MojTestAssert(obj == dbObj);

	// close and re-open the db
	err = db.close();
	MojTestErrCheck(err);

	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	err = obj.putString(_T("test"), _T("val"));
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	err = db.get(id, dbObj, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	MojTestAssert(obj == dbObj);
	MojString val;
	err = dbObj.getRequired(_T("test"), val);
	MojTestErrCheck(err);
	err = dbObj.getRequired(_T("1"), val);
	MojTestErrCheck(err);

	return MojErrNone;
}

/*MojErr MojDbCrudTest::objectWithNullCharTest(MojDb& db)
{
	MojObject obj;
	MojErr err = obj.fromJson(MojTestObjNullCharStr);
	MojTestErrCheck(err);

	// store and retrieve
	err = db.put(obj);
	MojTestErrCheck(err);

	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);

	MojObject dbObj;
	bool found;
	err = db.get(id, dbObj, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	MojTestAssert(obj == dbObj);

	return MojErrNone;
}*/

void MojDbCrudTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
