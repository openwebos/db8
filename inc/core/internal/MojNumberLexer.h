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
 *  @file MojNumberLexer.h
 *  Internal header that contains lexer implementation for number parsing
 */

#ifndef __MOJNUMBERLEXER_H
#define __MOJNUMBERLEXER_H

#include <cctype>
#include <string>

#include <glib.h>

#include <core/MojCoreDefs.h>
#include <core/MojErr.h>

namespace MojNumber {

    /**
     * Struct that encapsulates lexer related stuff
     */
    struct Lexer
    {
        /**
         * Main variant of call to lexer.
         *
         * TVisitor should follow model:
         * @code{.cpp}
         * struct MyVisitor {
         *     MojErr negative();
         *     MojErr digit(int digitValue);
         *     MojErr fractionDigit(int digitValue);
         *     MojErr negativeExponent();
         *     MojErr exponentDigit(int digitValue);
         *     MojErr end();
         * };
         * @endcode
         *
         * @param visitor object to which information will be reported to
         * @param number addres of number start
         * @param length length of number
         */
        template<typename TVisitor>
        static MojErr parse(TVisitor &visitor, const char *number, size_t length)
        {
            enum {
                StateNumber,
                StateMagnitude,
                StateMagnitudeDigits,
                StateFraction,
                StateFractionDigits,
                StateExponent,
                StateExponentFirstDigit,
                StateExponentDigits
            } state = StateNumber;

            MojErr err;

            for (const char *numberEnd = number + length;
                 (number != numberEnd) && (G_LIKELY(length != MojSizeMax) || *number != '\0');
                 ++number)
            {
                char c = *number;
                switch (state)
                {
                case StateNumber:
                    state = StateMagnitude;
                    if (c == '-')
                    {
                        err = visitor.negative();
                        MojErrCheck( err );
                        break;
                    }
                    else if (c == '+')
                    {
                        break;
                    }
                    else if (G_UNLIKELY(!isdigit(c)))
                    {
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected sign/digit but got '%c'", c);
                    }
                    // fall through case label

                case StateMagnitude:
                    state = StateMagnitudeDigits;
                    // fall through case label

                case StateMagnitudeDigits:
                    switch (c)
                    {
                    case '0' ... '9':
                        err = visitor.digit(c - '0');
                        MojErrCheck( err );
                        break;
                    case '.':
                        state = StateFraction;
                        break;
                    case 'e': case 'E':
                        state = StateExponent;
                        break;
                    default:
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected digit for magnitude or fraction/exponent delimitor but got '%c'", c);
                    }
                    break;

                case StateFraction:
                    switch (c)
                    {
                    case '0' ... '9':
                        err = visitor.fractionDigit(c - '0');
                        MojErrCheck( err );
                        state = StateFractionDigits;
                        break;
                    default:
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected digit for fraction but got '%c'", c);
                    }
                    break;

                case StateFractionDigits:
                    switch (c)
                    {
                    case '0' ... '9':
                        err = visitor.fractionDigit(c - '0');
                        MojErrCheck( err );
                        break;
                    case 'e': case 'E':
                        state = StateExponent;
                        break;
                    default:
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected digit for fraction or exponent delimitor but got '%c'", c);
                    }
                    break;

                case StateExponent:
                    state = StateExponentFirstDigit;
                    if (c == '-')
                    {
                        err = visitor.negativeExponent();
                        MojErrCheck( err );
                        break;
                    }
                    else if (c == '+')
                    {
                        break;
                    }
#ifdef MOJ_DEBUG
                    else if (G_UNLIKELY(!isdigit(c)))
                    {
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected sign/digit for exponent but got '%c'", c);
                    }
#endif
                    // fall through case label

                case StateExponentFirstDigit:
                    state = StateExponentDigits;
                    // fall through case label

                case StateExponentDigits:
                    switch (c)
                    {
                    case '0' ... '9':
                        err = visitor.exponentDigit(c - '0');
                        MojErrCheck( err );
                        break;
                    default:
                        MojErrThrowMsg(MojErrInvalidDecimal, "Expected digit for exponent but got '%c'", c);
                    }
                    break;
                }
            }

            // validate final state
            switch (state)
            {
            case StateMagnitudeDigits:
            case StateFractionDigits:
            case StateExponentDigits:
                // looks good
                break;

            default:
                MojErrThrowMsg( MojErrInvalidDecimal, "Unexpected end of number in state %d", state );
            };

            err = visitor.end();
            MojErrCheck( err );

            return MojErrNone;
        }

        template<typename TVisitor>
        static MojErr parse(TVisitor &visitor, const std::string &str)
        { return parse(visitor, str.data(), str.length()); }
    };

}

#endif
