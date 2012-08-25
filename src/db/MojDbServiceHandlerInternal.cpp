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

#include "db/MojDbServiceHandler.h"
#include "db/MojDbServiceDefs.h"
#include "db/MojDbServiceHandlerInternal.h"
#include "db/MojDbReq.h"
#include "core/MojServiceRequest.h"
#include "core/MojTime.h"

#include <sys/statvfs.h>

const MojDbServiceHandlerInternal::Method MojDbServiceHandlerInternal::s_privMethods[] = {
	{MojDbServiceDefs::PostBackupMethod, (Callback) &MojDbServiceHandlerInternal::handlePostBackup},
	{MojDbServiceDefs::PostRestoreMethod, (Callback) &MojDbServiceHandlerInternal::handlePostRestore},
	{MojDbServiceDefs::PreBackupMethod, (Callback) &MojDbServiceHandlerInternal::handlePreBackup},
	{MojDbServiceDefs::PreRestoreMethod, (Callback) &MojDbServiceHandlerInternal::handlePreRestore},
	{MojDbServiceDefs::ScheduledPurgeMethod, (Callback) &MojDbServiceHandlerInternal::handleScheduledPurge},
	{MojDbServiceDefs::SpaceCheckMethod, (Callback) &MojDbServiceHandlerInternal::handleSpaceCheck },
	{MojDbServiceDefs::ScheduledSpaceCheckMethod, (Callback) &MojDbServiceHandlerInternal::handleScheduledSpaceCheck },
	{NULL, NULL} };

const MojChar* const MojDbServiceHandlerInternal::PostBackupMethod = _T("internal/postBackup");
const MojChar* const MojDbServiceHandlerInternal::PostRestoreMethod = _T("internal/postRestore");
const MojChar* const MojDbServiceHandlerInternal::PreBackupMethod = _T("internal/preBackup");
const MojChar* const MojDbServiceHandlerInternal::PreRestoreMethod = _T("internal/preRestore");
const MojChar* const MojDbServiceHandlerInternal::ScheduledPurgeMethod = _T("internal/scheduledPurge");
const MojChar* const MojDbServiceHandlerInternal::SpaceCheckMethod = _T("internal/spaceCheck");
const MojChar* const MojDbServiceHandlerInternal::ScheduledSpaceCheckMethod = _T("internal/scheduledSpaceCheck");

const MojInt32 MojDbServiceHandlerInternal::SpaceCheckInterval = 60;
const MojChar* const MojDbServiceHandlerInternal::DatabaseRoot = "/var/db";
const MojDouble MojDbServiceHandlerInternal::SpaceAlertLevels[] = { 85.0, 90.0, 95.0 };
const MojChar* const MojDbServiceHandlerInternal::SpaceAlertNames[] = {"none",  "low", "medium", "high" };
const MojInt32 MojDbServiceHandlerInternal::NumSpaceAlertLevels =
	sizeof(MojDbServiceHandlerInternal::SpaceAlertLevels) / sizeof(MojDouble);

MojDbServiceHandlerInternal::AlertLevel MojDbServiceHandlerInternal::m_spaceAlertLevel = NeverSentSpaceAlert;
bool MojDbServiceHandlerInternal::m_compactRunning = false;


MojDbServiceHandlerInternal::MojDbServiceHandlerInternal(MojDb& db, MojReactor& reactor, MojService& service)
: MojDbServiceHandlerBase(db, reactor),
  m_spaceCheckTimer(NULL),
  m_service(service)
{
}

