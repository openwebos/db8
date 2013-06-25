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


#include "MojDbKindTest.h"
#include "db/MojDb.h"
#include "db/MojDbKind.h"
#include "db/MojDbReq.h"
#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else 
#error "Specify database engine"
#endif

#include "MojDbTestStorageEngine.h"
#include "db/MojDbServiceDefs.h"
#include "core/MojJson.h"

static const MojChar* const MojTestKindId =
	_T("KindTest:1");
static const MojChar* const MojTestObjStr =
	_T("{\"_kind\":\"KindTest:1\",\"foo\":1,\"bar\":2}");
static const MojChar* const MojTestObj2Str =
	_T("{\"_kind\":\"KindTest:1\",\"foo\":2,\"bar\":4}");
static const MojChar* const MojTestKindStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\"}");
static const MojChar* const MojTestIndexesFooStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const MojTestIndexesBarStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]}]}");
static const MojChar* const MojTestIndexesFooBarStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]}]}");
static const MojChar* const MojTestIndexesBarFooStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"bar\",\"props\":[{\"name\":\"bar\"}]},{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const MojTestIndexesFooBazStr =
	_T("{\"id\":\"KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"baz\",\"props\":[{\"name\":\"baz\"}]}]}");
static const MojChar* const MojTestChildKindStr =
	_T("{\"id\":\"ChildKindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"extends\":[\"ParentKindTest:1\"]}");
static const MojChar* const MojTestChildObjStr =
	_T("{\"_kind\":\"ChildKindTest:1\",\"_id\":\"myChild\",\"foo\":1,\"bar\":2}");
static const MojChar* const MojTestParentKindStr =
	_T("{\"id\":\"ParentKindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"extends\":[\"GrandparentKindTest:1\"]}");
static const MojChar* const MojTestParentObjStr =
	_T("{\"_kind\":\"ParentKindTest:1\",\"_id\":\"myParent\",\"foo\":3,\"bar\":4}");
static const MojChar* const MojTestGrandparentKindStr =
	_T("{\"id\":\"GrandparentKindTest:1\",")
	_T("\"owner\":\"mojodb.admin\"}");
static const MojChar* const MojTestContactKindStrBadId =
	_T("{\"id\":\"ContactTest:1\",")
	_T("\"owner\":\"com.palm.contacts\"}");
static const MojChar* const MojTestContactKindStrGoodId =
	_T("{\"id\":\"com.palm.contacts.contact:1\",")
	_T("\"owner\":\"com.palm.contacts\"}");
static const MojChar* const MojTestContact2KindStrGoodId =
	_T("{\"id\":\"com.palm.contacts.contact.addresses:1\",")
	_T("\"owner\":\"com.palm.contacts\"}");
static const MojChar* const MojTestContact2KindStrBadId =
	_T("{\"id\":\"com.palm.contacts:1\",")
	_T("\"owner\":\"com.palm.contacts\"}");
static const MojChar* const MojTestChildContactKind =
	_T("{\"id\":\"com.palm.contacts.person:1\",")
	_T("\"owner\":\"com.palm.contacts\",")
	_T("\"extends\":[\"GrandparentKindTest:1\"]}");
static const MojChar* const ExtendsValue =
	_T("[\"GrandparentKindTest:1\"]");
static const MojChar* const KindId =
	_T("com.palm.contacts.person:1");
static const MojChar* const TestChildContactObject =
	_T("{\"_kind\":\"com.palm.contacts.person:1\",\"foo\":1,\"bar\":2}");
static const MojChar* const MojTestObjectKindStr =
	_T("{\"id\":\"Object:1\",")
	_T("\"owner\":\"com.palm.contacts\"}");
static const MojChar* const MojTestObjectKindStr2 =
	_T("{\"id\":\"Object:1\",")
	_T("\"owner\":\"mojodb.admin\"}");
static const MojChar* const TestPermissionObject =
	_T("{\"_kind\":\"Permission:1\",\"type\":1,\"caller\":2}");
static const MojChar* const MojTestInvalidKindStr =
	_T("{\"id\":\"\",")
	_T("\"owner\":\"mojodb.admin\"}");
static const MojChar* const MojTestInvalidKind2Str =
	_T("{\"id\":\"InvalidKindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":\"randomStr\"}");
static const MojChar* const MojTestInvalidKind3Str =
	_T("{\"id\":\"InvalidKindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"randomStr\",\"props\":\"randomStr\"}]}");
static const MojChar* const MojTestKindIdTooLong =
	_T("{\"id\":")
	_T("\"KindTest2KindTest3KindTest4KindTest5KindTest6KindTest7")
	_T("KindTest8KindTest9KindTest10KindTest11KindTest12KindTest13")
	_T("KindTest14KindTest15KindTest16KindTest17KindTest18KindTest19")
	_T("KindTest20KindTest21KindTest22KindTest23KindTest24KindTest25")
	_T("KindTest26KindTest27KindTest28KindTest29KindTest30KindTest31")
	_T("KindTest32KindTest33KindTest34KindTest35KindTest36KindTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const MojTestKindIndexNameTooLong =
	_T("{\"id\":")
	_T("\"InvalidKindTest2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"KindTest2KindTest3KindTest4Kind")
	_T("Test5KindTest6KindTest7KindTest8KindTest9KindTest10KindTest11")
	_T("KindTest12KindTest13KindTest14KindTest15KindTest16KindTest17")
	_T("KindTest18KindTest19KindTest20KindTest21KindTest22KindTest23")
	_T("KindTest24KindTest25KindTest26KindTest27KindTest28KindTest29")
	_T("KindTest30KindTest31\",\"props\":[{\"name\":\"foo\"}]}]}");
static const MojChar* const MojTestKindIndexNameRepeat =
	_T("{\"id\":\"InvalidKindTest3:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"random\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true},")
	_T("{\"name\":\"random\",\"props\":\"randomStr\"}]}");
static const MojChar* const MojTestKindIndexNameRepeat2 =
	_T("{\"id\":\"InvalidKindTest3:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"random\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true},")
	_T("{\"name\":\"RANDOM\",\"props\":\"randomStr\"}]}");
static const MojChar* const MojTestKindIndexNameId =
	_T("{\"id\":\"InvalidKindTest3:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"_id\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true}]}");
