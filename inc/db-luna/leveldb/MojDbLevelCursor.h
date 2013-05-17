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

#ifndef MOJDBLEVELCURSOR_H
#define MOJDBLEVELCURSOR_H

#include <auto_ptr.h>

#include "leveldb/db.h"
#include "db/MojDbDefs.h"

#include <db-luna/leveldb/MojDbLevelIterator.h>

class MojDbLevelDatabase;
class MojDbLevelItem;

class MojDbLevelAbstractTxn;
class MojDbLevelTableTxn;

class MojDbLevelCursor : public MojNoCopy
{
public:
    MojDbLevelCursor();
    ~MojDbLevelCursor();

    MojErr open(MojDbLevelDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags);
    MojErr close();
    MojErr del();
    MojErr delPrefix(const MojDbKey& prefix);
    MojErr get(MojDbLevelItem& key, MojDbLevelItem& val, bool& foundOut, MojUInt32 flags);
    MojErr stats(MojSize& countOut, MojSize& sizeOut);
    MojErr statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut);

    enum LDB_FLAGS
    {
       e_First = 0, e_Last, e_Next, e_Prev, e_Range, e_Set, e_TotalFlags
    };
    size_t recSize() const;

private:
    leveldb::DB* m_db;
    MojDbLevelAbstractTxn* m_txn;
    MojDbLevelTableTxn* m_ttxn;
    std::auto_ptr<MojDbLevelIterator> m_it;

    MojSize m_warnCount;
};

#endif
