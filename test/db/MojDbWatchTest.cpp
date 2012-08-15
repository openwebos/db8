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


#include "MojDbWatchTest.h"
#include "db/MojDb.h"

static const MojChar* const MojKindStr =
	_T("{\"id\":\"WatchTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},")
	_T("{\"name\":\"foo_rev\",\"props\":[{\"name\":\"foo\"},{\"name\":\"_rev\"}], \"incDel\":true},")
	_T("{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");

class TestWatcher : public MojSignalHandler
{
public:
	TestWatcher() : m_slot(this, &TestWatcher::handleChange), m_count(0) { ++s_instanceCount; }
	~TestWatcher() { --s_instanceCount; }

	MojErr handleChange()
	{
		++m_count;
		return MojErrNone;
	}

	MojDb::WatchSignal::Slot<TestWatcher> m_slot;
	int m_count;
	static int s_instanceCount;
};

int TestWatcher::s_instanceCount = 0;

MojDbWatchTest::MojDbWatchTest()
: MojTestCase(_T("MojDbWatch"))
{
}

MojErr MojDbWatchTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	MojObject type;
	err = type.fromJson(MojKindStr);
	MojTestErrCheck(err);
	err = db.putKind(type);
	MojTestErrCheck(err);

	// eq
	err = eqTest(db);
	MojTestErrCheck(err);
	// gt
	err = gtTest(db);
	MojTestErrCheck(err);
	// lt
	err = ltTest(db);
	MojTestErrCheck(err);
	// cancel
	err = cancelTest(db);
	MojTestErrCheck(err);
	// range
	err = rangeTest(db);
	MojTestErrCheck(err);
	// pages
	err = pageTest(db);
	MojTestErrCheck(err);

	// make sure we're not hanging onto watcher references
	MojTestAssert(TestWatcher::s_instanceCount == 0);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbWatchTest::eqTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpEq, 1);
	MojTestErrCheck(err);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojRefCountedPtr<TestWatcher> watcher2(new TestWatcher);
	MojTestAssert(watcher2.get());
	bool fired = false;
	err = db.watch(query, cursor, watcher2->m_slot, fired);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(!fired);
	// puts
	MojObject id;
	err = put(db, 0, 0, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojTestAssert(watcher2->m_count == 0);
	err = put(db, 2, 2, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojTestAssert(watcher2->m_count == 0);
	err = put(db, 1, 1, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);
	err = put(db, 1, 1, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);
	// put, changing property not in index
	watcher.reset(new TestWatcher);
	MojTestAssert(watcher.get());
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = put(db, 1, 2, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	// dels
	watcher.reset(new TestWatcher);
	MojTestAssert(watcher.get());
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	watcher2.reset(new TestWatcher);
	MojTestAssert(watcher2.get());
	MojDbQuery queryWithRev;
	err = queryWithRev.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("foo"), MojDbQuery::OpEq, 1);
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("_rev"), MojDbQuery::OpGreaterThan, m_rev);
	MojTestErrCheck(err);
	err = queryWithRev.includeDeleted();
	MojTestErrCheck(err);
	fired = false;
	err = db.watch(queryWithRev, cursor, watcher2->m_slot, fired);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(!fired);
	MojTestAssert(watcher->m_count == 0);
	MojTestAssert(watcher2->m_count == 0);
	bool found;
	err = db.del(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);
	// ordering
	watcher.reset(new TestWatcher);
	MojTestAssert(watcher.get());
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = put(db, 1, 1, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);

	return MojErrNone;
}

MojErr MojDbWatchTest::gtTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpGreaterThan, 1);
	MojTestErrCheck(err);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojRefCountedPtr<TestWatcher> watcher2(new TestWatcher);
	MojTestAssert(watcher2.get());

	MojDbQuery queryWithRev;
	err = queryWithRev.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("foo"), MojDbQuery::OpEq, 2);
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("_rev"), MojDbQuery::OpGreaterThan, m_rev);
	MojTestErrCheck(err);
	bool fired = false;
	err = db.watch(queryWithRev, cursor, watcher2->m_slot, fired);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(!fired);
	MojObject id;
	err = put(db, 1, 1, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojTestAssert(watcher2->m_count == 0);
	err = put(db, 2, 2, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);
	err = put(db, 2, 2, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);

	return MojErrNone;
}

MojErr MojDbWatchTest::ltTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpLessThanEq, 45);
	MojTestErrCheck(err);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojRefCountedPtr<TestWatcher> watcher2(new TestWatcher);
	MojTestAssert(watcher2.get());
	MojDbQuery queryWithRev;
	err = queryWithRev.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("foo"), MojDbQuery::OpEq, 45);
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("_rev"), MojDbQuery::OpGreaterThan, m_rev);
	MojTestErrCheck(err);
	bool fired = false;
	err = db.watch(queryWithRev, cursor, watcher2->m_slot, fired);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(!fired);
	MojObject id;
	err = put(db, 46, 46, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojTestAssert(watcher2->m_count == 0);
	err = put(db, 45, 45, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);
	err = put(db, 45, 45, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);

	return MojErrNone;
}

