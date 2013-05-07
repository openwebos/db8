/* @@@LICENSE
*
*  Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*  Copyright (c) 2013 LG Electronics
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

#include "db-luna/MojDbLevelEngine.h"
#include "db-luna/MojDbLevelFactory.h"
#include "db-luna/MojDbLevelQuery.h"
#include "db/MojDbObjectHeader.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojString.h"
#include "core/MojTokenSet.h"

#include <sys/statvfs.h>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "leveldb/iterator.h"

//const MojChar* const MojDbLevelEnv::LockFileName = _T("_lock");
MojLogger MojDbLevelEngine::s_log(_T("db.ldb"));
static const MojChar* const MojEnvIndexDbName = _T("indexes.ldb");
static const MojChar* const MojEnvSeqDbName = _T("seq.ldb");

const MojChar* const MojDbLevelEnv::LockFileName = _T("_lock");


MojDbLevelCursor::MojDbLevelCursor()
   : m_it(NULL), m_txn(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelCursor::~MojDbLevelCursor()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelCursor::open(MojDbLevelDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags)
{
    MojAssert(db && txn);
    MojAssert(!m_it);
    MojLogTrace(MojDbLevelEngine::s_log);

    m_db = db->impl();
    m_it = m_db->NewIterator(leveldb::ReadOptions());
    m_txn = txn;
    MojLdbErrCheck(m_it->status(), _T("db->cursor"));
    m_warnCount = 0;

    return MojErrNone;
}

MojErr MojDbLevelCursor::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_it) {
        delete m_it;
        m_it = NULL;
    }
    return MojErrNone;
}

MojErr MojDbLevelCursor::del()
{
    MojAssert(m_it);
    MojLogTrace(MojDbLevelEngine::s_log);

    if(m_it->Valid())
    {
        leveldb::Slice key = m_it->value();
        //m_it->Next(); - not clear if we need it here
        if(m_txn)
        {
            MojDbLevelTxn * txn = dynamic_cast<MojDbLevelTxn *> (m_txn);
            if(txn)
                txn->impl()->Delete(key);
            else
                MojAssert(0);
        }
        else
        {
            m_db->Delete(leveldb::WriteOptions(), key);
        }
    }

    MojErr err = m_txn->offsetQuota(-(MojInt64) m_recSize);
    MojErrCheck(err);

    return MojErrNone;
}


MojErr MojDbLevelCursor::delPrefix(const MojDbKey& prefix)
{
    MojAssert(m_it);
    MojLogTrace(MojDbLevelEngine::s_log);
    //MojDbLevelItem val;
    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);
    leveldb::Iterator * it = m_it;

    it->SeekToFirst();
    it->Seek(*key.impl());
    while (it->Valid() && it->key().starts_with(*key.impl()))
    {
        err = del();
        MojErrCheck(err);
        MojLdbErrCheck(it->status(), _T("lbb->delPrefix"));
        it->Next();
    }
    return MojErrNone;
}

MojErr MojDbLevelCursor::get(MojDbLevelItem& key, MojDbLevelItem& val, bool& foundOut, MojUInt32 flags)
{
    MojAssert(m_it);
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    switch (flags)
    {
    case e_Set:
        m_it->Seek(*key.impl());
    break;
    case e_Next:
        m_it->Next();
    break;
    case e_Prev:
        m_it->Prev();
    break;
    case e_First:
        m_it->SeekToFirst();
    break;
    case e_Last:
        m_it->SeekToLast();
    break;
    default:
    break;
    }

    if(m_it->Valid() )
    {
        foundOut = true;
        *(val.impl()) = m_it->value();
        *(key.impl()) = m_it->key();
        m_recSize = key.size() + val.size();
    }

    MojLdbErrCheck(m_it->status(), _T("lbb->get"));
    return MojErrNone;
}

MojErr MojDbLevelCursor::stats(MojSize& countOut, MojSize& sizeOut)
{

    MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbcursor_stats: count: %d, size: %d, err: %d"), (int)countOut, (int)sizeOut, (int)err);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelCursor::statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut)
{
    countOut = 0;
    sizeOut = 0;
    m_warnCount = 0;    // debug
    MojDbLevelItem val;
    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    m_it->SeekToFirst();

    MojSize count = 0;
    MojSize size = 0;
    MojErrCheck(err);

    for (m_it->Seek(*key.impl()); m_it->Valid(); m_it->Next())
    {
        if(m_it->key().starts_with(*key.impl()) == false)
            break;
        ++count;
        // debug code for verifying index keys
        size += m_it->key().size() + m_it->value().size();
        m_it->Next();
    }
    sizeOut = size;
    countOut = count;

    return MojErrNone;
}


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
    leveldb::Status status = leveldb::DB::Open(options, m_file.data(), &m_db);

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

    if (!txn->isValid()) {
        MojLogInfo(MojDbLevelEngine::s_log, "Transaction is not valid, reinit transaction");
        err = this->beginTxn(*eng->getPostTransaction());
        MojErrCheck(err);
    }

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
    // first delete from DB
    // not clear if there's any usage for foundOut
    // maybe for debugging?
    bool foundOut = false;
    err = del(id, txn, foundOut);
    MojErrCheck(err);
    // add a new one
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
    MojLogTrace(MojDbLevelEngine::s_log);

    MojInt64 quotaOffset = val.size();
    if (updateIdQuota)
        quotaOffset += key.size();
    MojErr err = txn->offsetQuota(quotaOffset);
    MojErrCheck(err);
    leveldb::WriteBatch *batch = MojLdbTxnFromStorageTxn(txn);
    leveldb::Status s;

    if(batch)
    {
        batch->Put(*key.impl(), *val.impl());
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
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldb put: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n"),
                    this->m_name.data(), size1, str_buf, size2, err);
#endif

    if(batch)
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
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    std::string str;

    leveldb::Status s = m_db->Get(leveldb::ReadOptions(), *key.impl(), &str);

    //MojLdbErrCheck(s, _T("db->get"));

    if(s.IsNotFound() == false)
    {
        foundOut = true;
        *valOut.impl() = leveldb::Slice(str);
    }

    return MojErrNone;
}

MojErr MojDbLevelDatabase::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
   MojRefCountedPtr<MojDbLevelTxn> txn(new MojDbLevelTxn());

   MojAllocCheck(txn.get());
   MojErr err = txn->begin(this);
   MojErrCheck(err);
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
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    MojErr err = txn->offsetQuota(-(MojInt64) key.size());
    MojErrCheck(err);
    leveldb::WriteBatch * dbBatch = MojLdbTxnFromStorageTxn(txn);
    leveldb::Status st;

    if(dbBatch)
    {
        dbBatch->Delete(*key.impl());
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
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbdel: %s; keylen: %zu, key= %s; err = %d \n"), this->m_name.data(), size, str_buf, !st.ok());
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
        static_cast<MojDbLevelTxn*>(txn)->didUpdate(size);
    }
}

////////////////////MojDbLevelEnv////////////////////////////////////////////

// this class is mostly placeholder
MojDbLevelEnv::MojDbLevelEnv()
 : m_db(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}


MojDbLevelEnv::~MojDbLevelEnv()
{
    MojLogTrace(MojDbLevelEngine::s_log);
    close();
}

MojErr MojDbLevelEnv::configure(const MojObject& conf)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    bool found = false;
    MojString logDir;
    MojErr err = conf.get(_T("logDir"), logDir, found);
    MojErrCheck(err);

    m_logDir = logDir;

    return MojErrNone;
}

MojErr MojDbLevelEnv::open(const MojChar* dir)
{
    lockDir(dir);

    return MojErrNone;
}

MojErr MojDbLevelEnv::close()
{
    unlockDir();

    /*if (m_db)
        delete m_db;*/

    return MojErrNone;
}

