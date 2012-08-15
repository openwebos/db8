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


#include "MojTimeTest.h"
#include "core/MojTime.h"

MojTimeTest::MojTimeTest()
: MojTestCase(_T("MojTime"))
{
}

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

