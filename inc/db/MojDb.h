/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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


#ifndef MOJDB_H_
#define MOJDB_H_

#include "db/MojDbDefs.h"
#include "db/MojDbCursor.h"
#include "db/MojDbIdGenerator.h"
#include "db/MojDbKindEngine.h"
#include "db/MojDbPermissionEngine.h"
#include "db/MojDbQuotaEngine.h"
#include "db/MojDbSpaceAlert.h"
#include "db/MojDbQuotaCheckAlert.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDbShardIdCache.h"
#include "db/MojDbShardEngine.h"
#include "db/MojDbWatcher.h"
#include "db/MojDbReq.h"
#include "db/MojDbSearchCache.h"
#include "core/MojHashMap.h"
#include "core/MojSignal.h"
#include "core/MojString.h"
#include "core/MojFile.h"
#include "core/MojLogDb8.h"
#include "core/MojThread.h"

class MojDb : private MojNoCopy
{
public:
	enum MojFlags {
		FlagNone			= 0,
		FlagForce			= (1 << 0),
		FlagMerge			= (1 << 1),
		FlagPurge			= (1 << 2),
		FlagIgnoreMissing		= (1 << 3)
	};

	typedef MojSignal<> WatchSignal;

	static const MojChar* const ConfKey;
	static const MojChar* const DelKey;
	static const MojChar* const IdKey;
	static const MojChar* const IgnoreIdKey;
	static const MojChar* const KindKey;
	static const MojChar* const RevKey;
	static const MojChar* const SyncKey;
    static const MojChar* const KindIdPrefix;
    static const MojChar* const QuotaIdPrefix;
    static const MojChar* const PermissionIdPrefix;
	static const MojUInt32 AutoBatchSize;
	static const MojUInt32 AutoCompactSize;
    static const MojUInt32 TmpVersionFileLength;

	MojDb();
	virtual ~MojDb();

	MojErr configure(const MojObject& conf);
	MojErr drop(const MojChar* path);
	MojErr open(const MojChar* path, MojDbStorageEngine* engine = NULL);
	MojErr close();
	MojErr stats(MojObject& objOut, MojDbReqRef req = MojDbReq(), bool verify = false, MojString *pKind = NULL);
	MojErr quotaStats(MojObject& objOut, MojDbReqRef req = MojDbReq());
	MojErr updateLocale(const MojChar* locale, MojDbReqRef req = MojDbReq());
	MojErr getLocale(MojString& valOut, MojDbReq& req);
	MojErr compact();
	MojErr purge(MojUInt32& countOut, MojInt64 numDays = -1, MojDbReqRef req = MojDbReq());
	MojErr purgeStatus(MojObject& revOut, MojDbReqRef req = MojDbReq());
	MojErr dump(const MojChar* path, MojUInt32& countOut, bool incDel = true, MojDbReqRef req = MojDbReq(), bool backup = false,
			MojUInt32 maxBytes = 0, const MojObject* incrementalKey = NULL, MojObject* backupResponse = NULL);
	MojErr load(const MojChar* path, MojUInt32& countOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());

