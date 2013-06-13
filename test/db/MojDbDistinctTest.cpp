/* @@@LICENSE
*
*  Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "MojDbDistinctTest.h"
#include "db/MojDb.h"
#include "db/MojDbSearchCursor.h"
#include "core/MojObjectBuilder.h"

static const MojChar* const MojDistinctKindStr =
	_T("{\"id\":\"DistinctTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]},")
		_T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},")
	_T("]}");
static const MojChar* const MojDistinctTestObjects[] = {
	_T("{\"_id\":1,\"_kind\":\"DistinctTest:1\",\"bar\":\"c\",\"foo\":\"g\"}"),
	_T("{\"_id\":2,\"_kind\":\"DistinctTest:1\",\"bar\":\"d\",\"foo\":\"e\"}"),
	_T("{\"_id\":3,\"_kind\":\"DistinctTest:1\",\"bar\":\"a\",\"foo\":\"e\"}"),
	_T("{\"_id\":4,\"_kind\":\"DistinctTest:1\",\"bar\":\"b\",\"foo\":\"f\"}"),
	_T("{\"_id\":5,\"_kind\":\"DistinctTest:1\",\"bar\":\"a\",\"foo\":\"f\"}"),
	_T("{\"_id\":6,\"_kind\":\"DistinctTest:1\",\"bar\":\"b\",\"foo\":\"e\"}"),
	_T("{\"_id\":7,\"_kind\":\"DistinctTest:1\",\"bar\":\"c\",\"foo\":\"g\"}"),
	_T("{\"_id\":8,\"_kind\":\"DistinctTest:1\",\"bar\":\"a\",\"foo\":\"f\"}"),
	_T("{\"_id\":9,\"_kind\":\"DistinctTest:1\",\"bar\":\"b\",\"foo\":\"e\"}"),
	_T("{\"_id\":10,\"_kind\":\"DistinctTest:1\",\"bar\":\"b\",\"foo\":\"f\"}"),
	_T("{\"_id\":11,\"_kind\":\"DistinctTest:1\",\"bar\":\"c\",\"foo\":\"g\"}"),
	_T("{\"_id\":12,\"_kind\":\"DistinctTest:1\",\"bar\":\"c\",\"foo\":\"f\"}"),
	_T("{\"_id\":13,\"_kind\":\"DistinctTest:1\",\"bar\":\"b\",\"foo\":\"e\"}"),
	_T("{\"_id\":14,\"_kind\":\"DistinctTest:1\",\"bar\":\"a\",\"foo\":\"g\"}"),
	_T("{\"_id\":15,\"_kind\":\"DistinctTest:1\",\"bar\":\"c\",\"foo\":\"f\"}"),
};

MojDbDistinctTest::MojDbDistinctTest()
: MojTestCase(_T("MojDbDistinct"))
{
}

MojErr MojDbDistinctTest::run()
{
	// TODO : description, youngseung.ji
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// add kind
	MojObject kindObj;
	err = kindObj.fromJson(MojDistinctKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);

	// put test objects
	for (MojSize i = 0; i < sizeof(MojDistinctTestObjects) / sizeof(MojChar*); ++i) {
		MojObject obj;
		err = obj.fromJson(MojDistinctTestObjects[i]);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	// Start testing
	err = simpleTest(db);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbDistinctTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbDistinctTest::simpleTest(MojDb& db)
{
	MojErr err;
	MojDbQuery query;
	const MojChar* queryString;
	const MojChar* expectedIdsJson;
    MojString str;
    MojDbSearchCursor searchCursor(str);
    MojDbCursor cursor;

	//1st test
	queryString = _T("bar");
	expectedIdsJson = _T("[\"a\",\"b\",\"c\",\"d\"]");
	err = initQuery(query, queryString);
	MojTestErrCheck(err);
    err = check(db, query, searchCursor, queryString, expectedIdsJson);
	MojTestErrCheck(err);
    searchCursor.close();

	//test for find
    err = check(db, query, cursor, queryString, expectedIdsJson);
    MojTestErrCheck(err);
    cursor.close();

	//2nd test
	queryString = _T("foo");
	expectedIdsJson = _T("[\"e\",\"f\",\"g\"]");
	err = initQuery(query, queryString);
	MojTestErrCheck(err);

    err = check(db, query, searchCursor, queryString, expectedIdsJson);
    MojTestErrCheck(err);
    searchCursor.close();

	//test for find
    err = check(db, query, cursor, queryString, expectedIdsJson);
    MojTestErrCheck(err);
    cursor.close();

	return MojErrNone;
}

MojErr MojDbDistinctTest::check(MojDb& db, const MojDbQuery& query, MojDbCursor& cursor, const MojChar* queryString, const MojChar* expectedIdsJson)
{
	MojErr err = db.find(query, cursor);
	MojTestErrCheck(err);

	MojObjectBuilder builder;
	err = builder.beginArray();
	MojTestErrCheck(err);
	err = cursor.visit(builder);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = builder.endArray();
	MojTestErrCheck(err);
	MojObject results = builder.object();

	MojString json;
	err = results.toJson(json);
	MojTestErrCheck(err);

	MojObject expected;
	err = expected.fromJson(expectedIdsJson);
	MojTestErrCheck(err);

	// size check
	MojTestAssert(expected.size() == results.size());

	// value check
	MojObject::ConstArrayIterator j = results.arrayBegin();
	for (MojObject::ConstArrayIterator i = expected.arrayBegin();
			i != expected.arrayEnd(); ++i, ++j) {
		MojObject value;
		err = j->getRequired(queryString, value);
		MojTestErrCheck(err);
		MojTestAssert(*i == value);
	}
	return MojErrNone;
}

MojErr MojDbDistinctTest::initQuery(MojDbQuery& query, const MojChar* queryString)
{
	query.clear();
	MojErr err = query.from(_T("DistinctTest:1"));
	MojTestErrCheck(err);

	err = query.distinct(queryString);
	MojTestErrCheck(err);
    err = query.order(queryString);
    MojTestErrCheck(err);

	return MojErrNone;
}
