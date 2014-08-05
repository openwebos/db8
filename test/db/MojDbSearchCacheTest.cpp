/* @@@LICENSE
*
*      Copyright (c) 2014 LG Electronics, Inc.
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

#include "MojDbSearchCacheTest.h"
#include "db/MojDb.h"
#include "db/MojDbSearchCursor.h"
#include "db/MojDbSearchCache.h"
#include "core/MojObjectBuilder.h"
#include <cstring>
#include <map>

static const MojChar* const SearchCacheKindStr =
    _T("{\"id\":\"test.cache:1\",")
    _T("\"owner\":\"com.palm.configurator\",")
    _T("\"indexes\":[")
    _T("{\"name\":\"i1\",\"props\":[{\"name\":\"attr1\"}]},")
    _T("{\"name\":\"i2\",\"props\":[{\"name\":\"attr2\"}]}")
    _T("]}");

static const MojChar* const MojSearchCacheObjects[] = {
    _T("{\"_id\":\"id1\",\"_kind\":\"test.cache:1\",\"attr1\":1,\"attr2\":1}"),
    _T("{\"_id\":\"id2\",\"_kind\":\"test.cache:1\",\"attr1\":2,\"attr2\":2}"),
    _T("{\"_id\":\"id3\",\"_kind\":\"test.cache:1\",\"attr1\":3,\"attr2\":3}"),
    _T("{\"_id\":\"id4\",\"_kind\":\"test.cache:1\",\"attr1\":4,\"attr2\":4}"),
    _T("{\"_id\":\"id5\",\"_kind\":\"test.cache:1\",\"attr1\":5,\"attr2\":5}"),
    _T("{\"_id\":\"id6\",\"_kind\":\"test.cache:1\",\"attr1\":6,\"attr2\":6}"),
    _T("{\"_id\":\"id7\",\"_kind\":\"test.cache:1\",\"attr1\":7,\"attr2\":7}"),
    _T("{\"_id\":\"id8\",\"_kind\":\"test.cache:1\",\"attr1\":8,\"attr2\":8}"),
    _T("{\"_id\":\"id9\",\"_kind\":\"test.cache:1\",\"attr1\":9,\"attr2\":9}"),
    _T("{\"_id\":\"id10\",\"_kind\":\"test.cache:1\",\"attr1\":10,\"attr2\":10}")
};

static const MojChar* const OperatorQuery1 =
    _T("{\"from\":\"test.cache:1\",\"where\":[{\"prop\":\"attr1\",\"op\":\">\",\"val\":2}]}");

MojDbSearchCacheTest::MojDbSearchCacheTest()
: MojTestCase(_T("MojDbSearchCache"))
{
}

MojErr MojDbSearchCacheTest::run()
{
    MojDb db;
    MojErr err = db.open(MojDbTestDir);
    MojTestErrCheck(err);

    // add kind
    MojObject kindObj;
    err = kindObj.fromJson(SearchCacheKindStr);
    MojTestErrCheck(err);
    err = db.putKind(kindObj);
    MojTestErrCheck(err);

    // put test objects
    for (MojSize i = 0; i < sizeof(MojSearchCacheObjects) / sizeof(MojChar*); ++i) {
        MojObject obj;
        err = obj.fromJson(MojSearchCacheObjects[i]);
        MojTestErrCheck(err);
        err = db.put(obj);
        MojTestErrCheck(err);
    }

    err = testQueryKey();
    MojTestErrCheck(err);

    err = operatorTest(db);
    MojTestErrCheck(err);

    /*** :: TODO :: make query scenario
    err = queryTest(db);
    MojTestErrCheck(err);
    ***/

    err = db.close();
    MojTestErrCheck(err);

    return MojErrNone;
}

void MojDbSearchCacheTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
}

