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
#include "MojDbSandwichCursor.h"
#include "MojDbSandwichItem.h"
#include "db/MojDbIsamQuery.h"

class MojDbSandwichQuery : public MojDbIsamQuery
{
public:
	MojDbSandwichQuery();
	~MojDbSandwichQuery();

	MojErr open(MojDbSandwichDatabase* db, MojDbSandwichDatabase* joinDb,
			MojAutoPtr<MojDbQueryPlan> plan, MojDbStorageTxn* txn);
	MojErr getById(const MojObject& id, MojDbStorageItem*& itemOut, bool& foundOut);

	virtual MojErr close();

private:
	static const MojUInt32 SeekFlags;
	static const MojUInt32 SeekEmptyFlags[2];
	static const MojUInt32 NextFlags[2];

	virtual MojErr seekImpl(const ByteVec& key, bool desc, bool& foundOut);
	virtual MojErr next(bool& foundOut);
	virtual MojErr getVal(MojDbStorageItem*& itemOut, bool& foundOut);
	MojErr getKey(bool& foundOut, MojUInt32 flags);

	MojDbSandwichCursor m_cursor;
	MojDbSandwichItem m_key;
	MojDbSandwichItem m_val;
	MojDbSandwichItem m_primaryVal;
	MojDbSandwichDatabase* m_db;
};

#endif /* MOJDBLEVELQUERY_H_ */
