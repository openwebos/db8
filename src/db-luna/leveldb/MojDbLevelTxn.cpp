/* @@@LICENSE
*
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

#include "db-luna/leveldb/MojDbLevelEngine.h"
#include "db-luna/leveldb/MojDbLevelTxn.h"
#include "db-luna/leveldb/defs.h"
#include "db-luna/leveldb/MojDbLevelTxnIterator.h"

MojDbLevelTableTxn::MojDbLevelTableTxn() : m_db(NULL)
{
}

MojDbLevelTableTxn::~MojDbLevelTableTxn()
{
    if (m_db)
        abort();
    cleanup();
}

void MojDbLevelTableTxn::detach(MojDbLevelTxnIterator *it)
{
    MojAssert( it );
    size_t detached = m_iterators.erase(it);
    MojAssert( detached == 1 );
}

MojDbLevelTxnIterator* MojDbLevelTableTxn::createIterator()
{
    MojAssert( m_db );
    MojDbLevelTxnIterator* it = new MojDbLevelTxnIterator(this);
    m_iterators.insert(it);
    return it;
}

// class MojDbLevelTableTxn
MojErr MojDbLevelTableTxn::begin(leveldb::DB &db)
{
    MojAssert( m_iterators.empty() );

    // TODO: mutex and lock-file serialization
    m_db = &db;

    return MojErrNone;
}

MojErr MojDbLevelTableTxn::abort()
{
    MojAssert( m_db ); // database should exist

    cleanup();
    return MojErrNone;
}

void MojDbLevelTableTxn::Put(const leveldb::Slice& key,
                             const leveldb::Slice& val)
{
    // populate local view
    std::string keyStr = key.ToString();

    m_pendingValues[keyStr] = val.ToString();
    m_pendingDeletes.erase(keyStr); // if hidden before

    // notify all iterators
    for(std::set<MojDbLevelTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->notifyPut(key);
    }

}

leveldb::Status MojDbLevelTableTxn::Get(const leveldb::Slice& key,
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
    return m_db->Get(MojDbLevelEngine::getReadOptions(), key, &val);
}

void MojDbLevelTableTxn::Delete(const leveldb::Slice& key)
{
    // notify all iterators
    for(std::set<MojDbLevelTxnIterator*>::const_iterator i = m_iterators.begin();
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

MojErr MojDbLevelTableTxn::commitImpl()
{
    for(std::set<MojDbLevelTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->save();
    }

    leveldb::WriteBatch writeBatch;

    for (PendingDeletes::iterator it = m_pendingDeletes.begin(); it != m_pendingDeletes.end(); ++it) {
        writeBatch.Delete(*it);
    }

    for (PendingValues::iterator it = m_pendingValues.begin(); it != m_pendingValues.end(); ++it) {
        writeBatch.Put(it->first, it->second);
    }

    if (!m_pendingDeletes.empty() || !m_pendingValues.empty())
    {
        // Write to leveldb only if pending deletes/values.
        leveldb::Status s = m_db->Write(MojDbLevelEngine::getWriteOptions(), &writeBatch);
        MojLdbErrCheck(s, _T("db->Write"));
    }

    cleanup();

    for(std::set<MojDbLevelTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->restore();
    }

    return MojErrNone;
}

void MojDbLevelTableTxn::cleanup()
{
    m_db = NULL;
    m_pendingValues.clear();
    m_pendingDeletes.clear();

    for(std::set<MojDbLevelTxnIterator*>::const_iterator i = m_iterators.begin();
        i != m_iterators.end();
        ++i)
    {
        (*i)->detach();
    }
    m_iterators.clear();
}

// class MojDbLevelEnvTxn
MojErr MojDbLevelEnvTxn::begin(MojDbLevelEngine* eng)
{
    // TODO: mutex and lock-file serialization to implement strongest
    //       isolation level
    return MojErrNone;
}

MojErr MojDbLevelEnvTxn::abort()
{
    // Note creation of databases will not be rolled back
    return MojErrNone;
}

MojDbLevelTableTxn &MojDbLevelEnvTxn::tableTxn(leveldb::DB &db)
{
    // find within opened already
    for(TableTxns::iterator it = m_tableTxns.begin();
                            it != m_tableTxns.end();
                            ++it)
    {
        if ((*it)->db() == &db) return *(*it);
    }

    // create new
    MojSharedPtr<MojDbLevelTableTxn> txn;
    txn.reset(new MojDbLevelTableTxn());
    txn->begin(db);
    m_tableTxns.push_back(txn);
    return *txn;
}

MojErr MojDbLevelEnvTxn::commitImpl()
{
    MojErr accErr = MojErrNone;
    for(TableTxns::iterator it = m_tableTxns.begin();
                            it != m_tableTxns.end();
                            ++it)
    {
        MojErr err = (*it)->commitImpl();
        MojErrAccumulate(accErr, err);
    }
    return accErr;
}
