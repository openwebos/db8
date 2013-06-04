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


#include "db/MojDbServiceHandler.h"
#include "db/MojDbServiceHandlerInternal.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbSearchCursor.h"
#include "db/MojDbReq.h"
#include "db/MojDbIndex.h"
#include "core/MojJson.h"

const MojDbServiceHandler::SchemaMethod MojDbServiceHandler::s_pubMethods[] = {
	{MojDbServiceDefs::BatchMethod, (Callback) &MojDbServiceHandler::handleBatch, MojDbServiceHandler::BatchSchema},
	{MojDbServiceDefs::DelMethod, (Callback) &MojDbServiceHandler::handleDel, MojDbServiceHandler::DelSchema},
	{MojDbServiceDefs::DelKindMethod, (Callback) &MojDbServiceHandler::handleDelKind, MojDbServiceHandler::DelKindSchema},
	{MojDbServiceDefs::FindMethod, (Callback) &MojDbServiceHandler::handleFind, MojDbServiceHandler::FindSchema},
	{MojDbServiceDefs::GetMethod, (Callback) &MojDbServiceHandler::handleGet, MojDbServiceHandler::GetSchema},
	{MojDbServiceDefs::MergeMethod, (Callback) &MojDbServiceHandler::handleMerge, MojDbServiceHandler::MergeSchema},
	{MojDbServiceDefs::PurgeStatusMethod, (Callback) &MojDbServiceHandler::handlePurgeStatus, MojDbServiceHandler::PurgeStatusSchema},
	{MojDbServiceDefs::PutMethod, (Callback) &MojDbServiceHandler::handlePut, MojDbServiceHandler::PutSchema},
	{MojDbServiceDefs::PutKindMethod, (Callback) &MojDbServiceHandler::handlePutKind, MojDbServiceHandler::PutKindSchema},
	{MojDbServiceDefs::PutPermissionsMethod, (Callback) &MojDbServiceHandler::handlePutPermissions, MojDbServiceHandler::PutPermissionsSchema},
	{MojDbServiceDefs::ReserveIdsMethod, (Callback) &MojDbServiceHandler::handleReserveIds, MojDbServiceHandler::ReserveIdsSchema},
	{MojDbServiceDefs::SearchMethod, (Callback) &MojDbServiceHandler::handleSearch, MojDbServiceHandler::SearchSchema},
	{MojDbServiceDefs::WatchMethod, (Callback) &MojDbServiceHandler::handleWatch, MojDbServiceHandler::WatchSchema},
	{NULL, NULL, NULL} };

const MojDbServiceHandler::SchemaMethod MojDbServiceHandler::s_privMethods[] = {
	{MojDbServiceDefs::CompactMethod, (Callback) &MojDbServiceHandler::handleCompact, MojDbServiceHandler::CompactSchema},
	{MojDbServiceDefs::DumpMethod, (Callback) &MojDbServiceHandler::handleDump, MojDbServiceHandler::DumpSchema},
	{MojDbServiceDefs::LoadMethod, (Callback) &MojDbServiceHandler::handleLoad, MojDbServiceHandler::LoadSchema},
	{MojDbServiceDefs::PurgeMethod, (Callback) &MojDbServiceHandler::handlePurge, MojDbServiceHandler::PurgeSchema},
	{MojDbServiceDefs::PutQuotasMethod, (Callback) &MojDbServiceHandler::handlePutQuotas, MojDbServiceHandler::PutQuotasSchema},
	{MojDbServiceDefs::QuotaStatsMethod, (Callback) &MojDbServiceHandler::handleQuotaStats, MojDbServiceHandler::QuotaStatsSchema},
	{MojDbServiceDefs::StatsMethod, (Callback) &MojDbServiceHandler::handleStats, MojDbServiceHandler::StatsSchema},
	{NULL, NULL, NULL} };

