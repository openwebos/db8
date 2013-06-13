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


#ifndef MOJSIGNALINTERNAL_H_
#define MOJSIGNALINTERNAL_H_

template <class THANDLER>
MojErr MojSlot0<THANDLER>::invoke()
{
	return (m_handler->*m_method)();
}

template <class THANDLER, class TARG1>
MojErr MojSlot1<THANDLER, TARG1>::invoke(TARG1 arg1)
{
	return (m_handler->*m_method)(arg1);
}

template <class THANDLER, class TARG1, class TARG2>
MojErr MojSlot2<THANDLER, TARG1, TARG2>::invoke(TARG1 arg1, TARG2 arg2)
{
	return (m_handler->*m_method)(arg1, arg2);
}

template <class THANDLER, class TARG1, class TARG2, class TARG3>
MojErr MojSlot3<THANDLER, TARG1, TARG2, TARG3>::invoke(TARG1 arg1, TARG2 arg2, TARG3 arg3)
{
	return (m_handler->*m_method)(arg1, arg2, arg3);
}

template <class TARG1>
MojErr MojSignal1<TARG1>::call(TARG1 arg1)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase1<TARG1>* slot = static_cast<MojSlotBase1<TARG1>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		m_slots.pushBack(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

template <class TARG1>
MojErr MojSignal1<TARG1>::fire(TARG1 arg1)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase1<TARG1>* slot = static_cast<MojSlotBase1<TARG1>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		disconnect(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

template <class TARG1, class TARG2>
MojErr MojSignal2<TARG1, TARG2>::call(TARG1 arg1, TARG2 arg2)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase2<TARG1, TARG2>* slot = static_cast<MojSlotBase2<TARG1, TARG2>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		m_slots.pushBack(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1, arg2);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

template <class TARG1, class TARG2>
MojErr MojSignal2<TARG1, TARG2>::fire(TARG1 arg1, TARG2 arg2)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase2<TARG1, TARG2>* slot = static_cast<MojSlotBase2<TARG1, TARG2>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		disconnect(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1, arg2);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

template <class TARG1, class TARG2, class TARG3>
MojErr MojSignal3<TARG1, TARG2, TARG3>::call(TARG1 arg1, TARG2 arg2, TARG3 arg3)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase3<TARG1, TARG2, TARG3>* slot = static_cast<MojSlotBase3<TARG1, TARG2, TARG3>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		m_slots.pushBack(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1, arg2, arg3);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

template <class TARG1, class TARG2, class TARG3>
MojErr MojSignal3<TARG1, TARG2, TARG3>::fire(TARG1 arg1, TARG2 arg2, TARG3 arg3)
{
	MojThreadGuard guard(m_mutex);
	SlotList list = m_slots;
	while (!list.empty()) {
		MojSlotBase3<TARG1, TARG2, TARG3>* slot = static_cast<MojSlotBase3<TARG1, TARG2, TARG3>*>(list.popFront());
		MojRefCountedPtr<MojSignalHandler> handler(slot->handler());
		disconnect(slot);
		guard.unlock();
		MojErr err = slot->invoke(arg1, arg2, arg3);
		MojErrCheck(err);
		guard.lock();
	}
	return MojErrNone;
}

#endif /* MOJSIGNALINTERNAL_H_ */
