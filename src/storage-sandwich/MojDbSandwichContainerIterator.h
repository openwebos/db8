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

#ifndef MOJDBLEVELCOLLECTIONITERATOR_H
#define MOJDBLEVELCOLLECTIONITERATOR_H

#include "core/MojNoCopy.h"

#include <map>
#include <string>

class MojDbSandwichContainerIterator : public MojNoCopy
{
    typedef std::map<std::string, std::string> container_t;
    typedef container_t::iterator iterator_t;

public:
    MojDbSandwichContainerIterator(container_t& container);
    MojDbSandwichContainerIterator(iterator_t iterator, container_t& container);
    ~MojDbSandwichContainerIterator();

    inline bool isBegin() const { return m_start; }
    inline bool isEnd() const { return !isBegin() && m_it == m_container.end(); }
    inline bool isValid() const { return !(isBegin() || isEnd()); }

    void toBegin();
    void toEnd();
    void toFirst();
    void toLast();

    MojDbSandwichContainerIterator& operator++ ();
    //MojDbSandwichContainerIterator operator++ (int);
    MojDbSandwichContainerIterator& operator-- ();
    //MojDbSandwichContainerIterator operator-- (int);

    MojDbSandwichContainerIterator& operator= (iterator_t iterator);

    operator iterator_t () { return m_it; }
    iterator_t::value_type* operator->() { return &(*m_it); }
    const iterator_t::value_type* operator->() const { return &(*m_it); }

private:
    container_t& m_container;
    iterator_t m_it;

    bool m_start;
};

#endif
