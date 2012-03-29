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


#include "MojErrTest.h"
#include "core/MojLogEngine.h"

static MojErr throwAnErr(bool cond, MojErr errToThrow)
{
	if (cond)
		MojErrThrow(errToThrow);
	return MojErrNone;
}

static MojErr throwAnErrMsg(bool cond, MojErr errToThrow)
{
	if (cond)
		MojErrThrowMsg(errToThrow, _T("%s %d"), _T("hello"), 1);
	return MojErrNone;
}

static MojErr checkAnErr(bool cond, MojErr errToThrow, bool& completed)
{
	completed = false;
	MojErr err = throwAnErr(cond, errToThrow);
	MojErrCheck(err);
	completed = true;
	return MojErrNone;
}

MojErrTest::MojErrTest()
: MojTestCase(_T("MojErr"))
{
}

MojErr MojErrTest::run()
{
	MojErr err = throwAnErr(false, MojErrNoMem);
	MojTestAssert(err == MojErrNone);
	err = throwAnErr(true, MojErrNoMem);
	MojTestAssert(err == MojErrNoMem);
	bool caught = false;
	MojErrCatch(err, MojErrLocked) {
		caught = true;
	}
	MojTestAssert(!caught);
	MojErrCatch(err, MojErrNoMem) {
		caught = true;
	}
	MojTestAssert(caught);
	MojTestAssert(err == MojErrNone);
	err = throwAnErr(true, MojErrNoMem);
	caught = false;
	MojErrCatchAll(err) {
		caught = true;
	}
	MojTestAssert(caught);
	MojTestAssert(err == MojErrNone);

	MojErr eacc = MojErrNone;
	err = throwAnErrMsg(true, MojErrInvalidArg);
	MojErrAccumulate(eacc, err);
	MojTestAssert(eacc == MojErrInvalidArg);
	MojErrAccumulate(eacc, MojErrNone);
	MojTestAssert(eacc == MojErrInvalidArg);
	MojErrCatchAll(eacc);

	bool completed = false;
	err = checkAnErr(false, MojErrNoMem, completed);
	MojTestAssert(err == MojErrNone && completed);
	err = checkAnErr(true, MojErrNoMem, completed);
	MojTestAssert(err == MojErrNoMem && !completed);
	MojErrCatchAll(err);

	return MojErrNone;
}
