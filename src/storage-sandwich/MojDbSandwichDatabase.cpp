/* @@@LICENSE
*
*  Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include "MojDbSandwichDatabase.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichQuery.h"
#include "MojDbSandwichTxn.h"
#include "MojDbSandwichIndex.h"
#include "db/MojDb.h"
#include "defs.h"

#include "MojDbSandwichLazyUpdater.h"

////////////////////MojDbSandwichDatabase////////////////////////////////////////////

MojDbSandwichDatabase::MojDbSandwichDatabase(const MojDbSandwichEngine::BackendDb::Part& part)
    : m_db(part)
{
}

MojDbSandwichDatabase::~MojDbSandwichDatabase()
{
    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbSandwichDatabase::open(const MojChar* dbName, MojDbSandwichEngine* eng)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(dbName && eng);

    // save eng and name
    m_engine = eng;
    MojErr err = m_name.assign(dbName);
    MojErrCheck(err);

    if (engine()->lazySync())
        engine()->getUpdater()->open( getDb() );

    return MojErrNone;
}

leveldb::DB* MojDbSandwichDatabase::getDb()
{
    MojDbSandwichEngine::BackendDb& backendDb = engine()->impl();
    leveldb::DB* db = (*backendDb).get();

    return db;
}

MojErr MojDbSandwichDatabase::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err = MojErrNone;
    if (m_db.Valid()) {
        err = closeImpl();
        if (engine()->lazySync())
            engine()->getUpdater()->close( getDb() );
    }
    return err;
}

MojErr MojDbSandwichDatabase::drop(MojDbStorageTxn* txn)
{
    // TODO: implement this
    MojErrThrow(MojErrNotImplemented);
}

// not supported in levelDB
MojErr MojDbSandwichDatabase::mutexStats(int * total_mutexes, int * mutexes_free, int * mutexes_used,
     int * mutexes_used_highwater, int * mutexes_regionsize)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(m_engine);

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

MojErr MojDbSandwichDatabase::stats(MojDbStorageTxn* abstractTxn, MojSize& countOut, MojSize& sizeOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( dynamic_cast<MojDbSandwichEnvTxn *>(abstractTxn) );

    auto txn = static_cast<MojDbSandwichEnvTxn *>(abstractTxn);
    return stats(txn, countOut, sizeOut, {});
}

MojErr MojDbSandwichDatabase::insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(txn);

    MojErr err = put(id, val, txn, true);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(oldVal && txn);

    MojErr err = txn->offsetQuota(-(MojInt64) oldVal->size());
    MojErrCheck(err);

    // add/replace with a new one
    err = put(id, val, txn, false);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojDbSandwichItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    err = del(idItem, foundOut, txn);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    // stop here
    itemOut.reset();
    MojDbSandwichItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    MojRefCountedPtr<MojDbSandwichItem> valItem(new MojDbSandwichItem);
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

MojErr MojDbSandwichDatabase::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //MojAssert(m_db);

    MojRefCountedPtr<MojDbSandwichQuery> storageQuery(new MojDbSandwichQuery);
    MojAllocCheck(storageQuery.get());
    MojErr err = storageQuery->open(this, NULL, plan, txn);
    MojErrCheck(err);
    queryOut = storageQuery;

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojDbSandwichItem idItem;
    MojErr err = idItem.fromObject(id);
    MojErrCheck(err);
    MojDbSandwichItem valItem;
    err = valItem.fromBuffer(val);
    MojErrCheck(err);
    err = put(idItem, valItem, txn, updateIdQuota);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::put(MojDbSandwichItem& key, MojDbSandwichItem& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(m_db.Valid() );
    MojAssert( !txn || dynamic_cast<MojDbSandwichEnvTxn *> (txn) );

    MojErr err;
    if (txn)
    {
        MojInt64 quotaOffset = val.size();
        if (updateIdQuota)
            quotaOffset += key.size();
        err = txn->offsetQuota(quotaOffset);
        MojErrCheck(err);
    }

    MojDbSandwichEnvTxn * leveldb_txn = static_cast<MojDbSandwichEnvTxn *> (txn);

    leveldb::Status s;

    if(leveldb_txn)
    {
        s = leveldb_txn->ref(impl()).Put(*key.impl(), *val.impl());
    }
    else
        s = m_db.Put(*key.impl(), *val.impl());

#if defined(MOJ_DEBUG)
    char str_buf[1024];
    size_t size1 = key.size();
    size_t size2 = val.size();
    MojErr err2 = MojByteArrayToHex(key.data(), size1, str_buf);
    MojErrCheck(err2);
    if (size1 > 16) // if the object-id is in key
        strncat(str_buf, (char *)(key.data()) + (size1 - 17), 16);
    LOG_DEBUG("[db_ldb] ldb put: %s; keylen: %zu, key: %s ; vallen = %zu; err = %s\n",
        this->m_name.data(), size1, str_buf, size2, s.ToString().c_str());
#endif

    /*
    if(leveldb_txn)
        ;//MojLdbErrCheck(batch->status(), _T("db->put"));
    else
        */
        MojLdbErrCheck(s, _T("db->put"));


    postUpdate(txn, key.size() + val.size());

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::get(MojDbSandwichItem& key, MojDbStorageTxn* txn, bool forUpdate,
                               MojDbSandwichItem& valOut, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //MojAssert(m_db);
    MojAssert( !txn || dynamic_cast<MojDbSandwichEnvTxn *> (txn) );

    foundOut = false;
    std::string str;

    MojDbSandwichEnvTxn * leveldb_txn = static_cast<MojDbSandwichEnvTxn *> (txn);

    leveldb::Status s;
    if (leveldb_txn)
        s = leveldb_txn->ref(impl()).Get(*key.impl(), str);
    else
        s = m_db.Get(*key.impl(), str);

    if (s.IsNotFound()) return MojErrNone; // normal case

    MojLdbErrCheck(s, _T("db->get"));

    foundOut = true;
    valOut.fromBytes(reinterpret_cast<const MojByte*>(str.data()), str.size());

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //MojAssert( m_db );
   MojRefCountedPtr<MojDbSandwichEnvTxn> txn(new MojDbSandwichEnvTxn(m_engine->impl(), *engine()));
   MojAllocCheck(txn.get());

   txnOut = txn;
   return MojErrNone;

}

