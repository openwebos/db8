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


#include "MojDbClientTest.h"
#include "db/MojDbServiceClient.h"
#include "luna/MojLunaService.h"
#include "core/MojSocketService.h"
#include "core/MojEpollReactor.h"
#include "core/MojGmainReactor.h"
#include "db/MojDbServiceHandler.h"

int main(int argc, char** argv)
{
	MojDbClientTestRunner runner;
	return runner.main(argc, argv);
}

class MojDbClientTestResultHandler : public MojSignalHandler
{
public:
	MojDbClientTestResultHandler()
	: m_dbErr(MojErrNone),
	  m_callbackInvoked (false),
	  m_slot(this, &MojDbClientTestResultHandler::handleResult)
	{
	}

	virtual MojErr handleResult(MojObject& result, MojErr errCode)
	{
		m_callbackInvoked = true;
		m_result = result;
		m_dbErr = errCode;
		if (errCode != MojErrNone) {
			bool found = false;
			MojErr err = result.get(MojServiceMessage::ErrorTextKey, m_errTxt, found);
			MojTestErrCheck(err);
			MojTestAssert(found);
		}
		return MojErrNone;
	}

	void reset()
	{
		m_dbErr = MojErrNone;
		m_callbackInvoked = false;
		m_errTxt.clear();
		m_result.clear();
	}

	MojErr wait(MojService* service)
	{
		while (!m_callbackInvoked) {
			MojErr err = service->dispatch();
			MojErrCheck(err);
		}
		return MojErrNone;
	}

	MojErr m_dbErr;
	MojString m_errTxt;
	MojObject m_result;
	bool m_callbackInvoked;
	MojDbClient::Signal::Slot<MojDbClientTestResultHandler> m_slot;
};

class MojDbClientTestWatcher : public MojDbClientTestResultHandler
{
public:
	MojDbClientTestWatcher()
	: m_fired(false)
	{
	}

	virtual MojErr handleResult(MojObject& result, MojErr errCode)
	{
		if (!m_callbackInvoked) {
			// results callback
			MojErr err = MojDbClientTestResultHandler::handleResult(result, errCode);
			MojErrCheck(err);
			// check for an optional fired field
			bool fired = false;
			bool found = result.get(_T("fired"), fired);
			if (found)
				m_fired = fired;
			return MojErrNone;
		} else {
			// watch fire
			MojTestAssert(!m_fired);
			bool fired = false;
			MojErr err = result.getRequired(_T("fired"), fired);
			MojTestErrCheck(err);
			MojTestAssert(fired);
			m_fired = true;
			return MojErrNone;
		}
	}

	bool m_fired;
};

void MojDbClientTestRunner::runTests()
{
	test(MojDbClientTest());
}

MojDbClientTest::MojDbClientTest()
: MojTestCase(_T("MojDbClientTest"))
{
}

MojErr MojDbClientTest::run()
{
	MojErr err = init();
	MojTestErrCheck(err);

	err = registerType();
	MojTestErrCheck(err);

	err = testPut();
	MojTestErrCheck(err);

	err = testMultiPut();
	MojTestErrCheck(err);

	err = testGet();
	MojTestErrCheck(err);

	err = testMultiGet();
	MojTestErrCheck(err);

	err = testDel();
	MojTestErrCheck(err);

	err = testMultiDel();
	MojTestErrCheck(err);

	err = testQueryDel();
	MojTestErrCheck(err);

	err = testMerge();
	MojTestErrCheck(err);

	err = testMultiMerge();
	MojTestErrCheck(err);

	err = testQueryMerge();
	MojTestErrCheck(err);

	err = testBasicFind();
	MojTestErrCheck(err);

	err = testBasicSearch();
	MojTestErrCheck(err);

	err = testUpdatedFind();
	MojTestErrCheck(err);

	err = testWatchFind();
	MojTestErrCheck(err);

	err = testWatch();
	MojTestErrCheck(err);

	err = testErrorHandling(false);
	MojTestErrCheck(err);

	err = testErrorHandling(true);
	MojTestErrCheck(err);

	err = testHdlrCancel();
	MojTestErrCheck(err);

	err = testCompact();
	MojTestErrCheck(err);

	//err = testPurge();
	//MojTestErrCheck(err);

	//err = testPurgeStatus();
	//MojTestErrCheck(err);

	err = testDeleteKind();
	MojTestErrCheck(err);

	err = testBatch();
	MojTestErrCheck(err);

	err = testPermissions();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClientTest::init()
{
	if (!m_dbClient.get()) {
		MojAutoPtr<MojGmainReactor> reactor(new MojGmainReactor);
		MojTestAssert(reactor.get());
		MojErr err = reactor->init();
		MojTestErrCheck(err);

		MojAutoPtr<MojLunaService> svc(new MojLunaService);
		MojTestAssert(svc.get());
		err = svc->open(_T("mojodbclient-test"));
		MojTestErrCheck(err);
		err = svc->attach(reactor->impl());
		MojTestErrCheck(err);

		m_reactor = reactor;
		m_service = svc;
		m_dbClient.reset(new MojDbServiceClient(m_service.get()));
		MojTestAssert(m_dbClient.get());
	}
	return MojErrNone;
}

void MojDbClientTest::cleanup()
{
	MojErr err = init();
	MojAssert(!err);

	err = delTestData();
	MojAssert(err == MojErrNone);
}

MojErr MojDbClientTest::staleTestData(bool& resultOut)
{
	// check if kind registered.  if so, we didn't shut down
	// cleanly on last invocation of test
	MojString kindId;
	MojErr err = kindId.assign(_T("_kinds/LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojObject kindObj(kindId);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	MojObject idObj(kindId);
	err = m_dbClient->get(handler->m_slot, idObj);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));
	MojObject::ConstArrayIterator i = results.arrayBegin();
	resultOut = false;
	if (i != results.arrayEnd()) {
		bool deleted;
		if (!i->get(_T("_del"), deleted) || !deleted)
			resultOut = true; // kind object exists
	}

	return MojErrNone;
}

MojErr MojDbClientTest::delTestData()
{
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());
	MojErr err = m_dbClient->delKind(handler->m_slot, _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);
	MojTestAssert(handler->m_dbErr == MojErrDbKindNotRegistered || handler->m_dbErr == MojErrNone);

	return MojErrNone;
}

