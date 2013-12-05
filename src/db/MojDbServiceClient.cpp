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


#include "db/MojDbServiceClient.h"
#include "db/MojDbQuery.h"
#include "db/MojDbServiceDefs.h"
#include "core/MojJson.h"
#include "core/MojService.h"

MojDbServiceClient::MojDbServiceClient(MojService* service, const MojChar* serviceName)
: m_service(service),
  m_serviceName(serviceName)
{
	MojAssert(service);
}

MojErr MojDbServiceClient::putKind(Signal::SlotRef handler, const MojObject& obj)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);
	err = req->send(handler, m_serviceName, MojDbServiceDefs::PutKindMethod, obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::delKind(Signal::SlotRef handler, const MojChar* id)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.stringProp(MojDbServiceDefs::IdKey, id);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, MojDbServiceDefs::DelKindMethod);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::putPermissions(Signal::SlotRef handler, const MojObject* begin,const MojObject* end)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(handler, MojDbServiceDefs::PutPermissionsMethod, MojDbServiceDefs::PermissionsKey, begin, end);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::getPermissions(Signal::SlotRef handler, const MojChar* type, const MojChar* object)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(type && object);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	MojString str;
	err = writer.stringProp(MojDbServiceDefs::TypeKey, type);
	MojErrCheck(err);
	err = writer.stringProp(MojDbServiceDefs::ObjectKey, object);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, MojDbServiceDefs::GetPermissionsMethod);
	MojErrCheck(err);

	return MojErrNone;

	return MojErrNone;
}

