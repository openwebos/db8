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


#include "core/MojService.h"
#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"
#include "core/MojMessageDispatcher.h"
#include "core/MojServiceMessage.h"
#include "core/MojServiceRequest.h"
#include "core/MojTime.h"

const MojChar* const MojService::DefaultCategory = _T("/");
MojLogger MojService::s_log(_T("core.messageService"));

MojService::MojService(MojMessageDispatcher* queue)
: m_dispatcher(queue)
{
}

MojService::~MojService()
{
	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojService::open(const MojChar* serviceName)
{
	MojLogTrace(s_log);

	if (serviceName) {
		MojErr err = m_name.assign(serviceName);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojService::close()
{
	MojLogTrace(s_log);

	// cancel all pending subscriptions
	MojErr err = MojErrNone;
	for (SubscriptionMap::ConstIterator i = m_subscriptions.begin(); i != m_subscriptions.end();) {
		SubscriptionKey key = (i++).key();
		MojErr errClose = dispatchCancel(key);
		MojErrAccumulate(err, errClose);
	}
	// if you're hitting this assertion, you probably aren't releasing your message in your cancel handler
	MojAssert(m_subscriptions.empty());

	return err;
}

MojErr MojService::addCategory(const MojChar* categoryName, CategoryHandler* handler)
{
	MojAssert(categoryName && handler);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	MojAssert(!m_categories.contains(categoryName));
	MojRefCountedPtr<Category> category(new Category(this, handler));
	MojAllocCheck(category.get());

	MojString categoryStr;
	MojErr err = categoryStr.assign(categoryName);
	MojErrCheck(err);
	err = m_categories.put(categoryStr, category);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::send(MojServiceRequest* req, const MojChar* service, const MojChar* method, Token& tokenOut)
{
	MojAssert(req && service && method);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	// NOV-130089: sendImpl and addReqest must be atomic
	MojThreadGuard guard(m_mutex);

	MojErr err = sendImpl(req, service, method, tokenOut);
	MojErrCheck(err);
	err = addRequest(req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::cancel(MojServiceRequest* req)
{
	MojAssert(req);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	bool found = false;
	MojErr err = removeRequest(req, found);
	MojErrCheck(err);
	if (found) {
		err = cancelImpl(req);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojService::enableSubscription(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	MojErr err = addSubscription(msg);
	MojErrCheck(err);
	err = enableSubscriptionImpl(msg);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::handleMessage(DispatchMethod method, MojServiceMessage* msg)
{
	if (m_dispatcher) {
		msg->dispatchMethod(method);
		MojErr err = m_dispatcher->schedule(msg);
		MojErrCheck(err);
	} else {
		MojErr err = (this->*method)(msg);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojService::dispatchRequest(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	// get payload
	MojObject payload;
	MojErr reqErr;
	MojErr err = reqErr = msg->payload(payload);
	MojErrCatchAll(err) {
		return msg->replyError(reqErr);
	}
	// get callback
	Category* category = msg->serviceCategory();
	MojAssert(category);
	MojAssert(category && category->m_service == this);
	// invoke
	err = reqErr = category->m_handler->invoke(msg->method(), msg, payload);
	MojErrCatchAll(err) {
		MojString payloadStr;
		(void) payload.toJson(payloadStr);
		MojString errStr;
		(void) MojErrToString(reqErr, errStr);
		MojLogError(s_log, _T("%s (%d) - sender='%s' method='%s' payload='%s'"),
				errStr.data(), (int) reqErr, msg->senderName(), msg->method(), payloadStr.data());

		if (msg->numReplies() == 0) {
			return msg->replyError(reqErr);
		}
	}
	if (msg->numReplies() == 0 && msg->hasData()) {
		return msg->reply();
	}
	return MojErrNone;
}

MojErr MojService::dispatchReply(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	// parse payload
	MojObjectBuilder builder;
	MojObject payload;
	MojErr err = MojErrNone;
	MojInt64 errCode = MojErrNone;
	errCode = err = msg->payload(builder);
	MojErrCatchAll(err);
	if (errCode == MojErrNone)
		payload = builder.object();

	// get errCode
	bool retVal = false;
	if (payload.get(MojServiceMessage::ReturnValueKey, retVal) && !retVal) {
		if (!payload.get(MojServiceMessage::ErrorCodeKey, errCode))
			errCode = MojErrUnknown;
	}
	// find request
	MojRefCountedPtr<MojServiceRequest> req;
	err = getRequest(msg, req);
	MojErrCheck(err);

	// do the dispatch
	err = dispatchReplyImpl(req.get(), msg, payload, (MojErr) errCode);
	MojErrCatchAll(err);

	return MojErrNone;
}

MojErr MojService::dispatchCancel(const SubscriptionKey& key)
{
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	SubscriptionMap::ConstIterator i = m_subscriptions.find(key);
	if (i != m_subscriptions.end()) {
		MojRefCountedPtr<MojServiceMessage> msg = *i;
		guard.unlock();

		MojErr err = msg->dispatchCancel();
		MojErrCatchAll(err);
	}
	return MojErrNone;
}

MojErr MojService::dispatchCancel(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	SubscriptionKey key;
	MojErr err = key.init(msg);
	MojErrCheck(err);
	err = dispatchCancel(key);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::addRequest(MojServiceRequest* req)
{
	MojAssert(req);
	MojAssertMutexLocked(m_mutex);
	MojLogTrace(s_log);

	MojErr err = m_requests.put(req->token(), req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::getRequest(MojServiceMessage* msg, MojRefCountedPtr<MojServiceRequest>& reqOut)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	if (!m_requests.get(msg->token(), reqOut)) {
		MojErrThrow(MojErrResponseHandlerNotFound);
	}
	return MojErrNone;
}

MojErr MojService::removeRequest(MojServiceRequest* req, bool& foundOut)
{
	MojAssert(req);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	MojErr err = m_requests.del(req->token(), foundOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::addSubscription(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	SubscriptionKey key;
	MojErr err = key.init(msg);
	MojErrCheck(err);

	MojThreadGuard guard(m_mutex);
	MojAssert(!m_subscriptions.contains(key));
	err = m_subscriptions.put(key, msg);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::removeSubscription(MojServiceMessage* msg)
{
	MojAssert(msg);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);

	SubscriptionKey key;
	MojErr err = key.init(msg);
	MojErrCheck(err);

	MojThreadGuard guard(m_mutex);
	bool found = false;
	err = m_subscriptions.del(key, found);
	MojErrCheck(err);
	MojAssert(found);
	err = removeSubscriptionImpl(msg);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::getCategory(const MojChar* name, MojRefCountedPtr<Category>& catOut)
{
	MojAssert(name);
	MojAssertMutexUnlocked(m_mutex);
	MojLogTrace(s_log);
	MojThreadGuard guard(m_mutex);

	if (!m_categories.get(name, catOut)) {
		MojErrThrow(MojErrCategoryNotFound);
	}
	return MojErrNone;
}

MojErr MojService::CategoryHandler::addMethod(const MojChar* methodName, Callback callback, bool pub, const MojChar* schemaJson)
{
	MojAssert(methodName && callback);

	MojString str;
	MojErr err = str.assign(methodName);
	MojErrCheck(err);
	MojRefCountedPtr<MojSchema> schema;
	if (schemaJson) {
		MojObject schemaObj;
		err = schemaObj.fromJson(schemaJson);
		MojErrCheck(err);

		// add implicit $activity property
		MojObject properties;
		schemaObj.get(MojSchema::PropertiesKey, properties);
		MojObject activityProp;
		err = activityProp.putBool(MojSchema::OptionalKey, true);
		MojErrCheck(err);
		err = properties.put(_T("$activity"), activityProp);
		MojErrCheck(err);
		err = schemaObj.put(MojSchema::PropertiesKey, properties);
		MojErrCheck(err);

		schema.reset(new MojSchema);
		MojAllocCheck(schema.get());
		err = schema->fromObject(schemaObj);
		MojErrCheck(err);
	}
	MojAssert(!m_callbackMap.contains(str));
	err = m_callbackMap.put(str, CallbackInfo(callback, schema.get(), pub));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::CategoryHandler::addMethods(const Method* methods, bool pub)
{
	MojAssert(methods);

	for (const Method* method = methods; method->m_name != NULL; method++) {
		MojErr err = addMethod(method->m_name, method->m_callback, pub);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojService::CategoryHandler::addMethods(const SchemaMethod* methods, bool pub)
{
	MojAssert(methods);

	for (const SchemaMethod* method = methods; method->m_name != NULL; method++) {
		MojErr err = addMethod(method->m_name, method->m_callback, pub, method->m_schemaJson);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojService::CategoryHandler::invoke(const MojChar* method, MojServiceMessage* msg, MojObject& payload)
{
	MojAssert(method && msg);
	MojLogTrace(s_log);

	MojTime startTime;
	MojErr err = MojGetCurrentTime(startTime);
	MojErrCheck(err);

	// lookup callback
	CallbackInfo cb;
	if (!m_callbackMap.get(method, cb)) {
		MojErrThrow(MojErrMethodNotFound);
	}
	// validate schema
	if (cb.m_schema.get()) {
		MojSchema::Result res;
		err = cb.m_schema->validate(payload, res);
		MojErrCheck(err);
		if (!res.valid()) {
			MojErrThrowMsg(MojErrInvalidArg, _T("invalid parameters: caller='%s' error='%s'"), msg->senderName(), res.msg().data());
		}
	}
	// invoke method
	err = invoke(cb.m_callback, msg, payload);
	MojErrCheck(err);

	// log timing
	MojTime endTime;
	err = MojGetCurrentTime(endTime);
	MojErrCheck(err);
	MojLogInfo(s_log, _T("%s invoked: %.3fms"), method, (double) (endTime.microsecs() - startTime.microsecs()) / 1000);

	return MojErrNone;
}

MojErr MojService::CategoryHandler::invoke(Callback method, MojServiceMessage* msg, MojObject& payload)
{
	MojErr err = (this->*method)(msg, payload);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::SubscriptionKey::init(const MojServiceMessage* msg)
{
	MojAssert(msg);

	MojErr err = push((MojInt64) msg->token());
	MojErrCheck(err);
	err = pushString(msg->senderAddress());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::SubscriptionKey::init(const MojChar* sender)
{
	MojErr err = pushString(sender);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojService::SubscriptionKey::sender(MojString& senderOut) const
{
	bool valid = false;
	MojErr err = at(0, senderOut, valid);
	MojErrCheck(err);
	MojAssert(valid);
	MojUnused(valid);

	return MojErrNone;
}

MojService::Token MojService::SubscriptionKey::token() const
{
	MojInt64 tok;
	bool valid = at(1, tok);
	MojAssert(valid);
	MojUnused(valid);

	return (Token) tok;
}
