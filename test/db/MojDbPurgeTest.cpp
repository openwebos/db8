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


#include "MojDbPurgeTest.h"
#include "db/MojDb.h"
#include "core/MojTime.h"

static const MojChar* const MojKindStr =
	_T("{\"id\":\"PurgeTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"myId\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true}],")
	_T("\"schema\":{\"type\":\"object\"}}");
static const MojChar* const MojTestObjStr1 =
	_T("{\"_kind\":\"PurgeTest:1\",\"foo\":1,\"bar\":2}");


MojDbPurgeTest::MojDbPurgeTest()
: MojTestCase(_T("MojDbPurge"))
{
}

MojErr MojDbPurgeTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put type
	MojObject obj;
	err = obj.fromJson(MojKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	MojObject revNums[10];
	MojObject ids[10];
	//put 10 objects in the db
	for(int i = 0; i < 10; i++) {
		err = obj.fromJson(MojTestObjStr1);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		// get _rev and id
		MojObject rev;
		err = obj.getRequired(MojDb::RevKey, rev);
		MojTestErrCheck(err);
		revNums[i] = rev;
		MojObject id;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		ids[i] = id;
	}

	//purge now, there are no RevTimestamp entries
	MojUInt32 count = 0;
	err = db.purge(count, 30);
	MojTestErrCheck(err);

	err = checkObjectsPurged(db, count, 0, 10, 1, -1);
	MojTestErrCheck(err);

	//create a RevTimestamp entry - that's not more than PurgeNumDays days ago
	MojTime time;
	err = MojGetCurrentTime(time);
	MojTestErrCheck(err);
	err = createRevTimestamp(db, revNums[0], time.microsecs());
	MojTestErrCheck(err);

	//purge now, there are no RevTimestamp entries that are more than
	//PurgeNumDays ago, so nothing should be purged
	count = 0;
	err = db.purge(count, 30);
	MojTestErrCheck(err);

	err = checkObjectsPurged(db, count, 0, 10, 3, -1);
	MojTestErrCheck(err);

	//create a RevTimestamp entry for more than PurgeNumDays days ago
	err = MojGetCurrentTime(time);
	MojTestErrCheck(err);
	err = createRevTimestamp(db, revNums[9], time.microsecs() - (((MojInt64)40) * MojTime::UnitsPerDay));
	MojTestErrCheck(err);

	//purge now, since nothing has been deleted, nothing should be purged,
	//but the RevTimestamp object should be deleted
	count = 0;
	err = db.purge(count, 30);
	MojTestErrCheck(err);

	err = checkObjectsPurged(db, count, 0, 10, 4, -1);
	MojTestErrCheck(err);

	//delete something - this will set its revision number higher
	bool found;
	err = db.del(ids[0], found);
	MojTestErrCheck(err);
	MojTestAssert(found == true);

	//purge now, since nothing has been deleted prior to the revision
	//number, nothing should be purged
	count = 0;
	err = db.purge(count, 30);
	MojTestErrCheck(err);

	err = checkObjectsPurged(db, count, 0, 9, 5, -1);
	MojTestErrCheck(err);

	//delete another object
	err = db.del(ids[1], found);
	MojTestErrCheck(err);
	MojTestAssert(found == true);

	//create a RevTimestamp entry for more than PurgeNumDays days ago,
	//with the rev number of the 1st obj we deleted
	MojDbQuery query;
	err = query.from(_T("PurgeTest:1"));
	MojTestErrCheck(err);
	err = query.where(MojDb::IdKey, MojDbQuery::OpEq, ids[0]);
	MojTestErrCheck(err);
	err = query.includeDeleted();
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	MojObject objFromDb;
	err = cursor.get(objFromDb, found);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(found == true);

	MojObject revFromDb;
	err = objFromDb.getRequired(MojDb::RevKey, revFromDb);
	MojTestErrCheck(err);

	err = MojGetCurrentTime(time);
	MojTestErrCheck(err);
	err = createRevTimestamp(db, revFromDb, time.microsecs() - (((MojInt64)35) * MojTime::UnitsPerDay));
	MojTestErrCheck(err);

	//now purge, only id[0] should be purged
	count = 0;
	err = db.purge(count, 30);
	MojTestErrCheck(err);

	err = checkObjectsPurged(db, count, 1, 8, 6, revFromDb);
	MojTestErrCheck(err);

	//TODO 2.12.10 - this test does not pass yet, we need to fix calling delKind after a purge
	//err = delKindTest(db);
	//MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPurgeTest::delKindTest(MojDb& db)
{

	// start from scratch - purge everything
	MojUInt32 count = 0;
	MojErr err = db.purge(count, 0);
	MojTestErrCheck(err);
	MojTestAssert(count > 0);

	// purge again, make sure nothing is left
	err = db.purge(count, 0);
	MojTestErrCheck(err);
	MojTestAssert(count == 0);

	// make sure at least 2 objects exist
	MojDbQuery q;
	err = q.from(_T("PurgeTest:1"));
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(q, cursor);
	MojTestErrCheck(err);

	MojUInt32 objCount;
	err = cursor.count(objCount);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	if (objCount <= 1) {
		for (int i = objCount; i < 2; i++) {
			MojObject obj;
			err = obj.fromJson(MojTestObjStr1);
			MojTestErrCheck(err);
			err = db.put(obj);
			MojTestErrCheck(err);
			objCount++;
		}
	}

	// delete half the objects
	q.limit(objCount / 2);

	MojUInt32 delCount;
	err = db.del(q, delCount);
	MojTestErrCheck(err);

	// delete the kind
	MojString idStr;
	err = idStr.assign(_T("PurgeTest:1"));
	MojTestErrCheck(err);
	MojObject id(idStr);
	bool found;
	err = db.delKind(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	/*MojUInt32 deletedObjCount;
	err = q.where(_T("_del"), MojDbQuery::OpEq, true);
	MojTestErrCheck(err);
	q.includeDeleted(true);
	err = db.find(q, cursor);
	MojTestErrCheck(err);
	err = cursor.count(deletedObjCount);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	// now all the objects should be deleted
	MojTestAssert(deletedObjCount == objCount);*/

	// purge now
	err = db.purge(count, 0);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPurgeTest::checkObjectsPurged(MojDb& db, const MojUInt32& count, const MojSize& expectedCount,
		const MojSize& expectedNumObjects, const MojSize& expectedNumRevTimestampObjects, const MojObject& expectedLastPurgeRevNum)
{
	//check number of objects purged
	MojTestAssert(count == expectedCount);

	//there should still be expectedNumObjects test objects
	MojDbQuery query;
	MojErr err = query.from(_T("PurgeTest:1"));
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	MojUInt32 objCount;
	err = cursor.count(objCount);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	MojTestAssert(objCount == expectedNumObjects);

	//there should be expectedNumRevTimestampObjects RevTimestamp objects
	MojDbQuery revQuery;
	err = revQuery.from(MojDbKindEngine::RevTimestampId);
	MojTestErrCheck(err);

	MojDbCursor revCursor;
	err = db.find(revQuery, revCursor);
	MojTestErrCheck(err);

	MojUInt32 revTimestampObjCount;
	err = revCursor.count(revTimestampObjCount);
	MojTestErrCheck(err);
	err = revCursor.close();
	MojTestErrCheck(err);

	MojTestAssert(revTimestampObjCount == expectedNumRevTimestampObjects);

	//lastPurgedRevNum should be equal to the expectedLastPurgeRevNum
	MojObject revNum;
	err = db.purgeStatus(revNum);
	MojTestErrCheck(err);

	MojTestAssert(revNum == expectedLastPurgeRevNum);

	return MojErrNone;
}

MojErr MojDbPurgeTest::createRevTimestamp(MojDb& db, MojObject& revNum, MojInt64 timestamp)
{
	MojObject revTimeObj;
	MojErr err = revTimeObj.put(_T("rev"), revNum);
	MojTestErrCheck(err);
	err = revTimeObj.put(_T("timestamp"), timestamp);
	MojTestErrCheck(err);
	err = revTimeObj.putString(MojDb::KindKey, MojDbKindEngine::RevTimestampId);
	MojTestErrCheck(err);

	err = db.put(revTimeObj);
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbPurgeTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

