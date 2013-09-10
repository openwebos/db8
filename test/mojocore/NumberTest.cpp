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
 *  @file NumberTest.cpp
 */

#include <core/MojNumber.h>

#include "Runner.h"

namespace {
    /**
     * mock class for testing MojNumber::Lexer behaviour in terms actions
     */
    struct lexer_test_visitor
    {
        enum TokenType { TokenNegative, TokenMagnitude, TokenFraction, TokenNegativeExponent, TokenExponent, TokenAny };

        struct Token {
            TokenType type;
            int digit;

            Token(TokenType t, int x = -1) : type(t), digit(x) {}

            bool operator==(const Token &other) const {
                if (type == TokenAny || other.type == TokenAny) return true;
                return (type == other.type) && (digit == other.digit);
            }
        };

        std::vector<Token> expected;
        std::vector<Token>::const_iterator start;

        lexer_test_visitor(std::initializer_list<Token> init) : expected(init), start(expected.begin()) {}
        lexer_test_visitor(size_t n, const Token &token) : expected(n, token), start(expected.begin()) {}
        void reset() { start = expected.begin(); }

        MojErr verifyNext() {
            if (expected.end() == start) {
                ADD_FAILURE() << "Unexpected event";
                return MojErrInternal;
            }
            return MojErrNone;
        }

        MojErr negative() {
            MojErrCheck( verifyNext() );
            EXPECT_EQ( *start++, TokenNegative );
            return MojErrNone;
        }
        MojErr digit(int x) {
            MojErrCheck( verifyNext() );
            EXPECT_EQ( *start++, Token(TokenMagnitude, x) );
            return MojErrNone;
        }
        MojErr fractionDigit(int x ) {
            MojErrCheck( verifyNext() );
            EXPECT_EQ( *start++, Token(TokenFraction, x) );
            return MojErrNone;
        }
        MojErr negativeExponent() {
            MojErrCheck( verifyNext() );
            EXPECT_EQ( *start++, TokenNegativeExponent );
            return MojErrNone;
        }
        MojErr exponentDigit(int x) {
            MojErrCheck( verifyNext() );
            EXPECT_EQ( *start++, Token(TokenExponent, x) );
            return MojErrNone;
        }
        MojErr end()
        {
            EXPECT_TRUE( expected.end() == start || start->type == TokenAny );
            return MojErrNone;
        }
    };

    void PrintTo(const lexer_test_visitor::Token &token, ::std::ostream* os)
    {
        *os <<  "Token(" << token.type;
        if (token.digit != -1) *os << ", " << token.digit;
        *os  << ")";
    }

    const lexer_test_visitor::Token tok_any = { lexer_test_visitor::TokenAny };
    const lexer_test_visitor::Token tok_neg = { lexer_test_visitor::TokenNegative };
    const lexer_test_visitor::Token tok_negExp = { lexer_test_visitor::TokenNegativeExponent };
    lexer_test_visitor::Token tok_mag(int x) { return { lexer_test_visitor::lexer_test_visitor::TokenMagnitude, x }; }
    lexer_test_visitor::Token tok_frac(int x) { return { lexer_test_visitor::lexer_test_visitor::TokenFraction, x }; }
    lexer_test_visitor::Token tok_exp(int x) { return { lexer_test_visitor::lexer_test_visitor::TokenExponent, x }; }
}

/**
 * Basic test for lexer
 */
TEST(NumberTest, lexer_basic)
{
    lexer_test_visitor tv{{ tok_neg, tok_mag(3), tok_frac(1), tok_frac(4), tok_exp(0) }};

    // all valid representations of same number (from Lexer point of view)
    for (std::string sample : { "-3.14e0", "-3.14e+0", "-3.14E0", "-3.14E+0" })
    {
        SCOPED_TRACE(sample);
        tv.reset();
        MojExpectNoErr( MojNumber::Lexer::parse(tv, sample) );
    }
}

/**
 * Basic test for numbers with optional plus sign
 */
TEST(NumberTest, lexer_extra_plus)
{
    lexer_test_visitor tv{{ tok_mag(3), tok_frac(1), tok_frac(4), tok_exp(0) }};

    // all valid representations of same number (from Lexer point of view)
    for (std::string sample : { "3.14e0", "3.14e+0", "+3.14e0", "+3.14e+0",
                                "3.14E0", "3.14E+0", "+3.14E0", "+3.14E+0" })
    {
        tv.reset();
        MojExpectNoErr( MojNumber::Lexer::parse(tv, sample) )
            << "Lexer should report events for '" << sample << "' in a same way as for '3.14e0'";
    }
}


TEST(NumberTest, lexer_invalid)
{
    lexer_test_visitor tv(10, tok_any); // maximum 10 events

    // taken from original MojDecimalTest
    for (std::string sample : { "hello", "", ".1", "+-1", "e8", "1.", "1.e8",
                                "1.2e", "1.2e-", "-.e-" })
    {
        tv.reset();
        EXPECT_EQ( MojErrInvalidDecimal, MojNumber::Lexer::parse(tv, sample) )
            << "Lexer should report correct error on parsing '" << sample << "'";
    }
}

