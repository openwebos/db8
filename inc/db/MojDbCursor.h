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


#ifndef MOJDBCURSOR_H_
#define MOJDBCURSOR_H_

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDbQueryFilter.h"
#include "core/MojObjectFilter.h"
#include "core/MojNoCopy.h"

class MojDbCursor : private MojNoCopy
{
public:
	MojDbCursor();
	virtual ~MojDbCursor();
	virtual MojErr close();
	virtual MojErr get(MojDbStorageItem*& itemOut, bool& foundOut);
	virtual MojErr get(MojObject& objOut, bool& foundOut);
	virtual MojErr visit(MojObjectVisitor& visitor);
	virtual MojErr count(MojUInt32& countOut);
	virtual MojErr nextPage(MojDbQuery::Page& pageOut);

	bool isOpen() const { return m_storageQuery.get() != NULL; }
    const MojDbStorageQuery * storageQuery() { return m_storageQuery.get();  }
	const MojDbQuery& query() const { return m_query; }
	MojDbStorageTxn* txn() { return m_txn.get(); }
	void verifymode(bool bval) { m_vmode = bval;}
	bool verifymode() const{ return m_vmode;}
    void setIndex(MojDbIndex * ind) { m_dbIndex = ind; }
    

protected:
	friend class MojDbIndex;
	friend class MojDbKindEngine;
	friend class MojDbKind;

	virtual MojErr init(const MojDbQuery& query);
	MojErr initImpl(const MojDbQuery& query);
	MojErr visitObject(MojObjectVisitor& visitor, bool& foundOut);
	void txn(MojDbStorageTxn* txn, bool ownTxn);
	void kindEngine(MojDbKindEngine* kindEngine) { m_kindEngine = kindEngine; }
	void excludeKinds(const MojSet<MojString>& toExclude);

	bool m_ownTxn;
	MojErr m_lastErr;
	MojDbKindEngine* m_kindEngine;
	MojDbQuery m_query;
	MojRefCountedPtr<MojDbStorageTxn> m_txn;
	MojRefCountedPtr<MojDbStorageQuery> m_storageQuery;
	MojAutoPtr<MojObjectFilter> m_objectFilter;
	MojAutoPtr<MojDbQueryFilter> m_queryFilter;
	MojRefCountedPtr<MojDbWatcher> m_watcher;
	MojDbIndex* m_dbIndex;
	bool m_vmode;
};

#endif /* MOJDBCURSOR_H_ */
