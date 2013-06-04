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

#include "db-luna/leveldb/MojDbLevelTxnIterator.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#include "db-luna/leveldb/MojDbLevelTxn.h"

#include <iostream>

std::string string_to_hex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

MojDbLevelTxnIterator::MojDbLevelTxnIterator(MojDbLevelTableTxn *txn) :
    inserts(txn->m_pendingValues),
    deletes(txn->m_pendingDeletes),
    leveldb(txn->m_db),
    m_it(leveldb),
    m_insertsItertor (inserts),
    m_txn(txn)
{
    MojAssert( txn );
    m_fwd = true;
    // set iterator to first element
    first();
}

MojDbLevelTxnIterator::~MojDbLevelTxnIterator()
{
    if (m_txn) m_txn->detach(this);
}

void MojDbLevelTxnIterator::detach()
{
    MojAssert( m_txn );
    m_txn = 0;
}

void MojDbLevelTxnIterator::skipDeleted()
{
    if (m_fwd) {
        while (!m_it.isEnd() && isDeleted(m_it->key().ToString())) {
            ++m_it;
        }
    } else {
        while (!m_it.isBegin() && isDeleted(m_it->key().ToString())) {
            --m_it;
        }
    }
}

bool MojDbLevelTxnIterator::inTransaction() const
{
    if (!m_it.isValid())
        return true;

    if (!m_insertsItertor.isValid())
        return false;

    // Note that in case when both iterators points to the records with same
    // key we prefer to take one from transaction
    if (m_fwd) {
        if (m_it->key().ToString() < m_insertsItertor->first)
            return false;
        else
            return true;
    } else {
        if (m_it->key().ToString() > m_insertsItertor->first)
            return false;
        else
            return true;
    }
}

std::string MojDbLevelTxnIterator::getValue()
{
    if (inTransaction())
        return m_insertsItertor->second;
    else
        return m_it->value().ToString();
}

const std::string MojDbLevelTxnIterator::getKey() const
{
    if (inTransaction())
        return m_insertsItertor->first;
    else
        return m_it->key().ToString();
}

bool MojDbLevelTxnIterator::keyStartsWith(const std::string& key) const
{
    std::string currentKey = getKey();

    if (key.size() > currentKey.size())
        return false;

    return std::mismatch(key.begin(), key.end(), currentKey.begin()).first == key.end();
}

void MojDbLevelTxnIterator::first()
{
    m_fwd = true;

    m_it.toFirst();
    m_insertsItertor.toFirst();

    skipDeleted();
}

void MojDbLevelTxnIterator::last()
{
    m_fwd = false;

    m_it.toLast();
    m_insertsItertor.toLast();

    skipDeleted();
}

bool MojDbLevelTxnIterator::isValid() const
{
    return  m_it.isValid() || m_insertsItertor.isValid();
}

bool MojDbLevelTxnIterator::isDeleted(const std::string& key) const
{
    return ( deletes.find(key) != deletes.end() );
}


void MojDbLevelTxnIterator::prev()
{
    if (m_fwd) {
        m_fwd = false;
        if (m_insertsItertor.isValid() && m_it.isValid()) {
            if (m_it->key().ToString() > m_insertsItertor->first) {
                --m_it;
                skipDeleted();
            } else {
                --m_insertsItertor;
            }
        } else {
            if (m_insertsItertor.isEnd())
                --m_insertsItertor;

            if (m_it.isEnd())
                --m_it;
        }
    }

    if (m_insertsItertor.isBegin()) {
        --m_it;
        skipDeleted();
        return;
    }

    if (m_it.isBegin()) {
        --m_insertsItertor;
        return;
    }

    if (m_it->key().ToString() > m_insertsItertor->first) {
        --m_it;
        skipDeleted();

    } else if (m_it->key().ToString() == m_insertsItertor->first) {
        // advance both iterators to the next key value
        --m_insertsItertor;
        --m_it;
        skipDeleted();
    } else {
        --m_insertsItertor;
    }
}

void MojDbLevelTxnIterator::next()
{
    if (!m_fwd) {
        m_fwd = true;
        if (m_insertsItertor.isValid() && m_it.isValid()) {
            if (m_it->key().ToString() < m_insertsItertor->first) {
                ++m_it;
                skipDeleted();
            } else {
                ++m_insertsItertor;
            }
        } else {
            if (m_insertsItertor.isBegin())
                ++m_insertsItertor;

            if (m_it.isBegin())
                ++m_it;
        }
    }

    if (m_insertsItertor.isEnd()) {
        ++m_it;
        skipDeleted();

        return;
    }

    if (m_it.isEnd()) {
        ++m_insertsItertor;
        return;
    }

    if (m_it->key().ToString() < m_insertsItertor->first) {
        ++m_it;
        skipDeleted();
    } else if (m_it->key().ToString() == m_insertsItertor->first) {
        // advance both iterators to the next key value
        ++m_insertsItertor;
        ++m_it;
        skipDeleted();
    } else {
        ++m_insertsItertor;
    }
}

void MojDbLevelTxnIterator::seek(const std::string& key)
{
    m_fwd = true;

    m_it.seek(key);
    skipDeleted();

    m_insertsItertor = inserts.lower_bound(key);
}

leveldb::Status MojDbLevelTxnIterator::status() const
{
    return m_it->status();
}