MojErr MojDbClientTest::registerType()
{
	// register type
	MojErr err = writeTestObj(_T("{\"id\":\"LunaDbClientTest:1\",\"owner\":\"mojodbclient-test\",")
							  _T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}"), true);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClientTest::updateType()
{
	// update type - add a new index
	MojErr err = writeTestObj(_T("{\"id\":\"LunaDbClientTest:1\",\"owner\":\"mojodbclient-test\",")
							  _T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]},{\"name\":\"foobar\",\"props\":[{\"name\":\"foobar\"}]}]}"), true);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClientTest::writeTestObj(const MojChar* json, MojObject* idOut)
{
	return writeTestObj(json, false, idOut);
}

MojErr MojDbClientTest::writeTestObj(const MojChar* json, bool kind, MojObject* idOut)
{
	// not a test, per say, but a utility to populate the datastore for performing specific tests

	MojObject obj;
	MojErr err = obj.fromJson(json);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	if (kind) {
		err = m_dbClient->putKind(handler->m_slot, obj);
		MojTestErrCheck(err);
	} else {
		err = m_dbClient->put(handler->m_slot, obj);
		MojTestErrCheck(err);
	}

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);
	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	if (!kind) {
		MojTestAssert(handler->m_result.get(_T("results"), results));

		MojObject::ConstArrayIterator iter = results.arrayBegin();
		MojTestAssert(iter != results.arrayEnd());

		MojObject id;
		err = iter->getRequired(_T("id"), id);
		MojTestErrCheck(err);
		MojTestAssert(!id.undefined());
		if (idOut) {
			*idOut = id;
		}
	}

	return MojErrNone;
}

MojErr MojDbClientTest::testPut()
{
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	MojObject obj;
	MojErr err;

	err = obj.putString(_T("_kind"), _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), _T("test_put_foo"));
	MojTestErrCheck(err);
	err = obj.putString(_T("bar"), _T("test_put_bar"));
	MojTestErrCheck(err);

	err = m_dbClient->put(handler->m_slot, obj);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	MojTestAssert(iter->find("id") != iter->end());
	MojTestAssert(iter->find("rev") != iter->end());

	// verify only 1 result
	iter++;
	MojTestAssert(iter == results.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::testMultiPut()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	MojUInt32 count = 10;
	MojObject::ObjectVec array;

	for (MojSize i = 0; i < count; i++) {
		MojObject obj;

		err = obj.putString(_T("_kind"), _T("LunaDbClientTest:1"));
		MojTestErrCheck(err);
		err = obj.putString(_T("foo"), _T("test_multiput_foo"));
		MojTestErrCheck(err);
		err = obj.putString(_T("bar"), _T("test_multiput_bar"));
		MojTestErrCheck(err);
		err = array.push(obj);
		MojTestErrCheck(err);
	}

	err = m_dbClient->put(handler->m_slot, array.begin(), array.end());
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	for (MojSize i = 0; i < count; i++) {
		MojTestAssert(iter != results.arrayEnd());
		MojTestAssert(iter->find("id") != iter->end());
		MojTestAssert(iter->find("rev") != iter->end());
		iter++;
	}
	// verify no extra results
	MojTestAssert(iter == results.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::getTestObj(MojObject id, MojObject& objOut)
{
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	MojObject idObj(id);
	MojErr err = m_dbClient->get(handler->m_slot, idObj);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));
	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	objOut = *iter;

	return MojErrNone;
}

