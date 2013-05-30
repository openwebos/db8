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


#ifndef MOJOSINTERNAL_H_
#define MOJOSINTERNAL_H_

#ifdef MOJ_NEED_ATOMIC_INIT
inline void MojAtomicInit(MojAtomicT* a, gint32 i)
{
	MojAssert(a);
	MojAtomicSet(a, i);
}
#endif /* MOJ_NEED_ATOMIC_INIT */

#ifdef MOJ_NEED_ATOMIC_DESTROY
inline void MojAtomicDestroy(MojAtomicT* a)
{
	MojAssert(a);
}
#endif /* MOJ_NEED_ATOMIC_DESTROY */

#ifdef MOJ_NEED_ATOMIC_GET
inline gint32 MojAtomicGet(const MojAtomicT* a)
{
	MojAssert(a);
	return a->val;
}
#endif /* MOJ_NEED_ATOMIC_GET */

#ifdef MOJ_NEED_ATOMIC_SET
inline void MojAtomicSet(MojAtomicT* a, gint32 i)
{
	MojAssert(a);
	a->val = i;
}
#endif /* MOJ_NEED_ATOMIC_SET */

#ifdef MOJ_USE_ERRNO
inline MojErr MojErrno()
{
	return (MojErr) errno;
}
#endif /* MOJ_USE_ERRNO */

#ifdef MOJ_USE_GNU_STRERROR_R
inline const MojChar* MojStrError(int err, MojChar* buf, gsize bufLen)
{
	return strerror_r(err, buf, bufLen);
}
#endif /* MOJ_USE_GNU_STRERROR_R */

#ifdef MOJ_USE_POSIX_STRERROR_R
inline const MojChar* MojStrError(int err, MojChar* buf, gsize bufLen)
{
	strerror_r(err, buf, bufLen);
	return buf;
}
#endif /* MOJ_USE_POSIX_STRERROR_R */

#ifdef MOJ_NEED_DEBUGBREAK
inline void MojDebugBreak()
{
#ifdef MOJ_X86
	asm ("int $3");
#endif
}
#endif /* MOJ_NEED_DEBUGGREAK */

#ifdef MOJ_USE_ABORT
inline void MojAbort()
{
	abort();
}
#endif /* MOJ_USE_ABORT */

#ifdef MOJ_USE_FREE
inline void MojFree(void* p)
{
	free(p);
}
#endif /* MOJ_USE_FREE */

#ifdef MOJ_USE_MALLOC
inline void* MojMalloc(gsize size)
{
	return malloc(size);
}
#endif /* MOJ_USE_MALLOC */

#ifdef MOJ_USE_REALLOC
inline void* MojRealloc(void* p, gsize size)
{
	return realloc(p, size);
}
#endif /* MOJ_USE_REALLOC */

#ifdef MOJ_USE_RAND_R
inline int MojRand(unsigned int* seed)
{
	return rand_r(seed);
}
#endif /* MOJ_USE_RAND_R */

#ifdef MOJ_USE_BZERO
inline void MojZero(void* dest, gsize size)
{
	bzero(dest, size);
}
#endif /* MOJ_USE_BZERO */

#ifdef MOJ_USE_MEMCHR
inline const MojChar* MojMemChr(const MojChar* data, MojChar c, gsize len)
{
	MojAssert(data || len == 0);
	return (const MojChar*) memchr(data, c, len);
}
#endif /* MOJ_USE_MEMCHR */

#ifdef MOJ_USE_MEMRCHR
inline const MojChar* MojMemChrReverse(const MojChar* data, MojChar c, gsize len)
{
	MojAssert(data || len == 0);
	return (const MojChar*) memrchr(data, c, len);
}
#endif /* MOJ_USE_MEMRCHR */

#ifdef MOJ_USE_MEMCMP
inline int MojMemCmp(const void* p1, const void* p2, gsize size)
{
	MojAssert((p1 && p2) || size == 0);
	return memcmp(p1, p2, size);
}
#endif /* MOJ_USE_MEMCMP */

#ifdef MOJ_USE_MEMCPY
inline void* MojMemCpy(void* dest, const void* src, gsize size)
{
	MojAssert((dest && src) || size == 0);
	return memcpy(dest, (void*) src, size);
}
#endif /* MOJ_USE_MEMCPY */

#ifdef MOJ_USE_MEMMOVE
inline void* MojMemMove(void* dest, const void* src, gsize size)
{
	MojAssert((dest && src) || size == 0);
	return memmove(dest, (void*) src, size);
}
#endif /* MOJ_USE_MEMMOVE */

#ifdef MOJ_USE_STRCHR
inline const MojChar* MojStrChr(const MojChar* str, MojChar c)
{
	return strchr(str, c);
}
#endif /* MOJ_USE_STRCHR */

