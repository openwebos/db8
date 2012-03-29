/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#include "MojDbPerfReadTest.h"
#include "db/MojDb.h"
#include "core/MojTime.h"
#include "core/MojJson.h"

static const MojUInt64 numInsertForGet = 100;
static const MojUInt64 numInsertForFind = 500;
static const MojUInt64 numRepetitionsForGet = 100;
static const MojUInt64 numRepetitionsForFind = 20;

static MojTime totalTestTime;
static MojFile file;
const MojChar* const ReadTestFileName = _T("MojDbPerfReadTest.csv");

MojDbPerfReadTest::MojDbPerfReadTest()
: MojDbPerfTest(_T("MojDbPerfRead"))
{
}

MojErr MojDbPerfReadTest::run()
{
	MojErr err = file.open(ReadTestFileName, MOJ_O_RDWR | MOJ_O_CREAT | MOJ_O_TRUNC, MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojTestErrCheck(err);

	MojString m_buf;
	err = m_buf.format("MojoDb Read Performance Test,,,,,\n\nOperation,Kind,Total Time,Time Per Iteration,Time Per Object\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojDb db;
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	err = testGet(db);
	MojTestErrCheck(err);
	err = testFindAll(db);
	MojTestErrCheck(err);
	err = testFindPaged(db);
	MojTestErrCheck(err);

	err = MojPrintF("\n\n TOTAL TEST TIME: %llu microseconds\n\n", totalTestTime.microsecs());
	MojTestErrCheck(err);
	err = MojPrintF("\n-------\n");
	MojTestErrCheck(err);

	err = m_buf.format("\n\nTOTAL TEST TIME,,%llu,,,", totalTestTime.microsecs());
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	err = file.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::testFindPaged(MojDb& db)
{
	MojErr err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);
	err = MojPrintF("  PAGED FIND");
	MojTestErrCheck(err);
	err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);

	MojString m_buf;
	err = m_buf.format("\n\nPAGED FIND,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojDbQuery q;
	err = q.from(MojPerfSmKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgArrayKindId);
	MojTestErrCheck(err);
	err = findObjsPaged(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj, q);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::testFindAll(MojDb& db)
{
	MojErr err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);
	err = MojPrintF("  BASIC FIND");
	MojTestErrCheck(err);
	err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);

	MojString m_buf;
	err = m_buf.format("\n\nBASIC FIND,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	// find with limits
	MojDbQuery q;
	err = q.from(MojPerfSmKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj, q);
	MojTestErrCheck(err);
	err = q.from(MojPerfLgArrayKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj, q);
	MojTestErrCheck(err);

	err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);
	err = MojPrintF("  FIND WITH SELECT");
	MojTestErrCheck(err);
	err = MojPrintF("\n--------------\n");
	MojTestErrCheck(err);

	err = m_buf.format("\n\nFIND WITH SELECT,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	// find with select and limits
	MojDbQuery q2;
	err = q2.from(MojPerfSmKindId);
	MojTestErrCheck(err);
	err = q2.select(_T("first"));
	MojTestErrCheck(err);
	err = q2.select(_T("last"));
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj, q2);
	MojTestErrCheck(err);
	err = q2.from(MojPerfMedKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedKindId,  &MojDbPerfTest::createMedObj, q2);
	MojTestErrCheck(err);
	err = q2.from(MojPerfLgKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgKindId,  &MojDbPerfTest::createLargeObj, q2);
	MojTestErrCheck(err);
	MojDbQuery q3;
	err = q3.from(MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = q3.select(_T("name.first"));
	MojTestErrCheck(err);
	err = q3.select(_T("name.last"));
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj, q3);
	MojTestErrCheck(err);
	MojDbQuery q4;
	err = q4.from(MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = q4.select(_T("med.name.first"));
	MojTestErrCheck(err);
	err = q4.select(_T("med.name.last"));
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj, q4);
	MojTestErrCheck(err);
	MojDbQuery q5;
	err = q5.from(MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = q5.select(_T("names"));
	MojTestErrCheck(err);
	err = q5.select(_T("last"));
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj, q5);
	MojTestErrCheck(err);
	err = q5.from(MojPerfLgArrayKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgArrayKindId,  &MojDbPerfTest::createLargeArrayObj, q5);
	MojTestErrCheck(err);

	// find with where and limits
	err = MojPrintF("\n----------------------------\n");
	MojTestErrCheck(err);
	err = MojPrintF("  FIND WHERE FIRST = JOHN");
	MojTestErrCheck(err);
	err = MojPrintF("\n----------------------------\n");
	MojTestErrCheck(err);

	err = m_buf.format("\n\nFIND WHERE FIRST = JOHN,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojDbQuery q6;
	err = q6.from(MojPerfSmKindId);
	MojTestErrCheck(err);
	MojString str;
	err = str.assign(_T("John"));
	MojTestErrCheck(err);
	err = q6.where(_T("first"), MojDbQuery::OpEq, str);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj, q6);
	MojTestErrCheck(err);
	err = q6.from(MojPerfMedKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedKindId,  &MojDbPerfTest::createMedObj, q6);
	MojTestErrCheck(err);
	err = q6.from(MojPerfLgKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgKindId,  &MojDbPerfTest::createLargeObj, q6);
	MojTestErrCheck(err);
	MojDbQuery q7;
	err = q7.from(MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = q7.where(_T("name.first"), MojDbQuery::OpEq, str);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj, q7);
	MojTestErrCheck(err);
	MojDbQuery q8;
	err = q8.from(MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = q8.where(_T("med.name.first"), MojDbQuery::OpEq, str);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj, q8);
	MojTestErrCheck(err);
	MojDbQuery q9;
	err = q9.from(MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = q9.where(_T("names"), MojDbQuery::OpEq, str);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj, q9);
	MojTestErrCheck(err);
	err = q9.from(MojPerfLgArrayKindId);
	MojTestErrCheck(err);
	err = findObjs(db, MojPerfLgArrayKindId,  &MojDbPerfTest::createLargeArrayObj, q9);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::testGet(MojDb& db)
{
	MojString m_buf;
	MojErr err = m_buf.format("GET,,,,,\n");
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	err = getObjs(db, MojPerfSmKindId, &MojDbPerfTest::createSmallObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfMedKindId, &MojDbPerfTest::createMedObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfLgKindId, &MojDbPerfTest::createLargeObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfMedNestedKindId, &MojDbPerfTest::createMedNestedObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfLgNestedKindId, &MojDbPerfTest::createLargeNestedObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfMedArrayKindId, &MojDbPerfTest::createMedArrayObj);
	MojTestErrCheck(err);
	err = getObjs(db, MojPerfLgArrayKindId, &MojDbPerfTest::createLargeArrayObj);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::findObjsPaged(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64), MojDbQuery& query)
{
	// register all the kinds
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn and get them individually and in groups
	MojObject ids;
	err = putObjs(db, kindId, numInsertForFind, createFn, ids);
	MojTestErrCheck(err);

	MojUInt32 count;
	MojTime countTime;
	MojDbQuery::Page nextPage;

	MojTime find30Time;
	query.limit(30);
	err = timeFind(db, query, find30Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);

	MojUInt64 findTime = find30Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %llu of kind %s %llu times (with writer): %llu microsecs\n", 30, numInsertForFind, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (30 * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);

	MojString m_buf;
	err = m_buf.format("Find %d objects out of %llu %llu times (with writer),%s,%llu,%llu,%llu,\n", 30, numInsertForFind, numRepetitionsForFind, kindId, findTime, findTime/numRepetitionsForFind, findTime/(30*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);


	MojTime find2nd30Time;
	query.page(nextPage);
	err = timeFind(db, query, find2nd30Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find2nd30Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find 2nd %d objects out of %llu of kind %s %llu times (with writer): %llu microsecs\n", 30, numInsertForFind, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (30 * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find 2nd %d objects out of %llu %llu times (with writer),%s,%llu,%llu,%llu,\n", 30, numInsertForFind, numRepetitionsForFind, kindId, findTime, findTime/numRepetitionsForFind, findTime/(30*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find3rd30Time;
	query.page(nextPage);
	err = timeFind(db, query, find3rd30Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find3rd30Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find 3rd %d objects out of %llu of kind %s %llu times (with writer): %llu microsecs\n", 30, numInsertForFind, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (30 * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find 3rd %d objects out of %llu %llu times (with writer),%s,%llu,%llu,%llu,\n", 30, numInsertForFind, numRepetitionsForFind, kindId, findTime, findTime/numRepetitionsForFind, findTime/(30*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find4th30Time;
	query.page(nextPage);
	err = timeFind(db, query, find4th30Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find4th30Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find 4th %d objects out of %llu of kind %s %llu times (with writer): %llu microsecs\n", 30, numInsertForFind, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (30 * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find 4th %d objects out of %llu %llu times (with writer),%s,%llu,%llu,%llu,\n", 30, numInsertForFind, numRepetitionsForFind, kindId, findTime, findTime/numRepetitionsForFind, findTime/(30*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find5th30Time;
	query.page(nextPage);
	err = timeFind(db, query, find5th30Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find5th30Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find 5th %d objects out of %llu of kind %s %llu times (with writer): %llu microsecs\n", 30, numInsertForFind, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (30 * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find 5th %d objects out of %llu %llu times (with writer),%s,%llu,%llu,%llu,\n", 30, numInsertForFind, numRepetitionsForFind, kindId, findTime, findTime/numRepetitionsForFind, findTime/(30*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::findObjs(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64), MojDbQuery& query)
{
	// register all the kinds
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn and get them individually and in groups
	MojObject ids;
	err = putObjs(db, kindId, numInsertForFind, createFn, ids);
	MojTestErrCheck(err);

	MojString str;
	MojObject queryObj;
	err = query.toObject(queryObj);
	MojTestErrCheck(err);
	err = queryObj.stringValue(str);
	MojTestErrCheck(err);
	err = MojPrintF("\n QUERY is: %s", str.data());
	MojTestErrCheck(err);

	MojString m_buf;
	err = m_buf.format("QUERY is: %s,,,,,\n", str.data());
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojUInt32 count;
	MojTime countTime;
	MojDbQuery::Page nextPage;

	MojTime find10Time3;
	MojUInt32 limit = 10;
	query.limit(limit);
	err = timeFind(db, query, find10Time3, true, nextPage, true, count, countTime);
	MojTestErrCheck(err);
	MojUInt32 numObjs = (count > limit) ? limit : count;

	MojUInt64 findTime = find10Time3.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with writer and count): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs\n", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("   count: %d, countTime: %llu microsecs", count, countTime.microsecs() / numRepetitionsForFind);
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with writer and count),%s,%llu,%llu,%llu,\nCount: %d,%s,%llu,,,\n", limit, count, numRepetitionsForFind, kindId,
			findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind),count,kindId,countTime.microsecs()/numRepetitionsForFind);
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find10Time;
	err = timeFind(db, query, find10Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find10Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with writer): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with writer),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
				findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find10Time2;
	err = timeFind(db, query, find10Time2, false, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find10Time2.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with eater): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with eater),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
					findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find50Time;
	limit = 50;
	query.limit(limit);
	err = timeFind(db, query, find50Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find50Time.microsecs();
	numObjs = (count > limit) ? limit : count;

	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with writer): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with writer),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
					findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find50Time2;
	err = timeFind(db, query, find50Time2, false, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find50Time2.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with eater): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with eater),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
					findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find100Time;
	limit = 100;
	query.limit(limit);
	err = timeFind(db, query, find100Time, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	numObjs = (count > limit) ? limit : count;
	findTime = find100Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with writer): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with writer),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
				findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime find100Time2;
	err = timeFind(db, query, find100Time2, false, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = find100Time2.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with eater): %llu microsecs\n", limit, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with eater),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
				findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime findAllTime;
	limit = MojUInt32Max;
	query.limit(limit);
	err = timeFind(db, query, findAllTime, true, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	numObjs = (count > limit) ? limit : count;
	findTime = findAllTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with writer): %llu microsecs\n", count, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with writer),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
					findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime findAllTime2;
	err = timeFind(db, query, findAllTime2, false, nextPage, false, count, countTime);
	MojTestErrCheck(err);
	findTime = findAllTime2.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to find %d objects out of %d of kind %s %llu times (with eater): %llu microsecs\n", count, count, kindId, numRepetitionsForFind, findTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (findTime) / (numObjs * numRepetitionsForFind));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Find %d objects out of %d %llu times (with eater),%s,%llu,%llu,%llu,\n", limit, count, numRepetitionsForFind, kindId,
					findTime, findTime/numRepetitionsForFind, findTime/(numObjs*numRepetitionsForFind));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);


	return MojErrNone;
}

