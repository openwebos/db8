/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#include "db/MojDbKind.h"
#include "db/MojDb.h"
#include "db/MojDbReq.h"
#include "db/MojDbIsamQuery.h"
#include "db/MojDbIndex.h"

const MojChar* const MojDbKind::CountKey = _T("count");
const MojChar* const MojDbKind::DelCountKey = _T("delCount");
const MojChar* const MojDbKind::DelSizeKey = _T("delSize");
const MojChar* const MojDbKind::ExtendsKey = _T("extends");
const MojChar* const MojDbKind::IndexesKey = _T("indexes");
const MojChar* const MojDbKind::NameKey = _T("name");
const MojChar* const MojDbKind::ObjectsKey = _T("objects");
const MojChar* const MojDbKind::OwnerKey = _T("owner");
const MojChar* const MojDbKind::RevisionSetsKey = _T("revSets");
const MojChar* const MojDbKind::SchemaKey = _T("schema");
const MojChar* const MojDbKind::SizeKey = _T("size");
const MojChar* const MojDbKind::SyncKey = _T("sync");
const MojChar* const MojDbKind::IdIndexJson = _T("{\"name\":\"_id\",\"props\":[{\"name\":\"_id\"}],\"incDel\":true}");
const MojChar* const MojDbKind::IdIndexName = _T("_id");
const MojChar* const MojDbKind::PermissionType = _T("db.kind");
const MojChar* const MojDbKind::VerifyCountKey = _T("_vcount");
const MojChar* const MojDbKind::VerifyWarnCountKey = _T("_vwarncount");
const MojChar* const MojDbKind::VerifyDelCountKey = _T("_vdelcount");
const MojChar* const MojDbKind::WarnKey = _T("warn");
const MojChar MojDbKind::VersionSeparator = _T(':');

MojLogger MojDbKind::s_log(_T("db.kind"));

MojDbKind::MojDbKind(MojDbStorageDatabase* db, MojDbKindEngine* kindEngine, bool builtIn)
: m_version(0),
  m_db(db),
  m_kindEngine(kindEngine),
  m_backup(false),
  m_builtin(builtIn)
{
	MojLogTrace(s_log);
}

MojDbKind::~MojDbKind()
{
	MojLogTrace(s_log);

	MojErr err = close();
	MojErrCatchAll(err);

	MojAssert(m_supers.empty());
	MojAssert(m_subs.empty());
}

bool MojDbKind::extends(const MojString& id) const
{
	MojLogTrace(s_log);

	if (m_id == id)
		return true;
	for (KindVec::ConstIterator i = m_supers.begin(); i != m_supers.end(); ++i) {
		if ((*i)->extends(id))
			return true;
	}
	return false;
}

