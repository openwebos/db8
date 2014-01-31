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


#include "db/MojDbIndex.h"
#include "db/MojDb.h"
#include "db/MojDbCursor.h"
#include "db/MojDbKind.h"
#include "db/MojDbQueryPlan.h"
#include "core/MojObject.h"
#include "core/MojObjectSerialization.h"

const MojChar* const MojDbIndex::CountKey = _T("count");
const MojChar* const MojDbIndex::DelMissesKey = _T("delmisses");
const MojChar* const MojDbIndex::DefaultKey = _T("default");
const MojChar* const MojDbIndex::IncludeDeletedKey = _T("incDel");
const MojChar* const MojDbIndex::MultiKey = _T("multi");
const MojChar* const MojDbIndex::NameKey = _T("name");
const MojChar* const MojDbIndex::PropsKey = _T("props");
const MojChar* const MojDbIndex::SizeKey = _T("size");
const MojChar* const MojDbIndex::TypeKey = _T("type");
const MojChar* const MojDbIndex::WatchesKey = _T("watches");

//db.index

MojDbIndex::MojDbIndex(MojDbKind* kind, MojDbKindEngine* kindEngine)
: m_preCommitSlot(this, &MojDbIndex::handlePreCommit),
  m_postCommitSlot(this, &MojDbIndex::handlePostCommit),
  m_kind(kind),
  m_kindEngine(kindEngine),
  m_collection(NULL),
  m_idIndex(MojInvalidSize),
  m_includeDeleted(false),
  m_ready(false),
  m_delMisses(0)
{
}

MojDbIndex::~MojDbIndex()
{
}

MojErr MojDbIndex::fromObject(const MojObject& obj, const MojString& locale) 
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// check name
	MojString name;
	MojErr err = obj.getRequired(NameKey, name);
	MojErrCheck(err);
	err = validateName(name);
	MojErrCheck(err);
	m_name = name;
	m_locale = locale;
    if(m_locale == _T("en_CN"))
       m_locale = locale;

	// get deleted flag
	bool includeDel = false;
	if (obj.get(IncludeDeletedKey, includeDel)) {
		incDel(includeDel);
	}
	// add props
	MojObject props;
	err = obj.getRequired(PropsKey, props);
	MojErrCheck(err);
	MojObject propObj;
	MojSize j = 0;
	while (props.at(j++, propObj)) {
		err = addProp(propObj);
		MojErrCheck(err);
	}
	if (m_props.empty()) {
		MojErrThrowMsg(MojErrDbInvalidIndex, _T("db: no properties specified in index '%s'"), name.data());
	}
	m_obj = obj;

	return MojErrNone;
}

MojErr MojDbIndex::addProp(const MojObject& propObj, bool pushFront)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(!isOpen());

	// create extractor
	MojRefCountedPtr<MojDbExtractor> extractor;
	MojErr err = createExtractor(propObj, extractor);
	MojErrCheck(err);
	const MojString& name = extractor->name();

	// check for repeats
	if (m_propNames.find(name) != MojInvalidSize) {
		MojErrThrowMsg(MojErrDbInvalidIndex, _T("db: invalid index '%s' - property '%s' repeated"), m_name.data(), name.data());
	}

	// add to vecs
	if (pushFront) {
		err = m_propNames.insert(0, 1, name);
		MojErrCheck(err);
		err = m_props.insert(0, 1, extractor);
		MojErrCheck(err);
	} else {
		err = m_propNames.push(name);
		MojErrCheck(err);
		err = m_props.push(extractor);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIndex::open(MojDbStorageIndex* index, const MojObject& id, MojDbReq& req, bool created)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(!isOpen() && !m_props.empty());
	MojAssert(index);

	// we don't want to take built-in props into account when sorting indexes,
	m_sortKey = m_propNames;
	m_id = id;

	MojDbKey idKey;
	MojErr err = idKey.assign(id);
	MojErrCheck(err);
	err = m_idSet.put(idKey);
	MojErrCheck(err);
	err = addBuiltinProps();
	MojErrCheck(err);

	if (created && !isIdIndex()) {
		// if this index was just created, we need to re-index before committing the transaction
		MojDbStorageTxn* txn = req.txn();
		txn->notifyPreCommit(m_preCommitSlot);
		txn->notifyPostCommit(m_postCommitSlot);
	} else {
		// otherwise it's ready
		m_ready = true;
	}
	// and we're open
	m_index.reset(index);
	m_collection = m_index.get();

	return MojErrNone;
}

