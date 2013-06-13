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


#include "MojThreadTest.h"
#include "core/MojThread.h"
#include "core/MojTime.h"

struct MojThreadTestArgs
{
	MojThreadTestArgs() : m_counter(0), m_wait(true) {}

	MojInt32 m_counter;
	bool m_wait;
	MojAtomicInt m_atomicCounter;
	MojThreadMutex m_mutex;
	MojThreadRwLock m_rwlock;
	MojThreadCond m_countCond;
	MojThreadCond m_waitCond;
};

static const int MojTestNumThreads = 10;
static const int MojTestNumIterations = 1000;

static MojErr MojThreadTestFn(void* arg)
{
	static MojThreadLocalValue<int, MojThreadLocalValueZeroCtor<int> > s_localVal;
	MojThreadTestArgs* targs = (MojThreadTestArgs*) arg;
	MojTestAssert(targs);

	for (int i = 0; i < MojTestNumIterations; ++i) {
		targs->m_atomicCounter.increment();
		MojThreadGuard guard(targs->m_mutex, false);
		if (!guard.tryLock())
			guard.lock();
		++(targs->m_counter);
		if (targs->m_counter == (MojTestNumThreads * MojTestNumIterations)) {
			MojErr err = targs->m_countCond.signal();
			MojTestErrCheck(err);
		}
	}

	MojErr err = MojThreadYield();
	MojTestErrCheck(err);

	MojThreadGuard guard(targs->m_mutex);
	while (targs->m_wait) {
		err = targs->m_waitCond.wait(targs->m_mutex);
		MojTestErrCheck(err);
	}
	guard.unlock();

	int* localVal = NULL;
	err = s_localVal.get(localVal);
	MojTestErrCheck(err);
	MojTestAssert(localVal && *localVal == 0);

	for (int i = 0; i < MojTestNumIterations; ++i) {
		++(*localVal);
		targs->m_atomicCounter.decrement();
		{
			MojThreadReadGuard readGuard(targs->m_rwlock);
			MojTestAssert(targs->m_counter > 0);
		}
		MojThreadReadGuard readGuard(targs->m_rwlock, false);
		if (!readGuard.tryLock())
			readGuard.lock();
		MojTestAssert(targs->m_counter > 0);
		readGuard.unlock();

		{
			MojThreadWriteGuard writeGuard(targs->m_rwlock);
			++(targs->m_counter);
		}

		MojThreadWriteGuard writeGuard(targs->m_rwlock, false);
		if (!writeGuard.tryLock())
			writeGuard.lock();
		targs->m_counter -= 2;
	}
	MojTestAssert(*localVal == MojTestNumIterations);

	return MojErrNone;
}

static MojErr MojThreadTestErrFn(void* arg)
{
	return MojErrNotFound;
}

MojThreadTest::MojThreadTest()
: MojTestCase(_T("MojThread"))
{
}

MojErr MojThreadTest::run()
{
	MojErr err = basicTest();
	MojTestErrCheck(err);
	err = errTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojThreadTest::basicTest()
{
	MojVector<MojThreadT> threads;
	MojThreadTestArgs args;
	for (int i = 0; i < MojTestNumThreads; ++i) {
		MojThreadT thread = MojInvalidThread;
		MojErr err = MojThreadCreate(thread, MojThreadTestFn, &args);
		MojTestErrCheck(err);
		MojTestAssert(thread != MojInvalidThread);
		err = threads.push(thread);
		MojTestErrCheck(err);
	}

	{
		MojThreadGuard guard(args.m_mutex);
		while (args.m_counter < (MojTestNumThreads * MojTestNumIterations)) {
			MojErr err = args.m_countCond.wait(args.m_mutex);
			MojErrCheck(err);
		}
		MojTestAssert(args.m_counter == (MojTestNumThreads * MojTestNumIterations));
		MojTestAssert(args.m_atomicCounter == (MojTestNumThreads * MojTestNumIterations));
		guard.unlock();
		MojErr err = MojSleep(MojMillisecs(500));
		MojTestErrCheck(err);
		guard.lock();
		args.m_wait = false;
		err = args.m_waitCond.broadcast();
		MojTestErrCheck(err);
	}

	for (MojVector<MojThreadT>::ConstIterator i = threads.begin();
		 i != threads.end();
		 ++i) {
		MojErr threadErr = MojErrNone;
		MojErr err = MojThreadJoin(*i, threadErr);
		MojTestErrCheck(err);
		MojTestErrCheck(threadErr);
	}
	MojTestAssert(args.m_counter == 0);
	MojTestAssert(args.m_atomicCounter == 0);

	return MojErrNone;
}

MojErr MojThreadTest::errTest()
{
	MojThreadT thread = MojInvalidThread;
	MojErr err = MojThreadCreate(thread, MojThreadTestErrFn, NULL);
	MojTestErrCheck(err);
	MojErr threadErr = MojErrNone;
	err = MojThreadJoin(thread, threadErr);
	MojTestErrCheck(err);
	MojTestErrExpected(threadErr, MojErrNotFound);

	return MojErrNone;
}
