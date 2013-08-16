/* @@@LICENSE
*
*      Copyright (c) 2013 LG Electronics, Inc.
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


#include "MojDbShardManagerTest.h"
#include "db/MojDb.h"
#include "db/MojDbShardEngine.h"
#include "db/MojDbShardIdCache.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else
#error "Doesn't specified database type. See macro MOJ_USE_BDB and MOJ_USE_LDB"
#endif


MojDbShardManagerTest::MojDbShardManagerTest()
    : MojTestCase(_T("MojDbShardManagerTest"))
{
}

MojErr MojDbShardManagerTest::run()
{
    //setup the test storage engine
#ifdef MOJ_USE_BDB
    MojDbStorageEngine::setEngineFactory (new MojDbBerkeleyFactory);
#elif MOJ_USE_LDB
    MojDbStorageEngine::setEngineFactory (new MojDbLevelFactory);
#else
#error "Not defined engine type"
#endif

    MojErr err = MojErrNone;
    MojDb db;

    cleanup();

    // open
    //err = db.open(MojDbTestDir);
    //MojTestErrCheck(err);

    MojDbShardIdCache* p_cache = db.shardIdCache();
    MojDbShardEngine* p_eng = db.shardEngine();

    (void)_testShardIdCache(p_cache);

    (void)_testShardManager(p_eng);

    //err = db.close();
    //MojTestErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardManagerTest::_testShardIdCache (MojDbShardIdCache* ip_cache)
{
    MojUInt32 arr[] = { 0x01, 0x000000FF, 0x0000FFFF, 0x00FFFFFF, 0xFFFFFFFF, 100 };

    ip_cache->init();

    displayMessage("ShardIdCache: Put 5 id's.. ");
    for (MojInt16 i = 0; i < 5; i++)
    {
        ip_cache->put(arr[i]);
    }

    displayMessage("compare.. ");
    for (MojInt16 i = 0; i < 5; i++)
    {
        if (!ip_cache->isExist(arr[i]))
        {
            displayMessage("[id %d is wrong] ", i);
        }
    }

    if (ip_cache->isExist(arr[5]))
        displayMessage("compare id6 is wrong ");

    displayMessage("done\n");

    return MojErrNone;
}

MojErr MojDbShardManagerTest::_testShardManager (MojDbShardEngine* ip_eng)
{
    MojUInt32 id;
    MojString str;

    //generate id
    str.assign("SunDisk1");
    //MojDbShardEngine::computeShardId(str, id);

    //
    //ip_eng->getIdForPath(str, id);

    return MojErrNone;
}

MojErr MojDbShardManagerTest::displayMessage(const MojChar* format, ...)
{
    va_list args;
    va_start (args, format);
    MojErr err = MojVPrintF(format, args);
    va_end(args);
    MojErrCheck(err);

    return MojErrNone;
}

void MojDbShardManagerTest::cleanup()
{
    (void) MojRmDirRecursive(MojDbTestDir);
}