static const MojChar* const MojTestKindIndexNameInvalidChars =
	_T("{\"id\":\"InvalidKindTest3:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"123*abc\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true}]}");

MojDbKindTest::MojDbKindTest()
: MojTestCase(_T("MojDbKind"))
{
}

MojErr MojDbKindTest::run()
{
	MojErr err = testIds();
	MojTestErrCheck(err);
	err = testUpdate();
	MojTestErrCheck(err);
	err = testUpdateWithObjects();
	MojTestErrCheck(err);
	//err = testPermissions();
	MojTestErrCheck(err);
	err = testPutKind();
	MojTestErrCheck(err);
	err = testDelKind();
	MojTestErrCheck(err);
	err = testReparent();
	MojTestErrCheck(err);
	/*err = testPutKindWithPermissions();
	MojTestErrCheck(err);
	err = testDelKindWithPermissions();
	MojTestErrCheck(err);
	err = testPutGetPermissions();
	MojTestErrCheck(err);
	err = testPutBuiltInKinds();
	MojTestErrCheck(err);
	err = testInvalidKinds();
	MojTestErrCheck(err);
	err = testObjectPermissions();
	MojTestErrCheck(err);*/

	return MojErrNone;
}

MojErr MojDbKindTest::testIds()
{
	MojDbKind t(NULL, NULL);
	MojObject obj;
	MojString str;
	MojErr err = str.assign(_T("TestKind:15"));
	MojTestErrCheck(err);
	err = t.init(str);
	MojTestErrCheck(err);
	MojTestAssert(t.id() == _T("TestKind:15"));
	MojTestAssert(t.name() == _T("TestKind"));
	MojTestAssert(t.version() == 15);
	MojDbKind t3(NULL, NULL);
	err = str.assign(_T("TestKind15"));
	MojTestErrCheck(err);
	err = t3.init(str);
	MojTestErrExpected(err, MojErrDbMalformedId);
	MojDbKind t4(NULL, NULL);
	err = str.assign(_T("TestKind:9999999999"));
	MojTestErrCheck(err);
	err = t4.init(str);
	MojTestErrExpected(err, MojErrDbMalformedId);

	return MojErrNone;
}

