/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

/****************************************************************
 *  @file TextCollatorTest.cpp
 *  Verify TextCollator. Based on MojDbTextCollatorTest.cpp
 ****************************************************************/

#include "db/MojDbTextCollator.h"

#include "MojDbCoreTest.h"

namespace {
    const MojChar* const MojCollatorKindStr =
            _T("{\"id\":\"CollatorTest:1\",")
            _T("\"owner\":\"mojodb.admin\",")
            _T("\"indexes\":[")
                    _T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"primary\"}]},")
            _T("]}");
    const MojChar* const MojCollatorTestObjects[] = {
            _T("{\"_id\":1,\"_kind\":\"CollatorTest:1\",\"foo\":\"APPLE\"}"),
            _T("{\"_id\":0,\"_kind\":\"CollatorTest:1\",\"foo\":\"aardvark\"}"),
            _T("{\"_id\":3,\"_kind\":\"CollatorTest:1\",\"foo\":\"BOY\"}"),
            _T("{\"_id\":2,\"_kind\":\"CollatorTest:1\",\"foo\":\"ball\"}"),
    };
}

struct TextCollatorTest : public MojDbCoreTest
{
    void SetUp()
    {
        MojDbCoreTest::SetUp();
    }
};

TEST_F(TextCollatorTest, simple)
{
    MojDbTextCollator colEn1;
    MojAssertNoErr( colEn1.init(_T("en_US"), MojDbCollationPrimary) );
    MojDbTextCollator colEn2;
    MojAssertNoErr( colEn2.init(_T("en_US"), MojDbCollationSecondary) );
    MojDbTextCollator colEn3;
    MojAssertNoErr( colEn3.init(_T("en_US"), MojDbCollationTertiary) );

    MojString str0;
    MojString str1;
    MojAssertNoErr( str1.assign(_T("cote")) );
    MojString str2;
    MojAssertNoErr( str2.assign(_T("CotE")) );
    MojString str3;
    MojAssertNoErr( str3.assign(_T("coté")) );
    MojString str4;
    MojAssertNoErr( str4.assign(_T("côte")) );
    MojString str5;
    MojAssertNoErr( str5.assign(_T("côTe")) );
    MojString str6;
    MojAssertNoErr( str6.assign(_T("carap")) );
    MojString str7;
    MojAssertNoErr( str7.assign(_T("f")) );
    MojString str8;
    MojAssertNoErr( str8.assign(_T("four")) );

    // primary strength
    MojDbKey key1;
    MojExpectNoErr( colEn1.sortKey(str0, key1) );
    MojDbKey key2;
    MojExpectNoErr( colEn1.sortKey(str0, key2) );
    EXPECT_EQ( key1, key2 );
    MojExpectNoErr( colEn1.sortKey(str1, key1) );
    MojExpectNoErr( colEn1.sortKey(str1, key2) );
    EXPECT_EQ( key1, key2 );
    MojExpectNoErr( colEn1.sortKey(str2, key2) );
    EXPECT_EQ( key1, key2 );
    MojDbKey key3;
    MojExpectNoErr( colEn1.sortKey(str3, key3) );
    EXPECT_EQ( key1, key3 );
    MojDbKey key4;
    MojExpectNoErr( colEn1.sortKey(str4, key4) );
    EXPECT_EQ( key1, key4 );
    MojDbKey key5;
    MojExpectNoErr( colEn1.sortKey(str5, key5) );
    EXPECT_EQ( key1, key5 );
    MojDbKey key6;
    MojExpectNoErr( colEn1.sortKey(str6, key6) );
    EXPECT_NE( key1, key6 );

    // secondary strength
    MojExpectNoErr( colEn2.sortKey(str1, key1) );
    MojExpectNoErr( colEn2.sortKey(str1, key2) );
    EXPECT_EQ( key1, key2 );
    MojExpectNoErr( colEn2.sortKey(str2, key2) );
    EXPECT_EQ( key1, key2 );
    MojExpectNoErr( colEn2.sortKey(str3, key3) );
    EXPECT_NE( key1, key3 );
    MojExpectNoErr( colEn2.sortKey(str4, key4) );
    EXPECT_NE( key1, key4 );
    EXPECT_NE( key3, key4 );
    MojExpectNoErr( colEn2.sortKey(str5, key5) );
    EXPECT_NE( key1, key5 );
    EXPECT_EQ( key4, key5 );
    MojExpectNoErr( colEn2.sortKey(str6, key6) );
    EXPECT_NE( key1, key6 );

    // tertiary strength
    MojExpectNoErr( colEn3.sortKey(str1, key1) );
    MojExpectNoErr( colEn3.sortKey(str1, key2) );
    EXPECT_EQ( key1, key2 );
    MojExpectNoErr( colEn3.sortKey(str2, key2) );
    EXPECT_NE( key1, key2 );
    MojExpectNoErr( colEn3.sortKey(str3, key3) );
    EXPECT_NE( key1, key3 );
    MojExpectNoErr( colEn3.sortKey(str4, key4) );
    EXPECT_NE( key1, key4 );
    EXPECT_NE( key3, key4 );
    MojExpectNoErr( colEn3.sortKey(str5, key5) );
    EXPECT_NE( key1, key5 );
    EXPECT_NE( key4, key5 );
    MojExpectNoErr( colEn3.sortKey(str6, key6) );
    EXPECT_NE( key1, key6 );

    // prefix
    MojExpectNoErr( colEn1.sortKey(str7, key1) );
    MojExpectNoErr( colEn1.sortKey(str8, key2) );
    MojAssertNoErr( key1.byteVec().pop() );
    EXPECT_TRUE( key1.prefixOf(key2) )
        << "key1 should be prefix of key2";
}