MojErr MojDbPerfReadTest::getObjs(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64))
{
	// register all the kinds
	MojTime time;
	MojErr err = putKinds(db, time);
	MojTestErrCheck(err);

	// put objects using createFn and get them individually and in groups
	MojObject ids;
	err = putObjs(db, kindId, numInsertForGet, createFn, ids);
	MojTestErrCheck(err);

	MojTime singleObjTime;
	MojObject id;
	bool exists = false;
	exists = ids.at(0, id);
	MojTestAssert(exists);
	err = timeGet(db, id, singleObjTime);
	MojTestErrCheck(err);
	MojUInt64 getTime = singleObjTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get object %d of kind %s %llu times: %llu microsecs\n", 0, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / numRepetitionsForGet);
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	MojString m_buf;
	err = m_buf.format("Get object %d %llu times,%s,%llu,%llu,%llu,\n", 0, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/(1*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime singleObj2Time;
	exists = false;
	exists = ids.at(numInsertForGet / 2, id);
	MojTestAssert(exists);
	err = timeGet(db, id, singleObj2Time);
	MojTestErrCheck(err);
	getTime = singleObj2Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get object %llu of kind %s %llu times: %llu microsecs\n", numInsertForGet/2, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / numRepetitionsForGet);
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get object %llu %llu times,%s,%llu,%llu,%llu,\n", numInsertForGet/2, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/(1*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime singleObj3Time;
	exists = false;
	exists = ids.at(numInsertForGet - 1, id);
	MojTestAssert(exists);
	err = timeGet(db, id, singleObj3Time);
	MojTestErrCheck(err);
	getTime = singleObj3Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get object %llu of kind %s %llu times: %llu microsecs\n", numInsertForGet - 1, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / numRepetitionsForGet);
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get object %llu %llu times,%s,%llu,%llu,%llu,\n", numInsertForGet - 1, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/(1*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batchTime;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayBegin() + 10, batchTime, true);
	MojTestErrCheck(err);
	getTime = batchTime.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %d of kind %s %llu times (using writer): %llu microsecs\n", 0, 10, kindId, numRepetitionsForGet, batchTime.microsecs());
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (batchTime.microsecs()) / (10 * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %d %llu times (using writer),%s,%llu,%llu,%llu,\n", 0, 10, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/(10*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batch2Time;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayBegin() + 10, batch2Time, false);
	MojTestErrCheck(err);
	getTime = batch2Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %d of kind %s %llu times (using eater): %llu microsecs\n", 0, 10, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / (10 * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %d %llu times (using eater),%s,%llu,%llu,%llu,\n", 0, 10, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/(10*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batch3Time;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayBegin() + numInsertForGet / 2, batch3Time, true);
	MojTestErrCheck(err);
	getTime = batch3Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %llu of kind %s %llu times (using writer): %llu microsecs\n", 0, numInsertForGet / 2, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / ((numInsertForGet / 2) * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %llu %llu times (using writer),%s,%llu,%llu,%llu,\n", 0, numInsertForGet/2, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/((numInsertForGet/2)*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batch4Time;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayBegin() + numInsertForGet / 2, batch4Time, false);
	MojTestErrCheck(err);
	getTime = batch4Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %llu of kind %s %llu times (using eater): %llu microsecs\n", 0, numInsertForGet / 2, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / ((numInsertForGet / 2) * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %llu %llu times (using eater),%s,%llu,%llu,%llu,\n", 0, numInsertForGet/2, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/((numInsertForGet/2)*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batch5Time;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayEnd(), batch5Time, true);
	MojTestErrCheck(err);
	getTime = batch5Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %llu of kind %s %llu times (using writer): %llu microsecs\n", 0, numInsertForGet, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / (numInsertForGet * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %llu %llu times (using writer),%s,%llu,%llu,%llu,\n", 0, numInsertForGet, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/((numInsertForGet)*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	MojTime batch6Time;
	err = timeBatchGet(db, ids.arrayBegin(), ids.arrayEnd(), batch6Time, false);
	MojTestErrCheck(err);
	getTime = batch6Time.microsecs();
	err = MojPrintF("\n -------------------- \n");
	MojTestErrCheck(err);
	err = MojPrintF("   time to get objects %d to %llu of kind %s %llu times (using eater): %llu microsecs\n", 0, numInsertForGet, kindId, numRepetitionsForGet, getTime);
	MojTestErrCheck(err);
	err = MojPrintF("   time per object: %llu microsecs", (getTime) / (numInsertForGet * numRepetitionsForGet));
	MojTestErrCheck(err);
	err = MojPrintF("\n\n");
	MojTestErrCheck(err);
	err = m_buf.format("Get objects %d to %llu %llu times (using eater),%s,%llu,%llu,%llu,\n", 0, numInsertForGet, numRepetitionsForGet, kindId, getTime, getTime/numRepetitionsForGet, getTime/((numInsertForGet)*numRepetitionsForGet));
	MojTestErrCheck(err);
	err = fileWrite(file, m_buf);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfReadTest::timeFind(MojDb& db, MojDbQuery& query, MojTime& findTime, bool useWriter, MojDbQuery::Page& nextPage, bool doCount, MojUInt32& count, MojTime& countTime)
{
	MojTime startTime;
	MojTime endTime;
	for (MojUInt64 i = 0; i < numRepetitionsForFind; i++) {
		MojErr err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		MojDbCursor cursor;
		err = db.find(query, cursor);
		MojTestErrCheck(err);
		if (useWriter) {
			MojJsonWriter writer;
			err = cursor.visit(writer);
			MojTestErrCheck(err);
			err = cursor.nextPage(nextPage);
			MojTestErrCheck(err);
		} else {
			MojObjectEater eater;
			err = cursor.visit(eater);
			MojTestErrCheck(err);
			err = cursor.nextPage(nextPage);
			MojTestErrCheck(err);
		}

		if (!doCount) {
			err = cursor.close();
			MojTestErrCheck(err);
		}
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		findTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);

		if (doCount) {
			err = MojGetCurrentTime(startTime);
			MojTestErrCheck(err);
			err = cursor.count(count);
			MojTestErrCheck(err);
			err = cursor.close();
			MojTestErrCheck(err);
			err = MojGetCurrentTime(endTime);
			MojTestErrCheck(err);
			countTime += (endTime - startTime);
			totalTestTime += (endTime - startTime);
		}
	}

	return MojErrNone;
}

MojErr MojDbPerfReadTest::timeGet(MojDb& db, MojObject& id, MojTime& getTime)
{
	MojTime startTime;
	MojTime endTime;
	for (MojUInt64 i = 0; i < numRepetitionsForGet; i++) {
		MojErr err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		MojObject obj;
		bool found;
		err = db.get(id, obj, found);
		MojTestErrCheck(err);
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		MojTestAssert(found);
		getTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfReadTest::timeBatchGet(MojDb& db, const MojObject* begin, const MojObject* end, MojTime& batchGetTime, bool useWriter)
{
	MojTime startTime;
	MojTime endTime;
	for (MojUInt64 i = 0; i < numRepetitionsForGet; i++) {
		MojErr err = MojGetCurrentTime(startTime);
		MojTestErrCheck(err);
		if (useWriter) {
			MojJsonWriter writer;
			err = db.get(begin, end, writer);
			MojTestErrCheck(err);
		} else {
			MojObjectEater eater;
			err = db.get(begin, end, eater);
			MojTestErrCheck(err);
		}
		err = MojGetCurrentTime(endTime);
		MojTestErrCheck(err);
		batchGetTime += (endTime - startTime);
		totalTestTime += (endTime - startTime);
	}

	return MojErrNone;
}

MojErr MojDbPerfReadTest::putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
		MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64), MojObject& ids)
{
	for (MojUInt64 i = 0; i < numInsert; i++) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, kindId);
		MojTestErrCheck(err);
		err = (*this.*createFn)(obj, i);
		MojTestErrCheck(err);

		err = db.put(obj);
		MojTestErrCheck(err);

		MojObject id;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		err = ids.push(id);
		MojTestErrCheck(err);
	}

	return MojErrNone;
}

void MojDbPerfReadTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
