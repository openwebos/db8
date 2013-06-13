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


#include "MojStringTest.h"
#include "core/MojString.h"

MojStringTest::MojStringTest()
: MojTestCase("MojString")
{
}

MojErr MojStringTest::run()
{
	MojString str1;
	MojString str2;
	MojString str3;
	MojString str4;
	MojString str5;
	const MojChar* const chars1 = _T("hello");
	const MojChar* const chars2 = _T("howdy!!");
	const MojChar* const chars3 = _T(" hi there  ");
	MojAutoArrayPtr<MojChar> bigFormatStr;
	MojVector<MojString> vec;
	MojSize idx;

	// empty strings
	MojErr err = str2.assign(_T(""));
	MojTestErrCheck(err);
	str3.assign(str2);
	MojTestAssert(str1.data() == str2.data());
	MojTestAssert(str2.data() == str3.data());
	MojTestAssert(str1.length() == 0);
	MojTestAssert(str2.length() == 0);
	MojTestAssert(str3.length() == 0);
	MojTestAssert(str1.empty());
	MojTestAssert(str2.empty());
	MojTestAssert(str3.empty());
	MojTestAssert(str1.compare(_T("")) == 0);
	MojTestAssert(str1.compare(_T(""), 0) == 0);
	MojTestAssert(str1.compareCaseless(_T("")) == 0);
	MojTestAssert(str1.compareCaseless(_T(""), 0) == 0);
	MojTestAssert(str1.startsWith(_T("")));
	MojTestAssert(!str1.startsWith(_T("hello")));
	MojTestAssert(str1 == _T(""));
	MojTestAssert(str1 >= _T(""));
	MojTestAssert(str1 <= _T(""));
	MojTestAssert(!(str1 != _T("")));
	MojTestAssert(!(str1 > _T("")));
	MojTestAssert(!(str1 < _T("")));
	str2.clear();
	MojTestAssert(str1.data() == str2.data());
	str1.swap(str1);
	MojTestAssert(str1 == str2);
	err = str1.assign(_T(""));
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.truncate(0);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.resize(0);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.assign(NULL, 0);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.append(_T(""));
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.append(NULL, 0);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	MojTestAssert(str1.find(_T('h')) == MojInvalidIndex);
	MojTestAssert(str1.rfind(_T('h')) == MojInvalidIndex);
	err = str1.substring(0, 0, str2);
	MojTestErrCheck(err);
	MojTestAssert(str2.empty());
	err = str1.split(_T(' '), vec);
	MojTestErrCheck(err);
	MojTestAssert(vec.empty());
	MojString::Iterator iter;
	err = str1.begin(iter);
	MojTestErrCheck(err);
	MojTestAssert(iter == str1.begin());
	MojTestAssert(*iter == 0);
	err = str1.end(iter);
	MojTestErrCheck(err);
	MojTestAssert(iter == str1.end());
	MojTestAssert(*iter == 0);

	// non-empty strings
	err = str1.assign(chars1);
	MojTestErrCheck(err);
	err = str2.assign(chars2);
	MojTestErrCheck(err);
	str3.assign(str1);
	err = str4.assign(_T("hEllo World!"), 5);
	MojTestErrCheck(err);
	err = str4.setAt(0, _T('H'));
	MojTestErrCheck(err);
	err = str4.setAt(4, _T('O'));
	MojTestErrCheck(err);
	MojTestAssert(str4 == _T("HEllO"));
	MojTestAssert(str1.data() != chars1);
	MojTestAssert(str1.length() == 5);
	MojTestAssert(str2.length() == 7);
	MojTestAssert(str3.length() == 5);
	MojTestAssert(!str1.empty());
	MojTestAssert(!str2.empty());
	MojTestAssert(!str3.empty());
	MojTestAssert(str1.compare(chars1) == 0);
	MojTestAssert(str1.compare(chars1, 3) == 0);
	MojTestAssert(str1.compare(chars1, 3000) == 0);
	MojTestAssert(str1.compare(str3) == 0);
	MojTestAssert(str1.compare(chars2) < 0);
	MojTestAssert(str1.compare(chars2, 2) < 0);
	MojTestAssert(str1.compare(str2) < 0);
	MojTestAssert(str1.compare(str4) > 0);
	MojTestAssert(str2.compare(chars1) > 0);
	MojTestAssert(str2.compare(str1) > 0);
	MojTestAssert(str1.compareCaseless(chars1) == 0);
	MojTestAssert(str1.compareCaseless(chars1, 3) == 0);
	MojTestAssert(str1.compareCaseless(str3) == 0);
	MojTestAssert(str1.compareCaseless(str4) == 0);
	MojTestAssert(str1.compareCaseless(chars2) < 0);
	MojTestAssert(str1.compareCaseless(str2) < 0);
	MojTestAssert(str2.compareCaseless(chars1) > 0);
	MojTestAssert(str2.compareCaseless(str1) > 0);
	MojTestAssert(str1.startsWith(_T("")));
	MojTestAssert(str1.startsWith(_T("he")));
	MojTestAssert(str1.startsWith(chars1));
	MojTestAssert(str1.startsWith(str1));
	MojTestAssert(str1.at(0) == chars1[0] && str1.at(3) == chars1[3]);
	MojTestAssert(str1 == chars1);
	MojTestAssert(str1 == str3);
	MojTestAssert(str1 != chars2);
	MojTestAssert(str1 != str2);
	MojTestAssert(str1 != str4);
	MojTestAssert(str1 < chars2);
	MojTestAssert(str1 < str2);
	MojTestAssert(str1 <= chars2);
	MojTestAssert(str1 <= str2);
	MojTestAssert(str1 <= chars1);
	MojTestAssert(str1 <= str3);
	MojTestAssert(str2 > chars1);
	MojTestAssert(str2 > str1);
	MojTestAssert(str2 >= chars1);
	MojTestAssert(str2 >= str2);
	MojTestAssert(str1 >= chars1);
	MojTestAssert(str1 >= str3);
	str1.clear();
	MojTestAssert(str1.empty());
	MojTestAssert(str1.length() == 0);
	MojTestAssert(str1 == _T(""));

	// self-assignment
	err = str1.assign(chars1);
	MojTestErrCheck(err);
	str1.assign(str1);
	MojTestAssert(str1 == chars1);
	err = str1.substring(0, 2, str1);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("he"));

	// find char
	err = str1.assign(chars3);
	MojTestErrCheck(err);
	idx = str1.find(_T(' '));
	MojTestAssert(idx == 0);
	idx = str1.find(_T(' '), idx + 1);
	MojTestAssert(idx == 3);
	idx = str1.find(_T(' '), idx + 1);
	MojTestAssert(idx == 9);
	idx = str1.find(_T(' '), idx + 1);
	MojTestAssert(idx == 10);
	idx = str1.find(_T('?'), idx);
	MojTestAssert(idx == MojInvalidIndex);
	idx = str1.rfind(_T(' '));
	MojTestAssert(idx == 10);
	idx = str1.rfind(_T(' '), idx - 1);
	MojTestAssert(idx == 9);
	idx = str1.rfind(_T(' '), idx - 1);
	MojTestAssert(idx == 3);
	idx = str1.rfind(_T(' '), idx - 1);
	MojTestAssert(idx == 0);
	idx = str1.rfind(_T('&'));
	MojTestAssert(idx == MojInvalidIndex);
	// find str
	idx = str1.find(_T(" "));
	MojTestAssert(idx == 0);
	idx = str1.find(_T(" "), idx + 1);
	MojTestAssert(idx == 3);
	idx = str1.find(_T(" "), idx + 1);
	MojTestAssert(idx == 9);
	idx = str1.find(_T(" "), idx + 1);
	MojTestAssert(idx == 10);
	idx = str1.find(_T("?"), idx);
	MojTestAssert(idx == MojInvalidIndex);
	idx = str1.find(_T("hi"));
	MojTestAssert(idx == 1);

	// split
	err = str1.assign(_T(" hi   there   !  "));
	MojTestErrCheck(err);
	err = str1.split(_T(' '), vec);
	MojTestErrCheck(err);
	MojTestAssert(vec.size() == 3);
	MojTestAssert(vec[0] == _T("hi"));
	MojTestAssert(vec[1] == _T("there"));
	MojTestAssert(vec[2] == _T("!"));
	err = str1.assign(_T("     "));
	MojTestErrCheck(err);
	err = str1.split(_T(' '), vec);
	MojTestErrCheck(err);
	MojTestAssert(vec.empty());
	err = str1.assign(_T("howdy"));
	MojTestErrCheck(err);
	err = str1.split(_T(' '), vec);
	MojTestErrCheck(err);
	MojTestAssert(vec.size() == 1);
	MojTestAssert(vec[0] == _T("howdy"));
	err = str1.assign(_T("a b"));
	MojTestErrCheck(err);
	err = str1.split(_T(' '), vec);
	MojTestErrCheck(err);
	MojTestAssert(vec.size() == 2);
	MojTestAssert(vec[0] == _T("a"));
	MojTestAssert(vec[1] == _T("b"));

	// ref counting
	err = str1.assign(chars1);
	MojTestErrCheck(err);
	str2 = str1;
	MojTestAssert(str1.data() == str2.data());
	err = str2.assign(chars2);
	MojTestErrCheck(err);
	MojTestAssert(str1.data() != str2.data());
	str1.swap(str2);
	MojTestAssert(str1 == chars2);
	MojTestAssert(str2 == chars1);

	// format
	err = str1.format(_T("hello %s %d"), _T("world"), 14);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello world 14"));
	MojTestAssert(str1.length() == 14);
	MojTestAssert(str1.first() == _T('h') && str1.last() == _T('4'));
	bigFormatStr.reset(new MojChar[10003]);
	MojAllocCheck(bigFormatStr.get());
	MojStrCpy(bigFormatStr.get(), _T("%s"));
	for (int i = 2; i < 10002; i++)
		bigFormatStr.get()[i] = _T('f');
	bigFormatStr.get()[10002] = _T('\0');
	err = str1.format(bigFormatStr.get(), _T("hi"));
	MojTestErrCheck(err);
	MojTestAssert(str1.length() == 10002);
	MojTestAssert(str1.at(0) == _T('h') && str1.at(1) == _T('i'));
	for (int i = 2; i < 10002; ++i)
		MojTestAssert(str1.at(i) == _T('f'));

	// append
	str1.clear();
	for (int i = 0; i < 1000; ++i) {
		err = str1.append(_T('a'));
		MojTestErrCheck(err);
		MojTestAssert(str1.at(i) == _T('a'));
	}
	MojTestAssert(str1.length() == 1000);
	str1.clear();
	err = str1.append(_T("hello"));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello"));
	err = str1.append(_T('!'));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello!"));
	err = str1.appendFormat(_T(" %s"), _T("world"));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello! world"));
	err = str1.append(_T('!'));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello! world!"));

	// truncate
	err = str1.truncate(1000);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello! world!"));
	err = str1.truncate(5);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello"));
	err = str1.truncate(0);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T(""));

	// reserve
	MojTestAssert(str5.capacity() == 0);
	err = str5.reserve(0);
	MojTestErrCheck(err);
	MojTestAssert(str5.capacity() == 0);
	err = str5.reserve(5);
	MojTestErrCheck(err);
	MojTestAssert(str5.capacity() == 5);
	err = str5.reserve(1);
	MojTestErrCheck(err);
	MojTestAssert(str5.capacity() == 5);
	err = str5.assign(_T("hello"));
	MojTestErrCheck(err);
	err = str5.reserve(1000);
	MojTestErrCheck(err);
	MojTestAssert(str5.capacity() == 1000);
	MojTestAssert(str5 == _T("hello"));

	// resize
	err = str1.assign(_T("hello"));
	MojTestErrCheck(err);
	err = str1.resize(4);
	MojTestErrCheck(err);
	MojTestAssert(str1.length() == 4);
	MojTestAssert(str1 == _T("hell"));
	err = str1.resize(100);
	MojTestErrCheck(err);
	MojTestAssert(str1.length() == 100);
	err = str1.resize(1);
	MojTestAssert(str1.length() == 1);
	MojTestAssert(str1 == _T("h"));
	err = str1.resize(0);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());

	// ref-count
	err = str1.assign(_T("hello"));
	MojTestErrCheck(err);
	str2 = str1;
	err = str1.setAt(0, _T('H'));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("Hello"));
	MojTestAssert(str2 == _T("hello"));
	str2 = str1;
	err = str1.format(_T("%d"), 52);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("52"));
	MojTestAssert(str2 == _T("Hello"));
	str2 = str1;
	err = str1.append(_T('0'));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("520"));
	MojTestAssert(str2 == _T("52"));
	str2 = str1;
	err = str1.append(_T("1"));
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("5201"));
	MojTestAssert(str2 == _T("520"));
	str2 = str1;
	err = str1.appendFormat(_T("%d"), 89);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("520189"));
	MojTestAssert(str2 == _T("5201"));
	str2 = str1;
	err = str1.reserve(1000);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("520189"));
	MojTestAssert(str2 == _T("520189"));
	str2 = str1;
	err = str1.truncate(2);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("52"));
	MojTestAssert(str2 == _T("520189"));

	// base64
	MojVector<MojByte> bv;
	str1.clear();
	err = str1.base64Encode(bv);
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	err = str1.base64Decode(bv);
	MojTestErrCheck(err);
	MojTestAssert(bv.empty());
	MojByte input[] = {0x14, 0xfb, 0x9c, 0x03, 0xd9};
	err = bv.assign(input, input + sizeof(input));
	MojTestErrCheck(err);
	err = str1.base64Encode(bv);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("4EiR+xZ="));
	MojVector<MojByte> bv2;
	err = str1.base64Decode(bv2);
	MojTestErrCheck(err);
	MojTestAssert(bv == bv2);
	err = str1.base64Encode(bv, false);
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("4EiR+xZ"));
	err = str1.base64Decode(bv2);
	MojTestErrCheck(err);
	MojTestAssert(bv == bv2);

	// tolower
	str1.clear();
	err = str1.toLower();
	MojTestErrCheck(err);
	MojTestAssert(str1.empty());
	MojTestAssert(str1 == _T(""));
	err = str1.assign(_T("HeLlO"));
	MojTestErrCheck(err);
	err = str1.toLower();
	MojTestErrCheck(err);
	MojTestAssert(str1 == _T("hello"));
	err = str1.assign(_T("AnOtHER"));
	MojErrCheck(err);
	err = str1.toLower();
	MojErrCheck(err);
	MojTestAssert(str1 == _T("another"));

	return MojErrNone;
}
