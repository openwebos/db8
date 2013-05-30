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


#include "MojDbQueryTest.h"
#include "db/MojDb.h"
#include "db/MojDbCursor.h"

static const int MojTestNumObjects = 1000;
static const MojChar* const MojKind1Str =
	_T("{\"id\":\"QueryTest1:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]},{\"name\":\"idfoo\",\"props\":[{\"name\":\"_id\"},{\"name\":\"foo\"}]}]}");
static const MojChar* const MojKind2Str =
	_T("{\"id\":\"QueryTest2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},")
		_T("{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]},")
		_T("{\"name\":\"multiprop\",\"props\":[{\"name\":\"foobar\",\"type\":\"multi\",\"include\":[{\"name\":\"foo\"},{\"name\":\"bar\"}]}]}")
	_T("]}");
static const MojChar* const MojKind3Str =
	_T("{\"id\":\"QueryTest3:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"extends\":[\"QueryTest2:1\"],")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");
static const MojChar* const MojKind4Str =
	_T("{\"id\":\"QueryTest4:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const MojKind5Str =
	_T("{\"id\":\"QueryTest5:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}]}");
	static const MojChar* const MojKind5_1Str =
	_T("{\"id\":\"QueryTest5:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}],\"incDel\":true}]}");

static const MojChar* const MojNestedObjStr =
	_T("{\"foobar\":\"nested string\"}");
static const MojChar* const MojDoubleNestedObjStr =
	_T("{\"foobar\":{\"another\":\"nested string\"}}");

struct TestEq
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 == obj2;
	}
};

struct TestNotEq
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 != obj2;
	}
};

struct TestLessThan
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 < obj2;
	}
};

struct TestLessThanEq
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 <= obj2;
	}
};

struct TestGreaterThan
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 > obj2;
	}
};

struct TestGreaterThanEq
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		return obj1 >= obj2;
	}
};

struct TestPrefix
{
	bool operator()(const MojObject& obj1, const MojObject& obj2) const
	{
		MojString str1;
		(void) obj1.stringValue(str1);
		MojString str2;
		(void) obj2.stringValue(str2);
		return str1.startsWith(str2);
	}
};

MojDbQueryTest::MojDbQueryTest()
: MojTestCase(_T("MojDbQuery"))
{
}

