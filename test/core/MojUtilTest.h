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


#ifndef MOJUTILTEST_H_
#define MOJUTILTEST_H_

#include "MojCoreTestRunner.h"

class MojUtilTest: public MojTestCase
{
public:
	MojUtilTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr testBase64(const MojByte* src, MojSize size, const MojChar* expected);
	MojErr testBase64Mime(const MojByte* src, MojSize size, const MojChar* expected);
	MojErr testBase64Err(const MojChar* str);
	MojErr testBase64MimeErr(const MojChar* str);
	MojErr writeFile(const MojChar* path, const MojChar* data);
};

#endif /* MOJUTILTEST_H_ */
