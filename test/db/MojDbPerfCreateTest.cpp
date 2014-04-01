/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include "db/MojDbStorageEngine.h"

#include "MojDbPerfCreateTest.h"
#include "db/MojDb.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else
    #error "Doesn't specified database type. See macro MOJ_USE_BDB and MOJ_USE_LDB"
#endif

static const MojUInt64 numInsert = 1000;
static const int numRepetitions = 5;

extern MojUInt64 allTestsTime;
static MojUInt64 totalTestTime = 0;
static MojFile file;
const MojChar* const CreateTestFileName = _T("MojDbPerfCreateTest.csv");

MojDbPerfCreateTest::MojDbPerfCreateTest()
: MojDbPerfTest(_T("MojDbPerfCreate"))
{
}

MojErr MojDbPerfCreateTest::run()
{
	MojErr err = file.open(CreateTestFileName, MOJ_O_RDWR | MOJ_O_CREAT | MOJ_O_TRUNC, MOJ_S_IRUSR | MOJ_S_IWUSR);

	MojString buf;
	err = buf.format("MojoDb Create Performance Test,,,,,\n\nOperation,Kind,Total Time,Time Per Iteration,Time Per Object\n");
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	err = testCreate();
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

MojErr MojDbPerfCreateTest::testCreate()
{
	//setup the test storage engine
#ifdef MOJ_USE_BDB
    MojDbStorageEngine::setEngineFactory (new MojDbBerkeleyFactory);
#elif MOJ_USE_LDB
    MojDbStorageEngine::setEngineFactory (new MojDbLevelFactory);
#else
    #error "Not defined engine type"
#endif
	MojDb db;

	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// time put kind
	MojUInt64 putKindTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putKinds(db, putKindTime);
		MojTestErrCheck(err);
		err = delKinds(db);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   putKind took: %llu nanosecs", (putKindTime / (numKinds * numRepetitions)));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("put Kind,all %llu kinds,%llu,%llu,%llu,\n", numKinds, putKindTime, putKindTime/numRepetitions, putKindTime / (numKinds * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	// insert objects with one index
	err = testInsertSmallObj(db, MojPerfSmKindId);
	MojTestErrCheck(err);
	err = testInsertMedObj(db, MojPerfMedKindId);
	MojTestErrCheck(err);
	err = testInsertLgObj(db, MojPerfLgKindId);
	MojTestErrCheck(err);
	err = testInsertMedNestedObj(db, MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = testInsertLgNestedObj(db, MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = testInsertMedArrayObj(db, MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = testInsertLgArrayObj(db, MojPerfLgArrayKindId);
	MojTestErrCheck(err);

	// insert objects with two indices
	err = testInsertSmallObj(db, MojPerfSmKind2Id);
	MojTestErrCheck(err);
	err = testInsertMedObj(db, MojPerfMedKind2Id);
	MojTestErrCheck(err);
	err = testInsertLgObj(db, MojPerfLgKind2Id);
	MojTestErrCheck(err);
	err = testInsertMedNestedObj(db, MojPerfMedNestedKind2Id);
	MojTestErrCheck(err);
	err = testInsertLgNestedObj(db, MojPerfLgNestedKind2Id);
	MojTestErrCheck(err);
	err = testInsertMedArrayObj(db, MojPerfMedArrayKind2Id);
	MojTestErrCheck(err);
	err = testInsertLgArrayObj(db, MojPerfLgArrayKind2Id);
	MojTestErrCheck(err);

	// batch insert with one index
	err = testBatchInsertLgObj(db, MojPerfLgKindId);
	MojTestErrCheck(err);
	err = testBatchInsertLgNestedObj(db, MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = testBatchInsertLgArrayObj(db, MojPerfLgArrayKindId);
	MojTestErrCheck(err);

	// batch insert with two indices
	err = testBatchInsertLgObj(db, MojPerfLgKind2Id);
	MojTestErrCheck(err);
	err = testBatchInsertLgNestedObj(db, MojPerfLgNestedKind2Id);
	MojTestErrCheck(err);
	err = testBatchInsertLgArrayObj(db, MojPerfLgArrayKind2Id);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertSmallObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with small objects (5 properties)
	MojUInt64 smallObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putSmallObj(db, kindId, smallObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, smallObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (smallObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, smallObjTime, smallObjTime/numRepetitions, smallObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with medium objects (10 properties)
	MojUInt64 medObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putMedObj(db, kindId, medObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, medObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (medObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, medObjTime, medObjTime/numRepetitions, medObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large objects (20 properties)
	MojUInt64 largeObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putLargeObj(db, kindId, largeObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, largeObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (largeObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, largeObjTime, largeObjTime/numRepetitions, largeObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with med nested objects (2 levels of nesting, 10 properties)
	MojUInt64 medNestedObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putMedNestedObj(db, kindId, medNestedObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, medNestedObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (medNestedObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, medNestedObjTime, medNestedObjTime/numRepetitions, medNestedObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large nested objects (3 levels of nesting, 20 properties)
	MojUInt64 largeNestedObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putLargeNestedObj(db, kindId, largeNestedObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s nested objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, largeNestedObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (largeNestedObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, largeNestedObjTime, largeNestedObjTime/numRepetitions, largeNestedObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);


	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with med array objects (10 properties + 1 array of 5 elements)
	MojUInt64 medArrayObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putMedArrayObj(db, kindId, medArrayObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count, MojDb::FlagPurge);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, medArrayObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (medArrayObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, medArrayObjTime, medArrayObjTime/numRepetitions, medArrayObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);


	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojUInt64 lgArrayObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putLargeArrayObj(db, kindId, lgArrayObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, lgArrayObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (lgArrayObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, lgArrayObjTime, lgArrayObjTime/numRepetitions, lgArrayObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojUInt64 batchLgObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = batchPutLargeObj(db, kindId, batchLgObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, batchLgObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu nanosecs", (batchLgObjTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (batchLgObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, batchLgObjTime, batchLgObjTime/numRepetitions, batchLgObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojUInt64 batchLgNestedObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putLargeArrayObj(db, kindId, batchLgNestedObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, batchLgNestedObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu nanosecs", (batchLgNestedObjTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (batchLgNestedObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, batchLgNestedObjTime, batchLgNestedObjTime/numRepetitions, batchLgNestedObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojUInt64 time = 0;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojUInt64 batchLgArrayObjTime = 0;
	for (int i = 0; i < numRepetitions; i++) {
		err = putLargeArrayObj(db, kindId, batchLgArrayObjTime);
		MojTestErrCheck(err);
		MojDbQuery q;
		err = q.from(kindId);
		MojTestErrCheck(err);
		MojUInt32 count = 0;
		err = db.del(q, count);
		MojTestErrCheck(err);
	}

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu nanosecs\n", numInsert, kindId, numRepetitions, batchLgArrayObjTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu nanosecs", (batchLgArrayObjTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu nanosecs", (batchLgArrayObjTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, batchLgArrayObjTime, batchLgArrayObjTime/numRepetitions, batchLgArrayObjTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putSmallObj(MojDb& db, const MojChar* kindId, MojUInt64& smallObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createSmallObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		smallObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedObj(MojDb& db, const MojChar* kindId, MojUInt64& medObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		medObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeObj(MojDb& db, const MojChar* kindId, MojUInt64& largeObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		largeObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeObj(MojDb& db, const MojChar* kindId, MojUInt64& largeObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	MojObject objArray;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeObj(obj, i);
		MojTestErrCheck(err);
		err = objArray.push(obj);
		MojTestErrCheck(err);
	}

	MojObject::ArrayIterator begin;
	MojErr err = objArray.arrayBegin(begin);
	MojTestErrCheck(err);
	MojObject::ConstArrayIterator end = objArray.arrayEnd();
	clock_gettime(CLOCK_REALTIME, &startTime);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	clock_gettime(CLOCK_REALTIME, &endTime);
	largeObjTime += timeDiff(startTime, endTime);
	totalTestTime += timeDiff(startTime, endTime);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedNestedObj(MojDb& db, const MojChar* kindId, MojUInt64& medNestedObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedNestedObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		medNestedObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeNestedObj(MojDb& db, const MojChar* kindId, MojUInt64& lgNestedObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeNestedObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		lgNestedObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeNestedObj(MojDb& db, const MojChar* kindId, MojUInt64& lgNestedObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	MojObject objArray;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeNestedObj(obj, i);
		MojTestErrCheck(err);
		err = objArray.push(obj);
		MojTestErrCheck(err);
	}

	MojObject::ArrayIterator begin;
	MojErr err = objArray.arrayBegin(begin);
	MojTestErrCheck(err);
	MojObject::ConstArrayIterator end = objArray.arrayEnd();
	clock_gettime(CLOCK_REALTIME, &startTime);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	clock_gettime(CLOCK_REALTIME, &endTime);
	lgNestedObjTime += timeDiff(startTime, endTime);
	totalTestTime += timeDiff(startTime, endTime);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedArrayObj(MojDb& db, const MojChar* kindId, MojUInt64& medArrayObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedArrayObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		medArrayObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeArrayObj(MojDb& db, const MojChar* kindId, MojUInt64& lgArrayObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeArrayObj(obj, i);
		MojTestErrCheck(err);

		clock_gettime(CLOCK_REALTIME, &startTime);
		err = db.put(obj);
		MojTestErrCheck(err);
		clock_gettime(CLOCK_REALTIME, &endTime);
		lgArrayObjTime += timeDiff(startTime, endTime);
		totalTestTime += timeDiff(startTime, endTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeArrayObj(MojDb& db, const MojChar* kindId, MojUInt64& lgArrayObjTime)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;

	MojObject objArray;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeArrayObj(obj, i);
		MojTestErrCheck(err);
		err = objArray.push(obj);
		MojTestErrCheck(err);
	}

	MojObject::ArrayIterator begin;
	MojErr err = objArray.arrayBegin(begin);
	MojTestErrCheck(err);
	MojObject::ConstArrayIterator end = objArray.arrayEnd();
	clock_gettime(CLOCK_REALTIME, &startTime);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	clock_gettime(CLOCK_REALTIME, &endTime);
	lgArrayObjTime += timeDiff(startTime, endTime);
	totalTestTime += timeDiff(startTime, endTime);

	return MojErrNone;
}

void MojDbPerfCreateTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
