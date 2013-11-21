/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "db/MojDbKindState.h"
#include "db/MojDbStorageEngine.h"
#include "db/MojDbReq.h"
#include "core/MojObjectSerialization.h"

const MojChar* const MojDbKindState::IndexIdsKey = _T("indexIds");
const MojChar* const MojDbKindState::KindTokensKey = _T("kindTokens");
const MojChar* const MojDbKindState::TokensKey = _T("tokens");

MojDbKindState::MojDbKindState(const MojString& kindId, MojDbKindEngine* kindEngine)
: m_kindId(kindId),
  m_kindToken(MojInt64Max),
  m_nextToken(MojObjectWriter::TokenStartMarker),
  m_kindEngine(kindEngine)
{
}

MojErr MojDbKindState::init(const StringSet& strings, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojAssert(m_kindEngine);
	MojThreadGuard guard(m_lock);

	// get kind token
	MojErr err = initKindToken(req);
	MojErrCheck(err);
	// TODO: and go inside this bugging function
	err = initTokens(req, strings);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::indexId(const MojChar* indexName, MojDbReq& req, MojObject& idOut, bool& createdOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(indexName);

	MojThreadGuard guard(m_lock);

	MojErr err = id(indexName, IndexIdsKey, req, idOut, createdOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::delIndex(const MojChar* indexName, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(indexName);
	MojThreadGuard guard(m_lock);

	MojObject obj;
	MojRefCountedPtr<MojDbStorageItem> item;
	MojErr err = readIds(IndexIdsKey, req, obj, item);
	MojErrCheck(err);
	bool found = false;
	err = obj.del(indexName, found);
    MojErrCheck(err);
	MojAssert(found);
	err = writeIds(IndexIdsKey, obj, req, item);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::tokenSet(TokenVec& vecOut, MojObject& tokensObjOut) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojThreadGuard guard (m_lock);

	vecOut = m_tokenVec;
	tokensObjOut = m_tokensObj;

	return MojErrNone;
}

MojErr MojDbKindState::addToken(const MojChar* propName, MojUInt8& tokenOut, TokenVec& vecOut, MojObject& tokenObjOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(propName);

	MojThreadGuard guard(m_lock);

	MojErr err = addPropImpl(propName, true, tokenOut, vecOut, tokenObjOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::addPropImpl(const MojChar* propName, bool write, MojUInt8& tokenOut, TokenVec& vecOut, MojObject& tokenObjOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(propName);
	MojAssertMutexLocked(m_lock);

	tokenOut = MojTokenSet::InvalidToken;

	// check if we've already added the prop
	MojUInt32 token;
	bool found = false;
	MojErr err = m_tokensObj.get(propName, token, found);
	MojErrCheck(err);
	if (found) {
		MojAssert(token <= MojUInt8Max);
		tokenOut = (MojUInt8) token;
		return MojErrNone;
	}
	// update the db and our in-memory state
	if (m_nextToken < MojUInt8Max) {
		// update copies of obj and vec
		MojObject obj(m_tokensObj);
		TokenVec tokenVec(m_tokenVec);
		MojString prop;
		err = prop.assign(propName);
		MojErrCheck(err);
		err = tokenVec.push(prop);
		MojErrCheck(err);
		err = obj.put(propName, m_nextToken);
		MojErrCheck(err);
		// write object
		if (write) {
			err = writeTokens(obj);
			MojErrCheck(err);
		}
		// update state and return values
		m_tokensObj = obj;
		m_tokenVec.swap(tokenVec);
		tokenOut = m_nextToken++;
		vecOut = m_tokenVec;
		tokenObjOut = m_tokensObj;
	}
	return MojErrNone;
}

MojErr MojDbKindState::initKindToken(MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertMutexLocked(m_lock);

	// load tokens
	MojObject token;
	bool created = false;
	MojErr err = id(m_kindId, KindTokensKey, req, token, created);
	MojErrCheck(err);
	m_kindToken = token.intValue();

	return MojErrNone;
}


MojErr MojDbKindState::initTokens(MojDbReq& req, const StringSet& strings)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	// TODO: bug inside this function. (latest strace step)
	MojAssertMutexLocked(m_lock);

	// TODO: filing load tokens. Go inside readObj
	// load tokens
	MojErr err = readObj(TokensKey, m_tokensObj, m_kindEngine->kindDb(), req.txn(), m_oldTokensItem);
	MojErrCheck(err);

	// populate token vec
	MojUInt8 maxToken = 0;
	err = m_tokenVec.resize(m_tokensObj.size());
	MojErrCheck(err);
	for (MojObject::ConstIterator i = m_tokensObj.begin(); i != m_tokensObj.end(); ++i) {
		MojString key = i.key();
		MojInt64 value = i.value().intValue();
		MojSize idx = (MojSize) (value - MojObjectWriter::TokenStartMarker);
		if (value < MojObjectWriter::TokenStartMarker || value >= MojUInt8Max || idx >= m_tokenVec.size()) {
			MojErrThrow(MojErrDbInvalidToken);
		}
		if (value > maxToken) {
			maxToken = (MojUInt8) value;
		}
		err = m_tokenVec.setAt(idx, key);
		MojErrCheck(err);
	}
	if (maxToken > 0) {
		m_nextToken = (MojUInt8) (maxToken + 1);
	}

	// add strings
	bool updated = false;
	for (StringSet::ConstIterator i = strings.begin(); i != strings.end(); ++i) {
		if (!m_tokensObj.contains(*i)) {
			updated = true;
			MojUInt8 token = 0;
			TokenVec tokenVec;
			MojObject tokenObj;
			err = addPropImpl(*i, false, token, tokenVec, tokenObj);
			MojErrCheck(err);
		}
	}
	if (updated) {
		err = writeTokens(m_tokensObj);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKindState::id(const MojChar* name, const MojChar* objKey, MojDbReq& req, MojObject& idOut, bool& createdOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(name && objKey);
	MojAssertMutexLocked(m_lock);

	createdOut = false;

	// reload object each time because last update may have been rolled back
	MojObject obj;
	MojRefCountedPtr<MojDbStorageItem> item;
	MojErr err = readIds(objKey, req, obj, item);
	MojErrCheck(err);

	if (!obj.get(name, idOut)) {
		// get next id from seq
		MojDbStorageSeq* seq = m_kindEngine->indexSeq();
		MojAssert(seq);
		MojInt64 id = 0;
		MojErr err = seq->get(id);
		MojErrCheck(err);
		// update copy of id map and write it out
		err = obj.put(name, id);
		MojErrCheck(err);
		err = writeIds(objKey, obj, req, item);
		MojErrCheck(err);
		// update retval
		idOut = id;
		createdOut = true;
	}
	return MojErrNone;
}

MojErr MojDbKindState::readIds(const MojChar* key, MojDbReq& req, MojObject& objOut, MojRefCountedPtr<MojDbStorageItem>& itemOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojErr err = readObj(key, objOut, m_kindEngine->indexIdDb(), req.txn(), itemOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::writeIds(const MojChar* key, const MojObject& obj, MojDbReq& req, MojRefCountedPtr<MojDbStorageItem>& oldItem)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojErr err = writeObj(key, obj, m_kindEngine->indexIdDb(), req.txn(), oldItem);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::writeTokens(const MojObject& tokensObj)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(m_kindEngine);
	MojAssertMutexLocked(m_lock);

	MojDbStorageDatabase* db = m_kindEngine->kindDb();
	MojRefCountedPtr<MojDbStorageTxn> txn;
	MojErr err = db->beginTxn(txn);
	MojErrCheck(err);
	MojAssert(txn.get());
	err = writeObj(TokensKey, tokensObj, db, txn.get(), m_oldTokensItem);
	MojErrCheck(err);
	err = txn->commit();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKindState::writeObj(const MojChar* key, const MojObject& val, MojDbStorageDatabase* db,
		MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageItem>& oldItem)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(key && db && txn);
	MojAssertMutexLocked(m_lock);

	// reconstitute obj
	MojObject obj;
	if (oldItem.get()) {
		MojErr err = oldItem->toObject(obj, *m_kindEngine, false);
		MojErrCheck(err);
	}
	MojErr err = obj.put(key, val);
	MojErrCheck(err);

	// we directly use the storage engine apis because the higher-level apis depend on the kind state.
	MojBuffer buf;
	err = obj.toBytes(buf);
	MojErrCheck(err);

	txn->quotaEnabled(false);
	if (oldItem.get()) {
		err = db->update(m_kindId, buf, oldItem.get(), txn);
		MojErrCheck(err);
	} else {
		err = db->insert(m_kindId, buf, txn);
		MojErrCheck(err);
		// get old item so we can update it next time
		err = db->get(m_kindId, txn, false, oldItem);
                MojAssert(oldItem.get());
		MojErrCheck(err);
	}
	txn->quotaEnabled(true);

	return MojErrNone;
}

MojErr MojDbKindState::readObj(const MojChar* key, MojObject& val, MojDbStorageDatabase* db,
		MojDbStorageTxn* txn, MojRefCountedPtr<MojDbStorageItem>& oldItem)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(key && db);

	MojErr err = db->get(m_kindId, txn, false, oldItem);
	MojErrCheck(err);
	if (oldItem.get()) {
		// get objects
		MojObject obj;

		// TODO: this bugging ( on calling getObj)
		err = oldItem->toObject(obj, *m_kindEngine, false);
		MojErrCheck(err);
		obj.get(key, val);
	}
	return MojErrNone;
}