#ifdef MOJ_USE_STRRCHR
inline const MojChar* MojStrChrReverse(const MojChar* str, MojChar c)
{
	return strrchr(str, c);
}
#endif /* MOJ_USE_STRRCHR */

#ifdef MOJ_USE_STRSTR
inline const MojChar* MojStrStr(const MojChar* haystack, const MojChar* needle)
{
	return strstr(haystack, needle);
}
#endif /* MOJ_USE_STRSTR */

#ifdef MOJ_USE_STRCASECMP
inline int MojStrCaseCmp(const MojChar* str1, const MojChar* str2)
{
	MojAssert(str1 && str2);
	return strcasecmp(str1, str2);
}
#endif /* MOJ_USE_STRCASECMP */

#ifdef MOJ_USE_STRNCASECMP
inline int MojStrNCaseCmp(const MojChar* str1, const MojChar* str2, gsize len)
{
	MojAssert(str1 && str2);
	return strncasecmp(str1, str2, len);
}
#endif /* MOJ_USE_STRNCASECMP */

#ifdef MOJ_USE_STRCMP
inline int MojStrCmp(const MojChar* str1, const MojChar* str2)
{
	MojAssert(str1 && str2);
	return strcmp(str1, str2);
}
#endif /* MOJ_USE_STRCMP */

#ifdef MOJ_USE_STRNCMP
inline int MojStrNCmp(const MojChar* str1, const MojChar* str2, gsize len)
{
	MojAssert(str1 && str2);
	return strncmp(str1, str2, len);
}
#endif /* MOJ_USE_STRNCMP */

#ifdef MOJ_USE_STRCPY
inline MojChar* MojStrCpy(MojChar* strDest, const MojChar* strSrc)
{
	MojAssert(strDest && strSrc);
	return strcpy(strDest, strSrc);
}
#endif /* MOJ_USE_STRCMP */

#ifdef MOJ_USE_STRNCPY
inline MojChar* MojStrNCpy(MojChar* strDest, const MojChar* strSrc, gsize n)
{
	MojAssert(strDest && strSrc);
	return strncpy(strDest, strSrc, n);
}
#endif /* MOJ_USE_STRNCPY */

#ifdef MOJ_USE_STRLEN
inline gsize MojStrLen(const MojChar* str)
{
	MojAssert(str);
	return (gsize) strlen(str);
}
#endif /* MOJ_USE_STRLEN */

#ifdef MOJ_USE_STRTOD
inline gdouble MojStrToDouble(const MojChar* str, const MojChar** end)
{
	MojAssert(str);
	return strtod(str, (MojChar**) end);
}
#endif /* MOJ_USE_STRTOD */

#ifdef MOJ_USE_STRTOLL
inline gint64 MojStrToInt64(const MojChar* str, const MojChar** end, int base)
{
	MojAssert(str);
	return strtoll(str, (MojChar**) end, base);
}
#endif /* MOJ_USE_STRTOLL */

#ifdef MOJ_USE_ISALNUM
inline bool MojIsAlNum(MojChar c)
{
	return (bool) isalnum(c);
}
#endif /* MOJ_USE_ISALNUM */

#ifdef MOJ_USE_ISSPACE
inline bool MojIsSpace(MojChar c)
{
	return (bool) isspace(c);
}
#endif /* MOJ_USE_ISSPACE */

#ifdef MOJ_USE_ISDIGIT
inline bool MojIsDigit(MojChar c)
{
	return (bool) isdigit(c);
}
#endif /* MOJ_USE_ISDIGIT */

#ifdef MOJ_USE_ISXDIGIT
inline bool MojIsHexDigit(MojChar c)
{
	return (bool) isxdigit(c);
}
#endif /* MOJ_USE_ISXDIGIT */

#ifdef MOJ_USE_TOLOWER
inline MojChar MojToLower(MojChar c)
{
	return (MojChar) tolower(c);
}
#endif /* MOJ_USE_TOLOWER */

#ifdef MOJ_USE_TOUPPER
inline MojChar MojToUpper(MojChar c)
{
	return (MojChar) toupper(c);
}
#endif /* MOJ_USE_TOUPPER */

#ifdef MOJ_LITTLE_ENDIAN
inline guint16 MojUInt16ToBigEndian(guint16 val)
{
	return (guint16) (((val & 0xFF00) >> 8) + ((val & 0xFF) << 8));
}

inline guint32 MojUInt32ToBigEndian(guint32 val)
{
	return ((val & 0xFF000000) >> 24) + ((val & 0xFF0000) >> 8)
		+ ((val & 0xFF00) << 8) + ((val & 0xFF) << 24);
}

