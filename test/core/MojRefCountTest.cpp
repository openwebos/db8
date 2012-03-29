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


#include "MojRefCountTest.h"
#include "core/MojAutoPtr.h"

class FooCounted : public MojRefCounted
{
public:
	FooCounted() { ++s_instanceCount; }
	~FooCounted() { --s_instanceCount; }

	static int s_instanceCount;
};

class BarCounted : public FooCounted
{
public:
	BarCounted() {}
};

int FooCounted::s_instanceCount = 0;

MojRefCountTest::MojRefCountTest()
: MojTestCase(_T("MojRefCount"))
{
}

MojErr MojRefCountTest::run()
{
	int* p1 = NULL;
	int* p2 = NULL;

	p1 = (int*) MojRefCountAlloc(0);
	MojTestAssert(p1);
	p2 = (int*) MojRefCountAlloc(sizeof(int));
	MojTestAssert(p2);
	*p2 = 50;
	MojTestAssert(MojRefCountGet(p1) == 1);
	MojTestAssert(MojRefCountGet(p2) == 1);
	MojRefCountRetain(p1);
	MojTestAssert(MojRefCountGet(p1) == 2);
	MojRefCountRetain(p2);
	MojTestAssert(MojRefCountGet(p2) == 2);
	MojTestAssert(*p2 == 50);
	MojRefCountRelease(p1);
	MojTestAssert(MojRefCountGet(p1) == 1);
	MojRefCountRelease(p2);
	MojTestAssert(MojRefCountGet(p2) == 1);
	MojRefCountRelease(p1);
	MojRefCountRelease(p2);

	p1 = (int*) MojRefCountRealloc(NULL, 1);
	MojTestAssert(p1);
	MojTestAssert(MojRefCountGet(p1) == 1);
	p1 = (int*) MojRefCountRealloc(p1, 100);
	MojTestAssert(p1);
	p1 = (int*) MojRefCountRealloc(p1, 0);
	MojTestAssert(p1 == NULL);

	{
		MojRefCountedPtr<FooCounted> f1(new FooCounted());
		MojTestAssert(f1.get());
		f1.reset(new FooCounted());
		MojTestAssert(f1.get());
		MojRefCountedPtr<FooCounted> f2;
		MojTestAssert(!f2.get());
		f2.reset(new FooCounted());
		MojTestAssert(f2.get());
		MojRefCountedPtr<FooCounted> f3(f2);
		MojTestAssert(f3 == f2);
		f2 = f1;
		MojTestAssert(f3 != f2);
		MojTestAssert(f2 == f1);
		MojRefCountedPtr<BarCounted> b1;
		f2 = b1;
	}
	MojTestAssert(FooCounted::s_instanceCount == 0);

	return MojErrNone;
}
