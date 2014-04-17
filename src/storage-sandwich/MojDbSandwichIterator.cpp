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

#include <cassert>

#include <leveldb/db.h>
#include "MojDbSandwichIterator.h"
#include "MojDbSandwichEngine.h"
#include <iostream>

MojDbSandwichIterator::MojDbSandwichIterator (database_t* database)
    : m_database(database)
{
    m_it = m_database->NewIterator().release();
    toFirst();
}

MojDbSandwichIterator::~MojDbSandwichIterator()
{
    delete m_it;
#ifdef MOJ_DEBUG
    m_it = 0;
#endif
}

MojDbSandwichIterator& MojDbSandwichIterator::operator++ ()
{
    assert( !m_end );
    if (m_start) {
        toFirst();
        return *this;
    }

    if (m_it->Valid()) {
        m_it->Next();

        if (!m_it->Valid()) {
            m_end = true;
        }

    } else {
        m_end = true;
    }

    return *this;
}

MojDbSandwichIterator& MojDbSandwichIterator::operator-- ()
{
    assert( !m_start );
    if (m_end) {
        toLast();
        return *this;
    }

    if (m_it->Valid()) {
        m_it->Prev();

        if (!m_it->Valid()) {
            m_start = true;
        }
    } else {
        m_start = true;
    }

    return *this;
}

void MojDbSandwichIterator::save()
{
    if (m_it->Valid())
        savedKey = m_it->key();
    else
        savedKey.clear();

    delete m_it;
#ifdef MOJ_DEBUG
    m_it = 0;
#endif
}

void MojDbSandwichIterator::restore()
{
    m_it = m_database->NewIterator().release();

    if (!savedKey.empty())
        seek(savedKey.ToString());
}

void MojDbSandwichIterator::seek(const std::string& key)
{
    m_it->Seek(key);
    m_start = false;
    m_end = !m_it->Valid();
}

void MojDbSandwichIterator::toBegin()
{
    m_start = true;
    m_end = false;
}

void MojDbSandwichIterator::toEnd()
{
    m_start = false;
    m_end = true;
}

void MojDbSandwichIterator::toFirst()
{
    m_it->SeekToFirst();
    m_start = false;
    m_end = !m_it->Valid();
}

void MojDbSandwichIterator::toLast()
{
    m_it->SeekToLast();

    m_start = !m_it->Valid();
    m_end = false;
}
