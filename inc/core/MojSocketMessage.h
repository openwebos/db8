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


#ifndef MOJSOCKETMESSAGE_H_
#define MOJSOCKETMESSAGE_H_

#include "core/MojServiceMessage.h"
#include "core/MojObject.h"

class MojSocketService;

class MojSocketMessage : public MojServiceMessage
{
public:
	MojSocketMessage();
	~MojSocketMessage();


	virtual const MojChar* category() const;
	virtual const MojChar* method() const;
	virtual const MojChar* payload() const;

	virtual bool isResponse() const;
	virtual MojErr messageToken(Token& tokenOut) const;
	virtual MojErr senderToken(Token& tokenOut) const;
	static MojErr senderToken(MojSockT socket, Token& tokenOut);

	MojErr init(const MojString& category, const MojString& method, const MojChar* jsonPayload, const Token& tok);
	void setMethod(const MojString& method) { m_method = method; }
	void setToken(const Token& token) { m_token = token; }

	MojErr extract(const guint8* data, gsize len, gsize& dataConsumed);
	gsize serializedSize() const;
	MojErr serialize(MojDataWriter& writer) const;

	void setReplyInfo(MojSockT sock, MojSocketService* service);
	MojSockT getSocket() const { return m_socket; }

protected:
	virtual MojErr sendResponse(const MojChar* payload);

private:
	enum {
		None = 0,
		Response = 1
	};
	typedef guint32 Flags;

	MojErr extractString(MojDataReader& reader, MojString& strOut);
	MojErr serializeString(MojDataWriter& writer, const MojString& str) const;

	MojString m_category;
	MojString m_method;
	MojString m_payloadJson;
	Token m_token;
	Flags m_flags;

	MojSockT m_socket;
	MojSocketService* m_service;
};

#endif /* MOJSOCKETMESSAGE_H_ */
