/* @@@LICENSE
 *
 * Copyright (c) 2014 LG Electronics
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

#include <leveldb/status.h>
#include "core/MojCoreDefs.h"
#include "core/MojErr.h"
#include "db-luna/leveldb/defs.h"

MojErr LdbToMojErr(const leveldb::Status& s)
{
    if (!s.ok()) {
        if (s.IsIOError())
            return MojErrDbIO;
        else if (s.IsCorruption())
            return MojErrDbCorruptDatabase;
        else
            return MojErrDbFatal;
    }

    return MojErrNone;
}

