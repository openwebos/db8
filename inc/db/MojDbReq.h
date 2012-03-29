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


#ifndef MOJDBREQ_H_
#define MOJDBREQ_H_

#include "db/MojDbDefs.h"
#include "core/MojString.h"

struct MojDbReqRef
{
	explicit MojDbReqRef(MojDbReq& req) : m_req(req) {}
	MojDbReq* operator->() { return &m_req; }
	operator MojDbReq&() { return m_req; }
	
	MojDbReq& m_req;
};

class MojDbReq : private MojNoCopy
{
public:
	MojDbReq(bool admin = true);
	~MojDbReq();

	MojErr domain(const MojChar* domain);
	void admin(bool privilege) { m_admin = privilege; }
	void txn(MojDbStorageTxn* txn) { m_txn = txn; }
	void verifymode(bool vmode) { m_vmode = vmode;}
	void fixmode(bool fmode) { m_fixmode = fmode;}
	void batchsize(MojInt32 bsize) { m_batchSize = bsize;}
	void autobatch(bool bmode) { m_autobatch = bmode;}

	void beginBatch();
	MojErr endBatch();

	MojErr begin(MojDb* db, bool lockSchema);
	MojErr end(bool commitNow = true);
	MojErr abort();
	MojErr curKind(const MojDbKind* kind);
	MojErr startanother(MojDb* db);

	const MojString& domain() const { return m_domain; }
	bool admin() const { return m_admin; }
	bool batch() const { return m_batch; }
	bool autobatch() const { return m_autobatch; }
	MojDbStorageTxn* txn() const { return m_txn.get(); }
	bool fixmode() const {return m_fixmode;}
	bool verifymode() {return m_vmode;}
	MojInt32 batchsize() {return m_batchSize;}

	operator MojDbReqRef() { return MojDbReqRef(*this); }
	


private:
	void lock(MojDb* db, bool lockSchema);
	void unlock();
	MojErr commit();

	MojString m_domain;
	MojRefCountedPtr<MojDbStorageTxn> m_txn;
	MojUInt32 m_beginCount;
	MojDb* m_db;
	bool m_admin;
	bool m_batch;
	bool m_schemaLocked;	
	bool m_vmode;
	bool m_fixmode;
	MojInt32 m_batchSize;
	bool m_autobatch;
};

class MojDbAdminGuard : private MojNoCopy
{
public:
	MojDbAdminGuard(MojDbReq& req, bool setPrivilege = true);
	~MojDbAdminGuard() { unset(); }

	void set() { m_req.admin(true); }
	void unset() { m_req.admin(m_oldPrivilege); }

private:
	MojDbReq& m_req;
	bool m_oldPrivilege;
};

#endif /* MOJDBREQ_H_ */
