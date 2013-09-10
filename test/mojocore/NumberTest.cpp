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