MojErr MojDbLevelEnv::lockDir(const MojChar* path)
{
    MojAssert(path);

    MojErr err = MojCreateDirIfNotPresent(path);
    MojErrCheck(err);
    err = m_lockFileName.format(_T("%s/%s"), path, LockFileName);
    MojErrCheck(err);
    err = m_lockFile.open(m_lockFileName, MOJ_O_RDWR | MOJ_O_CREAT, MOJ_S_IRUSR | MOJ_S_IWUSR);
    MojErrCheck(err);
    err = m_lockFile.lock(LOCK_EX | LOCK_NB);
    MojErrCatch(err, MojErrWouldBlock) {
        (void) m_lockFile.close();
        MojErrThrowMsg(MojErrLocked, _T("Database at '%s' locked by another instance."), path);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelEnv::unlockDir()
{
    MojErr err = MojErrNone;
    if (m_lockFile.open()) {
            // unlink before we close to ensure that we hold
        // the lock to the bitter end
        MojErr errClose = MojUnlink(m_lockFileName);
        MojErrAccumulate(err, errClose);
        errClose = m_lockFile.close();
        MojErrAccumulate(err, errClose);
    }

    return err;
}

MojErr MojDbLevelEnv::postCommit(MojSize updateSize)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    return MojErrNone;
}

////////////////////MojDbLevelEngine////////////////////////////////////////////

MojDbLevelEngine::MojDbLevelEngine()
: m_isOpen(false)
{
    MojLogTrace(s_log);
}


MojDbLevelEngine::~MojDbLevelEngine()
{
    MojLogTrace(s_log);

    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelEngine::configure(const MojObject& conf)
{
    MojLogTrace(s_log);

    return MojErrNone;
}

// although we might have only 1 real Level DB - just to be safe and consistent
// provide interface to add/drop multiple DB
MojErr MojDbLevelEngine::drop(const MojChar* path, MojDbStorageTxn* txn)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    MojAssert(m_isOpen);

    MojThreadGuard guard(m_dbMutex);

    // close sequences
    for (SequenceVec::ConstIterator i = m_seqs.begin(); i != m_seqs.end(); ++i) {
        MojErr err = (*i)->close();
        MojErrCheck(err);
    }
    m_seqs.clear();

    // drop databases
    for (DatabaseVec::ConstIterator i = m_dbs.begin(); i != m_dbs.end(); ++i) {
        MojErr err = (*i)->closeImpl();
        MojErrCheck(err);
        err = (*i)->drop(txn);
        MojErrCheck(err);
    }
    m_dbs.clear();

    return MojErrNone;
}
MojErr MojDbLevelEngine::open(const MojChar* path)
{
    MojAssert(path);
    MojAssert(!m_env.get() && !m_isOpen);
    MojLogTrace(s_log);

    // this is more like a placeholder
    MojRefCountedPtr<MojDbLevelEnv> env(new MojDbLevelEnv);
    MojAllocCheck(env.get());
    MojErr err = env->open(path);
    MojErrCheck(err);
    err = open(NULL, env.get());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelEngine::open(const MojChar* path, MojDbEnv* env)
{
    MojDbLevelEnv* bEnv = static_cast<MojDbLevelEnv *> (env);
    MojAssert(bEnv);
    MojAssert(!m_env.get() && !m_isOpen);
    MojLogTrace(s_log);

    m_env.reset(bEnv);
    if (path) {
        MojErr err = m_path.assign(path);
        MojErrCheck(err);
        // create dir
        err = MojCreateDirIfNotPresent(path);
        MojErrCheck(err);
    }
    // bool created = false;
    // we don't need to open any DB here since we have a single leveldb
    // and it should be open by MojoDB core code

//  /*TODO: FIX THIS!!! */
    /*m_isOpen = false;

    // open seqence db
    bool created = false;
    m_seqDb.reset(new MojDbLevelDatabase);
    MojAllocCheck(m_seqDb.get());
    MojErr err = m_seqDb->open(MojEnvSeqDbName, this, created, NULL);
    MojErrCheck(err);

    // open index db
    m_indexDb.reset(new MojDbLevelDatabase);
    MojAllocCheck(m_indexDb.get());
    err = m_indexDb->open(MojEnvIndexDbName, this, created, NULL);
    MojErrCheck(err);*/
    m_isOpen = true;

    return MojErrNone;
}

MojErr MojDbLevelEngine::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);
    MojErr err = MojErrNone;
    MojErr errClose = MojErrNone;

    // close dbs
    /*if (m_seqDb.get()) {
        errClose = m_seqDb->close();
        MojErrAccumulate(err, errClose);
        m_seqDb.reset();
    }
    if (m_indexDb.get()) {
        errClose = m_indexDb->close();
        MojErrAccumulate(err, errClose);
        m_indexDb.reset();
    }*/
    m_env.reset();
    m_isOpen = false;

    return err;
}

