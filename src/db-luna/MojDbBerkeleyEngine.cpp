/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#include "db-luna/MojDbBerkeleyEngine.h"
#include "db-luna/MojDbBerkeleyFactory.h"
#include "db-luna/MojDbBerkeleyQuery.h"
#include "db/MojDbObjectHeader.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojString.h"
#include "core/MojTokenSet.h"

#include <sys/statvfs.h>

// GENERAL
static const int MojDbFileMode = MOJ_S_IRUSR | MOJ_S_IWUSR;
// DB
static const MojUInt32 MojDbOpenFlags = DB_THREAD /*| DB_MULTIVERSION*/; // NOTE: MULTIVERSION disabled due to leaked transactions
static const MojUInt32 MojDbGetFlags = DB_READ_COMMITTED;
// LOG
static const MojUInt32 MojLogFlags = 0;
static const MojUInt32 MojLogArchiveFlags = DB_ARCH_ABS;
// ENV
static const MojUInt32 MojEnvFlags = DB_AUTO_COMMIT | DB_NOMMAP;
static const MojUInt32 MojEnvOpenFlags = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL |
		DB_INIT_TXN | DB_RECOVER | DB_THREAD | DB_PRIVATE | DB_DIRECT_DB;
static const bool MojEnvDefaultPrivate = true;
static const bool MojEnvDefaultLogAutoRemove = true;
static const MojUInt32 MojEnvDefaultLogFileSize = 1024 * 1024 * 2; // 2MB
static const MojUInt32 MojEnvDefaultLogRegionSize = 1024 * 512; // 512 KB
static const MojUInt32 MojEnvDefaultCacheSize = 1024 * 1024 * 2; // 2MB
static const MojUInt32 MojEnvDefaultMaxLocks = 8000; // max db size / page size
static const MojUInt32 MojEnvDefaultMaxLockers = 1000 * 2; // twice the default
static const MojUInt32 MojEnvDefaultCheckpointMinKb = 512; // 1MB
static const MojUInt32 MojEnvDefaultCheckpointMinMinutes = 0;
static const MojUInt32 MojEnvDefaultCompactStepSize = 25000;
static const MojChar* const MojEnvSeqDbName = _T("sequences.db");
static const MojChar* const MojEnvIndexDbName = _T("indexes.db");
// SEQ
static const MojUInt32 MojSeqOpenFlags = DB_CREATE | DB_THREAD;
static const MojUInt32 MojSeqGetFlags = DB_TXN_NOSYNC;
static const MojUInt32 MojSeqCacheSize = 25;
// TXN
static const MojUInt32 MojTxnBeginFlags = DB_READ_COMMITTED;
static const MojUInt32 MojTxnMax = 100; // need up to one per cache page for mvcc

const MojChar* const MojDbBerkeleyEnv::LockFileName = _T("_lock");
MojLogger MojDbBerkeleyEngine::s_log(_T("db.bdb"));

MojDbBerkeleyCursor::MojDbBerkeleyCursor()
: m_dbc(NULL),
  m_txn(NULL),
  m_recSize(0)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyCursor::~MojDbBerkeleyCursor()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyCursor::open(MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags)
{
	MojAssert(db && txn);
	MojAssert(!m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB* bdb = db->impl();
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	DBC* dbc = NULL;
	int dbErr = bdb->cursor(bdb, dbTxn, &dbc, flags);
	MojBdbErrCheck(dbErr, _T("db->cursor"));
	MojAssert(dbc);
	m_dbc = dbc;
	m_txn = txn;
	m_warnCount = 0;

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_dbc) {
		int dbErr = m_dbc->close(m_dbc);
		MojBdbErrAccumulate(err, dbErr, _T("dbc->close"));
		m_dbc = NULL;
		m_txn = NULL;
	}
	return err;
}

