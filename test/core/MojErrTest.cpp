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


#include "MojErrTest.h"
#include "core/MojLogEngine.h"

/**
****************************************************************************************************
* Filename              : MojErrTest.cpp
* Description           : Source file for MojErr test.
****************************************************************************************************
**/

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
/**
****************************************************************************************************
* @run              Testcase to check runtime error handling and logging of errors in DB8.
                    1. Throw and Catch utility are used to handle run time errors.
                       Ex:
                          throwAnErr(true, MojErrNoMem) -> API to throw an error
                          MojErrCatch(err, MojErrNoMem) -> API to catch an error.
                          MojErrCatchAll(err)           -> API to catch an error.
                          In catch block err should not be equal to MojErrNone.

                    2. MojErrAccumulate(..): This function writes error messages with information
                       like file name, function name, line number to logger.

                    *  throwAnErr(false,MojErrNoMem) - Error is caught by MojErrCatch(..) function
                                                       on setting the condition to 'true'.
                                                       MojErrNoMem cannot be caught as condition =
                                                       'false'.

                    *  throwAnErr(true,MojErrNoMem)  - This error is caught as it is thrown with
                                                       condition = 'true'.

                    *  throwAnErrMsg(true, MojErrInvalidArg)  - Writes error message to the logger
                                                                on setting the condition to 'true'

* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
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