MojErr MojDbLevelEngine::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    MojAssert(!txnOut.get());
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelTxn> txn(new MojDbLevelTxn());
    MojAllocCheck(txn.get());
    txnOut = txn;

    if (!m_dbs.empty()) {
        MojDbLevelDatabase * db = m_dbs[0].get();
        return db->beginTxn(txnOut);
        postTransaction = NULL;
    } else {
        postTransaction = &txnOut;
    }

    return MojErrNone;

    // TODO: Check this

    /*MojAssert(m_dbs.size() != 0);*/
}

MojErr MojDbLevelEngine::openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut)
{
    MojAssert(name && !dbOut.get());
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelDatabase> db(new MojDbLevelDatabase());
    MojAllocCheck(db.get());
    bool created = false;

    MojErr err = db->open(name, this, created, txn);
    MojErrCheck(err);

    if (m_dbs.find(db) == MojInvalidIndex) {
        m_dbs.push(db);

        return MojErrDbFatal;
    }

    dbOut = db;

    return MojErrNone;
}

MojErr MojDbLevelEngine::openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut)
{
    MojAssert(name && !seqOut.get());
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelSeq> seq(new MojDbLevelSeq());
    MojAllocCheck(seq.get());
    //TODO: debug this!!!
    //MojErr err = seq->open(name, m_seqDb.get());

    MojAssert(m_dbs[0].get());

    MojErr err = seq->open(name, m_dbs[0].get());
    MojErrCheck(err);
    seqOut = seq;
    m_seqs.push(seq);

    return MojErrNone;
}


