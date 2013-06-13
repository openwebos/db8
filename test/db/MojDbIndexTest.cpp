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


#include "MojDbIndexTest.h"
#include "db/MojDb.h"
#include "db/MojDbIndex.h"
#include "db/MojDbQuery.h"
#include "core/MojObjectSerialization.h"

class TestIndex : public MojDbStorageIndex
{
public:
	TestIndex(bool incDel) : m_incDel(incDel), m_putCount(0), m_delCount(0) {}

	virtual MojErr close()
	{
		return MojErrNone;
	}

	virtual MojErr drop(MojDbStorageTxn* txn)
	{
		return MojErrNone;
	}

	virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
	{
		return MojErrNone;
	}

	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn)
	{
		MojErr err = m_set.put(key);
		MojErrCheck(err);
		++m_putCount;
		return MojErrNone;
	}

	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn)
	{
		bool found = false;
		MojErr err = m_set.del(key, found);
		MojErrCheck(err);
		if (!found)
			MojErrThrow(MojErrNotFound);
		++m_delCount;
		return MojErrNone;
	}

	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
	{
		return MojErrNone;
	}

	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
	{
		return MojErrNone;
	}

	typedef MojSet<MojDbKey> IndexSet;
	IndexSet m_set;
	bool m_incDel;
	MojSize m_putCount;
	MojSize m_delCount;
};

class TestTxn : public MojDbStorageTxn
{
public:
	virtual MojErr commitImpl()
	{
		return MojErrNone;
	}

	virtual MojErr abort()
	{
		return MojErrNone;
	}

    virtual bool isValid()
    {
        return true;
    }
};

MojDbIndexTest::MojDbIndexTest()
: MojTestCase("MojDbIndex")
{
}

