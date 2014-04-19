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

#include "MojDbSandwichCursor.h"
#include "MojDbSandwichDatabase.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichTxn.h"
#include "MojDbSandwichItem.h"
#include "db-luna/leveldb/defs.h"
#include "core/MojLogDb8.h"

MojDbSandwichCursor::MojDbSandwichCursor() :
    m_txn(0)
{
}

MojDbSandwichCursor::~MojDbSandwichCursor()
{
    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbSandwichCursor::open(MojDbSandwichDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(db && txn);
    MojAssert(db->impl().Valid());
    MojAssert( dynamic_cast<MojDbSandwichEnvTxn *>(txn) );

    m_txn = static_cast<MojDbSandwichEnvTxn *>(txn);
    m_part = m_txn->ref(db->impl());
    m_txnIt = m_part.NewIterator();
    MojAssert( m_txnIt.get() );
    m_warnCount = 0;

    m_txnIt->SeekToFirst();

    return MojErrNone;
}

MojErr MojDbSandwichCursor::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    m_txn = 0;
    m_part = {};
    m_txnIt.reset();

    return MojErrNone;
}

MojErr MojDbSandwichCursor::del()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( m_txn && m_part.Valid() );
    MojAssert( m_txnIt.get() );

    const size_t delSize = recSize();

    m_part.Delete(m_txnIt->key());
    MojErr err = m_txn->offsetQuota(-(MojInt64) delSize);
    MojErrCheck(err);

    return MojErrNone;
}


MojErr MojDbSandwichCursor::delPrefix(const MojDbKey& prefix)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( m_txnIt.get() );

    MojDbSandwichItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    const auto& searchKey = *key.impl();
    m_txnIt->Seek(searchKey);

    if (!m_txnIt->Valid()) {
        return MojErrNone;
    }

    while (m_txnIt->Valid() && m_txnIt->key().starts_with(searchKey)) {
        err = del();
        MojErrCheck(err);
        m_txnIt->Next();
    }
    return MojErrNone;
}

MojErr MojDbSandwichCursor::get(MojDbSandwichItem& key, MojDbSandwichItem& val, bool& foundOut, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( m_txnIt.get() );

    foundOut = false;
    switch (flags)
    {
    case e_Set:
        m_txnIt->Seek(*key.impl());
    break;

    case e_Next:
        m_txnIt->Next();
    break;
    case e_Prev:
        m_txnIt->Prev();
    break;
    case e_First:
        m_txnIt->SeekToFirst();
    break;
    case e_Last:
        m_txnIt->SeekToLast();
    break;
    default:
    break;
    }

    if(m_txnIt->Valid())
    {
        foundOut = true;
        key.from(m_txnIt->key());
        val.from(m_txnIt->value());

    }

    return MojErrNone;
}

MojErr MojDbSandwichCursor::stats(MojSize& countOut, MojSize& sizeOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( m_txnIt.get() );

    MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
    LOG_DEBUG("[db_ldb] ldbcursor_stats: count: %d, size: %d, err: %d", (int)countOut, (int)sizeOut, (int)err);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichCursor::statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( m_txnIt.get() );

    countOut = 0;
    sizeOut = 0;

    MojDbSandwichItem val;
    MojDbSandwichItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    MojSize count = 0;
    MojSize size = 0;
    MojErrCheck(err);

    const auto& searchKey = *key.impl();
    m_txnIt->Seek(searchKey);

    while (m_txnIt->Valid() && m_txnIt->key().starts_with(searchKey)) {
        ++count;
        size += recSize();
        m_txnIt->Next();
    }
    sizeOut = size;
    countOut = count;

    return MojErrNone;
}

size_t MojDbSandwichCursor::recSize() const
{
    if (!m_txnIt->Valid())
        return 0;
    return m_txnIt->key().size() + m_txnIt->value().size();
}
