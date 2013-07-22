/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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

/**
****************************************************************************************************
* Filename              : MojMessageDispatcherTest.cpp
* Description           : Source file for MojMessageDispatcher test.
****************************************************************************************************
**/

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
/**
****************************************************************************************************
* @run              For all incoming messages,queue name is extracted from message and checked in
                    both pending and scheduled lists.If queue is not present in both the lists,new
                    queue is created and added to the schedule list with the message.
                    Thread is awakened to read the queued message from schedule list for further
                    processing. When message is getting processed, corresponding queue is kept in
                    pending list. After completing the processing,queue is removed from pending list
                    and added to scheduled list if more messages to be processed, otherwise queue is
                    deleted.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
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
