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


#ifndef MOJDBISAMQUERY_H_
#define MOJDBISAMQUERY_H_

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"

class MojDbIsamQuery : public MojDbStorageQuery
{
public:
	typedef MojVector<MojString> StringVec;

	virtual ~MojDbIsamQuery();
	virtual MojErr close();
	virtual MojErr get(MojDbStorageItem*& itemOut, bool& foundOut);
	virtual MojErr getId(MojObject& idOut, guint32& groupOut, bool& foundOut);
	virtual MojErr count(guint32& countOut);
	virtual MojErr nextPage(MojDbQuery::Page& pageOut);
	virtual guint32 groupCount() const;
	

protected:

	friend class MojDbKind;
	enum State {
		StateInvalid,
		StateSeek,
		StateNext
	};

	typedef MojVector<guint8> ByteVec;
	typedef MojVector<MojDbKeyRange> RangeVec;

	virtual MojErr seekImpl(const ByteVec& key, bool desc, bool& foundOut) = 0;
	virtual MojErr next(bool& foundOut) = 0;
	virtual MojErr getVal(MojDbStorageItem*& itemOut, bool& foundOut) = 0;

	MojDbIsamQuery();
	MojErr open(MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn);
	MojErr getImpl(MojDbStorageItem*& itemOut, bool& foundOut, bool getItem);
	void init();
	bool match();
	bool limitEnforced() { return m_count >= m_plan->limit(); }
	int compareKey(const ByteVec& key);
	MojErr incrementCount();
	MojErr seek(bool& foundOut);
	MojErr getKey(guint32& groupOut, bool& foundOut);
	MojErr saveEndKey();
	MojErr parseId(MojObject& idOut);
	MojErr checkExclude(MojDbStorageItem* item, bool& excludeOut);

	bool m_isOpen;
	guint32 m_count;
	State m_state;
	RangeVec::ConstIterator m_iter;
	MojDbStorageTxn* m_txn;
	gsize m_keySize;
	const guint8* m_keyData;
	MojAutoPtr<MojDbQueryPlan> m_plan;
};

#endif /* MOJDBISAMQUERY_H_ */
