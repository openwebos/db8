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
/**
****************************************************************************************************
* Filename              : MojSchemaTest.cpp

* Description           : Source file for MojSchema test.
****************************************************************************************************
**/


#include "MojSchemaTest.h"
#include "core/MojSchema.h"

MojSchemaTest::MojSchemaTest()
: MojTestCase(_T("MojSchema"))
{
}

/**
***************************************************************************************************
* @run                The MojSchema provides a mechanism to validate the JSON data objects.
                      Schema Test include the following:

                      1.typeTest
                      2.disallowTest
                      3.propertiesTest
                      4.ItemsTest
                      5.requiresTest
                      6.minmaxTest
                      7.arrayTest
                      8.stringTest
                      9.enumTest
                      10.divisibleTest
* @param            : None
* @retval           : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @typeTest           Different data types like array,int,string,Object,number and null are added
                      in this test case to check the functionality of type parameter.
                      eg:err = checkValid(_T("{\"type\":\"object\"}"),_T("{}"), true);
* @param            : None
* @retval           : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @disallowTest                 The disallow test checks the functionality of diallow attribute
                                which is similar to type except that it won't allow the specified
                                type in the schema.
                                eg:err = checkValid(_T("{\"disallow\":\"null\"}"),_T("5"),true);
                                  //null type is not allowed in the schema.
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @propertiesTest              The properties parameter functionality is verified in this test.
                               This properties is related with Object.For the Object type
                               definition defining some object properties using the properties key.
                               eg:err = checkValid(_T("{\"properties\":{}}"), _T("{\"foo\":
                                        \"bar\"}"),true);
* @param                      :  None
* @retval                     :  MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @itemsTest                    The items parameter functionality is verified in this test.This
                                property related to schema or array of schemas.When this is an
                                object or schema and the instance value is an array ,all the items
                                in the array should confirm to this schema.
                                eg:err = checkValid(_T("{\"items\":{\"type\":\"integer\"}}"),
                                 _T("[]"), true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @requiresTest                The requires keyword functionality is verified in this test.The
                               requires property tells that the property given by requires
                               attribute must be present in the containing instance object.
                               eg:err = checkValid(_T("{\"properties\":{\"foo\":{\"optional\":
                                         true,\"requires\":\"bar\"}}}"), _T("{}"),true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @minmaxTest                   The minimum and maximum keyword functionality is verified in this
                                test.

                                The minimum indicates the minimum value for the instance property
                                when the type of the instance value is a number.
                                eg:err = checkValid(_T("{\"minimum\":2}"),_T("2.0"),true);

                                The maximum indicates the maximum value for the instance property
                                when the type of the instance value is a number.
                                eg: err = checkValid(_T("{\"maximum\":2}"),_T("2"),true);

                                If the minimum is defined, the minimumCanEqual indicates whether
                                or not the instance  property value can equal the minimum.
                                eg:err = checkValid(_T("{\"minimum\":2,\"minimumCanEqual\":false}")
                                         ,_T("2.1"),true);

                                If the maximum is defined, the maximumCanEqual indicates whether
                                or not the instance property value can equal the maximum.
                                eg:err = checkValid(_T("{\"maximum\":2,\"maximumCanEqual\":true}")
                                         ,_T("2"),true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @arrayTest                    This test case holds good when the type is array.

                                The minItems indicates the minimum number of values in an array
                                when an array is the instance value.
                                eg:err = checkValid(_T("{\"minItems\":0}"),_T("[]"),true);

                                The maxItems indicates the maximum number of values in an array
                                when an array is the instance value.
                                eg:err = checkValid(_T("{\"maxItems\":2}"), _T("[1,2]"), true);

                                The unique indicates that all the items in an array must be unique
                                (no two identical values) within that array when an array is the
                                instance value
                                eg:err = checkValid(_T("{\"uniqueItems\":true}"),_T("[1,2,3,4]"),
                                          true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/
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
/**
***************************************************************************************************
* @stringTest                   The minLength and maxLength functionality  is tested in this test.

                                When the instance value is a string, the maxlenth indicates maximum
                                length of the string.
                                eg:err = checkValid(_T("{\"maxLength\":6}"),_T("\"hello\""),true);

                                When the instance value is a string, the minlength indicates
                                minimum length of the string.
                                eg:err = checkValid(_T("{\"minLength\":0}"),_T("\"hello\""),true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/

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
/**
***************************************************************************************************
* @enumTest                      The enum functionality is for array type and its functionality is
                                 verified in this test.
                                 The enum  provides an enumeration of possible values that are
                                 valid for the instance property. This should be an array, and each
                                 item in the array represents a possible value for the instance
                                 value.If "enum" is included,the instance value must be one of the
                                 values in enum array in order for the schema to be valid
                                 eg:err = checkValid(_T("{\"enum\":[1]}"),_T("1"),true);
                                    err = checkValid(_T("{\"enum\":[1,false,\"hello\"]}"),_T("1"),
                                          true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/

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
/**
***************************************************************************************************
* @divisibleTest                The divisibleBy functionality is verified in this test.
                                This divisibleBy indicates that the instance property value must be
                                divisible by the given schema value when the instance property
                                value is a number.
                                eg:(void) checkValid(_T("{\"divisibleBy\":0}"), _T("18"),false);
                                   (void) checkValid(_T("{\"divisibleBy\":1}"), _T("18"),true);
* @param                      : None
* @retval                     : MojErr
***************************************************************************************************
**/

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
/**
***************************************************************************************************
* @checkValid                   The CheckValid function takes two parameters as input and validates
                                SchemaJson and instanceJson according to the expected value.
                                If expected is true then validation will return true else if
                                expected is false then the validation should return false.
                                eg:(void) checkValid(_T("{\"divisibleBy\":3}"),_T("18.0"),true);
                                   (void) checkValid(_T("{\"divisibleBy\":3}"),_T("18.1"),false);
* @param                      : MojChar*,MojChar*,bool
* @retval                     : MojErr
***************************************************************************************************
**/

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
