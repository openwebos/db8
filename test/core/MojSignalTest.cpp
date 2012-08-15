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


#include "MojSignalTest.h"
#include "core/MojSignal.h"

typedef MojSignal<> TestSignal0;
typedef MojSignal<int> TestSignal1;
typedef MojSignal<int, const char*> TestSignal2;
typedef MojSignal<int, const char*, double> TestSignal3;

class TestHandler : public MojSignalHandler
{
public:
	TestHandler()
	: m_slot0(this, &TestHandler::handle0),
	  m_slot0Cancel(this, &TestHandler::handle0Cancel),
	  m_slot1(this, &TestHandler::handle1),
	  m_slot1Cancel(this, &TestHandler::handle1Cancel),
	  m_slot2(this, &TestHandler::handle2),
	  m_slot2Cancel(this, &TestHandler::handle2Cancel),
	  m_slot3(this, &TestHandler::handle3),
	  m_slot3Cancel(this, &TestHandler::handle3Cancel),
	  m_handle0Count(0),
	  m_handle1Count(0),
	  m_handle2Count(0),
	  m_handle3Count(0),
	  m_lastInt(0),
	  m_lastStr(NULL),
	  m_lastDouble(0)
	{
		++s_instances;
	}

	~TestHandler()
	{
		--s_instances;
	}

	MojErr handle0()
	{
		m_handle0Count++;
		return MojErrNone;
	}

	MojErr handle0Cancel()
	{
		handle0();
		m_slot0Cancel.cancel();
		return MojErrNone;
	}

	MojErr handle1(int val)
	{
		m_handle1Count++;
		m_lastInt = val;
		return MojErrNone;
	}

	MojErr handle1Cancel(int val)
	{
		handle1(val);
		m_slot1Cancel.cancel();
		return MojErrNone;
	}

	MojErr handle2(int val, const char* val2)
	{
		m_handle2Count++;
		m_lastInt = val;
		m_lastStr = val2;
		return MojErrNone;
	}

	MojErr handle2Cancel(int val, const char* val2)
	{
		handle2(val, val2);
		m_slot2Cancel.cancel();
		return MojErrNone;
	}

	MojErr handle3(int val, const char* val2, double val3)
	{
		m_handle3Count++;
		m_lastInt = val;
		m_lastStr = val2;
		m_lastDouble = val3;
		return MojErrNone;
	}

	MojErr handle3Cancel(int val, const char* val2, double val3)
	{
		handle3(val, val2, val3);
		m_slot3Cancel.cancel();
		return MojErrNone;
	}

	TestSignal0::Slot<TestHandler> m_slot0;
	TestSignal0::Slot<TestHandler> m_slot0Cancel;
	TestSignal1::Slot<TestHandler> m_slot1;
	TestSignal1::Slot<TestHandler> m_slot1Cancel;
	TestSignal2::Slot<TestHandler> m_slot2;
	TestSignal2::Slot<TestHandler> m_slot2Cancel;
	TestSignal3::Slot<TestHandler> m_slot3;
	TestSignal3::Slot<TestHandler> m_slot3Cancel;
	int m_handle0Count;
	int m_handle1Count;
	int m_handle2Count;
	int m_handle3Count;
	int m_lastInt;
	const char* m_lastStr;
	double m_lastDouble;
	static MojAtomicInt s_instances;
};

MojAtomicInt TestHandler::s_instances;

class TestSource : public MojSignalHandler
{
public:
	TestSource()
	: m_signal0(this),
   	  m_signal1(this),
	  m_signal2(this),
	  m_signal3(this),
	  m_numCancels(0)
	{
	}

	void add0Slot(TestSignal0::SlotRef slot)
	{
		m_signal0.connect(slot);
	}

	MojErr call0()
	{
		return m_signal0.call();
	}

	MojErr fire0()
	{
		return m_signal0.fire();
	}

	void add1Slot(TestSignal1::SlotRef slot)
	{
		m_signal1.connect(slot);
	}

	MojErr fire1(int val)
	{
		return m_signal1.fire(val);
	}

	void add2Slot(TestSignal2::SlotRef slot)
	{
		m_signal2.connect(slot);
	}

	MojErr fire2(int val, const char* val2)
	{
		return m_signal2.fire(val, val2);
	}

	void add3Slot(TestSignal3::SlotRef slot)
	{
		m_signal3.connect(slot);
	}

	MojErr fire3(int val, const char* val2, double val3)
	{
		return m_signal3.fire(val, val2, val3);
	}

	virtual MojErr handleCancel()
	{
		++m_numCancels;
		return MojErrNone;
	}

	TestSignal0 m_signal0;
	TestSignal1 m_signal1;
	TestSignal2 m_signal2;
	TestSignal3 m_signal3;
	int m_numCancels;
};

MojSignalTest::MojSignalTest()
: MojTestCase(_T("MojSignal"))
{
}

