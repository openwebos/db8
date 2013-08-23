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

#ifndef MOJDBLUNASERVICEPDM_H
#define MOJDBLUNASERVICEPDM_H

#include "db/MojDbDefs.h"
#include "db/MojDbServiceHandlerInternal.h"
#include "db/MojDbServiceHandler.h"
#include "core/MojReactorApp.h"
#include "core/MojGmainReactor.h"
#include "core/MojMessageDispatcher.h"
#include "luna/MojLunaService.h"
#include "db-luna/MojDbLunaServicePdmHandler.h"
#include <list>

class MojDb;

class MojDbLunaServicePdm
{
public:
    typedef std::list<MojDb*> database_list_t;

    MojDbLunaServicePdm(MojMessageDispatcher& dispatcher);
    MojErr configure(const MojObject& conf);
    MojErr init(MojReactor& reactor);
    MojErr open(MojGmainReactor& reactor, const MojChar* MojdbPDMClientName, const MojChar* pdmServiceName);
    MojErr close();

    MojLunaService& service() { return m_service; }
    bool isEnabled() const { return m_isEnabled; }

    MojErr addDatabase(MojDb* database);
    database_list_t* getActiveDatabases() { return &m_databases; }

private:
    MojLunaService m_service;
    MojRefCountedPtr<MojDbLunaServicePdmHandler> m_handler;

    bool m_isEnabled;
    static MojLogger s_log;
    static const MojChar* const ConfKey;
    database_list_t m_databases;
};

#endif
