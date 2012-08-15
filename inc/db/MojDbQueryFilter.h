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


#ifndef MOJDBQUERYFILTER_H_
#define MOJDBQUERYFILTER_H_

#include "db/MojDbDefs.h"
#include "db/MojDbQuery.h"

class MojDbQueryFilter
{
public:
	MojDbQueryFilter();
	~MojDbQueryFilter();

	MojErr init(const MojDbQuery& query);
	bool test(const MojObject& obj) const;

private:
	static bool testLower(const MojDbQuery::WhereClause& clause, const MojObject& val);
	static bool testUpper(const MojDbQuery::WhereClause& clause, const MojObject& val);

	MojDbQuery::WhereMap m_clauses;
};

#endif /* MOJDBQUERYFILTER_H_ */