MojErr MojDbSearchCacheTest::testQueryKey()
{
    MojDbQuery query1;
    MojErr err=query1.from(_T("test:1"));
    MojTestErrCheck(err);
    err=query1.where(_T("attr1"), MojDbQuery::OpGreaterThan, 1);
    MojTestErrCheck(err);

    MojDbSearchCache::QueryKey key1;
    key1.fromQuery(query1, 1234);

    // Verify fromQuery() works fine.
    MojTestAssert(key1.getKind() == _T("test:1"));
    MojTestAssert(key1.getRev() == 1234);

    MojDbSearchCache::QueryKey key2;
    key2.fromQuery(query1, 1235);

    // key1 shall be less then key2.
    //
    MojTestAssert(key1 < key2);
    MojTestAssert(!(key2 < key1));
    MojTestAssert(!(key1 == key2));

    // key1 shall be equal to key1.
    //
    MojTestAssert(!(key1 < key1));
    MojTestAssert(key1 == key1);

    return MojErrNone;
}

MojErr MojDbSearchCacheTest::prepareIdSet(MojDbSearchCache::IdSet& a_idSet, const char* a_nameArray[], unsigned a_size)
{
    MojErr err=MojErrNone;

    for (unsigned i=0; i<a_size; ++i)
    {
        MojObject id;
        err = id.putString(_T("name"), a_nameArray[i]);
        MojTestErrCheck(err);
        err = a_idSet.push(id);
        MojTestErrCheck(err);
    }

    return err;
}

MojErr MojDbSearchCacheTest::testCache()
{
    MojDbSearchCache cache;

    // Prepare query, "from":"test:1", "where":[{"prop":"attr1", "op":">", "val":1}]
    //
    MojDbQuery query1;
    MojErr err=query1.from(_T("test:1"));
    MojTestErrCheck(err);
    err=query1.where(_T("attr1"), MojDbQuery::OpGreaterThan, 1);
    MojTestErrCheck(err);

    // Create QueryKey for query where kind revision=1000.
    //
    MojDbSearchCache::QueryKey key1;
    err=key1.fromQuery(query1, 1000);
    MojTestErrCheck(err);

    // Same query, but new kind revision.
    MojDbSearchCache::QueryKey key2;
    err=key2.fromQuery(query1, 1001);
    MojTestErrCheck(err);

    // Prepare ID Set which includes 5 persons' name.
    //
    MojDbSearchCache::IdSet idSet1;
    const char *nameArray1[] = { "Junku", "Hongbin", "Hyungjoon", "Seonghwan", "Steve" };
    err=prepareIdSet(idSet1, nameArray1, (sizeof(nameArray1)/sizeof(nameArray1[0])));
    MojTestErrCheck(err);

    // Create cache.
    err=cache.createCache(key1, idSet1);
    MojTestErrCheck(err);

    // We should find it here.
    MojTestAssert(true == cache.contain(key1));
    MojTestErrCheck(err);

    // Get from the cache.
    MojDbSearchCache::IdSet resultIds;
    err=cache.getIdSet(key1, resultIds);
    MojTestErrCheck(err);

    // Checks whether it's same.
    MojTestAssert(idSet1 == resultIds);



    // Checks whether we don't have key2.
    MojTestAssert(false == cache.contain(key2));

    // Prepare next items, idSet2.
    MojDbSearchCache::IdSet idSet2;
    const char *nameArray2[] = { "Jack", "Bill", "Kerhii" };
    err=prepareIdSet(idSet2, nameArray2, (sizeof(nameArray2)/sizeof(nameArray2[0])));
    MojTestErrCheck(err);

    // Update the cache with ID Set2.
    // Then, it should replace IdSet1, since it's on same kind.
    //
    err=cache.updateCache(key2, idSet2);
    MojTestErrCheck(err);

    MojDbSearchCache::IdSet resultIds2;
    MojTestAssert(true == cache.contain(key2));
    err=cache.getIdSet(key1, resultIds2);
    MojTestErrCheck(err);

    MojTestAssert(idSet1 != resultIds2);
    MojTestAssert(idSet2 == resultIds2);

    err=cache.destroyCache(key1);
    MojTestErrCheck(err);

    MojTestAssert(false == cache.contain(key2));

    return MojErrNone;
}

