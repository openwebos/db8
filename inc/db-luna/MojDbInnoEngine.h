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


#ifndef MOJDBINNOENGINE_H_
#define MOJDBINNOENGINE_H_

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"
#include "embedded_innodb-1.0/innodb.h"

#define MojInnoErrCheck(E, FNAME)				if (E != DB_SUCCESS) MojErrThrowMsg(MojDbInnoEngine::translateErr(E), _T("%s: %s"), FNAME, ib_strerror(E))
#define MojInnoErrAccumulate(EACC, E, FNAME)	if (E != DB_SUCCESS) MojErrAccumulate(EACC, MojDbInnoEngine::translateErr(E))
#define MojInnoTxnFromStorageTxn(TXN)			((TXN) ? static_cast<MojDbInnoTxn*>(TXN)->impl() : NULL)

class MojDbInnoCursor;
class MojDbInnoDatabase;
class MojDbInnoEngine;
class MojDbInnoIndex;
class MojDbInnoItem;
class MojDbInnoQuery;
class MojDbInnoSeq;
class MojDbInnoTuple;
class MojDbInnoTxn;

class MojDbInnoCursor : public MojNoCopy
{
public:
	MojDbInnoCursor();
	~MojDbInnoCursor();

	MojErr open(MojDbInnoDatabase& db, MojDbStorageTxn* txn);
	MojErr close();
	MojErr first(bool& foundOut);
	MojErr last(bool& foundOut);
	MojErr next(bool& foundOut);
	MojErr prev(bool& foundOut);
	MojErr seek(const MojDbInnoTuple& tuple, ib_srch_mode_t mode, int& compOut, bool& foundOut);

	MojErr del();
	MojErr del(const MojDbInnoTuple& tuple, bool& foundOut);
	MojErr get(MojDbInnoTuple& tupleOut, bool& foundOut);
	MojErr get(const MojDbInnoTuple& searchTuple, MojDbInnoTuple& tupleOut, bool& foundOut);
	MojErr insert(const MojDbInnoTuple& tuple);
	MojErr update(const MojDbInnoTuple& oldTuple, const MojDbInnoTuple& newTuple);
	MojErr truncate(ib_id_t& newIdOut);

	ib_crsr_t impl() { return m_crsr; }

private:
	ib_crsr_t m_crsr;
};

class MojDbInnoTuple : private MojNoCopy
{
public:
	static const ib_ulint_t KeyCol = 0;
	static const ib_ulint_t ValCol = 1;

	MojDbInnoTuple() : m_tpl(NULL) {}
	~MojDbInnoTuple();

	MojErr openReadWrite(MojDbInnoCursor& cur);
	MojErr openSearch(MojDbInnoCursor& cur);
	MojErr set(const guint8* data, gsize size, ib_ulint_t col);
	MojErr set(const MojObject& val, ib_ulint_t col);
	MojErr setKey(const guint8* data, gsize size) { return set(data, size, KeyCol); }
	MojErr setKey(const MojObject& key) { return set(key, KeyCol); }
	MojErr setKeyVal(const MojObject& key, const MojObject& val);
	MojErr visit(MojObjectVisitor& visitor) const;

	void get(const guint8*& dataOut, gsize& sizeOut, ib_ulint_t col);
	void getKey(const guint8*& dataOut, gsize& sizeOut) { return get(dataOut, sizeOut, KeyCol); }

	ib_tpl_t impl() { return m_tpl; }
	const ib_tpl_t impl() const { return m_tpl; }

protected:
	ib_tpl_t m_tpl;
};

class MojDbInnoDatabase : public MojDbStorageDatabase
{
public:
	static const MojChar* const KeyColName;
	static const MojChar* const ValColName;

	MojDbInnoDatabase();
	~MojDbInnoDatabase();

	MojErr open(const MojChar* dbName, MojDbInnoEngine* eng, MojDbStorageTxn* txn, bool valCol, bool& createdOut);
	virtual MojErr close();
	virtual MojErr drop(MojDbStorageTxn* txn);
	virtual MojErr insert(const MojObject& id, const MojObject& val, MojDbStorageTxn* txn);
	virtual MojErr update(const MojObject& id, const MojObject& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn);
	virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut);
	virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojAutoPtr<MojDbStorageItem>& itemOut);
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojAutoPtr<MojDbStorageQuery>& queryOut);
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
	virtual MojErr openIndex(const MojChar* name, const MojChar* idxJson, MojDbStorageTxn* txn, MojAutoPtr<MojDbStorageIndex>& indexOut, bool& createdOut);

	ib_id_t id() { return m_id; }
	MojDbInnoEngine* engine() { return m_eng; }
	const MojString& name() { return m_name; }

private:
	void postUpdate(MojDbStorageTxn* txn);

	ib_id_t m_id;
	MojDbInnoEngine* m_eng;
	MojString m_name;
	MojVector<MojString> m_primaryProps;
};