MojErr MojDbIndex::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (isOpen()) {
		if (m_index.get()) {
			MojErr err = m_index->close();
			MojErrCheck(err);
			m_index.reset();
		}
		m_watcherVec.clear();
		m_collection = NULL;
	}
	return MojErrNone;
}

MojErr MojDbIndex::stats(MojObject& objOut, MojSize& usageOut, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	
	MojSize count = 0;
	MojSize size = 0;
	MojErr err = m_index->stats(req.txn(), count, size);

    LOG_DEBUG("[db_mojodb] IndexStats: Kind: %s;Index: %s; Id: %zX; count= %zu; size= %zu; delMisses = %d, err= %d \n",
        m_kind->name().data(), m_name.data(), idIndex(), count, size, m_delMisses, err);

	MojErrCheck(err);
	usageOut += size;

	err = objOut.put(SizeKey, (MojInt64) size);
	MojErrCheck(err);
	err = objOut.put(CountKey, (MojInt64) count);
	MojErrCheck(err);
	err = objOut.put(DelMissesKey, (MojInt64) m_delMisses); // cumulative since start
	MojErrCheck(err);

	MojThreadReadGuard guard(m_lock);
	if (!m_watcherMap.empty()) {
		MojObject watcherInfo;
		for (WatcherMap::ConstIterator i = m_watcherMap.begin(); i != m_watcherMap.end(); ++i) {
			err = watcherInfo.put(i.key(), (MojInt64) i.value());
			MojErrCheck(err);
		}
		err = objOut.put(WatchesKey, watcherInfo);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIndex::drop(MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());

	MojErr err = m_index->drop(req.txn());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndex::updateLocale(const MojChar* locale, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	MojAssert(locale);

	bool haveCollate = false;
	for (PropVec::ConstIterator i = m_props.begin(); i != m_props.end(); ++i) {
       if ((*i)->collation() != MojDbCollationInvalid) {
			haveCollate = true;
			MojErr err = (*i)->updateLocale(locale);
			MojErrCheck(err);
       }
	}
	if (haveCollate) {
		// drop and reindex
		MojErr err = drop(req);
		MojErrCheck(err);
		err = build(req.txn());
		MojErrCheck(err);
	}
    m_locale.assign(locale);
    
	return MojErrNone;
}

MojErr MojDbIndex::update(const MojObject* newObj, const MojObject* oldObj, MojDbStorageTxn* txn, bool forcedel)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	MojAssert(newObj || oldObj);

	// figure out which versions we include
	bool includeOld = includeObj(oldObj);
	bool includeNew = includeObj(newObj);

	if (includeNew && !includeOld) {
		// we include the new but not the old, so just put all the new keys
		MojAssert(newObj);
		KeySet newKeys;
		MojErr err = getKeys(*newObj, newKeys);
		MojErrCheck(err);
		err = insertKeys(newKeys, txn);
		MojErrCheck(err);
		err = notifyWatches(newKeys, txn);
		MojErrCheck(err);
        LOG_DEBUG("[db_mojodb] IndexAdd: %s; Keys= %zu \n", this->m_name.data(), newKeys.size());
	} else if (includeOld && !includeNew) {
		// we include the old but not the new objects, so del all the old keys
		MojAssert(oldObj);
		KeySet oldKeys;
		MojErr err = getKeys(*oldObj, oldKeys);
		MojErrCheck(err);
		err = delKeys(oldKeys, txn, forcedel);
		MojErrCheck(err);
		err = notifyWatches(oldKeys, txn);
		MojErrCheck(err);
        LOG_DEBUG("[db_mojodb] IndexDel: %s; Keys= %zu \n", this->name().data(), oldKeys.size());
	} else if (includeNew && includeOld) {
		// we include old and new objects
		MojAssert(newObj && oldObj);
		KeySet newKeys;
		MojErr err = getKeys(*newObj, newKeys);
		MojErrCheck(err);
		KeySet oldKeys;
		err = getKeys(*oldObj, oldKeys);
		MojErrCheck(err);
		// we need to put the keys that are in the new set, but not in the old
		KeySet keysToPut;
		err = newKeys.diff(oldKeys, keysToPut);
		MojErrCheck(err);
		// we need to del the keys that are in the old set, but not in the new
		KeySet keysToDel;
		err = oldKeys.diff(newKeys, keysToDel);
		MojErrCheck(err);
		err = delKeys(keysToDel, txn, forcedel);
		MojErrCheck(err);
		err = insertKeys(keysToPut, txn);

        LOG_DEBUG("[db_mojodb] IndexMerge: %s; OldKeys= %zu; NewKeys= %zu; Dropped= %zu; Added= %zu ; err = %d\n",
			this->name().data(), oldKeys.size(), newKeys.size(), keysToDel.size(), keysToPut.size(), (int)err);

		MojErrCheck(err);
		// notify on union of old and new keys
		err = newKeys.put(oldKeys);
		MojErrCheck(err);
		err = notifyWatches(newKeys, txn);
		MojErrCheck(err);
	
	}
	return MojErrNone;
}

MojErr MojDbIndex::find(MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());

	MojAutoPtr<MojDbQueryPlan> plan(new MojDbQueryPlan(*m_kindEngine));
	MojAllocCheck(plan.get());
	MojErr err = plan->init(cursor.query(), *this);
	MojErrCheck(err);
	if (watcher) {
		// we have to add the watch before beginning the txn or we may miss events
		MojAssert(cursor.txn() == NULL);
		err = addWatch(*plan, cursor, watcher, req);
		MojErrCheck(err);
	}
	if (!cursor.txn()) {
		MojDbStorageTxn* txn = req.txn();
		bool cursorOwnsTxn = !(req.batch() || txn);
		if (txn) {
			cursor.txn(txn, cursorOwnsTxn);
		} else {
			MojRefCountedPtr<MojDbStorageTxn> localTxn;
			err = m_collection->beginTxn(localTxn);
			MojErrCheck(err);
			cursor.txn(localTxn.get(), cursorOwnsTxn);
			req.txn(localTxn.get());
		}
	}
	cursor.m_dbIndex = this;	// for debugging
	err = m_collection->find(plan, cursor.txn(), cursor.m_storageQuery);
	MojErrCheck(err);
	cursor.m_watcher = watcher;
	
	return MojErrNone;
}

