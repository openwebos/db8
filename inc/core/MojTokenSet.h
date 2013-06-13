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


#ifndef MOJTOKENSET_H_
#define MOJTOKENSET_H_

#include "core/MojCoreDefs.h"
#include "core/MojHashMap.h"
#include "core/MojSharedTokenSet.h"

class MojTokenSet : public MojNoCopy
{
public:
	static const MojUInt8 InvalidToken = 0;
	typedef MojSharedTokenSet::TokenVec TokenVec;

	MojTokenSet();

	MojErr init(MojSharedTokenSet* sharedSet);
	MojErr tokenFromString(const MojChar* str, MojUInt8& tokenOut, bool add);
	MojErr stringFromToken(MojUInt8 token, MojString& propNameOut) const;

private:
	MojRefCountedPtr<MojSharedTokenSet> m_sharedTokenSet;

	TokenVec m_tokenVec;
	MojObject m_obj;
};

#endif /* MOJTOKENSET_H_ */
