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
* Filename              : MojTimeTest.cpp

* Description           : Source file for MojTime test.
****************************************************************************************************
**/
#include "MojTimeTest.h"
#include "core/MojTime.h"

MojTimeTest::MojTimeTest()
: MojTestCase(_T("MojTime"))
{
}

/**
***************************************************************************************************
* @run                    This function tests the time functionality.A MojTime instance is created
                          which initializes the m_val of MojTime class.The functions used in this
                          test suite are the following:
                          1.fromTimeVal:This function gets the values from  MojTimeValT and updates
                             m_val.
                            eg:time.fromTimeval(&tv);
                          2.toTimeval:This function extracts the m_val and assigns to MojTimeValT.
                            eg:time.toTimeval(&tv2);
                          3.fromTimespec:This function gets the values from  MojTimespecT and updates
                             m_val.
                            eg:time.fromTimespec(&ts);
                          4.toTimespec:This function extracts the m_val and assigns to MojTimespecT.
                            eg:time.toTimespec(&ts2);
                          5.secs/millisecs/microsecs/millisecsPart/microsecsPart:
                             The secs,millisecs,microsecs,millisecsPart,microsecsPart extracts the
                             seconds,milliseconds,microseconds,millisecsPart,microsecsPart from the
                             m_val respectively.
                          6.Operator overloading:
                             This include the operator overloading functionality for ==,!=,<,<=,>,>=
                             ,=,+=,-=,*=,/=,%=,++,--,postincrement,postdecrement,+,-,*,/% operators
                             eg:MojTestAssert(time++ == 1);
                                MojTestAssert(++time == 3);
                                MojTestAssert((time *= -10) == 10);
                                time = MojTime(8) / MojTime(4);
                           7.MojSecs/MojMillisecs/MojMicrosecs:This function gets the seconds,
                             milliseconds and micro seconds and assigns to respective m_val.
                             eg:time = MojSecs(3);
                                time = MojMillisecs(45);
                                time = MojMicrosecs(5);
* @param                : None
* @retval               : MojErr
***************************************************************************************************
**/
MojErr MojTimeTest::run()
{
	MojTimevalT tv;
	tv.tv_sec = 400;
	tv.tv_usec = 54321;
	MojTime time = -8;
	time.fromTimeval(&tv);
	MojTestAssert(time == 400054321);
	MojTestAssert(time.secs() == 400);
	MojTestAssert(time.millisecs() == 400054);
	MojTestAssert(time.microsecs() == 400054321);
	MojTestAssert(time.millisecsPart() == 54);
	MojTestAssert(time.microsecsPart() == 54321);
	MojTimevalT tv2;
	time.toTimeval(&tv2);
	MojTestAssert(tv.tv_sec == tv2.tv_sec && tv.tv_usec == tv2.tv_usec);
	MojTimespecT ts;
	ts.tv_sec = 400;
	ts.tv_nsec = 54321;
	time.fromTimespec(&ts);
	MojTestAssert(time == 400000054);
	MojTimespecT ts2;
	time.toTimespec(&ts2);
	MojTestAssert(ts2.tv_sec = 400 && ts2.tv_nsec == 54000);

	time = MojSecs(3);
	MojTestAssert(time == 3000000);
	time = MojMillisecs(45);
	MojTestAssert(time == 45000);
	time = MojMicrosecs(5);
	MojTestAssert(time == 5);
	time = MojSecs(1) + MojMillisecs(2) + MojMicrosecs(3);
	MojTestAssert(time == 1002003);

	time = 1;
	MojTestAssert(time++ == 1);
	MojTestAssert(time == 2);
	MojTestAssert(++time == 3);
	MojTestAssert(--time == 2);
	MojTestAssert(time-- == 2);
	MojTestAssert(time == 1);
	MojTestAssert((time += 2) == 3);
	MojTestAssert((time -= 4) == -1);
	MojTestAssert((time *= -10) == 10);
	MojTestAssert((time /= 2) == 5);
	time = MojTime(1) + MojTime(2);
	MojTestAssert(time == 3);
	time = MojTime(8) - MojTime(6);
	MojTestAssert(time == 2);
	time = MojTime(8) * MojTime(6);
	MojTestAssert(time == 48);
	time = MojTime(8) / MojTime(4);
	MojTestAssert(time == 2);

	return MojErrNone;
}

