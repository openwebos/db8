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

#include "MojDbSandwichDatabase.h"
#include "MojDbSandwichQuery.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichTxn.h"
#include "db/MojDbQueryPlan.h"
#include "core/MojObjectSerialization.h"

enum LDB_FLAGS
{
   e_First = 0, e_Last, e_Next, e_Prev, e_Range, e_Set, e_TotalFlags
};

const MojUInt32 MojDbSandwichQuery::SeekFlags = e_Set;
const MojUInt32 MojDbSandwichQuery::SeekEmptyFlags[2] = {e_First, e_Last};
const MojUInt32 MojDbSandwichQuery::NextFlags[2] = {e_Next, e_Prev};

MojDbSandwichQuery::MojDbSandwichQuery()
{
}

MojDbSandwichQuery::~MojDbSandwichQuery()
{
    MojErr err = close();
    MojErrCatchAll(err);
}

MojErr MojDbSandwichQuery::open(MojDbSandwichDatabase* db, MojDbSandwichDatabase* joinDb,
        MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* abstractTxn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(!m_isOpen);
    MojAssert(db && db->impl().Valid() && plan.get());
    MojAssert( dynamic_cast<MojDbSandwichEnvTxn *>(abstractTxn) );

    auto txn = static_cast<MojDbSandwichEnvTxn *>(abstractTxn);

    MojErr err = MojDbIsamQuery::open(plan, txn);
    MojErrCheck(err);

    m_it = txn->ref(db->impl()).NewIterator();

    m_db = joinDb;

    return MojErrNone;
}

MojErr MojDbSandwichQuery::close()
{
    MojErr err = MojErrNone;

    if (m_isOpen) {
        MojErr errClose = MojDbIsamQuery::close();
        MojErrAccumulate(err, errClose);
        m_it.reset();
        m_db = NULL;
        m_isOpen = false;
    }
    return err;
}

MojErr MojDbSandwichQuery::getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut)
{
    itemOut = NULL;
    foundOut = false;
    MojDbSandwichItem* item = NULL;
    MojErr err = MojErrNone;

    if (m_db) {
        // retrun val from primary db
        MojDbSandwichItem primaryKey;
        err = primaryKey.fromObject(id);
        MojErrCheck(err);
        err = m_db->get(primaryKey, m_txn, false, m_primaryVal, foundOut);
        MojErrCheck(err);
        if (!foundOut) {
            char s[1024];
            size_t size = primaryKey.size();
            (void) MojByteArrayToHex(primaryKey.data(), size, s);
            LOG_DEBUG("[db_ldb] bdbq_byId_warnindex: KeySize: %zu; %s ;id: %s \n",
                size, s, primaryKey.data()+1);

            //MojErrThrow(MojErrDbInconsistentIndex);
            MojErrThrow(MojErrInternalIndexOnFind);
        }
        item = &m_primaryVal;
    } else {
        // return val from cursor
        item = &m_val;
    }
    item->id(id);

    // check for exclusions
    bool exclude = false;
    err = checkExclude(item, exclude);
    MojErrCheck(err);
    if (!exclude) {
        itemOut = item;
    }
    return MojErrNone;
}

MojErr MojDbSandwichQuery::seekImpl(const ByteVec& key, bool desc, bool& foundOut)
{
    MojAssert( m_it );

    if (key.empty()) {
        // if key is empty, seek to beginning (or end if desc)
        MojErr err = getKey(foundOut, SeekEmptyFlags[desc]);
        MojErrCheck(err);
    } else {
        // otherwise seek to the key
        MojErr err = m_key.fromBytes(key.begin(), key.size());
        MojErrCheck(err);
        err = getKey(foundOut, SeekFlags);
        MojErrCheck(err);
        // if descending, skip the first result (which is outside the range)
        if (desc) {
            err = next(foundOut);
            MojErrCheck(err);
        }
    }
    return MojErrNone;
}

MojErr MojDbSandwichQuery::next(bool& foundOut)
{
    MojAssert( m_it );
    MojAssert(m_state == StateNext);

    MojErr err = getKey(foundOut, NextFlags[m_plan->desc()]); // get next or previous
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichQuery::getVal(MojDbStorageItem*& itemOut, bool& foundOut)
{
    MojObject id;
    MojErr err = parseId(id);
    MojErrCheck(err);
    err = getById(id, itemOut, foundOut);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbSandwichQuery::getKey(bool& foundOut, MojUInt32 flags)
{
    foundOut = false;
    switch (flags)
    {
    case e_Set: m_it->Seek(*m_key.impl()); break;
    case e_Next: m_it->Next(); break;
    case e_Prev: m_it->Prev(); break;
    case e_First: m_it->SeekToFirst(); break;
    case e_Last: m_it->SeekToLast(); break;
    default: MojAssert( e_First <= flags && flags < e_TotalFlags );
    }
    if (m_it->Valid())
    {
        m_key.from(m_it->key());
        m_val.from(m_it->value());
        m_keyData = m_key.data();
        m_keySize = m_key.size();
        foundOut = true;
    }

    return MojErrNone;
}
