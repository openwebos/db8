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


#ifndef MOJAUTOPTR_H_
#define MOJAUTOPTR_H_

#include "core/MojCoreDefs.h"
#include "core/MojAtomicInt.h"
#include "core/MojNoCopy.h"

template<class T>
class MojSmartPtrBase : private MojNoCopy
{
public:
	T* get() const { return m_p; }
	T* operator->() const { MojAssert(m_p); return m_p; }

	bool operator==(const MojSmartPtrBase& rhs) const { return get() == rhs.get(); }
	bool operator!=(const MojSmartPtrBase& rhs) const { return get() != rhs.get(); }
	bool operator<(const MojSmartPtrBase& rhs) const { return get() < rhs.get(); }
	bool operator<=(const MojSmartPtrBase& rhs) const { return get() <= rhs.get(); }
	bool operator>(const MojSmartPtrBase& rhs) const { return get() > rhs.get(); }
	bool operator>=(const MojSmartPtrBase& rhs) const { return get() >= rhs.get(); }

protected:
	MojSmartPtrBase(T* p = NULL) : m_p(p) {}
	T*	m_p;
};

template<class T, class DTOR>
struct MojAutoPtrRef
{
	explicit MojAutoPtrRef(T* p = NULL) : m_p(p) {}
	T* m_p;
};

template<class T, class DTOR>
class MojAutoPtrBase : public MojSmartPtrBase<T>
{
public:
	typedef MojSmartPtrBase<T> Base;
	explicit MojAutoPtrBase(T* p = NULL) : Base(p) {}
	MojAutoPtrBase(MojAutoPtrBase& ap) : Base(ap.release()) {}
	MojAutoPtrBase(MojAutoPtrRef<T, DTOR> ref) : Base(ref.m_p) {}
	~MojAutoPtrBase() { if (m_p) DTOR()(m_p); }

	T* release() { T* p = m_p; m_p = NULL; return p; }
	void reset(T* p = NULL) { if (m_p) DTOR()(m_p); m_p = p; }

	MojAutoPtrBase& operator=(MojAutoPtrBase& rhs) { reset(rhs.release()); return *this; }
	MojAutoPtrBase& operator=(MojAutoPtrRef<T, DTOR> rhs) { reset(rhs.m_p); return *this; }

	template <class P>
	operator MojAutoPtrRef<P, DTOR>() { return MojAutoPtrRef<P, DTOR>(release()); }

protected:
	using Base::m_p;
};

template<class T>
struct MojDeleteDtor
{
	void operator()(T* p) { delete p; }
};

template<class T>
class MojAutoPtr : public MojAutoPtrBase<T, MojDeleteDtor<T> >
{
public:
	typedef MojAutoPtrBase<T, MojDeleteDtor<T> > Base;
	explicit MojAutoPtr(T* p = NULL) : Base(p) {}
	MojAutoPtr(MojAutoPtr& ap) : Base(ap) {}
	MojAutoPtr(MojAutoPtrRef<T, MojDeleteDtor<T> > ref) : Base(ref) {}
	template<class P>
	MojAutoPtr(MojAutoPtr<P>& ap) : Base(ap.release()) {}

	T& operator*() const { MojAssert(Base::m_p); return *Base::m_p; }
	template<class P>
	MojAutoPtr& operator=(MojAutoPtr<P>& ap) { this->reset(ap.release()); return *this; }
	template<class P>
	operator MojAutoPtr<P>() { return MojAutoPtr<P>(Base::release()); }
};

template<class T>
struct MojArrayDeleteDtor
{
	void operator()(T* p) { delete[] p; }
};

template<class T>
class MojAutoArrayPtr : public MojAutoPtrBase<T, MojArrayDeleteDtor<T> >
{
public:
	typedef MojAutoPtrBase<T, MojArrayDeleteDtor<T> > Base;
	explicit MojAutoArrayPtr(T* p = NULL) : Base(p) {}
	MojAutoArrayPtr(MojAutoArrayPtr& ap) : Base(ap) {}
	MojAutoArrayPtr(MojAutoPtrRef<T, MojArrayDeleteDtor<T> > ref) : Base(ref) {}

	T& operator*() const { MojAssert(Base::m_p); return *Base::m_p; }
	T& operator[](MojSize idx) { return Base::m_p[idx]; }
	const T& operator[](MojSize idx) const { return Base::m_p[idx]; }
};

struct MojFreeDtor
{
	void operator()(void* p) { MojFree(p); }
};

template<class T>
class MojAutoFreePtr : public MojAutoPtrBase<T, MojFreeDtor>
{
public:
	typedef MojAutoPtrBase<T, MojFreeDtor> Base;
	explicit MojAutoFreePtr(T* p = NULL) : Base(p) {}
	MojAutoFreePtr(MojAutoFreePtr& ap) : Base(ap) {}
	MojAutoFreePtr(MojAutoPtrRef<T, MojFreeDtor> ref) : Base(ref) {}
	T& operator*() const { MojAssert(Base::m_p); return *Base::m_p; }
	MojAutoFreePtr& operator=(MojAutoFreePtr& ap) { reset(ap.release()); return *this; }
};

template<class T>
struct MojCloseDtor
{
	void operator()(T* p) { p->close(); }
};

