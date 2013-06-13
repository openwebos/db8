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


#ifndef MOJOBJECTFILTERTEST_H_
#define MOJOBJECTFILTERTEST_H_

#include "MojCoreTestRunner.h"
#include "db/MojDbQuery.h"
#include "core/MojObjectFilter.h"

class MojObjectFilterTest: public MojTestCase {
public:
	MojObjectFilterTest();

	virtual MojErr run();

private:
	MojErr testFilteredParse(const MojChar* toParse, MojObject& expected, MojDbQuery::StringSet& set);
	MojErr testFilteredParseReuseVisitor(const MojChar* toParse, MojObject& expected, MojObjectFilter& visitor);
	MojErr testNestedArray();
	MojErr testNestedArray2();
	MojErr testNestedArray3();
	MojErr testNestedArray4();
};

#endif /* MOJOBJECTFILTERTEST_H_ */
