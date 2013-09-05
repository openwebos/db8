/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
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
 * LICENSE@@@
 ****************************************************************/

/**
 *  @file DecimalTest.cpp
 */

#include <cmath>

#include <core/MojDecimal.h>

#include "Runner.h"

TEST(DecimalTest, ctor_magnitude_fraction)
{
    MojDecimal d2(100, 65432);

    EXPECT_EQ( 100, d2.magnitude() );
    EXPECT_EQ( 65432, d2.fraction() );
}

TEST(DecimalTest, ctor_double)
{
    EXPECT_EQ( MojDecimal(2,718282), MojDecimal(2.718282) );
}

TEST(DecimalTest, ctor_double_with_rounding)
{
    EXPECT_EQ( MojDecimal(0, 123457), MojDecimal(0.123456789) );
}

TEST(DecimalTest, floatValue)
{
    MojDecimal d2(100, 65432);
    MojDecimal d3(3.1415);

    MojDouble d = d2.floatValue();
    // to work-around x*10^(-n) to y*2^(-m) conversion use integer part
    EXPECT_EQ( 100065432, round(d * 1000000) );

    d = d3.floatValue();
    EXPECT_EQ( 31415, round(d * 10000) );
}

TEST(DecimalTest, parse_with_rounding)
{
    MojDecimal d;
    MojAssertNoErr( d.assign("0.123456789") );
    EXPECT_EQ( MojDecimal(0,123457), d );
}

/**
 * @test Situation when number without exponent can't be represented in numbers
 *       grid of mantissa. I.e. 0.000000123e7 = 1.23
 */
TEST(DecimalTest, parse_unbalanced_small)
{
    MojDecimal d;
    MojAssertNoErr( d.assign("0.000000123e7") );
    EXPECT_EQ( MojDecimal(1,230000), d );
}

/**
 * @test Similar to parser_unbalanced_small but with big number.
 *       I.e. 123000000000000000000e-20 = 1.23
 */
TEST(DecimalTest, parse_unbalanced_big)
{
    MojDecimal d;
    MojAssertNoErr( d.assign("123000000000000000000e-20") );
    EXPECT_EQ( MojDecimal(1,230000), d );
}

/**
 * @test Decimal overflow situation due to big mantissa
 */
TEST(DecimalTest, parse_overflow_mantissa)
{
    MojDecimal d;
    EXPECT_EQ( MojErrValueOutOfRange, d.assign("123000000000000000000") );
}

/**
 * @test Decimal overflow situation due to big exponent
 */
TEST(DecimalTest, parse_overflow_exponent)
{
    MojDecimal d;
    EXPECT_EQ( MojErrValueOutOfRange, d.assign("1.23e20") );
}

/**
 * @test Boundary case for decimal overflow situation when exponent is still in
 *       allowed range, but product of both mantissa and exponent goes out of
 *       MojDecimal.
 */
TEST(DecimalTest, parse_overflow_exponent_bound)
{
    MojDecimal d(0, 0);

    // log_10 {2^63 - 1} = 18.96
    // 18 - MojDecimal::Precision = 12
    // M * 10^12 > ((2^63 - 1) / 10^6 )  ==>  M > 9.22

    EXPECT_EQ( MojErrValueOutOfRange, d.assign("11e12") );
    EXPECT_EQ( MojDecimal(0, 0), d ); // should keep old value
}

TEST(DecimalTest, parse_9e12)
{
    MojDecimal d;
    MojExpectNoErr( d.assign("9e12") );
    EXPECT_EQ( MojDecimal(9e12), d );
}

TEST(DecimalTest, floatValue_reconvert)
{
    for (MojUInt32 fraction = 0; fraction < MojDecimal::Numerator; ++fraction)
    {
        MojDecimal d(0, fraction);
        ASSERT_EQ( d, MojDecimal(d.floatValue()) );
    }
}

