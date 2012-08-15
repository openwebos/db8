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


#ifndef MOJOBJECTBUILDER_H_
#define MOJOBJECTBUDILER_H_

#include "core/MojCoreDefs.h"
#include "core/MojObject.h"

class MojObjectBuilder : public MojObjectVisitor
{
public:
	MojObjectBuilder();

	virtual MojErr reset();
	virtual MojErr beginObject();
	virtual MojErr endObject();
	virtual MojErr beginArray();
	virtual MojErr endArray();
	virtual MojErr propName(const MojChar* name, MojSize len);
	virtual MojErr nullValue();
	virtual MojErr boolValue(bool val);
	virtual MojErr intValue(MojInt64 val);
	virtual MojErr decimalValue(const MojDecimal& val);
	virtual MojErr stringValue(const MojChar* val, MojSize len);

	const MojObject& object() const { return m_obj; }
	MojObject& object() { return m_obj; }

private:
	struct Rec
	{
		Rec(MojObject::Type type, MojString& propName) : m_obj(type), m_propName(propName) {}
		MojObject m_obj;
		MojString m_propName;
	};
	typedef MojVector<Rec> ObjStack;

	MojErr push(MojObject::Type type);
	MojErr pop();
	MojErr value(const MojObject& val);
	MojObject& back() { return const_cast<MojObject&>(m_stack.back().m_obj); }

	ObjStack m_stack;
	MojObject m_obj;
	MojString m_propName;
};

#endif /* MOJOBJECTBUILDER_H_ */
