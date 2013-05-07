/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics
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

#ifndef MOJDBLEVELENGINE_H_
#define MOJDBLEVELENGINE_H_

#include "leveldb/db.h"
#include "db/MojDbDefs.h"
#include "db/MojDbObjectHeader.h"
#include "db/MojDbStorageEngine.h"
#include "core/MojBuffer.h"
#include "core/MojFile.h"

#define MojLdbErrCheck(E, FNAME)                if (!E.ok()) MojErrThrowMsg(MojErrDbFatal, _T("ldb: %s - %s"), FNAME, E.ToString().data())
#define MojLdbErrAccumulate(EACC, E, FNAME)     if (!E.ok()) MojErrAccumulate(EACC, MojErrDbFatal)

class MojDbLevelCursor;
class MojDbLevelDatabase;
class MojDbLevelEngine;
class MojDbLevelEnv;
class MojDbLevelIndex;
class MojDbLevelItem;
class MojDbLevelQuery;
class MojDbLevelSeq;

class MojDbLevelCursor : public MojNoCopy
{
public:
    MojDbLevelCursor();
    ~MojDbLevelCursor();

    MojErr open(MojDbLevelDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags);
    MojErr close();
    MojErr del();
    MojErr delPrefix(const MojDbKey& prefix);
    MojErr get(MojDbLevelItem& key, MojDbLevelItem& val, bool& foundOut, MojUInt32 flags);
    MojErr stats(MojSize& countOut, MojSize& sizeOut);
    MojErr statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut);

    leveldb::Iterator* impl() { return m_it; }

    enum LDB_FLAGS
    {
       e_First = 0, e_Last, e_Next, e_Prev, e_Range, e_Set, e_TotalFlags
    };

private:
    leveldb::Iterator* m_it;
    leveldb::DB* m_db;
    MojDbStorageTxn* m_txn;
    MojSize m_recSize;
    MojSize m_warnCount;
};

class MojDbLevelDatabase : public MojDbStorageDatabase
{
public:
    MojDbLevelDatabase();
    ~MojDbLevelDatabase();

    MojErr open(const MojChar* dbName, MojDbLevelEngine* env, bool& createdOut, MojDbStorageTxn* txn);
    virtual MojErr close();
    virtual MojErr drop(MojDbStorageTxn* txn);
    virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut);
    virtual MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn);
    virtual MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn);
    virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut);
    virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut);
    virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut);
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
    virtual MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut);

//hack:
    virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutex_regionsize);

    MojErr put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota);
    MojErr put(MojDbLevelItem& key, MojDbLevelItem& val, MojDbStorageTxn* txn, bool updateIdQuota);
    MojErr del(MojDbLevelItem& key, bool& foundOut, MojDbStorageTxn* txn);
    MojErr get(MojDbLevelItem& key, MojDbStorageTxn* txn, bool forUpdate, MojDbLevelItem& valOut, bool& foundOut);

    leveldb::DB* impl() { return m_db; }
    MojDbLevelEngine* engine() { return m_engine; }

private:
    friend class MojDbLevelEngine;
    friend class MojDbLevelIndex;

    //MojErr verify();
    MojErr closeImpl();
    void postUpdate(MojDbStorageTxn* txn, MojSize updateSize);

    leveldb::DB* m_db;
    MojDbLevelEngine* m_engine;
    MojString m_file;
    MojString m_name;
    MojVector<MojString> m_primaryProps;
};

class MojDbLevelEnv : public MojDbEnv
{
public:
    static MojLogger s_log;

    MojDbLevelEnv();
    ~MojDbLevelEnv();

    MojErr configure(const MojObject& conf);
    MojErr open(const MojChar* path);
    MojErr close();
    MojErr postCommit(MojSize updateSize);

    void* impl() { return m_db; }
    static MojErr translateErr(int dbErr);

private:
    static const MojChar* const LockFileName;

    MojErr lockDir(const MojChar* path);
    MojErr unlockDir();

    MojString m_lockFileName;
    MojFile m_lockFile;
    MojString m_logDir;
    leveldb::DB* m_db;
};

