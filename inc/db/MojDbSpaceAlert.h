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

#ifndef MOJDBSPACEALERT_H
#define MOJDBSPACEALERT_H

#include "db/MojDbDefs.h"
#include "core/MojString.h"
#include "core/MojSignal.h"
#include "luna/MojLunaMessage.h"
#include "luna/MojLunaRequest.h"

class MojDbSpaceAlert
{
    class SpaceCheckHandler : public MojSignalHandler
    {
    public:
        SpaceCheckHandler(MojDbSpaceAlert* parent, MojServiceMessage* msg);
        ~SpaceCheckHandler();

        MojErr dispatchUpdate(const MojObject& message);
        MojErr handleCancel(MojServiceMessage* msg);

        MojDbSpaceAlert* m_parent;
        MojRefCountedPtr<MojServiceMessage> m_msg;
        MojServiceMessage::CancelSignal::Slot<SpaceCheckHandler> m_cancelSlot;
    };
public:
    typedef enum { NeverSentSpaceAlert = -2, NoSpaceAlert = -1, AlertLevelLow, AlertLevelMedium, AlertLevelHigh } AlertLevel;

    MojDbSpaceAlert(MojDb& db);
    MojErr configure(const MojString& databaseRoot);
    AlertLevel spaceAlertLevel();

    void subscribe(MojServiceMessage* msg);
    const MojString& getDatabaseRoot() const { return m_databaseRoot; }

    MojErr doSpaceCheck();
    MojErr doSpaceCheck(AlertLevel& level, int& bytesUsed, int& bytesAvailable);
    gboolean periodicSpaceCheck(gpointer data);
    const MojChar* getAlertName(AlertLevel alertLevel) const { return SpaceAlertNames[alertLevel - NoSpaceAlert]; }

private:
    MojString m_databaseRoot;
    GSource* m_spaceCheckTimer;
    AlertLevel m_spaceAlertLevel;
    bool m_compactRunning;

    MojVector<MojRefCountedPtr<SpaceCheckHandler> > m_spaceCheckHandlers;
    MojDb& m_db;

    static const MojInt32 SpaceCheckInterval;
    static const MojDouble SpaceAlertLevels[];
    static const MojChar* const SpaceAlertNames[];
    static const MojInt32 NumSpaceAlertLevels;
};

#endif
