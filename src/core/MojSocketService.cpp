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


#include "core/MojSocketService.h"
#include "core/MojDataSerialization.h"

MojSocketService::SockHandler::SockHandler(MojSocketService& service)
: m_service(service)
{
}

MojSocketService::SockHandler::~SockHandler()
{
	(void) close();
}

MojErr MojSocketService::SocHandler::close()
{
	MojErr err = MojErrNone;
	MojErr errClose = MojSignalHandler::close();
	MojErrAccumulate(err, errClose);
	errClose = m_sock.close();
	MojErrAccumulate(err, errClose);

	return err;
}

MojSocketService::Connection::Connection(MojSocketService& service)
: SockHandler(service),
  m_state(StateInit)
  m_writer(m_writeBuf),
  m_readSlot(this, &Connection::read),
  m_flushSlot(this, &Connection::flush)
{
}

MojErr MojSocketService::Connection::open(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);

	m_sock.open(sock);
	MojErr err = start();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::open(const MojSockAddr& addr)
{
	MojErr err = m_sock.open();
	MojErrCheck(err);
	err = m_sock.connect(addr);
	MojErrCheck(err);
	err = start();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::start()
{
	// todo: set no-delay
	m_state = StateInit;
	MojErr err = writeOpen();
	MojErrCheck(err);
	err = readMsg();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::writeOpen()
{
	MojErr err = writeHeader(sizeof(MojUInt32) * 2, TypeOpen);
	MojErrCheck(err);
	err = m_writer.writeUInt32(Version);
	MojErrCheck(err);
	err = m_writer.writeUInt32(KeepAliveTime);
	MojErrCheck(err);
	err = flush();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::writeClose()
{
	MojErr err = writeHeader(0, TypeClose);
	MojErrCheck(err);
	err = flush();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::writeRequest(Message* msg)
{
	MojSize size = sizeof(MojUInt32) // payload size
			+ app.length() + 1 // app id size
			+ method.length() + 1; // method size
	MojErr err = writeHeader(size, TypeRequest);
	MojErrCheck(err);
	err = m_writer.writeUInt32(payload.size());
	MojErrCheck(err);
	err = m_writer.writeString(app);
	MojErrCheck(err);
	err = m_writer.writeString(method);
	MojErrCheck(err);
	m_writeBuf.append(payload);
	err = flush();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::writeHeader(MojSize size, Type type)
{
	MojSize totalSize = size
			+ sizeof(MojUInt16) // header size
			+ sizeof(MojUInt8) // msg type
			+ sizeof(MojUInt32); // msg id
	MojAssert(totalSize <= MojUInt16Max);

	MojErr err = m_writer.writeUInt16((MojUInt16) totalSize);
	MojErrCheck(err);
	err = m_writer.writeUInt8(type);
	MojErrCheck(err);
	err = m_writer.writeUInt32(m_id++);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Connection::flush(MojSockT sock)
{
	MojAssert(sock == m_sock);

	while (!m_writeBuf.empty()) {
		MojIoVecT vec[IoVecSize];
		MojSize vecSize = 0;
		MojErr err = m_writeBuf.readVec(vec, IoVecSize, vecSize);
		MojErrCheck(err);
		MojSize bytesWritten = 0;
		err = m_sock.writev(vec, size, bytesWritten);
		MojErrCatch(err, MojErrWouldBlock) {
			err = reactor().notifyWritable(sock, m_flushSlot);
			MojErrCheck(err);
			break;
		}
		MojErrCheck(err);
		m_writeBuf.advanceRead(bytesWritten);
	}
	return MojErrNone;
}

MojErr MojSocketService::Connection::read(MojSockT sock)
{
	for (;;) {
		MojIoVecT vec[IoVecSize];
		MojSize vecSize = 0;
		MojErr err = m_readBuf.writeVec(vec, IoVecSize, size);
		MojErrCheck(err);
		MojSize bytesRead = 0;
		err = m_sock.readv(vec, size, bytesRead);
		MojErrCheck(err);
		MojErrCatch(err, MojErrWouldBlock) {
			err = reactor().notifyReadable(sock, m_readSlot);
			MojErrCheck(err);
			break;
		}
		MojErrCheck(err);
		m_readBuf.advanceWrite(bytesRead);
		err = handleRead();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSocketService::Connection::handleRead()
{
	MojErr err = MojErrNone;
	switch (m_state) {
	case StateHeaderSize: {
		if (m_readBuf.size() < sizeof(MojUInt16))
			break;
		err = readHeaderSize();
		MojErrCheck(err);
		// no-break: fall through to header
	}
	case StateHeader: {
		if (m_readBuf.size() < m_expectedSize)
			break;
		err = readHeader();
		MojErrCheck(err);
		// no-break: fall through to payload
	}
	case StatePayload: {
		if (m_readBuf.size() < m_expectedSize)
			break;
		err = readPayload();
		MojErrCheck(er);
		break;
	}
	default:
		MojAssertNotReached();
	}
	return MojErrNone;
}

MojErr MojSocketService::Connection::readHeaderSize()
{
	return MojErrNone;
}

MojErr MojSocketService::Connection::readHeader()
{
	return MojErrNone;
}

MojErr MojSocketService::Connection::readPayload()
{
	return MojErrNone;
}

MojSocketService::Server::Server(MojSocketService& service)
: SockHandler(service),
  m_acceptSlot(&Server::accept)
{
}

MojErr MojSocketService::Server::open(const MojChar* path)
{
	MojAssert(path);

	// unlink path if it already exists
	if (MojStat(path) != MojErrNotFound) {
		MojErr err = MojUnlink(path);
		MojErrCheck(err);
	}
	MojSockAddr addr;
	MojErr err = addr.fromPath(path);
	MojErrCheck(err);
	err = open(addr);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Server::open(const MojChar* host, MojInt32 port)
{
	MojAssert(host);

	MojSockAddr addr;
	MojErr err = addr.fromHostPort(host, port);
	MojErrCheck(err);
	err = open(addr);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Server::open(const MojSockAddr& addr)
{
	MojErr err = m_sock.open(MOJ_PF_LOCAL, MOJ_SOCK_STREAM);
	MojErrCheck(err);
	err = m_sock.bind(addr);
	MojtErrCheck(err);
	err = m_sock.listen(ListenBacklog);
	MojErrCheck(err);
	err = m_sock.setNonblocking(true);
	MojErrCheck(err);
	err = accept(m_sock);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::Server::accept(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);
	MojAssert(sock == m_sock);

	MojSockT newSock = MojInvalidSock;
	MojErr err = m_sock.accept(newSock);
	MojErrCatch(err, MojErrWouldBlock) {
		err = reactor().notifyReadable(sock, m_acceptSlot);
		MojErrCheck(err);
		return MojErrNone;
	}
	MojErrCheck(err);

	ConnectionPtr con(new Connection(m_service));
	MojAllocCheck(con.get());
	err = m_service.addConnection(con);
	MojErrCheck(err);
	err = con->open(newSock);
	MojErrCheck(err);

	return MojErrNone;
}

MojSocketService::MojSocketService(MojReactor& reactor)
: m_nextToken(0),
  m_reactor(reactor)
{
}

MojErr MojSocketService::open(const MojChar* serviceName)
{
	MojErr err = MojService::open(serviceName);
	MojErrCheck(err);

	if (serviceName) {
		// unlink name if it exists
		err = MojUnlink(serviceName);
		MojErrCatch(err, MojErrNotFound);
		MojErrCheck(err);

		m_listenSocket = socket(AF_LOCAL, SOCK_STREAM, 0);
		MojSockErrGoto(m_listenSocket, MojErrSocket, error);

		struct sockaddr_un sockaddr;
		MojZero(&sockaddr, sizeof(sockaddr));
		sockaddr.sun_family = AF_LOCAL;
		if (MojStrLen(serviceName) > sizeof(sockaddr.sun_path)-1) {
			MojErrThrow(MojErrInvalidUri);
		}
		MojStrNCpy(sockaddr.sun_path, serviceName, sizeof(sockaddr.sun_path)-1);

		MojSockErrGoto(bind(m_listenSocket, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr_un)), MojErrSocket, error);
		MojSockErrGoto(listen(m_listenSocket, s_listenBacklog), MojErrSocket, error);

		// register listen socket with reactor
		MojSharedPtr<MojReactor::EventHandler> listenHandler;
		err = listenHandler.reset(new ListenHandler(*this));
		MojErrCheck(err);
		MojAllocCheck(listenHandler.get());

		err = m_reactor.addSock(m_listenSocket, MojReactor::FlagRead, listenHandler);
		MojErrCheck(err);
	}
	return MojErrNone;

error:
	if (m_listenSocket != -1) {
		MojFileClose(m_listenSocket);
		m_listenSocket = -1;
	}
	MojErrThrow(err);
}

MojErr MojSocketService::close()
{
	if (m_listenSocket != -1) {
		MojFileClose(m_listenSocket);
		m_listenSocket = -1;
	}

	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;
	while (!m_conInfoMap.empty()) {
		errClose = handleConnectionClose(m_conInfoMap.begin().key());
		MojErrAccumulate(errClose, err);
	}

	errClose = MojService::close();
	MojErrAccumulate(errClose, err);

	return err;
}

MojErr MojSocketService::dispatchNextMsg()
{
	MojErr err = m_reactor.dispatch();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::addToReactor(MojSockT sock, MojReactor::Flags flags)
{
	MojSharedPtr<MojReactor::EventHandler> connectionHandler;
	MojErr err = connectionHandler.reset(new ConnectionHandler(*this, sock));
	if (!connectionHandler.get()) {
		err = MojErrNoMem;
	}
	MojErrCheck(err);
	err = m_reactor.addSock(sock, flags, connectionHandler);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::acceptConnection()
{
	MojErr err = MojErrNone;
	MojSockT sock;
	int flags = -1;
	struct sockaddr_un cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	MojSharedPtr<ConInfo> info;

	// accept connection
	MojZero(&cliaddr, sizeof(cliaddr));
	sock = accept(m_listenSocket, (struct sockaddr*)&cliaddr, &clilen);
	MojSockErrGoto(sock, MojErrSocket, error);

	// set nonblocking
	flags = fcntl(sock, F_GETFL, 0);
	MojSockErrGoto(flags, MojErrSocket, error);
	MojSockErrGoto(fcntl(sock, F_SETFL, flags | O_NONBLOCK), MojErrSocket, error);

	err = info.reset(new ConInfo(sock));
	MojErrGoto(err, error);
	if (!info.get()) {
		err = MojErrNoMem;
		MojErrGoto(err, error);
	}
	err = info->m_sockName.assign(cliaddr.sun_path);
	MojErrGoto(err, error);

	// add socket to name->socket map
	MojAssert(!m_conMap.contains(info->m_sockName));
	err = m_conMap.put(info->m_sockName, sock);
	MojErrGoto(err, error);

	// add new socket to reactor and wait for input data
	info->m_reactorFlags = MojReactor::FlagRead | MojReactor::FlagHup;
	err = addToReactor(sock, info->m_reactorFlags);
	MojErrCheck(err);

	MojAssert(!m_conInfoMap.contains(sock));
	err = m_conInfoMap.put(sock, info);
	MojErrGoto(err, error);

	return MojErrNone;

error:
	if (info.get() && !info->m_sockName.empty() && m_conMap.contains(info->m_sockName)) {
		bool didDel;
		m_conMap.del(info->m_sockName, didDel); // todo: log error
	}
	if (m_conInfoMap.contains(sock)) {
		bool didDel;
		m_conInfoMap.del(sock, didDel); // todo: log error
	}
	if (sock != -1) {
		MojFileClose(sock);
	}
	MojErrThrow(err);
}

MojErr MojSocketService::handleInput(MojSockT sock)
{
	MojAssert(m_conInfoMap.contains(sock));
	ConInfo* info = (*m_conInfoMap.find(sock)).get();
	MojSocketMessageParser& parser = info->m_parser;
	MojRefCountedPtr<MojSocketMessage> msg;

	bool complete;
	MojErr err = parser.readFromSocket(sock, msg, complete);
	MojErrCheck(err);

	if (!complete) {
		return MojErrNone;
	}

	MojAssert(msg.get());
	msg->setReplyInfo(sock, this);

	err = dispatchMessage(msg.get());
	MojErrGoto(err, reset_parser);

reset_parser:
	parser.reset();
	return err;
}

MojErr MojSocketService::handleOutput(MojSockT sock)
{
	MojAssert(m_conInfoMap.contains(sock));
	ConInfo* info = (*m_conInfoMap.find(sock)).get();

	bool didBlock;
	MojErr err = info->m_outputBuf.write(didBlock);
	MojErrCheck(err);

	// set or clear reactor output flag as necessary
	MojReactor::Flags flagsRequired = info->m_reactorFlags;

	if (didBlock && (info->m_reactorFlags & MojReactor::FlagWrite) == 0) {
		flagsRequired |= MojReactor::FlagWrite;
	} else if (!didBlock && (info->m_reactorFlags & MojReactor::FlagWrite) != 0) {
		flagsRequired &= ~MojReactor::FlagWrite;
	}
	if (flagsRequired != info->m_reactorFlags) {
		err = m_reactor.updateSock(sock, flagsRequired);
		MojErrCheck(err);
		info->m_reactorFlags = flagsRequired;
	}

	return MojErrNone;
}

MojErr MojSocketService::getCon(const MojString& sockName, MojSockT& sockOut)
{
	MojErr err = MojErrNone;
	MojSharedPtr<ConInfo> info;
	int flags = -1;
	sockOut = -1;

	ConMap::ConstIterator i = m_conMap.find(sockName);
	if (i != m_conMap.end()) {
		sockOut = *i;
		return MojErrNone;
	}

	sockOut = socket(AF_LOCAL, SOCK_STREAM, 0);
	MojSockErrGoto(sockOut, MojErrSocket, error);

	err = info.reset(new ConInfo(sockOut));
	MojErrGoto(err, error);
	if (!info.get()) {
		err = MojErrNoMem;
		MojErrGoto(err, error);
	}
	info->m_sockName = sockName;
	info->m_reactorFlags = MojReactor::FlagRead | MojReactor::FlagHup;

	struct sockaddr_un cliaddr;
	struct sockaddr_un servaddr;

	// bind cliaddr to a temp file name
	MojZero(&cliaddr, sizeof(cliaddr));
	MojAssert(L_tmpnam < sizeof(cliaddr.sun_path));
	if (!tmpnam(cliaddr.sun_path)) {
		err = MojErrInternal;
		MojErrGoto(err, error);
	}
	cliaddr.sun_family = AF_LOCAL;
	MojSockErrGoto(bind(sockOut, (struct sockaddr*)&cliaddr, sizeof(cliaddr)), MojErrSocket, error);

	// set nonblocking
	flags = fcntl(sockOut, F_GETFL, 0);
	MojSockErrGoto(flags, MojErrSocket, error);
    MojSockErrGoto(fcntl(sockOut, F_SETFL, flags | O_NONBLOCK), MojErrSocket, error);

    // connect to service name
	MojZero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	MojStrNCpy(servaddr.sun_path, sockName, sizeof(servaddr.sun_path)-1);
	if (connect(sockOut, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		if (errno == EINPROGRESS) {
			// connect blocked - register for output event for connection completion notification
			info->m_reactorFlags |= MojReactor::FlagWrite;
		} else {
			err = MojErrSocket;
			MojErrGoto(err, error);
		}
	}

	err = m_conMap.put(sockName, sockOut);
	MojErrGoto(err, error);

	err = addToReactor(sockOut, info->m_reactorFlags);
	MojErrCheck(err);

	MojAssert(!m_conInfoMap.contains(sockOut));
	err = m_conInfoMap.put(sockOut, info);
	MojErrGoto(err, error);

	return MojErrNone;

error:
	if (sockOut != -1) {
		MojFileClose(sockOut);
		sockOut = -1;
	}
	MojErrThrow(err);
}

MojErr MojSocketService::sendMessage(MojSockT sock, const MojSocketMessage& msg)
{
	MojAssert(m_conInfoMap.contains(sock));
	ConInfo& info = **m_conInfoMap.find(sock);

	MojSocketMessageEncoder encoder(msg);
	MojDataWriter writer;
	MojErr err = encoder.writeToBuffer(writer);
	MojErrCheck(err);
	err = info.m_outputBuf.append(writer.data());
	MojErrCheck(err);

	if ((info.m_reactorFlags & MojReactor::FlagWrite) == 0) {
		// only write if we're not currently blocked and waiting for a write callback
		err = handleOutput(sock);
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojSocketService::handleConnectionClose(MojSockT sock)
{
	bool didDel;
	MojErr err = MojErrNone;

	MojFileClose(sock);

	err = m_reactor.removeSock(sock);
	MojErrCheck(err);

	MojAssert(m_conInfoMap.contains(sock));
	MojSharedPtr<ConInfo> info = *m_conInfoMap.find(sock);
	err = m_conInfoMap.del(sock, didDel);
	MojErrCheck(err);
	MojAssert(didDel);

	// delete token->socket mappings for active requests associated with this connection
	for (MojSet<MojServiceMessage::Token>::ConstIterator i = info->m_activeRequests.begin();
		i != info->m_activeRequests.end();
		i++) {
		err = m_activeRequests.del(*i, didDel);
		MojErrCheck(err);
		MojAssert(didDel);
	}

	// delete name->socket mapping
	err = m_conMap.del(info->m_sockName, didDel);
	MojErrCheck(err);
	MojAssert(didDel);

	// delete cancellation handlers associated with this connection
	MojServiceMessage::Token senderToken;
	err = MojSocketMessage::senderToken(sock, senderToken);
	MojErrCheck(err);
	SubscriptionKey senderKey;
	err = senderKey.init(senderToken);
	MojErrCheck(err);

	do {
		CancelHandlerMap::ConstIterator i = m_cancelHandlers.lowerBound(senderKey);
		if (i == m_cancelHandlers.end())
			break;
		MojServiceMessage::Token sender;
		err = i.key().senderToken(sender);
		MojErrCheck(err);
		if (sender != senderToken)
			break;
		err = dispatchCancel(i.key()); // invalidates iterator
		MojErrCheck(err);
	} while (true);

	return MojErrNone;
}

MojErr MojSocketService::parseUri(const MojChar* uri, MojString& socknameOut, MojString& categoryOut, MojString& methodOut)
{
	//TODO: support categories
	// currently treating uri as sockname/method

	const MojChar* method = uri + MojStrLen(uri) - 1;
	while (method > uri+1 && *(method-1) != _T('/')) {
		method--;
	}
	if (method <= uri+1)  {
		MojErrThrow(MojErrInvalidUri);
	}
	MojErr err = methodOut.assign(method);
	MojErrCheck(err);
	MojSize nameLen = method - uri - 1;
	err = socknameOut.assign(uri, nameLen);
	MojErrCheck(err);

	err = categoryOut.assign(DefaultCategory);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSocketService::sendRequestMsg(const MojChar* uri, const MojChar* json, MojServiceMessage::Token& tokenOut, bool multiResponse)
{
	MojString sockName;
	MojString method;
	MojString category;
	MojErr err = parseUri(uri, sockName, category, method);
	MojErrCheck(err);

	tokenOut = MojServiceMessage::Token(m_nextToken++);
	MojSocketMessage msg;
	err = msg.init(category, method, json, tokenOut);
	MojErrCheck(err);

	MojSockT sock;
	err = getCon(sockName, sock);
	MojErrCheck(err);

	err = sendMessage(sock, msg);
	MojErrCheck(err);

	if (multiResponse) {
		MojAssert(!m_activeRequests.contains(tokenOut));
		err = m_activeRequests.put(tokenOut, sock);
		MojErrCheck(err);
		err = (*m_conInfoMap.find(sock))->m_activeRequests.put(tokenOut);
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojErr MojSocketService::cancelMultiResponseRequest(MojServiceMessage::Token token)
{
	RequestMap::ConstIterator i = m_activeRequests.find(token);
	if (i == m_activeRequests.end()) {
		MojErrThrow(MojErrInvalidToken);
	}
	MojSockT sock = *i;
	MojAssert(sock != -1);

	MojSocketMessage msg;
	MojString method;
	MojErr err = method.assign(_T("cancel"));
	MojErrCheck(err);
	msg.setMethod(method);
	msg.setToken(token);

	err = sendMessage(sock, msg);
	MojErrCheck(err);

	bool didDel;
	err = m_activeRequests.del(token, didDel);
	MojErrCheck(err);
	MojAssert(didDel);

	MojAssert(m_conInfoMap.contains(sock));
	ConInfo& info = **m_conInfoMap.find(sock);
	info.m_activeRequests.del(token, didDel);
	MojAssert(didDel);

	return MojErrNone;
}
