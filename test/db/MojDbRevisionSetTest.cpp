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


#include "MojDbRevisionSetTest.h"
#include "db/MojDb.h"
#include "db/MojDbRevisionSet.h"

static const MojChar* const JsonRs1 = _T("{\"name\":\"revFoo\", \"props\":[{\"name\":\"foo\"}]}");
static const MojChar* const JsonKind1 = _T("{\"id\":\"RevSetTest:1\",\"owner\":\"mojodb.admin\",\"revSets\":[{\"name\":\"revFoo\", \"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const JsonObj1 = _T("{\"_kind\":\"RevSetTest:1\",\"_rev\":1, \"foo\":\"hello\", \"bar\":\"world\"}");

static const MojChar* const JsonRs2 = _T("{\"name\":\"revFooBar\", \"props\":[{\"name\":\"foo\"},{\"name\":\"bar\"}]}");
static const MojChar* const JsonKind2 = _T("{\"id\":\"RevSetTest:1\",\"owner\":\"mojodb.admin\",\"revSets\":[{\"name\":\"revFooBar\", \"props\":[{\"name\":\"foo\"},{\"name\":\"bar\"}]}]}");
static const MojChar* const JsonObj2 = _T("{\"_kind\":\"RevSetTest:1\",\"_rev\":1, \"foo\":\"hello\", \"bar\":\"world\", \"baz\":false}");

static const MojChar* const JsonRs3 = _T("{\"name\":\"revFooBarBaz\", \"props\":[{\"name\":\"foo\"},{\"name\":\"bar.baz\"}]}");
static const MojChar* const JsonKind3 = _T("{\"id\":\"RevSetTest:1\",\"owner\":\"mojodb.admin\",\"revSets\":[{\"name\":\"revFooBarBaz\", \"props\":[{\"name\":\"foo\"},{\"name\":\"bar.baz\"}]}]}");
static const MojChar* const JsonObj3 = _T("{\"_kind\":\"RevSetTest:1\",\"_rev\":1, \"foo\":\"hello\", \"bar\":[{\"baz\":false},{\"baz\":2},{\"baz\":\"yo\"}]}");
static const MojChar* const JsonObj3_2 = _T("{\"_kind\":\"RevSetTest:1\",\"_rev\":2, \"foo\":\"hello\", \"bar\":[{\"baz\":true},{\"baz\":2},{\"baz\":\"yo\"}]}");
static const MojChar* const JsonObj3_3 = _T("{\"_kind\":\"RevSetTest:1\",\"_rev\":3, \"foo\":\"hello\", \"bar\":[{\"baz\":false},{\"baz\":\"yo\"}]}");


MojDbRevisionSetTest::MojDbRevisionSetTest()
: MojTestCase("MojDbRevisionSet")
{
}

MojErr MojDbRevisionSetTest::run()
{
	MojErr err = standAloneTest();
	MojTestErrCheck(err);
	err = dbTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbRevisionSetTest::standAloneTest()
{
	// create one prop rs
	MojObject obj;
	MojErr err = obj.fromJson(JsonRs1);
	MojTestErrCheck(err);
	MojDbRevisionSet rs1;
	err = rs1.fromObject(obj);
	MojTestErrCheck(err);

	// one prop, no-old
	MojObject obj1;
	err = obj1.fromJson(JsonObj1);
	MojTestErrCheck(err);
	err = rs1.update(&obj1, NULL);
	MojTestErrCheck(err);
	MojInt64 rev;
	MojTestAssert(obj1.get(_T("revFoo"), rev) && rev == 1);

	// one prop, unchanged
	MojObject obj2 = obj1;
	err = obj2.put(_T("_rev"), 2);
	MojTestErrCheck(err);
	err = obj2.put(_T("bar"), true);
	MojTestErrCheck(err);
	err = rs1.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFoo"), rev) && rev == 1);

	// one prop, changed
	err = obj2.put(_T("foo"), 87);
	MojTestErrCheck(err);
	err = rs1.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFoo"), rev) && rev == 2);

	// create two prop rs
	err = obj.fromJson(JsonRs2);
	MojTestErrCheck(err);
	MojDbRevisionSet rs2;
	err = rs2.fromObject(obj);
	MojTestErrCheck(err);

	// two prop, no-old
	err = obj1.fromJson(JsonObj2);
	MojTestErrCheck(err);
	err = rs2.update(&obj1, NULL);
	MojTestErrCheck(err);
	MojTestAssert(obj1.get(_T("revFooBar"), rev) && rev == 1);

	// two prop, unchanged
	obj2 = obj1;
	err = obj2.put(_T("_rev"), 2);
	MojTestErrCheck(err);
	err = obj2.put(_T("baz"), true);
	MojTestErrCheck(err);
	err = rs2.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBar"), rev) && rev == 1);

	// two prop, changed one
	err = obj2.put(_T("bar"), 87);
	MojTestErrCheck(err);
	err = rs2.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBar"), rev) && rev == 2);

	// two prop, removed one
	obj2 = obj1;
	err = obj2.put(_T("_rev"), 3);
	MojTestErrCheck(err);
	bool found = false;
	err = obj2.del(_T("foo"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = rs2.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBar"), rev) && rev == 3);

	// create array prop rs
	err = obj.fromJson(JsonRs3);
	MojTestErrCheck(err);
	MojDbRevisionSet rs3;
	err = rs3.fromObject(obj);
	MojTestErrCheck(err);

	// array, no-old
	err = obj1.fromJson(JsonObj3);
	MojTestErrCheck(err);
	err = rs3.update(&obj1, NULL);
	MojTestErrCheck(err);
	MojTestAssert(obj1.get(_T("revFooBarBaz"), rev) && rev == 1);

	// array, unchanged
	obj2 = obj1;
	err = obj2.put(_T("_rev"), 2);
	MojTestErrCheck(err);
	err = obj2.put(_T("boo"), true);
	MojTestErrCheck(err);
	err = rs3.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBarBaz"), rev) && rev == 1);

	// array, changed one
	err = obj2.fromJson(JsonObj3_2);
	MojTestErrCheck(err);
	err = rs3.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBarBaz"), rev) && rev == 2);

	// two prop, removed one
	err = obj2.fromJson(JsonObj3_3);
	MojTestErrCheck(err);
	err = rs3.update(&obj2, &obj1);
	MojTestErrCheck(err);
	MojTestAssert(obj2.get(_T("revFooBarBaz"), rev) && rev == 3);

	return MojErrNone;
}

MojErr MojDbRevisionSetTest::dbTest()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// create one prop rs
	MojObject obj;
	err = obj.fromJson(JsonKind1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// one prop, no-old
	MojObject obj1;
	err = obj1.fromJson(JsonObj1);
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	err = checkRev(obj1, _T("revFoo"), true);
	MojTestErrCheck(err);

	// one prop, unchanged
	MojObject obj2 = obj1;
	err = obj2.put(_T("_rev"), 1);
	MojTestErrCheck(err);
	err = obj2.put(_T("bar"), true);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFoo"), false);
	MojTestErrCheck(err);

	// one prop, changed
	err = obj2.put(_T("foo"), 87);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFoo"), true);
	MojTestErrCheck(err);

	// create two prop rs
	err = obj.fromJson(JsonKind2);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// two prop, no-old
	err = obj1.fromJson(JsonObj2);
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	err = checkRev(obj1, _T("revFooBar"), true);
	MojTestErrCheck(err);

	// two prop, unchanged
	obj2 = obj1;
	err = obj2.put(_T("baz"), true);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFooBar"), false);
	MojTestErrCheck(err);

	// two prop, changed one
	err = obj2.put(_T("bar"), 87);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2,_T("revFooBar"), true);
	MojTestErrCheck(err);

	// two prop, removed one
	bool found = false;
	err = obj2.del(_T("foo"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2,_T("revFooBar"), true);
	MojTestErrCheck(err);

	// create array prop rs
	err = obj.fromJson(JsonKind3);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// array, no-old
	err = obj1.fromJson(JsonObj3);
	MojTestErrCheck(err);
	err = db.put(obj1);
	MojTestErrCheck(err);
	err = checkRev(obj1, _T("revFooBarBaz"), true);
	MojTestErrCheck(err);

	// array, unchanged
	obj2 = obj1;
	err = obj2.put(_T("boo"), true);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFooBarBaz"), false);
	MojTestErrCheck(err);

	// array, changed one
	err = obj2.fromJson(JsonObj3_2);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFooBarBaz"), true);
	MojTestErrCheck(err);

	// array, removed one
	err = obj2.fromJson(JsonObj3_3);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	err = checkRev(obj2, _T("revFooBarBaz"), true);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbRevisionSetTest::checkRev(const MojObject& obj, const MojChar* propName, bool eq)
{
	MojInt64 rev;
	MojTestAssert(obj.get(MojDb::RevKey, rev));
	MojInt64 rev2;
	MojTestAssert(obj.get(propName, rev2));
	if (eq) {
		MojTestAssert(rev == rev2);
	} else {
		MojTestAssert(rev > rev2);
	}
	return MojErrNone;
}

void MojDbRevisionSetTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

