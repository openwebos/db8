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


#ifndef MOJDBWATCHTEST_H_
#define MOJDBWATCHTEST_H_

#include "MojDbTestRunner.h"

class MojDbWatchTest : public MojTestCase
{
public:
	MojDbWatchTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr eqTest(MojDb& db);
	MojErr gtTest(MojDb& db);
	MojErr ltTest(MojDb& db);
	MojErr cancelTest(MojDb& db);
	MojErr rangeTest(MojDb& db);
	MojErr pageTest(MojDb& db);
	MojErr limitTest(MojDb& db);

	MojErr put(MojDb& db, const MojObject& fooVal, const MojObject& barVal, MojObject& idOut, MojInt64& revOut);
	MojErr merge(MojDb& db, const MojObject& id, const MojObject& barVal);

	MojInt64 m_rev;
};

#endif /* MOJDBWATCHTEST_H_ */
