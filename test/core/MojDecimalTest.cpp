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
* Filename              : MojDecimalTest.cpp
* Description           : Source file for MojDecimal test.
****************************************************************************************************
**/

#include <cmath>

#include "MojDecimalTest.h"
#include "core/MojDecimal.h"

MojDecimalTest::MojDecimalTest()
: MojTestCase(_T("MojDecimal"))
{
}
/**
****************************************************************************************************
* @run
                 1. MojDecimal class stores decimal numbers. Internally it stores each
                    decimal number in the form of magnitude and fraction with 6 digit
                    precision. It is possible to read both magnitude and fraction
                    parts using utility functions.
                    Ex: magnitude() returns magnitude value.
                        fraction() returns fraction value.
                        floatValue() utility converts magnitude and fraction into double number and
                        returns to caller.
                 2. magnitude and fraction digits are internally stored as long long int number.
                 3. stringValue() utility function converts decimal numbers and copies in the form
                    of string.
* @param         :  None
* @retval        :  MojErr
****************************************************************************************************
**/
MojErr MojDecimalTest::run()
{
	MojDecimal d1;
	MojDecimal d2(100, 65432);
	MojDecimal d3(3.1415);
	MojDecimal d4(d2);

	MojTestAssert(d1.magnitude() == 0 && d1.fraction() == 0);
	MojTestAssert(d2.magnitude() == 100 && d2.fraction() == 65432);
	MojDouble d = d2.floatValue();
    // to work-around x*10^(-n) to y*2^(-m) conversion use integer part
    MojTestAssert(round(d * 1000000) == 100065432);
	MojTestAssert(d3.magnitude() == 3 && d3.fraction() == 141500);
	d = d3.floatValue();
    MojTestAssert(round(d * 10000) == 31415);
	MojTestAssert(d4.magnitude() == 100 && d2.fraction() == 65432);

	MojTestAssert(d1 < d3);
	MojTestAssert(d2 == d4);
	MojTestAssert(d2 <= d4);
	MojTestAssert(d1 <= d3);
	MojTestAssert(d2 > d3);
	MojTestAssert(d2 >= d1);
	MojTestAssert(d2 >= d3);

	d1.assign(MojDecimal::MagnitudeMax, MojDecimal::FractionMax);
	MojTestAssert(d1.magnitude() == MojDecimal::MagnitudeMax && d1.fraction() == MojDecimal::FractionMax);
	d1.assign(MojDecimal::MagnitudeMin, MojDecimal::FractionMax);
	MojTestAssert(d1.magnitude() == MojDecimal::MagnitudeMin && d1.fraction() == MojDecimal::FractionMax);
	d2.assign(28.89);
	MojTestAssert(d2.magnitude() == 28 && d2.fraction() == 890000);

	d1.assign(-5.28);
	d = d1.floatValue();
    MojTestAssert(round(d * 100) == -528);
	MojInt64 m = d1.magnitude();
	MojInt64 f = d1.fraction();
	MojTestAssert(m == -5 && f == 280000);
	d1.assign(-987, 654);
	MojTestAssert(d1.magnitude() == -987 && d1.fraction() == 654);
	d = d1.floatValue();
    MojTestAssert(round(d * 1000000) == -987000654);

	MojChar buf[MojDecimal::MaxStringSize];
	MojErr err = MojDecimal().stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("0.0")));
	err = MojDecimal(0, 0).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("0.0")));
	err = MojDecimal(1, 0).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("1.0")));
	err = MojDecimal(0, 100000).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("0.1")));
	err = MojDecimal(0, 7).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("0.000007")));
	err = MojDecimal(1234567, 654321).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("1234567.654321")));
	err = MojDecimal(4, 300).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("4.0003")));

	err = MojDecimal(-1, 7).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("-1.000007")));
	err = MojDecimal(-1234567, 654321).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("-1234567.654321")));
	err = MojDecimal(-4, 300).stringValue(buf, sizeof(buf));
	MojTestErrCheck(err);
	MojTestAssert(!MojStrCmp(buf, _T("-4.0003")));


	err = d1.assign(_T("hello"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T(""));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T(".1"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("+-1"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("e8"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("1."));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("1.e8"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("1.2e"));
	MojTestErrExpected(err, MojErrInvalidDecimal);
	err = d1.assign(_T("3.2e-"));
	MojTestErrExpected(err, MojErrInvalidDecimal);

	err = d1.assign(_T("1"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 0));
	err = d1.assign(_T("-1"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(-1, 0));
	err = d1.assign(_T("123"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(123, 0));
	err = d1.assign(_T("+123"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(123, 0));
	err = d1.assign(_T("-16789"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(-16789, 0));
	err = d1.assign(_T("1.0"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 0));
	err = d1.assign(_T("1.1"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 100000));
	err = d1.assign(_T("1.01"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 10000));
	err = d1.assign(_T("1.123456"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 123456));
	err = d1.assign(_T("+1.12345678"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1, 123457));
	err = d1.assign(_T("1e3"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1000, 0));
	err = d1.assign(_T("1.0e3"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(1000, 0));
	err = d1.assign(_T("-1e3"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(-1000, 0));
	err = d1.assign(_T("1e-3"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(0, 1000));
	err = d1.assign(_T("+1.12345678e5"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(112345, 678000));
	err = d1.assign(_T("+1.12345678e-5"));
	MojTestErrCheck(err);
	MojTestAssert(d1 == MojDecimal(0, 11));

	return MojErrNone;
}