template<class T>
class MojAutoCloser : public MojAutoPtrBase<T, MojCloseDtor<T> >
{
public:
	typedef MojAutoPtrBase<T, MojCloseDtor<T> > Base;
	explicit MojAutoCloser(T* p = NULL) : Base(p) {}
	MojAutoCloser(MojAutoCloser& ap) : Base(ap) {}
	MojAutoCloser(MojAutoPtrRef<T, MojCloseDtor<T> > ref) : Base(ref) {}
	MojAutoCloser& operator=(MojAutoCloser& ap) { reset(ap.release()); return *this; }
};

template<class T, class DTOR>
struct MojSharedPtrImpl
{
	MojSharedPtrImpl(T* p = NULL) : m_p(p), m_refcount(1) {}
	~MojSharedPtrImpl() { destroy(); }
	void destroy() { MojAssert(m_refcount == 0); if (m_p) DTOR()(m_p); }

	T* m_p;
	MojAtomicInt m_refcount;
};

template<class T, class DTOR>
struct MojSharedPtrRef
{
	explicit MojSharedPtrRef(MojSharedPtrImpl<T, DTOR>* impl) : m_impl(impl) {}
	MojSharedPtrImpl<T, DTOR>* m_impl;
};

template<class T, class DTOR>
class MojSharedPtrBase
{
public:
	MojSharedPtrBase() : m_impl(NULL) {}
	MojSharedPtrBase(const MojSharedPtrBase& ptr) : m_impl(ptr.m_impl) { init(); }
	MojSharedPtrBase(MojSharedPtrRef<T, DTOR> ref) : m_impl(ref.m_impl) { init(); }
	~MojSharedPtrBase() { destroy(); }

	MojInt32 refcount() { return m_impl ? m_impl->m_refcount.value() : 0; }
	T* get() const { return m_impl ? m_impl->m_p : NULL; }
	void reset() { destroy(); m_impl = NULL; }
	MojErr reset(T* p);
	MojErr resetChecked(T* p);

	T* operator->() const { MojAssert(m_impl); return m_impl->m_p; }
	T& operator*() const { MojAssert(m_impl); return *m_impl->m_p; }
	MojSharedPtrBase& operator=(const MojSharedPtrBase& sp) { assign(sp.m_impl); return *this; }
	MojSharedPtrBase& operator=(MojSharedPtrRef<T, DTOR> rhs) { assign(rhs.m_impl); return *this; }
	bool operator==(const MojSharedPtrBase& rhs) const { return get() == rhs.get(); }
	bool operator!=(const MojSharedPtrBase& rhs) const { return get() != rhs.get(); }
	bool operator<(const MojSharedPtrBase& rhs) const { return get() < rhs.get(); }
	bool operator<=(const MojSharedPtrBase& rhs) const { return get() <= rhs.get(); }
	bool operator>(const MojSharedPtrBase& rhs) const { return get() > rhs.get(); }
	bool operator>=(const MojSharedPtrBase& rhs) const { return get() >= rhs.get(); }

	template <class P>
	operator MojSharedPtrRef<P, DTOR>() { return MojSharedPtrRef<P, DTOR>(m_impl); }

private:
	typedef MojSharedPtrImpl<T, DTOR> Impl;

	void init() { if (m_impl) ++(m_impl->m_refcount); }
	void destroy();
	void assign(Impl* impl);

	Impl* m_impl;
};

template<class T>
class MojSharedPtr : public MojSharedPtrBase<T, MojDeleteDtor<T> >
{
public:
	typedef MojSharedPtrBase<T, MojDeleteDtor<T> > Base;
	MojSharedPtr() : Base() {}
	MojSharedPtr(const MojSharedPtr& sp) : Base(sp) {}
	MojSharedPtr(MojSharedPtrRef<T, MojDeleteDtor<T> > ref) : Base(ref) {}
	template<class P>
	MojSharedPtr(const MojSharedPtr<P>& sp) : Base(reinterpret_cast<const MojSharedPtr<T>&>(sp)) { testAssignable((P*) NULL); }

	template<class P>
	MojSharedPtr& operator=(const MojSharedPtr<P>& rhs) { testAssignable((P*) NULL); return operator=(reinterpret_cast<const MojSharedPtr&>(rhs)); }
private:
	void testAssignable(T* p) {}
};

template<class T>
class MojSharedArrayPtr : public MojSharedPtrBase<T, MojArrayDeleteDtor<T> >
{
public:
	typedef MojSharedPtrBase<T, MojArrayDeleteDtor<T> > Base;
	MojSharedArrayPtr() : Base() {}
	MojSharedArrayPtr(const MojSharedArrayPtr& sp) : Base(sp) {}
	MojSharedArrayPtr(MojSharedPtrRef<T, MojArrayDeleteDtor<T> > ref) : Base(ref) {}
};

template<class T>
class MojSharedFreePtr : public MojSharedPtrBase<T, MojFreeDtor>
{
public:
	typedef MojSharedPtrBase<T, MojFreeDtor> Base;
	MojSharedFreePtr() : Base() {}
	MojSharedFreePtr(const MojSharedFreePtr& sp) : Base(sp) {}
	MojSharedFreePtr(MojSharedPtrRef<T, MojFreeDtor> ref) : Base(ref) {}
};

#include "core/internal/MojAutoPtrInternal.h"

#endif /* MOJAUTOPTR_H_ */
