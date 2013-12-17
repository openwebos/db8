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


#ifndef MOJDBSTORAGEENGINE_H_
#define MOJDBSTORAGEENGINE_H_

#include "db/MojDbDefs.h"
#include "db/MojDbWatcher.h"
#include "db/MojDbQuotaEngine.h"
#include "core/MojAutoPtr.h"
#include "core/MojObject.h"
#include "core/MojVector.h"
#include "core/MojSignal.h"

class MojDbEnv : public MojRefCounted
{
  public:
    virtual ~MojDbEnv() {}
    virtual MojErr configure(const MojObject& conf) = 0;
    virtual MojErr open(const MojChar* path) = 0;
};

class MojDbStorageItem : public MojRefCounted
{
public:
	virtual ~MojDbStorageItem() {}
	virtual MojErr close() = 0;
	virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine) = 0;
	virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const = 0;
	virtual const MojObject& id() const = 0;
	virtual MojSize size() const = 0;

	MojErr toObject(MojObject& objOut, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
	MojErr toJson(MojString& strOut, MojDbKindEngine& kindEngine) const;

protected:
	MojObject m_id;
};

class MojDbStorageQuery : public MojRefCounted
{
public:
	typedef MojVector<MojByte> ByteVec;
	typedef MojSet<MojString> StringSet;

	MojDbStorageQuery() {}
	virtual ~MojDbStorageQuery() {}
	virtual MojErr close() = 0;
	virtual MojErr get(MojDbStorageItem*& itemOut, bool& foundOut) = 0;
	virtual MojErr getId(MojObject& idOut, MojUInt32& groupOut, bool& foundOut) = 0;
	virtual MojErr getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut) = 0;
	virtual MojErr count(MojUInt32& countOut) = 0;
	virtual MojErr nextPage(MojDbQuery::Page& pageOut) = 0;
	virtual void excludeKinds(const StringSet& toExclude) { m_excludeKinds = toExclude; }
	virtual MojUInt32 groupCount() const = 0;
	const MojDbKey& endKey() const { return m_endKey; }
	StringSet& excludeKinds() { return m_excludeKinds; }
	bool verify() { return m_verify; }
	void verify(bool bVal) { m_verify = bVal; }

protected:
	MojDbKey m_endKey;
	StringSet m_excludeKinds;
	bool m_verify;

};

class MojDbStorageSeq : public MojRefCounted
{
public:
	virtual ~MojDbStorageSeq() {}
	virtual MojErr close() = 0;
	virtual MojErr get(MojInt64& valOut) = 0;
};

class MojDbStorageTxn : public MojSignalHandler
{
public:
	typedef MojVector<MojByte> ByteVec;
	typedef MojSignal<MojDbStorageTxn*> CommitSignal;

	virtual ~MojDbStorageTxn() {}
	virtual MojErr abort() = 0;
    virtual bool isValid() = 0;
	MojErr commit();

	MojErr addWatcher(MojDbWatcher* watcher, const MojDbKey& key);
	MojErr offsetQuota(MojInt64 amount);
	void quotaEnabled(bool val) { m_quotaEnabled = val; }
	void refreshQuotas() { m_refreshQuotas = true; }
	void reverseTransaction(bool isReverse = true) { m_reverseTransaction = isReverse; }
	bool isReversed() const { return m_reverseTransaction; }

	void notifyPreCommit(CommitSignal::SlotRef slot);
	void notifyPostCommit(CommitSignal::SlotRef slot);

protected:
	MojDbStorageTxn();
	virtual MojErr commitImpl() = 0;

private:
	friend class MojDbQuotaEngine;

	struct WatcherInfo
	{
		WatcherInfo(MojDbWatcher* watcher, const MojDbKey& key)
		: m_watcher(watcher), m_key(key) {}

		MojRefCountedPtr<MojDbWatcher> m_watcher;
		MojDbKey m_key;
	};
	typedef MojVector<WatcherInfo> WatcherVec;

	bool m_quotaEnabled;
	bool m_refreshQuotas;
	MojDbQuotaEngine* m_quotaEngine;
	MojDbQuotaEngine::OffsetMap m_offsetMap;
	MojRefCountedPtr<MojDbQuotaEngine::Offset> m_curQuotaOffset;
	WatcherVec m_watchers;
	CommitSignal m_preCommit;
	CommitSignal m_postCommit;
    bool m_reverseTransaction;
};

