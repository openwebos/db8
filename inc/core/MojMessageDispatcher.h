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


#ifndef MOJMESSAGEDISPATCHER_H_
#define MOJMESSAGEDISPATCHER_H_

#include "core/MojCoreDefs.h"
#include "core/MojMessage.h"
#include "core/MojString.h"
#include "core/MojThread.h"

class MojMessageDispatcher : private MojNoCopy
{
public:
	MojMessageDispatcher();
	~MojMessageDispatcher();

	MojErr schedule(MojMessage* msg);
	MojErr start(MojInt32 numThreads);
	MojErr stop();
	MojErr wait();

private:
	class Queue
	{
	public:
		typedef MojList<MojMessage, &MojMessage::m_queueEntry> MessageList;

		bool empty() const { return m_messageList.empty(); }
		const MojString& name() const { return m_name; }

		MojErr init(const MojChar* name) { return m_name.assign(name); }
		MojRefCountedPtr<MojMessage> pop();
		void push(MojMessage* msg);

		MojString m_name;
		MojListEntry m_entry;
		MessageList m_messageList;
	};

	typedef MojList<Queue, &Queue::m_entry> QueueList;
	typedef MojVector<MojThreadT> ThreadVec;

	MojErr dispatch(bool& stoppedOut);

	static Queue* findQueue(const MojChar* name, QueueList& list);
	static MojErr threadMain(void* arg);

	MojThreadMutex m_mutex;
	MojThreadCond m_cond;
	ThreadVec m_threads;
	QueueList m_scheduledList;
	QueueList m_pendingList;
	bool m_stop;
};

#endif /* MOJMESSAGEQUEUE_H_ */
