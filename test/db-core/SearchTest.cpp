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
 *  @file SearchTest.cpp
 *  Verify OpSearch. Based on MojDbSearchTest.cpp
 ****************************************************************/

#include "core/MojObjectBuilder.h"
#include "db/MojDbSearchCursor.h"

#include "MojDbCoreTest.h"

namespace {
    const MojChar* const MojSearchKindStr =
            _T("{\"id\":\"SearchTest:1\",")
            _T("\"owner\":\"mojodb.admin\",")
            _T("\"indexes\":[")
                    _T("{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"tokenize\":\"all\",\"collate\":\"primary\"}]},")
                    _T("{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\",\"tokenize\":\"all\",\"collate\":\"primary\"}]},")
                    _T("{\"name\":\"multiprop\",\"props\":[{\"name\":\"multiprop\",\"type\":\"multi\",\"collate\":\"primary\",\"include\":[{\"name\":\"hello\",\"tokenize\":\"all\"},{\"name\":\"world\",\"tokenize\":\"all\"}]}]}")
            _T("]}");
    const MojChar* const MojSearchTestObjects[] = {
            _T("{\"_id\":1,\"_kind\":\"SearchTest:1\",\"bar\":4,\"foo\":\"four score & seven years\"}"),
            _T("{\"_id\":2,\"_kind\":\"SearchTest:1\",\"bar\":3,\"foo\":\"our fathers brought forth\"}"),
            _T("{\"_id\":3,\"_kind\":\"SearchTest:1\",\"bar\":2,\"foo\":\"four fathers forth\"}"),
            _T("{\"_id\":4,\"_kind\":\"SearchTest:1\",\"bar\":1,\"foo\":\"four\"}"),
            _T("{\"_id\":5,\"_kind\":\"SearchTest:1\",\"foo\":\"cote\"}"),
            _T("{\"_id\":6,\"_kind\":\"SearchTest:1\",\"foo\":\"CotE\"}"),
            _T("{\"_id\":7,\"_kind\":\"SearchTest:1\",\"foo\":\"coté\"}"),
            _T("{\"_id\":8,\"_kind\":\"SearchTest:1\",\"foo\":\"côte\"}"),
            _T("{\"_id\":9,\"_kind\":\"SearchTest:1\",\"foo\":\"côTe\"}"),
            _T("{\"_id\":10,\"_kind\":\"SearchTest:1\",\"foo\":\"carap\"}"),
            _T("{\"_id\":11,\"_kind\":\"SearchTest:1\",\"foo\":\"APPLE\"}"),
            _T("{\"_id\":12,\"_kind\":\"SearchTest:1\",\"foo\":\"aardvark\"}"),
            _T("{\"_id\":13,\"_kind\":\"SearchTest:1\",\"foo\":\"AbAcus\"}"),
            _T("{\"_id\":14,\"_kind\":\"SearchTest:1\",\"hello\":\"this is a test\"}"),
            _T("{\"_id\":15,\"_kind\":\"SearchTest:1\",\"world\":\"this is not a test\"}")
    };
}

struct SearchTest : public MojDbCoreTest
{
    void SetUp()
    {
        MojDbCoreTest::SetUp();

        // add kind
        MojObject kindObj;
        MojAssertNoErr( kindObj.fromJson(MojSearchKindStr) );
        MojAssertNoErr( db.putKind(kindObj) );

        // put test objects
        for (MojSize i = 0; i < sizeof(MojSearchTestObjects) / sizeof(MojChar*); ++i) {
            MojObject obj;
            MojAssertNoErr( obj.fromJson(MojSearchTestObjects[i]) );
            MojAssertNoErr( db.put(obj) )
                << "Object #" << i << " should be put into db";
        }
    }

    void initQuery(MojDbQuery& query, const MojChar* queryStr, const MojChar* orderBy = NULL,
                   const MojObject& barVal = MojObject::Undefined, bool desc = false)
    {
        query.clear();
        MojAssertNoErr( query.from(_T("SearchTest:1")) );
        MojString val;
        MojAssertNoErr( val.assign(queryStr) );
        MojAssertNoErr( query.where(_T("foo"), MojDbQuery::OpSearch, val, MojDbCollationPrimary) );
        query.desc(desc);
        if (!barVal.undefined()) {
                MojAssertNoErr( query.where(_T("bar"), MojDbQuery::OpEq, barVal) );
        }
        if (orderBy) {
                MojAssertNoErr( query.order(orderBy) );
        }
    }

