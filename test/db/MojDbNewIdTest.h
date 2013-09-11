/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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


#ifndef MOJDBNEWIDTEST_H_
#define MOJDBNEWIDTEST_H_

#include "MojDbTestRunner.h"
#include "db/MojDbShardEngine.h"

class MojDbNewIdTest : public MojTestCase
{
public:
    MojDbNewIdTest();

    virtual MojErr run();
    virtual void cleanup();

private:
    MojErr initShardEngine (MojDbShardEngine* ip_engine);
    MojErr putTestKind(MojDb& db);
    MojErr putTest(MojDb& db);
    MojErr duplicateTest(MojDb& db);
    MojUInt32 getKindCount (const MojChar* MojQueryStr, MojDb& db);
    MojErr addShardIdTest(MojDb& db);
    MojErr generateItem (MojUInt32 i_id, MojDbShardEngine::ShardInfo& o_shardInfo);
};

#endif /* MOJDBNEWIDTEST_H_ */
