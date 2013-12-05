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


#include "db/MojDbReq.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDb.h"

MojDbReq::MojDbReq(bool admin)
: m_beginCount(0),
  m_db(NULL),
  m_admin(admin),
  m_batch(false),
  m_schemaLocked(false),
  m_vmode(false),
  m_fixmode(false),
  m_batchSize(0),
  m_autobatch(false)

{
}

MojDbReq::~MojDbReq()
{
	unlock();
}

MojErr MojDbReq::domain(const MojChar* val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = m_domain.assign(val);
	MojErrCheck(err);
	MojSize pos = m_domain.find(_T(' '));
	if (pos != MojInvalidSize) {
		err = m_domain.truncate(pos);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbReq::abort()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	unlock();
	m_beginCount = 0;
	m_batch = false;
	if (m_txn.get()) {
		MojErr err = m_txn->abort();
		m_txn.reset();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbReq::curKind(const MojDbKind* kind)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return m_db->quotaEngine()->curKind(kind, m_txn.get());
}

MojErr MojDbReq::begin(MojDb* db, bool lockSchema)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(db);

	if (m_beginCount++ == 0) {
		lock(db, lockSchema);

		if (!m_txn.get()) {
			MojErr err = db->storageEngine()->beginTxn(m_txn);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

// start another transaction with the same criteria. Assumes, a transaction was committed earlier. Use with caution: cannot be used for
// operations requiring schema lock. A quick implementation to test the model.

MojErr MojDbReq::startanother(MojDb* db)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(db);

	MojErr err = commit();
	MojErrCheck(err);

	lock(db, false);	// schema lock is set to false.

	if (!m_txn.get()) {
		MojErr err = db->storageEngine()->beginTxn(m_txn);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbReq::end(bool commitNow)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (--m_beginCount == 0 && !m_batch) {
		if (commitNow) {
			MojErr err = commit();
			MojErrCheck(err);
		} else {
			m_txn.reset();
			unlock();
		}
	}
	return MojErrNone;
}

void MojDbReq::beginBatch()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	m_batch = true;
	m_beginCount = 0;
	m_txn.reset();
}

MojErr MojDbReq::endBatch()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	m_batch = false;
	MojErr err = commit();
	MojErrCheck(err);

	return MojErrNone;
}

void MojDbReq::lock(MojDb* db, bool lockSchema)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(m_db == NULL || m_db == db);

	if (m_db == NULL) {
		m_db = db;
		if (lockSchema) {
			m_schemaLocked = true;
			db->writeLock();
		} else {
			db->readLock();
		}
	} else {
		// if we want to lock the schema and already have a lock, it must be a schema lock
		MojAssert(!lockSchema || m_schemaLocked);
	}
}

void MojDbReq::unlock()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (m_db) {
		m_db->unlock();
		m_db = NULL;
		m_schemaLocked = false;
	}
}

MojErr MojDbReq::commit()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	if (m_txn.get()) {
		MojErr errAcc = m_txn->commit();
		MojErrAccumulate(err, errAcc);
		m_txn.reset();
	}
	unlock();
	return err;
}

MojDbAdminGuard::MojDbAdminGuard(MojDbReq& req, bool setPrivilege)
: m_req(req),
  m_oldPrivilege(req.admin())
{
	if(setPrivilege) {
		set();
	}
}
