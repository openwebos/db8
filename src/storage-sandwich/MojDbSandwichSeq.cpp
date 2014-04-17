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

#include "MojDbSandwichSeq.h"
#include "MojDbSandwichDatabase.h"
#include "MojDbSandwichEngine.h"

// at this point it's just a placeholder
MojDbSandwichSeq::~MojDbSandwichSeq()
{
    if (m_db) {
        MojErr err = close();
        MojErrCatchAll(err);
    }
}

MojErr MojDbSandwichSeq::open(const MojChar* name, MojDbSandwichDatabase* db)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(db);

    //MojAssert(!m_seq);
    MojErr err;

    m_db = db;
    err = m_key.fromBytes(reinterpret_cast<const MojByte  *>(name), strlen(name));
    MojErrCheck(err);

    MojDbSandwichItem val;
    bool found = false;
    err = m_db->get(m_key, NULL, false, val, found);
    MojErrCheck(err);

    if (found)
    {
        MojObject valObj;
        err = val.toObject(valObj);
        MojErrCheck(err);
        m_next = valObj.intValue();
    }
    else
    {
        m_next = 0;
    }

    m_allocated = m_next;

    return MojErrNone;
}

MojErr MojDbSandwichSeq::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (m_db) {
        MojErr err = store(m_next);
        MojErrCheck(err);
        m_db = NULL;
    }
    return MojErrNone;
}

MojErr MojDbSandwichSeq::get(MojInt64& valOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (m_next == m_allocated)
    {
        MojErr err = MojDbSandwichSeq::allocateMore();
        MojErrCheck(err);
        MojAssert(m_allocated > m_next);
    }
    valOut = m_next++;

    return MojErrNone;
}

MojErr MojDbSandwichSeq::allocateMore()
{
    return store(m_allocated + 100);
}

MojErr MojDbSandwichSeq::store(MojInt64 next)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert( next >= m_next );
    MojErr err;
    MojDbSandwichItem val;

#ifdef MOJ_DEBUG
    // ensure that our state is consistent with db
    bool found = false;
    err = m_db->get(m_key, NULL, false, val, found);
    MojErrCheck(err);

    if (found)
    {
        MojObject valObj;
        err = val.toObject(valObj);
        MojErrCheck(err);
        if (m_allocated != valObj.intValue()) return MojErrDbInconsistentIndex;
    }
    else
    {
        if (m_allocated != 0) return MojErrDbInconsistentIndex;
    }

#endif

    err = val.fromObject(next);
    MojErrCheck(err);
    err = m_db->put(m_key, val, NULL, false);
    MojErrCheck(err);
    m_allocated = next;

    return MojErrNone;
}
