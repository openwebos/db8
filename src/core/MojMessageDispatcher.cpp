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


#include "core/MojMessageDispatcher.h"

MojMessageDispatcher::MojMessageDispatcher()
: m_stop(false)
{
}

MojMessageDispatcher::~MojMessageDispatcher()
{
	MojErr err = stop();
	MojErrCatchAll(err);
	err = wait();
	MojErrCatchAll(err);
}

MojErr MojMessageDispatcher::schedule(MojMessage* msg)
{
	MojAssert(msg);
	MojThreadGuard guard(m_mutex);

	const MojChar* queueName = msg->queue();
	// first look for a scheduled queue
	Queue* queue = findQueue(queueName, m_scheduledList);
	if (!queue) {
		// then look for a pending queue
		queue = findQueue(queueName, m_pendingList);

		// then create a new queue if we didn't find one
		if (!queue) {
			MojAutoPtr<Queue> newQueue(new Queue);
			MojAllocCheck(newQueue.get());
			queue = newQueue.get();
			MojErr err = queue->init(queueName);
			MojErrCheck(err);

			// add it to scheduled list and wake up a thread
			m_scheduledList.pushBack(newQueue.release());
			err = m_cond.signal();
			MojErrCheck(err);
		}
	}
	queue->push(msg);

	return MojErrNone;
}

MojErr MojMessageDispatcher::start(MojInt32 numThreads)
{
	for (MojInt32 i = 0; i < numThreads; ++i) {
		MojThreadT thread = MojInvalidThread;
		MojErr err = MojThreadCreate(thread, threadMain, this);
		MojErrCheck(err);
		MojAssert(thread != MojInvalidThread);
		err = m_threads.push(thread);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojMessageDispatcher::stop()
{
	MojThreadGuard guard(m_mutex);

	m_stop = true;
	MojErr err = m_cond.broadcast();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojMessageDispatcher::wait()
{
	MojErr err = MojErrNone;
	for (ThreadVec::ConstIterator i = m_threads.begin();
		 i != m_threads.end();
		 ++i) {
		MojErr threadErr = MojErrNone;
		MojErr joinErr = MojThreadJoin(*i, threadErr);
		MojErrAccumulate(err, threadErr);
		MojErrAccumulate(err, joinErr);
	}
	m_threads.clear();

	return err;
}

MojErr MojMessageDispatcher::dispatch(bool& stoppedOut)
{
	MojThreadGuard guard(m_mutex);

	// wait for a message
	stoppedOut = false;
	while (m_scheduledList.empty() && !m_stop) {
		MojErr err = m_cond.wait(m_mutex);
		MojErrCheck(err);
	}

	if (m_scheduledList.empty()) {
		// stop if we didn't get a message
		MojAssert(m_stop);
		stoppedOut = true;
	} else {
		// move queue from scheduled list to pending list and pop a message
		Queue* queue = m_scheduledList.popFront();
		m_pendingList.pushBack(queue);
		MojRefCountedPtr<MojMessage> msg = queue->pop();

		// unlock and dispatch
		guard.unlock();
		MojErr err = msg->dispatch();
		MojErrCatchAll(err);
		guard.lock();

		// remove from pending list
		m_pendingList.erase(queue);
		if (queue->empty()) {
			// we're done with this queue
			delete queue;
		} else {
			// if queue has more messages, add it back to scheduled list
			// no need to signal since this thread will loop back around and pick it up
			m_scheduledList.pushBack(queue);
		}
	}
	return MojErrNone;
}

MojMessageDispatcher::Queue* MojMessageDispatcher::findQueue(const MojChar* name, QueueList& list)
{
	for (QueueList::Iterator i = list.begin(); i != list.end(); ++i) {
		if ((*i)->name() == name)
			return *i;
	}
	return NULL;
}

MojErr MojMessageDispatcher::threadMain(void* arg)
{
	MojMessageDispatcher* queue = (MojMessageDispatcher*) arg;
	MojAssert(queue);

	bool stop = false;
	do {
		MojErr err = queue->dispatch(stop);
		MojErrCatchAll(err);
	} while (!stop);

	return MojErrNone;
}

void MojMessageDispatcher::Queue::push(MojMessage* msg)
{
	MojAssert(msg);

	m_messageList.pushBack(msg);
	msg->retain();
}

MojRefCountedPtr<MojMessage> MojMessageDispatcher::Queue::pop()
{
	MojRefCountedPtr<MojMessage> msg(m_messageList.popFront());
	msg->release();
	return msg;
}
