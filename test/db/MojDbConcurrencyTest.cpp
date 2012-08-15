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


#include "MojDbConcurrencyTest.h"
#include "db/MojDb.h"
#include "db/MojDbQuery.h"
#include "db/MojDbCursor.h"

static const MojChar* const TestKind =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");
static const MojChar* const TestJson =
	_T("{\"_kind\":\"Test:1\",\"foo\":100,\"bar\":5000}");
static const MojUInt32 TestNumObjects = 10000;

static MojErr delThread(void* arg)
{
	MojDb* db = (MojDb*) arg;
	MojDbQuery query;
	MojErr err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db->del(query, count);
	MojErrCheck(err); // not TestErrCheck, since failure is expected
	MojTestAssert(count == TestNumObjects);

	return MojErrNone;
}

static MojErr queryThread(void* arg)
{
	MojDb* db = (MojDb*) arg;

	MojDbQuery query;
	MojErr err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	query.desc(true);
	MojDbCursor cursor;
	err = db->find(query, cursor);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	for (;;) {
		MojObject obj;
		bool found = false;
		err = cursor.get(obj, found);
		MojErrCheck(err); // not TestErrCheck, since failure is expected
		if (!found)
			break;
		++count;
	}
	err = cursor.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojDbConcurrencyTest::MojDbConcurrencyTest()
: MojTestCase(_T("MojDbConcurrency"))
{
}

MojErr MojDbConcurrencyTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	MojObject kind;
	err = kind.fromJson(TestKind);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);

	for (MojUInt32 i = 0; i < TestNumObjects; ++i) {
		MojObject obj;
		err = obj.fromJson(TestJson);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	MojThreadT thread1 = MojInvalidThread;
	MojThreadT thread2 = MojInvalidThread;
	err = MojThreadCreate(thread1, delThread, &db);
	MojTestErrCheck(err);
	err = MojThreadCreate(thread2, queryThread, &db);
	MojTestErrCheck(err);
	MojErr err1 = MojErrNone;
	MojErr err2 = MojErrNone;
	err = MojThreadJoin(thread1, err1);
	MojTestErrCheck(err);
	err = MojThreadJoin(thread2, err2);
	MojTestErrCheck(err);
	MojTestAssert(err1 == MojErrNone || err1 == MojErrDbDeadlock);
	MojTestAssert(err2 == MojErrNone || err2 == MojErrDbDeadlock);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbConcurrencyTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
