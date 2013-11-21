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


#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "db/MojDbIndex.h"
#include "db/MojDbTextTokenizer.h"
#include "core/MojObjectSerialization.h"

MojDbQueryPlan::MojDbQueryPlan(MojDbKindEngine& kindEngine)
: m_idPropIndex(0),
  m_groupCount(0),
  m_kindEngine(kindEngine)
{
}

MojDbQueryPlan::~MojDbQueryPlan()
{
}

MojErr MojDbQueryPlan::init(const MojDbQuery& query, const MojDbIndex& index)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	m_query = query;
	m_locale = index.locale();
	m_idPropIndex = index.idIndex();
	m_ranges.clear();

	MojErr err = MojErrNone;
	if (index.includeDeleted() && !m_query.where().contains(MojDb::DelKey)) {
		err = m_query.where(MojDb::DelKey, MojDbQuery::OpEq, false);
		MojErrCheck(err);
	}
	// build ranges from where clauses
	err = buildRanges(index);
	MojErrCheck(err);
	if (query.desc()) {
		// reverse ranges if descending
		err = m_ranges.reverse();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQueryPlan::buildRanges(const MojDbIndex& index)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(!index.props().empty());

	MojErr err = MojErrNone;
	const MojDbQuery::WhereMap& where = m_query.where();
	const StringVec& props = index.props();
	const MojDbQuery::WhereClause* lastClause = NULL;
	MojDbKeyBuilder lowerBuilder;
	MojDbKeyBuilder upperBuilder;
	MojDbKeyBuilder::KeySet prefixKeys;

	err = pushVal(lowerBuilder, index.id(), NULL);
	MojErrCheck(err);
	err = pushVal(upperBuilder, index.id(), NULL);
	MojErrCheck(err);

	// push vals for all props in index prop order to create upper and lower bounds
	for (StringVec::ConstIterator i = props.begin(); i != props.end(); ++i) {
		// get clause for prop
		MojDbQuery::WhereMap::ConstIterator clause = where.find(*i);
		if (clause == where.end()) {
			MojAssert((MojSize)(i - props.begin()) == where.size());
			break;
		}

		// create collator
		MojRefCountedPtr<MojDbTextCollator> collator;
		MojDbCollationStrength coll = clause->collation();
		if (coll != MojDbCollationInvalid) {
			collator.reset(new MojDbTextCollator);
			MojAllocCheck(collator.get());
			err = collator->init(m_locale, coll);
			MojErrCheck(err);
		}

		// push vals for clause
		MojDbQuery::CompOp lowerOp = clause->lowerOp();
		if (lowerOp == MojDbQuery::OpSearch) {
			// tokenize search strings
			err = pushSearch(lowerBuilder, upperBuilder, clause->lowerVal(), collator.get());
			MojErrCheck(err);
		} else {
			// copy off prefix before writing an inequality val
			if (lowerOp != MojDbQuery::OpEq && lowerOp != MojDbQuery::OpPrefix) {
				err = lowerBuilder.keys(prefixKeys);
				MojErrCheck(err);
				if (prefixKeys.empty()) {
					// if there is no prefix yet, put an empty key
					err = prefixKeys.put(MojDbKey());
					MojErrCheck(err);
				}
			}
			// append lower-value to lower-key
			err = pushVal(lowerBuilder, clause->lowerVal(), collator.get());
			MojErrCheck(err);
			// append appropriate value to upper-key
			if (clause->upperOp() == MojDbQuery::OpNone) {
				err = pushVal(upperBuilder, clause->lowerVal(), collator.get());
				MojErrCheck(err);
			} else {
				err = pushVal(upperBuilder, clause->upperVal(), collator.get());
				MojErrCheck(err);
			}
		}
		lastClause = &clause.value();
	}

	// go from builders to ranges
	KeySet lowerKeys;
	err = lowerBuilder.keys(lowerKeys);
	MojErrCheck(err);
	KeySet upperKeys;
	err = upperBuilder.keys(upperKeys);
	MojErrCheck(err);
	if (prefixKeys.empty()) {
		prefixKeys = lowerKeys;
	}
	err = rangesFromKeySets(lowerKeys, upperKeys, prefixKeys, lastClause);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryPlan::rangesFromKeySets(const KeySet& lowerKeys, const KeySet& upperKeys, const KeySet& prefixKeys,
		const MojDbQuery::WhereClause* clause)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(lowerKeys.size() == upperKeys.size() && lowerKeys.size() == prefixKeys.size());

	MojUInt32 index = 0;
	KeySet::ConstIterator lowerIter = lowerKeys.begin();
	KeySet::ConstIterator upperIter = upperKeys.begin();
	KeySet::ConstIterator prefixIter = prefixKeys.begin();
	while (lowerIter != lowerKeys.end()) {
		MojErr err = rangesFromKeys(*lowerIter, *upperIter, *prefixIter, index, clause);
		MojErrCheck(err);
		++index;
		++lowerIter;
		++upperIter;
		++prefixIter;
	}
	return MojErrNone;
}

