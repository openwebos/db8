/****************************************************************
 * @@@LICENSE
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
 * LICENSE@@@
 ****************************************************************/

/**
 *  @file BatchTest.cpp
 */

#include "MojDbCoreTest.h"

using ::testing::PrintToString;

namespace {
    const MojChar* const MojKindStr =
        _T("{\"id\":\"com.webos.test:1\", \"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"sourceIndex\",\"props\":[{\"name\":\"sourceIndex\"}, {\"name\":\"Handle\"}]}]}");

    const MojChar* const MojTestObj1Str = _T("{")
                  _T("\"_kind\":\"com.webos.test:1\",")
                  _T("\"channelId\":\"3_1_15_15_0_16_0\",")
                  _T("\"programId\":\"0_16_0\",")
                  _T("\"signalChannelId\":\"0_16_0\",")
                  _T("\"channelMode\":\"Cable\",")
                  _T("\"channelModeId\":1,")
                  _T("\"channelType\":\"CableDigitalTV\",")
                  _T("\"channelTypeId\":4,")
                  _T("\"channelNumber\":\"15\",")
                  _T("\"channelName\":\"Nickelodeon\",")
                  _T("\"satelliteName\":\"\",")
                  _T("\"Handle\":0,")
                  _T("\"configurationId\":0,")
                  _T("\"Frequency\":0,")
                  _T("\"Bandwidth\":0,")
                  _T("\"serviceType\":1,")
                  _T("\"favoriteGroup\":[],")
                  _T("\"favoriteIdxA\":0,")
                  _T("\"favoriteIdxB\":0,")
                  _T("\"favoriteIdxC\":0,")
                  _T("\"favoriteIdxD\":0,")
                  _T("\"shortCut\":0,")
                  _T("\"sourceIndex\":3,")
                  _T("\"physicalNumber\":1,")
                  _T("\"channelMajMinNo\":\"04-00015-000-001\",")
                  _T("\"sortIndex\":\"0\",")
                  _T("\"majorNumber\":15,")
                  _T("\"minorNumber\":15,")
                  _T("\"TSID\":0,")
                  _T("\"SVCID\":16,")
                  _T("\"ONID\":0,")
                  _T("\"skipped\":false,")
                  _T("\"locked\":false,")
                  _T("\"descrambled\":true,")
                  _T("\"scrambled\":false,")
                  _T("\"fineTuned\":false,")
                  _T("\"DTV\":true,")
                  _T("\"ATV\":false,")
                  _T("\"Invisible\":false,")
                  _T("\"Numeric\":false,")
                  _T("\"specialService\":false,")
                  _T("\"HDTV\":false,")
                  _T("\"TV\":true,")
                  _T("\"Radio\":false,")
                  _T("\"Data\":false,")
                  _T("\"satelliteLcn\":false,")
                  _T("\"PrimaryCh\":true,")
                  _T("\"GroupIdList\":[{\"GroupId\":0},{\"GroupId\":1}],")
                  _T("\"CASystemIDListCount\":0,")
                  _T("\"CASystemIDList\":{}")
               _T("}");

const MojChar* const MojTestObj2Str = _T("{")
               _T("\"_kind\":\"com.webos.test:1\",")
               _T("\"channelId\":\"3_1_911_20_6_17_1000\",")
               _T("\"programId\":\"6_17_1000\",")
               _T("\"signalChannelId\":\"6_17_1000\",")
               _T("\"channelMode\":\"Cable\",")
               _T("\"channelModeId\":1,")
               _T("\"channelType\":\"CableDigitalTV\",")
               _T("\"channelTypeId\":4,")
               _T("\"channelNumber\":\"911\",")
               _T("\"channelName\":\"Eurosport\",")
               _T("\"satelliteName\":\"\",")
               _T("\"Handle\":0,")
               _T("\"configurationId\":0,")
               _T("\"Frequency\":738000,")
               _T("\"Bandwidth\":0,")
               _T("\"serviceType\":1,")
               _T("\"favoriteGroup\":[],")
               _T("\"favoriteIdxA\":0,")
               _T("\"favoriteIdxB\":0,")
               _T("\"favoriteIdxC\":0,")
               _T("\"favoriteIdxD\":0,")
               _T("\"shortCut\":0,")
               _T("\"sourceIndex\":3,")
               _T("\"physicalNumber\":1,")
               _T("\"channelMajMinNo\":\"04-00911-000-001\",")
               _T("\"sortIndex\":\"0\",")
               _T("\"majorNumber\":911,")
               _T("\"minorNumber\":20,")
               _T("\"TSID\":6,")
               _T("\"SVCID\":17,")
               _T("\"ONID\":1000,")
               _T("\"skipped\":false,")
               _T("\"locked\":false,")
               _T("\"descrambled\":true,")
               _T("\"scrambled\":false,")
               _T("\"fineTuned\":false,")
               _T("\"DTV\":true,")
               _T("\"ATV\":false,")
               _T("\"Invisible\":false,")
               _T("\"Numeric\":false,")
               _T("\"specialService\":false,")
               _T("\"HDTV\":false,")
               _T("\"TV\":true,")
               _T("\"Radio\":false,")
               _T("\"Data\":false,")
               _T("\"satelliteLcn\":false,")
               _T("\"PrimaryCh\":true,")
               _T("\"GroupIdList\":[{\"GroupId\":0},{\"GroupId\":1}],")
               _T("\"CASystemIDListCount\":0,")
               _T("\"CASystemIDList\":{}")
            _T("}");


struct BatchTest : public MojDbCoreTest
{
    void SetUp()
    {
        MojDbCoreTest::SetUp();

        // add type
        MojObject obj;
        MojAssertNoErr( obj.fromJson(MojKindStr) );
        MojAssertNoErr( db.putKind(obj) );
    }
};

TEST_F(BatchTest, mergePutf)
{
    MojDbReq req;
    MojErr err = req.begin ( &db, false );
    MojExpectNoErr ( err );

    MojObject object1;
    err = object1.fromJson(MojTestObj1Str);
    MojExpectNoErr ( err );

    err = db.put ( object1, MojDb::FlagNone, req );
    MojExpectNoErr ( err );

    MojDbQuery query;
    err = query.from("com.webos.test:1");
    MojExpectNoErr ( err );
    err = query.where("sourceIndex", MojDbQuery::OpEq, 3);
    MojExpectNoErr ( err );
    err = query.where("Handle", MojDbQuery::OpEq, 0);
    MojExpectNoErr ( err );

    MojObject object2;
    err = object2.fromJson(MojTestObj2Str);
    MojExpectNoErr ( err );

    MojUInt32 count = 0;
    err = db.merge(query, object2, count, MojDb::FlagNone, req);
    MojExpectNoErr ( err );

    ASSERT_EQ(1, count);
}

}
