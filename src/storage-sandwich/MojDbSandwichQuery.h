/* @@@LICENSE
*
* Copyright (c) 2009-2014 LG Electronics, Inc.
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

#ifndef MOJDBLEVELQUERY_H_
#define MOJDBLEVELQUERY_H_

#include "db/MojDbDefs.h"
#include "MojDbSandwichEngine.h"
#include "MojDbSandwichItem.h"
#include "db/MojDbIsamQuery.h"

class MojDbSandwichQuery final : public MojDbIsamQuery
{
public:
	MojDbSandwichQuery();
	~MojDbSandwichQuery() override;

	MojErr open(MojDbSandwichDatabase* db, MojDbSandwichDatabase* joinDb,
			MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn);
	MojErr getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut) override;

	MojErr close() override;

private:
	MojErr seekImpl(const ByteVec& key, bool desc, bool& foundOut) override;
	MojErr next(bool& foundOut) override;
	MojErr getVal(MojDbStorageItem*& itemOut, bool& foundOut) override;
	MojErr readEntry(bool &foundOut);;

	std::unique_ptr<leveldb::Iterator> m_it;
	MojDbSandwichItem m_key;
	MojDbSandwichItem m_val;
	MojDbSandwichItem m_primaryVal;
	MojDbSandwichDatabase* m_db;
};

#endif /* MOJDBLEVELQUERY_H_ */