    void check(MojDb& db, const MojChar* queryStr, const MojChar* expectedIdsJson,
               const MojChar* orderBy = NULL, const MojObject& barVal = MojObject::Undefined, bool desc = false)
    {
        MojDbQuery query;
        ASSERT_NO_FATAL_FAILURE( initQuery(query, queryStr, orderBy, barVal, desc) );
        ASSERT_NO_FATAL_FAILURE( check(db, query, expectedIdsJson) );
    }

    void check(MojDb& db, const MojDbQuery& query, const MojChar* expectedIdsJson)
    {
        MojString str;
        MojDbSearchCursor cursor(str);
        MojAssertNoErr( db.find(query, cursor) );

        MojObjectBuilder builder;
        MojAssertNoErr( builder.beginArray() );
        MojAssertNoErr( cursor.visit(builder) );
        MojAssertNoErr( cursor.close() );
        MojAssertNoErr( builder.endArray() );
        MojObject results = builder.object();

        MojString json;
        MojAssertNoErr( results.toJson(json) );

        MojObject expected;
        MojAssertNoErr( expected.fromJson(expectedIdsJson) );

        ASSERT_EQ( expected.size(), results.size() )
            << "Amount of expected ids should match amount of ids in result";
        MojObject::ConstArrayIterator j = results.arrayBegin();
        size_t n = 0;
        for (MojObject::ConstArrayIterator i = expected.arrayBegin();
             j != results.arrayEnd() && i != expected.arrayEnd();
             ++i, ++j, ++n)
        {
            MojObject id;
            MojAssertNoErr( j->getRequired(MojDb::IdKey, id) )
                << "MojDb::IdKey(\"" << MojDb::IdKey << "\") should present in item #" << n;
            ASSERT_EQ( *i, id );
        }
    }
};

TEST_F(SearchTest, simple)
{
    // just foo
    EXPECT_NO_FATAL_FAILURE( check(db, _T("bogus"), _T("[]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[1,2,3,4]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("F"), _T("[1,2,3,4]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("fo"), _T("[1,2,3,4]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("four"), _T("[1,3,4]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("score"), _T("[1]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("four years"), _T("[1]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("Four Years"), _T("[1]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("four decades"), _T("[]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("fathers forth"), _T("[2,3]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("four f"), _T("[1,3,4]")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("four f fo fou"), _T("[1,3,4]")) );

    // bar and foo
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[3]"), NULL, 2) );

    // order by bar
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[4,3,2,1]"), _T("bar")) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[1,2,3,4]"), _T("bar"), MojObject::Undefined, true) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[4,3,1,2]"), _T("foo")) );

    // array value for bar
    MojObject array;
    MojAssertNoErr( array.push(1) );
    MojAssertNoErr( array.push(2) );
    EXPECT_NO_FATAL_FAILURE( check(db, _T("f"), _T("[4,3]"), _T("bar"), array) );

    // limit
    MojDbQuery query;
    EXPECT_NO_FATAL_FAILURE( initQuery(query, _T("f")) );
    query.limit(2);
    EXPECT_NO_FATAL_FAILURE( check(db, query, _T("[1,2]")) );
    EXPECT_NO_FATAL_FAILURE( initQuery(query, _T("f"), _T("bar")) );
    query.limit(2);
    EXPECT_NO_FATAL_FAILURE( check(db, query, _T("[4,3]")) );

    // accent insensitivity
    EXPECT_NO_FATAL_FAILURE( check(db, _T("COTE"), _T("[5,6,7,8,9]")) );

    // case-insensitive ordering
    EXPECT_NO_FATAL_FAILURE( check(db, _T("a"), _T("[12,13,11]"), _T("foo")) );

    // multi-prop
    query.clear();
    MojAssertNoErr( query.from(_T("SearchTest:1")) );
    MojString val;
    MojAssertNoErr( val.assign(_T("test")) );
    MojAssertNoErr( query.where(_T("multiprop"), MojDbQuery::OpSearch, val, MojDbCollationPrimary) );
    EXPECT_NO_FATAL_FAILURE( check(db, query, _T("[14,15]")) );
}

TEST_F(SearchTest, filter)
{
    MojDbQuery query;
    EXPECT_NO_FATAL_FAILURE( initQuery(query, _T("f")) );
    MojAssertNoErr( query.filter(_T("bar"), MojDbQuery::OpEq, 1) );
    EXPECT_NO_FATAL_FAILURE( check(db, query, _T("[4]")) );

    EXPECT_NO_FATAL_FAILURE( initQuery(query, _T("f")) );
    MojAssertNoErr( query.filter(_T("bar"), MojDbQuery::OpGreaterThan, 1) );
    EXPECT_NO_FATAL_FAILURE( check(db, query, _T("[1,2,3]")) );
}
