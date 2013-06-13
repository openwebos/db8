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


#include "MojDbSliceTest.h"
#include "db/MojDb.h"

static const MojChar* const MojKindStrBasicSlice =
	_T("{\"_kind\":\"Kind:1\",\"_id\":\"_kinds/BasicSliceTest:1\"}");

static const MojChar* const MojBasicSlice1 =
	_T("{\"objects\":[{\"_kind\":\"BasicSliceTest:1\",\"a\":{\"b\":100},\"c\":{\"d\":{\"e\":200,\"f\":300},\"g\":400}}, \"h\":500]}");


MojDbSliceTest::MojDbSliceTest()
: MojTestCase(_T("MojDbSlice"))
{
}

MojErr MojDbSliceTest::run()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(MojKindStrBasicSlice);
	MojTestErrCheck(err);
	err = db.put(kind);
	MojTestErrCheck(err);

	//ask for slices when nothing exists in the database
	err = queryNothingExists(db);
	MojTestErrCheck(err);

	//ask for slices that exist in the object when one item is in the db - ask for all the leaf node slices
	MojObject obj;
	err = obj.fromJson(MojBasicSlice1);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	MojVector<MojObject> objects;
	objects.push(obj);
	MojVector<MojString> slices;
	MojString slice;
	slice.assign(_T("a.b"));
	slices.push(slice);
	slice.assign(_T("c.d.e"));
	slices.push(slice);
	slice.assign(_T("c.d.f"));
	slices.push(slice);
	slice.assign(_T("c.g"));
	slices.push(slice);
	slice.assign(_T("h.i"));
	slices.push(slice);
	err = querySlicesExist(db, slices, objects);
	MojTestErrCheck(err);

	//ask for slices that exist in the object when one item is in the db - ask for some of the leaf node slices
	MojObject obj2;
	err = obj2.fromJson(MojBasicSlice1);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);
	objects.clear();
	objects.push(obj2);
	slices.clear();
	slice.assign(_T("a.b"));
	slices.push(slice);
	slice.assign(_T("c.d.e"));
	slices.push(slice);
	slice.assign(_T("c.g"));
	slices.push(slice);
	slice.assign(_T("h.i"));
	slices.push(slice);
	err = querySlicesExist(db, slices, objects);
	MojTestErrCheck(err);

	//ask for slices that exist in the object when one item is in the db - ask for whole objects

	//ask for slices that exist in the object when 2 items are in the db

	//ask for slices out of order that exist

	//ask for redundant slices that exist

	//create a 3rd object with only some of the properties filled out

	//ask for slices that exist - 2 of the returned objects should be complete, the 3rd will be partial

	//ask for slices that do not exist

	//ask for slices that don't exist and then some that do

	//ask for slices that exist and then some that don't

	//ask for a sub-property without fully qualifying it

	//ask for nothing - should get an error

	//create object with an array of values, ask for array slice

	//create object with an array of objects, ask for slice of property from the array.

	return MojErrNone;
}

MojErr MojDbSliceTest::queryNothingExists(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("BasicSliceTest:1"));
	MojTestErrCheck(err);
	err = query.select(_T("a"));
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	MojSize count;
	err = cursor.count(count);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(count == 0);

	return MojErrNone;
}

MojErr MojDbSliceTest::querySlicesExist(MojDb& db, MojVector<MojString> slices, MojVector<MojObject> expectedObjects)
{
	MojDbQuery query;
	MojErr err = query.from(_T("BasicSliceTest:1"));
	MojTestErrCheck(err);
	for (MojVector<MojString>::ConstIterator i = slices.begin(); i != slices.end(); ++i) {
		err = query.select(*i);
		MojTestErrCheck(err);
	}

	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);

	MojSize count;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == expectedObjects.size());


	return MojErrNone;
}

void MojDbSliceTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}

