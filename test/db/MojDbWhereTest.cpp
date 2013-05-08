/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics, Inc.
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


#include "MojDbWhereTest.h"
#include "db/MojDb.h"


static const MojChar* const TestKind =
	_T("{\"id\":\"Test:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"testindex1\",\"props\":[{\"name\":\"data1\"}]},{\"name\":\"testindex2\",\"props\":[{\"name\":\"data1\"},{\"name\":\"data2\"}]}]}");
static const MojChar* const TestJson1 =
	_T("{\"_kind\":\"Test:1\",\"data1\":1000,\"data2\":1100,\"data3\":1200}");
static const MojChar* const TestJson2 =
	_T("{\"_kind\":\"Test:1\",\"data1\":2000,\"data2\":2100,\"data3\":2200}");
static const MojChar* const TestJson3 =
	_T("{\"_kind\":\"Test:1\",\"data1\":3000,\"data2\":3100,\"data3\":3200}");

MojDbWhereTest::MojDbWhereTest()
: MojTestCase(_T("MojDbWhere"))
{
}

MojErr MojDbWhereTest::run()
{
	MojDb db;

	// open
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// add type
	MojObject obj;
	err = obj.fromJson(TestKind);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// add record1
	err = obj.fromJson(TestJson1);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	// add record2
	err = obj.fromJson(TestJson2);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	// add record3
	err = obj.fromJson(TestJson3);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	/* 1 column test
	 * indexes : props:[ {data1}]
	 * find{query:where:[{data1 >= 1000},{data1 <= 2000}]}}
	 */
	err = displayMessage(_T("\n\tfind{query:where:[{data1 >= 1000},{data1 <= 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query1;
	err = query1.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query1.where(_T("data1"), MojDbQuery::OpGreaterThanEq, 1000);
	MojTestErrCheck(err);
	err = query1.where(_T("data1"), MojDbQuery::OpLessThanEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor1;
	err = db.find(query1, cursor1);

	if (err == MojErrNone)
	{
		MojUInt32 count1;
		err = cursor1.count(count1);
		MojTestErrCheck(err);
		err = cursor1.close();
		MojTestErrCheck(err);
		MojTestAssert(count1 == 2);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* 2 columns test
	 * indexes : props:[{data1,data2}]
	 * (1) where:[{data1 = 1000},{data2 = 2000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 = 1000},{data2 = 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query2;
	err = query2.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query2.where(_T("data1"), MojDbQuery::OpEq, 1000);
	MojTestErrCheck(err);
	err = query2.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor2;
	err = db.find(query2, cursor2);

	if (err == MojErrNone)
	{
		MojUInt32 count2;
		err = cursor2.count(count2);
		MojTestErrCheck(err);
		err = cursor2.close();
		MojTestErrCheck(err);
		MojTestAssert(count2 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (2) where:[{data2 = 1000},{data1 = 2000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data2 = 1000},{data1 = 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query3;
	err = query3.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query3.where(_T("data2"), MojDbQuery::OpEq, 1000);
	MojTestErrCheck(err);
	err = query3.where(_T("data1"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor3;
	err = db.find(query3, cursor3);

	if (err == MojErrNone)
	{
		MojUInt32 count3;
		err = cursor3.count(count3);
		MojTestErrCheck(err);
		err = cursor3.close();
		MojTestErrCheck(err);
		MojTestAssert(count3 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (3) where:[{data1 <= 1000},{data2 = 2000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 <= 1000},{data2 = 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query4;
	err = query4.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query4.where(_T("data1"), MojDbQuery::OpLessThanEq, 1000);
	MojTestErrCheck(err);
	err = query4.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor4;
	err = db.find(query4, cursor4);

	if (err == MojErrNone)
	{
	    MojUInt32 count4;
	    err = cursor4.count(count4);
	    MojTestErrCheck(err);
		err = cursor4.close();
		MojTestErrCheck(err);
	    MojTestAssert(count4 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (4) where:[{data2 <= 1000},{data1 = 2000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data2 <= 1000},{data1 = 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query5;
	err = query5.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query5.where(_T("data2"), MojDbQuery::OpLessThanEq, 1000);
	MojTestErrCheck(err);
	err = query5.where(_T("data1"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor5;
	err = db.find(query5, cursor5);

	if (err == MojErrNone)
	{
	    MojUInt32 count5;
	    err = cursor5.count(count5);
	    MojTestErrCheck(err);
		err = cursor5.close();
		MojTestErrCheck(err);
	    MojTestAssert(count5 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (5) where:[{data1 <= 1000},{data2 <= 2000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 <= 1000},{data2 <= 2000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query6;
	err = query6.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query6.where(_T("data1"), MojDbQuery::OpLessThanEq, 1000);
	MojTestErrCheck(err);
	err = query6.where(_T("data2"), MojDbQuery::OpLessThanEq, 2000);
	MojTestErrCheck(err);

	MojDbCursor cursor6;
	err = db.find(query6, cursor6);

	if (err == MojErrNone)
	{
	    MojUInt32 count6;
	    err = cursor6.count(count6);
	    MojTestErrCheck(err);
		err = cursor6.close();
		MojTestErrCheck(err);
	    MojTestAssert(count6 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* 3 columns test
	 * indexes : props:[{data1,data2,data3}]
	 * (1) where:[{data1 = 1000},{data2 = 2000},{data3 = 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 = 1000},{data2 = 2000},{data3 = 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query7;
	err = query7.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query7.where(_T("data1"), MojDbQuery::OpEq, 1000);
	MojTestErrCheck(err);
	err = query7.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);
	err = query7.where(_T("data3"), MojDbQuery::OpEq, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor7;
	err = db.find(query7, cursor7);

	if (err == MojErrNone)
	{
	    MojUInt32 count7;
	    err = cursor7.count(count7);
	    MojTestErrCheck(err);
		err = cursor7.close();
		MojTestErrCheck(err);
	    MojTestAssert(count7 == 1);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (2) where:[{data1 < 1000},{data2 = 2000},{data3 = 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 < 1000},{data2 = 2000},{data3 = 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query9;
	err = query9.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query9.where(_T("data1"), MojDbQuery::OpLessThan, 1000);
	MojTestErrCheck(err);
	err = query9.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);
	err = query9.where(_T("data3"), MojDbQuery::OpEq, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor9;
	err = db.find(query9, cursor9);

	if (err == MojErrNone)
	{
	    MojUInt32 count9;
	    err = cursor9.count(count9);
	    MojTestErrCheck(err);
		err = cursor9.close();
		MojTestErrCheck(err);
	    MojTestAssert(count9 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (3) where:[{data1 = 1000},{data2 < 2000},{data3 = 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 = 1000},{data2 < 2000},{data3 = 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query10;
	err = query10.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query10.where(_T("data1"), MojDbQuery::OpEq, 1000);
	MojTestErrCheck(err);
	err = query10.where(_T("data2"), MojDbQuery::OpLessThan, 2000);
	MojTestErrCheck(err);
	err = query10.where(_T("data3"), MojDbQuery::OpEq, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor10;
	err = db.find(query10, cursor10);

	if (err == MojErrNone)
	{
	    MojUInt32 count10;
	    err = cursor10.count(count10);
	    MojTestErrCheck(err);
		err = cursor10.close();
		MojTestErrCheck(err);
		MojTestAssert(count10 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (4) where:[{data1 = 1000},{data2 = 2000},{data3 < 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 = 1000},{data2 = 2000},{data3 < 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query11;
	err = query11.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query11.where(_T("data1"), MojDbQuery::OpEq, 1000);
	MojTestErrCheck(err);
	err = query11.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);
	err = query11.where(_T("data3"), MojDbQuery::OpLessThan, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor11;
	err = db.find(query11, cursor11);

	if (err == MojErrNone)
	{
	    MojUInt32 count11;
	    err = cursor11.count(count11);
	    MojTestErrCheck(err);
		err = cursor11.close();
		MojTestErrCheck(err);
		MojTestAssert(count11 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (5) where:[{data1 < 1000},{data2 = 2000},{data3 < 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 < 1000},{data2 = 2000},{data3 < 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query12;
	err = query12.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query12.where(_T("data1"), MojDbQuery::OpLessThan, 1000);
	MojTestErrCheck(err);
	err = query12.where(_T("data2"), MojDbQuery::OpEq, 2000);
	MojTestErrCheck(err);
	err = query12.where(_T("data3"), MojDbQuery::OpLessThan, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor12;
	err = db.find(query12, cursor12);

	if (err == MojErrNone)
	{
	    MojUInt32 count12;
	    err = cursor12.count(count12);
	    MojTestErrCheck(err);
		err = cursor12.close();
		MojTestErrCheck(err);
		MojTestAssert(count12 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	/* (6) where:[{data1 < 1000},{data2 < 2000},{data3 < 3000}]
	 */
	err = displayMessage(_T("\tfind{query:where:[{data1 < 1000},{data2 < 2000},{data3 < 3000}]} ..."));
	MojErrCheck(err);

	MojDbQuery query13;
	err = query13.from(_T("Test:1"));
	MojTestErrCheck(err);
	err = query13.where(_T("data1"), MojDbQuery::OpLessThan, 1000);
	MojTestErrCheck(err);
	err = query13.where(_T("data2"), MojDbQuery::OpLessThan, 2000);
	MojTestErrCheck(err);
	err = query13.where(_T("data3"), MojDbQuery::OpLessThan, 3000);
	MojTestErrCheck(err);

	MojDbCursor cursor13;
	err = db.find(query13, cursor13);

	if (err == MojErrNone)
	{
	    MojUInt32 count13;
	    err = cursor13.count(count13);
	    MojTestErrCheck(err);
		err = cursor13.close();
		MojTestErrCheck(err);
		MojTestAssert(count13 == 0);
		err = displayMessage(_T("OK\n"));
		MojErrCheck(err);
	}
	else
	{
		err = displayMessage(_T("FAILED\n"));
		MojErrCheck(err);
	}

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbWhereTest::displayMessage(const MojChar* format, ...)
{
	va_list args;
	va_start (args, format);
	MojErr err = MojVPrintF(format, args);
	va_end(args);
	MojErrCheck(err);

	return MojErrNone;
}

void MojDbWhereTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
