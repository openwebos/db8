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

#ifndef MOJDBLEVELTXNITERATOR_H
#define MOJDBLEVELTXNITERATOR_H

#include "core/MojNoCopy.h"

#include "db-luna/leveldb/MojDbLevelContainerIterator.h"
#include "db-luna/leveldb/MojDbLevelIterator.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <map>
#include <string>
#include <set>

class MojDbLevelTableTxn;

/**
 * Class provides cursor based, transaction logic to work with leveldb databases
 * @class MojDbLevelTxnIterator
 */
class MojDbLevelTxnIterator : public MojNoCopy
{
    typedef std::map<std::string, std::string> inserts_t;
    typedef std::set<std::string> deletes_t;
public:
    MojDbLevelTxnIterator(MojDbLevelTableTxn *txn);
    ~MojDbLevelTxnIterator();

    std::string getValue();
    const std::string getKey() const;
    bool keyStartsWith(const std::string& key) const;

    void first();
    void last();
    bool isValid() const;
    bool isDeleted(const std::string& key) const;
    void prev();
    void next();
    void seek(const std::string& key);
    void save() { m_it.save(); }
    void restore() { m_it.restore(); }

    leveldb::Status status() const;
private:
    void skipDeleted();

    inserts_t &inserts;
    deletes_t &deletes;

    leveldb::DB* leveldb;

    bool m_fwd;

    MojDbLevelIterator m_it;
    MojDbLevelContainerIterator m_insertsItertor;

    MojDbLevelTableTxn *m_txn;
};

#endif
