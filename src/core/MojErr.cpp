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


#include "core/MojCoreDefs.h"
#include "core/MojLogEngine.h"
#include "core/MojString.h"

struct MojErrStr
{
	const MojErr m_err;
	const MojChar* const m_str;
};

struct MojErrMessageValue
{
	MojErr m_err;
	MojString m_msg;
};

static MojErr MojErrGetLocal(MojErrMessageValue*& valOut);

static const MojSize MojStrErrorBufSize = 256;
static const MojErrStr s_errStrings[] = {
	// INTERNAL ERRORS
	{MojErrInternal, _T("generic internal fault")},
	{MojErrAlreadyOpen, _T("already open")},
	{MojErrDirCycle, _T("directory cycle")},
	{MojErrFormat, _T("invalid format code")},
	{MojErrInsufficientBuf, _T("insufficient buffer space")},
	{MojErrInvalidBase64Data, _T("invalid base64 data")},
	{MojErrInvalidDecimal, _T("invalid decimal")},
	{MojErrInvalidObject, _T("invalid object")},
	{MojErrInvalidSchema, _T("invalid schema")},
	{MojErrLocked, _T("locked")},
	{MojErrNotInitialized, _T("not initialized")},
	{MojErrNotImpl, _T("not implemented")},
	{MojErrNotOpen, _T("not open")},
	{MojErrPathTooLong, _T("path too long")},
	{MojErrRequiredPropNotFound, _T("required property not found")},
	{MojErrSchemaValidation, _T("schema validation failed")},
	{MojErrSockAlreadyRegistered, _T("socked already registered")},
	{MojErrTestFailed, _T("test failed")},
	{MojErrTypeCoercion, _T("type coercion failed")},
	{MojErrUnexpectedEof, _T("unexpected end of file")},
	{MojErrUnknown, _T("unknown error")},
	{MojErrValueOutOfRange, _T("value out of range")},
	{MojErrInternalIndexOnDel, _T("index not found on del")},
	{MojErrInternalIndexOnFind, _T("index val not found on find")},
	{MojErrTranSizeLimit, _T("Transaction size limit reached")},
	// OBJECT ERRORS
	{MojErrObjectHeaderUnexpectedMarker, _T("object header: unexpected marker")},
	{MojErrObjectReaderUnexpectedMarker, _T("object reader: unexpected marker")},
	// JSON ERRORS
	{MojErrJson, _T("json: generic fault")},
	{MojErrJsonParseArray, _T("json: error parsing array")},
	{MojErrJsonParseBool, _T("json: error parsing bool")},
	{MojErrJsonParseComment, _T("json: error parsing comment")},
	{MojErrJsonParseDecimal, _T("json: error parsing decimal")},
	{MojErrJsonParseEof, _T("json: unexpected end of file")},
	{MojErrJsonParseEscape, _T("json: error parsing escape sequence")},
	{MojErrJsonParseInt, _T("json: error parsing int")},
	{MojErrJsonParseMaxDepth, _T("json: max depth exceeded")},
	{MojErrJsonParseNull, _T("json: error parsing null")},
	{MojErrJsonParsePropName, _T("json: error parsing property name")},
	{MojErrJsonParseUnexpected, _T("json: unexpected character")},
	{MojErrJsonParseValueSep, _T("json: error parsing value separator")},
	// LOG ERRORS
	{MojErrLogAppenderNotFound, _T("log: appender not found")},
	{MojErrLogLevelNotFound, _T("log: level not found")},
	// DB ERRORS
	{MojErrDb , _T("db: generic fault")},
	{MojErrDbAccessDenied, _T("db: access denied")},
	{MojErrDbBackupFull, _T("db: backup file is full")},
	{MojErrDbCorruptDatabase, _T("db: corrupt database")},
	{MojErrDbCreateFailed, _T("db: create failed")},
	{MojErrDbCursorAlreadyOpen, _T("db: cursor already open")},
	{MojErrDbDeadlock, _T("db: deadlock")},
	{MojErrDbFatal, _T("db: fatal error")},
	{MojErrDbHeaderVersionMismatch, _T("db: header version mismatch")},
	{MojErrDbInconsistentIndex, _T("db: inconsistent index")},
	{MojErrDbInvalidBatch, _T("db: invalid operation in batch")},
	{MojErrDbInvalidCaller, _T("db: invalid caller")},
	{MojErrDbInvalidCollation, _T("db: invalid collation")},
	{MojErrDbInvalidFilterOp, _T("db: invalid filter op")},
	{MojErrDbInvalidIndex, _T("db: invalid index")},
	{MojErrDbInvalidIndexName, _T("db: invalid index name")},
	{MojErrDbInvalidKey, _T("db: invalid key")},
	{MojErrDbInvalidKindToken, _T("db: invalid kind token")},
	{MojErrDbInvalidOperation, _T("db: invalid operation in permissions")},
	{MojErrDbInvalidOwner, _T("db: invalid owner for kind")},
	{MojErrDbInvalidPermissions, _T("db: invalid permissions")},
	{MojErrDbInvalidRevisionSet, _T("db: invalid revision set")},
	{MojErrDbInvalidQuery, _T("db: invalid query")},
	{MojErrDbInvalidQueryCollationMismatch, _T("db: collations on property do not match")},
	{MojErrDbInvalidQueryOp, _T("db: invalid query operation")},
	{MojErrDbInvalidQueryOpCombo, _T("db: invalid combination of query operations")},
	{MojErrDbInvalidQueryPage, _T("db: invalid query page")},
	{MojErrDbInvalidSequence, _T("db: invalid sequence")},
	{MojErrDbInvalidToken, _T("db: invalid token for property name")},
	{MojErrDbInvalidTokenization, _T("db: invalid tokenization")},
	{MojErrDbKindNotRegistered, _T("db: kind not registered")},
	{MojErrDbKindNotSpecified, _T("db: kind not specified")},
	{MojErrDbMalformedId, _T("db: malformed id")},
	{MojErrDbMaxCountExceeded, _T("db: max count exceeded")},
	{MojErrDbMaxRetriesExceeded, _T("db: max retries exceeded")},
	{MojErrDbNoIndexForQuery, _T("db: no index for query")},
	{MojErrDbObjectNotFound, _T("db: object not found")},
	{MojErrDbPermissionDenied, _T("db: permission denied")},
	{MojErrDbQuotaExceeded, _T("db: quota exceeded")},
	{MojErrDbRevisionMismatch, _T("db: revision mismatch")},
	{MojErrDbRevNotSpecified, _T("db: _rev must be specified for existing objects")},
	{MojErrDbStorageEngineNotFound, _T("db: storage engine not found")},
	{MojErrDbUnicode, _T("db: unicode error")},
	{MojErrDbVerificationFailed, _T("db: verification failed")},
	{MojErrDbVersionMismatch, _T("db: database version mismatch")},
	{MojErrDbWatcherNotRegistered, _T("db: watcher not registered")},
	{MojErrDbWatchUnsupported, _T("db: watch not supported for query")},
	{MojErrDbWarnings, _T("db: warnings - run stats")},
	{MojErrDbKindHasSubKinds, _T("db: can not delete kind that has sub-kinds")},
    {MojErrDbInvalidShardId, _T("db: invalid shard id")},
	// LS ERRORS
	{MojErrLuna, _T("luna: generic fault")},
	{MojErrCategoryNotFound, _T("luna: category not found")},
	{MojErrMethodNotFound, _T("luna: method not found")},
	{MojErrResponseHandlerNotFound, _T("luna: response handler not found")},
	{MojErrInvalidMsg, _T("luna: invalid message")},
	{MojErrInvalidToken, _T("luna: invalid token")},

	// THE END
	{MojErrNone, _T("")}
};