MojErr MojDbIndexTest::run()
{
	MojErr err = invalidTest();
	MojTestErrCheck(err);
	err = simpleTest();
	MojTestErrCheck(err);
	err = nestedTest();
	MojTestErrCheck(err);
	err = compoundTest();
	MojTestErrCheck(err);
	err = canAnswerTest();
	MojTestErrCheck(err);
	err = deletedTest();
	MojTestErrCheck(err);
	err = wildcardTest();
	MojTestErrCheck(err);
	err = defaultValuesTest();
	MojTestErrCheck(err);
	err = multiTest();
	MojTestErrCheck(err);
	err = tokenizeTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::invalidTest()
{
	MojErr err = checkInvalid(MojErrDbInvalidIndex,
			_T("{\"name\":\"test\",\"props\":[")
				_T("{\"name\":\"foo\"},")
				_T("{\"name\":\"bar\"},")
				_T("{\"name\":\"foo\"}")
			_T("]}"));
	MojTestErrCheck(err);
	err = checkInvalid(MojErrDbInvalidIndex,
			_T("{\"name\":\"test\",\"incDel\":\"true\",\"props\":[")
				_T("{\"name\":\"foo\"},")
				_T("{\"name\":\"_del\"},")
				_T("{\"name\":\"foo\"}")
			_T("]}"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::simpleTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 1 && ti.m_delCount == 0 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = put(index, 1, _T("{\"foo\":2}"), _T("{\"foo\":1}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 1 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, 2);
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":2}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 2 && ti.m_set.size() == 0);
	err = put(index, 2, _T("{\"foo\":[3,4,5]}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 5 && ti.m_delCount == 2 && ti.m_set.size() == 3);
	err = assertContains(ti, 2, 3);
	MojTestErrCheck(err);
	err = assertContains(ti, 2, 4);
	MojTestErrCheck(err);
	err = assertContains(ti, 2, 5);
	MojTestErrCheck(err);
	err = put(index, 2, _T("{\"foo\":[5,6]}"), _T("{\"foo\":[3,4,4,4,3,3,4,3,5]}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 6 && ti.m_delCount == 4 && ti.m_set.size() == 2);
	err = assertContains(ti, 2, 5);
	MojTestErrCheck(err);
	err = assertContains(ti, 2, 6);
	MojTestErrCheck(err);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::nestedTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo.bar.baz"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo.bar.baz\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":{\"bar\":{\"baz\":1}}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 1 && ti.m_delCount == 0 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = put(index, 1, _T("{\"foo\":{\"bar\":{\"baz\":[2,3,4]}}}"), _T("{\"foo\":{\"bar\":{\"baz\":1}}}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 4 && ti.m_delCount == 1 && ti.m_set.size() == 3);
	err = assertContains(ti, 1, 2);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 3);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 4);
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":{\"bar\":{\"baz\":[2,3,4]}}}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 4 && ti.m_delCount == 4 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":{\"bar\":[{\"baz\":5},{\"baz\":6},{\"baz\":7},{\"baz\":8}]}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 8 && ti.m_delCount == 4 && ti.m_set.size() == 4);
	err = assertContains(ti, 1, 5);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 6);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 7);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 8);
	MojTestErrCheck(err);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::compoundTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	err = prop.putString(MojDbIndex::NameKey, _T("bar"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	err = prop.putString(MojDbIndex::NameKey, _T("baz"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":2}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":1,\"baz\":2}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":2,\"baz\":3}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 1 && ti.m_delCount == 0 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, _T("[1,2,3]"));
	MojTestErrCheck(err);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":-2,\"baz\":3}"), _T("{\"foo\":1,\"bar\":2,\"baz\":3}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 1 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, _T("[1,-2,3]"));
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":1,\"bar\":-2,\"baz\":3}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 2 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":[2,-2],\"baz\":3}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 4 && ti.m_delCount == 2 && ti.m_set.size() == 2);
	err = assertContains(ti, 1, _T("[1,2,3]"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, _T("[1,-2,3]"));
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":1,\"bar\":[2,-2],\"baz\":3}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 4 && ti.m_delCount == 4 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":[2,2,2],\"baz\":3}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 5 && ti.m_delCount == 4 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, _T("[1,2,3]"));
	MojTestErrCheck(err);
	err = put(index, 1, _T("{\"foo\":1,\"bar\":[2,8],\"baz\":[3,4]}"), _T("{\"foo\":1,\"bar\":[2,2,2],\"baz\":3}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 8 && ti.m_delCount == 4 && ti.m_set.size() == 4);
	err = assertContains(ti, 1, _T("[1,2,3]"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, _T("[1,2,4]"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, _T("[1,8,3]"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, _T("[1,8,4]"));
	MojTestErrCheck(err);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::canAnswerTest()
{
    // < - only prop
    MojErr err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // <,> - only prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5},{\"prop\":\"foo\",\"op\":\">\",\"val\":0}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,= - only prop, _id
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"_id\",\"op\":\"=\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // < - unindexed prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"bar\",\"op\":\"<\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // =,= - unindexed prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // =,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,>
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\">\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,>=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\">=\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,<
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"<\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,<=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"<=\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,!=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"!=\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,%
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"%\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // >,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\">\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // >=,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\">=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // <,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // <=,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // !=,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"!=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // %,=
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"%\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // =,>,none
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\">\",\"val\":5}]}"),
            true
            );
    MojTestErrCheck(err);
    // =,>,none - order by > prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\">\",\"val\":5}],\"orderBy\":\"bar\"}"),
            true
            );
    MojTestErrCheck(err);
    // =,=,none - order by unreferenced prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}],\"orderBy\":\"foobar\"}"),
            true
            );
    MojTestErrCheck(err);
    // =,=,none - order by first = prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}],\"orderBy\":\"foo\"}"),
            true
            );
    MojTestErrCheck(err);
    // =,>,none - order by = prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\">\",\"val\":5}],\"orderBy\":\"foo\"}"),
            true
            );
    MojTestErrCheck(err);
    // =[],>,none - order by =[] prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":[5,6]},{\"prop\":\"bar\",\"op\":\">\",\"val\":5}],\"orderBy\":\"foo\"}"),
            true
            );
    MojTestErrCheck(err);
    // =,none,none - order by last prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5}],\"orderBy\":\"foobar\"}"),
            false
            );
    MojTestErrCheck(err);
    // =,=,none - order by last prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}],\"orderBy\":\"foobar\"}"),
            true
            );
    MojTestErrCheck(err);
    // =,=,none - order by unindexed prop
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"foobar\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5},{\"prop\":\"bar\",\"op\":\"=\",\"val\":5}],\"orderBy\":\"carap\"}"),
            false
            );
    MojTestErrCheck(err);
    // incDel query on incDel index
    err = assertCanAnswer(
        _T("[{\"name\":\"foo\"}]"),
        _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5},{\"prop\":\"_del\",\"op\":\"=\",\"val\":true}]}"),
        true, true
        );
    MojTestErrCheck(err);
    // non-incDel query on incDel index
    err = assertCanAnswer(
        _T("[{\"name\":\"foo\"}]"),
        _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5}]}"),
        true, true
        );
    MojTestErrCheck(err);
    // incDel query on non-incDel index
    err = assertCanAnswer(
        _T("[{\"name\":\"foo\"}]"),
        _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"<\",\"val\":5},{\"prop\":\"_del\",\"op\":\"=\",\"val\":true}]}"),
        false, false
        );
    MojTestErrCheck(err);
    // matched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\",\"collate\":\"primary\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5,\"collate\":\"primary\"}]}"),
            true
            );
    MojTestErrCheck(err);
    // matched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\",\"collate\":\"secondary\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5,\"collate\":\"secondary\"}]}"),
            true
            );
    MojTestErrCheck(err);
    // matched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\",\"collate\":\"tertiary\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5,\"collate\":\"tertiary\"}]}"),
            true
            );
    MojTestErrCheck(err);
    // mismatched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\",\"collate\":\"primary\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5}]}"),
            false
            );
    MojTestErrCheck(err);
    // mismatched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5,\"collate\":\"primary\"}]}"),
            false
            );
    MojTestErrCheck(err);
    // mismatched collate
    err = assertCanAnswer(
            _T("[{\"name\":\"foo\",\"collate\":\"secondary\"}]"),
            _T("{\"from\":\"Test:1\",\"where\":[{\"prop\":\"foo\",\"op\":\"=\",\"val\":5,\"collate\":\"tertiary\"}]}"),
            false
            );
    MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::deletedTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(true));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	index.incDel(true);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 1 && ti.m_delCount == 0 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":1}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 1 && ti.m_set.size() == 1);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":1,\"_del\":true}"), true);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 2 && ti.m_set.size() == 0);

	return MojErrNone;
}

