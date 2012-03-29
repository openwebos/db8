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


#include "db/MojDbQuery.h"
#include "db/MojDbUtils.h"
#include "core/MojObjectBuilder.h"

const MojChar* const MojDbQuery::SelectKey = _T("select");
const MojChar* const MojDbQuery::FromKey = _T("from");
const MojChar* const MojDbQuery::WhereKey = _T("where");
const MojChar* const MojDbQuery::FilterKey = _T("filter");
const MojChar* const MojDbQuery::OrderByKey = _T("orderBy");
const MojChar* const MojDbQuery::DescKey = _T("desc");
const MojChar* const MojDbQuery::IncludeDeletedKey = _T("incDel");
const MojChar* const MojDbQuery::LimitKey = _T("limit");
const MojChar* const MojDbQuery::PageKey = _T("page");
const MojChar* const MojDbQuery::PropKey = _T("prop");
const MojChar* const MojDbQuery::OpKey = _T("op");
const MojChar* const MojDbQuery::ValKey = _T("val");
const MojChar* const MojDbQuery::CollateKey = _T("collate");
const MojChar* const MojDbQuery::DelKey = _T("_del");

MojLogger MojDbQuery::s_log(_T("db.query"));

MojDbQuery::WhereClause::WhereClause()
: m_lowerOp(OpNone),
  m_upperOp(OpNone),
  m_collation(MojDbCollationInvalid)
{
}

MojErr MojDbQuery::WhereClause::setLower(CompOp op, const MojObject& val, MojDbCollationStrength coll)
{
	if (m_lowerOp != OpNone)
		MojErrThrow(MojErrDbInvalidQueryOpCombo);

	MojErr err = collation(coll);
	MojErrCheck(err);

	m_lowerOp = op;
	m_lowerVal = val;

	return MojErrNone;
}

MojErr MojDbQuery::WhereClause::setUpper(CompOp op, const MojObject& val, MojDbCollationStrength coll)
{
	if (m_upperOp != OpNone)
		MojErrThrow(MojErrDbInvalidQueryOpCombo);

	MojErr err = collation(coll);
	MojErrCheck(err);

	m_upperOp = op;
	m_upperVal = val;

	return MojErrNone;
}

MojErr MojDbQuery::WhereClause::collation(MojDbCollationStrength collation)
{
	if (m_collation != MojDbCollationInvalid && collation != m_collation)
		MojErrThrow(MojErrDbInvalidQueryCollationMismatch);
	m_collation = collation;

	return MojErrNone;
}

bool MojDbQuery::WhereClause::operator==(const WhereClause& rhs) const
{
	return (m_lowerOp == rhs.m_lowerOp &&
			m_upperOp == rhs.m_upperOp &&
			m_lowerVal == rhs.m_lowerVal &&
			m_upperVal == rhs.m_upperVal);
}

