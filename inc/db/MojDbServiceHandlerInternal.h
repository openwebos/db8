/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBSERVICEHANDLERINTERNAL_H_
#define MOJDBSERVICEHANDLERINTERNAL_H_

#include "db/MojDbDefs.h"
#include "db/MojDb.h"
#include "db/MojDbServiceHandlerBase.h"
#include "core/MojService.h"
#include "core/MojServiceMessage.h"
#include "core/MojServiceRequest.h"
#include "core/MojVector.h"
#include "luna/MojLunaService.h"

class MojDbServiceHandlerInternal : public MojDbServiceHandlerBase
{
public:
	// method and service names
	static const MojChar* const PostBackupMethod;
	static const MojChar* const PostRestoreMethod;
	static const MojChar* const PreBackupMethod;
	static const MojChar* const PreRestoreMethod;
	static const MojChar* const ScheduledPurgeMethod;
	static const MojChar* const SpaceCheckMethod;
	static const MojChar* const ScheduledSpaceCheckMethod;

	typedef enum { NeverSentSpaceAlert = -2, NoSpaceAlert = -1, AlertLevelLow, AlertLevelMedium,
		AlertLevelHigh } AlertLevel;

	MojDbServiceHandlerInternal(MojDb& db, MojReactor& reactor, MojService& service);
    static AlertLevel spaceAlertLevel();
    
    virtual MojErr open();  
	virtual MojErr close();

	MojErr configure(bool fatalError);

private:
	class PurgeHandler : public MojSignalHandler
	{
	public:
       PurgeHandler(MojDbServiceHandlerInternal* serviceHandler, const MojObject& activityId, bool doSpaceCheck = false);
		MojErr init();

	private:
		MojErr handleAdopt(MojObject& payload, MojErr errCode);
		MojErr handleComplete(MojObject& payload, MojErr errCode);
		MojErr initPayload(MojObject& payload);

		bool m_handled;
        bool m_doSpaceCheck;
        MojObject m_activityId;
		MojDbServiceHandlerInternal* m_serviceHandler;
		MojRefCountedPtr<MojServiceRequest> m_subscription;
		MojServiceRequest::ReplySignal::Slot<PurgeHandler> m_adoptSlot;
		MojServiceRequest::ReplySignal::Slot<PurgeHandler> m_completeSlot;
	};

	class LocaleHandler : public MojSignalHandler
	{
	public:
		LocaleHandler(MojDb& db);
		MojErr handleResponse(MojObject& payload, MojErr errCode);

		MojDb& m_db;
		MojServiceRequest::ReplySignal::Slot<LocaleHandler> m_slot;
	};

	
	class AlertHandler : public MojSignalHandler
	{
	public:
		AlertHandler(MojService& service, const MojObject& payload);
		MojErr send();
		MojErr handleBootStatusResponse(MojObject& payload, MojErr err);
		MojErr handleAlertResponse(MojObject& payload, MojErr errCode);

		MojServiceRequest::ReplySignal::Slot<AlertHandler> m_bootStatusSlot;
		MojServiceRequest::ReplySignal::Slot<AlertHandler> m_alertSlot;
		MojRefCountedPtr<MojServiceRequest> m_subscription;
		MojService& m_service;
		MojObject m_payload;
	};

	class SpaceCheckHandler : public MojSignalHandler
	{
	public:
		SpaceCheckHandler(MojDbServiceHandlerInternal* parent, MojServiceMessage* msg);
		~SpaceCheckHandler();

		MojErr dispatchUpdate(const MojObject& message);
		MojErr handleCancel(MojServiceMessage* msg);

		MojDbServiceHandlerInternal* m_parent;
		MojRefCountedPtr<MojServiceMessage> m_msg;
		MojServiceMessage::CancelSignal::Slot<SpaceCheckHandler> m_cancelSlot;
	};
	

	MojErr handlePostBackup(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePostRestore(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePreBackup(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePreRestore(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleScheduledPurge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleSpaceCheck(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleScheduledSpaceCheck(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);

	MojErr requestLocale();
	MojErr doSpaceCheck();
    static MojErr doSpaceCheck(AlertLevel& level, int& bytesUsed, int& bytesAvailable);
	static gboolean periodicSpaceCheck(gpointer data);
	MojErr generateSpaceAlert(AlertLevel level, MojInt64 bytesUsed, MojInt64 bytesAvailable);
	MojErr generateFatalAlert();
	MojErr generateAlert(const MojChar* event, const MojObject& msg);

	GSource* m_spaceCheckTimer;
	MojVector<MojRefCountedPtr<SpaceCheckHandler> > m_spaceCheckHandlers;
	static AlertLevel m_spaceAlertLevel;
	static bool m_compactRunning;

	MojService& m_service;
	MojRefCountedPtr<LocaleHandler> m_localeChangeHandler;
	static const Method s_privMethods[];

	// Some of these need to be changed to configuration items
	static const MojInt32 SpaceCheckInterval;
	static const MojChar* const DatabaseRoot;
	static const MojDouble SpaceAlertLevels[];
	static const MojChar* const SpaceAlertNames[];
	static const MojInt32 NumSpaceAlertLevels;
};

#endif /* MOJDBSERVICEHANDLERINTERNAL_H_ */
