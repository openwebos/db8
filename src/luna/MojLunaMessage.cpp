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


#include "luna/MojLunaMessage.h"
#include "luna/MojLunaErr.h"
#include "luna/MojLunaService.h"
#include "core/MojJson.h"
#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"

MojLunaMessage::MojLunaMessage(MojService* service)
: MojServiceMessage(service, NULL),
  m_msg(NULL),
  m_response(false),
  m_token(LSMESSAGE_TOKEN_INVALID)
{
}

MojLunaMessage::MojLunaMessage(MojService* service, LSMessage* msg, MojService::Category* category, bool response)
: MojServiceMessage(service, category),
  m_msg(msg),
  m_response(response),
  m_token(LSMESSAGE_TOKEN_INVALID)
{
	MojAssert(msg);

	LSMessageRef(msg);
}

MojLunaMessage::~MojLunaMessage()
{
	MojErr err = close();
	MojErrCatchAll(err);

	releaseMessage();
}

const MojChar* MojLunaMessage::queue() const
{
	if (m_response) {
		return _T("__reply__");
	} else {
		return senderAddress();
	}
}

MojErr MojLunaMessage::payload(MojObjectVisitor& visitor) const
{
	MojErr err = MojJsonParser::parse(visitor, payload());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojLunaMessage::payload(MojObject& objOut) const
{
	if (m_payload.undefined()) {
		MojObjectBuilder builder;
		MojErr err = payload(builder);
		MojErrCheck(err);
		m_payload = builder.object();
	}
	objOut = m_payload;

	return MojErrNone;
}

MojServiceMessage::Token MojLunaMessage::token() const
{
	if (m_token != LSMESSAGE_TOKEN_INVALID)
		return m_token;
	if (m_response)
		return (Token) LSMessageGetResponseToken(m_msg);
	return (Token) LSMessageGetToken(m_msg);
}

MojErr MojLunaMessage::processSubscriptions()
{
	MojObject payload;
	MojErr err = this->payload(payload);
	MojErrCheck(err);

	// TODO: generalize this and make it part of method registration
	bool subscribe = false;
	if (!MojStrCaseCmp(method(), _T("watch"))) {
		subscribe = true;
	} else {
		if (!payload.get(_T("watch"), subscribe)) {
			payload.get(_T("subscribe"), subscribe);
		}
	}
	if (subscribe) {
		err = enableSubscription();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojLunaMessage::replyImpl()
{
	MojAssert(hasData());

	const MojChar* json = m_writer.json();
	MojLogInfo(MojLunaService::s_log, _T("response sent: %s"), json);

	MojLunaErr lserr;
	bool retVal = LSMessageRespond(m_msg, json, lserr);
	MojLsErrCheck(retVal, lserr);

	return MojErrNone;
}

void MojLunaMessage::reset(LSMessage* msg)
{
	releaseMessage();
	m_msg = msg;
}

void MojLunaMessage::releaseMessage()
{
	if (m_msg)
		LSMessageUnref(m_msg);
}
