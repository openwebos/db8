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


#ifndef MOJSERVICEMESSAGE_H_
#define MOJSERVICEMESSAGE_H_

#include "core/MojCoreDefs.h"
#include "core/MojMessage.h"
#include "core/MojService.h"

class MojServiceMessage : public MojMessage
{
public:
	typedef MojUInt32 Token;
	typedef MojSignal<MojServiceMessage*> CancelSignal;

	static const MojChar* const ReturnValueKey;
	static const MojChar* const ErrorCodeKey;
	static const MojChar* const ErrorTextKey;

	virtual ~MojServiceMessage();

	MojSize numReplies() const { return m_numReplies; }
	MojService::Category* serviceCategory() const { return m_category; }
	bool subscribed() const { return m_subscribed; }
	bool fixmode() const { return m_fixmode; }
	void fixmode(bool bVal) { m_fixmode = bVal; }
	virtual MojObjectVisitor& writer() = 0;
	virtual const MojChar* appId() const = 0;
	virtual const MojChar* category() const = 0;
	virtual const MojChar* method() const = 0;
	virtual const MojChar* senderAddress() const = 0;
	virtual const MojChar* senderId() const= 0;
	virtual const MojChar* senderName() const;
	virtual MojErr payload(MojObjectVisitor& visitor) const = 0;
	virtual MojErr payload(MojObject& objOut) const = 0;
	virtual Token token() const = 0;
	virtual bool hasData() const = 0;

	void notifyCancel(CancelSignal::SlotRef cancelHandler);

	MojErr reply();
	MojErr reply(const MojObject& payload);
	MojErr replySuccess();
	MojErr replySuccess(MojObject& payload);
	MojErr replyError(MojErr code);
	MojErr replyError(MojErr code, const MojChar* text);


protected:
	friend class MojService;

	MojServiceMessage(MojService* service, MojService::Category* category);
	MojErr close();
	MojErr dispatchCancel() { return m_cancelSignal.fire(this); }
	MojErr enableSubscription();
	virtual MojErr handleCancel();
	virtual MojErr replyImpl() = 0;
	virtual MojErr dispatch();
	void dispatchMethod(MojService::DispatchMethod method) { m_dispatchMethod = method; }

	MojSize m_numReplies;
	CancelSignal m_cancelSignal;
	MojService* m_service;
	MojService::Category* m_category;
	MojService::DispatchMethod m_dispatchMethod;
	bool m_subscribed;
	bool m_fixmode;
};

#endif /* MOJSERVICEMESSAGE_H_ */
