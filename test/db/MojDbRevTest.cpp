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


#include "MojDbRevTest.h"
#include "db/MojDb.h"

static const MojChar* const MojKindStr =
	_T("{\"id\":\"RevTest:1\",\"owner\":\"mojodb.admin\"}");
static const MojChar* const MojTestObjStr1 =
	_T("{\"_kind\":\"RevTest:1\",\"foo\":1,\"bar\":2}");
static const MojChar* const MojTestObjStr2 =
	_T("{\"_kind\":\"RevTest:1\",\"foo\":100,\"bar\":2}");
static const MojChar* const MojTestObjStr3 =
	_T("{\"_kind\":\"RevTest:1\",\"foo\":100}");
static const MojChar* const MojTestObjStr4 =
	_T("{\"_kind\":\"RevTest:1\",\"foo\":2}");
static const MojChar* const MojTestObjStr5 =
	_T("{\"_kind\":\"RevTest:1\",\"_del\":false}");

MojDbRevTest::MojDbRevTest()
: MojTestCase(_T("MojDbRev"))
{
}

MojErr MojDbRevTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put type
	MojObject obj;
	err = obj.fromJson(MojKindStr);
	MojTestErrCheck(err);
	// put obj
	err = db.putKind(obj);
	MojTestErrCheck(err);
	err = obj.fromJson(MojTestObjStr1);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	MojObject id;
	err = obj.getRequired(MojDb::IdKey, id);
	MojTestErrCheck(err);
	// get obj and verify rev eq
	MojObject rev;
	err = obj.getRequired(MojDb::RevKey, rev);
	MojTestErrCheck(err);
	err = checkRevEq(db, id, rev);
	MojTestErrCheck(err);
	// put identical obj and verify rev not changed
	err = db.put(obj);
	MojTestErrCheck(err);
	err = checkRevEq(db, id, rev);
	MojTestErrCheck(err);
	// put with changed prop and verify rev gt
	err = obj.fromJson(MojTestObjStr2);
	MojTestErrCheck(err);
	err = obj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = db.put(obj); // this put will fail because we haven't included a rev
	MojTestErrExpected(err, MojErrDbRevNotSpecified);
	bool found = false;
	err = db.del(id, found); // verify that we can put without a rev if the object is deleted
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.put(obj);
	MojTestErrCheck(err);
	err = obj.getRequired(MojDb::RevKey, rev);
	MojTestErrCheck(err);
	err = obj.put(MojDb::RevKey, 1);
	MojTestErrCheck(err);
	err = db.put(obj); // this put will fail because the revision num is lower
	MojTestErrExpected(err, MojErrDbRevisionMismatch);
	err = obj.put(MojDb::RevKey, rev);
	MojTestErrCheck(err);
	err = db.put(obj); // now this should succeed
	MojTestErrCheck(err);
	// merge with unchanged prop and verify rev not changed
	err = obj.fromJson(MojTestObjStr3);
	MojTestErrCheck(err);
	err = obj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrCheck(err);
	err = checkRevEq(db, id, rev);
	MojTestErrCheck(err);
	// merge with changed prop and verify rev gt
	err = obj.fromJson(MojTestObjStr4);
	MojTestErrCheck(err);
	err = obj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrCheck(err);
	err = checkRevGt(db, id, rev);
	MojTestErrCheck(err);
	// query merge with unchanged prop and verify rev not changed
	MojDbQuery query;
	err = query.from("RevTest:1");
	MojTestErrCheck(err);
	err = obj.fromJson(MojTestObjStr4);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db.merge(query, obj, count);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);
	err = checkRevEq(db, id, rev);
	MojTestErrCheck(err);
	// query merge with changed prop and verify rev gt
	err = obj.fromJson(MojTestObjStr3);
	MojTestErrCheck(err);
	err = db.merge(query, obj, count);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);
	err = checkRevGt(db, id, rev);
	MojTestErrCheck(err);
	// del verify rev gt
	err = db.del(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = checkRevGt(db, id, rev);
	MojTestErrCheck(err);
	// del again and verify rev not changed
	err = db.del(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = checkRevEq(db, id, rev);
	MojTestErrCheck(err);
	// undel and verify rev gt
	err = obj.fromJson(MojTestObjStr5);
	MojTestErrCheck(err);
	err = obj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = db.merge(obj);
	MojTestErrCheck(err);
	err = checkRevGt(db, id, rev);
	MojTestErrCheck(err);
	// query del and verify rev gt
	err = db.del(query, count);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);
	err = checkRevGt(db, id, rev);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbRevTest::checkRevEq(MojDb& db, const MojObject& id, const MojObject& rev)
{
	MojObject obj;
	bool found = false;
	MojErr err = db.get(id, obj, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojObject rev2;
	err = obj.getRequired(MojDb::RevKey, rev2);
	MojTestErrCheck(err);
	MojTestAssert(rev == rev2);

	return MojErrNone;
}

MojErr MojDbRevTest::checkRevGt(MojDb& db, const MojObject& id, MojObject& rev)
{
	MojObject obj;
	bool found = false;
	MojErr err = db.get(id, obj, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojObject rev2;
	err = obj.getRequired(MojDb::RevKey, rev2);
	MojTestErrCheck(err);
	MojTestAssert(rev2 > rev);
	rev = rev2;

	return MojErrNone;
}

void MojDbRevTest::cleanup()
{
	MojDb db;
	(void) db.drop(MojDbTestDir);
}

