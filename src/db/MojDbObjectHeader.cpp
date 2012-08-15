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


#include "db/MojDbObjectHeader.h"
#include "db/MojDb.h"
#include "core/MojObjectBuilder.h"

MojDbObjectHeader::MojDbObjectHeader()
: m_rev(0),
  m_del(false),
  m_read(false)
{
}

MojDbObjectHeader::MojDbObjectHeader(const MojObject& id)
: m_id(id),
  m_rev(0),
  m_del(false),
  m_read(false)
{
}

MojErr MojDbObjectHeader::addTo(MojObject& obj) const
{
	MojErr err = obj.put(MojDb::IdKey, m_id);
	MojErrCheck(err);
	err = obj.put(MojDb::RevKey, m_rev);
	MojErrCheck(err);
	err = obj.putString(MojDb::KindKey, m_kindId);
	MojErrCheck(err);
	if (m_del) {
		err = obj.putBool(MojDb::DelKey, m_del);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbObjectHeader::extractFrom(MojObject& obj)
{
	// id
	bool found;
	MojErr err = obj.del(MojDb::IdKey, found);
	MojErrCheck(err);
	MojAssert(found);
	// kind
	err = obj.getRequired(MojDb::KindKey, m_kindId);
	MojErrCheck(err);
	err = obj.del(MojDb::KindKey, found);
	MojErrCheck(err);
	MojAssert(found);
	// rev
	err = obj.getRequired(MojDb::RevKey, m_rev);
	MojErrCheck(err);
	err = obj.del(MojDb::RevKey, found);
	MojErrCheck(err);
	MojAssert(found);
	// del
	bool del = false;
	if (obj.get(MojDb::DelKey, del) && del) {
		m_del = del;
	}
	return MojErrNone;
}

MojErr MojDbObjectHeader::read(MojDbKindEngine& kindEngine)
{
	if (m_read)
		return MojErrNone;

	// version
	m_read = true;
	MojUInt8 version = 0;
	MojDataReader& dataReader = m_reader.dataReader();
	MojErr err = dataReader.readUInt8(version);
	MojErrCheck(err);
	if (version != Version)
		MojErrThrow(MojErrDbHeaderVersionMismatch);
	// kindId
	MojObjectBuilder builder;
	err = m_reader.next(builder);
	MojErrCheck(err);
	MojInt64 kindTok = builder.object().intValue();
	err = kindEngine.idFromToken(kindTok, m_kindId);
	MojErrCheck(err);
	// rev
	err = m_reader.next(builder);
	MojErrCheck(err);
	m_rev = builder.object().intValue();
	// del
	if (m_reader.peek() != MojObjectWriter::MarkerHeaderEnd) {
		err = m_reader.next(builder);
		MojErrCheck(err);
		m_del = builder.object().boolValue();
	}
	// stuff we don't know about yet
	while (m_reader.peek() != MojObjectWriter::MarkerHeaderEnd) {
		MojObjectEater eater;
		err = m_reader.next(eater);
		MojErrCheck(err);
	}
	// header end
	MojUInt8 end;
	err = dataReader.readUInt8(end);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbObjectHeader::write(MojBuffer& buf, MojDbKindEngine& kindEngine)
{
	MojDataWriter dataWriter(buf);
	MojObjectWriter objectWriter(buf, NULL);
	// version
	MojErr err = dataWriter.writeUInt8(Version);
	MojErrCheck(err);
	// kind
	MojInt64 kindTok = 0;
	err = kindEngine.tokenFromId(m_kindId, kindTok);
	MojErrCheck(err);
	err = objectWriter.intValue(kindTok);
	MojErrCheck(err);
	// rev
	err = objectWriter.intValue(m_rev);
	MojErrCheck(err);
	// del
	if (m_del) {
		err = objectWriter.boolValue(true);
		MojErrCheck(err);
	}
	// header end
	err = dataWriter.writeUInt8(MojObjectWriter::MarkerHeaderEnd);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbObjectHeader::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine)
{
	MojErr err = read(kindEngine);
	MojErrCheck(err);

	m_reader.skipBeginObj();
	err = visitor.beginObject();
	MojErrCheck(err);
	// id
	err = visitor.objectProp(MojDb::IdKey, m_id);
	MojErrCheck(err);
	// kind
	err = visitor.stringProp(MojDb::KindKey, m_kindId);
	MojErrCheck(err);
	// rev
	err = visitor.intProp(MojDb::RevKey, m_rev);
	MojErrCheck(err);
	// del
	if (m_del) {
		err = visitor.boolProp(MojDb::DelKey, m_del);
		MojErrCheck(err);
	}
	return MojErrNone;
}

void MojDbObjectHeader::reset()
{
	m_id.clear();
	m_kindId.clear();
	m_rev = 0;
	m_del = false;
	m_read = false;
	m_reader.data(NULL, 0);
}

