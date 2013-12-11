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


#include "db/MojDbCursor.h"
#include "core/MojObjectBuilder.h"
#include "db/MojDb.h"
#include "db/MojDbIndex.h"

MojDbCursor::MojDbCursor()
: m_ownTxn(true),
  m_lastErr(MojErrNone),
  m_kindEngine(NULL),
  m_dbIndex(NULL)
{
}

MojDbCursor::~MojDbCursor()
{
	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDbCursor::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;
	if (m_storageQuery.get()) {
		if (m_watcher.get()) {
			errClose = m_watcher->activate(m_storageQuery->endKey());
			MojErrAccumulate(err, errClose);
		}
		errClose = m_storageQuery->close();
		MojErrAccumulate(err, errClose);
	}
	if (m_txn.get() && m_ownTxn) {
		if (m_lastErr == MojErrNone) {
			errClose = m_txn->commit();
			MojErrAccumulate(err, errClose);
		} else {
			if (m_lastErr == MojErrInternalIndexOnFind)
                LOG_WARNING(MSGID_MOJ_DB_CURSOR_WARNING, 1,
                    PMLOGFV("code", "%d", (int)m_lastErr),
                    "dbcursor_close: IndexFind Warning - abort tran; code = 'code'");
			errClose = m_txn->abort();
			MojErrAccumulate(err, errClose);
		}
	}
	m_lastErr = MojErrNone;

	m_storageQuery.reset();
    m_txn.reset();

	m_objectFilter.reset();
	m_watcher.reset();
	m_query.clear();
    m_dbIndex = NULL;

	return err;
}

MojErr MojDbCursor::get(MojDbStorageItem*& itemOut, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	foundOut = false;
	if (!m_storageQuery.get())
		MojErrThrow(MojErrNotOpen);
	m_storageQuery->verify(m_vmode);
	MojErr err = m_storageQuery->get(itemOut, foundOut);
	if (err == MojErrInternalIndexOnFind) {
		LOG_DEBUG("[db_mojodb] dbcursor_get1: IndexFind Warning; code = %d\n", (int)err);
	} 
	else
		MojErrAccumulate(m_lastErr, err);
	MojErrCheck(err);
    LOG_DEBUG("[db_mojodb] dbcursor_get_item: found: %d\n", (int)foundOut);
	return MojErrNone;
}

MojErr MojDbCursor::get(MojObject& objOut, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	foundOut = false;
	if (!m_storageQuery.get())
		MojErrThrow(MojErrNotOpen);

	MojObjectBuilder builder;
	MojErr err = visitObject(builder, foundOut);
	MojErrCheck(err);
	objOut = builder.object();
    LOG_DEBUG("[db_mojodb] dbcursor_get_obj: found: %d\n", (int)foundOut);
	return MojErrNone;
}

MojErr MojDbCursor::visit(MojObjectVisitor& visitor)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (!m_storageQuery.get())
		MojErrThrow(MojErrNotOpen);
	int i = 0;
	bool found = false;
	do {
		MojErr err = visitObject(visitor, found);
		if (err == MojErrInternalIndexOnFind) {
			LOG_DEBUG("[db_mojodb] dbcursor_visit indexwarn: %s; Index: %s \n", this->query().from().data(),
				((m_dbIndex) ? m_dbIndex->name().data() : NULL));
			MojErrAccumulate(m_lastErr, MojErrNone); // we need to clear the error so it wont bubble up
			found = true;  // to continue the loop
			continue;
		}
		MojErrCheck(err);
		i++;
	} while (found);
    LOG_DEBUG("[db_mojodb] dbcursor_visit: count: %d\n", (int)i);
	return MojErrNone;
}

MojErr MojDbCursor::count(MojUInt32& countOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	countOut = 0;
	if (!m_storageQuery.get())
		MojErrThrow(MojErrNotOpen);

	MojErr err = m_storageQuery->count(countOut);
	MojErrAccumulate(m_lastErr, err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCursor::nextPage(MojDbQuery::Page& pageOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (!m_storageQuery.get())
		MojErrThrow(MojErrNotOpen);

	MojErr err = m_storageQuery->nextPage(pageOut);
	MojErrAccumulate(m_lastErr, err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCursor::init(const MojDbQuery& query)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = query.validateFind();
	MojErrCheck(err);
	err = initImpl(query);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbCursor::initImpl(const MojDbQuery& query)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (!query.select().empty()) {
		m_objectFilter.reset(new MojObjectFilter);
		MojAllocCheck(m_objectFilter.get());
		MojErr err = m_objectFilter->init(query.select());
		MojErrCheck(err);
	}
	if (!query.filter().empty()) {
		m_queryFilter.reset(new MojDbQueryFilter);
		MojAllocCheck(m_queryFilter.get());
		MojErr err = m_queryFilter->init(query);
		MojErrCheck(err);
	}
	m_query = query;

	return MojErrNone;
}

MojErr MojDbCursor::visitObject(MojObjectVisitor& visitor, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojDbStorageItem* item = NULL;
	MojErr err = get(item, foundOut);
	MojErrAccumulate(m_lastErr, err);
	MojErrCheck(err);

	if (foundOut) {
		if (m_objectFilter.get() != NULL) {
			m_objectFilter->setVisitor(&visitor);
			err = item->visit(*(m_objectFilter.get()), *m_kindEngine);
			MojErrAccumulate(m_lastErr, err);
			MojErrCheck(err);
		} else {
			err = item->visit(visitor, *m_kindEngine);
			MojErrAccumulate(m_lastErr, err);
			MojErrCheck(err);
		}
	}

    LOG_DEBUG("[db_mojodb] dbcursor_visitObject: found: %d\n", (int)foundOut);
	return MojErrNone;
}

void MojDbCursor::txn(MojDbStorageTxn* txn, bool ownTxn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojAssert(!m_txn.get() && txn);
	m_txn = txn;
	m_ownTxn = ownTxn;
}

void MojDbCursor::excludeKinds(const MojSet<MojString>& toExclude)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojAssert(isOpen());
	m_storageQuery->excludeKinds(toExclude);
}
