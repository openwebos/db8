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

const MojChar* const MediaLinkDirectory = _T("/media/mountpoint");

MojLogger MojDbMediaLinkManager::s_log(_T("db.shardLinkManager"));


MojDbMediaLinkManager::MojDbMediaLinkManager()
{
    MojString linkName;
    MojErr err = linkName.format(MediaLinkDirectory);
    if (err == MojErrNone) {
        setLinkDirectory(linkName);
    } else {
        MojLogWarning(s_log, _T("Can't init default value %s"), MediaLinkDirectory);
    }

    //setLinkDirectory(MediaLinkDirectory);
}

MojErr MojDbMediaLinkManager::setLinkDirectory(const MojString& dir)
{
    m_dir = dir;
    return MojErrNone;
}

MojErr MojDbMediaLinkManager::createLink(const MojDbShardEngine::ShardInfo& shardId)
{
    MojLogTrace(s_log);
    MojErr err;

    MojString linkName = m_dir;
    err = linkName.format("%d", shardId.id);
    MojErrCheck(err);

    err = MojSymlink(shardId.mountPath, linkName);
    if (err != MojErrNone) {
        MojLogWarning(s_log, _T("Can't create symlink %s"), linkName.data());
    }

    return err;
}

MojErr MojDbMediaLinkManager::removeLink(const MojDbShardEngine::ShardInfo& shardId)
{
    MojLogTrace(s_log);
    MojErr err;

    MojString linkName = m_dir;
    err = linkName.format("%d", shardId.id);
    MojErrCheck(err);

    err = MojUnlink(linkName);
    if (err != MojErrNone) {
        MojLogWarning(s_log, _T("Can't remove symlink %s"), linkName.data());
    }

    return err;
}
