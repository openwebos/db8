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


#include "db/MojDbQueryFilter.h"

MojDbQueryFilter::MojDbQueryFilter()
{
}

MojDbQueryFilter::~MojDbQueryFilter()
{
}

MojErr MojDbQueryFilter::init(const MojDbQuery& query)
{
	m_clauses = query.filter();

	return MojErrNone;
}

bool MojDbQueryFilter::test(const MojObject& obj) const
{
	for (MojDbQuery::WhereMap::ConstIterator i = m_clauses.begin(); i != m_clauses.end(); ++i) {
		MojObject val;
		if (!obj.get(i.key(), val))
			return false;

		if (!(testLower(*i, val) && testUpper(*i, val))) {
			return false;
		}
	}
	return true;
}

bool MojDbQueryFilter::testLower(const MojDbQuery::WhereClause& clause, const MojObject& val)
{
	const MojObject& lowerVal = clause.lowerVal();
	switch (clause.lowerOp()) {
	case MojDbQuery::OpNone:
		return true;

	case MojDbQuery::OpEq:
		if (val.type() == MojObject::TypeArray) {
			MojObject::ConstArrayIterator end = lowerVal.arrayEnd();
			for (MojObject::ConstArrayIterator i = lowerVal.arrayBegin(); i != end; ++i) {
				if (val == *i)
					return true;
			}
			return false;
		} else {
			return val == lowerVal;
		}

	case MojDbQuery::OpNotEq:
		return val != lowerVal;

	case MojDbQuery::OpGreaterThan:
		return val > lowerVal;

	case MojDbQuery::OpGreaterThanEq:
		return val >= lowerVal;

	default:
		MojAssertNotReached();
		return false;
	}
}

bool MojDbQueryFilter::testUpper(const MojDbQuery::WhereClause& clause, const MojObject& val)
{
	const MojObject& upperVal = clause.upperVal();
	switch (clause.upperOp()) {
	case MojDbQuery::OpNone:
		return true;

	case MojDbQuery::OpLessThan:
		return val < upperVal;

	case MojDbQuery::OpLessThanEq:
		return val <= upperVal;

	default:
		MojAssertNotReached();
		return false;
	}
}