bool MojDbIndex::canAnswer(const MojDbQuery& query) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	MojAssert(!m_propNames.empty());

	// if this index is not ready yet, return false
	if (!m_ready)
		return false;

	// The goal here is to figure out whether the results for the query are contiguous in
	// the index. That will be the case if all the props referenced are contiguous in our
	// prop vector, any prop referenced by an inequality op comes at the end of the
	// range of referenced props, any unreferenced props in our vector are at the end,
	// and the order prop is either the last referenced prop or, if all ops are equality ops,
	// the prop following the last referenced prop.
	const MojDbQuery::WhereMap& map = query.where();
	MojSize numQueryProps = map.size();
	// if there are more props in the query than in our index, no can do
	if (numQueryProps > m_propNames.size())
		return false;
	// skip the first prop if we have an incDel index and the query does not reference the del prop
	StringVec::ConstIterator propName = m_propNames.begin();
	PropVec::ConstIterator prop = m_props.begin();
	if (m_includeDeleted && !map.contains(MojDb::DelKey)) {
		++propName;
		++prop;
	}
	// if there are no props in query, but the order matches our first prop, we're good
	const MojString& orderProp = query.order();
	if (numQueryProps == 0) {
		if (orderProp.empty()) {
			return isIdIndex();
		} else {
			return *propName == orderProp;
		}
	}
	// check all remaining props
	for (; propName != m_propNames.end(); ++propName, ++prop) {
		--numQueryProps;
		// ensure that the current prop is referenced in the query (no gaps)
		MojDbQuery::WhereMap::ConstIterator mapIter = map.find(*propName);
		if (mapIter == map.end())
			return false;
		// ensure that if it is an inequality op, it is at the end
		if (numQueryProps > 0 && mapIter->lowerOp() != MojDbQuery::OpEq)
			return false;
		// ensure that collation matches
		if (mapIter->collation() != (*prop)->collation())
			return false;
		// hooray, we made it through all the props in the query without failing any tests.
		// there may still be unreferenced props at the end of the vector, but that's ok.
		if (numQueryProps == 0) {
			// make sure that we can satisfy the ordering. if there is no ordering, or we are
			// ordering on a referenced prop, we're golden
			if (!orderProp.empty() && !map.contains(orderProp)) {
				// ensure that the order prop is next in our vec
				StringVec::ConstIterator next = propName + 1;
				if (next == m_propNames.end() || *next != orderProp)
					return false;
			}
			// can do!
			return true;
		}
	}
	return false;
}

