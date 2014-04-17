/* @@@LICENSE
*
* Copyright (c) 2009-2014 LG Electronics, Inc.
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

#include "MojDbSandwichFactory.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichEnv.h"

namespace { MojDbStorageEngine::Registrator<MojDbSandwichFactory> factorySandwichDB; }

MojErr MojDbSandwichFactory::create(MojRefCountedPtr<MojDbStorageEngine>& engineOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    engineOut.reset(new MojDbSandwichEngine());
    MojAllocCheck(engineOut.get());

    return MojErrNone;
}

MojErr MojDbSandwichFactory::createEnv(MojRefCountedPtr<MojDbEnv>& envOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    envOut.reset(new MojDbSandwichEnv());
    MojAllocCheck(envOut.get());

    return MojErrNone;
}

const MojChar* MojDbSandwichFactory::name() const { return _T("sandwich"); }