MojErr MojErrToString(MojErr errToGet, MojString& strOut)
{
	MojErrMessageValue* val = NULL;
	MojErr err = MojErrGetLocal(val);
	MojErrCheck(err);
	MojAssertNoLog(val);

	if (!val->m_msg.empty() && val->m_err == errToGet) {
		strOut = val->m_msg;
		return MojErrNone;
	}
	if (errToGet >= 0) {
		MojChar buf[MojStrErrorBufSize];
		buf[0] = _T('\0');
		err = strOut.assign(MojStrError(errToGet, buf, MojStrErrorBufSize));
		MojErrCheckNoLog(err);
		return MojErrNone;
	}
	for (const MojErrStr* i = s_errStrings; i->m_err != MojErrNone; ++i) {
		if (i->m_err == errToGet) {
			err = strOut.assign(i->m_str);
			MojErrCheckNoLog(err);
			return MojErrNone;
		}
	}
	err = strOut.assign(_T("unknown"));
	MojErrCheckNoLog(err);

	return MojErrNone;
}

MojErr MojErrThrown(MojErr errToSet, const MojChar* format, ...)
{
	// put a breakpoint here to break on error
	// get thread-local error message
	MojErrMessageValue* val = NULL;
	MojErr err = MojErrGetLocal(val);
	MojErrCheckNoLog(err);

	// update it
	if (format) {
		va_list args;
		va_start(args, format);
		err = val->m_msg.vformat(format, args);
		va_end(args);
		MojErrCheckNoLog(err);
	} else {
		val->m_msg.clear();
	}
	val->m_err = errToSet;

	return errToSet;
}

MojErr MojErrLogDebug(const MojChar* function, const MojChar* file, int line, const MojChar* action, MojErr errToLog, const MojChar* format, ...)
{
	MojString msg;
	MojErr err = msg.format(_T("%s: %s:%d - %s() - "), action, MojFileNameFromPath(file), line, function);
	MojErrCheckNoLog(err);
	MojSize len = msg.length();
	if (format) {
		va_list args;
		va_start(args, format);
		err = msg.appendVFormat(format, args);
		va_end(args);
		MojErrCheckNoLog(err);
	} else {
		MojString errString;
		err = MojErrToString(errToLog, errString);
		MojErrCheckNoLog(err);
		err = msg.append(errString.data());
		MojErrCheckNoLog(err);
	}
	if (len != msg.length()) {
		err = msg.append(_T(' '));
		MojErrCheckNoLog(err);
	}
	err = msg.appendFormat(_T("(%d)"), (int) errToLog);
	MojErrCheckNoLog(err);
    //LOG_ERROR(MSGID_ERROR_CALL, 0, "%s", msg.data());

	return MojErrNone;
}

void MojAssertLogDebug(const MojChar* function, const MojChar* file, int line, const MojChar* cond)
{
    //LOG_DEBUG(MSGID_MESSAGE_CALL, "ASSERT FAILED: %s (%s:%d) '%s'", function, MojFileNameFromPath(file), line, cond);
	MojAbort();
}

MojErr MojErrGetLocal(MojErrMessageValue*& valOut)
{
	static MojThreadLocalValue<MojErrMessageValue> s_val;
	return s_val.get(valOut);
}
