/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics
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


#include "core/MojObjectBuilder.h"

MojObjectBuilder::MojObjectBuilder()
{
}

MojErr MojObjectBuilder::reset()
{
	m_obj.clear();
	m_stack = ObjStack();
	m_propName.clear();
	return MojErrNone;
}

MojErr MojObjectBuilder::beginObject()
{
	MojErr err = push(MojObject::TypeObject);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::endObject()
{
	MojErr err = pop();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::beginArray()
{
	MojErr err = push(MojObject::TypeArray);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::endArray()
{
	return MojObjectBuilder::endObject();
}

MojErr MojObjectBuilder::propName(const MojChar* name, gsize len)
{
	MojErr err = m_propName.assign(name, len);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::nullValue()
{
	MojErr err = value(MojObject(MojObject::TypeNull));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::boolValue(bool val)
{
	MojErr err = value(val);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::intValue(gint64 val)
{
	MojErr err = value(val);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::decimalValue(const MojDecimal& val)
{
	MojErr err = value(val);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::stringValue(const MojChar* val, gsize len)
{
	MojString str;
	MojErr err = str.assign(val, len);
	MojErrCheck(err);
	err = value(str);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::push(MojObject::Type type)
{
	m_stack.push(Rec(type, m_propName));

	return MojErrNone;
}

MojErr MojObjectBuilder::pop()
{
	MojAssert(!m_stack.empty());

	const Rec& rec = m_stack.top();
	MojObject obj = rec.m_obj;
	m_propName = rec.m_propName;
	m_stack.pop();
	MojErr err = value(obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObjectBuilder::value(const MojObject& val)
{
	if (m_stack.empty()) {
		m_obj = val;
	} else {
		MojObject& obj = back();
		if (obj.type() == MojObject::TypeObject) {
			MojErr err = obj.put(m_propName, val);
			MojErrCheck(err);
		} else {
			MojAssert(obj.type() == MojObject::TypeArray);
			MojErr err = obj.push(val);
			MojErrCheck(err);
		}
	}

	return MojErrNone;
}
