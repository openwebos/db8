/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/
#include "db-luna/MojDbLunaServicePdmHandler.h"

#include "luna/MojLunaService.h"
#include "core/MojServiceMessage.h"
#include "core/MojService.h"
#include "db-luna/MojDbLunaServicePdm.h"
#include "db/MojDbShardEngine.h"

MojLogger MojDbLunaServicePdmHandler::s_log(_T("db-luna.shard"));

MojDbLunaServicePdmHandler::MojDbLunaServicePdmHandler(MojDbLunaServicePdm* parent, MojReactor& reactor)
: m_mojDbLunaServicePdm(parent),
m_dbErr(MojErrNone),
m_callbackInvoked (false),
m_reactor(reactor),
m_slot(this, &MojDbLunaServicePdmHandler::handleResult)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::MojDbLunaServicePdmHandler"));
}

MojErr MojDbLunaServicePdmHandler::open(MojLunaService* service, const MojChar* pdmServiceName)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, "Create subscribe request to PDM service");
    MojErr err;

    // send request
    MojRefCountedPtr<MojServiceRequest> req;
    err = service->createRequest(req);
    MojErrCheck(err);

    MojObject payload;
    err = payload.fromJson("{\"subscribe\":true}");
    MojErrCheck(err);

    MojLogDebug(s_log, "Send subscribe request to PDM service");
    err = req->send(m_slot, pdmServiceName, _T("listDevices"), payload, 777);   // @TODO: FIX THIS MAGIC NUMBER!!!!!!!
    MojErrCheck(err);

    /*std::cerr << "Wait" << std::endl;
     m_mediaClient->wait(m_service.get());*/


    return MojErrNone;
}

MojErr MojDbLunaServicePdmHandler::handleResult(MojObject& result, MojErr errCode)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::handleResult"));

    m_callbackInvoked = true;
    m_result = result;
    m_dbErr = errCode;
    if (errCode != MojErrNone) {
        bool found = false;
        MojErr err = result.get(MojServiceMessage::ErrorTextKey, m_errTxt, found);

        MojLogError(s_log, _T("Luna error response: %s"), m_errTxt.data());
        MojAssert(found);
    } else {
//#ifdef MOJ_DEBUG
        MojString resStr;
        result.toJson(resStr);

        //MojLogDebug(s_log, _T("Device list: %s"), resStr.data());
        MojLogInfo(s_log, _T("Device list: %s"), resStr.data());
//#endif
        MojObject deviceList;
        MojErr err = result.getRequired("devices", deviceList);
        MojErrCheck(err);

        std::set<MojString> notProcessedIds;
        copyShardCache(&notProcessedIds);

        for (MojObject::ConstArrayIterator i = deviceList.arrayBegin(); i != deviceList.arrayEnd(); ++i) {
            MojString deviceType;
            MojObject subDevices;

            err = i->getRequired("deviceType", deviceType);
            MojErrCheck(err);
            MojLogDebug(s_log, _T("Device type is: %s"), deviceType.data());

            if (deviceType != "usb") {
                MojLogInfo(s_log, "Got from PDM device, but device not usb media. Ignore it");
                // not usb media. PDM returns ALL list of media, like USB, Internal storage and other.
                // db8 intresting only in usb sticks
                continue;
            }

            err = i->getRequired("subDevices", subDevices);
            MojErrCheck(err);

            for (MojObject::ConstArrayIterator it = subDevices.arrayBegin(); it != subDevices.arrayEnd(); ++it) {
                MojDbShardEngine::ShardInfo shardInfo;
                err = processLunaResponse(&(*it), &shardInfo);
                MojErrCheck(err);

                if (shardInfo.deviceId.empty()) {
                    MojLogWarning(s_log, "Device id is empty, ignore it");
                    continue;
                }

                shardInfo.active = true;

                notProcessedIds.erase(shardInfo.deviceId);  // mark it as processed

                if (!existInCache(shardInfo.deviceId)) {
                    MojLogNotice(s_log, _T("Found new device %s. Add to device cache and send notification to shard engine"), shardInfo.deviceId.data());

                    m_shardCache[shardInfo.deviceId] = shardInfo;
                    err = m_mojDbLunaServicePdm->notifyShardEngine(shardInfo);
                    MojErrCheck(err);
                } else {
                    MojLogInfo(s_log, _T("Device uuid cached, it looks like it doesn't changed"));
                }
            } // end subDevices loop
        }   // end main list of devices loop

        // notify shard engine about all inactive shards
        for (std::set<MojString>::const_iterator i = notProcessedIds.begin(); i != notProcessedIds.end(); ++i) {
            ShardInfoListType::iterator shardCacheIterator = m_shardCache.find(*i);
            if (shardCacheIterator != m_shardCache.end()) {
                MojLogInfo(s_log, "Device %s not found in cache. Notify shard engine that shard not active", shardCacheIterator->second.deviceId.data());

                shardCacheIterator->second.active = false;
                err = m_mojDbLunaServicePdm->notifyShardEngine(shardCacheIterator->second);
                m_shardCache.erase(shardCacheIterator);
            }
        }
    }
    return MojErrNone;
}

void MojDbLunaServicePdmHandler::reset()
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::reset"));

    m_dbErr = MojErrNone;
    m_callbackInvoked = false;
    m_errTxt.clear();
    m_result.clear();
}

MojErr MojDbLunaServicePdmHandler::wait(MojService* service)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, _T("MojDbLunaServicePdmHandler::wait"));

    while (!m_callbackInvoked) {
        MojErr err = service->dispatch();
        MojErrCheck(err);
    }
    return MojErrNone;
}

MojErr MojDbLunaServicePdmHandler::processLunaResponse(const MojObject* response, MojDbShardEngine::ShardInfo* result)
{
    MojErr err;
    err = response->getRequired("deviceId", result->deviceId);
    MojErrCheck(err);
    MojLogDebug(s_log, _T("device id is: %s"), result->deviceId.data());

    err = response->getRequired("deviceName", result->deviceName);
    MojErrCheck(err);
    MojLogDebug(s_log, _T("device name is: %s"), result->deviceName.data());

    err = response->getRequired("deviceUri", result->deviceUri);
    MojErrCheck(err);
    MojLogDebug(s_log, _T("device uri is: %s"), result->deviceUri.data());

    // for media deviceUri and mount path are the same
    result->mountPath = result->deviceUri;

    return MojErrNone;
}

bool MojDbLunaServicePdmHandler::existInCache(const MojString& id)
{
    ShardInfoListType::iterator shardCacheIterator = m_shardCache.find(id);
    return (shardCacheIterator != m_shardCache.end());
}

void MojDbLunaServicePdmHandler::copyShardCache(std::set<MojString>* shardIdSet)
{
    for (ShardInfoListType::const_iterator shardInfoIterator = m_shardCache.begin(); shardInfoIterator != m_shardCache.end(); ++shardInfoIterator) {
        shardIdSet->insert(shardInfoIterator->first);
    }
}

