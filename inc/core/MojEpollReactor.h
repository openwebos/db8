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


#ifndef MOJEPOLLREACTOR_H_
#define MOJEPOLLREACTOR_H_

#include "core/MojReactor.h"
#include "core/MojHashMap.h"
#include "core/MojThread.h"

class MojEpollReactor : public MojReactor
{
public:
	MojEpollReactor();
	virtual ~MojEpollReactor();

	virtual MojErr init();
	virtual MojErr run();
	virtual MojErr stop();
	virtual MojErr dispatch();
	virtual MojErr addSock(MojSockT sock);
	virtual MojErr removeSock(MojSockT sock);
	virtual MojErr notifyReadable(MojSockT sock, SockSignal::SlotRef slot);
	virtual MojErr notifyWriteable(MojSockT sock, SockSignal::SlotRef slot);

private:
	enum {
		FlagNone = 0,
		FlagReadable = 1,
		FlagWriteable = 1 << 1
	};
	typedef gint32 Flags;

	class SockInfo : public MojSignalHandler
	{
	public:
		SockInfo();

		SockSignal m_readSig;
		SockSignal m_writeSig;
		Flags m_registeredFlags;
		Flags m_requestedFlags;
	};

	typedef MojRefCountedPtr<SockInfo> SockInfoPtr;
	typedef MojHashMap<MojSockT, SockInfoPtr> SockMap;

	static const int MaxEvents = 32;
	static const int ReadableEvents;
	static const int WriteableEvents;

	MojErr notify(MojSockT sock, SockSignal::SlotRef slot, Flags flags, SockSignal SockInfo::* sig);
	MojErr updateSock(MojSockT sock, SockInfo* info);
	MojErr dispatchEvent(struct epoll_event& event);
	MojErr control(MojSockT sock, int op, int events);
	MojErr wake();
	MojErr close();

	static MojErr closeSock(MojSockT& sock);
	static int flagsToEvents(Flags flags);
	static Flags eventsToFlags(int events);

	MojThreadMutex m_mutex;
	SockMap m_sockMap;
	MojSockT m_epollSock;
	MojSockT m_pipe[2];
	bool m_stop;
};

#endif /* MOJEPOLLREACTOR_H_ */