MojErr MojDbSandwichDatabase::openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(!indexOut.get());

    MojRefCountedPtr<MojDbSandwichIndex> index(new MojDbSandwichIndex());
    MojAllocCheck(index.get());
    MojErr err = index->open(id, this, txn);
    MojErrCheck(err);
    indexOut = index;

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::del(MojDbSandwichItem& key, bool& foundOut, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //MojAssert(m_db);
    MojAssert( !txn || dynamic_cast<MojDbSandwichEnvTxn *> (txn) );

    foundOut = false;
    MojErr err = txn->offsetQuota(-(MojInt64) key.size());
    MojErrCheck(err);

    MojDbSandwichEnvTxn * leveldb_txn = static_cast<MojDbSandwichEnvTxn *> (txn);

    leveldb::Status st;

    if(leveldb_txn)
    {
        leveldb_txn->ref(impl()).Delete(*key.impl());
    }
    else
        st = m_db.Delete(*key.impl());

#if defined(MOJ_DEBUG)
    char str_buf[1024];     // big enough for any key
    size_t size = key.size();
    MojErr err2 = MojByteArrayToHex(key.data(), size, str_buf);
    MojErrCheck(err2);
    if (size > 16)  // if the object-id is in key
        strncat(str_buf, (char *)(key.data()) + (size - 17), 16);
    LOG_DEBUG("[db_ldb] ldbdel: %s; keylen: %zu, key= %s; err = %s\n", this->m_name.data(), size, str_buf, st.ToString().c_str());
#endif

    if (st.IsNotFound() == false) {
        MojLdbErrCheck(st, _T("db->del"));
        foundOut = true;
    }
    postUpdate(txn, key.size());

    return MojErrNone;
}

MojErr MojDbSandwichDatabase::delPrefix(MojDbSandwichEnvTxn &txn, leveldb::Slice prefix)
{
    auto part = txn.ref(m_db);
    auto it = part.NewIterator();

    it->Seek(prefix);
    while (it->Valid() && it->key().starts_with(prefix))
    {
        auto key = it->key();

        size_t delSize = key.size() + it->value().size();
        MojErr err = txn.offsetQuota(-(MojInt64) delSize);
        MojErrCheck(err);

        auto s = part.Delete(key);
        MojLdbErrCheck(s, _T("db->delPrefix"));

        it->Next(); // skip this ghost record
    }
    return MojErrNone;
}

MojErr MojDbSandwichDatabase::stats(MojDbSandwichEnvTxn* txn, MojSize &countOut, MojSize &sizeOut, leveldb::Slice prefix)
{
    MojAssert( txn );
    MojDbSandwichEnvTxn::BackendDb::Part txnPart;
    leveldb::AnyDB *db; // points either to txn or to database part

    if (txn)
    {
        txnPart = txn->ref(m_db);
        db = &txnPart;
    }
    else
    {
        db = &m_db;
    }

    auto it = db->NewIterator();

    MojSize count = 0, size = 0;

    it->Seek(prefix);
    while (it->Valid() && it->key().starts_with(prefix))
    {
        ++count;
        size += it->key().size() + it->value().size();
        it->Next();
    }
    sizeOut = size;
    countOut = count;
    return MojErrNone;
}

void MojDbSandwichDatabase::compact()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //MojAssert(m_db);

    //m_db->CompactRange(NULL, NULL);
    //m_engine->m_sdb->CompactRange(NULL, NULL); // TODO: compact range for Part
}

MojErr MojDbSandwichDatabase::closeImpl()
{
//    if (m_db) {
//        delete m_db;
//        m_db = NULL;
//    }

    return MojErrNone;
}

void MojDbSandwichDatabase::postUpdate(MojDbStorageTxn* txn, MojSize size)
{
    if (txn) {
        // TODO: implement quotas
        // XXX: static_cast<MojDbSandwichTxn*>(txn)->didUpdate(size);
    } else {
        if (engine()->lazySync())
            engine()->getUpdater()->sendEvent( getDb() );
    }
}

