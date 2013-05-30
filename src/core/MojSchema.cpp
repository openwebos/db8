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


#include "core/MojSchema.h"

const MojChar* const MojSchema::AdditionalPropertiesKey = _T("additionalProperties");
const MojChar* const MojSchema::DisallowKey = _T("disallow");
const MojChar* const MojSchema::DivisibleByKey = _T("divisibleBy");
const MojChar* const MojSchema::EnumKey = _T("enum");
const MojChar* const MojSchema::ItemsKey = _T("items");
const MojChar* const MojSchema::MinimumKey = _T("minimum");
const MojChar* const MojSchema::MinimumCanEqualKey = _T("minimumCanEqual");
const MojChar* const MojSchema::MinItemsKey = _T("minItems");
const MojChar* const MojSchema::MinLengthKey = _T("minLength");
const MojChar* const MojSchema::MaximumKey = _T("maximum");
const MojChar* const MojSchema::MaximumCanEqualKey = _T("maximumCanEqual");
const MojChar* const MojSchema::MaxItemsKey = _T("maxItems");
const MojChar* const MojSchema::MaxLengthKey = _T("maxLength");
const MojChar* const MojSchema::OptionalKey = _T("optional");
const MojChar* const MojSchema::PropertiesKey = _T("properties");
const MojChar* const MojSchema::RequiresKey = _T("requires");
const MojChar* const MojSchema::TypeKey = _T("type");
const MojChar* const MojSchema::UniqueItemsKey = _T("uniqueItems");

const MojChar* const MojSchema::InvalidSchemaPrefix = _T("invalid schema");

MojSchema::MojSchema()
{
}

MojSchema::~MojSchema()
{
}

MojErr MojSchema::fromObject(const MojObject& obj)
{
	MojRefCountedPtr<SchemaRule> rule(new SchemaRule(this));
	MojAllocCheck(rule.get());
	MojErr err = rule->fromObject(obj);
	MojErrCheck(err);

	m_rule = rule;

	return MojErrNone;
}

