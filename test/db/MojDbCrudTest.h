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


#ifndef MOJDBCRUDTEST_H_
#define MOJDBCRUDTEST_H_

#include "MojDbTestRunner.h"

class MojDbCrudTest: public MojTestCase
{
public:
	MojDbCrudTest();

	virtual MojErr run();
	virtual void cleanup();

protected:
	MojErr lockTest();
	MojErr simpleTest(MojDb& db);
	MojErr bigTest(MojDb& db);
	MojErr mergeTest(MojDb& db);
	MojErr mergeArrayTest(MojDb& db);
	MojErr arrayTest(MojDb& db);
	MojErr defaultValuesTest(MojDb& db);
	MojErr persistenceTest();
	MojErr staleUpdateTest(MojDb& db);
	MojErr largeObjectTest(MojDb& db);
	//MojErr objectWithNullCharTest(MojDb& db);
};

#endif /* MOJDBCRUDTEST_H_ */
