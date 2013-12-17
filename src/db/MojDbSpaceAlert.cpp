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

#include "db/MojDbSpaceAlert.h"
#include "db/MojDb.h"
#include <sys/statvfs.h>

const MojInt64 MASSIVE_AVAILABLE_SPACE = 500000000L;

const MojInt32 MojDbSpaceAlert::SpaceCheckInterval = 60;
const MojDouble MojDbSpaceAlert::SpaceAlertLevels[] = { 85.0, 90.0, 95.0 };
const MojChar* const MojDbSpaceAlert::SpaceAlertNames[] = {"none",  "low", "medium", "high" };
const MojInt32 MojDbSpaceAlert::NumSpaceAlertLevels =
sizeof(MojDbSpaceAlert::SpaceAlertLevels) / sizeof(MojDouble);

MojDbSpaceAlert::MojDbSpaceAlert(MojDb& db)
 : m_db(db)
{
    m_spaceAlertLevel = NeverSentSpaceAlert;
    m_compactRunning = false;
}

MojErr MojDbSpaceAlert::configure(const MojString& databaseRoot)
{
    MojAssert(!databaseRoot.empty());
    m_databaseRoot = databaseRoot;

    return MojErrNone;
}

MojDbSpaceAlert::AlertLevel MojDbSpaceAlert::spaceAlertLevel()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if(m_compactRunning) // do the real space check - we have no idea how long it'll take to clean up everything
   {
       int bytesUsed = 0;
       int bytesAvailable = 0;
       AlertLevel alertLevel = AlertLevelHigh;
       doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
       return alertLevel;
   }
   return m_spaceAlertLevel;
}

void MojDbSpaceAlert::subscribe(MojServiceMessage* msg)
{
    MojRefCountedPtr<SpaceCheckHandler> handler = new SpaceCheckHandler(this, msg);
    m_spaceCheckHandlers.push(handler);
}

MojErr MojDbSpaceAlert::doSpaceCheck()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    AlertLevel alertLevel;
    int bytesUsed;
    int bytesAvailable;
    MojErr err = doSpaceCheck(alertLevel, bytesUsed, bytesAvailable);
    if (err != MojErrNone)
        return err;

    // first display message if needed
        if (alertLevel != m_spaceAlertLevel) {
            MojObject message;
            MojErr err = message.putString(_T("severity"), getAlertName(alertLevel));
            MojErrCheck(err);
            err = message.putInt(_T("bytesUsed"), bytesUsed);
            MojErrCheck(err);
            err = message.putInt(_T("bytesAvailable"), bytesAvailable);
            MojErrCheck(err);
            err = message.putString(_T("path"), m_databaseRoot.data());
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

MojErr MojDbSpaceAlert::doSpaceCheck(AlertLevel& alertLevel, int& bytesUsed, int& bytesAvailable)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(!m_databaseRoot.empty());

    alertLevel = NoSpaceAlert;
    bytesUsed = 0;
    bytesAvailable = 0;

    struct statvfs dbFsStat;

    int ret = ::statvfs(m_databaseRoot.data(), &dbFsStat);
    if (ret != 0) {
        LOG_ERROR(MSGID_DB_SERVICE_ERROR, 2,
                  PMLOGKFV("error", "%d", ret),
                  PMLOGKS("db_root", m_databaseRoot.data()),
                  "Error 'error' attempting to stat database filesystem mounted at 'db_root'");
                  return MojErrInternal;
    }

    fsblkcnt_t blocksUsed = dbFsStat.f_blocks - dbFsStat.f_bfree;

    MojInt64 bigBytesUsed = (MojInt64)blocksUsed * dbFsStat.f_frsize;
    MojInt64 bigBytesAvailable = (MojInt64)dbFsStat.f_blocks * dbFsStat.f_frsize;

    MojDouble percentUsed =
    ((MojDouble)blocksUsed / (MojDouble)dbFsStat.f_blocks) * 100.0;

    LOG_DEBUG("[db_mojodb] Database volume %.1f full", percentUsed);

    if (MASSIVE_AVAILABLE_SPACE > bigBytesAvailable) { // regardless of percent used, if available space is massive, don't set space alert
        int level;
        for (level = AlertLevelHigh; level > NoSpaceAlert; --level) {
            if (percentUsed >= SpaceAlertLevels[level])
                break;
        }
        alertLevel = (AlertLevel) level;
    }


    if ((AlertLevel)alertLevel > NoSpaceAlert) {
        LOG_WARNING(MSGID_MOJ_DB_SERVICE_WARNING, 2,
                    PMLOGKFV("volume", "%.1f", percentUsed),
                    PMLOGKS("severity", getAlertName(alertLevel)),
                    "Database volume usage 'volume', generating warning, severity");
    } else {
        if ((AlertLevel)alertLevel != m_spaceAlertLevel) {
            // Generate 'ok' message only if there has been a transition.
            LOG_DEBUG("[db_mojodb] Database volume usage %1.f, space ok, no warning needed.\n", percentUsed);
        }
    }

    // On embedded devices, available/used space normally well below 2GB, but on desktop environments cap to 2GB
    bytesUsed = (int)(bigBytesUsed > MojInt32Max ? MojInt32Max : bigBytesUsed);
    bytesAvailable = (int)(bigBytesAvailable > MojInt32Max ? MojInt32Max : bigBytesAvailable);

    return MojErrNone;
}

gboolean MojDbSpaceAlert::periodicSpaceCheck(gpointer data)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojDbSpaceAlert* base;

    base = static_cast<MojDbSpaceAlert *>(data);

    base->doSpaceCheck();

    /* Keep repeating at the given interval */
    return true;
}

MojDbSpaceAlert::SpaceCheckHandler::SpaceCheckHandler(MojDbSpaceAlert* parent, MojServiceMessage* msg)
: m_parent(parent)
, m_msg(msg)
, m_cancelSlot(this, &SpaceCheckHandler::handleCancel)
{
    MojAssert(msg);
    msg->notifyCancel(m_cancelSlot);
}

MojDbSpaceAlert::SpaceCheckHandler::~SpaceCheckHandler()
{
}

MojErr MojDbSpaceAlert::SpaceCheckHandler::dispatchUpdate(const MojObject& message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojErr err = MojErrNone;
    err = m_msg->reply(message);
    MojErrCheck(err);

    return err;
}

MojErr MojDbSpaceAlert::SpaceCheckHandler::handleCancel(MojServiceMessage* msg)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(msg == m_msg.get());

    m_msg.reset();

    MojSize index = 0;
    bool isFound = false;
    for (MojSize i = 0; i < m_parent->m_spaceCheckHandlers.size(); i++) {
        if (m_parent->m_spaceCheckHandlers.at(i).get() == this) {
            index = i;
            isFound = true;
            break;
        }
    }

    MojAssert(!isFound);
    m_parent->m_spaceCheckHandlers.erase(index);

    return MojErrNone;
}
