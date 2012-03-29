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


MojErr MojDecimal::assign(const MojChar* str)
{
	MojAssert(str);

	enum State {
		StateSign,
		StateIntegerBegin,
		StateInteger,
		StateFractionBegin,
		StateFraction,
		StateExponentSign,
		StateExponentBegin,
		StateExponent,
	};

	MojChar c = 0;
	State state = StateSign;
	MojInt64 rep = 0;
	MojInt64 exp = 0;
	MojInt64 fracMultiplier = Numerator / 10;
	bool negative = false;
	bool negativeExp = false;

	while ((c = *str)) {
Redo:
		switch (state) {
		case StateSign: {
			switch (c) {
			case '-':
				negative = true;
			case '+':
				state = StateIntegerBegin;
				break;
			default:
				state = StateIntegerBegin;
				goto Redo;
			}
			break;
		}
		case StateIntegerBegin: {
			if (!MojIsDigit(c))
				MojErrThrow(MojErrInvalidDecimal);
			rep = (c - '0') * Numerator;
			state = StateInteger;
			break;
		}
		case StateInteger: {
			switch (c) {
			case '.':
				state = StateFractionBegin;
				break;
			case 'E':
			case 'e':
				state = StateExponentSign;
				break;
			default:
				if (!MojIsDigit(c))
					MojErrThrow(MojErrInvalidDecimal);
				// TODO: deal with overflow
				rep = (rep * 10) + ((c - '0') * Numerator);
				break;
			}
			break;
		}
		case StateFractionBegin: {
			if (!MojIsDigit(c))
				MojErrThrow(MojErrInvalidDecimal);
			rep += (c - '0') * fracMultiplier;
			state = StateFraction;
			break;
		}
		case StateFraction: {
			switch (c) {
			case 'E':
			case 'e':
				state = StateExponentSign;
				break;
			default:
				if (!MojIsDigit(c))
					MojErrThrow(MojErrInvalidDecimal);
				fracMultiplier /= 10;
				rep += (c - '0') * fracMultiplier;
				break;
			}
			break;
		}
		case StateExponentSign: {
			switch (c) {
			case '-':
				negativeExp = true;
			case '+':
				state = StateExponentBegin;
				break;
			default:
				state = StateExponentBegin;
				goto Redo;
			}
			break;
		}
		case StateExponentBegin: {
			if (!MojIsDigit(c))
				MojErrThrow(MojErrInvalidDecimal);
			exp = c - '0';
			state = StateExponent;
			break;
		}
		case StateExponent: {
			if (!MojIsDigit(c))
				MojErrThrow(MojErrInvalidDecimal);
			exp = (exp * 10) + (c - '0');
			break;
		}
		}
		str++;
	}

	// must be in valid end state
	if (state != StateInteger && state != StateFraction && state != StateExponent)
		MojErrThrow(MojErrInvalidDecimal);
	// apply exponent
	if (negativeExp) {
		while (exp) {
			rep /= 10;
			exp--;
		}
	} else {
		while (exp) {
			// TODO: deal with overflow
			rep *= 10;
			exp--;
		}
	}
	// and negate
	if (negative)
		rep = -rep;

	m_rep = rep;

	return MojErrNone;
}
