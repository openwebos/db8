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


#ifndef MOJUTILINTERNAL_H_
#define MOJUTILINTERNAL_H_

#include "core/MojComp.h"

// TEMPLATE FUNCTION IMPLEMENTATIONS
template<class T>
void MojConstruct(T* p, MojSize numElems)
{
	MojAssert(p);
	// TODO: template specialization that's a no-op on scalar types
	for (T* end = p + numElems; p < end; ++p) {
		new(p) T();
	}
}

template<class T>
void MojDestroy(T* p, MojSize numElems)
{
	MojAssert(p);
	// TODO: template specialization that's a no-op on scalar types
	for (T* end = p + numElems; p < end; ++p) {
		p->~T();
	}
}

template<class T>
void MojDeleteRange(T iter, T iterEnd)
{
	for (; iter != iterEnd; ++iter)
		delete *iter;
}

template<class T>
T MojPostIncrement(T& t)
{
	T tmp(t);
	++t;
	return tmp;
}

template<class T>
T MojPostDecrement(T& t)
{
	T tmp(t);
	--t;
	return tmp;
}

template<class T>
T MojAbs(const T& t)
{
	return t < 0 ? -t : t;
}

template<class T>
inline T& MojMin(T& t1, T& t2)
{
	return (t1 < t2) ? t1 : t2;
}

template<class T>
inline const T& MojMin(const T& t1, const T& t2)
{
	return (t1 < t2) ? t1 : t2;
}

template<class T>
inline T& MojMax(T& t1, T& t2)
{
	return (t1 < t2) ? t2 : t1;
}

template<class T>
inline const T& MojMax(const T& t1, const T& t2)
{
	return (t1 < t2) ? t2 : t1;
}

template<class T>
void MojSwap(T& t1, T& t2)
{
	T tmp(t1);
	t1 = t2;
	t2 = tmp;
}

template<class T, class COMP>
MojSize MojBinarySearch(T key, const T* array, MojSize numElems)
{
	MojAssert (array || numElems == 0);

	MojSize low = 0;
	MojSize high = numElems;
	while (low < high) {
		MojSize mid = low + ((high - low) / 2);
		int comp = COMP()(key, array[mid]);
		if (comp == 0)
			return mid;
		else if (comp > 0)
			low = mid + 1;
		else
			high = mid;
	}
	return MojInvalidSize;
}

template<class T>
inline MojSize MojBinarySearch(T key, const T* array, MojSize numElems)
{
	return MojBinarySearch<T, MojComp<T> >(key, array, numElems);
}

template<class T, class COMP>
MojSize MojQuickSortPartition(T* array, MojSize left, MojSize right, const COMP& comp)
{
	// pick random pivot
	unsigned int seed = (unsigned int) left;
	MojSize pivot = left + (MojRand(&seed) % (right - left));
	MojSize last = right - 1;
	MojSize mid = left;
	// move pivot to end
	MojSwap(array[pivot], array[last]);
	// move all elements < pivot to left of mid
	for (MojSize i = left; i < last; ++i) {
		if (comp(array[i], array[last]) < 0) {
			MojSwap(array[i], array[mid]);
			++mid;
		}
	}
	// move pivot to mid
	MojSwap(array[mid], array[last]);

	return mid;
}

template<class T, class COMP>
void MojQuickSortImpl(T* array, MojSize left, MojSize right, const COMP& comp)
{
	MojAssert (array || (right - left) == 0);
	if ((left + 1) >= right)
		return;
	MojSize mid = MojQuickSortPartition<T, COMP>(array, left, right, comp);
	MojQuickSortImpl<T, COMP>(array, left, mid, comp);
	MojQuickSortImpl<T, COMP>(array, mid + 1, right, comp);
}

template<class T, class COMP>
inline void MojQuickSort(T* array, MojSize numElems, const COMP& comp = COMP())
{
	return MojQuickSortImpl<T, COMP>(array, 0, numElems, comp);
}

template<class T>
inline void MojQuickSort(T* array, MojSize numElems)
{
	return MojQuickSort<T, MojComp<T> >(array, numElems);
}

inline bool MojFlagGet(MojUInt32 flags, MojUInt32 mask)
{
	return (flags & mask) == mask;
}

inline void MojFlagSet(MojUInt32& flags, MojUInt32 mask, bool val)
{
	if (val)
		flags |= mask;
	else
		flags &= ~mask;
}


#endif /* MOJUTILINTERNAL_H_ */