MojErr MojDbQuery::Page::fromObject(const MojObject& obj)
{
	MojString str;
	MojErr err = obj.stringValue(str);
	MojErrCheck(err);
	err = str.base64Decode(m_key.byteVec());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::Page::toObject(MojObject& objOut) const
{
	MojString str;
	MojErr err = str.base64Encode(m_key.byteVec(), false);
	MojErrCheck(err);
	objOut = str;

	return MojErrNone;
}

MojDbQuery::MojDbQuery()
{
	init();
}

MojDbQuery::~MojDbQuery()
{
}

MojErr MojDbQuery::toObject(MojObject& objOut) const
{
	MojObjectBuilder builder;
	MojErr err = toObject(builder);
	MojErrCheck(err);
	objOut = builder.object();
	return MojErrNone;
}

MojErr MojDbQuery::toObject(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.beginObject();
	MojErrCheck(err);

	if (!m_selectProps.empty()) {
		err = visitor.propName(SelectKey, MojStrLen(SelectKey));
		MojErrCheck(err);
		err = visitor.beginArray();
		MojErrCheck(err);
		for (StringSet::ConstIterator i = m_selectProps.begin(); i != m_selectProps.end(); i++) {
			err = visitor.stringValue(*i, i->length());
			MojErrCheck(err);
		}
		err = visitor.endArray();
		MojErrCheck(err);
	}

	err = visitor.stringProp(FromKey, m_fromType);
	MojErrCheck(err);

	if (!m_whereClauses.empty()) {
		err = appendClauses(visitor, WhereKey, m_whereClauses);
		MojErrCheck(err);
	}

	if (!m_filterClauses.empty()) {
		err = appendClauses(visitor, WhereKey, m_whereClauses);
		MojErrCheck(err);
	}

	if (!m_orderProp.empty()) {
		err = visitor.stringProp(OrderByKey, m_orderProp);
		MojErrCheck(err);
	}

	if (m_desc) {
		err = visitor.boolProp(DescKey, true);
		MojErrCheck(err);
	}

	if (m_limit != LimitDefault) {
		err = visitor.intProp(LimitKey, m_limit);
		MojErrCheck(err);
	}

	if (!m_page.empty()) {
		MojObject obj;
		err = m_page.toObject(obj);
		MojErrCheck(err);
		err = visitor.objectProp(PageKey, obj);
		MojErrCheck(err);
	}

	err = visitor.endObject();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::fromObject(const MojObject& obj)
{
	// TODO: validate against query schema
	// select
	MojObject array;
	MojString str;
	if (obj.get(SelectKey, array)) {
		if(array.empty()) {
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: select clause but no selected properties"));
		}
		MojObject prop;
		MojSize i = 0;
		while (array.at(i++, prop)) {
			MojErr err = prop.stringValue(str);
			MojErrCheck(err);
			err = select(str);
			MojErrCheck(err);
		}
	}
	// from
	MojErr err = obj.getRequired(FromKey, str);
	MojErrCheck(err);
	err = from(str);
	MojErrCheck(err);
	// where
	if (obj.get(WhereKey, array)) {
		err = addClauses(m_whereClauses, array);
		MojErrCheck(err);
	}
	// filter
	if (obj.get(FilterKey, array)) {
		err = addClauses(m_filterClauses, array);
		MojErrCheck(err);
	}
	// order
	bool found = false;
	err = obj.get(OrderByKey, str, found);
	MojErrCheck(err);
	if (found) {
		err = order(str);
		MojErrCheck(err);
	}
	// desc
	bool descVal;
	if (obj.get(DescKey, descVal)) {
		desc(descVal);
	}
	// limit
	MojInt64 lim;
	if (obj.get(LimitKey, lim)) {
		if (lim < 0)
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: negative query limit"));
	} else {
		lim = LimitDefault;
	}
	limit((MojUInt32) lim);
	// page
	MojObject pageObj;
	if (obj.get(PageKey, pageObj)) {
		Page pagec;
		err = pagec.fromObject(pageObj);
		MojErrCheck(err);
		page(pagec);
	}
	bool incDel = false;
	if (obj.get(IncludeDeletedKey, incDel) && incDel) {
		err = includeDeleted();
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojDbQuery::clear()
{
	init();
	m_fromType.clear();
	m_selectProps.clear();
	m_whereClauses.clear();
	m_filterClauses.clear();
	m_page.clear();
}

MojErr MojDbQuery::select(const MojChar* propName)
{
	MojString str;
	MojErr err = str.assign(propName);
	MojErrCheck(err);
	err = m_selectProps.put(str);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::from(const MojChar* type)
{
	MojErr err = m_fromType.assign(type);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::where(const MojChar* propName, CompOp op, const MojObject& val, MojDbCollationStrength coll)
{
	MojErr err = addClause(m_whereClauses, propName, op, val, coll);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::filter(const MojChar* propName, CompOp op, const MojObject& val)
{
	MojErr err = addClause(m_filterClauses, propName, op, val, MojDbCollationInvalid);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::order(const MojChar* propName)
{
	MojErr err = m_orderProp.assign(propName);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::includeDeleted(bool val)
{
	if (val) {
		MojObject array;
		MojErr err = array.push(false);
		MojErrCheck(err);
		err = array.push(true);
		MojErrCheck(err);
		// TODO: Move string constants to single location
		err = where(DelKey, MojDbQuery::OpEq, array);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuery::validate() const
{
	bool hasInequalityOp = false;
	bool hasArrayVal = false;
	for (WhereMap::ConstIterator i = m_whereClauses.begin(); i != m_whereClauses.end(); ++i) {
		// verify that we only have inequality op on one prop
		if (i->lowerOp() != OpEq) {
			if (hasInequalityOp)
				MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query contains inequality operations on multiple properties"));
			hasInequalityOp = true;
		}
		// verify that we only have one array val and it's on an = or % op
		if (i->lowerVal().type() == MojObject::TypeArray && i.key() != DelKey) {
			if (hasArrayVal)
				MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query contains array values on multiple properties"));
			hasArrayVal = true;
		}
	}
	for (WhereMap::ConstIterator i = m_filterClauses.begin(); i != m_filterClauses.end(); ++i) {
		// verify that we only have inequality op on one prop
		if (i->lowerOp() == OpPrefix) {
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query contains prefix operator in filter"));
		}
		if (i->lowerOp() == OpSearch) {
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query contains search operator in filter"));
		}
	}
	return MojErrNone;
}

MojErr MojDbQuery::validateFind() const
{
	for (WhereMap::ConstIterator i = m_whereClauses.begin(); i != m_whereClauses.end(); ++i) {
		// if clause is order-defining, verify that it matches the order specified
		if (i->definesOrder() && !(m_orderProp.empty() || m_orderProp == i.key()))
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query order not compatible with where clause"));
		// disallow search operator
		if (i->lowerOp() == OpSearch) {
			MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: search operator not allowed in find"));
		}
	}
	if (!m_filterClauses.empty()) {
		MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: filter not allowed in find"));
	}
	return MojErrNone;
}

bool MojDbQuery::operator==(const MojDbQuery& rhs) const
{
	if (m_fromType != rhs.m_fromType ||
		m_selectProps != rhs.m_selectProps ||
	    m_whereClauses != rhs.m_whereClauses ||
	    m_page.key() != rhs.m_page.key() ||
		m_orderProp != rhs.m_orderProp ||
		m_limit != rhs.m_limit ||
		m_desc != rhs.m_desc) {
		return false;
	}
	return true;
}

void MojDbQuery::init()
{
	m_limit = LimitDefault;
	m_desc = false;
	m_forceIndex = NULL;
}

MojErr MojDbQuery::addClauses(WhereMap& map, const MojObject& array)
{
	MojObject clause;
	MojSize i = 0;
	while (array.at(i++, clause)) {
		MojString prop;
		MojErr err = clause.getRequired(PropKey, prop);
		MojErrCheck(err);
		MojString opStr;
		err = clause.getRequired(OpKey, opStr);
		MojErrCheck(err);
		CompOp op = OpNone;
		err = stringToOp(opStr, op);
		MojErrCheck(err);
		MojObject val;
		err = clause.getRequired(ValKey, val);
		MojErrCheck(err);
		MojDbCollationStrength coll = MojDbCollationInvalid;
		MojString collateStr;
		bool found = false;
		err = clause.get(CollateKey, collateStr, found);
		MojErrCheck(err);
		if (found) {
			err = MojDbUtils::collationFromString(collateStr, coll);
			MojErrCheck(err);
		}
		err = addClause(map, prop, op, val, coll);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuery::addClause(WhereMap& map, const MojChar* propName, CompOp op, const MojObject& val, MojDbCollationStrength coll)
{
	MojAssert(propName);

	// only allow valid ops
	if (!(op >= OpEq && op <= OpSearch))
		MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: invalid query op"));
	// only allow array values for = or % ops
	if (val.type() == MojObject::TypeArray && op != OpEq && op != OpPrefix)
		MojErrThrowMsg(MojErrDbInvalidQuery, _T("db: query contains array value for non-eq op"));

	// check to see if the prop is referenced in a prior clause
	WhereMap::Iterator iter;
	MojErr err = map.find(propName, iter);
	MojErrCheck(err);
	if (iter == map.end()) {
		// create new clause
		err = createClause(map, propName, op, val, coll);
		MojErrCheck(err);
	} else {
		// add clause to previously referenced prop.
		err = updateClause(iter.value(), op, val, coll);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbQuery::createClause(WhereMap& map, const MojChar* propName, CompOp op, MojObject val, MojDbCollationStrength coll)
{
	// construct the clause
	MojErr err = MojErrNone;
	WhereClause clause;
	switch (op) {
	case OpLessThan:
	case OpLessThanEq:
		// less-than ops define upper bound
		err = clause.setUpper(op, val, coll);
		MojErrCheck(err);
		break;
	default:
		// everything else is stored as lower bound
		err = clause.setLower(op, val, coll);
		MojErrCheck(err);
		break;
	}
	// add clause to map
	MojString str;
	err = str.assign(propName);
	MojErrCheck(err);
	err = map.put(str, clause);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::updateClause(WhereClause& clause, CompOp op, const MojObject& val, MojDbCollationStrength coll)
{
	// the only case where we can have two ops for the same prop is a combination
	// of '<' or '<=' with '>' or '>='
	switch (op) {
	case OpLessThan:
	case OpLessThanEq: {
		CompOp lowerOp = clause.lowerOp();
		if (lowerOp != OpNone && lowerOp != OpGreaterThan && lowerOp != OpGreaterThanEq)
			MojErrThrow(MojErrDbInvalidQueryOpCombo);
		MojErr err = clause.setUpper(op, val, coll);
		MojErrCheck(err);
		break;
	}
	case OpGreaterThan:
	case OpGreaterThanEq: {
		CompOp upperOp = clause.upperOp();
		if (upperOp != OpNone && upperOp != OpLessThan && upperOp != OpLessThanEq)
			MojErrThrow(MojErrDbInvalidQueryOpCombo);
		MojErr err = clause.setLower(op, val, coll);
		MojErrCheck(err);
		break;
	}
	default:
		// all other ops invalid in combination
		MojErrThrow(MojErrDbInvalidQueryOpCombo);
	}
	return MojErrNone;
}

const MojDbQuery::StrOp MojDbQuery::s_ops[] = {
	{_T("="), OpEq},
	{_T("!="), OpNotEq},
	{_T("<"), OpLessThan},
	{_T("<="), OpLessThanEq},
	{_T(">"), OpGreaterThan},
	{_T(">="), OpGreaterThanEq},
	{_T("%"), OpPrefix},
	{_T("?"), OpSearch},
	{NULL, OpNone}
};

MojErr MojDbQuery::stringToOp(const MojChar* str, CompOp& opOut)
{
	for (const StrOp* op = s_ops; op->m_str != NULL; ++op) {
		if (MojStrCmp(op->m_str, str) == 0) {
			opOut = op->m_op;
			return MojErrNone;
		}
	}
	MojErrThrow(MojErrDbInvalidQueryOp);
}

const MojChar* MojDbQuery::opToString(CompOp op)
{
	if (op == OpNone || op > OpPrefix)
		return NULL;
	return s_ops[op-1].m_str;
}

MojErr MojDbQuery::appendClauses(MojObjectVisitor& visitor, const MojChar* propName, const WhereMap& map)
{
	MojErr err = visitor.propName(propName);
	MojErrCheck(err);
	err = visitor.beginArray();
	MojErrCheck(err);
	for (WhereMap::ConstIterator i = map.begin(); i != map.end(); i++) {
		if (i->lowerOp() != OpNone) {
			err = appendClause(visitor, i.key(), i->lowerOp (), i->lowerVal(), i->collation());
			MojErrCheck(err);
		}
		if (i->upperOp() != OpNone) {
			err = appendClause(visitor, i.key(), i->upperOp (), i->upperVal(), i->collation());
			MojErrCheck(err);
		}
	}
	err = visitor.endArray();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQuery::appendClause(MojObjectVisitor& visitor, const MojChar* propName, CompOp op,
 							   const MojObject& val, MojDbCollationStrength coll)
{
	MojAssert(propName);

	MojErr err = visitor.beginObject();
	MojErrCheck(err);

	err = visitor.stringProp(PropKey, propName);
	MojErrCheck(err);
	const MojChar* opStr = opToString(op);
	if (!opStr) {
		MojErrThrow(MojErrDbInvalidQueryOp);
	}
	err = visitor.stringProp(OpKey, opStr);
	MojErrCheck(err);
	err = visitor.objectProp(ValKey, val);
	MojErrCheck(err);
	if (coll != MojDbCollationInvalid) {
		err = visitor.stringProp(CollateKey, MojDbUtils::collationToString(coll));
		MojErrCheck(err);
	}
	err = visitor.endObject();
	MojErrCheck(err);

	return MojErrNone;
}
