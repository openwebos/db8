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


#ifndef MOJDBINDEXTEST_H_
#define MOJDBINDEXTEST_H_

#include "MojDbTestRunner.h"

class TestIndex;

class MojDbIndexTest : public MojTestCase
{
public:
	MojDbIndexTest();

	virtual MojErr run();

private:
	MojErr invalidTest();
	MojErr simpleTest();
	MojErr nestedTest();
	MojErr compoundTest();
	MojErr canAnswerTest();
	MojErr deletedTest();
	MojErr wildcardTest();
	MojErr defaultValuesTest();
	MojErr multiTest();
	MojErr tokenizeTest();

	MojErr checkInvalid(MojErr errExpected, const MojChar* json);
	MojErr indexFromObject(MojDbIndex& index, const MojChar* json);
	MojErr put(MojDbIndex& index, MojObject id, const MojChar* jsonNew, const MojChar* jsonOld);
	MojErr del(MojDbIndex& index, MojObject id, const MojChar* json, bool purge = false);
	MojErr assertContains(TestIndex& ti, MojObject id, MojObject key);
	MojErr assertContains(TestIndex& ti, MojObject id, const MojChar* json);
	MojErr assertContains(TestIndex& ti, MojObject id, const MojDbKey& key);
	MojErr assertContainsText(TestIndex& ti, MojObject id, const MojChar* str);
	MojErr assertCanAnswer(const MojChar* propsJson, const MojChar* queryJson, bool result,
			bool indexIncludeDeleted = false);
};

#endif /* MOJDBINDEXTEST_H_ */
