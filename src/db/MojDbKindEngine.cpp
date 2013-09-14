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


#include "db/MojDbKindEngine.h"
#include "db/MojDb.h"
#include "db/MojDbKind.h"
#include "db/MojDbQuery.h"
#include "db/MojDbServiceDefs.h"

// prefixes
const MojChar* const MojDbKindEngine::KindIdPrefix = _T("_kinds/");
// db names
const MojChar* const MojDbKindEngine::KindsDbName = _T("kinds.db");
const MojChar* const MojDbKindEngine::IndexIdsDbName = _T("indexIds.db");
const MojChar* const MojDbKindEngine::IndexIdsSeqName = _T("indexId");
// Kind built-in
const MojChar* const MojDbKindEngine::KindKindId = _T("Kind:1");
const MojChar* const MojDbKindEngine::KindKindIdPrefix = _T("Kind:");
const MojChar* const MojDbKindEngine::KindKindJson =
	_T("{\"id\":\"Kind:1\",\"owner\":\"com.palm.admin\",")
	_T("\"indexes\":[{\"name\":\"_rev\",\"props\":[{\"name\":\"_rev\"}],\"incDel\":true},{\"name\":\"kindId\",\"props\":[{\"name\":\"id\"}]}]}");
// Object built-in
const MojChar* const MojDbKindEngine::RootKindId = _T("Object:1");
const MojChar* const MojDbKindEngine::RootKindJson =
	_T("{\"id\":\"Object:1\",\"owner\":\"com.palm.admin\",")
	_T("\"indexes\":[{\"name\":\"_sync_revIncDel\",\"props\":[{\"name\":\"_sync\", \"default\":false},{\"name\":\"_rev\"}],\"incDel\":true}]}");
// Rev-Time built-in
const MojChar* const MojDbKindEngine::RevTimestampId = _T("RevTimestamp:1");
const MojChar* const MojDbKindEngine::RevTimestampJson =
	_T("{\"id\":\"RevTimestamp:1\",\"owner\":\"com.palm.admin\",")
	_T("\"indexes\":[{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}]}");
// DB State built-in
const MojChar* const MojDbKindEngine::DbStateId = _T("DbState:1");
const MojChar* const MojDbKindEngine::DbStateJson =
	_T("{\"id\":\"DbState:1\",\"owner\":\"com.palm.admin\"}");
// Permission built-in
const MojChar* const MojDbKindEngine::PermissionId = _T("Permission:1");
const MojChar* const MojDbKindEngine::PermissionIdPrefix = _T("Permission:");
const MojChar* const MojDbKindEngine::PermissionJson =
	_T("{\"id\":\"Permission:1\",\"owner\":\"com.palm.admin\",\"sync\":true,")
	_T("\"indexes\":[{\"name\":\"permissionIdx\",\"props\":[{\"name\":\"object\"},{\"name\":\"type\"},{\"name\":\"caller\"}]}]}");
// Quota built-in
const MojChar* const MojDbKindEngine::QuotaId = _T("Quota:1");
const MojChar* const MojDbKindEngine::QuotaIdPrefix = _T("Quota:");
const MojChar* const MojDbKindEngine::QuotaJson =
	_T("{\"id\":\"Quota:1\",\"owner\":\"com.palm.admin\"}");

MojLogger MojDbKindEngine::s_log(_T("db.kindEngine"));

MojDbKindEngine::MojDbKindEngine()
: m_db(NULL)
{
	MojLogTrace(s_log);
}

MojDbKindEngine::~MojDbKindEngine()
{
	MojLogTrace(s_log);

	(void) close();
}

