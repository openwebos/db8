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

inline ::testing::AssertionResult noErr(MojErr err)
{
	if (err == MojErrNone) return ::testing::AssertionSuccess();
	else return ::testing::AssertionFailure() << std::to_string(err);
}

#define MojAssertNoErr( E ) ASSERT_TRUE( noErr(E) )
#define MojExpectNoErr( E ) EXPECT_TRUE( noErr(E) )

extern const char *tempFolder;

#endif