MojErr MojDbIndex::cancelWatch(MojDbWatcher* watcher)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	MojAssert(watcher);

    LOG_DEBUG("[db_mojodb] Index_cancelWatch: index name = %s; domain = %s\n",
		this->name().data(), watcher->domain().data());

	MojThreadWriteGuard guard(m_lock);
	MojSize idx;
	MojSize size = m_watcherVec.size();
	for (idx = 0; idx < size; ++idx) {
		if (m_watcherVec.at(idx).get() == watcher) {
			MojErr err = m_watcherVec.erase(idx);
			MojErrCheck(err);
			WatcherMap::Iterator iter;
			err = m_watcherMap.find(watcher->domain(), iter);
			MojErrCheck(err);
			if (iter != m_watcherMap.end()) {
				iter.value() -= 1;
				if (iter.value() == 0) {
					bool found = false;
					m_watcherMap.del(iter.key(), found);
					MojAssert(found);
					LOG_DEBUG("[db_mojodb] Index_cancelwatch: Domain Del found = %d; index name = %s; domain = %s\n",
 						(int)found, this->name().data(), watcher->domain().data());
				}
			}
			break;
		}
	}
	if (idx == size)
		MojErrThrow(MojErrDbWatcherNotRegistered);

	return MojErrNone;
}

MojErr MojDbIndex::createExtractor(const MojObject& propObj, MojRefCountedPtr<MojDbExtractor>& extractorOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(!isOpen());

	// type
	MojString type;
	bool found = false;
	MojErr err = propObj.get(TypeKey, type, found);
	MojErrCheck(err);
	if (found && type == MultiKey) {
		extractorOut.reset(new MojDbMultiExtractor);
		MojAllocCheck(extractorOut.get());
	} else {
		extractorOut.reset(new MojDbPropExtractor);
		MojAllocCheck(extractorOut.get());
	}
	err = extractorOut->fromObject(propObj, m_locale);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndex::addBuiltinProps()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojString idStr;
	MojErr err = idStr.assign(MojDb::IdKey);
	MojErrCheck(err);
	if (m_propNames.find(idStr) == MojInvalidSize) {
		MojObject idProp;
		err = idProp.put(NameKey, idStr);
		MojErrCheck(err);
		err = addProp(idProp);
		MojErrCheck(err);
	}
	if (m_includeDeleted && m_propNames.front() != MojDb::DelKey) {
		MojObject delProp;
		err = delProp.putString(NameKey, MojDb::DelKey);
		MojErrCheck(err);
		err = delProp.putBool(DefaultKey, false);
		MojErrCheck(err);
		err = addProp(delProp, true);
		MojErrCheck(err);
		MojAssert(m_propNames.front() == MojDb::DelKey);
	}
	m_idIndex = m_propNames.find(idStr);
	MojAssert(m_idIndex != MojInvalidSize);
	m_idIndex++; // account for prefix

	return MojErrNone;
}

