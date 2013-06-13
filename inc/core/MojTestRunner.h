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


#ifndef MOJTESTRUNNER_H_
#define MOJTESTRUNNER_H_

#include "core/MojCoreDefs.h"
#include "core/MojApp.h"
#include "core/MojThread.h"

#define MojTestAssert(COND)	do if (!(COND)) {MojTestRunner::instance()->testFailed(_T(#COND), _T(__FILE__), __LINE__); MojErrThrow(MojErrTestFailed);} while(0)
#define MojTestErrCheck(VAR) do if ((VAR) != MojErrNone) {MojTestRunner::instance()->testError((VAR), _T(__FILE__), __LINE__); MojErrThrow(VAR);} while(0)
#define MojTestErrExpected(VAR, VAL) do {if ((VAR) == (VAL)) VAR = MojErrNone; else MojTestAssert((VAR) == (VAL));} while(0)

struct MojTestCaseRef
{
	MojTestCaseRef(MojTestCase& tc) : m_case(tc) {}
	MojTestCase& m_case;
};

class MojTestCase
{
public:
    virtual ~MojTestCase() {}
	virtual MojErr run() = 0;
	virtual void failed() {}
	virtual void cleanup() {}
	const MojChar* name() const { return m_name; }

	operator MojTestCaseRef() { return MojTestCaseRef(*this); }

protected:
	friend class MojTestRunner;
	MojTestCase(const MojChar* name) : m_name(name), m_failed(false) {}

	const MojChar* m_name;
	bool m_failed;
};

class MojTestRunner : public MojApp
{
public:
	MojTestRunner();
	virtual ~MojTestRunner();

	static MojTestRunner* instance() { return s_instance; }
	void testError(MojErr errTest, const MojChar* file, int line);
	void testFailed(const MojChar* cond, const MojChar* file, int line);

protected:
	virtual MojErr run();
	virtual void runTests() = 0;
	void test(MojTestCase& test);
	void test(MojTestCaseRef ref) { test(ref.m_case); }

private:
	virtual MojErr handleArgs(const StringVec& args);
	bool testEnabled(MojTestCase& test);

	int m_numSucceeded;
	int m_numFailed;
	MojTestCase* m_curTest;
	StringVec m_testNames;
	MojThreadMutex m_mutex;
	static MojTestRunner* s_instance;
};

#endif /* MOJTESTRUNNER_H_ */
