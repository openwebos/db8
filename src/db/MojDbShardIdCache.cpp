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

//db.shardIdCache

MojDbShardIdCache::MojDbShardIdCache()
{
}

MojDbShardIdCache::~MojDbShardIdCache()
{
}

bool MojDbShardIdCache::isExist (const MojUInt32 id) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    return ( m_map.find(id) != m_map.end() );
}

void MojDbShardIdCache::put (const MojUInt32 id, const MojObject& obj)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::pair<std::map<MojUInt32, MojObject>::iterator, bool> ret;
    ret = m_map.insert(std::make_pair(id, obj));

    if(ret.second)
    {
        LOG_DEBUG("[db_shardIdCache] new element was inserted: [%x]\n", id);
    }
    else
    {
        LOG_WARNING(MSGID_DB_SHARDENGINE_WARNING, 1,
        		PMLOGFV("id", "%x", id),
        		"element already exist");
    }
}

bool MojDbShardIdCache::get (const MojUInt32 id, MojObject& o_obj) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::map<MojUInt32, MojObject>::const_iterator it;
    it = m_map.find(id);

    if(it != m_map.end())
    {
        o_obj = it->second;
        LOG_DEBUG("[db_shardIdCache] get element by id [%x]\n", id);
        return true;
    }

    return false;
}

bool MojDbShardIdCache::update (const MojUInt32 id, const MojObject& i_obj)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::map<MojUInt32, MojObject>::iterator it;
    it = m_map.find(id);

    if(it != m_map.end())
    {
        (*it).second = i_obj;
        LOG_DEBUG("[db_shardIdCache] update element by id", id);
        return true;
    }

    return false;
}

void MojDbShardIdCache::del (const MojUInt32 id)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if(m_map.erase(id) > 0) //was erased
    {
        LOG_DEBUG("[db_shardIdCache] id [%x] was erased", id);
    }
    else
    {
        LOG_WARNING(MSGID_DB_SHARDENGINE_WARNING, 1,
        		PMLOGFV("id", "%d", id), "id was not erased");
    }
}

void MojDbShardIdCache::clear (void)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    m_map.clear();
    LOG_DEBUG("[db_shardIdCache] map was cleaned");
}

