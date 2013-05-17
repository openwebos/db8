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

#include "db-luna/leveldb/MojDbLevelCursor.h"
#include "db-luna/leveldb/MojDbLevelDatabase.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#include "db-luna/leveldb/MojDbLevelTxn.h"
#include "db-luna/leveldb/MojDbLevelItem.h"
#include "db-luna/leveldb/defs.h"

MojDbLevelCursor::MojDbLevelCursor() :
    m_db(0),
    m_txn(0),
    m_ttxn(0)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelCursor::~MojDbLevelCursor()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelCursor::open(MojDbLevelDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    MojAssert( !m_it.get() );
    MojAssert(db && txn);
    MojAssert(db->impl());
    MojAssert( dynamic_cast<MojDbLevelAbstractTxn *>(txn) );

    m_db = db->impl();
    m_it.reset(new MojDbLevelIterator(m_db));
    MojLdbErrCheck((*m_it)->status(), _T("db->cursor"));

    m_txn = static_cast<MojDbLevelAbstractTxn *>(txn);
    m_ttxn = & m_txn->tableTxn(m_db);
    m_warnCount = 0;

    return MojErrNone;
}

MojErr MojDbLevelCursor::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    m_it.reset();
    m_txn = 0;
    m_ttxn = 0;
    return MojErrNone;
}

MojErr MojDbLevelCursor::del()
{
    MojAssert( m_it.get() );
    MojAssert( m_txn && m_ttxn );
    MojLogTrace(MojDbLevelEngine::s_log);
    MojAssert( dynamic_cast<MojDbLevelAbstractTxn *>(m_txn) );

    leveldb::Slice key = (*m_it)->key();
    const size_t delSize = recSize();

    //m_it->Next(); - not clear if we need it here
    m_ttxn->Delete(key);

    MojErr err = m_txn->offsetQuota(-(MojInt64) delSize);
    MojErrCheck(err);
    return MojErrNone;
}


MojErr MojDbLevelCursor::delPrefix(const MojDbKey& prefix)
{
    MojAssert( m_it.get() );
    MojLogTrace(MojDbLevelEngine::s_log);

    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    m_it->toFirst();
    m_it->seek(key.impl()->ToString());
    while (m_it->isValid() && (*m_it)->key().starts_with(*key.impl()))
    {
        err = del();
        MojErrCheck(err);
        MojLdbErrCheck((*m_it)->status(), _T("lbb->delPrefix"));
        ++(*m_it);
    }
    return MojErrNone;
}

MojErr MojDbLevelCursor::get(MojDbLevelItem& key, MojDbLevelItem& val, bool& foundOut, MojUInt32 flags)
{
    MojAssert( m_it.get() );
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    switch (flags)
    {
    case e_Set:
        // TODO: skip deleted inside transaction
        m_it->seek(key.impl()->ToString());
    break;

    case e_Next:
        MojAssert( !m_it->isEnd() );
        ++(*m_it);
    break;
    case e_Prev:
        MojAssert( !m_it->isBegin() );
        --(*m_it);
    break;
    case e_First:
        m_it->toFirst();
    break;
    case e_Last:
        m_it->toLast();
    break;
    default:
    break;
    }

    if(m_it->isValid() )
    {
        foundOut = true;
        val.from((*m_it)->value());
        key.from((*m_it)->key());
    }

    MojLdbErrCheck((*m_it)->status(), _T("lbb->get"));

    if (!m_it->isValid())
    {
        m_it->toLast();
        MojLdbErrCheck((*m_it)->status(), _T("lbb->get"));
    }

    return MojErrNone;
}

MojErr MojDbLevelCursor::stats(MojSize& countOut, MojSize& sizeOut)
{

    MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbcursor_stats: count: %d, size: %d, err: %d"), (int)countOut, (int)sizeOut, (int)err);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelCursor::statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut)
{
    countOut = 0;
    sizeOut = 0;
    m_warnCount = 0;    // debug
    MojDbLevelItem val;
    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    m_it->toFirst();

    MojSize count = 0;
    MojSize size = 0;
    MojErrCheck(err);

    for (m_it->seek(key.impl()->ToString()); m_it->isValid(); ++(*m_it))
    {
        if((*m_it)->key().starts_with(*key.impl()) == false)
            break;
        ++count;
        // debug code for verifying index keys
        size += recSize();
        ++(*m_it);
    }
    sizeOut = size;
    countOut = count;

    return MojErrNone;
}

size_t MojDbLevelCursor::recSize() const
{
    if (!m_it->isValid())
        return 0;
    return (*m_it)->key().size() + (*m_it)->value().size();
}
