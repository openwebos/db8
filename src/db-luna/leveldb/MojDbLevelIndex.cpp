/* @@@LICENSE
*
*  Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include "db-luna/leveldb/MojDbLevelIndex.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#include "db-luna/leveldb/MojDbLevelCursor.h"
#include "db-luna/leveldb/MojDbLevelQuery.h"

////////////////////MojDbIndex////////////////////////////////////////////

MojDbLevelIndex::MojDbLevelIndex()
: m_primaryDb(NULL)
{
}

MojDbLevelIndex::~MojDbLevelIndex()
{
    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelIndex::open(const MojObject& id, MojDbLevelDatabase* db, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(db && db->engine());

    m_id = id;

    // this is deprecated
    // m_db and m_primaryDb should point to the same object
    // leave both of them here more for debugging purposes

    m_db.reset(db->engine()->indexDb());
    m_primaryDb.reset(db);

    return MojErrNone;
}

MojErr MojDbLevelIndex::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    m_db.reset();
    m_primaryDb.reset();

    return MojErrNone;
}

MojErr MojDbLevelIndex::drop(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojDbLevelCursor cursor;
    MojErr err = cursor.open(m_db.get(), txn, 0);
    MojErrCheck(err);
    MojDbKey prefix;
    err = prefix.assign(m_id);
    MojErrCheck(err);
    err = cursor.delPrefix(prefix);
    MojErrCheck(err);
    err = cursor.close();
    MojErrCheck(err);

    return err;
}

MojErr MojDbLevelIndex::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojDbLevelCursor cursor;
    MojErr err = cursor.open(m_db.get(), txn, 0);
    MojErrCheck(err);
    MojDbKey prefix;
    err = prefix.assign(m_id);
    MojErrCheck(err);
    err = cursor.statsPrefix(prefix, countOut, sizeOut);
    MojErrCheck(err);
    err = cursor.close();
    MojErrCheck(err);

    return err;
}

MojErr MojDbLevelIndex::insert(const MojDbKey& key, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(txn);

    MojDbLevelItem keyItem;
    keyItem.fromBytesNoCopy(key.data(), key.size());
    MojDbLevelItem valItem;  // empty item? not clear why do we need to insert it
    MojErr err = m_db->put(keyItem, valItem, txn, true);
#ifdef MOJ_DEBUG
    char s[1024];
    size_t size1 = keyItem.size();
    size_t size2 = valItem.size();
    MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
    LOG_DEBUG("[db_ldb] ldbindexinsert: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n",
        m_db->m_name.data(), size1, s, size2, err);
#endif
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelIndex::del(const MojDbKey& key, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(txn);
    MojAssert(isOpen());

    MojDbLevelItem keyItem;
    keyItem.fromBytesNoCopy(key.data(), key.size());

    bool found = false;
    MojErr err = m_db->del(keyItem, found, txn);

#ifdef MOJ_DEBUG
    char s[1024];
    size_t size1 = keyItem.size();
    MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
    LOG_DEBUG("[db_ldb] ldbindexdel: %s; keylen: %zu, key: %s ; err = %d\n", m_db->m_name.data(), size1, s, err);
    if (!found)
        LOG_WARNING(MSGID_LEVEL_DB_WARNING, 1,
            PMLOGKS("index", s),
            "ldbindexdel_warn: not found: %s", s);
#endif

    MojErrCheck(err);
    if (!found) {
        //MojErrThrow(MojErrDbInconsistentIndex);   // fix this to work around to deal with out of sync indexes
        MojErrThrow(MojErrInternalIndexOnDel);
    }
    return MojErrNone;
}

MojErr MojDbLevelIndex::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(isOpen());
    MojAssert(plan.get() && txn);

    MojRefCountedPtr<MojDbLevelQuery> storageQuery(new MojDbLevelQuery());
    MojAllocCheck(storageQuery.get());
    MojErr err = storageQuery->open(m_db.get(), m_primaryDb.get(), plan, txn);
    MojErrCheck(err);
    queryOut = storageQuery;

    return MojErrNone;
}

MojErr MojDbLevelIndex::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    return m_primaryDb->beginTxn(txnOut);
}
