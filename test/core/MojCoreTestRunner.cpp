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
* Filename              : MojCoreTestRunner.cpp
* Description           : Executes testcases of core library.
****************************************************************************************************
**/

#include "MojCoreTestRunner.h"
#include "MojAtomicIntTest.h"
#include "MojAutoPtrTest.h"
#include "MojBufferTest.h"
#include "MojDataSerializationTest.h"
#include "MojDecimalTest.h"
#include "MojErrTest.h"
#include "MojObjectFilterTest.h"
#include "MojHashMapTest.h"
#include "MojJsonTest.h"
#include "MojListTest.h"
#include "MojLogTest.h"
#include "MojMapTest.h"
#include "MojMessageDispatcherTest.h"
#include "MojObjectTest.h"
#include "MojObjectSerializationTest.h"
#include "MojReactorTest.h"
#include "MojRefCountTest.h"
#include "MojSchemaTest.h"
#include "MojSetTest.h"
#include "MojSignalTest.h"
#include "MojStringTest.h"
#include "MojThreadTest.h"
#include "MojTimeTest.h"
#include "MojUtilTest.h"
#include "MojVectorTest.h"

int main(int argc, char** argv)
{
	MojCoreTestRunner runner;
	return runner.main(argc, argv);
}
/**
****************************************************************************************************
* @runTests         Test execution starts from here.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
void MojCoreTestRunner::runTests()
{
	test(MojAtomicIntTest());
	test(MojAutoPtrTest());
	test(MojBufferTest());
	test(MojDataSerializationTest());
	test(MojDecimalTest());
	test(MojErrTest());
	test(MojHashMapTest());
	test(MojJsonTest());
	test(MojListTest());
	test(MojMapTest());
	test(MojMessageDispatcherTest());
	test(MojObjectTest());
	test(MojObjectFilterTest());
	test(MojObjectSerializationTest());
	test(MojReactorTest());
	test(MojRefCountTest());
	test(MojSchemaTest());
	test(MojSetTest());
	test(MojSignalTest());
	test(MojStringTest());
	test(MojThreadTest());
	test(MojTimeTest());
	test(MojUtilTest());
	test(MojVectorTest());
}
