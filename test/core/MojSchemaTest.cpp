/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics, Inc.
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


#include "MojSchemaTest.h"
#include "core/MojSchema.h"

MojSchemaTest::MojSchemaTest()
: MojTestCase(_T("MojSchema"))
{
}

MojErr MojSchemaTest::run()
{
	MojErr err = typeTest();
	MojTestErrCheck(err);
	err = disallowTest();
	MojTestErrCheck(err);
	err = propertiesTest();
	MojTestErrCheck(err);
	err = itemsTest();
	MojTestErrCheck(err);
	err = requiresTest();
	MojTestErrCheck(err);
	err = minmaxTest();
	MojTestErrCheck(err);
	err = arrayTest();
	MojTestErrCheck(err);
	err = stringTest();
	MojTestErrCheck(err);
	err = enumTest();
	MojTestErrCheck(err);
	err = divisibleTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::typeTest()
{
	// null
	MojErr err = MojErrNone;
	err = checkValid(_T("{\"type\":\"null\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"null\"}"),
					 _T("5"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"null\"}"),
					 _T("{}"),
					 false);
	MojTestErrCheck(err);
	// object
	err = checkValid(_T("{\"type\":\"object\"}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"object\"}"),
					 _T("{\"hello\":\"world\"}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"object\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"object\",\"properties\":{\"foo\":{\"type\":\"object\",\"optional\":true,\"properties\":{\"bar\":{}}}}}"),
					 _T("{\"foo\":null}"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"object\"}"),
					 _T("[]"),
					 false);
	MojTestErrCheck(err);
	// array
	err = checkValid(_T("{\"type\":\"array\"}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"array\"}"),
					 _T("[\"hello\",\"world\"]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"array\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"array\"}"),
					 _T("{}"),
					 false);
	MojTestErrCheck(err);
	// string
	err = checkValid(_T("{\"type\":\"string\"}"),
					 _T("\"\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"string\"}"),
					 _T("\"hello world\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"string\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"string\"}"),
					 _T("{}"),
					 false);
	MojTestErrCheck(err);
	// boolean
	err = checkValid(_T("{\"type\":\"boolean\"}"),
					 _T("true"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"boolean\"}"),
					 _T("false"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"boolean\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"boolean\"}"),
					 _T("0"),
					 false);
	MojTestErrCheck(err);
	// integer
	err = checkValid(_T("{\"type\":\"integer\"}"),
					 _T("0"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"integer\"}"),
					 _T("-26"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"integer\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"integer\"}"),
					 _T("3.0"),
					 false);
	MojTestErrCheck(err);
	// number
	err = checkValid(_T("{\"type\":\"number\"}"),
					 _T("8"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"number\"}"),
					 _T("8.9"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"number\"}"),
					 _T("false"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":\"number\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	// union types
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("8"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("true"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("\"hello\""),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("[]"),
					 false);
	MojTestErrCheck(err);
	// union types with schema
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("8"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("{\"foo\":1}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"type\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("{\"foo\":false}"),
					 false);
	MojTestErrCheck(err);
	// in property
	err = checkValid(_T("{\"properties\":{\"foo\":{\"type\":\"integer\"}}}"),
					 _T("{\"foo\":1}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{\"type\":\"integer\"}}}"),
					 _T("{\"foo\":1.0}"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::disallowTest()
{
	// null
	MojErr err = MojErrNone;
	err = checkValid(_T("{\"disallow\":\"null\"}"),
					 _T("null"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"null\"}"),
					 _T("5"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"null\"}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	// object
	err = checkValid(_T("{\"disallow\":\"object\"}"),
					 _T("{}"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"object\"}"),
					 _T("{\"hello\":\"world\"}"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"object\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"object\"}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	// array
	err = checkValid(_T("{\"disallow\":\"array\"}"),
					 _T("[]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"array\"}"),
					 _T("[\"hello\",\"world\"]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"array\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"array\"}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	// string
	err = checkValid(_T("{\"disallow\":\"string\"}"),
					 _T("\"\""),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"string\"}"),
					 _T("\"hello world\""),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"string\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"string\"}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	// boolean
	err = checkValid(_T("{\"disallow\":\"boolean\"}"),
					 _T("false"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"boolean\"}"),
					 _T("true"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"boolean\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"boolean\"}"),
					 _T("0"),
					 true);
	MojTestErrCheck(err);
	// integer
	err = checkValid(_T("{\"disallow\":\"integer\"}"),
					 _T("0"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"integer\"}"),
					 _T("-26"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"integer\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"integer\"}"),
					 _T("3.0"),
					 true);
	MojTestErrCheck(err);
	// number
	err = checkValid(_T("{\"disallow\":\"number\"}"),
					 _T("8"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"number\"}"),
					 _T("8.9"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"number\"}"),
					 _T("true"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":\"number\"}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	// union disallows
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("8"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("false"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("\"hello\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",\"null\"]}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	// union disallows with schema
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("8"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("{\"foo\":1}"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"disallow\":[\"number\",\"boolean\",{\"properties\":{\"foo\":{\"type\":\"integer\"}}}]}"),
					 _T("{\"foo\":true}"),
					 true);
	MojTestErrCheck(err);
	// in property
	err = checkValid(_T("{\"properties\":{\"foo\":{\"disallow\":\"integer\"}}}"),
					 _T("{\"foo\":1}"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{\"disallow\":\"integer\"}}}"),
					 _T("{\"foo\":1.0}"),
					 true);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::propertiesTest()
{
	MojErr err = MojErrNone;
	// empty properties
	err = checkValid(_T("{\"properties\":{}}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{}}"),
					 _T("{\"foo\":\"bar\"}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{},\"additionalProperties\":false}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{},\"additionalProperties\":false}"),
					 _T("{\"foo\":\"bar\"}"),
					 false);
	MojTestErrCheck(err);
	// optional/required
	err = checkValid(_T("{\"properties\":{\"foo\":{},\"bar\":{\"optional\":true}}}"),
					 _T("{\"foo\":1, \"bar\":2}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{},\"bar\":{\"optional\":true}}}"),
					 _T("{\"foo\":1}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{},\"bar\":{\"optional\":true}}}"),
					 _T("{\"bar\":2}"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::itemsTest()
{
	MojErr err = MojErrNone;
	// non-array
	err = checkValid(_T("{\"items\":{\"type\":\"integer\"}}"),
					 _T("null"),
					 true);
	MojTestErrCheck(err);
	// empty array
	err = checkValid(_T("{\"items\":{\"type\":\"integer\"}}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	// array with values
	err = checkValid(_T("{\"items\":{\"type\":\"integer\"}}"),
					 _T("[1,2,3,4,5,6,7,8]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":{\"type\":\"integer\"}}"),
					 _T("[1,2,3,4,5,6,7,8.0]"),
					 false);
	MojTestErrCheck(err);
	// tuple items
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}]}"),
					 _T("[1,false]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}]}"),
					 _T("[1,false,null]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}]}"),
					 _T("[1,2]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}]}"),
					 _T("[1]"),
					 false);
	MojTestErrCheck(err);
	// additionalProperties
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}],\"additionalProperties\":false}"),
					 _T("[1,false]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}],\"additionalProperties\":false}"),
					 _T("[1,false,2]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}],\"additionalProperties\":{\"type\":\"null\"}}"),
					 _T("[1,false,null,null,null]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"items\":[{\"type\":\"integer\"},{\"type\":\"boolean\"}],\"additionalProperties\":{\"type\":\"null\"}}"),
					 _T("[1,false,null,5,null]"),
					 false);
	MojTestErrCheck(err);
		// in property
	err = checkValid(_T("{\"properties\":{\"foo\":{\"items\":{\"type\":\"integer\"}}}}"),
					 _T("{\"foo\":[1]}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{\"items\":{\"type\":\"integer\"}}}}"),
					 _T("{\"foo\":[true]}"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::requiresTest()
{
	MojErr err = MojErrNone;
	err = checkValid(_T("{\"properties\":{\"foo\":{\"optional\":true,\"requires\":\"bar\"}}}"),
					 _T("{\"foo\":1,\"bar\":2}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{\"optional\":true,\"requires\":\"bar\"}}}"),
					 _T("{}"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"properties\":{\"foo\":{\"optional\":true,\"requires\":\"bar\"}}}"),
					 _T("{\"foo\":1}"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::minmaxTest()
{
	MojErr err = MojErrNone;
	// min
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("2"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("2.0"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("2.1"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("18"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("1.9"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2}"),
					 _T("-4"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2,\"minimumCanEqual\":true}"),
					 _T("2"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2,\"minimumCanEqual\":false}"),
					 _T("2.1"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minimum\":2,\"minimumCanEqual\":false}"),
					 _T("2"),
					 false);
	MojTestErrCheck(err);
	// max
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("2"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("2.0"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("1.9"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("-7"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("2.1"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2}"),
					 _T("3"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2,\"maximumCanEqual\":true}"),
					 _T("2"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2,\"maximumCanEqual\":false}"),
					 _T("1.9"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maximum\":2,\"maximumCanEqual\":false}"),
					 _T("2"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::arrayTest()
{
	MojErr err = MojErrNone;
	// minItems
	err = checkValid(_T("{\"minItems\":5}"),
					 _T("false"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":0}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":1}"),
					 _T("[1]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":1}"),
					 _T("[1,2]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":3}"),
					 _T("[1,2,3]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":1}"),
					 _T("[]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minItems\":2}"),
					 _T("[1]"),
					 false);
	MojTestErrCheck(err);
	// maxItems
	err = checkValid(_T("{\"maxItems\":5}"),
					 _T("false"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":0}"),
					 _T("[]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":1}"),
					 _T("[1]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":2}"),
					 _T("[1,2]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":5}"),
					 _T("[1,2,3]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":0}"),
					 _T("[1]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxItems\":2}"),
					 _T("[1,2,3]"),
					 false);
	MojTestErrCheck(err);
	// uniqueItems
	err = checkValid(_T("{\"uniqueItems\":true}"),
					 _T("false"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"uniqueItems\":false}"),
					 _T("[1,1,1]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"uniqueItems\":true}"),
					 _T("[1]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"uniqueItems\":true}"),
					 _T("[1,2,3,4]"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"uniqueItems\":true}"),
					 _T("[1,1]"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"uniqueItems\":true}"),
					 _T("[1,2,3,4,5,6,7,8,9,0,1]"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::stringTest()
{
	MojErr err = MojErrNone;
	// minLength
	err = checkValid(_T("{\"minLength\":3}"),
					 _T("18"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":0}"),
					 _T("\"\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":0}"),
					 _T("\"hello\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":1}"),
					 _T("\"h\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":1}"),
					 _T("\"hello\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":1}"),
					 _T("\"\""),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"minLength\":6}"),
					 _T("\"hello\""),
					 false);
	MojTestErrCheck(err);
	// maxLength
	err = checkValid(_T("{\"maxLength\":3}"),
					 _T("18"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxLength\":0}"),
					 _T("\"\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxLength\":6}"),
					 _T("\"hello\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxLength\":1}"),
					 _T("\"h\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxLength\":0}"),
					 _T("\"j\""),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"maxLength\":2}"),
					 _T("\"hello\""),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::enumTest()
{
	MojErr err = MojErrNone;
	err = checkValid(_T("{\"enum\":[]}"),
					 _T("18"),
					 false);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"enum\":[1]}"),
					 _T("1"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"enum\":[1,false,\"hello\"]}"),
					 _T("1"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"enum\":[1,false,\"hello\"]}"),
					 _T("false"),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"enum\":[1,false,\"hello\"]}"),
					 _T("\"hello\""),
					 true);
	MojTestErrCheck(err);
	err = checkValid(_T("{\"enum\":[1,false,\"hello\"]}"),
					 _T("2"),
					 false);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojSchemaTest::divisibleTest()
{
	(void) checkValid(_T("{\"divisibleBy\":0}"),
					 _T("18"),
					 false);
	(void) checkValid(_T("{\"divisibleBy\":1}"),
					 _T("18"),
					 true);
	(void) checkValid(_T("{\"divisibleBy\":3}"),
					 _T("18"),
					 true);
	(void) checkValid(_T("{\"divisibleBy\":3}"),
					 _T("18.0"),
					 true);
	(void) checkValid(_T("{\"divisibleBy\":3}"),
					 _T("18.1"),
					 false);
	(void) checkValid(_T("{\"divisibleBy\":4}"),
					 _T("18"),
					 false);

	return MojErrNone;
}

MojErr MojSchemaTest::checkValid(const MojChar* schemaJson, const MojChar* instanceJson, bool expected)
{
	MojObject schemaObj;
	MojErr err = schemaObj.fromJson(schemaJson);
	MojTestErrCheck(err);
	MojSchema schema;
	err = schema.fromObject(schemaObj);
	MojTestErrCheck(err);
	MojObject instance;
	err = instance.fromJson(instanceJson);
	MojTestErrCheck(err);
	MojSchema::Result res;
	err = schema.validate(instance, res);
	MojTestErrCheck(err);
	MojTestAssert(res.valid() == expected);

	return MojErrNone;
}