bool MojDbIndex::includeObj(const MojObject* obj) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (!obj)
		return false;
	if (m_includeDeleted)
		return true;

	bool deleted = false;
	obj->get(MojDb::DelKey, deleted);
	return !deleted;
}

bool MojDbIndex::isIdIndex() const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return (m_propNames.size() == 1 && m_propNames.front() == MojDb::IdKey) ||
			(m_propNames.size() == 2 && m_propNames.front() == MojDb::DelKey && m_propNames.back() == MojDb::IdKey);
}

MojErr MojDbIndex::addWatch(const MojDbQueryPlan& plan, MojDbCursor& cursor, MojDbWatcher* watcher, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(watcher);

	// TODO: use interval tree instead of vector for watches
	MojThreadWriteGuard guard(m_lock);
	MojErr err = m_watcherVec.push(watcher);
	MojErrCheck(err);
	// update count map
	watcher->domain(req.domain());
	WatcherMap::Iterator iter;
	err = m_watcherMap.find(req.domain(), iter);
	MojErrCheck(err);
	if (iter == m_watcherMap.end()) {
		err = m_watcherMap.put(req.domain(), 1);
		MojErrCheck(err);
	} else {
		iter.value() += 1;
		if (iter.value() > WatchWarningThreshold) {
            LOG_WARNING(MSGID_MOJ_DB_INDEX_WARNING, 4,
            		PMLOGKS("domain", req.domain().data()),
            		PMLOGKFV("iter", "%zd", iter.value()),
            		PMLOGKS("kindId", m_kind->id().data()),
            		PMLOGKS("name", m_name.data()),
            		"db:'domain' has 'iter' watches open on index 'kindId - name'");
		}
	}
	LOG_DEBUG("[db_mojodb] DbIndex_addWatch - '%s' on index '%s - %s'",
		req.domain().data(),  m_kind->id().data(), m_name.data());
	// drop lock before acquiring watcher mutex in init
	guard.unlock();
	watcher->init(this, plan.ranges(), plan.desc(), false);

	return MojErrNone;
}

MojErr MojDbIndex::notifyWatches(const KeySet& keys, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);

	MojThreadReadGuard guard(m_lock);
	for (KeySet::ConstIterator i = keys.begin(); i != keys.end(); ++i) {
		MojErr err = notifyWatches(*i, txn);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbIndex::notifyWatches(const MojDbKey& key, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(txn);

	// TODO: this will be much more efficient with an interval tree
	for (WatcherVec::ConstIterator i = m_watcherVec.begin(); i != m_watcherVec.end(); ++i) {
		const RangeVec& ranges = (*i)->ranges();
		for (RangeVec::ConstIterator j = ranges.begin(); j != ranges.end(); ++j) {
			if (j->contains(key)) {
				LOG_DEBUG("[db_mojodb] DbIndex_notifywatches adding to txn - kind: %s; index %s;\n",
					((m_kind) ? m_kind->id().data() :NULL), ((m_name) ? m_name.data() : NULL));
				MojErr err = txn->addWatcher(i->get(), key);
				MojErrCheck(err);
				break;
			}
		}
	}
	return MojErrNone;
}

MojErr MojDbIndex::delKeys(const KeySet& keys, MojDbStorageTxn* txn, bool forcedel)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	int count = 0;
	for (KeySet::ConstIterator i = keys.begin(); i != keys.end(); ++i) {

		MojErr err = m_index->del(*i, txn);
#if defined(MOJ_DEBUG_LOGGING)
		char s[1024];
		char *s2 = NULL;
		if (m_kind)
			s2 = (char *)(m_kind->id().data());
		size_t size = (*i).size();
		MojErr err2 = MojByteArrayToHex((*i).data(), size, s); 
		MojErrCheck(err2); 
		if (size > 16)	// if the object-id is in key
			strncat(s, (char *)((*i).data()) + (size - 17), 16);
        LOG_DEBUG("[db_mojodb] delKey %d for: %s - %s; key= %s ; err= %d\n", count+1, s2, this->m_name.data(), s, err);
#endif

		// This has some potential risk
		if (err == MojErrInternalIndexOnDel) {
			m_delMisses++;
#if defined(MOJ_DEBUG_LOGGING)
            LOG_DEBUG("[db_mojodb] delKey %d for: %s - %s; key= %s; err = %d \n", count+1, s2, this->m_name.data(), s, err);
#endif
			if (forcedel)
				err = MojErrNone;
		}
		MojErrCheck(err);
		count++;
	}

	return MojErrNone;
}

