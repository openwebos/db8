/* @@@LICENSE
*
* Copyright (c) 2009-2014 LG Electronics, Inc.
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
#include <leveldb/txn_db.hpp>
#include <leveldb/bottom_db.hpp>

#include <core/MojString.h>
#include <core/MojErr.h>
#include <db/MojDbStorageEngine.h>
#include "MojDbSandwichEngine.h"

namespace leveldb
{
    class DB;
}

class MojDbSandwichEngine;
class MojDbSandwichTableTxn;
//class MojDbSandwichTxnIterator;

// Note we implement kinda dirty read with shadowing by local wirtes
class MojDbSandwichTableTxn
{
public:
    typedef leveldb::SandwichDB<leveldb::TxnDB<leveldb::BottomDB>> BackendDb;
    MojDbSandwichTableTxn(BackendDb &txn, const BackendDb::Part &db);
    ~MojDbSandwichTableTxn();

    //MojErr begin(MojDbSandwichEngine::BackendDb::Part &db);

    MojErr abort();

    bool isValid() { return m_db.Valid(); }
    BackendDb::Part* db() { return &m_db; }

    // operations
    void Put(const leveldb::Slice& key,
            const leveldb::Slice& val);

    leveldb::Status Get(const leveldb::Slice& key,
                        std::string& val);

    void Delete(const leveldb::Slice& key);
    std::unique_ptr<leveldb::Iterator> createIterator();
    //void detach(MojDbSandwichTxnIterator *it);

    MojErr commitImpl();

private:
    void cleanup();

    // where and how to write this batch
    BackendDb& m_txn;
    BackendDb::Part m_db;

    // local view for pending writes
    //typedef std::map<std::string, std::string> PendingValues;
    //typedef std::set<std::string> PendingDeletes;
    //PendingValues m_pendingValues;
    //PendingDeletes m_pendingDeletes;
    //std::set<MojDbSandwichTxnIterator*> m_iterators;

    //friend class MojDbSandwichTxnIterator;
};

class MojDbSandwichEnvTxn final : public MojDbStorageTxn
{
public:
    MojDbSandwichEnvTxn(MojDbSandwichEngine::BackendDb& db)
        : m_txn(db.ref<leveldb::TxnDB>())
    {

    }

    ~MojDbSandwichEnvTxn()
    { abort(); }

    MojErr begin(MojDbSandwichEngine* eng);
    MojErr abort();

    bool isValid() { return true; }

    MojDbSandwichTableTxn &tableTxn(MojDbSandwichEngine::BackendDb::Part &db);

private:
    MojErr commitImpl();

    typedef std::map<MojDbSandwichEngine::BackendDb::Part *, MojSharedPtr<MojDbSandwichTableTxn> > TableTxns;
    TableTxns m_tableTxns;
    MojDbSandwichTableTxn::BackendDb m_txn;
};

// Note: Current workaround uses EnvTxn not only for Env transaction but for
//       Database as well. Correct way would be to use different classes for that.
// TODO:
//   - We should rename TableTxn to TableData since it no more belongs to
//     StorageTxn
//   - Introduce new TableTxn that will simply return single tableData for
//     a database it was constructed with

#endif
