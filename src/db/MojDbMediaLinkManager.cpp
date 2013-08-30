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
#include <db/MojDb.h>
#include <sstream>
#include <algorithm>

const MojChar* const MediaLinkDirectory = _T("/media/mountpoint");

MojLogger MojDbMediaLinkManager::s_log(_T("db.shardLinkManager"));


MojDbMediaLinkManager::MojDbMediaLinkManager()
{
    MojLogTrace(s_log);

    m_dir.assign(MediaLinkDirectory);
}

MojErr MojDbMediaLinkManager::setLinkDirectory(const MojString& dir)
{
    m_dir = dir;
    return MojErrNone;
}

MojErr MojDbMediaLinkManager::getLinkPath(MojUInt32 shardId, MojString& linkPath)
{
    std::stringstream sstream;
    sstream << "0x" << std::hex << shardId;
    std::string result = sstream.str();

    std::transform(result.begin(), result.end(), result.begin(), toupper);

    return linkPath.format("%s/%s", m_dir.data(), result.c_str());
}

MojErr MojDbMediaLinkManager::createLink(const MojDbShardEngine::ShardInfo& shardInfo)
{
    MojLogTrace(s_log);

    MojErr err;
    MojString linkPath;

    err = getLinkPath(shardInfo.id, linkPath);
    MojErrCheck(err);

    MojLogDebug(s_log, _T("create link %s"), linkPath.data());

    MojUnlink(linkPath);
    err = MojSymlink(shardInfo.mountPath, linkPath);
    if (err != MojErrNone) {
        MojLogWarning(s_log, _T("Can't create symlink %s for %s"), linkPath.data(), shardInfo.mountPath.data());
    }

    return err;
}

MojErr MojDbMediaLinkManager::removeLink(const MojDbShardEngine::ShardInfo& shardInfo)
{
    MojLogTrace(s_log);
    MojErr err;

    MojString linkPath;
    err = getLinkPath(shardInfo.id, linkPath);
    MojErrCheck(err);

    err = MojUnlink(linkPath);
    if (err != MojErrNone) {
        MojLogWarning(s_log, _T("Can't remove symlink %s"), linkPath.data());
    }

    return err;
}
