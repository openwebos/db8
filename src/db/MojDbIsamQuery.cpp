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


#include "db/MojDbIsamQuery.h"
#include "db/MojDbQueryPlan.h"
#include "db/MojDb.h"
#include "db/MojDbCursor.h"
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
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojAssert(!m_isOpen);
	MojAssert(plan.get() && txn);

	m_isOpen = true;
	m_plan = plan;
	m_iter = m_plan->ranges().begin();
	m_state = StateSeek;
	m_txn = txn;
	m_endKey.clear();
    m_distinct = m_plan->query().distinct();
    m_ignoreInactiveShards = m_plan->query().ignoreInactiveShards();

	return MojErrNone;
}

MojErr MojDbIsamQuery::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = MojErrNone;

	if (m_isOpen) {
		m_plan.reset();
		init();
	}
	m_lastItem.reset();

	return err;
}

MojErr MojDbIsamQuery::get(MojDbStorageItem*& itemOut, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
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
    LOG_DEBUG("[db_mojodb] isamquery_get: found: %d; count1= %d; count2= %d\n", (int)foundOut, i1, i2);
	return MojErrNone;
}

MojErr MojDbIsamQuery::getId(MojObject& idOut, MojUInt32& groupOut, bool& foundOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

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
    LOG_TRACE("Entering function %s", __FUNCTION__);
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
        LOG_DEBUG("[db_mojodb] isamquery_count: from: %s; indexid: %zu; warnings: %d \n",
            from, m_plan->idIndex(), warns);
	}
	return MojErrNone;
}

MojErr MojDbIsamQuery::nextPage(MojDbQuery::Page& pageOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (limitEnforced()) {
		pageOut.key() = m_endKey;
	} else {
		pageOut.clear();
	}
	return MojErrNone;
}

MojUInt32 MojDbIsamQuery::groupCount() const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return m_plan->groupCount();
}

MojErr MojDbIsamQuery::distinct(MojDbStorageItem*& itemOut, bool& distinct)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojObject obj, destObj;
    MojErr err = itemOut->toObject(obj, m_plan->kindEngine());
    MojErrCheck(err);
    err = obj.getRequired(m_distinct, destObj);
    MojErrCheck(err);
    distinct = false;

    //if there is kept last element, check whether current element is duplicated with it
    if(m_lastItem.get())
    {
        MojObject compObj, distinctObj;
	    err = m_lastItem->toObject(compObj, m_plan->kindEngine());
        MojErrCheck(err);
        err = compObj.getRequired(m_distinct, distinctObj);
        MojErrCheck(err);
        if(!distinctObj.compare(destObj))
        {
            //In this case, current element is duplicated, so need to skip.
            distinct = true;
            return MojErrNone;
	    }
    }
    //keep the non-duplicated element
    m_lastItem.reset(new MojDbObjectItem(obj));
    itemOut = m_lastItem.get();

    return MojErrNone;
}

MojErr MojDbIsamQuery::getImpl(MojDbStorageItem*& itemOut, bool& foundOut, bool getItem)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	itemOut = NULL;
	MojUInt32 group = 0;
	MojErr err = getKey(group, foundOut);
	MojErrCheck(err);
	if (foundOut && getItem) {
		err = getVal(itemOut, foundOut);
		if (err == MojErrInternalIndexOnFind) {
#if defined (MOJ_DEBUG)
			char s[1024];
			char *s2 = NULL;
			MojErr err2 =  MojByteArrayToHex(m_keyData, m_keySize, s);
			MojErrCheck(err2);
			if (m_keySize > 17)
				s2 = ((char *)m_keyData) + m_keySize - 17;
			MojSize idIndex = m_plan->idIndex();
			const MojChar * from = m_plan->query().from().data();
			LOG_DEBUG("[db_mojodb] isamquery_warnindex: from: %s; indexid: %zu; group: %d; KeySize: %zu; %s ;id: %s \n",
				from, idIndex, (int)group, m_keySize, s, (s2?s2:"NULL"));
#endif
		}
		MojErrCheck(err);
	}
	if (foundOut) {
        //If distinct query is, We need to check that field is duplicated or not
        //In case of duplication, count will not incremented,
        //and set "itemOut" to NULL for getting next DB result.
        if(!m_distinct.empty() && itemOut)
        {
            bool distincted = false;
            err = distinct(itemOut, distincted);
            MojErrCheck(err);
            if(!distincted)
                incrementCount();
            else
                itemOut = NULL;
        }
        else
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
    LOG_TRACE("Entering function %s", __FUNCTION__);

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
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return MojLexicalCompare(m_keyData, m_keySize, key.begin(), key.size());
}

