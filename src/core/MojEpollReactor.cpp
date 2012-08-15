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


#include "core/MojEpollReactor.h"

#ifdef MOJ_USE_EPOLL
#include <sys/epoll.h>

const int MojEpollReactor::ReadableEvents = EPOLLIN | EPOLLERR | EPOLLHUP;
const int MojEpollReactor::WriteableEvents = EPOLLOUT | EPOLLERR | EPOLLHUP;

MojEpollReactor::MojEpollReactor()
: m_epollSock(MojInvalidSock),
  m_stop(false)
{
	m_pipe[0] = MojInvalidSock;
	m_pipe[1] = MojInvalidSock;
}

MojEpollReactor::~MojEpollReactor()
{
	(void) close();
}

MojErr MojEpollReactor::init()
{
	// open epoll descriptor
	MojAssert(m_epollSock == MojInvalidSock);
	m_epollSock = epoll_create(MaxEvents);
	if (m_epollSock < 0)
		MojErrThrowErrno(_T("epoll_create"));

	// set up pipe to safely shut down reactor from a signal handler
	MojErr err = MojPipe(m_pipe);
	MojErrCheck(err);
	// add read-end of pipe to epoll
	err = control(m_pipe[0], EPOLL_CTL_ADD, EPOLLIN);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojEpollReactor::run()
{
	m_stop = false;
	while (!m_stop) {
		MojErr err = dispatch();
		MojErrCatchAll(err);
	}
	return MojErrNone;
}

MojErr MojEpollReactor::stop()
{
	m_stop = true;
	MojErr err = wake();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojEpollReactor::dispatch()
{
	MojAssert(m_epollSock != MojInvalidSock);

	struct epoll_event events[MaxEvents];
	int nfds = epoll_wait(m_epollSock, events, MaxEvents, -1 /*no timeout*/);
	if (nfds < 0)
		MojErrThrowErrno(_T("epoll_wait"));

	for (int i = 0; i < nfds; ++i) {
		if (events[i].data.fd != m_pipe[0]) {
			MojErr err = dispatchEvent(events[i]);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojEpollReactor::addSock(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);

	MojThreadGuard guard(m_mutex);
	MojAssert(!m_sockMap.contains(sock));

	SockInfoPtr info(new SockInfo);
	MojAllocCheck(info.get());
	MojErr err = m_sockMap.put(sock, info);
	MojErrCheck(err);
	err = control(sock, EPOLL_CTL_ADD, 0);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojEpollReactor::removeSock(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);

	MojThreadGuard guard(m_mutex);
	MojAssert(m_sockMap.contains(sock));

	bool found = false;
	MojErr err = MojErrNone;
	MojErr errDel = m_sockMap.del(sock, found);
	MojErrAccumulate(err, errDel);
	if (!found)
		MojErrThrow(MojErrNotFound);
	errDel = control(sock, EPOLL_CTL_DEL, 0);
	MojErrAccumulate(err, errDel);

	return err;
}

MojErr MojEpollReactor::notifyReadable(MojSockT sock, SockSignal::SlotRef slot)
{
	return notify(sock, slot, FlagReadable, &SockInfo::m_readSig);
}

MojErr MojEpollReactor::notifyWriteable(MojSockT sock, SockSignal::SlotRef slot)
{
	return notify(sock, slot, FlagWriteable, &SockInfo::m_writeSig);
}

MojErr MojEpollReactor::notify(MojSockT sock, SockSignal::SlotRef slot, Flags flags, SockSignal SockInfo::* sig)
{
	MojAssert(sock != MojInvalidSock);

	MojThreadGuard guard(m_mutex);
	MojAssert(m_sockMap.contains(sock));

	SockMap::ConstIterator mapIter = m_sockMap.find(sock);
	if (mapIter == m_sockMap.end())
		MojErrThrow(MojErrNotFound);

	SockInfo* info = mapIter->get();
	(info->*sig).connect(slot);
	info->m_requestedFlags |= flags;
	MojErr err = updateSock(sock, info);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojEpollReactor::updateSock(MojSockT sock, SockInfo* info)
{
	MojAssert(info);
	MojAssertMutexLocked(m_mutex);

	if (info->m_requestedFlags != info->m_registeredFlags) {
		int events = flagsToEvents(info->m_requestedFlags);
		MojErr err = control(sock, EPOLL_CTL_MOD, events);
		MojErrCheck(err);
		info->m_registeredFlags = info->m_requestedFlags;
	}
	return MojErrNone;
}

MojErr MojEpollReactor::dispatchEvent(struct epoll_event& event)
{
	MojThreadGuard guard(m_mutex);

	MojSockT sock = event.data.fd;
	SockMap::ConstIterator mapIter = m_sockMap.find(sock);
	if (mapIter == m_sockMap.end())
		return MojErrNone;

	// increment ref count so info can't be deleted out from under us when we drop the lock
	SockInfoPtr info = *mapIter;
	Flags flags = eventsToFlags(event.events);
	MojAssert(flags != FlagNone);
	info->m_requestedFlags &= ~flags;
	info->m_registeredFlags = FlagNone;

	guard.unlock();
	if (flags & FlagReadable) {
		MojErr errSignal = info->m_readSig.fire(sock);
		MojErrCatchAll(errSignal);
	}
	if (flags & FlagWriteable) {
		MojErr errSignal = info->m_writeSig.fire(sock);
		MojErrCatchAll(errSignal);
	}
	guard.lock();

	MojErr err = updateSock(sock, info.get());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojEpollReactor::control(MojSockT sock, int op, int events)
{
	MojAssert(m_epollSock != MojInvalidSock);

	struct epoll_event ev;
	MojZero(&ev, sizeof(ev));
	ev.events = events;
	ev.data.fd = sock;
	if (epoll_ctl(m_epollSock, op, sock, &ev) < 0)
		MojErrThrowErrno(_T("epoll_ctl"));

	return MojErrNone;
}

MojErr MojEpollReactor::wake()
{
	if (m_pipe[1] != MojInvalidSock) {
		MojByte b = 1;
		MojSize sent = 0;
		MojErr err = MojSockSend(m_pipe[1], &b, sizeof(b), sent);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojEpollReactor::close()
{
	MojErr err = MojErrNone;
	MojErr errClose = closeSock(m_epollSock);
	MojErrAccumulate(err, errClose);
	errClose = closeSock(m_pipe[0]);
	MojErrAccumulate(err, errClose);
	errClose = closeSock(m_pipe[1]);
	MojErrAccumulate(err, errClose);

	return err;
}

MojErr MojEpollReactor::closeSock(MojSockT& sock)
{
	if (sock != MojInvalidSock) {
		MojErr err = MojSockClose(sock);
		MojErrCheck(err);
		sock = MojInvalidSock;
	}
	return MojErrNone;
}

int MojEpollReactor::flagsToEvents(Flags flags)
{
	MojAssert(flags != FlagNone);

	int events = EPOLLONESHOT;
	if (flags & FlagReadable)
		events |= ReadableEvents;
	if (flags & FlagWriteable)
		events |= WriteableEvents;
	return events;
}

MojEpollReactor::Flags MojEpollReactor::eventsToFlags(int events)
{
	Flags flags = FlagNone;
	if (events & ReadableEvents)
		flags |= FlagReadable;
	if (events & WriteableEvents)
		flags |= FlagWriteable;
	return flags;
}

MojEpollReactor::SockInfo::SockInfo()
: m_readSig(this),
  m_writeSig(this),
  m_registeredFlags(FlagNone),
  m_requestedFlags(FlagNone)
{
}

#endif // MOJ_USE_EPOLL
