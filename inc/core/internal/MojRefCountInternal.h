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


#ifndef MOJREFCOUNTINTERNAL_H_
#define MOJREFCOUNTINTERNAL_H_

#include "core/internal/MojUtilInternal.h"

#define MojRefCountPtrToAtomic(P) (((MojAtomicT*) P) - 1)

template<class T>
T* MojRefCountNew(MojSize numElems)
{
	T* t = (T*) MojRefCountAlloc(numElems * sizeof(T));
	if (t)
		MojConstruct(t, numElems);
	return t;
}

inline MojInt32 MojRefCountGet(void* p)
{
	MojAssert(p);
	return MojAtomicGet(MojRefCountPtrToAtomic(p));
}

inline void MojRefCountRetain(void* p)
{
	MojAssert(p);
	MojAtomicIncrement(MojRefCountPtrToAtomic(p));
}

template<class T>
void MojRefCountRelease(T* p, MojSize numElems)
{
	if (p) {
		MojAtomicT* a = MojRefCountPtrToAtomic(p);
		if (MojAtomicDecrement(a) == 0) {
			MojAtomicDestroy(a);
			MojDestroy(p, numElems);
			MojFree(a);
		}
	}
}

#endif /* MOJREFCOUNTINTERNAL_H_ */
