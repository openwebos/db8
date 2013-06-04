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


#include "MojDbSearchTest.h"
#include "db/MojDb.h"
#include "db/MojDbSearchCursor.h"
#include "core/MojObjectBuilder.h"

static const MojChar* const MojSearchKindStr =
	_T("{\"id\":\"SearchTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"tokenize\":\"all\",\"collate\":\"primary\"}]},")
		_T("{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\",\"tokenize\":\"all\",\"collate\":\"primary\"}]},")
		_T("{\"name\":\"multiprop\",\"props\":[{\"name\":\"multiprop\",\"type\":\"multi\",\"collate\":\"primary\",\"include\":[{\"name\":\"hello\",\"tokenize\":\"all\"},{\"name\":\"world\",\"tokenize\":\"all\"}]}]}")
	_T("]}");
static const MojChar* const MojSearchTestObjects[] = {
	_T("{\"_id\":1,\"_kind\":\"SearchTest:1\",\"bar\":4,\"foo\":\"four score & seven years\"}"),
	_T("{\"_id\":2,\"_kind\":\"SearchTest:1\",\"bar\":3,\"foo\":\"our fathers brought forth\"}"),
	_T("{\"_id\":3,\"_kind\":\"SearchTest:1\",\"bar\":2,\"foo\":\"four fathers forth\"}"),
	_T("{\"_id\":4,\"_kind\":\"SearchTest:1\",\"bar\":1,\"foo\":\"four\"}"),
	_T("{\"_id\":5,\"_kind\":\"SearchTest:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":6,\"_kind\":\"SearchTest:1\",\"foo\":\"CotE\"}"),
	_T("{\"_id\":7,\"_kind\":\"SearchTest:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":8,\"_kind\":\"SearchTest:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":9,\"_kind\":\"SearchTest:1\",\"foo\":\"côTe\"}"),
	_T("{\"_id\":10,\"_kind\":\"SearchTest:1\",\"foo\":\"carap\"}"),
	_T("{\"_id\":11,\"_kind\":\"SearchTest:1\",\"foo\":\"APPLE\"}"),
	_T("{\"_id\":12,\"_kind\":\"SearchTest:1\",\"foo\":\"aardvark\"}"),
	_T("{\"_id\":13,\"_kind\":\"SearchTest:1\",\"foo\":\"AbAcus\"}"),
	_T("{\"_id\":14,\"_kind\":\"SearchTest:1\",\"hello\":\"this is a test\"}"),
	_T("{\"_id\":15,\"_kind\":\"SearchTest:1\",\"world\":\"this is not a test\"}")
};

MojDbSearchTest::MojDbSearchTest()
: MojTestCase(_T("MojDbSearch"))
{
}

