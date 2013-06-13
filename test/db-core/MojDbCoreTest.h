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
 *  @file MojDbCoreTest.h
 ****************************************************************/

#ifndef __MOJDBCORETEST_H
#define __MOJDBCORETEST_H

#include "db/MojDb.h"

#include "Runner.h"

struct MojDbCoreTest : public ::testing::Test
{
    MojDb db;
    std::string path;

    void SetUp()
    {
        const ::testing::TestInfo* const test_info =
          ::testing::UnitTest::GetInstance()->current_test_info();

        path = std::string(tempFolder) + '/'
             + test_info->test_case_name() + '-' + test_info->name();

        // open
        MojAssertNoErr( db.open(path.c_str()) );
    }

    void TearDown()
    {
        // TODO: clean DB
        MojExpectNoErr( db.close() );
    }
};

#endif
