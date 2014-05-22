/* @@@LICENSE
 *
 *      Copyright (c) 2014 LG Electronics, Inc.
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

#ifndef MOJDBQUOTACHECKALERT_H
#define MOJDBQUOTACHECKALERT_H

#include "db/MojDbDefs.h"
#include "core/MojString.h"
#include "core/MojSignal.h"
#include "luna/MojLunaMessage.h"
#include "luna/MojLunaRequest.h"

typedef MojSignal<> QuotaAlertSignal;

class MojDbQuotaCheckAlert
{
private:
    class QuotaCheckNode : public MojSignalHandler
    {
    public:
        MojDbQuotaCheckAlert* mp_parent;
        MojRefCountedPtr<MojServiceMessage> m_msg;
        MojString m_owner;
        MojServiceMessage::CancelSignal::Slot<QuotaCheckNode> m_cancelSlot;

        QuotaCheckNode(MojDbQuotaCheckAlert* parent, MojServiceMessage* pMsg, const MojString& owner);
        ~QuotaCheckNode();
        MojErr handleAlert(const MojInt64& bytesUsed, const MojInt64& bytesAvailable);
        MojErr handleCancel(MojServiceMessage* pMsg);
    };

public:
    MojDbQuotaCheckAlert(MojDb& db);
    bool   isEmpty(void);
    MojErr subscribe(MojServiceMessage* pMsg, const MojString& owner);
    void   unsubscribe(const MojString& owner);
    MojErr checkQuota(MojServiceMessage* pMsg, const MojString& owner, MojInt64& bytesUsed, MojInt64& bytesAvailable);
    MojErr notifySubscriber (const MojChar* pServiceName, const MojInt64& bytesUsed, const MojInt64& bytesAvailable);
    QuotaCheckNode* getSubscriber (MojInt32 index);

private:
    MojDb& m_db;
    MojVector<MojRefCountedPtr<QuotaCheckNode> > m_quotaCheckSubscribers;
};

#endif //MOJDBQUOTACHECKALERT_H
