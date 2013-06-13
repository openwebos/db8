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


#include "core/MojServiceMessage.h"
#include "core/MojJson.h"

const MojChar* const MojServiceMessage::ReturnValueKey = _T("returnValue");
const MojChar* const MojServiceMessage::ErrorCodeKey = _T("errorCode");
const MojChar* const MojServiceMessage::ErrorTextKey = _T("errorText");

MojServiceMessage::MojServiceMessage(MojService* service, MojService::Category* category)
: m_numReplies(0),
  m_cancelSignal(this),
  m_service(service),
  m_category(category),
  m_dispatchMethod(NULL),
  m_subscribed(false),
  m_fixmode(false)
{
	MojAssert(service);
}

MojServiceMessage::~MojServiceMessage()
{
}

const MojChar* MojServiceMessage::senderName() const
{
	const MojChar* name = appId();
	if (name == NULL) {
		name = senderId();
		if (name == NULL)
			name = _T("");
	}
	return name;
}

void MojServiceMessage::notifyCancel(CancelSignal::SlotRef cancelHandler)
{
	// must be called before first reply
	MojAssert(m_numReplies == 0);

	m_cancelSignal.connect(cancelHandler);
}

MojErr MojServiceMessage::reply()
{
	MojErr err = replyImpl();
	MojErrCheck(err);
	err = writer().reset();
	MojErrCheck(err);

	if (++m_numReplies == 1) {
		if (m_cancelSignal.connected()) {
			err = enableSubscription();
			MojErrCheck(err);
		} else {
			// remove subscription
			err = handleCancel();
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojServiceMessage::reply(const MojObject& payload)
{
	MojErr err = payload.visit(writer());
	MojErrCheck(err);
	err = reply();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceMessage::replySuccess()
{
	MojObject payload;
	return replySuccess(payload);
}

MojErr MojServiceMessage::replySuccess(MojObject& payload)
{
	MojErr err = payload.putBool(ReturnValueKey, true);
	MojErrCheck(err);
	err = reply(payload);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceMessage::replyError(MojErr code)
{
	MojString str;
	MojErr err = MojErrToString(code, str);
	MojErrCheck(err);
	err = replyError(code, str);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceMessage::replyError(MojErr code, const MojChar* text)
{
	MojAssert(code != MojErrNone && text);

	MojObject payload;
	MojErr err = payload.putBool(ReturnValueKey, false);
	MojErrCheck(err);
	err = payload.putInt(ErrorCodeKey, (MojInt64) code);
	MojErrCheck(err);
	err = payload.putString(ErrorTextKey, text);
	MojErrCheck(err);

	// reset the writer first, then reply with this payload
	err = writer().reset();
	MojErrCheck(err);
	err = reply(payload);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceMessage::close()
{
	if (m_subscribed) {
		// remove subscription when we are deleted
		MojErr err = m_service->removeSubscription(this);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojServiceMessage::enableSubscription()
{
	if (!m_subscribed) {
		MojErr err = m_service->enableSubscription(this);
		MojErrCheck(err);
		m_subscribed = true;
	}
	return MojErrNone;
}

MojErr MojServiceMessage::handleCancel()
{
	MojAssert(m_service);
	if (m_subscribed) {
		MojErr err = m_service->removeSubscription(this);
		MojErrCheck(err);
		m_subscribed = false;
	}

	return MojErrNone;
}

MojErr MojServiceMessage::dispatch()
{
	MojAssert(m_service && m_dispatchMethod);

	MojErr err = (m_service->*m_dispatchMethod)(this);
	MojErrCheck(err);

	return MojErrNone;
}