// test operator is active properly (through create / find / update / delete operation)
MojErr MojDbSearchCacheTest::operatorTest(MojDb& db)
{
    MojDbSearchCache* cache  = new MojDbSearchCache();
    MojDbSearchCache::QueryKey key1, key2, key3, key4;
    MojObject queryObj;
    MojDbQuery query, query1, query2;
    MojErr err;
    MojUInt32 rev = 0;
    MojDbSearchCache::IdSet ids1, ids2;

    MojObject id;
    err = id.putString(_T("_id"), _T("id1"));
    MojTestErrCheck(err);
    err = ids1.push(id);
    MojTestErrCheck(err);
    err = ids2.push(id);
    MojTestErrCheck(err);

    err = query.from(_T("test.cache:1"));
    MojTestErrCheck(err);
    err = query.where(_T("attr1"), MojDbQuery::OpGreaterThan, 2);
    MojTestErrCheck(err);

    key1.setRev(rev);
    key1.setQuery(query);

    key2.setRev(rev);
    key2.setQuery(query);

    err = cache->createCache(key1, ids1);
    MojTestErrCheck(err);
    err = cache->createCache(key2, ids2);
    MojTestErrCheck(err);

    /*** :: For debug ::
    for(MojDbSearchCache::Container::const_iterator i = cache->container().begin(); i != cache->container().end(); ++i) {
        MojString str1, str2;
        MojObject obj;
        MojUInt32 rev = i->first.getRev();
        str1 = i->first.getQuery();
        for(MojDbSearchCache::IdSet::ConstIterator it = i->second.begin(); it != i->second.end(); ++it) {
            err = it->toJson(str2);
            MojTestErrCheck(err);
        }

        printf("key(rev, query) : (%d, %s), ids : %s\n", rev, str1.data(), str2.data());
    }
    ***/

    MojTestAssert(cache->container().count(key1) == 1);
    MojTestAssert(cache->container().size() == 1);
    MojTestAssert(cache->contain(key1) == true);

    // nothing to update -> no-op
    err = cache->updateCache(key1, ids1);
    MojTestErrCheck(err);
    MojTestAssert(cache->container().count(key1) == 1);
    MojTestAssert(cache->container().size() == 1);
    MojTestAssert(cache->contain(key1) == true);

    err = query1.from(_T("test.cache:1"));
    MojTestErrCheck(err);
    err = query1.where(_T("attr2"), MojDbQuery::OpGreaterThan, _T(6));
    MojTestErrCheck(err);
    key3.setRev(rev);
    key3.setQuery(query1);

    // key is changed, but kind is same --> update cache
    err = cache->updateCache(key3, ids1);
    MojTestAssert(cache->container().count(key3) == 1);
    MojTestAssert(cache->container().size() == 1);
    MojTestAssert(cache->contain(key3) == true);

    err = query2.from(_T("test.update:1"));
    MojTestErrCheck(err);
    err = query2.where(_T("attr2"), MojDbQuery::OpGreaterThan, _T(6));
    MojTestErrCheck(err);
    rev = 5;
    key4.setRev(rev);
    key4.setQuery(query2);

    // key is changed -> update cache(add new cache because it is owned by different kind)
    err = cache->updateCache(key4, ids1);
    MojTestErrCheck(err);
    MojTestAssert(cache->container().count(key4) == 1);
    MojTestAssert(cache->container().size() == 2);
    MojTestAssert(cache->contain(key4) == true);

    //delete the cache added
    err = cache->destroyCache(key4);
    MojTestErrCheck(err);
    MojTestAssert(cache->container().count(key4) == 0);
    MojTestAssert(cache->container().size() == 1);
    MojTestAssert(cache->contain(key4) == false);

    return MojErrNone;
}
