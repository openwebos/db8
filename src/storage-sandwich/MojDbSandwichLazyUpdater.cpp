/* @@@LICENSE
*
*  Copyright (c) 2014 LG Electronics, Inc.
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

#include "db/MojDb.h"
#include "MojDbSandwichLazyUpdater.h"
#include <time.h>

////////////////////MojDbSandwichLazyUpdater////////////////////////////////////////////

MojDbSandwichLazyUpdater::MojDbSandwichLazyUpdater()
    : m_thread(MojInvalidThread), m_stop(false)
{
}

MojDbSandwichLazyUpdater::~MojDbSandwichLazyUpdater()
{
    deinit();
}

MojErr MojDbSandwichLazyUpdater::start()
{
    MojErr err = MojErrNone;

    if (MojInvalidThread == m_thread)
    {
        err = MojThreadCreate(m_thread, &threadMain, this);
        MojErrCheck(err);
    }
    m_stop = false;

    return (err);
}

MojErr MojDbSandwichLazyUpdater::stop()
{
    MojErr err = MojErrNone;

    m_stop = true;
    return (err);
}

MojErr MojDbSandwichLazyUpdater::deinit()
{
    MojErr err = MojErrNone;

    stop();
    if (MojInvalidThread != m_thread)
    {
        MojErr threadErr = MojErrNone;
        MojErr joinErr = MojThreadJoin(m_thread, threadErr);

        MojErrAccumulate(err, threadErr);
        MojErrAccumulate(err, joinErr);
    }

    return (err);
}

MojErr MojDbSandwichLazyUpdater::threadMain(void* arg)
{
    MojDbSandwichLazyUpdater* thiz_class = (MojDbSandwichLazyUpdater*) arg;
    MojAssert(thiz_class);

    while (thiz_class->m_stop == false) {
        thiz_class->sync();
        MojSleep(UpdaterIntervalMsec * 1000);
    }

    return MojErrNone;
}

MojErr MojDbSandwichLazyUpdater::open(leveldb::DB* pdb)
{
    if(pdb == 0)
        return MojErrUnknown;

    MojThreadGuard guard(m_mutex);
    if(m_dbs.find(pdb) == m_dbs.end()) {
        m_dbs[pdb] = false;
    }

    return MojErrNone;
}

MojErr MojDbSandwichLazyUpdater::close(leveldb::DB* pdb)
{
    MojThreadGuard guard(m_mutex);
    if(m_dbs.find(pdb) != m_dbs.end()) {
        onesync(pdb);
        m_dbs.erase(pdb);
    }

    return MojErrNone;
}

MojErr MojDbSandwichLazyUpdater::sendEvent(leveldb::DB* pdb)
{
    MojThreadGuard guard(m_mutex);
    m_dbs[pdb] = true;

    return MojErrNone;
}

MojErr MojDbSandwichLazyUpdater::sync()
{
    Container dbs_copy;

    MojThreadGuard guard(m_mutex);
    dbs_copy.clear();
    for (Container::iterator it = m_dbs.begin();
        it != m_dbs.end();
        ++it)
    {
        if(it->first && it->second == true) {
            it->second = false;
            dbs_copy[it->first] = true;
        }
    }
    guard.unlock();

    for (Container::iterator it = dbs_copy.begin();
        it != dbs_copy.end();
        ++it) {
        if(it->first && it->second == true) {
            onesync(it->first);
        }
    }

    LOG_DEBUG("[db_ldb] do sync\n");
    return MojErrNone;
}

void MojDbSandwichLazyUpdater::onesync(leveldb::DB* pdb)
{
    // Delete meaning less data for sync.
    // TODO: Rationalize followings.
    leveldb::WriteOptions write_options;
    write_options.sync = true;
    pdb->Delete(write_options, "_______dummy______");
}

