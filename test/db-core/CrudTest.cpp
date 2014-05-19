/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2014 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

/**
 *  @file CrudTest.cpp
 */

#include "MojDbCoreTest.h"

using ::testing::PrintToString;

namespace {
    const MojChar* const MojKindStr =
        _T("{\"id\":\"Test:1\", \"owner\":\"mojodb.admin\"}");
    const MojChar* const MojTestObjStr =
        _T("{\"_kind\":\"Test:1\",\"a\":\"hello\",\"b\":null,\"c\":{\"d\":true,\"e\":false,\"f\":[86,75.4,[],{},\"foo\"]}}");
}

struct CrudTest : public MojDbCoreTest
{
    void SetUp()
    {
        MojDbCoreTest::SetUp();

        // add type
        MojObject obj;
        MojAssertNoErr( obj.fromJson(MojKindStr) );
        MojAssertNoErr( db.putKind(obj) );
    }
};

TEST_F(CrudTest, lock)
{
    MojDb db;

    MojExpectErr( MojErrLocked, db.open(path.c_str()) );
}

TEST_F(CrudTest, simple)
{
    MojObject obj1;
    MojObject obj2;
    MojObject id1;
    MojObject id2;
    MojString val;
    bool found = false;

    // put
    MojAssertNoErr( obj1.putString(_T("hello"), _T("world")) );
    MojAssertNoErr( obj1.putString(_T("_kind"), _T("Test:1")) );
    MojExpectNoErr( db.put(obj1) );
    EXPECT_TRUE( obj1.get(MojDb::IdKey, id1) );
    found = false;
    MojAssertNoErr( obj1.del(MojDb::IdKey, found) );
    EXPECT_TRUE( found );
    MojExpectNoErr( db.put(obj1) );
    EXPECT_TRUE( obj1.get(MojDb::IdKey, id2) );
    EXPECT_NE( id1, id2 );
    // get
    found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_TRUE( found );
    found = false;
    MojExpectNoErr( obj2.get(_T("hello"), val, found) );
    EXPECT_TRUE( found );
    EXPECT_STREQ( "world", val.data() ) << PrintToString(obj2);
    // del
    found = false;
    MojExpectNoErr( db.del(id1, found) );
    EXPECT_TRUE( found );
    found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_TRUE( found );
    bool deleted = false;
    EXPECT_TRUE( obj2.get(MojDb::DelKey, deleted) );
    EXPECT_TRUE( deleted );
    found = false;
    MojExpectNoErr( db.del(id1, found) );
    EXPECT_TRUE( found );
    found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_TRUE( found );
    deleted = false;
    EXPECT_TRUE( obj2.get(MojDb::DelKey, deleted) );
    EXPECT_TRUE( deleted );
    found = false;
    MojExpectNoErr( db.del(id1, found, MojDb::FlagPurge) );
    EXPECT_TRUE( found );
    found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_FALSE( found );
    MojExpectNoErr( db.del(id1, found, MojDb::FlagPurge) );
    EXPECT_FALSE( found );
    // reserveId
    MojObject idReserved;
    MojExpectNoErr( db.reserveId(idReserved) );
    EXPECT_FALSE( idReserved.null() );
}

TEST_F(CrudTest, big)
{
    MojObject obj1;
    MojObject obj2;
    MojObject objOrig;
    MojObject id1;
    MojString val;
    const int numObjects = 10000;
    bool found = false;

    MojAssertNoErr( objOrig.fromJson(MojTestObjStr) );
    obj1 = objOrig;

    for (int i = 0; i < numObjects; ++i) {
        MojExpectNoErr( db.put(obj1) );
        EXPECT_TRUE( obj1.get(MojDb::IdKey, id1) );
        MojExpectNoErr( db.get(id1, obj2, found) );
        EXPECT_TRUE( found );
        MojExpectNoErr( obj2.del(MojDb::RevKey, found) );
        EXPECT_TRUE( found );
        MojExpectNoErr( obj2.del(MojDb::IdKey, found) );
        EXPECT_TRUE( found );

        //delete the id that was assigned to obj2's array item
        MojObject::Iterator objIter;
        MojExpectNoErr( obj2.begin(objIter) );
        for(; objIter != obj2.end(); ++objIter) {
            MojObject& value = objIter.value();
            if (objIter.key() == _T("c")) {
                MojObject::Iterator nestedIter;
                MojExpectNoErr( value.begin(nestedIter) );
                for(; nestedIter != value.end(); ++nestedIter) {
                    if (nestedIter.key() == _T("f")) {
                        MojObject::ArrayIterator iter;
                        MojObject& f = nestedIter.value();
                        MojExpectNoErr( f.arrayBegin(iter) );
                        for (; iter != f.arrayEnd(); ++iter) {
                            if (iter->type() == MojObject::TypeObject) {
                                MojExpectNoErr( iter->del(MojDb::IdKey, found) );
                                EXPECT_TRUE( found );
                            }
                        }
                    }
                }
            }
        }

        EXPECT_EQ( objOrig, obj2 );
    }
}