MojErr MojSignalTest::run()
{
	MojErr err = test0();
	MojTestErrCheck(err);
	err = test1();
	MojTestErrCheck(err);
	err = test2();
	MojTestErrCheck(err);
	err = test3();
	MojTestErrCheck(err);

	MojTestAssert(TestHandler::s_instances == 0);

	return MojErrNone;
}

MojErr MojSignalTest::test0()
{
	// one handler fire
	MojRefCountedPtr<TestSource> source(new TestSource);
	MojAllocCheck(source.get());
	MojRefCountedPtr<TestHandler> handler1(new TestHandler);
	MojAllocCheck(handler1.get());
	source->add0Slot(handler1->m_slot0);
	MojTestAssert(handler1->refCount() == 2);
	source->call0();
	MojTestAssert(handler1->m_handle0Count == 1);
	MojTestAssert(handler1->refCount() == 2);
	source->fire0();
	MojTestAssert(handler1->m_handle0Count == 2);
	MojTestAssert(handler1->refCount() == 1);
	source->fire0();
	MojTestAssert(handler1->m_handle0Count == 2);
	source->call0();
	MojTestAssert(handler1->m_handle0Count == 2);
	// two handlers
	MojRefCountedPtr<TestHandler> handler2(new TestHandler);
	MojAllocCheck(handler2.get());
	source->add0Slot(handler1->m_slot0);
	source->add0Slot(handler2->m_slot0);
	source->call0();
	MojTestAssert(handler1->m_handle0Count == 3);
	MojTestAssert(handler2->m_handle0Count == 1);
	source->fire0();
	MojTestAssert(handler1->m_handle0Count == 4);
	MojTestAssert(handler2->m_handle0Count == 2);
	// cancel
	source->add0Slot(handler1->m_slot0);
	source->add0Slot(handler2->m_slot0);
	MojTestAssert(source->m_numCancels == 0);
	handler1->m_slot0.cancel();
	source->fire0();
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_handle0Count == 4);
	MojTestAssert(handler2->m_handle0Count == 3);
	// cancel from callback
	source->add0Slot(handler1->m_slot0Cancel);
	source->add0Slot(handler2->m_slot0);
	source->fire0();
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_handle0Count == 5);
	MojTestAssert(handler2->m_handle0Count == 4);
	source->add0Slot(handler2->m_slot0);
	source->fire0();
	MojTestAssert(handler1->m_handle0Count == 5);
	MojTestAssert(handler2->m_handle0Count == 5);
	// cancel from callback with call
	source->add0Slot(handler1->m_slot0Cancel);
	source->add0Slot(handler2->m_slot0);
	source->call0();
	MojTestAssert(source->m_numCancels == 2);
	MojTestAssert(handler1->m_handle0Count == 6);
	MojTestAssert(handler2->m_handle0Count == 6);
	source->call0();
	MojTestAssert(handler1->m_handle0Count == 6);
	MojTestAssert(handler2->m_handle0Count == 7);

	return MojErrNone;
}

MojErr MojSignalTest::test1()
{
	// one handler fire
	MojRefCountedPtr<TestSource> source(new TestSource);
	MojAllocCheck(source.get());
	MojRefCountedPtr<TestHandler> handler1(new TestHandler);
	MojAllocCheck(handler1.get());
	source->add1Slot(handler1->m_slot1);
	MojTestAssert(handler1->refCount() == 2);
	source->fire1(200);
	MojTestAssert(handler1->m_handle1Count == 1);
	MojTestAssert(handler1->m_lastInt == 200);
	MojTestAssert(handler1->refCount() == 1);
	source->fire1(200);
	MojTestAssert(handler1->m_handle1Count == 1);
	// two handlers
	MojRefCountedPtr<TestHandler> handler2(new TestHandler);
	MojAllocCheck(handler2.get());
	source->add1Slot(handler1->m_slot1);
	source->add1Slot(handler2->m_slot1);
	source->fire1(201);
	MojTestAssert(handler1->m_lastInt == 201);
	MojTestAssert(handler2->m_lastInt == 201);
	// cancel
	source->add1Slot(handler1->m_slot1);
	source->add1Slot(handler2->m_slot1);
	MojTestAssert(source->m_numCancels == 0);
	handler1->m_slot1.cancel();
	source->fire1(202);
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 201);
	MojTestAssert(handler2->m_lastInt == 202);
	// cancel from callback
	source->add1Slot(handler1->m_slot1Cancel);
	source->add1Slot(handler2->m_slot1);
	source->fire1(203);
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 203);
	MojTestAssert(handler2->m_lastInt == 203);
	source->add1Slot(handler2->m_slot1);
	source->fire1(204);
	MojTestAssert(handler1->m_lastInt == 203);
	MojTestAssert(handler2->m_lastInt == 204);

	return MojErrNone;
}

