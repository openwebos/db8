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

#include "core/MojNumber.h"
#include "core/MojDecimal.h"
#include "core/MojUtil.h"

void MojDecimal::assign(MojInt64 magnitude, MojUInt32 fraction)
{
	// TODO: think about whether these  should be asserts or errors
	MojAssert(magnitude <= MagnitudeMax && magnitude >= MagnitudeMin);
	MojAssert(fraction <= FractionMax);
	m_rep = magnitude * Numerator;
	if (magnitude < 0)
		m_rep -= fraction;
	else
		m_rep += fraction;
}

MojDouble MojDecimal::floatValue() const
{
	return ((MojDouble) m_rep) / ((MojDouble) Numerator);
}

MojUInt32 MojDecimal::fraction() const
{
	return (MojUInt32) (MojAbs(m_rep) % Numerator);
}

MojErr MojDecimal::stringValue(MojChar* buf, MojSize size) const
{
	MojAssert(buf);
	if (size < MaxStringSize)
		MojErrThrow(MojErrInsufficientBuf);

	// write the full rep in reverse order
	MojInt64 rep = MojAbs(m_rep);
	MojChar tmp[MaxStringSize];
	MojChar* decimalPoint = tmp + Precision;
	MojChar* pos = NULL;
	MojChar* firstDigit = NULL;
	for (pos = tmp; rep != 0; ++pos) {
		MojInt64 next = rep / 10;
		MojChar digit = (MojChar) (_T('0') + rep - (next * 10));
		if (digit != _T('0') && firstDigit == NULL)
			firstDigit = pos;
		*pos = digit;
		rep = next;
	}
	// zero pad up to the decimal point
	while (pos <= decimalPoint) {
		*(pos++) = _T('0');
	}
	// point first digit at the zero, if there was none
	if (firstDigit == NULL || firstDigit > (decimalPoint - 1)) {
		firstDigit = decimalPoint - 1;
	}
	// add minus sign
	if (m_rep < 0) {
		*(buf++) = _T('-');
	}
	// copy to buf, reversing, adding decimal point, and truncating trailing zeros
	while (--pos >= tmp && pos >= firstDigit) {
		*(buf++) = *pos;
		if (pos == decimalPoint)
			*(buf++) = _T('.');
	}
	*buf = _T('\0');

	return MojErrNone;
}


MojErr MojDecimal::assign(const MojChar* str, MojSize n)
{
	MojAssert(str);

    MojNumber::Parser p;
    MojErr err;

    err = MojNumber::Lexer::parse(p, str, n);
    MojErrCheck( err );

    err = p.toDecimal( *this );
    MojErrCheck( err );

    return MojErrNone;
}
