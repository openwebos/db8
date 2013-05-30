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


#include "core/MojObject.h"
#include "core/MojObjectBuilder.h"
#include "core/MojObjectSerialization.h"
#include "core/MojHashMap.h"
#include "core/MojJson.h"


#include <string.h>

const MojObject MojObject::Undefined(TypeUndefined);
const MojObject MojObject::Null(TypeNull);

void MojObject::clear(Type type)
{
	init(type);
}

MojErr MojObject::coerce(Type toType)
{
	if (toType != type()) {
		switch (toType) {
		case TypeString: {
			MojString str;
			MojErr err = stringValue(str);
			MojErrCheck(err);
			assign(str);
			break;
		}
		case TypeBool: {
			assign(boolValue());
			break;
		}
		case TypeDecimal: {
			assign(decimalValue());
			break;
		}
		case TypeInt: {
			assign(intValue());
			break;
		}
		default: {
			clear(toType);
			break;
		}
		}
	}
	return MojErrNone;
}

MojErr MojObject::fromJson(const MojChar* chars, gsize len)
{
	MojObjectBuilder builder;
	MojErr err = MojJsonParser::parse(builder, chars, len);
	MojErrCheck(err);
	assign(builder.object());
	return MojErrNone;
}

MojErr MojObject::toJson(MojString& strOut) const
{
	strOut.clear();
	MojJsonWriter writer;
	MojErr err = visit(writer);
	MojErrCheck(err);
	strOut = writer.json();
	return MojErrNone;
}

MojErr MojObject::fromBytes(const guint8* data, gsize size)
{
	MojObjectBuilder builder;
	MojErr err = MojObjectReader::read(builder, data, size);
	MojErrCheck(err);
	assign(builder.object());
	return MojErrNone;
}

MojErr MojObject::toBytes(MojBuffer& bufOut) const
{
	bufOut.clear();
	MojObjectWriter writer;
	MojErr err = visit(writer);
	MojErrCheck(err);
	bufOut = writer.buf();
	return MojErrNone;
}

