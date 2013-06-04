/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics
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
#include "core/MojTime.h"

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

static MojTime totalTestTime;
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

	err = MojPrintF("\n\n TOTAL TEST TIME: %llu microseconds\n\n", totalTestTime.microsecs());
	MojTestErrCheck(err);
	err = MojPrintF("\n-------\n");
	MojTestErrCheck(err);

	err = buf.format("\n\nTOTAL TEST TIME,,%llu,,,", totalTestTime.microsecs());
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
	MojTime putKindTime;
	for (int i = 0; i < numRepetitions; i++) {
		err = putKinds(db, putKindTime);
		MojTestErrCheck(err);
		err = delKinds(db);
		MojTestErrCheck(err);
	}

	MojUInt64 putKind = putKindTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   putKind took: %llu microsecs", (putKind / (numKinds * numRepetitions)));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString buf;
	err = buf.format("put Kind,all %llu kinds,%llu,%llu,%llu,\n", numKinds, putKind, putKind/numRepetitions, putKind / (numKinds * numRepetitions));
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
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with small objects (5 properties)
	MojTime smallObjTime;
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

	MojUInt64 putTime = smallObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with medium objects (10 properties)
	MojTime medObjTime;
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

	MojUInt64 putTime = medObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large objects (20 properties)
	MojTime largeObjTime;
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

	MojUInt64 putTime = largeObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with med nested objects (2 levels of nesting, 10 properties)
	MojTime medNestedObjTime;
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

	MojUInt64 putTime = medNestedObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large nested objects (3 levels of nesting, 20 properties)
	MojTime largeNestedObjTime;
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

	MojUInt64 putTime = largeNestedObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s nested objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);


	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertMedArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with med array objects (10 properties + 1 array of 5 elements)
	MojTime medArrayObjTime;
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

	MojUInt64 putTime = medArrayObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);


	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testInsertLgArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojTime lgArrayObjTime = 0;
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

	MojUInt64 putTime = lgArrayObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojTime batchLgObjTime = 0;
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

	MojUInt64 putTime = batchLgObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu microsecs", (putTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgNestedObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojTime batchLgNestedObjTime = 0;
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

	MojUInt64 putTime = batchLgNestedObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu microsecs", (putTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::testBatchInsertLgArrayObj(MojDb& db, const MojChar* kindId)
{
	// register all the kinds again
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// time put with large array objects (20 properties + 2 arrays of 5 elements each)
	MojTime batchLgArrayObjTime = 0;
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

	MojUInt64 putTime = batchLgArrayObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to batch put %llu %s objects %d times: %llu microsecs\n", numInsert, kindId, numRepetitions, putTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per batch: %llu microsecs", (putTime) / (numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (putTime) / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString buf;
	err = buf.format("batch put %llu objects %d times,%s,%llu,%llu,%llu,\n", numInsert, numRepetitions, kindId, putTime, putTime/numRepetitions, putTime / (numInsert * numRepetitions));
	MojTestErrCheck(err);
	err = fileWrite(file, buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putSmallObj(MojDb& db, const MojChar* kindId, MojTime& smallObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createSmallObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		smallObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedObj(MojDb& db, const MojChar* kindId, MojTime& medObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		medObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeObj(MojDb& db, const MojChar* kindId, MojTime& largeObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		largeObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeObj(MojDb& db, const MojChar* kindId, MojTime& largeObjTime)
{
	MojTime startTime;
	MojTime endTime;

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
	err = MojGetCurrentTime(startTime);
	MojTestErrCheck(err);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	err = MojGetCurrentTime(endTime);
	MojTestErrCheck(err);
	largeObjTime += (endTime - startTime);
	totalTestTime += (endTime - startTime);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedNestedObj(MojDb& db, const MojChar* kindId, MojTime& medNestedObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedNestedObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		medNestedObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeNestedObj(MojDb& db, const MojChar* kindId, MojTime& lgNestedObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeNestedObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		lgNestedObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeNestedObj(MojDb& db, const MojChar* kindId, MojTime& lgNestedObjTime)
{
	MojTime startTime;
	MojTime endTime;

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
	err = MojGetCurrentTime(startTime);
	MojTestErrCheck(err);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	err = MojGetCurrentTime(endTime);
	MojTestErrCheck(err);
	lgNestedObjTime += (endTime - startTime);
	totalTestTime += (endTime - startTime);

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putMedArrayObj(MojDb& db, const MojChar* kindId, MojTime& medArrayObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createMedArrayObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		medArrayObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::putLargeArrayObj(MojDb& db, const MojChar* kindId, MojTime& lgArrayObjTime)
{
	MojTime startTime;
	MojTime endTime;

	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = createLargeArrayObj(obj, i);
		MojTestErrCheck(err);

		err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		lgArrayObjTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfCreateTest::batchPutLargeArrayObj(MojDb& db, const MojChar* kindId, MojTime& lgArrayObjTime)
{
	MojTime startTime;
	MojTime endTime;

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
	err = MojGetCurrentTime(startTime);
	MojTestErrCheck(err);
	err = db.put(begin, end);
	MojTestErrCheck(err);
	err = MojGetCurrentTime(endTime);
	MojTestErrCheck(err);
	lgArrayObjTime += (endTime - startTime);
	totalTestTime += (endTime - startTime);

	return MojErrNone;
}

void MojDbPerfCreateTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
