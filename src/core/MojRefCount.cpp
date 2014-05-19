/* @@@LICENSE
*
*      Copyright (c) 2009-2014 LG Electronics, Inc.
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


#include "core/MojRefCount.h"

void* MojRefCountAlloc(MojSize n)
{
	void* p = MojMalloc(n + sizeof(MojAtomicT) + MojAtomicPadding);
	if (p) {
		MojAtomicT* a = (MojAtomicT*) p;
		MojAtomicInit(a, 1);
		p = MojRefCountPtrFromAtomic(a);
	}
	return p;
}

void* MojRefCountRealloc(void* p, MojSize n)
{
	void* pnew = NULL;
	if (n == 0) {
		MojRefCountRelease(p);
	} else if (!p) {
		pnew = MojRefCountAlloc(n);
	} else {
		MojAssert(MojRefCountGet(p) == 1);
		MojAtomicT* a = MojRefCountPtrToAtomic(p);
		pnew = MojRealloc(a, n + sizeof(MojAtomicT) + MojAtomicPadding);
		if (pnew)
			pnew = MojRefCountPtrFromAtomic(pnew);
	}
	return pnew;
}

void MojRefCountRelease(void* p)
{
	if (p) {
		MojAtomicT* a = MojRefCountPtrToAtomic(p);
		if (MojAtomicDecrement(a) == 0) {
			MojAtomicDestroy(a);
			MojFree(a);
		}
	}
}