MojErr MojDbQueryTest::run()
{
	MojErr err = serializationTest();
	MojTestErrCheck(err);
	err = basicTest();
	MojTestErrCheck(err);
	err = invalidTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::basicTest()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// add kind
	MojObject kindObj;
	err = kindObj.fromJson(MojKind1Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);
	err = kindObj.fromJson(MojKind2Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);
	err = kindObj.fromJson(MojKind3Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);
	err = kindObj.fromJson(MojKind4Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);
	err = kindObj.fromJson(MojKind5Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);

	// insert a bunch of QueryTest1 objects
	err = put(db);
	MojTestErrCheck(err);
	// insert a bunch of QueryTest2 objects
	err = put2(db, 1, _T("hello"));
	MojTestErrCheck(err);
	err = put2(db, 1, _T("hello world"));
	MojTestErrCheck(err);
	err = put2(db, 1, _T(""));
	MojTestErrCheck(err);
	err = put3(db, 2, _T("hello world!!!"));
	MojTestErrCheck(err);
	err = put3(db, 2, _T("hi"));
	MojTestErrCheck(err);
	err = put3(db, 3, _T("goodbye"));
	MojTestErrCheck(err);
	err = put3(db, 3, _T("goodboy"));
	MojTestErrCheck(err);
	err = put3(db, 4, _T("zanzabar"));
	MojTestErrCheck(err);

	MojObject nestedObj;
	err = nestedObj.fromJson(MojNestedObjStr);
	MojTestErrCheck(err);
	err = put4(db, nestedObj, _T("top level string"));
	MojTestErrCheck(err);
	err = nestedObj.fromJson(MojDoubleNestedObjStr);
	MojTestErrCheck(err);
	err = put4(db, nestedObj, _T("2nd obj"));
	MojTestErrCheck(err);

	// get all by kind
	err = eqTest(db);
	MojTestErrCheck(err);
	err = neqTest(db);
	MojTestErrCheck(err);
	err = ltTest(db);
	MojTestErrCheck(err);
	err = lteTest(db);
	MojTestErrCheck(err);
	err = gtTest(db);
	MojTestErrCheck(err);
	err = gteTest(db);
	MojTestErrCheck(err);
	err = gtltTest(db);
	MojTestErrCheck(err);
	err = limitTest(db);
	MojTestErrCheck(err);
	err = countTest(db);
	MojTestErrCheck(err);
	err = stringTest(db);
	MojTestErrCheck(err);
	err = multiTest(db);
	MojTestErrCheck(err);
	err = pageTest(db);
	MojTestErrCheck(err);
	err = idTest(db);
	MojTestErrCheck(err);
	err = kindTest(db);
	MojTestErrCheck(err);
	err = orderTest(db);
	MojTestErrCheck(err);
	err = dupTest(db);
	MojTestErrCheck(err);
	err = delTest(db);
	MojTestErrCheck(err);
	err = isolationTest(db);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::eqTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpPrefix, TestEq(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects - 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), -1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects, 0);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpPrefix, TestEq(), 8, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 0, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects - 2, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects, 0);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("foo"), 8, _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), 0, _T("bar"), MojDbQuery::OpEq, TestEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojTestNumObjects - 2, _T("bar"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects - 2, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), 9, _T("bar"), MojDbQuery::OpEq, TestEq(), 7, 0);
	MojTestErrCheck(err);
	// foo[]
	MojObject array;
	err = array.push(5);
	MojTestErrCheck(err);
	err = array.push(6);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), array, 2);
	MojTestErrCheck(err);
	err = array.push(5);
	MojTestErrCheck(err);
	err = array.push(6);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), array, 2);
	MojTestErrCheck(err);
	err = array.push(-79);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), array, 2);
	MojTestErrCheck(err);
	array.clear(MojObject::TypeArray);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), array, 0);
	MojTestErrCheck(err);
	err = array.push(-4543);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), array, 0);
	MojTestErrCheck(err);
	// foo[]bar
	array.clear();
	err = array.push(8);
	MojTestErrCheck(err);
	err = array.push(9);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), array, _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), array, _T("bar"), MojDbQuery::OpEq, TestEq(), 10, 0);
	MojTestErrCheck(err);
	// bar[]foo
	err = check(db, _T("QueryTest1:1"), _T("foo"), 8, _T("bar"), MojDbQuery::OpEq, TestEq(), array, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), 5, _T("bar"), MojDbQuery::OpEq, TestEq(), array, 0);
	MojTestErrCheck(err);

	// query where foo is a subobject
	MojObject nestedObj;
	err = nestedObj.fromJson(MojNestedObjStr);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest4:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), nestedObj, 1);
	MojTestErrCheck(err);
	err = nestedObj.fromJson(MojDoubleNestedObjStr);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest4:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), nestedObj, 1);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::neqTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 0, MojTestNumObjects - 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), MojTestNumObjects - 1, MojTestNumObjects - 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), -1, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpNotEq, TestNotEq(), 0, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpNotEq, TestNotEq(), MojTestNumObjects - 2, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpNotEq, TestNotEq(), 1, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpNotEq, TestNotEq(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 27, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), MojTestNumObjects - 2, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), -6, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 9, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 7, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::ltTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 8, 8);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 0, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), MojTestNumObjects - 1, MojTestNumObjects - 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), -1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), G_MININT64, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), G_MAXINT64, MojTestNumObjects);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThan, TestLessThan(), 8, 8);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThan, TestLessThan(), 0, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThan, TestLessThan(), MojTestNumObjects - 2, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThan, TestLessThan(), 1, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThan, TestLessThan(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 9, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 10, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 8, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 0, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), MojTestNumObjects - 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 9, _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), 100, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::lteTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 8, 9);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), MojTestNumObjects - 1, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), -1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), G_MININT64, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), G_MAXINT64, MojTestNumObjects);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 8, 10);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 0, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThanEq, TestLessThanEq(), MojTestNumObjects - 2, MojTestNumObjects );
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 1, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpLessThanEq, TestLessThanEq(), MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 9, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 10, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 7, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 1, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), MojTestNumObjects - 1, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 9, _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), 100, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::gtTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 8, MojTestNumObjects - 9);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 0, MojTestNumObjects - 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), MojTestNumObjects - 1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), -1, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), G_MININT64, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), G_MAXINT64, 0);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 8, MojTestNumObjects - 10);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 0, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThan, TestGreaterThan(), MojTestNumObjects - 2, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 1, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThan, TestGreaterThan(), MojTestNumObjects, 0);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 9, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 7, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 8, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 1, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), MojTestNumObjects - 2, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 9, _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 100, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::gteTest(MojDb& db)
{
	// foo
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 8, MojTestNumObjects - 8);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), MojTestNumObjects - 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), -1, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), G_MININT64, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), G_MAXINT64, 0);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 8, MojTestNumObjects - 8);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), MojTestNumObjects - 2, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 1, MojTestNumObjects - 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), MojTestNumObjects, 0);
	MojTestErrCheck(err);
	// foobar
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 9, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 7, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 8, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 2, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojTestNumObjects - 2, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), MojTestNumObjects - 2, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 9, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 100, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::gtltTest(MojDb& db)
{
	// foo
	MojErr err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 8, 10, 1);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 8, MojTestNumObjects, MojTestNumObjects - 9);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 0, 8, 7);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, -1, 8, 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, -1, MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 1, 0, 0);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 8, 10, 3);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 8, MojTestNumObjects - 1, MojTestNumObjects - 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 0, 8, 9);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, -1, 8, 9);
	MojTestErrCheck(err);
	err = checkRange(db, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 0, MojTestNumObjects - 1, MojTestNumObjects);
	MojTestErrCheck(err);
	// bar
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 7, 9, 2);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 7, MojTestNumObjects, MojTestNumObjects - 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 0, 8, 6);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, -1, 8, 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, -1, MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThan, 8, 10, 2);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThan, 8, MojTestNumObjects - 1, MojTestNumObjects - 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThanEq, 0, 8, 8);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThanEq, -1, 8, 10);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 0, MojTestNumObjects - 1, MojTestNumObjects);
	MojTestErrCheck(err);
	// foobar
	err = checkRange(db, _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 9, 9, 1);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThanEq, MojDbQuery::OpLessThanEq, 8, 9, 2);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 8, 9, 0);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 7, 10, 2);
	MojTestErrCheck(err);
	err = checkRange(db, _T("bar"), 8, _T("foo"), MojDbQuery::OpGreaterThan, MojDbQuery::OpLessThan, 100, 2000, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::limitTest(MojDb& db)
{
	MojErr err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), 23, 5, 5);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1, 5);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 1, 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 2, 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, 1, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), 0, _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, 0, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::countTest(MojDb& db)
{
	MojErr err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, 8, 1);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, 8, 1, 0);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, G_MININT64, 0);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, G_MININT64, 0, 0);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, MojTestNumObjects, MojTestNumObjects);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpLessThan, MojTestNumObjects, MojTestNumObjects, 5);
	MojTestErrCheck(err);
	err = checkCount(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThan, MojTestNumObjects + 1, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::stringTest(MojDb& db)
{
	MojString str;
	MojErr err = str.assign(_T("hello"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), str, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 3);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpGreaterThan, TestGreaterThan(), str, 4);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), str, 5);
	MojTestErrCheck(err);
	err = str.assign(_T("hi"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpLessThanEq, TestLessThanEq(), str, 7);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpLessThan, TestLessThan(), str, 6);
	MojTestErrCheck(err);
	err = str.assign(_T("h"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 4);
	MojTestErrCheck(err);
	err = str.assign(_T("he"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 3);
	MojTestErrCheck(err);
	err = str.assign(_T("bye"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 0);
	MojTestErrCheck(err);
	str.clear();
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 8);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), str, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), str, 7);
	MojTestErrCheck(err);
	err = str.assign(_T("hello"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("bar"), 1, _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), str, 2);
	MojTestErrCheck(err);
	// prefix array
	MojObject array;
	err = array.pushString(_T("hell"));
	MojTestErrCheck(err);
	err = array.pushString(_T("good"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), array, 5);
	MojTestErrCheck(err);
	err = array.pushString(_T("he"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), array, 5);
	MojTestErrCheck(err);
	err = array.pushString(_T("h"));
	MojTestErrCheck(err);
	err = array.pushString(_T("w"));
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest2:1"), _T("foo"), MojDbQuery::OpPrefix, TestPrefix(), array, 6);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::multiTest(MojDb& db)
{
	// multi
	MojString str;
	MojErr err = str.assign(_T("hello"));
	MojTestErrCheck(err);
	MojDbQuery query;
	err = query.from(_T("QueryTest2:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foobar"), MojDbQuery::OpEq, str);
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	guint32 count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
	}
	MojTestAssert(count == 1);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::pageTest(MojDb& db)
{
	MojErr err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1, _T("foo"));
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 1, _T("foo"));
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 1, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("foo"));
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("bar"));
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("bar"), true);
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 2, _T("foo"));
	MojTestErrCheck(err);
	err = checkPage(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 0, 2, _T("foo"), true);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::idTest(MojDb& db)
{
	MojErr err = check(db, _T("Object:1"), MojDb::IdKey, MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects + 15);
	MojTestErrCheck(err);
	err = check(db, _T("Object:1"), MojDb::IdKey, MojDbQuery::OpEq, TestEq(), -2, 0);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), MojDb::IdKey, m_id0, _T("foo"), MojDbQuery::OpEq, TestEq(), 0, 1);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), MojDb::IdKey, m_id0, _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 0, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::kindTest(MojDb& db)
{
	MojErr err = checkKind(db, _T("QueryTest1:1"), MojTestNumObjects);
	MojTestErrCheck(err);
	err = checkKind(db, _T("QueryTest1:1"), MojTestNumObjects, _T("_id"));
	MojTestErrCheck(err);
	err = checkKind(db, _T("QueryTest2:1"), 8);
	MojTestErrCheck(err);
	err = checkKind(db, _T("QueryTest3:1"), 5);
	MojTestErrCheck(err);
	err = checkKind(db, _T("Object:1"), MojTestNumObjects + 15);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::orderTest(MojDb& db)
{
	MojErr err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1, _T("foo"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 1, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 1, _T("foo"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpNotEq, TestNotEq(), 8, MojTestNumObjects - 1, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("foo"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("bar"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpGreaterThanEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("bar"), true);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 2, _T("foo"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 0, 2, _T("foo"), true);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("QueryTest1:1"), NULL, MojDbQuery::OpEq, TestGreaterThanEq(), 0, MojTestNumObjects, _T("foo"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::dupTest(MojDb& db)
{
	MojErr err = put(db);
	MojTestErrCheck(err);

	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 0, 2);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects - 1, 2);
	MojTestErrCheck(err);
	// bar
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 8, 4);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), 0, 4);
	MojTestErrCheck(err);
	err = check(db, _T("QueryTest1:1"), _T("bar"), MojDbQuery::OpEq, TestEq(), MojTestNumObjects - 2, 4);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::delTest(MojDb& db)
{
	// add some objects and delete half of them
	bool found = false;
	for (gsize i = 0; i < 10; ++i) {
		MojObject obj;
		MojErr err = obj.putInt(_T("foo"), 45);
		MojTestErrCheck(err);
		err = obj.putString(MojDb::KindKey, _T("QueryTest5:1"));
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		MojObject id;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		if (i % 2 == 0) {
			err = db.del(id, found);
			MojTestErrCheck(err);
			MojTestAssert(found);
		}
	}
	// add in index that includes deleted
	MojObject kindObj;
	MojErr err = kindObj.fromJson(MojKind5_1Str);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);

	// id index without deleted
	MojDbQuery query;
	err = query.from(_T("QueryTest5:1"));
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	guint32 count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 5);
	err = cursor.close();
	MojTestErrCheck(err);
	// id index with deleted
	err = query.includeDeleted();
	MojErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 10);
	err = cursor.close();
	MojTestErrCheck(err);

	// use foo index with incDel
	err = query.where(_T("foo"), MojDbQuery::OpEq, 45);
	MojTestErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 10);
	err = cursor.close();
	MojTestErrCheck(err);

	// use array query with incDel
	MojObject array;
	err = array.push(45);
	MojTestErrCheck(err);
	err = array.push(-45);
	MojTestErrCheck(err);
	query.clear();
	err = query.from(_T("QueryTest5:1"));
	MojTestErrCheck(err);
	err = query.includeDeleted(true);
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpEq, array);
	MojTestErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 10);
	err = cursor.close();
	MojTestErrCheck(err);

	// query for only deleted
	query.clear();
	err = query.from(_T("QueryTest5:1"));
	MojTestErrCheck(err);
	err = query.where(MojDb::DelKey, MojDbQuery::OpEq, true);
	MojTestErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 5);
	err = cursor.close();
	MojTestErrCheck(err);

	// no include deleted
	for (ObjectSet::ConstIterator i = m_ids.begin(); i != m_ids.end(); ++i) {
		bool found = false;
		err = db.del(*i, found);
		MojTestErrCheck(err);
		MojTestAssert(found);
	}
	err = check(db, _T("QueryTest1:1"), _T("foo"), MojDbQuery::OpEq, TestEq(), 8, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}