/**
 * Basic test for parser
 */
TEST(NumberTest, parser)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "-3.14e0") );
    EXPECT_TRUE( parser.haveFraction() );

    MojDecimal d;
    MojAssertNoErr( parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(-3, 140000), d );
}

TEST(NumberTest, parse_with_rounding)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "0.123456789") );
    EXPECT_TRUE( parser.haveFraction() );

    MojDecimal d;
    MojAssertNoErr( parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(0, 123457), d );
}

/**
 * @test Situation when number without exponent can't be represented in numbers
 *       grid of mantissa. I.e. 0.000000123e7 = 1.23
 */
TEST(NumberTest, parse_unbalanced_small)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "0.000000123e7") );
    EXPECT_TRUE( parser.haveFraction() );

    MojDecimal d;
    MojAssertNoErr( parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(1,230000), d );
}

/**
 * @test Similar to parse_unbalanced_small but with big number.
 *       I.e. 123000000000000000000e-20 = 1.23
 */
TEST(NumberTest, parse_unbalanced_big)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "123000000000000000000e-20") );
    EXPECT_TRUE( parser.haveFraction() );

    MojDecimal d;
    MojAssertNoErr( parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(1,230000), d );
}

/**
 * @test Situation when mantissa digits can't be stored in native number
 */
TEST(NumberTest, parse_mantissa_overflow)
{
    MojNumber::Parser parser;

    EXPECT_EQ( MojErrValueOutOfRange, MojNumber::Lexer::parse(parser, "111111111111111111111") );
}

/**
 * @test Situation when mantissa digits are close to boundary
 */
TEST(NumberTest, parse_mantissa_bound)
{
    MojNumber::Parser parser;

    // MojNumber::Parser allows mantissa nearly as big
    // as: (2^64-1 - 9) `div` 10 * 10 + 9 = 18446744073709551609
    MojExpectNoErr( MojNumber::Lexer::parse(parser, "18446744073709551609") );
}

/**
 * @test Situation when mantissa digits are just crossing boundary
 */
TEST(NumberTest, parse_mantissa_overflow_bound)
{
    MojNumber::Parser parser;

    // note that if we'll use zero as last digit it will not go to exponent
    // rather than to mantissa
    EXPECT_EQ( MojErrValueOutOfRange, MojNumber::Lexer::parse(parser, "18446744073709551611") );
}

/**
 * @test Situation when exponent digits can't be stored in native number
 */
TEST(NumberTest, parse_exponent_overflow)
{
    MojNumber::Parser parser;

    EXPECT_EQ( MojErrValueOutOfRange, MojNumber::Lexer::parse(parser, "0e100000000000000000000") );
}

/**
 * @test Decimal overflow situation due to big mantissa
 */
TEST(NumberTest, parse_decimal_overflow_by_mantissa)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "123000000000000000000") );

    // note that in this case we should get error during rendering
    MojDecimal d;
    EXPECT_EQ( MojErrValueOutOfRange, parser.toDecimal(d) );
}

/**
 * @test Decimal overflow situation due to big exponent
 */
TEST(NumberTest, parse_decimal_overflow_by_exponent)
{
    MojNumber::Parser parser;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "1.23e20") );

    MojDecimal d;
    EXPECT_EQ( MojErrValueOutOfRange, parser.toDecimal(d) );
}

/**
 * @test Boundary case for decimal overflow situation when exponent is still in
 *       allowed range, but product of both mantissa and exponent goes out of
 *       MojDecimal.
 */
TEST(NumberTest, parse_decimal_overflow_by_exponent_bound)
{
    MojNumber::Parser parser;
    MojDecimal d(0,0);

    // log_10 {2^63 - 1} = 18.96
    // 18 - MojDecimal::Precision = 12
    // M * 10^12 > ((2^63 - 1) / 10^6 )  ==>  M > 9.22

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "11e12") );
    EXPECT_EQ( MojErrValueOutOfRange, parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(0,0), d ); // should keep old value
}

/**
 * @test number very close to boundary of maximum MojDecimal but still
 *       parsable.
 *       I.e. ((2^63 - 1) / 10^6) / 10^12 = 9.223372036854775
 *       But double have issue to represent that number exactly, so we found
 *       some other that is close enough.
 */
TEST(NumberTest, parse_decimal_exponent_bound)
{
    MojNumber::Parser parser;
    MojDecimal d;

    MojAssertNoErr( MojNumber::Lexer::parse(parser, "9.22337203685477376e12") );
    MojExpectNoErr( parser.toDecimal(d) );
    EXPECT_EQ( MojDecimal(9.223372036854774376e12), d );
}
