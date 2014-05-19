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

class MojDbSandwichEnvTxn final : public MojDbStorageTxn
{
public:
    typedef leveldb::SandwichDB<leveldb::TxnDB<leveldb::BottomDB>> BackendDb;

    MojDbSandwichEnvTxn(MojDbSandwichEngine::BackendDb& db)
        : m_txn(db.ref<leveldb::TxnDB>())
    { }

    ~MojDbSandwichEnvTxn()
    { abort(); }

    MojErr abort() override;

    bool isValid() override
    { return true; }

    BackendDb::Part ref(MojDbSandwichEngine::BackendDb::Part &db)
    { return db.ref(m_txn); }

private:
    MojErr commitImpl() override;

    BackendDb m_txn;
};

#endif
