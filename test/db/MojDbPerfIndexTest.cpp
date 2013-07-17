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

/*
DB8 Index Performance Test
==========================
Overview
--------
Create a Kind with 8 fields and ten indexes, We will populate the records using randomization
techniques (see below).
Add 100 records, then perform 3 queries, delete 10 %, change 10%, perform the 3 queries again.
Then repeat adding 400 more records, then 1600 more, then 6400 more, then 25600 more records
Record the elapsed time after each phase.

Layout
------
Field name          Type        Contents
Street Address      String      Construct a street address by randomly creating a number between 1 and
                                9999, and then adding a street name randomly selected from 10 street
                                names: [“Elm St.”, “Lake View Drive”, “Main St.”, “Frontage Road”,
                                “Route 120”, “Madison Avenue”, “Heritage Circle”, “Bay Ave.”, “15
                                St.”, “Ave. B”]
                                Ex: “5111 Heritage Circle” or “25 Elm St.”
City                String      Randomly select a city name from ten city names:[“Lincoln City”,
                                “Clinton”, “Mammoth Falls”, “Eureka”, “Elk Village”, “Dartmouth”,
                                “Rome”, “Athens”, “Sunnyvale”, “Kiev”]
State               String      Randomly select a State name from ten state names: [“California”,
                                “Georgia”, “New York”, “New Jersey”, “Iowa”, “Indiana”, “New
                                Mexico”, “North Carolina”, North Dakota”]
Zipcode             String      Randomly create a 5 digit number, with the leading zeroes, in the range
                                “00001” to “99999”. However, leave 1 in 10 blank (for the Delete test)
Limit               Number      Randomly create a number in the range 1 to 99999999
LastUpdate          Date        Randomly create a valid date in the range Jan 1, 2012 to Dec 31, 2012
Next Update         Date        Add 3 months to the LastUpdate value
Description         String      Every second record add a description field, as follows:
                                “Description”:“Mary had a little lamb, its fleece was white as snow”
Indexes
-------
Index name          Fields      Contents
ZipAddress          2           “Street Address”, “ZipCode”
FullAddress         4           “Street Address”, “City”, “State”, “ZipCode”
Limit               1           “Limit”
ZipLimit            2           “Zipcode”, “Limit”
Update              1           “Last Update”
StateUpdate         2           “State”, “Last Update”
Next                1           “Next Update”
StateNext           2           “State”, “Next Update”
LastZipLimit        3           “Last Update”, “Zipcode”, “Limit”
LimitAddress        5           “Limit”, “Street Address”, “City”, “State”, “ZipCode”

Queries
-------
Query Name          Index           Contents
MissingZip          FullAddress     Only return records where Zipcode field is empty
Whales              LimitAddress    Only return records where Limit > 89999999
StateNew            StateUpdate     Only return records where State begins with “New”

Deletion
--------
Delete records using the “MissingZip” index. This will delete 10% of the records just added.

Update
------
Select records using the “Whales” query and change the limit field by adding 67 to the current
value. This will modify about 10% of the records
each time.
*/

#include <stdlib.h>
#include <time.h>
#include "db/MojDbStorageEngine.h"

#include "MojDbPerfIndexTest.h"
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

static const MojChar* const TestKind1Str =
	_T("{\"id\":\"Test1:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"ZipAddress\",\"props\":[{\"name\":\"Street Address\"},{\"name\":\"ZipCode\"}]}, \
	{\"name\":\"FullAddress\",\"props\":[{\"name\":\"StreetAddress\"},{\"name\":\"City\"},{\"name\":\"State\"},{\"name\":\"ZipCode\"}]}, \
	{\"name\":\"Limit\",\"props\":[{\"name\":\"Limit\"}]}, \
	{\"name\":\"ZipLimit\",\"props\":[{\"name\":\"ZipCode\"},{\"name\":\"Limit\"}]}, \
	{\"name\":\"Update\",\"props\":[{\"name\":\"Last Update\"}]}, \
	{\"name\":\"StateUpdate\",\"props\":[{\"name\":\"State\"},{\"name\":\"LastUpdate\"}]}, \
	{\"name\":\"Next\",\"props\":[{\"name\":\"NextUpdate\"}]}, \
	{\"name\":\"StateNext\",\"props\":[{\"name\":\"State\"},{\"name\":\"NextUpdate\"}]}, \
	{\"name\":\"LastZipLimit\",\"props\":[{\"name\":\"LastUpdate\"},{\"name\":\"ZipCode\"},{\"name\":\"Limit\"}]}, \
	{\"name\":\"LimitAddress\",\"props\":[{\"name\":\"Limit\"},{\"name\":\"StreetAddress\"},{\"name\":\"City\"},{\"name\":\"State\"},{\"name\":\"ZipCode\"}]}, \
	]}");

