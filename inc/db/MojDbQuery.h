/* @@@LICENSE
*
*  Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*  Copyright (c) 2013 LG Electronics
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


#ifndef MOJDBQUERY_H_
#define MOJDBQUERY_H_

#include "db/MojDbDefs.h"
#include "db/MojDbKey.h"
#include "core/MojHashMap.h"
#include "core/MojObject.h"
#include "core/MojSet.h"
#include "core/MojString.h"

class MojDbQuery
{
public:
	static const guint32 LimitDefault = G_MAXUINT32;

	typedef enum {
		OpNone,
		OpEq,
		OpNotEq,
		OpLessThan,
		OpLessThanEq,
		OpGreaterThan,
		OpGreaterThanEq,
		OpPrefix,
		OpSearch
	} CompOp;

	class WhereClause
	{
	public:
		WhereClause();

		MojErr setLower(CompOp op, const MojObject& val, MojDbCollationStrength coll);
		MojErr setUpper(CompOp op, const MojObject& val, MojDbCollationStrength coll);

		MojDbCollationStrength collation() const { return m_collation; }
		CompOp lowerOp() const { return m_lowerOp; }
		CompOp upperOp() const { return m_upperOp; }
		const MojObject& lowerVal() const { return m_lowerVal; }
		const MojObject& upperVal() const { return m_upperVal; }
		bool definesOrder() const { return m_lowerOp != OpEq || m_lowerVal.type() == MojObject::TypeArray; }
		bool operator==(const WhereClause& rhs) const;

	private:
		MojErr collation(MojDbCollationStrength collation);

		CompOp m_lowerOp;
		CompOp m_upperOp;
		MojObject m_lowerVal;
		MojObject m_upperVal;
		MojDbCollationStrength m_collation;
	};

	class Page
	{
	public:
		void clear() { m_key.clear(); }
		MojErr fromObject(const MojObject& obj);
		MojErr toObject(MojObject& objOut) const;
		bool empty() const { return m_key.empty(); }

		const MojDbKey& key() const { return m_key; }
		MojDbKey& key() { return m_key; }

	private:
		MojDbKey m_key;
	};

	typedef MojHashMap<MojString, WhereClause, const MojChar*> WhereMap;
	typedef MojSet<MojString> StringSet;

	static const MojChar* const SelectKey;
	static const MojChar* const FromKey;
	static const MojChar* const WhereKey;
	static const MojChar* const FilterKey;
	static const MojChar* const OrderByKey;
	static const MojChar* const DistinctKey;
	static const MojChar* const DescKey;
	static const MojChar* const IncludeDeletedKey;
	static const MojChar* const LimitKey;
	static const MojChar* const PageKey;
	static const MojChar* const PropKey;
	static const MojChar* const OpKey;
	static const MojChar* const ValKey;
	static const MojChar* const CollateKey;
	static const MojChar* const DelKey;
	static const guint32 MaxQueryLimit = 500;

	MojDbQuery();
	~MojDbQuery();

	MojErr fromObject(const MojObject& obj);
	MojErr toObject(MojObject& objOut) const;
	MojErr toObject(MojObjectVisitor& visitor) const;

	void clear();
	MojErr select(const MojChar* propName);
	MojErr from(const MojChar* type);
	MojErr where(const MojChar* propName, CompOp op, const MojObject& val, MojDbCollationStrength coll = MojDbCollationInvalid);
	MojErr filter(const MojChar* propName, CompOp op, const MojObject& val);
	MojErr order(const MojChar* propName);
	MojErr distinct(const MojChar* distinct);
	MojErr includeDeleted(bool val = true);
	void desc(bool val) { m_desc = val; }
	void limit(guint32 numResults) { m_limit = numResults; }
	void page(const Page& page) { m_page = page; }

	MojErr validate() const;
	MojErr validateFind() const;
	const StringSet& select() const { return m_selectProps; }
	const MojString& from() const { return m_fromType; }
	const WhereMap& where() const { return m_whereClauses; }
	const WhereMap& filter() const { return m_filterClauses; }
	const MojString& order() const { return m_orderProp; }
	const MojString& distinct() const { return m_distinct; }
	const Page& page() const { return m_page; }
	bool desc() const { return m_desc; }
	guint32 limit() const { return m_limit; }
	bool operator==(const MojDbQuery& rhs) const;

	static MojErr stringToOp(const MojChar* str, CompOp& opOut);

private:
	friend class MojDbQueryPlan;
	friend class MojDbKind;
	friend class MojDbIndex;

	struct StrOp
	{
		const MojChar* const m_str;
		const CompOp m_op;
	};
	static const StrOp s_ops[];

	void init();
	static MojErr addClauses(WhereMap& map, const MojObject& array);
	static MojErr addClause(WhereMap& map, const MojChar* propName, CompOp op, const MojObject& val, MojDbCollationStrength coll);
	static MojErr createClause(WhereMap& map, const MojChar* propName, CompOp op, MojObject val, MojDbCollationStrength coll);
	static MojErr updateClause(WhereClause& clause, CompOp op, const MojObject& val, MojDbCollationStrength coll);
	static const MojChar* opToString(CompOp op);
	static MojErr appendClauses(MojObjectVisitor& visitor, const MojChar* propName, const WhereMap& map);
	static MojErr appendClause(MojObjectVisitor& visitor, const MojChar* propName, CompOp op,
							  const MojObject& val, MojDbCollationStrength coll);

	MojString m_fromType;
	StringSet m_selectProps;
	WhereMap m_whereClauses;
	WhereMap m_filterClauses;
	Page m_page;
	MojString m_orderProp;
	MojString m_distinct;
	guint32 m_limit;
	bool m_desc;

	static MojLogger s_log;
	MojDbIndex *m_forceIndex;		// for debugging in stats
};

#endif /* MOJDBQUERY_H_ */
