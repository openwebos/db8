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


#include "MojDbLocaleTest.h"
#include "db/MojDb.h"
#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/MojDbLevelEngine.h"
#endif
#include "MojDbTestStorageEngine.h"

static const MojChar* const MojTestKindStr =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"secondary\"}]}]}");

static const MojChar* MojTestObjects[] = {
	_T("{\"_id\":1,\"_kind\":\"Test:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":2,\"_kind\":\"Test:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":3,\"_kind\":\"Test:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":4,\"_kind\":\"Test:1\",\"foo\":\"côté\"}"),
	NULL
};

MojDbLocaleTest::MojDbLocaleTest()
: MojTestCase(_T("MojDbLocale"))
{
}

MojErr MojDbLocaleTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put kind
	MojObject obj;
	err = obj.fromJson(MojTestKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	err = db.updateLocale(_T("fr_FR"));
	MojTestErrCheck(err);
	err = put(db);
	MojErrCheck(err);

	err = db.updateLocale(_T("en_US"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,2,3,4]"));
	MojTestErrCheck(err);

	err = db.updateLocale(_T("fr_FR"));
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);

	// close and reopen with test engine
	err = db.close();
	MojTestErrCheck(err);
#ifdef MOJ_USE_BDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbBerkeleyEngine());
#elif MOJ_USE_LDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbLevelEngine());
#else
    MojRefCountedPtr<MojDbStorageEngine> engine;
#endif
	MojAllocCheck(engine.get());
	MojRefCountedPtr<MojDbTestStorageEngine> testEngine(new MojDbTestStorageEngine(engine.get()));
	MojAllocCheck(testEngine.get());
	err = testEngine->open(MojDbTestDir);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir, testEngine.get());
	MojTestErrCheck(err);

	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);
	err = put(db);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);

	// fail txn commit
	err = testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);
	err = db.updateLocale(_T("en_US"));
	MojTestErrExpected(err, MojErrDbDeadlock);
	// make sure we still have fr ordering
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);
	err = put(db);
	MojTestErrCheck(err);
	err = checkOrder(db, _T("[1,3,2,4]"));
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbLocaleTest::put(MojDb& db)
{
	// make sure that we are still consistent by deleting and re-adding
	MojDbQuery query;
	MojErr err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db.del(query, count);
	MojTestErrCheck(err);

	for (const MojChar** i = MojTestObjects; *i != NULL; ++i) {
		MojObject obj;
		err = obj.fromJson(*i);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbLocaleTest::checkOrder(MojDb& db, const MojChar* expectedJson)
{
	MojObject expected;
	MojErr err = expected.fromJson(expectedJson);
	MojTestErrCheck(err);
	MojObject::ConstArrayIterator i = expected.arrayBegin();

	MojDbQuery query;
	err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query.order(_T("foo"));
	MojTestErrCheck(err);
	MojDbCursor cur;
	err = db.find(query, cur);
	MojTestErrCheck(err);
	for (;;) {
		MojObject obj;
		bool found = false;
		err = cur.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojTestAssert(i != expected.arrayEnd());
		MojObject id;
		MojTestAssert(obj.get(MojDb::IdKey, id));
		MojInt64 idInt = id.intValue();
		MojUnused(idInt);
		MojTestAssert(id == *i++);
	}
	return MojErrNone;
}

void MojDbLocaleTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
