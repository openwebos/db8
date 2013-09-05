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
#include <algorithm>

const MojChar* const MediaLinkDirectory = _T("/media/mountpoint");

MojLogger MojDbMediaLinkManager::s_log(_T("db.shardLinkManager"));


MojDbMediaLinkManager::MojDbMediaLinkManager()
{
    MojLogTrace(s_log);

    m_dir.assign(MediaLinkDirectory);
    m_baseDirCreated = false;
}

MojErr MojDbMediaLinkManager::setLinkDirectory(const MojString& dir)
{
    m_dir = dir;
    m_baseDirCreated = false;
    return MojErrNone;
}

MojErr MojDbMediaLinkManager::getLinkPath(MojUInt32 shardId, MojString& linkPath)
{
    std::stringstream sstream;
    sstream << std::hex << std::uppercase << shardId;
    std::string linkname = sstream.str();
    return linkPath.format("%s/%s", m_dir.data(), linkname.c_str());
}

MojErr MojDbMediaLinkManager::createLink(MojDbShardEngine::ShardInfo& shardInfo)
{
    MojLogTrace(s_log);

    MojErr err;
    MojString linkPath;

    // ensure base mount path exists
    if (!m_baseDirCreated) {
        err = MojMkDir(m_dir, MOJ_S_IRWXU);
        MojErrCheck(err);
        MojLogNotice (s_log, _T("Create base folder for mount symlinks: %s"), m_dir.data());
        m_baseDirCreated = true;
    }

    err = getLinkPath(shardInfo.id, linkPath);
    MojErrCheck(err);

    MojLogDebug(s_log, _T("create link %s"), linkPath.data());

    MojUnlink(linkPath);
    err = MojSymlink(shardInfo.deviceUri, linkPath);
    if (err == MojErrNone) {
        shardInfo.mountPath = linkPath;
        MojLogInfo(s_log, _T("Created symlink %s for %s"), linkPath.data(), shardInfo.deviceUri.data());
    } else {
        MojLogWarning(s_log, _T("Can't create symlink %s for %s"), linkPath.data(), shardInfo.deviceUri.data());
    }

    return err;
}

MojErr MojDbMediaLinkManager::removeLink(const MojDbShardEngine::ShardInfo& shardInfo)
{
    MojLogTrace(s_log);
    MojErr err;

    err = MojUnlink(shardInfo.mountPath);
    if (err != MojErrNone) {
        MojLogWarning(s_log, _T("Can't remove symlink %s"), shardInfo.mountPath.data());
    }

    return err;
}
