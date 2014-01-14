/* @@@LICENSE
*
*      Copyright (c) 2009-2014 LG Electronics, Inc.
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


#ifndef MOJERR_H_
#define MOJERR_H_

#ifdef MOJ_HAVE_ASSERT_H
#	include <assert.h>
#endif

#ifdef MOJ_HAVE_ERRNO_H
#	include <errno.h>
#endif

typedef enum {
	MojErrNone = 0,

	// OS ERRORS
	MojErrAccessDenied = EACCES,
	MojErrBusy = EBUSY,
	MojErrExists = EEXIST,
	MojErrInProgress = EINPROGRESS,
	MojErrInterrupted = EINTR,
	MojErrInvalidArg = EINVAL,
	MojErrNoMem = ENOMEM,
	MojErrNotFound = ENOENT,
	MojErrNotImplemented = ENOSYS,
	MojErrWouldBlock = EWOULDBLOCK,

	// INTERNAL ERRORS
	MojErrInternal = -1000,
	MojErrAlreadyOpen,
	MojErrDirCycle,
	MojErrFormat,
	MojErrInsufficientBuf,
	MojErrInvalidBase64Data,
	MojErrInvalidDecimal,
	MojErrInvalidObject,
	MojErrInvalidSchema,
	MojErrLocked,
	MojErrNotImpl,
	MojErrNotInitialized,
	MojErrNotOpen,
	MojErrPathTooLong,
	MojErrRequiredPropNotFound,
	MojErrSchemaValidation,
	MojErrSockAlreadyRegistered,
	MojErrTestFailed,
	MojErrTypeCoercion,
	MojErrUnexpectedEof,
	MojErrUnknown,
	MojErrValueOutOfRange,
	MojErrInternalIndexOnDel,
	MojErrInternalIndexOnFind,
	MojErrTranSizeLimit,

	// OBJECT ERRORS
	MojErrObjectHeaderUnexpectedMarker,
	MojErrObjectReaderUnexpectedMarker,

	// JSON ERRORS
	MojErrJson = -2000,
	MojErrJsonParseArray,
	MojErrJsonParseBool,
	MojErrJsonParseComment,
	MojErrJsonParseDecimal,
	MojErrJsonParseEof,
	MojErrJsonParseEscape,
	MojErrJsonParseInt,
	MojErrJsonParseMaxDepth,
	MojErrJsonParseNull,
	MojErrJsonParsePropName,
	MojErrJsonParseUnexpected,
	MojErrJsonParseValueSep,

	// LOG ERRORS
	MojErrLog = -3000,
	MojErrLogAppenderNotFound,
	MojErrLogLevelNotFound,

	// DB ERRORS
	MojErrDb = -4000,
	MojErrDbAccessDenied,
	MojErrDbBackupFull,
	MojErrDbCorruptDatabase,
	MojErrDbCreateFailed,
	MojErrDbCursorAlreadyOpen,
	MojErrDbDeadlock,
	MojErrDbFatal,
	MojErrDbHeaderVersionMismatch,
	MojErrDbInconsistentIndex,
	MojErrDbInvalidBatch,
	MojErrDbInvalidCaller,
	MojErrDbInvalidCollation,
	MojErrDbInvalidFilterOp,
	MojErrDbInvalidIndex,
	MojErrDbInvalidIndexName,
	MojErrDbInvalidKey,
	MojErrDbInvalidKindToken,
	MojErrDbInvalidOperation,
	MojErrDbInvalidOwner,
	MojErrDbInvalidPermissions,
	MojErrDbInvalidRevisionSet,
	MojErrDbInvalidQuery,
	MojErrDbInvalidQueryCollationMismatch,
	MojErrDbInvalidQueryOp,
	MojErrDbInvalidQueryOpCombo,
	MojErrDbInvalidQueryPage,
	MojErrDbInvalidSequence,
	MojErrDbInvalidToken,
	MojErrDbInvalidTokenization,
	MojErrDbKindNotRegistered,
	MojErrDbKindNotSpecified,
	MojErrDbMalformedId,
	MojErrDbMaxCountExceeded,
	MojErrDbMaxRetriesExceeded,
	MojErrDbNoIndexForQuery,
	MojErrDbObjectNotFound,
	MojErrDbPermissionDenied,
	MojErrDbQuotaExceeded,
	MojErrDbRevisionMismatch,
	MojErrDbRevNotSpecified,
	MojErrDbStorageEngineNotFound,
	MojErrDbUnicode,
	MojErrDbVerificationFailed,
	MojErrDbVersionMismatch,
	MojErrDbWatcherNotRegistered,
	MojErrDbWatchUnsupported,
	MojErrDbWarnings,
	MojErrDbKindHasSubKinds,
	MojErrDbInvalidShardId,
	MojErrDbIO,
	

	// LS ERRORS
	MojErrLuna = -5000,
	MojErrCategoryNotFound,
	MojErrMethodNotFound,
	MojErrResponseHandlerNotFound,
	MojErrInvalidMsg,
	MojErrInvalidToken,
} MojErr;

MojErr MojErrToString(MojErr err, MojString& strOut);
MojErr MojErrThrown(MojErr err, const MojChar* format, ...) MOJ_FORMAT_ATTR((printf, 2, 3));
MojErr MojErrLogDebug(const MojChar* function, const MojChar* file, int line, const MojChar* action, MojErr err, const MojChar* format, ...);
void MojAssertLogDebug(const MojChar* function, const MojChar* file, int line, const MojChar* cond);

#define MojErrThrowNoLog(ENO)			return MojErrThrown(ENO, NULL)
#define MojErrThrowMsgNoLog(ENO, ...) 	return MojErrThrown(ENO, __VA_ARGS__)
#define MojErrThrowErrnoNoLog(FNAME)	MojErrThrowNoLog(MojErrno())
#define MojErrAccumulateNoLog(EACC, E)	if (((E) != MojErrNone)) EACC = (MojErr) (E)
#define MojErrCatchNoLog(E, ENO)		if ((E) == (ENO) && !(E = MojErrNone))
#define MojErrCatchAllNoLog(E)			if ((E) && !(E = MojErrNone))
#define MojErrCheckNoLog(E)				if (E) return (MojErr) (E)
#define MojErrGotoNoLog(E, LABEL)		do if (E) {err = (MojErr) (E); goto LABEL;} while(0)
#define MojAllocCheckNoLog(P)			if ((P) == NULL) MojErrThrowNoLog(MojErrNoMem)

#if defined(MOJ_DEBUG) || defined(MOJ_DEBUG_LOGGING)
#	define MojErrLog(ACT,ENO)			MojErrLogDebug(_T(__FUNCTION__), _T(__FILE__), __LINE__, ACT, ENO, NULL)
#	define MojErrLogMsg(ACT,ENO,...)	MojErrLogDebug(_T(__FUNCTION__), _T(__FILE__), __LINE__, ACT, ENO, __VA_ARGS__)
#	define MojErrThrow(ENO)				do {MojErrLog(_T("THROWN"), ENO); MojErrThrowNoLog(ENO);} while(0)
#	define MojErrThrowMsg(ENO,...)		do {MojErrLogMsg(_T("THROWN"), ENO, __VA_ARGS__); MojErrThrowMsgNoLog(ENO, __VA_ARGS__);} while(0)
#	define MojErrThrowErrno(FNAME)		MojErrThrow(MojErrno())
#	define MojErrAccumulate(EACC, E)	if (((E) != MojErrNone)) MojErrLogMsg(_T("  ACCUMULATED"), E, _T("")), EACC = (MojErr) (E)
#	define MojErrCatch(E, ENO)			if ((E) == (ENO) && (MojErrLogMsg(_T("  CAUGHT"), (MojErr) E, _T("")), !(E = MojErrNone)))
#	define MojErrCatchAll(E)			if ((E) && (MojErrLogMsg(_T("  CAUGHT"), E, _T("")), !(E = MojErrNone)))
#	define MojErrCheck(E)				do if (E) {MojErrLogMsg(_T("  CHECKED"), (MojErr) E, _T("")); return (MojErr) (E);} while(0)
#	define MojErrGoto(E, LABEL)			do if (E) {MojErrLogMsg(_T("  CHECKED"), E, _T("")); MojErrGotoNoLog(E, LABEL);} while(0)
#	define MojAllocCheck(P)				if ((P) == NULL) MojErrThrow(MojErrNoMem)
#	ifdef MOJ_USE_ASSERT
#		define MojAssertNoLog(COND)		assert(COND)
#	endif
#	define MojAssert(COND)				if (!(COND)) MojAssertLogDebug(_T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__, _T(#COND))
#else
#	define MojErrThrow(ENO)				MojErrThrowNoLog(ENO)
#	define MojErrThrowMsg(ENO,...)		MojErrThrowMsgNoLog(ENO, __VA_ARGS__)
#	define MojErrThrowErrno(FNAME)		MojErrThrowErrnoNoLog(FNAME)
#	define MojErrAccumulate(EACC, E)	MojErrAccumulateNoLog(EACC, E)
#	define MojErrCatch(E, ENO)			MojErrCatchNoLog(E, ENO)
#	define MojErrCatchAll(E)			MojErrCatchAllNoLog(E)
#	define MojErrCheck(E)				MojErrCheckNoLog(E)
#	define MojErrGoto(E, LABEL)			MojErrGotoNoLog(E, LABEL)
#	define MojAllocCheck(P)				MojAllocCheckNoLog(P)
#	define MojAssertNoLog(COND)
#	define MojAssert(COND)
#endif

#define MojAssertNotReachedNoLog()		MojAssertNoLog(false)
#define MojAssertNotReached()			MojAssert(false)

#include "core/MojLog.h"

#endif /* MOJERR_H_ */
