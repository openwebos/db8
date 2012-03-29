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


#ifndef MOJDBBERKELEYENGINE_H_
#define MOJDBBERKELEYENGINE_H_

#include "db.h"
#include "db/MojDbDefs.h"
#include "db/MojDbObjectHeader.h"
#include "db/MojDbStorageEngine.h"
#include "core/MojBuffer.h"
#include "core/MojFile.h"

#define MojBdbErrCheck(E, FNAME)				if (E) MojErrThrowMsg(MojDbBerkeleyEnv::translateErr(E), _T("bdb: %s - %s"), FNAME, db_strerror(E))
#define MojBdbErrAccumulate(EACC, E, FNAME)		if (E) MojErrAccumulate(EACC, MojDbBerkeleyEnv::translateErr(E))
#define MojBdbTxnFromStorageTxn(TXN)			((TXN) ? static_cast<MojDbBerkeleyTxn*>(TXN)->impl() : NULL)

class MojDbBerkeleyCursor;
class MojDbBerkeleyDatabase;
class MojDbBerkeleyEngine;
class MojDbBerkeleyEnv;
class MojDbBerkeleyIndex;
class MojDbBerkeleyItem;
class MojDbBerkeleyQuery;
class MojDbBerkeleySeq;
class MojDbBerkeleyTxn;

class MojDbBerkeleyCursor : public MojNoCopy
{
public:
	MojDbBerkeleyCursor();
	~MojDbBerkeleyCursor();

	MojErr open(MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags);
	MojErr close();
	MojErr del();
	MojErr delPrefix(const MojDbKey& prefix);
	MojErr get(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, bool& foundOut, MojUInt32 flags);
	MojErr stats(MojSize& countOut, MojSize& sizeOut);
	MojErr statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut);

	DBC* impl() { return m_dbc; }

private:
	DBC* m_dbc;
	MojDbStorageTxn* m_txn;
	MojSize m_recSize;
	MojSize m_warnCount;
};

class MojDbBerkeleyDatabase : public MojDbStorageDatabase
{
public:
	MojDbBerkeleyDatabase();
	~MojDbBerkeleyDatabase();

	MojErr open(const MojChar* dbName, MojDbBerkeleyEngine* env, bool& createdOut, MojDbStorageTxn* txn);
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

// work-around - specific for BDB only
	virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutex_regionsize);

      	MojErr put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota);
	MojErr put(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, MojDbStorageTxn* txn, bool updateIdQuota);
	MojErr del(MojDbBerkeleyItem& key, bool& foundOut, MojDbStorageTxn* txn);
	MojErr get(MojDbBerkeleyItem& key, MojDbStorageTxn* txn, bool forUpdate, MojDbBerkeleyItem& valOut, bool& foundOut);

	DB* impl() { return m_db; }
	MojDbBerkeleyEngine* engine() { return m_engine; }

private:
	friend class MojDbBerkeleyEngine;
	friend class MojDbBerkeleyIndex;

	MojErr verify();
	MojErr closeImpl();
	void postUpdate(MojDbStorageTxn* txn, MojSize updateSize);

	DB* m_db;
	MojDbBerkeleyEngine* m_engine;
	MojString m_file;
	MojString m_name;
	MojVector<MojString> m_primaryProps;
};

class MojDbBerkeleyEnv : public MojDbEnv
{
public:
	static MojLogger s_log;

	MojDbBerkeleyEnv();
	~MojDbBerkeleyEnv();

	MojErr configure(const MojObject& conf);
	MojErr open(const MojChar* path);
	MojErr close();
	MojErr postCommit(MojSize updateSize);
	MojErr checkpoint(MojUInt32 minKB);

	const MojUInt32 compactStepSize(void) const { return m_compactStepSize; }
	DB_ENV* impl() { return m_env; }
	static MojErr translateErr(int dbErr);

private:
	static const MojChar* const LockFileName;

	MojErr tryCheckpoint(MojUInt32 minKB);
	MojErr checkpointImpl(MojUInt32 minKB);
	MojErr create(const MojChar* dir);
	MojErr purgeLogs();
	MojErr lockDir(const MojChar* path);
	MojErr unlockDir();

	static void errcall(const DB_ENV *dbenv, const char *errpfx, const char *msg);
	static void msgcall(const DB_ENV *dbenv, const char *msg);

	MojAtomicInt m_updateSize;
	MojThreadMutex m_checkpointMutex;
	MojString m_lockFileName;
	MojFile m_lockFile;
	DB_ENV* m_env;

