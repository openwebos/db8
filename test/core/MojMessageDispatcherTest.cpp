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


#include "MojMessageDispatcherTest.h"
#include "core/MojMessage.h"
#include "core/MojMessageDispatcher.h"

static const MojInt32 MojTestNumThreads = 5;
static const MojInt32 MojTestNumMessages = 50000;
static MojAtomicInt s_messageCount;

class MojTestMessage : public MojMessage
{
public:
	MojTestMessage(int i) : m_i(i) {}

	MojErr init()
	{
		return m_queue.format(_T("%d"), m_i % 10);
	}

	virtual const MojChar* queue() const
	{
		return m_queue;
	}

	virtual MojErr dispatch()
	{
		s_messageCount.decrement();
		return MojErrNone;
	}

	MojString m_queue;
	int m_i;
};

MojMessageDispatcherTest::MojMessageDispatcherTest()
: MojTestCase(_T("MojMessageQueue"))
{
}

MojErr MojMessageDispatcherTest::run()
{
	MojMessageDispatcher queue;
	MojErr err = queue.start(MojTestNumThreads);
	MojTestErrCheck(err);

	// push messages
	for (int i = 0; i < MojTestNumMessages; ++i) {
		MojRefCountedPtr<MojTestMessage> msg(new MojTestMessage(i));
		MojAllocCheck(msg.get());
		err = msg->init();
		MojTestErrCheck(err);
		err = queue.schedule(msg.get());
		MojErrCheck(err);
		err = MojThreadYield();
		MojTestErrCheck(err);
		s_messageCount.increment();
	}

	err = queue.stop();
	MojTestErrCheck(err);
	err = queue.wait();
	MojTestErrCheck(err);

	MojTestAssert(s_messageCount == 0);

	return MojErrNone;
}
