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
 *  @file TestTxn.h
 ****************************************************************/

#ifndef __TestTxn_h__
#define __TestTxn_h__

#include "db-luna/leveldb/MojDbLevelTxn.h"

#include "TestLdb.h"

struct TestTxn: public TestLdb
{
    MojDbLevelTableTxn ttxn;

    static void initTxnSampleA(MojDbLevelTableTxn &ttxn)
    {
        ttxn.Put("a", "txn-0");
        ttxn.Delete("b");
        ttxn.Put("c", "txn-1");
        ttxn.Put("d", "txn-2");
        ttxn.Delete("e");
        ttxn.Put("f", "txn-3");
        ttxn.Put("h", "txn-4");
    }
};

#endif

