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


#ifndef MOJGMAINREACTOR_H_
#define MOJGMAINREACTOR_H_

#include "core/MojCoreDefs.h"
#include "core/MojHashMap.h"
#include "core/MojReactor.h"
#include "core/MojThread.h"
#include "core/MojVector.h"
#include "glib.h"

class MojGmainReactor : public MojReactor
{
public:
	MojGmainReactor();
	virtual ~MojGmainReactor();

	virtual MojErr init();
	virtual MojErr run();
	virtual MojErr stop();
	virtual MojErr dispatch();
	virtual MojErr addSock(MojSockT sock);
	virtual MojErr removeSock(MojSockT sock);
	virtual MojErr notifyReadable(MojSockT sock, SockSignal::SlotRef slot);
	virtual MojErr notifyWriteable(MojSockT sock, SockSignal::SlotRef slot);

	GMainLoop* impl() { return m_mainLoop.get(); }

private:
	static const guint ReadableEvents = (G_IO_IN | G_IO_HUP | G_IO_ERR);
	static const guint WriteableEvents = (G_IO_OUT | G_IO_ERR);

	struct LoopDestructor
	{
		void operator()(GMainLoop* loop)
		{
			g_main_loop_unref(loop);
		}
	};

	struct SourceDestructor
	{
		void operator()(GSource* source)
		{
			g_source_unref(source);
		}
	};

	class SockInfo : public MojSignalHandler
	{
	public:
		SockInfo(MojSockT sock, MojThreadMutex& mutex);
		MojErr dispatch();

		guint m_flags;
		GPollFD m_pollFd;
		SockSignal m_readSig;
		SockSignal m_writeSig;
		MojThreadMutex& m_mutex;
	};

	struct Source : public GSource
	{
		SockInfo* m_info;
	};

	typedef MojAutoPtrBase<GMainLoop, LoopDestructor> LoopPtr;
	typedef MojSharedPtrBase<Source, SourceDestructor> SourcePtr;
	typedef MojHashMap<MojSockT, SourcePtr> SourceMap;

	MojErr notify(MojSockT sock, SockSignal::SlotRef slot, guint events, SockSignal SockInfo::* sig);
	void wake();

	static gboolean prepare(GSource* source, gint* timeout);
	static gboolean check(GSource* source);
	static gboolean dispatch(GSource* source, GSourceFunc callback, gpointer user_data);
	static void finalize(GSource* source);

	LoopPtr m_mainLoop;
	SourceMap m_sources;
	MojThreadMutex m_mutex;

	static GSourceFuncs s_sourceFuncs;
};

#endif /* MOJGMAINREACTOR_H_ */
