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


#ifndef MOJDBQUOTAENGINE_H_
#define MOJDBQUOTAENGINE_H_

#include "db/MojDbDefs.h"
#include "db/MojDbPutHandler.h"
#include "core/MojMap.h"
#include "core/MojSignal.h"
#include "core/MojString.h"

class MojDbQuotaEngine : public MojDbPutHandler
{
public:
	class Quota : public MojRefCounted
	{
	public:
		Quota(gint64 size) : m_size(size), m_usage(0) {}
		gint64 size() const;
		gint64 available() const;
		gint64 usage() const;
		void size(gint64 val);
		void usage(gint64 val);
		void offset(gint64 off);

	private:
		gint64 m_size;
		gint64 m_usage;
		mutable MojThreadMutex m_mutex;
	};

	class Offset : public MojSignalHandler
	{
	public:
		Offset(const MojString& kindId);
		MojErr apply(gint64 offset);
		gint64 offset() const { return m_offset; }

	private:
		friend class MojDbQuotaEngine;

		MojString m_kindId;
		gint64 m_offset;
		MojRefCountedPtr<Quota> m_quota;
	};

	typedef MojMap<MojString, MojRefCountedPtr<Offset>, const MojChar* > OffsetMap;

	MojDbQuotaEngine();
	~MojDbQuotaEngine();

	virtual MojErr open(const MojObject& conf, MojDb* db, MojDbReq& req);
	virtual MojErr close();
	virtual MojErr put(MojObject& quota, MojDbReq& req, bool putObj = true);

	MojErr quotaForOwner(const MojString& owner, MojDbStorageTxn* txn);
	MojErr curKind(const MojDbKind* kind, MojDbStorageTxn* txn);
	MojErr applyUsage(MojDbStorageTxn* txn);
	MojErr applyQuota(MojDbStorageTxn* txn);
	MojErr kindUsage(const MojChar* kindId, gint64& usageOut, MojDbStorageTxn* txn);
	MojErr quotaUsage(const MojChar* owner, gint64& sizeOut, gint64& usageOut);
	MojErr refresh();
	MojErr stats(MojObject& objOut, MojDbReq& req);

private:
	friend class MojDbQuotaCommitHandler;

	static const MojChar* const QuotasKey;
	static const MojChar* const UsageDbName;

	typedef MojMap<MojString, MojRefCountedPtr<Quota>, const MojChar*, LengthComp> QuotaMap;

	MojErr refreshImpl(MojDbStorageTxn* txn);
	MojErr quotaForKind(const MojDbKind* kind, MojRefCountedPtr<Quota>& quotaOut);
	MojErr applyOffset(const MojString& kindId, gint64 offset, MojDbStorageTxn* txn);
	MojErr getUsage(const MojString& kindId, MojDbStorageTxn* txn, bool forUpdate, gint64& usageOut, MojRefCountedPtr<MojDbStorageItem>& itemOut);
	MojErr initUsage(MojDbKind* kind, MojDbReq& req);
	MojErr insertUsage(const MojString& kindId, gint64 usage, MojDbStorageTxn* txn);
	MojErr commitQuota(const MojString& owner, gint64 size);

	bool m_isOpen;
	QuotaMap m_quotas;
	MojRefCountedPtr<MojDbStorageDatabase> m_usageDb;
};

#endif /* MOJDBQUOTAENGINE_H_ */
