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


#include "MojDbQuotaTest.h"
#include "db/MojDb.h"
#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else 
#error "Specify database engine"
#endif
#include "MojDbTestStorageEngine.h"

static const MojChar* const MojTestKind1Str1 =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"com.foo.bar\"}");

static const MojChar* const MojTestKind1Str2 =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"com.foo.bar\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"secondary\"}]}]}");

static const MojChar* const MojTestKind2Str1 =
	_T("{\"id\":\"Test2:1\",")
	_T("\"owner\":\"com.foo.bar\"}");

static const MojChar* const MojTestKind2Str2 =
	_T("{\"id\":\"Test2:1\",")
	_T("\"owner\":\"com.foo.baz\"}");

static const MojChar* const MojTestKind2Str3 =
	_T("{\"id\":\"Test2:1\",\"extends\":[\"Test:1\"],")
	_T("\"owner\":\"com.foo.baz\"}");

static const MojChar* const MojTestKind3Str1 =
	_T("{\"id\":\"Test3:1\",")
	_T("\"owner\":\"com.foo.boo\"}");

static const MojChar* const MojTestKind3Str2 =
	_T("{\"id\":\"Test3:1\",")
	_T("\"owner\":\"com.bar\"}");

static const MojChar* MojTestKind1Objects[] = {
	_T("{\"_id\":1,\"_kind\":\"Test:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":2,\"_kind\":\"Test:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":3,\"_kind\":\"Test:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":4,\"_kind\":\"Test:1\",\"foo\":\"côté\"}"),
	NULL
};

static const MojChar* MojTestKind2Objects[] = {
	_T("{\"_id\":5,\"_kind\":\"Test2:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":6,\"_kind\":\"Test2:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":7,\"_kind\":\"Test2:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":8,\"_kind\":\"Test2:1\",\"foo\":\"côté\"}"),
	NULL
};

static const MojChar* MojTestKind3Objects[] = {
	_T("{\"_id\":9,\"_kind\":\"Test3:1\",\"foo\":\"cote\"}"),
	_T("{\"_id\":10,\"_kind\":\"Test3:1\",\"foo\":\"coté\"}"),
	_T("{\"_id\":11,\"_kind\":\"Test3:1\",\"foo\":\"côte\"}"),
	_T("{\"_id\":12,\"_kind\":\"Test3:1\",\"foo\":\"côté\"}"),
	NULL
};

MojDbQuotaTest::MojDbQuotaTest()
: MojTestCase(_T("MojDbQuota"))
{
}

void MojDbQuotaTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbQuotaTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	MojObject obj;
	err = obj.fromJson(MojTestKind1Str1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	err = testUsage(db);
	MojTestErrCheck(err);
	err = testMultipleQuotas(db);
	MojTestErrCheck(err);
	err = testEnforce(db);
	MojTestErrCheck(err);

	err = db.close();
	MojErrCheck(err);

	err = testErrors();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaTest::testUsage(MojDb& db)
{
	// put quota
	MojObject obj;
	MojErr err = obj.fromJson(_T("{\"owner\":\"com.foo.bar\",\"size\":1000}"));
	MojErrCheck(err);
	err = db.putQuotas(&obj, &obj + 1);
	MojErrCheck(err);
	// empty
	gint64 kindUsage = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage == 0);
	gint64 quotaUsage = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage == 0);
	// new obj
	err = put(db, MojTestKind1Objects[0]);
	MojTestErrCheck(err);
	gint64 kindUsage1 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage1);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage1 > 0);
	gint64 quotaUsage1 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage1 > 0);
	// add prop to existing obj
	err = obj.fromJson(MojTestKind1Objects[0]);
	MojTestErrCheck(err);
	err = obj.put(_T("bar"), 2);
	MojTestErrCheck(err);
	err = db.put(obj, MojDb::FlagForce);
	MojTestErrCheck(err);
	gint64 kindUsage2 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage2);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage2 > kindUsage1);
	gint64 quotaUsage2 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage2);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage2 > quotaUsage1);
	// add 2nd obj
	err = put(db, MojTestKind1Objects[1]);
	MojTestErrCheck(err);
	gint64 kindUsage3 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage3);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage3 > kindUsage2);
	gint64 quotaUsage3 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage3);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage3 > quotaUsage2);
	// del first obj
	bool found = false;
	err = db.del(1, found, MojDb::FlagPurge);
	MojTestErrCheck(err);
	MojTestAssert(found);
	gint64 kindUsage4 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage4);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage4 == kindUsage3 - kindUsage2);
	gint64 quotaUsage4 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage4);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage4 == quotaUsage3 - quotaUsage2);
	// add index
	err = obj.fromJson(MojTestKind1Str2);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	gint64 kindUsage5 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage5);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage5 > kindUsage4);
	gint64 quotaUsage5 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage5);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage5 > quotaUsage4);
	// update locale
	err = db.updateLocale(_T("FR_fr"));
	MojTestErrCheck(err);
	err = db.updateLocale(_T("EN_us"));
	MojTestErrCheck(err);
	gint64 kindUsage6 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage6);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage6 == kindUsage5);
	gint64 quotaUsage6 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage6);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage6 == quotaUsage5);
	// drop index
	err = obj.fromJson(MojTestKind1Str1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	gint64 kindUsage7 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage7);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage7 == kindUsage4);
	gint64 quotaUsage7 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage7);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage7 == quotaUsage4);
	// drop kind
	MojString kindStr;
	err = kindStr.assign(_T("Test:1"));
	MojTestErrCheck(err);
	err = db.delKind(kindStr, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	gint64 kindUsage8 = 0;
	err = getKindUsage(db, _T("Test:1"), kindUsage8);
	MojTestErrCheck(err);
	MojTestAssert(kindUsage8 == 0);
	gint64 quotaUsage8 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage8);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage8 == 0);
	err = db.quotaStats(obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaTest::testMultipleQuotas(MojDb& db)
{
	// quota for com.foo.baz
	MojObject obj;
	MojErr err = obj.fromJson(_T("{\"owner\":\"com.foo.baz\",\"size\":1000}"));
	MojErrCheck(err);
	err = db.putQuotas(&obj, &obj + 1);
	MojErrCheck(err);
	// register kinds
	err = obj.fromJson(MojTestKind1Str1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	err = obj.fromJson(MojTestKind2Str1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	// put object of kind1 and kind2
	err = put(db, MojTestKind1Objects[0]);
	MojTestErrCheck(err);
	err = put(db, MojTestKind2Objects[0]);
	MojTestErrCheck(err);
	gint64 quotaUsage1 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1);
	MojTestErrCheck(err);
	gint64 quotaUsage2 = 0;
	err = getQuotaUsage(db, _T("com.foo.baz"), quotaUsage2);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage1 > 0);
	MojTestAssert(quotaUsage2 == 0);
	// change owner of kind2 to com.foo.baz
	err = obj.fromJson(MojTestKind2Str2);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	gint64 quotaUsage3 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage3);
	MojTestErrCheck(err);
	gint64 quotaUsage4 = 0;
	err = getQuotaUsage(db, _T("com.foo.baz"), quotaUsage4);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage3 > 0);
	MojTestAssert(quotaUsage3 == quotaUsage4);
	// make kind2 inherit from kind1
	err = obj.fromJson(MojTestKind2Str3);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	gint64 quotaUsage5 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage5);
	MojTestErrCheck(err);
	gint64 quotaUsage6 = 0;
	err = getQuotaUsage(db, _T("com.foo.baz"), quotaUsage6);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage5 > quotaUsage1);
	MojTestAssert(quotaUsage6 == 0);
	// kind3 and object
	err = obj.fromJson(MojTestKind3Str1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);
	err = put(db, MojTestKind3Objects[0]);
	MojTestErrCheck(err);
	gint64 quotaUsage7 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage7);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage7 == quotaUsage5);
	// wildcard
	err = obj.fromJson(_T("{\"owner\":\"com.foo.*\",\"size\":1000}"));
	MojErrCheck(err);
	err = db.putQuotas(&obj, &obj + 1);
	MojErrCheck(err);
	gint64 quotaUsage8 = 0;
	err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage8);
	MojTestErrCheck(err);
	gint64 quotaUsage9 = 0;
	err = getQuotaUsage(db, _T("com.foo.*"), quotaUsage9);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage8 == quotaUsage5);
	MojTestAssert(quotaUsage9 > 0);

	return MojErrNone;
}