MojErr MojDbBerkeleyCursor::del()
{
	MojAssert(m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_dbc->del(m_dbc, 0);
	MojBdbErrCheck(dbErr, _T("dbc->del"));
	MojErr err = m_txn->offsetQuota(-(MojInt64) m_recSize);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::delPrefix(const MojDbKey& prefix)
{
	MojDbBerkeleyItem val;
	MojDbBerkeleyItem key;
	MojErr err = key.fromBytes(prefix.data(), prefix.size());
	MojErrCheck(err);

	bool found = false;
	err = get(key, val, found, DB_SET_RANGE);
	MojErrCheck(err);
	while (found && key.hasPrefix(prefix)) {
		err = del();
		MojErrCheck(err);
		err = get(key, val, found, DB_NEXT);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::get(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, bool& foundOut, MojUInt32 flags)
{
	MojAssert(m_dbc);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	int dbErr = m_dbc->get(m_dbc, key.impl(), val.impl(), flags);
	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("dbc->get"));
		foundOut = true;
		m_recSize = key.size() + val.size();
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::stats(MojSize& countOut, MojSize& sizeOut)
{
	
	MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
	MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbcursor_stats: count: %d, size: %d, err: %d"), (int)countOut, (int)sizeOut, (int)err);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyCursor::statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut)
{
	countOut = 0;
	sizeOut = 0;
	m_warnCount = 0;	// debug
	MojDbBerkeleyItem val;
	MojDbBerkeleyItem key;
	MojErr err = key.fromBytes(prefix.data(), prefix.size());
	MojErrCheck(err);
	
	MojSize count = 0;
	MojSize size = 0;
	bool found = false;
	err = get(key, val, found, DB_SET_RANGE);
	MojErrCheck(err);
	while (found && key.hasPrefix(prefix)) {
		++count;
		// debug code for verifying index keys
		size += key.size() + val.size();
		err = get(key, val, found, DB_NEXT);
		MojErrCheck(err);
	}
	sizeOut = size;
	countOut = count;

	return MojErrNone;
}

MojDbBerkeleyDatabase::MojDbBerkeleyDatabase()
: m_db(NULL)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyDatabase::~MojDbBerkeleyDatabase()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyDatabase::open(const MojChar* dbName, MojDbBerkeleyEngine* eng, bool& createdOut, MojDbStorageTxn* txn)
{
	MojAssert(dbName && eng);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

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
	DB* db = NULL;
	int dbErr = db_create(&db, eng->env()->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_create"));
	MojAssert(db);
	m_db = db;

	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	MojUInt32 flags = MojDbOpenFlags;
	if (!dbTxn)
		flags |= DB_AUTO_COMMIT;
	// try once without the DB_CREATE flag
	dbErr = m_db->open(m_db, dbTxn, m_file, dbName, DB_BTREE, flags, MojDbFileMode);
	if (dbErr == MojErrNotFound) {
		// if open failed, we know that we had to create the db
		createdOut = true;
		flags |= DB_CREATE;
		dbErr = m_db->open(m_db, dbTxn, m_file, dbName, DB_BTREE, flags, MojDbFileMode);
	}
	MojBdbErrCheck(dbErr, _T("db->open"));
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

MojErr MojDbBerkeleyDatabase::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_db) {
		MojErr errClose = closeImpl();
		MojErrAccumulate(err, errClose);
		m_primaryProps.clear();
		engine()->removeDatabase(this);
	}
	return err;
}

MojErr MojDbBerkeleyDatabase::drop(MojDbStorageTxn* txn)
{
	MojAssert(m_engine && !m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* env = m_engine->env()->impl();
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	MojUInt32 flags = dbTxn ? 0 : DB_AUTO_COMMIT;
	int dbErr = env->dbremove(env, dbTxn, m_file, NULL, flags);
	MojBdbErrCheck(dbErr, _T("env->dbremove"));

	return MojErrNone;
}


MojErr MojDbBerkeleyDatabase::mutexStats(int * total_mutexes, int * mutexes_free, int * mutexes_used,
	 int * mutexes_used_highwater, int * mutexes_regionsize)
{
	MojAssert(m_engine);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* env = m_engine->env()->impl();
	DB_MUTEX_STAT * statp = NULL;
	int dbErr = env->mutex_stat(env, &statp, 0);
	MojBdbErrCheck(dbErr, _T("env->mutex_stat"));
	
	if (total_mutexes)
		*total_mutexes = statp->st_mutex_cnt;
	if (mutexes_free)
		*mutexes_free = statp->st_mutex_free;
	if (mutexes_used)
		*mutexes_used = statp->st_mutex_inuse;
	if (mutexes_used_highwater)
		*mutexes_used_highwater = statp->st_mutex_inuse_max;
	if (mutexes_regionsize)
		*mutexes_regionsize = (int)(statp->st_regsize);

	free(statp);

	return MojErrNone;
}


MojErr MojDbBerkeleyDatabase::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
	MojErr err = cursor.open(this, txn, 0);
	MojErrCheck(err);
	err = cursor.stats(countOut, sizeOut);
	MojErrCheck(err);
	err = cursor.close();
	MojErrCheck(err);

	return err;
}

MojErr MojDbBerkeleyDatabase::insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn)
{
	MojAssert(txn);

	MojErr err = put(id, val, txn, true);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn)
{
	MojAssert(oldVal && txn);

	MojErr err = txn->offsetQuota(-(MojInt64) oldVal->size());
	MojErrCheck(err);
	err = put(id, val, txn, false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	err = del(idItem, foundOut, txn);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	itemOut.reset();
	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	MojRefCountedPtr<MojDbBerkeleyItem> valItem(new MojDbBerkeleyItem);
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

MojErr MojDbBerkeleyDatabase::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyQuery> storageQuery(new MojDbBerkeleyQuery);
	MojAllocCheck(storageQuery.get());
	MojErr err = storageQuery->open(this, NULL, plan, txn);
	MojErrCheck(err);
	queryOut = storageQuery;

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::put(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem idItem;
	MojErr err = idItem.fromObject(id);
	MojErrCheck(err);
	MojDbBerkeleyItem valItem;
	err = valItem.fromBuffer(val);
	MojErrCheck(err);
	err = put(idItem, valItem, txn, updateIdQuota);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::put(MojDbBerkeleyItem& key, MojDbBerkeleyItem& val, MojDbStorageTxn* txn, bool updateIdQuota)
{
	MojAssert(m_db && txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojInt64 quotaOffset = val.size();
	if (updateIdQuota)
		quotaOffset += key.size();
	MojErr err = txn->offsetQuota(quotaOffset);
	MojErrCheck(err);
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->put(m_db, dbTxn, key.impl(), val.impl(), 0);
#if defined(MOJ_DEBUG)
	char s[1024];  
	int size1 = key.size();
	int size2 = val.size();
	MojErr err2 = MojByteArrayToHex(key.data(), size1, s); 
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(key.data()) + (size1 - 17), 16);
	MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbput: %s; keylen: %d, key: %s ; vallen = %d; err = %d\n"), 
					this->m_name.data(), size1, s, size2, err);		
#endif
	MojBdbErrCheck(dbErr, _T("db->put"));
	postUpdate(txn, key.size() + val.size());

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::get(MojDbBerkeleyItem& key, MojDbStorageTxn* txn, bool forUpdate,
								  MojDbBerkeleyItem& valOut, bool& foundOut)
{
	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	// acquire a write lock if we are going to do an update
	int flags = MojDbGetFlags;
	if (forUpdate)
		flags |= DB_RMW;
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->get(m_db, dbTxn, key.impl(), valOut.impl(), 0);
	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("db->get"));
		foundOut = true;
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojAssert(!txnOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = m_engine->beginTxn(txnOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut)
{
	MojAssert(!indexOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyIndex> index(new MojDbBerkeleyIndex());
	MojAllocCheck(index.get());
	MojErr err = index->open(id, this, txn);
	MojErrCheck(err);
	indexOut = index;

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::del(MojDbBerkeleyItem& key, bool& foundOut, MojDbStorageTxn* txn)
{
	
	MojAssert(m_db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	foundOut = false;
	MojErr err = txn->offsetQuota(-(MojInt64) key.size());
	MojErrCheck(err);
	DB_TXN* dbTxn = MojBdbTxnFromStorageTxn(txn);
	int dbErr = m_db->del(m_db, dbTxn, key.impl(), 0);
	
#if defined(MOJ_DEBUG)
	char s[1024]; 	// big enough for any key
	int size = key.size();
	MojErr err2 = MojByteArrayToHex(key.data(), size, s); 
	MojErrCheck(err2);
	if (size > 16)	// if the object-id is in key
		strncat(s, (char *)(key.data()) + (size - 17), 16);
	MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbdel: %s; keylen: %d, key= %s; err= %d \n"), this->m_name.data(), size, s, dbErr);
#endif

	if (dbErr != DB_NOTFOUND) {
		MojBdbErrCheck(dbErr, _T("db->get"));
		foundOut = true;
	}
	postUpdate(txn, key.size());

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::verify()
{
	DB* db = NULL;
	int dbErr = db_create(&db, m_engine->env()->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_create"));
	MojAssert(db);
	dbErr = db->verify(db, m_file, m_name, NULL, 0);
	MojErrCatch(dbErr, MojErrNotFound);
	MojBdbErrCheck(dbErr, _T("db_verify"));

	return MojErrNone;
}

MojErr MojDbBerkeleyDatabase::closeImpl()
{
	int dbErr = m_db->close(m_db, 0);
	m_db = NULL;
	MojBdbErrCheck(dbErr, _T("db->close"));

	return MojErrNone;
}

void MojDbBerkeleyDatabase::postUpdate(MojDbStorageTxn* txn, MojSize size)
{
	if (txn) {
		static_cast<MojDbBerkeleyTxn*>(txn)->didUpdate(size);
	}
}

MojDbBerkeleyEnv::MojDbBerkeleyEnv()
: m_env(NULL),
  m_flags(MojEnvOpenFlags),
  m_logFlags(MojLogFlags),
  m_logFileSize(MojEnvDefaultLogFileSize),
  m_logRegionSize(MojEnvDefaultLogRegionSize),
  m_cacheSize(MojEnvDefaultCacheSize),
  m_maxLocks(MojEnvDefaultMaxLocks),
  m_maxLockers(MojEnvDefaultMaxLockers),
  m_checkpointMinKb(MojEnvDefaultCheckpointMinKb),
  m_compactStepSize(MojEnvDefaultCompactStepSize) 
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojFlagSet(m_flags, DB_PRIVATE, MojEnvDefaultPrivate);
	MojFlagSet(m_logFlags, DB_LOG_AUTO_REMOVE, MojEnvDefaultLogAutoRemove);
}

MojDbBerkeleyEnv::~MojDbBerkeleyEnv()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyEnv::configure(const MojObject& conf)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	bool dbPrivate = false;
	if (!conf.get(_T("private"), dbPrivate))
		dbPrivate = MojEnvDefaultPrivate;
	bool logAutoRemove = false;
	if (!conf.get(_T("logAutoRemove"), logAutoRemove))
		logAutoRemove = MojEnvDefaultLogAutoRemove;
	bool found = false;
	MojString logDir;
	MojErr err = conf.get(_T("logDir"), logDir, found);
	MojErrCheck(err);
	MojUInt32 logFileSize = 0;
	err = conf.get(_T("logFileSize"), logFileSize, found);
	MojErrCheck(err);
	if (!found)
		logFileSize = MojEnvDefaultLogFileSize;
	MojUInt32 logRegionSize = 0;
	err = conf.get(_T("logRegionSize"), logRegionSize, found);
	MojErrCheck(err);
	if (!found)
		logRegionSize = MojEnvDefaultLogRegionSize;
	MojUInt32 cacheSize = 0;
	err = conf.get(_T("cacheSize"), cacheSize, found);
	MojErrCheck(err);
	if (!found)
		cacheSize = MojEnvDefaultCacheSize;
	MojUInt32 maxLocks = 0;
	err = conf.get(_T("maxLocks"), maxLocks, found);
	MojErrCheck(err);
	if (!found)
		maxLocks = MojEnvDefaultMaxLocks;
	MojUInt32 maxLockers = 0;
	err = conf.get(_T("maxLockers"), maxLockers, found);
	MojErrCheck(err);
	if (!found)
		maxLockers = MojEnvDefaultMaxLockers;
	MojUInt32 minKb = 0;
	err = conf.get(_T("checkpointMinKb"), minKb, found);
	MojErrCheck(err);
	if (!found)
		minKb = MojEnvDefaultCheckpointMinKb;
	MojUInt32 compactStepSize = 0;
	err = conf.get(_T("compactStepSize"), compactStepSize, found);
	MojErrCheck(err);
	if (!found)
		compactStepSize = MojEnvDefaultCompactStepSize;
	
	MojFlagSet(m_flags, DB_PRIVATE, dbPrivate);
	MojFlagSet(m_logFlags, DB_LOG_AUTO_REMOVE, logAutoRemove);
	m_logDir = logDir;
	m_logFileSize = logFileSize;
	m_cacheSize = cacheSize;
	m_checkpointMinKb = minKb;
	m_maxLocks = maxLocks;
	m_maxLockers = maxLockers;
	m_compactStepSize = compactStepSize;

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::open(const MojChar* dir)
{
	MojAssert(dir);
	MojAssert(!m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// lock env
	MojErr err = lockDir(dir);
	MojErrCheck(err);
	// create env
	DB_ENV* env = NULL;
	int dbErr = db_env_create(&env, 0);
	MojBdbErrCheck(dbErr, _T("db_env_create"));
	MojAssert(env);
	m_env = env;
	// set flags
	dbErr = m_env->set_flags(m_env, MojEnvFlags, 1);
	MojBdbErrCheck(dbErr, _T("dbenv->log_set_flags"));
	// configure logging
	if (!m_logDir.empty()) {
		dbErr = m_env->set_lg_dir(m_env, m_logDir);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_dir"));
		err = MojCreateDirIfNotPresent(m_logDir);
		MojErrCheck(err);
	}
	dbErr = m_env->log_set_config(m_env, m_logFlags, 1);
	MojBdbErrCheck(dbErr, _T("dbenv->log_set_config"));
	if (m_logFileSize > 0) {
		dbErr = m_env->set_lg_max(m_env, m_logFileSize);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_max"));
	}
	if (m_logRegionSize > 0) {
		dbErr = m_env->set_lg_regionmax(m_env, m_logRegionSize);
		MojBdbErrCheck(dbErr, _T("dbenv->set_lg_regionmax"));
	}
	// set cache size
	dbErr = m_env->set_cachesize(m_env, 0, m_cacheSize, 0);
	MojBdbErrCheck(dbErr, _T("dbenv->set_cachesize"));
	// set max txns
	dbErr = m_env->set_tx_max(m_env, MojTxnMax);
	MojBdbErrCheck(dbErr, _T("dbenv->set_tx_max"));
	// set deadlock policy
	dbErr = m_env->set_lk_detect(m_env, DB_LOCK_DEFAULT);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_detect"));
	// set err/msg callbacks
	m_env->set_errcall(m_env, errcall);
	m_env->set_msgcall(m_env, msgcall);
	// set max lock counts
	dbErr = m_env->set_lk_max_locks(m_env, m_maxLocks);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_locks"));
	dbErr = m_env->set_lk_max_objects(m_env, m_maxLocks);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_objects"));
	dbErr = m_env->set_lk_max_lockers(m_env, m_maxLockers);
	MojBdbErrCheck(dbErr, _T("dbenv->set_lk_max_lockers"));
	// open env
	dbErr = m_env->open(m_env, dir, MojEnvOpenFlags, MojDbFileMode);
	MojBdbErrCheck(dbErr, _T("dbenv->open"));

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;

	if (m_env) {
		// checkpoint before close
		errClose = checkpoint(0);
		MojErrAccumulate(err, errClose);
		int dbErr = m_env->close(m_env, 0);
		MojBdbErrAccumulate(err, dbErr, _T("dbenv->close"));
		m_env = NULL;
	}
	errClose = unlockDir();
	MojErrAccumulate(err, errClose);

	return err;
}

MojErr MojDbBerkeleyEnv::postCommit(MojSize updateSize)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// every N updates, check to see if we have enough log data
	// to justify a checkpoint
	if ((m_updateSize += (MojInt32) updateSize) >= (m_checkpointMinKb * 1024)) {
		MojErr err = tryCheckpoint(0);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::checkpoint(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojThreadGuard guard(m_checkpointMutex);
	MojErr err = checkpointImpl(minKB);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::tryCheckpoint(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojThreadGuard guard(m_checkpointMutex, false);
	if (guard.tryLock()) {
		MojErr err = checkpointImpl(minKB);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::checkpointImpl(MojUInt32 minKB)
{
	MojAssert(m_env);
	MojAssertMutexLocked(m_checkpointMutex);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_env->txn_checkpoint(m_env, minKB, 0, 0);
	MojBdbErrCheck(dbErr, _T("dbenv->txn_checkpoint"));
	// this is an approximation, so it's ok if we don't count a few
	m_updateSize = 0;
	if (MojFlagGet(m_logFlags, DB_LOG_AUTO_REMOVE)) {
		MojErr err = purgeLogs();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::purgeLogs()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	// get list of archivable logs
	char** logs = NULL;
	int dbErr = m_env->log_archive(m_env, &logs, MojLogArchiveFlags);
	MojBdbErrCheck(dbErr, _T("dbenv->log_archive"));
	// delete them all
	if (logs) {
		MojAutoFreePtr<char*> ap(logs);
		for (char** curLog = logs; *curLog; ++curLog) {
			MojErr err = MojUnlink(*curLog);
			MojErrCatch(err, MojErrNotFound);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyEnv::lockDir(const MojChar* path)
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

MojErr MojDbBerkeleyEnv::unlockDir()
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

MojErr MojDbBerkeleyEnv::translateErr(int dbErr)
{
	switch (dbErr) {
	case DB_LOCK_DEADLOCK:
		return MojErrDbDeadlock;
	case DB_NOTFOUND:
		return MojErrNotFound;
	case DB_RUNRECOVERY:
		return MojErrDbFatal;
	case DB_VERIFY_BAD:
		return MojErrDbVerificationFailed;
	}
	return (MojErr) dbErr;
}

void MojDbBerkeleyEnv::errcall(const DB_ENV *dbenv, const char *errpfx, const char *msg)
{
	MojLogError(MojDbBerkeleyEngine::s_log, "bdb: %s\n", msg);
}

void MojDbBerkeleyEnv::msgcall(const DB_ENV *dbenv, const char *msg)
{
	MojLogError(MojDbBerkeleyEngine::s_log, "bdb: %s\n", msg);
}

MojDbBerkeleyEngine::MojDbBerkeleyEngine()
: m_isOpen(false)
{
	MojLogTrace(s_log);
}

MojDbBerkeleyEngine::~MojDbBerkeleyEngine()
{
	MojLogTrace(s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyEngine::configure(const MojObject& conf)
{
	MojLogTrace(s_log);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::drop(const MojChar* path, MojDbStorageTxn* txn)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojAssert(m_isOpen);

	MojThreadGuard guard(m_dbMutex);

	// close sequences
	for (SequenceVec::ConstIterator i = m_seqs.begin(); i != m_seqs.end(); ++i) {
		MojErr err = (*i)->closeImpl();
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

MojErr MojDbBerkeleyEngine::open(const MojChar* path)
{
	MojAssert(path);
	MojAssert(!m_env.get() && !m_isOpen);
	MojLogTrace(s_log);

	MojRefCountedPtr<MojDbBerkeleyEnv> env(new MojDbBerkeleyEnv);
	MojAllocCheck(env.get());
	MojErr err = env->open(path);
	MojErrCheck(err);
	err = open(NULL, env.get());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::open(const MojChar* path, MojDbEnv* env)
{
    MojDbBerkeleyEnv* bEnv = static_cast<MojDbBerkeleyEnv *> (env);
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
	// open seqence db
	bool created = false;
	m_seqDb.reset(new MojDbBerkeleyDatabase);
	MojAllocCheck(m_seqDb.get());
	MojErr err = m_seqDb->open(MojEnvSeqDbName, this, created, NULL);
	MojErrCheck(err);
	// open index db
	m_indexDb.reset(new MojDbBerkeleyDatabase);
	MojAllocCheck(m_indexDb.get());
	err = m_indexDb->open(MojEnvIndexDbName, this, created, NULL);
	MojErrCheck(err);
	m_isOpen = true;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojErr err = MojErrNone;
	MojErr errClose = MojErrNone;

	// close dbs
	if (m_seqDb.get()) {
		errClose = m_seqDb->close();
		MojErrAccumulate(err, errClose);
		m_seqDb.reset();
	}
	if (m_indexDb.get()) {
		errClose = m_indexDb->close();
		MojErrAccumulate(err, errClose);
		m_indexDb.reset();
	}
	m_env.reset();
	m_isOpen = false;

	return err;
}

MojErr MojDbBerkeleyEngine::compact()
{
	const char * DatabaseRoot = "/var/db"; // FIXME: Should not be hard-coded, but so is the disk space monitor!

	struct statvfs statAtBeginning, statAfterCompact, statAtEnd;

	MojLogTrace(MojDbBerkeleyEngine::s_log);

	struct timeval totalStartTime = {0,0}, totalStopTime = {0,0};

	gettimeofday(&totalStartTime, NULL);

	memset(&statAtBeginning, '\0', sizeof(statAtBeginning));
	::statvfs(DatabaseRoot, &statAtBeginning);

	const int blockSize = (int)statAtBeginning.f_bsize;

	// checkpoint before compact
	MojErr err = m_env->checkpoint(0);
	MojErrCheck(err);

	memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
	::statvfs(DatabaseRoot, &statAfterCompact);
	
	int pre_compact_reclaimed_blocks = (int)(statAfterCompact.f_bfree - statAtBeginning.f_bfree);
	
	MojLogWarning(s_log, _T("Starting compact: Checkpoint freed %d bytes. Volume %s has %lu bytes free out of %lu bytes (%.1f full)\n"),
		pre_compact_reclaimed_blocks * blockSize,
		DatabaseRoot, statAfterCompact.f_bfree * blockSize,
		statAfterCompact.f_blocks * blockSize,
		(float)(statAfterCompact.f_blocks - statAfterCompact.f_bfree) * 100.0 / (float)statAfterCompact.f_blocks);


	// Retrieve setting for record count used to break up compact operations
	const int stepSize = m_env->compactStepSize();
	
	memset(&statAtBeginning, '\0', sizeof(statAtBeginning));
	::statvfs(DatabaseRoot, &statAtBeginning);

	int total_pages_examined = 0, total_pages_freed = 0, total_pages_truncated = 0;
	int max_pages_examined = 0, max_pages_freed = 0, max_pages_truncated = 0;
	int total_log_generation_blocks = 0, total_reclaimed_blocks = 0;
	int max_log_generation_blocks = 0, max_reclaimed_blocks = 0;

	int total_compact_time = 0, total_step_time = 0;
	int max_compact_time = 0, max_step_time = 0;
	
	int total_key_total = 0, total_value_total = 0;
	int max_key_total = 0, max_value_total = 0;

	MojThreadGuard guard(m_dbMutex);
	// call compact on each database
	for (DatabaseVec::ConstIterator i = m_dbs.begin(); i != m_dbs.end(); ++i) {
		DB* db = (*i)->impl();
		DB_COMPACT c_data;
		MojZero(&c_data, sizeof(c_data));

		DBC * dbc = NULL;
		int dbErr;
		DBT key1, key2;
		DBT value;

		memset(&key1, '\0', sizeof(key1));
		memset(&key2, '\0', sizeof(key2));
		memset(&value, '\0', sizeof(value));
		key1.flags = DB_DBT_REALLOC;
		key2.flags = DB_DBT_REALLOC;
		value.flags = DB_DBT_REALLOC;

		int key1_count = 0, key2_count = 0;

		dbErr = 0;

		// Continue compacting the database by chunks until we run into an error. If a stepSize
		// isn't configured, don't chunk it at all.
		while ((stepSize >= 1) && (dbErr == 0)) {

			// Construct key to step forward by a set number of records, to select the compact window.
			// We close the cursor after we've found the next key, so it won't keep a lock open that
			// could disrupt the compaction. Without locking, we might miss an insertion or deletion
			// happening between compactions, but that 
				   
			int key_total = 0, value_total = 0; // Tracked only for debugging purposes.
			   
			dbErr = db->cursor(db, NULL, &dbc, 0);

			if (dbErr == 0) {

				if (key1.data == NULL) {
					// Move the cursor to the beginning of the database
					dbErr = dbc->get(dbc, &key1, &value, DB_FIRST);
				
					key_total += key1.size;
					value_total += value.size;

					// discard key1, we don't want the key for the beginning
					if (key1.data)
						free(key1.data);
						
					key1.data = NULL;
					key1.size = 0;

				} else {
					// move the cursor to the location of the prior key.
					// If that exact key is missing, this should choose the
					// next one.
					dbErr = dbc->get(dbc, &key1, &value, DB_SET_RANGE);
				}
				
				int elapsedStepTimeMS = 0;
				
				if (dbErr == DB_NOTFOUND) {
					// If we didn't find a first key, the DB is presumably empty,
					// and we shouldn't search for the end key.
					
					dbErr = 0;

					if (key1.data)
						free(key1.data);
					key1.data = NULL;
					key1.size = 0;
					
					if (key2.data)
						free(key2.data);
					key2.data = NULL;
					key2.size = 0;

				} else if (dbErr == 0) {
				
					int count;
					// Move the cursor forward by the chosen stepSize.
					// May exit early with error DB_NOTFOUND, indicating end of database.
					
					struct timeval startTime = {0,0}, stopTime = {0,0};
	
					gettimeofday(&startTime, NULL);

					for (count = 0; (dbErr == 0) && (count < stepSize); count++) {
						dbErr = dbc->get(dbc, &key2, &value, DB_NEXT);
						
						key_total += key2.size;
						value_total += value.size;
					}
					
					key2_count = key1_count + count;
	
					if (dbErr == DB_NOTFOUND) {
						dbErr = 0;
						
						if (key2.data)
							free(key2.data);
						key2.data = NULL;
						key2.size = 0;
					}
					
					gettimeofday(&stopTime, NULL);
				
					elapsedStepTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
							  (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;
				}
				
				dbc->close(dbc);
				
				if (dbErr != 0)
					break;

				// Compact from key1 to key2. (The documentation says it starts at 'the
				// smallest key greater than or equal to the specified key', and ends at
				// 'the page with the smallest key greater than the specified key'. I don't
				// know exactly what that means regarding inclusivity, so this procedure may
				// not be fully compacting the pages which contain the keys.)


				MojLogInfo(s_log, _T("Compacting %s (partial from ~record %d to %d). Stepped over %d/%d bytes of keys/values in %dms.\n"), (*i)->m_name.data(),
					key1_count, key2_count,
					key_total, value_total,
					elapsedStepTimeMS);

			        struct statvfs statBeforeCompact, statAfterCompact, statAfterCheckpoint;
			        
				memset(&statBeforeCompact, '\0', sizeof(statBeforeCompact));
				::statvfs(DatabaseRoot, &statBeforeCompact);
				
				struct timeval startTime = {0,0}, stopTime = {0,0};
	
				gettimeofday(&startTime, NULL);

				MojZero(&c_data, sizeof(c_data));
				dbErr = db->compact(db, NULL, key1.data ? &key1 : NULL, key2.data ? &key2 : NULL, &c_data, DB_FREE_SPACE, NULL);
				
				gettimeofday(&stopTime, NULL);
				
				int elapsedCompactTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
						           (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;
				
		                MojLogInfo(s_log, _T("Compact stats of %s (partial from ~record %d to %d): time %dms, compact_deadlock=%d, compact_pages_examine=%d, compact_pages_free=%d, compact_levels=%d, compact_pages_truncated=%d\n"),
        		        	(*i)->m_name.data(),
        		        	key1_count, key2_count,
        		        	elapsedCompactTimeMS,
                			c_data.compact_deadlock, c_data.compact_pages_examine,	
               			 	c_data.compact_pages_free, c_data.compact_levels, c_data.compact_pages_truncated);
				
				total_compact_time += elapsedCompactTimeMS;
				if (elapsedCompactTimeMS > max_compact_time)
					max_compact_time = elapsedCompactTimeMS;
				total_step_time += elapsedStepTimeMS;
				if (elapsedStepTimeMS > max_step_time)
					max_step_time = elapsedStepTimeMS;
				
				total_key_total += key_total;
				if (key_total > max_key_total)
					max_key_total = key_total;
				total_value_total += value_total;
				if (value_total > max_value_total)
					max_value_total = value_total;
               			 	
				total_pages_examined += c_data.compact_pages_examine;
				if ((int)c_data.compact_pages_examine > max_pages_examined)
					max_pages_examined = c_data.compact_pages_examine;
				total_pages_freed += c_data.compact_pages_free;
				if ((int)c_data.compact_pages_free > max_pages_freed)
					max_pages_freed = c_data.compact_pages_free;
				total_pages_truncated += c_data.compact_pages_truncated;
				if ((int)c_data.compact_pages_truncated > max_pages_truncated)
					max_pages_truncated = c_data.compact_pages_truncated;

				memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
				::statvfs(DatabaseRoot, &statAfterCompact);
				
				int log_generation_blocks = (int)(statBeforeCompact.f_bfree - statAfterCompact.f_bfree);
			
				total_log_generation_blocks += log_generation_blocks;
				if (log_generation_blocks > max_log_generation_blocks)
					max_log_generation_blocks = log_generation_blocks;

				err = m_env->checkpoint(0);
				MojErrCheck(err);

				memset(&statAfterCompact, '\0', sizeof(statAfterCheckpoint));
				::statvfs(DatabaseRoot, &statAfterCheckpoint);
			
				int reclaimed_blocks = (int)(statAfterCheckpoint.f_bfree - statBeforeCompact.f_bfree);
			
				total_reclaimed_blocks += reclaimed_blocks;
				if (reclaimed_blocks > max_reclaimed_blocks)
					max_reclaimed_blocks = reclaimed_blocks;
			
				MojLogInfo(s_log, _T("Compact of %s (partial from ~record %d to %d) generated %d bytes of log data, ultimately reclaiming %d bytes after checkpoint.\n"),
					(*i)->m_name.data(),
					key1_count, key2_count,
					log_generation_blocks * blockSize,
					reclaimed_blocks * blockSize);
			
				// copy key2 over key1
				if (key1.data)
					free(key1.data);
				key1.data = key2.data;
				key1.size = key2.size;
				key2.data = NULL;
				key2.size = 0;
				key1_count = key2_count;
				
				// if key2 was empty, then we are done.
				if (key1.data == NULL)
					break;
				
			}
	

		}
			
		if (key1.data)
			free(key1.data);
		if (key2.data)
			free(key2.data);
		if (value.data)
			free(value.data);
		
		
		// If no step size was configured, fall back and do a complete compact. Do the same
		// if there was an error performing the chunked compaction. The complete compact risks
		// running out of disk space, but that's preferable to not compacting at all, which will
		// also likely eventually lead to running out of space.

		if (dbErr == DB_LOCK_DEADLOCK) {
			// But for deadlock, we should just give up, as this might
			// happen in normal use.
			MojBdbErrCheck(dbErr, _T("cursor and compact deadlocked"));
		}

		if ((stepSize <= 1) || (dbErr != 0)) {
			MojLogInfo(s_log, "Compacting %s\n", (*i)->m_name.data());	

		        struct statvfs statBeforeCompact, statAfterCompact, statAfterCheckpoint;
			        
			memset(&statBeforeCompact, '\0', sizeof(statBeforeCompact));
			::statvfs(DatabaseRoot, &statBeforeCompact);

			struct timeval startTime = {0,0}, stopTime = {0,0};
	
			gettimeofday(&startTime, NULL);

			MojZero(&c_data, sizeof(c_data));
		        dbErr = db->compact(db, NULL, NULL, NULL, &c_data, DB_FREE_SPACE, NULL);

			gettimeofday(&stopTime, NULL);
				
			int elapsedCompactTimeMS = (int)(stopTime.tv_sec - startTime.tv_sec) * 1000 +
					           (int)(stopTime.tv_usec - startTime.tv_usec) / 1000;

			total_compact_time += elapsedCompactTimeMS;
			if (elapsedCompactTimeMS > max_compact_time)
				max_compact_time = elapsedCompactTimeMS;
					           
       	        	MojLogInfo(s_log, "Compact stats of %s: time %dms, compact_deadlock=%d, compact_pages_examine=%d, compact_pages_free=%d, compact_levels=%d, compact_pages_truncated=%d\n",
                		(*i)->m_name.data(),
                		elapsedCompactTimeMS,
                		c_data.compact_deadlock, c_data.compact_pages_examine,
                		c_data.compact_pages_free, c_data.compact_levels, c_data.compact_pages_truncated);

			total_pages_examined += c_data.compact_pages_examine;
			if ((int)c_data.compact_pages_examine > max_pages_examined)
				max_pages_examined = c_data.compact_pages_examine;
			total_pages_freed += c_data.compact_pages_free;
			if ((int)c_data.compact_pages_free > max_pages_freed)
				max_pages_freed = c_data.compact_pages_free;
			total_pages_truncated += c_data.compact_pages_truncated;
			if ((int)c_data.compact_pages_truncated > max_pages_truncated)
				max_pages_truncated = c_data.compact_pages_truncated;

			memset(&statAfterCompact, '\0', sizeof(statAfterCompact));
			::statvfs(DatabaseRoot, &statAfterCompact);
			
			int log_generation_blocks = (int)(statBeforeCompact.f_bfree - statAfterCompact.f_bfree);
			
			total_log_generation_blocks += log_generation_blocks;
			if (log_generation_blocks > max_log_generation_blocks)
				max_log_generation_blocks = log_generation_blocks;

			err = m_env->checkpoint(0);
			MojErrCheck(err);

			memset(&statAfterCompact, '\0', sizeof(statAfterCheckpoint));
			::statvfs(DatabaseRoot, &statAfterCheckpoint);
			
			int reclaimed_blocks = (int)(statAfterCheckpoint.f_bfree - statBeforeCompact.f_bfree);
			
			total_reclaimed_blocks += reclaimed_blocks;
			if (reclaimed_blocks > max_reclaimed_blocks)
				max_reclaimed_blocks = reclaimed_blocks;
			
			MojLogInfo(s_log, "Compact of %s generated %d bytes of log data, ultimately reclaiming %d bytes after checkpoint.\n",
				(*i)->m_name.data(),
				log_generation_blocks * blockSize,
				reclaimed_blocks * blockSize);
		}
		MojBdbErrCheck(dbErr, _T("db->compact"));

	}
	guard.unlock();
	
	
	gettimeofday(&totalStopTime, NULL);
				
	int elapsedTotalMS = (int)(totalStopTime.tv_sec - totalStartTime.tv_sec) * 1000 +
		             (int)(totalStopTime.tv_usec - totalStartTime.tv_usec) / 1000;

	memset(&statAtEnd, '\0', sizeof(statAtEnd));
	::statvfs(DatabaseRoot, &statAtEnd);
	
	int compact_freed_blocks = (int)(statAtEnd.f_bfree - statAtBeginning.f_bfree);

	MojLogWarning(s_log, _T("During compact: %d db pages examined (max burst %d), %d db pages freed (max burst %d), "
			     "%d db pages truncated (max burst %d), "
	                     "%d log bytes created by compacts (max burst %d), "
	                     "%d bytes reclaimed by checkpoints (max burst %d), "
	                     "%d bytes of keys stepped over (max burst %d), "
	                     "%d bytes of values stepped over (max burst %d), "
	                     "%dms spent in stepping (max burst %dms), "
	                     "%dms spent in compact (max burst %dms)\n"),
	                     total_pages_examined, max_pages_examined, total_pages_freed, max_pages_freed,
	                     total_pages_truncated, max_pages_truncated,
	                     total_log_generation_blocks * blockSize, max_log_generation_blocks * blockSize,
	                     total_reclaimed_blocks * blockSize, max_reclaimed_blocks * blockSize,
	                     total_key_total, max_key_total,
	                     total_value_total, max_value_total,
	                     total_step_time, max_step_time,
	                     total_compact_time, max_step_time
	                     );

	MojLogWarning(s_log, _T("Compact complete: took %dms, freed %d bytes (including pre-checkpoint of %d bytes). Volume %s has %lu bytes free out of %lu bytes (%.1f full)\n"),
		elapsedTotalMS,
		compact_freed_blocks * blockSize,
		pre_compact_reclaimed_blocks * blockSize,
		DatabaseRoot,
		statAfterCompact.f_bfree * blockSize,
		 statAfterCompact.f_blocks * blockSize,
		 (float)(statAfterCompact.f_blocks - statAfterCompact.f_bfree) * 100.0 / (float)statAfterCompact.f_blocks);

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojAssert(!txnOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyTxn> txn(new MojDbBerkeleyTxn());
	MojAllocCheck(txn.get());
	MojErr err = txn->begin(this);
	MojErrCheck(err);
	txnOut = txn;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut)
{
	MojAssert(name && !dbOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyDatabase> db(new MojDbBerkeleyDatabase());
	MojAllocCheck(db.get());
	bool created = false;
	MojErr err = db->open(name, this, created, txn);
	MojErrCheck(err);
	dbOut = db;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut)
{
	MojAssert(name && !seqOut.get());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleySeq> seq(new MojDbBerkeleySeq());
	MojAllocCheck(seq.get());
	MojErr err = seq->open(name, m_seqDb.get());
	MojErrCheck(err);
	seqOut = seq;

	return MojErrNone;
}

MojErr MojDbBerkeleyEngine::addDatabase(MojDbBerkeleyDatabase* db)
{
	MojAssert(db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojThreadGuard guard(m_dbMutex);

	return m_dbs.push(db);
}

MojErr MojDbBerkeleyEngine::removeDatabase(MojDbBerkeleyDatabase* db)
{
	MojAssert(db);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
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

MojErr MojDbBerkeleyEngine::addSeq(MojDbBerkeleySeq* seq)
{
	MojAssert(seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojThreadGuard guard(m_dbMutex);

	return m_seqs.push(seq);
}

MojErr MojDbBerkeleyEngine::removeSeq(MojDbBerkeleySeq* seq)
{
	MojAssert(seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
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

MojDbBerkeleyIndex::MojDbBerkeleyIndex()
: m_primaryDb(NULL)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyIndex::~MojDbBerkeleyIndex()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err =  close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyIndex::open(const MojObject& id, MojDbBerkeleyDatabase* db, MojDbStorageTxn* txn)
{
	MojAssert(db && db->engine());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_id = id;
	m_db.reset(db->engine()->indexDb());
	m_primaryDb.reset(db);

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_db.reset();
	m_primaryDb.reset();

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::drop(MojDbStorageTxn* txn)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
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

MojErr MojDbBerkeleyIndex::stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyCursor cursor;
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

MojErr MojDbBerkeleyIndex::insert(const MojDbKey& key, MojDbStorageTxn* txn)
{
	MojAssert(txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem keyItem;
	keyItem.fromBytesNoCopy(key.data(), key.size());
	MojDbBerkeleyItem valItem;  // ???
	MojErr err = m_db->put(keyItem, valItem, txn, true);
#ifdef MOJ_DEBUG
	char s[1024];  
	int size1 = keyItem.size();
	int size2 = valItem.size();
	MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s); 
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
	MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbindexinsert: %s; keylen: %d, key: %s ; vallen = %d; err = %d\n"), 
					m_db->m_name.data(), size1, s, size2, err);		
#endif
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::del(const MojDbKey& key, MojDbStorageTxn* txn)
{
	MojAssert(txn);
	MojAssert(isOpen());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojDbBerkeleyItem keyItem;
	keyItem.fromBytesNoCopy(key.data(), key.size());

	bool found = false;
	MojErr err = m_db->del(keyItem, found, txn);

#ifdef MOJ_DEBUG
	char s[1024];  
	int size1 = keyItem.size();
	MojErr err2 = MojByteArrayToHex(keyItem.data(), size1, s); 
	MojErrCheck(err2);
	if (size1 > 16)	// if the object-id is in key
		strncat(s, (char *)(keyItem.data()) + (size1 - 17), 16);
	MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbindexdel: %s; keylen: %d, key: %s ; err = %d\n"), m_db->m_name.data(), size1, s, err);
	if (!found)
		MojLogWarning(MojDbBerkeleyEngine::s_log, _T("bdbindexdel_warn: not found: %s \n"), s);	
#endif

	MojErrCheck(err);
	if (!found) {	  
		//MojErrThrow(MojErrDbInconsistentIndex);   // fix this to work around to deal with out of sync indexes
		MojErrThrow(MojErrInternalIndexOnDel);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut)
{
	MojAssert(isOpen());
	MojAssert(plan.get() && txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojRefCountedPtr<MojDbBerkeleyQuery> storageQuery(new MojDbBerkeleyQuery());
	MojAllocCheck(storageQuery.get());
	MojErr err = storageQuery->open(m_db.get(), m_primaryDb.get(), plan, txn);
	MojErrCheck(err);
	queryOut = storageQuery;

	return MojErrNone;
}

MojErr MojDbBerkeleyIndex::beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	return m_primaryDb->beginTxn(txnOut);
}

MojDbBerkeleyItem::MojDbBerkeleyItem()
: m_free(MojFree)
{
	MojZero(&m_dbt, sizeof(DBT));
	m_dbt.flags = DB_DBT_REALLOC;
}

MojErr MojDbBerkeleyItem::kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine)
{
	MojErr err = m_header.read(kindEngine);
	MojErrCheck(err);

	kindIdOut = m_header.kindId();

	return MojErrNone;
}

MojErr MojDbBerkeleyItem::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected) const
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

void MojDbBerkeleyItem::id(const MojObject& id)
{
	m_header.reset();
	m_header.id(id);
	m_header.reader().data(data(), size());
}

void MojDbBerkeleyItem::clear()
{
	freeData();
	m_free = MojFree;
	m_dbt.data = NULL;
	m_dbt.size = 0;
}

bool MojDbBerkeleyItem::hasPrefix(const MojDbKey& prefix) const
{
	return (size() >= prefix.size() &&
			MojMemCmp(data(), prefix.data(), prefix.size()) == 0);
}

MojErr MojDbBerkeleyItem::toArray(MojObject& arrayOut) const
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

MojErr MojDbBerkeleyItem::toObject(MojObject& objOut) const
{
	MojObjectBuilder builder;
	MojErr err = MojObjectReader::read(builder, data(), size());
	MojErrCheck(err);
	objOut = builder.object();

	return MojErrNone;
}

void MojDbBerkeleyItem::fromBytesNoCopy(const MojByte* bytes, MojSize size)
{
	MojAssert(bytes || size == 0);
	MojAssert(size <= MojUInt32Max);
	
	setData(const_cast<MojByte*> (bytes), size, NULL);
}

MojErr MojDbBerkeleyItem::fromBuffer(MojBuffer& buf)
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

MojErr MojDbBerkeleyItem::fromBytes(const MojByte* bytes, MojSize size)
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

MojErr MojDbBerkeleyItem::fromObject(const MojObject& obj)
{
	MojObjectWriter writer;
	MojErr err = obj.visit(writer);
	MojErrCheck(err);
	err = fromBuffer(writer.buf());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyItem::fromObjectVector(const MojVector<MojObject>& vec)
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

void MojDbBerkeleyItem::freeData()
{
	if (m_free)
		m_free(m_dbt.data);
}

void MojDbBerkeleyItem::setData(MojByte* bytes, MojSize size, void (*free)(void*))
{
	MojAssert(bytes);
	freeData();
	m_free = free;
	m_dbt.size = (MojUInt32) size;
	m_dbt.data = bytes;

	m_header.reader().data(bytes, size);
}

MojDbBerkeleySeq::~MojDbBerkeleySeq()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleySeq::open(const MojChar* name, MojDbBerkeleyDatabase* db)
{
	MojAssert(!m_seq);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	m_db = db;
	MojString strName;
	MojErr err = strName.assign(name);
	MojErrCheck(err);
	MojDbBerkeleyItem dbt;
	err = dbt.fromObject(strName);
	MojErrCheck(err);
	DB_SEQUENCE* seq = NULL;
	int dbErr = db_sequence_create(&seq, db->impl(), 0);
	MojBdbErrCheck(dbErr, _T("db_sequence_create"));
	MojAssert(seq);
	m_seq = seq;
	dbErr = m_seq->set_cachesize(m_seq, MojSeqCacheSize);
	MojBdbErrCheck(dbErr, _T("seq->set_cachesize"));
	dbErr = m_seq->open(m_seq, NULL, dbt.impl(), MojSeqOpenFlags);
	MojBdbErrCheck(dbErr, _T("seq->open"));

	// keep a reference to this seq
	err = db->engine()->addSeq(this);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleySeq::close()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	MojErr err = MojErrNone;
	if (m_seq) {
		MojErr errClose = closeImpl();
		MojErrAccumulate(err, errClose);
		m_db->engine()->removeSeq(this);
		m_db = NULL;
	}
	return err;
}

MojErr MojDbBerkeleySeq::closeImpl()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_seq->close(m_seq, 0);
	m_seq = NULL;
	MojBdbErrCheck(dbErr, _T("seq->close"));

	return MojErrNone;
}

MojErr MojDbBerkeleySeq::get(MojInt64& valOut)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	db_seq_t id = 0;
	int dbErr = m_seq->get(m_seq, NULL, 1, &id, MojSeqGetFlags);
	MojBdbErrCheck(dbErr, _T("seq->get"));
	valOut = (MojInt64) id;

	return MojErrNone;
}

MojDbBerkeleyTxn::MojDbBerkeleyTxn()
: m_engine(NULL),
  m_txn(NULL),
  m_updateSize(0)
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);
}

MojDbBerkeleyTxn::~MojDbBerkeleyTxn()
{
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	if (m_txn) {
		abort();
	}
}

MojErr MojDbBerkeleyTxn::begin(MojDbBerkeleyEngine* eng)
{
	MojAssert(!m_txn && eng && eng->env());
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	DB_ENV* dbEnv = eng->env()->impl();
	DB_TXN* txn = NULL;
	int dbErr = dbEnv->txn_begin(dbEnv, NULL, &txn, MojTxnBeginFlags);
	MojBdbErrCheck(dbErr, _T("env->txn_begin"));
	MojAssert(txn);
	m_txn = txn;
	m_engine = eng;

	return MojErrNone;
}

MojErr MojDbBerkeleyTxn::commitImpl()
{
	MojAssert(m_txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);

	int dbErr = m_txn->commit(m_txn, 0);
	m_txn = NULL;
	MojBdbErrCheck(dbErr, _T("txn->commit"));

	if (m_updateSize != 0) {
		MojErr err = m_engine->env()->postCommit(m_updateSize);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyTxn::abort()
{
	MojAssert(m_txn);
	MojLogTrace(MojDbBerkeleyEngine::s_log);
	MojLogWarning(MojDbBerkeleyEngine::s_log, _T("bdb: transaction aborted"));

	int dbErr = m_txn->abort(m_txn);
	m_txn = NULL;
	MojBdbErrCheck(dbErr, _T("txn->abort"));

	return MojErrNone;
}
