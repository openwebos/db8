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


#ifndef MOJSHAREDTOKENSET_H_
#define MOJSHAREDTOKENSET_H_

#include "core/MojHashMap.h"
#include "core/MojObject.h"
#include "core/MojThread.h"
#include "core/MojVector.h"
#include "db/MojDbStorageEngine.h"

class MojSharedTokenSet : public MojRefCounted
{
public:
	typedef MojVector<MojString> TokenVec;

	virtual MojErr addToken(const MojChar* str, MojUInt8& tokenOut, TokenVec& vecOut, MojObject& tokenObjOut) = 0;
	virtual MojErr tokenSet(TokenVec& vecOut, MojObject& tokenObjOut) const = 0;
};

#endif /* MOJSHAREDTOKENSET_H_ */
