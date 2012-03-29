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


#include "MojDbTestRunner.h"
#include "MojDbBulkTest.h"
#include "MojDbConcurrencyTest.h"
#include "MojDbCrudTest.h"
#include "MojDbDumpLoadTest.h"
#include "MojDbIndexTest.h"
#include "MojDbKindTest.h"
#include "MojDbLocaleTest.h"
#include "MojDbPermissionTest.h"
#include "MojDbPurgeTest.h"
#include "MojDbQueryTest.h"
#include "MojDbQueryFilterTest.h"
#include "MojDbQuotaTest.h"
#include "MojDbRevTest.h"
#include "MojDbRevisionSetTest.h"
#include "MojDbSearchTest.h"
#include "MojDbTextCollatorTest.h"
#include "MojDbTextTokenizerTest.h"
#include "MojDbWatchTest.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-luna/MojDbLevelFactory.h"
#endif

const MojChar* const MojDbTestDir = _T("mojodb-test-dir");

int main(int argc, char** argv)
{
	MojDbTestRunner runner;
   // set up bdb first
#ifdef MOJ_USE_BDB
   MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
   MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#endif
    
	return runner.main(argc, argv);
}

void MojDbTestRunner::runTests()
{
	test(MojDbBulkTest());
	test(MojDbConcurrencyTest());
	test(MojDbCrudTest());
	test(MojDbDumpLoadTest());
	test(MojDbIndexTest());
	test(MojDbKindTest());
	test(MojDbLocaleTest());
	test(MojDbPermissionTest());
	test(MojDbPurgeTest());
	test(MojDbQueryTest());
	test(MojDbQueryFilterTest());
	test(MojDbQuotaTest());
	test(MojDbRevTest());
	test(MojDbRevisionSetTest());
	test(MojDbSearchTest());
	test(MojDbTextCollatorTest());
	test(MojDbTextTokenizerTest());
	test(MojDbWatchTest());
}
