/* @@@LICENSE
*
* Copyright (c) 2014 LG Electronics, Inc.
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

#ifndef MOJDBLEVELDATABASELAZYUPDATER_H
#define MOJDBLEVELDATABASELAZYUPDATER_H

#include <map>

#include <leveldb/db.h>
#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"

class MojDbSandwichLazyUpdater
{
public:
    static const MojUInt32 UpdaterIntervalMsec = 1000;

    MojDbSandwichLazyUpdater();
    ~MojDbSandwichLazyUpdater();

    MojErr open(leveldb::DB* pdb);
    MojErr close(leveldb::DB* pdb);
    MojErr sendEvent(leveldb::DB* pdb);
    MojErr sync();

    MojErr start();
    MojErr stop();

protected:
    void onesync(leveldb::DB* pdb);
    MojErr deinit();

private:
    typedef std::map<leveldb::DB*, bool> Container;

    static MojErr threadMain(void* arg);

    MojThreadT m_thread;
    mutable MojThreadMutex m_mutex;
    bool m_stop;

    Container m_dbs;
};

#endif

