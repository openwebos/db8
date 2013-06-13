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


#include "MojDbQueryFilterTest.h"
#include "db/MojDbQueryFilter.h"

MojDbQueryFilterTest::MojDbQueryFilterTest()
: MojTestCase(_T("MojDbQueryFilter"))
{
}

MojErr MojDbQueryFilterTest::run()
{
	MojErr err = MojErrNone;
	// =
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\"=\",\"val\":1}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\"=\",\"val\":1}]"),
				false);
	MojTestErrCheck(err);
	err = check(_T("{}"),
				_T("[{\"prop\":\"foo\",\"op\":\"=\",\"val\":1}]"),
				false);
	MojTestErrCheck(err);
	// !=
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\"!=\",\"val\":1}]"),
				false);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\"!=\",\"val\":1}]"),
				true);
	MojTestErrCheck(err);
	// <
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<\",\"val\":2}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<\",\"val\":2}]"),
				false);
	MojTestErrCheck(err);
	// <=
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<=\",\"val\":1}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<=\",\"val\":1}]"),
				false);
	MojTestErrCheck(err);
	// >
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":2}]"),
				false);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":1}]"),
				true);
	MojTestErrCheck(err);
	// >=
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\">=\",\"val\":1}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\">=\",\"val\":3}]"),
				false);
	MojTestErrCheck(err);
	// <
	err = check(_T("{\"foo\":1}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<\",\"val\":2}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":2}"),
				_T("[{\"prop\":\"foo\",\"op\":\"<\",\"val\":2}]"),
				false);
	MojTestErrCheck(err);
	// > <
	err = check(_T("{\"foo\":3}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":1},{\"prop\":\"foo\",\"op\":\"<\",\"val\":5}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":5}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":1},{\"prop\":\"foo\",\"op\":\"<\",\"val\":5}]"),
				false);
	MojTestErrCheck(err);
	// foo and bar
	err = check(_T("{\"foo\":3,\"bar\":3}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":1},{\"prop\":\"bar\",\"op\":\"<\",\"val\":5}]"),
				true);
	MojTestErrCheck(err);
	err = check(_T("{\"foo\":3,\"bar\":6}"),
				_T("[{\"prop\":\"foo\",\"op\":\">\",\"val\":1},{\"prop\":\"bar\",\"op\":\"<\",\"val\":5}]"),
				false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbQueryFilterTest::check(const MojChar* objJson, const MojChar* filterJson, bool expected)
{
	MojObject obj;
	MojErr err = obj.fromJson(objJson);
	MojTestErrCheck(err);
	MojObject filterObj;
	err = filterObj.fromJson(filterJson);
	MojTestErrCheck(err);
	MojObject queryObj;
	err = queryObj.putString(_T("from"), _T("com.test.foo:1"));
	MojTestErrCheck(err);
	err = queryObj.put(_T("filter"), filterObj);
	MojTestErrCheck(err);
	MojDbQuery query;
	err = query.fromObject(queryObj);
	MojTestErrCheck(err);
	MojDbQueryFilter filter;
	err = filter.init(query);
	MojTestErrCheck(err);
	MojTestAssert(filter.test(obj) == expected);

	return MojErrNone;
}