MojErr MojSignalTest::test2()
{
	// one handler fire
	MojRefCountedPtr<TestSource> source(new TestSource);
	MojAllocCheck(source.get());
	MojRefCountedPtr<TestHandler> handler1(new TestHandler);
	MojAllocCheck(handler1.get());
	source->add2Slot(handler1->m_slot2);
	MojTestAssert(handler1->refCount() == 2);
	source->fire2(200, "hello");
	MojTestAssert(handler1->m_handle2Count == 1);
	MojTestAssert(handler1->m_lastInt == 200);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello"));
	MojTestAssert(handler1->refCount() == 1);
	source->fire2(200, "hello");
	MojTestAssert(handler1->m_handle2Count == 1);
	// two handlers
	MojRefCountedPtr<TestHandler> handler2(new TestHandler);
	MojAllocCheck(handler2.get());
	source->add2Slot(handler1->m_slot2);
	source->add2Slot(handler2->m_slot2);
	source->fire2(201, "hello2");
	MojTestAssert(handler1->m_lastInt == 201);
	MojTestAssert(handler2->m_lastInt == 201);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello2"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello2"));
	// cancel
	source->add2Slot(handler1->m_slot2);
	source->add2Slot(handler2->m_slot2);
	MojTestAssert(source->m_numCancels == 0);
	handler1->m_slot2.cancel();
	source->fire2(202, "hello3");
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 201);
	MojTestAssert(handler2->m_lastInt == 202);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello2"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello3"));
	// cancel from callback
	source->add2Slot(handler2->m_slot2);
	source->add2Slot(handler1->m_slot2Cancel);
	source->fire2(203, "hello4");
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 203);
	MojTestAssert(handler2->m_lastInt == 203);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello4"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello4"));
	source->add2Slot(handler2->m_slot2);
	source->fire2(204, "hello5");
	MojTestAssert(handler1->m_lastInt == 203);
	MojTestAssert(handler2->m_lastInt == 204);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello4"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello5"));

	return MojErrNone;
}

MojErr MojSignalTest::test3()
{
	// one handler fire
	MojRefCountedPtr<TestSource> source(new TestSource);
	MojAllocCheck(source.get());
	MojRefCountedPtr<TestHandler> handler1(new TestHandler);
	MojAllocCheck(handler1.get());
	source->add3Slot(handler1->m_slot3);
	MojTestAssert(handler1->refCount() == 2);
	source->fire3(200, "hello", 1.0);
	MojTestAssert(handler1->m_handle3Count == 1);
	MojTestAssert(handler1->m_lastInt == 200 && handler1->m_lastDouble == 1.0);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello"));
	MojTestAssert(handler1->refCount() == 1);
	source->fire3(200, "hello", 1.0);
	MojTestAssert(handler1->m_handle3Count == 1);
	// two handlers
	MojRefCountedPtr<TestHandler> handler2(new TestHandler);
	MojAllocCheck(handler2.get());
	source->add3Slot(handler1->m_slot3);
	source->add3Slot(handler2->m_slot3);
	source->fire3(201, "hello2", 2.0);
	MojTestAssert(handler1->m_lastInt == 201 && handler1->m_lastDouble == 2.0);
	MojTestAssert(handler2->m_lastInt == 201 && handler2->m_lastDouble == 2.0);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello2"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello2"));
	// cancel
	source->add3Slot(handler1->m_slot3);
	source->add3Slot(handler2->m_slot3);
	MojTestAssert(source->m_numCancels == 0);
	handler1->m_slot3.cancel();
	source->fire3(202, "hello3", 3.0);
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 201 && handler1->m_lastDouble == 2.0);
	MojTestAssert(handler2->m_lastInt == 202 && handler2->m_lastDouble == 3.0);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello2"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello3"));
	// cancel from callback
	source->add3Slot(handler1->m_slot3Cancel);
	source->add3Slot(handler2->m_slot3);
	source->fire3(203, "hello4", 4.0);
	MojTestAssert(source->m_numCancels == 1);
	MojTestAssert(handler1->m_lastInt == 203 && handler1->m_lastDouble == 4.0);
	MojTestAssert(handler2->m_lastInt == 203 && handler2->m_lastDouble == 4.0);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello4"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello4"));
	source->add3Slot(handler2->m_slot3);
	source->fire3(204, "hello5", 5.0);
	MojTestAssert(handler1->m_lastInt == 203 && handler1->m_lastDouble == 4.0);
	MojTestAssert(handler2->m_lastInt == 204 && handler2->m_lastDouble == 5.0);
	MojTestAssert(!MojStrCmp(handler1->m_lastStr, "hello4"));
	MojTestAssert(!MojStrCmp(handler2->m_lastStr, "hello5"));

	return MojErrNone;
}
