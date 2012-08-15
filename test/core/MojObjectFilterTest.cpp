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


#include "MojObjectFilterTest.h"
#include "db/MojDb.h"
#include "core/MojJson.h"
#include "core/MojObjectFilter.h"

static const MojChar* const FullObject =
	_T("{\"a\":{\"b\":100},\"c\":{\"d\":{\"e\":200,\"f\":300},\"g\":400}, \"h\":{\"i\":500}}");
static const MojChar* const SomeLeafNodes =
	_T("{\"a\":{\"b\":100},\"c\":{\"d\":{\"e\":200},\"g\":400}, \"h\":{\"i\":500}}");
static const MojChar* const SomeSubObjects1 =
	_T("{\"a\":{\"b\":100}, \"h\":{\"i\":500}}");
static const MojChar* const SomeSubObjects2 =
	_T("{\"a\":{\"b\":100}, \"c\":{\"d\":{\"e\":200,\"f\":300},\"g\":400}}");

static const MojChar* const PartialObject =
	_T("{\"a\":{\"b\":100},\"c\":{\"g\":400}, \"h\":{\"i\":500}}");
static const MojChar* const PartialObjectResult1 =
	_T("{\"a\":{\"b\":100}}");
static const MojChar* const PartialObjectResult2 =
	_T("{\"h\":{\"i\":500}}");
static const MojChar* const PartialObjectResult3 =
	_T("{}");

static const MojChar* const EmptyObject =
	_T("{}");

static const MojChar* const EmptySubObject =
	_T("{\"a\":{}}");

static const MojChar* const EmptyArray =
	_T("{\"a\":[]}");

static const MojChar* const EmptyNestedArray =
	_T("{\"a\":[{\"b\":[], \"c\":300}]}");
static const MojChar* const EmptyNestedArrayResult1 =
	_T("{\"a\":[{\"b\":[]}]}");
static const MojChar* const EmptyNestedArrayResult2 =
	_T("{\"a\":[{\"c\":300}]}");

static const MojChar* const Array =
	_T("{\"a\":[100,200,300]}");

static const MojChar* const ArrayObject =
	_T("{\"a\":{\"b\":100},\"c\":[\"200\", \"300\", \"400\"], \"h\":{\"i\":500}}");
static const MojChar* const ArrayObjectResult1 =
	_T("{\"c\":[\"200\", \"300\", \"400\"]}");
static const MojChar* const ArrayObjectResult2 =
	_T("{\"a\":{\"b\":100}, \"c\":[\"200\", \"300\", \"400\"]}");
static const MojChar* const ArrayObjectResult3 =
	_T("{\"c\":[\"200\", \"300\", \"400\"], \"h\":{\"i\":500}}");

static const MojChar* const ArraySubObject =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200, \"e\":300}, {\"d\":400, \"e\":500}, {\"d\":600, \"e\":700}], \"h\":{\"i\":800}}");
static const MojChar* const ArraySubObjectResult1 =
	_T("{\"c\":[{\"d\":200, \"e\":300}, {\"d\":400, \"e\":500}, {\"d\":600, \"e\":700}]}");
static const MojChar* const ArraySubObjectResult2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200}, {\"d\":400}, {\"d\":600}]}");
static const MojChar* const ArraySubObjectResult3 =
	_T("{\"c\":[{\"e\":300}, {\"e\":500}, {\"e\":700}], \"h\":{\"i\":800}}");

static const MojChar* const NestedArray =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200, \"e\":[300,400]}, {\"d\":500, \"e\":[600,700]}, {\"d\":800, \"e\":[900,1000]}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArrayResult1 =
	_T("{\"c\":[{\"d\":200, \"e\":[300,400]}, {\"d\":500, \"e\":[600,700]}, {\"d\":800, \"e\":[900,1000]}]}");
static const MojChar* const NestedArrayResult2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const NestedArrayResult3 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArrayResult4 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":[300,400]}, {\"e\":[600,700]}, {\"e\":[900,1000]}]}");
static const MojChar* const NestedArrayResult5 =
	_T("{\"c\":[{\"e\":[300,400]}, {\"e\":[600,700]}, {\"e\":[900,1000]}], \"h\":{\"i\":1100}}");

static const MojChar* const NestedArray2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200, \"f\":450}, {\"d\":500, \"f\":750}, {\"d\":800, \"f\":1050}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArray2Result1 =
	_T("{\"c\":[{\"d\":200, \"f\":450}, {\"d\":500, \"f\":750}, {\"d\":800, \"f\":1050}]}");