#define PHASE_1_RECORDS_NUMBER 100
#define PHASE_2_RECORDS_NUMBER 400
#define PHASE_3_RECORDS_NUMBER 1600
#define PHASE_4_RECORDS_NUMBER 6400
#define PHASE_5_RECORDS_NUMBER 25600

static const MojChar* const StreetNames[] = {	_T("Elm St."), _T("Lake View Drive"), _T("Main St."), _T("Frontage Road"), _T("Route 120"),
												_T("Madison Ave."), _T("Heritage Circle"), _T("Bay Ave."), _T("15 St."), _T("Ave. B") };

static const MojChar* const CityNames[] = {	_T("Lincoln City"), _T("Clinton"), _T("Mammoth Falls"), _T("Eureka"), _T("Elk Village"),
												_T("Dartmouth"), _T("Rome"), _T("Athens"), _T("Sunnyvale"), _T("Kiev") };

static const MojChar* const StateNames[] = {	_T("California"), _T("Georgia"), _T("New York"), _T("New Jersey"), _T("Iowa"),
												_T("Indiana"), _T("New Mexico"), _T("North Carolina"), _T("North Dakota") };

static const MojChar* const Monthes[] = { _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"), _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec") };

static const MojChar* const Description = _T("Mary had a little lamb, its fleece was white as snow");

MojDbPerfIndexTest::MojDbPerfIndexTest()
: MojTestCase(_T("MojDbPerfIndex"))
{
	srand( time(NULL) + getpid());
}

MojErr MojDbPerfIndexTest::run()
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

	//_displayMessage("\n<<< Please, cleanup folder %s before this test! >>>\n", MojDbTestDir);
	cleanup();

	// open
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// add type
	MojObject obj;
	err = obj.fromJson(TestKind1Str);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	for (MojUInt32 i = 0; i < 5; ++i)
	{
		_runPhaseTest(db, i);
		MojTestErrCheck(err);
	}

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfIndexTest::_runPhaseTest (MojDb& db, MojUInt32 i_phase_number)
{
	MojErr err;
	MojUInt32 number_records;
	MojUInt32 arr_number_records[] = { PHASE_1_RECORDS_NUMBER, PHASE_2_RECORDS_NUMBER, PHASE_3_RECORDS_NUMBER, PHASE_4_RECORDS_NUMBER, PHASE_5_RECORDS_NUMBER };

	if( i_phase_number > 4 )
	{
		_displayMessage("Error: phase %d index is out of allowed range!", i_phase_number + 1);
		return MojErrInvalidArg;
	}

	struct timespec ts1, ts2;
	int time_result = clock_gettime(CLOCK_REALTIME, &ts1);
	number_records = arr_number_records[i_phase_number];

	_displayMessage("\tPhase [%d] running..\n\t\t", i_phase_number + 1);

	// add new records
	err = _generateNewRecords(db, number_records);
	MojTestErrCheck(err);

	err = _performThreeQueries(db);
	MojTestErrCheck(err);

	// delete ~10% records
	err = _delete10PersentsRecords(db);
	MojTestErrCheck(err);

    //change 10% records
	err = _change10PersentsRecords(db);
	MojTestErrCheck(err);

	err = _performThreeQueries(db);
	MojTestErrCheck(err);

	if (time_result == 0)
	{
		time_result = clock_gettime(CLOCK_REALTIME, &ts2);
	}

	if (time_result == 0)
	{
		time_t diff = (ts2.tv_sec - ts1.tv_sec) * 1000 + abs(1000000000 + ts2.tv_nsec - ts1.tv_nsec - 1000000000) / 1000000;
		_displayMessage("\n\tcompleted with time: %ums\n", diff);
	}

	return err;
}

