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


#ifndef MOJOBJECT_H_
#define MOJOBJECT_H_

#include "core/MojCoreDefs.h"
#include "core/MojDecimal.h"
#include "core/MojMap.h"
#include "core/MojString.h"
#include "core/MojVector.h"

class MojObject
{
public:
	typedef enum {
		TypeUndefined,
		TypeNull,
		TypeObject,
		TypeArray,
		TypeString,
		TypeBool,
		TypeDecimal,
		TypeInt
	} Type;

	static const MojObject Undefined;
	static const MojObject Null;

	typedef MojVector<MojByte> ByteVec;
	typedef MojVector<MojObject> ObjectVec;
	typedef ObjectVec::ConstIterator ConstArrayIterator;
	typedef ObjectVec::Iterator ArrayIterator;
	typedef MojMap<MojString, MojObject, const MojChar*> PropMap;
	typedef PropMap::ConstIterator ConstIterator;
	typedef PropMap::Iterator Iterator;

	MojObject() { m_implementation = new UndefinedImpl(); }
	MojObject(const MojObject& obj) { m_implementation = 0; init(obj); }
	MojObject(bool val) { m_implementation = new BoolImpl(val); }
	MojObject(MojInt64 val) { m_implementation = new IntImpl(val); }
	MojObject(MojInt32 val) { m_implementation = new IntImpl(val); }
	MojObject(const MojDecimal& val) { m_implementation = new DecimalImpl(val); }
	MojObject(const MojString& val) { m_implementation = new StringImpl(val); }
	explicit MojObject(Type type) { m_implementation = 0; init(type); }
	~MojObject() { release(); }

	inline Type type() const { return impl()->type(); }

	/**
	 * If object is container, return count of elements in container
	 * @return how many elements in container
	 */
	MojSize size() const { return impl()->containerSize(); }

	/**
	 * If object is container, return if container is null
	 */
	bool empty() const { return size() == 0; }
	bool null() const { return type() == TypeNull; }
	bool undefined() const { return type() == TypeUndefined; }
	MojSize hashCode() const { return impl()->hashCode(); }
	MojErr visit(MojObjectVisitor& visitor) const { return impl()->visit(visitor); }

	bool boolValue() const { return impl()->boolValue(); }
	MojInt64 intValue() const { return impl()->intValue(); }
	MojDecimal decimalValue() const { return impl()->decimalValue(); }
	MojErr stringValue(MojString& valOut) const { return impl()->stringValue(valOut); }

	void clear(Type type = TypeUndefined);
	MojErr coerce(Type toType);
	MojErr fromJson(const MojString& str) { return fromJson(str, str.length()); }
	MojErr fromJson(const MojChar* str) { return fromJson(str, MojSizeMax); }
	MojErr fromJson(const MojChar* chars, MojSize len);
	MojErr toJson(MojString& strOut) const;
	MojErr fromBytes(const MojByte* data, MojSize size);
	MojErr toBytes(ByteVec& vecOut) const;
	MojErr toBytes(MojBuffer& bufOut) const;
	MojErr toBase64(MojString& strOut) const;

	void assign(const MojObject& val);
	int compare(const MojObject& val) const;

	// object-property methods
	MojErr begin(Iterator& iter);
	ConstIterator begin() const;
	ConstIterator end() const { return ConstIterator(); }
	MojErr put(const MojString& key, const MojObject& val);
	MojErr put(const MojChar* key, const MojObject& val);
	MojErr putString(const MojChar* key, const MojChar* val);
	MojErr putString(const MojChar* key, const MojString& val) { return put(key, MojObject(val)); }
	MojErr putBool(const MojChar* key, bool val) { return put(key, MojObject(val)); }
	MojErr putInt(const MojChar* key, MojInt64 val) { return put(key, MojObject(val)); }
	MojErr putDecimal(const MojChar* key, const MojDecimal& val) { return put(key, MojObject(val)); }
	MojErr del(const MojChar* key, bool& foundOut) { return impl()->del(key, foundOut); }
	MojErr find(const MojChar* key, Iterator& iter);
	ConstIterator find(const MojChar* key) const;
	bool contains(const MojChar* key) const { return find(key) != end(); }
	bool get(const MojChar* key, MojObject& valOut) const { return impl()->get(key, valOut); }
	bool get(const MojChar* key, bool& valOut) const;
	bool get(const MojChar* key, MojInt64& valOut) const;
	bool get(const MojChar* key, MojDecimal& valOut) const;
	MojErr get(const MojChar* key, MojInt32& valOut, bool& foundOut) const;
	MojErr get(const MojChar* key, MojUInt32& valOut, bool& foundOut) const;
	MojErr get(const MojChar* key, MojUInt64& valOut, bool& foundOut) const;
	MojErr get(const MojChar* key, MojString& valOut, bool& foundOut) const;
	MojErr getRequired(const MojChar* key, MojObject& valOut) const;
	MojErr getRequired(const MojChar* key, bool& valOut) const;
	MojErr getRequired(const MojChar* key, MojInt32& valOut) const;
	MojErr getRequired(const MojChar* key, MojUInt32& valOut) const;
	MojErr getRequired(const MojChar* key, MojInt64& valOut) const;
	MojErr getRequired(const MojChar* key, MojUInt64& valOut) const;
	MojErr getRequired(const MojChar* key, MojDecimal& valOut) const;
	MojErr getRequired(const MojChar* key, MojString& valOut) const;