TEST_F(TextCollatorTest, query)
{
    // put type
    MojObject obj;
    MojAssertNoErr( obj.fromJson(MojCollatorKindStr) );
    MojAssertNoErr( db.putKind(obj) );

    // put test objects
    for (MojSize i = 0; i < sizeof(MojCollatorTestObjects) / sizeof(MojChar*); ++i) {
            MojObject obj;
            MojAssertNoErr( obj.fromJson(MojCollatorTestObjects[i]) );
            MojExpectNoErr( db.put(obj) )
                << "Should be able to put object #" << i;
    }

    // verify find result order
    MojDbQuery query;
    MojAssertNoErr( query.from(_T("CollatorTest:1")) );
    MojAssertNoErr( query.order(_T("foo")) );
    MojDbCursor cursor;
    MojExpectNoErr( db.find(query, cursor) );
    bool found = false;
    MojInt64 index = 0;
    for (;;) {
            MojObject obj;
            MojAssertNoErr( cursor.get(obj, found) );
            if (!found)
                    break;
            MojInt64 id = 0;
            MojExpectNoErr( obj.getRequired(MojDb::IdKey, id) )
                << "Object #" << index << " should have Id key";
            EXPECT_EQ( index++, id );
    }
    MojExpectNoErr( cursor.close() );

    // verify prefix match
    MojString val;
    MojAssertNoErr( val.assign(_T("a")) );
    MojAssertNoErr( query.where(_T("foo"), MojDbQuery::OpPrefix, val, MojDbCollationPrimary) );
    MojExpectNoErr( db.find(query, cursor) );
    index = 0;
    for (;;) {
            MojObject obj;
            MojAssertNoErr( cursor.get(obj, found) );
            if (!found)
                    break;
            MojInt64 id = 0;
            MojExpectNoErr( obj.getRequired(MojDb::IdKey, id) )
                << "Object #" << index << " should have Id key";
            EXPECT_EQ( index++, id );
    }
    EXPECT_EQ( 2, index );
    MojExpectNoErr( cursor.close() );
}
