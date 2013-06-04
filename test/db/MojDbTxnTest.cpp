/* @@@LICENSE
*
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

#include "MojDbTxnTest.h"
#include "db/MojDb.h"

namespace {

const char* const MojKindStr =
    _T("{\"id\":\"Test:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}");
}


MojDbTxnTest::MojDbTxnTest()
: MojTestCase(_T("MojDbTxn"))
{
}

MojErr MojDbTxnTest::run()
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
        err = obj.put(_T("foo"), (i + 25) % 100);
        MojTestErrCheck(err);
        err = obj.put(_T("bar"), i % 3);
        MojTestErrCheck(err);
        err = db.put(obj);
        MojTestErrCheck(err);
    }

    // db: x0 = (25, 0), (26, 1), (27, 2), (28, 0) .. x74 = (99,2), x75 = (0,0) .. x99 = (24,0)

    {
        MojDbQuery query;
        err = query.from(_T("Test:1"));
        MojTestErrCheck(err);
        err = query.where(_T("foo"), MojDbQuery::OpLessThan, 50);
        MojTestErrCheck(err);

        MojObject update;
        err = update.put(_T("bar"), -1);
        MojTestErrCheck(err);
        MojUInt32 count = 0;
        err = db.merge(query, update, count);
        MojTestErrCheck(err);
        MojTestAssert(count == 50);
    }

    // db: x0 = (25, -1) .. x24 = (49,-1), x25 = (50,1)i .. x74 = (99,2), x75 = (0,-1) .. x99 = (24, -1)

    // test visibility with update
    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        MojDbQuery query;
        err = query.from(_T("Test:1"));
        MojTestErrCheck(err);
        err = query.where(_T("bar"), MojDbQuery::OpEq, -1);
        MojTestErrCheck(err);

        MojObject update;
        err = update.put(_T("bar"), -2);
        MojTestErrCheck(err);

        MojUInt32 count = 0;
        err = db.merge(query, update, count, MojDb::FlagNone, req);
        MojTestErrCheck(err);
        MojTestAssert(count == 50);

        // txn: x0 = (25, -2) .. x24 = (49,-2), x25 = (50,1) .. x74 = (99,2), x75 = (0,-2) .. x99 = (24, -2)

        // visible within transaction
        {
            MojDbQuery query;
            err = query.from(_T("Test:1"));
            MojTestErrCheck(err);
            err = query.where(_T("bar"), MojDbQuery::OpEq, -2);
            MojTestErrCheck(err);

            MojObject update;
            err = update.put(_T("bar"), -2);
            MojTestErrCheck(err);

            MojUInt32 count = 0;
            err = db.merge(query, update, count, MojDb::FlagNone, req);
            MojTestErrCheck(err);
            MojTestAssert(count == 50);
        }

#ifdef MOJ_USE_LDB
        // With BerkeleyDB parallel transaction is locked

        // invisible outside of transaction
        {
            MojDbQuery query;
            err = query.from(_T("Test:1"));
            MojTestErrCheck(err);
            err = query.where(_T("bar"), MojDbQuery::OpEq, -2);
            MojTestErrCheck(err);

            MojObject update;
            err = update.put(_T("bar"), -2);
            MojTestErrCheck(err);

            MojUInt32 count = 0;
            err = db.merge(query, update, count);
            MojTestErrCheck(err);
            MojTestAssert(count == 0);
        }
#endif
    }

    // invisible after aborted transaction
    {
        MojDbQuery query;
        err = query.from(_T("Test:1"));
        MojTestErrCheck(err);
        err = query.where(_T("bar"), MojDbQuery::OpEq, -2);
        MojTestErrCheck(err);

        MojObject update;
        err = update.put(_T("bar"), -2);
        MojTestErrCheck(err);

        MojUInt32 count = 0;
        err = db.merge(query, update, count);
        MojTestErrCheck(err);
        MojTestAssert(count == 0);
    }

    // test visibility with delete
    {
        MojDbReq req;
        // start transaction
        req.begin(&db, false);

        MojDbQuery query;
        err = query.from(_T("Test:1"));
        MojTestErrCheck(err);
        err = query.where(_T("bar"), MojDbQuery::OpEq, -1);
        MojTestErrCheck(err);

        MojUInt32 count = 0;
        err = db.del(query, count, MojDb::FlagNone, req);
        MojTestErrCheck(err);
        MojTestAssert(count == 50);

        // txn: x25 = (50,1) .. x74 = (99,2)

        // visible within transaction
        {
            MojDbQuery query;
            err = query.from(_T("Test:1"));
            MojTestErrCheck(err);
            err = query.where(_T("bar"), MojDbQuery::OpLessThan, 2);
            MojTestErrCheck(err);

            MojObject update;
            err = update.put(_T("bar"), -3);
            MojTestErrCheck(err);

            MojUInt32 count = 0;
            err = db.merge(query, update, count, MojDb::FlagNone, req);
            MojTestErrCheck(err);
            MojTestAssert(count == 33);
        }

#ifdef MOJ_USE_LDB
        // With BerkeleyDB parallel transaction is locked

        // invisible outside of transaction
        {
            MojDbQuery query;
            err = query.from(_T("Test:1"));
            MojTestErrCheck(err);
            err = query.where(_T("bar"), MojDbQuery::OpLessThan, 2);
            MojTestErrCheck(err);

            MojObject update;
            err = update.put(_T("bar"), -3);
            MojTestErrCheck(err);

            MojUInt32 count = 0;
            err = db.merge(query, update, count);
            MojTestErrCheck(err);
            MojTestAssert(count == 83);
        }
#endif
    }

    // invisible after aborted transaction
    {
        MojDbQuery query;
        err = query.from(_T("Test:1"));
        MojTestErrCheck(err);
        err = query.where(_T("bar"), MojDbQuery::OpLessThan, 2);
        MojTestErrCheck(err);

        MojObject update;
        // Note that if we change bar=1 here we might get double-update when
        // record we just updated moved into records range ahead of our current
        // cursor position

        MojUInt32 count = 0;
        err = db.merge(query, update, count);
        MojTestErrCheck(err);
        MojTestAssert(count == 83);
    }

    err = db.close();
    MojTestErrCheck(err);

    return MojErrNone;
}

void MojDbTxnTest::cleanup()
{
    MojRmDirRecursive(MojDbTestDir);
}