MojErr MojDbClientTest::testGet()
{
	// write an object
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_get_foo\",\"bar\":\"test_get_bar\"})"), &id);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	MojObject idObj(id);
	err = m_dbClient->get(handler->m_slot, idObj);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	MojObject idReturned = -1;
	MojTestErrCheck(iter->getRequired("_id", idReturned));
	MojTestAssert(idReturned == id);
	MojTestAssert(iter->find("_rev") != iter->end());

	MojString foo;
	MojTestErrCheck(iter->getRequired(_T("foo"), foo));
	MojTestAssert(foo == _T("test_get_foo"));
	MojString bar;
	MojTestErrCheck(iter->getRequired(_T("bar"), bar));
	MojTestAssert(bar == _T("test_get_bar"));

	// verify only 1 result
	iter++;
	MojTestAssert(iter == results.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::testMultiGet()
{
	MojUInt32 itemCount = 10;
	MojErr err = MojErrNone;
	MojVector<MojObject> ids;

	for (MojUInt32 i = 0; i < itemCount; i++) {
		MojObject id;
		err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_multiget_foo\",\"bar\":\"test_multiget_bar\"})"), &id);
		MojTestErrCheck(err);
		err = ids.push(id);
		MojTestErrCheck(err);
	}

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	MojObject::ObjectVec array;
	for (MojUInt32 i = 0; i < itemCount; i++)
	{
		err = array.push(ids.at(i));
		MojTestErrCheck(err);
	}

	err = m_dbClient->get(handler->m_slot, array.begin(), array.end());
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojUInt32 cReceived = 0;
	for (MojObject::ConstArrayIterator i = results.arrayBegin(); i != results.arrayEnd(); i++) {
		MojObject id = -1;
		MojTestAssert(i->get("_id", id));
		MojTestAssert(id != -1);
		MojTestAssert(id == ids.at(cReceived++));
	}

	MojTestAssert(cReceived == itemCount);
	return MojErrNone;
}

MojErr MojDbClientTest::testDel()
{
	// write an object
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_del_foo\",\"bar\":\"test_del_bar\"})"), &id);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	// delete
	MojObject idObj(id);
	err = m_dbClient->del(handler->m_slot, idObj);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	MojTestAssert(iter->find("id") != iter->end());
	MojTestAssert(iter->find("rev") != iter->end());

	// verify only 1 result
	iter++;
	MojTestAssert(iter == results.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::testQueryDel()
{
	// write an object
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_qdel_foo\",\"bar\":\"test_qdel_bar\"})"));
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_qdel_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	// delete
	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	err = m_dbClient->del(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojInt64 count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == 1);

	return MojErrNone;
}

MojErr MojDbClientTest::testMultiDel()
{
	MojUInt32 itemCount = 10;
	MojErr err = MojErrNone;
	MojVector<MojObject> ids;

	for (MojUInt32 i = 0; i < itemCount; i++) {
		MojObject id;
		err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_multidel_foo\",\"bar\":\"test_multidel_\"})"), &id);
		MojTestErrCheck(err);
		err = ids.push(id);
		MojTestErrCheck(err);
	}

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	MojObject::ObjectVec array;
	for (MojUInt32 i = 0; i < itemCount; i++)
	{
		MojObject idObj(ids.at(i));
		err = array.push(idObj);
		MojTestErrCheck(err);
	}

	err = m_dbClient->del(handler->m_slot, array.begin(), array.end());
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	for (MojSize i = 0; i < itemCount; i++) {
		MojTestAssert(iter != results.arrayEnd());
		MojTestAssert(iter->find("id") != iter->end());
		MojTestAssert(iter->find("rev") != iter->end());
		iter++;
	}
	// verify no extra results
	MojTestAssert(iter == results.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::testMerge()
{
	// write an object
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_merge_foo\",\"bar\":\"test_merge_\"})"), &id);
	MojTestErrCheck(err);

	// modify select field
	MojObject obj2;
	err = obj2.put(_T("_id"), id);
	MojTestErrCheck(err);
	err = obj2.putString(_T("bar"), _T("test_merge_bar_post"));
	MojTestErrCheck(err);

	// merge object
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());;

	err = m_dbClient->merge(handler->m_slot, obj2);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result count
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	MojTestAssert(iter->find("id") != iter->end());
	MojTestAssert(iter->find("rev") != iter->end());

	// verify only 1 result
	iter++;
	MojTestAssert(iter == results.arrayEnd());

	// verify field modified
	MojObject objRetrieved;
	err = this->getTestObj(id, objRetrieved);
	MojTestErrCheck(err);
	MojString barRetrieved;
	err = objRetrieved.getRequired(_T("bar"), barRetrieved);
	MojTestErrCheck(err);
	MojTestAssert(barRetrieved == _T("test_merge_bar_post"));

	return MojErrNone;
}

