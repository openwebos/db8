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


#ifndef MOJDBPERFDELETETEST_H_
#define MOJDBPERFDELETETEST_H_

#include "MojDbPerfTest.h"
#include "db/MojDb.h"

class MojDbPerfDeleteTest: public MojDbPerfTest {
public:
	MojDbPerfDeleteTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testDelete();

	MojErr delObjects(MojDb& db, const MojChar* kindId, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64));

	MojErr putObjs(MojDb& db, const MojChar* kindId, MojUInt64 numInsert,
			MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64), MojObject& objs);

	MojErr mergeObj(MojDb& db, MojObject& obj, MojTime& objTime);
	MojErr batchDelObj(MojDb& db, MojObject* begin, const MojObject* end, MojUInt32& countOut, MojTime& objTime, MojUInt32 flags = MojDb::FlagNone);
	MojErr queryDelObj(MojDb& db, MojDbQuery& query, MojUInt32& countOut, MojTime& objTime, MojUInt32 flags = MojDb::FlagNone);
	MojErr delObj(MojDb& db, MojObject& id, MojTime& objTime, MojUInt32 flags = MojDb::FlagNone);
	MojErr batchPutObj(MojDb& db, MojObject* begin, const MojObject* end, MojTime& objTime);
	MojErr updateKind(const MojChar* kindId, const MojChar* kindJson, const MojChar* extraIdxJson, MojErr (MojDbPerfTest::*createFn) (MojObject&, MojUInt64));
	MojErr timeUpdateKind(MojDb& db, const MojChar* kindJson, MojObject& kindObj, MojTime& addIndexTime, MojTime& dropIndexTime);
};

#endif /* MOJDBPERFDELETETEST_H_ */