static const MojChar* const NestedArray2Result2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"f\":450}, {\"f\":750}, {\"f\":1050}]}");
static const MojChar* const NestedArray2Result3 =
	_T("{\"c\":[{\"f\":450}, {\"f\":750}, {\"f\":1050}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArray2Result4 =
	_T("{\"a\":{\"b\":100}}");
static const MojChar* const NestedArray2Result5 =
	_T("{\"h\":{\"i\":1100}}");
static const MojChar* const NestedArray2Result6 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"f\":450}, {\"f\":750}, {\"f\":1050}]}");
static const MojChar* const NestedArray2Result7 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}], \"h\":{\"i\":1100}}");

static const MojChar* const NestedArray3 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200, \"e\":{\"f\":250, \"g\":[300,400]}}, {\"d\":500, \"e\":{\"f\":550, \"g\":[600,700]}}, {\"d\":800, \"e\":{\"f\":850, \"g\":[900,1000]}}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArray3Result1 =
	_T("{\"c\":[{\"d\":200, \"e\":{\"f\":250, \"g\":[300,400]}}, {\"d\":500, \"e\":{\"f\":550, \"g\":[600,700]}}, {\"d\":800, \"e\":{\"f\":850, \"g\":[900,1000]}}]}");
static const MojChar* const NestedArray3Result2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const NestedArray3Result3 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArray3Result4 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":{\"f\":250, \"g\":[300,400]}}, {\"e\":{\"f\":550, \"g\":[600,700]}}, {\"e\":{\"f\":850, \"g\":[900,1000]}}]}");
static const MojChar* const NestedArray3Result5 =
	_T("{\"c\":[{\"e\":{\"f\":250, \"g\":[300,400]}}, {\"e\":{\"f\":550, \"g\":[600,700]}}, {\"e\":{\"f\":850, \"g\":[900,1000]}}], \"h\":{\"i\":1100}}");
static const MojChar* const NestedArray3Result6 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":{\"g\":[300,400]}}, {\"e\":{\"g\":[600,700]}}, {\"e\":{\"g\":[900,1000]}}]}");
static const MojChar* const NestedArray3Result7 =
	_T("{\"c\":[{\"e\":{\"g\":[300,400]}}, {\"e\":{\"g\":[600,700]}}, {\"e\":{\"g\":[900,1000]}}], \"h\":{\"i\":1100}}");

static const MojChar* const NestedArray4 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"d\":200, \"e\":{\"f\":250, \"g\":[300,400], \"h\":450}}, {\"d\":500, \"e\":{\"f\":550, \"g\":[600,700], \"h\":750}}, {\"d\":800, \"e\":{\"f\":850, \"g\":[900,1000], \"h\":1050}}], \"i\":{\"j\":1100}}");
static const MojChar* const NestedArray4Result1 =
	_T("{\"c\":[{\"d\":200, \"e\":{\"f\":250, \"g\":[300,400], \"h\":450}}, {\"d\":500, \"e\":{\"f\":550, \"g\":[600,700], \"h\":750}}, {\"d\":800, \"e\":{\"f\":850, \"g\":[900,1000], \"h\":1050}}]}");
static const MojChar* const NestedArray4Result2 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":{\"h\":450}}, {\"e\":{\"h\":750}}, {\"e\":{\"h\":1050}}]}");
static const MojChar* const NestedArray4Result3 =
	_T("{\"c\":[{\"e\":{\"h\":450}}, {\"e\":{\"h\":750}}, {\"e\":{\"h\":1050}}], \"i\":{\"j\":1100}}");
static const MojChar* const NestedArray4Result4 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":{\"f\":250, \"g\":[300,400], \"h\":450}}, {\"e\":{\"f\":550, \"g\":[600,700], \"h\":750}}, {\"e\":{\"f\":850, \"g\":[900,1000], \"h\":1050}}]}");
static const MojChar* const NestedArray4Result5 =
	_T("{\"c\":[{\"e\":{\"f\":250, \"g\":[300,400], \"h\":450}}, {\"e\":{\"f\":550, \"g\":[600,700], \"h\":750}}, {\"e\":{\"f\":850, \"g\":[900,1000], \"h\":1050}}], \"i\":{\"j\":1100}}");
static const MojChar* const NestedArray4Result6 =
	_T("{\"a\":{\"b\":100},\"c\":[{\"e\":{\"g\":[300,400], \"h\":450}}, {\"e\":{\"g\":[600,700], \"h\":750}}, {\"e\":{\"g\":[900,1000], \"h\":1050}}]}");
static const MojChar* const NestedArray4Result7 =
	_T("{\"c\":[{\"e\":{\"g\":[300,400], \"h\":450}}, {\"e\":{\"g\":[600,700], \"h\":750}}, {\"e\":{\"g\":[900,1000], \"h\":1050}}], \"i\":{\"j\":1100}}");

