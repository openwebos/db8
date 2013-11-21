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


#include "core/MojTestRunner.h"
#include "core/MojLogEngine.h"
#include "core/MojUtil.h"

MojTestRunner* MojTestRunner::s_instance = NULL;

MojTestRunner::MojTestRunner()
: m_numSucceeded(0),
  m_numFailed(0),
  m_curTest(NULL)
{
	MojAssert(s_instance == NULL);
	s_instance = this;
}

MojTestRunner::~MojTestRunner()
{
	MojAssert(s_instance == this);
	s_instance = NULL;
}

MojErr MojTestRunner::handleArgs(const StringVec& args)
{
	MojErr err = MojApp::handleArgs(args);
	MojErrCheck(err);
	m_testNames = args;

	return MojErrNone;
}

MojErr MojTestRunner::run()
{
	runTests();
	MojPrintF(_T("\n-----------------------------------\n")
			  _T("Results:     %d succeeded, %d failed\n"),
			  m_numSucceeded, m_numFailed);

	return (MojErr) m_numFailed;
}

void MojTestRunner::test(MojTestCase& test)
{
	if (!testEnabled(test))
		return;

	MojPrintF(_T("%-24s: "), test.name());
	fflush(stdout);

	m_curTest = &test;
	test.cleanup();
	MojErr err = test.run();
	test.cleanup();
	m_curTest = NULL;

	if (err == MojErrNone) {
		m_numSucceeded++;
		MojPrintF(_T("Succeeded\n"));
	} else {
		m_numFailed++;
	}
}

bool MojTestRunner::testEnabled(MojTestCase& test)
{
	if (m_testNames.empty())
		return true;
	for (MojVector<MojString>::ConstIterator i = m_testNames.begin();
	     i != m_testNames.end();
	     ++i) {
		if (*i == test.name())
			return true;
	}
	return false;
}

void MojTestRunner::testError(MojErr errTest, const MojChar* file, int line)
{
	MojString str;
	MojErrToString(errTest, str);
	testFailed(str, file, line);
}

void MojTestRunner::testFailed(const MojChar* cond, const MojChar* file, int line)
{
	MojAssert(m_curTest);
	MojThreadGuard guard(m_mutex);

	// put a breakpoint here to break on failed tests
	if (!m_curTest->m_failed) {
		MojPrintF(_T("FAILED\n"));
		m_curTest->m_failed = true;
		m_curTest->failed();
	}
	MojPrintF(_T("ASSERTION FAILED: %s:%d   (%s)\n"), MojFileNameFromPath(file), line, cond);
}
