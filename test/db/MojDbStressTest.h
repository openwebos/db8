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


#ifndef MOJDBSTRESSTEST_H_
#define MOJDBSTRESSTEST_H_

#include "db/MojDbDefs.h"
#include "db/MojDbClient.h"
#include "luna/MojLunaService.h"
#include "core/MojTestRunner.h"

class MojDbStressTestRunner : public MojTestRunner
{
private:
	void runTests();
};

class MojDbStressTest : public MojTestCase
{
public:
	MojDbStressTest();

	virtual MojErr run();
	virtual void failed();
	virtual void cleanup();

private:
	static const MojUInt32 BatchSize = 50;
	static const MojUInt32 NumObjects = 4000;
	static const MojUInt32 NumIterations = 1;
	static const MojUInt32 NumFullTests = 0; //10;
	static const MojUInt32 NumKindTests = 0;
	static const MojUInt32 NumConcurrentTests = 1;
	static const MojUInt32 NumConcurrentTestPuts = 50000;
	static const MojUInt32 NumPutKinds = 50;
	static const MojUInt32 NumBatches = 0;//(NumObjects / NumFullTests) / BatchSize;
	static const MojUInt32 NumConcurrentPuts = 3;
	static const MojChar* const KindTestKindId;
	static const MojChar* const KindTestKindGoodJson;
	static const MojChar* const KindTestKindBadJson;
	static const MojChar* const FullTestKindId;
	static const MojChar* const FullTestKindJson;
	static const MojChar* const ConcurrentTestKindId;
	static const MojChar* const ConcurrentTestKindJson;

	class TestHandler : public MojSignalHandler
	{
	public:
		TestHandler(MojDbStressTest* test, const MojChar* name, MojUInt32 kindNum);

		virtual MojErr init();
		virtual MojErr run() = 0;

	protected:
		MojDbStressTest* m_test;
		MojAutoPtr<MojLunaService> m_service;
		MojAutoPtr<MojDbClient> m_client;
		MojString m_kindId;
		MojString m_owner;
		const MojChar* m_name;
		MojUInt32 m_kindNum;
	};

	class KindTest : public TestHandler
	{
	public:
		KindTest(MojDbStressTest* test, MojUInt32 kindNum);

		virtual MojErr run();

	private:
		MojErr putKindBad1();
		MojErr handlePutKindBad1(MojObject& result, MojErr errCode);

		MojErr putKindGood();
		MojErr handlePutKindGood(MojObject& result, MojErr errCode);

		MojErr putKindBad2();
		MojErr handlePutKindBad2(MojObject& result, MojErr errCode);

		MojErr delKind();
		MojErr handleDelKind(MojObject& result, MojErr errCode);

		MojDbClient::Signal::Slot<KindTest> m_putKindBad1Slot;
		MojDbClient::Signal::Slot<KindTest> m_putKindGoodSlot;
		MojDbClient::Signal::Slot<KindTest> m_putKindBad2Slot;
		MojDbClient::Signal::Slot<KindTest> m_delKindSlot;
		MojUInt32 m_numPutsRemaining;
	};

	class FullTest : public TestHandler
	{
	public:
		FullTest(MojDbStressTest* test, MojUInt32 kindNum);

		virtual MojErr run();

	private:
		MojErr putKind();
		MojErr handlePutKind(MojObject& result, MojErr errCode);

		MojErr watch();
		MojErr handleWatch(MojObject& result, MojErr errCode);

		MojErr put();
		MojErr handlePut(MojObject& result, MojErr errCode);

		MojErr merge();
		MojErr handleMerge(MojObject& result, MojErr errCode);

		MojErr find(const MojObject& page = MojObject::Undefined);
		MojErr handleFind(MojObject& result, MojErr errCode);
		MojErr handleFindDesc(MojObject& result, MojErr errCode);

		MojErr delKind();
		MojErr handleDelKind(MojObject& result, MojErr errCode);

		MojDbClient::Signal::Slot<FullTest> m_putKindSlot;
		MojDbClient::Signal::Slot<FullTest> m_watchSlot;
		MojDbClient::Signal::Slot<FullTest> m_putSlot;
		MojDbClient::Signal::Slot<FullTest> m_mergeSlot;
		MojDbClient::Signal::Slot<FullTest> m_findSlot;
		MojDbClient::Signal::Slot<FullTest> m_findDescSlot;
		MojDbClient::Signal::Slot<FullTest> m_delKindSlot;
		MojVector<MojDbClient::Signal::Slot<FullTest> > m_putSlots;
		MojObject m_lastRev;
		MojUInt32 m_numBatchesRemaining;
	};

	class ConcurrentTest : public TestHandler
	{
	public:
		ConcurrentTest(MojDbStressTest* test, MojUInt32 kindNum);

		virtual MojErr run();

	private:
		class ResponseHandler : public MojSignalHandler
		{
		public:
			ResponseHandler(ConcurrentTest* test)
			: m_test(test), m_slot(this, &ResponseHandler::handleResponse) {}

			MojDbClient::Signal::SlotRef slot() { return m_slot; }

		private:
			MojErr handleResponse(MojObject& result, MojErr errCode) { return m_test->done(); }

			ConcurrentTest* m_test;
			MojDbClient::Signal::Slot<ResponseHandler> m_slot;
		};

		MojErr handlePutKind(MojObject& result, MojErr errCode);
		MojErr done();

		MojDbClient::Signal::Slot<ConcurrentTest> m_putKindSlot;
		MojUInt32 m_numPutsRemaining;
		bool m_kindPut;
	};

	typedef MojVector<MojRefCountedPtr<TestHandler> > TestVec;

	MojErr init();
	MojErr runTests();
	MojErr runTest(TestHandler* test);
	MojErr testDone(TestHandler* test);

	MojAutoPtr<MojGmainReactor> m_reactor;
	TestVec m_tests;
	TestVec m_prevTests;
	MojUInt32 m_numTests;
	MojUInt32 m_numIterationsRemaining;
};

#endif /* MOJDBSTRESSTEST_H_ */