MojErr MojObject::toBytes(ByteVec& vecOut) const
{
	vecOut.clear();
	MojObjectWriter writer;
	MojErr err = visit(writer);
	MojErrCheck(err);
	err = writer.buf().toByteVec(vecOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojObject::toBase64(MojString& strOut) const
{
	strOut.clear();
	ByteVec vec;
	MojErr err = toBytes(vec);
	MojErrCheck(err);
	err = strOut.base64Encode(vec, false);
	MojErrCheck(err);

	return MojErrNone;
}

void MojObject::assign(const MojObject& val)
{
  init(val);
}

int MojObject::compare(const MojObject& val) const
{
	Type thisType = type();
	Type valType = val.type();
	if (thisType != valType)
		return thisType - valType;
	return impl()->compare(*val.impl());
}

MojErr MojObject::begin(Iterator& iter)
{
	PropMap* map = const_cast<PropMap*> (impl()->map());
	if (map) {
		MojErr err = map->begin(iter);
		MojErrCheck(err);
	} else {
		iter = Iterator();
	}
	return MojErrNone;
}

MojObject::ConstIterator MojObject::begin() const
{
	const PropMap* map = impl()->map();
	if (map)
		return map->begin();
	return ConstIterator();
}

MojErr MojObject::put(const MojChar* key, const MojObject& val)
{
	MojAssert(key);

	MojString keyStr;
	MojErr err = keyStr.assign(key);
	MojErrCheck(err);
	err = put(keyStr, val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::put(const MojString& key, const MojObject& val)
{
	MojAssert(key);

	ObjectImpl& obj = ensureObject();
	MojErr err = obj.m_props.put(key, val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::putString(const MojChar* key, const MojChar* val)
{
	MojAssert(key && val);

	MojString strVal;
	MojErr err = strVal.assign(val);
	MojErrCheck(err);
	err = put(key, MojObject(strVal));
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::find(const MojChar* key, Iterator& iter)
{
    // TODO:  remove const_cast, use mutable
	PropMap* map = const_cast<PropMap*> (impl()->map());
	if (map) {
		MojErr err = map->find(key, iter);
		MojErrCheck(err);
	} else {
		iter = Iterator();
	}
	return MojErrNone;
}

MojObject::ConstIterator MojObject::find(const MojChar* key) const
{
	const PropMap* map = impl()->map();
	if (map)
		return map->find(key);
	return ConstIterator();
}

bool MojObject::get(const MojChar* key, bool& valOut) const
{
	MojObject obj;
	if (get(key, obj)) {
		valOut = obj.boolValue();
		return true;
	}
	return false;
}

bool MojObject::get(const MojChar* key, gint64& valOut) const
{
	MojObject obj;
	if (get(key, obj)) {
		valOut = obj.intValue();
		return true;
	}
	return false;
}

MojErr MojObject::get(const MojChar* key, gint32& valOut, bool& foundOut) const
{
	foundOut = false;
	gint64 val;
	if (get(key, val)) {
		if (val >= G_MININT32 && val <= G_MAXINT32) {
			valOut = (gint32) val;
			foundOut= true;
		} else {
			MojErrThrowMsg(MojErrValueOutOfRange, _T("value out of range for prop: '%s'"), key);
		}
	}
	return MojErrNone;
}

MojErr MojObject::get(const MojChar* key, guint32& valOut, bool& foundOut) const
{
	foundOut = false;
	gint64 val;
	if (get(key, val)) {
		if (val >= 0 && val <= G_MAXUINT32) {
			valOut = (guint32) val;
			foundOut= true;
		} else {
			MojErrThrowMsg(MojErrValueOutOfRange, _T("value out of range for prop: '%s'"), key);
		}
	}
	return MojErrNone;
}

MojErr MojObject::get(const MojChar* key, guint64& valOut, bool& foundOut) const
{
	foundOut = false;
	gint64 val;
	if (get(key, val)) {
		if (val >= 0) {
			valOut = (guint64) val;
			foundOut= true;
		} else {
			MojErrThrowMsg(MojErrValueOutOfRange, _T("value out of range for prop: '%s'"), key);
		}
	}
	return MojErrNone;
}

bool MojObject::get(const MojChar* key, MojDecimal& valOut) const
{
	MojObject obj;
	if (get(key, obj)) {
		valOut = obj.decimalValue();
		return true;
	}
	return false;
}

MojErr MojObject::get(const MojChar* key, MojString& valOut, bool& foundOut) const
{
	foundOut = false;
	MojObject obj;
	if (get(key, obj)) {
		MojErr err = obj.stringValue(valOut);
		MojErrCheck(err);
		foundOut = true;
	}
	return MojErrNone;
}

template<class T>
MojErr MojObject::getRequiredT(const MojChar* key, T& valOut) const
{
	MojAssert(key);
	if (!get(key, valOut))
		MojErrThrowMsg(MojErrRequiredPropNotFound, _T("required prop not found: '%s'"), key);
	return MojErrNone;
}

template<class T>
MojErr MojObject::getRequiredErrT(const MojChar* key, T& valOut) const
{
	MojAssert(key);
	bool found = false;
	MojErr err = get(key, valOut, found);
	MojErrCheck(err);
	if (!found)
		MojErrThrowMsg(MojErrRequiredPropNotFound, _T("required prop not found: '%s'"), key);
	return MojErrNone;
}

MojErr MojObject::getRequired(const MojChar* key, MojObject& valOut) const
{
	return getRequiredT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, bool& valOut) const
{
	return getRequiredT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, gint32& valOut) const
{
	return getRequiredErrT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, guint32& valOut) const
{
	return getRequiredErrT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, gint64& valOut) const
{
	return getRequiredT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, guint64& valOut) const
{
	return getRequiredErrT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, MojDecimal& valOut) const
{
	return getRequiredT(key, valOut);
}

MojErr MojObject::getRequired(const MojChar* key, MojString& valOut) const
{
	return getRequiredErrT(key, valOut);
}

MojErr MojObject::push(const MojObject& val)
{
	ArrayImpl& array = ensureArray();

	MojErr err = array.m_vec.push(val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::pushString(const MojChar* val)
{
	MojString str;
	MojErr err = str.assign(val);
	MojErrCheck(err);
	err = push(str);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::setAt(gsize idx, const MojObject& val)
{
	ArrayImpl& array = ensureArray();

	if (array.m_vec.size() <= idx) {
		MojErr err = array.m_vec.resize(idx + 1);
		MojErrCheck(err);
	}
	MojErr err = array.m_vec.setAt(idx, val);
	MojErrCheck(err);
	return MojErrNone;
}

bool MojObject::at(gsize idx, bool& valOut) const
{
	MojObject obj;
	bool found = at(idx, obj);
	if (found) {
		valOut = obj.boolValue();
	} else {
		valOut = false;
	}
	return found;
}

bool MojObject::at(gsize idx, gint64& valOut) const
{
	MojObject obj;
	bool found = at(idx, obj);
	if (found) {
		valOut = obj.intValue();
	} else {
		valOut = 0;
	}
	return found;
}

bool MojObject::at(gsize idx, MojDecimal& valOut) const
{
	MojObject obj;
	bool found = at(idx, obj);
	if (found) {
		valOut = obj.decimalValue();
	} else {
		valOut = MojDecimal();
	}
	return found;
}

MojErr MojObject::at(gsize idx, MojString& valOut, bool& foundOut) const
{
	MojObject obj;
	foundOut = at(idx, obj);
	if (foundOut) {
		MojErr err = obj.stringValue(valOut);
		MojErrCheck(err);
	} else {
		valOut.clear();
	}
	return MojErrNone;
}

bool MojObject::operator==(const MojObject& rhs) const
{
	if (type() != rhs.type())
		return false;
	return impl()->equals(*rhs.impl());
}

void MojObject::init(const MojObject& obj)
{
    release();

    if (obj.impl())
        m_implementation = obj.impl()->clone();
    else
        m_implementation = new UndefinedImpl();
}

void MojObject::init(Type type)
{
    release();

    switch(type) {
	case TypeObject:
		m_implementation = new ObjectImpl();
		break;
	case TypeArray:
		m_implementation = new ArrayImpl();
		break;
	case TypeString:
		m_implementation = new StringImpl();
		break;
	case TypeBool:
		m_implementation = new BoolImpl();
		break;
	case TypeDecimal:
		m_implementation = new DecimalImpl();
		break;
	case TypeInt:
		m_implementation = new IntImpl();
		break;
	case TypeNull:
		m_implementation = new NullImpl();
		break;
	case TypeUndefined:
		m_implementation = new UndefinedImpl();
		break;
	default:
        m_implementation = new UndefinedImpl();
        MojAssertNotReached(); 	// fall through to undefined
	}
}

void MojObject::release()
{
    if (m_implementation)  {
        delete m_implementation;  m_implementation = 0;
    }

}

MojObject::ObjectImpl& MojObject::ensureObject()
{
    ObjectImpl* myImpl;
    if (type() != TypeObject) {
        release();
        myImpl = new ObjectImpl();
        m_implementation = myImpl;
    } else {
        myImpl = dynamic_cast<ObjectImpl*>(m_implementation);
    }
    return *myImpl;
}

MojObject::ArrayImpl& MojObject::ensureArray()
{
    ArrayImpl* myImpl;
    if (type() != TypeArray) {
        release();
        myImpl = new ArrayImpl();
        m_implementation = myImpl;
    } else {
        myImpl = dynamic_cast<ArrayImpl*>(m_implementation);
    }
    return *myImpl;
}

MojErr MojObject::Impl::stringValue(MojString& valOut) const
{
	valOut.clear();
	MojJsonWriter writer;
	MojErr err = visit(writer);
	MojErrCheck(err);
	valOut = writer.json();
	return MojErrNone;
}

bool MojObject::Impl::get(const MojChar* key, MojObject& valOut) const
{
	valOut.clear();
	return false;
}

MojErr MojObject::Impl::del(const MojChar* key, bool& foundOut)
{
	foundOut = false;
	return MojErrNone;
}

int MojObject::NullImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	return 0;
}

bool MojObject::NullImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	return true;
}

gsize MojObject::NullImpl::hashCode() const
{
	return TypeNull;
}

MojErr MojObject::NullImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.nullValue();
	MojErrCheck(err);
	return MojErrNone;
}

gsize MojObject::UndefinedImpl::hashCode() const
{
	return TypeUndefined;
}

int MojObject::ObjectImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const ObjectImpl& rhsImpl = static_cast<const ObjectImpl&>(rhs);
	return m_props.compare(rhsImpl.m_props);
}

bool MojObject::ObjectImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const ObjectImpl& rhsImpl = static_cast<const ObjectImpl&>(rhs);
	return m_props == rhsImpl.m_props;
}

gsize MojObject::ObjectImpl::hashCode() const
{
	gsize hash = TypeObject;
	for (ConstIterator i = m_props.begin(); i != m_props.end(); ++i) {
		hash ^= MojHash(i.key());
		hash ^= i.value().hashCode();
	}
	return hash;
}

gsize MojObject::ObjectImpl::containerSize() const
{
	return m_props.size();
}

bool MojObject::ObjectImpl::boolValue() const
{
	return !m_props.empty();
}

MojErr MojObject::ObjectImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.beginObject();
	MojErrCheck(err);
	for (PropMap::ConstIterator i = m_props.begin(); i != m_props.end(); ++i) {
		const MojString& propName = i.key();
		err = visitor.propName(propName, propName.length());
		MojErrCheck(err);
		err = i.value().impl()->visit(visitor);
		MojErrCheck(err);
	}
	err = visitor.endObject();
	MojErrCheck(err);
	return MojErrNone;
}

const MojObject::PropMap* MojObject::ObjectImpl::map() const
{
	return &m_props;
}

bool MojObject::ObjectImpl::get(const MojChar* key, MojObject& valOut) const
{
	MojAssert(key);
	bool found = m_props.get(key, valOut);
	if (!found)
		valOut.clear();
	return found;
}

MojErr MojObject::ObjectImpl::del(const MojChar* key, bool& foundOut)
{
	MojAssert(key);
	return m_props.del(key, foundOut);
}

int MojObject::ArrayImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const ArrayImpl& rhsImpl = static_cast<const ArrayImpl&>(rhs);
	return m_vec.compare(rhsImpl.m_vec);
}

bool MojObject::ArrayImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const ArrayImpl& rhsImpl = static_cast<const ArrayImpl&>(rhs);
	return m_vec == rhsImpl.m_vec;
}

gsize MojObject::ArrayImpl::hashCode() const
{
	gsize hash = TypeArray;
	for (ConstArrayIterator i = m_vec.begin(); i != m_vec.end(); ++i) {
		hash ^= i->hashCode();
	}
	return hash;
}

gsize MojObject::ArrayImpl::containerSize() const
{
	return m_vec.size();
}

bool MojObject::ArrayImpl::boolValue() const
{
	return !m_vec.empty();
}

MojErr MojObject::ArrayImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.beginArray();
	MojErrCheck(err);
	for (ObjectVec::ConstIterator i = m_vec.begin(); i != m_vec.end(); ++i) {
		err = i->impl()->visit(visitor);
		MojErrCheck(err);
	}
	err = visitor.endArray();
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObject::ArrayImpl::arrayBegin(ArrayIterator& iter)
{
	MojErr err = m_vec.begin(iter);
	MojErrCheck(err);
	return MojErrNone;
}

MojObject::ConstArrayIterator MojObject::ArrayImpl::arrayBegin() const
{
	return m_vec.begin();
}

MojObject::ConstArrayIterator MojObject::ArrayImpl::arrayEnd() const
{
	return m_vec.end();
}

bool MojObject::ArrayImpl::at(gsize idx, MojObject& objOut) const
{
	if (idx >= m_vec.size())
		return false;
	objOut = m_vec.at(idx);
	return true;
}

int MojObject::StringImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const StringImpl& rhsImpl = static_cast<const StringImpl&>(rhs);
	return m_val.compare(rhsImpl.m_val);
}

bool MojObject::StringImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const StringImpl& rhsImpl = static_cast<const StringImpl&>(rhs);
	return m_val == rhsImpl.m_val;
}