MojErr MojDbWatchTest::cancelTest(MojDb& db)
{
	// cancel find
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpLessThanEq, 45);
	MojTestErrCheck(err);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	watcher->m_slot.cancel();
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	watcher->m_slot.cancel();
	MojTestAssert(watcher->m_count == 0);
	MojObject id;
	err = put(db, 1, 1, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	// cancel watch
	watcher.reset(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbQuery queryWithRev;
	err = queryWithRev.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("foo"), MojDbQuery::OpEq, 45);
	MojTestErrCheck(err);
	err = queryWithRev.where(_T("_rev"), MojDbQuery::OpGreaterThan, m_rev);
	MojTestErrCheck(err);
	bool fired = false;
	err = db.watch(queryWithRev, cursor, watcher->m_slot, fired);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(!fired);
	MojTestAssert(watcher->m_count == 0);
	watcher->m_slot.cancel();
	MojTestAssert(watcher->m_count == 0);
	err = put(db, 45, 45, id, m_rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);

	return MojErrNone;
}

MojErr MojDbWatchTest::rangeTest(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpGreaterThan, 5);
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpLessThan, 100);
	MojTestErrCheck(err);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	MojObject id;
	MojInt64 rev;
	err = put(db, 5, 5, id, rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	err = put(db, 100, 100, id, rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	err = put(db, 6, 6, id, rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);
	watcher.reset(new TestWatcher);
	MojTestAssert(watcher.get());
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);
	err = put(db, 99, 99, id, rev);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 1);

	return MojErrNone;
}

MojErr MojDbWatchTest::pageTest(MojDb& db)
{
	MojObject id;
	MojObject idFirst;
	MojObject idFourth;
	MojObject idLast;
	MojInt64 rev;
	for (int i = 100; i < 150; ++i) {
		MojErr err = put(db, 100, i, id, rev);
		MojTestErrCheck(err);
		if (i == 100) {
			idFirst = id;
		} else if (i == 103) {
			idFourth = id;
		} else if (i == 149) {
			idLast = id;
		}
	}
	MojDbQuery query;
	MojErr err = query.from(_T("WatchTest:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpGreaterThanEq, 100);
	MojTestErrCheck(err);
	query.limit(3);
	MojRefCountedPtr<TestWatcher> watcher(new TestWatcher);
	MojTestAssert(watcher.get());
	MojDbCursor cursor;
	err = db.find(query, cursor, watcher->m_slot);
	MojTestErrCheck(err);
	bool found = false;
	MojUInt32 count = 0;
	do {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (found)
			++count;
	} while (found);
	MojTestAssert(count == 3);
	MojDbQuery::Page page;
	err = cursor.nextPage(page);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	err = merge(db, idFourth, 53);
	MojTestErrCheck(err);
	MojTestAssert(watcher->m_count == 0);

	query.page(page);
	MojRefCountedPtr<TestWatcher> watcher2(new TestWatcher);
	MojTestAssert(watcher2.get());
	err = db.find(query, cursor, watcher2->m_slot);
	MojTestErrCheck(err);
	found = false;
	count = 0;
	do {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (found)
			++count;
	} while (found);
	MojTestAssert(count == 3);
	err = cursor.close();
	MojTestErrCheck(err);
	err = db.del(idFirst, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 0);
	err = db.del(idFourth, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	MojTestAssert(watcher->m_count == 1);
	MojTestAssert(watcher2->m_count == 1);

	// desc order
	query.page(MojDbQuery::Page());
	query.desc(true);
	MojRefCountedPtr<TestWatcher> watcher3(new TestWatcher);
	MojTestAssert(watcher3.get());
	err = db.find(query, cursor, watcher3->m_slot);
	MojTestErrCheck(err);

	found = false;
	count = 0;
	do {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (found)
			++count;
	} while (found);
	MojTestAssert(count == 3);
	err = cursor.close();
	MojTestErrCheck(err);

	err = merge(db, idLast, 53);
	MojTestErrCheck(err);
	MojTestAssert(watcher3->m_count == 1);

	MojRefCountedPtr<TestWatcher> watcher4(new TestWatcher);
	MojTestAssert(watcher4.get());
	err = db.find(query, cursor, watcher4->m_slot);
	MojTestErrCheck(err);

	found = false;
	count = 0;
	do {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (found)
			++count;
	} while (found);
	MojTestAssert(count == 3);

	err = cursor.close();
	MojTestErrCheck(err);
	err = merge(db, idLast, 54);
	MojTestErrCheck(err);
	MojTestAssert(watcher4->m_count == 1);

	return MojErrNone;
}

MojErr MojDbWatchTest::limitTest(MojDb& db)
{
	return MojErrNone;
}

MojErr MojDbWatchTest::put(MojDb& db, const MojObject& fooVal, const MojObject& barVal, MojObject& idOut, MojInt64& revOut)
{
	MojObject obj;
	MojErr err = obj.putString(_T("_kind"), _T("WatchTest:1"));
	MojTestErrCheck(err);
	err = obj.put(_T("foo"), fooVal);
	MojTestErrCheck(err);
	err = obj.put(_T("bar"), barVal);
	MojTestErrCheck(err);

	err = db.put(obj);
	MojTestErrCheck(err);

	err = obj.getRequired(MojDb::IdKey, idOut);
	MojTestErrCheck(err);
	err = obj.getRequired(MojDb::RevKey, revOut);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbWatchTest::merge(MojDb& db, const MojObject& id, const MojObject& barVal)
{
	MojObject obj;
	MojErr err = obj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	err = obj.put(_T("bar"), barVal);
	MojTestErrCheck(err);

	err = db.merge(obj);
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbWatchTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