	MojString m_logDir;
	MojUInt32 m_flags;
	MojUInt32 m_logFlags;
	MojUInt32 m_logFileSize;
	MojUInt32 m_logRegionSize;
	MojUInt32 m_cacheSize;
	MojUInt32 m_maxLocks;
	MojUInt32 m_maxLockers;
	MojUInt32 m_checkpointMinKb;
	MojUInt32 m_compactStepSize;
};

class MojDbBerkeleyEngine : public MojDbStorageEngine
{
public:
	static MojLogger s_log;

	MojDbBerkeleyEngine();
	~MojDbBerkeleyEngine();

	virtual MojErr configure(const MojObject& conf);
	virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn);
	virtual MojErr open(const MojChar* path);
	virtual MojErr open(const MojChar* path, MojDbEnv* env);
	virtual MojErr close();
	virtual MojErr compact();
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
	virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut);
	virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut);

	const MojString& path() const { return m_path; }
	MojDbBerkeleyEnv* env() { return m_env.get(); }
	MojDbBerkeleyDatabase* indexDb() { return m_indexDb.get(); }
	MojErr addDatabase(MojDbBerkeleyDatabase* db);
	MojErr removeDatabase(MojDbBerkeleyDatabase* db);
	MojErr addSeq(MojDbBerkeleySeq* seq);
	MojErr removeSeq(MojDbBerkeleySeq* seq);

private:
	typedef MojVector<MojRefCountedPtr<MojDbBerkeleyDatabase> > DatabaseVec;
	typedef MojVector<MojRefCountedPtr<MojDbBerkeleySeq> > SequenceVec;

	MojRefCountedPtr<MojDbBerkeleyEnv> m_env;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_seqDb;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_indexDb;
	MojThreadMutex m_dbMutex;
	MojString m_path;
	DatabaseVec m_dbs;
	SequenceVec m_seqs;
	bool m_isOpen;
};

class MojDbBerkeleyIndex : public MojDbStorageIndex
{
public:
	MojDbBerkeleyIndex();
	virtual ~MojDbBerkeleyIndex();

	MojErr open(const MojObject& id, MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn);
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
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_primaryDb;
	MojRefCountedPtr<MojDbBerkeleyDatabase> m_db;
};

class MojDbBerkeleyItem : public MojDbStorageItem
{
public:
	MojDbBerkeleyItem();
	virtual ~MojDbBerkeleyItem() { freeData(); }
	virtual MojErr close() { return MojErrNone; }
	virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine);
	virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
	virtual const MojObject& id() const { return m_header.id(); }
	virtual MojSize size() const { return m_dbt.size; }

	void clear();
	const MojByte* data() const { return (const MojByte*) m_dbt.data; }
	bool hasPrefix(const MojDbKey& prefix) const;
	MojErr toArray(MojObject& arrayOut) const;
	MojErr toObject(MojObject& objOut) const;

	void id(const MojObject& id);
	void fromBytesNoCopy(const MojByte* bytes, MojSize size);
	MojErr fromBuffer(MojBuffer& buf);
	MojErr fromBytes(const MojByte* bytes, MojSize size);
	MojErr fromObject(const MojObject& obj);
	MojErr fromObjectVector(const MojVector<MojObject>& vec);

	DBT* impl() { return &m_dbt; }

private:
	void freeData();
	void setData(MojByte* bytes, MojSize size, void (*free)(void*));

	DBT m_dbt;
	MojAutoPtr<MojBuffer::Chunk> m_chunk;
	mutable MojDbObjectHeader m_header;
	void (*m_free)(void*);
};

class MojDbBerkeleySeq : public MojDbStorageSeq
{
public:
	MojDbBerkeleySeq() : m_db(NULL), m_seq(NULL) {}
	~MojDbBerkeleySeq();

	MojErr open(const MojChar* name, MojDbBerkeleyDatabase* db);
	virtual MojErr close();
	virtual MojErr get(MojInt64& valOut);

	DB_SEQUENCE* impl() { return m_seq; }

private:
	friend class MojDbBerkeleyEngine;

	MojErr closeImpl();

	MojDbBerkeleyDatabase* m_db;
	DB_SEQUENCE* m_seq;
};

class MojDbBerkeleyTxn : public MojDbStorageTxn
{
public:
	MojDbBerkeleyTxn();
	~MojDbBerkeleyTxn();

	MojErr begin(MojDbBerkeleyEngine* env);
	virtual MojErr abort();

	DB_TXN* impl() { return m_txn; }
	MojDbBerkeleyEngine* engine() { return m_engine; }
	void didUpdate(MojSize size) { m_updateSize += size; }
	MojSize updateSize() const { return m_updateSize; }

private:
	virtual MojErr commitImpl();

	MojDbBerkeleyEngine* m_engine;
	DB_TXN* m_txn;
	MojSize m_updateSize;
};

#endif /* MOJDBBERKELEYENGINE_H_ */
