/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics
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


#include "MojDbPerfTestRunner.h"
#include "MojDbPerfCreateTest.h"
#include "MojDbPerfReadTest.h"
#include "MojDbPerfUpdateTest.h"
#include "MojDbPerfDeleteTest.h"
#include "MojDbPerfIndexTest.h"


const MojChar* const MojDbTestDir = _T("mojodb-test-dir");

int main(int argc, char** argv)
{
	MojDbPerfTestRunner runner;
	return runner.main(argc, argv);
}

void MojDbPerfTestRunner::runTests()
{
	test(MojDbPerfIndexTest());
	test(MojDbPerfCreateTest());
	test(MojDbPerfReadTest());
	test(MojDbPerfUpdateTest());
	test(MojDbPerfDeleteTest());
}
