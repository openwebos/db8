/* @@@LICENSE
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
* LICENSE@@@ */

#ifndef MOJDBLEVELSEQ_H_
#define MOJDBLEVELSEQ_H_

#include "MojDbSandwichDatabase.h"
#include "MojDbSandwichItem.h"

class MojDbSandwichSeq : public MojDbStorageSeq
{
public:
    MojDbSandwichSeq() : m_db(NULL), m_next(0), m_allocated(0) {}
    ~MojDbSandwichSeq();

    MojErr open(const MojChar* name, MojDbSandwichDatabase* db);
    virtual MojErr close();
    virtual MojErr get(MojInt64& valOut);


private:
    friend class MojDbSandwichEngine;

    MojErr store(MojInt64 next);
    MojErr allocateMore();

    MojDbSandwichDatabase* m_db;
    MojDbSandwichItem m_key;
    MojInt64 m_next;
    MojInt64 m_allocated;
};

#endif