gsize MojObject::StringImpl::hashCode() const
{
	return MojHasher<MojString>()(m_val);
}

bool MojObject::StringImpl::boolValue() const
{
	return !m_val.empty();
}

gint64 MojObject::StringImpl::intValue() const
{
	return MojStrToInt64(m_val, NULL, 0);
}

MojDecimal MojObject::StringImpl::decimalValue() const
{
	// TODO: skip double conversion
	gdouble d = MojStrToDouble(m_val, NULL);
	return MojDecimal(d);
}

MojErr MojObject::StringImpl::stringValue(MojString& valOut) const
{
	valOut = m_val;
	return MojErrNone;
}

MojErr MojObject::StringImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.stringValue(m_val, m_val.length());
	MojErrCheck(err);
	return MojErrNone;
}

int MojObject::BoolImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const BoolImpl& rhsImpl = static_cast<const BoolImpl&>(rhs);
	return m_val - rhsImpl.m_val;
}

bool MojObject::BoolImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const BoolImpl& rhsImpl = static_cast<const BoolImpl&>(rhs);
	return m_val == rhsImpl.m_val;
}

gsize MojObject::BoolImpl::hashCode() const
{
	return TypeBool + m_val;
}

MojErr MojObject::BoolImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.boolValue(m_val);
	MojErrCheck(err);
	return MojErrNone;
}

