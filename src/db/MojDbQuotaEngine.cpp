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


#include "db/MojDbQuotaEngine.h"
#include "db/MojDb.h"
#include "db/MojDbKind.h"
#include "db/MojDbServiceDefs.h"

const MojChar* const MojDbQuotaEngine::QuotasKey = _T("quotas");
const MojChar* const MojDbQuotaEngine::UsageDbName = _T("quotaUsage");

class MojDbQuotaCommitHandler : public MojSignalHandler
{
public:
	MojDbQuotaCommitHandler(MojDbQuotaEngine* engine, MojString& owner, MojInt64 size, MojDbStorageTxn* txn);
	MojErr handleCommit(MojDbStorageTxn* txn);

	MojDbQuotaEngine* m_engine;
	MojString m_owner;
	MojInt64 m_size;
	MojDbStorageTxn::CommitSignal::Slot<MojDbQuotaCommitHandler> m_slot;
};

MojDbQuotaEngine::MojDbQuotaEngine()
: MojDbPutHandler(MojDbKindEngine::QuotaId, QuotasKey),
  m_isOpen(false)
{
}

MojDbQuotaEngine::~MojDbQuotaEngine()
{
}

MojErr MojDbQuotaEngine::open(const MojObject& conf, MojDb* db, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(db);

	MojErr err = db->storageEngine()->openDatabase(_T("UsageDbName"), req.txn(), m_usageDb);
	MojErrCheck(err);
	err = MojDbPutHandler::open(conf, db, req);
	MojErrCheck(err);

	MojDbKindEngine::KindMap& kinds = db->kindEngine()->kindMap();
	for (MojDbKindEngine::KindMap::ConstIterator i = kinds.begin();
		 i != kinds.end(); ++i) {
		err = initUsage(i.value().get(), req);
		MojErrCheck(err);
	}
	err = refreshImpl(req.txn());
	MojErrCheck(err);

	m_isOpen = true;

	return MojErrNone;
}

MojErr MojDbQuotaEngine::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	MojErr errClose = MojDbPutHandler::close();
	MojErrAccumulate(err, errClose);
	if (m_usageDb.get()) {
		errClose = m_usageDb->close();
		MojErrAccumulate(err, errClose);
		m_usageDb.reset();
	}
	m_quotas.clear();
	m_isOpen = false;

	return err;
}

