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

#include "MojDbPerfDeleteTest.h"
#include "db/MojDb.h"
#include "core/MojJson.h"

static const MojUInt64 numInsertForDel = 1000;
static const MojUInt64 numSingleDelIterations = 1000;
static const MojUInt64 numBatchPutIterations = 100;
static const MojUInt64 numMergeIterations = 1000;
static const MojUInt64 numBatchDelIterations = 100;
static const MojUInt64 numObjectsBeforeUpdateKind = 1000;
static const MojUInt64 numUpdateKindIterations = 50;

extern MojUInt64 allTestsTime;
const MojChar* const DeleteTestFileName = _T("MojDbPerfDeleteTest.csv");
static MojUInt64 totalTestTime = 0;
static MojFile file;

MojDbPerfDeleteTest::MojDbPerfDeleteTest()
: MojDbPerfTest(_T("MojDbPerfDelete"))
{
}

MojErr MojDbPerfDeleteTest::run()
{
	MojErr err = file.open(DeleteTestFileName, MOJ_O_RDWR | MOJ_O_CREAT | MOJ_O_TRUNC, MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("MojoDb Delete Performance Test,,,,,\n\nOperation,Kind,Total Time,Time Per Iteration,Time Per Object\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = testDelete();
	MojTestErrCheck(err);
	allTestsTime += totalTestTime;

	err = MojPrintF("\n\n TOTAL TEST TIME: %llu nanoseconds. | %10.3f seconds.\n\n", totalTestTime, totalTestTime / 1000000000.0f);
	MojTestErrCheck(err);
	err = MojPrintF("\n-------\n");
	MojTestErrCheck(err);

	err = buf.format("\n\nTOTAL TEST TIME,,%llu,,,", totalTestTime);
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = file.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::testDelete()
{
	MojString buf;
	MojErr err = buf.format("\nDELETE,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojDb db;

    if (lazySync())
    {
        err=db.configure(lazySyncConfig());
        MojTestErrCheck(err);
    }

	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	err = delObjects(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	err = delObjects(db, MojPerfSmKind2Id, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedKind2Id, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgKind2Id, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedNestedKind2Id, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgNestedKind2Id, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfMedArrayKind2Id, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = delObjects(db, MojPerfLgArrayKind2Id, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::delObjects(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64))
{
	// register all the kinds
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn
	MojObject objs;
	err = putObjs(db, kindId, numInsertForDel, createFn, objs);
	MojTestErrCheck(err);

	MojObject midId;
	bool found = objs.at(numInsertForDel/2, midId);
	MojTestAssert(found);

	MojUInt64 objTime = 0;
	err = delObj(db, midId, objTime);
	MojTestErrCheck(err);
	MojUInt64 delTime = objTime;
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   deleting single object - index %llu - of kind %s %llu times took: %llu nanosecs\n", numInsertForDel/2, kindId, numSingleDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per delete: %llu nanosecs", (delTime) / (numSingleDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("Delete single object - index %llu - %llu times,%s,%llu,%llu,%llu,\n", numInsertForDel/2, numSingleDelIterations, kindId, delTime, delTime/numSingleDelIterations, delTime/(1*numSingleDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojObject midId2;
	found = objs.at(numInsertForDel/2, midId2);
	MojTestAssert(found);

	MojUInt64 objTime2 = 0;
	err = delObj(db, midId2, objTime2, MojDb::FlagPurge);
	MojTestErrCheck(err);
	delTime = objTime2;
	err = MojPrintF("   deleting single object with purge flag - index %llu - of kind %s %llu times took: %llu nanosecs\n", numInsertForDel/2, kindId, numSingleDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per delete: %llu nanosecs", (delTime) / (numSingleDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Delete single object with purge flag - index %llu - %llu times,%s,%llu,%llu,%llu,\n", numInsertForDel/2 - 1, numSingleDelIterations, kindId, delTime, delTime/numSingleDelIterations, delTime/(1*numSingleDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 batchTime = 0;
	MojObject::ArrayIterator beginArr;
	err = objs.arrayBegin(beginArr);
	MojTestErrCheck(err);
	MojUInt32 count;
	err = batchDelObj(db, beginArr, beginArr + 20, count, batchTime);
	MojTestErrCheck(err);
	delTime = batchTime;
	err = MojPrintF("   deleting batch - %d objects - of kind %s %llu times took: %llu nanosecs\n", count, kindId, numBatchDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch merge: %llu nanosecs\n", (delTime) / (numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (delTime) / (count * numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Batch del %d objects %llu times,%s,%llu,%llu,%llu,\n", count, numBatchDelIterations, kindId, delTime, delTime/numBatchDelIterations, delTime/(count*numBatchDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 batchTime2 = 0;
	count = 0;
	err = batchDelObj(db, beginArr, beginArr + 20, count, batchTime2, MojDb::FlagPurge);
	MojTestErrCheck(err);
	delTime = batchTime2;
	err = MojPrintF("   deleting batch - %d objects - with purge flag of kind %s %llu times took: %llu nanosecs\n", count, kindId, numBatchDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch merge: %llu nanosecs\n", (delTime) / (numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (delTime) / (count * numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Batch del %d objects with purge flag %llu times,%s,%llu,%llu,%llu,\n", count, numBatchDelIterations, kindId, delTime, delTime/numBatchDelIterations, delTime/(count*numBatchDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 delQueryTime = 0;
	MojTestErrCheck(err);
	MojDbQuery query;
	err = query.from(kindId);
	MojTestErrCheck(err);
	query.limit(20);
	query.desc(true);

	MojObject queryObj;
	err = query.toObject(queryObj);
	MojTestErrCheck(err);
	MojString queryStr;
	err = queryObj.stringValue(queryStr);
	MojTestErrCheck(err);

	count = 0;
	err = queryDelObj(db, query, count, delQueryTime);
	MojTestErrCheck(err);
	delTime = delQueryTime;
	err = MojPrintF("   deleting with query - %d objects - of kind %s %llu times took: %llu nanosecs\n", count, kindId, numBatchDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per merge: %llu nanosecs\n", (delTime) / (numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (delTime) / (count * numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Deleting by query %s,,,,,\nDelete %d objects by query %llu times,%s,%llu,%llu,%llu,\n", queryStr.data(), count, numBatchDelIterations, kindId, delTime, delTime/numBatchDelIterations, delTime/(count*numBatchDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 delQueryTime2 = 0;
	count = 0;
	err = queryDelObj(db, query, count, delQueryTime2, MojDb::FlagPurge);
	MojTestErrCheck(err);
	delTime = delQueryTime2;
	err = MojPrintF("   deleting with query - %d objects - with purge flag of kind %s %llu times took: %llu nanosecs\n", count, kindId, numBatchDelIterations, delTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per merge: %llu nanosecs\n", (delTime) / (numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (delTime) / (count * numBatchDelIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Deleting by query with purge flag %s,,,,,\nDelete %d objects by query with purge flag %llu times,%s,%llu,%llu,%llu,\n\n", queryStr.data(), count, numBatchDelIterations, kindId, delTime, delTime/numBatchDelIterations, delTime/(count*numBatchDelIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::delObj(MojDb& db, MojObject& obj, MojUInt64& objTime, MojUInt32 flags)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numSingleDelIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojObject id;
		MojErr err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		bool found = false;
		err = db.del(id, found, flags);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);

		//put the object back
		err = obj.del(MojDb::RevKey, found);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::batchDelObj(MojDb& db, MojObject* begin, const MojObject* end, MojUInt32& countOut, MojUInt64& objTime, MojUInt32 flags)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	MojObject ids;
	for (const MojObject* i = begin; i != end; ++i) {
		MojObject id;
		MojErr err = i->getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		err = ids.push(id);
		MojTestErrCheck(err);
	}

	for (MojUInt64 i = 0; i < numBatchDelIterations; ++i) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		countOut = 0;
		MojObject delObjs;
		MojErr err = db.del(ids.arrayBegin(), ids.arrayEnd(), countOut, delObjs, flags);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);

		//put the objects back
		for (MojObject* i = begin; i != end; ++i) {
			bool found;
			err = i->del(MojDb::RevKey, found);
			MojTestErrCheck(err);
		}
		err = db.put(begin, end);
		MojTestErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::queryDelObj(MojDb& db, MojDbQuery& query, MojUInt32& countOut, MojUInt64& objTime, MojUInt32 flags)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	//find all the objects described by the query
	MojObject objs;
	MojDbCursor cursor;
	MojErr err = db.find(query, cursor);
	MojTestErrCheck(err);
	MojJsonWriter writer;
	err = writer.beginArray();
	MojTestErrCheck(err);
	err = cursor.visit(writer);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = writer.endArray();
	MojTestErrCheck(err);
	MojString objStr = writer.json();
	err = objs.fromJson(objStr);
	MojTestErrCheck(err);


	for (MojUInt64 i = 0; i < numBatchDelIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		countOut = 0;
		MojErr err = db.del(query, countOut, flags);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);

		//put everything back
		MojObject::ArrayIterator begin;
		err = objs.arrayBegin(begin);
		MojTestErrCheck(err);
		MojObject::ConstArrayIterator end = objs.arrayEnd();
		for (MojObject* i = begin; i != end; ++i) {
			bool found;
			err = i->del(MojDb::RevKey, found);
			MojTestErrCheck(err);
		}
		err = db.put(begin, end);
		MojTestErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojDbPerfDeleteTest::putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
		MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64), MojObject& objs)
{
	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = (*this.*createFn)(obj, i);
		MojTestErrCheck(err);

		err = db.put(obj);
		MojTestErrCheck(err);

		err = objs.push(obj);
		MojTestErrCheck(err);
	}

	return MojErrNone;
}

void MojDbPerfDeleteTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
