/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013-2014 LG Electronics, Inc.
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
 *  @file Runner.h
 ****************************************************************/

#ifndef __RUNNER_H
#define __RUNNER_H

#include <string>

#include <core/MojString.h>
#include <core/MojErr.h>
#include <core/MojObject.h>

#include "gtest/gtest.h"

namespace std {
    inline std::string to_string(MojErr err)
    {
        MojString strOut;
        if (MojErrToString(err, strOut) == MojErrNone)
        { return std::string(strOut.data(), strOut.length()); }
        else
        { return "Unknown error #" + std::to_string(int(err)); }
    }
}

inline ::testing::AssertionResult isErr(MojErr err, MojErr exp = MojErrNone)
{
    if (exp == err) return ::testing::AssertionSuccess();
    else
    {
        return ::testing::AssertionFailure()
            << "Expected " << std::to_string(exp)
            << " but got " << std::to_string(err);
    }
}

#define MojAssertNoErr( E ) ASSERT_TRUE( isErr(E) )
#define MojExpectNoErr( E ) EXPECT_TRUE( isErr(E) )

#define MojAssertErr( ExpE, E ) ASSERT_TRUE( isErr(E, ExpE) )
#define MojExpectErr( ExpE, E ) EXPECT_TRUE( isErr(E, ExpE) )

extern const char *tempFolder;

inline void PrintTo(const MojString &obj, ::std::ostream *os)
{
    MojString str;
    *os << ::testing::PrintToString(std::string(str.data(), str.length()));
}

inline void PrintTo(const MojObject &obj, ::std::ostream *os)
{
    MojString str;
    MojAssertNoErr( obj.toJson(str) );
    *os << std::string(str.data(), str.length());
}

#endif
