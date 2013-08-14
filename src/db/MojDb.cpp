/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include <unicode/uloc.h>

#include "db/MojDb.h"
#include "db/MojDbQuery.h"
#include "db/MojDbKind.h"
#include "db/MojDbReq.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbObjectHeader.h"
#include "core/MojJson.h"
#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojTime.h"
#include "core/MojFile.h"

const MojChar* const MojDb::AdminRole = _T("admin");
const MojChar* const MojDb::ObjDbName = _T("objects.db");
const MojChar* const MojDb::IdSeqName = _T("id");
const MojChar* const MojDb::ConfKey = _T("db");
const MojChar* const MojDb::DelKey = _T("_del");
const MojChar* const MojDb::IdKey = _T("_id");
const MojChar* const MojDb::IgnoreIdKey = _T("_ignoreId");
const MojChar* const MojDb::KindKey = _T("_kind");
const MojChar* const MojDb::RevKey = _T("_rev");
const MojChar* const MojDb::RevNumKey = _T("rev");
const MojChar* const MojDb::RoleType = _T("db.role");
const MojChar* const MojDb::SyncKey = _T("_sync");
const MojChar* const MojDb::TimestampKey = _T("timestamp");
const MojChar* const MojDb::LastPurgedRevKey = _T("lastPurgedRev");
const MojChar* const MojDb::LocaleKey = _T("locale");
const MojChar* const MojDb::DbStateObjId = _T("_internal/dbstate");
const MojChar* const MojDb::VersionFileName = _T("_version");
const MojChar* const MojDb::KindIdPrefix = _T("_kinds/");
const MojChar* const MojDb::QuotaIdPrefix = _T("_quotas/");
const MojChar* const MojDb::PermissionIdPrefix = _T("_permissions/");
const MojUInt32 MojDb::AutoBatchSize = 1000;
const MojUInt32 MojDb::AutoCompactSize = 5000;

MojLogger MojDb::s_log(_T("db.mojodb"));


MojDb::MojDb()
: m_purgeWindow(PurgeNumDaysDefault),
  m_loadStepSize(LoadStepSizeDefault),
  m_isOpen(false)
{
	MojLogTrace(s_log);

    UErrorCode error;
    uloc_setDefault("en_US", &error);
}

