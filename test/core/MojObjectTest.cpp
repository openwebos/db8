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


#include "MojObjectTest.h"
#include "core/MojHashMap.h"
#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"
#include "core/MojString.h"

MojObjectTest::MojObjectTest()
: MojTestCase("MojObject")
{
}

MojErr MojObjectTest::run()
{
	MojObject obj0;
	MojObject obj1;
	MojObject obj2;
	MojObject obj3;
	MojObject obj4;
	MojString str1;
	MojObjectBuilder builder;
	int count = 0;
	bool found = false;

	// empty object
	MojTestAssert(obj1.type() == MojObject::TypeUndefined);
	MojErr err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeNull);
	MojTestAssert(obj1.type() == MojObject::TypeNull);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeObject);
	MojTestAssert(obj1.type() == MojObject::TypeObject);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeArray);
	MojTestAssert(obj1.type() == MojObject::TypeArray);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeString);
	MojTestAssert(obj1.type() == MojObject::TypeString);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeBool);
	MojTestAssert(obj1.type() == MojObject::TypeBool);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeDecimal);
	MojTestAssert(obj1.type() == MojObject::TypeDecimal);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	obj1.clear(MojObject::TypeInt);
	MojTestAssert(obj1.type() == MojObject::TypeInt);
	err = emptyTest(obj1);
	MojTestErrCheck(err);
	err = obj2.fromJson("{}");
	MojTestAssert(obj2.type() == MojObject::TypeObject);
	MojTestErrCheck(err);
	err = emptyTest(obj2);
	MojTestErrCheck(err);
	err = obj3.put(_T("hello"), 5LL);
	MojTestErrCheck(err);
	err = obj3.del(_T("hello"), found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = emptyTest(obj3);
	MojTestErrCheck(err);
	err = obj3.put(_T("hello"), 5LL);
	MojTestErrCheck(err);
	obj3.clear();
	err = emptyTest(obj3);
	MojTestErrCheck(err);

	// put/get/del
	err = putTest(obj1);
	MojTestErrCheck(err);
	err = getTest(obj1);
	MojTestErrCheck(err);
	err = obj1.visit(builder);
	MojTestErrCheck(err);
	err = getTest(builder.object());
	MojTestErrCheck(err);
	err = obj1.toJson(str1);
	MojTestErrCheck(err);
	err = obj2.fromJson(str1);
	MojTestErrCheck(err);
	err = getTest(obj2);
	MojTestErrCheck(err);
	obj3 = obj2;
	err = getTest(obj3);
	MojTestErrCheck(err);
	obj3.clear();
	MojTestAssert(obj3.empty());

	// iterator
	for (MojObject::ConstIterator i = obj2.begin(); i != obj2.end(); ++i) {
		count++;
		obj3.put(i.key(), i.value());
	}
	MojTestAssert(count == 10);
	err = getTest(obj3);
	MojTestErrCheck(err);

	// types
	err = typeTest();
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectTest::emptyTest(MojObject& obj)
{
	MojObject obj2;
	MojString str1;
	MojString str2;

	bool found = false;

	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());

	MojTestAssert(obj.begin() == obj.end());
	MojTestAssert(!obj.contains(_T("hello")));
	MojTestAssert(!obj.get(_T("hi"), obj2));
	MojErr err = obj.del(_T("hello"), found);
	MojTestErrCheck(err);

	obj2.assign(obj);
	MojTestAssert(obj2.type() == obj.type());
	MojTestAssert(obj2 == obj);
	MojTestAssert(!(obj2 != obj));

	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	err = obj2.stringValue(str2);
	MojTestErrCheck(err);
	MojTestAssert(str1 == str2);

	return MojErrNone;
}

