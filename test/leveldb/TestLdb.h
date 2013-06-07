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
 *  @file TestLdb.h
 ****************************************************************/

#ifndef __TestLdb_h__
#define __TestLdb_h__

#include <leveldb/db.h>

#include "core/MojString.h"

#include "Runner.h"

#define AssertLdbOk( S ) ASSERT_TRUE( (S).ok() ) << (S).ToString();
#define ExpectLdbOk( S ) EXPECT_TRUE( (S).ok() ) << (S).ToString();

struct TestLdb : public ::testing::Test
{
    MojString file;
    leveldb::DB* db;

    void SetUp()
    {
        MojAssertNoErr( file.format(_T("%s/%s"), tempFolder, "playground.ldb") );

        leveldb::Options options;
        options.create_if_missing = true;
        AssertLdbOk( leveldb::DB::Open(options, file.data(), &db) );
    }

    void TearDown()
    {
        delete db;
        leveldb::Options options;
        leveldb::DestroyDB(file.data(), options);
    }

    void initSample()
    {
        leveldb::WriteOptions wo;
        AssertLdbOk( db->Put(wo, "b", "db-0") );
        AssertLdbOk( db->Put(wo, "d", "db-1") );
        AssertLdbOk( db->Put(wo, "e", "db-2") );
        AssertLdbOk( db->Put(wo, "g", "db-3") );
    }
};

#endif
