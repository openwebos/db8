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

using namespace std;

MojLogger MojDbShardIdCache::s_log(_T("db.shardIdCache"));

MojDbShardIdCache::MojDbShardIdCache()
{
    MojLogTrace(s_log);
}

MojDbShardIdCache::~MojDbShardIdCache()
{
    MojLogTrace(s_log);
}

bool MojDbShardIdCache::isExist (const MojUInt32 id) const
{
    return ( m_map.find(id) != m_map.end() );
}

void MojDbShardIdCache::put (const MojUInt32 id, const MojObject& obj)
{
    std::pair<std::map<MojUInt32, MojObject>::iterator, bool> ret;
    ret = m_map.insert(std::make_pair(id, obj));

    if(ret.second)
    {
        MojLogInfo(MojDbShardIdCache::s_log, _T("new element was inserted: [%x]\n"), id);
    }
    else
    {
        MojLogWarning(MojDbShardIdCache::s_log, _T("element already exist: [%x]\n"), id);
    }
}

bool MojDbShardIdCache::get (const MojUInt32 id, MojObject& o_obj) const
{
    std::map<MojUInt32, MojObject>::const_iterator it;
    it = m_map.find(id);

    if(it != m_map.end())
    {
        o_obj = it->second;
        MojLogInfo(MojDbShardIdCache::s_log, _T("get element by id [%x]\n"), id);
        return true;
    }

    return false;
}

bool MojDbShardIdCache::update (const MojUInt32 id, const MojObject& i_obj)
{
    std::map<MojUInt32, MojObject>::iterator it;
    it = m_map.find(id);

    if(it != m_map.end())
    {
        (*it).second = i_obj;
        MojLogInfo(MojDbShardIdCache::s_log, _T("update element by id [%x]\n"), id);
        return true;
    }

    return false;
}

void MojDbShardIdCache::del (const MojUInt32 id)
{
    if(m_map.erase(id) > 0) //was erased
    {
        MojLogInfo(MojDbShardIdCache::s_log, _T("id [%x] was erased\n"), id);
    }
    else
    {
        MojLogWarning(MojDbShardIdCache::s_log, _T("id [%x] was not erased\n"), id);
    }
}

void MojDbShardIdCache::clear (void)
{
    m_map.clear();
    MojLogInfo(MojDbShardIdCache::s_log, _T("map was cleaned\n"));
}

