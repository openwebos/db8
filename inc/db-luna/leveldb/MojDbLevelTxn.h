/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics, Inc.
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
class MojDbLevelTxnIterator;

class MojDbLevelAbstractTxn : public MojDbStorageTxn
{
public:
    virtual MojDbLevelTableTxn &tableTxn(leveldb::DB *db) = 0;
};

// Note we implement kinda dirty read with shadowing by local wirtes
class MojDbLevelTableTxn
{
public:
    MojDbLevelTableTxn();
    ~MojDbLevelTableTxn();

    MojErr begin(
            leveldb::DB* db,
            const leveldb::WriteOptions& options = leveldb::WriteOptions()
            );

    MojErr abort();

    bool isValid() { return (m_db == NULL); }
    leveldb::DB *db() { return m_db; }

    MojDbLevelTableTxn &tableTxn(leveldb::DB *db);

    // operations
    void Put(const leveldb::Slice& key,
            const leveldb::Slice& val);

    leveldb::Status Get(const leveldb::Slice& key,
                        std::string& val);

    void Delete(const leveldb::Slice& key);
    MojDbLevelTxnIterator* createIterator();
    void detach(MojDbLevelTxnIterator *it);

    MojErr commitImpl();

private:
    void cleanup();

    // where and how to write this batch
    leveldb::DB *m_db;
    leveldb::WriteOptions m_writeOptions;

    // local view for pending writes
    typedef std::map<std::string, std::string> PendingValues;
    typedef std::set<std::string> PendingDeletes;
    PendingValues m_pendingValues;
    PendingDeletes m_pendingDeletes;
    std::set<MojDbLevelTxnIterator*> m_iterators;

    friend class MojDbLevelTxnIterator;
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

// Note: Current workaround uses EnvTxn not only for Env transaction but for
//       Database as well. Correct way would be to use different classes for that.
// TODO:
//   - We should rename TableTxn to TableData since it no more belongs to
//     StorageTxn
//   - Introduce new TableTxn that will simply return single tableData for
//     a database it was constructed with

#endif

