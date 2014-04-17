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

#include "MojDbSandwichContainerIterator.h"

MojDbSandwichContainerIterator::MojDbSandwichContainerIterator (std::map<std::string, std::string>& database)
    : m_container(database)
{
    toFirst();
}

MojDbSandwichContainerIterator::MojDbSandwichContainerIterator(iterator_t iterator, std::map<std::string, std::string>& database)
    : m_container(database), m_it(iterator)
{
}

MojDbSandwichContainerIterator::~MojDbSandwichContainerIterator()
{
}

MojDbSandwichContainerIterator& MojDbSandwichContainerIterator::operator++ ()
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
MojDbSandwichContainerIterator MojDbSandwichContainerIterator::operator++(int )
{
    MojDbSandwichContainerIterator oldValue(*this);

    ++(*this);

    return oldValue;
}*/

MojDbSandwichContainerIterator& MojDbSandwichContainerIterator::operator-- ()
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
MojDbSandwichContainerIterator MojDbSandwichContainerIterator::operator-- (int)
{
    MojDbSandwichContainerIterator oldValue(*this);

    --(*this);

    return oldValue;
}*/

MojDbSandwichContainerIterator& MojDbSandwichContainerIterator::operator= (iterator_t iterator)
{
    m_it = iterator;
    m_start = false;

    return *this;
}

void MojDbSandwichContainerIterator::toBegin()
{
    m_start = true;
}

void MojDbSandwichContainerIterator::toEnd()
{
    m_it = m_container.end();
}

void MojDbSandwichContainerIterator::toFirst()
{
    m_it = m_container.begin();
    m_start = false;
}

void MojDbSandwichContainerIterator::toLast()
{
    m_start = m_container.empty();
    if (!m_start) m_it = --m_container.end();
}
