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


#ifndef MOJTHREADINTERNAL_H_
#define MOJTHREADINTERNAL_H_

#include "core/MojAutoPtr.h"

inline MojThreadMutex::MojThreadMutex()
#ifdef MOJ_DEBUG
: m_owner(MojInvalidThreadId)
#endif
{
	MojErr err = MojThreadMutexInit(&m_mutex);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojThreadMutex::~MojThreadMutex()
{
#ifdef MOJ_DEBUG
	MojAssert(m_owner == MojInvalidThreadId);
#endif
	MojErr err = MojThreadMutexDestroy(&m_mutex);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline void MojThreadMutex::lock()
{
#ifdef MOJ_DEBUG
	MojAssert(m_owner != MojThreadCurrentId());
#endif
	MojErr err = MojThreadMutexLock(&m_mutex);
	MojAssert(err == MojErrNone);
	MojUnused(err);
#ifdef MOJ_DEBUG
	MojAssert(m_owner == MojInvalidThreadId);
	m_owner = MojThreadCurrentId();
#endif
}

inline bool MojThreadMutex::tryLock()
{
#ifdef MOJ_DEBUG
	MojAssert(m_owner != MojThreadCurrentId());
#endif
	MojErr err = MojThreadMutexTryLock(&m_mutex);
	MojAssert(err == MojErrNone || err == MojErrBusy);
	MojUnused(err);
	bool acquired = (err == MojErrNone);
#ifdef MOJ_DEBUG
	if (acquired)
		m_owner = MojThreadCurrentId();
#endif
	return acquired;
}

inline void MojThreadMutex::unlock()
{
#ifdef MOJ_DEBUG
	MojAssert(m_owner == MojThreadCurrentId());
	m_owner = MojInvalidThreadId;
#endif
	MojErr err = MojThreadMutexUnlock(&m_mutex);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojThreadRwLock::MojThreadRwLock()
#ifdef MOJ_DEBUG
  : m_writer(MojInvalidThreadId)
#endif
{
	MojErr err = MojThreadRwLockInit(&m_lock);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojThreadRwLock::~MojThreadRwLock()
{
#ifdef MOJ_DEBUG
	MojAssert(m_readerCount == 0 && m_writer == MojInvalidThreadId);
#endif
	MojErr err = MojThreadRwLockDestroy(&m_lock);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline void MojThreadRwLock::readLock()
{
	MojErr err = MojThreadRwLockReadLock(&m_lock);
	MojAssert(err == MojErrNone);
	MojUnused(err);
#ifdef MOJ_DEBUG
	MojAssert(m_readerCount >= 0);
	MojAssert(m_writer == MojInvalidThreadId);
	++m_readerCount;
#endif
}

inline void MojThreadRwLock::writeLock()
{
	MojErr err = MojThreadRwLockWriteLock(&m_lock);
	MojAssert(err == MojErrNone);
	MojUnused(err);
#ifdef MOJ_DEBUG
	MojAssert(m_readerCount == 0);
	MojAssert(m_writer == MojInvalidThreadId);
	m_writer = MojThreadCurrentId();
#endif
}

inline bool MojThreadRwLock::tryReadLock()
{
	MojErr err = MojThreadRwLockTryReadLock(&m_lock);
	MojAssert(err == MojErrNone || err == MojErrBusy);
	MojUnused(err);
	bool acquired = (err == MojErrNone);
#ifdef MOJ_DEBUG
	if (acquired) {
		MojAssert(m_readerCount >= 0);
		MojAssert(m_writer == MojInvalidThreadId);
		++m_readerCount;
	}
#endif
	return acquired;
}

inline bool MojThreadRwLock::tryWriteLock()
{
	MojErr err = MojThreadRwLockTryWriteLock(&m_lock);
	MojAssert(err == MojErrNone || err == MojErrBusy);
	MojUnused(err);
	bool acquired = (err == MojErrNone);
#ifdef MOJ_DEBUG
	if (acquired) {
		MojAssert(m_readerCount == 0);
		MojAssert(m_writer == MojInvalidThreadId);
		m_writer = MojThreadCurrentId();
	}
#endif
	return acquired;
}

inline void MojThreadRwLock::unlock()
{
#ifdef MOJ_DEBUG
	if (m_writer == MojThreadCurrentId()) {
		MojAssert(m_readerCount == 0);
		m_writer = MojInvalidThreadId;
	} else {
		MojAssert(m_readerCount >= 1);
		--m_readerCount;
	}
#endif
	MojErr err = MojThreadRwLockUnlock(&m_lock);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojThreadCond::MojThreadCond()
{
	MojErr err = MojThreadCondInit(&m_cond);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojThreadCond::~MojThreadCond()
{
	MojErr err = MojThreadCondDestroy(&m_cond);
	MojAssert(err == MojErrNone);
	MojUnused(err);
}

inline MojErr MojThreadCond::wait(MojThreadMutex& mutex)
{
#ifdef MOJ_DEBUG
	MojAssert(mutex.m_owner == MojThreadCurrentId());
	mutex.m_owner = MojInvalidThreadId;
#endif
	MojErr err = MojThreadCondWait(&m_cond, &mutex.m_mutex);
#ifdef MOJ_DEBUG
	MojAssert(mutex.m_owner == MojInvalidThreadId);
	mutex.m_owner = MojThreadCurrentId();
#endif
	return err;
}

inline MojThreadGuard::MojThreadGuard(MojThreadMutex& mutex)
: m_mutex(mutex),
  m_locked(true)
{
	mutex.lock();
}

inline MojThreadGuard::MojThreadGuard(MojThreadMutex& mutex, bool lockNow)
: m_mutex(mutex),
  m_locked(lockNow)
{
	if (lockNow)
		mutex.lock();
}

inline MojThreadGuard::~MojThreadGuard()
{
	if (m_locked)
		m_mutex.unlock();
}

inline void MojThreadGuard::lock()
{
	MojAssert(!m_locked);
	m_mutex.lock();
	m_locked = true;
}

inline void MojThreadGuard::unlock()
{
	MojAssert(m_locked);
	m_mutex.unlock();
	m_locked = false;
}

inline bool MojThreadGuard::tryLock()
{
	MojAssert(!m_locked);
	m_locked = m_mutex.tryLock();
	return m_locked;
}

inline MojThreadReadGuard::MojThreadReadGuard(MojThreadRwLock& lock)
: m_lock(lock),
  m_locked(true)
{
	lock.readLock();
}

inline MojThreadReadGuard::MojThreadReadGuard(MojThreadRwLock& lock, bool lockNow)
: m_lock(lock),
  m_locked(lockNow)
{
	if (lockNow)
		lock.readLock();
}

inline MojThreadReadGuard::~MojThreadReadGuard()
{
	if (m_locked)
		m_lock.unlock();
}

inline void MojThreadReadGuard::lock()
{
	MojAssert(!m_locked);
	m_lock.readLock();
	m_locked = true;
}

inline void MojThreadReadGuard::unlock()
{
	MojAssert(m_locked);
	m_lock.unlock();
	m_locked = false;
}

inline bool MojThreadReadGuard::tryLock()
{
	MojAssert(!m_locked);
	m_locked = m_lock.tryReadLock();
	return m_locked;
}

inline MojThreadWriteGuard::MojThreadWriteGuard(MojThreadRwLock& lock)
: m_lock(lock),
  m_locked(true)
{
	lock.writeLock();
}

inline MojThreadWriteGuard::MojThreadWriteGuard(MojThreadRwLock& lock, bool lockNow)
: m_lock(lock),
  m_locked(lockNow)
{
	if (lockNow)
		lock.writeLock();
}

inline MojThreadWriteGuard::~MojThreadWriteGuard()
{
	if (m_locked)
		m_lock.unlock();
}

inline void MojThreadWriteGuard::lock()
{
	MojAssert(!m_locked);
	m_lock.writeLock();
	m_locked = true;
}

inline void MojThreadWriteGuard::unlock()
{
	MojAssert(m_locked);
	m_lock.unlock();
	m_locked = false;
}

inline bool MojThreadWriteGuard::tryLock()
{
	MojAssert(!m_locked);
	m_locked = m_lock.tryWriteLock();
	return m_locked;
}

template <class T, class CTOR>
MojThreadLocalValue<T, CTOR>::MojThreadLocalValue()
: m_key(MojInvalidThreadKey)
{
	MojThreadKeyT key;
	MojErr err = MojThreadKeyCreate(key, &destroy);
	if (err == MojErrNone) {
		m_key = key;
	} else {
		MojAssertNotReached();
	}
}

template <class T, class CTOR>
MojThreadLocalValue<T, CTOR>::~MojThreadLocalValue()
{
	if (m_key != MojInvalidThreadKey) {
		// main thread tss won't get cleanup up unless we do it here
		T* val = static_cast<T*>(MojThreadGetSpecific(m_key));
		MojErr err = MojThreadSetSpecific(m_key, NULL);
		if (err == MojErrNone) {
			delete val;
		} else {
			MojAssertNotReached();
		}

		err = MojThreadKeyDelete(m_key);
		MojErrCatchAll(err);
	}
}

template <class T, class CTOR>
MojErr MojThreadLocalValue<T, CTOR>::get(T*& valOut)
{
	valOut = NULL;
	if (m_key == MojInvalidThreadKey)
		MojErrThrow(MojErrNotInitialized);

	valOut = static_cast<T*>(MojThreadGetSpecific(m_key));
	if (valOut == NULL) {
		CTOR ctor;
		MojAutoPtr<T> newVal(ctor());
		MojAllocCheck(newVal.get());
		MojErr err = MojThreadSetSpecific(m_key, newVal.get());
		MojErrCheck(err);
		valOut = newVal.release();
	}
	return MojErrNone;
}

template <class T, class CTOR>
void MojThreadLocalValue<T, CTOR>::destroy(void* val)
{
	delete static_cast<T*>(val);
}

#endif /* MOJTHREADINTERNAL_H_ */
