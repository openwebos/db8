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


#ifndef MOJDBSEARCHCACHETEST_H_
#define MOJDBSEARCHCACHETEST_H_

#include "MojDbTestRunner.h"
#include "db/MojDbSearchCache.h"

class MojDbSearchCacheTest : public MojTestCase
{
public:
    MojDbSearchCacheTest();

    virtual MojErr run();
    virtual void cleanup();

private:
    MojErr operatorTest(MojDb& db);

    MojErr testQueryKey();
    MojErr testCache();
    MojErr prepareIdSet(MojDbSearchCache::IdSet& a_idSet, const char* a_nameArray[], unsigned a_size);

    MojErr initQuery(MojDbQuery& query, const MojChar* queryStr,
            const MojChar* orderBy = NULL, const MojObject& barVal = MojObject::Undefined, bool desc = false);
    MojErr check(MojDb& db, const MojChar* queryStr, const MojChar* expectedIdsJson,
            const MojChar* orderBy = NULL, const MojObject& barVal = MojObject::Undefined, bool desc = false);
    MojErr check(MojDb& db, const MojDbQuery& query, const MojChar* expectedIdsJson);
};
#endif /* MOJDBSEARCHCACHETEST_H_ */
