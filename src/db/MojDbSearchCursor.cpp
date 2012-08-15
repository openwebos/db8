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


#include "db/MojDbSearchCursor.h"
#include "db/MojDbExtractor.h"
#include "db/MojDbIndex.h"
#include "db/MojDb.h"

MojDbSearchCursor::MojDbSearchCursor(MojString localeStr)
: m_limit(0),
  m_pos(NULL),
  m_locale(localeStr)
{
}

MojDbSearchCursor::~MojDbSearchCursor()
{
}

MojErr MojDbSearchCursor::close()
{
	MojErr err = MojErrNone;
	MojErr errClose = MojDbCursor::close();
	MojErrAccumulate(err, errClose);

	m_limit = 0;
	m_pos = NULL;
	m_limitPos = NULL;
	m_items.clear();

	return err;
}

MojErr MojDbSearchCursor::get(MojDbStorageItem*& itemOut, bool& foundOut)
{
	foundOut = false;
	MojErr err = begin();
	MojErrCheck(err);

	if (m_pos != m_limitPos) {
		foundOut = true;
		itemOut = m_pos->get();
		++m_pos;
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::count(MojUInt32& countOut)
{
	countOut = 0;
	MojErr err = begin();
	MojErrCheck(err);
	countOut = (MojUInt32) m_items.size();

	return MojErrNone;
}

MojErr MojDbSearchCursor::nextPage(MojDbQuery::Page& pageOut)
{
	// TODO: support paging by writing index
	return MojErrNone;
}

MojErr MojDbSearchCursor::init(const MojDbQuery& query)
{
	MojErr err = initImpl(query);
	MojErrCheck(err);

	// override limit and sort since we need to retrieve everything
	// and sort before re-imposing limit
	m_limit = query.limit();
	m_orderProp = query.order();
	m_query.limit(MaxResults);
	err = m_query.order(_T(""));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbSearchCursor::begin()
{
	if (!loaded()) {
		MojErr err = load();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::load()
{
	// pull unique ids from index
	ObjectSet ids;
	MojErr err = loadIds(ids);
	MojErrCheck(err);

	// load objects into memory
	err = loadObjects(ids);
	MojErrCheck(err);
	// sort results
	if (!m_orderProp.empty()) {
		err = sort();
		MojErrCheck(err);
	}
	// reverse for desc
	if (m_query.desc()) {
		err = m_items.reverse();
		MojErrCheck(err);
	}
	// set limit and pos
	if (m_limit >= m_items.size()) {
		m_limitPos = m_items.end();
	} else {
		m_limitPos = m_items.begin() + m_limit;
	}
	m_pos = m_items.begin();

	return MojErrNone;
}

MojErr MojDbSearchCursor::loadIds(ObjectSet& idsOut)
{
	MojUInt32 groupNum = 0;
	bool found = false;
	MojSharedPtr<ObjectSet> group;
	GroupMap groupMap;

	for(;;) {
		// get current id
		MojObject id;
		MojUInt32 idGroupNum = 0;
		MojErr err = m_storageQuery->getId(id, idGroupNum, found);
		MojErrCheck(err);
		if (!found)
			break;

		// if it is in a new group, create a new set
		if (!group.get() || idGroupNum != groupNum) {
			// find/create new group
			GroupMap::Iterator iter;
			err = groupMap.find(idGroupNum, iter);
			MojErrCheck(err);
			if (iter != groupMap.end()) {
				group = iter.value();
			} else {
				err = group.resetChecked(new ObjectSet);
				MojErrCheck(err);
				err = groupMap.put(idGroupNum, group);
				MojErrCheck(err);
			}
			groupNum = idGroupNum;
		}
		// add id to current set
		err = group->put(id);
		MojErrCheck(err);
	}

	// no matches unless all groups are accounted for
	MojUInt32 groupCount = m_storageQuery->groupCount();
	for (MojUInt32 i = 0; i < groupCount; ++i) {
		if (!groupMap.contains(i))
			return MojErrNone;
	}

	// find intersection of all groups
	GroupMap::ConstIterator begin = groupMap.begin();
	for (GroupMap::ConstIterator i = begin; i != groupMap.end(); ++i) {
		if (i == begin) {
			// special handling for first group
			idsOut = *(i.value());
		} else {
			MojErr err = idsOut.intersect(*(i.value()));
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbSearchCursor::loadObjects(const ObjectSet& ids)
{
	MojInt32 warns = 0;
	for (ObjectSet::ConstIterator i = ids.begin(); i != ids.end(); ++i) {
		// get item by id
		MojObject obj;
		MojDbStorageItem* item = NULL;
		bool found = false;
		MojErr err = m_storageQuery->getById(*i, item, found);
		if (err == MojErrInternalIndexOnFind) {
			warns++;
			continue;
		}
		
		MojErrCheck(err);
		if (found) {
			// get object from item
			err = item->toObject(obj, *m_kindEngine);
			MojErrCheck(err);
			// filter results
			if (m_queryFilter.get() && !m_queryFilter->test(obj))
				continue;
			// create object item
			MojRefCountedPtr<MojDbObjectItem> item(new MojDbObjectItem(obj));
			MojAllocCheck(item.get());
			// add to vec
			err = m_items.push(item);
			MojErrCheck(err);
		}
	}
	if (warns > 0) 
		MojLogWarning(MojDb::s_log, _T("Search warnings: %d \n"), warns);
	return MojErrNone;
}

MojErr MojDbSearchCursor::sort()
{
	MojAssert(!m_orderProp.empty());

	// TODO: instead of parsing all objects, find the serialized field in the object and compare it directly
	// create extractor for sort prop
	MojRefCountedPtr<MojDbTextCollator> collator(new MojDbTextCollator);
	MojAllocCheck(collator.get());
	// TODO: use real locale
	//MojErr err = collator->init(_T(""), MojDbCollationPrimary);
    MojErr err = MojErrNone;
    
    if(m_dbIndex) 
       err = collator->init(m_dbIndex->locale(), MojDbCollationPrimary);
    else 
       err = collator->init(m_locale, MojDbCollationPrimary);
    MojErrCheck(err);
    
	MojDbPropExtractor extractor;
	extractor.collator(collator.get());
	err = extractor.prop(m_orderProp);
	MojErrCheck(err);

	// create sort keys
	MojDbKeyBuilder builder;
	ItemVec::Iterator begin;
	err = m_items.begin(begin);
	MojErrCheck(err);
	for (ItemVec::Iterator i = begin; i != m_items.end(); ++i) {
		KeySet keys;
		err = extractor.vals((*i)->obj(), keys);
		MojErrCheck(err);
		(*i)->sortKeys(keys);
	}

	// sort
	err = m_items.sort();
	MojErrCheck(err);

	return MojErrNone;
}
