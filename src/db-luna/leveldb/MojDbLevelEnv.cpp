/* @@@LICENSE
*
*  Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*  Copyright (c) 2013 LG Electronics
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

#include "db-luna/leveldb/MojDbLevelEnv.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"

const MojChar* const MojDbLevelEnv::LockFileName = _T("_ldblock");

// this class is mostly placeholder
MojDbLevelEnv::MojDbLevelEnv()
 : m_db(NULL)
{
    MojLogTrace(MojDbLevelEngine::s_log);
}


MojDbLevelEnv::~MojDbLevelEnv()
{
    MojLogTrace(MojDbLevelEngine::s_log);
    close();
}

MojErr MojDbLevelEnv::configure(const MojObject& conf)
{
    MojLogTrace(MojDbLevelEngine::s_log);

    bool found = false;
    MojString logDir;
    MojErr err = conf.get(_T("logDir"), logDir, found);
    MojErrCheck(err);

    m_logDir = logDir;

    return MojErrNone;
}

MojErr MojDbLevelEnv::open(const MojChar* dir)
{
    return lockDir(dir);
}

MojErr MojDbLevelEnv::close()
{
    unlockDir();

    /*if (m_db)
        delete m_db;*/

    return MojErrNone;
}

MojErr MojDbLevelEnv::lockDir(const MojChar* path)
{
    MojAssert(path);

    MojErr err = MojCreateDirIfNotPresent(path);
    MojErrCheck(err);
    err = m_lockFileName.format(_T("%s/%s"), path, LockFileName);
    MojErrCheck(err);
    err = m_lockFile.open(m_lockFileName, MOJ_O_RDWR | MOJ_O_CREAT, MOJ_S_IRUSR | MOJ_S_IWUSR);
    MojErrCheck(err);
    err = m_lockFile.lock(LOCK_EX | LOCK_NB);
    MojErrCatch(err, MojErrWouldBlock) {
        (void) m_lockFile.close();
        MojErrThrowMsg(MojErrLocked, _T("Database at '%s' locked by another instance."), path);
    }
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbLevelEnv::unlockDir()
{
    MojErr err = MojErrNone;
    if (m_lockFile.open()) {
            // unlink before we close to ensure that we hold
        // the lock to the bitter end
        MojErr errClose = MojUnlink(m_lockFileName);
        MojErrAccumulate(err, errClose);
        errClose = m_lockFile.close();
        MojErrAccumulate(err, errClose);
    }

    return err;
}

MojErr MojDbLevelEnv::postCommit(gsize updateSize)
{
    MojLogTrace(MojDbLevelEngine::s_log);
    return MojErrNone;
}