MojErr MojDbSearchTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// add kind
	MojObject kindObj;
	err = kindObj.fromJson(MojSearchKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kindObj);
	MojTestErrCheck(err);

	// put test objects
	for (MojSize i = 0; i < sizeof(MojSearchTestObjects) / sizeof(MojChar*); ++i) {
		MojObject obj;
		err = obj.fromJson(MojSearchTestObjects[i]);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	err = simpleTest(db);
	MojTestErrCheck(err);
	err = filterTest(db);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbSearchTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbSearchTest::simpleTest(MojDb& db)
{
	// just foo
	MojErr err = check(db, _T("bogus"), _T("[]"));
	MojTestErrCheck(err);
	err = check(db, _T("f"), _T("[1,2,3,4]"));
	MojTestErrCheck(err);
	err = check(db, _T("F"), _T("[1,2,3,4]"));
	MojTestErrCheck(err);
	err = check(db, _T("fo"), _T("[1,2,3,4]"));
	MojTestErrCheck(err);
	err = check(db, _T("four"), _T("[1,3,4]"));
	MojTestErrCheck(err);
	err = check(db, _T("score"), _T("[1]"));
	MojTestErrCheck(err);
	err = check(db, _T("four years"), _T("[1]"));
	MojTestErrCheck(err);
	err = check(db, _T("Four Years"), _T("[1]"));
	MojTestErrCheck(err);
	err = check(db, _T("four decades"), _T("[]"));
	MojTestErrCheck(err);
	err = check(db, _T("fathers forth"), _T("[2,3]"));
	MojTestErrCheck(err);
	err = check(db, _T("four f"), _T("[1,3,4]"));
	MojTestErrCheck(err);
	err = check(db, _T("four f fo fou"), _T("[1,3,4]"));
	MojTestErrCheck(err);
	// bar and foo
	err = check(db, _T("f"), _T("[3]"), NULL, 2);
	MojTestErrCheck(err);
	// order by bar
	err = check(db, _T("f"), _T("[4,3,2,1]"), _T("bar"));
	MojTestErrCheck(err);
	err = check(db, _T("f"), _T("[1,2,3,4]"), _T("bar"), MojObject::Undefined, true);
	MojTestErrCheck(err);
	err = check(db, _T("f"), _T("[4,3,1,2]"), _T("foo"));
	MojTestErrCheck(err);
	// array value for bar
	MojObject array;
	err = array.push(1);
	MojErrCheck(err);
	err = array.push(2);
	MojErrCheck(err);
	err = check(db, _T("f"), _T("[4,3]"), _T("bar"), array);
	MojTestErrCheck(err);

	// limit
	MojDbQuery query;
	err = initQuery(query, _T("f"));
	MojTestErrCheck(err);
	query.limit(2);
	err = check(db, query, _T("[1,2]"));
	MojTestErrCheck(err);
	err = initQuery(query, _T("f"), _T("bar"));
	MojTestErrCheck(err);
	query.limit(2);
	err = check(db, query, _T("[4,3]"));
	MojTestErrCheck(err);

	// accent insensitivity
	err = check(db, _T("COTE"), _T("[5,6,7,8,9]"));
	MojTestErrCheck(err);

	// case-insensitive ordering
	err = check(db, _T("a"), _T("[12,13,11]"), _T("foo"));
	MojTestErrCheck(err);

	// multi-prop
	query.clear();
	err = query.from(_T("SearchTest:1"));
	MojTestErrCheck(err);
	MojString val;
	err = val.assign(_T("test"));
	MojTestErrCheck(err);
	err = query.where(_T("multiprop"), MojDbQuery::OpSearch, val, MojDbCollationPrimary);
	MojTestErrCheck(err);
	err = check(db, query, _T("[14,15]"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbSearchTest::filterTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = initQuery(query, _T("f"));
	MojTestErrCheck(err);
	err = query.filter(_T("bar"), MojDbQuery::OpEq, 1);
	MojTestErrCheck(err);
	err = check(db, query, _T("[4]"));
	MojTestErrCheck(err);

	err = initQuery(query, _T("f"));
	MojTestErrCheck(err);
	err = query.filter(_T("bar"), MojDbQuery::OpGreaterThan, 1);
	MojTestErrCheck(err);
	err = check(db, query, _T("[1,2,3]"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbSearchTest::initQuery(MojDbQuery& query, const MojChar* queryStr, const MojChar* orderBy, const MojObject& barVal, bool desc)
{
	query.clear();
	MojErr err = query.from(_T("SearchTest:1"));
	MojTestErrCheck(err);
	MojString val;
	err = val.assign(queryStr);
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpSearch, val, MojDbCollationPrimary);
	MojTestErrCheck(err);
	query.desc(desc);
	if (!barVal.undefined()) {
		err = query.where(_T("bar"), MojDbQuery::OpEq, barVal);
		MojTestErrCheck(err);
	}
	if (orderBy) {
		err = query.order(orderBy);
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbSearchTest::check(MojDb& db, const MojChar* queryStr, const MojChar* expectedIdsJson,
		const MojChar* orderBy, const MojObject& barVal, bool desc)
{
	MojDbQuery query;
	MojErr err = initQuery(query, queryStr, orderBy, barVal, desc);
	MojTestErrCheck(err);
	err = check(db, query, expectedIdsJson);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbSearchTest::check(MojDb& db, const MojDbQuery& query, const MojChar* expectedIdsJson)
{
   MojString str;
   MojDbSearchCursor cursor(str);
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

	MojTestAssert(expected.size() == results.size());
	MojObject::ConstArrayIterator j = results.arrayBegin();
	for (MojObject::ConstArrayIterator i = expected.arrayBegin();
			i != expected.arrayEnd(); ++i, ++j) {
		MojObject id;
		err = j->getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		MojTestAssert(*i == id);
	}
	return MojErrNone;
}
