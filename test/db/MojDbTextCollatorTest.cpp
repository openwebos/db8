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


#include "MojDbTextCollatorTest.h"
#include "db/MojDb.h"
#include "db/MojDbTextCollator.h"
#include "db/MojDbKey.h"

static const MojChar* const MojCollatorKindStr1 =
	_T("{\"id\":\"CollatorTest:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"primary\"}]},")
	_T("]}");
static const MojChar* const MojCollatorTestObjects1[] = {
	_T("{\"_id\":1,\"_kind\":\"CollatorTest:1\",\"foo\":\"APPLE\"}"),
	_T("{\"_id\":0,\"_kind\":\"CollatorTest:1\",\"foo\":\"aardvark\"}"),
	_T("{\"_id\":3,\"_kind\":\"CollatorTest:1\",\"foo\":\"BOY\"}"),
	_T("{\"_id\":2,\"_kind\":\"CollatorTest:1\",\"foo\":\"ball\"}"),
};

static const MojChar* const MojCollatorKindStr2 =
    _T("{\"id\":\"CollatorTest:2\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[")
        _T("{\"name\":\"filename\",\"props\":[{\"name\":\"filename\",\"collate\":\"identical\"}]},")
    _T("]}");
static const MojChar* const MojCollatorTestObjects2[] = {
    _T("{\"_id\":66,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB\u30AD\"}"),
    _T("{\"_id\":65,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB021\u30AD\"}"),
    _T("{\"_id\":64,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB1\u30AD\"}"),
    _T("{\"_id\":63,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB01\u30AD\"}"),
    _T("{\"_id\":62,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB001\u30AD\"}"),
    _T("{\"_id\":61,\"_kind\":\"CollatorTest:2\",\"filename\":\"\u30AB \u30AD\"}"),
    _T("{\"_id\":60,\"_kind\":\"CollatorTest:2\",\"filename\":\"\uFF76\"}"),
    _T("{\"_id\":59,\"_kind\":\"CollatorTest:2\",\"filename\":\"c1\"}"),
    _T("{\"_id\":58,\"_kind\":\"CollatorTest:2\",\"filename\":\"c01\"}"),
    _T("{\"_id\":57,\"_kind\":\"CollatorTest:2\",\"filename\":\"c001\"}"),
    _T("{\"_id\":56,\"_kind\":\"CollatorTest:2\",\"filename\":\"b1\"}"),
    _T("{\"_id\":55,\"_kind\":\"CollatorTest:2\",\"filename\":\"b01\"}"),
    _T("{\"_id\":54,\"_kind\":\"CollatorTest:2\",\"filename\":\"b001\"}"),
    _T("{\"_id\":53,\"_kind\":\"CollatorTest:2\",\"filename\":\"B\"}"),
    _T("{\"_id\":52,\"_kind\":\"CollatorTest:2\",\"filename\":\"b\"}"),
    _T("{\"_id\":51,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC021A1\"}"),
    _T("{\"_id\":50,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC10A1\"}"),
    _T("{\"_id\":49,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC001D1\"}"),
    _T("{\"_id\":48,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC1A1\"}"),
    _T("{\"_id\":47,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC01A1\"}"),
    _T("{\"_id\":46,\"_kind\":\"CollatorTest:2\",\"filename\":\"ABC001A1\"}"),
    _T("{\"_id\":45,\"_kind\":\"CollatorTest:2\",\"filename\":\"abc.divx\"}"),
    _T("{\"_id\":44,\"_kind\":\"CollatorTest:2\",\"filename\":\"abc.avi\"}"),
    _T("{\"_id\":43,\"_kind\":\"CollatorTest:2\",\"filename\":\"abc_d.mpeg\"}"),
    _T("{\"_id\":42,\"_kind\":\"CollatorTest:2\",\"filename\":\"abc\"}"),
    _T("{\"_id\":41,\"_kind\":\"CollatorTest:2\",\"filename\":\"ab.mpg\"}"),
    _T("{\"_id\":40,\"_kind\":\"CollatorTest:2\",\"filename\":\"aB\"}"),
    _T("{\"_id\":39,\"_kind\":\"CollatorTest:2\",\"filename\":\"ab\"}"),
    _T("{\"_id\":38,\"_kind\":\"CollatorTest:2\",\"filename\":\"a1_1\"}"),
    _T("{\"_id\":37,\"_kind\":\"CollatorTest:2\",\"filename\":\"a1\"}"),
    _T("{\"_id\":36,\"_kind\":\"CollatorTest:2\",\"filename\":\"a01\"}"),
    _T("{\"_id\":35,\"_kind\":\"CollatorTest:2\",\"filename\":\"a001\"}"),
    _T("{\"_id\":34,\"_kind\":\"CollatorTest:2\",\"filename\":\"a-b\"}"),
    _T("{\"_id\":33,\"_kind\":\"CollatorTest:2\",\"filename\":\"A\"}"),
    _T("{\"_id\":32,\"_kind\":\"CollatorTest:2\",\"filename\":\"a\"}"),
    _T("{\"_id\":31,\"_kind\":\"CollatorTest:2\",\"filename\":\"1235 Main St.\"}"),
    _T("{\"_id\":30,\"_kind\":\"CollatorTest:2\",\"filename\":\"123 Main St.\"}"),
    _T("{\"_id\":29,\"_kind\":\"CollatorTest:2\",\"filename\":\"100\"}"),
    _T("{\"_id\":28,\"_kind\":\"CollatorTest:2\",\"filename\":\"23 Main St.\"}"),
    _T("{\"_id\":27,\"_kind\":\"CollatorTest:2\",\"filename\":\"021\"}"),
    _T("{\"_id\":26,\"_kind\":\"CollatorTest:2\",\"filename\":\"11\"}"),
    _T("{\"_id\":25,\"_kind\":\"CollatorTest:2\",\"filename\":\"10\"}"),
    _T("{\"_id\":24,\"_kind\":\"CollatorTest:2\",\"filename\":\"3 Main St.\"}"),
    _T("{\"_id\":23,\"_kind\":\"CollatorTest:2\",\"filename\":\"1\"}"),
    _T("{\"_id\":22,\"_kind\":\"CollatorTest:2\",\"filename\":\"01\"}"),
    _T("{\"_id\":21,\"_kind\":\"CollatorTest:2\",\"filename\":\"001\"}"),
    _T("{\"_id\":20,\"_kind\":\"CollatorTest:2\",\"filename\":\"~\"}"),
    _T("{\"_id\":19,\"_kind\":\"CollatorTest:2\",\"filename\":\"=\"}"),
    _T("{\"_id\":18,\"_kind\":\"CollatorTest:2\",\"filename\":\"+\"}"),
    _T("{\"_id\":17,\"_kind\":\"CollatorTest:2\",\"filename\":\"^\"}"),
    _T("{\"_id\":16,\"_kind\":\"CollatorTest:2\",\"filename\":\"`\"}"),
    _T("{\"_id\":15,\"_kind\":\"CollatorTest:2\",\"filename\":\"%\"}"),
    _T("{\"_id\":14,\"_kind\":\"CollatorTest:2\",\"filename\":\"#\"}"),
    _T("{\"_id\":13,\"_kind\":\"CollatorTest:2\",\"filename\":\"&\"}"),
    _T("{\"_id\":12,\"_kind\":\"CollatorTest:2\",\"filename\":\"*\"}"),
    _T("{\"_id\":11,\"_kind\":\"CollatorTest:2\",\"filename\":\"@\"}"),
    _T("{\"_id\":10,\"_kind\":\"CollatorTest:2\",\"filename\":\"}\"}"),
    _T("{\"_id\":9,\"_kind\":\"CollatorTest:2\",\"filename\":\"{\"}"),
    _T("{\"_id\":8,\"_kind\":\"CollatorTest:2\",\"filename\":\"]\"}"),
    _T("{\"_id\":7,\"_kind\":\"CollatorTest:2\",\"filename\":\"[\"}"),
    _T("{\"_id\":6,\"_kind\":\"CollatorTest:2\",\"filename\":\"!\"}"),
    _T("{\"_id\":5,\"_kind\":\"CollatorTest:2\",\"filename\":\";\"}"),
    _T("{\"_id\":4,\"_kind\":\"CollatorTest:2\",\"filename\":\",\"}"),
    _T("{\"_id\":3,\"_kind\":\"CollatorTest:2\",\"filename\":\"-\"}"),
    _T("{\"_id\":2,\"_kind\":\"CollatorTest:2\",\"filename\":\"_\"}"),
    _T("{\"_id\":1,\"_kind\":\"CollatorTest:2\",\"filename\":\" \"}")
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
    MojErr err = simpleTest1();
	MojErrCheck(err);
    err = queryTest1();
	MojErrCheck(err);

    err = simpleTest2();
    MojErrCheck(err);
    err = queryTest2();
    MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbTextCollatorTest::simpleTest1()
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

MojErr MojDbTextCollatorTest::simpleTest2()
{
    MojDbTextCollator colEn1;
    MojErr err = colEn1.init(_T("en_US"), MojDbCollationIdentical);
    MojTestErrCheck(err);;

    MojString str1;
    err = str1.assign(_T("a01b"));
    MojTestErrCheck(err);
    MojString str2;
    err = str2.assign(_T("a001b"));
    MojTestErrCheck(err);
    MojString str3;
    err = str3.assign(_T("23 Main St."));
    MojTestErrCheck(err);
    MojString str4;
    err = str4.assign(_T("123 Main St."));
    MojTestErrCheck(err);
    MojString str5;
    err = str5.assign(_T("a"));
    MojTestErrCheck(err);
    MojString str6;
    err = str6.assign(_T("A"));
    MojTestErrCheck(err);
    MojString str7;
    err = str7.assign(_T("\u30A2")); // Full-width Kana
    MojTestErrCheck(err);
    MojString str8;
    err = str8.assign(_T("\uFF71")); // Half-width Kana
    MojTestErrCheck(err);

    // identical strength
    MojDbKey key1;
    err = colEn1.sortKey(str1, key1);
    MojTestErrCheck(err);
    MojDbKey key2;
    err = colEn1.sortKey(str2, key2);
    MojTestErrCheck(err);
    MojTestAssert(key1 > key2);

    MojDbKey key3;
    err = colEn1.sortKey(str3, key3);
    MojTestErrCheck(err);
    MojDbKey key4;
    err = colEn1.sortKey(str4, key4);
    MojTestErrCheck(err);
    MojTestAssert(key3 < key4);

    MojDbKey key5;
    err = colEn1.sortKey(str5, key5);
    MojTestErrCheck(err);
    MojDbKey key6;
    err = colEn1.sortKey(str6, key6);
    MojTestErrCheck(err);
    MojTestAssert(key5 < key6);

    MojDbKey key7;
    err = colEn1.sortKey(str7, key7);
    MojTestErrCheck(err);
    MojDbKey key8;
    err = colEn1.sortKey(str8, key8);
    MojTestErrCheck(err);
    MojTestAssert(key7 < key8);

    return MojErrNone;
}

MojErr MojDbTextCollatorTest::queryTest1()
{
	MojDb db;
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// put type
	MojObject obj;
    err = obj.fromJson(MojCollatorKindStr1);
	MojTestErrCheck(err);
	err = db.putKind(obj);
	MojTestErrCheck(err);

	// put test objects
    for (MojSize i = 0; i < sizeof(MojCollatorTestObjects1) / sizeof(MojChar*); ++i) {
		MojObject obj;
        err = obj.fromJson(MojCollatorTestObjects1[i]);
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
	MojInt64 index = 0;
	for (;;) {
		MojObject obj;
		err = cursor.get(obj, found);
		MojTestErrCheck(err);
		if (!found)
			break;
		MojInt64 id = 0;
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
		MojInt64 id = 0;
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

MojErr MojDbTextCollatorTest::queryTest2()
{
    MojDb db;
    MojErr err = db.open(MojDbTestDir);
    MojTestErrCheck(err);

    // put type
    MojObject obj;
    err = obj.fromJson(MojCollatorKindStr2);
    MojTestErrCheck(err);
    err = db.putKind(obj);
    MojTestErrCheck(err);

    // put test objects
    for (MojSize i = 0; i < sizeof(MojCollatorTestObjects2) / sizeof(MojChar*); ++i) {
        MojObject obj;
        err = obj.fromJson(MojCollatorTestObjects2[i]);
        MojTestErrCheck(err);
        err = db.put(obj, MojDb::FlagForce);
        MojTestErrCheck(err);
    }

    // verify find result order
    MojDbQuery query;
    err = query.from(_T("CollatorTest:2"));
    MojTestErrCheck(err);
    err = query.order(_T("filename"));
    MojTestErrCheck(err);
    MojDbCursor cursor;
    err = db.find(query, cursor);
    MojTestErrCheck(err);
    bool found = false;
    MojInt64 index = 1;
    for (;;) {
        MojObject obj;
        err = cursor.get(obj, found);
        MojTestErrCheck(err);
        if (!found)
            break;
        MojInt64 id = 0;
        err = obj.getRequired(MojDb::IdKey, id);
        MojTestErrCheck(err);
        MojTestAssert(id == index++);
    }
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
