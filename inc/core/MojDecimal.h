/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDECIMAL_H_
#define MOJDECIMAL_H_

#include "core/MojCoreDefs.h"
#include "core/MojHasher.h"

class MojDecimal
{
public:
	static const gint32 Numerator = 1000000;
	static const gsize Precision = 6;
	static const gsize MaxStringSize = 24;
	static const gint64 MagnitudeMax = G_MAXINT64 / Numerator - Numerator;
	static const gint64 MagnitudeMin = G_MININT64 / Numerator + Numerator;
	static const guint32 FractionMax = Numerator - 1;
	static const guint32 FractionMin = 0;

	MojDecimal() : m_rep(0) {}
	MojDecimal(const MojDecimal& dec) : m_rep(dec.m_rep) {}
	explicit MojDecimal(gint64 magnitude, guint32 fraction = 0) { assign(magnitude, fraction); }
	explicit MojDecimal(gdouble d) { assign(d); }

	gint64 magnitude() const { return m_rep / Numerator; }
	guint32 fraction() const;
	gdouble floatValue() const;
	gint64 rep() const { return m_rep; }
	MojErr stringValue(MojChar* buf, gsize size) const;

	MojErr assign(const MojChar* str);
	void assign(gint64 magnitude, guint32 fraction);
	void assign(gdouble d) { m_rep = (gint64) (d * ((gdouble) Numerator)); }
	void assignRep(gint64 rep) { m_rep = rep; }

	MojDecimal& operator=(const MojDecimal& rhs) { m_rep = rhs.m_rep; return *this; }
	bool operator==(const MojDecimal& rhs) const { return m_rep == rhs.m_rep; }
	bool operator!=(const MojDecimal& rhs) const { return m_rep != rhs.m_rep; }
	bool operator<(const MojDecimal& rhs) const { return m_rep < rhs.m_rep; }
	bool operator<=(const MojDecimal& rhs) const { return m_rep <= rhs.m_rep; }
	bool operator>(const MojDecimal& rhs) const { return m_rep > rhs.m_rep; }
	bool operator>=(const MojDecimal& rhs) const { return m_rep >= rhs.m_rep; }

private:
	gint64 m_rep;
};

template<>
struct MojHasher<MojDecimal>
{
	gsize operator()(const MojDecimal& val)
	{
		gint64 rep = val.rep();
		return MojHash(&rep, sizeof(rep));
	}
};

#endif /* MOJDECIMAL_H_ */