MojErr MojDbKindEngine::open(MojDb* db, MojDbReq& req)
{
	MojAssert(db);
	MojAssertWriteLocked(db->m_schemaLock);
	MojLogTrace(s_log);

	// open kind db and index seq
	m_db = db;
	MojDbStorageEngine* engine = db->storageEngine();
	MojDbStorageTxn* txn = req.txn();
	MojAssert(engine);
	MojErr err = engine->openDatabase(KindsDbName, txn, m_kindDb);
	MojErrCheck(err);
	err = engine->openDatabase(IndexIdsDbName, txn, m_indexIdDb);
	MojErrCheck(err);
	err = engine->openSequence(IndexIdsSeqName, txn, m_indexIdSeq);
	MojErrCheck(err);
	// built-in kinds
	err = addBuiltin(RootKindJson, req);
	MojErrCheck(err);
	err = addBuiltin(KindKindJson, req);
	MojErrCheck(err);
	err = addBuiltin(RevTimestampJson, req);
	MojErrCheck(err);
	err = addBuiltin(DbStateJson, req);
	MojErrCheck(err);
	err = addBuiltin(PermissionJson, req);
	MojErrCheck(err);
	err = addBuiltin(QuotaJson, req);
	MojErrCheck(err);
	// built-in indexes
	err = setupRootKind();
	MojErrCheck(err);
	// locale
	err = db->getLocale(m_locale, req);
	MojErrCheck(err);
	// load kinds from obj db
	err = loadKinds(req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::close()
{
	MojLogTrace(s_log);
	MojErr err = MojErrNone;

	if (isOpen()) {
		// close all kinds
		for (KindMap::ConstIterator i = m_kinds.begin(); i != m_kinds.end(); ++i) {
			MojErr errClose = (*i)->close();
			MojErrAccumulate(err, errClose);
		}
		m_kinds.clear();
		// close index seq/db
		MojErr errClose = m_indexIdSeq->close();
		MojErrAccumulate(err, errClose);
		m_indexIdSeq.reset();
		errClose = m_indexIdDb->close();
		MojErrAccumulate(err, errClose);
		m_indexIdDb.reset();
		// close kind db
		errClose = m_kindDb->close();
		MojErrAccumulate(err, errClose);
		m_kindDb.reset();
		m_locale.clear();
		m_db = NULL;
	}
	return err;
}

MojErr MojDbKindEngine::stats(MojObject& objOut, MojDbReq& req, bool verify, MojString *pKind)
{
	MojLogTrace(s_log);
	MojAssert(isOpen());


	for (KindMap::ConstIterator i = m_kinds.begin(); i != m_kinds.end(); ++i) {
		MojObject kindAnalysis;
		MojSize usage = 0;
		if (pKind && *pKind != (*i)->id())
			continue;		// get stats for only one kind
		MojErr err = (*i)->stats(kindAnalysis, usage, req, verify);
		if (err != MojErrNone)
			continue;
		err = objOut.put((*i)->id(), kindAnalysis);
		if (err != MojErrNone)
			continue;		// go through all kinds, even if some have errors
	}
	return MojErrNone;
}

MojErr MojDbKindEngine::updateLocale(const MojChar* locale, MojDbReq& req)
{
	MojAssert(locale);
	MojAssert(isOpen());
	MojAssertWriteLocked(m_db->m_schemaLock);
	MojLogTrace(s_log);

	MojErr err = m_locale.assign(locale);
	MojErrCheck(err);
	for (KindMap::ConstIterator i = m_kinds.begin(); i != m_kinds.end(); ++i) {
		err = (*i)->updateLocale(locale, req);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKindEngine::update(MojObject* newObj, const MojObject* oldObj, MojDbReq& req, MojDbOp op, MojTokenSet& tokenSetOut, bool checkSchema)
{
	MojAssert(isOpen());
	MojAssert(newObj || oldObj);
	MojLogTrace(s_log);

	MojDbKind* oldKind = NULL;
	MojErr err = MojErrNone;
	if (oldObj) {
		err = getKind(*oldObj, oldKind);
		MojErrCheck(err);
	}
	MojDbKind* newKind = NULL;
	err = getKind(newObj ? *newObj : *oldObj, newKind);
	MojErrCheck(err);

	if (oldKind && oldKind != newKind) {
		// if the kind changes, remove this object from the old
		// kind and add it to the new kind
       err = oldKind->update(NULL, oldObj, OpDelete, req, checkSchema);
		MojErrCheck(err);
		err = newKind->update(newObj, NULL, OpCreate, req, checkSchema);
		MojErrCheck(err);
	} else {
       err = newKind->update(newObj, oldObj, op, req, checkSchema);
		MojErrCheck(err);
	}
	err = newKind->tokenSet(tokenSetOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::find(const MojDbQuery& query, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op)
{
	MojAssert(isOpen());
	MojLogTrace(s_log);

	MojErr err = query.validate();
	MojErrCheck(err);
    // In order to find collate index, cursor needs kindEngine
    cursor.kindEngine(this);
	err = cursor.init(query);
	MojErrCheck(err);

	MojDbKind* kind = NULL;
	err = getKind(query.from().data(), kind);
	MojErrCheck(err);
	MojAssert(kind);
	err = kind->find(cursor, watcher, req, op);
	MojErrCheck(err);
	cursor.kindEngine(this);

	return MojErrNone;
}

MojErr MojDbKindEngine::checkPermission(const MojString& id, MojDbOp op, MojDbReq& req)
{
	MojAssert(isOpen());
	MojLogTrace(s_log);

	MojDbKind* kind = NULL;
	MojErr err = getKind(id.data(), kind);
	MojErrCheck(err);
	err = kind->checkPermission(op, req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::checkOwnerPermission(const MojString& id, MojDbReq& req)
{
	MojAssert(isOpen());
	MojLogTrace(s_log);

	MojDbKind* kind = NULL;
	MojErr err = getKind(id.data(), kind);
	MojErrCheck(err);
	err = kind->checkOwnerPermission(req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::getKinds(MojVector<MojObject>& kindsOut)
{
	MojLogTrace(s_log);

	MojDbKind* rootKind = NULL;
	MojErr err = getKind(RootKindId, rootKind);
	MojErrCheck(err);

	err = rootKind->subKinds(kindsOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::putKind(const MojObject& obj, MojDbReq& req, bool builtin)
{
	MojAssert(isOpen());
	MojAssertWriteLocked(m_db->m_schemaLock);
	MojLogTrace(s_log);

	// parse out id
	MojString id;
	MojErr err = obj.getRequired(MojDbServiceDefs::IdKey, id);
	MojErrCheck(err);

	KindMap::ConstIterator i = m_kinds.find(id);
	if (i == m_kinds.end()) {
		// new kind
		err = createKind(id, obj, req, builtin);
		MojErrCheck(err);
	} else {
		// existing kind
        MojLogNotice(s_log, _T("UpdatingKind: %s \n"), id.data());
		err = (*i)->configure(obj, m_kinds, m_locale, req);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKindEngine::delKind(const MojString& id, MojDbReq& req)
{
	MojAssert(isOpen());
	MojAssertWriteLocked(m_db->m_schemaLock);
	MojLogTrace(s_log);

	KindMap::ConstIterator i = m_kinds.find(id);
	if (i != m_kinds.end()) {
		// notify kind obj
		MojErr err = (*i)->drop(req);
		MojErrCheck(err);
		// delete from map
		bool found = false;
		err = m_kinds.del(id, found);
		MojErrCheck(err);
		MojAssert(found);
	}
	return MojErrNone;
}

MojErr MojDbKindEngine::reloadKind(const MojString& id, const MojObject& kindObj, MojDbReq& req)
{
	MojAssert(isOpen());
	MojAssertWriteLocked(m_db->m_schemaLock);
	MojLogTrace(s_log);

	// delete old one
	bool found = false;
	MojErr err = m_kinds.del(id, found);
	MojErrCheck(err);
	// replace with new one
	err = createKind(id, kindObj, req);
	MojErrCheck(err);

	return MojErrNone;
}

MojDbPermissionEngine* MojDbKindEngine::permissionEngine()
{
	return m_db->permissionEngine();
}

MojDbQuotaEngine* MojDbKindEngine::quotaEngine()
{
	return m_db->quotaEngine();
}

MojErr MojDbKindEngine::tokenFromId(const MojChar* id, MojInt64& tokOut)
{
	MojAssert(id);

	MojDbKind* kind;
	MojErr err = getKind(id, kind);
	MojErrCheck(err);
	tokOut = kind->token();

	return MojErrNone;
}

MojErr MojDbKindEngine::idFromToken(MojInt64 tok, MojString& idOut)
{
	TokMap::ConstIterator i = m_tokens.find(tok);
	if (i == m_tokens.end())
		MojErrThrow(MojErrDbInvalidKindToken);
	idOut = i.value();

	return MojErrNone;
}

MojErr MojDbKindEngine::tokenSet(const MojChar* kindName, MojTokenSet& tokenSetOut)
{
	MojLogTrace(s_log);
	
	MojAssert(kindName);
	MojDbKind* kind = NULL;
	MojErr err = getKind(kindName, kind);
	MojErrCheck(err);
	MojAssert(kind);
	err = kind->tokenSet(tokenSetOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::setupRootKind()
{
	MojLogTrace(s_log);
	MojAssertWriteLocked(m_db->m_schemaLock);

	MojDbKind* kind = NULL;
	MojErr err = getKind(RootKindId, kind);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::addBuiltin(const MojChar* json, MojDbReq& req)
{
	MojLogTrace(s_log);
	MojAssertWriteLocked(m_db->m_schemaLock);

	MojObject obj;
	MojErr err = obj.fromJson(json);
	MojErrCheck(err);
	err = putKind(obj, req, true);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::createKind(const MojString& id, const MojObject& obj, MojDbReq& req, bool builtin)
{
	MojRefCountedPtr<MojDbKind> kind(new MojDbKind(m_db->storageDatabase(), this, builtin));
	MojAllocCheck(kind.get());
	MojErr err = kind->init(id);
	MojErrCheck(err);
	err = kind->configure(obj, m_kinds, m_locale, req);
	MojErrCheck(err);
	// update kind map
	err = m_kinds.put(id, kind);
	MojErrCheck(err);
	err = m_tokens.put(kind->token(), id);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::loadKinds(MojDbReq& req)
{
	MojAssert(isOpen());
	MojAssertWriteLocked(m_db->m_schemaLock);
	MojLogTrace(s_log);

	MojDbQuery query;
	MojErr err = query.from(KindKindId);
	MojErrCheck(err);
	MojDbCursor cursor;
	err = m_db->find(query, cursor, req);
	MojErrCheck(err);

	for (;;) {
		MojObject obj;
		bool found = false;
		err = cursor.get(obj, found);
		MojErrCheck(err);
		if (!found)
			break;
		// load kind
		MojErr loadErr = err = putKind(obj, req);
		MojErrCatchAll(err) {
			MojString id;
			bool found = false;
			MojErr err = obj.get(MojDbServiceDefs::IdKey, id, found);
			MojErrCheck(err);
			MojString errStr;
			MojErrToString(loadErr, errStr);
			MojLogError(s_log, _T("error loading kind '%s' - %s"), id.data(), errStr.data());
		}
	}
	err = cursor.close();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::getKind(const MojObject& obj, MojDbKind*& kind)
{
	MojLogTrace(s_log);

	MojString kindName;
	bool found = false;
	MojErr err = obj.get(MojDb::KindKey, kindName, found);
	MojErrCheck(err);
	if (!found)
		MojErrThrow(MojErrDbKindNotSpecified);
	err = getKind(kindName.data(), kind);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindEngine::getKind(const MojChar* kindName, MojDbKind*& kind)
{
	MojLogTrace(s_log);

	KindMap::ConstIterator iter = m_kinds.find(kindName);
	if (iter == m_kinds.end())
		MojErrThrowMsg(MojErrDbKindNotRegistered, _T("kind not registered: '%s'"), kindName);
	kind = iter.value().get();

	return MojErrNone;
}

MojErr MojDbKindEngine::formatKindId(const MojChar* id, MojString& dbIdOut)
{
    MojErr err = dbIdOut.format(_T("_kinds/%s"), id);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * addShardIdToMasterKind
 * ###################################################################
 * TODO: Rewrite this function for MojDbKindEngine usage! [18 Sep 13]
 * ###################################################################
 * Add shard ID into Kind:1
 * 1. Find a tuple that matches _kind from Kind:1
 * 2. If _kind does not exist in Kind:1, return "Kind not Registered"
 * 3. Extract shardIds array from the tuple
 * 4. Push shardId if shardId does not exist in extracted shardIds array
 *
 * @param shardId
 *   shard id
 *
 * @param obj
 *   kind id
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbKindEngine::addShardIdToMasterKind (MojString shardId, MojObject& obj, MojDbReq& req)
{
    bool foundOut;
    MojString kindId;
    MojErr err = obj.get(MojDb::KindKey, kindId, foundOut);
    MojErrCheck(err);
    if(foundOut && !shardId.empty() && !kindId.startsWith(MojDbKindEngine::KindKindId)) {
        // get _id from _kind
        MojString id;
        err = formatKindId(kindId.data(), id);
        MojErrCheck(err);
        // make query
        MojDbQuery query;
        err = query.from(MojDbKindEngine::KindKindId);
        MojErrCheck(err);
        MojObject idObj(id);
        err = query.where(MojDb::IdKey, MojDbQuery::OpEq, idObj);
        MojErrCheck(err);
        // find cursor using query
        MojDbCursor cursor;
        err = m_db->find(query, cursor);
        MojErrCheck(err);
        // get item from cursor
        MojDbStorageItem* prevItem = NULL;
        err = cursor.get(prevItem, foundOut);
        MojErrCheck(err);
        if(foundOut) {
            // convert extracted item to object type
            MojObject oldObj;
            err = prevItem->toObject(oldObj, m_db->m_kindEngine);
            MojErrCheck(err);
            MojObject mergedObj(oldObj);
            // extract shardIds from result object
            MojObject shardIdsObj;
            oldObj.get(MojDbServiceDefs::ShardIdKey, shardIdsObj);
            // check if shardId exists in extracted shardIds
            bool isExist = false;
            MojString shardIdStr;
            for(MojSize i=0; i<shardIdsObj.size(); i++) {
                shardIdsObj.at(i, shardIdStr, foundOut);
                if(shardIdStr.compare(shardId.data()) == 0) {
                    isExist = true;
                    break;
                }
            }
            if(!isExist) {
                // push shardId
                err = shardIdsObj.pushString(shardId.data());
                MojErrCheck(err);
                err = mergedObj.put(MojDbServiceDefs::ShardIdKey, shardIdsObj);
                MojErrCheck(err);
                // update Kind:1
                err = m_db->putObj(id, mergedObj, &oldObj, prevItem, req, OpUpdate);
                MojErrCheck(err);
            }
        } else {
            MojErrThrow(MojErrDbKindNotRegistered);
        }
    }

    return MojErrNone;
}

/**
 * removeShardIdsFromMasterKind
 * ###################################################################
 * TODO: Rewrite this function for MojDbKindEngine usage! [18 Sep 13]
 * ###################################################################
 *
 * @param kindId
 *   kind id
 *
 * @param shardIds
 *   vector of shard id's to remove
 *
 * @param req
 *   batch support
 *
 * @return MojErr
 */
MojErr MojDbKindEngine::removeShardIdsFromMasterKind (const MojString& kindId, const MojVector<MojUInt32>& shardIds, MojDbReq& req)
{
    bool foundOut;
    MojErr err;
    bool forceCommit = false;

    if(!kindId.empty() && (shardIds.size() > 0))
    {
        //MojErrCheck( db()->beginReq(req) );
        err = req.begin(db(), false);
        MojErrCheck(err);
        MojLogNotice(s_log, _T("Starting for Kind: %s"), kindId.data());
        // get _id from _kind
        MojString id;
        err = formatKindId(kindId.data(), id);
        MojErrCheck(err);
        // make query
        MojDbQuery query;
        err = query.from(MojDbKindEngine::KindKindId);
        MojErrCheck(err);
        MojObject idObj(id);
        err = query.where(MojDb::IdKey, MojDbQuery::OpEq, idObj);
        MojErrCheck(err);
        // find cursor using query
        MojDbCursor cursor;
        err = m_db->find(query, cursor, req);
        MojErrCheck(err);
        // get item from cursor
        MojDbStorageItem* prevItem = NULL;
        err = cursor.get(prevItem, foundOut);
        MojErrCheck(err);

        if(foundOut) {
            // convert extracted item to object type
            MojObject oldObj;
            MojLogNotice(s_log, _T("Processing Kind: %s"), kindId.data());
            err = prevItem->toObject(oldObj, m_db->m_kindEngine);
            MojErrCheck(err);
            MojObject mergedObj(oldObj);
            // extract shardIds from result object
            MojObject shardIdsObj;
            oldObj.get(MojDbServiceDefs::ShardIdKey, shardIdsObj);
            if (shardIdsObj.size() > 0) { // shard array may be empty
                MojString shardIdStr;
                MojUInt32 id_uint32;
                uint32_t countDeleted = 0;;
                for(MojSize i=0; i<shardIdsObj.size(); ) { //  shards registered for the Kind
                    shardIdsObj.at(i, shardIdStr, foundOut);
                    err = MojDbShardEngine::convertId(shardIdStr, id_uint32);
                    MojErrCheck(err);

                    if(shardIds.find(id_uint32) != MojInvalidIndex) { // is shard from Kind in the delete shard list ?
                        shardIdsObj.delString(i);
                        countDeleted ++;  // Note:  don't increment 'i' if we delete an entry in the array, otherwise we will skip elements
                        MojLogNotice(s_log, _T("removed shard id %s for Kind: %s"), shardIdStr.data(), kindId.data());
                    } else {
                        MojLogNotice(s_log, _T("shard id %s not associated with Kind: %s"), shardIdStr.data(), kindId.data());
                        i++;
                    }
                }

                if (countDeleted) { // Only update if a change has been made !!
                    err = mergedObj.put(MojDbServiceDefs::ShardIdKey, shardIdsObj);
                    MojErrCheck(err);
                    // update Kind:1
                    err = m_db->putObj(id, mergedObj, &oldObj, prevItem, req, OpUpdate);
                    MojErrCheck(err);
                    MojLogNotice(s_log, _T("Update object for Kind: %s"), kindId.data());
                    forceCommit = true;
                } else {
                    MojLogNotice(s_log, _T("No matching shards for Kind: %s"), kindId.data()); // TODO:  to info
                }
            } else {
                MojLogNotice(s_log, _T("No shards known for Kind: %s"), kindId.data()); // TODO:  to info
            }
        }
        else {
            MojErrThrow(MojErrDbKindNotRegistered);
        }

        err = req.end(forceCommit);
        MojErrCheck(err);

        MojLogNotice(s_log, _T("End for Kind: %s"), kindId.data()); // TODO: remove
    } else {
        MojLogWarning(s_log, _T("Empty parameters for Kind: %s"), kindId.data());
    }


    return MojErrNone;
}

