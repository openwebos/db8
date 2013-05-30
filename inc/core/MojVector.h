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


#ifndef MOJVECTOR_H_
#define MOJVECTOR_H_

#include "core/MojCoreDefs.h"
#include "core/MojComp.h"
#include "core/MojRefCount.h"
#include "core/MojUtil.h"

template<class T, class EQ = MojEq<T>, class COMP = MojComp<T> >
class MojVector
{
public:
	typedef T ValueType;
	typedef T* Iterator;
	typedef const T* ConstIterator;

	MojVector() : m_begin(NULL), m_end(NULL), m_endAlloc(NULL) {}
	MojVector(const MojVector& v);
	~MojVector() { release(); }

	gsize size() const { return end() - begin(); }
	gsize capacity() const { return m_endAlloc - begin(); }
	bool empty() const { return end() == begin(); }

	ConstIterator begin() const { return ConstIterator(m_begin); }
	ConstIterator end() const { return ConstIterator(m_end); }

	const ValueType& at(gsize idx) const { MojAssert(idx < size()); return begin()[idx]; }
	const ValueType& front() const { MojAssert(!empty()); return *begin(); }
	const ValueType& back() const { MojAssert(!empty()); return *(end() - 1); }
	gsize find(const ValueType& ValueType, gsize startIdx = 0) const;

	void clear();
	void swap(MojVector& v) { MojSwap(*this, v); }

	void assign(const MojVector& v);
	MojErr assign(ConstIterator rangeBegin, ConstIterator rangeEnd);
	int compare(const MojVector& v) const;

	MojErr begin(Iterator& i);
	MojErr end(Iterator& i);
	MojErr reserve(gsize numElems);
	MojErr resize(gsize numElems, const ValueType& val = ValueType());
	MojErr push(const ValueType& ValueType);
	MojErr pop() { MojAssert(!empty()); return erase(size() - 1); }
	MojErr setAt(gsize idx, const ValueType& val);
	MojErr append(ConstIterator rangeBegin, ConstIterator rangeEnd);
	MojErr insert(gsize idx, gsize numElems, const ValueType& val);
	MojErr insert(gsize idx, ConstIterator rangeBegin, ConstIterator rangeEnd);
	MojErr erase(gsize idx, gsize numElems = 1);
	MojErr reverse();
	MojErr sort();

	MojVector& operator=(const MojVector& rhs) { assign(rhs); return *this; }
	const ValueType& operator[](gsize idx) const { return at(idx); }
	bool operator==(const MojVector& rhs) const;
	bool operator!=(const MojVector& rhs) const { return !operator==(rhs); }
	bool operator<(const MojVector& rhs) const { return compare(rhs) < 0; }
	bool operator<=(const MojVector& rhs) const { return compare(rhs) <= 0; }
	bool operator>(const MojVector& rhs) const { return compare(rhs) > 0; }
	bool operator>=(const MojVector& rhs) const { return compare(rhs) >= 0; }

private:
	static const gsize InitialSize;

	MojErr shift(gsize idx, gsize numElems);
	MojErr realloc(gsize numElems);
	MojErr ensureWritable();
	MojErr ensureSpace(gsize numElems);
	void reset(Iterator begin, Iterator end, Iterator endAlloc);
	void release();

	Iterator m_begin;
	Iterator m_end;
	Iterator m_endAlloc;
};

#include "core/internal/MojVectorInternal.h"

#endif /* MOJVECTOR_H_ */
