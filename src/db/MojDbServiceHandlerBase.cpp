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


#include "db/MojDbServiceHandlerBase.h"
#include "db/MojDbServiceDefs.h"
#include "core/MojReactor.h"
#include "core/MojTime.h"
#include "core/MojJson.h"

//db.serviceHandler

MojDbServiceHandlerBase::MojDbServiceHandlerBase(MojDb& db, MojReactor& reactor)
: m_db(db),
  m_reactor(reactor)
{
}

MojErr MojDbServiceHandlerBase::invoke(Callback method, MojServiceMessage* msg, MojObject& payload)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(method && msg);

	MojUInt32 retries = 0;
	for (;;) {
		MojErr err = invokeImpl(method, msg, payload);
		MojErrCatch(err, MojErrDbFatal) {
			LOG_CRITICAL(MSGID_DB_SERVICE_ERROR, 0, "db: fatal error; shutting down");
			m_reactor.stop();
			MojErrThrow(MojErrDbFatal);
		}
		MojErrCatch(err, MojErrDbDeadlock) {
			if (++retries >= MaxDeadlockRetries) {
                LOG_ERROR(MSGID_DB_SERVICE_ERROR, 0, "db: deadlock detected; max retries exceeded");
				MojErrThrow(MojErrDbMaxRetriesExceeded);
			}

			LOG_WARNING(MSGID_MOJ_DB_SERVICE_WARNING, 1,
					PMLOGFV("retries", "%d", retries),
					"db: deadlock detected; attempting retry");
			err = msg->writer().reset();
			MojErrCheck(err);
			err = MojSleep(DeadlockSleepMillis * 1000);
			MojErrCheck(err);
			continue;
		}
		MojErrCatch(err, MojErrInternalIndexOnDel) {
			if (++retries >= MaxIndexlockRetries) {
                LOG_ERROR(MSGID_DB_SERVICE_ERROR, 0, "db: indexlock_warning max retries exceeded");
				MojErrThrow(MojErrDbInconsistentIndex);
			}

			LOG_WARNING(MSGID_MOJ_DB_SERVICE_WARNING, 1,
					PMLOGFV("retries", "%d", retries),
					"db: indexlock_conflict; attempting retry");
			err = msg->writer().reset();
			MojErrCheck(err);
			err = MojSleep(DeadlockSleepMillis * 1000);
			MojErrCheck(err);
			continue;
		}
		MojErrCheck(err);
		break;
	}
	return MojErrNone;
}

MojErr MojDbServiceHandlerBase::invokeImpl(Callback method, MojServiceMessage* msg, MojObject& payload)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojDbReq req(false);
	MojString errStr;
	MojString payloadstr;
		
	MojErr err = req.domain(msg->senderName());
	MojErrCheck(err);

	req.beginBatch();
	err = (this->*((DbCallback) method))(msg, payload, req);
	(void) MojErrToString(err, errStr);
	(void) payload.toJson(payloadstr);
    LOG_DEBUG("[db_mojodb] db_method: %s, err: (%d) - %s; sender= %s;\n payload=%s; \n response= %s\n",
        msg->method(), (int)err, errStr.data(), msg->senderName(), payloadstr.data(), ((MojJsonWriter&)(msg->writer())).json().data());

	MojErrCheck(err);
	err = req.endBatch();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerBase::formatCount(MojServiceMessage* msg, MojUInt32 count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(msg);

	MojObjectVisitor& writer = msg->writer();
	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.intProp(MojDbServiceDefs::CountKey, count);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerBase::formatPut(MojServiceMessage* msg, const MojObject* begin, const MojObject* end)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(msg);

	MojObjectVisitor& writer = msg->writer();
	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.boolProp(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::ResultsKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);
	
	for (const MojObject* i = begin; i != end;  ++i) {
		bool ignoreId = false;
		if (i->get(MojDb::IgnoreIdKey, ignoreId) && ignoreId)
			continue;
		err = formatPutAppend(writer, *i);
		MojErrCheck(err);
	}
	err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerBase::formatPutAppend(MojObjectVisitor& writer, const MojObject& result)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObject id;
	MojErr err = result.getRequired(MojDb::IdKey, id);
	MojErrCheck(err);
	MojObject rev;

	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.objectProp(MojDbServiceDefs::IdKey, id);
	MojErrCheck(err);
	if (result.get(MojDb::RevKey, rev)) {
		err = writer.objectProp(MojDbServiceDefs::RevKey, rev);
		MojErrCheck(err);
	}
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}
