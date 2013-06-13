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


#ifndef MOJLUNAMESSAGE_H_
#define MOJLUNAMESSAGE_H_

#include "luna/MojLunaDefs.h"
#include "luna/MojLunaService.h"
#include "core/MojJson.h"
#include "core/MojServiceMessage.h"

class MojLunaMessage : public MojServiceMessage
{
public:
	MojLunaMessage(MojService* service);
	MojLunaMessage(MojService* service, LSMessage* msg, MojService::Category* category = NULL, bool response = false);
	~MojLunaMessage();

	virtual MojObjectVisitor& writer() { return m_writer; }
	virtual bool hasData() const { return (!m_writer.json().empty()); }
	virtual const MojChar* appId() const { return LSMessageGetApplicationID(m_msg); }
	virtual const MojChar* category() const { return LSMessageGetCategory(m_msg); }
	virtual const MojChar* method() const { return LSMessageGetMethod(m_msg); }
	virtual const MojChar* senderId() const { return LSMessageGetSenderServiceName(m_msg); }
	virtual const MojChar* senderAddress() const { return LSMessageGetSender(m_msg); }
	virtual const MojChar* queue() const;
	virtual Token token() const;
	virtual MojErr payload(MojObjectVisitor& visitor) const;
	virtual MojErr payload(MojObject& objOut) const;
	const MojChar* payload() const { return LSMessageGetPayload(m_msg); }

	LSMessage* impl() { return m_msg; }
	void reset(LSMessage* msg = NULL);
	bool isPublic() const { return LSMessageIsPublic(reinterpret_cast<MojLunaService*>(m_service)->m_service, m_msg); }

private:
	friend class MojLunaService;

	virtual MojErr replyImpl();
	MojErr processSubscriptions();
	void releaseMessage();

	MojLunaService* lunaService() { return reinterpret_cast<MojLunaService*>(m_service); }
	void token(Token tok) { m_token = tok; }

	mutable MojObject m_payload;
	LSMessage* m_msg;
	bool m_response;
	Token m_token;
	MojJsonWriter m_writer;
};

#endif /* MOJLUNAMESSAGE_H_ */