MojErr MojDbServiceHandlerInternal::open()
{
	MojLogTrace(s_log);

	MojErr err = addMethods(s_privMethods, false);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::close()
{
	MojLogTrace(s_log);
	return MojErrNone;
}

MojDbServiceHandlerInternal::AlertLevel MojDbServiceHandlerInternal::spaceAlertLevel()
{
   if(m_compactRunning) // do the real space check - we have no idea how long it'll take to clean up everything
   {
      int bytesUsed = 0;
      int bytesAvailable = 0;
      AlertLevel alertLevel = AlertLevelHigh;
      doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
      return alertLevel;
   }
   return MojDbServiceHandlerInternal::m_spaceAlertLevel;
}

MojErr MojDbServiceHandlerInternal::configure(bool fatalError)
{
	MojLogTrace(s_log);

	if (fatalError) {
		MojErr err = generateFatalAlert();
		MojErrCheck(err);
	} else {
		MojErr err = requestLocale();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handlePostBackup(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	// Chance to clean up anything we might have done during backup.
	// Don't need to delete the backup file - BackupService handles that
	MojErr err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handlePostRestore(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojString dir;
	MojErr err = payload.getRequired(MojDbServiceDefs::DirKey, dir);
	MojErrCheck(err);

	MojObject files;
	err = payload.getRequired(MojDbServiceDefs::FilesKey, files);
	MojErrCheck(err);

	// load each file, in order
	for (MojObject::ConstArrayIterator i = files.arrayBegin(); i != files.arrayEnd(); ++i) {
		MojUInt32 count = 0;
		MojString fileName;
		err = i->stringValue(fileName);
		MojErrCheck(err);
		MojString path;
		err = path.format(_T("%s/%s"), dir.data(), fileName.data());
		MojErrCheck(err);
		err = m_db.load(path, count, MojDb::FlagForce, req);
		MojErrCheck(err);
	}

	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handlePreBackup(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojString dir;
	err = payload.getRequired(MojDbServiceDefs::DirKey, dir);
	MojErrCheck(err);
	MojObject dumpResponse;

	MojObject backupParams;
	MojUInt32 bytes = 0;
	err = payload.getRequired(MojDbServiceDefs::BytesKey, bytes);
	MojErrCheck(err);
	MojObject incrementalKey;
	payload.get(MojDbServiceDefs::IncrementalKey, incrementalKey);

	MojTime curTime;
	err = MojGetCurrentTime(curTime);
	MojErrCheck(err);
	MojString backupFileName;
	err = backupFileName.format(_T("%s-%llu.%s"), _T("backup"), curTime.microsecs(), _T("json"));
	MojErrCheck(err);

	MojUInt32 count = 0;
	MojString backupPath;
	err = backupPath.format(_T("%s/%s"), dir.data(), backupFileName.data());
	MojErrCheck(err);
	err = m_db.dump(backupPath, count, true, req, true, bytes, incrementalKey.empty() ? NULL : &incrementalKey, &dumpResponse);
	MojErrCheck(err);

	err = dumpResponse.put(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	MojObject files(MojObject::TypeArray);
	if (count > 0) {
		err = files.pushString(backupFileName);
		MojErrCheck(err);
	}
	err = dumpResponse.put(MojDbServiceDefs::FilesKey, files);
	MojErrCheck(err);
	err = msg->reply(dumpResponse);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handlePreRestore(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojString version;
	bool found = false;
	bool proceed = true;
	MojErr err = payload.get(MojDbServiceDefs::VersionKey, version, found);
	MojErrCheck(err);
	if (found) {
		MojObject versionObj;
		err = versionObj.fromJson(version);
		MojErrCheck(err);
		if (versionObj > m_db.version()) {
			proceed = false;
		}
	}

	MojObject response;
	err = response.put(MojDbServiceDefs::ProceedKey, proceed);
	MojErrCheck(err);
	err = response.put(MojServiceMessage::ReturnValueKey, true);
	MojErrCheck(err);
	err = msg->reply(response);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handleScheduledPurge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject activity;
	MojErr err = payload.getRequired(_T("$activity"), activity);
	MojErrCheck(err);
	MojObject activityId;
	err = activity.getRequired(_T("activityId"), activityId);
	MojErrCheck(err);

	MojRefCountedPtr<PurgeHandler> handler(new PurgeHandler(this, activityId));
	MojAllocCheck(handler.get());
	err = handler->init();
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handleScheduledSpaceCheck(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojObject activity;
	MojErr err = payload.getRequired(_T("$activity"), activity);
	MojErrCheck(err);
	MojObject activityId;
	err = activity.getRequired(_T("activityId"), activityId);
	MojErrCheck(err);

	MojRefCountedPtr<PurgeHandler> handler(new PurgeHandler(this, activityId, true));
	MojAllocCheck(handler.get());
	err = handler->init();
	MojErrCheck(err);
	err = msg->replySuccess();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::handleSpaceCheck(MojServiceMessage* msg, MojObject& payload, MojDbReq& req)
{
	MojAssert(msg);
	MojLogTrace(s_log);

	MojErr err = MojErrNone;
	MojObject response;
	AlertLevel alertLevel;
	int bytesUsed;
	int bytesAvailable;
	bool subscribed = msg->subscribed();

	if (subscribed) {
		MojRefCountedPtr<SpaceCheckHandler> handler = new SpaceCheckHandler(this, msg);
		m_spaceCheckHandlers.push(handler);
	}

	err = doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
	MojErrCheck(err);

	err = response.putString(_T("severity"), SpaceAlertNames[alertLevel - NoSpaceAlert]);
	MojErrCheck(err);
	err = response.putInt(_T("bytesUsed"), bytesUsed);
	MojErrCheck(err);
	err = response.putInt(_T("bytesAvailable"), bytesAvailable);
	MojErrCheck(err);
		
	err = response.putBool("subscribed", subscribed);
	MojErrCheck(err);
	err = msg->reply(response);
	MojErrCheck(err);

	return err;
}


MojErr MojDbServiceHandlerInternal::requestLocale()
{
	// register with the system service for locale changes
	MojObject keys;
	MojErr err = keys.pushString(MojDbServiceDefs::LocaleKey);
	MojErrCheck(err);
	MojObject localePayload;
	err = localePayload.put(MojDbServiceDefs::KeysKey, keys);
	MojErrCheck(err);
	err = localePayload.put(MojDbServiceDefs::SubscribeKey, true);
	MojErrCheck(err);

	MojRefCountedPtr<MojServiceRequest> localeReq;
	err = m_service.createRequest(localeReq);
	MojErrCheck(err);
	m_localeChangeHandler.reset(new MojDbServiceHandlerInternal::LocaleHandler(m_db));
	MojAllocCheck(m_localeChangeHandler.get());
	err = localeReq->send(m_localeChangeHandler->m_slot, _T("com.palm.systemservice"), _T("getPreferences"), localePayload, MojServiceRequest::Unlimited);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::doSpaceCheck()
{
	AlertLevel alertLevel;
	int bytesUsed;
	int bytesAvailable;
	MojErr err = doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
	if (err != MojErrNone)
		return err;

    // first display message if needed
	if (alertLevel != m_spaceAlertLevel) {		
		MojObject message;
		MojErr err = message.putString(_T("severity"), SpaceAlertNames[alertLevel - NoSpaceAlert]);
		MojErrCheck(err);
		err = message.putInt(_T("bytesUsed"), bytesUsed);
		MojErrCheck(err);
		err = message.putInt(_T("bytesAvailable"), bytesAvailable);
		MojErrCheck(err);

		for (MojSize i = 0; i < m_spaceCheckHandlers.size(); i++)
			m_spaceCheckHandlers.at(i)->dispatchUpdate(message);
	}
    
    // if we are close to db full - let's do purge and compact
    // if alertLevel is AlertLevelLow - purge everything except last day
    // otherwise purge and compact everything
    // we do the purge/compact procedure ONLY once per each alertLevel change
    {
       MojUInt32 count = 0;
       m_compactRunning = true;
       if( (alertLevel >= AlertLevelLow) && (alertLevel != m_spaceAlertLevel) )
       {
          MojDbReq adminRequest(true);
          adminRequest.beginBatch();
          if(alertLevel > AlertLevelLow)
             m_db.purge(count, 0, adminRequest); // purge everything
          else
             m_db.purge(count, 1, adminRequest); // save last day
          adminRequest.endBatch();
       }
       
       if(count > 0) 
       {
          MojDbReq compactRequest(false);
          compactRequest.beginBatch();
          MojErr err = m_db.compact();
          MojErrCheck(err);
          compactRequest.endBatch();

          // check again
          doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
       }
       m_compactRunning = false;
    }
    
    m_spaceAlertLevel = alertLevel;

	return err;
}

MojErr MojDbServiceHandlerInternal::doSpaceCheck(MojDbServiceHandlerInternal::AlertLevel& alertLevel,
												 int& bytesUsed, int& bytesAvailable)	
{
	MojLogTrace(s_log);

	alertLevel = NoSpaceAlert;
	bytesUsed = 0;
	bytesAvailable = 0;

	struct statvfs dbFsStat;

	int ret = ::statvfs(DatabaseRoot, &dbFsStat);
	if (ret != 0) {
		MojLogError(s_log, _T("Error %d attempting to stat database "
			"filesystem mounted at \"%s\""), ret, DatabaseRoot);
		return MojErrInternal;
	}

	fsblkcnt_t blocksUsed = dbFsStat.f_blocks - dbFsStat.f_bfree;

	MojDouble percentUsed =
		((MojDouble)blocksUsed / (MojDouble)dbFsStat.f_blocks) * 100.0;

	MojLogInfo(s_log, _T("Database volume %.1f full"), percentUsed);

	int level;
	for (level = AlertLevelHigh; level > NoSpaceAlert; --level) {
		if (percentUsed >= SpaceAlertLevels[level])
			break;
	}

	alertLevel = (AlertLevel) level;

	if ((AlertLevel)alertLevel > NoSpaceAlert) {
		MojLogWarning(s_log, _T("Database volume %.1f full, generating "
								"warning, severity \"%s\""), percentUsed,
					  SpaceAlertNames[alertLevel - NoSpaceAlert]);
	} else {
		if ((AlertLevel)alertLevel != m_spaceAlertLevel) {
			// Generate 'ok' message only if there has been a transition.
			MojLogWarning(s_log, _T("Database volume %1.f full, space ok, no warning needed.\n"), percentUsed);
		}
	}

	bytesUsed = (int)(blocksUsed * dbFsStat.f_frsize);
	bytesAvailable = (int)(dbFsStat.f_blocks * dbFsStat.f_frsize);

	return MojErrNone;
}

gboolean MojDbServiceHandlerInternal::periodicSpaceCheck(gpointer data)
{
	MojDbServiceHandlerInternal* base;

	base = static_cast<MojDbServiceHandlerInternal *>(data);

	base->doSpaceCheck();

	/* Keep repeating at the given interval */
	return true;
}

MojErr MojDbServiceHandlerInternal::generateFatalAlert()
{
	MojLogTrace(s_log);

	MojObject message;
	MojErr err = message.putString(_T("errorType"), _T("fsckError"));
	MojErrCheck(err);
	err = message.putBool(_T("returnValue"), true);
	MojErrCheck(err);
	err = generateAlert(_T("showTokenError"), message);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::generateAlert(const MojChar* event, const MojObject& msg)
{
	MojLogTrace(s_log);

	MojObject payload;
	MojErr err = payload.putString(_T("event"), event);
	MojErrCheck(err);
	err = payload.put(_T("message"), msg);
	MojErrCheck(err);

	MojRefCountedPtr<AlertHandler> handler(new AlertHandler(m_service, payload));
	MojAllocCheck(handler.get());
	err = handler->send();
	MojErrCheck(err);

	return MojErrNone;
}

MojDbServiceHandlerInternal::PurgeHandler::PurgeHandler(MojDbServiceHandlerInternal* serviceHandler, const MojObject& activityId, bool doSpaceCheck)
: m_handled(false),
  m_doSpaceCheck(doSpaceCheck),
  m_activityId(activityId),
  m_serviceHandler(serviceHandler),
  m_adoptSlot(this, &PurgeHandler::handleAdopt),
  m_completeSlot(this, &PurgeHandler::handleComplete)
{
}

MojErr MojDbServiceHandlerInternal::PurgeHandler::init()
{
	MojLogTrace(s_log);

	MojObject payload;
	MojErr err = initPayload(payload);
	MojErrCheck(err);
	err = payload.putBool(_T("wait"), true);
	MojErrCheck(err);
	err = payload.putBool(_T("subscribe"), true);
	MojErrCheck(err);
	err = m_serviceHandler->m_service.createRequest(m_subscription);
	MojErrCheck(err);
	err = m_subscription->send(m_adoptSlot, _T("com.palm.activitymanager"), _T("adopt"), payload, MojServiceRequest::Unlimited);
	MojErrCheck(err);
 
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::PurgeHandler::handleAdopt(MojObject& payload, MojErr errCode)
{
	MojLogTrace(s_log);
	MojErrCheck(errCode);

	// do the purge and compact
	if (!m_handled) {
		m_handled = true;
        MojErr err = MojErrNone;
        if( m_doSpaceCheck ) 
        {
           err = m_serviceHandler->doSpaceCheck();
           MojErrCheck(err);
        }
        else
        {
           MojUInt32 count = 0;
           err = m_serviceHandler->m_db.purge(count);
           MojErrCheck(err);
           err = m_serviceHandler->m_db.compact();
           MojErrCheck(err);
        }
        
		// complete the activity
		MojObject requestPayload;
		err = initPayload(requestPayload);
		MojErrCheck(err);
		err = requestPayload.putBool(_T("restart"), true);
		MojErrCheck(err);
		MojRefCountedPtr<MojServiceRequest> req;
		err = m_serviceHandler->m_service.createRequest(req);
		MojErrCheck(err);
		err = req->send(m_completeSlot, _T("com.palm.activitymanager"), _T("complete"), requestPayload);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::PurgeHandler::handleComplete(MojObject& payload, MojErr errCode)
{
	MojLogTrace(s_log);

	// cancel subscription
	m_adoptSlot.cancel();
	m_subscription.reset();
	if (errCode != MojErrNone) {
		MojLogError(s_log, _T("error completing activity: %d"), errCode);
		MojErrThrow(errCode);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::PurgeHandler::initPayload(MojObject& payload)
{
	MojErr err = payload.put(_T("activityId"), m_activityId);
	MojErrCheck(err);
	err = payload.putString(_T("activityName"), _T("mojodb scheduled purge"));
	MojErrCheck(err);

	return MojErrNone;
}

MojDbServiceHandlerInternal::LocaleHandler::LocaleHandler(MojDb& db)
:  m_db(db),
   m_slot(this, &LocaleHandler::handleResponse)
{
}

MojErr MojDbServiceHandlerInternal::LocaleHandler::handleResponse(MojObject& payload, MojErr errCode)
{
	MojLogTrace(s_log);

	if (errCode != MojErrNone) {
		MojLogError(s_log, _T("error from system service, locale query: %d"), errCode);
		MojErrThrow(errCode);
	}

	MojObject locale;
	if (payload.get(MojDbServiceDefs::LocaleKey, locale)) {
		MojString language;
		MojErr err = locale.getRequired(MojDbServiceDefs::LanguageCodeKey, language);
		MojErrCheck(err);
		MojString country;
		err = locale.getRequired(MojDbServiceDefs::CountryCodeKey, country);
		MojErrCheck(err);
        country.toUpper();
        MojString str;
		err = str.format(_T("%s_%s"), language.data(), country.data());
		MojErrCheck(err);
        // check for Chineese
        if(str == "zh_CN")
           str.append("@collation=pinyin");
        
		err = m_db.updateLocale(str);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojDbServiceHandlerInternal::AlertHandler::AlertHandler(MojService& service, const MojObject& payload)
: m_bootStatusSlot(this, &AlertHandler::handleBootStatusResponse),
  m_alertSlot(this, &AlertHandler::handleAlertResponse),
  m_service(service),
  m_payload(payload)
{
}

MojErr MojDbServiceHandlerInternal::AlertHandler::send()
{
	MojLogTrace(s_log);

	MojObject payload;
	MojErr err = payload.put(_T("subscribe"), true);
	MojErrCheck(err);
	err = m_service.createRequest(m_subscription);
	MojErrCheck(err);
	err = m_subscription->send(m_bootStatusSlot, _T("com.palm.systemmanager"), _T("getBootStatus"), payload, MojServiceRequest::Unlimited);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::AlertHandler::handleBootStatusResponse(MojObject& payload, MojErr errCode)
{
	MojLogTrace(s_log);
	if (errCode != MojErrNone) {
		MojLogError(s_log, _T("error attempting to get sysmgr boot status %d"), errCode);
		MojErrThrow(errCode);
	}

	bool finished = false;
	MojErr err = payload.getRequired(_T("finished"), finished);
	MojErrCheck(err);
	if (finished) {
		m_subscription.reset();
		MojRefCountedPtr<MojServiceRequest> req;
		err = m_service.createRequest(req);
		MojErrCheck(err);
		err = req->send(m_alertSlot, _T("com.palm.systemmanager"), _T("publishToSystemUI"), m_payload);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbServiceHandlerInternal::AlertHandler::handleAlertResponse(MojObject& payload, MojErr errCode)
{
	MojLogTrace(s_log);
	if (errCode != MojErrNone) {
		MojLogError(s_log, _T("error attempting to display alert: %d"), errCode);
		MojErrThrow(errCode);
	}
	return MojErrNone;
}

MojDbServiceHandlerInternal::SpaceCheckHandler::SpaceCheckHandler(MojDbServiceHandlerInternal* parent, MojServiceMessage* msg)
	: m_parent(parent)
	, m_msg(msg)
	, m_cancelSlot(this, &SpaceCheckHandler::handleCancel)
{
	MojAssert(msg);
	msg->notifyCancel(m_cancelSlot);
}

MojDbServiceHandlerInternal::SpaceCheckHandler::~SpaceCheckHandler()
{
}

MojErr MojDbServiceHandlerInternal::SpaceCheckHandler::dispatchUpdate(const MojObject& message)
{
	MojErr err = MojErrNone;
	err = m_msg->reply(message);
	MojErrCheck(err);

	return err;
}

MojErr MojDbServiceHandlerInternal::SpaceCheckHandler::handleCancel(MojServiceMessage* msg)
{
	MojAssert(msg == m_msg.get());

	m_msg.reset();

	MojSize index = -1;
	for (MojSize i = 0; i < m_parent->m_spaceCheckHandlers.size(); i++) {
		if (m_parent->m_spaceCheckHandlers.at(i).get() == this) {
			index = i;
			break;
		}
	}

	MojAssert(index != -1);
	m_parent->m_spaceCheckHandlers.erase(index);
	
	return MojErrNone;
}
