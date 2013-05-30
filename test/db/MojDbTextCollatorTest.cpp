/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#include "MojDbTextCollatorTest.h"
#include "db/MojDb.h"
#include "db/MojDbTextCollator.h"
#include "db/MojDbKey.h"

static const MojChar* const MojCollatorKindStr =
	_T("{\"id\":\"CollatorTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"primary\"}]},")
	_T("]}");
static const MojChar* const MojCollatorTestObjects[] = {
	_T("{\"_id\":1,\"_kind\":\"CollatorTest:1\",\"foo\":\"APPLE\"}"),
	_T("{\"_id\":0,\"_kind\":\"CollatorTest:1\",\"foo\":\"aardvark\"}"),
	_T("{\"_id\":3,\"_kind\":\"CollatorTest:1\",\"foo\":\"BOY\"}"),
	_T("{\"_id\":2,\"_kind\":\"CollatorTest:1\",\"foo\":\"ball\"}"),
};

MojDbTextCollatorTest::MojDbTextCollatorTest()
: MojTestCase(_T("MojDbTextCollator"))
{
}

MojDbTextCollatorTest::~MojDbTextCollatorTest()
{
}

MojErr MojDbTextCollatorTest::run()
{
	MojErr err = simpleTest();
	MojErrCheck(err);
	err = queryTest();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbTextCollatorTest::simpleTest()
{
	MojDbTextCollator colEn1;
	MojErr err = colEn1.init(_T("en_US"), MojDbCollationPrimary);
	MojTestErrCheck(err);
	MojDbTextCollator colEn2;
	err = colEn2.init(_T("en_US"), MojDbCollationSecondary);
	MojTestErrCheck(err);
	MojDbTextCollator colEn3;
	err = colEn3.init(_T("en_US"), MojDbCollationTertiary);
	MojTestErrCheck(err);

	MojString str0;
	MojString str1;
	err = str1.assign(_T("cote"));
	MojTestErrCheck(err);
	MojString str2;
	err = str2.assign(_T("CotE"));
	MojTestErrCheck(err);
	MojString str3;
	err = str3.assign(_T("coté"));
	MojTestErrCheck(err);
	MojString str4;
	err = str4.assign(_T("côte"));
	MojTestErrCheck(err);
	MojString str5;
	err = str5.assign(_T("côTe"));
	MojTestErrCheck(err);
	MojString str6;
	err = str6.assign(_T("carap"));
	MojTestErrCheck(err);
	MojString str7;
	err = str7.assign(_T("f"));
	MojTestErrCheck(err);
	MojString str8;
	err = str8.assign(_T("four"));
	MojTestErrCheck(err);

	// primary strength
	MojDbKey key1;
	err = colEn1.sortKey(str0, key1);
	MojTestErrCheck(err);
	MojDbKey key2;
	err = colEn1.sortKey(str0, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	err = colEn1.sortKey(str1, key1);
	MojTestErrCheck(err);
	err = colEn1.sortKey(str1, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	err = colEn1.sortKey(str2, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	MojDbKey key3;
	err = colEn1.sortKey(str3, key3);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key3);
	MojDbKey key4;
	err = colEn1.sortKey(str4, key4);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key4);
	MojDbKey key5;
	err = colEn1.sortKey(str5, key5);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key5);
	MojDbKey key6;
	err = colEn1.sortKey(str6, key6);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key6);

	// secondary strength
	err = colEn2.sortKey(str1, key1);
	MojTestErrCheck(err);
	err = colEn2.sortKey(str1, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	err = colEn2.sortKey(str2, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	err = colEn2.sortKey(str3, key3);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key3);
	err = colEn2.sortKey(str4, key4);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key4);
	MojTestAssert(key3 != key4);
	err = colEn2.sortKey(str5, key5);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key5);
	MojTestAssert(key4 == key5);
	err = colEn2.sortKey(str6, key6);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key6);

	// tertiary strength
	err = colEn3.sortKey(str1, key1);
	MojTestErrCheck(err);
	err = colEn3.sortKey(str1, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 == key2);
	err = colEn3.sortKey(str2, key2);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key2);
	err = colEn3.sortKey(str3, key3);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key3);
	err = colEn3.sortKey(str4, key4);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key4);
	MojTestAssert(key3 != key4);
	err = colEn3.sortKey(str5, key5);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key5);
	MojTestAssert(key4 != key5);
	err = colEn3.sortKey(str6, key6);
	MojTestErrCheck(err);
	MojTestAssert(key1 != key6);

	// prefix
	err = colEn1.sortKey(str7, key1);
	MojTestErrCheck(err);
	err = colEn1.sortKey(str8, key2);
	MojTestErrCheck(err);
	err = key1.byteVec().pop();
	MojTestErrCheck(err);
	MojTestAssert(key1.prefixOf(key2));

	return MojErrNone;
}


MojErr MojDbTextCollatorTest::queryTest()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put type
	MojObject obj;
	err = obj.fromJson(MojCollatorKindStr);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// put test objects
	for (gsize i = 0; i < sizeof(MojCollatorTestObjects) / sizeof(MojChar*); ++i) {
		MojObject obj;
		err = obj.fromJson(MojCollatorTestObjects[i]);
		MojTestErrCheck(err);
		err = db.put(obj);
		MojTestErrCheck(err);
	}

	// verify find result order
	MojDbQuery query;
	err = query.from(_T("CollatorTest:1"));
	MojTestErrCheck(err);
	err = query.order(_T("foo"));
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	bool found = false;
	gint64 index = 0;
	for (;;) {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		gint64 id = 0;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		MojTestAssert(id == index++);
	}
	err = cursor.close();
	MojTestErrCheck(err);

	// verify prefix match
	MojString val;
	err = val.assign(_T("a"));
	MojTestErrCheck(err);
	err = query.where(_T("foo"), MojDbQuery::OpPrefix, val, MojDbCollationPrimary);
	MojTestErrCheck(err);
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	index = 0;
	for (;;) {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		gint64 id = 0;
		err = obj.getRequired(MojDb::IdKey, id);
		MojTestErrCheck(err);
		MojTestAssert(id == index++);
	}
	MojTestAssert(index == 2);
	err = cursor.close();
	MojTestErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

void MojDbTextCollatorTest::cleanup()
{
	(void) MojRmDirRecursive(MojDbTestDir);
}
