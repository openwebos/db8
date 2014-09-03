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


#include "MojDbPerfUpdateTest.h"
#include "db/MojDb.h"

static const MojUInt64 numInsertForPut = 1000;
static const MojUInt64 numPutIterations = 1000;
static const MojUInt64 numBatchPutIterations = 100;
static const MojUInt64 numMergeIterations = 1000;
static const MojUInt64 numBatchMergeIterations = 100;
static const MojUInt64 numObjectsBeforeUpdateKind = 1000;
static const MojUInt64 numUpdateKindIterations = 50;

extern MojUInt64 allTestsTime;
const MojChar* const UpdateTestFileName = _T("MojDbPerfUpdateTest.csv");
static MojUInt64 totalTestTime = 0;
static MojFile file;

MojDbPerfUpdateTest::MojDbPerfUpdateTest()
: MojDbPerfTest(_T("MojDbPerfUpdate"))
{
}

MojErr MojDbPerfUpdateTest::run()
{
	MojErr err = file.open(UpdateTestFileName, MOJ_O_RDWR | MOJ_O_CREAT | MOJ_O_TRUNC, MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("MojoDb Update Performance Test,,,,,\n\nOperation,Kind,Total Time,Time Per Iteration,Time Per Object\n");
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

	err = testPut(db);
	MojTestErrCheck(err);
	err = testMerge(db);
	MojTestErrCheck(err);
	err = testUpdateKind(db);
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

	err = db.close();
	MojTestErrCheck(err);

	err = file.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::testUpdateKind(MojDb& db)
{
	MojString buf;
	MojErr err = buf.format("\nUPDATE KIND,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = updateKind(db, MojPerfSmKindId, MojPerfSmKindStr, MojPerfSmKindExtraIndex, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfMedKindId, MojPerfMedKindStr, MojPerfMedKindExtraIndex, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfLgKindId, MojPerfLgKindStr, MojPerfLgKindExtraIndex, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfMedNestedKindId, MojPerfMedNestedKindStr, MojPerfMedNestedKindExtraIndex, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfLgNestedKindId, MojPerfLgNestedKindStr, MojPerfLgNestedKindExtraIndex, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfMedArrayKindId, MojPerfMedArrayKindStr, MojPerfMedArrayKindExtraIndex, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = updateKind(db, MojPerfLgArrayKindId, MojPerfLgArrayKindStr, MojPerfLgArrayKindExtraIndex, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::testMerge(MojDb& db)
{
	MojString buf;
	MojErr err = buf.format("\nUPDATE VIA MERGE,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = updateObjsViaMerge(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = updateObjsViaMerge(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::testPut(MojDb& db)
{
	MojString buf;
	MojErr err = buf.format("\nUPDATE VIA PUT,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = updateObjsViaPut(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	err = updateObjsViaPut(db, MojPerfSmKind2Id, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedKind2Id, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgKind2Id, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedNestedKind2Id, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgNestedKind2Id, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfMedArrayKind2Id, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = updateObjsViaPut(db, MojPerfLgArrayKind2Id, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::updateKind(MojDb& db, const MojChar* kindId, const MojChar* kindJson, const MojChar* extraIdxJson, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64))
{
	// register all the kinds
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn
	MojObject objs;
	err = putObjs(db, kindId, numObjectsBeforeUpdateKind, createFn, objs);
	MojTestErrCheck(err);

	// add an index
	MojObject kindObj;
	err = kindObj.fromJson(kindJson);
	MojTestErrCheck(err);
	MojObject indexes;
	kindObj.get(_T("indexes"), indexes);
	MojTestErrCheck(err);

	MojObject extraIdx;
	err = extraIdx.fromJson(extraIdxJson);
	MojTestErrCheck(err);
	indexes.push(extraIdx);

	err = kindObj.put(_T("indexes"), indexes);
	MojTestErrCheck(err);

	MojUInt64 addIndexTime = 0;
	MojUInt64 dropIndexTime = 0;
	err = timeUpdateKind(db, kindJson, kindObj, addIndexTime, dropIndexTime);
	MojTestErrCheck(err);

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   updating kind %s - adding index %s %llu times took: %llu nanosecs\n", kindId, extraIdxJson, numUpdateKindIterations, addIndexTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per add/reindex: %llu nanosecs\n", (addIndexTime) / (numUpdateKindIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   updating kind %s - dropping index %s %llu times took: %llu nanosecs\n", kindId, extraIdxJson, numUpdateKindIterations, dropIndexTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per drop: %llu nanosecs", (dropIndexTime) / (numUpdateKindIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("Updating kind %s - adding index %s %llu times,%s,%llu,%llu,%llu,\nUpdating kind %s - dropping index %s %llu times,%s,%llu,%llu,%llu,\n",
			kindId, extraIdxJson, numUpdateKindIterations, kindId, addIndexTime, addIndexTime/numUpdateKindIterations, addIndexTime/(1*numUpdateKindIterations),
			kindId, extraIdxJson, numUpdateKindIterations, kindId, dropIndexTime, dropIndexTime/numUpdateKindIterations, dropIndexTime/(1*numUpdateKindIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::timeUpdateKind(MojDb& db, const MojChar* kindJson, MojObject& kindObj, MojUInt64& addIndexTime, MojUInt64& dropIndexTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;
	MojObject origKindObj;
	MojErr err = origKindObj.fromJson(kindJson);
	MojTestErrCheck(err);

	for (MojUInt64 i = 0; i < numUpdateKindIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		bool found;
		err = kindObj.del(MojDb::RevKey, found);
		MojTestErrCheck(err);
		err = db.putKind(kindObj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		addIndexTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = origKindObj.del(MojDb::RevKey, found);
		MojTestErrCheck(err);
		err = db.putKind(origKindObj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		dropIndexTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::updateObjsViaMerge(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64))
{
	// register all the kinds
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn
	MojObject objs;
	err = putObjs(db, kindId, numInsertForPut, createFn, objs);
	MojTestErrCheck(err);

	MojObject midObj;
	bool found = objs.at(numInsertForPut/2, midObj);
	MojTestAssert(found);

	MojUInt64 objTime = 0;
	err = mergeObj(db, midObj, objTime);
	MojTestErrCheck(err);
	MojUInt64 mergeTime = objTime;
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   merging single object - index %llu - of kind %s %llu times took: %llu nanosecs\n", numInsertForPut/2, kindId, numMergeIterations, mergeTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per merge: %llu nanosecs", (mergeTime) / (numMergeIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("Merge single object - index %llu - %llu times,%s,%llu,%llu,%llu,\n", numInsertForPut/2, numMergeIterations, kindId, mergeTime, mergeTime/numMergeIterations, mergeTime/(1*numMergeIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 batchTime = 0;
	MojObject::ArrayIterator beginArr;
	err = objs.arrayBegin(beginArr);
	MojErrCheck(err);
	err = batchMergeObj(db, beginArr, beginArr + (numInsertForPut / 10), batchTime);
	MojTestErrCheck(err);
	mergeTime = batchTime;
	err = MojPrintF("   merging batch - %llu objects - of kind %s %llu times took: %llu nanosecs\n", numInsertForPut/10, kindId, numBatchMergeIterations, mergeTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch merge: %llu nanosecs\n", (mergeTime) / (numBatchMergeIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (mergeTime) / (numInsertForPut/10 * numBatchMergeIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Batch merge %llu objects %llu times,%s,%llu,%llu,%llu,\n", numInsertForPut/10, numBatchMergeIterations, kindId, mergeTime, mergeTime/numBatchMergeIterations, mergeTime/(numInsertForPut/10*numBatchMergeIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 mergeQueryTime = 0;
	MojTestErrCheck(err);
	MojDbQuery query;
	err = query.from(kindId);
	MojTestErrCheck(err);
	query.limit(numInsertForPut/5);
	query.desc(true);

	MojObject queryObj;
	err = query.toObject(queryObj);
	MojTestErrCheck(err);
	MojString queryStr;
	err = queryObj.stringValue(queryStr);
	MojTestErrCheck(err);

	MojObject props;
	err = props.putString(_T("newKey"), _T("here's a new value"));
	MojTestErrCheck(err);
	MojUInt32 count;
	err = queryMergeObj(db, query, props, count, mergeQueryTime);
	MojTestErrCheck(err);
	mergeTime = mergeQueryTime;
	err = MojPrintF("   merging with query - %d objects - of kind %s %llu times took: %llu nanosecs\n", count, kindId, numBatchMergeIterations, mergeTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per merge: %llu nanosecs\n", (mergeTime) / (numBatchMergeIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (mergeTime) / (count * numBatchMergeIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Merge with query %s,,,,,\nMerge with query - %d objects - %llu times,%s,%llu,%llu,%llu,\n", queryStr.data(), count, numBatchMergeIterations, kindId, mergeTime, mergeTime/numBatchMergeIterations, mergeTime/(count*numBatchMergeIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::mergeObj(MojDb& db, MojObject& obj, MojUInt64& objTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numMergeIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojErr err = obj.putInt(_T("newProp"), i);
		MojTestErrCheck(err);
		err = db.merge(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::batchMergeObj(MojDb& db, MojObject* begin, const MojObject* end, MojUInt64& objTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numBatchMergeIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojErr err = db.merge(begin, end);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::queryMergeObj(MojDb& db, MojDbQuery& query, MojObject& props, MojUInt32& count, MojUInt64& objTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numBatchMergeIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojErr err = db.merge(query, props, count);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::updateObjsViaPut(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64))
{
	// register all the kinds
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn
	MojObject objs;
	err = putObjs(db, kindId, numInsertForPut, createFn, objs);
	MojTestErrCheck(err);

	MojObject midObj;
	bool found = objs.at(numInsertForPut/2, midObj);
	MojTestAssert(found);

	MojUInt64 objTime = 0;
	err = putObj(db, midObj, objTime);
	MojTestErrCheck(err);
	MojUInt64 putTime = objTime;
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   putting single object - index %llu - of kind %s %llu times took: %llu nanosecs\n", numInsertForPut/2, kindId, numPutIterations, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per put: %llu nanosecs", (putTime) / (numPutIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("Put single object - index %llu - %llu times,%s,%llu,%llu,%llu,\n", numInsertForPut/2, numPutIterations, kindId, putTime, putTime/numPutIterations, putTime/(1*numPutIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	MojUInt64 batchTime = 0;
	MojObject::ArrayIterator beginArr;
	err = objs.arrayBegin(beginArr);
	MojErrCheck(err);
	err = batchPutObj(db, beginArr, beginArr + (numInsertForPut / 10), batchTime);
	putTime = batchTime;
	MojTestErrCheck(err);
	err = MojPrintF("   putting batch - %llu objects - of kind %s %llu times took: %llu nanosecs\n", numInsertForPut/10, kindId, numBatchPutIterations, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch put: %llu nanosecs\n", (putTime) / (numBatchPutIterations));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (putTime) / (numInsertForPut/10 * numBatchPutIterations));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	err = buf.format("Batch put %llu objects %llu times,%s,%llu,%llu,%llu,\n", numInsertForPut/10, numBatchPutIterations, kindId, putTime, putTime/numBatchPutIterations, putTime/(numInsertForPut/10*numBatchPutIterations));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::putObj(MojDb& db, MojObject& obj, MojUInt64& objTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numPutIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojErr err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::batchPutObj(MojDb& db, MojObject* begin, const MojObject* end, MojUInt64& objTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numBatchPutIterations; i++) {
		clock_gettime(CLOCK_REALTIME, &startTime);
		MojErr err = db.put(begin, end);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		objTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfUpdateTest::putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
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

void MojDbPerfUpdateTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