	// array methods
	MojErr arrayBegin(ArrayIterator& iter) { return impl()->arrayBegin(iter); }
	ConstArrayIterator arrayBegin() const { return impl()->arrayBegin(); }
	ConstArrayIterator arrayEnd() const { return impl()->arrayEnd(); }
	MojErr push(const MojObject& val);
	MojErr pushString(const MojChar* val);
	MojErr setAt(MojSize idx, const MojObject& val);
	bool at(MojSize idx, MojObject& objOut) const { return impl()->at(idx, objOut); }
	bool at(MojSize idx, bool& valOut) const;
	bool at(MojSize idx, MojInt64& valOut) const;
	bool at(MojSize idx, MojDecimal& valOut) const;
	MojErr at(MojSize idx, MojString& valOut, bool& foundOut) const;

	MojObject& operator=(const MojObject& rhs) { assign(rhs); return *this; }
	bool operator==(const MojObject& rhs) const;
	bool operator!=(const MojObject& rhs) const { return !operator==(rhs); }
	bool operator<(const MojObject& rhs) const { return compare(rhs) < 0; }
	bool operator<=(const MojObject& rhs) const { return compare(rhs) <= 0; }
	bool operator>(const MojObject& rhs) const { return compare(rhs) > 0; }
	bool operator>=(const MojObject& rhs) const { return compare(rhs) >= 0; }

private:
	class Impl
	{
	public:
		virtual ~Impl() {}
		virtual Type type() const = 0;
		virtual Impl* clone() const = 0;
		virtual int compare (const Impl& rhs) const = 0;
		virtual bool equals(const Impl& rhs) const = 0;
		virtual MojSize hashCode() const = 0;
		virtual MojSize objectSize() const = 0;
		virtual MojSize containerSize() const { return 0; }
		virtual bool boolValue() const { return false; }
		virtual MojInt64 intValue() const { return 0; }
		virtual MojDecimal decimalValue() const { return MojDecimal(); }
		virtual MojErr stringValue(MojString& valOut) const;
		virtual MojErr visit(MojObjectVisitor& visitor) const = 0;
		virtual const PropMap* map() const { return NULL; }
		virtual bool get(const MojChar* key, MojObject& valOut) const;
		virtual MojErr del(const MojChar* key, bool& foundOut);
		virtual MojErr arrayBegin(ArrayIterator& iter) { iter = NULL; return MojErrNone; }
		virtual ConstArrayIterator arrayBegin() const { return NULL; }
		virtual ConstArrayIterator arrayEnd() const { return NULL; }
		virtual bool at(MojSize idx, MojObject& objOut) const { objOut.clear(); return false; }
	};
	class NullImpl : public Impl
	{
	public:
		virtual Type type() const { return TypeNull; }
		virtual Impl* clone() const { return new NullImpl(); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojSize objectSize() const { return sizeof(NullImpl); }
	};
	class UndefinedImpl : public NullImpl
	{
	public:
		virtual Type type() const { return TypeUndefined; }
		virtual Impl* clone() const { return new UndefinedImpl(); }
		virtual MojSize hashCode() const;
		virtual MojSize objectSize() const  { return sizeof(UndefinedImpl); }
	};
	class ObjectImpl : public Impl
	{
	public:
		virtual Type type() const { return TypeObject; }
		virtual Impl* clone() const { return new  ObjectImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual MojSize containerSize() const;
		virtual bool boolValue() const;
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual const PropMap* map() const;
		virtual bool get(const MojChar* key, MojObject& valOut) const;
		virtual MojErr del(const MojChar* key, bool& foundOut);
		virtual MojSize objectSize() const  { return sizeof(ObjectImpl); }

		PropMap m_props;
	};
	class ArrayImpl : public Impl
	{
	public:
		virtual Type type() const { return TypeArray; }
		virtual Impl* clone() const { return new ArrayImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual MojSize containerSize() const;
		virtual bool boolValue() const;
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojErr arrayBegin(ArrayIterator& iter);
		virtual ConstArrayIterator arrayBegin() const;
		virtual ConstArrayIterator arrayEnd() const;
		virtual bool at(MojSize idx, MojObject& objOut) const;
		virtual MojSize objectSize() const  { return sizeof(ArrayImpl); }

		ObjectVec m_vec;
	};
	class StringImpl : public Impl
	{
	public:
		StringImpl() {}
		StringImpl(const MojString& str) : m_val(str) {}
		virtual Type type() const { return TypeString; }
		virtual Impl* clone() const { return new StringImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual bool boolValue() const;
		virtual MojInt64 intValue() const;
		virtual MojDecimal decimalValue() const;
		virtual MojErr stringValue(MojString& valOut) const;
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojSize objectSize() const  { return sizeof(StringImpl); }

		MojString m_val;
	};
	class BoolImpl : public Impl
	{
	public:
		BoolImpl() : m_val(false) {}
		BoolImpl(bool val) : m_val(val) {}
		virtual Type type() const { return TypeBool; }
		virtual Impl* clone() const { return new BoolImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual bool boolValue() const { return m_val; }
		virtual MojInt64 intValue() const { return m_val; }
		virtual MojDecimal decimalValue() const { return MojDecimal(m_val, 0); }
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojSize objectSize() const  { return sizeof(BoolImpl); }

		bool m_val;
	};
	class DecimalImpl : public Impl
	{
	public:
		DecimalImpl() {}
		DecimalImpl(const MojDecimal& val) : m_val(val) {}
		virtual Type type() const { return TypeDecimal; }
		virtual Impl* clone() const { return new DecimalImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual bool boolValue() const { return m_val.rep() != 0; }
		virtual MojInt64 intValue() const { return m_val.magnitude(); }
		virtual MojDecimal decimalValue() const { return m_val; }
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojSize objectSize() const  { return sizeof(DecimalImpl); }

		MojDecimal m_val;
	};
	class IntImpl : public Impl
	{
	public:
		IntImpl() : m_val(0) {}
		IntImpl(MojInt64 val) : m_val(val) {}
		virtual Type type() const { return TypeInt; }
		virtual Impl* clone() const { return new IntImpl(*this); }
		virtual int compare (const Impl& rhs) const;
		virtual bool equals(const Impl& rhs) const;
		virtual MojSize hashCode() const;
		virtual bool boolValue() const { return m_val != 0; }
		virtual MojInt64 intValue() const { return m_val; }
		virtual MojDecimal decimalValue() const { return MojDecimal(m_val); }
		virtual MojErr visit(MojObjectVisitor& visitor) const;
		virtual MojSize objectSize() const  { return sizeof(IntImpl); }

