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

#include "db-luna/leveldb/MojDbLevelDatabase.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#include "db-luna/leveldb/MojDbLevelCursor.h"
#include "db-luna/leveldb/MojDbLevelQuery.h"
#include "db-luna/leveldb/MojDbLevelTxn.h"
#include "db-luna/leveldb/MojDbLevelIndex.h"
#include "db/MojDb.h"
#include "db-luna/leveldb/defs.h"

////////////////////MojDbLevelDatabase////////////////////////////////////////////

MojDbLevelDatabase::MojDbLevelDatabase()
: m_db(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelDatabase::~MojDbLevelDatabase()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelDatabase::open(const MojChar* dbName, MojDbLevelEngine* eng, bool& createdOut, MojDbStorageTxn* txn)
{
    MojAssert(dbName && eng);
    MojAssert(!txn || txn->isValid());
    MojLogTrace(MojDbLevelEngine::s_log);

    // save eng, name and file
    createdOut = false;
    m_engine = eng;
    MojErr err = m_name.assign(dbName);
    MojErrCheck(err);
    const MojString& engPath = eng->path();
    if (engPath.empty()) {
        err = m_file.assign(dbName);
        MojErrCheck(err);
    } else {
        err = m_file.format(_T("%s/%s"), engPath.data(), dbName);
        MojErrCheck(err);
    }

    // create and open db
    leveldb::Options options;
    options.create_if_missing = true;
    options.paranoid_checks = true;
    leveldb::Status status = leveldb::DB::Open(options, m_file.data(), &m_db);

    if (status.IsCorruption()) {    // database corrupted
        // try restore database
        // AHTUNG! After restore database can lost some data!
        status = leveldb::RepairDB(m_file.data(), options);
        MojLdbErrCheck(status, _T("db corrupted"));
        status = leveldb::DB::Open(options, m_file.data(), &m_db);  // database restored, re-open
    }

    MojLdbErrCheck(status, _T("db_create"));
    MojAssert(m_db);

    // set up prop-vec for primary key queries
    MojString idStr;
    err = idStr.assign(MojDb::IdKey);
    MojErrCheck(err);
    err = m_primaryProps.push(idStr);
    MojErrCheck(err);

    //keep a reference to this database
    err = eng->addDatabase(this);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelDatabase::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err = MojErrNone;
    if (m_db) {
        err = closeImpl();
        m_primaryProps.clear();
        engine()->removeDatabase(this);
    }
    return err;
}

MojErr MojDbLevelDatabase::drop(MojDbStorageTxn* txn)
{
    // TODO: implement this
    return MojErrNone;
}

// not supported in levelDB
MojErr MojDbLevelDatabase::mutexStats(int * total_mutexes, int * mutexes_free, int * mutexes_used,
     int * mutexes_used_highwater, int * mutexes_regionsize)
{
    MojAssert(m_engine);
    MojLogTrace(MojDbLevelEngine::s_log);

    if (total_mutexes)
        *total_mutexes = 0;

    if (mutexes_free)
        *mutexes_free = 0;
    if (mutexes_used)
        *mutexes_used = 0;
    if (mutexes_used_highwater)
        *mutexes_used_highwater = 0;
    if (mutexes_regionsize)
        *mutexes_regionsize = 0;

    return MojErrNone;
}

MojErr MojDbLevelDatabase::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojDbLevelCursor cursor;
    MojErr err = cursor.open(this, txn, 0);
    MojErrCheck(err);
    err = cursor.stats(countOut, sizeOut);
    MojErrCheck(err);
    err = cursor.close();
    MojErrCheck(err);

    return err;
}

MojErr MojDbLevelDatabase::insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn)
{
    MojAssert(txn);

    MojErr err = put(id, val, txn, true);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelDatabase::update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn)
{
    MojAssert(oldVal && txn);

    MojErr err = txn->offsetQuota(-(MojInt64) oldVal->size());
    MojErrCheck(err);

    // add/replace with a new one
    err = put(id, val, txn, false);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelDatabase::del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojDbLevelItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    err = del(idItem, foundOut, txn);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelDatabase::get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    // stop here
    itemOut.reset();
    MojDbLevelItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    MojRefCountedPtr<MojDbLevelItem> valItem(new MojDbLevelItem);
    MojAllocCheck(valItem.get());
    bool found = false;
    err = get(idItem, txn, forUpdate, *valItem, found);
    MojErrCheck(err);
    if (found) {
        valItem->id(id);
        itemOut = valItem;
    }
    return MojErrNone;
}

MojErr MojDbLevelDatabase::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
    MojAssert(m_db);
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelQuery> storageQuery(new MojDbLevelQuery);
    MojAllocCheck(storageQuery.get());
    MojErr err = storageQuery->open(this, NULL, plan, txn);
    MojErrCheck(err);
    queryOut = storageQuery;

    return MojErrNone;
}