MojErr MojDbQuotaTest::testEnforce(MojDb& db)
{
	// set quota size to current usage
	gint64 quotaUsage1 = 0;
	MojErr err = getQuotaUsage(db, _T("com.foo.bar"), quotaUsage1);
	MojTestErrCheck(err);
	MojObject obj;
	err = obj.putString(_T("owner"), _T("com.foo.bar"));
	MojErrCheck(err);
	err = obj.putInt(_T("size"), quotaUsage1);
	MojErrCheck(err);
	err = db.putQuotas(&obj, &obj + 1);
	MojErrCheck(err);
	err = put(db, MojTestKind1Objects[3]);
	MojTestErrExpected(err, MojErrDbQuotaExceeded);
	// Try to delete the kind
	MojString kindStr;
	err = kindStr.assign(_T("Test:1"));
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(kindStr, found);

    //The delete should be failure, because it contain sub kind "Test2:1"
    MojTestErrExpected(err,MojErrDbKindHasSubKinds); 
    MojTestAssert(!found); 

	return MojErrNone;
}

MojErr MojDbQuotaTest::testErrors()
{
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
	MojErr err = testEngine->open(MojDbTestDir);
	MojTestErrCheck(err);

	MojDb db;
	err = db.open(MojDbTestDir, testEngine.get());
	MojTestErrCheck(err);

	// test that failed put does not affect quota
	gint64 quotaUsage1 = 0;
	err = getQuotaUsage(db, _T("com.foo.*"), quotaUsage1);
	MojTestErrCheck(err);
	err = testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);
	err = put(db, MojTestKind3Objects[1]);
	MojTestErrExpected(err, MojErrDbDeadlock);
	gint64 quotaUsage2 = 0;
	err = getQuotaUsage(db, _T("com.foo.*"), quotaUsage2);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage2 == quotaUsage1);
	// test that failed putQuota has no effect
	err = testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);
	MojObject obj;
	err = obj.fromJson(_T("{\"owner\":\"com.foo.boo\",\"size\":1000}"));
	MojErrCheck(err);
	err = db.putQuotas(&obj, &obj + 1);
	MojTestErrExpected(err, MojErrDbDeadlock);
	gint64 quotaUsage3 = 0;
	err = getQuotaUsage(db, _T("com.foo.*"), quotaUsage3);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage3 == quotaUsage1);
	// test that failed putKind has no effect
	err = testEngine->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);
	err = obj.fromJson(MojTestKind3Str2);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrExpected(err, MojErrDbDeadlock);
	gint64 quotaUsage4 = 0;
	err = getQuotaUsage(db, _T("com.foo.*"), quotaUsage4);
	MojTestErrCheck(err);
	MojTestAssert(quotaUsage4 == quotaUsage1);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaTest::put(MojDb& db, const MojChar* objJson)
{
	MojObject obj;
	MojErr err = obj.fromJson(objJson);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaTest::getKindUsage(MojDb& db, const MojChar* kindId, gint64& usageOut)
{
	MojRefCountedPtr<MojDbStorageTxn> txn;
	MojErr err = db.storageEngine()->beginTxn(txn);
	MojTestErrCheck(err);
	err = db.quotaEngine()->kindUsage(kindId, usageOut, txn.get());
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaTest::getQuotaUsage(MojDb& db, const MojChar* owner, gint64& usageOut)
{
	gint64 size;
	MojErr err = db.quotaEngine()->quotaUsage(owner, size, usageOut);
	MojTestErrCheck(err);

	return MojErrNone;
}
