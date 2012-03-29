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


#ifndef MOJSIGNAL_H_
#define MOJSIGNAL_H_

#include "core/MojCoreDefs.h"
#include "core/MojList.h"
#include "core/MojThread.h"
#include "core/MojVector.h"

class MojSignalBase;
class MojSlotBase;
class MojSignal0;
template <class ARG1> class MojSignal1;
template <class ARG1, class ARG2> class MojSignal2;
template <class ARG1, class ARG2, class ARG3> class MojSignal3;

class MojSignalHandler : public MojRefCounted
{
protected:
	friend class MojSignalBase;

	virtual MojErr handleCancel();
};

class MojSlotBase : private MojNoCopy
{
public:
	void cancel();
	virtual MojSignalHandler* handler() = 0;

protected:
	MojSlotBase();
	~MojSlotBase();
	void disconnect();

private:
	friend class MojSignalBase;

	MojListEntry m_entry;
	MojSignalBase* m_signal;
	bool m_connected;
};

class MojSlotBase0 : public MojSlotBase
{
public:
	virtual MojErr invoke() = 0;
};

template <class TARG1>
class MojSlotBase1 : public MojSlotBase
{
public:
	virtual MojErr invoke(TARG1) = 0;
};

template <class TARG1, class TARG2>
class MojSlotBase2 : public MojSlotBase
{
public:
	virtual MojErr invoke(TARG1, TARG2) = 0;
};

template <class TARG1, class TARG2, class TARG3>
struct MojSlotBase3 : public MojSlotBase
{
public:
	virtual MojErr invoke(TARG1, TARG2, TARG3) = 0;
};

template <class THANDLER>
class MojSlot0 : public MojSlotBase0
{
public:
	typedef MojErr (THANDLER::* MethodType)();

	MojSlot0(THANDLER* handler, MethodType method) : m_handler(handler), m_method(method) {}
	virtual MojSignalHandler* handler() { return m_handler; }
	virtual MojErr invoke();

private:
	THANDLER* m_handler;
	MethodType m_method;
};

template <class THANDLER, class TARG1>
class MojSlot1 : public MojSlotBase1<TARG1>
{
public:
	typedef MojErr (THANDLER::* MethodType)(TARG1 arg1);

	MojSlot1(THANDLER* handler, MethodType method) : m_handler(handler), m_method(method) {}
	virtual MojSignalHandler* handler() { return m_handler; }
	virtual MojErr invoke(TARG1 arg1);

private:
	THANDLER* m_handler;
	MethodType m_method;
};

template <class THANDLER, class TARG1, class TARG2>
class MojSlot2 : public MojSlotBase2<TARG1, TARG2>
{
public:
	typedef MojErr (THANDLER::* MethodType)(TARG1 arg1, TARG2 arg2);

	MojSlot2(THANDLER* handler, MethodType method) : m_handler(handler), m_method(method) {}
	virtual MojSignalHandler* handler() { return m_handler; }
	virtual MojErr invoke(TARG1 arg1, TARG2 arg2);

private:
	THANDLER* m_handler;
	MethodType m_method;
};

template <class THANDLER, class TARG1, class TARG2, class TARG3>
class MojSlot3 : public MojSlotBase3<TARG1, TARG2, TARG3>
{
public:
	typedef MojErr (THANDLER::* MethodType)(TARG1 arg1, TARG2 arg2, TARG3 arg3);

	MojSlot3(THANDLER* handler, MethodType method) : m_handler(handler), m_method(method) {}
	virtual MojSignalHandler* handler() { return m_handler; }
	virtual MojErr invoke(TARG1 arg1, TARG2 arg2, TARG3 arg3);

private:
	THANDLER* m_handler;
	MethodType m_method;
};

class MojSignalBase : private MojNoCopy
{
public:
	bool connected() const { return !m_slots.empty(); }

protected:
	friend class MojSlotBase;
	typedef MojList<MojSlotBase, &MojSlotBase::m_entry> SlotList;

	MojSignalBase(MojSignalHandler* handler);
	~MojSignalBase();
	void connect(MojSlotBase* slot);
	void cancel(MojSlotBase* slot);
	void disconnect(MojSlotBase* slot) { MojAssert(slot->m_signal == this); slot->disconnect(); }

	MojThreadMutex m_mutex;
	SlotList m_slots;
	MojSignalHandler* m_handler;
};

