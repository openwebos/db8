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


#include "core/MojServiceRequest.h"
#include "core/MojService.h"

MojServiceRequest::MojServiceRequest(MojService* service)
: m_service(service),
  m_numReplies(0),
  m_numRepliesExpected(0),
  m_token(0),
  m_signal(this),
  m_extSignal(this)
{
}

MojServiceRequest::~MojServiceRequest()
{
}

MojErr MojServiceRequest::send(ReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
							   MojUInt32 numReplies)
{
	MojAssert(service && method && numReplies);
	// reusing a request is not allowed
	MojAssert(m_token == 0);

	m_numRepliesExpected = numReplies;
	m_signal.connect(handler);

	MojErr err = m_service->send(this, service, method, m_token);
	MojErrCheck(err);
	err = writer().reset();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceRequest::send(ReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
							   const MojObject& payload, MojUInt32 numReplies)
{
	MojAssert(service && method && numReplies);

	MojErr err = payload.visit(writer());
	MojErrCheck(err);
	err = send(handler, service, method, numReplies);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceRequest::send(ExtendedReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
							   MojUInt32 numReplies)
{
	MojAssert(service && method && numReplies);
	// reusing a request is not allowed
	MojAssert(m_token == 0);

	m_numRepliesExpected = numReplies;
	m_extSignal.connect(handler);

	MojErr err = m_service->send(this, service, method, m_token);
	MojErrCheck(err);
	err = writer().reset();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceRequest::send(ExtendedReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
							   const MojObject& payload, MojUInt32 numReplies)
{
	MojAssert(service && method && numReplies);

	MojErr err = payload.visit(writer());
	MojErrCheck(err);
	err = send(handler, service, method, numReplies);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojServiceRequest::dispatchReply(MojServiceMessage *msg, MojObject& payload, MojErr msgErr)
{
	++m_numReplies;
	if (m_extSignal.connected()) {
		MojErr err = m_extSignal.call(msg, payload, msgErr);
		MojErrCheck(err);
	} else {
		MojErr err = m_signal.call(payload, msgErr);
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojServiceRequest::handleCancel()
{
	MojErr err = m_service->cancel(this);
	MojErrCheck(err);

	return MojErrNone;
}
