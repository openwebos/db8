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


#ifndef MOJOSMACINTERNAL_H_
#define MOJOSMACINTERNAL_H_

#include <libkern/OSAtomic.h>

inline MojInt32 MojAtomicAdd(MojAtomicT* a, MojInt32 incr)
{
	MojAssert(a);
	return OSAtomicAdd32(incr, &a->val);
}

inline MojInt32 MojAtomicIncrement(MojAtomicT* a)
{
	MojAssert(a);
	return OSAtomicIncrement32(&a->val);
}

inline MojInt32 MojAtomicDecrement(MojAtomicT* a)
{
	MojAssert(a);
	return OSAtomicDecrement32(&a->val);
}

#endif /* MOJOSMACINTERNAL_H_ */
