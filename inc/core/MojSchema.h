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


#ifndef MOJSCHEMA_H_
#define MOJSCHEMA_H_

#include "core/MojCoreDefs.h"
#include "core/MojMap.h"
#include "core/MojObject.h"
#include "core/MojSet.h"
#include "core/MojVector.h"

class MojSchema : public MojRefCounted
{
public:
	static const MojChar* const AdditionalPropertiesKey;
	static const MojChar* const DisallowKey;
	static const MojChar* const DivisibleByKey;
	static const MojChar* const EnumKey;
	static const MojChar* const ItemsKey;
	static const MojChar* const MinimumKey;
	static const MojChar* const MinimumCanEqualKey;
	static const MojChar* const MinItemsKey;
	static const MojChar* const MinLengthKey;
	static const MojChar* const MaximumKey;
	static const MojChar* const MaximumCanEqualKey;
	static const MojChar* const MaxItemsKey;
	static const MojChar* const MaxLengthKey;
	static const MojChar* const OptionalKey;
	static const MojChar* const PropertiesKey;
	static const MojChar* const RequiresKey;
	static const MojChar* const TypeKey;
	static const MojChar* const UniqueItemsKey;

	typedef MojSet<MojString> StringSet;

	class Result
	{
	public:
		Result() : m_valid(false) {}
		bool valid() const { return m_valid; }
		const MojString& msg() const { return m_msg; }

	private:
		friend class MojSchema;

		void valid(bool val) { m_valid = val; }

		bool m_valid;
		MojString m_msg;
	};

	MojSchema();
	~MojSchema();

	void clear() { m_rule.reset(); }
	MojErr fromObject(const MojObject& obj);
	MojErr validate(const MojObject& obj, Result& resOut) const;

	const StringSet strings() const { return m_strings; }

private:
	static const MojChar* const InvalidSchemaPrefix;

	class Rule : public MojRefCounted
	{
	public:
		Rule(MojSchema* schema) : m_schema(schema) {}
		virtual ~Rule() {}
		virtual MojErr fromObject(const MojObject& obj) { return MojErrNone; }
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const = 0;

	protected:
		MojSchema* m_schema;
	};

	class SchemaRule : public Rule
	{
	public:
		SchemaRule(MojSchema* schema);

		bool optional() const { return m_optional; }

		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	protected:
		typedef MojVector<MojRefCountedPtr<Rule> > RuleVec;

		MojErr addRule(MojRefCountedPtr<Rule> rule, const MojObject& obj);

		RuleVec m_rules;
		bool m_optional;
	};

	class SimpleTypeRule : public Rule
	{
	public:
		SimpleTypeRule(MojSchema* schema) : Rule(schema) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		typedef MojVector<MojObject::Type> TypeVec;

		TypeVec m_types;
	};

	class UnionTypeRule : public SchemaRule
	{
	public:
		UnionTypeRule(MojSchema* schema) : SchemaRule(schema) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		bool valid(const MojObject& val) const;
	};

	class SimpleDisallowRule : public SimpleTypeRule
	{
	public:
		SimpleDisallowRule(MojSchema* schema) : SimpleTypeRule(schema) {}
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;
	};

	class UnionDisallowRule : public UnionTypeRule
	{
	public:
		UnionDisallowRule(MojSchema* schema) : UnionTypeRule(schema) {}
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;
	};

	class AdditionalPropertiesRule : public Rule
	{
	public:
		AdditionalPropertiesRule(MojSchema* schema);

		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojRefCountedPtr<SchemaRule> m_schemaRule;
		bool m_allowAdditional;
	};

	class PropertiesRule : public Rule
	{
	public:
		PropertiesRule(MojSchema* schema) : Rule(schema) {}
		void additionalProperties(AdditionalPropertiesRule* rule) { m_additionalRule.reset(rule); }
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		typedef MojMap<MojString, MojRefCountedPtr<SchemaRule> > PropMap;

		PropMap m_props;
		MojRefCountedPtr<AdditionalPropertiesRule> m_additionalRule;
	};

	class SimpleItemsRule : public Rule
	{
	public:
		SimpleItemsRule(MojSchema* schema) : Rule(schema) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojRefCountedPtr<SchemaRule> m_schemaRule;
	};

	class TupleItemsRule : public UnionTypeRule
	{
	public:
		TupleItemsRule(MojSchema* schema) : UnionTypeRule(schema) {}
		void additionalProperties(AdditionalPropertiesRule* rule) { m_additionalRule.reset(rule); }
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojRefCountedPtr<AdditionalPropertiesRule> m_additionalRule;
	};

	class RequiresRule : public Rule
	{
	public:
		RequiresRule(MojSchema* schema) : Rule(schema) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojString m_requiredProp;
	};

	template<class COMP1, class COMP2>
	class CompRule : public Rule
	{
	public:
		CompRule(MojSchema* schema);
		void canEqual(bool val) { m_canEqual = val; }
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojDecimal m_val;
		bool m_canEqual;
	};
	typedef CompRule<MojLessThanEq<MojDecimal>, MojLessThan<MojDecimal> > MaxRule;
	typedef CompRule<MojGreaterThanEq<MojDecimal>, MojGreaterThan<MojDecimal> > MinRule;

	template<class COMP>
	class ArrayLenRule : public Rule
	{
	public:
		ArrayLenRule(MojSchema* schema) : Rule(schema), m_val(0) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojInt64 m_val;
	};
	typedef ArrayLenRule<MojLessThanEq<MojInt64> > MaxItemsRule;
	typedef ArrayLenRule<MojGreaterThanEq<MojInt64> > MinItemsRule;

	class UniqueItemsRule : public Rule
	{
	public:
		UniqueItemsRule(MojSchema* schema) : Rule(schema) {}
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;
	};

	template<class COMP>
	class StringLenRule : public Rule
	{
	public:
		StringLenRule(MojSchema* schema) : Rule(schema), m_val(0) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		MojInt64 m_val;
	};
	typedef StringLenRule<MojLessThanEq<MojInt64> > MaxLenRule;
	typedef StringLenRule<MojGreaterThanEq<MojInt64> > MinLenRule;

	class EnumRule : public Rule
	{
	public:
		EnumRule(MojSchema* schema);
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		typedef MojSet<MojObject> ObjectSet;

		ObjectSet m_vals;
	};

	class DivisibleRule : public Rule
	{
	public:
		DivisibleRule(MojSchema* schema) : Rule(schema), m_val(0) {}
		virtual MojErr fromObject(const MojObject& obj);
		virtual MojErr validate(const MojObject& val, const MojObject& parent, Result& resOut) const;

	private:
		typedef MojSet<MojObject> ObjectSet;

		MojInt64 m_val;
	};

	MojRefCountedPtr<Rule> m_rule;
	StringSet m_strings;
};

#endif /* MOJSCHEMA_H_ */
