/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
 *
 * LICENSE@@@
****************************************************************/

/****************************************************************
*  @file MojDbLevelTxn.h
****************************************************************/

#ifndef __MOJDBLEVELTXN_H
#define __MOJDBLEVELTXN_H

#include <map>
#include <set>
#include <list>
#include <string>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <core/MojString.h>
#include <core/MojErr.h>
#include <db/MojDbStorageEngine.h>

namespace leveldb
{
    class DB;
}

class MojDbLevelEngine;
class MojDbLevelTableTxn;

class MojDbLevelAbstractTxn : public MojDbStorageTxn
{
public:
    virtual MojDbLevelTableTxn &tableTxn(leveldb::DB *db) = 0;
};

// Note we implement kinda dirty read with shadowing by local wirtes
class MojDbLevelTableTxn : public MojDbLevelAbstractTxn
{
public:
    MojDbLevelTableTxn() : m_db(NULL)
    {}

    ~MojDbLevelTableTxn()
    { if (m_db) abort(); }

    MojErr begin(
            leveldb::DB* db,
            const leveldb::WriteOptions& options = leveldb::WriteOptions()
            );

    MojErr abort();

    bool isValid() { return (m_db == NULL); }
    leveldb::DB *db() { return m_db; }

    MojDbLevelTableTxn &tableTxn(leveldb::DB *db);

    void Put(const leveldb::Slice& key,
            const leveldb::Slice& val);

    leveldb::Status Get(const leveldb::Slice& key,
                        std::string& val);

    void Delete(const leveldb::Slice& key);

private:
    MojErr commitImpl();

    void cleanup();

    leveldb::WriteBatch m_writeBatch;

    // where and how to write this batch
    leveldb::DB *m_db;
    leveldb::WriteOptions m_writeOptions;

    // local view for pending writes
    typedef std::map<std::string, std::string> PendingValues;
    typedef std::set<std::string> PendingDeletes;
    PendingValues m_pendingValues;
    PendingDeletes m_pendingDeletes;
};

class MojDbLevelEnvTxn : public MojDbLevelAbstractTxn
{
public:
    ~MojDbLevelEnvTxn()
    { abort(); }

    MojErr begin(MojDbLevelEngine* eng);
    MojErr abort();

    bool isValid() { return true; }

    MojDbLevelTableTxn &tableTxn(leveldb::DB *db);

private:
    MojErr commitImpl();

    typedef std::list<MojSharedPtr<MojDbLevelTableTxn> > TableTxns;
    TableTxns m_tableTxns;
};

#endif
