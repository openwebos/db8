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


#include "MojLunaServiceTest.h"
#include "core/MojOs.h"
#include "core/MojObject.h"
#include "core/MojJson.h"
#include "core/MojServiceMessage.h"
#include "core/MojServiceRequest.h"
//#define USE_SOCKET_SERVICE

#ifdef USE_SOCKET_SERVICE
#include "luna/MojSocketService.h"
#include "luna/MojEpollReactor.h"
#define SERVICE_URI _T("/tmp/mojsock")
const MojChar* ServiceName = _T("/tmp/mojsock");
#else
#include "core/MojGmainReactor.h"
#include "luna/MojLunaService.h"
#define SERVICE_URI _T("luna://com.palm.mojlstest")
const MojChar* ServiceName = _T("com.palm.mojlstest");
#endif // USE_SOCKET_SERVICE

static const MojUInt32 multiResponseCount = 10;

class MojLunaTestService : public MojService::CategoryHandler
{
public:
	MojLunaTestService(bool& stop, MojService& service)
	: m_stop(stop),
  	  m_cCancellableEchos(0),
  	  m_service(service)
	{
	}

	MojErr echo(MojServiceMessage* msg, const MojObject& payload)
	{
		MojString echoStr;
		MojErr err = payload.toJson(echoStr);
		MojTestErrCheck(err);

		MojObject response;
		err = response.putBool(MojServiceMessage::ReturnValueKey, true);
		MojTestErrCheck(err);
		err = response.putString(_T("echo"), echoStr);
		MojTestErrCheck(err);

		err = msg->reply(response);
		MojTestErrCheck(err);
		return MojErrNone;
	}