inline gint64 MojInt64ToBigEndian(gint64 val)
{
	return ((val & 0xFF00000000000000LL) >> 56) + ((val & 0xFF000000000000LL) >> 40)
		+ ((val & 0xFF0000000000LL) >> 24) + ((val & 0xFF00000000LL) >> 8)
		+ ((val & 0xFF000000) << 8) + ((val & 0xFF0000) << 24)
		+ ((val & 0xFF00) << 40) + ((val & 0xFF) << 56);
}
#else /* MOJ_LITTLE_ENDIAN */
inline guint16 MojUInt16ToBigEndian(guint16 val)
{
	return val;
}

inline guint32 MojUInt32ToBigEndian(guint32 val)
{
	return val;
}

inline gint64 MojInt64ToBigEndian(gint64 val)
{
	return val;
}
#endif /* MOJ_LITTLE_ENDIAN */

#ifdef MOJ_USE_PTHREADS
inline MojErr MojThreadDetach(MojThreadT thread)
{
	return (MojErr) pthread_detach(thread);
}

inline MojErr MojThreadYield()
{
#ifdef __USE_GNU
	return (MojErr) pthread_yield();
#else
 	pthread_yield_np();
	return MojErrNone;
#endif
}

inline MojThreadT MojThreadCurrent()
{
	return pthread_self();
}

inline MojThreadT MojThreadCurrentId()
{
	return pthread_self();
}

inline MojErr MojThreadMutexInit(MojThreadMutexT* mutex)
{
	MojAssert(mutex);
	return (MojErr) pthread_mutex_init(mutex, NULL);
}

inline MojErr MojThreadMutexDestroy(MojThreadMutexT* mutex)
{
	MojAssert(mutex);
	return (MojErr) pthread_mutex_destroy(mutex);
}

inline MojErr MojThreadMutexLock(MojThreadMutexT* mutex)
{
	MojAssert(mutex);
	return (MojErr) pthread_mutex_lock(mutex);
}

inline MojErr MojThreadMutexTryLock(MojThreadMutexT* mutex)
{
	MojAssert(mutex);
	return (MojErr) pthread_mutex_trylock(mutex);
}

inline MojErr MojThreadMutexUnlock(MojThreadMutexT* mutex)
{
	MojAssert(mutex);
	return (MojErr) pthread_mutex_unlock(mutex);
}

inline MojErr MojThreadCondInit(MojThreadCondT* cond)
{
	MojAssert(cond);
	return (MojErr) pthread_cond_init(cond, NULL);
}

inline MojErr MojThreadCondDestroy(MojThreadCondT* cond)
{
	MojAssert(cond);
	return (MojErr) pthread_cond_destroy(cond);
}

inline MojErr MojThreadCondSignal(MojThreadCondT* cond)
{
	MojAssert(cond);
	return (MojErr) pthread_cond_signal(cond);
}

inline MojErr MojThreadCondBroadcast(MojThreadCondT* cond)
{
	MojAssert(cond);
	return (MojErr) pthread_cond_broadcast(cond);
}

inline MojErr MojThreadCondWait(MojThreadCondT* cond, MojThreadMutexT* mutex)
{
	MojAssert(cond && mutex);
	return (MojErr) pthread_cond_wait(cond, mutex);
}

inline MojErr MojThreadRwLockInit(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_init(lock, NULL);
}

inline MojErr MojThreadRwLockDestroy(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_destroy(lock);
}

inline MojErr MojThreadRwLockReadLock(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_rdlock(lock);
}

inline MojErr MojThreadRwLockTryReadLock(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_tryrdlock(lock);
}

inline MojErr MojThreadRwLockWriteLock(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_wrlock(lock);
}

inline MojErr MojThreadRwLockTryWriteLock(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_trywrlock(lock);
}

inline MojErr MojThreadRwLockUnlock(MojThreadRwLockT* lock)
{
	MojAssert(lock);
	return (MojErr) pthread_rwlock_unlock(lock);
}

inline MojErr MojThreadKeyCreate(MojThreadKeyT& keyOut, MojDestructorFn destructor)
{
	return (MojErr) pthread_key_create(&keyOut, destructor);
}

inline MojErr MojThreadKeyDelete(MojThreadKeyT key)
{
	return (MojErr) pthread_key_delete(key);
}

inline MojErr MojThreadSetSpecific(MojThreadKeyT key, void* val)
{
	return (MojErr) pthread_setspecific(key, val);
}

inline void* MojThreadGetSpecific(MojThreadKeyT key)
{
	return pthread_getspecific(key);
}
#endif /* MOJ_USE_PTHREADS */

#ifdef MOJ_LINUX
#	include "core/internal/MojOsLinuxInternal.h"
#endif
#ifdef MOJ_MAC
#	include "core/internal/MojOsMacInternal.h"
#endif
#ifdef MOJ_WIN
#	include "core/internal/MojOsWinInternal.h"
#endif

#endif /* MOJOSINTERNAL_H_ */
