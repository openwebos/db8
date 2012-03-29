/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBKINDENGINE_H_
#define MOJDBKINDENGINE_H_

#include "db/MojDbDefs.h"
#include "db/MojDbWatcher.h"
#include "core/MojAutoPtr.h"
#include "core/MojHashMap.h"
#include "core/MojString.h"
#include "core/MojThread.h"
#include "core/MojTokenSet.h"

class MojDbKindEngine : private MojNoCopy
{
public:
	static const MojChar* const IndexesKindId;
	static const MojChar* const RootKindId;
	static const MojChar* const KindKindId;
	static const MojChar* const KindKindIdPrefix;
	static const MojChar* const RevTimestampId;
	static const MojChar* const DbStateId;
	static const MojChar* const PermissionId;
	static const MojChar* const PermissionIdPrefix;
	static const MojChar* const QuotaId;
	static const MojChar* const QuotaIdPrefix;

	typedef MojHashMap<MojString, MojRefCountedPtr<MojDbKind>, const MojChar*> KindMap;

	MojDbKindEngine();
	~MojDbKindEngine();

	MojErr open(MojDb* db, MojDbReq& req);
	MojErr close();
	MojErr stats(MojObject& objOut, MojDbReq& req, bool verify, MojString *pKind);
	MojErr updateLocale(const MojChar* locale, MojDbReq& req);

	MojErr update(MojObject* newObj, const MojObject* oldObj, MojDbReq& req,
                  MojDbOp op, MojTokenSet& tokenSetOut, bool checkSchema = true);
	MojErr find(const MojDbQuery& query, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op);
	MojErr checkPermission(const MojString& id, MojDbOp op, MojDbReq& req);
	MojErr checkOwnerPermission(const MojString& id, MojDbReq& req);

	MojErr getKinds(MojVector<MojObject>& kindsOut);
	MojErr putKind(const MojObject& obj, MojDbReq& req, bool builtin = false);
	MojErr delKind(const MojString& id, MojDbReq& req);
	MojErr reloadKind(const MojString& id, const MojObject& kindObj, MojDbReq& req);

	MojDb* db() { return m_db; }
	MojDbPermissionEngine* permissionEngine();
	MojDbQuotaEngine* quotaEngine();
	MojDbStorageSeq* indexSeq() { return m_indexIdSeq.get(); }
	MojDbStorageDatabase* indexIdDb() { return m_indexIdDb.get(); }
	MojDbStorageDatabase* kindDb() { return m_kindDb.get(); }
	KindMap& kindMap() { return m_kinds; }
	MojErr tokenSet(const MojChar* kindName, MojTokenSet& tokenSetOut);
	MojErr tokenFromId(const MojChar* id, MojInt64& tokOut);
	MojErr idFromToken(MojInt64 tok, MojString& idOut);
	MojErr getKind(const MojObject& obj, MojDbKind*& kind);
	MojErr getKind(const MojChar* kindName, MojDbKind*& kind);


private:
	typedef MojHashMap<MojInt64, MojString> TokMap;
	typedef MojErr (MojDbKind::* ConfigUpdateHandler)(const MojObject& obj, const KindMap& map, MojDbStorageTxn* txn);
	typedef MojErr (MojDbKind::* ConfigDeleteHandler)(const KindMap& map, MojDbStorageTxn* txn);

	static const MojChar* const KindIdPrefix;
	static const MojChar* const KindsDbName;
	static const MojChar* const IndexIdsDbName;
	static const MojChar* const IndexIdsSeqName;
	static const MojChar* const RootKindJson;
	static const MojChar* const KindKindJson;
	static const MojChar* const RevTimestampJson;
	static const MojChar* const DbStateJson;
	static const MojChar* const PermissionJson;
	static const MojChar* const QuotaJson;

	bool isOpen() const { return m_db != NULL; }
	MojErr setupRootKind();
	MojErr addBuiltin(const MojChar* json, MojDbReq& req);
	MojErr createKind(const MojString& id, const MojObject& obj, MojDbReq& req, bool builtIn = false);
	MojErr loadKinds(MojDbReq& req);


	MojDb* m_db;
	MojRefCountedPtr<MojDbStorageDatabase> m_kindDb;
	MojRefCountedPtr<MojDbStorageDatabase> m_indexIdDb;
	MojRefCountedPtr<MojDbStorageSeq> m_indexIdSeq;
	KindMap m_kinds;
	TokMap m_tokens;
	MojString m_locale;
	static MojLogger s_log;
};

#endif /* MOJDBKINDENGINE_H_ */
