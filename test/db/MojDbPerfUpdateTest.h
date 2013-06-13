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


#ifndef MOJDBPERFUPDATETEST_H_
#define MOJDBPERFUPDATETEST_H_

#include "MojDbPerfTest.h"

class MojDbPerfUpdateTest: public MojDbPerfTest {
public:
	MojDbPerfUpdateTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testPut(MojDb& db);
	MojErr testMerge(MojDb& db);
	MojErr testUpdateKind(MojDb& db);

	MojErr updateObjsViaPut(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64));
	MojErr updateObjsViaMerge(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64));

	MojErr putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
			MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64), MojObject& objs);

	MojErr mergeObj(MojDb& db, MojObject& obj, MojTime& objTime);
	MojErr batchMergeObj(MojDb& db, MojObject* begin, const MojObject* end, MojTime& objTime);
	MojErr queryMergeObj(MojDb& db, MojDbQuery& query, MojObject& props, MojUInt32& count, MojTime& objTime);
	MojErr putObj(MojDb& db, MojObject& obj, MojTime& objTime);
	MojErr batchPutObj(MojDb& db, MojObject* begin, const MojObject* end, MojTime& objTime);
	MojErr updateKind(MojDb& db, const MojChar* kindId, const MojChar* kindJson, const MojChar* extraIdxJson, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64));
	MojErr timeUpdateKind(MojDb& db, const MojChar* kindJson, MojObject& kindObj, MojTime& addIndexTime, MojTime& dropIndexTime);
};

#endif /* MOJDBPERFUPDATETEST_H_ */
