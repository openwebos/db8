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

#include "db/MojDbQuotaCheckAlert.h"
#include "db/MojDb.h"

/*
 * MojDbQuotaCheckAlert
 */
MojDbQuotaCheckAlert::MojDbQuotaCheckAlert(MojDb& db)
 : m_db(db)
{
}

bool MojDbQuotaCheckAlert::isEmpty(void)
{
    return(m_quotaCheckSubscribers.size() == 0);
}

MojErr MojDbQuotaCheckAlert::subscribe(MojServiceMessage* pMsg, const MojString& owner)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(pMsg);
    MojErr err;
    MojString myOwner(owner);

    if(owner.empty())
    {
        err = myOwner.assign(pMsg->senderName());
        MojErrCheck(err);
    }

    //exist?
    for (MojSize i = 0; i < m_quotaCheckSubscribers.size(); ++i)
    {
        MojAssert(m_quotaCheckSubscribers.at(i).get());
        MojChar* pSender = getSubscriber(i)->m_owner.data();

        if(MojStrCmp(owner.data(), pSender) == 0)
        {
            return MojErrNone;
        }
    }

    //add new
    MojRefCountedPtr<QuotaCheckNode> handler = new QuotaCheckNode(this, pMsg, myOwner);
    m_quotaCheckSubscribers.push(handler);

    return MojErrNone;
}

void MojDbQuotaCheckAlert::unsubscribe(const MojString& owner)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(owner.empty());
    for (MojSize i = 0; i < m_quotaCheckSubscribers.size(); ++i)
    {
        MojChar* pSender = getSubscriber(i)->m_owner.data();
        if(MojStrCmp(owner.data(), pSender) == 0) {
            m_quotaCheckSubscribers.erase(i);
            break;
        }
    }
}

MojErr MojDbQuotaCheckAlert::notifySubscriber (const MojChar* pServiceName, const MojInt64& bytesUsed, const MojInt64& bytesAvailable)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(pServiceName);

    for (MojSize i = 0; i < m_quotaCheckSubscribers.size(); ++i)
    {
        MojAssert(m_quotaCheckSubscribers.at(i).get());
        MojChar* pSender = getSubscriber(i)->m_owner.data();

        if(MojStrCmp(pServiceName, pSender) == 0)
        {
            getSubscriber(i)->handleAlert(bytesUsed, bytesAvailable);
            break;
        }
    }

    return MojErrNone;
}

MojErr MojDbQuotaCheckAlert::checkQuota(MojServiceMessage* pMsg, const MojString& owner, MojInt64& bytesUsed, MojInt64& bytesAvailable)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    //ignore request without sender's name
    if (owner.empty())
    {
        MojErrThrow(MojErrNotFound);
    }

    //get info from quota engine by 'sender name'
    MojDbQuotaEngine* quotaEngine = m_db.quotaEngine();

    if(quotaEngine != NULL)
    {
        MojErr err = quotaEngine->quotaUsage(owner.data(), bytesAvailable, bytesUsed);
        MojErrCheck(err);
    }

    return MojErrNone;
}

MojDbQuotaCheckAlert::QuotaCheckNode* MojDbQuotaCheckAlert::getSubscriber (MojInt32 index)
{
    MojAssert(index < m_quotaCheckSubscribers.size());
    return(m_quotaCheckSubscribers.at(index).get());
}

/*
 * MojDbQuotaCheckAlert::QuotaCheckNode
 */
MojDbQuotaCheckAlert::QuotaCheckNode::QuotaCheckNode (MojDbQuotaCheckAlert* parent, MojServiceMessage* pMsg, const MojString& owner)
: mp_parent(parent)
, m_msg(pMsg)
, m_cancelSlot(this, &QuotaCheckNode::handleCancel)
{
    MojAssert(pMsg);
    pMsg->notifyCancel(m_cancelSlot);
    (void)m_owner.assign(owner);
}

MojDbQuotaCheckAlert::QuotaCheckNode::~QuotaCheckNode ()
{
}

MojErr MojDbQuotaCheckAlert::QuotaCheckNode::handleAlert(const MojInt64& bytesUsed, const MojInt64& bytesAvailable)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    MojAssert(m_msg.get());
    MojObject response;

    //put here info about actual quota status
    MojErr err = response.putBool(MojServiceMessage::ReturnValueKey, true);
    MojErrCheck(err);

    err = response.putString(_T("owner"), m_owner.data());
    MojErrCheck(err);
    err = response.putInt(_T("bytesUsed"), bytesUsed);
    MojErrCheck(err);
    err = response.putInt(_T("bytesAvailable"), bytesAvailable);
    MojErrCheck(err);
    err = response.putBool("subscribed", true);
    MojErrCheck(err);

    err = m_msg->reply(response);
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbQuotaCheckAlert::QuotaCheckNode::handleCancel(MojServiceMessage* pMsg)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    MojAssert(pMsg == m_msg.get());
    return MojErrNone;
}