// placeholder
MojErr MojDbLevelEngine::compact()
{
    return MojErrNone;
}
MojErr MojDbLevelEngine::addDatabase(MojDbLevelDatabase* db)
{
    MojAssert(db);
    MojLogTrace(MojDbLevelEngine::s_log);
    MojThreadGuard guard(m_dbMutex);

    if (m_dbs.find(db) != MojInvalidIndex) {
        MojLogError(MojDbLevelEngine::s_log, _T("Database already in database pool"));
        return MojErrDbFatal;
    }

    return m_dbs.push(db);
}

MojErr MojDbLevelEngine::removeDatabase(MojDbLevelDatabase* db)
{
    MojAssert(db);
    MojLogTrace(MojDbLevelEngine::s_log);
    MojThreadGuard guard(m_dbMutex);

    MojSize idx;
    MojSize size = m_dbs.size();
    for (idx = 0; idx < size; ++idx) {
        if (m_dbs.at(idx).get() == db) {
            MojErr err = m_dbs.erase(idx);
            MojErrCheck(err);
            break;
        }
    }
    return MojErrNone;
}

MojErr MojDbLevelEngine::addSeq(MojDbLevelSeq* seq)
{
    MojAssert(seq);
    MojLogTrace(MojDbLevelEngine::s_log);
    MojThreadGuard guard(m_dbMutex);

    return m_seqs.push(seq);
}

MojErr MojDbLevelEngine::removeSeq(MojDbLevelSeq* seq)
{
    MojAssert(seq);
    MojLogTrace(MojDbLevelEngine::s_log);
    MojThreadGuard guard(m_dbMutex);

    MojSize idx;
    MojSize size = m_seqs.size();
    for (idx = 0; idx < size; ++idx) {
        if (m_seqs.at(idx).get() == seq) {
            MojErr err = m_seqs.erase(idx);
            MojErrCheck(err);
            break;
        }
    }
    return MojErrNone;
}

////////////////////MojDbIndex////////////////////////////////////////////

MojDbLevelIndex::MojDbLevelIndex()
: m_primaryDb(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelIndex::~MojDbLevelIndex()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err =  close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelIndex::open(const MojObject& id, MojDbLevelDatabase* db, MojDbStorageTxn* txn)
{
    MojAssert(db && db->engine());
    MojLogTrace(MojDbLevelEngine::s_log);

    m_id = id;

    // this is deprecated
    // m_db and m_primaryDb should point to the same object
    // leave both of them here more for debugging purposes

    // TODO: Debug THIS!!
    /*m_db.reset(db->engine()->indexDb());
    m_primaryDb.reset(db);*/

    m_db.reset(db);
    m_primaryDb.reset(db);

    return MojErrNone;
}

MojErr MojDbLevelIndex::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    m_db.reset();
    m_primaryDb.reset();

    return MojErrNone;
}