class MojSignal0 : public MojSignalBase
{
public:
	typedef MojSlotBase0& SlotRef;

	template <class THANDLER>
	class Slot : public MojSlot0<THANDLER>
	{
	public:
		typedef MojErr (THANDLER::* MethodType)();
		Slot(THANDLER* handler, MethodType method)
		: MojSlot0<THANDLER>(handler, method) {}
	};

	MojSignal0(MojSignalHandler* handler) : MojSignalBase(handler) {}
	void connect(SlotRef ref) { MojSignalBase::connect(&ref); }
	MojErr call();
	MojErr fire();
};

template <class TARG1>
class MojSignal1 : public MojSignalBase
{
public:
	typedef MojSlotBase1<TARG1>& SlotRef;

	template <class THANDLER>
	class Slot : public MojSlot1<THANDLER, TARG1>
	{
	public:
		typedef MojErr (THANDLER::* MethodType)(TARG1 arg1);
		Slot(THANDLER* handler, MethodType method)
		: MojSlot1<THANDLER, TARG1>(handler, method) {}
	};

	MojSignal1(MojSignalHandler* handler) : MojSignalBase(handler) {}
	void connect(SlotRef ref) { MojSignalBase::connect(&ref); }
	MojErr call(TARG1 arg1);
	MojErr fire(TARG1 arg1);
};

template <class TARG1, class TARG2>
class MojSignal2 : public MojSignalBase
{
public:
	typedef MojSlotBase2<TARG1, TARG2>& SlotRef;

	template <class THANDLER>
	class Slot : public MojSlot2<THANDLER, TARG1, TARG2>
	{
	public:
		typedef MojErr (THANDLER::* MethodType)(TARG1 arg1, TARG2 arg2);
		Slot(THANDLER* handler, MethodType method)
		: MojSlot2<THANDLER, TARG1, TARG2>(handler, method) {}
	};

	MojSignal2(MojSignalHandler* handler) : MojSignalBase(handler) {}
	void connect(SlotRef ref) { MojSignalBase::connect(&ref); }
	MojErr call(TARG1 arg1, TARG2 arg2);
	MojErr fire(TARG1 arg1, TARG2 arg2);
};

template <class TARG1, class TARG2, class TARG3>
class MojSignal3 : public MojSignalBase
{
public:
	typedef MojSlotBase3<TARG1, TARG2, TARG3>& SlotRef;

	template <class THANDLER>
	class Slot : public MojSlot3<THANDLER, TARG1, TARG2, TARG3>
	{
	public:
		typedef MojErr (THANDLER::* MethodType)(TARG1 arg1, TARG2 arg2, TARG3 arg3);
		Slot(THANDLER* handler, MethodType method)
		: MojSlot3<THANDLER, TARG1, TARG2, TARG3>(handler, method) {}
	};

	MojSignal3(MojSignalHandler* handler) : MojSignalBase(handler) {}
	void connect(SlotRef ref) { MojSignalBase::connect(&ref); }
	MojErr call(TARG1 arg1, TARG2 arg2, TARG3 arg3);
	MojErr fire(TARG1 arg1, TARG2 arg2, TARG3 arg3);
};

template <class TARG1=MojTagNil, class TARG2=MojTagNil, class TARG3=MojTagNil>
class MojSignal : public MojSignal3<TARG1, TARG2, TARG3>
{
public:
	MojSignal(MojSignalHandler* handler) : MojSignal3<TARG1, TARG2, TARG3>(handler) {}
};

template <>
class MojSignal<MojTagNil, MojTagNil, MojTagNil> : public MojSignal0
{
public:
	MojSignal(MojSignalHandler* handler) : MojSignal0(handler) {}
};

template <class TARG1>
class MojSignal<TARG1, MojTagNil, MojTagNil> : public MojSignal1<TARG1>
{
public:
	MojSignal(MojSignalHandler* handler) : MojSignal1<TARG1>(handler) {}
};

template <class TARG1, class TARG2>
class MojSignal<TARG1, TARG2, MojTagNil> : public MojSignal2<TARG1, TARG2>
{
public:
	MojSignal(MojSignalHandler* handler) : MojSignal2<TARG1, TARG2>(handler) {}
};

#include "core/internal/MojSignalInternal.h"

#endif /* MOJSIGNAL_H_ */