MojErr MojDbIndexTest::wildcardTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo.*.bar"));
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":{\"bar\":1}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 0 && ti.m_delCount == 0 && ti.m_set.size() == 0);
	err = put(index, 1, _T("{\"foo\":{\"1\":{\"bar\":1},\"2\":{\"bar\":2},\"3\":{\"bar\":3}}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 3 && ti.m_delCount == 0 && ti.m_set.size() == 3);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 2);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 3);
	MojTestErrCheck(err);
	err = put(index, 1, _T("{\"foo\":{\"2\":{\"bar\":2},\"3\":{\"bar\":3}}}"), _T("{\"foo\":{\"1\":{\"bar\":1},\"2\":{\"bar\":2},\"3\":{\"bar\":3}}}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 3 && ti.m_delCount == 1 && ti.m_set.size() == 2);
	err = assertContains(ti, 1, 2);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 3);
	MojTestErrCheck(err);
	err = del(index, 1, _T("{\"foo\":{\"2\":{\"bar\":2},\"3\":{\"bar\":3}}}"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 3 && ti.m_delCount == 3 && ti.m_set.size() == 0);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::defaultValuesTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojObject prop;
	MojErr err = prop.putString(MojDbIndex::NameKey, _T("foo"));
	MojTestErrCheck(err);
	err = prop.putInt(MojDbIndex::DefaultKey, 100);
	MojTestErrCheck(err);
	err = index.addProp(prop);
	MojTestErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"bar\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 1 && ti.m_delCount == 0 && ti.m_set.size() == 1);
	err = put(index, 1, _T("{\"foo\":5}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 2 && ti.m_delCount == 0 && ti.m_set.size() == 2);
	err = put(index, 1, _T("{\"foo\":{\"bar\":3}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 3 && ti.m_delCount == 0 && ti.m_set.size() == 3);

	err = index.close();
	MojTestErrCheck(err);

	MojDbIndex index2(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex2(new TestIndex(false));
	MojAllocCheck(storageIndex2.get());
	TestIndex& ti2 = *storageIndex2;

	MojObject prop2;
	err = prop2.putString(MojDbIndex::NameKey, _T("bar"));
	MojTestErrCheck(err);
	err = index2.addProp(prop2);
	MojTestErrCheck(err);
	err = index2.open(storageIndex2.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index2, 1, _T("{\"bar\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti2.m_putCount == 1 && ti2.m_delCount == 0 && ti2.m_set.size() == 1);
	err = put(index2, 1, _T("{\"foo\":5}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti2.m_putCount == 1 && ti2.m_delCount == 0 && ti2.m_set.size() == 1);
	err = put(index2, 1, _T("{\"foo\":{\"bar\":3}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti2.m_putCount == 1 && ti2.m_delCount == 0 && ti2.m_set.size() == 1);

	err = index2.close();
	MojTestErrCheck(err);

	MojDbIndex index3(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex3(new TestIndex(false));
	MojAllocCheck(storageIndex3.get());
	TestIndex& ti3 = *storageIndex3;

	MojObject prop3;
	err = prop3.putString(MojDbIndex::NameKey, _T("bar"));
	MojTestErrCheck(err);
	err = index3.addProp(prop3);
	MojTestErrCheck(err);
	err = prop3.putString(MojDbIndex::NameKey, _T("foo"));
	MojTestErrCheck(err);
	err = prop3.putInt(MojDbIndex::DefaultKey, 100);
	MojTestErrCheck(err);
	err = index3.addProp(prop3);
	MojTestErrCheck(err);
	err = index3.open(storageIndex3.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index3, 1, _T("{\"bar\":1}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti3.m_putCount == 1 && ti3.m_delCount == 0 && ti3.m_set.size() == 1);
	err = put(index3, 1, _T("{\"foo\":5}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti3.m_putCount == 1 && ti3.m_delCount == 0 && ti3.m_set.size() == 1);
	err = put(index3, 1, _T("{\"foo\":{\"bar\":3}}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti3.m_putCount == 1 && ti3.m_delCount == 0 && ti3.m_set.size() == 1);
	err = put(index3, 1, _T("{\"foo\":5, \"bar\":5}"), NULL);
	MojTestErrCheck(err);
	MojTestAssert(ti3.m_putCount == 2 && ti3.m_delCount == 0 && ti3.m_set.size() == 2);

	err = index3.close();
	MojTestErrCheck(err);

	return MojErrNone;
}


MojErr MojDbIndexTest::multiTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojErr err = indexFromObject(index,
			_T("{\"name\":\"test\",\"props\":[")
				_T("{\"name\":\"multiprop\",\"type\":\"multi\",\"include\":[{\"name\":\"foo\"},{\"name\":\"bar\"},{\"name\":\"baz\"}]}")
			_T("]}"));
	MojErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":1,\"bar\":2,\"baz\":3}"), NULL);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 1);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 2);
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 3);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 3 && ti.m_delCount == 0 && ti.m_set.size() == 3);
	err = put(index, 1, _T("{\"foo\":5}"), _T("{\"foo\":1,\"bar\":2,\"baz\":3}"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 5);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 4 && ti.m_delCount == 3 && ti.m_set.size() == 1);
	err = put(index, 1, _T("{\"bar\":6}"), _T("{\"foo\":5}"));
	MojTestErrCheck(err);
	err = assertContains(ti, 1, 6);
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 5 && ti.m_delCount == 4 && ti.m_set.size() == 1);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::tokenizeTest()
{
	MojDbIndex index(NULL, NULL);
	MojRefCountedPtr<TestIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	TestIndex& ti = *storageIndex;

	MojErr err = indexFromObject(index,
			_T("{\"name\":\"test\",\"props\":[")
				_T("{\"name\":\"foo\",\"tokenize\":\"all\",\"collate\":\"primary\"}")
			_T("]}"));
	MojErrCheck(err);
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	err = put(index, 1, _T("{\"foo\":\"four score and seven years ago\"}"), NULL);
	MojTestErrCheck(err);
	err = assertContainsText(ti, 1, _T("four"));
	MojTestErrCheck(err);
	err = assertContainsText(ti, 1, _T("score"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 6 && ti.m_delCount == 0 && ti.m_set.size() == 6);
	err = put(index, 1, _T("{\"foo\":\"our fathers put\"}"), _T("{\"foo\":\"four score and seven years ago\"}"));
	MojTestErrCheck(err);
	err = assertContainsText(ti, 1, _T("fathers"));
	MojTestErrCheck(err);
	MojTestAssert(ti.m_putCount == 9 && ti.m_delCount == 6 && ti.m_set.size() == 3);

	err = index.close();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::checkInvalid(MojErr errExpected, const MojChar* json)
{
	MojDbIndex index(NULL, NULL);
	MojErr err = indexFromObject(index, json);
	MojTestErrExpected(err, errExpected);

	return MojErrNone;
}

MojErr MojDbIndexTest::indexFromObject(MojDbIndex& index, const MojChar* json)
{
	MojObject obj;
	MojErr err = obj.fromJson(json);
	MojErrCheck(err);
	err = index.fromObject(obj, MojString());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::put(MojDbIndex& index, MojObject id, const MojChar* jsonNew, const MojChar* jsonOld)
{
	MojObject newObj;
	MojErr err = newObj.fromJson(jsonNew);
	MojTestErrCheck(err);
	err = newObj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	MojObject oldObj;
	if (jsonOld) {
		err = oldObj.fromJson(jsonOld);
		MojTestErrCheck(err);
		err = oldObj.put(MojDb::IdKey, id);
		MojTestErrCheck(err);
	}
	TestTxn txn;
	err = index.update(&newObj, jsonOld ? &oldObj : NULL, &txn, false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::del(MojDbIndex& index, MojObject id, const MojChar* json, bool purge)
{
	MojObject oldObj;
	MojErr err = oldObj.fromJson(json);
	MojTestErrCheck(err);
	err = oldObj.put(MojDb::IdKey, id);
	MojTestErrCheck(err);
	MojObject newObj = oldObj;
	TestTxn txn;
	err = newObj.put(MojDb::DelKey, true);
	MojTestErrCheck(err);
	err = index.update(purge ? NULL: &newObj, &oldObj, &txn, false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::assertContains(TestIndex& ti, MojObject id, MojObject key)
{
	MojObjectWriter writer;
	MojErr err = key.visit(writer);
	MojTestErrCheck(err);
	err = id.visit(writer);
	MojTestErrCheck(err);
	MojDbKey compoundKey;
	err = compoundKey.assign(writer.buf());
	MojTestErrCheck(err);
	err = assertContains(ti, id, compoundKey);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::assertContains(TestIndex& ti, MojObject id, const MojChar* json)
{
	MojObject array;
	MojErr err = array.fromJson(json);
	MojTestErrCheck(err);
	MojObjectWriter(writer);
	MojObject val;
	MojSize idx = 0;
	while (array.at(idx++, val)) {
		err = val.visit(writer);
		MojTestErrCheck(err);
	}
	err = id.visit(writer);
	MojTestErrCheck(err);
	MojDbKey key;
	err = key.assign(writer.buf());
	MojTestErrCheck(err);
	err = assertContains(ti, id, key);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::assertContains(TestIndex& ti, MojObject id, const MojDbKey& key)
{
	MojDbKey prefixedKey(key);
	MojErr err = prefixedKey.byteVec().insert(0, 1, MojObjectWriter::MarkerZeroIntValue);
	MojErrCheck(err);
	if (ti.m_incDel) {
		MojDbKey::ByteVec vec = prefixedKey.byteVec();
		MojErr err = vec.insert(1, 1, MojObjectWriter::MarkerTrueValue);
		MojTestErrCheck(err);
		MojDbKey keyTrue;
		err = keyTrue.assign(vec.begin(), vec.size());
		MojTestErrCheck(err);

		err = vec.setAt(1, MojObjectWriter::MarkerFalseValue);
		MojTestErrCheck(err);
		MojDbKey keyFalse;
		err = keyFalse.assign(vec.begin(), vec.size());
		MojTestErrCheck(err);
		MojTestAssert(ti.m_set.contains(keyTrue) || ti.m_set.contains(keyFalse));
	} else {
		MojTestAssert(ti.m_set.contains(prefixedKey));
	}
	return MojErrNone;
}

MojErr MojDbIndexTest::assertContainsText(TestIndex& ti, MojObject id, const MojChar* str)
{
	MojString strObj;
	MojErr err = strObj.assign(str);
	MojErrCheck(err);
	MojRefCountedPtr<MojDbTextCollator> collator(new MojDbTextCollator);
	MojAllocCheck(collator.get());
	err = collator->init(_T("en_US"), MojDbCollationPrimary);
	MojTestErrCheck(err);
	MojDbKey key;
	err = collator->sortKey(strObj, key);
	MojTestErrCheck(err);

	MojObjectWriter writer;
	err = id.visit(writer);
	MojTestErrCheck(err);
	const MojByte* idData = NULL;
	MojSize idSize = 0;
	err = writer.buf().data(idData, idSize);
	MojTestErrCheck(err);
	err = key.byteVec().append(idData, idData + idSize);
	MojTestErrCheck(err);
	err = assertContains(ti, id, key);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndexTest::assertCanAnswer(const MojChar* propsJson, const MojChar* queryJson,
		bool result, bool indexIncludeDeleted)
{
	MojObject propsObj;
	MojErr err = propsObj.fromJson(propsJson);
	MojTestErrCheck(err);
	MojObject queryObj;
	err = queryObj.fromJson(queryJson);
	MojTestErrCheck(err);

	MojDbIndex index(NULL, NULL);
	MojObject prop;
	MojSize i = 0;
	for (;;) {
		if(propsObj.at(i++, prop)) {
			err = index.addProp(prop);
			MojTestErrCheck(err);
		} else {
			break;
		}
	}
	index.incDel(indexIncludeDeleted);
	MojRefCountedPtr<MojDbStorageIndex> storageIndex(new TestIndex(false));
	MojAllocCheck(storageIndex.get());
	MojDbReq req;
	err = index.open(storageIndex.get(), (MojInt64) 0, req);
	MojTestErrCheck(err);

	MojDbQuery query;
	err = query.fromObject(queryObj);
	MojTestErrCheck(err);
	MojTestAssert(index.canAnswer(query) == result);

	return MojErrNone;
}
