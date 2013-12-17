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


#ifndef MOJDBKIND_H_
#define MOJDBKIND_H_

#include "db/MojDbDefs.h"
#include "db/MojDbIndex.h"
#include "db/MojDbPermissionEngine.h"
#include "db/MojDbRevisionSet.h"
#include "db/MojDbKindState.h"
#include "core/MojHashMap.h"
#include "core/MojObject.h"
#include "core/MojSchema.h"
#include "core/MojSet.h"
#include "core/MojString.h"
#include "core/MojTokenSet.h"
#include "core/MojVector.h"

class MojDbKind : public MojRefCounted
{
public:
	typedef MojHashMap<MojString, MojRefCountedPtr<MojDbKind>, const MojChar*> KindMap;
	typedef MojVector<MojString> StringVec;
	typedef MojVector<MojDbKind*> KindVec;
	typedef MojVector<MojByte> ByteVec;

	static const MojChar* const CountKey;
	static const MojChar* const DelCountKey;
	static const MojChar* const DelSizeKey;
	static const MojChar* const ExtendsKey;
	static const MojChar* const IndexesKey;
	static const MojChar* const NameKey;
	static const MojChar* const ObjectsKey;
	static const MojChar* const OwnerKey;
	static const MojChar* const PermissionType;
	static const MojChar* const RevisionSetsKey;
	static const MojChar* const SchemaKey;
	static const MojChar* const SizeKey;
	static const MojChar* const SyncKey;
	static const MojChar* const IdIndexName;
	static const MojChar* const VerifyCountKey;
	static const MojChar* const VerifyWarnCountKey;
	static const MojChar* const VerifyDelCountKey;
	static const MojChar* const WarnKey;

	MojDbKind(MojDbStorageDatabase* db, MojDbKindEngine* kindEngine, bool builtIn = false);
	~MojDbKind();

	bool extends(const MojString& id) const;
	const MojString& id() const { return m_id; }
	const MojString& name() const { return m_name; }
	const MojString& owner() const { return m_owner; }
	const MojObject& object() const { return m_obj; }
	const StringVec& superIds() const { return m_superIds; }
	const KindVec& supers() const { return m_supers; }
    bool isBuiltin() const { return m_builtin; }
	MojDbKindEngine* kindEngine() const { return m_kindEngine; }
	MojInt64 token() const { return m_state->token(); }
	MojUInt32 version() const { return m_version; }
	MojUInt32 nsubkinds() const {return (MojUInt32)m_subs.size(); }

	MojErr stats(MojObject& objOut, MojSize& usageOut, MojDbReq& req, bool verify);
	MojErr verifyIndex(MojDbIndex *pIndex, MojObject &iinfo, MojDbReq& req);
	MojErr init(const MojString& id);
	MojErr configure(const MojObject& obj, const KindMap& map, const MojString& locale, MojDbReq& req);
	MojErr addIndex(const MojRefCountedPtr<MojDbIndex>& index);
	MojErr addSuper(MojDbKind* kind);
	MojErr drop(MojDbReq& req);
	MojErr close();
	MojErr updateLocale(const MojChar* locale, MojDbReq& req);

	MojErr update(MojObject* newObj, const MojObject* oldObj, MojDbOp op,
                  MojDbReq& req, bool checkSchema = true);
	MojErr find(MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req, MojDbOp op);
	MojErr subKinds(MojVector<MojObject>& kindsOut, const MojDbKind* parent = NULL);
	MojErr tokenSet(MojTokenSet& tokenSetOut);
	MojErr checkPermission(MojDbOp op, MojDbReq& req);
	MojErr checkOwnerPermission(MojDbReq& req);
    MojDbIndex* indexForCollation(const MojDbQuery& query) { return indexForQuery(query); }
	
private:
	struct IndexComp
	{
		int operator()(const MojRefCountedPtr<MojDbIndex>& i1, const MojRefCountedPtr<MojDbIndex>& i2) const
		{
			return i1->sortKey().compare(i2->sortKey());
		}
	};
	typedef MojVector<MojRefCountedPtr<MojDbIndex>, MojEq<MojRefCountedPtr<MojDbIndex> >, IndexComp> IndexVec;
	typedef MojVector<MojRefCountedPtr<MojDbRevisionSet> > RevSetVec;
	typedef MojSet<MojObject> ObjectSet;
	typedef MojSet<MojString> StringSet;

	static const MojChar* const IdIndexJson;
	static const MojChar VersionSeparator;
	static const MojSize KindIdLenMax = 256;

	bool hasOwnerPermission(MojDbReq& req);
	MojDbIndex* indexForQuery(const MojDbQuery& query) const;
	MojDbPermissionEngine::Value objectPermission(const MojChar* op, MojDbReq& req);
	MojErr deny(MojDbReq& req);
	MojErr updateIndexes(const MojObject* newObj, const MojObject* oldObj, const MojDbReq& req, MojDbOp op, MojVector<MojDbKind*>& kindVec, MojInt32& idxcount);
	MojErr updateOwnIndexes(const MojObject* newObj, const MojObject* oldObj, const MojDbReq& req, MojInt32& idxcount);
	MojErr preUpdate(MojObject* newObj, const MojObject* oldObj, MojDbReq& req);
	MojErr configureIndexes(const MojObject& obj, const MojString& locale, MojDbReq& req);
	MojErr configureRevSets(const MojObject& obj);
	MojErr updateSupers(const KindMap& map, const StringVec& superIds, bool updating, MojDbReq& req);
	MojErr diffSupers(const KindMap& map, const StringVec& vec1, const StringVec& vec2, KindVec& diffOut);
	MojErr clearSupers();
	MojErr openIndex(MojDbIndex* index, MojDbReq& req);
	MojErr dropIndex(MojDbIndex* index, MojDbReq& req);
	static MojErr removeKind(KindVec& vec, MojDbKind* kind);
	static const MojChar* stringFromOperation(MojDbOp op);

	MojObject m_obj;
	MojString m_id;
	MojString m_name;
	MojString m_owner;
	MojSchema m_schema;
	ObjectSet m_indexObjects;
	IndexVec m_indexes;
	RevSetVec m_revSets;
	StringVec m_superIds;
	KindVec m_supers;
	KindVec m_subs;
	MojUInt32 m_version;
	MojDbStorageDatabase* m_db;
	MojDbKindEngine* m_kindEngine;
	MojRefCountedPtr<MojDbKindState> m_state;
	bool m_backup;
	bool m_builtin;
};

#endif /* MOJDBKIND_H_ */