MojErr MojDbIsamQuery::incrementCount()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

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
    LOG_TRACE("Entering function %s", __FUNCTION__);
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
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(m_isOpen);

	foundOut = false;
	MojErr err = MojErrNone;
	RangeVec::ConstIterator end = m_plan->ranges().end();
    uint32_t countFound = 0;
    uint32_t countIgnored = 0;

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
            // XXX: virtual shards (will be removed later)
#if defined (MOJ_DEBUG)
             countFound++;
#endif
            if (m_ignoreInactiveShards) {
                bool exclude;
                err = checkShard(exclude);
                MojErrCheck( err );
                if (exclude) {
                    // skip excluded records
                    // this actually will loop through next one/several records
                    // once match() ends we will jump to next range
                    m_state = StateNext;
#if defined (MOJ_DEBUG)
                    countIgnored++;
#endif
                    continue;
                }
            }

			groupOut = m_iter->group();
			foundOut = true;
			break;
		}
		// key fell outside of range, so move on to next range
		m_state = StateSeek;
		++m_iter;
	}
    LOG_DEBUG("[db_mojodb] Matching records found: %d, ignore: %d", countFound, countIgnored);
	return MojErrNone;
}

MojErr MojDbIsamQuery::saveEndKey()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

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
    LOG_TRACE("Entering function %s", __FUNCTION__);

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
    LOG_TRACE("Entering function %s", __FUNCTION__);

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

MojErr MojDbIsamQuery::checkShard(bool &excludeOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    excludeOut = false;

    MojDb *db = m_plan->kindEngine().db();
    MojDbShardEngine* shardEngine = db->shardEngine();
    MojErr err;

    // extract shard id
    MojObject id;
    err = parseId(id);
    MojErrCheck( err );
#ifdef MOJ_DEBUG
    {
        // dump id as JSON
        MojString idJson;
        err = id.toJson(idJson);
        MojErrCheck( err );
        LOG_DEBUG("[db_mojodb] id = %.*s\n", idJson.length(), idJson.begin());
    }
#endif

    MojUInt32 shardId;
    // Note: we should work-around pre-shared _id (those that are not
    //       generated by db8)
    if (G_UNLIKELY(id.type() != MojObject::TypeString))
    {
        // non-string _id ... Probably tests or nasty client
        shardId = MojDbIdGenerator::MainShardId;
    }
    else
    {
        err = MojDbIdGenerator::extractShard(id, shardId);
        MojErrCatch( err, MojErrInvalidBase64Data ) {
            // Looks like someone used string id, but with some custom
            // contents. Yet another nasty client...
            MojString idJson;
            err = id.toJson(idJson);
            MojErrCheck( err );

            LOG_DEBUG("[db_mojodb] Found id (%.*s) with a non-existing shardId (%08x)\n", idJson.length(), idJson.begin(), shardId);

            shardId = MojDbIdGenerator::MainShardId;
        }

        if (G_UNLIKELY(shardId != MojDbIdGenerator::MainShardId))
        {
            bool found;
            // Note: Even if we'll be able to extract some non main shard
            //       id there is a still chance that some nasty client used
            //       96 bits encoded in base64. Or more correctly used
            //       string that can be decoded from base64 into 96 bits.
            MojDbShardEngine::ShardInfo shardInfo;

            // Note: shardEngine should keep ShardInfo in memory to keep
            //       this reasonable.
            err = shardEngine->get(shardId, shardInfo, found);
            MojErrCheck(err);

            // Hope this is actually an shard id. But there is
            // still exists a chance...
            if (G_LIKELY(found))
            {
                // main shard is always active
                if (!shardInfo.active) {
                    excludeOut = true;
                }
            }
            else
            {
                // Arghh... Who did that?!

                MojString idJson;
                err = id.toJson(idJson);
                MojErrCheck( err );

                LOG_DEBUG("[db_mojodb] Found id (%.*s) with a non-existing shardId (%08x)\n", idJson.length(), idJson.begin(), shardId);
                shardId = MojDbIdGenerator::MainShardId; // ignore it and use main shard
            }
        }
    }

    return MojErrNone;
}
