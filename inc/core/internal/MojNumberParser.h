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
 *  @file MojNumberParser.h
 *  Header file that contains implementation for number parser and can be fused
 *  with appropriate lexer part.
 */

#ifndef __MOJNUMBERPARSER_H
#define __MOJNUMBERPARSER_H

#include <cinttypes>

#include <glib.h>

#include <core/MojDecimal.h>

namespace MojNumber {

    /**
     * Implementation of natural power calculation for using with integers and
     * with any object for which pow() can't be applied cleanly
     */
    template<typename T>
    T npow(T x, size_t exp)
    {
        constexpr size_t firstBit = (INT_MAX >> 1) + 1;
        T y = 1;
        for(size_t bit = firstBit; bit != 0; bit >>= 1)
        {
            y *= y;
            if ((exp & bit) != 0) y *= x;
        }
        return y;
    }

    /**
     * Class that provides state of parser and can be used as visitor for MojNumber::Lexer
     */
    class Parser
    {
        bool positive;
        uint64_t value;
        int valueExp;

        bool exponentPositive;
        int exponent;

        static constexpr int base = 10;
        static constexpr uint64_t bound = (UINT64_MAX - (base - 1)) / base;

        static constexpr int exponentBase = 10;
        static constexpr int exponentBound = (INT_MAX - (base - 1)) / base;

        static constexpr int int64_pow10_max = 18; // log_10 {2^63 - 1} = 18.96

    public:
        Parser()
        { reset(); }

        /**
         * Reset parser to initial state
         *
         * Can be used for parser re-use
         */
        void reset()
        {
            positive = true;
            value = 0;
            valueExp = 0;
            exponentPositive = true;
            exponent = 0;
        }

        /**
         * Check if we have any fractional part
         *
         * Can be used to differentiate integers and floating/decimal numbers
         */
        bool haveFraction() const
        { return (valueExp + exponent) < 0; }

        /**
         * Render parsed number as MojDecimal
         */
        MojErr toDecimal(MojDecimal &decimal) const
        {
            MojUInt64 rep = value;
            const int alignExp = MojDecimal::Precision + valueExp + exponent;

            if (alignExp < 0)
            {
                // drop all but last extra digits
                if (alignExp < -1) rep /= npow((uint64_t)base, (size_t)(-alignExp - 1));

                // round logic
                if ((int)((rep % base) * 2) > base)
                {
                    rep = rep / base + 1;
                }
                else
                {
                    rep /= base;
                }
            }
            else
            {
                // check that we able to call npow(10, alignExp)
                if (G_UNLIKELY(alignExp > int64_pow10_max))
                {
                    MojErrThrowMsg( MojErrValueOutOfRange, "Too big value for MojDecimal %" PRIu64 " * %d^%d",
                                    value, base, alignExp - MojDecimal::Precision );
                }

                const int64_t factor = npow((uint64_t)base, alignExp);

                // check that value with exponent will not cross boundary
                if (G_UNLIKELY(value > (INT64_MAX / factor)))
                {
                    MojErrThrowMsg( MojErrValueOutOfRange, "Too big value for MojDecimal %" PRIu64 " * %" PRIu64 " / %d",
                                    value, factor, MojDecimal::Numerator );
                }
                rep *= factor;
            }
            if (!positive) rep = -rep;
            decimal.assignRep(rep);
            return MojErrNone;
        }

        // interface for MojNumber::Lexer
        MojErr negative() {
            positive = false;
            return MojErrNone;
        }

        MojErr digit(int x) {
            if (G_UNLIKELY(value != 0 && x == 0)) // special handling for trailing zeroes
            {
                if (G_UNLIKELY(valueExp == INT_MAX))
                {
                    MojErrThrowMsg( MojErrValueOutOfRange, "Too big value for decimal %" PRIu64 " * %d^%d", value, base, valueExp );
                }
                ++valueExp; // postpone
                return MojErrNone;
            }

            MojErr err;
            err = realignValue();
            MojErrCheck( err );

            err = pushDigit(x);
            MojErrCheck( err );

            return MojErrNone;
        }

        MojErr fractionDigit(int x) {
            MojErr err;
            err = realignValue(); // if there was some leading
            MojErrCheck( err );

            --valueExp;
            err = pushDigit(x);
            MojErrCheck( err );


            return MojErrNone;
        }

        MojErr negativeExponent() {
            exponentPositive = false;
            return MojErrNone;
        }

        MojErr exponentDigit(int x) {
            if (G_UNLIKELY(exponent == 0 && x == 0)) // skip leading zeroes
            {
                return MojErrNone;
            }

            if (G_UNLIKELY(exponent >= exponentBound))
            {
                MojErrThrowMsg( MojErrValueOutOfRange, "Too big value for exponent %d * %d + %d", exponent, exponentBase, x );
            }

            exponent = exponent * exponentBase + x;
            return MojErrNone;
        }

        MojErr end()
        {
            if (!exponentPositive) exponent = -exponent;
            return MojErrNone;
        }

    private:
        /**
         * Re-align value according to valueExp to work as with simple integer
         */
        MojErr realignValue() {
            if (G_UNLIKELY(valueExp > 0)) // re-align from sequence of zeroes
            {
                value *= npow((uint64_t)base, valueExp);
                valueExp = 0;
            }
            return MojErrNone;
        }

        /**
         * Push single digit into value
         */
        MojErr pushDigit(int x) {
            MojAssert( valueExp <= 0 );
            if (G_UNLIKELY(value > bound))
            {
                MojErrThrowMsg( MojErrValueOutOfRange, "Too big value for decimal %" PRIu64 " * %d + %d", value, base, x );
            }

            value = value * base + x;
            return MojErrNone;
        }

    };
}

#endif
