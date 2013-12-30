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

#include "db/MojDbMediaLinkManager.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDb.h"
#include "core/MojOs.h"
#include <sstream>
#include <unistd.h>
#include <algorithm>

//db.shardLinkManager

MojDbMediaLinkManager::MojDbMediaLinkManager()
{
}

MojErr MojDbMediaLinkManager::setLinkDirectory(const MojString& dir)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    m_dir = dir;
    return MojErrNone;
}

MojErr MojDbMediaLinkManager::getLinkPath(MojUInt32 shardId, MojString& linkPath)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojErr err;
    if (m_dir.empty()) {
        err = m_dir.assign(MojDbServiceDefs::DefaultMediaLinkPath);
        MojErrCheck(err);
    }

    std::stringstream sstream;
    sstream << std::hex << std::uppercase << shardId;
    std::string linkname = sstream.str();

    return linkPath.format("%s/%s", m_dir.data(), linkname.c_str());
}

MojErr MojDbMediaLinkManager::processShardInfo(MojDbShardInfo& shardInfo)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if (shardInfo.active)
        return createLink(shardInfo);
    else
        return removeLink(shardInfo);
}

MojErr MojDbMediaLinkManager::createLink(MojDbShardInfo& shardInfo)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err;
    MojString linkPath;

    err = getLinkPath(shardInfo.id, linkPath);
    MojErrCheck(err);

    if ( access(linkPath.data(), F_OK) != 0)
    {
         LOG_DEBUG("[db_shardLinkManager] create link %s", linkPath.data());
         err = MojSymlink(shardInfo.deviceUri, linkPath);

         if (err == MojErrNone) {
             LOG_DEBUG("[db_shardLinkManager] Created symlink %s for %s", linkPath.data(), shardInfo.deviceUri.data());
         } else {
             LOG_DEBUG("[db_shardLinkManager] Can't create symlink %s for %s", linkPath.data(), shardInfo.deviceUri.data());
             return MojErrAccessDenied;
         }
    }

    shardInfo.mountPath = linkPath;

    return err;
}

MojErr MojDbMediaLinkManager::removeLink(const MojDbShardInfo& shardInfo)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojErr err;

    err = MojUnlink(shardInfo.mountPath);
    if (err != MojErrNone) {
        LOG_DEBUG("[db_shardLinkManager] Can't remove symlink %s", shardInfo.mountPath.data());
    }

    return err;
}
