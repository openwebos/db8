/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics, Inc.
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

#include "db-luna/leveldb/MojDbLevelIterator.h"
#include "leveldb/db.h"

#include <iostream>

MojDbLevelIterator::MojDbLevelIterator (database_t* database)
    : m_database(database)
{
    m_it = m_database->NewIterator(leveldb::ReadOptions());
    toFirst();
}

MojDbLevelIterator::MojDbLevelIterator(iterator_t* iterator, database_t* database)
    : m_database(database), m_it(iterator)
{
}

MojDbLevelIterator::~MojDbLevelIterator()
{
    delete m_it;
#ifdef MOJ_DEBUG
    m_it = 0;
#endif
}

MojDbLevelIterator& MojDbLevelIterator::operator++ ()
{
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

MojDbLevelIterator& MojDbLevelIterator::operator-- ()
{
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

void MojDbLevelIterator::save()
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

void MojDbLevelIterator::restore()
{
    m_it = m_database->NewIterator(leveldb::ReadOptions());

    if (!savedKey.empty())
        seek(savedKey.ToString());
}

void MojDbLevelIterator::seek(const std::string& key)
{
    m_it->Seek(key);
    if (!m_it->Valid()) {
        m_end = true;
        m_start = true;
    } else {
        m_end = false;
        m_start = false;
    }
}

void MojDbLevelIterator::toBegin()
{
    m_it->SeekToFirst();
    if (m_it->Valid()) {
        m_start = true;
        m_end = false;
    } else {
        m_start = true;
        m_end = true;
    }
}

void MojDbLevelIterator::toEnd()
{
    m_it->SeekToLast();

    if (m_it->Valid()) {
        m_start = false;
        m_end = true;
    } else {
        m_start = true;
        m_end = true;
    }
}

void MojDbLevelIterator::toFirst()
{
    m_it->SeekToFirst();

    m_start = !m_it->Valid();
    m_end = !m_it->Valid();
}
void MojDbLevelIterator::toLast()
{
    m_it->SeekToLast();

    m_start = !m_it->Valid();
    m_end = !m_it->Valid();
}
