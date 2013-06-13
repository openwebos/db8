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


#ifndef MOJSERVICEREQUEST_H_
#define MOJSERVICEREQUEST_H_

#include "core/MojCoreDefs.h"
#include "core/MojSignal.h"

class MojServiceRequest : public MojSignalHandler
{
public:
	typedef MojSignal<MojObject&, MojErr> ReplySignal;
	typedef MojSignal<MojServiceMessage *, MojObject&, MojErr> ExtendedReplySignal;

	typedef MojUInt32 Token;

	static const MojUInt32 Unlimited = MojUInt32Max;

	virtual ~MojServiceRequest();
	virtual MojObjectVisitor& writer() = 0;

	MojUInt32 numReplies() const { return m_numReplies; }
	MojUInt32 numRepliesExpected() const { return m_numRepliesExpected; }
	Token token() const { return m_token; }

	/* Simple/original signals */
	MojErr send(ReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
				MojUInt32 numReplies = 1);
	MojErr send(ReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
				const MojObject& payload, MojUInt32 numReplies = 1);

	/* Extended signals */
	MojErr send(ExtendedReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
				MojUInt32 numReplies = 1);
	MojErr send(ExtendedReplySignal::SlotRef handler, const MojChar* service, const MojChar* method,
				const MojObject& payload, MojUInt32 numReplies = 1);

protected:
	MojServiceRequest(MojService* service);

private:
	friend class MojService;
	friend class MojLunaService;

	MojErr dispatchReply(MojServiceMessage *msg, MojObject& payload, MojErr msgErr);
	virtual MojErr handleCancel();

	MojService* m_service;
	MojUInt32 m_numReplies;
	MojUInt32 m_numRepliesExpected;
	Token m_token;
	ReplySignal m_signal;
	ExtendedReplySignal m_extSignal;
};

#endif /* MOJSERVICEREQUEST_H_ */
