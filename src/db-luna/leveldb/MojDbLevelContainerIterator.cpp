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

#include "db-luna/leveldb/MojDbLevelContainerIterator.h"

MojDbLevelContainerIterator::MojDbLevelContainerIterator (std::map<std::string, std::string>& database)
    : m_container(database)
{
    toFirst();
}

MojDbLevelContainerIterator::MojDbLevelContainerIterator(iterator_t iterator, std::map<std::string, std::string>& database)
    : m_container(database), m_it(iterator)
{
}

MojDbLevelContainerIterator::~MojDbLevelContainerIterator()
{
}

MojDbLevelContainerIterator& MojDbLevelContainerIterator::operator++ ()
{
    if (m_it != --m_container.end()) {
        if (m_start)
            m_start = false;

        ++m_it;
    } else {
        m_end = true;
    }

    return *this;
}

/*
MojDbLevelContainerIterator MojDbLevelContainerIterator::operator++(int )
{
    MojDbLevelContainerIterator oldValue(*this);

    ++(*this);

    return oldValue;
}*/

MojDbLevelContainerIterator& MojDbLevelContainerIterator::operator-- ()
{
    if (m_it == m_container.begin()) {
        m_start = true;
    } else {
        if (m_end)
            m_end = false;

        --m_it;
    }

    return *this;
}

/*
MojDbLevelContainerIterator MojDbLevelContainerIterator::operator-- (int)
{
    MojDbLevelContainerIterator oldValue(*this);

    --(*this);

    return oldValue;
}*/

MojDbLevelContainerIterator& MojDbLevelContainerIterator::operator= (iterator_t iterator)
{
    m_it = iterator;
    m_start = false;

    if (m_it == m_container.end()) {
        m_end = true;
    } else {
        m_end = false;
    }

    return *this;
}

void MojDbLevelContainerIterator::toBegin()
{
    m_it = m_container.begin();
    m_start = true;
    m_end = false;
}

void MojDbLevelContainerIterator::toEnd()
{
    m_it = m_container.end();
    m_start = false;
    m_end = true;
}

void MojDbLevelContainerIterator::toFirst()
{
    if (m_container.empty()) {
        m_start = true;
        m_end = true;
        m_it = m_container.end();
    } else {
        m_start = false;
        m_end = false;
        m_it = m_container.begin();
    }
}
void MojDbLevelContainerIterator::toLast()
{
    if (m_container.empty()) {
        m_start = true;
        m_end = true;
        m_it = m_container.end();
    } else {
        m_start = false;
        m_end = false;
        m_it = --m_container.end();
    }
}
