/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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


#ifndef MOJTIMEINTERNAL_H_
#define MOJTIMEINTERNAL_H_

inline void MojTime::fromTimeval(const MojTimevalT* tv)
{
	MojAssert(tv);
	m_val = tv->tv_sec * UnitsPerSec + tv->tv_usec;
}

inline void MojTime::toTimeval(MojTimevalT* tvOut) const
{
	MojAssert(tvOut);
	tvOut->tv_sec = (MojTimeT) secs();
	tvOut->tv_usec = microsecsPart();
}

inline void MojTime::fromTimespec(const MojTimespecT* ts)
{
	MojAssert(ts);
	m_val = ts->tv_sec * UnitsPerSec + ts->tv_nsec / 1000;
}

inline void MojTime::toTimespec(MojTimespecT* tsOut) const
{
	MojAssert(tsOut);
	tsOut->tv_sec = (MojTimeT) secs();
	tsOut->tv_nsec = microsecsPart() * 1000;
}

#endif /* MOJTIMEINTERNAL_H_ */
