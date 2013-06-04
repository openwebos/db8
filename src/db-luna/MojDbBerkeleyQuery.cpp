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


#include "db-luna/MojDbBerkeleyQuery.h"
#include "db-luna/MojDbBerkeleyEngine.h"
#include "db/MojDbQueryPlan.h"
#include "core/MojObjectSerialization.h"

const MojUInt32 MojDbBerkeleyQuery::OpenFlags = 0; //DB_TXN_SNAPSHOT;
const MojUInt32 MojDbBerkeleyQuery::SeekFlags = DB_SET_RANGE;
const MojUInt32 MojDbBerkeleyQuery::SeekIdFlags = DB_GET_BOTH_RANGE;
const MojUInt32 MojDbBerkeleyQuery::SeekEmptyFlags[2] = {DB_FIRST, DB_LAST};
const MojUInt32 MojDbBerkeleyQuery::NextFlags[2] = {DB_NEXT, DB_PREV};

MojDbBerkeleyQuery::MojDbBerkeleyQuery()
{
}

MojDbBerkeleyQuery::~MojDbBerkeleyQuery()
{
	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDbBerkeleyQuery::open(MojDbBerkeleyDatabase* db, MojDbBerkeleyDatabase* joinDb,
		MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn)
{
	MojAssert(!m_isOpen);
	MojAssert(db && db->impl() && plan.get());

	MojErr err = MojDbIsamQuery::open(plan, txn);
	MojErrCheck(err);
	err = m_cursor.open(db, txn, OpenFlags);
	MojErrCheck(err);

	m_db = joinDb;

	return MojErrNone;
}

MojErr MojDbBerkeleyQuery::close()
{
	MojErr err = MojErrNone;

	if (m_isOpen) {
		MojErr errClose = MojDbIsamQuery::close();
		MojErrAccumulate(err, errClose);
		errClose = m_cursor.close();
		MojErrAccumulate(err, errClose);
		m_db = NULL;
	}
	return err;
}

MojErr MojDbBerkeleyQuery::getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut)
{
	itemOut = NULL;
	foundOut = false;
	MojDbBerkeleyItem* item = NULL;
	MojErr err = MojErrNone;

	if (m_db) {
		// retrun val from primary db
		MojDbBerkeleyItem primaryKey;
		err = primaryKey.fromObject(id);
		MojErrCheck(err);
		err = m_db->get(primaryKey, m_txn, false, m_primaryVal, foundOut);
		MojErrCheck(err);
		if (!foundOut) {
			char s[1024];
			int size = (int)primaryKey.size();
			(void) MojByteArrayToHex(primaryKey.data(), size, s); 
			MojLogInfo(MojDbBerkeleyEngine::s_log, _T("bdbq_byId_warnindex: KeySize: %d; %s ;id: %s \n"), 
								 size, s, primaryKey.data()+1);
		
			//MojErrThrow(MojErrDbInconsistentIndex);
			MojErrThrow(MojErrInternalIndexOnFind);
		}
		item = &m_primaryVal;
	} else {
		// return val from cursor
		item = &m_val;
	}
	item->id(id);

	// check for exclusions
	bool exclude = false;
	err = checkExclude(item, exclude);
	MojErrCheck(err);
	if (!exclude) {
		itemOut = item;
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyQuery::seekImpl(const ByteVec& key, bool desc, bool& foundOut)
{
	if (key.empty()) {
		// if key is empty, seek to beginning (or end if desc)
		MojErr err = getKey(foundOut, SeekEmptyFlags[desc]);
		MojErrCheck(err);
	} else {
		// otherwise seek to the key
		MojErr err = m_key.fromBytes(key.begin(), key.size());
		MojErrCheck(err);
		err = getKey(foundOut, SeekFlags);
		MojErrCheck(err);
		// if descending, skip the first result (which is outside the range)
		if (desc) {
			err = next(foundOut);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbBerkeleyQuery::next(bool& foundOut)
{
	MojAssert(m_state == StateNext);

	MojErr err = getKey(foundOut, NextFlags[m_plan->desc()]);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyQuery::getVal(MojDbStorageItem*& itemOut, bool& foundOut)
{
	MojObject id;
	MojErr err = parseId(id);
	MojErrCheck(err);
	err = getById(id, itemOut, foundOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbBerkeleyQuery::getKey(bool& foundOut, MojUInt32 flags)
{
	MojErr err = m_cursor.get(m_key, m_val, foundOut, flags);
	MojErrCheck(err);
	m_keyData = m_key.data();
	m_keySize = m_key.size();

	return MojErrNone;
}