static const MojChar* const ArrayWithDifferentObjects =
	_T("{\"a\":[{\"c\":100}, {\"b\":100}, {\"b\":100}]}");
static const MojChar* const ArrayWithDifferentObjectsResult1 =
	_T("{\"a\":[{\"b\":100}, {\"b\":100}]}");

static const MojChar* const ArrayWithEmptyObjects =
	_T("{\"a\":[{}, {\"b\":100}, {}]}");
static const MojChar* const ArrayWithEmptyObjectsResult1 =
	_T("{\"a\":[{\"b\":100}]}");

static const MojChar* const SubArrayWithEmptyObjects =
	_T("{\"a\":[{}, {\"b\":100}, {\"c\":200}, {\"b\":100, \"d\":[{\"e\":300}]}, {\"d\":[{\"e\":300}]}, {\"d\":[]}]}");
static const MojChar* const SubArrayWithEmptyObjectsResult1 =
	_T("{\"a\":[{\"b\":100}, {\"b\":100}]}");
static const MojChar* const SubArrayWithEmptyObjectsResult2 =
	_T("{\"a\":[{\"d\":[{\"e\":300}]}, {\"d\":[{\"e\":300}]}, {\"d\":[]}]}");
static const MojChar* const SubArrayWithEmptyObjectsResult3 =
	_T("{\"a\":[{\"d\":[{\"e\":300}]}, {\"d\":[{\"e\":300}]}]}");

static const MojChar* const CDFromFullObject =
	_T("{\"c\":{\"d\":{\"e\":200,\"f\":300}}}");
static const MojChar* const CDFromPartialObject =
	_T("{}");
static const MojChar* const CDFromArrayObject =
	_T("{}");
static const MojChar* const CDFromArraySubObject =
	_T("{\"c\":[{\"d\":200}, {\"d\":400}, {\"d\":600}]}");
static const MojChar* const CDFromNestedArray =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const CDFromNestedArray2 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const CDFromNestedArray3 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const CDFromNestedArray4 =
	_T("{\"c\":[{\"d\":200}, {\"d\":500}, {\"d\":800}]}");
static const MojChar* const CDFromArrayWithDifferentObjects =
	_T("{}");
static const MojChar* const CDFromArrayWithEmptyObjects =
	_T("{}");
static const MojChar* const CDFromSubArrayWithEmptyObjects =
	_T("{}");


MojObjectFilterTest::MojObjectFilterTest()
: MojTestCase(_T("MojObjectFilter"))
{
}

