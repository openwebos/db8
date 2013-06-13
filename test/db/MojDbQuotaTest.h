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


#ifndef MOJDBQUOTATEST_H_
#define MOJDBQUOTATEST_H_

#include "MojDbTestRunner.h"

class MojDbQuotaTest : public MojTestCase
{
public:
	MojDbQuotaTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testUsage(MojDb& db);
	MojErr testMultipleQuotas(MojDb& db);
	MojErr testEnforce(MojDb& db);
	MojErr testErrors();
	MojErr put(MojDb& db, const MojChar* objJson);
	MojErr getKindUsage(MojDb& db, const MojChar* kindId, MojInt64& usageOut);
	MojErr getQuotaUsage(MojDb& db, const MojChar* owner, MojInt64& usageOut);
};

#endif /* MOJDBQUOTATEST_H_ */
