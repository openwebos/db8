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

#include "MojDbSandwichTxnIterator.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichTxn.h"

namespace {
    inline bool operator<(const leveldb::Slice &a, const leveldb::Slice &b)
    { return a.compare(b) < 0; }
    inline bool operator>(const leveldb::Slice &a, const leveldb::Slice &b)
    { return a.compare(b) > 0; }

    enum Order { LT = -1, EQ = 0, GT = 1 };

    Order compare(const MojDbSandwichIterator &x, const MojDbSandwichContainerIterator &y)
    {
        const bool xb = x.isBegin(),
                   xe = x.isEnd(),
                   yb = y.isBegin(),
                   ye = y.isEnd();
        if ((xb && yb) || (xe && ye)) return EQ;
        if (xb || ye) return LT;
        if (xe || yb) return GT;

        int r = x->key().ToString().compare(y->first);
        return r < 0 ? LT :
               r > 0 ? GT :
                       EQ ;
    }

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
}

MojDbSandwichTxnIterator::MojDbSandwichTxnIterator(MojDbSandwichTableTxn *txn) :
    inserts(txn->m_pendingValues),
    deletes(txn->m_pendingDeletes),
    leveldb(txn->m_db),
    m_it(leveldb),
    m_insertsItertor (inserts),
    m_txn(txn)
{
    MojAssert( txn );
    // set iterator to first element
    first();
}

MojDbSandwichTxnIterator::~MojDbSandwichTxnIterator()
{
    if (m_txn) m_txn->detach(this);
}

void MojDbSandwichTxnIterator::detach()
{
    MojAssert( m_txn );
    m_txn = 0;
}

void MojDbSandwichTxnIterator::notifyPut(const leveldb::Slice &key)
{
    // Idea here is to re-align our transaction iterator if new key is in range
    // of keys where database iterator and transaction iterator points to.
    // Need to take care of next cases:
    // 1) going forward: m_it->key() <= key < m_insertsItertor->first
    // 2) going backward: m_insertIterator->first < key <= m_it->key()

    // lets see in which relations we are with leveldb iterator
    bool keyLtDb; // key < m_it->key()
    bool keyEqDb; // key == m_it->key()
    if (m_it.isBegin()) keyLtDb = false, keyEqDb = false;
    else if (m_it.isEnd()) keyLtDb = true, keyEqDb = false;
    else
    {
        keyLtDb = key < m_it->key();
        keyEqDb = key == m_it->key();
    }

    // lets see in which relations we are with transaction iterator
    bool keyLtTxn;
    bool keyEqTxn;
    if (m_insertsItertor.isBegin()) keyLtTxn = false, keyEqTxn = false;
    else if (m_insertsItertor.isEnd()) keyLtTxn = true, keyEqTxn = false;
    else
    {
        keyLtTxn = key < m_insertsItertor->first;
        keyEqTxn = key == m_insertsItertor->first;
    }

    if (m_fwd)
    {
        // exclude case: key < db || txn <= key
        if (keyLtDb || !keyLtTxn) return;

        --m_insertsItertor; // set to a new key
    }
    else
    {
        // exclude case: key <= txn || db < key
        if (keyLtTxn || keyEqTxn || (!keyLtDb && !keyEqDb)) return;

        ++m_insertsItertor; // set to a new key
    }
}

void MojDbSandwichTxnIterator::notifyDelete(const leveldb::Slice &key)
{
    // if no harm for txn iterator - nothing to do
    if (!m_insertsItertor.isValid()) return;
    if (m_insertsItertor->first != key) return;

    if (m_fwd)
    {
        MojAssert( !isBegin() );
        switch (compare(m_it, m_insertsItertor))
        {
        case EQ:
            ++m_it; skipDeleted();
            ++m_insertsItertor;
            break;

        default: MojAssert( !"happens" );
        case GT:
        case LT:
            ++m_insertsItertor;
            break;
        }
    }
    else
    {
        MojAssert( !isEnd() );
        switch (compare(m_it, m_insertsItertor))
        {
        case EQ:
            --m_it; skipDeleted();
            --m_insertsItertor;
            break;

        default: MojAssert( !"happens" );
        case GT:
        case LT:
            --m_insertsItertor;
            break;
        }
    }
    m_invalid = true;
}

