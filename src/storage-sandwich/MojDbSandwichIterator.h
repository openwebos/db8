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

#ifndef MOJDBLEVELITERATOR_H
#define MOJDBLEVELITERATOR_H

#include "core/MojNoCopy.h"
#include "MojDbSandwichEngine.h"

#include <string>
#include <leveldb/slice.h>

namespace leveldb
{
    class Iterator;
    class DB;
}

class MojDbSandwichIterator : public MojNoCopy
{

    typedef leveldb::Iterator iterator_t;
    typedef MojDbSandwichEngine::BackendDb::Part database_t;
    typedef leveldb::Slice key_t;

public:
    MojDbSandwichIterator(database_t* database);
    ~MojDbSandwichIterator();

    inline bool isBegin() const { return m_start; }
    inline bool isEnd() const { return m_end; }
    inline bool isValid() const { return !(m_start || m_end); }

    void save();
    void restore();

    MojDbSandwichIterator& operator++ ();
    //MojDbSandwichIterator operator++ (int);
    MojDbSandwichIterator& operator-- ();
    //MojDbSandwichIterator operator-- (int);

    //MojDbSandwichIterator& operator= (iterator_t* iterator);
    void seek(const std::string& key);

    iterator_t* operator->() { return m_it; }
    const iterator_t* operator->() const { return m_it; }

    void toBegin();
    void toEnd();
    void toFirst();
    void toLast();

private:
    database_t* m_database;
    iterator_t* m_it;
    key_t savedKey;

    bool m_start;
    bool m_end;
};

#endif