MojErr MojDbIndex::insertKeys(const KeySet& keys, MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	int count = 0;
	for (KeySet::ConstIterator i = keys.begin(); i != keys.end(); ++i) {

		MojErr err = m_index->insert(*i, txn);
#if defined(MOJ_DEBUG_LOGGING)
		char s[1024];
		size_t size = (*i).size();
		MojErr err2 = MojByteArrayToHex((*i).data(), size, s); 
		MojErrCheck(err2);
		if (size > 16)	// if the object-id is in key
			strncat(s, (char *)((*i).data()) + (size - 17), 16);
        LOG_DEBUG("[db_mojodb] insertKey %d for: %s; key= %s ; err= %d\n", count+1, this->m_name.data(), s, err);
#endif
		MojErrCheck(err);
		count ++;
	}

	return MojErrNone;
}

MojErr MojDbIndex::getKeys(const MojObject& obj, KeySet& keysOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// build the set of unique keys from object
	MojDbKeyBuilder builder;
	MojErr err = builder.push(m_idSet);
	MojErrCheck(err);
	MojSize idx = 0;
	for (PropVec::ConstIterator i = m_props.begin();
		 i != m_props.end();
		 ++i, ++idx) {
		KeySet vals;
		err = (*i)->vals(obj, vals);
		MojErrCheck(err);
		err = builder.push(vals);
		MojErrCheck(err);
	}
	err = builder.keys(keysOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndex::handlePreCommit(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = m_kind->kindEngine()->db()->quotaEngine()->curKind(m_kind, txn);
	MojErrCheck(err);
	err = build(txn);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndex::handlePostCommit(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	m_ready = true;
	return MojErrNone;
}

MojErr MojDbIndex::build(MojDbStorageTxn* txn)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(isOpen());
	MojAssert(m_kind && m_kindEngine);
	MojAssert(m_props.size() > 1);

	// query for all existing objects of this type and add them to the index.
	MojDbQuery query;
	MojErr err = query.from(m_kind->id());
	MojErrCheck(err);
	if (m_includeDeleted) {
		err = query.includeDeleted();
		MojErrCheck(err);
	}

	MojDbCursor cursor;
	MojDbReq adminRequest(true);
	adminRequest.txn(txn);
	err = m_kindEngine->find(query, cursor, NULL, adminRequest, OpRead);
	MojErrCheck(err);

	for (;;) {
		MojObject obj;
		bool found = false;
		err = cursor.get(obj, found);
		MojErrCheck(err);
		if (!found)
			break;
		// add this object to the index
		err = update(&obj, NULL, txn, false);
		MojErrCheck(err);
	}
	err = cursor.close();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbIndex::validateName(const MojString& name)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (name.length() > MaxIndexNameLen) {
		MojErrThrowMsg(MojErrDbInvalidIndexName, _T("db: index name '%s' invalid: length is %zd chars, max is %zd"), name.data(), name.length(), MaxIndexNameLen);
	}
	for (MojString::ConstIterator i = name.begin(); i < name.end(); ++i) {
		if (!MojIsAlNum(*i) && *i != _T('_')) {
			MojErrThrowMsg(MojErrDbInvalidIndexName, _T("db: index name '%s' invalid: char at position %zd not allowed"), name.data(), (MojSize) (i - name.begin()));
		}
	}
	return MojErrNone;
}