MojErr MojDbLevelIndex::drop(MojDbStorageTxn* txn)
{
    MojLogTrace(MojDbLevelEngine::s_log);

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
    MojLogTrace(MojDbLevelEngine::s_log);

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
    MojAssert(txn);
    MojLogTrace(MojDbLevelEngine::s_log);

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
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbindexinsert: %s; keylen: %zu, key: %s ; vallen = %zu; err = %d\n"),
                    m_db->m_name.data(), size1, s, size2, err);
#endif
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelIndex::del(const MojDbKey& key, MojDbStorageTxn* txn)
{
    MojAssert(txn);
    MojAssert(isOpen());
    MojLogTrace(MojDbLevelEngine::s_log);

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
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbindexdel: %s; keylen: %zu, key: %s ; err = %d\n"), m_db->m_name.data(), size1, s, err);
    if (!found)
        MojLogWarning(MojDbLevelEngine::s_log, _T("ldbindexdel_warn: not found: %s \n"), s);
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
    MojAssert(isOpen());
    MojAssert(plan.get() && txn);
    MojLogTrace(MojDbLevelEngine::s_log);

    MojRefCountedPtr<MojDbLevelQuery> storageQuery(new MojDbLevelQuery());
    MojAllocCheck(storageQuery.get());
    MojErr err = storageQuery->open(m_db.get(), m_primaryDb.get(), plan, txn);
    MojErrCheck(err);
    queryOut = storageQuery;

    return MojErrNone;
}

MojErr MojDbLevelIndex::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    return m_primaryDb->beginTxn(txnOut);
}

////////////////////MojDbLevelItem////////////////////////////////////////////

MojDbLevelItem::MojDbLevelItem()
: m_free(MojFree)
{
}

MojErr MojDbLevelItem::kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine)
{
    MojErr err = m_header.read(kindEngine);
    MojErrCheck(err);

    kindIdOut = m_header.kindId();

    return MojErrNone;
}

MojErr MojDbLevelItem::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected) const
{
    MojErr err = MojErrNone;
    MojTokenSet tokenSet;
    if (headerExpected) {
        err = m_header.visit(visitor, kindEngine);
        MojErrCheck(err);
        const MojString& kindId = m_header.kindId();
        err = kindEngine.tokenSet(kindId, tokenSet);
        MojErrCheck(err);
        m_header.reader().tokenSet(&tokenSet);
        err = m_header.reader().read(visitor);
        MojErrCheck(err);
    } else {
        MojObjectReader reader(data(), size());
        err = reader.read(visitor);
        MojErrCheck(err);
    }
    return MojErrNone;
}

void MojDbLevelItem::id(const MojObject& id)
{
    m_header.reset();
    m_header.id(id);
    m_header.reader().data(data(), size());
}

void MojDbLevelItem::clear()
{
    freeData();
    m_free = MojFree;
    m_slice.clear();
}

bool MojDbLevelItem::hasPrefix(const MojDbKey& prefix) const
{
    return (size() >= prefix.size() &&
            MojMemCmp(data(), prefix.data(), prefix.size()) == 0);
}

MojErr MojDbLevelItem::toArray(MojObject& arrayOut) const
{
    MojObjectBuilder builder;
    MojErr err = builder.beginArray();
    MojErrCheck(err);
    err = MojObjectReader::read(builder, data(), size());
    MojErrCheck(err);
    err = builder.endArray();
    MojErrCheck(err);
    arrayOut = builder.object();

    return MojErrNone;
}

MojErr MojDbLevelItem::toObject(MojObject& objOut) const
{
    MojObjectBuilder builder;
    MojErr err = MojObjectReader::read(builder, data(), size());
    MojErrCheck(err);
    objOut = builder.object();

    return MojErrNone;
}

void MojDbLevelItem::fromBytesNoCopy(const MojByte* bytes, MojSize size)
{
    MojAssert(bytes || size == 0);
    MojAssert(size <= MojUInt32Max);

    setData(const_cast<MojByte*> (bytes), size, NULL);
}

