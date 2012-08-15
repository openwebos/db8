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


#ifndef MOJDEFS_H_
#define MOJDEFS_H_

#include "core/internal/MojConfig.h"
#include "core/internal/MojNew.h"

#ifdef MOJ_HAVE_STDDEF_H
#	include <stddef.h>
#endif

#ifdef MOJ_HAVE_STDINT_H
#	include <stdint.h>
#endif

#ifdef MOJ_HAVE_LIMITS_H
#	include <limits.h>
#endif

// CONDITIONAL TYPE DECLARATIONS
#ifdef MOJ_USE_WCHAR
#	ifndef _T
#		define _T(X) L##X
#	endif
	typedef wchar_t MojChar;
#else
#	ifndef _T
#		define _T(X) X
#	endif
	typedef char MojChar;
#endif

#ifdef MOJ_USE_LONG_LONG
	typedef long long MojInt64;
	typedef unsigned long long MojUInt64;
#endif

#ifdef MOJ_USE_SIZE_T
	typedef size_t MojSize;
#endif

#ifdef MOJ_USE_INTPTR_T
	typedef intptr_t MojIntPtr;
#endif

// MACROS
#define MojUnused(VAR) ((void) (VAR))

// STANDARD TYPE DECLARATIONS
typedef unsigned char MojByte;
typedef signed char MojInt8;
typedef unsigned char MojUInt8;
typedef short MojInt16;
typedef unsigned short MojUInt16;
typedef int MojInt32;
typedef unsigned int MojUInt32;
typedef double MojDouble;

// LIMITS
const MojByte MojByteMax = 0xFF;
const MojInt8 MojInt8Max = 0x7F;
const MojInt8 MojInt8Min = (MojInt8) 0x80;
const MojUInt8 MojUInt8Max = 0xFF;
const MojInt16 MojInt16Max = 0x7FFF;
const MojInt16 MojInt16Min = (MojInt16) 0x8000;
const MojUInt16 MojUInt16Max = 0xFFFF;
const MojInt32 MojInt32Max = 0x7FFFFFFF;
const MojInt32 MojInt32Min = 0x80000000;
const MojUInt32 MojUInt32Max = 0xFFFFFFFF;
const MojInt64 MojInt64Max = 0x7FFFFFFFFFFFFFFFLL;
const MojInt64 MojInt64Min = 0x8000000000000000LL;
const MojUInt64 MojUInt64Max = 0xFFFFFFFFFFFFFFFFULL;
const MojSize MojSizeMax = (MojSize) MojUInt64Max;
const MojSize MojInvalidSize = MojSizeMax;
const MojSize MojInvalidIndex = MojSizeMax;

#ifdef MOJ_USE_NAME_MAX
	const MojSize MojNameMax = NAME_MAX;
#endif

#ifdef MOJ_USE_PATH_MAX
	const MojSize MojPathMax = PATH_MAX;
#endif

// ATTRIBUTES
#ifdef MOJ_USE_FORMAT_ATTR
#	define MOJ_FORMAT_ATTR(ARG) __attribute__ ((format ARG))
#else
#	define MOJ_FORMAT_ATTR(ARG)
#endif

// TAGS
struct MojTagFalse {};
struct MojTagTrue {};
struct MojTagNil {};

// FORWARD CLASS DECLARATIONS
class MojApp;
class MojAtomicInt;
class MojBuffer;
class MojDataReader;
class MojDataWriter;
class MojDecimal;
class MojEpollReactor;
class MojGmainReactor;
class MojJsonParser;
class MojJsonWriter;
class MojLogAppender;
class MojLogEngine;
class MojLogger;
class MojMessage;
class MojMessageDispatcher;
class MojObject;
class MojObjectBuilder;
class MojObjectEater;
class MojObjectReader;
class MojObjectWriter;
class MojObjectVisitor;
class MojReactor;
class MojRefCounted;
class MojSchema;
class MojService;
class MojServiceApp;
class MojServiceMessage;
class MojServiceRequest;
class MojSignalHandler;
class MojSock;
class MojSockAddr;
class MojString;
class MojTestCase;
class MojTestRunner;
class MojThreadCond;
class MojThreadGuard;
class MojThreadMutex;
class MojThreadReadGuard;
class MojThreadRwLock;
class MojThreaadWriteGuard;
class MojTime;

#include "core/MojErr.h"
#include "core/MojOs.h"

#endif /* MOJDEFS_H_ */
