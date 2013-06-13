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


#include "luna/MojLunaService.h"
#include "luna/MojLunaErr.h"
#include "luna/MojLunaMessage.h"
#include "luna/MojLunaRequest.h"
#include "core/MojObject.h"

const MojChar* const MojLunaService::UriScheme = _T("palm");

MojLunaService::MojLunaService(bool allowPublicMethods, MojMessageDispatcher* queue)
: MojService(queue),
  m_allowPublicMethods(allowPublicMethods),
  m_service(NULL),
  m_handle(NULL),
  m_loop(NULL)
{
	MojLogTrace(s_log);
}

MojLunaService::~MojLunaService()
{
	MojLogTrace(s_log);

	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojLunaService::open(const MojChar* serviceName)
{
	MojLogTrace(s_log);

	MojErr err =  MojService::open(serviceName);
	MojErrCheck(err);

	bool retVal;
	MojLunaErr lserr;

	// create service handle
	if (m_allowPublicMethods) {
		retVal = LSRegisterPalmService(serviceName, &m_service, lserr);
		MojLsErrCheck(retVal, lserr);

		LSHandle* handle = LSPalmServiceGetPublicConnection(m_service);
		retVal = LSSubscriptionSetCancelFunction(handle, handleCancel, this, lserr);
		MojLsErrCheck(retVal, lserr);

		handle = LSPalmServiceGetPrivateConnection(m_service);
		retVal = LSSubscriptionSetCancelFunction(handle, handleCancel, this, lserr);
		MojLsErrCheck(retVal, lserr);
	} else {
		retVal = LSRegister(serviceName, &m_handle, lserr);
		MojLsErrCheck(retVal, lserr);
		retVal = LSSubscriptionSetCancelFunction(m_handle, handleCancel, this, lserr);
		MojLsErrCheck(retVal, lserr);
	}
	return MojErrNone;
}

MojErr MojLunaService::close()
{
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojErr errClose = MojService::close();
	MojErrAccumulate(err, errClose);

	// destroy service handle
	MojLunaErr lserr;
	bool retVal;
	if (m_service) {
		retVal = LSUnregisterPalmService(m_service, lserr);
		m_service = NULL;
		m_handle = NULL;
		MojLsErrAccumulate(err, retVal, lserr);
	} else if (m_handle) {
		retVal = LSUnregister(m_handle, lserr);
		m_handle = NULL;
		MojLsErrAccumulate(err, retVal, lserr);
	}
	return err;
}

// Note: this is a blocking interface exclusively for unit tests.
// It will not deliver cancel callbacks if clients drop off the bus.
MojErr MojLunaService::dispatch()
{
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	MojAssert(m_loop);
	GMainContext* context = g_main_loop_get_context(m_loop);
	g_main_context_iteration(context, true);

	return MojErrNone;
}

MojErr MojLunaService::addCategory(const MojChar* name, CategoryHandler* handler)
{
	MojAssert(name && handler);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	// create array of LSMethods
	MethodVec pubMethods;
	MethodVec privMethods;
	const CategoryHandler::CallbackMap& callbacks = handler->callbacks();
	for (CategoryHandler::CallbackMap::ConstIterator i = callbacks.begin();
		 i != callbacks.end();
		 i++) {
			LSMethod m = {i.key(), &handleRequest};

			MethodVec& methods = i->m_pub ? pubMethods : privMethods;
			MojErr err = methods.push(m);
			MojErrCheck(err);
	}
    LSMethod nullMethod = {NULL, NULL};
    MojErr err = pubMethods.push(nullMethod);
    MojErrCheck(err);
    err = privMethods.push(nullMethod);
    MojErrCheck(err);

    // create category object to hang on to method array
	MojRefCountedPtr<LunaCategory> cat(new LunaCategory(this, handler, pubMethods, privMethods));
	MojAllocCheck(cat.get());
	LSMethod* pubLsMethods = const_cast<LSMethod*>(cat->m_pubMethods.begin());
	LSMethod* privLsMethods = const_cast<LSMethod*>(cat->m_privMethods.begin());

	MojLunaErr lserr;
    bool retVal;
    if (m_service) {
    	retVal = LSPalmServiceRegisterCategory(m_service, name, pubLsMethods, privLsMethods, NULL, cat.get(), lserr);
    	MojLsErrCheck(retVal, lserr);
    } else {
    	MojAssert(m_handle);
    	retVal = LSRegisterCategory(m_handle, name, privLsMethods, NULL, NULL, lserr);
    	MojLsErrCheck(retVal, lserr);
        retVal = LSCategorySetData(m_handle, name, cat.get(), lserr);
        MojLsErrCheck(retVal, lserr);
    }

	MojString categoryStr;
	err = categoryStr.assign(name);
	MojErrCheck(err);
	err = m_categories.put(categoryStr, cat);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojLunaService::createRequest(MojRefCountedPtr<MojServiceRequest>& reqOut)
{
	MojLogTrace(s_log);

	reqOut.reset(new MojLunaRequest(this));
	MojAllocCheck(reqOut.get());

	return MojErrNone;
}

MojErr MojLunaService::createRequest(MojRefCountedPtr<MojServiceRequest>& reqOut, bool onPublic)
{
	MojLogTrace(s_log);

	reqOut.reset(new MojLunaRequest(this, onPublic));
	MojAllocCheck(reqOut.get());

	return MojErrNone;
}

MojErr MojLunaService::createRequest(MojRefCountedPtr<MojServiceRequest>& reqOut, bool onPublic, const MojString& proxyRequester)
{
	MojLogTrace(s_log);

	reqOut.reset(new MojLunaRequest(this, onPublic, proxyRequester));
	MojAllocCheck(reqOut.get());

	return MojErrNone;
}

MojErr MojLunaService::createRequest(MojRefCountedPtr<MojServiceRequest>& reqOut, bool onPublic, const char *proxyRequester)
{
	MojLogTrace(s_log);

	MojString proxyRequesterString;
	MojErr err = proxyRequesterString.assign(proxyRequester);
	MojErrCheck(err);

	reqOut.reset(new MojLunaRequest(this, onPublic, proxyRequesterString));
	MojAllocCheck(reqOut.get());

	return MojErrNone;
}

MojErr MojLunaService::attach(GMainLoop* loop)
{
	MojAssert(loop);
	MojLogTrace(s_log);

	MojLunaErr lserr;
	bool retVal;
	if (m_service) {
		retVal= LSGmainAttachPalmService(m_service, loop, lserr);
		MojLsErrCheck(retVal, lserr);
	} else {
		MojAssert(m_handle);
		retVal= LSGmainAttach(m_handle, loop, lserr);
		MojLsErrCheck(retVal, lserr);
	}
	m_loop = loop;

	return MojErrNone;
}

LSPalmService* MojLunaService::getService()
{
	return m_service;
}

LSHandle* MojLunaService::getHandle(bool onPublic)
{
	LSHandle* handle;
	if (m_service) {
		if (onPublic) {
			handle = LSPalmServiceGetPublicConnection(m_service);
		} else {
			handle = LSPalmServiceGetPrivateConnection(m_service);
		}
	} else {
		handle = m_handle;
	}
	return handle;
}

MojErr MojLunaService::sendImpl(MojServiceRequest* req, const MojChar* service, const MojChar* method, Token& tokenOut)
{
	MojAssert(req && service && method);
	MojAssert(m_service || m_handle);
	MojAssertMutexLocked(m_mutex);
	MojLogTrace(s_log);

	MojLunaRequest* lunaReq = static_cast<MojLunaRequest*>(req);
	const MojChar* json = lunaReq->payload();
	MojLogInfo(s_log, _T("request sent: %s"), json);

	MojString uri;
	MojErr err = uri.format(_T("%s://%s/%s"), UriScheme, service, method);
	MojErrCheck(err);

	MojLunaErr lserr;
	LSMessageToken lsToken;
	LSHandle* handle = getHandle(lunaReq->onPublic());
	if (req->numRepliesExpected() > 1) {
		if (!lunaReq->isProxyRequest()) {
			bool retVal = LSCall(handle, uri, json, &handleResponse, this, &lsToken, lserr);
			MojLsErrCheck(retVal, lserr);
		} else {
			bool retVal = LSCallFromApplication(handle, uri, json, lunaReq->getRequester(), &handleResponse, this, &lsToken, lserr);
			MojLsErrCheck(retVal, lserr);
		}
	} else {
		if (!lunaReq->isProxyRequest()) {
			bool retVal = LSCallOneReply(handle, uri, json, &handleResponse, this, &lsToken, lserr);
			MojLsErrCheck(retVal, lserr);
		} else {
			bool retVal = LSCallFromApplicationOneReply(handle, uri, json, lunaReq->getRequester(), &handleResponse, this, &lsToken, lserr);
			MojLsErrCheck(retVal, lserr);
		}
	}
	tokenOut = (Token) lsToken;

	return MojErrNone;
}

MojErr MojLunaService::cancelImpl(MojServiceRequest* req)
{
	MojAssert(req);
	MojAssert(m_service || m_handle);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	MojLunaRequest* lunaReq = static_cast<MojLunaRequest*>(req);
	if (req->numRepliesExpected() > 1 && !lunaReq->cancelled()) {
		LSMessageToken lsToken = (LSMessageToken) req->token();
		MojAssert(lsToken != LSMESSAGE_TOKEN_INVALID);
		MojLunaErr lserr;
		LSHandle* handle = getHandle(lunaReq->onPublic());
		bool cancelled = LSCallCancel(handle, lsToken, lserr);
		MojLsErrCheck(cancelled, lserr);
		lunaReq->cancelled(true);
	}
	return MojErrNone;
}

MojErr MojLunaService::dispatchReplyImpl(MojServiceRequest* req, MojServiceMessage *msg, MojObject& payload, MojErr errCode)
{
	MojAssert(req);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	// don't automatically cancel a subscription if the payload has "subscribed":true
	bool subscribed = false;
	payload.get(_T("subscribed"), subscribed);

	// remove if there are more responses coming. due to the vagaries of the luna-service protocol,
	// we never really know for sure, but this is a good guess.
	bool remove = (req->numReplies() + 1 >= req->numRepliesExpected()) || (errCode != MojErrNone && !subscribed);
	MojErr err = req->dispatchReply(msg, payload, errCode);
	MojErrCatchAll(err) {
		remove = true;
	}
	if (remove) {
		if (req->numRepliesExpected() == 1) {
			bool found = false;
			err = removeRequest(req, found);
			MojErrCheck(err);
		} else {
			err = cancel(req);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojLunaService::enableSubscriptionImpl(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	MojString token;
	MojErr err = token.format(_T("%d"), msg->token());
	MojErrCheck(err);

    MojLunaErr lserr;
    MojLunaMessage* lsMessage = static_cast<MojLunaMessage*>(msg);
    LSHandle* handle = LSMessageGetConnection(lsMessage->impl());
    bool retVal = LSSubscriptionAdd(handle, token, lsMessage->impl(), lserr);
    MojLsErrCheck(retVal, lserr);

    return MojErrNone;
}

MojErr MojLunaService::removeSubscriptionImpl(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexLocked(m_mutex);
	MojLogTrace(s_log);

	/*MojString token;
	MojErr err = token.format(_T("%d"), msg->token());
	MojErrCheck(err);

    MojLunaErr lserr;
    MojLunaMessage* lsMessage = static_cast<MojLunaMessage*>(msg);
    LSHandle* handle = LSMessageGetConnection(lsMessage->impl());

    LSSubscriptionIter* iter = NULL;
    bool retVal = LSSubscriptionAcquire(handle, token, &iter, lserr);
    MojLsErrCheck(retVal, lserr);
    while (LSSubscriptionHasNext(iter)) {
    	LSMessage* msg = LSSubscriptionNext(iter);
    	MojAssert(msg);
		MojUnused(msg);
    	LSSubscriptionRemove(iter);
    }
    LSSubscriptionRelease(iter);*/

    return MojErrNone;
}

bool MojLunaService::handleCancel(LSHandle* sh, LSMessage* msg, void* ctx)
{
	MojAssert(sh && msg && ctx);
	MojLogTrace(s_log);
	MojLunaService* service = static_cast<MojLunaService*>(ctx);

	MojRefCountedPtr<MojLunaMessage> request(new MojLunaMessage(service, msg));
	MojAllocCheck(request.get());
	MojLogInfo(s_log, _T("cancel received: %s"), request->payload());
	MojErr err = service->MojService::handleCancel(request.get());
	MojErrCatchAll(err);

	return true;
}

bool MojLunaService::handleRequest(LSHandle* sh, LSMessage* msg, void* ctx)
{
	MojAssert(sh && msg && ctx);
	MojLogTrace(s_log);
	MojLunaService::Category* category = static_cast<Category*>(ctx);
	MojLunaService* service = static_cast<MojLunaService*>(category->m_service);

	MojRefCountedPtr<MojLunaMessage> request(new MojLunaMessage(service, msg, category));
	MojAllocCheck(request.get());
	MojLogInfo(s_log, _T("request received: %s"), request->payload());

	MojErr reqErr;
	MojErr err = reqErr = request->processSubscriptions();
	MojErrCatchAll(err) {
		(void) request->replyError(reqErr);
		return true;
	}

	err = service->MojService::handleRequest(request.get());
	MojErrCatchAll(err);

	return true;
}

bool MojLunaService::handleResponse(LSHandle* sh, LSMessage* msg, void* ctx)
{
	MojAssert(sh && msg && ctx);
	MojLogTrace(s_log);
	MojLunaService* service = static_cast<MojLunaService*>(ctx);

	MojRefCountedPtr<MojLunaMessage> request(new MojLunaMessage(service, msg, NULL, true));
	MojAllocCheck(request.get());
	MojLogInfo(s_log, _T("response received: %s"), request->payload());
	MojErr err = service->handleReply(request.get());
	MojErrCatchAll(err);

	return true;
}
