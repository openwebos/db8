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


#ifndef MOJDBSERVICEHANDLER_H_
#define MOJDBSERVICEHANDLER_H_

#include "db/MojDbServiceHandlerBase.h"
#include "db/MojDbShardInfo.h"

#include <map>
#include <set>

class MojDbServiceHandler : public MojDbServiceHandlerBase
{
public:
	static const MojUInt32 MaxQueryLimit = MojDbQuery::MaxQueryLimit;
	static const MojUInt32 MaxReserveIdCount = MaxQueryLimit * 2;

	MojDbServiceHandler(MojDb& db, MojReactor& reactor);

	virtual MojErr open();
	virtual MojErr close();

private:
	static const MojChar* const BatchSchema;
	static const MojChar* const CompactSchema;
	static const MojChar* const DelSchema;
	static const MojChar* const DelKindSchema;
	static const MojChar* const DumpSchema;
	static const MojChar* const FindSchema;
	static const MojChar* const GetSchema;
	static const MojChar* const LoadSchema;
	static const MojChar* const MergeSchema;
	static const MojChar* const PurgeSchema;
	static const MojChar* const PurgeStatusSchema;
	static const MojChar* const PutSchema;
	static const MojChar* const PutKindSchema;
	static const MojChar* const PutPermissionsSchema;
	static const MojChar* const PutQuotasSchema;
	static const MojChar* const QuotaStatsSchema;
	static const MojChar* const ReserveIdsSchema;
	static const MojChar* const SearchSchema;
	static const MojChar* const StatsSchema;
	static const MojChar* const WatchSchema;
	static const MojChar* const QuotaCheckSchema;
	static const MojChar* const ListActiveMediaSchema;
	static const MojChar* const ShardInfoSchema;
	static const MojChar* const ShardKindSchema;
	static const MojChar* const SetShardModeSchema;
	static const MojChar* const RegisterMediaSchema;

	typedef MojMap<MojString, DbCallback, const MojChar*, MojComp<const MojChar*>, MojCompAddr<DbCallback> > BatchMap;
	typedef std::map<MojString, MojDbShardInfo> shard_cache_t;

	class Watcher : public MojSignalHandler
	{
	public:
		Watcher(MojServiceMessage* msg);

		MojErr handleWatch();
		MojErr handleCancel(MojServiceMessage* msg);

		MojRefCountedPtr<MojServiceMessage> m_msg;
		MojDb::WatchSignal::Slot<Watcher> m_watchSlot;
		MojServiceMessage::CancelSignal::Slot<Watcher> m_cancelSlot;
	};

	MojErr handleBatch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleCompact(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleDel(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleDelKind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleDump(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleFind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleGet(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleLoad(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleMerge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePurge(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePurgeStatus(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePut(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePutKind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePutPermissions(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handlePutQuotas(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleQuotaStats(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleReserveIds(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleSearch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleStats(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleWatch(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleListActiveMedia(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleShardInfo(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleShardKind(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleSetShardMode(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);
	MojErr handleRegisterMedia(MojServiceMessage* msg, MojObject& payload, MojDbReq& req);

	MojErr findImpl(MojServiceMessage* msg, MojObject& payload, MojDbReq& req, MojDbCursor& cursor, bool doCount);
	MojErr convert(const MojObject& object, MojDbShardInfo& shardInfo);
	bool   existInCache(const MojString& id);
	void   copyShardCache(std::set<MojString>* shardIdSet);

	BatchMap m_batchCallbacks;
	shard_cache_t m_shardCache;

	static const SchemaMethod s_pubMethods[];
	static const SchemaMethod s_privMethods[];
	static const Method s_batchMethods[];
};

#endif /* MOJDBSERVICEHANDLER_H_ */