		MojInt64 m_val;
	};

	MojObject(const MojChar*); // avoid coercion for illegal assignment
	void init(Type type);
	void init(const MojObject& obj);
	void release();
	ObjectImpl& ensureObject();
	ArrayImpl& ensureArray();
	Impl* impl() { return m_implementation; }
	const Impl* impl() const { return m_implementation; }

	template<class T>
	MojErr getRequiredT(const MojChar* key, T& valOut) const;
	template<class T>
	MojErr getRequiredErrT(const MojChar* key, T& valOut) const;

	Impl* m_implementation;
};

class MojObjectVisitor : private MojNoCopy
{
public:
	virtual ~MojObjectVisitor() {}

	// virtual interface
	virtual MojErr reset() = 0;
	virtual MojErr beginObject() = 0;
	virtual MojErr endObject() = 0;
	virtual MojErr beginArray() = 0;
	virtual MojErr endArray() = 0;
	virtual MojErr propName(const MojChar* name, MojSize len) = 0;
	virtual MojErr nullValue() = 0;
	virtual MojErr boolValue(bool val) = 0;
	virtual MojErr intValue(MojInt64 val) = 0;
	virtual MojErr decimalValue(const MojDecimal& val) = 0;
	virtual MojErr stringValue(const MojChar* val, MojSize len) = 0;

	// convenience methods
	MojErr propName(const MojChar* name) { return propName(name, MojStrLen(name)); }
	MojErr stringValue(const MojChar* val) { return stringValue(val, MojStrLen(val)); }
	MojErr boolProp(const MojChar* name, bool val);
	MojErr intProp(const MojChar* name, MojInt64 val);
	MojErr decimalProp(const MojChar* name, const MojDecimal& val);
	MojErr stringProp(const MojChar* name, const MojChar* val);
	MojErr objectProp(const MojChar* name, const MojObject& val);
};

class MojObjectEater : public MojObjectVisitor
{
public:
	virtual MojErr reset() { return MojErrNone; }
	virtual MojErr beginObject() { return MojErrNone; }
	virtual MojErr endObject() { return MojErrNone; }
	virtual MojErr beginArray() { return MojErrNone; }
	virtual MojErr endArray() { return MojErrNone; }
	virtual MojErr propName(const MojChar* name, MojSize len) { return MojErrNone; }
	virtual MojErr nullValue() { return MojErrNone; }
	virtual MojErr boolValue(bool val) { return MojErrNone; }
	virtual MojErr intValue(MojInt64 val) { return MojErrNone; }
	virtual MojErr decimalValue(const MojDecimal& val) { return MojErrNone; }
	virtual MojErr stringValue(const MojChar* val, MojSize len) { return MojErrNone; }
};

template<>
struct MojComp<const MojObject>
{
	int operator()(const MojObject& val1, const MojObject& val2)
	{
		return val1.compare(val2);
	}
};

template<>
struct MojComp<MojObject> : public MojComp<const MojObject> {};

template<>
struct MojHasher<MojObject>
{
	MojSize operator()(const MojObject& val)
	{
		return val.hashCode();
	}
};

#endif /* MOJOBJECT_H_ */