MojErr MojDbLevelItem::fromBuffer(MojBuffer& buf)
{
    clear();
    if (!buf.empty()) {
        MojErr err = buf.release(m_chunk);
        MojErrCheck(err);
        MojAssert(m_chunk.get());
        setData(m_chunk->data(), m_chunk->dataSize(), NULL);
    }
    return MojErrNone;
}

MojErr MojDbLevelItem::fromBytes(const MojByte* bytes, MojSize size)
{
    MojAssert (bytes || size == 0);

    if (size == 0) {
        clear();
    } else {
        MojByte* newBytes = (MojByte*)MojMalloc(size);
        MojAllocCheck(newBytes);
        MojMemCpy(newBytes, bytes, size);
        setData(newBytes, size, MojFree);
    }
    return MojErrNone;
}

MojErr MojDbLevelItem::fromObject(const MojObject& obj)
{
    MojObjectWriter writer;
    MojErr err = obj.visit(writer);
    MojErrCheck(err);
    err = fromBuffer(writer.buf());
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelItem::fromObjectVector(const MojVector<MojObject>& vec)
{
    MojAssert(!vec.empty());

    MojObjectWriter writer;
    for (MojVector<MojObject>::ConstIterator i = vec.begin();
        i != vec.end();
        ++i) {
        MojErr err = i->visit(writer);
        MojErrCheck(err);
    }
    fromBuffer(writer.buf());

    return MojErrNone;
}

void MojDbLevelItem::freeData()
{
   // since we are using levelDB::Slice  - no need for memory cleanup
}

void MojDbLevelItem::setData(MojByte* bytes, MojSize size, void (*free)(void*))
{
    MojAssert(bytes);
    freeData();
    m_free = free;
    m_slice = leveldb::Slice( (const char *)bytes, size);
    m_header.reader().data(bytes, size);
}

/////////////////////////////MojDbLevelSeq//////////////////////////////////////////////////////

// at this point it's just a placeholder
MojDbLevelSeq::~MojDbLevelSeq()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelSeq::open(const MojChar* name, MojDbLevelDatabase* db)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    //MojAssert(!m_seq);

    m_db = db;

    return MojErrNone;
}

MojErr MojDbLevelSeq::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    return MojErrNone;
}

MojErr MojDbLevelSeq::get(MojInt64& valOut)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    // just a number
    valOut = (MojInt64) 1;

    return MojErrNone;
}

///////////////////////////////MojDbLevelTxn////////////////////////////////////////////////////

MojDbLevelTxn::MojDbLevelTxn()
: m_db(NULL),
  m_batch(NULL),
  m_updateSize(0)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelTxn::~MojDbLevelTxn()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_batch) {
        abort();
    }
}
MojErr MojDbLevelTxn::begin(MojDbLevelDatabase* db)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    MojAssert(!m_batch);
    MojAssert(db);
    MojAssert(db->impl());

    m_db = db;
    m_batch = new leveldb::WriteBatch();
    return MojErrNone;
}

MojErr MojDbLevelTxn::commitImpl()
{
    MojAssert(m_batch);
    MojLogTrace(MojDbLevelEngine::s_log);

        leveldb::WriteOptions write_options;
        write_options.sync = true;

    MojAssert(m_db->impl());

        leveldb::Status s = m_db->impl()->Write(write_options, m_batch);
    delete m_batch;
        m_batch = NULL;

    MojLdbErrCheck(s, _T("txn->commit"));

    if (m_updateSize != 0) {
        MojErr err = m_db->engine()->env()->postCommit(m_updateSize);
        MojErrCheck(err);
    }
    return MojErrNone;
}

MojErr MojDbLevelTxn::abort()
{
    MojAssert(m_batch);
    MojLogTrace(MojDbLevelEngine::s_log);
    MojLogWarning(MojDbLevelEngine::s_log, _T("ldb: transaction aborted"));

    m_batch->Clear();
    delete m_batch;
    m_batch = NULL;

    return MojErrNone;
}

bool MojDbLevelTxn::isValid()
{
    return (m_db);
}


///////////////////////////////////////////////////////////////////////////////////
