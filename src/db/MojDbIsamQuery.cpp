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


#include "db/MojDbIsamQuery.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"

MojDbIsamQuery::MojDbIsamQuery()
{
	init();
}

MojDbIsamQuery::~MojDbIsamQuery()
{
	MojErr err = close();
	MojErrCatchAll(err);
}

MojErr MojDbIsamQuery::open(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn)
{
	MojAssert(!m_isOpen);
	MojAssert(plan.get() && txn);

	m_isOpen = true;
	m_plan = plan;
	m_iter = m_plan->ranges().begin();
	m_state = StateSeek;
	m_txn = txn;
	m_endKey.clear();

	return MojErrNone;
}

MojErr MojDbIsamQuery::close()
{
	MojErr err = MojErrNone;

	if (m_isOpen) {
		m_plan.reset();
		init();
	}
	return err;
}

MojErr MojDbIsamQuery::get(MojDbStorageItem*& itemOut, bool& foundOut)
{
	MojAssert(m_isOpen);

	itemOut = NULL;
	int i1 = 0; 
	int i2 = 0;
	// loop until we get an actual item or there are no more db entries
	do {
		i1++;
		MojErr err = getImpl(itemOut, foundOut, true);
		if (err == MojErrInternalIndexOnFind && !m_verify) {
			foundOut = true;  
			itemOut = NULL;
			i2++;
			continue;
		}
		MojErrCheck(err);
	} while (itemOut == NULL && foundOut);
	MojLogInfo(MojDb::s_log, _T("isamquery_get: found: %d; count1= %d; count2= %d\n"), (int)foundOut, i1, i2);
	return MojErrNone;
}

