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

#include <cassert>

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
    assert( !isEnd() );
    if (m_start)
    {
        toFirst();
    }
    else
    {
        ++m_it;
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
    assert( !isBegin() );
    if (m_it == m_container.begin())
    {
        toBegin();
    }
    else
    {
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

    return *this;
}

void MojDbLevelContainerIterator::toBegin()
{
    m_start = true;
}

void MojDbLevelContainerIterator::toEnd()
{
    m_it = m_container.end();
}

void MojDbLevelContainerIterator::toFirst()
{
    m_it = m_container.begin();
    m_start = false;
}

void MojDbLevelContainerIterator::toLast()
{
    m_start = m_container.empty();
    if (!m_start) m_it = --m_container.end();
}
