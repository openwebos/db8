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


#include "MojJsonTest.h"
#include "core/MojJson.h"

MojJsonTest::MojJsonTest()
: MojTestCase("MojJson")
{
}

MojErr MojJsonTest::run()
{
	const MojChar* empty = _T("{}");
	const MojChar* allTypes = _T("{\"a\":{},\"b\":[],\"c\":null,\"d\":\"hello\",\"e\":123,\"f\":-45678900000")
		_T(",\"g\":7.89,\"h\":0.00042,\"i\":true,\"j\":false,\"k\":{\"a\":73},\"l\":[1,2.34,true,false,null,[]")
		_T(",[1,2,3],{},{\"1\":1}]}");
	const MojChar* allTypesWhitespace = _T(" {\t\"a\"\r: /**/   { \t}\f,/*hello**/ \"b\"\r\n:/*hi*/\n[ ]  ,   \"c\" :  null\n\n\n\n,\t\t\t\"d\" :  \"hello\" ,  \"e\" :  123 , \"f\"  : -45678900000")
		_T(" , \"g\"\r\r:  7.89 ,\f\"h\" : 4.2e-4 ,  \"i\" :  true ,  \"j\" :  false , \"k\" : { \"a\" : 73 }  , \"l\" :  [ 1 , 2.34 , true  , false , null , [ ]")
		_T(" , [ 1 , 2 , 3], { } , { \"1\" : 1 } ] }    ");
	const MojChar* nullStr = _T("null");
	const MojChar* intStr1 = _T("8");
	const MojChar* intStr2 = _T("8");
	const MojChar* boolStrTrue = _T("true");
	const MojChar* boolStrFalse = _T("false");
	const MojChar* MojDoubleStr1 = _T("-2.6e0");
	const MojChar* MojDoubleStr2 = _T("-2.6");
	const MojChar* MojDoubleStr3 = _T("2.6e-1");
	const MojChar* MojDoubleStr4 = _T("0.26");
	const MojChar* escapeStr1 = _T("\"\\bh\\r\\ne\\tll\\u0020\\u001F\\u0001\\\\o\\/\\\"\"");
	const MojChar* escapeStr2 = _T("\"\\bh\\r\\ne\\tll \\u001F\\u0001\\\\o/\\\"\"");
	const MojChar  negativeChars[] = {'"',(MojChar) 0xE2, (MojChar) 0x80, (MojChar) 0x99,'"',0x0};

	MojErr err = test(empty, empty);
	MojTestErrCheck(err);
	err = test(allTypes, allTypes);
	MojTestErrCheck(err);
	err = test(allTypesWhitespace, allTypes);
	MojTestErrCheck(err);
	err = test(nullStr, nullStr);
	MojTestErrCheck(err);
	err = test(intStr1, intStr2);
	MojTestErrCheck(err);
	err = test(boolStrTrue, boolStrTrue);
	MojTestErrCheck(err);
	err = test(boolStrFalse, boolStrFalse);
	MojTestErrCheck(err);
	err = test(MojDoubleStr1, MojDoubleStr2);
	MojTestErrCheck(err);
	err = test(MojDoubleStr3, MojDoubleStr4);
	MojTestErrCheck(err);
	err = test(escapeStr1, escapeStr2);
	MojTestErrCheck(err);
	err = test(negativeChars, negativeChars);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojJsonTest::test(const MojChar* str, const MojChar* expected)
{
	MojJsonWriter writer;
	MojObject obj1;
	MojObject obj2;

	MojErr err = MojJsonParser::parse(writer, str);
	MojTestErrCheck(err);
	MojTestAssert(writer.json() == expected);

	err = obj1.fromJson(str);
	MojTestErrCheck(err);
	err = obj2.fromJson(expected);
	MojTestErrCheck(err);
	MojTestAssert(obj1 == obj2);

	return MojErrNone;
}
