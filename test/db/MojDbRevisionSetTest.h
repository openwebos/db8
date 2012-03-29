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


#ifndef MOJDBREVISIONSETTEST_H_
#define MOJDBREVISIONSETTEST_H_

#include "MojDbTestRunner.h"

class MojDbRevisionSetTest : public MojTestCase
{
public:
	MojDbRevisionSetTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr standAloneTest();
	MojErr dbTest();
	MojErr checkRev(const MojObject& obj, const MojChar* propName, bool eq);
};

#endif /* MOJDBREVISIONSETTEST_H_ */