MojErr MojDbKind::stats(MojObject& objOut, MojSize& usageOut, MojDbReq& req, bool verify)
{
	MojLogTrace(s_log);

#if defined(TESTDBKIND)
	MojLogInfo(s_log, _T("Subkinds for - %s ; count = %d\n"), m_id.data(), m_subs.size());
	int n = 0;
	for (KindVec::ConstIterator i = m_subs.begin(); i != m_subs.end(); ++i) {
		MojLogInfo(s_log, _T("SubKind %d: %s"), n++, (*i)->id().data());
	}
	MojLogInfo(s_log, _T("Supers for - %s ; count = %d\n"), m_id.data(), m_supers.size());
	n = 0;
	for (KindVec::ConstIterator i = m_supers.begin(); i != m_supers.end(); ++i) {
		MojLogInfo(s_log, _T("Super %d: %s"), n++, (*i)->id().data());
	}
#endif 
	// analyze objects
	MojDbQuery query;
	MojErr err = query.from(m_id);
	MojErrCheck(err);
	err = query.includeDeleted(true);
	MojErrCheck(err);
	MojDbCursor cursor;
	err = m_kindEngine->find(query, cursor, NULL, req, OpRead);
	MojLogInfo(s_log, _T("KindStats start: %s ; Indexes = %d; Using Index: %s; \n"), 
				m_id.data(), m_indexes.size(), cursor.m_dbIndex->name().data());

	MojErrCheck(err);
	MojSize count = 0;
	MojSize size = 0;
	MojSize delCount = 0;
	MojSize delSize = 0;
	MojSize warnings = 0;
	for (;;) {
		MojDbStorageItem* item = NULL;
		bool found = false;
		cursor.verifymode(true);
		err = cursor.get(item, found);
		if (err == MojErrInternalIndexOnFind) {
			warnings++;
			continue;
		}
		if (err != MojErrNone)		// for all other errors break and dump current stats 
			break;	
		if (!found)
			break;
		MojObject obj;
		err = item->toObject(obj, *m_kindEngine, true);
		if (err != MojErrNone)
			break;
		bool deleted = false;
		if (obj.get(MojDb::DelKey, deleted) && deleted) {
			delSize += item->size();
			delCount++;
		} else {
			size += item->size();
			count++;
		}
	}

	MojLogInfo(s_log, _T("KindStats Summary: %s : Count: %d; delCount: %d; warnings: %d \n"), m_id.data(), count, delCount, warnings);
	
	usageOut += size + delSize;
	MojObject info;
	err = info.put(SizeKey, (MojInt64) size);
	MojErrCheck(err);
	err = info.put(CountKey, (MojInt64) count);
	MojErrCheck(err);
	if (delCount > 0) {
		err = info.put(DelSizeKey, (MojInt64) delSize);
		MojErrCheck(err);
		err = info.put(DelCountKey, (MojInt64) delCount);
		MojErrCheck(err);
	}
	if (warnings > 0) {
		err = info.put(WarnKey, (MojInt64) warnings);
		MojErrCheck(err);
	}
	err = objOut.put(ObjectsKey, info);
	MojErrCheck(err);

	// and indexes
	MojObject indexes;
	
	for (IndexVec::ConstIterator i = m_indexes.begin(); i != m_indexes.end(); ++i) {
		MojObject indexInfo;
		err = (*i)->stats(indexInfo, usageOut, req);
		MojErrCheck(err);

		if (verify) {
			MojDbIndex *pi = i->get();
			MojErr err2 = verifyIndex(pi, indexInfo, req);
			MojErrCheck(err2);
		}

		err = indexes.put((*i)->name(), indexInfo);
		MojErrCheck(err);
	}
	err = objOut.put(IndexesKey, indexes);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::verifyIndex(MojDbIndex *pIndex, MojObject &iinfo, MojDbReq& req)
{
	// Goes throudh each index entry and verifies that it points to a valid object
	// For debugging purposes as stats for indexes does not access the target objects
	// Index->stats function does not have enough context to find the object
	// db/stats usage '{"verify":true,"kind":"xyz"}' - each optional
	
	MojDbQuery query;
	MojErr err = query.from(m_id);
	MojErrCheck(err);
	err = query.includeDeleted(true);
	MojErrCheck(err);
	MojDbCursor cursor;
	query.m_forceIndex = pIndex;		// Important: Otherwise, it will pick the default index
	cursor.verifymode(true);		// to get the errors
	err = m_kindEngine->find(query, cursor, NULL, req, OpRead);
	MojLogInfo(s_log, _T("Kind_verifyIndex: Kind: %s; Index: %s; idIndex: %X; size: %d; CursorIndex: %s \n"), m_name.data(), 
					pIndex->name().data(), pIndex->idIndex(), pIndex->size(), cursor.m_dbIndex->name().data());
	MojErrCheck(err);
	MojSize count = 0;
	MojSize delCount = 0;
	MojSize warnCount = 0;
	char s[1024];
	for (;;) {
		MojDbStorageItem* item = NULL;
		bool found = false;
		err = cursor.get(item, found);
		if (err == MojErrInternalIndexOnFind) {
			warnCount++;
			MojDbIsamQuery *iquery = (MojDbIsamQuery *)cursor.m_storageQuery.get();
			MojErr err2 =  MojByteArrayToHex(iquery->m_keyData, iquery->m_keySize, s); 
			MojErrCheck(err2);
			MojChar *ids = (iquery->m_keySize > 18) ? (MojChar *)(iquery->m_keyData + iquery->m_keySize - 17) : NULL; 
			MojLogInfo(s_log, _T("VerifyIndex Warning: %s; KeySize: %d; %s ;id: %s \n"), 
					cursor.m_dbIndex->name().data(), iquery->m_keySize, s, ids);
			continue;
		}
		MojErrCheck(err);
		if (!found)
			break;
		MojObject obj;
		err = item->toObject(obj, *m_kindEngine, true);
		MojErrCheck(err);
		bool deleted = false;
		if (obj.get(MojDb::DelKey, deleted) && deleted) {
			delCount++;
		} else {
			count++;
		}
	}

	MojLogInfo(s_log, _T("Kind_verifyIndex Counts: Kind: %s; Index: %s; count: %d; delcount: %d; warnings: %d \n"), m_name.data(), 
					pIndex->name().data(), count, delCount, warnCount);
	
	err = iinfo.put(VerifyCountKey, (MojInt64)count);
	MojErrCheck(err);
	err = iinfo.put(VerifyWarnCountKey, (MojInt64) warnCount);
	MojErrCheck(err);
	err = iinfo.put(VerifyDelCountKey, (MojInt64) delCount);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::init(const MojString& id)
{
	MojLogTrace(s_log);

	// parse name and version out of id
	if (id.length() > KindIdLenMax)
		MojErrThrowMsg(MojErrDbMalformedId, _T("db: kind id too long"));
	MojSize sepIdx = id.rfind(VersionSeparator);
	if (sepIdx == MojInvalidIndex)
		MojErrThrow(MojErrDbMalformedId);
	MojErr err = id.substring(0, sepIdx, m_name);
	MojErrCheck(err);
	MojString str;
	err = id.substring(sepIdx + 1, id.length() - sepIdx - 1, str);
	MojErrCheck(err);
	const MojChar* end = NULL;
	MojInt64 ver = MojStrToInt64(str.data(), &end, 0);
	if (*end != '\0' || ver < 0 || ver > MojUInt32Max)
		MojErrThrow(MojErrDbMalformedId);
	m_version = (MojUInt32) ver;
	m_id = id;

	return MojErrNone;
}

MojErr MojDbKind::configure(const MojObject& obj, const KindMap& map, const MojString& locale, MojDbReq& req)
{
	MojLogTrace(s_log);

	// get owner before checking permissions
	MojString owner;
	MojErr err = obj.getRequired(OwnerKey, owner);
	MojErrCheck(err);
	// only admin can change owner
	if (!m_owner.empty() && m_owner != owner && !req.admin()) {
		err = deny(req);
		MojErrCheck(err);
	}
	m_owner = owner;

	err = checkPermission(OpKindUpdate, req);
	MojErrCheck(err);

	// schema
	MojObject schema;
	if (obj.get(SchemaKey,schema)) {
		err = m_schema.fromObject(schema);
		MojErrCheck(err);
	}
	// supers
	StringVec superIds;
	MojObject supers;
	if (obj.get(ExtendsKey, supers)) {
		MojObject::ConstArrayIterator end = supers.arrayEnd();
		for (MojObject::ConstArrayIterator i = supers.arrayBegin(); i != end; ++i) {
			MojString str;
			err = i->stringValue(str);
			MojErrCheck(err);
			err = superIds.push(str);
			MojErrCheck(err);
		}
	}
	// backup
	bool backup = false;
	if (obj.get(SyncKey, backup))
		m_backup = backup;
	bool updating = !m_obj.undefined();

	// load state
	m_state.reset(new MojDbKindState(m_id, m_kindEngine));
	MojAllocCheck(m_state.get());
	err = m_state->init(m_schema.strings(), req);
	MojErrCheck(err);
	// indexes
	err = configureIndexes(obj, locale, req);
	MojErrCheck(err);
	// supers
	err = updateSupers(map, superIds, updating, req);
	MojErrCheck(err);
	// revision sets
	err = configureRevSets(obj);
	MojErrCheck(err);

	// keep a copy of obj
	m_obj = obj;

	return MojErrNone;
}

MojErr MojDbKind::addIndex(const MojRefCountedPtr<MojDbIndex>& index)
{
	MojLogTrace(s_log);

	MojErr err = m_indexes.push(index);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::drop(MojDbReq& req)
{
	MojLogTrace(s_log);

	MojErr err = checkPermission(OpKindUpdate, req);
	MojErrCheck(err);
	err = req.curKind(this);
	MojErrCheck(err);

	// drop indexes
	MojErr errAcc = MojErrNone;
	for (IndexVec::ConstIterator i = m_indexes.begin();
		 i != m_indexes.end(); ++i) {
		errAcc = dropIndex(i->get(), req);
		MojErrAccumulate(err, errAcc);
	}
	m_indexes.clear();

	// remove self from super/sub kinds
	errAcc = close();
	MojErrAccumulate(err, errAcc);

	return err;
}

MojErr MojDbKind::close()
{
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojErr errClose = clearSupers();
	MojErrAccumulate(err, errClose);

	for (IndexVec::ConstIterator i = m_indexes.begin();
		 i != m_indexes.end(); ++i) {
		errClose = (*i)->close();
		MojErrAccumulate(err, errClose);
	}

	m_indexes.clear();
	m_indexObjects.clear();
	m_schema.clear();
	m_revSets.clear();
	m_superIds.clear();

	return err;
}

MojErr MojDbKind::updateLocale(const MojChar* locale, MojDbReq& req)
{
	MojLogTrace(s_log);
	MojAssert(locale);

	MojErr err = req.curKind(this);
	MojErrCheck(err);
	for (IndexVec::ConstIterator i = m_indexes.begin(); i != m_indexes.end(); ++i) {
		err = (*i)->updateLocale(locale, req);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKind::update(MojObject* newObj, const MojObject* oldObj, MojDbOp op, MojDbReq& req, bool checkSchema)
{
	MojLogTrace(s_log);

	MojErr err = checkPermission(op, req);
	MojErrCheck(err);
	err = req.curKind(this);
	MojErrCheck(err);

#if defined(TESTDBKIND)
	MojString s;
	MojErr e2;
	
	if (oldObj) {
		e2 = oldObj->toJson(s);
		MojLogInfo(s_log, _T("Kind_Update_OldObj: %s ;\n"), s.data());
	}
	if (newObj) {
		e2 = newObj->toJson(s);
		MojLogInfo(s_log, _T("Kind_Update_NewObj: %s ;\n"), s.data());
	}
#endif
	if (newObj) {
		// add the _backup property if not set
		if (m_backup && !newObj->contains(MojDb::SyncKey)) {
			err = newObj->putBool(MojDb::SyncKey, true);
			MojErrCheck(err);
		}

		// TEMPORARY!!! This should be done in pre-update to also check parent kinds
        	// warning message comes from preUpdate
		if(checkSchema)
		{
		   
		   MojSchema::Result res;
		   err = m_schema.validate(*newObj, res);
		   MojErrCheck(err);
		   if (!res.valid())
		   {
		      MojErrThrowMsg(MojErrSchemaValidation, _T("schema validation failed for kind '%s': %s"),
		                     m_id.data(), res.msg().data());
		   }
		}
        
	}

	// update revSets and validate schema
	err = preUpdate(newObj, oldObj, req);
	MojErrCheck(err);
	// update indexes
	MojVector<MojDbKind*> kindVec;
	MojInt32 idxcount = 0;
	err = updateIndexes(newObj, oldObj, req, op, kindVec, idxcount);
	MojLogInfo(s_log, _T("Kind_UpdateIndexes_End: %s; supers = %d; indexcount = %d; updated = %d \n"), this->id().data(), 
				m_supers.size(), m_indexes.size(), idxcount);

	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::find(MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op)
{
	MojLogTrace(s_log);

	MojErr err = checkPermission(op, req);
	MojErrCheck(err);
	const MojDbQuery& query = cursor.query();
	MojDbIndex* index = indexForQuery(query);
	if (index == NULL)
		MojErrThrow(MojErrDbNoIndexForQuery);

	cursor.m_dbIndex = index;
	MojLogInfo(s_log, _T("Dbkind_find: Kind: %s, UsingIndex: %s, order: %s, limit: %d \n"), m_id.data(), index->name().data(), 
				query.order().data(), (int)query.limit());
	err = index->find(cursor, watcher, req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::subKinds(MojVector<MojObject>& kindsOut, const MojDbKind* parent)
{
	if (!m_supers.empty() && parent != m_supers[0])
		return MojErrNone;

	if (!m_builtin) {
		MojErr err = kindsOut.push(m_obj);
		MojErrCheck(err);
	}
	for (KindVec::ConstIterator i = m_subs.begin(); i != m_subs.end(); ++i) {
		MojErr err = (*i)->subKinds(kindsOut, this);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKind::tokenSet(MojTokenSet& tokenSetOut)
{
	MojErr err = tokenSetOut.init(m_state.get());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::checkPermission(MojDbOp op, MojDbReq& req)
{
	// if this request has admin privileges, skip the permissions check
	if (hasOwnerPermission(req))
		return MojErrNone;

	if (op == OpKindUpdate) {
		MojErr err = deny(req);
		MojErrCheck(err);
	} else {
		// check if permissions are set on this kind
		const MojChar* opStr = stringFromOperation(op);
		MojDbPermissionEngine::Value val = objectPermission(opStr, req);
		if (val != MojDbPermissionEngine::ValueAllow) {
			MojErr err = deny(req);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbKind::checkOwnerPermission(MojDbReq& req)
{
	if (!hasOwnerPermission(req)) {
		MojErr err = deny(req);
		MojErrCheck(err);
	}
	return MojErrNone;
}

bool MojDbKind::hasOwnerPermission(MojDbReq& req)
{
	return (req.admin() || req.domain() == m_owner);
}

MojErr MojDbKind::addSuper(MojDbKind* kind)
{
	MojAssert(kind);
	MojLogTrace(s_log);

	// we may have temporary cycles, so ignore them
	if (kind->extends(m_id))
		return MojErrNone;
	// if our current super is the root kind, remove it
	if (!m_supers.empty() && m_supers.front()->id() == MojDbKindEngine::RootKindId) {
		MojDbKind* root = m_supers.front();
		MojErr err = root->removeKind(root->m_subs, this);
		MojErrCheck(err);
		m_supers.clear();
	}
	// update vecs
	MojErr err = m_supers.push(kind);
	MojErrCheck(err);
	err = kind->m_subs.push(this);
	MojErrCheck(err);

	return MojErrNone;
}

MojDbPermissionEngine::Value MojDbKind::objectPermission(const MojChar* op, MojDbReq& req)
{
	MojDbPermissionEngine::Value val = m_kindEngine->permissionEngine()->
			check(PermissionType, m_id, req.domain(), op);
	if (val == MojDbPermissionEngine::ValueUndefined && !m_supers.empty()) {
		val = m_supers[0]->objectPermission(op, req);
	}
	return val;
}

MojErr MojDbKind::deny(MojDbReq& req)
{
	MojLogWarning(s_log, _T("db: permission denied for caller '%s' on kind '%s'"), req.domain().data(), m_id.data());
	if (m_kindEngine->permissionEngine()->enabled()) {
		// don't leak any information in an error message
		MojErrThrow(MojErrDbPermissionDenied);
	}
	return MojErrNone;
}

MojErr MojDbKind::updateIndexes(const MojObject* newObj, const MojObject* oldObj, const MojDbReq& req, MojDbOp op, MojVector<MojDbKind*>& kindVec, MojInt32& idxcount)
{
	MojErr err = kindVec.push(this);
	MojErrCheck(err);

	// update supers
	for (KindVec::ConstIterator i = m_supers.begin();
		 i != m_supers.end(); ++i) {
		if (kindVec.find((*i), 0) == MojInvalidIndex) {
			err = (*i)->updateIndexes(newObj, oldObj, req, op, kindVec, idxcount);
			MojErrCheck(err);
		} else {
			return MojErrNone;
		}
	}
	err = updateOwnIndexes(newObj, oldObj, req, idxcount);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::updateOwnIndexes(const MojObject* newObj, const MojObject* oldObj, const MojDbReq& req, MojInt32& idxcount)
{
	
	MojInt32 count = 0;

	for (IndexVec::ConstIterator i = m_indexes.begin();
		 i != m_indexes.end(); ++i) {
		count++;
		MojErr err = (*i)->update(newObj, oldObj, req.txn(), req.fixmode());
		MojErrCheck(err);
	}
	MojLogInfo(s_log, _T("Kind_UpdateOwnIndexes: %s; count: %d \n"), this->id().data(), count);
	
	idxcount += count;
	return MojErrNone;
}

MojErr MojDbKind::preUpdate(MojObject* newObj, const MojObject* oldObj, MojDbReq& req)
{
	// update supers
	for (KindVec::ConstIterator i = m_supers.begin();
		 i != m_supers.end(); ++i) {
		MojErr err = (*i)->preUpdate(newObj, oldObj, req);
		MojErrCheck(err);
	}
	// process new obj
	if (newObj) {
		// update revSets
		for (RevSetVec::ConstIterator i = m_revSets.begin();
			 i != m_revSets.end(); ++i) {
			MojErr err = (*i)->update(newObj, oldObj);
			MojErrCheck(err);
		}
		// validate schemas
		MojSchema::Result res;
		MojErr err = m_schema.validate(*newObj, res);
		MojErrCheck(err);
		if (!res.valid()) {
			//MojErrThrowMsg(MojErrSchemaValidation, _T("schema validation failed for kind '%s': %s"), m_id.data(), res.msg().data());
			MojLogWarning(s_log, _T("schema validation failed for kind '%s': %s"), m_id.data(), res.msg().data());
		}
	}
	return MojErrNone;
}

MojErr MojDbKind::configureIndexes(const MojObject& obj, const MojString& locale, MojDbReq& req)
{
	MojLogTrace(s_log);

	// make sure indexes changes count against our usage
	MojErr err = req.curKind(this);
	MojErrCheck(err);

	// add default id index to set
	MojObject idIndex;
	err = idIndex.fromJson(IdIndexJson);
	MojErrCheck(err);
	ObjectSet newIndexObjects;
	err = newIndexObjects.put(idIndex);
	MojErrCheck(err);
	// change back to a set and use contains
	MojSet<MojString> indexNames;
	MojString defaultIdxName;
	err = defaultIdxName.assign(IdIndexName);
	MojErrCheck(err);
	err = indexNames.put(defaultIdxName);
	MojErrCheck(err);
	// add indexes to set to uniquify and order them
	MojObject indexArray;
	if (obj.get(IndexesKey, indexArray)) {
		MojObject::ConstArrayIterator end = indexArray.arrayEnd();
		for (MojObject::ConstArrayIterator i = indexArray.arrayBegin(); i != end; ++i) {
			MojString indexName;
			err = i->getRequired(MojDbIndex::NameKey, indexName);
			MojErrCheck(err);
			err = indexName.toLower();
			MojErrCheck(err);
			if (!indexNames.contains(indexName)) {
				MojObject idx = *i;
				// make sure we keep the lower-cased index name
				err = idx.putString(MojDbIndex::NameKey, indexName);
				MojErrCheck(err);
				err = newIndexObjects.put(idx);
				MojErrCheck(err);
				err = indexNames.put(indexName);
				MojErrCheck(err);
			} else {
				MojErrThrowMsg(MojErrDbInvalidIndexName, _T("db: cannot repeat index name: '%s'"), indexName.data());
			}
		}
	}
	// figure out what to add and what to delete
	ObjectSet toDrop;
	err = m_indexObjects.diff(newIndexObjects, toDrop);
	MojErrCheck(err);
	ObjectSet toAdd;
	err = newIndexObjects.diff(m_indexObjects, toAdd);
	MojErrCheck(err);
	// drop deleted indexes
	IndexVec newIndexes;
	for (IndexVec::ConstIterator i = m_indexes.begin(); i != m_indexes.end(); ++i) {
		if (toDrop.contains((*i)->object())) {
			err = dropIndex(i->get(), req);
			MojErrCheck(err);
		} else {
			err = newIndexes.push(*i);
			MojErrCheck(err);
		}
	}
	// add new indexes
	for (ObjectSet::ConstIterator i = toAdd.begin(); i != toAdd.end(); ++i) {
		// create index
		MojRefCountedPtr<MojDbIndex> index(new MojDbIndex(this, m_kindEngine));
		MojAllocCheck(index.get());
		err = index->fromObject(*i, locale);
		MojErrCheck(err);
		// open index
		err = openIndex(index.get(), req);
		MojErrCheck(err);
		err = newIndexes.push(index);
		MojErrCheck(err);
	}
	// sort indexes by the prop vec so that for indexes that share prop prefixes, the shortest one comes first
	err = newIndexes.sort();
	MojErrCheck(err);
	// update members
	m_indexObjects = newIndexObjects;
	m_indexes = newIndexes;

	return MojErrNone;
}

MojErr MojDbKind::configureRevSets(const MojObject& obj)
{
	MojLogTrace(s_log);

	m_revSets.clear();
	MojSet<MojString> setNames;
	MojObject array;
	if (obj.get(RevisionSetsKey, array)) {
		MojObject::ConstArrayIterator end = array.arrayEnd();
		for (MojObject::ConstArrayIterator i = array.arrayBegin(); i != end; ++i) {
			MojRefCountedPtr<MojDbRevisionSet> set(new MojDbRevisionSet());
			MojAllocCheck(set.get());

			MojErr err = set->fromObject(*i);
			MojErrCheck(err);
			if (setNames.contains(set->name())) {
				MojErrThrowMsg(MojErrDbInvalidRevisionSet, _T("db: cannot repeat revSet name: '%s'"), set->name().data());
			}
			err = setNames.put(set->name());
			MojErrCheck(err);
			err = m_revSets.push(set);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojDbIndex* MojDbKind::indexForQuery(const MojDbQuery& query) const
{
	MojLogTrace(s_log);

	if (query.m_forceIndex)
		return query.m_forceIndex;			// for stats verify 
	// check our indexes
	for (IndexVec::ConstIterator i = m_indexes.begin();
		 i != m_indexes.end(); ++i) {
		if ((*i)->canAnswer(query)) {		// will this always find the best index?
			return i->get();
		}
	}
	// not found
	return NULL;
}

MojErr MojDbKind::updateSupers(const KindMap& map, const StringVec& superIds, bool updating, MojDbReq& req)
{
	MojLogTrace(s_log);
	MojInt32 indexes = 0;
	if (updating) {
		KindVec addedSupers;
		MojErr err = diffSupers(map, superIds, m_superIds, addedSupers);
		MojErrCheck(err);
		KindVec removedSupers;
		err = diffSupers(map, m_superIds, superIds, removedSupers);
		MojErrCheck(err);

		// remove/add our objects from new/removed supers
		if (!addedSupers.empty() || !removedSupers.empty()) {
			MojDbQuery query;
			err = query.from(m_id);
			MojErrCheck(err);
			err = query.includeDeleted(true);
			MojErrCheck(err);
			MojDbCursor cursor;
			cursor.kindEngine(m_kindEngine);
			err = find(cursor, NULL, req, OpKindUpdate);
			MojErrCheck(err);
			for (;;) {
				MojObject obj;
				bool found = false;
				err = cursor.get(obj, found);
				MojErrCheck(err);
				if (!found)
					break;

				for (KindVec::ConstIterator i = addedSupers.begin(); i != addedSupers.end(); ++i) {
					err = (*i)->updateOwnIndexes(&obj, NULL, req, indexes);
					MojErrCheck(err);
				}
				for (KindVec::ConstIterator i = removedSupers.begin(); i != removedSupers.end(); ++i) {
					err = (*i)->updateOwnIndexes(NULL, &obj, req, indexes);
					MojErrCheck(err);
				}
			}
		}
	}

	// remove old supers
	m_superIds = superIds;
	MojErr err = clearSupers();
	MojErrCheck(err);
	// look for new supers
	for (StringVec::ConstIterator i = m_superIds.begin(); i != m_superIds.end(); ++i) {
		KindMap::ConstIterator mapIter = map.find(*i);
		if (mapIter != map.end()) {
			err = addSuper(mapIter->get());
			MojErrCheck(err);
		}
	}
	// look for kinds that extend us
	for (KindMap::ConstIterator i = map.begin(); i != map.end(); ++i) {
		if ((*i)->m_superIds.find(m_id) != MojInvalidIndex) {
			err = (*i)->addSuper(this);
			MojErrCheck(err);
		}
	}
	// add root kind if we have no supers
	if (m_supers.empty()) {
		KindMap::ConstIterator mapIter = map.find(MojDbKindEngine::RootKindId);
		if (mapIter != map.end()) {
			err = addSuper(mapIter->get());
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbKind::diffSupers(const KindMap& map, const StringVec& vec1, const StringVec& vec2, KindVec& diffOut)
{
	for (StringVec::ConstIterator i = vec1.begin(); i != vec1.end(); ++i) {
		if (vec2.find(*i) == MojInvalidIndex) {
			KindMap::ConstIterator kindIter = map.find(*i);
			if (kindIter != map.end()) {
				MojErr err = diffOut.push(kindIter->get());
				MojErrCheck(err);
			}
		}
	}
	return MojErrNone;
}

MojErr MojDbKind::clearSupers()
{
	MojLogTrace(s_log);

	// remove old supers
	MojErr err = MojErrNone;
	for (KindVec::ConstIterator i = m_supers.begin(); i != m_supers.end(); ++i) {
		err = (*i)->removeKind((*i)->m_subs, this);
		MojErrCheck(err);
	}
	m_supers.clear();
	// remove old subs
	for (KindVec::ConstIterator i = m_subs.begin(); i != m_subs.end(); ++i) {
		err = (*i)->removeKind((*i)->m_supers, this);
		MojErrCheck(err);
	}
	m_subs.clear();

	return MojErrNone;
}

MojErr MojDbKind::openIndex(MojDbIndex* index, MojDbReq& req)
{
	MojAssert(index);
	MojAssert(m_db);
	MojLogTrace(s_log);

	// construct storage index name
	MojString name;
	MojErr err = name.format(_T("%s-%d-%s"), m_name.data(), m_version, index->name().data());
	MojErrCheck(err);
	// get id
	MojObject id;
	bool created = false;
	err = m_state->indexId(index->name(), req, id, created);
	MojErrCheck(err);
	// open
	MojRefCountedPtr<MojDbStorageIndex> storageIndex;
	err = m_db->openIndex(id, req.txn(), storageIndex);
	MojErrCheck(err);
	err = index->open(storageIndex.get(), id, req, created);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::dropIndex(MojDbIndex* index, MojDbReq& req)
{
	MojAssert(index);

	MojErr err = m_state->delIndex(index->name(), req);
	MojErrCheck(err);
	err = index->drop(req);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKind::removeKind(KindVec& vec, MojDbKind* kind)
{
	MojAssert(kind);
	MojLogTrace(s_log);

	MojSize idx = vec.find(kind);
	MojAssert(idx != MojInvalidIndex);
	MojErr err = vec.erase(idx, 1);
	MojErrCheck(err);

	return MojErrNone;
}

const MojChar* MojDbKind::stringFromOperation(MojDbOp op)
{
	switch (op) {
	case OpNone:
		return _T("none");
	case OpCreate:
		return _T("create");
	case OpRead:
		return _T("read");
	case OpUpdate:
		return _T("update");
	case OpDelete:
		return _T("delete");
	case OpExtend:
		return _T("extend");
	default:
		return _T("unknown");
	}
}