MojErr MojDbServiceClient::put(Signal::SlotRef handler, const MojObject* begin,
							   const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(handler, MojDbServiceDefs::PutMethod, MojDbServiceDefs::ObjectsKey, begin, end, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::get(Signal::SlotRef handler, const MojObject* idsBegin, const MojObject* idsEnd)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(handler, MojDbServiceDefs::GetMethod, MojDbServiceDefs::IdsKey, idsBegin, idsEnd);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::del(Signal::SlotRef handler, const MojObject* idsBegin,
							   const MojObject* idsEnd, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(handler, MojDbServiceDefs::DelMethod, MojDbServiceDefs::IdsKey, idsBegin, idsEnd, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::del(Signal::SlotRef handler, const MojDbQuery& query, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(handler, MojDbServiceDefs::DelMethod, query, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::merge(Signal::SlotRef handler, const MojObject* begin,
								 const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(handler, MojDbServiceDefs::MergeMethod, MojDbServiceDefs::ObjectsKey, begin, end, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::merge(Signal::SlotRef handler, const MojDbQuery& query,
								 const MojObject& props, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::QueryKey);
	MojErrCheck(err);
	err = query.toObject(writer);
	MojErrCheck(err);
	err = writer.objectProp(MojDbServiceDefs::PropsKey, props);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, MojDbServiceDefs::MergeMethod);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::find(Signal::SlotRef handler, const MojDbQuery& query,
								bool watch, bool count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(handler, MojDbServiceDefs::FindMethod, query, MojDb::FlagNone, watch, count, watch + 1);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::search(Signal::SlotRef handler, const MojDbQuery& query,
								  bool watch, bool count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(handler, MojDbServiceDefs::SearchMethod, query, MojDb::FlagNone, watch, count, watch + 1);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::watch(Signal::SlotRef handler, const MojDbQuery& query)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(handler, MojDbServiceDefs::WatchMethod, query, MojDb::FlagNone, false, false, 2);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::createBatch(MojAutoPtr<MojDbBatch>& batchOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = writer.beginObject();
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::OperationsKey);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);

	batchOut.reset(new MojDbServiceClientBatch(req.get(), this));
	MojAllocCheck(batchOut.get());

	return MojErrNone;
}

MojErr MojDbServiceClient::compact(Signal::SlotRef handler)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = emptyRequest(handler, MojDbServiceDefs::CompactMethod);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::purge(Signal::SlotRef handler, MojUInt32 window)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = intRequest(handler, MojDbServiceDefs::PurgeMethod, MojDbServiceDefs::WindowKey, window);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::purgeStatus(Signal::SlotRef handler)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = emptyRequest(handler, MojDbServiceDefs::PurgeStatusMethod);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::reserveIds(Signal::SlotRef handler, MojUInt32 count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = intRequest(handler, MojDbServiceDefs::ReserveIdsMethod, MojDbServiceDefs::CountKey, count);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::emptyRequest(Signal::SlotRef handler, const MojChar* method)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);
	MojObject payload(MojObject::TypeObject);
	err = req->send(handler, m_serviceName, method, payload);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::intRequest(Signal::SlotRef handler, const MojChar* method, const MojChar* prop, MojInt64 val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);
	MojObjectVisitor& writer = req->writer();
	err = formatIntParams(writer, prop, val);
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, method);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::arrayRequest(Signal::SlotRef handler, const MojChar* method, const MojChar* prop,
										const MojObject* begin, const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = formatArrayParams(writer, prop, begin, end, flags);
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, method);
	MojErrCheck(err);

	return MojErrNone;
}


MojErr MojDbServiceClient::queryRequest(Signal::SlotRef handler, const MojChar* method,
										const MojDbQuery& query, MojUInt32 flags, bool watch, bool count, MojUInt32 numReplies)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojRefCountedPtr<MojServiceRequest> req;
	MojErr err = m_service->createRequest(req);
	MojErrCheck(err);

	MojObjectVisitor& writer = req->writer();
	err = formatQueryParams(writer, query, flags, watch, count);
	MojErrCheck(err);

	err = req->send(handler, m_serviceName, method, numReplies);
	MojErrCheck(err);

	return MojErrNone;
}


MojErr MojDbServiceClient::formatArrayParams(MojObjectVisitor& writer, const MojChar* prop,
											const MojObject* begin, const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = formatFlags(writer, flags);
	MojErrCheck(err);
	err = writer.propName(prop);
	MojErrCheck(err);
	err = writer.beginArray();
	MojErrCheck(err);
	for (const MojObject* i = begin; i != end; ++i) {
		err = i->visit(writer);
		MojErrCheck(err);
	}
	err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::formatIntParams(MojObjectVisitor& writer, const MojChar* prop, MojInt64 val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.intProp(prop, val);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::formatEmptyParams(MojObjectVisitor& writer)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::formatQueryParams(MojObjectVisitor& writer, const MojDbQuery& query, MojUInt32 flags, bool watch, bool count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = formatFlags(writer, flags);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::QueryKey);
	MojErrCheck(err);
	err = query.toObject(writer);
	MojErrCheck(err);
	if (count) {
		err = writer.boolProp(MojDbServiceDefs::CountKey, true);
		MojErrCheck(err);
	}
	if (watch) {
		err = writer.boolProp(MojDbServiceDefs::WatchKey, true);
		MojErrCheck(err);
	}
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClient::formatFlags(MojObjectVisitor& writer, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	if (MojFlagGet(flags, MojDb::FlagPurge)) {
		MojErr err = writer.boolProp(MojDbServiceDefs::PurgeKey, true);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojDbServiceClientBatch::MojDbServiceClientBatch(MojServiceRequest* req, MojDbServiceClient* client)
: m_req(req),
  m_client(client)
{
}

MojErr MojDbServiceClientBatch::execute(Signal::SlotRef handler)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();
	MojErr err = writer.endArray();
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	err = m_req->send(handler, m_client->m_serviceName, MojDbServiceDefs::BatchMethod);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::put(const MojObject* begin, const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(MojDbServiceDefs::PutMethod, MojDbServiceDefs::ObjectsKey, begin, end, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::get(const MojObject* idsBegin, const MojObject* idsEnd)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(MojDbServiceDefs::GetMethod, MojDbServiceDefs::IdsKey, idsBegin, idsEnd);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::del(const MojObject* idsBegin, const MojObject* idsEnd, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(MojDbServiceDefs::DelMethod, MojDbServiceDefs::IdsKey, idsBegin, idsEnd, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::del(const MojDbQuery& query, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(MojDbServiceDefs::DelMethod, query, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::merge(const MojObject* begin, const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = arrayRequest(MojDbServiceDefs::MergeMethod, MojDbServiceDefs::ObjectsKey, begin, end, flags);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::merge(const MojDbQuery& query, const MojObject& props, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::QueryKey);
	MojErrCheck(err);
	err = query.toObject(writer);
	MojErrCheck(err);
	err = writer.objectProp(MojDbServiceDefs::PropsKey, props);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);
	
	return MojErrNone;
}

MojErr MojDbServiceClientBatch::find(const MojDbQuery& query, bool returnCount)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(MojDbServiceDefs::FindMethod, query, MojDb::FlagNone, returnCount);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::search(const MojDbQuery& query, bool returnCount)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = queryRequest(MojDbServiceDefs::SearchMethod, query, MojDb::FlagNone, returnCount);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::startRequest(MojObjectVisitor& writer, const MojChar* method)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = writer.beginObject();
	MojErrCheck(err);
	err = writer.stringProp(MojDbServiceDefs::MethodKey, method);
	MojErrCheck(err);
	err = writer.propName(MojDbServiceDefs::ParamsKey);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::emptyRequest(const MojChar* method)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();

	MojErr err = startRequest(writer, method);
	MojErrCheck(err);
	err = MojDbServiceClient::formatEmptyParams(writer);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::intRequest(const MojChar* method, const MojChar* prop, MojInt64 val)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();

	MojErr err = startRequest(writer, method);
	MojErrCheck(err);
	err = MojDbServiceClient::formatIntParams(writer, prop, val);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::arrayRequest(const MojChar* method, const MojChar* prop, const MojObject* begin, const MojObject* end, MojUInt32 flags)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();

	MojErr err = startRequest(writer, method);
	MojErrCheck(err);
	err = MojDbServiceClient::formatArrayParams(writer, prop, begin, end, flags);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbServiceClientBatch::queryRequest(const MojChar* method, const MojDbQuery& query, MojUInt32 flags, bool count)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojObjectVisitor& writer = m_req->writer();

	MojErr err = startRequest(writer, method);
	MojErrCheck(err);
	err = MojDbServiceClient::formatQueryParams(writer, query, flags, false, count);
	MojErrCheck(err);
	err = writer.endObject();
	MojErrCheck(err);

	return MojErrNone;
}