void MojDbSandwichTxnIterator::skipDeleted()
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

bool MojDbSandwichTxnIterator::inTransaction() const
{
    // Note that in case when both iterators points to the records with same
    // key we prefer to take one from transaction
    switch( compare(m_it, m_insertsItertor) )
    {
    case LT: return !m_fwd;
    case GT: return m_fwd;
    case EQ: return true;
    default: MojAssert( !"happens" );
    }
    return true;
}

std::string MojDbSandwichTxnIterator::getValue()
{
    if (inTransaction())
        return m_insertsItertor->second;
    else
        return m_it->value().ToString();
}

const std::string MojDbSandwichTxnIterator::getKey() const
{
    if (inTransaction())
        return m_insertsItertor->first;
    else
        return m_it->key().ToString();
}

bool MojDbSandwichTxnIterator::keyStartsWith(const std::string& key) const
{
    std::string currentKey = getKey();

    if (key.size() > currentKey.size())
        return false;

    return std::mismatch(key.begin(), key.end(), currentKey.begin()).first == key.end();
}

void MojDbSandwichTxnIterator::first()
{
    m_fwd = true;
    m_invalid = false;

    m_it.toFirst();
    m_insertsItertor.toFirst();

    skipDeleted();
}

void MojDbSandwichTxnIterator::last()
{
    m_fwd = false;
    m_invalid = false;

    m_it.toLast();
    m_insertsItertor.toLast();

    skipDeleted();
}

bool MojDbSandwichTxnIterator::isValid() const
{
    return  !m_invalid && (m_it.isValid() || m_insertsItertor.isValid());
}

bool MojDbSandwichTxnIterator::isDeleted(const std::string& key) const
{
    return ( deletes.find(key) != deletes.end() );
}


void MojDbSandwichTxnIterator::prev()
{
    MojAssert( !isBegin() );
    if (isEnd()) return last();

    if (m_fwd) {
        const std::string &currentKey = getKey();
        m_fwd = false;
        m_invalid = false;

        // after switching direction we may end up with one of the iterators
        // pointing to one of the tails
        if (m_it.isEnd()) { --m_it; skipDeleted(); }
        else if (m_insertsItertor.isEnd()) --m_insertsItertor;
        else
        {
            if (currentKey < getKey()) prev();
        }
        MojAssert( getKey() == currentKey );
    }
    else if (m_invalid)
    {
        // we already jumped here from deleted record
        m_invalid = false;
        return;
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

    MojAssert( !m_it.isEnd() );
    MojAssert( !m_insertsItertor.isEnd() );

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

void MojDbSandwichTxnIterator::next()
{
    MojAssert( !isEnd() );
    if (isBegin()) return first();

    if (!m_fwd) {
        const std::string &currentKey = getKey();
        m_fwd = true;
        m_invalid = false;

        // after switching direction we may end up with one of the iterators
        // pointing to one of the tails
        if (m_it.isBegin()) { ++m_it; skipDeleted(); }
        else if (m_insertsItertor.isBegin()) ++m_insertsItertor;
        else
        {
            if (getKey() < currentKey) next();
        }
        MojAssert( getKey() == currentKey );
    }
    else if (m_invalid)
    {
        // we already jumped here from deleted record
        m_invalid = false;
        return;
    }

    m_invalid = false;

    if (m_insertsItertor.isEnd()) {
        ++m_it;
        skipDeleted();
        return;
    }

    if (m_it.isEnd()) {
        ++m_insertsItertor;
        return;
    }

    MojAssert( !m_it.isBegin() );
    MojAssert( !m_insertsItertor.isBegin() );

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

void MojDbSandwichTxnIterator::seek(const std::string& key)
{
    m_fwd = true;
    m_invalid = false;

    m_it.seek(key);
    skipDeleted();

    m_insertsItertor = inserts.lower_bound(key);
}

leveldb::Status MojDbSandwichTxnIterator::status() const
{
    return m_it->status();
}
