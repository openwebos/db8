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


#include "MojDbStressTest.h"
#include "core/MojGmainReactor.h"
#include "core/MojLogEngine.h"
#include "luna/MojLunaService.h"
#include "db/MojDbServiceClient.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbServiceHandler.h"

int main(int argc, char** argv)
{
	MojDbStressTestRunner runner;
	return runner.main(argc, argv);
}

void MojDbStressTestRunner::runTests()
{
	test(MojDbStressTest());
}

const MojChar* const MojDbStressTest::KindTestKindId = _T("KindTest%d:1");
const MojChar* const MojDbStressTest::KindTestKindGoodJson =
	_T("{\"id\":\"%s\",")
	_T("\"owner\":\"%s\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"rev\",\"props\":[{\"name\":\"_rev\"}]},")
		_T("{\"name\":\"foo1\",\"props\":[{\"name\":\"foo1\"}]},")
		_T("{\"name\":\"foo2\",\"props\":[{\"name\":\"foo2\"}]},")
		_T("{\"name\":\"foo3\",\"props\":[{\"name\":\"foo3\"}]},")
		_T("{\"name\":\"foo4\",\"props\":[{\"name\":\"foo4\"}]},")
		_T("{\"name\":\"foo5\",\"props\":[{\"name\":\"foo5\"}]}")
	_T("]}");
const MojChar* const MojDbStressTest::KindTestKindBadJson =
	_T("{\"id\":\"%s\",")
	_T("\"owner\":\"%s\",")
	_T("\"indexes\":[")
		_T("{},")
	_T("]}");
const MojChar* const MojDbStressTest::FullTestKindId = _T("FullTest%d:1");
const MojChar* const MojDbStressTest::FullTestKindJson =
	_T("{\"id\":\"%s\",")
	_T("\"owner\":\"%s\",")
	_T("\"indexes\":[")
		_T("{\"name\":\"rev\",\"props\":[{\"name\":\"_rev\"}]},")
		_T("{\"name\":\"foo1\",\"props\":[{\"name\":\"foo1\"}]},")
		_T("{\"name\":\"foo2\",\"props\":[{\"name\":\"foo2\"}]},")
		_T("{\"name\":\"foo3\",\"props\":[{\"name\":\"foo3\"}]},")
		_T("{\"name\":\"foo4\",\"props\":[{\"name\":\"foo4\"}]},")
		_T("{\"name\":\"foo5\",\"props\":[{\"name\":\"foo5\"}]}")
	_T("]}");
const MojChar* const MojDbStressTest::ConcurrentTestKindId = _T("ConcurrentTest%d:1");
const MojChar* const MojDbStressTest::ConcurrentTestKindJson =
	_T("{\"id\":\"%s\",")
	_T("\"owner\":\"%s\"}");

MojDbStressTest::MojDbStressTest()
: MojTestCase(_T("MojDbStressTest")),
  m_numTests(0),
  m_numIterationsRemaining(NumIterations)
{
}

MojErr MojDbStressTest::run()
{
	MojLogEngine::instance()->reset(MojLogger::LevelError);

	MojErr err = init();
	MojTestErrCheck(err);
	err = runTests();
	MojTestErrCheck(err);
	err = m_reactor->run();
	MojTestErrCheck(err);

	MojTestAssert(m_numTests == 0);

	return MojErrNone;
}