MojErr MojDbQueryTest::isolationTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("QueryTest3:1"));
	MojTestErrCheck(err);
	guint32 count = 0;
	// del old objs
	err = db.del(query, count);
	MojTestErrCheck(err);
	// put in a bunch of objs with bar value of 1
	for (int i = 0; i < MojTestNumObjects; ++i) {
		MojObject obj;
		MojErr err = obj.putString(_T("_kind"), _T("QueryTest2:1"));
		MojTestErrCheck(err);
		err = obj.put(_T("bar"), 1);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		MojObject id;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		err = m_ids.put(id);
		MojTestErrCheck(err);
	}
	// open cursor
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	// update bar to 2
	MojObject props;
	err = props.put(_T("bar"), 2);
	MojTestErrCheck(err);
	err = db.merge(query, props, count);
	MojTestErrCheck(err);
	//
	count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojObject val;
		MojTestAssert(obj.get(_T("bar"), val) && val == 1);
		++count;
	}
	MojTestAssert(count == 0);

	return MojErrNone;
}

MojErr MojDbQueryTest::invalidTest()
{
	// invalid combos with =
	MojErr err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":1},{\"prop\":\"a\",\"op\":\"=\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":1},{\"prop\":\"a\",\"op\":\"<\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":1},{\"prop\":\"a\",\"op\":\">\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	// invalid <,<=/>,>= combos
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"<\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"<=\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"=\",\"val\":2}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\">\",\"val\":2},{\"prop\":\"b\",\"op\":\">\",\"val\":4}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\">\",\"val\":2},{\"prop\":\"b\",\"op\":\">=\",\"val\":4}]}"),
		MojErrDbInvalidQueryOpCombo);
	MojTestErrCheck(err);
	// invalid collation
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3,\"collate\":\"primary\"},{\"prop\":\"b\",\"op\":\">\",\"val\":2,\"collate\":\"tertiary\"}]}"),
		MojErrDbInvalidQueryCollationMismatch);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"b\",\"op\":\"<\",\"val\":3,\"collate\":\"backwards\"}]}"),
		MojErrDbInvalidCollation);
	MojTestErrCheck(err);
	// multiple non-eq props
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"!=\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"<\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"<=\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\">\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\">=\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":3},{\"prop\":\"b\",\"op\":\"%\",\"val\":2}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	// multiple array props
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":[3,4]},{\"prop\":\"b\",\"op\":\"=\",\"val\":[5,6]}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":[3,4]},{\"prop\":\"b\",\"op\":\"%\",\"val\":[5,6]}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	// non-allowed array prop
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":[3,4]}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"!=\",\"val\":[3,4]}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	// invalid orderings
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"<\",\"val\":2}],\"orderBy\":\"bar\"}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"!=\",\"val\":2}],\"orderBy\":\"bar\"}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"=\",\"val\":[2,3]}],\"orderBy\":\"bar\"}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);
	// no search op for find
	err = checkInvalid(
		_T("{\"from\":\"Foo:1\",\"where\":[{\"prop\":\"a\",\"op\":\"?\",\"val\":\"hello world\"}]}"),
		MojErrDbInvalidQuery);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::put(MojDb& db)
{
	for (int i = 0; i < MojTestNumObjects; ++i) {
		MojObject obj;
		MojErr err = obj.putString(_T("_kind"), _T("QueryTest1:1"));
		MojTestErrCheck(err);
		err = obj.putInt(_T("foo"), i);
		MojTestErrCheck(err);
		err = obj.putInt(_T("bar"), i & ~1);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		MojObject id;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		err = m_ids.put(id);
		MojTestErrCheck(err);
		if (i == 0)
			m_id0 = id;
	}
	return MojErrNone;
}

MojErr MojDbQueryTest::put2(MojDb& db, const MojObject& val1, const MojChar* val2)
{
	MojObject obj;
	MojErr err = obj.putString(_T("_kind"), _T("QueryTest2:1"));
	MojTestErrCheck(err);
	err = obj.put(_T("bar"), val1);
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), val2);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = m_ids.put(id);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::put3(MojDb& db, const MojObject& val1, const MojChar* val2)
{
	MojObject obj;
	MojErr err = obj.putString(_T("_kind"), _T("QueryTest3:1"));
	MojTestErrCheck(err);
	err = obj.put(_T("bar"), val1);
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), val2);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = m_ids.put(id);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::put4(MojDb& db, const MojObject& val1, const MojChar* val2)
{
	MojObject obj;
	MojErr err = obj.putString(_T("_kind"), _T("QueryTest4:1"));
	MojTestErrCheck(err);
	err = obj.put(_T("foo"), val1);
	MojTestErrCheck(err);
	err = obj.putString(_T("bar"), val2);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = m_ids.put(id);
	MojTestErrCheck(err);

	return MojErrNone;
}