MojErr MojDbClientTest::testQueryMerge()
{
	// write an object
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_qmerge_foo\",\"bar\":\"test_qmerge_bar\"})"), &id);
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_qmerge_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	// modify "bar" property
	MojObject props;
	err = props.putString(_T("bar"), _T("test_qmerge_bar_post"));
	MojTestErrCheck(err);

	// merge
	err = m_dbClient->merge(handler->m_slot, q, props);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result count
	MojTestErrCheck(handler->m_dbErr);
	MojInt64 count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == 1);

	// verify retrieved obj is merged
	MojObject objRetrieved;
	err = this->getTestObj(id, objRetrieved);
	MojTestErrCheck(err);
	MojString barRetrieved;
	err = objRetrieved.getRequired(_T("bar"), barRetrieved);
	MojTestErrCheck(err);
	MojTestAssert(barRetrieved == _T("test_qmerge_bar_post"));

	return MojErrNone;
}

MojErr MojDbClientTest::testMultiMerge()
{
	MojUInt32 itemCount = 10;
	MojErr err = MojErrNone;
	MojVector<MojObject> ids;

	for (MojUInt32 i = 0; i < itemCount; i++) {
		MojObject id;
		err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_multimerge_foo\",\"bar\":\"test_multimerge_bar\"})"), &id);
		MojTestErrCheck(err);
		err = ids.push(id);
		MojTestErrCheck(err);
	}

	// modify select field in each object
	MojObject::ObjectVec array;
	for (MojUInt32 i = 0; i < itemCount; i++) {
		MojObject obj;
		err = obj.put(_T("_id"), ids.at(i));
		MojTestErrCheck(err);
		err = obj.putString(_T("bar"), _T("test_multimerge_bar_post"));
		MojTestErrCheck(err);
		err = array.push(obj);
		MojTestErrCheck(err);
	}

	// merge objects
	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());
	err = m_dbClient->merge(handler->m_slot, array.begin(), array.end());
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result count
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	for (MojSize i = 0; i < itemCount; i++) {
		MojTestAssert(iter != results.arrayEnd());
		MojTestAssert(iter->find("id") != iter->end());
		MojTestAssert(iter->find("rev") != iter->end());
		iter++;
	}
	// verify no extra results
	MojTestAssert(iter == results.arrayEnd());

	// verify objects merged
	for (MojUInt32 i = 0; i < itemCount; i++) {
		MojObject objRetrieved;
		err = this->getTestObj(ids.at(i), objRetrieved);
		MojTestErrCheck(err);
		MojString barRetrieved;
		err = objRetrieved.getRequired(_T("bar"), barRetrieved);
		MojTestErrCheck(err);
		MojTestAssert(barRetrieved == _T("test_multimerge_bar_post"));
	}

	return MojErrNone;
}

MojErr MojDbClientTest::verifyFindResults(const MojDbClientTestResultHandler* handler,
		MojObject idExpected, const MojChar* barExpected)
{
	// verify that all results of a query have the same "bar" value, and that the specified
	// if is present in one result

	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	bool foundId = false;
	for (MojObject::ConstArrayIterator i = results.arrayBegin(); i != results.arrayEnd(); i++) {
		MojString bar;
		MojErr err = i->getRequired(_T("bar"), bar);
		MojTestErrCheck(err);
		MojTestAssert(bar == barExpected);
		MojObject id;
		err = i->getRequired(_T("_id"), id);
		MojTestErrCheck(err);
		if (id == idExpected) {
			foundId = true;
		}
	}
	MojTestAssert(foundId);

	return MojErrNone;
}

MojErr MojDbClientTest::testBasicFind()
{
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_basic_find_foo\",\"bar\":\"test_basic_find_bar\"})"), &id);
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_basic_find_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	err = m_dbClient->find(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	err = this->verifyFindResults(handler.get(), id, _T("test_basic_find_bar"));
	MojTestErrCheck(err);

	handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	q.limit(MojDbServiceHandler::MaxQueryLimit*2);

	err = m_dbClient->find(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(handler->m_dbErr != MojErrNone);

	return MojErrNone;
}


MojErr MojDbClientTest::testBasicSearch()
{
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_basic_search_foo\",\"bar\":\"test_basic_search_bar\"})"), &id);
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_basic_search_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	err = m_dbClient->search(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	err = this->verifyFindResults(handler.get(), id, _T("test_basic_search_bar"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClientTest::testUpdatedFind()
{
	MojErr err = updateType();
	MojTestErrCheck(err);

	MojObject id;
	err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"bar\":\"test_updated_find_bar\",\"foobar\":\"test_updated_find_foobar\"})"), &id);
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString baz;
	baz.assign(_T("test_updated_find_foobar"));
	MojObject whereObj(baz);
	err = q.where(_T("foobar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	err = m_dbClient->find(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	err = this->verifyFindResults(handler.get(), id, _T("test_updated_find_bar"));
	MojTestErrCheck(err);

	return MojErrNone;

}

MojErr MojDbClientTest::testWatchFind()
{
	MojObject id;
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_watchfind_foo\",\"bar\":\"test_watchfind_bar\"})"), &id);
	MojTestErrCheck(err);

	// select obj via query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_watchfind_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestWatcher> handler = new MojDbClientTestWatcher;
	MojAllocCheck(handler.get());

	err = m_dbClient->find(handler->m_slot, q, true);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	err = verifyFindResults(handler.get(), id, _T("test_watchfind_bar"));
	MojTestErrCheck(err);

	// add obj that matches query
	err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":1,\"bar\":\"test_watchfind_bar\"})"));
	MojTestErrCheck(err);

	// check if callback fired via dispatchfrom writeTestObj()
	if (handler->m_fired)
		return MojErrNone;

	// block until watch CB received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);
	MojTestAssert(handler->m_fired);

	return MojErrNone;
}