MojErr MojObjectTest::putTest(MojObject& obj)
{
	MojString str;
	MojObject obj2;
	MojObject array1;
	MojObject str2;
	MojObject bool2(false);
	MojObject int2(-260000000000LL);
	MojObject MojDecimal2(MojDecimal(2.34));

	MojErr err = str.assign(_T("howdy"));
	MojTestErrCheck(err);
	str2 = str;

	err = obj2.put(_T("nested"), str);
	MojTestErrCheck(err);
	err = obj.put(_T("object"), obj2);
	MojTestErrCheck(err);
	err = obj.put(_T("string"), str2);
	MojTestErrCheck(err);
	err = obj.put(_T("bool1"), true);
	MojTestErrCheck(err);
	err = obj.put(_T("bool2"), bool2);
	MojTestErrCheck(err);
	err = obj.put(_T("int1"), 50);
	MojTestErrCheck(err);
	err = obj.put(_T("int2"), int2);
	MojTestErrCheck(err);
	err = obj.put(_T("MojDecimal1"), MojDecimal(0.1));
	MojTestErrCheck(err);
	err = obj.put(_T("MojDecimal2"), MojDecimal2);
	MojTestErrCheck(err);
	err = obj.put(_T("null"), MojObject(MojObject::TypeNull));
	MojTestErrCheck(err);
	// array
	err = array1.push(MojObject(MojObject::TypeObject));
	MojTestErrCheck(err);
	err = array1.push(MojObject(MojObject::TypeArray));
	MojTestErrCheck(err);
	err = array1.push(str2);
	MojTestErrCheck(err);
	err = array1.push(true);
	MojTestErrCheck(err);
	err = array1.setAt(3, false);
	MojTestErrCheck(err);
	err = array1.push(MojDecimal(3, 140000));
	MojTestErrCheck(err);
	err = array1.push(100);
	MojTestErrCheck(err);
	err = array1.push(MojObject(MojObject::TypeNull));
	MojTestErrCheck(err);
	err = array1.setAt(7, 4);
	MojTestErrCheck(err);
	err = obj.put(_T("array"), array1);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectTest::getTest(const MojObject& obj)
{
	MojObject obj2;
	MojObject obj3;
	MojObject array1;
	MojString str1;
	MojSize size;

	size = obj.size();
	MojTestAssert(size == 10);
	MojTestAssert(obj.get(_T("object"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeObject);
	MojTestAssert(obj2.get(_T("nested"), obj3));
	MojErr err = obj3.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("howdy"));
	MojTestAssert(obj.get(_T("string"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeString);
	err = obj2.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("howdy"));
	MojTestAssert(obj.get(_T("bool1"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeBool);
	MojTestAssert(obj2 == true);
	MojTestAssert(obj2 != false);
	MojTestAssert(obj.get(_T("bool2"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeBool);
	MojTestAssert(obj2 == false);
	MojTestAssert(obj2 != true);
	MojTestAssert(obj.get(_T("MojDecimal1"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeDecimal);
	MojTestAssert(obj2 == MojDecimal(0, 100000));
	MojTestAssert(obj2 != MojDecimal(2.34));
	MojTestAssert(obj.get(_T("MojDecimal2"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeDecimal);
	MojTestAssert(obj2 == MojDecimal(2.34));
	MojTestAssert(obj2 != MojDecimal(0.1));
	MojTestAssert(obj.get(_T("int1"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeInt);
	MojTestAssert(obj2 == 50);
	MojTestAssert(obj2 != -260000000000LL);
	MojTestAssert(obj.get(_T("int2"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeInt);
	MojTestAssert(obj2 == -260000000000LL);
	MojTestAssert(obj2 != 50);
	MojTestAssert(obj.get(_T("null"), obj2));
	MojTestAssert(obj2.type() == MojObject::TypeNull);
	// array
	MojTestAssert(obj.get(_T("array"), array1));
	MojTestAssert(array1.type() == MojObject::TypeArray);
	MojTestAssert(array1.size() == 8);
	MojTestAssert(array1.at(0, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeObject);
	MojTestAssert(obj2.empty());
	MojTestAssert(array1.at(1, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeArray);
	MojTestAssert(obj2.empty());
	MojTestAssert(array1.at(2, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeString);
	err = obj2.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("howdy"));
	MojTestAssert(array1.at(3, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeBool);
	MojTestAssert(obj2 == false);
	MojTestAssert(array1.at(4, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeDecimal);
	MojTestAssert(obj2 == MojDecimal(3.14));
	MojTestAssert(array1.at(5, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeInt);
	MojTestAssert(obj2 == 100);
	MojTestAssert(array1.at(6, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeNull);
	MojTestAssert(array1.at(7, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeInt);
	MojTestAssert(obj2 == 4);
	MojTestAssert(!array1.at(8, obj2));

	return MojErrNone;
}

MojErr MojObjectTest::typeTest()
{
	MojObject obj;
	MojObject obj2;
	MojString str1;
	MojString str2;
	MojHashMap<MojObject, MojObject> map;

	// null
	MojTestAssert(obj.type() == MojObject::TypeUndefined);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	MojErr err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("null"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	MojTestAssert(obj == obj2);
	MojTestAssert(obj.compare(obj2) == 0);
	err = obj.coerce(MojObject::TypeNull);
	MojTestErrCheck(err);
	MojTestAssert(obj.type() == MojObject::TypeNull);
	err = obj.coerce(MojObject::TypeString);
	MojTestErrCheck(err);
	MojTestAssert(obj == str1);
	err = obj.coerce(MojObject::TypeBool);
	MojTestErrCheck(err);
	MojTestAssert(obj == true);
	err = obj.coerce(MojObject::TypeInt);
	MojTestErrCheck(err);
	MojTestAssert(obj == 1);
	err = obj.coerce(MojObject::TypeDecimal);
	MojTestErrCheck(err);
	MojTestAssert(obj.type() == MojObject::TypeDecimal && obj == MojDecimal(1, 0));
	err = obj.coerce(MojObject::TypeObject);
	MojTestErrCheck(err);
	MojTestAssert(obj.type() == MojObject::TypeObject);
	err = obj.coerce(MojObject::TypeArray);
	MojTestErrCheck(err);
	MojTestAssert(obj.type() == MojObject::TypeArray);

	// object
	err = obj.put(_T("hello"), 5);
	MojTestErrCheck(err);
	MojTestAssert(obj.type() == MojObject::TypeObject);
	MojTestAssert(obj.size() == 1);
	MojTestAssert(!obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("{\"hello\":5}"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	obj.clear(MojObject::TypeObject);
	MojTestAssert(obj.type() == MojObject::TypeObject);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("{}"));
	// array
	for (int i = 0; i < 1000; ++i) {
		err = obj.push(i);
		MojTestErrCheck(err);
	}
	MojTestAssert(obj.type() == MojObject::TypeArray);
	MojTestAssert(obj.size() == 1000);
	MojTestAssert(!obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	for (int i = 0; i < 1000; ++i) {
		MojTestAssert(obj.at(i, obj2));
		MojTestAssert(obj2 == i);
	}
	MojTestAssert(!obj.at(1000, obj2));
	err = obj.setAt(1001, 1001);
	MojTestErrCheck(err);
	MojTestAssert(obj.size() == 1002);
	MojTestAssert(obj.at(1000, obj2));
	MojTestAssert(obj2.type() == MojObject::TypeUndefined);
	obj.clear(MojObject::TypeArray);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("[]"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	// string
	err = str1.assign(_T("yo"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.type() == MojObject::TypeString);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str2);
	MojTestErrCheck(err);
	MojTestAssert(str1 == str2);
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	obj.clear(MojObject::TypeString);
	MojTestAssert(obj.boolValue() == false);
	err = str1.assign(_T("12345"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == 12345);
	MojTestAssert(obj.decimalValue() == MojDecimal(12345, 0));
	err = str1.assign(_T("-67890"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == -67890);
	MojTestAssert(obj.decimalValue() == MojDecimal(-67890, 0));
	err = str1.assign(_T("12345000000000"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == 12345000000000LL);
	err = str1.assign(_T("12345.6789"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == 12345);
	MojTestAssert(obj.decimalValue() == MojDecimal(12345, 678900));
	err = str1.assign(_T("1.0e3"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == 1);
	MojTestAssert(obj.decimalValue() == MojDecimal(1000, 0));
	err = str1.assign(_T("45hello"));
	MojTestErrCheck(err);
	obj = str1;
	MojTestAssert(obj.intValue() == 45);
	MojTestAssert(obj.decimalValue() == MojDecimal(45, 0));
	// bool
	obj = true;
	MojTestAssert(obj.type() == MojObject::TypeBool);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == 1);
	MojTestAssert(obj.decimalValue() == MojDecimal(1, 0));
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("true"));
	obj.clear(MojObject::TypeBool);
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("false"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	// MojDecimal
	obj = MojDecimal(3, 140000);
	MojTestAssert(obj.type() == MojObject::TypeDecimal);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == 3);
	MojTestAssert(obj.decimalValue() == MojDecimal(3.14));
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("3.14"));
	obj.clear(MojObject::TypeDecimal);
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("0.0"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));
	// MojDecimal
	obj = -987654321;
	MojTestAssert(obj.type() == MojObject::TypeInt);
	MojTestAssert(obj.size() == 0);
	MojTestAssert(obj.empty());
	MojTestAssert(obj.boolValue() == true);
	MojTestAssert(obj.intValue() == -987654321);
	MojTestAssert(obj.decimalValue() == MojDecimal(-987654321, 0));
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("-987654321"));
	obj.clear(MojObject::TypeInt);
	MojTestAssert(obj.boolValue() == false);
	MojTestAssert(obj.intValue() == 0);
	MojTestAssert(obj.decimalValue() == MojDecimal());
	err = obj.stringValue(str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("0"));
	err = map.put(obj, obj);
	MojTestErrCheck(err);
	MojTestAssert(map.contains(obj));

	return MojErrNone;
}