class MojDbStorageCollection : public MojRefCounted
{
public:
	virtual ~MojDbStorageCollection() {}
	virtual MojErr close() = 0;
	virtual MojErr drop(MojDbStorageTxn* txn) = 0;
	virtual MojErr stats(MojDbStorageTxn* txn, MojSize& countOut, MojSize& sizeOut) = 0;
	virtual MojErr find(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageQuery>& queryOut) = 0;
	virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut) = 0;
};

class MojDbStorageIndex : public MojDbStorageCollection
{
public:
	typedef MojVector<MojByte> ByteVec;

	virtual ~MojDbStorageIndex() {}
	virtual MojErr insert(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojDbKey& key, MojDbStorageTxn* txn) = 0;
};

class MojDbStorageDatabase : public MojDbStorageCollection
{
public:
	virtual ~MojDbStorageDatabase() {}
	virtual MojErr insert(const MojObject& id, MojBuffer& val, MojDbStorageTxn* txn) = 0;
	virtual MojErr update(const MojObject& id, MojBuffer& val, MojDbStorageItem* oldVal, MojDbStorageTxn* txn) = 0;
	virtual MojErr del(const MojObject& id, MojDbStorageTxn* txn, bool& foundOut) = 0;
	virtual MojErr get(const MojObject& id, MojDbStorageTxn* txn, bool forUpdate, MojRefCountedPtr<MojDbStorageItem>& itemOut) = 0;
	virtual MojErr openIndex(const MojObject& id, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageIndex>& indexOut) = 0;
//hack:
	virtual MojErr mutexStats(int* total_mutexes, int* mutexes_free, int* mutexes_used, int* mutexes_used_highwater, int* mutexes_regionsize) 
		{ if (total_mutexes) *total_mutexes = 0;
		  if (mutexes_free) *mutexes_free = 0;
		  if (mutexes_used) *mutexes_used = 0;
		  if (mutexes_used_highwater) *mutexes_used_highwater = 0;
		  if (mutexes_regionsize) *mutexes_regionsize = 0;
		  return MojErrNone;
		 }
};

class MojDbStorageEngineFactory : public MojRefCounted
{
public:
    virtual ~MojDbStorageEngineFactory() {}
    virtual MojErr create(MojRefCountedPtr<MojDbStorageEngine>& engineOut) const = 0;
    virtual MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) const = 0;
    virtual const MojChar* name() const = 0;
};

class MojDbStorageEngine : public MojRefCounted
{
public:
    static MojErr createDefaultEngine(MojRefCountedPtr<MojDbStorageEngine>& engineOut);
    static MojErr createEngine(const MojChar* name, MojRefCountedPtr<MojDbStorageEngine>& engineOut);
    static MojErr createEnv(MojRefCountedPtr<MojDbEnv>& envOut) { return m_factory->createEnv(envOut); };
    static MojErr setEngineFactory(MojDbStorageEngineFactory* factory);
    static const MojDbStorageEngineFactory* engineFactory() {return m_factory.get();};

    virtual ~MojDbStorageEngine() {}
    virtual MojErr configure(const MojObject& config) = 0;
    virtual MojErr drop(const MojChar* path, MojDbStorageTxn* txn) = 0;
    virtual MojErr open(const MojChar* path) = 0;
    virtual MojErr open(const MojChar* path, MojDbEnv * env) = 0;
    virtual MojErr close() = 0;
    virtual MojErr compact() = 0;
    virtual MojErr beginTxn(MojRefCountedPtr<MojDbStorageTxn>& txnOut) = 0;
    virtual MojErr openDatabase(const MojChar* name, MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageDatabase>& dbOut) = 0;
    virtual MojErr openSequence(const MojChar* name, MojDbStorageTxn* txn,  MojRefCountedPtr<MojDbStorageSeq>& seqOut) = 0;

protected:
	MojDbStorageEngine();
	static MojRefCountedPtr<MojDbStorageEngineFactory> m_factory;

};


#endif /* MOJDBSTORAGEENGINE_H_ */