MojErr MojDbClientTest::testWatch()
{
	MojErr err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":\"test_watch_foo\",\"bar\":\"test_watch_bar\"})"));
	MojTestErrCheck(err);

	// format query
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_watch_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestWatcher> handler = new MojDbClientTestWatcher;
	MojAllocCheck(handler.get());

	err = m_dbClient->watch(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	// make sure no results returned
	MojTestAssert(!handler->m_result.contains(_T("results")));
	// but the watch should have fired because something already exists in the db that matches our watch
	MojTestAssert(handler->m_fired);

	// create a new query for a different value of bar
	MojDbQuery q2;
	err = q2.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar2;
	bar2.assign(_T("test_watch_bar2"));
	MojObject whereObj2(bar2);
	err = q2.where(_T("bar"), MojDbQuery::OpEq, whereObj2);
	MojTestErrCheck(err);

	// and watch that query
	handler.reset(new MojDbClientTestWatcher);
	MojAllocCheck(handler.get());

	err = m_dbClient->watch(handler->m_slot, q2);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	// make sure no results returned
	MojTestAssert(!handler->m_result.contains(_T("results")));
	// and that the watch has NOT fired
	MojTestAssert(!handler->m_fired);

	// add obj that matches query
	err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":1,\"bar\":\"test_watch_bar2\"})"));
	MojTestErrCheck(err);

	// if callback hasn't fired yet via dispatch from writeTestObj(), wait for it now
	if (!handler->m_fired) {
		// block until watch CB received
		err = handler->wait(m_dbClient->service());
		MojTestErrCheck(err);
	}

	MojTestAssert(handler->m_fired);
	// cleanup afterwards so that subsequent runs will work
	MojRefCountedPtr<MojDbClientTestResultHandler> resultHandler = new MojDbClientTestResultHandler;
	MojAllocCheck(resultHandler.get());

	err = m_dbClient->del(resultHandler->m_slot, q2);
	MojTestErrCheck(err);

	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(resultHandler->m_dbErr == MojErrNone);

	return MojErrNone;
}

