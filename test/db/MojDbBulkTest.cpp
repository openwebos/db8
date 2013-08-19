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

/**
****************************************************************************************************
* Filename              : MojBulkTest.cpp
* Description           : Source file for MojBulk test.
****************************************************************************************************
**/

#include "MojDbBulkTest.h"
#include "db/MojDb.h"

static const MojChar* const MojKindStr =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");

MojDbBulkTest::MojDbBulkTest()
: MojTestCase(_T("MojDbBulk"))
{
}

/**
****************************************************************************************************
* @run              Inserts bulk data into the database using 'put' utility command.
                    Data is inserted by running through the loop for a registered kind.
                    Data is queried and updated using 'merge' utility function.

                    Control Flow :
                    1. Database Open
                    2. Kind registration
                    3. Inserting bulk data.
                    4. Querying and Updating the data.
                    5. Verify by checking the count of records.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
MojErr MojDbBulkTest::run()
{
	MojDb db;

	// open
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);
	// add type
	MojObject obj;
	err = obj.fromJson(MojKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	for (int i = 0; i < 100; ++i) {
		MojObject obj;
		MojErr err = obj.putString(MojDb::KindKey, _T("Test:1"));
		MojTestErrCheck(err);
		err = obj.put(_T("foo"), i);
		MojTestErrCheck(err);
		err = obj.put(_T("bar"), i);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	MojDbQuery query;
	err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpLessThan, 50);
	MojTestErrCheck(err);
	MojObject update;
	err = update.put(_T("foo"), -1);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db.merge(query, update, count);
	MojTestErrCheck(err);
	MojTestAssert(count == 50);

	query.clear();
	err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		if (bar < 50) {
			MojTestAssert(foo == -1);
		} else {
			MojTestAssert(foo == bar);
		}
		++count;
	}
	MojTestAssert(count == 100);
	err = cursor.close();
	MojTestErrCheck(err);

	err = query.where(_T("foo"), MojDbQuery::OpEq, -1);
	MojTestErrCheck(err);
	err = db.del(query, count);
	MojTestErrCheck(err);
	MojTestAssert(count == 50);

	query.clear();
	err = query.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(foo == bar);
		++count;
	}
	MojTestAssert(count == 50);
	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbBulkTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