const MojDbServiceHandler::Method MojDbServiceHandler::s_batchMethods[] = {
	{MojDbServiceDefs::DelMethod, (Callback) &MojDbServiceHandler::handleDel},
	{MojDbServiceDefs::FindMethod, (Callback) &MojDbServiceHandler::handleFind},
	{MojDbServiceDefs::GetMethod, (Callback) &MojDbServiceHandler::handleGet},
	{MojDbServiceDefs::MergeMethod, (Callback) &MojDbServiceHandler::handleMerge},
	{MojDbServiceDefs::PutMethod, (Callback) &MojDbServiceHandler::handlePut},
	{MojDbServiceDefs::SearchMethod, (Callback) &MojDbServiceHandler::handleSearch},
	{NULL, NULL}
};

MojDbServiceHandler::MojDbServiceHandler(MojDb& db, MojReactor& reactor)
: MojDbServiceHandlerBase(db, reactor)
{
}

MojErr MojDbServiceHandler::open()
{
	MojLogTrace(s_log);

	MojErr err = addMethods(s_pubMethods, true);
	MojErrCheck(err);
	err = addMethods(s_privMethods, false);
	MojErrCheck(err);

	for (const Method* method = s_batchMethods; method->m_name != NULL; method++) {
		MojString methodStr;
		err = methodStr.assign(method->m_name);
		MojErrCheck(err);
		err = m_batchCallbacks.put(methodStr, (DbCallback) method->m_callback);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandler::close()
{
	MojLogTrace(s_log);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleBatch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject operations;
	MojErr err = payload.getRequired(MojDbServiceDefs::OperationsKey, operations);
	MojErrCheck(err);
	MojObject::ArrayIterator begin;
	err = operations.arrayBegin(begin);
	MojErrCheck(err);

	//start the response array
	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::ResponsesKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);

	for (MojObject* i = begin; i != operations.arrayEnd(); ++i) {
		MojString method;
		err = i->getRequired(MojDbServiceDefs::MethodKey, method);
		MojErrCheck(err);
		MojObject params;
		err = i->getRequired(MojDbServiceDefs::ParamsKey, params);
		MojErrCheck(err);
		// not allowed to specify watch key in a batch
		bool watchValue = false;
		if (payload.get(MojDbServiceDefs::WatchKey, watchValue) && watchValue) {
			MojErrThrow(MojErrDbInvalidBatch);
		}
		DbCallback cb;
		if (!m_batchCallbacks.get(method, cb)) {
			MojErrThrow(MojErrDbInvalidBatch);
		}
		err = (this->*cb)(msg, params, req);
		MojErrCheck(err);
	}

	err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleCompact(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojErr err = m_db.compact();
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleDel(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojUInt32 count = 0;
	MojUInt32 flags = MojDb::FlagNone;

	bool purge = false;
	if (payload.get(MojDbServiceDefs::PurgeKey, purge) && purge) {
		flags |= MojDb::FlagPurge;
	}

	MojObject obj;
	if (payload.get(MojDbServiceDefs::IdsKey, obj)) {
		if (payload.contains(MojDbServiceDefs::QueryKey))
			MojErrThrowMsg(MojErrInvalidArg, _T("db: cannot have both an objects argument and a query argument"));
		MojObject deletedObjects;
		err = m_db.del(obj.arrayBegin(), obj.arrayEnd(), count, deletedObjects, flags, req);
		MojErrCheck(err);
		err = formatPut(msg, deletedObjects.arrayBegin(), deletedObjects.arrayEnd());
		MojErrCheck(err);
	}
	else if (payload.get(MojDbServiceDefs::QueryKey, obj)) {
		MojDbQuery query;
		err = query.fromObject(obj);
		MojErrCheck(err);
		MojUInt32 queryCount = 0;
		err = m_db.del(query, queryCount, flags, req);
		MojErrCheck(err);
		count += queryCount;
		err = formatCount(msg, count);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandler::handleDelKind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject id;
	MojErr err = payload.getRequired(MojDbServiceDefs::IdKey, id);
	MojErrCheck(err);
	
	bool found = false;
	err = m_db.delKind(id, found, MojDb::FlagNone, req);
	MojErrCheck(err);

    if (!found)
    	MojErrThrow(MojErrDbKindNotRegistered);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleDump(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojString path;
	MojErr err = payload.getRequired(MojDbServiceDefs::PathKey, path);
	MojErrCheck(err);

	bool incDel = true;
	payload.get(MojDbServiceDefs::IncludeDeletedKey, incDel);

	MojUInt32 count = 0;
	err = m_db.dump(path, count, incDel, req);
	MojErrCheck(err);

	err = formatCount(msg, count);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleFind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);
	
	bool doCount = false;
	payload.get(MojDbServiceDefs::CountKey, doCount);

	MojDbCursor cursor;
	MojErr err = findImpl(msg, payload, req, cursor, doCount);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleGet(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject ids;
	MojErr err = payload.getRequired(MojDbServiceDefs::IdsKey, ids);
	MojErrCheck(err);

	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::ResultsKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);
	err = m_db.get(ids.arrayBegin(), ids.arrayEnd(), writer, req);
	MojErrCheck(err);
	err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleLoad(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojString path;
	MojErr err = payload.getRequired(MojDbServiceDefs::PathKey, path);
	MojErrCheck(err);

	MojUInt32 count = 0;
	err = m_db.load(path, count, MojDb::FlagNone, req);
	MojErrCheck(err);

	err = formatCount(msg, count);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleMerge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojUInt32 count = 0;
	MojObject obj;
	

	if (payload.get(MojDbServiceDefs::ObjectsKey, obj)) {
		if (payload.contains(MojDbServiceDefs::QueryKey))
			MojErrThrowMsg(MojErrInvalidArg, _T("db: cannot have both an objects param and a query param"));
		MojObject::ArrayIterator begin;
		bool ignoreM = false;
		payload.get(MojDbServiceDefs::IgnoreMissingKey, ignoreM);
		MojUInt32 mflags = MojDb::FlagNone;
		if (ignoreM)
			mflags = mflags | MojDb::FlagIgnoreMissing;
		err = obj.arrayBegin(begin);
		MojErrCheck(err);
		MojObject::ConstArrayIterator end = obj.arrayEnd();
		err = m_db.merge(begin, end, mflags, req);
		MojErrCheck(err);
		err = formatPut(msg, begin, end); // TO DO: we need to drop non-existing objects
		MojErrCheck(err);
	}
	else if (payload.get(MojDbServiceDefs::QueryKey, obj)) {
		MojObject props;
		err = payload.getRequired(MojDbServiceDefs::PropsKey, props);
		MojErrCheck(err);
		bool ignoreM = false;
		if (payload.get(MojDbServiceDefs::IgnoreMissingKey, ignoreM))
			MojErrThrowMsg(MojErrInvalidArg, _T("db: ignoreMissing - invalid option for merge query"));

		MojDbQuery query;
		err = query.fromObject(obj);
		MojErrCheck(err);
		MojUInt32 queryCount = 0;
		err = m_db.merge(query, props, queryCount, MojDb::FlagNone, req);
		MojErrCheck(err);
		count += queryCount;
		err = formatCount(msg, count);
		MojErrCheck(err);
	}
	else {
		MojErrThrowMsg(MojErrInvalidArg, _T("db: either objects or query param required for merge"));
	}

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePurge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojInt64 window = -1;
	payload.get(MojDbServiceDefs::WindowKey, window);

	MojUInt32 count = 0;
	MojErr err = m_db.purge(count, window, req);
	MojLogInfo(s_log, _T("PurgeComplete: Count: %d \n"), count);
	MojErrCheck(err);
	err = formatCount(msg, count);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePurgeStatus(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject revNum;
	MojErr err = m_db.purgeStatus(revNum);
	MojErrCheck(err);

	MojObject response;
	err = response.put(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = response.put(MojDbServiceDefs::RevKey, revNum);
	MojErrCheck(err);
	err = msg->reply(response);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePut(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

    // check space level
    if( MojDbServiceHandlerInternal::spaceAlertLevel() == MojDbServiceHandlerInternal::AlertLevelHigh)
       return MojErrDbQuotaExceeded;
    
	MojObject obj;
	MojErr err = payload.getRequired(MojDbServiceDefs::ObjectsKey, obj);
	MojErrCheck(err);
	MojObject::ArrayIterator begin;
	err = obj.arrayBegin(begin);
	MojErrCheck(err);
	MojObject::ConstArrayIterator end = obj.arrayEnd();
	err = m_db.put(begin, end, MojDb::FlagNone, req);
	MojErrCheck(err);
	err = formatPut(msg, begin, end);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePutKind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);
    
    // check space level
    if( MojDbServiceHandlerInternal::spaceAlertLevel() == MojDbServiceHandlerInternal::AlertLevelHigh)
       return MojErrDbQuotaExceeded;

    // check alert level 
	MojErr err = m_db.putKind(payload, MojDb::FlagNone, req);
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePutPermissions(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

    // check space level
    if( MojDbServiceHandlerInternal::spaceAlertLevel() == MojDbServiceHandlerInternal::AlertLevelHigh)
       return MojErrDbQuotaExceeded;

	MojObject permissionsArr;
	MojErr err = payload.getRequired(MojDbServiceDefs::PermissionsKey, permissionsArr);
	MojErrCheck(err);
	MojObject::ArrayIterator begin;
	err = permissionsArr.arrayBegin(begin);
	MojErrCheck(err);
	MojObject::ConstArrayIterator end = permissionsArr.arrayEnd();
	err = m_db.putPermissions(begin, end, req);
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handlePutQuotas(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

    // check space level
    if( MojDbServiceHandlerInternal::spaceAlertLevel() == MojDbServiceHandlerInternal::AlertLevelHigh)
       return MojErrDbQuotaExceeded;

	MojObject quotas;
	MojErr err = payload.getRequired(MojDbServiceDefs::QuotasKey, quotas);
	MojErrCheck(err);
	MojObject::ArrayIterator begin;
	err = quotas.arrayBegin(begin);
	MojErrCheck(err);
	MojObject::ConstArrayIterator end = quotas.arrayEnd();
	err = m_db.putQuotas(begin, end, req);
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleQuotaStats(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject results(MojObject::TypeObject);
	MojErr err = m_db.quotaStats(results, req);
	MojErrCheck(err);

	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.objectProp(MojDbServiceDefs::ResultsKey, results);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleReserveIds(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

    // check space level
    if( MojDbServiceHandlerInternal::spaceAlertLevel() == MojDbServiceHandlerInternal::AlertLevelHigh)
       return MojErrDbQuotaExceeded;

	MojInt64 count;
	MojErr err = payload.getRequired(MojDbServiceDefs::CountKey, count);
	MojErrCheck(err);

	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::IdsKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);

	if (count > (MojInt64) MaxReserveIdCount) {
		MojErrThrowMsg(MojErrDbMaxCountExceeded, _T("cannot reserve more than %d ids"), MaxReserveIdCount);
	}
	for (MojInt64 i = 0; i < count; ++i) {
		MojObject id;
		err = m_db.reserveId(id);
		MojErrCheck(err);
		err = id.visit(writer);
		MojErrCheck(err);
	}

	err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleSearch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

    MojString localeStr;
    MojErr err = m_db.getLocale(localeStr, req);
	MojErrCheck(err);
    
	MojDbSearchCursor cursor(localeStr);
    err = findImpl(msg, payload, req, cursor, true);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleStats(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);


	MojString forKind;

	bool verify = false;
	payload.get(MojDbServiceDefs::VerifyKey, verify);

	MojString *pKind = NULL;
	bool found = false;
	MojErr err = payload.get(MojDbServiceDefs::KindKey, forKind, found);
	MojErrCheck(err);
	if (found)
		pKind = &forKind;
	

	MojObject results;
	err = m_db.stats(results, req, verify, pKind);
	MojErrCheck(err);

	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.objectProp(MojDbServiceDefs::ResultsKey, results);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::handleWatch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject queryObj;
	MojErr err = payload.getRequired(MojDbServiceDefs::QueryKey, queryObj);
	MojErrCheck(err);
	
	MojRefCountedPtr<Watcher> watcher(new Watcher(msg));
	MojAllocCheck(watcher.get());

	MojDbQuery query;
	err = query.fromObject(queryObj);
	MojErrCheck(err);
	bool fired = false;
	MojDbCursor cursor;
	err = m_db.watch(query, cursor, watcher->m_watchSlot, fired, req);
	MojErrCheck(err);

	MojLogInfo(s_log, _T("handleWatch: %s, err: (%d); sender= %s;\n fired=%d; \n"),
				msg->method(), (int)err, msg->senderName(), (int)fired);

	if (!fired) {
		err = msg->replySuccess();
		MojErrCheck(err);
	}

	err = cursor.close();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::findImpl(MojServiceMessage* msg, MojObject& payload, MojDbReq& req, MojDbCursor& cursor, bool doCount)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject queryObj;
	MojErr err = payload.getRequired(MojDbServiceDefs::QueryKey, queryObj);
	MojErrCheck(err);
	bool doWatch = false;
	payload.get(MojDbServiceDefs::WatchKey, doWatch);

	MojDbQuery query;
	err = query.fromObject(queryObj);
	MojErrCheck(err);
	MojUInt32 limit = query.limit();
	if (limit == MojDbQuery::LimitDefault){
		query.limit(MaxQueryLimit);
	} else if (limit > MaxQueryLimit) {
		MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: limit greater than %d not allowed"), MaxQueryLimit);
	}

	if (doWatch) {
		MojRefCountedPtr<Watcher> watcher(new Watcher(msg));
		MojAllocCheck(watcher.get());
		err = m_db.find(query, cursor, watcher->m_watchSlot, req);
		MojErrCheck(err);
	} else {
		err = m_db.find(query, cursor, req);
		MojErrCheck(err);
	}

	// append results
	MojObjectVisitor& writer = msg->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::ResultsKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);
	err = cursor.visit(writer);
	MojErrCheck(err);
	err = writer.endArray();
	MojErrCheck(err);

	// append next page
	MojDbQuery::Page page;
	err = cursor.nextPage(page);
	MojErrCheck(err);
	if (!page.empty()) {
		MojObject pageObj;
		err = page.toObject(pageObj);
		MojErrCheck(err);
		err = writer.objectProp(MojDbServiceDefs::NextKey, pageObj);
		MojErrCheck(err);
	}

	// append count
	if (doCount) {
		MojUInt32 count = 0;
		err = cursor.count(count);
		MojErrCheck(err);
		err = writer.intProp(MojDbServiceDefs::CountKey, (MojInt64) count);
		MojErrCheck(err);
	}

	err = writer.endObject();
	MojErrCheck(err);

	if (doWatch) {
		// if this is a watched query, it cannot be part of a batch so it's safe to reply here
		err = msg->reply();
		MojErrCheck(err);
	}

	// notifications can fire any time after the cursor is closed,
	// so don't close it until after sending the reply.
	err = cursor.close();
	MojErrCheck(err);

	return MojErrNone;
}

MojDbServiceHandler::Watcher::Watcher(MojServiceMessage* msg)
: m_msg(msg),
  m_watchSlot(this, &Watcher::handleWatch),
  m_cancelSlot(this, &Watcher::handleCancel)
{
	MojAssert(msg);
	msg->notifyCancel(m_cancelSlot);
}

MojErr MojDbServiceHandler::Watcher::handleWatch()
{
	MojAssert(m_msg.get());
	MojLogTrace(s_log);

	// release all references before doing anything that can fail
	m_cancelSlot.cancel();
	MojRefCountedPtr<MojServiceMessage> msg = m_msg;

	MojLogInfo(s_log, _T("Watcher_handleWatch: %s, - sender= %s; appId= %s; subscribed= %d; replies= %zu;\n response= %s\n"), msg->method(), msg->senderName(), msg->appId(), (int)msg->subscribed(), msg->numReplies(), ((MojJsonWriter&)(msg->writer())).json().data());

	m_msg.reset();

	MojObjectVisitor& writer = msg->writer();
	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.boolProp(MojDbServiceDefs::FiredKey, true);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	err = msg->reply();

	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandler::Watcher::handleCancel(MojServiceMessage* msg)
{
	MojAssert(msg == m_msg.get());
	MojLogTrace(s_log);

	m_watchSlot.cancel();
	m_msg.reset();

	return MojErrNone;
}
