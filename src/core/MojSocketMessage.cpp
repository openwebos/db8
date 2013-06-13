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


#include "core/MojSocketMessage.h"
#include "core/MojSocketService.h"
#include "core/MojDataSerialization.h"
#include <sys/socket.h>

MojSocketMessage::MojSocketMessage()
: m_flags(None),
  m_socket(-1),
  m_service(NULL)
{
}

MojSocketMessage::~MojSocketMessage()
{
	(void) close();
}

MojErr MojSocketMessage::clone(MojAutoPtr<MojServiceMessage>& msgOut)
{
	msgOut.reset(new MojSocketMessage);
	MojAllocCheck(msgOut.get());
	static_cast<MojSocketMessage&>(*msgOut) = *this;

	return MojErrNone;
}

MojSocketMessage& MojSocketMessage::operator=(const MojSocketMessage& rhs)
{
	m_category  = rhs.m_category;
	m_method = rhs.m_method;
	m_payloadJson = rhs.m_payloadJson;
	m_token = rhs.m_token;
	m_flags = rhs.m_flags;

	m_socket = rhs.m_socket;
	m_service = rhs.m_service;
	m_numReplies = rhs.m_numReplies;

	return *this;
}

void MojSocketMessage::setReplyInfo(MojSockT sock, MojSocketService* service)
{
	m_socket = sock;
	m_service = service;
}

MojErr MojSocketMessage::serialize(MojDataWriter& writer) const
{
	MojErr err = writer.writeUInt32(m_flags);
	MojErrCheck(err);
	err = writer.writeInt64(m_token.intValue());
	MojErrCheck(err);
	err = serializeString(writer, m_category);
	MojErrCheck(err);
	err = serializeString(writer, m_method);
	MojErrCheck(err);
	err = serializeString(writer, m_payloadJson);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketMessage::serializeString(MojDataWriter& writer, const MojString& str) const
{
	MojUInt32 len = str.length();
	MojErr err = writer.writeUInt32(len);
	MojErrCheck(err);
	if (len > 0) {
		err = writer.writeChars(str, len);
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojSize MojSocketMessage::serializedSize() const
{
	return sizeof(Flags) +    // flags
	sizeof(MojInt64) +        // token
	sizeof(MojUInt32) +       // category length
	m_category.length() +     // category
	sizeof(MojUInt32) +       // method length
	m_method.length() +       // method
	sizeof(MojUInt32) +       // payload length
	m_payloadJson.length();   // payload
}

MojErr MojSocketMessage::extract(const MojByte* data, MojSize len, MojSize& dataConsumed)
{
	MojDataReader reader(data, len);

	// flags
	MojErr err = reader.readUInt32(m_flags);
	MojErrCheck(err);

	// token
	MojInt64 tok;
	err = reader.readInt64(tok);
	MojErrCheck(err);
	m_token = Token(tok);

	err = extractString(reader, m_category);
	MojErrCheck(err);
	err = extractString(reader, m_method);
	MojErrCheck(err);
	err = extractString(reader, m_payloadJson);
	MojErrCheck(err);

	dataConsumed = reader.pos() - reader.begin();
	return MojErrNone;
}

MojErr MojSocketMessage::extractString(MojDataReader& reader, MojString& strOut)
{
	MojUInt32 len;
	MojErr err = reader.readUInt32(len);
	MojErrCheck(err);
	if (len) {
		if (reader.available() < len) {
			MojErrThrow(MojErrUnexpectedEof);
		}
		err = strOut.assign((const MojChar*)reader.pos(), len);
		MojErrCheck(err);
		err = reader.skip(len);
		MojErrCheck(err);
	} else {
		strOut.clear();
	}

	return MojErrNone;
}

MojErr MojSocketMessage::init(const MojString& category, const MojString& method, const MojChar* jsonPayload, const Token& tok)
{
	m_category = category;
	m_method = method;
	m_token = tok;
	MojErr err = m_payloadJson.assign(jsonPayload);
	MojErrCheck(err);

	return MojErrNone;
}

const MojChar* MojSocketMessage::category() const
{
	return m_category;
}

const MojChar* MojSocketMessage::method() const
{
	return m_method;
}

const MojChar* MojSocketMessage::payload() const
{
	return m_payloadJson;
}

bool MojSocketMessage::isResponse() const
{
	return m_flags & Response;
}

MojErr MojSocketMessage::messageToken(Token& tokenOut) const
{
	MojAssert(!m_token.null());
	tokenOut = m_token;
	return MojErrNone;
}

MojErr MojSocketMessage::senderToken(Token& tokenOut) const
{
	return senderToken(m_socket, tokenOut);
}

MojErr MojSocketMessage::senderToken(MojSockT socket, Token& tokenOut)
{
	MojAssert(socket != -1);
	tokenOut = Token((MojInt64)socket);
	return MojErrNone;
}

MojErr MojSocketMessage::sendResponse(const MojChar* payload)
{
	MojAssert(m_socket != -1);
	MojAssert(m_service);
	MojAssert(!m_token.null());
	MojAssert(!m_method.empty());
	MojAssert((m_flags & Response) == 0);

	// format a response message
	MojSocketMessage resp;
	MojErr err = resp.init(m_category, m_method, payload, m_token);
	MojErrCheck(err);
	resp.m_flags |= Response;

	// pass response to service
	err = m_service->sendMessage(m_socket, resp);
	MojErrCheck(err);

	return MojErrNone;
}
