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


#include "core/MojSignal.h"

MojSignalHandler::~MojSignalHandler()
{
}

MojErr MojSignalHandler::handleCancel()
{
	return MojErrNone;
}

MojSlotBase::MojSlotBase()
: m_signal(NULL),
  m_connected(false)
{
}

MojSlotBase::~MojSlotBase()
{
	MojAssert(!m_connected);
}

void MojSlotBase::cancel()
{
   if(m_signal == NULL)
      return;

   if (m_connected) {
      m_signal->cancel(this);
      m_signal = NULL;
   }
}

void MojSlotBase::disconnect()
{
	MojAssertMutexLocked(m_signal->m_mutex);
	MojAssert(m_connected);

	m_connected = false;
	handler()->release();
}

MojSignalBase::MojSignalBase(MojSignalHandler* handler)
: m_handler(handler)
{
	MojAssert(handler);
}

MojSignalBase::~MojSignalBase()
{
	MojThreadGuard guard(m_mutex);
	while (!m_slots.empty())
		m_slots.popFront()->disconnect();
}

void MojSignalBase::connect(MojSlotBase* slot)
{
	MojThreadGuard guard(m_mutex);
	MojAssert(slot);
	MojAssert(!slot->m_connected);

	slot->handler()->retain();
	slot->m_signal = this;
	slot->m_connected = true;
	m_slots.pushBack(slot);
}

void MojSignalBase::cancel(MojSlotBase* slot)
{
	MojThreadGuard guard(m_mutex);
    if(slot->m_connected == false)
       return;

    guard.unlock();

	// add ref to make sure that handler isn't destroyed before handleCancel returns.
	MojRefCountedPtr<MojSignalHandler> handler(m_handler);
	MojErr err = m_handler->handleCancel();
	MojErrCatchAll(err);

    guard.lock();
    // Not sure if it's possible but who knows what might happened once we unlock the mutex
    if(slot->m_connected == false)
       return;
	MojAssert(slot && m_slots.contains(slot));
	m_slots.erase(slot);
	slot->disconnect();
    guard.unlock();
}

MojErr MojSignal0::call()
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase0* slot = static_cast<MojSlotBase0*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		m_slots.pushBack(slot);
		guard.unlock();
		MojErr err = slot->invoke();
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

MojErr MojSignal0::fire()
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase0* slot = static_cast<MojSlotBase0*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		disconnect(slot);
		guard.unlock();
		MojErr err = slot->invoke();
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}
