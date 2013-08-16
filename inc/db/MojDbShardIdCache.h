/* @@@LICENSE
*
*      Copyright (c) 2013 LG Electronics, Inc.
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


#ifndef MOJDBSHARDIDCACHE_H_
#define MOJDBSHARDIDCACHE_H_

#include "core/MojCoreDefs.h"
#include "core/MojErr.h"
#include <set>

class MojDbShardIdCache : private MojNoCopy
{
public:

    MojDbShardIdCache();
    ~MojDbShardIdCache();

    MojErr init ();
    bool isExist (MojUInt32 id);
    MojErr put (MojUInt32 id);
    MojErr del (MojUInt32 id);

private:
    MojErr _read (void);
    MojErr _write (void);

    std::set<MojUInt32> m_set;
    static MojLogger s_log;
};


#endif /* MOJDBSHARDIDCACHE_H_ */
