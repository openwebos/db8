/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics
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

MojDbLevelCursor::MojDbLevelCursor()
   : m_it(NULL), m_txn(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}

MojDbLevelCursor::~MojDbLevelCursor()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbLevelCursor::open(MojDbLevelDatabase* db, MojDbStorageTxn* txn, guint32 flags)
{
    MojAssert(db && txn);
    MojAssert(!m_it);
    MojLogTrace(MojDbLevelEngine::s_log);

    m_db = db->impl();
    m_it = m_db->NewIterator(leveldb::ReadOptions());
    m_txn = txn;
    MojLdbErrCheck(m_it->status(), _T("db->cursor"));
    m_warnCount = 0;

    return MojErrNone;
}

MojErr MojDbLevelCursor::close()
{
    MojLogTrace(MojDbLevelEngine::s_log);

    if (m_it) {
        delete m_it;
        m_it = NULL;
    }
    return MojErrNone;
}

MojErr MojDbLevelCursor::del()
{
    MojAssert(m_it);
    MojAssert( !m_txn || dynamic_cast<MojDbLevelAbstractTxn *>(m_txn) );
    MojLogTrace(MojDbLevelEngine::s_log);

    if(m_it->Valid())
    {
        leveldb::Slice key = m_it->key();
        const size_t recSize = key.size() + m_it->value().size();

        //m_it->Next(); - not clear if we need it here
        if(m_txn)
        {
            MojDbLevelAbstractTxn *txn = static_cast<MojDbLevelAbstractTxn *>(m_txn);
            txn->tableTxn(m_db).Delete(key);

            MojErr err = m_txn->offsetQuota(-(gint64) recSize);
            MojErrCheck(err);
        }
        else
        {
            m_db->Delete(leveldb::WriteOptions(), key);
        }
    }

    return MojErrNone;
}


MojErr MojDbLevelCursor::delPrefix(const MojDbKey& prefix)
{
    MojAssert(m_it);
    MojLogTrace(MojDbLevelEngine::s_log);
    //MojDbLevelItem val;
    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);
    leveldb::Iterator * it = m_it;

    it->SeekToFirst();
    it->Seek(*key.impl());
    while (it->Valid() && it->key().starts_with(*key.impl()))
    {
        err = del();
        MojErrCheck(err);
        MojLdbErrCheck(it->status(), _T("lbb->delPrefix"));
        it->Next();
    }
    return MojErrNone;
}

MojErr MojDbLevelCursor::get(MojDbLevelItem& key, MojDbLevelItem& val, bool& foundOut, guint32 flags)
{
    MojAssert(m_it);
    MojLogTrace(MojDbLevelEngine::s_log);

    foundOut = false;
    switch (flags)
    {
    case e_Set:
        // TODO: skip deleted inside transaction
        m_it->Seek(*key.impl());
    break;

    case e_Next:
        MojAssert(m_it->Valid());
        m_it->Next();
    break;
    case e_Prev:
        MojAssert(m_it->Valid());
        m_it->Prev();
    break;
    case e_First:
        m_it->SeekToFirst();
    break;
    case e_Last:
        m_it->SeekToLast();
    break;
    default:
    break;
    }

    if(m_it->Valid() )
    {
        foundOut = true;
        val.from(m_it->value());
        key.from(m_it->key());
    }

    MojLdbErrCheck(m_it->status(), _T("lbb->get"));

    if (!m_it->Valid())
    {
        m_it->SeekToLast();
        MojLdbErrCheck(m_it->status(), _T("lbb->get"));
    }

    return MojErrNone;
}

MojErr MojDbLevelCursor::stats(gsize& countOut, gsize& sizeOut)
{

    MojErr err = statsPrefix(MojDbKey(), countOut, sizeOut);
    MojLogInfo(MojDbLevelEngine::s_log, _T("ldbcursor_stats: count: %d, size: %d, err: %d"), (int)countOut, (int)sizeOut, (int)err);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelCursor::statsPrefix(const MojDbKey& prefix, gsize& countOut, gsize& sizeOut)
{
    countOut = 0;
    sizeOut = 0;
    m_warnCount = 0;    // debug
    MojDbLevelItem val;
    MojDbLevelItem key;
    MojErr err = key.fromBytes(prefix.data(), prefix.size());
    MojErrCheck(err);

    m_it->SeekToFirst();

    gsize count = 0;
    gsize size = 0;
    MojErrCheck(err);

    for (m_it->Seek(*key.impl()); m_it->Valid(); m_it->Next())
    {
        if(m_it->key().starts_with(*key.impl()) == false)
            break;
        ++count;
        // debug code for verifying index keys
        size += m_it->key().size() + m_it->value().size();
        m_it->Next();
    }
    sizeOut = size;
    countOut = count;

    return MojErrNone;
}