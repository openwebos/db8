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


#ifndef MOJDBCLIENT_H_
#define MOJDBCLIENT_H_

#include "db/MojDbDefs.h"
#include "db/MojDb.h"
#include "core/MojServiceRequest.h"

class MojDbBatch : private MojNoCopy
{
public:
	typedef MojServiceRequest::ReplySignal Signal;

	MojDbBatch() {}
	virtual ~MojDbBatch() {}

	virtual MojErr execute(Signal::SlotRef handler) = 0;
	virtual MojErr put(const MojObject* begin, const MojObject* end, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr get(const MojObject* idsBegin, const MojObject* idsEnd) = 0;
	virtual MojErr del(const MojObject* idsBegin, const MojObject* idsEnd, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr del(const MojDbQuery& query, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr merge(const MojObject* begin, const MojObject* end, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr merge(const MojDbQuery& query, const MojObject& props, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr find(const MojDbQuery& query, bool returnCount = false) = 0;
	virtual MojErr search(const MojDbQuery& query, bool returnCount = false) = 0;

	MojErr put(const MojObject& obj, guint32 flags = MojDb::FlagNone);
	MojErr get(const MojObject& id);
	MojErr del(const MojObject& id, guint32 flags = MojDb::FlagNone);
	MojErr merge(const MojObject& obj, guint32 flags = MojDb::FlagNone);
};

class MojDbClient : public MojSignalHandler
{
public:
	typedef MojServiceRequest::ReplySignal Signal;

	MojDbClient() {}
	virtual ~MojDbClient() {}

	virtual MojErr putKind(Signal::SlotRef handler, const MojObject& obj) = 0;
	virtual MojErr delKind(Signal::SlotRef handler, const MojChar* id) = 0;

	virtual MojErr putPermissions(Signal::SlotRef handler, const MojObject* begin,const MojObject* end) = 0;
	virtual MojErr getPermissions(Signal::SlotRef handler, const MojChar* type, const MojChar* object) = 0;

	virtual MojErr put(Signal::SlotRef handler, const MojObject* begin,
					   const MojObject* end, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr get(Signal::SlotRef handler, const MojObject* idsBegin,
					   const MojObject* idsEnd) = 0;
	virtual MojErr del(Signal::SlotRef handler, const MojObject* idsBegin,
					   const MojObject* idsEnd, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr del(Signal::SlotRef handler, const MojDbQuery& query,
					   guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr merge(Signal::SlotRef handler, const MojObject* begin,
						 const MojObject* end, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr merge(Signal::SlotRef handler, const MojDbQuery& query,
						 const MojObject& props, guint32 flags = MojDb::FlagNone) = 0;
	virtual MojErr find(Signal::SlotRef handler, const MojDbQuery& query,
						bool watch = false, bool returnCount = false) = 0;
	virtual MojErr search(Signal::SlotRef handler, const MojDbQuery& query,
						  bool watch = false, bool returnCount = false) = 0;
	virtual MojErr watch(Signal::SlotRef handler, const MojDbQuery& query) = 0;

	virtual MojErr compact(Signal::SlotRef handler) = 0;
	virtual MojErr createBatch(MojAutoPtr<MojDbBatch>& batchOut) = 0;
	virtual MojErr purge(Signal::SlotRef handler, guint32 window) = 0;
	virtual MojErr purgeStatus(Signal::SlotRef handler) = 0;
	virtual MojErr reserveIds(Signal::SlotRef handler, guint32 count) = 0;

	MojErr putPermission(Signal::SlotRef handler, const MojObject& obj);
	MojErr put(Signal::SlotRef handler, const MojObject& obj, guint32 flags = MojDb::FlagNone);
	MojErr get(Signal::SlotRef handler, const MojObject& id);
	MojErr del(Signal::SlotRef handler, const MojObject& id, guint32 flags = MojDb::FlagNone);
	MojErr merge(Signal::SlotRef handler, const MojObject& obj, guint32 flags = MojDb::FlagNone);
};

#endif /* MOJDBCLIENT_H_ */
