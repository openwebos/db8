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

#include <glib.h>

#include "core/MojGmainReactor.h"

GSourceFuncs MojGmainReactor::s_sourceFuncs = {
  &MojGmainReactor::prepare, &MojGmainReactor::check, &MojGmainReactor::dispatch, &MojGmainReactor::finalize, 0, 0
};

MojGmainReactor::MojGmainReactor()
{
}

MojGmainReactor::~MojGmainReactor()
{
	MojAssert(m_sources.empty());

	for (SourceMap::ConstIterator i = m_sources.begin();
		 i != m_sources.end(); ++i) {
		g_source_destroy(i->get());
	}
}

MojErr MojGmainReactor::init()
{
    // init glib threads - must be done BEFORE any other call to glib!!!!
    if (!g_thread_supported())
    {
        g_thread_init(NULL);
    }
	MojAssert(!m_mainLoop.get());
	m_mainLoop.reset(g_main_loop_new(NULL, false));
	MojAllocCheck(m_mainLoop.get());

	return MojErrNone;
}

MojErr MojGmainReactor::run()
{
	MojAssert(m_mainLoop.get());
	g_main_loop_run(m_mainLoop.get());

	return MojErrNone;
}

MojErr MojGmainReactor::stop()
{
	if (m_mainLoop.get()) {
		g_main_loop_quit(m_mainLoop.get());
	}
	return MojErrNone;
}

MojErr MojGmainReactor::dispatch()
{
	MojAssert(m_mainLoop.get());

	GMainContext* context = g_main_loop_get_context(m_mainLoop.get());
	MojAssert(context);
	g_main_context_iteration(context, true);

	return MojErrNone;
}

MojErr MojGmainReactor::addSock(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);
	MojAssert(m_mainLoop.get());

	MojThreadGuard guard(m_mutex);
	MojAssert(!m_sources.contains(sock));

	// alloc info and source
	SourcePtr source;
	MojErr err = source.resetChecked((Source*) g_source_new(&s_sourceFuncs, sizeof(Source)));
	MojErrCheck(err);
	source->m_info = new SockInfo(sock, m_mutex);
	MojAllocCheck(source->m_info);
	source->m_info->retain();
	// add to vec
	err = m_sources.put(sock, source);
	MojErrCheck(err);
	// attach
	g_source_attach(source.get(), g_main_loop_get_context(m_mainLoop.get()));
	g_source_add_poll(source.get(), &source->m_info->m_pollFd);

	return MojErrNone;
}

MojErr MojGmainReactor::removeSock(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);
	MojAssert(m_mainLoop.get());

	MojThreadGuard guard(m_mutex);
	MojAssert(m_sources.contains(sock));

	// destroy the g_source
	SourceMap::ConstIterator source = m_sources.find(sock);
	if (source == m_sources.end())
		MojErrThrow(MojErrNotFound);
	g_source_destroy(source->get());
	// and delete it from the sources map
	bool found = false;
	MojErr err = m_sources.del(sock, found);
	MojErrCheck(err);
	MojAssert(found);

	return MojErrNone;
}

MojErr MojGmainReactor::notifyReadable(MojSockT sock, SockSignal::SlotRef slot)
{
	return notify(sock, slot, ReadableEvents, &SockInfo::m_readSig);
}

MojErr MojGmainReactor::notifyWriteable(MojSockT sock, SockSignal::SlotRef slot)
{
	return notify(sock, slot, WriteableEvents, &SockInfo::m_writeSig);
}

MojErr MojGmainReactor::notify(MojSockT sock, SockSignal::SlotRef slot, guint events, SockSignal SockInfo::* sig)
{
	MojAssert(sock != MojInvalidSock);
	MojAssert(m_mainLoop.get());

	MojThreadGuard guard(m_mutex);
	MojAssert(m_sources.contains(sock));

	SourceMap::ConstIterator source = m_sources.find(sock);
	if (source == m_sources.end())
		MojErrThrow(MojErrNotFound);

	SockInfo* info = (*source)->m_info;
	(info->*sig).connect(slot);
	guint newEvents = info->m_pollFd.events | events;
	if (info->m_pollFd.events != newEvents) {
		info->m_pollFd.events = (gushort) newEvents;
		wake();
	}
	return MojErrNone;
}

void MojGmainReactor::wake()
{
	GMainContext* context = g_main_loop_get_context(m_mainLoop.get());
	MojAssert(context);
	g_main_context_wakeup(context);
}

gboolean MojGmainReactor::prepare(GSource* source, gint* timeout)
{
	MojAssert(source && timeout);
	*timeout = -1;
	return false;
}

gboolean MojGmainReactor::check(GSource* gsource)
{
	Source* source = (Source*) gsource;
	MojAssert(source && source->m_info);

	return source->m_info->m_pollFd.revents;
}

gboolean MojGmainReactor::dispatch(GSource* gsource, GSourceFunc callback, gpointer user_data)
{
	Source* source = (Source*) gsource;
	MojAssert(source && source->m_info);

	MojErr err = source->m_info->dispatch();
	MojErrCatchAll(err);

	return true;
}

void MojGmainReactor::finalize(GSource* gsource)
{
	MojAssert(gsource);
	Source* source = (Source*) gsource;

	source->m_info->release();
}

MojGmainReactor::SockInfo::SockInfo(MojSockT sock, MojThreadMutex& mutex)
: m_readSig(this),
  m_writeSig(this),
  m_mutex(mutex)
{
	m_pollFd.fd = sock;
	m_pollFd.events = 0;
	m_pollFd.revents = 0;
}

MojErr MojGmainReactor::SockInfo::dispatch()
{
	MojErr err = MojErrNone;
	MojErr errDispatch = MojErrNone;

	bool readable = m_pollFd.revents & ReadableEvents;
	bool writeable = m_pollFd.revents & WriteableEvents;
	// remove fired events from registered events
	{
		MojThreadGuard guard(m_mutex);
		guint newEvents = 0;
		if ((m_pollFd.events & ReadableEvents) && !readable)
			newEvents |= ReadableEvents;
		if ((m_pollFd.events & WriteableEvents) && !writeable)
			newEvents |= WriteableEvents;
		m_pollFd.events = (gushort) newEvents;
	}
	// dispatch
	if (readable) {
		errDispatch = m_readSig.fire(m_pollFd.fd);
		MojErrAccumulate(err, errDispatch);
	}
	if (writeable) {
		errDispatch = m_writeSig.fire(m_pollFd.fd);
		MojErrAccumulate(err, errDispatch);
	}
	m_pollFd.revents = 0;

	return err;
}