MojErr MojDbLevelDatabase::put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojDbLevelItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    MojDbLevelItem valItem;
    err = valItem.fromBuffer(val);
    MojErrCheck(err);
    err = put(idItem, valItem, txn, updateIdQuota);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelDatabase::put(MojDbLevelItem& key, MojDbLevelItem& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
    MojAssert(m_db );
    MojAssert( !txn || dynamic_cast<MojDbLevelAbstractTxn *> (txn) );
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err;
    if (txn)
    {
        MojInt64 quotaOffset = val.size();
        if (updateIdQuota)
            quotaOffset += key.size();
        err = txn->offsetQuota(quotaOffset);
        MojErrCheck(err);
    }

    MojDbLevelAbstractTxn * leveldb_txn = static_cast<MojDbLevelAbstractTxn *> (txn);

    leveldb::Status s;

    if(leveldb_txn)
    {
        leveldb_txn->tableTxn(impl()).Put(*key.impl(), *val.impl());
    }
    else
        s = m_db->Put(leveldb::WriteOptions(), *key.impl(), *val.impl());

#if defined(MOJ_DEBUG)
    char str_buf[1024];
    size_t size1 = key.size();
    size_t size2 = val.size();
    MojErr err2 = MojByteArrayToHex(key.data(), size1, str_buf);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(str_buf, (char *)(key.data()) + (size1 - 17), 16);
    MojLogDebug(MojDbLevelEngine::s_log, _T("ldb put: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n"),
                    this->m_name.data(), size1, str_buf, size2, err);
#endif

    if(leveldb_txn)
        ;//MojLdbErrCheck(batch->status(), _T("db->put"));
    else
        MojLdbErrCheck(s, _T("db->put"));


    postUpdate(txn, key.size() + val.size());

    return MojErrNone;
}

MojErr MojDbLevelDatabase::get(MojDbLevelItem& key, MojDbStorageTxn* txn, bool forUpdate,
                               MojDbLevelItem& valOut, bool& foundOut)
{
    MojAssert(m_db);
    MojAssert( !txn || dynamic_cast<MojDbLevelAbstractTxn *> (txn) );
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    std::string str;

    MojDbLevelAbstractTxn * leveldb_txn = static_cast<MojDbLevelAbstractTxn *> (txn);

    leveldb::Status s;
    if (leveldb_txn)
        s = leveldb_txn->tableTxn(impl()).Get(*key.impl(), str);
    else
        s = m_db->Get(leveldb::ReadOptions(), *key.impl(), &str);

    //MojLdbErrCheck(s, _T("db->get"));

    if(s.IsNotFound() == false)
    {
        foundOut = true;
        valOut.fromBytes(reinterpret_cast<const MojByte*>(str.data()), str.size());
    }

    return MojErrNone;
}

MojErr MojDbLevelDatabase::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
   MojRefCountedPtr<MojDbLevelEnvTxn> txn(new MojDbLevelEnvTxn());
   MojAllocCheck(txn.get());

   // force TableTxn for this database to start
   txn->tableTxn(impl()).begin(impl());

   txnOut = txn;
   return MojErrNone;

}

MojErr MojDbLevelDatabase::openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut)
{
    MojAssert(!indexOut.get());
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelIndex> index(new MojDbLevelIndex());
    MojAllocCheck(index.get());
    MojErr err = index->open(id, this, txn);
    MojErrCheck(err);
    indexOut = index;

    return MojErrNone;
}

MojErr MojDbLevelDatabase::del(MojDbLevelItem& key, bool& foundOut, MojDbStorageTxn* txn)
{

    MojAssert(m_db);
    MojAssert( !txn || dynamic_cast<MojDbLevelAbstractTxn *> (txn) );
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    MojErr err = txn->offsetQuota(-(MojInt64) key.size());
    MojErrCheck(err);

    MojDbLevelAbstractTxn * leveldb_txn = static_cast<MojDbLevelAbstractTxn *> (txn);

    leveldb::Status st;

    if(leveldb_txn)
    {
        leveldb_txn->tableTxn(impl()).Delete(*key.impl());
    }
    else
        st = m_db->Delete(leveldb::WriteOptions(), *key.impl());

#if defined(MOJ_DEBUG)
    char str_buf[1024];     // big enough for any key
    size_t size = key.size();
    MojErr err2 = MojByteArrayToHex(key.data(), size, str_buf);
    MojErrCheck(err2);
    if (size > 16)  // if the object-id is in key
        strncat(str_buf, (char *)(key.data()) + (size - 17), 16);
    MojLogDebug(MojDbLevelEngine::s_log, _T("ldbdel: %s; keylen: %zu, key= %s; err = %d \n"), this->m_name.data(), size, str_buf, !st.ok());
#endif

    if (st.IsNotFound() == false) {
        MojLdbErrCheck(st, _T("db->del"));
        foundOut = true;
    }
    postUpdate(txn, key.size());

    return MojErrNone;
}

MojErr MojDbLevelDatabase::closeImpl()
{
    if (m_db) {
        delete m_db;
        m_db = NULL;
    }

    return MojErrNone;
}

void MojDbLevelDatabase::postUpdate(MojDbStorageTxn* txn, MojSize size)
{
    if (txn) {
        // TODO: implement quotas
        // XXX: static_cast<MojDbLevelTxn*>(txn)->didUpdate(size);
    }
}
