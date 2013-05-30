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


#ifndef MOJSOCKETENCODING_H_
#define MOJSOCKETENCODING_H_

#include "core/MojNoCopy.h"
#include "core/MojSocketMessage.h"

class MojSocketMessageHeader
{
public:
	static const guint32 s_protocolVersion = 1;
	static const guint32 s_maxMsgSize = 32768;

	MojSocketMessageHeader();
	MojSocketMessageHeader(const 	MojSocketMessage& message);

	MojErr write(MojDataWriter& writer) const;
	MojErr read(MojDataReader& reader);
	void reset();

	guint32 version() const { return m_version; }
	guint32 messageLen() const { return m_messageLen; }

private:
	guint32 m_version;
	guint32 m_messageLen;
};

class MojSocketMessageParser : private MojNoCopy
{
public:
	MojSocketMessageParser();
	~MojSocketMessageParser();

	MojErr readFromSocket(MojSockT sock, MojRefCountedPtr<MojSocketMessage>& msgOut, bool& completeOut);
	MojErr writeToBuffer(const MojSocketMessage& msg, MojVector<guint8> buffer, const guint8* begin);

	void reset();
	bool inProgress() const;

private:
	guint8 m_headerBuf[sizeof(MojSocketMessageHeader)];
	gsize m_headerBytes;
	MojSocketMessageHeader m_header;
	guint8* m_messageData;
	gsize m_messageBytesRead;
};

class MojSocketMessageEncoder : private MojNoCopy
{
public:
	MojSocketMessageEncoder(const MojSocketMessage& msg) : m_msg(msg) {}
	MojErr writeToBuffer(MojDataWriter& writer);

private:
	const MojSocketMessage& m_msg;
};

#endif /* MOJSOCKETENCODING_H_ */