TEST_F(CrudTest, merge)
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
    MojAssertNoErr( str.assign(_T("merge1")) );
    id1 = str;

    MojAssertNoErr( obj1.fromJson(json1) );
    MojExpectNoErr( db.put(obj1) );
    MojAssertNoErr( obj1.fromJson(json2) );
    MojExpectNoErr( db.merge(obj1) );
    bool found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_TRUE( found );
    MojAssertNoErr( obj3.fromJson(json3) );
    MojExpectNoErr( obj2.del(_T("_rev"), found) );
    EXPECT_TRUE( found );
    EXPECT_EQ( obj3, obj2 );

    // check that merging undeletes an object
    MojExpectNoErr( db.del(id1, found) );
    EXPECT_TRUE( found );
    MojAssertNoErr( obj1.fromJson(json1) );
    MojExpectNoErr( db.merge(obj1) );
    MojExpectNoErr( db.get(id1, obj1, found) );
    EXPECT_TRUE( found );
    EXPECT_FALSE( obj2.contains(MojDb::DelKey) );
}

TEST_F(CrudTest, mergeArray)
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
	MojAssertNoErr( str.assign(_T("arrayMerge1")) );
	id1 = str;

	MojAssertNoErr( str.assign(_T("arrayMerge2")) );
	id2 = str;

	MojAssertNoErr( str.assign(_T("objMerge")) );
	id3 = str;

	MojAssertNoErr( obj1.fromJson(baseObj) );
	MojExpectNoErr( db.put(obj1) );
	bool found = false;
	MojExpectNoErr( db.get(id1, obj2, found) );
	EXPECT_TRUE( found );
	MojObject array;
	EXPECT_TRUE( obj2.get(_T("array"), array) );

	MojObject aId;
	MojObject bId;
	MojSize count = 0;
	for (MojObject::ConstArrayIterator i = array.arrayBegin(); i != array.arrayEnd(); ++i, ++count) {
		if (i->type() == MojObject::TypeObject) {
			MojObject item;
			if (i->get(_T("a"), item)) {
				MojExpectNoErr( i->getRequired(MojDb::IdKey, aId) );
			} else if (i->get(_T("b"), item)) {
				MojExpectNoErr( i->getRequired(MojDb::IdKey, bId) );
			}
		}
	}
	EXPECT_FALSE( aId.undefined() );
	EXPECT_FALSE( bId.undefined() );
	EXPECT_EQ( 7ul, count );

	// try to merge the same object - with a and b's ids the same
	MojExpectNoErr( db.merge(obj1) );

	MojExpectNoErr( db.get(id1, obj3, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj3.get(_T("array"), array) );
	EXPECT_EQ( 7ul, array.size() );

	// merge an object with missing array ids - generates dupes
	MojAssertNoErr( obj1.fromJson(baseObj) );
	MojExpectNoErr( db.merge(obj1) );

	MojExpectNoErr( db.get(id1, obj4, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj4.get(_T("array"), array) );
	//MojTestAssert(count == 9); 2.3.10 - changing behavior of array merges to not be additive
	EXPECT_EQ( 7ul, array.size() );

	// merge just the array subobjects
	MojObject array5;
	MojObject a;
	MojAssertNoErr( a.put(MojDb::IdKey, aId) );
	MojAssertNoErr( a.put(_T("a"), 25) );
	MojObject b;
	MojAssertNoErr( b.put(MojDb::IdKey, bId) );
	MojAssertNoErr( b.putString(_T("b"), _T("test2")) );
	MojAssertNoErr( b.putString(_T("c"), _T("new prop")) );
	MojObject c;
	MojAssertNoErr( c.putString(_T("c"), _T("new array item")) );
	MojAssertNoErr( array5.push(a) );
	MojAssertNoErr( array5.push(b) );
	MojAssertNoErr( array5.push(c) );
	MojAssertNoErr( obj5.put(_T("array"), array5) );
	MojAssertNoErr( obj5.putString(_T("_kind"), _T("Test:1")) );
	MojAssertNoErr( obj5.putString(_T("_id"), _T("arrayMerge1")) );

	MojExpectNoErr( db.merge(obj5) );

	MojExpectNoErr( db.get(id1, obj6, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj6.get(_T("array"), array) );
	//MojTestAssert(count == 10); 2.3.10 - changing behavior of array merges to not be additive
	EXPECT_EQ( 3ul, array.size() );

	// merge an array subobject with fewer properties than are in the db
	// and make sure the merge is correct
	MojObject array6;
	MojExpectNoErr( b.del(_T("b"), found) );
	EXPECT_TRUE( found );
	MojExpectNoErr( b.del(_T("c"), found) );
	EXPECT_TRUE( found );
	MojAssertNoErr( b.put(_T("d"), 15) );
	MojAssertNoErr( array6.push(b) );
	MojAssertNoErr( obj5.put(_T("array"), array6) );

	MojExpectNoErr( db.merge(obj5) );
	MojExpectNoErr( db.get(id1, obj6, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj6.get(_T("array"), array) );
	MojString bStr;
	MojAssertNoErr( bStr.assign(_T("test2")) );
	MojString cStr;
	MojAssertNoErr( cStr.assign(_T("new prop")) );
	count = 0;
	MojObject::ConstArrayIterator arrayEnd = array.arrayEnd();
	for (MojObject::ConstArrayIterator i = array.arrayBegin(); i != arrayEnd; ++i, ++count) {
		if (i->type() == MojObject::TypeObject) {
			MojObject id;
			MojExpectNoErr( i->getRequired(MojDb::IdKey, id) );
			if (id == bId) {
				MojObject key;
				MojExpectNoErr( i->getRequired(_T("d"), key) );
				EXPECT_EQ( MojObject(15), key );
				EXPECT_FALSE( i->get(_T("b"), key) );
				//MojTestAssert(key == bStr);
				EXPECT_FALSE( i->get(_T("c"), key) );
				//MojTestAssert(key == cStr);
			}
		}
	}
	EXPECT_EQ( 1ul, count );

	MojAssertNoErr( obj7.fromJson(baseObj2) );
	MojExpectNoErr( db.put(obj7) );
	MojExpectNoErr( db.get(id2, obj8, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj8.get(_T("array"), array) );
	EXPECT_EQ( 3ul, array.size() );

	MojAssertNoErr( obj7.fromJson(baseObj2) );
	MojExpectNoErr( db.merge(obj7) );
	MojExpectNoErr( db.get(id2, obj9, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj9.get(_T("array"), array) );
	EXPECT_EQ( 3ul, array.size() );

	MojAssertNoErr( obj7.fromJson(mergeObj) );
	MojExpectNoErr( db.merge(obj7) );
	MojExpectNoErr( db.get(id2, obj10, found) );
	EXPECT_TRUE( found );
	EXPECT_TRUE( obj10.get(_T("array"), array) );
	//MojTestAssert(count == 4); 2.3.10 - changing behavior of array merges to not be additive
	EXPECT_EQ( 3ul, array.size() );

	MojAssertNoErr( obj11.fromJson(baseObj3) );
	MojExpectNoErr( db.put(obj11) );
	MojExpectNoErr( db.get(id3, obj12, found) );
	EXPECT_TRUE( found );
	MojObject subObject;
	EXPECT_TRUE( obj12.get(_T("subObject"), subObject) );

	MojObject nestedId;
	EXPECT_FALSE( subObject.get(MojDb::IdKey, nestedId) );
	EXPECT_TRUE( nestedId.undefined() );
}

TEST_F(CrudTest, persistence)
{
    // drop old
    MojExpectNoErr( db.drop(path.c_str()) );

    // open (now we work with local db)
    MojDb db;
    MojAssertNoErr( db.open(path.c_str()) );
    // add type
    MojObject kind;
    MojAssertNoErr( kind.fromJson(MojKindStr) );
    MojAssertNoErr( db.putKind(kind) );

    // put
    MojObject obj1;
    MojExpectNoErr( obj1.putString(_T("hello"), _T("world")) );
    MojExpectNoErr( obj1.putString(_T("_kind"), _T("Test:1")) );
    MojExpectNoErr( db.put(obj1) ) << PrintToString(obj1);
    MojObject id1;
    EXPECT_TRUE( obj1.get(MojDb::IdKey, id1) );
    // reopen
    MojExpectNoErr( db.close() );
    MojAssertNoErr( db.open(path.c_str()) );
    // get
    MojObject obj2;
    bool found = false;
    MojExpectNoErr( db.get(id1, obj2, found) );
    EXPECT_TRUE( found );
    MojString val;
    found = false;
    MojExpectNoErr( obj2.get(_T("hello"), val, found) );
    EXPECT_TRUE( found );
    MojString exp;
    MojAssertNoErr( exp.assign("world") );
    EXPECT_STREQ( _T("world"), val.data() ) << PrintToString(obj2);
    MojExpectNoErr( db.close() );
}
