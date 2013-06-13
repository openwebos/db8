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


#ifndef MOJDBSEARCHTEST_H_
#define MOJDBSEARCHTEST_H_

#include "MojDbTestRunner.h"

class MojDbSearchTest : public MojTestCase
{
public:
	MojDbSearchTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr simpleTest(MojDb& db);
	MojErr filterTest(MojDb& db);

	MojErr initQuery(MojDbQuery& query, const MojChar* queryStr,
			const MojChar* orderBy = NULL, const MojObject& barVal = MojObject::Undefined, bool desc = false);
	MojErr check(MojDb& db, const MojChar* queryStr, const MojChar* expectedIdsJson,
			const MojChar* orderBy = NULL, const MojObject& barVal = MojObject::Undefined, bool desc = false);
	MojErr check(MojDb& db, const MojDbQuery& query, const MojChar* expectedIdsJson);
};

#endif /* MOJDBSEARCHTEST_H_ */