MojDb::~MojDb()
{
	MojLogTrace(s_log);

	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDb::configure(const MojObject& conf)
{
	MojLogTrace(s_log);
	MojThreadWriteGuard guard(m_schemaLock);

	MojErr err = requireNotOpen();
	MojErrCheck(err);
	bool found = false;
	MojObject dbConf;
	if (conf.get(ConfKey, dbConf)) {
		err = dbConf.get(_T("storageEngine"), m_engineName, found);
		MojErrCheck(err);
		found = dbConf.get(_T("purgeWindow"), m_purgeWindow);
		if (!found) {
			m_purgeWindow = PurgeNumDaysDefault;
		}
		found = dbConf.get(_T("loadStepSize"), m_loadStepSize);
		if (!found) {
			m_loadStepSize = LoadStepSizeDefault;
		}
		m_conf = dbConf;
	}
	return MojErrNone;
}

MojErr MojDb::drop(const MojChar* path)
{
	MojAssert(path);
	MojLogTrace(s_log);

	MojErr err = requireOpen();
	MojErrCheck(err);

	MojDbReq req;
	err = req.begin(this, true);
	MojErrCheck(err);
	err = m_storageEngine->drop(path, req.txn());
	MojErrCheck(err);
	err = req.end();
	MojErrCheck(err);
	err = close();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::open(const MojChar* path, MojDbStorageEngine* engine)
{
	MojAssert(path);
	MojLogTrace(s_log);

	MojErr err = requireNotOpen();
	MojErrCheck(err);

    MojLogNotice(s_log, _T("opening: '%s'..."), path);

	MojAutoCloser<MojDb> closer(this);
	m_isOpen = true;

	// check the database version number and bail if there's a mismatch
	err = checkDbVersion(path);
	MojErrCheck(err);
	// engine
	if (engine == NULL) {
		err = createEngine();
		MojErrCheck(err);
		MojAssert(m_storageEngine.get());
		err = m_storageEngine->configure(m_conf);
		MojErrCheck(err);
		err = m_storageEngine->open(path);
		MojErrCheck(err);
	} else {
		m_storageEngine.reset(engine);
	}

	MojDbReq req;
	err = req.begin(this, true);
	MojErrCheck(err);

	// db
    MojLogInfo(s_log, _T("Open Database: '%s'"), ObjDbName);
	err = m_storageEngine->openDatabase(ObjDbName, req.txn(), m_objDb);
	MojErrCheck(err);
	MojAssert(m_objDb.get());

	// seq
    MojLogInfo(s_log, _T("Open Database: '%s'"), IdSeqName);
	err = m_storageEngine->openSequence(IdSeqName, req.txn(), m_idSeq);
	MojErrCheck(err);
	MojAssert(m_idSeq.get());

	// kinds
    MojLogInfo(s_log, _T("Open Kind Engine"));
	err = m_kindEngine.open(this, req);
    MojLogInfo(s_log, _T("Kind Opened..."));
	MojErrCheck(err);

	// perms
    MojLogInfo(s_log, _T("Open Permissions"));
	err = m_permissionEngine.open(m_conf, this, req);
	MojErrCheck(err);

	// quota
	err = m_quotaEngine.open(m_conf, this, req);
	MojErrCheck(err);
	err = req.end();
	MojErrCheck(err);

	// idgen
	err = m_idGenerator.init();
	MojErrCheck(err);

	closer.release();
    MojLogNotice(s_log, _T("open completed"));

	return MojErrNone;
}

MojErr MojDb::close()
{
	MojLogTrace(s_log);
	MojThreadWriteGuard guard(m_schemaLock);

	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;

	if (m_isOpen) {
        MojLogNotice(s_log, _T("closing..."));

		errClose = m_quotaEngine.close();
		MojErrAccumulate(err, errClose);
		errClose = m_permissionEngine.close();
		MojErrAccumulate(err, errClose);
		errClose = m_kindEngine.close();
		MojErrAccumulate(err, errClose);
		if (m_idSeq.get()) {
			errClose = m_idSeq->close();
			MojErrAccumulate(err, errClose);
			m_idSeq.reset();
		}
		if (m_objDb.get()) {
			errClose = m_objDb->close();
			MojErrAccumulate(err, errClose);
			m_objDb.reset();
		}
		if (m_storageEngine.get()) {
			errClose = m_storageEngine->close();
			MojErrAccumulate(err, errClose);
			m_storageEngine.reset();
		}
		m_isOpen = false;
        MojLogNotice(s_log, _T("close completed"));
	}
	return err;
}

MojErr MojDb::del(const MojObject& id, bool& foundOut, MojUInt32 flags, MojDbReqRef req)
{
	MojLogTrace(s_log);

	foundOut = false;
	MojUInt32 count = 0;
	MojObject delObj;
	MojErr err = del(&id, &id + 1, count, delObj, flags, req);
	MojErrCheck(err);
	foundOut = count;

	return MojErrNone;
}

MojErr MojDb::del(const MojObject* idsBegin, const MojObject* idsEnd, MojUInt32& countOut, MojObject& arrOut, MojUInt32 flags, MojDbReqRef req)
{
	MojAssert(idsBegin || idsBegin == idsEnd);
	MojAssert(idsEnd >= idsBegin);
	MojLogTrace(s_log);

	countOut = 0;
	MojErr err = beginReq(req);
	MojErrCheck(err);

	// do the dels
	MojUInt32 count= 0;
	for (const MojObject* i = idsBegin; i != idsEnd; ++i) {
		MojObject foundObj;
		bool found = false;
		err = delImpl(*i, found, foundObj, req, flags);
		MojErrCheck(err);
		if (found) {
			count++;
			err = arrOut.push(foundObj);
			MojErrCheck(err);
		}
	}
	// commit txn
	err = req->end();
	MojErrCheck(err);
	countOut = count;

	return MojErrNone;
}

MojErr MojDb::del(const MojDbQuery& query, MojUInt32& countOut, MojUInt32 flags, MojDbReqRef req)
{
	MojLogTrace(s_log);
	countOut = 0;

	MojErr err = beginReq(req);
	MojErrCheck(err);

	err = delImpl(query, countOut, req, flags);
	MojErrCheck(err);

	err = req->end();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::delKind(const MojObject& id, bool& foundOut, MojUInt32 flags, MojDbReqRef req)
{
	MojLogTrace(s_log);
	foundOut = false;

	MojErr err = beginReq(req, true);
	MojErrCheck(err);

	// del kind obj
	MojString idStr;
	err = id.stringValue(idStr);
	MojErrCheck(err);

	MojDbKind *pk = NULL;

	// If Kinds has sub-kinds, we give an error
	err = m_kindEngine.getKind(idStr.data(), pk);
	MojErrCheck(err);

	if (pk->nsubkinds() > 0) {
		MojLogWarning(s_log, _T("delKind_error: %s has %d subkinds \n"), idStr.data(), pk->nsubkinds());
		MojErrThrow(MojErrDbKindHasSubKinds);
	}
    else
        MojLogNotice(s_log, _T("delKind: %s \n"), idStr.data());

	err = m_kindEngine.checkOwnerPermission(idStr, req);
	MojErrCheck(err);

	MojString dbId;
	err = formatKindId(idStr, dbId);
	MojErrCheck(err);
	MojObject deleted;
	bool found = false;
	MojDbAdminGuard adminGuard(req);
	err = delImpl(dbId, found, deleted, req, flags);
	MojErrCheck(err);

	if (found) {
		// del objects
		MojDbQuery query;
		err = query.from(idStr);
		MojErrCheck(err);
		err = query.includeDeleted(true);
		MojErrCheck(err);
		MojUInt32 count;
		req->fixmode(true); 
		err = delImpl(query, count, req, flags | FlagPurge);
		MojErrCheck(err);

		// del associated permissions
		query.clear();
		err = query.from(MojDbKindEngine::PermissionId);
		MojErrCheck(err);
		err = query.where(MojDbServiceDefs::ObjectKey, MojDbQuery::OpEq, idStr);
		MojErrCheck(err);
		req->fixmode(true);
		err = delImpl(query, count, req, flags);
		MojErrCheck(err);

		// del kind
		MojErr errAcc = m_kindEngine.delKind(idStr, req);
		MojErrAccumulate(err, errAcc);
	}
	err = commitKind(idStr, req, err);
	MojErrCheck(err);
	foundOut = found;

	return MojErrNone;
}

MojErr MojDb::get(const MojObject& id, MojObject& objOut, bool& foundOut, MojDbReqRef req)
{
	MojLogTrace(s_log);

	objOut.clear();
	foundOut = false;
	MojObjectBuilder builder;

	MojErr err = get(&id, &id + 1, builder, req);
	MojErrCheck(err);

	objOut = builder.object();
	foundOut = !objOut.undefined();

	return MojErrNone;
}

MojErr MojDb::get(const MojObject* idsBegin, const MojObject* idsEnd, MojObjectVisitor& visitor, MojDbReqRef req)
{
	MojAssert(idsBegin || idsBegin == idsEnd);
	MojAssert(idsEnd >= idsBegin);
	MojLogTrace(s_log);

	MojErr err = beginReq(req);
	MojErrCheck(err);

	// do the gets
	for (const MojObject* i = idsBegin; i != idsEnd; ++i) {
		err = getImpl(*i, visitor, OpRead, req);
		MojErrCheck(err);
	}
	// commit txn
	err = req->end();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::merge(const MojDbQuery& query, const MojObject& props, MojUInt32& countOut, MojUInt32 flags, MojDbReqRef req)
{
	MojLogTrace(s_log);

	countOut = 0;
	MojErr err = beginReq(req);
	MojErrCheck(err);

	MojDbCursor cursor;
	err = findImpl(query, cursor, NULL, req, OpUpdate);
	MojErrCheck(err);
	MojAssert(cursor.txn());

	MojUInt32 count = 0;
	MojUInt32 warns = 0;
	bool found = false;
	MojObject prev;
	for (;;) {
		// get prev rev from cursor
		MojDbStorageItem* prevItem = NULL;
		err = cursor.get(prevItem, found);
		if (err == MojErrInternalIndexOnFind) {
			warns++;
			continue;
		}
		MojErrCheck(err);
		if (!found)
			break;
		err = prevItem->toObject(prev, m_kindEngine);
		MojErrCheck(err);
		// merge obj into prev
		MojObject merged;
		err = mergeInto(merged, props, prev);
		MojErrCheck(err);
		// and update the db
		const MojObject& id = prevItem->id();
		err = putObj(id, merged, &prev, prevItem, req, OpUpdate);
		MojErrCheck(err);
		++count;
	}
	if (warns > 0)
		MojLogWarning(s_log, _T("Merge index_warnings: %s; count: %d\n"), query.from().data(), warns);
	err = cursor.close();
	MojErrCheck(err);
	err = req->end();
	MojErrCheck(err);

	countOut = count;

	return MojErrNone;
}

MojErr MojDb::put(MojObject& obj, MojUInt32 flags, MojDbReqRef req)
{
	MojLogTrace(s_log);

	MojErr err = put(&obj, &obj + 1, flags, req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::put(MojObject* begin, const MojObject* end, MojUInt32 flags, MojDbReqRef req)
{
	MojAssert(begin || begin == end);
	MojAssert(end >= begin);
	MojLogTrace(s_log);

    // Max shard ID(0xFFFFFFFF) converted base-64 is "zzzzzk"
    if(m_shardId.length() > 6 || m_shardId.compare(_T("zzzzzk")) > 0) {
        MojLogWarning(s_log, _T("Invalid shard ID length\n"));
        MojErrThrowMsg(MojErrDbMalformedId, _T("db: Invalid shard ID length"));
    }

	MojErr err = beginReq(req);
	MojErrCheck(err);

	for (MojObject* i = begin; i != end; ++i) {
		err = putImpl(*i, flags, req);
		MojErrCheck(err);
	}
	err = req->end();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::putKind(MojObject& obj, MojUInt32 flags, MojDbReqRef req)
{
	MojErr err = beginReq(req, true);
	MojErrCheck(err);

    // get id
    MojString id;
    err = obj.getRequired(MojDbServiceDefs::IdKey, id);
    MojErrCheck(err);

	MojLogInfo(s_log, _T("putKind: %s \n"), id.data());

	// set _kind and _id
	err = obj.putString(KindKey, MojDbKindEngine::KindKindId);
	MojErrCheck(err);
	MojString dbId;
	err = formatKindId(id, dbId);
	MojErrCheck(err);
	err = obj.putString(IdKey, dbId);
	MojErrCheck(err);

	// put the object
	MojDbAdminGuard guard(req);
	err = putImpl(obj, flags | FlagForce, req);
	MojErrCheck(err);
	guard.unset();

	// attempt the putKind
	MojErr errAcc = m_kindEngine.putKind(obj, req);
	MojErrAccumulate(err, errAcc);

	err = commitKind(id, req, err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::putConfig(MojObject* begin, const MojObject* end, MojDbReq& req, MojDbPutHandler& handler)
{
	MojAssert(begin || begin == end);
	MojAssert(end >= begin);
	MojLogTrace(s_log);

	MojErr err = beginReq(req, true);
	MojErrCheck(err);

	for (MojObject* i = begin; i != end; ++i) {
		err = handler.put(*i, req);
		MojErrCheck(err);
	}

	err = req.end();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::find(const MojDbQuery& query, MojDbCursor& cursor, MojDbReqRef req)
{
	MojLogTrace(s_log);

	MojErr err = beginReq(req);
	MojErrCheck(err);

	err = findImpl(query, cursor, NULL, req, OpRead);
	MojErrCheck(err);

	err = req->end(false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::find(const MojDbQuery& query, MojDbCursor& cursor, WatchSignal::SlotRef watchHandler, MojDbReqRef req)
{
	MojLogTrace(s_log);

	MojErr err = beginReq(req);
	MojErrCheck(err);

	MojRefCountedPtr<MojDbWatcher> watcher(new MojDbWatcher(watchHandler));
	MojAllocCheck(watcher.get());
	err = findImpl(query, cursor, watcher.get(), req, OpRead);
	MojErrCheck(err);

	err = req->end(false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::reserveId(MojObject& idOut)
{
	MojLogTrace(s_log);

	MojErr err = requireOpen();
	MojErrCheck(err);

	err = m_idGenerator.id(idOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::watch(const MojDbQuery& query, MojDbCursor& cursor, WatchSignal::SlotRef watchHandler, bool& firedOut, MojDbReqRef req)
{
	MojLogTrace(s_log);

	firedOut = false;

	MojErr err = beginReq(req);
	MojErrCheck(err);

	MojRefCountedPtr<MojDbWatcher> watcher(new MojDbWatcher(watchHandler));
	MojAllocCheck(watcher.get());

	MojDbQuery limitedQuery = query;
	limitedQuery.limit(1);
	err = findImpl(limitedQuery, cursor, watcher.get(), req, OpRead);
	MojErrCheck(err);

	MojDbStorageItem* item = NULL;
	bool found = false;
	cursor.verifymode(false);
	err = cursor.get(item, found);
	MojErrCheck(err);

	if (found) {
       const MojDbKey& key = cursor.storageQuery()->endKey();
       err = watcher->fire(key);
       MojErrCheck(err);
       firedOut = true;
	}

	err = req->end(false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::createEngine()
{
	if (m_engineName.empty()) {
		MojErr err = MojDbStorageEngine::createDefaultEngine(m_storageEngine);
		MojErrCheck(err);
	} else {
		MojErr err = MojDbStorageEngine::createEngine(m_engineName, m_storageEngine);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDb::requireOpen()
{
    if (!m_isOpen) {
		MojErrThrowMsg(MojErrNotOpen, _T("db not open"));
    }
	return MojErrNone;
}

MojErr MojDb::requireNotOpen()
{
	if (m_isOpen)
		MojErrThrowMsg(MojErrAlreadyOpen, _T("db already open"));
	return MojErrNone;
}

MojErr MojDb::mergeInto(MojObject& dest, const MojObject& obj, const MojObject& prev)
{
	MojLogTrace(s_log);

	// TODO: support field deletion
	// TODO: move merge fn out of db
	MojErr err;
	MojObject::ConstIterator objIter = obj.begin();
	MojObject::ConstIterator prevIter = prev.begin();
	while (objIter != obj.end() || prevIter != prev.end()) {
		// compare keys from two iters
		int comp;
		if (objIter == obj.end()) {
			comp = 1;
		} else if (prevIter == prev.end()) {
			comp = -1;
		} else {
			comp = objIter.key().compare(prevIter.key());
		}
		// put the appropriate value into dest
		if (comp > 0) {
			err = dest.put(prevIter.key(), prevIter.value());
			MojErrCheck(err);
			++prevIter;
		} else if (comp < 0) {
			err = dest.put(objIter.key(), objIter.value());
			MojErrCheck(err);
			++objIter;
		} else {
			MojObject::Type newType = objIter.value().type();
			MojObject::Type prevType = prevIter.value().type();
			if (newType == MojObject::TypeObject && prevType == MojObject::TypeObject) {
				MojObject merged(MojObject::TypeObject);
				err = mergeInto(merged, objIter.value(), prevIter.value());
				MojErrCheck(err);
				err = dest.put(objIter.key(), merged);
				MojErrCheck(err);
			} else {
				err = dest.put(objIter.key(), objIter.value());
				MojErrCheck(err);
			}
			++prevIter;
			++objIter;
		}
	}
	return MojErrNone;
}

MojErr MojDb::putObj(const MojObject& id, MojObject& obj, const MojObject* oldObj,
					 MojDbStorageItem* oldItem, MojDbReq& req, MojDbOp op, bool checkSchema)
{
	MojLogTrace(s_log);

	// if nothing changed, don't do the update
	if (oldObj != NULL && obj == *oldObj)
		return MojErrNone;

	// update revision
	MojInt64 rev;
	MojErr err = nextId(rev);
	MojErrCheck(err);
	err = obj.put(RevKey, rev);
	MojErrCheck(err);

	// assign id
	MojObject putId = id;
	if (putId.undefined()) {
        err = m_idGenerator.id(putId, m_shardId);
		MojErrCheck(err);
		err = obj.put(IdKey, putId);
		MojErrCheck(err);
        m_shardId.clear();
	}

	// assign ids to subobjects in arrays - only for regular objects
	MojString kindName;
	bool found = false;
	err = obj.get(MojDb::KindKey, kindName, found);
	MojErrCheck(err);
	if (!found)
		MojErrThrow(MojErrDbKindNotSpecified);
	if (!kindName.startsWith(MojDbKindEngine::KindKindIdPrefix)) {
		err = assignIds(obj);
		MojErrCheck(err);
	}

	// validate, update indexes, etc.
	MojTokenSet tokenSet;
	err = m_kindEngine.update(&obj, oldObj, req, op, tokenSet, checkSchema);
	MojErrCheck(err);

	// serialize object
	MojDbObjectHeader header(putId);
	err = header.extractFrom(obj);
	MojErrCheck(err);
	MojBuffer buf;
	err = header.write(buf, m_kindEngine);
	MojErrCheck(err);
	MojObjectWriter writer(buf, &tokenSet);
	err = obj.visit(writer);
	MojErrCheck(err);

	// store it in the db
	if (oldItem) {
		err = m_objDb->update(putId, buf, oldItem, req.txn());
		MojErrCheck(err);
	} else {
		err = m_objDb->insert(putId, buf, req.txn());
		MojErrCheck(err);
	}

	// put header back in the object for response purposes
	err = header.addTo(obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::delObj(const MojObject& id, const MojObject& obj, MojDbStorageItem* item, MojObject& foundObjOut, MojDbReq& req, MojUInt32 flags)
{
	MojAssert(item);
	MojLogTrace(s_log);

	if (MojFlagGet(flags, FlagPurge)) {
		// update indexes
		MojTokenSet tokenSet;
		// we want purge to force delete
		req.fixmode(true);
		MojErr err = m_kindEngine.update(NULL, &obj, req, OpDelete, tokenSet);
		MojErrCheck(err);
		// gross layering violation
		err = req.txn()->offsetQuota(-(MojInt64) item->size());
		MojErrCheck(err);
		// permanently delete
		bool found = false;
		err = m_objDb->del(id, req.txn(), found);
		MojErrCheck(err);
		if (!found)
			MojErrThrow(MojErrDbCorruptDatabase);
		err = foundObjOut.put(IdKey, id);
		MojErrCheck(err);
	} else {
		// set deleted flag and put if we are not purging
		MojObject newObj = obj;
		MojErr err = newObj.putBool(DelKey, true);
		MojErrCheck(err);
		err = putObj(id, newObj, &obj, item, req, OpDelete);
		MojErrCheck(err);
		foundObjOut = newObj;
	}
	return MojErrNone;
}

MojErr MojDb::delImpl(const MojObject& id, bool& foundOut, MojObject& foundObjOut, MojDbReq& req, MojUInt32 flags)
{
	MojLogTrace(s_log);

	foundObjOut.clear();
	// get object, so we can find the type
	MojRefCountedPtr<MojDbStorageItem> item;
	MojErr err = m_objDb->get(id, req.txn(), true, item);
	MojErrCheck(err);
	if (item.get()) {
		// and delete it
		MojObject obj;
		err = item->toObject(obj, m_kindEngine);
		MojErrCheck(err);
		err = delObj(id, obj, item.get(), foundObjOut, req, flags);
		MojErrCheck(err);
		foundOut = true;
	}
	return MojErrNone;
}

MojErr MojDb::delImpl(const MojDbQuery& quer, MojUInt32& countOut, MojDbReq& req, MojUInt32 flags)
{
	MojLogTrace(s_log);

	countOut = 0;
	MojInt32 warns = 0;
    MojDbQuery newQuery = quer;
    MojUInt32 queryLimit = newQuery.limit();

    if(newQuery.limit() == MojDbQuery::LimitDefault)
        newQuery.limit(AutoBatchSize);

    while(queryLimit > 0)
    {
        MojDbCursor cursor;
        MojErr err = findImpl(newQuery, cursor, NULL, req, OpDelete);

        MojErrCheck(err);
        MojAssert(cursor.txn());

        MojUInt32 count = 0;
        MojUInt32 numberInBatch = 0;

        bool found = false;

        MojObject obj;
        for (;;) {
            MojDbStorageItem* item = NULL;
            err = cursor.get(item, found);
            // We simply skip ghost keys and continue; A warning is already written to the system log earlier
            if (err == MojErrInternalIndexOnFind) {
                warns++;
                numberInBatch++;
                continue;
            }
            MojErrCheck(err);
            if (!found)
                break;
            err = item->toObject(obj, m_kindEngine);
            MojErrCheck(err);
            const MojObject& id = item->id();
            MojObject deleted;
            err = delObj(id, obj, item, deleted, req, flags);
            MojErrCheck(err);
            ++count;
            numberInBatch++;
        }

        if (warns > 0)
            MojLogNotice(s_log, _T("delquery index_warnings: %s, count: %d\n"), newQuery.from().data(), warns);
        countOut += count;

        err = cursor.close();
        MojErrCheck(err);

        if(numberInBatch >= AutoBatchSize) // sing > - just in case something messed up
        {
            err = commitBatch(req);
            MojErrCheck(err);
        }

        if(count == 0)
            break;

        queryLimit -= newQuery.limit();
        if(queryLimit > AutoBatchSize)
            newQuery.limit(AutoBatchSize);
        else
            newQuery.limit(queryLimit);
    }

    return MojErrNone;
}



MojErr MojDb::findImpl(const MojDbQuery& query, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op)
{
	MojLogTrace(s_log);

	if (cursor.isOpen())
		MojErrThrow(MojErrDbCursorAlreadyOpen);

	MojErr err = m_kindEngine.find(query, cursor, watcher, req, op);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::getImpl(const MojObject& id, MojObjectVisitor& visitor, MojDbOp op, MojDbReq& req)
{
	MojRefCountedPtr<MojDbStorageItem> item;
	MojErr err = m_objDb->get(id, req.txn(), false, item);
	MojErrCheck(err);
	if (item.get()) {
		MojString kindId;
		err = item->kindId(kindId, m_kindEngine);
		MojErrCheck(err);
		err = m_kindEngine.checkPermission(kindId, op, req);
		MojErrCheck(err);
		err = item->visit(visitor, m_kindEngine);
		MojErrCheck(err);
		err = item->close();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDb::putImpl(MojObject& obj, MojUInt32 flags, MojDbReq& req, bool checkSchema)
{
	MojLogTrace(s_log);

	MojRefCountedPtr<MojDbStorageItem> prevItem;
	MojObject id;
	if (obj.get(IdKey, id)) {
        // Attach shard ID only put query but putKind and merge when shard ID exists
        if (MojFlagGet(flags, FlagNone)) {
            MojErr err = attachShardId(id);
            MojErrCheck(err);
        }
		// get previous revision if we have an id
		MojErr err = m_objDb->get(id, req.txn(), true, prevItem);
		MojErrCheck(err);
	}
	if (MojFlagGet(flags, FlagIgnoreMissing) && MojFlagGet(flags, FlagMerge)) {
		if (!prevItem.get()) {
			MojErr err = obj.putBool(MojDb::IgnoreIdKey, true);	// so that we can drop it in output
			MojErrCheck(err);
			return MojErrNone;
		}
	}

	MojDbOp op = OpCreate;
	MojObject* prevPtr = NULL;
	MojObject prev;
	if (prevItem.get()) {
		// deal with prev, if it exists
		op = OpUpdate;
		prevPtr = &prev;
		MojErr err = prevItem->toObject(prev, m_kindEngine);
		MojErrCheck(err);

		if (MojFlagGet(flags, FlagMerge)) {
			// do merge
			MojObject merged;
			err = mergeInto(merged, obj, prev);
			MojErrCheck(err);
			obj = merged;
		} else if (!MojFlagGet(flags, FlagForce)) {
			// if the force flag is not set, throw error if we are updating
			// an existing, non-deleted object and no rev was specified
			MojInt64 rev;
			if (obj.get(RevKey, rev) == false) {
				bool deleted = false;
				if (!prev.get(DelKey, deleted) || !deleted)
					MojErrThrow(MojErrDbRevNotSpecified);
			}
		}
	}

	// if the new object has a rev and it doesn't match the old rev, don't do the update
	MojInt64 newRev;
	if (prevPtr != NULL && obj.get(RevKey, newRev)) {
		MojInt64 oldRev;
		MojErr err = prevPtr->getRequired(RevKey, oldRev);
		MojErrCheck(err);
		if (!MojFlagGet(flags, FlagForce) && newRev != oldRev)
			MojErrThrowMsg(MojErrDbRevisionMismatch, _T("db: revision mismatch - expected %lld, got %lld"), oldRev, newRev);
	}

	// save it
	MojErr err = putObj(id, obj, prevPtr, prevItem.get(), req, op, checkSchema);
	MojErrCheck(err);

	if (prevItem.get()) {
		err = prevItem->close();
		MojErrCheck(err);
	}
	return MojErrNone;
}

/***********************************************************************
 * attachShardId
 *
 * Attach shard ID in front of _id
 * 1. decode _id and get byte vector
 * 2. decode shard ID and get byte vector
 * 3. merge shard ID byte vector and _id byte vector
 * 4. replace old _id with new one
 ***********************************************************************/
MojErr MojDb::attachShardId(MojObject& id)
{
    MojString idStr;
    MojErr err = id.stringValue(idStr);
    MojErrCheck(err);
    if (!(idStr.startsWith(MojDb::KindIdPrefix) || idStr.startsWith(MojDb::DbStateObjId)
        || idStr.startsWith(MojDb::QuotaIdPrefix) || idStr.startsWith(MojDb::PermissionIdPrefix))) {
        // Max 96bit _id(0xFFFFFFFFFFFFFFFFFFFFFFFF) converted base-64 is "zzzzzzzzzzzzzzzz"
        if (idStr.length() > 16 || idStr.compare(_T("zzzzzzzzzzzzzzzz")) > 0) {
            MojErrThrowMsg(MojErrDbMalformedId, _T("db: Invalid _id length"));
        }
        if (!m_shardId.empty()) {
            // Max 64bit _id(0xFFFFFFFFFFFFFFFF) converted base-64 is "zzzzzzzzzzw"
            if(idStr.length() <= 11 && idStr.compare(_T("zzzzzzzzzzw")) <= 0) {
                // decode _id
                MojVector<MojByte> idByteVec;
                err = idStr.base64Decode(idByteVec);
                MojErrCheck(err);
                // decode shard ID
                MojVector<MojByte> byteVec;
                err = m_shardId.base64Decode(byteVec);
                MojErrCheck(err);
                // attach shard ID in front of _id
                byteVec.append(idByteVec.begin(), idByteVec.end());
                err = idStr.base64Encode(byteVec, false);
                MojErrCheck(err);
                // replace old _id with new one attached shard ID
                id = MojObject(idStr);
            }
        }
    }

	return MojErrNone;
}

MojErr MojDb::nextId(MojInt64& idOut)
{
	MojErr err = m_idSeq->get(idOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::getLocale(MojString& valOut, MojDbReq& req)
{
	MojObject curLocale;
	MojErr err = getState(LocaleKey, curLocale, req);
	MojErrCheck(err);
	err = curLocale.stringValue(valOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::getState(const MojChar* key, MojObject& valOut, MojDbReq& req)
{
	MojLogTrace(s_log);

	MojString idStr;
	MojErr err = idStr.assign(DbStateObjId);
	MojErrCheck(err);
	MojObject id(idStr);

	MojRefCountedPtr<MojDbStorageItem> item;
	err = m_objDb->get(id, req.txn(), false, item);
	MojErrCheck(err);
	if (item.get()) {
		MojObject obj;
		err = item->toObject(obj, m_kindEngine, true);
		MojErrCheck(err);
		MojObject val;
		if (obj.get(key, val))	{
			valOut = val;
		}
	}
	return MojErrNone;
}

MojErr MojDb::updateState(const MojChar* key, const MojObject& val, MojDbReq& req)
{
	MojLogTrace(s_log);

	MojObject obj;
	MojErr err = obj.putString(IdKey, DbStateObjId);
	MojErrCheck(err);
	err = obj.putString(KindKey, MojDbKindEngine::DbStateId);
	MojErrCheck(err);
	err = obj.put(key, val);
	MojErrCheck(err);
	err = putImpl(obj, FlagMerge, req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::checkDbVersion(const MojChar* path)
{
	MojAssert(path);
	MojLogTrace(s_log);

	MojString version;
	MojString versionFileName;
	MojErr err = versionFileName.format(_T("%s/%s"), path, VersionFileName);
	MojErrCheck(err);
	err = MojFileToString(versionFileName, version);
	MojErrCatch(err, MojErrNotFound) {
		// if the version file is not found, create it
		// make sure the directory exists
		err = MojCreateDirIfNotPresent(path);
		MojErrCheck(err);
		err = version.format(_T("%lld"), DatabaseVersion);
		MojErrCheck(err);
		err = MojFileFromString(versionFileName, version);
		MojErrCheck(err);
	} else {
		MojObject versionObj;
		err = versionObj.fromJson(version);
		MojErrCheck(err);
		if (versionObj != DatabaseVersion) {
			MojErrThrowMsg(MojErrDbVersionMismatch,
					_T("db: version mismatch: expected '%lld', got '%lld'"),
					DatabaseVersion, versionObj.intValue());
		}
	}
	return MojErrNone;
}

MojErr MojDb::beginReq(MojDbReq& req, bool lockSchema)
{
	MojErr err = requireOpen();
	MojErrCheck(err);

	err = req.begin(this, lockSchema);
	MojErrCheck(err);

	if (m_permissionEngine.check(RoleType, AdminRole, req.domain(), MojDbPermissionEngine::WildcardOperation) == MojDbPermissionEngine::ValueAllow) {
		req.admin(true);
	}
	return MojErrNone;
}

MojErr MojDb::commitBatch(MojDbReq& req)
{
	// commit current batch and get things reset for next batch
	// Can NOT have db cursor open at this stage and new queries have to be started
	// Use with Caution otherwise you get db fatal errors

	// commit current and start another tran. Do NOT use for things that require schema lock
	MojErr err;

	err = req.end();
	if (err != MojErrNone)
        MojLogNotice(s_log, _T("CommitBatch req.end: err= %d\n"), (int)err);
	MojErrCheck(err);

	err = req.endBatch();
	if (err != MojErrNone)
        MojLogNotice(s_log, _T("CommitBatch req.endbatch: err= %d\n"), (int)err);
	MojErrCheck(err);

	req.beginBatch();

	err = beginReq(req, false);
	if (err != MojErrNone)
        MojLogNotice(s_log, _T("CommitBatch ended: err= %d\n"), (int)err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::commitKind(const MojString& id, MojDbReq& req, MojErr err)
{
	// if put failed, abort the txn
	MojErr errAcc = err;
	if (err == MojErrNone) {
		// refresh quotas
		req.txn()->refreshQuotas();
		errAcc = req.end();
		MojErrAccumulate(err, errAcc);
	} else {
		errAcc = req.abort();
		MojErrAccumulate(err, errAcc);
	}

	// if anything failed after the kind engine update, reload the kind
	if (err != MojErrNone) {
		errAcc = reloadKind(id);
		MojErrAccumulate(err, errAcc);
	}
	return err;
}

MojErr MojDb::reloadKind(const MojString& id)
{
	MojString dbId;
	MojErr err = formatKindId(id, dbId);
	MojErrCheck(err);

	MojDbReq req;
	err = req.begin(this, true);
	MojErrCheck(err);

	bool found = false;
	MojObject obj;
	err = get(dbId, obj, found, req);
	MojErrCheck(err);
	if (found) {
		err = m_kindEngine.reloadKind(id, obj, req);
		MojErrCheck(err);
	} else {
		err = m_kindEngine.delKind(id, req);
		MojErrCheck(err);
	}
	// refresh quotas
	req.txn()->refreshQuotas();
	err = req.end();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDb::assignIds(MojObject& obj)
{
	MojObject::Iterator objIter;
	MojErr err = obj.begin(objIter);
	MojErrCheck(err);
	MojObject::ConstIterator objEnd = obj.end();
	for (; objIter != objEnd; ++objIter) {
		MojObject& value = objIter.value();
		MojObject::Type type = value.type();
		if (type == MojObject::TypeArray) {
			MojObject::ArrayIterator arrayIter;
			err = value.arrayBegin(arrayIter);
			MojErrCheck(err);
			MojObject::ConstArrayIterator arrayEnd = value.arrayEnd();
			for (; arrayIter != arrayEnd; ++arrayIter) {
				if (arrayIter->type() == MojObject::TypeObject) {
					// assign an id to this object, and call assignIds recursively
					if (!arrayIter->contains(IdKey)) {
						MojInt64 id;
						MojErr err = nextId(id);
						MojErrCheck(err);
						MojString idStr;
						err = idStr.format(_T("%llx"), id);
						MojErrCheck(err);
						err = arrayIter->put(IdKey, idStr);
						MojErrCheck(err);
					}
					err = assignIds(*arrayIter);
					MojErrCheck(err);
				}
			}
		} else if (type == MojObject::TypeObject) {
			err = assignIds(value);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDb::formatKindId(const MojChar* id, MojString& dbIdOut)
{
	MojErr err = dbIdOut.format(_T("_kinds/%s"), id);
	MojErrCheck(err);

	return MojErrNone;
}
