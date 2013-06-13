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


#ifndef MOJDBQUERYTEST_H_
#define MOJDBQUERYTEST_H_

#include "MojDbTestRunner.h"
#include "db/MojDbQuery.h"

class MojDbQueryTest : public MojTestCase
{
public:
	MojDbQueryTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	typedef MojSet<MojObject> ObjectSet;

	MojErr basicTest();
	MojErr eqTest(MojDb& db);
	MojErr neqTest(MojDb& db);
	MojErr ltTest(MojDb& db);
	MojErr lteTest(MojDb& db);
	MojErr gtTest(MojDb& db);
	MojErr gteTest(MojDb& db);
	MojErr gtltTest(MojDb& db);
	MojErr limitTest(MojDb& db);
	MojErr countTest(MojDb& db);
	MojErr stringTest(MojDb& db);
	MojErr multiTest(MojDb& db);
	MojErr pageTest(MojDb& db);
	MojErr idTest(MojDb& db);
	MojErr kindTest(MojDb& db);
	MojErr orderTest(MojDb& db);
	MojErr dupTest(MojDb& db);
	MojErr delTest(MojDb& db);
	MojErr isolationTest(MojDb& db);
	MojErr invalidTest();
	MojErr serializationTest();
	MojErr basicFromObject();
	MojErr basicToObject();
	MojErr optionsFromObect();
	MojErr optionsToObect();
	MojErr complexClauseFromObject();
	MojErr complexClauseToObject();

	MojErr put(MojDb& db);
	MojErr put2(MojDb& db, const MojObject& val1, const MojChar* val2);
	MojErr put3(MojDb& db, const MojObject& val1, const MojChar* val2);
	MojErr put4(MojDb& db, const MojObject& val1, const MojChar* val2);
	template<class COMP>
	MojErr check(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
			const COMP& comp, const MojObject& expectedVal2, MojSize expectedCount, MojUInt32 limit = MojUInt32Max);
	template <class COMP>
	MojErr check(MojDb& db, const MojChar* kind, const MojChar* prop1, const MojObject& expectedVal1,
			const MojChar* prop2, MojDbQuery::CompOp op,
			const COMP& comp, const MojObject& expectedVal2, MojSize expectedCount, MojUInt32 limit = MojUInt32Max);
	template <class COMP>
	MojErr checkVal(const MojObject& obj, const MojChar* prop, const MojObject expectedVal, const COMP& comp);
	template<class COMP>
	MojErr checkOrder(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
			const COMP& comp, const MojObject& expectedVal, MojSize expectedCount, const MojChar* order, bool desc = false);
	template<class COMP>
	MojErr checkPage(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
			const COMP& comp, const MojObject& expectedVal, MojSize expectedCount, const MojChar* order, bool desc = false);
	MojErr checkRange(MojDb& db, const MojChar* prop, MojDbQuery::CompOp opLower, MojDbQuery::CompOp opUpper,
			const MojObject& valLower, const MojObject& valUpper, MojSize expectedCount);
	MojErr checkRange(MojDb& db, const MojChar* prop1, const MojObject& val1,
			const MojChar* prop2, MojDbQuery::CompOp lowerOp, MojDbQuery::CompOp upperOp,
			const MojObject& lowerVal, const MojObject& upperVal, MojSize expectedCount);
	MojErr checkCount(MojDb& db, const MojChar* kind, const MojChar* prop, MojDbQuery::CompOp op,
			const MojObject& expectedVal, MojSize expectedCount, MojUInt32 limit = MojUInt32Max);
	MojErr checkKind(MojDb& db, const MojChar* kind, MojSize expectedCount, const MojChar* orderBy = NULL);
	MojErr checkInvalid(const MojChar* queryJson, MojErr expectedErr);

	ObjectSet m_ids;
	MojObject m_id0;
};

#endif /* MOJDBQUERYTEST_H_ */
