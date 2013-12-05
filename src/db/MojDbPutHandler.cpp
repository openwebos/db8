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


#include "db/MojDbPutHandler.h"
#include "db/MojDb.h"

//db.putHandler

MojDbPutHandler::MojDbPutHandler(const MojChar* kindId, const MojChar* confProp)
: m_kindId(kindId),
  m_confProp(confProp),
  m_db(NULL)
{
}

MojDbPutHandler::~MojDbPutHandler()
{
}

MojErr MojDbPutHandler::open(const MojObject& conf, MojDb* db, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertWriteLocked(db->schemaLock());

	m_db = db;
	// configure
	MojErr err = configure(conf, req);
	MojErrCheck(err);
	// load permissions from db
	MojDbQuery query;
	err = query.from(m_kindId);
	MojErrCheck(err);
	MojDbCursor cursor;
	err = db->find(query, cursor, req);
	MojErrCheck(err);
	for (;;) {
		bool found = false;
		MojObject perm;
		err = cursor.get(perm, found);
		MojErrCheck(err);
		if (!found)
			break;
		err = put(perm, req, false);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbPutHandler::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	m_db = NULL;

	return MojErrNone;
}

MojErr MojDbPutHandler::configure(const MojObject& conf, MojDbReq& req)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssertWriteLocked(m_db->schemaLock());

	// built-in permissions
	MojObject objects;
	if (conf.get(m_confProp, objects)) {
		MojObject::ConstArrayIterator end = objects.arrayEnd();
		for (MojObject::ConstArrayIterator i = objects.arrayBegin();
			 i != end; ++i) {
			MojObject perm(*i);
			MojErr err = put(perm, req, false);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojDbPutHandler::validateWildcard(const MojString& val, MojErr errToThrow)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (val.empty())
		MojErrThrow(errToThrow);
	MojSize wildcardPos = val.find(_T('*'));
	if (wildcardPos != MojInvalidSize) {
		if (wildcardPos != val.length() - 1 ||
			(wildcardPos != 0 && val.at(wildcardPos - 1) != _T('.'))) {
			MojErrThrowMsg(errToThrow, _T("db: invalid wildcard in - '%s'"), val.data());
		}
	}
	return MojErrNone;
}

bool MojDbPutHandler::matchWildcard(const MojString& expr, const MojChar* val, MojSize len)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (expr.last() == _T('*') &&
		len >= (expr.length() - 1) &&
		MojStrNCmp(expr, val, expr.length() - 1) == 0) {
		return true;
	}
	return false;
}