MojErr MojDbClientTest::testErrorHandling(bool multiResponse)
{
	// error handling is implemented identically for all
	// single-response calls, and for all mult-response calls.
	// we therefore test find() as a representative example,
	// with and without watch semantics (for multi and  single
	// repsonses, respectively)

	// query on un-indexed field
	MojDbQuery q;
	MojErr err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString baz;
	baz.assign(_T("baz_invalid"));
	MojObject whereObj(baz);
	err = q.where(_T("baz"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestResultHandler> handler = new MojDbClientTestResultHandler;
	MojAllocCheck(handler.get());

	bool watch = multiResponse;
	err = m_dbClient->find(handler->m_slot, q, watch);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(handler->m_dbErr == MojErrDbNoIndexForQuery);

	// verify that successful callback dispatch on a new handler
	// still works after an error
	err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":1,\"bar\":\"1\"})"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbClientTest::testHdlrCancel()
{
	// issue a watch command, cancel handler, and
	// trigger a watch event - verify that handler callback
	// is not invoked

	// format query
	MojDbQuery q;
	MojErr err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString bar;
	bar.assign(_T("test_hdlr_cancel_bar"));
	MojObject whereObj(bar);
	err = q.where(_T("bar"), MojDbQuery::OpEq, whereObj);
	MojTestErrCheck(err);

	MojRefCountedPtr<MojDbClientTestWatcher> handler = new MojDbClientTestWatcher;
	MojAllocCheck(handler.get());

	err = m_dbClient->watch(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received, cancel handler
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);
	MojTestErrCheck(handler->m_dbErr);
	handler->m_slot.cancel();

	// add obj that matches query
	err = writeTestObj(_T("{\"_kind\":\"LunaDbClientTest:1\",\"foo\":1,\"bar\":\"test_hdlr_cancel_bar\"})"));
	MojTestErrCheck(err);

	// without cancel, the callback is fired via a dispatch from writeTestObj()
	// verify that the callback was not invoked
	MojTestAssert(!handler->m_fired);

	// cleanup afterwards so that subsequent runs will work
	MojRefCountedPtr<MojDbClientTestResultHandler> resultHandler = new MojDbClientTestResultHandler;
	MojAllocCheck(resultHandler.get());

	err = m_dbClient->del(resultHandler->m_slot, q);
	MojTestErrCheck(err);

	err = resultHandler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(resultHandler->m_dbErr == MojErrNone);

	return MojErrNone;
}

MojErr MojDbClientTest::testCompact()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//compact
	err = m_dbClient->compact(handler->m_slot);
	MojTestErrCheck(err);

	//block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify no error
	MojTestErrCheck(handler->m_dbErr);

	return MojErrNone;
}

MojErr MojDbClientTest::testPurge()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//purge everything in the dB
	err = m_dbClient->purge(handler->m_slot, 0);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result has a count attribute
	MojTestErrCheck(handler->m_dbErr);
	MojInt64 count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));

	// put 10 objects into the database
	MojObject::ObjectVec array;
	MojSize numObjects = 10;
	for (MojSize i = 0; i < numObjects; i++) {
		MojObject obj;

		err = obj.putString(_T("_kind"), _T("LunaDbClientTest:1"));
		MojTestErrCheck(err);
		err = obj.putString(_T("foo"), _T("test_purge_foo"));
		MojTestErrCheck(err);
		err = obj.putString(_T("bar"), _T("test_purge_bar"));
		MojTestErrCheck(err);
		err = array.push(obj);
		MojTestErrCheck(err);
	}

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());
	err = m_dbClient->put(handler->m_slot, array.begin(), array.end());
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	for (MojSize i = 0; i < numObjects; i++) {
		MojTestAssert(iter != results.arrayEnd());
		MojTestAssert(iter->find("id") != iter->end());
		MojTestAssert(iter->find("rev") != iter->end());
		iter++;
	}
	// verify no extra results
	MojTestAssert(iter == results.arrayEnd());

	//delete numToDelete of the objects
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());
	MojUInt32 numToDelete = 6;
	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	q.limit(numToDelete);

	// delete
	err = m_dbClient->del(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result
	MojTestErrCheck(handler->m_dbErr);
	count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == numToDelete);

	//purge all deleted objects
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());
	err = m_dbClient->purge(handler->m_slot, 0);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify purge count
	MojTestErrCheck(handler->m_dbErr);
	count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == numToDelete);

	//purge again - shouldn't purge anything
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());
	err = m_dbClient->purge(handler->m_slot, 0);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify purge count
	MojTestErrCheck(handler->m_dbErr);
	count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == 0);

	return MojErrNone;
}

MojErr MojDbClientTest::testPurgeStatus()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//getPurgeStatus
	err = m_dbClient->purgeStatus(handler->m_slot);
	MojTestErrCheck(err);

	//block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify result has a revNum attribute
	MojTestErrCheck(handler->m_dbErr);
	MojObject revNum;
	MojTestAssert(handler->m_result.get("rev", revNum));

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	// put 1 object into the database
	MojObject obj;
	err = obj.putString(_T("_kind"), _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), _T("test_purgeStatus_foo"));
	MojTestErrCheck(err);
	err = obj.putString(_T("bar"), _T("test_purgeStatus_bar"));
	MojTestErrCheck(err);

	err = m_dbClient->put(handler->m_slot, obj);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// get revNum of the put
	MojTestErrCheck(handler->m_dbErr);
	MojObject results;
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject firstResult;
	MojTestAssert(results.at(0, firstResult));
	MojObject rev;
	MojTestAssert(firstResult.get("rev", rev));
	MojObject id;
	MojTestAssert(firstResult.get("id", id));

	//delete the object
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	err = m_dbClient->del(handler->m_slot, id);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// make sure one object was deleted
	MojTestErrCheck(handler->m_dbErr);
	MojTestAssert(handler->m_result.get(_T("results"), results));

	MojObject::ConstArrayIterator iter = results.arrayBegin();
	MojTestAssert(iter != results.arrayEnd());
	MojTestAssert(iter->find("id") != iter->end());
	MojTestAssert(iter->find("rev") != iter->end());

	// verify only 1 result
	iter++;
	MojTestAssert(iter == results.arrayEnd());

	//purge
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	err = m_dbClient->purge(handler->m_slot, 0);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify purge count
	MojTestErrCheck(handler->m_dbErr);
	MojInt64 count = -1;
	MojTestAssert(handler->m_result.get(_T("count"), count));
	MojTestAssert(count == 1);

	//get the purge status
	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	err = m_dbClient->purgeStatus(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// check the last purge rev num
	MojTestErrCheck(handler->m_dbErr);
	MojObject lastRevNum;
	MojTestAssert(handler->m_result.get(_T("rev"), lastRevNum));
	MojTestAssert(lastRevNum.intValue() > rev.intValue());

	return MojErrNone;
}

