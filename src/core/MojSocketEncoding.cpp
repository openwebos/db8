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


#include "core/MojSocketEncoding.h"
#include "core/MojDataSerialization.h"
#include <sys/socket.h>

#define MojParserRecvCheck(C) if (C == 0) { MojErrThrow(MojErrSocket); } \
							  else if (C < 0) { return (errno == EAGAIN) ? MojErrNone : MojErrSocket; }

MojSocketMessageHeader::MojSocketMessageHeader()
{
	reset();
}

MojSocketMessageHeader::MojSocketMessageHeader(const MojSocketMessage& message)
: m_version(s_protocolVersion),
  m_messageLen(message.serializedSize())
{
}

void MojSocketMessageHeader::reset()
{
	m_version = 0;
	m_messageLen = 0;
}

MojErr MojSocketMessageHeader::write(MojDataWriter& writer) const
{
	MojErr err = writer.writeUInt32(m_version);
	MojErrCheck(err);
	err = writer.writeUInt32(m_messageLen);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketMessageHeader::read(MojDataReader& reader)
{
	MojErr err = reader.readUInt32(m_version);
	MojErrCheck(err);

	if (m_version != s_protocolVersion) {
		MojErrThrow(MojErrInvalidMsg);
	}

	err = reader.readUInt32(m_messageLen);
	MojErrCheck(err);
	if (m_messageLen > s_maxMsgSize || m_messageLen <= 0) {
		MojErrThrow(MojErrInvalidMsg);
	}

	return MojErrNone;
}

MojSocketMessageParser::MojSocketMessageParser()
: m_headerBytes(0),
  m_messageData(NULL),
  m_messageBytesRead(0)
{
	MojZero(m_headerBuf, sizeof(m_headerBuf));
}

MojSocketMessageParser::~MojSocketMessageParser()
{
	if (m_messageData) {
		delete[] m_messageData;
	}
}

void MojSocketMessageParser::reset()
{
	MojZero(m_headerBuf, sizeof(m_headerBuf));
	m_headerBytes = 0;
	m_header.reset();
	if (m_messageData) {
		delete[] m_messageData;
		m_messageData = NULL;
	}
	m_messageBytesRead = NULL;
}

bool MojSocketMessageParser::inProgress() const
{
	if (m_headerBytes == 0)
		return false;
	if (m_headerBytes < sizeof(m_header))
		return true;
	if  (m_messageBytesRead < m_header.messageLen())
		return true;

	return false;
}

MojErr MojSocketMessageParser::readFromSocket(MojSockT sock, MojRefCountedPtr<MojSocketMessage>& msgOut, bool& completeOut)
{
	MojErr err = MojErrNone;
	MojAssert(sock != -1);
	completeOut = false;

	while (m_headerBytes < sizeof(m_header)) {
		MojInt32 sz = recv(sock, m_headerBuf+m_headerBytes, sizeof(m_header) - m_headerBytes, 0);
		MojParserRecvCheck(sz);
		m_headerBytes += sz;
		if (m_headerBytes == sizeof(m_header)) {
			MojDataReader reader(m_headerBuf, m_headerBytes);
			err = m_header.read(reader);
			MojErrCheck(err);
			m_messageData = new MojByte[m_header.messageLen()];
			MojAllocCheck(m_messageData);
		}
	}

	while (m_messageBytesRead < m_header.messageLen()) {
		MojInt32 sz = recv(sock, m_messageData + m_messageBytesRead, m_header.messageLen() - m_messageBytesRead, 0);
		MojParserRecvCheck(sz);
		m_messageBytesRead += sz;
	}

	msgOut.reset(new MojSocketMessage);
	if (!msgOut.get()) {
		err = MojErrNoMem;
		MojErrGoto(err, done);
	}
	MojSize cbConsumed;
	err = msgOut->extract(m_messageData, m_header.messageLen(), cbConsumed);
	MojErrGoto(err, done);
	if (m_header.messageLen() != cbConsumed) {
		err = MojErrInvalidMsg;
		MojErrGoto(err, done);
	}

	completeOut = true;

done:
	if (m_messageData) {
		delete[] m_messageData;
		m_messageData = NULL;
	}

	return err;
}

MojErr MojSocketMessageEncoder::writeToBuffer(MojDataWriter& writer)
{
	// initialize header
	MojSocketMessageHeader header(m_msg);
	if (header.messageLen() > MojSocketMessageHeader::s_maxMsgSize) {
		MojErrThrow(MojErrInvalidMsg);
	}

	// reserve buffer space
	MojSize reserve = header.messageLen() + sizeof(header);
	MojErr err = writer.reserve(reserve);
	MojErrCheck(err);

	// write header and message to buffer
	err = header.write(writer);
	MojErrCheck(err);
	err = m_msg.serialize(writer);
	MojErrCheck(err);

	return MojErrNone;
}
