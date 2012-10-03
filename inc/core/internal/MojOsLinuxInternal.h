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


#ifndef MOJOSLINUXINTERNAL_H_
#define MOJOSLINUXINTERNAL_H_

#if defined(MOJ_X86)
inline MojInt32 MojAtomicAdd(MojAtomicT* a, MojInt32 incr)
{
	MojAssert(a);
	MojInt32 i = incr;
	asm volatile(
			"lock xaddl %0, %1"
				: "+r" (i), "+m" (a->val)
				: : "memory");
	return incr + i;
}
#elif defined(MOJ_ARM)
inline MojInt32 MojAtomicAdd(MojAtomicT* a, MojInt32 incr)
{
	MojAssert(a);
#if __LINUX_ARM_ARCH__ < 6
	/* This could be used for all of the assembly language blocks, but we
         * don't want to perturb the code at the moment */
	return  __sync_add_and_fetch(&a->val, incr);
#else
	/* This only assembles for ARMv6 and later */
	MojUInt32 tmp;
	MojInt32 res;
	asm volatile(
			"1:	ldrex %0, [%2]\n"
			"add %0, %0, %3\n"
			"strex %1, %0, [%2]\n"
			"teq %1, #0\n"
			"bne 1b"
				: "=&r" (res), "=&r" (tmp)
				: "r" (&a->val), "Ir" (incr)
				: "cc");
	return res;
#endif
}
#endif

inline MojInt32 MojAtomicIncrement(MojAtomicT* a)
{
	return MojAtomicAdd(a, 1);
}

inline MojInt32 MojAtomicDecrement(MojAtomicT* a)
{
	return MojAtomicAdd(a, -1);
}

#endif /* MOJOSLINUXINTERNAL_H_ */