MojErr MojDbQuotaEngine::put(MojObject& obj, MojDbReq& req, bool putObj)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertWriteLocked(m_db->schemaLock());

	// check for admin permission
	if (!req.admin()) {
		MojErrThrow(MojErrDbPermissionDenied);
	}

	// pull params out of object
	MojString owner;
	MojErr err = obj.getRequired(MojDbServiceDefs::OwnerKey, owner);
	MojErrCheck(err);
	MojInt64 size = 0;
	err = obj.getRequired(MojDbServiceDefs::SizeKey, size);
	MojErrCheck(err);

	// validate owner
	err = validateWildcard(owner, MojErrDbInvalidOwner);
	MojErrCheck(err);

	// put object
	if (putObj) {
		MojString id;
		err = id.format(_T("_quotas/%s"), owner.data());
		MojErrCheck(err);
		err = obj.put(MojDb::IdKey, id);
		MojErrCheck(err);
		err = obj.putString(MojDb::KindKey, MojDbKindEngine::QuotaId);
		MojErrCheck(err);

		MojDbAdminGuard adminGuard(req);
		err = m_db->put(obj, MojDb::FlagForce, req);
		MojErrCheck(err);

		// defer commit of quota until txn commit
		MojRefCountedPtr<MojDbQuotaCommitHandler> handler(new MojDbQuotaCommitHandler(this, owner, size, req.txn()));
		MojAllocCheck(handler.get());
	} else {
		err = commitQuota(owner, size);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::curKind(const MojDbKind* kind, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(kind && txn);

	txn->m_quotaEngine = this;
	const MojString& id = kind->id();
	// set the current offset
	OffsetMap::ConstIterator i = txn->m_offsetMap.find(id);
	if (i == txn->m_offsetMap.end()) {
		txn->m_curQuotaOffset.reset(new MojDbQuotaEngine::Offset(id));
		MojAllocCheck(txn->m_curQuotaOffset.get());
		MojErr err = txn->m_offsetMap.put(id, txn->m_curQuotaOffset);
		MojErrCheck(err);
	} else {
		txn->m_curQuotaOffset = i.value();
	}
	// find the quota for this kind
	MojErr err = quotaForKind(kind, txn->m_curQuotaOffset->m_quota);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaEngine::applyUsage(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	for (OffsetMap::ConstIterator i = txn->m_offsetMap.begin();
		 i != txn->m_offsetMap.end(); ++i) {
		MojErr err = applyOffset(i.key(), i.value()->offset(), txn);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::applyQuota(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	for (OffsetMap::ConstIterator i = txn->m_offsetMap.begin();
		 i != txn->m_offsetMap.end(); ++i) {
		if (i.value()->m_quota.get()) {
			i.value()->m_quota->offset(i.value()->offset());
		}
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::kindUsage(const MojChar* kindId, MojInt64& usageOut, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojString kindStr;
	MojErr err = kindStr.assign(kindId);
	MojErrCheck(err);
	MojRefCountedPtr<MojDbStorageItem> item;
	err = getUsage(kindStr, txn, false, usageOut, item);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaEngine::quotaUsage(const MojChar* owner, MojInt64& sizeOut, MojInt64& usageOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	sizeOut = 0;
	usageOut = 0;
	QuotaMap::ConstIterator iter = m_quotas.find(owner);
	if (iter == m_quotas.end()) {
		MojErrThrow(MojErrNotFound);
	}
	sizeOut = iter.value()->size();
	usageOut = iter.value()->usage();

	return MojErrNone;
}

MojErr MojDbQuotaEngine::refresh()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertWriteLocked(m_db->schemaLock());

	MojRefCountedPtr<MojDbStorageTxn> txn;
	MojErr err = m_db->storageEngine()->beginTxn(txn);
	MojErrCheck(err);
	err = refreshImpl(txn.get());
	MojErrCheck(err);
	err = txn->commit();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuotaEngine::stats(MojObject& objOut, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// check for admin permission
	if (!req.admin()) {
		MojErrThrow(MojErrDbPermissionDenied);
	}
	for (QuotaMap::ConstIterator i = m_quotas.begin(); i != m_quotas.end(); ++i) {
		MojObject quota;
		MojErr err = quota.put(MojDbServiceDefs::SizeKey, i.value()->size());
		MojErrCheck(err);
		err = quota.put(MojDbServiceDefs::UsedKey, i.value()->usage());
		MojErrCheck(err);
		err = objOut.put(i.key(), quota);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::refreshImpl(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);
	MojAssertWriteLocked(m_db->schemaLock());

	// zero out quota usages
	for (QuotaMap::ConstIterator i = m_quotas.begin(); i != m_quotas.end(); ++i) {
		(*i)->usage(0);
	}
	// recalculate from kind usages
	MojDbKindEngine::KindMap& kinds = m_db->kindEngine()->kindMap();
	for (MojDbKindEngine::KindMap::ConstIterator i = kinds.begin();
		 i != kinds.end(); ++i) {
		MojRefCountedPtr<Quota> quota;
		MojErr err = quotaForKind(i->get(), quota);
		MojErrCheck(err);
		if (quota.get()) {
			MojInt64 usage = 0;
			err = kindUsage((*i)->id(), usage, txn);
			MojErrCheck(err);
			quota->offset(usage);
		}
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::quotaForKind(const MojDbKind* kind, MojRefCountedPtr<Quota>& quotaOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// prefer to use super's quota
	const MojDbKind::KindVec& supers = kind->supers();
	for (MojDbKind::KindVec::ConstIterator i = supers.begin(); i != supers.end(); ++i) {
		// don't want to inherit admin quota, so don't check root kind
		if ((*i)->id() != MojDbKindEngine::RootKindId) {
			MojErr err = quotaForKind(*i, quotaOut);
			MojErrCheck(err);
			if (quotaOut.get())
				return MojErrNone;
		}
	}
	// check for quota that matches owner of this kindS
	const MojString& owner = kind->owner();
	QuotaMap::ConstIterator iter = m_quotas.find(owner);
	if (iter == m_quotas.end()) {
		for (iter = m_quotas.begin(); iter != m_quotas.end(); ++iter) {
			if (matchWildcard(iter.key(), owner, owner.length()))
				break;
		}
	}
	if (iter != m_quotas.end()) {
		quotaOut = *iter;
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::applyOffset(const MojString& kindId, MojInt64 offset, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);

	// get old value
	MojRefCountedPtr<MojDbStorageItem> item;
	MojInt64 usage = 0;
	MojErr err = getUsage(kindId, txn, true, usage, item);
	MojErrCheck(err);
	if (item.get()) {
		MojInt64 newUsage = offset + usage;

		MojAssert(newUsage >= 0);
		MojObject newVal(newUsage);
		MojBuffer buf;
		err = newVal.toBytes(buf);
		MojErrCheck(err);
		// insert new record
		txn->quotaEnabled(false);
		err = m_usageDb->update(kindId, buf, item.get(), txn);
		MojErrCheck(err);
		txn->quotaEnabled(true);
	} else {
		err = insertUsage(kindId, offset, txn);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::getUsage(const MojString& kindId, MojDbStorageTxn* txn, bool forUpdate, MojInt64& usageOut, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);

	usageOut = 0;
	MojErr err = m_usageDb->get(kindId, txn, forUpdate, itemOut);
	MojErrCheck(err);
	if (itemOut.get()) {
		MojObject val;
		err = itemOut->toObject(val, *m_db->kindEngine(), false);
		MojErrCheck(err);
		usageOut = val.intValue();
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::initUsage(MojDbKind* kind, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(kind);

	MojRefCountedPtr<MojDbStorageItem> item;
	MojErr err = m_usageDb->get(kind->id(), req.txn(), false, item);
	MojErrCheck(err);
	if (!item.get()) {
		MojObject stats;
		MojSize usage = 0;
		err = kind->stats(stats, usage, req, false);
		MojErrCheck(err);
		err = insertUsage(kind->id(), (MojInt64) usage, req.txn());
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuotaEngine::insertUsage(const MojString& kindId, MojInt64 usage, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);
	MojAssert(usage >= 0);

	// serialize value
	MojObject val(usage);
	MojBuffer buf;
	MojErr err = val.toBytes(buf);
	MojErrCheck(err);
	// insert new record
	txn->quotaEnabled(false);
	err = m_usageDb->insert(kindId, buf, txn);
	MojErrCheck(err);
	txn->quotaEnabled(true);

	return MojErrNone;
}

MojErr MojDbQuotaEngine::commitQuota(const MojString& owner, MojInt64 size)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertWriteLocked(m_db->schemaLock());

	// get or create quota object
	QuotaMap::Iterator iter;
	MojErr err = m_quotas.find(owner, iter);
	MojErrCheck(err);
	if (iter == m_quotas.end()) {
		MojRefCountedPtr<Quota> quota(new Quota(size));
		MojAllocCheck(quota.get());
		err = m_quotas.put(owner, quota);
		MojErrCheck(err);
	} else {
		iter.value()->size(size);
	}
	if (m_isOpen) {
		err = refresh();
		MojErrCheck(err);
	}

	return MojErrNone;
}

MojInt64 MojDbQuotaEngine::Quota::size() const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	MojInt64 size = m_size;
	return size;
}

MojInt64 MojDbQuotaEngine::Quota::available() const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	MojInt64 available = m_size - m_usage;
	available = (available < 0) ? 0 : available;
	return available;
}

MojInt64 MojDbQuotaEngine::Quota::usage() const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	MojInt64 usage = m_usage;
	return usage;
}

void MojDbQuotaEngine::Quota::size(MojInt64 val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	m_size = val;
}

void MojDbQuotaEngine::Quota::usage(MojInt64 val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	m_usage = val;
}

void MojDbQuotaEngine::Quota::offset(MojInt64 off)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojThreadGuard guard(m_mutex);
	m_usage += off;
}

MojDbQuotaEngine::Offset::Offset(const MojString& kindId)
: m_kindId(kindId),
  m_offset(0)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}

MojErr MojDbQuotaEngine::Offset::apply(MojInt64 offset)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojInt64 newOffset = m_offset + offset;
	if (m_quota.get() && newOffset > m_quota->available())
		MojErrThrow(MojErrDbQuotaExceeded);
	m_offset = newOffset;

	return MojErrNone;
}

MojDbQuotaCommitHandler::MojDbQuotaCommitHandler(MojDbQuotaEngine* engine, MojString& owner, MojInt64 size, MojDbStorageTxn* txn)
: m_engine(engine),
  m_owner(owner),
  m_size(size),
  m_slot(this, &MojDbQuotaCommitHandler::handleCommit)
{
	txn->notifyPostCommit(m_slot);
}

MojErr MojDbQuotaCommitHandler::handleCommit(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = m_engine->commitQuota(m_owner, m_size);
	MojErrCheck(err);

	return MojErrNone;
}