MojErr MojDbIsamQuery::getId(MojObject& idOut, MojUInt32& groupOut, bool& foundOut)
{
	MojErr err = getKey(groupOut, foundOut);
	MojErrCheck(err);
	if (foundOut) {
		err = parseId(idOut);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::count(MojUInt32& countOut)
{
	MojAssert(m_isOpen);

	countOut = 0;
	MojInt32 warns = 0; 
	m_plan->limit(MojUInt32Max);
	bool found = false;
	do {
		// Iterate over all the db results but only count
		// the ones that would not be excluded.  If we're not
		// excluding any kinds, getImpl does not need to get the
		// storage item.
		MojDbStorageItem* item = NULL;
		//MojErr err = getImpl(item, found, !m_excludeKinds.empty());  // orig
		//to ensure that we do not count ghost keys, me need to always try to get the item as well
		MojErr err = getImpl(item, found, true);
		if (err == MojErrInternalIndexOnFind) {
			found = true;			// to continue with counting
			warns++;
			continue;			// we ignore such keys; it is not counted in getImpl either
		}
		MojErrCheck(err);
	} while (found);
	countOut = m_count;
	if (warns > 0) {
		const MojChar * from = m_plan->query().from().data();
		MojLogInfo(MojDb::s_log, _T("isamquery_count: from: %s; indexid: %zu; warnings: %d \n"), 
								 from, m_plan->idIndex(), warns);
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::nextPage(MojDbQuery::Page& pageOut)
{
	if (limitEnforced()) {
		pageOut.key() = m_endKey;
	} else {
		pageOut.clear();
	}
	return MojErrNone;
}

MojUInt32 MojDbIsamQuery::groupCount() const
{
	return m_plan->groupCount();
}

MojErr MojDbIsamQuery::getImpl(MojDbStorageItem*& itemOut, bool& foundOut, bool getItem)
{
	itemOut = NULL;
	MojUInt32 group = 0;
	MojErr err = getKey(group, foundOut);
	MojErrCheck(err);
	if (foundOut && getItem) {
		err = getVal(itemOut, foundOut);
		if (err == MojErrInternalIndexOnFind) {
//#if defined (MOJ_DEBUG) 
			char s[1024];
			char *s2 = NULL;
			MojErr err2 =  MojByteArrayToHex(m_keyData, m_keySize, s); 
			MojErrCheck(err2);
			if (m_keySize > 17)
				s2 = ((char *)m_keyData) + m_keySize - 17;
			MojSize idIndex = m_plan->idIndex();
			const MojChar * from = m_plan->query().from().data();
			MojLogInfo(MojDb::s_log, _T("isamquery_warnindex: from: %s; indexid: %zu; group: %d; KeySize: %zu; %s ;id: %s \n"), 
								 from, idIndex, (int)group, m_keySize, s, s2);
//#endif 
		}
		MojErrCheck(err);
	}
	if (foundOut) {
		incrementCount();
	}
	return MojErrNone;
}

void MojDbIsamQuery::init()
{
	m_isOpen = false;
	m_count = 0;
	m_state = StateInvalid;
	m_iter = NULL;
	m_txn = NULL;
	m_keySize = 0;
	m_keyData = NULL;
	m_verify = false;
}

bool MojDbIsamQuery::match()
{
	// compare against lower key when descending, upper otherwise
	bool desc = m_plan->desc();
	const ByteVec& key = m_iter->key(!desc).byteVec();
	if (key.empty())
		return true;
	// test for >= when descending, < otherwise
	int comp = compareKey(key);
	return (comp >= 0) == desc;
}

int MojDbIsamQuery::compareKey(const ByteVec& key)
{
	return MojLexicalCompare(m_keyData, m_keySize, key.begin(), key.size());
}

MojErr MojDbIsamQuery::incrementCount()
{
	++m_count;
	// if we hit the limit, keep the end key
	// so we can adjust our watchers to respect the limit
	if (limitEnforced()) {
		MojErr err = saveEndKey();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::seek(bool& foundOut)
{
	MojAssert(m_state == StateSeek);

	// if descending, seek to upper bound, lower otherwise
	bool desc = m_plan->desc();
	const ByteVec& key = m_iter->key(desc).byteVec();
	m_state = StateNext;

	// if the last key returned while iterating over the previous range is >=
	// our first key, we can use our current position instead of seeking
	if (m_keySize > 0) {
		if ((compareKey(key) < 0) == desc) {
			foundOut = true;
			return MojErrNone;
		}
	}
	MojErr err = seekImpl(key, desc, foundOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIsamQuery::getKey(MojUInt32& groupOut, bool& foundOut)
{
	MojAssert(m_isOpen);

	foundOut = false;
	MojErr err = MojErrNone;
	RangeVec::ConstIterator end = m_plan->ranges().end();

	// Note, we consider the limit enforced when the count equals the limit.
	// The count is incremented outside of getKey, depending on
	// whether the item matching this key can be returned to the caller.
	while (m_iter != end && !limitEnforced()) {
		// do the get
		bool keyFound = false;
		if (m_state == StateSeek) {
			err = seek(keyFound);
			MojErrCheck(err);
		} else {
			err = next(keyFound);
			MojErrCheck(err);
		}
		// we're done if we hit the end of the index
		if (!keyFound) {
			m_iter = end;
			break;
		}

		// check to see if we're still in the range
		if (match()) {
			groupOut = m_iter->group();
			foundOut = true;
			break;
		}
		// key fell outside of range, so move on to next range
		m_state = StateSeek;
		++m_iter;
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::saveEndKey()
{
	MojErr err = m_endKey.assign(m_keyData, m_keySize);
	MojErrCheck(err);
	// if we're going in ascending order, the end is one past
	if (!m_plan->desc()) {
		err = m_endKey.increment();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::parseId(MojObject& idOut)
{
	// parse id out of key
	MojObjectEater eater;
	MojObjectReader reader(m_keyData, m_keySize);
	for (MojSize i = 0; i < m_plan->idIndex(); ++i) {
		MojErr err = reader.nextObject(eater);
		MojErrCheck(err);
	}
	MojObjectBuilder builder;
	MojErr err = reader.nextObject(builder);
	MojErrCheck(err);
	idOut = builder.object();

	return MojErrNone;
}

MojErr MojDbIsamQuery::checkExclude(MojDbStorageItem* item, bool& excludeOut)
{
	excludeOut = false;
	if (!m_excludeKinds.empty()) {
		MojString kindId;
		MojErr err = item->kindId(kindId, m_plan->kindEngine());
		MojErrCheck(err);
		if (m_excludeKinds.contains(kindId)) {
			excludeOut = true;
		}
	}
	return MojErrNone;
}
