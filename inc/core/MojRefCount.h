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


#ifndef MOJREFCOUNT_H_
#define MOJREFCOUNT_H_

#include "core/MojCoreDefs.h"
#include "core/MojAtomicInt.h"
#include "core/MojAutoPtr.h"

// allocation utils
void* MojRefCountAlloc(gsize numBytes);
template<class T>
T* MojRefCountNew(gsize numElems);
void* MojRefCountRealloc(void* p, gsize n);
gint32 MojRefCountGet(void* p);
void MojRefCountRetain(void* p);
void MojRefCountRelease(void* p);
template<class T>
void MojRefCountRelease(T* p, gsize numElems);

// ref-counted object
class MojRefCounted : private MojNoCopy
{
public:
	void retain() { ++m_refCount; }
	void release() { if (--m_refCount == 0) delete this; }
	gint32 refCount() const { return m_refCount.value(); }

protected:
	virtual ~MojRefCounted() {}

private:
	MojAtomicInt m_refCount;
};

// smart ptr to ref-counted object
template<class T>
struct MojRefCountedPtrRef
{
	explicit MojRefCountedPtrRef(T* p = NULL) : m_p(p) {}
	T* m_p;
};

template<class T>
class MojRefCountedPtr : public MojSmartPtrBase<T>
{
public:
	typedef MojSmartPtrBase<T> Base;
	MojRefCountedPtr(T* p = NULL) : Base(p) { retain(); }
	MojRefCountedPtr(const MojRefCountedPtr& ap) : Base(ap.get()) { retain(); }
	MojRefCountedPtr(MojRefCountedPtrRef<T> ref) : Base(ref.m_p) { retain(); }
	~MojRefCountedPtr() { release(); }

	void reset(T* p = NULL) { release(); m_p = p; retain(); }
	MojRefCountedPtr& operator=(const MojRefCountedPtr& rhs) { reset(rhs.get()); return *this; }
	MojRefCountedPtr& operator=(MojRefCountedPtrRef<T> rhs) { reset(rhs.m_p); return *this; }

	T& operator*() const { MojAssert(Base::m_p); return *Base::m_p; }
	template<class P>
	MojRefCountedPtr& operator=(const MojRefCountedPtr<P>& ap) { reset(ap.get()); return *this; }
	template <class P>
	operator MojRefCountedPtrRef<P>() { return MojRefCountedPtrRef<P>(m_p); }
	template<class P>
	operator MojRefCountedPtr<P>() { return MojRefCountedPtr<P>(m_p); }

private:
	using Base::m_p;

	void retain() { if (m_p) m_p->retain(); }
	void release() { if (m_p) m_p->release(); }
};

#include "core/internal/MojRefCountInternal.h"

#endif /* MOJREFCOUNT_H_ */
