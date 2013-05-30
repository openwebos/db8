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


#ifndef MOJHASHER_H_
#define MOJHASHER_H_

#include "core/MojCoreDefs.h"
#include "core/MojUtil.h"

template<class T>
struct MojHasher
{
};

template<>
struct MojHasher<const MojChar*>
{
	gsize operator()(const MojChar* str)
	{
		return MojHash(str);
	}
};

template<>
struct MojHasher<MojChar*> : public MojHasher<const MojChar*> {};

template<class T>
struct MojIntHasher
{
	gsize operator()(T i)
	{
		return MojHash(&i, sizeof(i));
	}
};

template<>
struct MojHasher<gint8> : public MojIntHasher<gint8> {};
template<>
struct MojHasher<guint8> : public MojIntHasher<guint8> {};
template<>
struct MojHasher<gint16> : public MojIntHasher<gint16> {};
template<>
struct MojHasher<guint16> : public MojIntHasher<guint16> {};
template<>
struct MojHasher<gint32> : public MojIntHasher<gint32> {};
template<>
struct MojHasher<guint32> : public MojIntHasher<guint32> {};
template<>
struct MojHasher<gint64> : public MojIntHasher<gint64> {};
template<>
struct MojHasher<guint64> : public MojIntHasher<guint64> {};
template<>
struct MojHasher<class T*> : public MojIntHasher<T*> {};
template<>
struct MojHasher<long unsigned int> : public MojIntHasher<long unsigned int> {};

#endif /* MOJHASHER_H_ */