	MojErr del(const MojObject& id, bool& foundOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
	MojErr del(const MojObject* idsBegin, const MojObject* idsEnd, MojUInt32& countOut, MojObject& arrOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
	MojErr del(const MojDbQuery& query, MojUInt32& countOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
	MojErr delKind(const MojObject& id, bool& foundOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
	MojErr get(const MojObject& id, MojObject& objOut, bool& foundOut, MojDbReqRef req = MojDbReq());
	MojErr get(const MojObject* idsBegin, const MojObject* idsEnd, MojObjectVisitor& visitor, MojDbReqRef req = MojDbReq());
	MojErr find(const MojDbQuery& query, MojDbCursor& cursor, MojDbReqRef req = MojDbReq());
	MojErr find(const MojDbQuery& query, MojDbCursor& cursor, WatchSignal::SlotRef watchHandler, MojDbReqRef req = MojDbReq());
	MojErr merge(MojObject& obj, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq()) { return put(obj, flags | FlagMerge, req); }
	MojErr merge(MojObject* begin, const MojObject* end, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq()) { return put(begin, end, flags | FlagMerge, req); }
	MojErr merge(const MojDbQuery& query, const MojObject& props, MojUInt32& countOut, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
    MojErr put(MojObject& obj, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq(), MojString shardId = MojString());
    MojErr put(MojObject* begin, const MojObject* end, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq(), MojString shardId = MojString());
	MojErr putKind(MojObject& obj, MojUInt32 flags = FlagNone, MojDbReqRef req = MojDbReq());
	MojErr putPermissions(MojObject* begin, const MojObject* end, MojDbReqRef req = MojDbReq()) { return putConfig(begin, end, req, m_permissionEngine); }
	MojErr putQuotas(MojObject* begin, const MojObject* end, MojDbReqRef req = MojDbReq()) { return putConfig(begin, end, req, m_quotaEngine); }
	MojErr reserveId(MojObject& idOut);
	MojErr watch(const MojDbQuery& query, MojDbCursor& cursor, WatchSignal::SlotRef watchHandler, bool& firedOut, MojDbReqRef req = MojDbReq());

	const MojThreadRwLock& schemaLock() { return m_schemaLock; }
	MojDbKindEngine* kindEngine() { return &m_kindEngine; }
	MojDbPermissionEngine* permissionEngine() { return &m_permissionEngine; }
	MojDbQuotaEngine* quotaEngine() { return &m_quotaEngine; }
	MojDbStorageEngine* storageEngine() { return m_storageEngine.get(); }
	MojDbStorageDatabase* storageDatabase() { return m_objDb.get(); }
    MojDbShardEngine* shardEngine () { return &m_shardEngine; }
	MojInt64 version() { return DatabaseVersion; }
	MojErr commitBatch(MojDbReq& req);
    MojInt64 purgeWindow() {return m_purgeWindow;}

    // Search Cache
    MojDbSearchCache* searchCache() { return &m_searchCache; }

    //verify _kind
    MojErr isValidKind (MojString& i_kindStr, bool & ret);
    //successful, if records for the _kind have been written to this shard
    MojErr isSupported (MojString& i_shardId, MojString& i_kindStr, bool & ret);

    MojDbSpaceAlert& getSpaceAlert() { return m_spaceAlert; }
    MojDbQuotaCheckAlert& getQuotaAlert() { return m_quotaAlert; }

private:
	friend class MojDbKindEngine;
	friend class MojDbReq;

	static const MojChar* const AdminRole;
	static const MojChar* const DbStateObjId;
	static const MojChar* const IdSeqName;
	static const MojChar* const LastPurgedRevKey;
	static const MojChar* const LocaleKey;
	static const MojChar* const ObjDbName;
	static const MojChar* const RevNumKey;
	static const MojChar* const RoleType;
	static const MojChar* const TimestampKey;
	static const MojChar* const VersionFileName;

	static const MojInt64 DatabaseVersion = 8;
	static const int PurgeNumDaysDefault = 14;
        // The magic number 173 is just an arbitrary number in the high hundreds, which is prime. Primality is
        // not required, just handy to avoid any likliehood of synchronizing with loaded data sets.
	static const MojInt64 LoadStepSizeDefault = 173;

	void readLock() { m_schemaLock.readLock(); }
	void writeLock() { m_schemaLock.writeLock(); }
	void unlock() { m_schemaLock.unlock(); }

	MojErr createEngine();
	MojErr requireOpen();
	MojErr requireNotOpen();
	MojErr mergeInto(MojObject& dest, const MojObject& obj, const MojObject& prev);
	MojErr mergeArray(MojObject& dest, const MojObject& obj, MojObject& prev);
    MojErr putObj(const MojObject& id, MojObject& obj, const MojObject* oldObj,
                  MojDbStorageItem* oldItem, MojDbReq& req, MojDbOp op,
                  bool checkSchema = true, MojString shardId = MojString(), bool reverseTransaction = true);
	MojErr delObj(const MojObject& id, const MojObject& obj, MojDbStorageItem* item, MojObject& foundObjOut, MojDbReq& req, MojUInt32 flags);
	MojErr delImpl(const MojObject& id, bool& foundOut, MojObject& foundObjOut, MojDbReq& req, MojUInt32 flags);
	MojErr delImpl(const MojDbQuery& query, MojUInt32& countOut, MojDbReq& req, MojUInt32 flags = FlagNone);
    MojErr putImpl(MojObject& obj, MojUInt32 flags, MojDbReq& req, bool checkSchema = true,
				MojString shardId = MojString(), bool reverseTransaction = true);
    MojErr addShardIdToMasterKind(MojString shardId, MojObject& obj, MojDbReqRef req);
	MojErr putConfig(MojObject* begin, const MojObject* end, MojDbReq& req, MojDbPutHandler& handler);

	MojErr updateLocaleImpl(const MojString& oldLocale, const MojString& newLocale, MojDbReq& req);
	MojErr dumpImpl(MojFile& file, bool backup, bool incDel, const MojObject& revParam, const MojObject& delRevParam, bool skipKinds, MojUInt32& countOut, MojDbReq& req,
			MojObject* response, const MojChar* keyName, MojSize& bytesWritten, MojSize& warns, MojUInt32 maxBytes = 0);
	MojErr dumpObj(MojFile& file, MojObject obj, MojSize& bytesWrittenOut, MojUInt32 maxBytes = 0);
	MojErr findImpl(const MojDbQuery& query, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op);
	MojErr getImpl(const MojObject& id, MojObjectVisitor& visitor, MojDbOp op, MojDbReq& req);
	MojErr handleBackupFull(const MojObject& revParam, const MojObject& delRevParam, MojObject& response, const MojChar* keyName);
	MojErr insertIncrementalKey(MojObject& response, const MojChar* keyName, const MojObject& curRev);
	MojErr loadImpl(MojObject& obj, MojUInt32 flags, MojDbReq& req);
	MojErr purgeImpl(MojObject& obj, MojUInt32& countOut, MojDbReq& req);

    MojErr attachShardId(MojString shardId, MojObject& id);
	MojErr nextId(MojInt64& idOut);
	MojErr getState(const MojChar* key, MojObject& valOut, MojDbReq& req);
	MojErr updateState(const MojChar* key, const MojObject& val, MojDbReq& req);
	MojErr checkDbVersion(const MojChar* path);
    MojErr createVersionFile(const MojChar* path, const MojString versionFileName);

	MojErr beginReq(MojDbReq& req, bool lockSchema = false);
	MojErr commitKind(const MojString& id, MojDbReq& req, MojErr err);
	MojErr reloadKind(const MojString& id);
	MojErr assignIds(MojObject& objOut);
    MojErr checkSameKind(const MojObject& obj, const MojString& id, const MojDbReq& req, bool &isSame);

	MojDbQuotaCheckAlert m_quotaAlert;
    MojDbSpaceAlert m_spaceAlert;
	MojRefCountedPtr<MojDbStorageEngine> m_storageEngine;
	MojRefCountedPtr<MojDbStorageDatabase> m_objDb;
	MojRefCountedPtr<MojDbStorageSeq> m_idSeq;
	MojDbIdGenerator m_idGenerator;
	MojDbKindEngine m_kindEngine;
	MojDbPermissionEngine m_permissionEngine;
    MojDbQuotaEngine m_quotaEngine;
	MojDbShardEngine m_shardEngine;
	MojThreadRwLock m_schemaLock;
	MojString m_engineName;
	MojObject m_conf;
	MojInt64 m_purgeWindow;
	MojInt64 m_loadStepSize;
	bool m_isOpen;
    // Search Cache
    MojDbSearchCache m_searchCache;

	MojThreadMutex m_compact_mutex;
};

#endif /* MOJDB_H_ */
