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

#include "db/MojDbShardIdCache.h"
#include "db/MojDb.h"
#include "db/MojDbServiceDefs.h"
#include <iostream>
#include <fstream>

using namespace std;

static const MojChar* const dbPathName = _T("shard_id_cache");
MojLogger MojDbShardIdCache::s_log(_T("db.shardIdCache"));

MojDbShardIdCache::MojDbShardIdCache()
{
    MojLogTrace(s_log);
}

MojDbShardIdCache::~MojDbShardIdCache()
{
    MojLogTrace(s_log);
}

MojErr MojDbShardIdCache::MojDbShardIdCache::init ()
{
    return (_read());
}

bool MojDbShardIdCache::isExist (MojUInt32 id)
{
    return ( m_set.find(id) != m_set.end() );
}

MojErr MojDbShardIdCache::put (MojUInt32 id)
{
    MojErr err = MojErrNone;
    std::pair<std::set<MojUInt32>::iterator,bool> ret;
    ret = m_set.insert(id);

    if(ret.second) //new element was inserted
    {
        MojLogInfo(MojDbShardIdCache::s_log, _T("new element was inserted [%x]\n"), id);
        err = _write();
    }
    else
    {
        MojLogWarning(MojDbShardIdCache::s_log, _T("id [%x] is already exist\n"), id);
    }

    return err;
}

MojErr MojDbShardIdCache::del (MojUInt32 id)
{
    MojErr err = MojErrNone;
    if (!m_set.empty())
    {
        if(m_set.erase(id) > 0) //was erased
        {
            MojLogInfo(MojDbShardIdCache::s_log, _T("id [%x] was erased\n"), id);
            err = _write();
        }
        else
        {
            MojLogWarning(MojDbShardIdCache::s_log, _T("id [%x] was not erased\n"), id);
        }
    }

    return err;
}

MojErr MojDbShardIdCache::_write (void)
{
    MojErr err = MojErrNone;

    ofstream out_file;
    std::set<MojUInt32>::iterator it;

    out_file.open(dbPathName);

    if (out_file.is_open())
    {
        for (it = m_set.begin(); it != m_set.end(); ++it)
        {
            out_file << *it << " ";
        }

        out_file.flush();
        out_file.close();
        MojLogInfo(MojDbShardIdCache::s_log, _T("cache write succeeded\n"));
    }
    else
    {
        err = MojErrNotOpen;
        MojLogWarning(MojDbShardIdCache::s_log, _T("can't write cache\n"));
    }

    return err;
}

MojErr MojDbShardIdCache::_read (void)
{
    MojErr err = MojErrNone;

    if (!m_set.empty())
    {
        m_set.clear();
    }

    ifstream in_file;
    MojUInt32 id;

    in_file.open(dbPathName);

    if (in_file.is_open())
    {
        while (!in_file.eof())
        {
            in_file >> id;
            m_set.insert(id);
        }

        in_file.close();
        MojLogInfo(MojDbShardIdCache::s_log, _T("cache read succeeded\n"));
    }
    else
    {
        err = MojErrNotOpen;
        MojLogWarning(MojDbShardIdCache::s_log, _T("can't read cache\n"));
    }

    return err;
}