class MojDbLevelEngine : public MojDbStorageEngine
{
public:
    static MojLogger s_log;

    MojDbLevelEngine();
    ~MojDbLevelEngine();

    virtual MojErr configure(const MojObject& conf);
    virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn);
    virtual MojErr open(const MojChar* path);
    virtual MojErr open(const MojChar* path, MojDbEnv* env);
    virtual MojErr close();
    virtual MojErr compact();
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
    virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut) ;
    virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut) ;

    const MojString& path() const { return m_path; }
    MojDbLevelEnv* env() { return m_env.get(); }
    MojErr addDatabase(MojDbLevelDatabase* db);
    MojErr removeDatabase(MojDbLevelDatabase* db);
    MojErr addSeq(MojDbLevelSeq* seq);
    MojErr removeSeq(MojDbLevelSeq* seq);

    MojDbLevelDatabase* indexDb() { return m_indexDb.get(); }

private:
    typedef MojVector<MojRefCountedPtr<MojDbLevelDatabase> > DatabaseVec;
    typedef MojVector<MojRefCountedPtr<MojDbLevelSeq> > SequenceVec;

    MojRefCountedPtr<MojDbLevelEnv> m_env;
    MojThreadMutex m_dbMutex;
    MojRefCountedPtr<MojDbLevelDatabase> m_indexDb;
    MojRefCountedPtr<MojDbLevelDatabase> m_seqDb;
    MojString m_path;
    DatabaseVec m_dbs;
    SequenceVec m_seqs;
    bool m_isOpen;
};

class MojDbLevelIndex : public MojDbStorageIndex
{
public:
    MojDbLevelIndex();
    virtual ~MojDbLevelIndex();

    MojErr open(const MojObject& id, MojDbLevelDatabase* db, MojDbStorageTxn* txn);
    virtual MojErr close();
    virtual MojErr drop(MojDbStorageTxn* txn);
    virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut);
    virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn);
    virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn);
    virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut);
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);

private:
    bool isOpen() const { return m_primaryDb.get() != NULL; }

    MojObject m_id;
    MojRefCountedPtr<MojDbLevelDatabase> m_primaryDb;
    MojRefCountedPtr<MojDbLevelDatabase> m_db;
};

class MojDbLevelItem : public MojDbStorageItem
{
public:
    MojDbLevelItem();
    virtual ~MojDbLevelItem() { freeData(); }
    virtual MojErr close() { return MojErrNone; }
    virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine);
    virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
    virtual const MojObject& id() const { return m_header.id(); }
    virtual MojSize size() const { return m_slice.size(); }

    void clear();
    const MojByte* data() const { return (const MojByte*) m_slice.data(); }
    bool hasPrefix(const MojDbKey& prefix) const;
    MojErr toArray(MojObject& arrayOut) const;
    MojErr toObject(MojObject& objOut) const;

    void id(const MojObject& id);
    void fromBytesNoCopy(const MojByte* bytes, MojSize size);
    MojErr fromBuffer(MojBuffer& buf);
    MojErr fromBytes(const MojByte* bytes, MojSize size);
    MojErr fromObject(const MojObject& obj);
    MojErr fromObjectVector(const MojVector<MojObject>& vec);

    leveldb::Slice* impl() { return &m_slice; }

private:
    void freeData();
    void setData(MojByte* bytes, MojSize size, void (*free)(void*));

    leveldb::Slice m_slice;
    MojAutoPtr<MojBuffer::Chunk> m_chunk;
    mutable MojDbObjectHeader m_header;
    void (*m_free)(void*);
};

class MojDbLevelSeq : public MojDbStorageSeq
{
public:
    MojDbLevelSeq() : m_db(NULL) {}
    ~MojDbLevelSeq();

    MojErr open(const MojChar* name, MojDbLevelDatabase* db);
    virtual MojErr close();
    virtual MojErr get(MojInt64& valOut);


private:
    friend class MojDbLevelEngine;

    MojDbLevelDatabase* m_db;
};

#endif /* MOJDBLEVELENGINE_H_ */