class MojDbInnoEngine : public MojDbStorageEngine
{
public:
	static const MojChar* const SequenceDbName;
	static const gsize CfgAdditionalMemPoolSizeDefault;
	static const gsize CfgBufferPoolSizeDefault;
	static const gsize CfgLogFileSizeDefault;
	static const guint32 CfgFileIoThreadsDefault;
	static const guint32 CfgReadIoThreadsDefault;
	static const guint32 CfgWriteIoThreadsDefault;
	static MojLogger s_log;

	MojDbInnoEngine();
	~MojDbInnoEngine();

	virtual MojErr configure(const MojObject& conf);
	virtual MojErr compact();
	virtual MojErr drop(const MojChar* path);
	virtual MojErr open(const MojChar* path);
	virtual MojErr close();
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
	virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojAutoPtr<MojDbStorageDatabase>& dbOut);
	virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojAutoPtr<MojDbStorageSeq>& seqOut);

	const MojString& path() const { return m_path; }
	const MojString& name() const { return m_name; }
	static MojErr translateErr(ib_err_t ibErr);

private:
	typedef MojVector<MojDbInnoDatabase*> DatabaseVec;

	MojErr create(const MojChar* dir);
	static int log(ib_msg_stream_t stream, const char* format, ...);

	MojString m_path;
	MojString m_name;
	MojDbInnoDatabase m_sequences;
	gsize m_cfgAdditionalMemPoolSize;
	gsize m_cfgBufferPoolSize;
	gsize m_cfgLogFileSize;
	guint32 m_cfgFileIoThreads;
	guint32 m_cfgReadIoThreads;
	guint32 m_cfgWriteIoThreads;
	bool m_started;
	static bool s_started;
};

class MojDbInnoIndex : public MojDbStorageIndex
{
public:
	MojDbInnoIndex();
	virtual ~MojDbInnoIndex();

	MojErr open(const MojChar* name, const MojChar* idxJson, MojDbInnoDatabase* db, MojDbStorageTxn* txn, bool& createdOut);
	virtual MojErr close();
	virtual MojErr drop(MojDbStorageTxn* txn);
	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn);
	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn);
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojAutoPtr<MojDbStorageQuery>& queryOut);
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);

private:
	bool isOpen() const { return m_primaryDb != NULL; }

	MojDbInnoDatabase* m_primaryDb;
	MojDbInnoDatabase m_db;
	MojString m_jsonFilePath;
};

class MojDbInnoItem : public MojDbStorageItem
{
public:
	MojDbInnoItem();
	virtual MojErr close();
	virtual MojErr visit(MojObjectVisitor& visitor) const;

	MojErr open(MojDbInnoDatabase* db, MojDbStorageTxn* txn, bool forRead, bool forUpdate = false);
	MojDbInnoCursor& cursor() { return m_cursor; }
	MojDbInnoTuple& tuple() { return m_tuple; }

protected:
	MojDbInnoCursor m_cursor;
	MojDbInnoTuple m_tuple;
};

class MojDbInnoSeq : public MojDbStorageSeq
{
public:
	static const gint64 Increment = 50;

	MojDbInnoSeq();
	~MojDbInnoSeq();

	MojErr open(const MojChar* name, MojDbInnoDatabase* db, MojDbStorageTxn* txn);
	virtual MojErr close();
	virtual MojErr get(gint64& valOut);

private:
	void reset();
	MojErr incrementSaved();

	MojThreadMutex m_mutex;
	MojObject m_key;
	gint64 m_val;
	gint64 m_savedVal;
	MojDbInnoDatabase* m_db;
};

class MojDbInnoTableSchema : private MojNoCopy
{
public:
	static const ib_tbl_fmt_t TableFormat;
	static const gsize PageSize;
	static const gsize MaxKeySize;

	MojDbInnoTableSchema() : m_sch(NULL) {}
	~MojDbInnoTableSchema();

	MojErr open(const char* tableName);
	MojErr close();
	MojErr addCol(const char* name, ib_col_type_t type, ib_col_attr_t attr);
	MojErr addIndex(const char* colName, bool primary);
	ib_tbl_sch_t impl() { return m_sch; }

private:
	ib_tbl_sch_t m_sch;
};

class MojDbInnoTxn : public MojDbStorageTxn
{
public:
	static const ib_trx_level_t Level;

	MojDbInnoTxn() : m_txn(NULL), m_schemaLocked(false) {}
	~MojDbInnoTxn();

	virtual MojErr abort();
	MojErr begin();
	MojErr lockSchema(bool exclusive);
	MojErr unlockSchema();
	MojErr releaseSchemaLock();
	void schemaLocked(bool locked) { m_schemaLocked = locked; }
	ib_trx_t impl() { return m_txn; }

private:
	virtual MojErr commitImpl();

	ib_trx_t m_txn;
	bool m_schemaLocked;
};

#endif /* MOJDBINNOENGINE_H_ */
