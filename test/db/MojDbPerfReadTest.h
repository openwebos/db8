/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBPERFREADTEST_H_
#define MOJDBPERFREADTEST_H_

#include "MojDbPerfTest.h"
#include "db/MojDbQuery.h"

class MojDbPerfReadTest : public MojDbPerfTest {
public:
	MojDbPerfReadTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testGet(MojDb& db);
	MojErr testFindAll(MojDb& db);
	MojErr testFindPaged(MojDb& db);

	MojErr getObjs(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64));
	MojErr findObjs(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64), MojDbQuery& q);
	MojErr findObjsPaged(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn)(MojObject&, MojUInt64), MojDbQuery& query);

	MojErr timeGet(MojDb& db, MojObject& id, MojTime& getTime);
	MojErr timeBatchGet(MojDb& db, const MojObject* begin, const MojObject* end, MojTime& batchGetTime, bool useWriter);
	MojErr timeFind(MojDb& db, MojDbQuery& query, MojTime& findTime, bool useWriter, MojDbQuery::Page& nextPage, bool doCount, MojUInt32& count, MojTime& countTime);


	MojErr putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
			MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64), MojObject& ids);

};


#endif /* MOJDBPERFREADTEST_H_ */
