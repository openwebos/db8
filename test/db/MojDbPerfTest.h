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


#ifndef MOJDBPERFTEST_H_
#define MOJDBPERFTEST_H_

#include "MojDbPerfTestRunner.h"
#include "core/MojFile.h"

#include <time.h>

class MojDbPerfTest : public MojTestCase {
public:
	MojDbPerfTest(const MojChar* name);

	virtual MojErr run() = 0;
	virtual void cleanup() = 0;

	MojErr putKinds(MojDb& db, MojUInt64& putKindTime);
	MojErr timePutKind(MojDb& db, MojUInt64& putKindTime, MojObject kind);
	MojErr delKinds(MojDb& db);

	MojErr createSmallObj(MojObject& obj, MojUInt64 i);
	MojErr createMedObj(MojObject& obj, MojUInt64 i);
	MojErr createLargeObj(MojObject& obj, MojUInt64 i);
	MojErr createMedNestedObj(MojObject& obj, MojUInt64 i);
	MojErr createLargeNestedObj(MojObject& obj, MojUInt64 i);
	MojErr createMedArrayObj(MojObject& obj, MojUInt64 i);
	MojErr createLargeArrayObj(MojObject& obj, MojUInt64 i);

	MojErr fileWrite(MojFile& file, MojString buf);
	
	MojUInt64 timeDiff(timespec start, timespec end);

    MojObject lazySyncConfig() const;
    bool lazySync() const { return m_lazySync; }

    bool m_lazySync;

	static const MojChar* s_lastNames[];
	static const MojChar* s_firstNames[];
	static const MojChar* const MojPerfSmKindId;
	static const MojChar* const MojPerfSmKindStr;
	static const MojChar* const MojPerfMedKindId;
	static const MojChar* const MojPerfMedKindStr;
	static const MojChar* const MojPerfLgKindId;
	static const MojChar* const MojPerfLgKindStr;
	static const MojChar* const MojPerfMedNestedKindId;
	static const MojChar* const MojPerfMedNestedKindStr;
	static const MojChar* const MojPerfLgNestedKindId;
	static const MojChar* const MojPerfLgNestedKindStr;
	static const MojChar* const MojPerfMedArrayKindId;
	static const MojChar* const MojPerfMedArrayKindStr;
	static const MojChar* const MojPerfLgArrayKindId;
	static const MojChar* const MojPerfLgArrayKindStr;
	static const MojChar* const MojPerfSmKind2Id;
	static const MojChar* const MojPerfSmKind2Str;
	static const MojChar* const MojPerfMedKind2Id;
	static const MojChar* const MojPerfMedKind2Str;
	static const MojChar* const MojPerfLgKind2Id;
	static const MojChar* const MojPerfLgKind2Str;
	static const MojChar* const MojPerfMedNestedKind2Id;
	static const MojChar* const MojPerfMedNestedKind2Str;
	static const MojChar* const MojPerfLgNestedKind2Id;
	static const MojChar* const MojPerfLgNestedKind2Str;
	static const MojChar* const MojPerfMedArrayKind2Id;
	static const MojChar* const MojPerfMedArrayKind2Str;
	static const MojChar* const MojPerfLgArrayKind2Id;
	static const MojChar* const MojPerfLgArrayKind2Str;
	static const MojChar* const MojPerfSmKindExtraIndex;
	static const MojChar* const MojPerfMedKindExtraIndex;
	static const MojChar* const MojPerfLgKindExtraIndex;
	static const MojChar* const MojPerfMedNestedKindExtraIndex;
	static const MojChar* const MojPerfLgNestedKindExtraIndex;
	static const MojChar* const MojPerfMedArrayKindExtraIndex;
	static const MojChar* const MojPerfLgArrayKindExtraIndex;

	static const MojUInt64 numKinds = 14;
};


#endif /* MOJDBPERFTEST_H_ */