	MojErr slowEcho(MojServiceMessage* msg, const MojObject& payload)
	{
		sleep(1);

		MojErr err = echo(msg, payload);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr multiEcho(MojServiceMessage* msg, const MojObject& payload)
	{
		for (MojUInt32 i = 0; i < multiResponseCount; i++) {
			MojErr err = echo(msg, payload);
			MojTestErrCheck(err);
		}

		return MojErrNone;
	}

	class EchoCancelHdlr : public MojSignalHandler
	{
	public:
		EchoCancelHdlr(MojUInt32& cCancellableEchos)
		: m_cCancellableEchos(cCancellableEchos), m_slot(this, &EchoCancelHdlr::handleCancel) {}

		MojErr handleCancel(MojServiceMessage* msg)
		{
			MojTestAssert(m_cCancellableEchos > 0);
			m_cCancellableEchos--;

			return MojErrNone;
		}

		MojUInt32& m_cCancellableEchos;
		MojServiceMessage::CancelSignal::Slot<EchoCancelHdlr> m_slot;
	};

	MojErr cancellableEcho(MojServiceMessage* msg, const MojObject& payload)
	{
		MojRefCountedPtr<EchoCancelHdlr> hdlr(new EchoCancelHdlr(m_cCancellableEchos));
		MojAllocCheck(hdlr.get());
		msg->notifyCancel(hdlr->m_slot);
		m_cCancellableEchos++;

		MojErr err = echo(msg, payload);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr shutDown(MojServiceMessage* msg, const MojObject& payload)
	{
		m_stop = true;
		return MojErrNone;
	}

	virtual MojErr open(const MojChar* serviceName)
	{
		MojErr err = m_service.open(serviceName);
		MojTestErrCheck(err);

		typedef struct {
			const MojChar* m_name;
			Callback m_callback;
		} Method;

		static const Method methods[] = {
				{_T("echo"), (Callback)&MojLunaTestService::echo},
				{_T("slowecho"), (Callback)&MojLunaTestService::slowEcho},
				{_T("multiecho"), (Callback)&MojLunaTestService::multiEcho},
				{_T("cancellableecho"), (Callback)&MojLunaTestService::cancellableEcho},
				{_T("shutdown"), (Callback)&MojLunaTestService::shutDown},
				{NULL, NULL}};

		for (const Method* method = methods; method->m_name != NULL; method++) {
			MojErr err = addMethod(method->m_name, method->m_callback);
			MojErrCheck(err);
		}

		err = m_service.addCategory(MojService::DefaultCategory, this);
		MojErrCheck(err);

		return MojErrNone;
	}

	bool& m_stop;
	MojUInt32 m_cCancellableEchos;
	MojService& m_service;
};

class MojLunaTestClient
{
public:
	MojLunaTestClient(MojService& service)
	: m_service(service),
	  m_pendingResponseCount(0)
	{
	}

	MojErr stopService()
	{
		MojRefCountedPtr<MojServiceRequest> req;
		MojErr err = m_service.createRequest(req);
		MojTestErrCheck(err);
		MojObject expected;
		MojRefCountedPtr<EchoResponseHdlr> handler(new EchoResponseHdlr(expected, ++m_pendingResponseCount));
		MojAllocCheck(handler.get());
		err = req->send(handler->m_slot, ServiceName, _T("shutdown"), MojObject(MojObject::TypeObject));
		MojTestErrCheck(err);

		return MojErrNone;

	}
	MojErr testEchoRequest()
	{
		// send a valid echo request and verify response

		const MojChar* echoStr = _T("{\"hello\":\"world\"}");
		MojObject expectedResponse;
		MojErr err = formatEchoResponse(echoStr, expectedResponse);
		MojTestErrCheck(err);

		err = sendEcho(echoStr, expectedResponse, _T("echo"));
		MojTestErrCheck(err);

		err = receiveResponses();
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr testInvalidPayload()
	{
		// send an invaild error request and verify error code response

		MojObject expectedResponse;
		MojErr err = formatErrResponse(MojErrJsonParsePropName, expectedResponse);
		MojTestErrCheck(err);

		// send echo with invalid json
		err = sendEcho(_T("{{\"hello\":\"world\"}"), expectedResponse, _T("echo"));
		MojTestErrCheck(err);

		err = receiveResponses();
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr testQueue()
	{
		// send a high-latency request and build up a service-side queue
		const MojChar* echoStr = _T("{\"hello\":\"world\"}");
		MojObject expectedResponse;
		MojErr err = formatEchoResponse(echoStr, expectedResponse);
		MojTestErrCheck(err);

		err = sendEcho(echoStr, expectedResponse, _T("slowecho"));
		MojTestErrCheck(err);

		for (int i = 0; i < 10; i++) {
			err = sendEcho(echoStr, expectedResponse, _T("echo"));
			MojTestErrCheck(err);
		}

		err = receiveResponses();
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr testMultiEcho()
	{
		const MojChar* echoStr = _T("{\"hello\":\"world\"}");
		MojObject expectedResponse;
		MojErr err = formatEchoResponse(echoStr, expectedResponse);
		MojTestErrCheck(err);

		MojRefCountedPtr<MojServiceRequest> req;
		err = m_service.createRequest(req);
		MojTestErrCheck(err);

		m_pendingResponseCount+= multiResponseCount;
		MojRefCountedPtr<EchoMultiResponseHdlr> hdlr(new EchoMultiResponseHdlr(expectedResponse, m_pendingResponseCount, multiResponseCount));
		MojAllocCheck(hdlr.get());

		MojObject payload;
		err = payload.fromJson(echoStr);
		MojTestErrCheck(err);
		err = req->send(hdlr->m_slot, ServiceName, _T("multiecho"), payload, MojServiceRequest::Unlimited);
		MojTestErrCheck(err);

		err = receiveResponses();
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr testCancel()
	{
		const MojChar* echoStr = _T("{\"hello\":\"world\"}");
		MojObject expectedResponse;
		MojErr err = formatEchoResponse(echoStr, expectedResponse);
		MojTestErrCheck(err);

		MojRefCountedPtr<MojServiceRequest> req;
		err = m_service.createRequest(req);
		MojTestErrCheck(err);

		int callbackCount = 2;
		MojRefCountedPtr<EchoMultiResponseHdlr> handler(new EchoMultiResponseHdlr(expectedResponse, ++m_pendingResponseCount, callbackCount));
		MojTestAssert(handler.get());

		MojObject payload;
		err = payload.fromJson(echoStr);
		MojTestErrCheck(err);
		err = req->send(handler->m_slot, ServiceName, _T("cancellableecho"), payload, MojServiceRequest::Unlimited);
		MojTestErrCheck(err);

		err = receiveResponses();
		MojTestErrCheck(err);

		MojTestAssert(handler->callbacksRemaining() == 1); // still have callback registered because 1 more response is expected

		// cancel request
		handler->m_slot.cancel();
		sleep(1);

		return MojErrNone;
	}

	MojErr testInvalidMethod()
	{
		MojObject expectedResponse;
		MojErr err = formatErrResponse(MojErrMethodNotFound, expectedResponse);
		MojTestErrCheck(err);

		// send echo with invalid method
		err = sendEcho(_T("{\"hello\":\"world\"}"), expectedResponse, _T("bogusmethod"));
		MojTestErrCheck(err);

		err = receiveResponses();
		MojTestErrCheck(err);

		return MojErrNone;
	}

private:
	MojErr receiveResponses()
	{
		MojTestAssert(m_pendingResponseCount > 0);

		// block until response received
		while (m_pendingResponseCount > 0) {
			MojErr err = m_service.dispatch();
			MojTestErrCheck(err);
		}

		return MojErrNone;
	}

	class EchoResponseHdlr : public MojSignalHandler
	{
	public:
		EchoResponseHdlr(const MojObject& expected, MojUInt32& pendingCount)
		: m_expected(expected),
		  m_pendingCount(pendingCount),
		  m_slot(this, &EchoResponseHdlr::handleResponse)
		{
		}

		MojErr handleResponse(MojObject& payload, MojErr errCode)
		{
			MojTestAssert(m_pendingCount > 0);
			MojTestAssert(payload == m_expected);

			m_pendingCount--;
			return MojErrNone;
		}

		MojObject m_expected;
		MojUInt32& m_pendingCount;
		MojServiceRequest::ReplySignal::Slot<EchoResponseHdlr> m_slot;
	};

	class EchoMultiResponseHdlr : public MojSignalHandler
	{
	public:
		EchoMultiResponseHdlr(const MojObject& expected, MojUInt32& pendingCount, MojUInt32 callbackCount)
		: m_expected(expected),
		  m_pendingCount(pendingCount),
		  m_callbacksRemaining(callbackCount),
		  m_slot(this, &EchoMultiResponseHdlr::handleResponse)
		{
		}

		MojErr handleResponse(MojObject& payload, MojErr errCode)
		{
			MojTestAssert(m_pendingCount > 0);
			MojTestAssert(payload == m_expected);

			m_pendingCount--;
			MojTestAssert(m_callbacksRemaining > 0);
			if (--m_callbacksRemaining == 0) {
				m_slot.cancel();
			}
			return MojErrNone;
		}

		MojUInt32 callbacksRemaining() const { return m_callbacksRemaining; }

		MojObject m_expected;
		MojUInt32& m_pendingCount;
		MojUInt32 m_callbacksRemaining;
		MojServiceRequest::ReplySignal::Slot<EchoMultiResponseHdlr> m_slot;
	};

	MojErr sendEcho(const MojChar* json, const MojObject& expectedResponse, const MojChar* method)
	{
		MojRefCountedPtr<EchoResponseHdlr> handler(new EchoResponseHdlr(expectedResponse, ++m_pendingResponseCount));
		MojAllocCheck(handler.get());

		MojRefCountedPtr<MojServiceRequest> req;
		MojErr err = m_service.createRequest(req);
		MojTestErrCheck(err);

		MojObject payload;
		err = payload.fromJson(json);
		MojTestErrCheck(err);
		err = req->send(handler->m_slot, ServiceName, method, payload);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr formatErrResponse(MojErr errExpected, MojObject& objOut)
	{
		MojString errTxt;
		MojErr err = MojErrToString(errExpected, errTxt);
		MojTestErrCheck(err);

		// format expected error return
		err = objOut.putBool(MojServiceMessage::ReturnValueKey, false);
		MojErrCheck(err);
		err = objOut.putInt(MojServiceMessage::ErrorCodeKey, (MojInt64)errExpected);
		MojErrCheck(err);
		err = objOut.putString(MojServiceMessage::ErrorTextKey, errTxt);
		MojErrCheck(err);

		return MojErrNone;
	}

	MojErr formatEchoResponse(const MojChar* str, MojObject& respOut)
	{
		MojErr err = respOut.putBool(MojServiceMessage::ReturnValueKey, true);
		MojTestErrCheck(err);
		err = respOut.putString(_T("echo"), str);
		MojTestErrCheck(err);
		return MojErrNone;
	}

	MojService& m_service;
	MojUInt32 m_pendingResponseCount;
};

MojErr serviceThread(void*)
{
	MojErr err = MojErrNone;
	bool stop = false;

#ifdef USE_SOCKET_SERVICE
	MojEpollReactor reactor;
	MojSocketService msgService(reactor);
	err = reactor.init();
	MojErrCheck(err);
#else
	MojGmainReactor reactor;
	err = reactor.init();
	MojErrCheck(err);
	MojLunaService msgService;
	err = msgService.attach(reactor.impl());
	MojErrCheck(err);
#endif

	MojRefCountedPtr<MojLunaTestService> service(new MojLunaTestService(stop, msgService));
	MojAllocCheck(service.get());
	err = service->open(ServiceName);
	MojErrCheck(err);

	while (!stop) {
		err = msgService.dispatch();
		MojErrCatchAll(err);
	}
	err = msgService.close();
	MojErrCheck(err);

	return MojErrNone;
}

int main(int argc, char** argv)
{
	MojThreadT thread;
	MojErr err = MojThreadCreate(thread, serviceThread, NULL);
	MojErrCheck(err);

	MojLunaServiceTestRunner runner;
	return runner.main(argc, argv);
}

void MojLunaServiceTestRunner::runTests()
{
	test(MojLunaServiceTest());
}

MojLunaServiceTest::MojLunaServiceTest()
: MojTestCase(_T("MojLunaServiceTest"))
{
}

MojErr MojLunaServiceTest::run()
{
	MojErr err = MojErrNone;

#ifdef USE_SOCKET_SERVICE
	MojEpollReactor reactor;
	MojSocketService service(reactor);
	err = reactor.init();
	MojErrCheck(err);
#else
	MojGmainReactor reactor;
	err = reactor.init();
	MojErrCheck(err);
	MojLunaService service;
	err = service.attach(reactor.impl());
	MojErrCheck(err);
#endif

	MojLunaTestClient clientService(service);
	err = service.open(NULL);
	MojTestErrCheck(err);

	err = clientService.testEchoRequest();
	MojTestErrCheck(err);

//	err = clientService.testInvalidPayload();
//	MojTestErrCheck(err);

	err = clientService.testInvalidMethod();
	MojTestErrCheck(err);

	err = clientService.testQueue();
	MojTestErrCheck(err);

	err = clientService.testMultiEcho();
	MojTestErrCheck(err);

	err = clientService.testCancel();
	MojTestErrCheck(err);

	// must be last!
	err = clientService.stopService();
	MojTestErrCheck(err);

	err = service.close();
	MojTestErrCheck(err);

	return MojErrNone;
}