MojErr MojDbPerfIndexTest::_generateNewRecords (MojDb& db, MojUInt32 i_number_records)
{
	MojErr err = MojErrNone;
	MojString str_value;
	MojObject obj;
	MojObject obj_description;
	MojUInt32 count = 0;
	time_t time_stamp;

	obj_description.pushString(Description);

	for(MojUInt32 i = 0; i < i_number_records; ++i)
	{
		MojErr err = obj.putString(_T("_kind"), _T("Test1:1"));
		MojTestErrCheck(err);

		err = _generateStreetAddress(str_value);
		MojTestErrCheck(err);
		MojObject obj1(str_value);
		err = obj.put(_T("StreetAddress"), obj1);
		MojTestErrCheck(err);

		err = _generateCityName(str_value);
		MojTestErrCheck(err);
		MojObject obj2(str_value);
		err = obj.put(_T("City"), obj2);
		MojTestErrCheck(err);

		err = _generateStateName(str_value);
		MojTestErrCheck(err);
		MojObject obj3(str_value);
		err = obj.put(_T("State"), obj3);
		MojTestErrCheck(err);

		if( i % 10) // leave 1 in 10 blank (for the Delete test)
		{
			err = _generateZipCode(str_value);
			MojTestErrCheck(err);
		}
		else
		{
			str_value.assign("");
		}

		MojObject obj4(str_value);
		err = obj.put(_T("ZipCode"), obj4);
		MojTestErrCheck(err);

		MojObject obj5;
		_generateLimitCode(obj5);
		err = obj.put(_T("Limit"), obj5);
		MojTestErrCheck(err);

		err = _generateValidDate(str_value, time_stamp);
		MojTestErrCheck(err);
		MojObject obj6(str_value);
		err = obj.put(_T("LastUpdate"), obj6);
		MojTestErrCheck(err);

		err = _add3monthToDate(str_value, time_stamp);
		MojTestErrCheck(err);
		MojObject obj7(str_value);
		err = obj.put(_T("NextUpdate"), obj7);
		MojTestErrCheck(err);

		if ( i % 2 )
		{
			// add description to every second record
			err = obj.put(_T("Description"), obj_description);
			MojTestErrCheck(err);
		}

		err = db.put(obj);
		MojTestErrCheck(err);
		count++;
		obj.clear();
	}

	_displayMessage("created: %d, ", count);

	return err;
}

MojErr MojDbPerfIndexTest::_performThreeQueries (MojDb& db)
{
	MojErr err;
	MojString str_value;

	str_value.assign("");
	MojObject empty_string(str_value);
	MojDbQuery query1;

	// make 1-st query
	err = query1.from(_T("Test1:1"));
	MojTestErrCheck(err);
	err = query1.where(_T("ZipCode"), MojDbQuery::OpEq, empty_string);
	MojTestErrCheck(err);

	MojDbCursor cursor1;
	err = db.find(query1, cursor1);

	if (err == MojErrNone)
	{
		MojUInt32 count1;
		err = cursor1.count(count1); // not used, but require to spend time for exec!
		MojTestErrCheck(err);
		err = cursor1.close();
		MojTestErrCheck(err);
	}

	// make 2-nd query
	MojDbQuery query2;
	err = query2.from(_T("Test1:1"));
	MojTestErrCheck(err);
	err = query2.where(_T("Limit"), MojDbQuery::OpGreaterThan, 89999999);
	MojTestErrCheck(err);

	MojDbCursor cursor2;
	err = db.find(query2, cursor2);

	if (err == MojErrNone)
	{
		MojUInt32 count2;
		err = cursor2.count(count2); // not used, but require to spend time for exec!
		MojTestErrCheck(err);
		err = cursor2.close();
		MojTestErrCheck(err);
	}

	// make 3-rd query
	MojString new_string;
	MojDbQuery query3;
	new_string.assign("New");

	err = query3.from(_T("Test1:1"));
	MojTestErrCheck(err);
	err = query3.where(_T("State"), MojDbQuery::OpPrefix, new_string);
	MojTestErrCheck(err);

	MojDbCursor cursor3;
	err = db.find(query3, cursor3);

	if (err == MojErrNone)
	{
		MojUInt32 count3;
		err = cursor3.count(count3); // not used, but require to spend time for exec!
		MojTestErrCheck(err);
		err = cursor3.close();
		MojTestErrCheck(err);
	}

	return err;
}

MojErr MojDbPerfIndexTest::_delete10PersentsRecords (MojDb& db)
{
	MojErr err = MojErrNone;
	MojString str_value;

	str_value.assign("");
	MojObject nullable(str_value);
	MojDbQuery query;
	MojUInt32 count = 0;
	MojUInt32 count2 = 0;

	err = query.from(_T("Test1:1"));
	MojTestErrCheck(err);
	err = query.where(_T("ZipCode"), MojDbQuery::OpEq, nullable);
	MojTestErrCheck(err);

	query.limit(3000);

	MojDbReq request;
	err = db.del(query, count, MojDb::FlagNone, request);
	MojTestErrCheck(err);

	MojDbReq request2;
	err = db.del(query, count2, MojDb::FlagNone, request2);
	MojTestErrCheck(err);

	_displayMessage("deleted: %d (2-->%d), ", count, count2);
	MojTestAssert(count2 == 0);

	return err;
}

