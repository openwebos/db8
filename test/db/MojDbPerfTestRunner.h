/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBPERFTESTRUNNER_H_
#define MOJDBPERFTESTRUNNER_H_

#include "db/MojDbDefs.h"
#include "core/MojTestRunner.h"

extern const MojChar* const MojDbTestDir;

class MojDbPerfTestRunner : public MojTestRunner
{
private:
	void runTests();
};

#endif /* MOJDBPERFTESTRUNNER_H_ */
