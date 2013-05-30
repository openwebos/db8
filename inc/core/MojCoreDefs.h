/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics, Inc.
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

#include <glib.h>

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

// MACROS
#define MojUnused(VAR) ((void) (VAR))

const gsize MojInvalidSize = G_MAXSIZE;
const gsize MojInvalidIndex = G_MAXSIZE;

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
