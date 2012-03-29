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


#ifndef MOJDBOBJECTHEADER_H_
#define MOJDBOBJECTHEADER_H_

#include "core/MojNoCopy.h"
#include "core/MojObject.h"
#include "core/MojObjectSerialization.h"

class MojDbObjectHeader : private MojNoCopy
{
public:
	MojDbObjectHeader();
	MojDbObjectHeader(const MojObject& id);

	MojErr addTo(MojObject& obj) const;
	MojErr extractFrom(MojObject& obj);

	MojErr read(MojDbKindEngine& kindEngine);
	MojErr write(MojBuffer& buf, MojDbKindEngine& kindEngine);
	MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine);
	void reset();

	void id(const MojObject& id) { m_id = id; }
	const MojObject& id() const { return m_id; }
	const MojString& kindId() const { return m_kindId; }
	MojObjectReader& reader() { return m_reader; }

private:
	static const MojUInt8 Version = 1;

	MojErr readHeader();
	MojErr readRev();
	MojErr readSize();
	
	MojObjectReader m_reader;
	MojObject m_id;
	MojString m_kindId;
	MojInt64 m_rev;
	bool m_del;
	bool m_read;
};

#endif /* MOJDBOBJECTHEADER_H_ */