MojErr MojDbKindTest::testUpdate()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put obj w/o kind
	MojObject obj;
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrExpected(err, MojErrDbKindNotRegistered);
	// put kind
	MojObject kind;
	err = kind.fromJson(MojTestKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// put obj w/ kind
	err = db.put(obj);
	MojTestErrCheck(err);
	// del kind
	MojObject kindId;
	err = kind.getRequired(MojDbServiceDefs::IdKey, kindId);
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(kindId, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	// put obj after del kind
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrExpected(err, MojErrDbKindNotRegistered);
	// re-put kind
	err = kind.fromJson(MojTestKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// put obj w/ kind
	err = db.put(obj);
	MojTestErrCheck(err);

	// query no indexes
	MojDbQuery fooQuery;
	err = fooQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = fooQuery.where(_T("foo"), MojDbQuery::OpEq, 8);
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(fooQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);
	// add index on foo
	MojObject indexes;
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query on foo w/ index
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// delete kind
	err = db.delKind(kindId, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	// put kind back
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query w/ re-added kind
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// del kind
	MojObject indexesId;
	err = indexes.getRequired(MojDbServiceDefs::IdKey, indexesId);
	MojTestErrCheck(err);
	found = false;
	err = db.delKind(indexesId, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	// query after kind deleted
	err = db.find(fooQuery, cursor);
	MojTestErrExpected(err, MojErrDbKindNotRegistered);
	// re-put indexes
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query after re-put indexes
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// replace foo index with bar index
	err = indexes.fromJson(MojTestIndexesBarStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query for foo w/ no foo index
	err = db.find(fooQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);
	// query for bar w/ index
	MojDbQuery barQuery;
	err = barQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = barQuery.where(_T("bar"), MojDbQuery::OpEq, 24);
	MojTestErrCheck(err);
	err = db.find(barQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// add foo index
	err = indexes.fromJson(MojTestIndexesFooBarStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query for foo and bar w/ indexes
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = db.find(barQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// switch order of indexes in array
	err = indexes.fromJson(MojTestIndexesBarFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query for foo and bar w/ indexes
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = db.find(barQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	// replace bar index w/ baz index
	err = indexes.fromJson(MojTestIndexesFooBazStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query for bar w/o index
	err = db.find(barQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);
	// query for foo and baz w/ indexes
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojDbQuery bazQuery;
	err = bazQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = bazQuery.where(_T("baz"), MojDbQuery::OpEq, 24);
	MojTestErrCheck(err);
	err = db.find(bazQuery, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testUpdateWithObjects()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(MojTestKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// put obj w/ kind
	MojObject obj;
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	// query no indexes
	MojDbQuery fooQuery;
	err = fooQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = fooQuery.where(_T("foo"), MojDbQuery::OpLessThan, 8);
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(fooQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);
	// add index on foo
	MojObject indexes;
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query on foo w/ index
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	int count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(foo == count);
		MojTestAssert(bar == count*2);
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// put another obj w/ kind
	MojObject obj2;
	err = obj2.fromJson(MojTestObj2Str);
	MojTestErrCheck(err);
	err = db.put(obj2);
	MojTestErrCheck(err);

	// add index on bar
	MojObject foobarIndex;
	err = foobarIndex.fromJson(MojTestIndexesFooBarStr);
	MojTestErrCheck(err);
	err = db.putKind(foobarIndex);
	MojTestErrCheck(err);
	// query on bar w/ index
	MojDbQuery barQuery;
	err = barQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = barQuery.where(_T("bar"), MojDbQuery::OpLessThan, 8);
	MojTestErrCheck(err);
	err = db.find(barQuery, cursor);
	MojTestErrCheck(err);
	count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(foo == count);
		MojTestAssert(bar == count*2);
	}
	MojTestAssert(count == 2);
	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testPermissions()
{
	/* create this scenario:
	 * GrandparentKind - allows com.palm.* and denies *
	 * ParentKind - allows com.palm.contacts
	 * 				- denies com.palm.mail
	 * ChildKind- doesn't specify any permissions
	 */
/*
	MojDbKind childKind;
	MojDbKind parentKind;
	MojDbKind grandParentKind;

	MojString childName;
	MojErr err = childName.assign(_T("Child:1"));
	MojTestErrCheck(err);
	err = childKind.init(childName, NULL, NULL);
	MojTestErrCheck(err);
	MojString parentName;
	err = parentName.assign(_T("Parent:1"));
	MojTestErrCheck(err);
	err = parentKind.init(parentName, NULL, NULL);
	MojTestErrCheck(err);
	MojString grandparentName;
	err = grandparentName.assign(_T("Grandparent:1"));
	MojTestErrCheck(err);
	err = grandParentKind.init(grandparentName, NULL, NULL);
	MojTestErrCheck(err);

	MojUInt32 allowMask = 0;
	MojUInt32 denyMask = 0;
	MojFlagSet(allowMask, OpCreate, true);
	MojFlagSet(allowMask, OpRead, true);
	MojFlagSet(denyMask, OpUpdate, true);
	err = grandParentKind.setPermission(allowMask, denyMask, _T("com.palm.*"));
	MojTestErrCheck(err);

	allowMask = 0;
	denyMask = 0;
	MojFlagSet(denyMask, OpCreate, true);
	MojFlagSet(denyMask, OpRead, true);
	MojFlagSet(denyMask, OpUpdate, true);
	MojFlagSet(denyMask, OpDelete, true);
	err = grandParentKind.setPermission(allowMask, denyMask, _T("*"));
	MojTestErrCheck(err);

	allowMask = 0;
	denyMask = 0;
	MojFlagSet(allowMask, OpRead, true);
	MojFlagSet(allowMask, OpUpdate, true);
	MojFlagSet(allowMask, OpDelete, true);
	err = parentKind.setPermission(allowMask, denyMask, _T("com.palm.contacts"));
	MojTestErrCheck(err);

	allowMask = 0;
	denyMask = 0;
	MojFlagSet(denyMask, OpCreate, true);
	MojFlagSet(denyMask, OpRead, true);
	MojFlagSet(denyMask, OpDelete, true);
	err = parentKind.setPermission(allowMask, denyMask, _T("com.palm.mail"));
	MojTestErrCheck(err);

	err = childKind.addSuper(&parentKind);
	MojTestErrCheck(err);
	err = parentKind.addSuper(&grandParentKind);
	MojTestErrCheck(err);

	MojDbReq info(false);
	err = info.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);
	//test com.palm.contacts permissions on GrandparentKind
	err = grandParentKind.checkPermission(info, OpCreate, NULL);
	MojTestErrCheck(err);
	err = grandParentKind.checkPermission(info, OpRead, NULL);
	MojTestErrCheck(err);
	err = grandParentKind.checkPermission(info, OpUpdate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = grandParentKind.checkPermission(info, OpDelete, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	//test com.palm.contacts permissions on ParentKind
	err = parentKind.checkPermission(info, OpCreate, NULL);
	MojTestErrCheck(err);
	err = parentKind.checkPermission(info, OpRead, NULL);
	MojTestErrCheck(err);
	err = parentKind.checkPermission(info, OpUpdate, NULL);
	MojTestErrCheck(err);
	err = parentKind.checkPermission(info, OpDelete, NULL);
	MojTestErrCheck(err);

	//test com.palm.contacts permissions on ChildKind
	err = childKind.checkPermission(info, OpCreate, NULL);
	MojTestErrCheck(err);
	err = childKind.checkPermission(info, OpRead, NULL);
	MojTestErrCheck(err);
	err = childKind.checkPermission(info, OpUpdate, NULL);
	MojTestErrCheck(err);
	err = childKind.checkPermission(info, OpDelete, NULL);
	MojTestErrCheck(err);

	MojDbReq mailInfo(false);
	err = mailInfo.domain(_T("com.palm.mail"));
	MojTestErrCheck(err);
	//test com.palm.mail permissions on GrandparentKind
	err = grandParentKind.checkPermission(mailInfo, OpCreate, NULL);
	MojTestErrCheck(err);
	err = grandParentKind.checkPermission(mailInfo, OpRead, NULL);
	MojTestErrCheck(err);
	err = grandParentKind.checkPermission(mailInfo, OpUpdate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = grandParentKind.checkPermission(mailInfo, OpDelete, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	//test com.palm.contacts permissions on ParentKind
	err = parentKind.checkPermission(mailInfo, OpCreate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = parentKind.checkPermission(mailInfo, OpRead, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = parentKind.checkPermission(mailInfo, OpUpdate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = parentKind.checkPermission(mailInfo, OpDelete, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	//test com.palm.contacts permissions on ChildKind
	err = childKind.checkPermission(mailInfo, OpCreate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = childKind.checkPermission(mailInfo, OpRead, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = childKind.checkPermission(mailInfo, OpUpdate, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = childKind.checkPermission(mailInfo, OpDelete, NULL);
	MojTestErrExpected(err, MojErrDbAccessDenied);
*/
	return MojErrNone;
}

MojErr MojDbKindTest::testPutKind()
{
	//setup the test storage engine
#ifdef MOJ_USE_BDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbBerkeleyEngine());
#elif MOJ_USE_LDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbLevelEngine());
#else
    MojRefCountedPtr<MojDbStorageEngine> engine;
#endif
	MojAllocCheck(engine.get());
	MojRefCountedPtr<MojDbStorageEngine> testEngine(new MojDbTestStorageEngine(engine.get()));
	MojAllocCheck(testEngine.get());
	MojErr err = testEngine->open(MojDbTestDir);
	MojTestErrCheck(err);

	MojDb db;
	err = db.open(MojDbTestDir, testEngine.get());
	MojTestErrCheck(err);

	MojObject kind;
	err = kind.fromJson(MojTestKindStr);
	MojTestErrCheck(err);

	// del kind so we start fresh
	MojObject kindId;
	err = kind.getRequired(MojDbServiceDefs::IdKey, kindId);
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(kindId, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	// put kind
	err = db.putKind(kind);
	MojTestErrCheck(err);

	// put obj w/ kind
	MojObject obj;
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	// query no indexes
	MojDbQuery fooQuery;
	err = fooQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = fooQuery.where(_T("foo"), MojDbQuery::OpLessThan, 8);
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(fooQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);
	// add index on foo
	MojObject indexes;
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query on foo w/ index
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	int count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(foo == count);
		MojTestAssert(bar == count*2);
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	MojDbTestStorageEngine* engineImpl = (MojDbTestStorageEngine*)db.storageEngine();
	err = engineImpl->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);

	//add an index on bar
	MojObject barIndex;
	err = barIndex.fromJson(MojTestIndexesBarStr);
	MojTestErrCheck(err);
	err = db.putKind(barIndex);  // this transaction commit should fail
	MojTestErrExpected(err, MojErrDbDeadlock);

	//shouldn't be able to query on bar, but should still be able to query on foo
	MojDbQuery barQuery;
	err = barQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = barQuery.where(_T("bar"), MojDbQuery::OpLessThan, 8);
	MojTestErrCheck(err);
	err = db.find(barQuery, cursor);
	MojTestErrExpected(err, MojErrDbNoIndexForQuery);

	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);

	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testDelKind()
{
	//setup the test storage engine
#ifdef MOJ_USE_BDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbBerkeleyEngine());
#elif MOJ_USE_LDB
	MojRefCountedPtr<MojDbStorageEngine> engine(new MojDbLevelEngine());
#else
    MojRefCountedPtr<MojDbStorageEngine> engine;
#endif
	MojAllocCheck(engine.get());
	MojRefCountedPtr<MojDbStorageEngine> testEngine(new MojDbTestStorageEngine(engine.get()));
	MojAllocCheck(testEngine.get());
	MojErr err = testEngine->open(MojDbTestDir);
	MojErrCheck(err);

	MojDb db;
	err = db.open(MojDbTestDir, testEngine.get());
	MojTestErrCheck(err);

	MojObject kind;
	err = kind.fromJson(MojTestKindStr);
	MojTestErrCheck(err);

	// del everything so we start fresh
	MojObject id;
	err = kind.getRequired(MojDbServiceDefs::IdKey, id);
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	// put kind
	err = db.putKind(kind);
	MojTestErrCheck(err);

	// put obj w/ kind
	MojObject obj;
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);

	// add index on foo
	MojObject indexes;
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);
	// query on foo w/ index
	MojDbCursor cursor;
	MojDbQuery fooQuery;
	err = fooQuery.from(_T("KindTest:1"));
	MojTestErrCheck(err);
	err = fooQuery.where(_T("foo"), MojDbQuery::OpLessThan, 8);
	MojTestErrCheck(err);
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	int count = 0;
	for (;;) {
		found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	// del kind - make sure everything is gone
	MojObject kindId;
	err = kind.getRequired(MojDbServiceDefs::IdKey, kindId);
	MojTestErrCheck(err);
	found = false;
	err = db.delKind(kindId, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	//try to delete all objects again, but nothing should be deleted
	MojDbQuery query;
	err = query.from(MojDbKindEngine::RootKindId);
	MojTestErrCheck(err);
	MojUInt32 delCount = 0;
	err = db.del(query, delCount);
	MojTestErrCheck(err);
	MojTestAssert(delCount == 0);

	//try to put an object of this kind
	err = obj.fromJson(MojTestObjStr);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrExpected(err, MojErrDbKindNotRegistered);

	//put the kind back
	err = indexes.fromJson(MojTestIndexesFooStr);
	MojTestErrCheck(err);
	err = db.putKind(indexes);
	MojTestErrCheck(err);

	//now try to delete the kind with an error on transaction commit
	MojDbTestStorageEngine* engineImpl = (MojDbTestStorageEngine*)db.storageEngine();
	err = engineImpl->setNextError(_T("txn.commit"), MojErrDbDeadlock);
	MojTestErrCheck(err);

	found = false;
	err = db.delKind(kindId, found);  //the transaction commit should fail
	MojTestErrExpected(err, MojErrDbDeadlock);
	MojTestAssert(!found);

	//now putting an object of this kind should still work
	err = db.put(obj);
	MojTestErrCheck(err);

	//and querying for it should also work
	err = db.find(fooQuery, cursor);
	MojTestErrCheck(err);
	count = 0;
	for (;;) {
		bool found = false;
		MojObject obj;
		MojErr err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		++count;
		MojInt64 foo;
		err = obj.getRequired(_T("foo"), foo);
		MojTestErrCheck(err);
		MojInt64 bar;
		err = obj.getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(foo == count);
		MojTestAssert(bar == count*2);
	}
	MojTestAssert(count == 1);
	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testReparent()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// create Kind1 with index on foo
	MojObject kind;
	err = kind.fromJson(_T("{\"id\":\"KindA:1\",\"owner\":\"foo\",\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]}]}"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// create KindB
	err = kind.fromJson(_T("{\"id\":\"KindB:1\",\"owner\":\"foo\"}"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// create object of KindB with foo prop
	MojObject obj;
	err = obj.fromJson(_T("{\"_id\":1000,\"_kind\":\"KindB:1\",\"foo\":1}"));
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	// make KindB extend kindA
	err = kind.fromJson(_T("{\"id\":\"KindB:1\",\"owner\":\"foo\",\"extends\":[\"KindA:1\"]}"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// update foo prop on KindB object
	err = obj.put(_T("foo"), 2);
	MojTestErrCheck(err);
	err = db.put(obj);
	MojTestErrCheck(err);
	// query on KindA
	MojDbQuery query;
	err = query.from(_T("KindA:1"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpGreaterThan, 0);
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(count == 1);
	// unparent KindB from KindA
	err = kind.fromJson(_T("{\"id\":\"KindB:1\",\"owner\":\"foo\"}"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);
	// query on KindA
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	MojTestAssert(count == 0);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testPutKindWithPermissions()
{
/*	MojDb db;
	MojObject conf;
	MojObject dbObj;
	MojErr err = dbObj.putBool(_T("accessControl"), true);
	MojTestErrCheck(err);
	err = conf.put(_T("db"), dbObj);
	MojTestErrCheck(err);

	err = db.configure(conf);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// del everything so we start fresh
	MojDbQuery query;
	err = query.from(MojDbKindEngine::RootKindId);
	MojTestErrCheck(err);
	MojUInt32 delCount;
	err = db.del(query, delCount);
	MojTestErrCheck(err);
	MojTestAssert(delCount > 0);

	MojDbReq contactInfo(false);
	err = contactInfo.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);
	contactInfo.beginBatch();

	MojObject kind;
	err = kind.fromJson(MojTestContactKindStrBadId);
	MojTestErrCheck(err);

	// put kind - kindId is not valid relative to caller
	err = db.putKind(kind, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbMalformedId);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// put kind - owner invalid
	err = kind.putString(MojDbServiceDefs::OwnerKey, _T(""));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidOwner);

	// put same kind with admin privileges - anything goes
	err = kind.putString(MojDbServiceDefs::OwnerKey, _T("com.palm.contacts"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);

	// put same kind - try to change owner
	err = kind.putString(MojDbServiceDefs::OwnerKey, _T("com.palm.calendar"));
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrCheck(err);

	MojDbReq calInfo(false);
	err = calInfo.domain(_T("com.palm.calendar"));
	MojTestErrCheck(err);
	calInfo.beginBatch();

	// put kind - caller is invalid relative to kindId
	MojObject kind2;
	err = kind2.fromJson(MojTestContactKindStrGoodId);
	MojTestErrCheck(err);
	err = db.putKind(kind2, MojDb::FlagNone, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = calInfo.abort();
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	// put same kind with correct caller (kindId's prefix and caller's prefix match)
	contactInfo.beginBatch();
	err = db.putKind(kind2, MojDb::FlagNone, contactInfo);
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// put kind - kindId starts with caller string but doesn't have a '.' after caller
	contactInfo.beginBatch();
	MojObject kind3;
	err = kind3.fromJson(MojTestContact2KindStrBadId);
	MojTestErrCheck(err);
	err = db.putKind(kind3, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbMalformedId);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// put kind - kindId starts with caller and has multiple levels after that
	contactInfo.beginBatch();
	MojObject kind4;
	err = kind4.fromJson(MojTestContact2KindStrGoodId);
	MojTestErrCheck(err);
	err = db.putKind(kind4, MojDb::FlagNone, contactInfo);
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	//have mojodb.admin put the grandfather kind
	MojObject grandfather;
	err = grandfather.fromJson(MojTestGrandparentKindStr);
	MojTestErrCheck(err);
	err = db.putKind(grandfather);
	MojTestErrCheck(err);

	//now have contacts try to put the contact child kind
	contactInfo.beginBatch();
	MojObject contactChild;
	err = contactChild.fromJson(MojTestChildContactKind);
	MojTestErrCheck(err);
	err = db.putKind(contactChild, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	//have mojodb.admin put the contact child kind
	err = db.putKind(contactChild);
	MojTestErrCheck(err);

	// try to update kind4 to extend Grandparent kind
	contactInfo.beginBatch();
	MojObject extends;
	err = extends.fromJson(ExtendsValue);
	MojTestErrCheck(err);
	err = kind4.put(_T("extends"), extends);
	MojTestErrCheck(err);
	err = db.putKind(kind4, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// try to put a kind with an invalid id
	MojObject invalidKind;
	err = invalidKind.fromJson(MojTestInvalidKindStr);
	MojTestErrCheck(err);

	err = db.putKind(invalidKind);
	MojTestErrExpected(err, MojErrDbMalformedId);

	return MojErrNone;
}

MojErr MojDbKindTest::testDelKindWithPermissions()
{
	MojDb db;
	MojObject conf;
	MojObject dbObj;
	MojErr err = dbObj.putBool(_T("accessControl"), true);
	MojTestErrCheck(err);
	err = conf.put(_T("db"), dbObj);
	MojTestErrCheck(err);

	err = db.configure(conf);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	MojDbReq contactInfo(false);
	err = contactInfo.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);

	MojDbReq calInfo(false);
	err = calInfo.domain(_T("com.palm.calendar"));
	MojTestErrCheck(err);

	//when you delete a kind, the caller must be the owner of the kind OR mojodb.admin
	MojObject contactKind;
	err = contactKind.fromJson(MojTestContactKindStrGoodId);
	MojTestErrCheck(err);

	// try deleting using calendar as caller - should fail
	calInfo.beginBatch();
	MojObject id;
	err = contactKind.getRequired(MojDbServiceDefs::IdKey, id);
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(id, found, MojDb::FlagNone, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	MojTestAssert(!found);
	err = calInfo.abort();
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	// delete with contacts as caller
	contactInfo.beginBatch();
	err = db.delKind(id, found, MojDb::FlagNone, contactInfo);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// delete a contact owned kind with mojodb.admin as caller
	MojObject contactKind2;
	err = contactKind2.fromJson(MojTestContact2KindStrGoodId);
	MojTestErrCheck(err);

	err = contactKind2.getRequired(MojDbServiceDefs::IdKey, id);
	MojTestErrCheck(err);
	err = db.delKind(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	// delete a mojodb.admin owned kind with contacts as caller
	MojObject grandfather;
	err = grandfather.fromJson(MojTestGrandparentKindStr);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = grandfather.getRequired(MojDbServiceDefs::IdKey, id);
	MojTestErrCheck(err);
	err = db.delKind(id, found, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	MojTestAssert(!found);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// delete it with mojodb.admin as caller
	err = db.delKind(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);

	return MojErrNone;
}

MojErr MojDbKindTest::testPutGetPermissions()
{
	MojDb db;
	MojObject conf;
	MojObject dbObj;
	MojErr err = dbObj.putBool(_T("accessControl"), true);
	MojTestErrCheck(err);
	err = conf.put(_T("db"), dbObj);
	MojTestErrCheck(err);

	err = db.configure(conf);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	MojDbReq contactInfo(false);
	err = contactInfo.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);

	MojDbReq calInfo(false);
	err = calInfo.domain(_T("com.palm.calendar"));
	MojTestErrCheck(err);

	//when you read permissions on a kind, the caller must be the owner of the kind OR mojodb.admin
	MojObject contactKind;
	err = contactKind.fromJson(MojTestContactKindStrGoodId);
	MojTestErrCheck(err);

	// try reading permissions using calendar as caller - should fail
	MojString permissionType;
	err = permissionType.assign(MojDbServiceDefs::DbKindValue);
	MojTestErrCheck(err);
	MojString kindId;
	err = kindId.assign(KindId);
	MojTestErrCheck(err);

	MojDbCursor cursor;
	calInfo.beginBatch();
	err = db.getPermissions(permissionType, kindId, cursor, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = cursor.close();
	MojTestErrCheck(err);
	err = calInfo.abort();
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	// read permissions with contacts as caller
	contactInfo.beginBatch();
	err = db.getPermissions(permissionType, kindId, cursor, contactInfo);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	// read permissions with admin caller
	err = db.getPermissions(permissionType, kindId, cursor);
	MojTestErrCheck(err);
	err = cursor.close();
	MojTestErrCheck(err);

	MojObject permission;
	err = permission.putString(MojDbServiceDefs::TypeKey, MojDbServiceDefs::DbKindValue);
	MojTestErrCheck(err);
	err = permission.putString(MojDbServiceDefs::CallerKey, _T("com.palm.calendar"));
	MojTestErrCheck(err);
	err = permission.putString(MojDbServiceDefs::ObjectKey, KindId);
	MojTestErrCheck(err);
	MojObject operations;
	err = operations.putString(MojDbServiceDefs::CreateKey, MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);
	err = permission.put(MojDbServiceDefs::OperationsKey, operations);
	MojTestErrCheck(err);

	//try to put a child contact object from calendar
	calInfo.beginBatch();
	MojObject obj;
	err = obj.fromJson(TestChildContactObject);
	MojTestErrCheck(err);
	err = db.put(obj, MojDb::FlagNone, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = calInfo.abort();
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	calInfo.beginBatch();
	err = db.putPermissions(&permission, &permission + 1, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = calInfo.abort();
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putPermissions(&permission, &permission + 1, contactInfo);
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	//now that permissions have been set, create an object of that kind from calendar
	calInfo.beginBatch();
	err = db.put(obj, MojDb::FlagNone, calInfo);
	MojTestErrCheck(err);
	err = calInfo.endBatch();
	MojTestErrCheck(err);

	// try to put permissions on a kind that doesn't exist
	MojObject permission2;
	err = permission2.putString(MojDbServiceDefs::TypeKey, MojDbServiceDefs::DbKindValue);
	MojTestErrCheck(err);
	err = permission2.putString(MojDbServiceDefs::CallerKey, _T("com.palm.calendar"));
	MojTestErrCheck(err);
	err = permission2.putString(MojDbServiceDefs::ObjectKey, _T("invalidId"));
	MojTestErrCheck(err);
	MojObject operations2;
	err = operations2.putString(MojDbServiceDefs::CreateKey, MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);
	err = permission2.put(MojDbServiceDefs::OperationsKey, operations2);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putPermissions(&permission2, &permission2 + 1, contactInfo);
	MojTestErrExpected(err, MojErrDbMalformedId);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	MojObject invalidOperationKey;
	err = invalidOperationKey.putString(_T("fakeKey"), MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);

	err = permission.put(MojDbServiceDefs::OperationsKey, invalidOperationKey);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putPermissions(&permission, &permission + 1, contactInfo);
	MojTestErrExpected(err, MojErrDbInvalidOperation);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	MojObject invalidOperationValue;
	err = invalidOperationValue.putString(MojDbServiceDefs::CreateKey, _T("fakeValue"));
	MojTestErrCheck(err);

	err = permission.put(MojDbServiceDefs::OperationsKey, invalidOperationValue);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putPermissions(&permission, &permission + 1, contactInfo);
	MojTestErrExpected(err, MojErrDbInvalidOperation);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);
*/
	return MojErrNone;
}

MojErr MojDbKindTest::testPutBuiltInKinds()
{
	MojDb db;
	MojObject conf;
	MojObject dbObj;
	MojErr err = dbObj.putBool(_T("accessControl"), true);
	MojTestErrCheck(err);
	err = conf.put(_T("db"), dbObj);
	MojTestErrCheck(err);

	err = db.configure(conf);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	MojDbReq contactInfo(false);
	err = contactInfo.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);

	MojObject rootKind;
	err = rootKind.fromJson(MojTestObjectKindStr);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putKind(rootKind, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	MojObject rootKind2;
	err = rootKind2.fromJson(MojTestObjectKindStr2);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.putKind(rootKind2, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	//try putting kind with admin privileges
	err = db.putKind(rootKind2, MojDb::FlagNone);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	MojObject permission;
	err = permission.fromJson(TestPermissionObject);
	MojTestErrCheck(err);

	contactInfo.beginBatch();
	err = db.put(permission, MojDb::FlagNone, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	MojDbQuery query;
	err = query.from(_T("Permission:1"));
	MojTestErrCheck(err);

	MojDbCursor cursor;
	contactInfo.beginBatch();
	err = db.find(query, cursor, contactInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = cursor.close();
	MojTestErrCheck(err);
	err = contactInfo.abort();
	MojTestErrCheck(err);
	err = contactInfo.endBatch();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindTest::testInvalidKinds()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(MojTestInvalidKind2Str);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	// this doesn't throw an error for now - schema validation will catch this.  
	// adding this test just to make sure we don't crash
	MojTestErrCheck(err);

	// put another kind
	err = kind.fromJson(MojTestInvalidKind3Str);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndex);

	// put kind
	err = kind.fromJson(MojTestKindIdTooLong);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbMalformedId);

	// invalid index names
	err = kind.fromJson(MojTestKindIndexNameTooLong);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndexName);

	err = kind.fromJson(MojTestKindIndexNameRepeat);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndexName);

	err = kind.fromJson(MojTestKindIndexNameRepeat2);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndexName);

	err = kind.fromJson(MojTestKindIndexNameId);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndexName);

	err = kind.fromJson(MojTestKindIndexNameInvalidChars);
	MojTestErrCheck(err);
	err = db.putKind(kind);
	MojTestErrExpected(err, MojErrDbInvalidIndexName);

	return MojErrNone;
}

MojErr MojDbKindTest::testObjectPermissions()
{
	/* create this scenario:
	 * GrandparentKind - allows com.palm.* and denies *
	 * ParentKind - allows com.palm.contacts
	 * 				- denies com.palm.mail
	 * ChildKind- doesn't specify any permissions
	 */

	MojDb db;
	MojObject conf;
	MojObject dbObj;
	MojErr err = dbObj.putBool(_T("accessControl"), true);
	MojTestErrCheck(err);
	err = conf.put(_T("db"), dbObj);
	MojTestErrCheck(err);

	err = db.configure(conf);
	MojTestErrCheck(err);
	err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// del everything so we start fresh
	MojDbQuery query;
	err = query.from(MojDbKindEngine::RootKindId);
	MojTestErrCheck(err);
	MojUInt32 delCount;
	err = db.del(query, delCount);
	MojTestErrCheck(err);
	MojTestAssert(delCount > 0);

	MojDbReq adminInfo(true);
	adminInfo.beginBatch();

	MojObject kind;
	err = kind.fromJson(MojTestGrandparentKindStr);
	MojTestErrCheck(err);

	err = db.putKind(kind, MojDb::FlagNone, adminInfo);
	MojTestErrCheck(err);
	err = kind.fromJson(MojTestParentKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind, MojDb::FlagNone, adminInfo);
	MojTestErrCheck(err);
	err = kind.fromJson(MojTestChildKindStr);
	MojTestErrCheck(err);
	err = db.putKind(kind, MojDb::FlagNone, adminInfo);
	MojTestErrCheck(err);
	err = adminInfo.endBatch();
	MojTestErrCheck(err);


	adminInfo.beginBatch();
	MojObject permission;
	err = permission.putString(MojDbServiceDefs::TypeKey, MojDbKind::PermissionType);
	MojTestErrCheck(err);
	err = permission.putString(MojDbServiceDefs::CallerKey, _T("com.palm.contacts"));
	MojTestErrCheck(err);
	err = permission.putString(MojDbServiceDefs::ObjectKey, _T("ParentKindTest:1"));
	MojTestErrCheck(err);
	MojObject operation;
	err = operation.putString(MojDbServiceDefs::ReadKey, MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);
	err = permission.put(MojDbServiceDefs::OperationsKey, operation);
	MojTestErrCheck(err);

	MojObject permissionsArray;
	err = permissionsArray.push(permission);
	MojTestErrCheck(err);

	MojObject permission2;
	err = permission2.putString(MojDbServiceDefs::TypeKey, MojDbKind::PermissionType);
	MojTestErrCheck(err);
	err = permission2.putString(MojDbServiceDefs::CallerKey, _T("com.palm.calendar"));
	MojTestErrCheck(err);
	err = permission2.putString(MojDbServiceDefs::ObjectKey, _T("ChildKindTest:1"));
	MojTestErrCheck(err);
	err = permission2.put(MojDbServiceDefs::OperationsKey, operation);
	MojTestErrCheck(err);

	err = permissionsArray.push(permission2);
	MojTestErrCheck(err);

	MojObject permission3;
	err = permission3.putString(MojDbServiceDefs::TypeKey, MojDbKind::PermissionType);
	MojTestErrCheck(err);
	err = permission3.putString(MojDbServiceDefs::CallerKey, _T("com.palm.messaging"));
	MojTestErrCheck(err);
	err = permission3.putString(MojDbServiceDefs::ObjectKey, _T("ParentKindTest:1"));
	MojTestErrCheck(err);
	err = operation.putString(MojDbServiceDefs::UpdateKey, MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);
	err = operation.putString(MojDbServiceDefs::DeleteKey, MojDbServiceDefs::AllowValue);
	MojTestErrCheck(err);
	err = permission3.put(MojDbServiceDefs::OperationsKey, operation);
	MojTestErrCheck(err);

	err = permissionsArray.push(permission3);
	MojTestErrCheck(err);

	MojObject permission4;
	err = permission4.putString(MojDbServiceDefs::TypeKey, MojDbKind::PermissionType);
	MojTestErrCheck(err);
	err = permission4.putString(MojDbServiceDefs::CallerKey, _T("com.palm.messaging"));
	MojTestErrCheck(err);
	err = permission4.putString(MojDbServiceDefs::ObjectKey, _T("ChildKindTest:1"));
	MojTestErrCheck(err);
	MojObject operation2;
	err = operation2.putString(MojDbServiceDefs::ReadKey, MojDbServiceDefs::DenyValue);
	MojTestErrCheck(err);
	err = operation2.putString(MojDbServiceDefs::UpdateKey, MojDbServiceDefs::DenyValue);
	MojTestErrCheck(err);
	err = operation2.putString(MojDbServiceDefs::DeleteKey, MojDbServiceDefs::DenyValue);
	MojTestErrCheck(err);
	err = permission4.put(MojDbServiceDefs::OperationsKey, operation2);
	MojTestErrCheck(err);

	err = permissionsArray.push(permission4);
	MojTestErrCheck(err);

	MojObject::ArrayIterator begin;
	err = permissionsArray.arrayBegin(begin);
	MojErrCheck(err);
	MojObject::ConstArrayIterator end = permissionsArray.arrayEnd();
	err = db.putPermissions(begin, end, adminInfo);
	MojTestErrCheck(err);
	adminInfo.endBatch();
	MojTestErrCheck(err);

	MojObject parent;
	err = parent.fromJson(MojTestParentObjStr);
	MojTestErrCheck(err);
	MojObject child;
	err = child.fromJson(MojTestChildObjStr);
	MojTestErrCheck(err);

	MojObject objects;
	err = objects.push(parent);
	MojTestErrCheck(err);
	err = objects.push(child);
	MojTestErrCheck(err);

	adminInfo.beginBatch();
	MojObject::ArrayIterator objBegin;
	err = objects.arrayBegin(objBegin);
	err = db.put(objBegin, objects.arrayEnd(), MojDb::FlagNone, adminInfo);
	MojTestErrCheck(err);
	err = adminInfo.endBatch();
	MojTestErrCheck(err);

	MojDbReq calInfo(false);
	err = calInfo.domain(_T("com.palm.calendar"));
	MojTestErrCheck(err);

	MojDbReq contactsInfo(false);
	err = contactsInfo.domain(_T("com.palm.contacts"));
	MojTestErrCheck(err);

	MojString parentId;
	err = parentId.assign(_T("myParent"));
	MojTestErrCheck(err);

	MojString childId;
	err = childId.assign(_T("myChild"));
	MojTestErrCheck(err);

	// cal can read child
	MojObject childFromDB;
	bool found;
	err = db.get(childId, childFromDB, found, calInfo);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = childFromDB.del(MojDb::RevKey, found);
	MojTestErrCheck(err);
	MojTestAssert(child == childFromDB);

	// contacts can read child since it can read the parent
	err = db.get(childId, childFromDB, found, contactsInfo);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = childFromDB.del(MojDb::RevKey, found);
	MojTestErrCheck(err);
	MojTestAssert(child == childFromDB);

	// contacts can read parent
	MojObject parentFromDB;
	err = db.get(parentId, parentFromDB, found, contactsInfo);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = parentFromDB.del(MojDb::RevKey, found);
	MojTestErrCheck(err);
	MojTestAssert(parent == parentFromDB);

	// cal cannot read parent
	err = db.get(parentId, parentFromDB, found, calInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	// random can't read either
	MojDbReq randomInfo(false);
	err = randomInfo.domain(_T("random"));
	MojTestErrCheck(err);
	err = db.get(childId, childFromDB, found, randomInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);
	err = db.get(parentId, parentFromDB, found, randomInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	// messaging can read parent but not child
	MojDbReq messagingInfo(false);
	err = messagingInfo.domain(_T("com.palm.messaging"));
	MojTestErrCheck(err);
	err = db.get(parentId, parentFromDB, found, messagingInfo);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = parentFromDB.del(MojDb::RevKey, found);
	MojTestErrCheck(err);
	MojTestAssert(parent == parentFromDB);
	err = db.get(childId, childFromDB, found, messagingInfo);
	MojTestErrExpected(err, MojErrDbAccessDenied);

	MojDbQuery q;
	err = q.from(_T("ParentKindTest:1"));
	MojTestErrCheck(err);

	MojDbCursor cursor;
	err = db.find(q, cursor, messagingInfo);
	MojTestErrCheck(err);
	MojJsonWriter writer;
	err = writer.beginArray();
	MojTestErrCheck(err);
	err = cursor.visit(writer);
	MojTestErrCheck(err);
	err = writer.endArray();
	MojTestErrCheck(err);

	err = cursor.close();
	MojTestErrCheck(err);

	MojObject results;
	err = results.fromJson(writer.json());
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	for (MojObject::ConstArrayIterator i = results.arrayBegin(); i != results.arrayEnd(); ++i) {
		count++;
	}
	MojTestAssert(count == 1);

	MojObject props;
	err = props.put(_T("foo"), 5);
	MojTestErrCheck(err);
	count = 0;
	err = db.merge(q, props, count, MojDb::FlagNone, messagingInfo);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);

	count = 0;
	err = db.del(q, count, MojDb::FlagNone, messagingInfo);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);

	count = 0;
	err = db.find(q, cursor, messagingInfo);
	MojTestErrCheck(err);
	err = writer.reset();
	MojTestErrCheck(err);
	err = writer.beginArray();
	MojTestErrCheck(err);
	err = cursor.visit(writer);
	MojTestErrCheck(err);
	err = writer.endArray();
	MojTestErrCheck(err);

	err = cursor.close();
	MojTestErrCheck(err);

	err = results.fromJson(writer.json());
	MojTestErrCheck(err);
	for (MojObject::ConstArrayIterator i = results.arrayBegin(); i != results.arrayEnd(); ++i) {
		count++;
	}
	MojTestAssert(count == 0);

	return MojErrNone;
}

void MojDbKindTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