template<class COMP>
MojErr MojDbQueryTest::check(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
		const COMP& comp, const MojObject& expectedVal, gsize expectedCount, guint32 limit)
{
	MojDbQuery query;
	MojErr err = query.from(kind);
	MojTestErrCheck(err);
	err = query.where(prop, op, expectedVal);
	MojTestErrCheck(err);
	if (limit != G_MAXUINT32)
		query.limit(limit);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		err = checkVal(obj, prop, expectedVal, comp);
		MojTestErrCheck(err);
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

template <class COMP>
MojErr MojDbQueryTest::check(MojDb& db, const MojChar* kind, const MojChar* prop1, const MojObject& expectedVal1,
		const MojChar* prop2, MojDbQuery::CompOp op, const COMP& comp, const MojObject& expectedVal2,
		gsize expectedCount, guint32 limit)
{
	MojDbQuery query;
	MojErr err = query.from(kind);
	MojTestErrCheck(err);
	err = query.where(prop1, MojDbQuery::OpEq, expectedVal1);
	MojTestErrCheck(err);
	err = query.where(prop2, op, expectedVal2);
	MojTestErrCheck(err);
	if (limit != G_MAXUINT32)
		query.limit(limit);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		err = checkVal(obj, prop1, expectedVal1, TestEq());
		MojTestErrCheck(err);
		err = checkVal(obj, prop2, expectedVal2, comp);
		MojTestErrCheck(err);
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

template <class COMP>
MojErr MojDbQueryTest::checkVal(const MojObject& obj, const MojChar* prop, const MojObject expectedVal, const COMP& comp)
{
	MojObject val;
	MojTestAssert(obj.get(prop, val));
	if (expectedVal.type() == MojObject::TypeArray) {
		bool matchedArray = false;
		for (MojObject::ConstArrayIterator i = expectedVal.arrayBegin();
				i != expectedVal.arrayEnd(); ++i) {
			if (comp(val, *i)) {
				matchedArray = true;
				break;
			}
		}
		MojTestAssert(matchedArray);
	} else {
		MojTestAssert(comp(val, expectedVal));
	}
	return MojErrNone;
}

template<class COMP>
MojErr MojDbQueryTest::checkOrder(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
		const COMP& comp, const MojObject& expectedVal, gsize expectedCount, const MojChar* order, bool desc)
{
	MojDbQuery query;
	MojErr err = query.from(kind);
	MojTestErrCheck(err);
	if (prop) {
		err = query.where(prop, op, expectedVal);
		MojTestErrCheck(err);
	}
	err = query.order(order);
	MojTestErrCheck(err);
	query.desc(desc);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	MojObject prev;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;

		MojObject val;
		if (prop) {
			MojTestAssert(obj.get(prop, val) && comp(val, expectedVal));
		}
		MojTestAssert(obj.get(order, val));
		if (count > 0) {
			if (desc) {
				MojTestAssert(val <= prev);
			} else {
				MojTestAssert(val >= prev);
			}
		}
		prev = val;
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

template<class COMP>
MojErr MojDbQueryTest::checkPage(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
		const COMP& comp, const MojObject& expectedVal, gsize expectedCount, const MojChar* order, bool desc)
{
	MojDbQuery::Page page;
	guint32 count = 0;
	gsize pageCount = 0;

	do {
		MojDbQuery query;
		MojErr err = query.from(kind);
		MojTestErrCheck(err);
		err = query.where(prop, op, expectedVal);
		MojTestErrCheck(err);
		err = query.order(order);
		MojTestErrCheck(err);
		query.page(page);
		query.limit(1);
		query.desc(desc);

		MojDbCursor cursor;
		err = db.find(query, cursor);
		MojTestErrCheck(err);

		for (;;) {
			bool found = false;
			MojObject obj;
			MojErr err = cursor.get(obj, found);
			MojTestErrCheck(err);
			if (!found)
				break;

			MojObject val;
			MojTestAssert(obj.get(prop, val) && comp(val, expectedVal));
			++count;
		}
		MojTestAssert(count > 0 || pageCount == 0);
		err = cursor.nextPage(page);
		MojTestErrCheck(err);
		err = cursor.close();
		MojTestErrCheck(err);
		++pageCount;
	} while (!page.empty());
	MojTestAssert(count == expectedCount);

	return MojErrNone;
}

MojErr MojDbQueryTest::checkRange(MojDb& db, const MojChar* prop, MojDbQuery::CompOp lowerOp, MojDbQuery::CompOp upperOp,
		const MojObject& lowerVal, const MojObject& upperVal, gsize expectedCount)
{
	MojDbQuery query;
	MojErr err = query.from(_T("QueryTest1:1"));
	MojTestErrCheck(err);
	err = query.where(prop, lowerOp, lowerVal);
	MojTestErrCheck(err);
	err = query.where(prop, upperOp, upperVal);
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;

		MojObject val;
		MojTestAssert(obj.get(prop, val));
		if (lowerOp == MojDbQuery::OpGreaterThan) {
			MojTestAssert(val > lowerVal);
		} else {
			MojTestAssert(val >= lowerVal);
		}
		if (upperOp == MojDbQuery::OpLessThan) {
			MojTestAssert(val < upperVal);
		} else {
			MojTestAssert(val <= upperVal);
		}
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::checkRange(MojDb& db, const MojChar* prop1, const MojObject& val1,
		const MojChar* prop2, MojDbQuery::CompOp lowerOp, MojDbQuery::CompOp upperOp,
		const MojObject& lowerVal, const MojObject& upperVal, gsize expectedCount)
{
	MojDbQuery query;
	MojErr err = query.from(_T("QueryTest1:1"));
	MojTestErrCheck(err);
	err = query.where(prop1, MojDbQuery::OpEq, val1);
	MojTestErrCheck(err);
	err = query.where(prop2, lowerOp, lowerVal);
	MojTestErrCheck(err);
	err = query.where(prop2, upperOp, upperVal);
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojObject val;
		MojTestAssert(obj.get(prop1, val) && val == val1);
		MojTestAssert(obj.get(prop2, val));
		if (lowerOp == MojDbQuery::OpGreaterThan) {
			MojTestAssert(val > lowerVal);
		} else {
			MojTestAssert(val >= lowerVal);
		}
		if (upperOp == MojDbQuery::OpLessThan) {
			MojTestAssert(val < upperVal);
		} else {
			MojTestAssert(val <= upperVal);
		}
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::checkCount(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
		const MojObject& expectedVal, gsize expectedCount, guint32 limit)
{
	MojDbQuery query;
	MojErr err = query.from(kind);
	MojTestErrCheck(err);
	err = query.where(prop, op, expectedVal);
	MojTestErrCheck(err);
	if (limit != G_MAXUINT32)
		query.limit(limit);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	if (limit != G_MAXUINT32) {
		bool found = false;
		while (count < limit) {
			MojObject obj;
			err = cursor.get(obj, found);
			MojErrCheck(err);
			if (!found)
				break;
		}
	}

	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == expectedCount);
	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::checkKind(MojDb& db, const MojChar* kind, gsize expectedCount, const MojChar* orderBy)
{
	MojDbQuery query;
	MojErr err = query.from(kind);
	MojTestErrCheck(err);
	if (orderBy) {
		err = query.order(orderBy);
		MojErrCheck(err);
	}

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	guint32 count = 0;
	MojString kindStr;
	err = kindStr.assign(kind);
	MojErrCheck(err);
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojTestErrCheck(err);
		++count;
	}
	MojTestAssert(count == expectedCount);

	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::serializationTest()
{
	MojErr err = basicFromObject();
	MojTestErrCheck(err);
	err = basicToObject();
	MojTestErrCheck(err);
	err = optionsFromObect();
	MojTestErrCheck(err);
	err = optionsToObect();
	MojTestErrCheck(err);
	err = complexClauseFromObject();
	MojTestErrCheck(err);
	err = complexClauseToObject();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryTest::basicFromObject()
{
	const MojChar* json1 = _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":1}]}");

	// simple query from obj
	MojObject obj1;
	MojErr err = obj1.fromJson(json1);
	MojTestErrCheck(err);
	MojDbQuery q1;
	err = q1.fromObject(obj1);
	MojTestErrCheck(err);
	MojTestAssert(q1.from()== _T("Test:1"));
	const MojDbQuery::WhereMap& where = q1.where();
	for (MojDbQuery::WhereMap::ConstIterator i = where.begin(); i != where.end(); i++) {
		if (i.key() == _T("foo")) {
			MojTestAssert(i->upperOp() == MojDbQuery::OpNone);
			MojTestAssert(i->lowerOp() == MojDbQuery::OpEq);
			MojTestAssert(i->lowerVal().intValue() == 1);
		} else {
			// unexpected where clause
			MojTestAssert(0);
		}
	}

	return MojErrNone;
}

MojErr MojDbQueryTest::basicToObject()
{
	MojDbQuery q;
	MojErr err = q.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojObject whereVal(1);
	err = q.where(_T("foo"), MojDbQuery::OpEq, whereVal);
	MojTestErrCheck(err);

	MojObject obj;
	err = q.toObject(obj);
	MojTestErrCheck(err);

	// test object against known valid string
	MojObject obj2;
	err = obj2.fromJson(_T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":1}]}"));
	MojTestErrCheck(err);
	MojTestAssert(obj == obj2);

	return MojErrNone;
}

MojErr MojDbQueryTest::optionsFromObect()
{
	const MojChar* json = _T("{\"from\":\"Test:1\",\"limit\":1, \"desc\":true}");

	MojObject obj;
	MojErr err = obj.fromJson(json);
	MojTestErrCheck(err);
	MojDbQuery q;
	err = q.fromObject(obj);
	MojTestErrCheck(err);
	MojTestAssert(q.from()== _T("Test:1"));
	MojTestAssert(q.limit() == 1);
	MojTestAssert(q.desc() == true);

	return MojErrNone;
}

MojErr MojDbQueryTest::optionsToObect()
{
	MojDbQuery q;
	MojErr err = q.from(_T("Test:1"));
	MojTestErrCheck(err);
	q.limit(1);
	q.desc(true);
	MojObject obj;
	err = q.toObject(obj);
	MojTestErrCheck(err);

	// test against known valid object
	MojObject obj2;
	err = obj2.fromJson(_T("{\"from\":\"Test:1\",\"limit\":1, \"desc\":true}"));
	MojTestErrCheck(err);
	MojTestAssert(obj == obj2);

	return MojErrNone;
}

MojErr MojDbQueryTest::complexClauseToObject()
{
	MojDbQuery q;
	MojErr err = q.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojObject val1(1);
	MojObject val10(10);
	err = q.where(_T("bar"), MojDbQuery::OpEq, val1);
	MojTestErrCheck(err);
	err = q.where(_T("foo"), MojDbQuery::OpGreaterThan, val1);
	MojTestErrCheck(err);
	err = q.where(_T("foo"), MojDbQuery::OpLessThan, val10);
	MojTestErrCheck(err);

	MojObject obj;
	err = q.toObject(obj);
	MojTestErrCheck(err);

	// test object against known valid string
	MojObject obj2;
	err = obj2.fromJson(_T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\">\",\"val\":1},")
						_T("{\"prop\":\"foo\",\"op\":\"<\",\"val\":10},{\"prop\":\"bar\",\"op\":\"=\",\"val\":1}]}"));
	MojTestErrCheck(err);

	MojTestAssert(obj == obj2);

	return MojErrNone;
}

MojErr MojDbQueryTest::complexClauseFromObject()
{
	const MojChar* json1 = _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":10},")
						   _T("{\"prop\":\"foo\",\"op\":\">\",\"val\":1},{\"prop\":\"bar\",\"op\":\"=\",\"val\":1}]}";)

	MojObject obj1;
	MojErr err = obj1.fromJson(json1);
	MojTestErrCheck(err);
	MojDbQuery q1;
	err = q1.fromObject(obj1);
	MojTestErrCheck(err);
	MojTestAssert(q1.from()== _T("Test:1"));
	const MojDbQuery::WhereMap& where = q1.where();
	bool foundFoo = false;
	bool foundBar = false;

	for (MojDbQuery::WhereMap::ConstIterator i = where.begin(); i != where.end(); i++) {
		if (i.key() == _T("foo")) {
			foundFoo = true;
			MojTestAssert(i->upperOp() == MojDbQuery::OpLessThan);
			MojTestAssert(i->upperVal().intValue() == 10);

			MojTestAssert(i->lowerOp() == MojDbQuery::OpGreaterThan);
			MojTestAssert(i->lowerVal().intValue() == 1);
		} else if (i.key() == _T("bar")) {
			foundBar = true;
			MojTestAssert(i->upperOp() == MojDbQuery::OpNone);
			MojTestAssert(i->lowerOp() == MojDbQuery::OpEq);
			MojTestAssert(i->lowerVal().intValue() == 1);
		} else {
			// unexpected where clause
			MojTestAssert(0);
		}
	}
	MojTestAssert(foundFoo);
	MojTestAssert(foundBar);

	return MojErrNone;
}

MojErr MojDbQueryTest::checkInvalid(const MojChar* queryJson, MojErr expectedErr)
{
	MojAssert(queryJson);

	MojObject queryObj;
	MojErr err = queryObj.fromJson(queryJson);
	MojTestErrCheck(err);
	MojDbQuery query;
	MojErr errCheck = query.fromObject(queryObj);
	MojErrAccumulate(err, errCheck);
	errCheck = query.validate();
	MojErrAccumulate(err, errCheck);
	errCheck = query.validateFind();
	MojErrAccumulate(err, errCheck);
	MojTestErrExpected(err, expectedErr);

	return MojErrNone;
}

void MojDbQueryTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
