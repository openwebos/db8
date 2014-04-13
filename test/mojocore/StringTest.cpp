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
 *  @file StringTest.cpp
 */

#include "core/MojString.h"

#include "Runner.h"

TEST(StringTest, rfind)
{
    MojString str;
    const size_t len = 13 - 1 - sizeof(MojAtomicT);
    MojAssertNoErr( str.assign("axaaaaaaaaaaa", len) );
    // optimized memrchr may read more bytes than requested for allocation
    // in Valgrind pre 3.8.1 this causes a false positive about reading 8 bytes
    // by memrchr
    EXPECT_EQ( 1, str.rfind('x') );
    EXPECT_EQ( 1, str.find('x') );

    const size_t len2 = 16 - 1 - sizeof(MojAtomicT);
    MojAssertNoErr( str.assign("axaaaaaaaaaaa", len2) );
    EXPECT_EQ( 1, str.rfind('x') );
}
