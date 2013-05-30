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


#include "MojObjectSerializationTest.h"
#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"

MojObjectSerializationTest::MojObjectSerializationTest()
: MojTestCase(_T("MojObjectSerialization"))
{
}

MojErr MojObjectSerializationTest::run()
{
	const MojChar* json =
		_T("{\"i1\":-9223372036854775807, \"i2\":-888888, \"i3\":-45, \"i4\":0, \"i5\":100, ")
		_T("\"i6\":255, \"i7\":9999, \"i8\":65535, \"i9\":65536, \"i10\":4294967295, ")
		_T("\"i11\":4294967296, \"i12\":9223372036854775807, ")
		_T("\"d1\":3.14, \"d2\":0.2, \"s1\":\"hello\\t\\\"world\\\"\", \"s2\":\"\", ")
		_T("\"o1\":{\"a1\":[[[],[]],2,3,4,{},{\"t\":4},null,true,false,4.35], \"i1\":42}, ")
		_T("\"n1\":null, \"b1\":true, \"b2\":false}");

	MojObject obj;
	MojErr err = obj.fromJson(json);
	MojTestErrCheck(err);
	MojObjectWriter writer;
	err = obj.visit(writer);
	MojTestErrCheck(err);
	MojObjectBuilder builder;

	const guint8* data = NULL;
	gsize size = 0;
	err = writer.buf().data(data, size);
	MojTestErrCheck(err);
	err = MojObjectReader::read(builder, data, size);
	MojTestErrCheck(err);
	MojTestAssert(obj == builder.object());

	// comparisons
	MojVector<MojObject> vec;
	err = vec.push(MojObject());
	MojTestErrCheck(err);
	err = vec.push(1);
	MojTestErrCheck(err);
	err = vec.push(255);
	MojTestErrCheck(err);
	err = vec.push(256);
	MojTestErrCheck(err);
	err = vec.push(70000);
	MojTestErrCheck(err);
	err = vec.push(5000000000LL);
	MojTestErrCheck(err);
	err = vec.push(G_MAXINT64);
	MojTestErrCheck(err);
	err = vec.push(G_MININT64);
	MojTestErrCheck(err);
	err = vec.push(-1);
	MojTestErrCheck(err);
	err = vec.push(-8);
	MojTestErrCheck(err);
	err = vec.push(-99999999999LL);
	MojTestErrCheck(err);
	err = vec.push(G_MININT64);
	MojTestErrCheck(err);
	err = vec.push(true);
	MojTestErrCheck(err);
	err = vec.push(false);
	MojTestErrCheck(err);
	err = vec.push(MojDecimal());
	MojTestErrCheck(err);
	err = vec.push(MojDecimal(3, 14159));
	MojTestErrCheck(err);
	err = vec.push(MojDecimal(-9999, 55555));
	MojTestErrCheck(err);
	MojString str;
	err = vec.push(str);
	MojTestErrCheck(err);
	err = str.assign(_T("hello"));
	MojTestErrCheck(err);
	for (MojVector<MojObject>::ConstIterator i = vec.begin(); i != vec.end(); ++i) {
		for (MojVector<MojObject>::ConstIterator j = vec.begin(); j != vec.end(); ++j) {
			err = compTest(*i, *j);
			MojTestErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojObjectSerializationTest::compTest(const MojObject& obj1, const MojObject& obj2)
{
	MojObjectWriter writer1;
	MojErr err = obj1.visit(writer1);
	MojTestErrCheck(err);
	MojObjectWriter writer2;
	err = obj2.visit(writer2);
	MojTestErrCheck(err);
	int compObj = obj1.compare(obj2);
	const guint8* data1 = NULL;
	const guint8* data2 = NULL;
	gsize size1 = 0;
	gsize size2 = 0;
	err = writer1.buf().data(data1, size1);
	MojTestErrCheck(err);
	err = writer2.buf().data(data2, size2);
	MojTestErrCheck(err);
	int compBin = MojLexicalCompare(data1, size1, data2, size2);
	MojTestAssert(compObj == compBin || (compObj > 0 && compBin > 0) || (compObj < 0 && compBin < 0));
	return MojErrNone;
}

