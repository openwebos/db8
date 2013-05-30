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


#ifndef MOJUTIL_H_
#define MOJUTIL_H_

#include "core/MojCoreDefs.h"

template<class T>
void MojConstruct(T* p, gsize numElems);
template<class T>
void MojDestroy(T* p, gsize numElems);
template<class T>
void MojDeleteRange(T iter, T iterEnd);

template<class T>
T MojPostIncrement(T& t);
template<class T>
T MojPostDecrement(T& t);

template<class T>
T MojAbs(const T& t);

template<class T>
T& MojMin(T& t1, T& t2);
template<class T>
const T& MojMin(const T& t1, const T& t2);

template<class T>
T& MojMax(T& t1, T& t2);
template<class T>
const T& MojMax(const T& t1, const T& t2);

template<class T>
void MojSwap(T& t1, T& t2);

template<class T, class COMP>
gsize MojBinarySearch(T key, const T* array, gsize numElems);
template<class T>
gsize MojBinarySearch(T key, const T* array, gsize numElems);

template<class T, class COMP>
void MojQuickSort(T* array, gsize numElems, const COMP& comp = COMP());
template<class T>
void MojQuickSort(T* array, gsize numElems);

bool MojFlagGet(guint32 flags, guint32 mask);
void MojFlagSet(guint32& flags, guint32 mask, bool val);

gsize MojHash(const void* p, gsize size);
gsize MojHash(const MojChar* str);

inline gsize MojBase64EncodedLenMax(gsize srcSize) { return ((srcSize + 2) / 3) * 4; }
inline gsize MojBase64DecodedSizeMax(gsize encodedLen) { return ((encodedLen + 3) / 4) * 3; }
MojErr MojBase64Encode(const guint8* src, gsize srcSize, MojChar* bufOut, gsize bufLen, gsize& lenOut, bool pad = true);
MojErr MojBase64EncodeMIME(const guint8* src, gsize srcSize, MojChar* bufOut, gsize bufLen, gsize& lenOut, bool pad = true);
MojErr MojBase64Decode(const MojChar* src, gsize srcLen, guint8* bufOut, gsize bufSize, gsize& sizeOut);
MojErr MojBase64DecodeMIME(const MojChar* src, gsize srcLen, guint8* bufOut, gsize bufSize, gsize& sizeOut);


int MojLexicalCompare(const guint8* data1, gsize size1, const guint8* data2, gsize size2);
gsize MojPrefixSize(const guint8* data1, gsize size1, const guint8* data2, gsize size2);

MojErr MojCreateDirIfNotPresent(const MojChar* path);
MojErr MojRmDirRecursive(const MojChar* path);
MojErr MojFileToString(const MojChar* path, MojString& strOut);
MojErr MojFileFromString(const MojChar* path, const MojChar* data);
const MojChar* MojFileNameFromPath(const MojChar* path);
MojErr MojUInt8ArrayToHex(const guint8 *bytes, gsize len, MojChar *s);

#include "core/internal/MojUtilInternal.h"

#endif /* MOJUTIL_H_ */