MojErr MojDbQueryPlan::rangesFromKeys(MojDbKey lowerKey, MojDbKey upperKey, MojDbKey prefix, MojUInt32 index,
		const MojDbQuery::WhereClause* clause)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	MojUInt32 group = 0;
	MojDbQuery::CompOp lowerOp = MojDbQuery::OpEq;
	MojDbQuery::CompOp upperOp = MojDbQuery::OpNone;
	if (clause) {
		lowerOp = clause->lowerOp();
		upperOp = clause->upperOp();
	}

	// set up upper bound
	switch (upperOp) {
	case MojDbQuery::OpNone:
		MojAssert(lowerOp != MojDbQuery::OpNone);
		upperKey = prefix;
		// no break. fall through to OpLessThanEq case

	case MojDbQuery::OpLessThanEq:
		// match while less-than ++upperKey
		err = upperKey.increment();
		MojErrCheck(err);
		break;

	default:
		MojAssert(upperOp == MojDbQuery::OpLessThan);
		break;
	}

	// set up lower bound
	switch (lowerOp) {
	case MojDbQuery::OpNone:
		// seek to prefix and match while less than upperKey
		MojAssert(upperOp != MojDbQuery::OpNone);
		err = addRange(lowerKey, upperKey);
		MojErrCheck(err);
		break;

	case MojDbQuery::OpSearch:
		group = index % m_groupCount;
		// no break. fall through to OpPrefix case

	case MojDbQuery::OpPrefix:
		// remove null terminator
		MojAssert(!prefix.empty());
		if (prefix.byteVec().back() == 0) {
			err = prefix.byteVec().pop();
			MojErrCheck(err);
		}
		// no break. fall through to OpEq case

	case MojDbQuery::OpEq:
		// seek to lowerKey and match while less than ++prefix
		err = prefix.increment();
		MojErrCheck(err);
		err = addRange(lowerKey, prefix, group);
		MojErrCheck(err);
		break;

	case MojDbQuery::OpNotEq:
		// seek to prefix and match while less than lowerKey
		err = addRange(prefix, lowerKey);
		MojErrCheck(err);
		// seek to ++lowerKey, and match while less than ++prefix
		err = lowerKey.increment();
		MojErrCheck(err);
		err = prefix.increment();
		MojErrCheck(err);
		err = addRange(lowerKey, prefix);
		MojErrCheck(err);
		break;

	case MojDbQuery::OpGreaterThan:
		// seek to ++lowerKey and match while less than upperKey
		err = lowerKey.increment();
		MojErrCheck(err);
		// no break. fall through to OpGreaterThanEq case

	case MojDbQuery::OpGreaterThanEq:
		// seek to lowerKey and match while less than upperKey
		err = addRange(lowerKey, upperKey);
		MojErrCheck(err);
		break;

	default:
		MojAssertNotReached();
		break;
	}
	return MojErrNone;
}

MojErr MojDbQueryPlan::addRange(const MojDbKey& lowerKey, const MojDbKey& upperKey, MojUInt32 group)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojDbKeyRange range(lowerKey, upperKey, group);
	// tweak range according to page
	const MojDbKey& pageKey = m_query.page().key();
	bool desc = m_query.desc();
	if (!pageKey.empty()) {
		// check to see if range is entirely before current page
		const MojDbKey& ub = range.key(!desc);
		if (!ub.empty() && (pageKey.compare(ub) <= 0) == desc) {
			// range is before current page, so discard it
			return MojErrNone;
		}
		// check to see if current page is contained in range
		const MojDbKey& lb = range.key(desc);
		if (lb.empty() || (pageKey.compare(lb) < 0) == desc) {
			// modify bound to start at page
			range.key(desc) = pageKey;
		}
	}
	// don't add range if it falls entirely inside previous range. this can happen
	// when searching on two prefixes, one of which is a prefix of the other
	if (!m_ranges.empty() && m_ranges.back().contains(range)) {
		return MojErrNone;
	}
	MojErr err = m_ranges.push(range);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryPlan::pushSearch(MojDbKeyBuilder& lowerBuilder, MojDbKeyBuilder& upperBuilder,
		const MojObject& val, MojDbTextCollator* collator)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	// get text
	MojString text;
	MojErr err = val.stringValue(text);
	MojErrCheck(err);
	MojDbKeyBuilder::KeySet toks;

	// tokenize
	MojRefCountedPtr<MojDbTextTokenizer> tokenizer(new MojDbTextTokenizer);
	MojAllocCheck(tokenizer.get());
	err = tokenizer->init(m_locale);
	MojErrCheck(err);
	err = tokenizer->tokenize(text, collator, toks);
	MojErrCheck(err);

	// remove prefixes
	MojDbKeyBuilder::KeySet::Iterator i;
	err = toks.begin(i);
	MojErrCheck(err);
	MojDbKeyBuilder::KeySet::ConstIterator prev = toks.end();
	while (i != toks.end()) {
		if (prev != toks.end() && prev->stringPrefixOf(*i)) {
			bool found = false;
			err = toks.del(*prev, found);
			MojErrCheck(err);
			MojAssert(found);
		}
		prev = i;
		++i;
	}

	// push toks
	err = lowerBuilder.push(toks);
	MojErrCheck(err);
	err = upperBuilder.push(toks);
	MojErrCheck(err);
	m_groupCount = (MojUInt32) toks.size();

	return MojErrNone;
}

MojErr MojDbQueryPlan::pushVal(MojDbKeyBuilder& builder, const MojObject& val, MojDbTextCollator* collator)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;
	MojDbKey key;
	MojDbKeyBuilder::KeySet keys;
	if (val.type() == MojObject::TypeArray) {
		MojObject::ConstArrayIterator end = val.arrayEnd();
		for (MojObject::ConstArrayIterator i = val.arrayBegin(); i != end; ++i) {
			err = key.assign(*i, collator);
			MojErrCheck(err);
			err = keys.put(key);
			MojErrCheck(err);
		}
	} else {
		err = key.assign(val, collator);
		MojErrCheck(err);
		err = keys.put(key);
		MojErrCheck(err);
	}
	err = builder.push(keys);
	MojErrCheck(err);

	return MojErrNone;
}

