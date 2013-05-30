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


#ifndef MOJDBQUERYPLAN_H_
#define MOJDBQUERYPLAN_H_

#include "db/MojDbDefs.h"
#include "db/MojDbQuery.h"
#include "db/MojDbKey.h"

class MojDbQueryPlan : private MojNoCopy
{
public:
	typedef MojVector<MojDbKeyRange> RangeVec;
	typedef MojVector<MojString> StringVec;

	MojDbQueryPlan(MojDbKindEngine& kindEngine);
	~MojDbQueryPlan();

	MojErr init(const MojDbQuery& query, const MojDbIndex& index);
	void limit(guint32 val) { m_query.limit(val); }
	void desc(bool val) { m_query.desc(val); }

	const RangeVec& ranges() const { return m_ranges; }
	const MojDbKey& pageKey() const { return m_query.page().key(); }
	guint32 groupCount() const { return m_groupCount; }
	guint32 limit() const { return m_query.limit(); }
	gsize idIndex() const { return m_idPropIndex; }
	bool desc() const { return m_query.desc(); }
	const MojDbQuery& query() const { return m_query; }
	MojDbKindEngine& kindEngine() const { return m_kindEngine; }

private:
	typedef MojSet<MojDbKey> KeySet;
	typedef MojVector<guint8> ByteVec;

	MojErr buildRanges(const MojDbIndex& index);
	MojErr rangesFromKeySets(const KeySet& lowerKeys, const KeySet& upperKeys, const KeySet& prefixKeys,
			const MojDbQuery::WhereClause* clause);
	MojErr rangesFromKeys(MojDbKey lowerKey, MojDbKey upperKey, MojDbKey prefix, guint32 index,
			const MojDbQuery::WhereClause* clause);
	MojErr addRange(const MojDbKey& lowerKey, const MojDbKey& upperKey, guint32 group = 0);
	MojErr pushSearch(MojDbKeyBuilder& lowerBuilder, MojDbKeyBuilder& upperBuilder, const MojObject& val, MojDbTextCollator* collator);
	static MojErr pushVal(MojDbKeyBuilder& builder, const MojObject& val, MojDbTextCollator* collator);

	RangeVec m_ranges;
	MojDbQuery m_query;
	MojString m_locale;
	gsize m_idPropIndex;
	guint32 m_groupCount;
	MojDbKindEngine& m_kindEngine;
};

#endif /* MOJDBQUERYPLAN_H_	 */
