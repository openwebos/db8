/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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

#ifndef MOJDBLEVELENGINE_H_
#define MOJDBLEVELENGINE_H_

#include <leveldb/db.h>
#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"
#include "core/MojLogDb8.h"

class MojDbLevelDatabase;
class MojDbLevelEnv;
class MojDbLevelSeq;

class MojDbLevelEngine : public MojDbStorageEngine
{
public:
    MojDbLevelEngine();
    ~MojDbLevelEngine();

    virtual MojErr configure(const MojObject& conf);
    virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn);
    virtual MojErr open(const MojChar* path);
    virtual MojErr open(const MojChar* path, MojDbEnv* env);
    virtual MojErr close();
    virtual MojErr compact();
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut);
    virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut) ;
    virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageSeq>& seqOut) ;

    const MojString& path() const { return m_path; }
    MojDbLevelEnv* env() { return m_env.get(); }
    MojErr addDatabase(MojDbLevelDatabase* db);
    MojErr removeDatabase(MojDbLevelDatabase* db);
    MojErr addSeq(MojDbLevelSeq* seq);
    MojErr removeSeq(MojDbLevelSeq* seq);

    MojDbLevelDatabase* indexDb() { return m_indexDb.get(); }

private:
    typedef MojVector<MojRefCountedPtr<MojDbLevelDatabase> > DatabaseVec;
    typedef MojVector<MojRefCountedPtr<MojDbLevelSeq> > SequenceVec;

    MojRefCountedPtr<MojDbLevelEnv> m_env;
    MojThreadMutex m_dbMutex;
    MojRefCountedPtr<MojDbLevelDatabase> m_indexDb;
    MojRefCountedPtr<MojDbLevelDatabase> m_seqDb;
    MojString m_path;
    DatabaseVec m_dbs;
    SequenceVec m_seqs;
    bool m_isOpen;
};

#endif /* MOJDBLEVELENGINE_H_ */

