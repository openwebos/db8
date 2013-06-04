/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBINDEX_H_
#define MOJDBINDEX_H_

#include "db/MojDbDefs.h"
#include "db/MojDbExtractor.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDbWatcher.h"
#include "core/MojSet.h"
#include "core/MojThread.h"

class MojDbIndex : public MojSignalHandler
{
public:
	static const MojChar* const CountKey;
	static const MojChar* const DelMissesKey;
	static const MojChar* const DefaultKey;
	static const MojChar* const IncludeDeletedKey;
	static const MojChar* const MultiKey;
	static const MojChar* const NameKey;
	static const MojChar* const PropsKey;
	static const MojChar* const SizeKey;
	static const MojChar* const TypeKey;
	static const MojChar* const WatchesKey;
	static const MojSize MaxIndexNameLen = 128;

	typedef MojVector<MojString> StringVec;

	MojDbIndex(MojDbKind* kind, MojDbKindEngine* kindEngine);
	~MojDbIndex();

	MojErr fromObject(const MojObject& obj, const MojString& locale);
	MojErr addProp(const MojObject& propObj, bool pushFront = false);
	void incDel(bool val) { MojAssert(!isOpen()); m_includeDeleted = val; }

	MojErr open(MojDbStorageIndex* index, const MojObject& id, MojDbReq& req, bool created = false);
	MojErr close();
	MojErr stats(MojObject& objOut, MojSize& usageOut, MojDbReq& req);
	MojErr drop(MojDbReq& req);
	MojErr updateLocale(const MojChar* locale, MojDbReq& req);

	MojErr find(MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req);
	MojErr update(const MojObject* newObj, const MojObject* oldObj, MojDbStorageTxn* txn, bool forcedel);
	MojErr cancelWatch(MojDbWatcher* watcher);

	bool canAnswer(const MojDbQuery& query) const;
	bool includeDeleted() const { return m_includeDeleted; }
	MojSize idIndex() const { return m_idIndex; }
	MojSize size() const { return m_props.size(); }
	const MojObject& id() const { return m_id; }
	const MojObject& object() const { return m_obj; }
	const StringVec& props() const { return m_propNames; }
	const StringVec& sortKey() const { return m_sortKey; }
	const MojString& locale() const { return m_locale; }
	const MojString& name() const { return m_name; }

private:
	static const MojSize WatchWarningThreshold = 20;

	typedef MojVector<MojDbKeyRange> RangeVec;
	typedef MojVector<MojRefCountedPtr<MojDbExtractor> > PropVec;
	typedef MojVector<MojByte> ByteVec;
	typedef MojSet<MojDbKey> KeySet;
	typedef MojSet<MojObject> ObjectSet;
	typedef MojVector<MojObject> ObjectVec;
	typedef MojVector<MojRefCountedPtr<MojDbWatcher> > WatcherVec;
	typedef MojMap<MojString, MojSize> WatcherMap;
	typedef MojDbStorageTxn::CommitSignal::Slot<MojDbIndex> CommitSlot;

	bool isOpen() const { return m_collection != NULL; }
	bool isIdIndex() const;
	bool includeObj(const MojObject* obj) const;
	MojErr createExtractor(const MojObject& propObj, MojRefCountedPtr<MojDbExtractor>& extractorOut);
	MojErr addBuiltinProps();
	MojErr addWatch(const MojDbQueryPlan& plan, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req);
	MojErr notifyWatches(const KeySet& keys, MojDbStorageTxn* txn);
	MojErr notifyWatches(const MojDbKey& key, MojDbStorageTxn* txn);
	MojErr delKeys(const KeySet& keys, MojDbStorageTxn* txn, bool forcedel);
	MojErr insertKeys(const KeySet& keys, MojDbStorageTxn* txn);
	MojErr getKeys(const MojObject& obj, KeySet& keysOut) const;
	MojErr handlePreCommit(MojDbStorageTxn* txn);
	MojErr handlePostCommit(MojDbStorageTxn* txn);
	MojErr build(MojDbStorageTxn* txn);
	static MojErr validateName(const MojString& name);

	MojString m_name;
	MojString m_locale;
	StringVec m_propNames;
	StringVec m_sortKey;
	PropVec m_props;
	MojObject m_obj;
	MojObject m_id;
	KeySet m_idSet;
	WatcherVec m_watcherVec;
	WatcherMap m_watcherMap;
	MojThreadRwLock m_lock;
	CommitSlot m_preCommitSlot;
	CommitSlot m_postCommitSlot;
	MojRefCountedPtr<MojDbStorageIndex> m_index;
	MojDbKind* m_kind;
	MojDbKindEngine* m_kindEngine;
	MojDbStorageCollection* m_collection;
	MojSize m_idIndex;
	bool m_includeDeleted;
	bool m_ready;
	MojUInt32 m_delMisses;

	static MojLogger s_log;
};

#endif /* MOJDBINDEX_H_ */