MojErr MojDbPerfIndexTest::_change10PersentsRecords (MojDb& db)
{
	MojErr err = MojErrNone;
	MojDbQuery query;
	MojUInt32 rec_count = 0;
	MojDbCursor cursor;

	err = query.from(_T("Test1:1"));
	MojTestErrCheck(err);
	err = query.where(_T("Limit"), MojDbQuery::OpGreaterThan, 89999999);
	MojTestErrCheck(err);

	err = db.find(query, cursor);

	if (err == MojErrNone)
	{
		err = _cursorChange10PersentsRecords(db, cursor, rec_count);
		MojTestErrCheck(err);
		cursor.close();
	}

	MojUInt32 rec_count2 = 0;
	MojDbCursor cursor2;

	err = db.find(query, cursor2);

	if (err == MojErrNone)
	{
		err = _cursorChange10PersentsRecords(db, cursor2, rec_count2);
		MojTestErrCheck(err);
		cursor2.close();
	}

	_displayMessage("changed: %d (2-->%d)", rec_count, rec_count2);
	MojTestAssert(rec_count == rec_count2);

	return err;
}

MojErr MojDbPerfIndexTest::_cursorChange10PersentsRecords (MojDb& db, MojDbCursor& cursor, MojUInt32& count_affected)
{
	MojErr err = MojErrNone;

	for (;;)
	{
		MojUInt32 count = 0;
		MojObject dbObj;
		bool found;
		err = cursor.get(dbObj, found);
		MojTestErrCheck(err);
		if (!found)
			break;

		// update
		MojInt32 value;
		MojObject update;
		err = dbObj.get(_T("Limit"), value, found);

		if( found )
		{
			MojDbQuery subquery;
			err = subquery.from(_T("Test1:1"));
			MojTestErrCheck(err);
			err = subquery.where(_T("Limit"), MojDbQuery::OpEq, value);

			MojTestErrCheck(err);

			value = value + 67;
			err = update.put(_T("Limit"), value);
			MojTestErrCheck(err);

			err = db.merge(subquery, update, count);
			MojTestErrCheck(err);

			count_affected++;
		}
	}

	return err;
}

void MojDbPerfIndexTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbPerfIndexTest::_generateStreetAddress (MojString& o_string)
{
	MojInt32 index = abs( rand() ) % 10;
	MojInt32 value = abs( rand() ) % 9999 + 1;
	MojErr err = o_string.format("%d %s", value, StreetNames[index]);

	return err;
}

MojErr MojDbPerfIndexTest::_generateCityName (MojString& o_string)
{
	MojInt32 index = abs( rand() ) % 10;
	MojErr err = o_string.format("%s", CityNames[index]);

	return err;
}

MojErr MojDbPerfIndexTest::_generateStateName (MojString& o_string)
{
	MojInt32 index = abs( rand() ) % 9;
	MojErr err = o_string.format("%s", StateNames[index]);

	return err;
}

MojErr MojDbPerfIndexTest::_generateZipCode (MojString& o_string)
{
	MojInt32 value = abs( rand() ) % 9999 + 1;
	MojErr err = o_string.format("%04d", value);

	return err;
}

void MojDbPerfIndexTest::_generateLimitCode (MojObject& o_object)
{
	MojInt32 value = abs( rand() ) % 99999999 + 1;
	MojObject obj(value);
	o_object = obj;
}

/*
 * Generate valid date in range: Jan 1, 2012 - Dec 31, 2012
 */
MojErr MojDbPerfIndexTest::_generateValidDate (MojString& o_string, time_t& o_value)
{
	time_t value_begin = 1325376000; // --> Jan 1, 2012
	time_t value_end = 1356998399; // --> Dec 31, 2012
	time_t diff = value_end - value_begin + 1;
	time_t gen = value_begin + abs( rand() ) % diff;

	struct tm const *tmr = gmtime(&gen);
	// tmr->tm_year = The number of years since 1900
	MojErr err = o_string.format("%s %d, %d", Monthes[tmr->tm_mon], tmr->tm_mday, tmr->tm_year + 1900);
	o_value = gen;

	return MojErrNone;
}

/*
 * Generate valid date from the i_value + 3 month
 */
MojErr MojDbPerfIndexTest::_add3monthToDate (MojString& o_string, time_t& i_value)
{
	time_t gen = i_value;
	struct tm const *tmr = gmtime(&gen);
	MojInt32 tm_mon = (tmr->tm_mon + 3) % 12;
	MojInt32 tm_year = tmr->tm_year + ((tmr->tm_mon + 3) < 12 ? 0 : 1);
	// tmr->tm_year = The number of years since 1900
	MojErr err = o_string.format("%s %d, %d", Monthes[tm_mon], tmr->tm_mday, tm_year + 1900);

	return MojErrNone;
}

MojErr MojDbPerfIndexTest::_displayMessage (const MojChar* format, ...)
{
	va_list args;
	va_start (args, format);
	MojErr err = MojVPrintF(format, args);
	va_end(args);
	MojErrCheck(err);

	return MojErrNone;
}
