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


#include "MojAutoPtrTest.h"
#include "core/MojAutoPtr.h"
#include "core/MojString.h"

class FooTest {
	int i;
};

class BarTest : public FooTest {
	int j;
};

static MojAutoPtr<FooTest> makeFooAuto()
{
	return MojAutoPtr<FooTest>(new FooTest);
}

static MojErr takeFooAuto(MojAutoPtr<FooTest> p)
{
	MojTestAssert(p.get());
	return MojErrNone;
}

static MojSharedPtr<FooTest> makeFooShared()
{
	MojSharedPtr<FooTest> p;
	(void) p.reset(new FooTest);
	return p;
}

static MojErr takeFooShared(MojSharedPtr<FooTest> p)
{
	MojTestAssert(p.get());
	return MojErrNone;
}

MojAutoPtrTest::MojAutoPtrTest()
: MojTestCase("MojAutoPtr")
{
}

MojErr MojAutoPtrTest::run()
{
	MojErr err = MojErrNone;
	MojAutoPtr<int> intPtr1;
	MojAutoPtr<MojString> strPtr1;
	MojAutoArrayPtr<MojChar> charPtr1;
	MojAutoPtr<int> intPtr2(new int(5));
	MojAutoPtr<MojString> strPtr2(new MojString());
	MojAutoArrayPtr<MojChar> charPtr2(new MojChar[6]);
	MojAutoPtr<int> intPtr3(new int(5));
	MojAutoPtr<MojString> strPtr3(new MojString());
	MojAutoArrayPtr<MojChar> charPtr3(new MojChar[6]);
	MojAutoPtr<int> intPtr4(intPtr3);
	MojAutoPtr<MojString> strPtr4(strPtr3);
	MojAutoArrayPtr<MojChar> charPtr4(charPtr3);
	MojSharedPtr<int> intSp1;
	MojSharedPtr<int> intSp2(intSp1);
	MojSharedPtr<MojString> strSp1;
	MojSharedPtr<MojString> strSp2(strSp1);
	MojSharedArrayPtr<MojChar> charSp1;
	MojSharedArrayPtr<MojChar> charSp2;

	MojTestAssert(!intPtr1.get());
	MojTestAssert(!strPtr1.get());
	MojTestAssert(!charPtr1.get());
	MojTestAssert(!intPtr3.get());
	MojTestAssert(!strPtr3.get());
	MojTestAssert(!charPtr3.get());
	MojTestAssert(intPtr2.get());
	MojTestAssert(strPtr2.get());
	MojTestAssert(charPtr2.get());
	MojTestAssert(intPtr4.get());
	MojTestAssert(strPtr4.get());
	MojTestAssert(charPtr4.get());
	MojTestAssert(!intPtr1.release());

	intPtr1.reset();
	strPtr1.reset();
	charPtr1.reset();
	MojTestAssert(*intPtr2.get() == 5);
	MojTestAssert(*intPtr2 == 5);
	MojTestAssert(strPtr2->empty());
	MojStrCpy(charPtr2.get(), _T("hello"));
	MojTestAssert(*charPtr2 == _T('h'));
	err = strPtr2->assign(charPtr2.get());
	MojTestErrCheck(err);
	MojTestAssert(*strPtr2 == charPtr2.get());
	intPtr1 = intPtr2;
	strPtr1 = strPtr2;
	charPtr1 = charPtr2;
	MojTestAssert(!intPtr2.get());
	MojTestAssert(!strPtr2.get());
	MojTestAssert(!charPtr2.get());
	MojTestAssert(intPtr1.get());
	MojTestAssert(strPtr1.get());
	MojTestAssert(charPtr1.get());
	MojTestAssert(*intPtr1 == 5);
	MojTestAssert(*strPtr1 == _T("hello"));
	MojTestAssert(*charPtr1 == _T('h'));

	delete intPtr1.release();

	// related type
	MojAutoPtr<FooTest> foop;
	MojAutoPtr<BarTest> barp(new BarTest());
	MojAllocCheck(barp.get());
	BarTest* barraw = barp.get();
	foop = barp;
	MojTestAssert(foop.get() == barraw);
	MojTestAssert(barp.get() == NULL);
	MojAutoPtr<FooTest> foop2(barp);

	MojSharedPtr<FooTest> foosp;
	MojSharedPtr<BarTest> barsp;
	foosp = barsp;
	MojSharedPtr<FooTest> foosp2(barsp);

	// shared ptrs
	MojTestAssert(!intSp1.get());
	MojTestAssert(intSp1.refcount() == 0);
	MojTestAssert(!intSp2.get());
	MojTestAssert(intSp2.refcount() == 0);
	MojTestAssert(!strSp1.get());
	MojTestAssert(strSp1.refcount() == 0);
	MojTestAssert(!strSp2.get());
	MojTestAssert(strSp2.refcount() == 0);
	MojTestAssert(!charSp1.get());
	MojTestAssert(charSp1.refcount() == 0);
	MojTestAssert(!charSp2.get());
	MojTestAssert(charSp2.refcount() == 0);

	err = intSp1.reset(new int(10));
	MojTestErrCheck(err);
	MojTestAssert(intSp1.get() && *intSp1 == 10);
	MojTestAssert(!intSp2.get());
	MojTestAssert(intSp1.refcount() == 1);
	intSp2 = intSp1;
	MojTestAssert(intSp1.get() && *intSp1 == 10);
	MojTestAssert(intSp2.get() && *intSp2 == 10);
	MojTestAssert(intSp1.refcount() == 2);
	MojTestAssert(intSp2.refcount() == 2);
	err = intSp1.reset(new int(20));
	MojTestErrCheck(err);
	MojTestAssert(intSp1.get() && *intSp1 == 20);
	MojTestAssert(intSp2.get() && *intSp2 == 10);
	MojTestAssert(intSp1.refcount() == 1);
	MojTestAssert(intSp2.refcount() == 1);
	intSp2 = intSp1;
	MojTestAssert(intSp1.get() && *intSp1 == 20);
	MojTestAssert(intSp2.get() && *intSp2 == 20);
	MojTestAssert(intSp1.refcount() == 2);
	MojTestAssert(intSp2.refcount() == 2);
	err = intSp1.reset(NULL);
	MojTestErrCheck(err);
	MojTestAssert(!intSp1.get());
	MojTestAssert(intSp1.refcount() == 0);
	MojTestAssert(intSp2.get() && *intSp2 == 20);
	MojTestAssert(intSp2.refcount() == 1);
	err = strSp1.reset(new MojString);
	MojTestErrCheck(err);
	MojTestAssert(strSp1.get());
	MojTestAssert(strSp1->empty());
	MojTestAssert(*strSp1 == _T(""));
	strSp2 = strSp1;
	MojTestAssert(*strSp2 == _T(""));
	err = charSp1.reset(new MojChar[100]);
	MojErrCheck(err);
	MojTestAssert(charSp1.get());
	charSp2 = charSp1;
	MojTestAssert(charSp1.get() == charSp2.get());
	err = charSp1.resetChecked(new MojChar[100]);
	MojErrCheck(err);
	err = charSp1.resetChecked(NULL);
	MojTestErrExpected(err, MojErrNoMem);

	// refs
	MojAutoPtr<FooTest> foop3 = makeFooAuto();
	takeFooAuto(foop3);
	takeFooAuto(makeFooAuto());
	MojSharedPtr<FooTest> foosp3 = makeFooShared();
	takeFooShared(foosp3);
	takeFooShared(makeFooShared());

	return MojErrNone;
}