MojErr MojDbStressTest::runTests()
{
        m_prevTests = m_tests;
	m_tests.clear();

	for (MojUInt32 i = 0; i < NumKindTests; ++i) {
		MojRefCountedPtr<KindTest> test(new KindTest(this, m_numIterationsRemaining * NumKindTests + i));
		MojAllocCheck(test.get());
		MojErr err = runTest(test.get());
		MojTestErrCheck(err);
	}
	for (MojUInt32 i = 0; i < NumFullTests; ++i) {
		MojRefCountedPtr<FullTest> test(new FullTest(this, m_numIterationsRemaining * NumFullTests + i));
		MojAllocCheck(test.get());
		MojErr err = runTest(test.get());
		MojTestErrCheck(err);
	}
	for (MojUInt32 i = 0; i < NumConcurrentTests; ++i) {
		MojRefCountedPtr<ConcurrentTest> test(new ConcurrentTest(this, m_numIterationsRemaining * NumConcurrentTests + i));
		MojAllocCheck(test.get());
		MojErr err = runTest(test.get());
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStressTest::runTest(TestHandler* test)
{
	MojTestAssert(test);

	MojPrintF(_T("iteration %d\n"), NumIterations - m_numIterationsRemaining);
	MojErr err = test->init();
	MojTestErrCheck(err);
	err = test->run();
	MojTestErrCheck(err);
	err = m_tests.push(test);
	MojTestErrCheck(err);
	m_numTests++;

	return MojErrNone;
}

MojErr MojDbStressTest::testDone(TestHandler* test)
{
	MojTestAssert(test);
	MojTestAssert(m_numTests > 0);
	MojPrintF(_T("testDone\n"));

	if (--m_numTests == 0) {
		if (--m_numIterationsRemaining == 0) {
			MojErr err = m_reactor->stop();
			MojTestErrCheck(err);
		} else {
			MojErr err = runTests();
			MojTestErrCheck(err);
		}
	}
	return MojErrNone;
}

void MojDbStressTest::cleanup()
{

}

void MojDbStressTest::failed()
{
	if (m_reactor.get()) {
		(void) m_reactor->stop();
	}
}

MojErr MojDbStressTest::init()
{
	MojAutoPtr<MojGmainReactor> reactor(new MojGmainReactor);
	MojAllocCheck(reactor.get());

	MojErr err = reactor->init();
	MojTestErrCheck(err);

	m_reactor = reactor;

	return MojErrNone;
}

MojDbStressTest::TestHandler::TestHandler(MojDbStressTest* test, const MojChar* name, MojUInt32 kindNum)
: m_test(test),
  m_name(name),
  m_kindNum(kindNum)
{
	MojAssert(test);
}

MojErr MojDbStressTest::TestHandler::init()
{
	m_service.reset(new MojLunaService);
	MojAllocCheck(m_service.get());
	m_client.reset(new MojDbServiceClient(m_service.get()));
	MojAllocCheck(m_client.get());

	MojErr err = m_owner.format(_T("%s%d"), m_name, m_kindNum);
	MojTestErrCheck(err);
	err = m_service->open(m_owner);
	MojTestErrCheck(err);
	err = m_service->attach(m_test->m_reactor->impl());
	MojTestErrCheck(err);

	return MojErrNone;
}

MojDbStressTest::KindTest::KindTest(MojDbStressTest* test, MojUInt32 kindNum)
: TestHandler(test, _T("KindTest"), kindNum),
  m_putKindBad1Slot(this, &KindTest::handlePutKindBad1),
  m_putKindGoodSlot(this, &KindTest::handlePutKindGood),
  m_putKindBad2Slot(this, &KindTest::handlePutKindBad2),
  m_delKindSlot(this, &KindTest::handleDelKind),
  m_numPutsRemaining(NumPutKinds)
{
}

MojErr MojDbStressTest::KindTest::run()
{
	MojErr err = m_kindId.format(KindTestKindId, m_kindNum);
	MojTestErrCheck(err);
	err = putKindBad1();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::putKindBad1()
{
	MojPrintF(_T("kindtest %d - putKindBad1\n"), m_kindNum);
	MojString kindJson;
	MojErr err = kindJson.format(KindTestKindBadJson, m_kindId.data(), m_owner.data());
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(kindJson);
	MojTestErrCheck(err);
	err = m_client->putKind(m_putKindBad1Slot, kind);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::handlePutKindBad1(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("kindtest %d - handlePutKindBad1\n"), m_kindNum);
	MojTestErrExpected(errCode, MojErrRequiredPropNotFound);
	MojErr err = putKindGood();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::putKindGood()
{
	MojPrintF(_T("kindtest %d - putKindGood\n"), m_kindNum);
	MojString kindJson;
	MojErr err = kindJson.format(KindTestKindGoodJson, m_kindId.data(), m_owner.data());
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(kindJson);
	MojTestErrCheck(err);
	err = m_client->putKind(m_putKindGoodSlot, kind);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::handlePutKindGood(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("kindtest %d - handlePutGood\n"), m_kindNum);
	MojTestErrCheck(errCode);
	MojErr err = putKindBad2();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::putKindBad2()
{
	MojPrintF(_T("kindtest %d - putKindBad2\n"), m_kindNum);
	MojString kindJson;
	MojErr err = kindJson.format(KindTestKindBadJson, m_kindId.data(), m_owner.data());
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(kindJson);
	MojTestErrCheck(err);
	err = m_client->putKind(m_putKindBad2Slot, kind);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::handlePutKindBad2(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("kindtest %d - handlePutKindBad2\n"), m_kindNum);
	MojTestErrExpected(errCode, MojErrRequiredPropNotFound);
	MojErr err = delKind();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::delKind()
{
	MojPrintF(_T("kindtest %d - delKind\n"), m_kindNum);
	MojErr err = m_client->delKind(m_delKindSlot, m_kindId);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::KindTest::handleDelKind(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("kindtest %d - handleDelKind\n"), m_kindNum);
	MojTestErrCheck(errCode);
	if (--m_numPutsRemaining > 0) {
		MojErr err = putKindBad1();
		MojTestErrCheck(err);
	} else {
		MojErr err = m_test->testDone(this);
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojDbStressTest::FullTest::FullTest(MojDbStressTest* test, MojUInt32 kindNum)
: TestHandler(test, _T("FullTest"), kindNum),
  m_putKindSlot(this, &FullTest::handlePutKind),
  m_watchSlot(this, &FullTest::handleWatch),
  m_putSlot(this, &FullTest::handlePut),
  m_mergeSlot(this, &FullTest::handleMerge),
  m_findSlot(this, &FullTest::handleFind),
  m_findDescSlot(this, &FullTest::handleFindDesc),
  m_delKindSlot(this, &FullTest::handleDelKind),
  m_numBatchesRemaining(NumBatches)
{
}

MojErr MojDbStressTest::FullTest::run()
{
	MojErr err = putKind();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::putKind()
{
	MojPrintF(_T("fulltest %d - putKind\n"), m_kindNum);
	MojErr err = m_kindId.format(FullTestKindId, m_kindNum);
	MojTestErrCheck(err);
	MojString kindJson;
	err = kindJson.format(FullTestKindJson, m_kindId.data(), m_owner.data());
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(kindJson);
	MojTestErrCheck(err);
	err = m_client->putKind(m_putKindSlot, kind);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handlePutKind(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handlePutKind\n"), m_kindNum);
	MojTestErrCheck(errCode);
	MojErr err = watch();
	//MojErr err = put();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::watch()
{
	MojPrintF(_T("fulltest %d - watch\n"), m_kindNum);
	MojDbQuery query;
	MojErr err = query.from(m_kindId);
	MojTestErrCheck(err);
	if (!m_lastRev.undefined()) {
		err = query.where(_T("_rev"), MojDbQuery::OpGreaterThan, m_lastRev);
		MojTestErrCheck(err);
	}
	err = m_client->watch(m_watchSlot, query);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handleWatch(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handleWatch\n"), m_kindNum);
	MojTestErrCheck(errCode);
	bool fired = false;
	result.get(MojDbServiceDefs::FiredKey, fired);
	if (fired) {
		m_watchSlot.cancel();
		MojErr err = find();
		MojTestErrCheck(err);
	} else {
		MojErr err = put();
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::put()
{
	MojPrintF(_T("fulltest %d - put %d\n"), m_kindNum, NumBatches - m_numBatchesRemaining);
	// create objects for batch put
	MojVector<MojObject> batch;
	for (MojUInt32 i = 0; i < BatchSize; ++i) {
		MojObject obj;
		MojErr err = obj.putString(_T("_kind"), m_kindId);
		MojTestErrCheck(err);
		for (MojUInt32 j = 0; j < 10; ++j) {
			MojString foo;
			err = foo.format(_T("foo%d"), j);
			MojTestErrCheck(err);
			if (i & 1) {
				MojString bar;
				err = bar.format(_T("bar%d"), i * j);
				MojTestErrCheck(err);
				err = obj.putString(foo, bar);
				MojTestErrCheck(err);
			} else {
				err = obj.putInt(foo, i);
				MojTestErrCheck(err);
			}
		}
		err = batch.push(obj);
		MojTestErrCheck(err);
	}

	// put objects
	MojErr err = m_client->put(m_putSlot, batch.begin(), batch.end());
	MojTestErrCheck(err);
	m_numBatchesRemaining--;

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handlePut(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handlePut\n"), m_kindNum);
	MojTestErrCheck(errCode);
	//MojErr err = find();
	//MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::merge()
{
	MojPrintF(_T("fulltest %d - merge\n"), m_kindNum);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handleMerge(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handleMerge\n"), m_kindNum);
	MojTestErrCheck(errCode);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::find(const MojObject& pageKey)
{
	MojPrintF(_T("fulltest %d - find\n"), m_kindNum);
	MojDbQuery query;
	MojErr err = query.from(m_kindId);
	MojTestErrCheck(err);
	if (!pageKey.undefined()) {
		MojDbQuery::Page page;
		err = page.fromObject(pageKey);
		MojTestErrCheck(err);
		query.page(page);
	}
	query.desc(true);
	err = m_client->find(m_findDescSlot, query);
	MojTestErrCheck(err);
	query.desc(false);
	err = m_client->find(m_findSlot, query);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handleFind(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handleFind\n"), m_kindNum);
	MojTestErrCheck(errCode);

	MojObject results;
	MojErr err = result.getRequired(MojDbServiceDefs::ResultsKey, results);
	MojTestErrCheck(err);
	MojTestAssert(results.size() <= MojDbServiceHandler::MaxQueryLimit);
	for (MojObject::ConstArrayIterator i = results.arrayBegin(); i != results.arrayEnd(); ++i) {
		MojObject rev;
		err = i->getRequired(_T("_rev"), rev);
		MojTestErrCheck(err);
		if (rev > m_lastRev)
			m_lastRev = rev;
	}

	MojObject page;
	if (result.get(MojDbServiceDefs::NextKey, page)) {
		m_findSlot.cancel();
		err = find(page);
		MojTestErrCheck(err);
	} else if (m_numBatchesRemaining > 0) {
		err = watch();
		//err = put();
		MojTestErrCheck(err);
	} else {
		err = delKind();
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handleFindDesc(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handleFindDesc\n"), m_kindNum);
	MojTestErrCheck(errCode);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::delKind()
{
	MojPrintF(_T("fulltest %d - delKind\n"), m_kindNum);
	MojErr err = m_client->delKind(m_delKindSlot, m_kindId);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbStressTest::FullTest::handleDelKind(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("fulltest %d - handleDelKind\n"), m_kindNum);
	MojTestErrCheck(errCode);
	MojErr err = m_test->testDone(this);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojDbStressTest::ConcurrentTest::ConcurrentTest(MojDbStressTest* test, MojUInt32 kindNum)
: TestHandler(test, _T("ConcurrentTest"), kindNum),
  m_putKindSlot(this, &ConcurrentTest::handlePutKind),
  m_numPutsRemaining(NumConcurrentTestPuts),
  m_kindPut(false)
{
}

MojErr MojDbStressTest::ConcurrentTest::run()
{
	MojPrintF(_T("concurrenttest %d - putKind\n"), m_kindNum);
	MojString kindId;
	MojErr err = kindId.format(ConcurrentTestKindId, m_kindNum);
	MojTestErrCheck(err);
	MojString kindJson;
	err = kindJson.format(ConcurrentTestKindJson, kindId.data(), m_owner.data());
	MojTestErrCheck(err);

	// put kind
	MojObject kind;
	err = kind.fromJson(kindJson);
	MojTestErrCheck(err);
	err = m_client->putKind(m_putKindSlot, kind);
	MojTestErrCheck(err);

	// put objects
	for (MojUInt32 i = 0; i < m_numPutsRemaining; ++i) {
		MojObject obj;
		MojErr err = obj.putString(_T("_kind"), kindId);
		MojTestErrCheck(err);
		for (MojUInt32 j = 0; j < 10; ++j) {
			MojString foo;
			err = foo.format(_T("foo%d"), j);
			MojTestErrCheck(err);
			err = obj.putInt(foo, i);
			MojTestErrCheck(err);
		}
		MojRefCountedPtr<ResponseHandler> handler(new ResponseHandler(this));
		MojAllocCheck(handler.get());
		err = m_client->put(handler->slot(), obj);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStressTest::ConcurrentTest::done()
{
	MojPrintF(_T("concurrenttest - done (%d remaining) \n"), m_numPutsRemaining);
	if (--m_numPutsRemaining == 0 && m_kindPut) {
		MojErr err = m_test->testDone(this);
		MojTestErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbStressTest::ConcurrentTest::handlePutKind(MojObject& result, MojErr errCode)
{
	MojPrintF(_T("concurrenttest - handlePutKind\n"));
	MojTestErrCheck(errCode);
	m_kindPut = true;

	return MojErrNone;
}
