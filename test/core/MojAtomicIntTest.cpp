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
* Filename              : MojAtomicTest.cpp
* Description           : Source file for MojAtomic test.
****************************************************************************************************
**/

#include "MojAtomicIntTest.h"

MojAtomicIntTest::MojAtomicIntTest()
: MojTestCase(_T("MojAtomicInt"))
{
}
/**
****************************************************************************************************
* @run              MojAtomicInt is a wrapper class over int datatype.
                    1. For storing integer number, instance of MojAtomic class is created and
                       the value to be assigned is given as a parameter to the constructor.
                       By default 0 is assigned.
                    2. It provides utility functions to perform arithemetic operations atomically.
                       Ex: increments() adds 1 to number
                           decrement() subtracts 1 from number etc..
                    3. Provides overloaded function for all relational operators.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
MojErr MojAtomicIntTest::run ()
{
	MojAtomicInt i1;
	MojAtomicInt i2(25);
	MojAtomicInt i3(i2);

	MojTestAssert(i1 == 0);
	MojTestAssert(i2 == 25);
	MojTestAssert(i3 == 25);
	MojTestAssert(i2 == i3);
	MojTestAssert(i1 != i2);
	i1 = i3;
	i3 = 0;
	MojTestAssert(i3 == 0);
	MojTestAssert(i1 == 25);
	MojTestAssert(i1.increment() == 26);
	MojTestAssert(i1 == 26);
	MojTestAssert(++i3 == 1);
	MojTestAssert(i3 == 1);
	MojTestAssert(i1.decrement() == 25);
	MojTestAssert(i1 == 25);
	MojTestAssert(--i3 == 0);
	MojTestAssert(i3 == 0);

	return MojErrNone;
}
