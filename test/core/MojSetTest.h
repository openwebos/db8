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


#ifndef MOJSETTEST_H_
#define MOJSETTEST_H_

#include "MojCoreTestRunner.h"
#include "core/MojSet.h"

class MojSetTest : public MojTestCase
{
public:
	MojSetTest();

	virtual MojErr run();

private:
	MojSize count(MojSet<int>& set);
};

#endif /* MOJSETTEST_H_ */
