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


#ifndef MOJDBSERVICEHANDLERBASE_H_
#define MOJDBSERVICEHANDLERBASE_H_

#include "db/MojDbDefs.h"
#include "db/MojDb.h"
#include "core/MojService.h"
#include "core/MojServiceMessage.h"

class MojDbServiceHandlerBase : public MojService::CategoryHandler
{
public:
	static const guint32 DeadlockSleepMillis = 20;
	static const guint32 MaxDeadlockRetries = 20;
	static const guint32 MaxIndexlockRetries = 5;
	static const guint32 MinIndexlockRetries = 3;
	static const guint32 MaxBatchRetries = 1000;

	typedef MojErr (CategoryHandler::* DbCallback)(MojServiceMessage* msg, const MojObject& payload, MojDbReq& req);

	MojDbServiceHandlerBase(MojDb& db, MojReactor& reactor);

	virtual MojErr open() = 0;
	virtual MojErr close() = 0;

protected:

	virtual MojErr invoke(Callback method, MojServiceMessage* msg, MojObject& payload);
	MojErr invokeImpl(Callback method, MojServiceMessage* msg, MojObject& payload);

	static MojErr formatCount(MojServiceMessage* msg, guint32 count);
	static MojErr formatPut(MojServiceMessage* msg, const MojObject* begin, const MojObject* end);
	static MojErr formatPutAppend(MojObjectVisitor& writer, const MojObject& result);

	MojDb& m_db;
	MojReactor& m_reactor;
	static MojLogger s_log;
};

#endif /* MOJDBSERVICEHANDLERBASE_H_ */
