/****************************************************************
 * @@@LICENSE
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
 * LICENSE@@@
 ****************************************************************/

/****************************************************************
 *  @file Runner.cpp
 ****************************************************************/

#include <cstdlib>
#include "gtest/gtest.h"

#include "core/MojUtil.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#else
#error "Database Engine doesn't set. See README.txt"
#endif

const char *tempFolder;

int main(int argc, char **argv) {
    char buf[128] = "/tmp/mojodb-test-dir-XXXXXX";
    tempFolder = mkdtemp(buf);
    if (!tempFolder) tempFolder = "/tmp/mojodb-test-dir"; // fallback

   // set up engine
#ifdef MOJ_USE_BDB
   MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
   MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#endif

    // stup and run tests
    testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();

    // cleanup temp folder
    MojRmDirRecursive(tempFolder);

    return status;
}