MojErr MojDbClientTest::testDeleteKind()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//delete LunaDbClientTest kind
	MojString str;
	str.assign("LunaDbClientTest:1");
	err = m_dbClient->delKind(handler->m_slot, str);
	MojTestErrCheck(err);

	//block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	// verify there was no error
	MojTestErrCheck(handler->m_dbErr);

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	MojDbQuery q;
	err = q.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);

	err = m_dbClient->find(handler->m_slot, q);
	MojTestErrCheck(err);

	// block until repsonse received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(handler->m_dbErr == MojErrDbKindNotRegistered);

	return MojErrNone;
}

MojErr MojDbClientTest::testBatch()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//first put the kind outside a batch
	err = registerType();
	MojTestErrCheck(err);

	//now create a batch and put one object
	MojAutoPtr<MojDbBatch> batch;
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	MojObject obj;

	err = obj.putString(_T("_kind"), _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), _T("test_batch_foo"));
	MojTestErrCheck(err);
	err = obj.putString(_T("bar"), _T("test_batch_bar"));
	MojTestErrCheck(err);

	err = batch->put(obj);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	MojObject responses;
	MojTestAssert(handler->m_result.get(_T("responses"), responses));

	MojObject::ConstArrayIterator iter = responses.arrayBegin();
	MojTestAssert(iter != responses.arrayEnd());
	MojObject results;
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	MojObject::ConstArrayIterator putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojTestAssert(putIter->find("id") != putIter->end());
	MojTestAssert(putIter->find("rev") != putIter->end());

	// verify only 1 result
	putIter++;
	MojTestAssert(putIter == results.arrayEnd());
	iter++;
	MojTestAssert(iter == responses.arrayEnd());

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//create another batch with two puts, one that will fail
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	MojObject obj2;

	err = obj2.putString(_T("_kind"), _T("RandomKind:1"));
	MojTestErrCheck(err);
	err = obj2.putString(_T("foo"), _T("test_batch_foo"));
	MojTestErrCheck(err);
	err = obj2.putString(_T("bar"), _T("test_batch_bar"));
	MojTestErrCheck(err);

	err = obj.putString(_T("foo"), _T("test_batch_foo2"));
	MojTestErrCheck(err);
	err = obj.putString(_T("bar"), _T("test_batch_bar2"));
	MojTestErrCheck(err);

	err = batch->put(obj);
	MojTestErrCheck(err);

	err = batch->put(obj2);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestAssert(handler->m_dbErr == MojErrDbKindNotRegistered);
	MojObject errorCode;
	MojTestAssert(handler->m_result.get(_T("errorCode"), errorCode));
	MojObject errorText;
	MojTestAssert(handler->m_result.get(_T("errorText"), errorText));

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	//do a find in a batch and verify that the last transaction was aborted - there should only be one result
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	MojDbQuery query;
	err = query.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);

	err = batch->find(query, true);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	MojTestAssert(handler->m_result.get(_T("responses"), responses));

	iter = responses.arrayBegin();
	MojTestAssert(iter != responses.arrayEnd());
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojObject objId;
	err = putIter->getRequired(_T("_id"), objId);
	MojTestErrCheck(err);
	MojInt64 rev;
	err = putIter->getRequired(_T("_rev"), rev);
	MojTestErrCheck(err);
	MojInt64 count;
	err = iter->getRequired(_T("count"), count);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);

	// verify only 1 result
	putIter++;
	MojTestAssert(putIter == results.arrayEnd());
	iter++;
	MojTestAssert(iter == responses.arrayEnd());

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	// test what happens if you modify an object and then query for it in the same batch
	// (the query should find the modified object even though the txn hasn't committed)
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	err = obj.put(_T("_id"), objId);
	MojTestErrCheck(err);
	err = obj.putString(_T("foo"), _T("test_batch_modifiedFoo"));
	MojTestErrCheck(err);
	err = obj.put(_T("_rev"), rev);
	MojTestErrCheck(err);

	err = batch->put(obj);
	MojTestErrCheck(err);

	MojDbQuery fooQuery;
	err = fooQuery.from(_T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	MojString whereStr;
	err = whereStr.assign( _T("test_batch_modifiedFoo"));
	MojTestErrCheck(err);
	err = fooQuery.where(_T("foo"), MojDbQuery::OpEq, whereStr);
	MojTestErrCheck(err);

	err = batch->find(fooQuery, true);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	MojTestAssert(handler->m_result.get(_T("responses"), responses));

	iter = responses.arrayBegin();
	MojTestAssert(iter != responses.arrayEnd());
	//put result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojTestAssert(putIter->find(_T("id")) != putIter->end());
	MojTestAssert(putIter->find(_T("rev")) != putIter->end());

	iter++;
	//find result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojObject dbId;
	err = putIter->getRequired(_T("_id"), dbId);
	MojTestErrCheck(err);
	MojTestAssert(dbId == objId);
	MojTestAssert(putIter->find(_T("_rev")) != putIter->end());
	err = iter->getRequired(_T("count"), count);
	MojTestErrCheck(err);
	MojTestAssert(count == 1);

	//verify those were the only two results
	iter++;
	MojTestAssert(iter == responses.arrayEnd());

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	// test a delete and then a get in a batch
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	err = batch->del(objId);
	MojTestErrCheck(err);

	err = batch->get(objId);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	MojTestAssert(handler->m_result.get(_T("responses"), responses));

	iter = responses.arrayBegin();
	MojTestAssert(iter != responses.arrayEnd());
	//del result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojTestAssert(putIter->find("id") != iter->end());
	MojTestAssert(putIter->find("rev") != iter->end());

	// verify only 1 result
	putIter++;
	MojTestAssert(putIter == results.arrayEnd());

	iter++;
	//get result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	err = putIter->getRequired(_T("_id"), dbId);
	MojTestErrCheck(err);
	MojTestAssert(dbId == objId);
	bool deleted;
	err = putIter->getRequired(_T("_del"), deleted);
	MojTestErrCheck(err);
	MojTestAssert(deleted);

	//verify those were the only two results
	iter++;
	MojTestAssert(iter == responses.arrayEnd());

	handler.reset(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	// test a put, get, merge, get, del in a batch
	err = m_dbClient->createBatch(batch);
	MojTestErrCheck(err);
	MojTestAssert(batch.get());

	MojObject object;
	MojString myId;
	err = myId.assign(_T("myId"));
	MojTestErrCheck(err);
	err = object.putString(_T("_kind"), _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	err = object.putString(_T("foo"), _T("test_batch_myFoo"));
	MojTestErrCheck(err);
	err = object.putString(_T("_id"), myId);
	MojTestErrCheck(err);

	err = batch->put(object);
	MojTestErrCheck(err);

	err = batch->get(myId);
	MojTestErrCheck(err);

	MojObject object2;
	err = object2.putString(_T("_kind"), _T("LunaDbClientTest:1"));
	MojTestErrCheck(err);
	err = object2.putString(_T("bar"), _T("test_batch_myBar"));
	MojTestErrCheck(err);
	err = object2.putString(_T("_id"), myId);
	MojTestErrCheck(err);

	err = batch->merge(object2);
	MojTestErrCheck(err);

	err = batch->get(myId);
	MojTestErrCheck(err);

	err = batch->del(myId);
	MojTestErrCheck(err);

	err = batch->execute(handler->m_slot);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);

	MojTestErrCheck(handler->m_dbErr);
	MojTestAssert(handler->m_result.get(_T("responses"), responses));

	iter = responses.arrayBegin();
	MojTestAssert(iter != responses.arrayEnd());
	//put result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	err = putIter->getRequired(_T("id"), dbId);
	MojTestAssert(dbId == myId);
	MojTestAssert(putIter->find(_T("rev")) != putIter->end());

	iter++;
	//get result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	err = putIter->getRequired(_T("_id"), dbId);
	MojTestErrCheck(err);
	MojTestAssert(dbId == myId);
	MojTestAssert(putIter->find(_T("foo")) != putIter->end());
	MojTestAssert(putIter->find(_T("bar")) == putIter->end());

	iter++;
	//merge result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter->find("id") != putIter->end());
	MojTestAssert(putIter->find("rev") != putIter->end());
	// verify only 1 result
	putIter++;
	MojTestAssert(putIter == results.arrayEnd());


	iter++;
	//get result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	err = putIter->getRequired(_T("_id"), dbId);
	MojTestErrCheck(err);
	MojTestAssert(dbId == myId);
	MojTestAssert(putIter->find(_T("foo")) != putIter->end());
	MojTestAssert(putIter->find(_T("bar")) != putIter->end());

	iter++;
	//del result
	err = iter->getRequired(_T("results"), results);
	MojTestErrCheck(err);
	putIter = results.arrayBegin();
	MojTestAssert(putIter != results.arrayEnd());
	MojTestAssert(putIter->find("id") != iter->end());
	MojTestAssert(putIter->find("rev") != iter->end());

	// verify only 1 result
	putIter++;
	MojTestAssert(putIter == results.arrayEnd());

	//verify there were only four results
	iter++;
	MojTestAssert(iter == responses.arrayEnd());

	return MojErrNone;
}

MojErr MojDbClientTest::testPermissions()
{
	MojErr err;
	MojRefCountedPtr<MojDbClientTestResultHandler> handler(new MojDbClientTestResultHandler);
	MojAllocCheck(handler.get());

	MojObject perm;
	err = perm.fromJson(
			_T("{\"type\":\"foo\",\"caller\":\"com.test.foo\",\"operations\":{\"create\":\"allow\"},\"object\":\"permtest:1\"}")
			);
	MojTestErrCheck(err);
	err = m_dbClient->putPermission(handler->m_slot, perm);
	MojTestErrCheck(err);

	// block until response received
	err = handler->wait(m_dbClient->service());
	MojTestErrCheck(err);
	MojTestErrCheck(handler->m_dbErr);

	return MojErrNone;
}
