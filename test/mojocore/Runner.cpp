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

const char *tempFolder;

int main(int argc, char **argv) {
    char buf[128] = "/tmp/mojo-test-dir-XXXXXX";
    tempFolder = mkdtemp(buf);
    if (!tempFolder) tempFolder = "/tmp/mojo-test-dir"; // fallback

    // stup and run tests
    testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();

    // cleanup temp folder
    MojRmDirRecursive(tempFolder);

    return status;
}
