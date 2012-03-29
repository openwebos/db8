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


#ifndef MOJATOMICINT_H_
#define MOJATOMICINT_H_

#include "core/MojCoreDefs.h"

class MojAtomicInt
{
public:
	MojAtomicInt(MojInt32 i = 0) { MojAtomicInit(&m_a, i); }
	MojAtomicInt(const MojAtomicInt& ai) { MojAtomicInit(&m_a, ai.value()); }
	~MojAtomicInt() { MojAtomicDestroy(&m_a); }

	MojInt32 value() const { return MojAtomicGet(&m_a); }
	void set(MojInt32 i) { MojAtomicSet(&m_a, i); }
	MojInt32 add(MojInt32 i) { return MojAtomicAdd(&m_a,i); }
	MojInt32 increment() { return MojAtomicIncrement(&m_a); }
	MojInt32 decrement() { return MojAtomicDecrement(&m_a); }

	bool operator==(const MojAtomicInt& rhs) const { return value() == rhs.value(); }
	bool operator==(MojInt32 rhs) const { return value() == rhs; }
	bool operator!=(const MojAtomicInt& rhs) const { return value() != rhs.value(); }
	bool operator!=(MojInt32 rhs) const { return value() != rhs; }
	bool operator<(const MojAtomicInt& rhs) const { return value() < rhs.value(); }
	bool operator<(MojInt32 rhs) const { return value() < rhs; }
	bool operator<=(const MojAtomicInt& rhs) const { return value() <= rhs.value(); }
	bool operator<=(MojInt32 rhs) const { return value() <= rhs; }
	bool operator>(const MojAtomicInt& rhs) const { return value() > rhs.value(); }
	bool operator>(MojInt32 rhs) const { return value() > rhs; }
	bool operator>=(const MojAtomicInt& rhs) const { return value() >= rhs.value(); }
	bool operator>=(MojInt32 rhs) const { return value() >= rhs; }
	MojInt32 operator++() { return increment(); }
	MojInt32 operator--() { return decrement(); }
	MojAtomicInt& operator=(const MojAtomicInt& rhs) { set(rhs.value()); return *this;  }
	MojAtomicInt& operator=(MojInt32 rhs) { set(rhs); return *this; }
	MojAtomicInt& operator+=(MojInt32 rhs) { add(rhs); return *this; }

private:
	MojAtomicT m_a;
};

#endif /* MOJATOMICINT_H_ */
