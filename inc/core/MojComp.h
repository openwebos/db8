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


#ifndef MOJCOMP_H_
#define MOJCOMP_H_

#include "core/MojCoreDefs.h"

template<class T>
struct MojEq
{
	bool operator()(const T& val1, const T& val2) const
	{
		return val1 == val2;
	}
};

template<class T>
struct MojLessThan
{
	bool operator()(const T& val1, const T& val2) const
	{
		return val1 < val2;
	}
};

template<class T>
struct MojLessThanEq
{
	bool operator()(const T& val1, const T& val2) const
	{
		return val1 <= val2;
	}
};

template<class T>
struct MojGreaterThan
{
	bool operator()(const T& val1, const T& val2) const
	{
		return val1 > val2;
	}
};

template<class T>
struct MojGreaterThanEq
{
	bool operator()(const T& val1, const T& val2) const
	{
		return val1 >= val2;
	}
};

template<class T>
struct MojComp
{
	int operator()(const T& val1, const T& val2) const
	{
		if (val1 < val2)
			return -1;
		if (val2 < val1)
			return 1;
		return 0;
	}
};

template<class T>
struct MojCompAddr
{
	int operator()(const T& val1, const T& val2) const
	{
		if (&val1 < &val2)
			return -1;
		if (&val2 < &val1)
			return 1;
		return 0;
	}
};

// SPECIALIZATIONS
template<>
struct MojEq<const MojChar*>
{
	bool operator()(const MojChar* str1, const MojChar* str2) const
	{
		return MojStrCmp(str1, str2) == 0;
	}
};

template<>
struct MojEq<MojChar*> : public MojEq<const MojChar*> {};
template<>
struct MojEq<MojString> : public MojEq<const MojChar*> {};

template<>
struct MojComp<const MojChar*>
{
	int operator()(const MojChar* str1, const MojChar* str2) const
	{
		return MojStrCmp(str1, str2);
	}
};

template<>
struct MojComp<MojChar*> : public MojComp<const MojChar*> {};

template<>
struct MojComp<int>
{
	int operator()(int val1, int val2) const
	{
		return val1 - val2;
	}
};

#endif /* MOJCOMP_H_ */