MojErr MojObjectFilterTest::run()
{
	//parse a full object, ask for slices that exist - all the leaf node slices
	MojDbQuery::StringSet set;
	MojString slice;
	MojErr err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.f"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.i"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	MojObject expected;
	err = expected.fromJson(FullObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//parse a full object, ask for slices that exist - all the main objects
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(FullObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that exist in the object - ask for some of the leaf node slices - a.b, c.d.e, c.g, h
	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.i"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SomeLeafNodes);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that exist in the object when one item is in the db - ask for whole objects - a, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SomeSubObjects1);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that exist in the object when one item is in the db - ask for whole objects - a, c
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SomeSubObjects2);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices out of order that exist - h, a
	set.clear();
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SomeSubObjects1);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for redundant slices that exist - a, a.b, c, c.d.f
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.f"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SomeSubObjects2);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that don't exist - a.a, c.c, h.h
	set.clear();
	err = slice.assign(_T("a.a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that don't exist - a.a, c.c, h.h
	set.clear();
	err = slice.assign(_T("a.c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.x"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.q"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that don't exist - a.a, c.c, h.h
	set.clear();
	err = slice.assign(_T("a.a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for slices that don't exist - a.a, c.c, h.h
	set.clear();
	err = slice.assign(_T("a.c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d.r"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.x"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(FullObject, expected, set);
	MojTestErrCheck(err);

	//ask for a slice that does exist and a slice that doesn't - ask for a, c.d on the PartialObject
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(PartialObjectResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(PartialObject, expected, set);
	MojTestErrCheck(err);

	//ask for a slice that doesn't exist and then a slice that does - ask for c.d, h on PartialObject
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(PartialObjectResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(PartialObject, expected, set);
	MojTestErrCheck(err);

	//ask for a sub-property without fully qualifying it - ask for d
	set.clear();
	err = slice.assign(_T("d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(PartialObject, expected, set);
	MojTestErrCheck(err);

	//ask for a property from an empty object
	set.clear();
	err = slice.assign(_T("w"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyObject, expected, set);
	MojTestErrCheck(err);

	//ask for the empty sub object
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptySubObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptySubObject, expected, set);
	MojTestErrCheck(err);

	//ask for a property from an empty sub object
	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptySubObject, expected, set);
	MojTestErrCheck(err);

	//ask for a property from an empty array
	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyArray, expected, set);
	MojTestErrCheck(err);

	//ask for the empty array
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyArray);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyArray, expected, set);
	MojTestErrCheck(err);

	//ask for an array with an empty nested array
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyNestedArray);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyNestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for one property of the nested object
	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyNestedArrayResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyNestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for another property of the nested object
	set.clear();
	err = slice.assign(_T("a.c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyNestedArrayResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyNestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for a non-existent property of the nested object
	set.clear();
	err = slice.assign(_T("a.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyNestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for a non-existent property of the nested object that would come before any of the existing properties
	set.clear();
	err = slice.assign(_T("a.a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(EmptyNestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for an array property
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(Array);
	MojTestErrCheck(err);

	err = testFilteredParse(Array, expected, set);
	MojTestErrCheck(err);

	//ask for a subobject of an array that doesn't have subobjects
	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(EmptyObject);
	MojTestErrCheck(err);

	err = testFilteredParse(Array, expected, set);
	MojTestErrCheck(err);

	//create object with an array of values, ask for all main objects - a, c, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayObject);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayObject, expected, set);
	MojTestErrCheck(err);

	//just ask for the array slice - c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayObjectResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayObject, expected, set);
	MojTestErrCheck(err);

	//array slice and one other leaf node - a.b, c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayObjectResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayObject, expected, set);
	MojTestErrCheck(err);

	//array slice and one other leaf node - c, h.i
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h.i"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayObjectResult3);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayObject, expected, set);
	MojTestErrCheck(err);

	//create object with an array of objects, ask for all main objects - a, c, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArraySubObject);
	MojTestErrCheck(err);

	err = testFilteredParse(ArraySubObject, expected, set);
	MojTestErrCheck(err);

	//just ask for the array slice - c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArraySubObjectResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(ArraySubObject, expected, set);
	MojTestErrCheck(err);

	//ask for part of the array objects and one other leaf node - a.b, c.d
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArraySubObjectResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(ArraySubObject, expected, set);
	MojTestErrCheck(err);

	//ask for the other part of the array objects and the other node - c.d, h
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArraySubObjectResult3);
	MojTestErrCheck(err);

	err = testFilteredParse(ArraySubObject, expected, set);
	MojTestErrCheck(err);

	//Test NestedArray
	err = testNestedArray();
	MojTestErrCheck(err);

	//Test NestedArray2
	err = testNestedArray2();
	MojTestErrCheck(err);

	//Test NestedArray3
	err = testNestedArray3();
	MojTestErrCheck(err);

	//Test NestedArray4
	err = testNestedArray4();
	MojTestErrCheck(err);

	//Test ArrayWithDifferentObjects
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayWithDifferentObjects);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayWithDifferentObjects, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayWithDifferentObjectsResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayWithDifferentObjects, expected, set);
	MojTestErrCheck(err);

	//Test ArrayWithEmptyObjects
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayWithEmptyObjects);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(ArrayWithEmptyObjectsResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(ArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	//Test SubArrayWithEmptyObjects
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SubArrayWithEmptyObjects);
	MojTestErrCheck(err);

	err = testFilteredParse(SubArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SubArrayWithEmptyObjectsResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(SubArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("a.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SubArrayWithEmptyObjectsResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(SubArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("a.d.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(SubArrayWithEmptyObjectsResult3);
	MojTestErrCheck(err);

	err = testFilteredParse(SubArrayWithEmptyObjects, expected, set);
	MojTestErrCheck(err);

	//Test for c.d from various json strings
	MojObjectFilter visitor;

	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	err = visitor.init(set);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromFullObject);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(FullObject, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromPartialObject);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(PartialObject, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromArrayObject);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(ArrayObject, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromArraySubObject);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(ArraySubObject, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromNestedArray);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(NestedArray, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromNestedArray2);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(NestedArray2, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromNestedArray3);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(NestedArray3, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromNestedArray4);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(NestedArray4, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromArrayWithDifferentObjects);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(ArrayWithDifferentObjects, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromArrayWithEmptyObjects);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(ArrayWithEmptyObjects, expected, visitor);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(CDFromSubArrayWithEmptyObjects);
	MojTestErrCheck(err);

	err = testFilteredParseReuseVisitor(SubArrayWithEmptyObjects, expected, visitor);
	MojTestErrCheck(err);

	//ask for nothing - should get an error
	set.clear();
	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(PartialObjectResult3);
	MojTestErrCheck(err);

	//MojTestAssert(testFilteredParse(PartialObject, expected, set) == MojErrDbInvalidQuery);

	return MojErrNone;

}

MojErr MojObjectFilterTest::testNestedArray()
{
	MojDbQuery::StringSet set;
	MojString slice;
	MojObject expected;

	MojErr err;

	//create object with an array of objects with nested arrays, ask for all main objects - a, c, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	//just ask for the array slice - c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArrayResult1);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for part of the array objects and one other leaf node - a.b, c.d
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArrayResult2);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for the other part of the array objects and the other node - c.d, h
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArrayResult3);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and one other leaf node - a.b, c.e
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArrayResult4);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and the other node - c.e, h
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArrayResult5);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray, expected, set);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectFilterTest::testNestedArray2()
{
	MojDbQuery::StringSet set;
	MojString slice;
	MojObject expected;

	MojErr err;

	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result1);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.f"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result2);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.f"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result3);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result4);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result5);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.f"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result6);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray2Result7);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray2, expected, set);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectFilterTest::testNestedArray3() {
	MojDbQuery::StringSet set;
	MojString slice;
	MojObject expected;

	MojErr err;

	//create object with an array of objects with nested arrays, ask for all main objects - a, c, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//just ask for the array slice - c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result1);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for part of the array objects and one other leaf node - a.b, c.d
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result2);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for the other part of the array objects and the other node - c.d, h
	set.clear();
	err = slice.assign(_T("c.d"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result3);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and one other leaf node - a.b, c.e
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result4);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and the other node - c.e, h
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result5);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and one other leaf node - a.b, c.e.g
	set.clear();
	err = slice.assign(_T("c.e.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result6);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and the other node - c.e.g, h
	set.clear();
	err = slice.assign(_T("c.e.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray3Result7);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray3, expected, set);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectFilterTest::testNestedArray4() {
	MojDbQuery::StringSet set;
	MojString slice;
	MojObject expected;

	MojErr err;

	//create object with an array of objects with nested arrays, ask for all main objects - a, c, h
	set.clear();
	err = slice.assign(_T("a"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("i"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//just ask for the array slice - c
	set.clear();
	err = slice.assign(_T("c"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result1);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for part of the array objects and one other leaf node - a.b, c.d
	set.clear();
	err = slice.assign(_T("c.e.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result2);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for the other part of the array objects and the other node - c.d, h
	set.clear();
	err = slice.assign(_T("c.e.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("i"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result3);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and one other leaf node - a.b, c.e
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result4);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and the other node - c.e, h
	set.clear();
	err = slice.assign(_T("c.e"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("i.j"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result5);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and one other leaf node - a.b, c.e.g
	set.clear();
	err = slice.assign(_T("c.e.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.e.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("a.b"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result6);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	//ask for the nested array part of the array objects and the other node - c.e.g, h
	set.clear();
	err = slice.assign(_T("c.e.g"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("c.e.h"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);
	err = slice.assign(_T("i.j"));
	MojTestErrCheck(err);
	err = set.put(slice);
	MojTestErrCheck(err);

	expected.clear(MojObject::TypeObject);
	err = expected.fromJson(NestedArray4Result7);
	MojTestErrCheck(err);

	err = testFilteredParse(NestedArray4, expected, set);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectFilterTest::testFilteredParse(const MojChar* toParse, MojObject& expected, MojDbQuery::StringSet& set)
{
	MojJsonWriter writer;
	MojObjectFilter visitor;

	MojErr err = visitor.init(set);
	MojTestErrCheck(err);
	visitor.setVisitor(&writer);
	err = MojJsonParser::parse(visitor, toParse);
	MojTestErrCheck(err);

	MojString output = writer.json();
	MojObject result;
	err = result.fromJson(output);
	MojTestErrCheck(err);

	MojTestAssert(result == expected);
	return MojErrNone;
}

MojErr MojObjectFilterTest::testFilteredParseReuseVisitor(const MojChar* toParse, MojObject& expected, MojObjectFilter& visitor)
{
	MojJsonParser parser;
	MojJsonWriter writer;

	visitor.setVisitor(&writer);
	MojErr err = parser.parse(visitor, toParse);
	MojTestErrCheck(err);

	MojString output = writer.json();
	MojObject result;
	err = result.fromJson(output);
	MojTestErrCheck(err);

	MojTestAssert(result == expected);
	return MojErrNone;
}
