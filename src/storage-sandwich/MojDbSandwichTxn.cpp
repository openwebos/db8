/* @@@LICENSE
*
* Copyright (c) 2013-2014 LG Electronics, Inc.
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

#include "MojDbSandwichEngine.h"
#include "MojDbSandwichTxn.h"
#include "db-luna/leveldb/defs.h"
#include "MojDbSandwichTxnIterator.h"

MojDbSandwichTableTxn::MojDbSandwichTableTxn() : m_db(NULL)
{
}

MojDbSandwichTableTxn::~MojDbSandwichTableTxn()
{
    if (m_db)
        abort();
    cleanup();
}

void MojDbSandwichTableTxn::detach(MojDbSandwichTxnIterator *it)
{
    MojAssert( it );
    size_t detached = m_iterators.erase(it);
    MojAssert( detached == 1 );
}

MojDbSandwichTxnIterator* MojDbSandwichTableTxn::createIterator()
{
    MojAssert( m_db );
    MojDbSandwichTxnIterator* it = new MojDbSandwichTxnIterator(this);
    m_iterators.insert(it);
    return it;
}

// class MojDbSandwichTableTxn
MojErr MojDbSandwichTableTxn::begin(MojDbSandwichEngine::BackendDb::Part &db)
{
    MojAssert( m_iterators.empty() );

    // TODO: mutex and lock-file serialization
    m_db = &db;

    return MojErrNone;
}

MojErr MojDbSandwichTableTxn::abort()
{
    MojAssert( m_db ); // database should exist

    cleanup();
    return MojErrNone;
}

void MojDbSandwichTableTxn::Put(const leveldb::Slice& key,
                             const leveldb::Slice& val)
{
    // populate local view
    std::string keyStr = key.ToString();

    m_pendingValues[keyStr] = val.ToString();
    m_pendingDeletes.erase(keyStr); // if hidden before

    // notify all iterators
    for(std::set<MojDbSandwichTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->notifyPut(key);
    }

}

leveldb::Status MojDbSandwichTableTxn::Get(const leveldb::Slice& key,
                                        std::string& val)
{
    std::string keyStr = key.ToString();

    // for keys deleted in this transaction
    if (m_pendingDeletes.count(keyStr) > 0)
    {
        return leveldb::Status::NotFound("Deleted inside transaction");
    }

    // for keys added in this transaction
    PendingValues::const_iterator it = m_pendingValues.find(key.ToString());
    if (it != m_pendingValues.end())
    {
        val = it->second;
        return leveldb::Status::OK();
    }

    // go directly to db
    return m_db->Get(key, val);
}

void MojDbSandwichTableTxn::Delete(const leveldb::Slice& key)
{
    // notify all iterators
    for(std::set<MojDbSandwichTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->notifyDelete(key);
    }

    // populate local view
    std::string keyStr = key.ToString();

    m_pendingDeletes.insert(keyStr);
    m_pendingValues.erase(keyStr); // if set before

    // XXX: work around for delete by query
    // make this delete visible to cursors
    //m_db->Delete(m_writeOptions, key);
}

MojErr MojDbSandwichTableTxn::commitImpl()
{
    for(std::set<MojDbSandwichTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->save();
    }

    // XXX: this will go away with new transactions
    //  temporary simulate write-batch to sandwich part
    leveldb::Status s = leveldb::Status::OK();
    for (PendingDeletes::iterator it = m_pendingDeletes.begin(); s.ok() && it != m_pendingDeletes.end(); ++it) {
        s = m_db->Delete(*it);
    }

    for (PendingValues::iterator it = m_pendingValues.begin(); s.ok() && it != m_pendingValues.end(); ++it) {
        s = m_db->Put(it->first, it->second);
    }

    MojLdbErrCheck(s, _T("db->Write"));

    cleanup();

    for(std::set<MojDbSandwichTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->restore();
    }

    return MojErrNone;
}

void MojDbSandwichTableTxn::cleanup()
{
    m_db = NULL;
    m_pendingValues.clear();
    m_pendingDeletes.clear();

    for(std::set<MojDbSandwichTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->detach();
    }
    m_iterators.clear();
}

// class MojDbSandwichEnvTxn
MojErr MojDbSandwichEnvTxn::begin(MojDbSandwichEngine* eng)
{
    // TODO: mutex and lock-file serialization to implement strongest
    //       isolation level
    return MojErrNone;
}

MojErr MojDbSandwichEnvTxn::abort()
{
    // Note creation of databases will not be rolled back
    return MojErrNone;
}

MojDbSandwichTableTxn &MojDbSandwichEnvTxn::tableTxn(MojDbSandwichEngine::BackendDb::Part &db)
{
    // find within opened already
    for(TableTxns::iterator it = m_tableTxns.begin();
                            it != m_tableTxns.end();
                            ++it)
    {
        if ((*it)->db() == &db) return *(*it);
    }

    // create new
    MojSharedPtr<MojDbSandwichTableTxn> txn;
    txn.reset(new MojDbSandwichTableTxn());
    txn->begin(db);
    m_tableTxns.push_back(txn);
    return *txn;
}

MojErr MojDbSandwichEnvTxn::commitImpl()
{
    MojErr accErr = MojErrNone;

    if (!isReversed()) {
        for(TableTxns::iterator it = m_tableTxns.begin();
                            it != m_tableTxns.end();
                            ++it)
        {
            MojErr err = (*it)->commitImpl();
            MojErrAccumulate(accErr, err);
        }
    } else {
        for(TableTxns::reverse_iterator it = m_tableTxns.rbegin(); it != m_tableTxns.rend(); ++it)
            {
                MojErr err = (*it)->commitImpl();
                MojErrAccumulate(accErr, err);
            }
    }
    return accErr;
}
