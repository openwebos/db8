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


#ifndef MOJOBJECTTEST_H_
#define MOJOBJECTTEST_H_

#include "MojCoreTestRunner.h"

class MojObjectTest : public MojTestCase
{
public:
	MojObjectTest();

	MojErr run();

protected:
	MojObjectTest(const MojChar* name) : MojTestCase(name) {}

	MojErr emptyTest(MojObject& obj);
	MojErr putTest(MojObject& obj);
	MojErr getTest(const MojObject& obj);
	MojErr typeTest();
};

#endif /* MOJOBJECTTEST_H_ */
