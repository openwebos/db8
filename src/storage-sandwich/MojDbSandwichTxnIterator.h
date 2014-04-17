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

#ifndef MOJDBLEVELTXNITERATOR_H
#define MOJDBLEVELTXNITERATOR_H

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include "core/MojNoCopy.h"

#include "MojDbSandwichContainerIterator.h"
#include "MojDbSandwichIterator.h"
#include "MojDbSandwichEngine.h"

#include <map>
#include <string>
#include <set>

class MojDbSandwichTableTxn;

/**
 * Class provides cursor based, transaction logic to work with leveldb databases
 * @class MojDbSandwichTxnIterator
 */
class MojDbSandwichTxnIterator : public MojNoCopy
{
    typedef std::map<std::string, std::string> inserts_t;
    typedef std::set<std::string> deletes_t;
public:
    MojDbSandwichTxnIterator(MojDbSandwichTableTxn *txn);
    ~MojDbSandwichTxnIterator();

    std::string getValue();
    const std::string getKey() const;
    bool keyStartsWith(const std::string& key) const;

    /**
     *  Notify about record inserted/updated
     */
    void notifyPut(const leveldb::Slice &key);

    /**
     *  Notify that must be send before record deletion
     */
    void notifyDelete(const leveldb::Slice &key);

    void first();
    void last();
    bool isValid() const;
    bool isBegin() const { return m_it.isBegin() && m_insertsItertor.isBegin(); }
    bool isEnd() const { return m_it.isEnd() && m_insertsItertor.isEnd(); }
    bool isDeleted(const std::string& key) const;
    void prev();
    void next();
    void seek(const std::string& key);
    void save() { m_it.save(); }
    void restore() { m_it.restore(); }
    void detach();

    bool inTransaction() const;
    leveldb::Status status() const;
private:
    void skipDeleted();

    inserts_t &inserts;
    deletes_t &deletes;

    MojDbSandwichEngine::BackendDb::Part* leveldb;

    bool m_fwd, m_invalid;

    MojDbSandwichIterator m_it;
    MojDbSandwichContainerIterator m_insertsItertor;

    MojDbSandwichTableTxn *m_txn;
};

#endif