TEST(DecimalTest, original)
{
    MojDecimal d1;
    MojDecimal d2(100, 65432);
    MojDecimal d3(3.1415);
    MojDecimal d4(d2);

    EXPECT_EQ( 0, d1.magnitude() ) << "Default constructor should set magnitude to zero";
    EXPECT_EQ( 0, d1.fraction() ) << "Default constructor should set fraction to zero";

    EXPECT_EQ( 100, d2.magnitude() );
    EXPECT_EQ( 65432, d2.fraction() );
    // to work-around x*10^(-n) to y*2^(-m) conversion use integer part
    EXPECT_EQ( 100065432, round(d2.floatValue() * 1000000) );
    EXPECT_EQ( MojDecimal(100.065432), d2 );

    EXPECT_EQ( 3, d3.magnitude() );
    EXPECT_EQ( 141500, d3.fraction() );
    EXPECT_EQ( MojDecimal(3.1415), d3 );

    EXPECT_EQ( 100, d4.magnitude() ) << "Magnitude of copy should match";
    EXPECT_EQ( 65432, d4.fraction() ) << "Fraction of copy should match";
    EXPECT_EQ( d2, d4 ) << "Copy and original should match";

    EXPECT_TRUE(d1 < d3);
    EXPECT_TRUE(d2 == d4);
    EXPECT_TRUE(d2 <= d4);
    EXPECT_TRUE(d1 <= d3);
    EXPECT_TRUE(d2 > d3);
    EXPECT_TRUE(d1 >= d1);
    EXPECT_TRUE(d2 >= d3);

    d1.assign(MojDecimal::MagnitudeMax, MojDecimal::FractionMax);
    EXPECT_EQ( MojDecimal::MagnitudeMax, d1.magnitude() );
    EXPECT_EQ( (MojUInt32)MojDecimal::FractionMax, d1.fraction() );

    d2.assign(28.89);
    EXPECT_EQ( 28, d2.magnitude() );
    EXPECT_EQ( 890000, d2.fraction() );

    d1.assign(-5.28);
    EXPECT_EQ( -528, round(d1.floatValue() * 100) );
    EXPECT_EQ( -5, d1.magnitude() );
    EXPECT_EQ( 280000, d1.fraction() );

    d1.assign(-987, 654);
    EXPECT_EQ( -987, d1.magnitude() );
    EXPECT_EQ( 654, d1.fraction() );
    EXPECT_EQ( MojDecimal(-987.000654), d1 );

    MojChar buf[MojDecimal::MaxStringSize];
    MojAssertNoErr( MojDecimal().stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("0.0")));
    MojAssertNoErr( MojDecimal(0, 0).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("0.0")));
    MojAssertNoErr( MojDecimal(1, 0).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("1.0")));
    MojAssertNoErr( MojDecimal(0, 100000).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("0.1")));
    MojAssertNoErr( MojDecimal(0, 7).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("0.000007")));
    MojAssertNoErr( MojDecimal(1234567, 654321).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("1234567.654321")));
    MojAssertNoErr( MojDecimal(4, 300).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("4.0003")));

    MojAssertNoErr( MojDecimal(-1, 7).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("-1.000007")));
    MojAssertNoErr( MojDecimal(-1234567, 654321).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("-1234567.654321")));
    MojAssertNoErr( MojDecimal(-4, 300).stringValue(buf, sizeof(buf)) );
    ASSERT_TRUE(!MojStrCmp(buf, _T("-4.0003")));


    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("hello")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T(".1")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("+-1")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("e8")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("1.")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("1.e8")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("1.2e")) );
    EXPECT_EQ( MojErrInvalidDecimal, d1.assign(_T("3.2e-")) );

    MojAssertNoErr( d1.assign(_T("1")) );
    EXPECT_EQ( MojDecimal(1, 0), d1 );
    MojAssertNoErr( d1.assign(_T("-1")) );
    EXPECT_EQ( MojDecimal(-1, 0), d1 );
    MojAssertNoErr( d1.assign(_T("123")) );
    EXPECT_EQ( MojDecimal(123, 0), d1 );
    MojAssertNoErr( d1.assign(_T("+123")) );
    EXPECT_EQ( MojDecimal(123, 0), d1 );
    MojAssertNoErr( d1.assign(_T("-16789")) );
    EXPECT_EQ( MojDecimal(-16789, 0), d1 );
    MojAssertNoErr( d1.assign(_T("1.0")) );
    EXPECT_EQ( MojDecimal(1, 0), d1 );
    MojAssertNoErr( d1.assign(_T("1.1")) );
    EXPECT_EQ( MojDecimal(1, 100000), d1 );
    MojAssertNoErr( d1.assign(_T("1.01")) );
    EXPECT_EQ( MojDecimal(1, 10000), d1 );
    MojAssertNoErr( d1.assign(_T("1.123456")) );
    EXPECT_EQ( MojDecimal(1, 123456), d1 );
    MojAssertNoErr( d1.assign(_T("+1.12345678")) );
    EXPECT_EQ( MojDecimal(1, 123456), d1 );
    MojAssertNoErr( d1.assign(_T("1e3")) );
    EXPECT_EQ( MojDecimal(1000, 0), d1 );
    MojAssertNoErr( d1.assign(_T("1.0e3")) );
    EXPECT_EQ( MojDecimal(1000, 0), d1 );
    MojAssertNoErr( d1.assign(_T("-1e3")) );
    EXPECT_EQ( MojDecimal(-1000, 0), d1 );
    MojAssertNoErr( d1.assign(_T("1e-3")) );
    EXPECT_EQ( MojDecimal(0, 1000), d1 );
    MojAssertNoErr( d1.assign(_T("+1.12345678e5")) );
    EXPECT_EQ( MojDecimal(112345, 600000), d1 );
    MojAssertNoErr( d1.assign(_T("+1.12345678e-5")) );
    EXPECT_EQ( MojDecimal(0, 11), d1 );
}
