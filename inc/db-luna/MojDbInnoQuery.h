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


#ifndef MOJDBINNOQUERY_H_
#define MOJDBINNOQUERY_H_

#include "db/MojDbDefs.h"
#include "db/MojDbInnoEngine.h"
#include "db/MojDbIsamQuery.h"

class MojDbInnoQuery : public MojDbIsamQuery
{
public:
	MojDbInnoQuery();
	~MojDbInnoQuery();

	MojErr open(MojDbInnoDatabase* db, MojDbInnoDatabase* joinDb,
				MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn);

	virtual MojErr close();

private:
	virtual MojErr seekImpl(const ByteVec& key, bool desc, bool& foundOut);
	virtual MojErr next(bool& foundOut);
	virtual MojErr getVal(MojDbStorageItem*& itemOut);
	MojErr loadKey(bool& foundOut);

	MojDbInnoItem m_item;
	MojDbInnoItem m_primaryItem;
	MojDbInnoDatabase* m_db;
};

#endif /* MOJDBINNOQUERY_H_ */
