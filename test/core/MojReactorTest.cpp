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


#include "MojReactorTest.h"
#include "core/MojGmainReactor.h"
#include "core/MojEpollReactor.h"

class TestConnection : public MojSignalHandler
{
public:
	static const MojSize NumBytes;
	static const MojSize BufSize;
	static const MojChar* SockName;

	TestConnection(MojReactor& reactor, MojReactorTest& test)
	: m_sock(MojInvalidSock),
	  m_reactor(reactor),
	  m_test(test),
	  m_bytesToRead(NumBytes),
	  m_bytesToWrite(NumBytes),
	  m_connectSlot(this, &TestConnection::handleConnect),
	  m_readSlot(this, &TestConnection::handleReadable),
	  m_writeSlot(this, &TestConnection::handleWriteable)
	{
	}

	~TestConnection()
	{
		(void) MojSockClose(m_sock);
	}

	MojErr connect()
	{
		MojErr err = MojSockOpen(m_sock, MOJ_PF_LOCAL, MOJ_SOCK_STREAM);
		MojTestErrCheck(err);
		err = MojSockSetNonblocking(m_sock, true);
		MojTestErrCheck(err);
		err = handleConnect(m_sock);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr start(MojSockT sock)
	{
		m_sock = sock;
		MojErr err = MojSockSetNonblocking(m_sock, true);
		MojTestErrCheck(err);
		err = m_reactor.addSock(m_sock);
		MojTestErrCheck(err);
		err = handleWriteable(m_sock);
		MojTestErrCheck(err);
		err = handleReadable(m_sock);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr handleConnect(MojSockT sock)
	{
		struct sockaddr_un addr;
		MojZero(&addr, sizeof(addr));
		addr.sun_family = MOJ_PF_LOCAL;
		MojStrCpy(addr.sun_path, SockName);

		MojErr err = MojSockConnect(m_sock, (struct sockaddr*) &addr, sizeof(addr));
		MojErrCatch(err, MojErrInProgress) {
			err = m_reactor.notifyWriteable(sock, m_connectSlot);
			MojTestErrCheck(err);
			return MojErrNone;
		}
		MojTestErrCheck(err);
		err = start(sock);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojErr handleReadable(MojSockT sock)
	{
		MojErr err = MojErrNone;
		MojByte buf[BufSize];

		for (;;) {
			if (m_bytesToRead == 0) {
				err = done();
				MojTestErrCheck(err);
				break;
			}
			MojSize readSize = MojMin(BufSize, m_bytesToRead);
			MojSize bytesRead = 0;
			err = MojSockRecv(m_sock, buf, readSize, bytesRead);
			MojErrCatch(err, MojErrWouldBlock) {
				err = m_reactor.notifyReadable(sock, m_readSlot);
				MojTestErrCheck(err);
				break;
			}
			MojTestErrCheck(err);
			m_bytesToRead -= bytesRead;
		}
		return MojErrNone;
	}

	MojErr handleWriteable(MojSockT sock)
	{
		MojErr err = MojErrNone;
		MojByte buf[BufSize];
		MojZero(buf, BufSize);

		for (;;) {
			if (m_bytesToWrite == 0) {
				err = done();
				MojTestErrCheck(err);
				break;
			}
			MojSize writeSize = MojMin(BufSize, m_bytesToWrite);
			MojSize bytesWritten = 0;
			err = MojSockSend(m_sock, buf, writeSize, bytesWritten);
			MojErrCatch(err, MojErrWouldBlock) {
				err = m_reactor.notifyWriteable(sock, m_writeSlot);
				MojTestErrCheck(err);
				break;
			}
			MojTestErrCheck(err);
			m_bytesToWrite -= bytesWritten;
		}
		return MojErrNone;
	}

	MojErr done()
	{
		if (m_bytesToWrite == 0 && m_bytesToRead == 0) {
			MojErr err = m_reactor.removeSock(m_sock);
			MojTestErrCheck(err);
			err = m_test.connectionDone();
			MojTestErrCheck(err);
		}

		return MojErrNone;
	}

private:
	MojSockT m_sock;
	MojReactor& m_reactor;
	MojReactorTest& m_test;
	MojSize m_bytesToRead;
	MojSize m_bytesToWrite;
	MojReactor::SockSignal::Slot<TestConnection> m_connectSlot;
	MojReactor::SockSignal::Slot<TestConnection> m_readSlot;
	MojReactor::SockSignal::Slot<TestConnection> m_writeSlot;
};

const MojSize TestConnection::NumBytes = 1024 * 1024 * 10;
const MojSize TestConnection::BufSize = 1024;
const char* TestConnection::SockName = "mojreactortestsock";

MojReactorTest::MojReactorTest()
: MojTestCase(_T("MojReactor")),
  m_reactor(NULL)
{
}

MojErr MojReactorTest::run()
{
	MojErr err = MojErrNone;
#ifdef MOJ_USE_GLIB
	MojGmainReactor gmainReactor;
	err = simpleTest(gmainReactor);
	MojTestErrCheck(err);
#endif
#ifdef MOJ_USE_EPOLL
	MojEpollReactor epollReactor;
	err = simpleTest(epollReactor);
	MojTestErrCheck(err);
#endif
	return MojErrNone;
}

MojErr MojReactorTest::simpleTest(MojReactor& reactor)
{
	m_reactor = &reactor;
	m_count = 2;
	MojErr err = reactor.init();
	MojTestErrCheck(err);

	MojRefCountedPtr<TestConnection> con1(new TestConnection(reactor, *this));
	MojAllocCheck(con1.get());
	MojRefCountedPtr<TestConnection> con2(new TestConnection(reactor, *this));
	MojAllocCheck(con2.get());

	MojSockT listenSock = MojInvalidSock;
	err = MojSockOpen(listenSock, MOJ_PF_LOCAL, MOJ_SOCK_STREAM);
	MojTestErrCheck(err);
	MojSockAddrUnT addr;
	MojZero(&addr, sizeof(addr));
	addr.sun_family = MOJ_PF_LOCAL;
	MojStrCpy(addr.sun_path, TestConnection::SockName);
	(void) MojUnlink(TestConnection::SockName);
	err = MojSockBind(listenSock, (struct sockaddr*) &addr, sizeof(addr));
	MojTestErrCheck(err);
	err = MojSockListen(listenSock, 10);
	MojTestErrCheck(err);

	err = con1->connect();
	MojTestErrCheck(err);
	MojSockT acceptSock = MojInvalidSock;
	err = MojSockAccept(listenSock, acceptSock);
	MojTestErrCheck(err);
	err = con2->start(acceptSock);
	MojTestErrCheck(err);

	err = reactor.run();
	MojTestErrCheck(err);
	MojTestAssert(m_count == 0);

	return MojErrNone;
}

MojErr MojReactorTest::connectionDone()
{
	MojTestAssert(m_reactor && m_count.value());

	if (--m_count == 0)
		m_reactor->stop();

	return MojErrNone;
}

void MojReactorTest::cleanup()
{
	(void) MojUnlink(TestConnection::SockName);
}
