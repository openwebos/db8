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


#ifndef MOJDBSERVICECLIENT_H_
#define MOJDBSERVICECLIENT_H_

#include "db/MojDbDefs.h"
#include "db/MojDbClient.h"
#include "db/MojDbServiceDefs.h"

class MojDbServiceClientBatch : public MojDbBatch
{
public:
	virtual MojErr execute(Signal::SlotRef handler);
	virtual MojErr put(const MojObject* begin, const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr get(const MojObject* idsBegin, const MojObject* idsEnd);
	virtual MojErr del(const MojObject* idsBegin, const MojObject* idsEnd, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr del(const MojDbQuery& query, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr merge(const MojObject* begin, const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr merge(const MojDbQuery& query, const MojObject& props, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr find(const MojDbQuery& query, bool returnCount = false);
	virtual MojErr search(const MojDbQuery& query, bool returnCount = false);

private:
	friend class MojDbServiceClient;

	MojDbServiceClientBatch(MojServiceRequest* req, MojDbServiceClient* client);

	MojErr emptyRequest(const MojChar* method);
	MojErr intRequest(const MojChar* method, const MojChar* prop, MojInt64 val);
	MojErr arrayRequest(const MojChar* method, const MojChar* prop,
						const MojObject* begin, const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	MojErr queryRequest(const MojChar* method, const MojDbQuery& query, MojUInt32 flags, bool count = false);
	MojErr startRequest(MojObjectVisitor& writer, const MojChar* method);

	MojRefCountedPtr<MojServiceRequest> m_req;
	MojDbServiceClient* m_client;
};

class MojDbServiceClient : public MojDbClient
{
public:
	// provide non-virtual overloads from base class
	using MojDbClient::putPermission;
	using MojDbClient::put;
	using MojDbClient::get;
	using MojDbClient::del;
	using MojDbClient::merge;

	MojDbServiceClient(MojService* service, const MojChar* serviceName = MojDbServiceDefs::ServiceName);

	virtual MojErr putKind(Signal::SlotRef handler, const MojObject& obj);
	virtual MojErr delKind(Signal::SlotRef handler, const MojChar* id);

	virtual MojErr putPermissions(Signal::SlotRef handler, const MojObject* begin,const MojObject* end);
	virtual MojErr getPermissions(Signal::SlotRef handler, const MojChar* type, const MojChar* object);

	virtual MojErr put(Signal::SlotRef handler, const MojObject* begin,
					   const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr get(Signal::SlotRef handler, const MojObject* idsBegin,
					   const MojObject* idsEnd);
	virtual MojErr del(Signal::SlotRef handler, const MojObject* idsBegin,
					   const MojObject* idsEnd, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr del(Signal::SlotRef handler, const MojDbQuery& query,
					   MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr merge(Signal::SlotRef handler, const MojObject* begin,
						 const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr merge(Signal::SlotRef handler, const MojDbQuery& query,
						 const MojObject& props, MojUInt32 flags = MojDb::FlagNone);
	virtual MojErr find(Signal::SlotRef handler, const MojDbQuery& query,
						bool watch = false, bool returnCount = false);
	virtual MojErr search(Signal::SlotRef handler, const MojDbQuery& query,
						  bool watch = false, bool returnCount = false);
	virtual MojErr watch(Signal::SlotRef handler, const MojDbQuery& query);

	virtual MojErr compact(Signal::SlotRef handler);
	virtual MojErr createBatch(MojAutoPtr<MojDbBatch>& batchOut);
	virtual MojErr purge(Signal::SlotRef handler, MojUInt32 window);
	virtual MojErr purgeStatus(Signal::SlotRef handler);
	virtual MojErr reserveIds(Signal::SlotRef handler, MojUInt32 count);

	MojService* service() { return m_service; }

private:
	friend class MojDbServiceClientBatch;

	MojErr emptyRequest(Signal::SlotRef handler, const MojChar* method);
	MojErr intRequest(Signal::SlotRef handler, const MojChar* method, const MojChar* prop, MojInt64 val);
	MojErr arrayRequest(Signal::SlotRef handler, const MojChar* method, const MojChar* prop,
						const MojObject* begin, const MojObject* end, MojUInt32 flags = MojDb::FlagNone);
	MojErr queryRequest(Signal::SlotRef handler, const MojChar* method, const MojDbQuery& query, MojUInt32 flags = MojDb::FlagNone,
						bool watch = false, bool count = false, MojUInt32 numReplies = 1);

	static MojErr formatArrayParams(MojObjectVisitor& writer, const MojChar* prop,
									const MojObject* begin, const MojObject* end, MojUInt32 flags);
	static MojErr formatEmptyParams(MojObjectVisitor& writer);
	static MojErr formatIntParams(MojObjectVisitor& writer, const MojChar* prop, MojInt64 val);
	static MojErr formatQueryParams(MojObjectVisitor& writer, const MojDbQuery& query, MojUInt32 flags, bool watch, bool count);
	static MojErr formatFlags(MojObjectVisitor& writer, MojUInt32 flags);

	MojService* m_service;
	const MojChar* m_serviceName;
};

#endif /* MOJDBSERVICECLIENT_H_ */