int MojObject::DecimalImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const DecimalImpl& rhsImpl = static_cast<const DecimalImpl&>(rhs);
	return MojComp<MojDecimal>()(m_val, rhsImpl.m_val);
}

bool MojObject::DecimalImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const DecimalImpl& rhsImpl = static_cast<const DecimalImpl&>(rhs);
	return m_val == rhsImpl.m_val;
}

gsize MojObject::DecimalImpl::hashCode() const
{
	return MojHasher<MojDecimal>()(m_val);
}

MojErr MojObject::DecimalImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.decimalValue(m_val);
	MojErrCheck(err);
	return MojErrNone;
}

int MojObject::IntImpl::compare(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const IntImpl& rhsImpl = static_cast<const IntImpl&>(rhs);
	return MojComp<gint64>()(m_val, rhsImpl.m_val);
}

bool MojObject::IntImpl::equals(const MojObject::Impl& rhs) const
{
	MojAssert(rhs.type() == type());
	const IntImpl& rhsImpl = static_cast<const IntImpl&>(rhs);
	return m_val == rhsImpl.m_val;
}

gsize MojObject::IntImpl::hashCode() const
{
	return MojHasher<gint64>()(m_val);
}

MojErr MojObject::IntImpl::visit(MojObjectVisitor& visitor) const
{
	MojErr err = visitor.intValue(m_val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObjectVisitor::boolProp(const MojChar* name, bool val)
{
	MojAssert(name);
	MojErr err = propName(name);
	MojErrCheck(err);
	err = boolValue(val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObjectVisitor::intProp(const MojChar* name, gint64 val)
{
	MojAssert(name);
	MojErr err = propName(name);
	MojErrCheck(err);
	err = intValue(val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObjectVisitor::decimalProp(const MojChar* name, const MojDecimal& val)
{
	MojAssert(name);
	MojErr err = propName(name);
	MojErrCheck(err);
	err = decimalValue(val);
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObjectVisitor::stringProp(const MojChar* name, const MojChar* val)
{
	MojAssert(name && val);
	MojErr err = propName(name);
	MojErrCheck(err);
	err = stringValue(val, MojStrLen(val));
	MojErrCheck(err);
	return MojErrNone;
}

MojErr MojObjectVisitor::objectProp(const MojChar* name, const MojObject& val)
{
	MojAssert(name);
	MojErr err = propName(name);
	MojErrCheck(err);
	err = val.visit(*this);
	MojErrCheck(err);
	return MojErrNone;
}
