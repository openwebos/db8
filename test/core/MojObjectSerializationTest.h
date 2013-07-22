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

/**
****************************************************************************************************
* Filename              : MojObjectSerializationTest.h
* Description           : Header file for MojObjectSerialization test.
****************************************************************************************************
**/

#ifndef MOJOBJECTSERIALIZATIONTEST_H_
#define MOJOBJECTSERIALIZATIONTEST_H_

#include "MojCoreTestRunner.h"

class MojObjectSerializationTest : public MojTestCase
{
public:
	MojObjectSerializationTest();

	virtual MojErr run();

private:
	MojErr compTest(const MojObject& obj1, const MojObject& obj2);
};

#endif /* MOJOBJECTSERIALIZATIONTEST_H_ */
