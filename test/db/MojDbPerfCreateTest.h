/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBPERFCREATETEST_H_
#define MOJDBPERFCREATETEST_H_

#include "MojDbPerfTest.h"

class MojDbPerfCreateTest: public MojDbPerfTest {
public:
	MojDbPerfCreateTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testCreate();
	MojErr testInsertSmallObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertMedObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertLgObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertMedNestedObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertLgNestedObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertMedArrayObj(MojDb& db, const MojChar* kindId);
	MojErr testInsertLgArrayObj(MojDb& db, const MojChar* kindId);
	MojErr testBatchInsertLgObj(MojDb& db, const MojChar* kindId);
	MojErr testBatchInsertLgNestedObj(MojDb& db, const MojChar* kindId);
	MojErr testBatchInsertLgArrayObj(MojDb& db, const MojChar* kindId);

	MojErr putSmallObj(MojDb& db, const MojChar* kindId, MojTime& smallObjTime);
	MojErr putMedObj(MojDb& db, const MojChar* kindId, MojTime& medObjTime);
	MojErr putLargeObj(MojDb& db, const MojChar* kindId, MojTime& largeObjTime);
	MojErr putMedNestedObj(MojDb& db, const MojChar* kindId, MojTime& medNestedObjTime);
	MojErr putLargeNestedObj(MojDb& db, const MojChar* kindId, MojTime& lgNestedObjTime);
	MojErr putMedArrayObj(MojDb& db, const MojChar* kindId, MojTime& medArrayObjTime);
	MojErr putLargeArrayObj(MojDb& db, const MojChar* kindId, MojTime& lgArrayObjTime);
	MojErr batchPutLargeObj(MojDb& db, const MojChar* kindId, MojTime& largeObjTime);
	MojErr batchPutLargeNestedObj(MojDb& db, const MojChar* kindId, MojTime& lgNestedObjTime);
	MojErr batchPutLargeArrayObj(MojDb& db, const MojChar* kindId, MojTime& lgArrayObjTime);

};

#endif /* MOJDBPERFCREATETEST_H_ */