MojErr MojSchema::validate(const MojObject& obj, Result& resOut) const
{
	// result is valid until a rule fails
	resOut.valid(true);
	if (m_rule.get()) {
		MojErr err = m_rule->validate(obj, MojObject::Undefined, resOut);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojSchema::SchemaRule::SchemaRule(MojSchema* schema)
: Rule(schema),
  m_optional(false)
{
}

MojErr MojSchema::SchemaRule::fromObject(const MojObject& obj)
{
	// additionalProperties
	MojErr err = MojErrNone;
	MojObject val;
	MojRefCountedPtr<AdditionalPropertiesRule> additionalProperties;
	if (obj.get(AdditionalPropertiesKey, val)) {
		additionalProperties.reset(new AdditionalPropertiesRule(m_schema));
		MojAllocCheck(additionalProperties.get());
		err = additionalProperties->fromObject(val);
		MojErrCheck(err);
	}
	// properties
	if (obj.get(PropertiesKey, val) || additionalProperties.get()) {
		MojRefCountedPtr<PropertiesRule> rule(new PropertiesRule(m_schema));
		err = addRule(rule, val);
		MojErrCheck(err);
		rule->additionalProperties(additionalProperties.get());
	}
	// optional
	obj.get(OptionalKey, m_optional);
	// type
	if (obj.get(TypeKey, val)) {
		if (val.type() == MojObject::TypeArray) {
			err = addRule(new UnionTypeRule(m_schema), val);
			MojErrCheck(err);
		} else {
			err = addRule(new SimpleTypeRule(m_schema), val);
			MojErrCheck(err);
		}
	}
	// disallow
	if (obj.get(DisallowKey, val)) {
		if (val.type() == MojObject::TypeArray) {
			err = addRule(new UnionDisallowRule(m_schema), val);
			MojErrCheck(err);
		} else {
			err = addRule(new SimpleDisallowRule(m_schema), val);
			MojErrCheck(err);
		}
	}
	// items
	if (obj.get(ItemsKey, val)) {
		if (val.type() == MojObject::TypeArray) {
			MojRefCountedPtr<TupleItemsRule> rule(new TupleItemsRule(m_schema));
			err = addRule(rule, val);
			MojErrCheck(err);
			rule->additionalProperties(additionalProperties.get());
		} else {
			err = addRule(new SimpleItemsRule(m_schema), val);
			MojErrCheck(err);
		}
	}
	// requires
	if (obj.get(RequiresKey, val)) {
		err = addRule(new RequiresRule(m_schema), val);
		MojErrCheck(err);
	}
	// minimum
	if (obj.get(MinimumKey, val)) {
		MojRefCountedPtr<MinRule> rule(new MinRule(m_schema));
		err = addRule(rule, val);
		MojErrCheck(err);
		bool canEqual = false;
		if (obj.get(MinimumCanEqualKey, canEqual))
			rule->canEqual(canEqual);
	}
	// maximum
	if (obj.get(MaximumKey, val)) {
		MojRefCountedPtr<MaxRule> rule(new MaxRule(m_schema));
		err = addRule(rule, val);
		MojErrCheck(err);
		bool canEqual = false;
		if (obj.get(MaximumCanEqualKey, canEqual))
			rule->canEqual(canEqual);
	}
	// minItems
	if (obj.get(MinItemsKey, val)) {
		err = addRule(new MinItemsRule(m_schema), val);
		MojErrCheck(err);
	}
	// maxItems
	if (obj.get(MaxItemsKey, val)) {
		err = addRule(new MaxItemsRule(m_schema), val);
		MojErrCheck(err);
	}
	// uniqueItems
	bool boolVal;
	if (obj.get(UniqueItemsKey, boolVal) && boolVal) {
		err = addRule(new UniqueItemsRule(m_schema), boolVal);
		MojErrCheck(err);
	}
	// minLength
	if (obj.get(MinLengthKey, val)) {
		err = addRule(new MinLenRule(m_schema), val);
		MojErrCheck(err);
	}
	// maxLength
	if (obj.get(MaxLengthKey, val)) {
		err = addRule(new MaxLenRule(m_schema), val);
		MojErrCheck(err);
	}
	// enum
	if (obj.get(EnumKey, val)) {
		err = addRule(new EnumRule(m_schema), val);
		MojErrCheck(err);
	}
	// divisibleBy
	if (obj.get(DivisibleByKey, val)) {
		err = addRule(new DivisibleRule(m_schema), val);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::SchemaRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	for (RuleVec::ConstIterator i = m_rules.begin(); i != m_rules.end(); ++i) {
		MojErr err = (*i)->validate(val, parent, resOut);
		MojErrCheck(err);
		if (!resOut.valid())
			break;
	}
	return MojErrNone;
}

MojErr MojSchema::SchemaRule::addRule(MojRefCountedPtr<Rule> rule, const MojObject& obj)
{
	MojAllocCheck(rule.get());
	MojErr err = rule->fromObject(obj);
	MojErrCheck(err);
	err = m_rules.push(rule);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSchema::SimpleTypeRule::fromObject(const MojObject& obj)
{
	MojString typeStr;
	MojErr err = obj.stringValue(typeStr);
	MojErrCheck(err);
	MojObject::Type type = MojObject::TypeUndefined;
	if (typeStr == _T("null")) {
		type = MojObject::TypeNull;
	} else if (typeStr == _T("object")) {
		type = MojObject::TypeObject;
	} else if (typeStr == _T("array")) {
		type = MojObject::TypeArray;
	} else if (typeStr == _T("string")) {
		type = MojObject::TypeString;
	} else if (typeStr == _T("boolean")) {
		type = MojObject::TypeBool;
	} else if (typeStr == _T("integer")) {
		type = MojObject::TypeInt;
	} else if (typeStr == _T("number")) {
		err = m_types.push(MojObject::TypeDecimal);
		MojErrCheck(err);
		type = MojObject::TypeInt;
	} else {
		MojErrThrowMsg(MojErrInvalidSchema, _T("%s: invalid type '%s'"), InvalidSchemaPrefix, typeStr.data());
	}
	err = m_types.push(type);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSchema::SimpleTypeRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	MojObject::Type type = val.type();
	for (TypeVec::ConstIterator i = m_types.begin(); i != m_types.end(); ++i) {
		if (*i == type)
			return MojErrNone;
	}

	resOut.valid(false);
	MojErr err = resOut.m_msg.assign(_T("invalid type"));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSchema::UnionTypeRule::fromObject(const MojObject& obj)
{
	MojObject::ConstArrayIterator end = obj.arrayEnd();
	for (MojObject::ConstArrayIterator i = obj.arrayBegin(); i != end; ++i) {
		if (i->type() == MojObject::TypeString) {
			MojErr err = addRule(new SimpleTypeRule(m_schema), *i);
			MojErrCheck(err);
		} else {
			MojErr err = addRule(new SchemaRule(m_schema), *i);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojSchema::UnionTypeRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	for (RuleVec::ConstIterator i = m_rules.begin(); i != m_rules.end(); ++i) {
		resOut.valid(true);
		MojErr err = (*i)->validate(val, parent, resOut);
		MojErrCheck(err);
		if (resOut.valid())
			return MojErrNone;
	}
	resOut.valid(false);

	return MojErrNone;
}

MojErr MojSchema::SimpleDisallowRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	Result typeRes;
	typeRes.valid(true);
	MojErr err = SimpleTypeRule::validate(val, parent, typeRes);
	MojErrCheck(err);
	if (typeRes.valid()) {
		resOut.valid(false);
		err = resOut.m_msg.assign(_T("type disallowed"));
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::UnionDisallowRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	Result typeRes;
	typeRes.valid(true);
	MojErr err = UnionTypeRule::validate(val, parent, typeRes);
	MojErrCheck(err);
	if (typeRes.valid()) {
		resOut.valid(false);
		err = resOut.m_msg.assign(_T("type disallowed"));
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojSchema::AdditionalPropertiesRule::AdditionalPropertiesRule(MojSchema* schema)
: Rule(schema),
  m_allowAdditional(true)
{
}

MojErr MojSchema::AdditionalPropertiesRule::fromObject(const MojObject& obj)
{
	if (obj.type() == MojObject::TypeBool) {
		m_allowAdditional = obj.boolValue();
	} else {
		m_schemaRule.reset(new SchemaRule(m_schema));
		MojAllocCheck(m_schemaRule.get());
		MojErr err = m_schemaRule->fromObject(obj);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::AdditionalPropertiesRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (!m_allowAdditional) {
		// additionalProps not allowed - fail
		resOut.valid(false);
		MojErr err = resOut.m_msg.assign(_T("property not allowed"));
		MojErrCheck(err);
	} else if (m_schemaRule.get()) {
		// validate against additional props schema
		MojErr err = m_schemaRule->validate(val, parent, resOut);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::PropertiesRule::fromObject(const MojObject& obj)
{
	MojObject::ConstIterator end = obj.end();
	for (MojObject::ConstIterator i = obj.begin(); i != end; ++i) {
		MojRefCountedPtr<SchemaRule> rule(new SchemaRule(m_schema));
		MojAllocCheck(rule.get());
		MojErr err = rule->fromObject(*i);
		MojErrCheck(err);
		err = m_props.put(i.key(), rule);
		MojErrCheck(err);
		err = m_schema->m_strings.put(i.key());
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::PropertiesRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	MojErr err = MojErrNone;
	PropMap::ConstIterator iter = m_props.begin();
	PropMap::ConstIterator end = m_props.end();
	MojObject::ConstIterator valIter = val.begin();
	MojObject::ConstIterator valEnd = val.end();

	while (iter != end || valIter != valEnd) {
		// compare prop names
		int comp;
		if (iter == end) {
			comp = 1;
		} else if (valIter == valEnd) {
			comp = -1;
		} else {
			comp = iter.key().compare(valIter.key());
		}

		if (comp > 0) {
			// prop in val and not schema
			if (m_additionalRule.get()) {
				err = m_additionalRule->validate(*valIter, val, resOut);
				MojErrCheck(err);
				if (!resOut.valid()) {
					err = resOut.m_msg.appendFormat(_T(" - '%s'"), valIter.key().data());
					MojErrCheck(err);
					break;
				}
			}
			++valIter;
		} else if (comp < 0) {
			// prop in schema and not val
			if (!(*iter)->optional()) {
				resOut.valid(false);
				err = resOut.m_msg.format(_T("required property not found - '%s'"), iter.key().data());
				MojErrCheck(err);
				break;
			}
			++iter;
		} else {
			// prop in both val and schema
			err = (*iter)->validate(*valIter, val, resOut);
			MojErrCheck(err);
			if (!resOut.valid()) {
				err = resOut.m_msg.appendFormat(_T(" for property '%s'"), iter.key().data());
				MojErrCheck(err);
				break;
			}
			++valIter;
			++iter;
		}
	}
	return MojErrNone;
}

MojErr MojSchema::SimpleItemsRule::fromObject(const MojObject& obj)
{
	m_schemaRule.reset(new SchemaRule(m_schema));
	MojAllocCheck(m_schemaRule.get());
	MojErr err = m_schemaRule->fromObject(obj);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSchema::SimpleItemsRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	MojObject::ConstArrayIterator end = val.arrayEnd();
	for (MojObject::ConstArrayIterator i = val.arrayBegin(); i != end; ++i) {
		MojErr err = m_schemaRule->validate(*i, val, resOut);
		MojErrCheck(err);
		if (!resOut.valid())
			break;
	}
	return MojErrNone;
}

MojErr MojSchema::TupleItemsRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (val.type() != MojObject::TypeArray)
		return MojErrNone;

	MojErr err = MojErrNone;
	RuleVec::ConstIterator iter = m_rules.begin();
	RuleVec::ConstIterator end = m_rules.end();
	MojObject::ConstArrayIterator valIter = val.arrayBegin();
	MojObject::ConstArrayIterator valEnd = val.arrayEnd();
	// validate properties in tuple
	while (iter != end && valIter != valEnd) {
		err = (*iter)->validate(*valIter, val, resOut);
		MojErrCheck(err);
		if (!resOut.valid())
			return MojErrNone;
		++iter;
		++valIter;
	}
	if (iter != end) {
		resOut.valid(false);
		err = resOut.m_msg.format(_T("too few items in array"));
		MojErrCheck(err);
	}
	// validate additional properties
	if (m_additionalRule.get()) {
		while (valIter != valEnd) {
			err = m_additionalRule->validate(*valIter, val, resOut);
			MojErrCheck(err);
			if (!resOut.valid())
				break;
			++valIter;
		}
	}
	return MojErrNone;
}

MojErr MojSchema::RequiresRule::fromObject(const MojObject& obj)
{
	MojErr err = obj.stringValue(m_requiredProp);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojSchema::RequiresRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (!parent.contains(m_requiredProp)) {
		resOut.valid(false);
		MojErr err = resOut.m_msg.format(_T("required prop not found - '%s'"), m_requiredProp.data());
		MojErrCheck(err);
	}
	return MojErrNone;
}

template<class COMP1, class COMP2>
MojSchema::CompRule<COMP1, COMP2>::CompRule(MojSchema* schema)
: Rule(schema),
  m_canEqual(true)
{
}

template<class COMP1, class COMP2>
MojErr MojSchema::CompRule<COMP1, COMP2>::fromObject(const MojObject& obj)
{
	m_val = obj.decimalValue();

	return MojErrNone;
}

template<class COMP1, class COMP2>
MojErr MojSchema::CompRule<COMP1, COMP2>::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	MojObject::Type type = val.type();
	if (type == MojObject::TypeInt || type == MojObject::TypeDecimal) {
		MojDecimal decVal = val.decimalValue();
		bool valid = false;
		if (m_canEqual) {
			valid = COMP1()(decVal, m_val);
		} else {
			valid = COMP2()(decVal, m_val);
		}
		if (!valid) {
			resOut.valid(valid);
			MojErr err = resOut.m_msg.assign(_T("value out of range"));
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

template<class COMP>
MojErr MojSchema::ArrayLenRule<COMP>::fromObject(const MojObject& obj)
{
	m_val = obj.intValue();

	return MojErrNone;
}

template<class COMP>
MojErr MojSchema::ArrayLenRule<COMP>::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (val.type() == MojObject::TypeArray && !COMP()(val.size(), m_val)) {
		resOut.valid(false);
		MojErr err = resOut.m_msg.assign(_T("array length out of range"));
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::UniqueItemsRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (val.type() != MojObject::TypeArray)
		return MojErrNone;

	MojObject::ConstArrayIterator end = val.arrayEnd();
	for (MojObject::ConstArrayIterator i = val.arrayBegin(); i != end; ++i) {
		for (MojObject::ConstArrayIterator j = i + 1; j != end; ++j) {
			if (*i == *j) {
				resOut.valid(false);
				MojErr err = resOut.m_msg.assign(_T("duplicate items"));
				MojErrCheck(err);
				break;
			}
		}
	}
	return MojErrNone;
}

template<class COMP>
MojErr MojSchema::StringLenRule<COMP>::fromObject(const MojObject& obj)
{
	m_val = obj.intValue();

	return MojErrNone;
}

template<class COMP>
MojErr MojSchema::StringLenRule<COMP>::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (val.type() == MojObject::TypeString) {
		MojString str;
		MojErr err = val.stringValue(str);
		MojErrCheck(err);
		gint64 len = (gint64) str.length();
		if (!COMP()(len, m_val)) {
			resOut.valid(false);
			MojErr err = resOut.m_msg.assign(_T("string length out of range"));
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojSchema::EnumRule::EnumRule(MojSchema* schema)
: Rule(schema)
{
	MojAssert(schema);
}

MojErr MojSchema::EnumRule::fromObject(const MojObject& obj)
{
	MojObject::ConstArrayIterator end = obj.arrayEnd();
	for (MojObject::ConstArrayIterator i = obj.arrayBegin(); i != end; ++i) {
		MojErr err = m_vals.put(*i);
		MojErrCheck(err);
		if (i->type() == MojObject::TypeString) {
			MojString str;
			err = i->stringValue(str);
			MojErrCheck(err);
			err = m_schema->m_strings.put(str);
			MojErrCheck(err);
		}
	}
	return MojErrNone;
}

MojErr MojSchema::EnumRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	if (!m_vals.contains(val)) {
		resOut.valid(false);
		MojErr err = resOut.m_msg.assign(_T("invalid enum value"));
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojSchema::DivisibleRule::fromObject(const MojObject& obj)
{
	m_val = obj.intValue();

	return MojErrNone;
}

MojErr MojSchema::DivisibleRule::validate(const MojObject& val, const MojObject& parent, Result& resOut) const
{
	MojObject::Type type = val.type();
	if (type != MojObject::TypeInt && type != MojObject::TypeDecimal)
		return MojErrNone;

	if (m_val == 0 ||
		(val.intValue() % m_val) != 0 ||
		(type == MojObject::TypeDecimal && val.decimalValue().fraction() != 0)) {
		resOut.valid(false);
		MojErr err = resOut.m_msg.format(_T("value not divisible by %lld"), m_val);
		MojErrCheck(err);
	}
	return MojErrNone;
}
