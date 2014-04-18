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

#ifndef MOJDBLEVELCURSOR_H
#define MOJDBLEVELCURSOR_H

#include <leveldb/db.h>
#include "db/MojDbDefs.h"
#include "MojDbSandwichEngine.h"

#include <MojDbSandwichIterator.h>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    #include <boost/smart_ptr/scoped_ptr.hpp>
#else
    #include <auto_ptr.h>
#endif

class MojDbSandwichDatabase;
class MojDbSandwichItem;

class MojDbSandwichEnvTxn;
class MojDbSandwichTableTxn;
//class MojDbSandwichTxnIterator;

class MojDbSandwichCursor : public MojNoCopy
{
public:
    MojDbSandwichCursor();
    ~MojDbSandwichCursor();

    MojErr open(MojDbSandwichDatabase* db, MojDbStorageTxn* txn, MojUInt32 flags);
    MojErr close();
    MojErr del();
    MojErr delPrefix(const MojDbKey& prefix);
    MojErr get(MojDbSandwichItem& key, MojDbSandwichItem& val, bool& foundOut, MojUInt32 flags);
    MojErr stats(MojSize& countOut, MojSize& sizeOut);
    MojErr statsPrefix(const MojDbKey& prefix, MojSize& countOut, MojSize& sizeOut);

    enum LDB_FLAGS
    {
       e_First = 0, e_Last, e_Next, e_Prev, e_Range, e_Set, e_TotalFlags
    };
    size_t recSize() const;

private:
    MojDbSandwichEngine::BackendDb::Part* m_db;
    MojDbSandwichEnvTxn* m_txn;
    MojDbSandwichTableTxn* m_ttxn;
//#ifdef __GXX_EXPERIMENTAL_CXX0X__
//    boost::scoped_ptr<MojDbSandwichTxnIterator> m_txnIt;
//#else
//    std::auto_ptr<MojDbSandwichTxnIterator> m_txnIt;
//#endif
    std::unique_ptr<leveldb::Iterator> m_txnIt;
    MojSize m_warnCount;
};

#endif
