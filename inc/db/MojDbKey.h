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


#ifndef MOJDBKEY_H_
#define MOJDBKEY_H_

#include "db/MojDbDefs.h"
#include "core/MojBuffer.h"
#include "core/MojObject.h"
#include "core/MojSet.h"
#include "core/MojVector.h"

class MojDbKey
{
public:
	typedef MojVector<MojByte> ByteVec;

	MojDbKey() {}
	explicit MojDbKey(const ByteVec& vec) : m_vec(vec) {}

	void clear() { m_vec.clear(); }
	MojErr assign(const MojByte* data, MojSize size) { return m_vec.assign(data, data + size); }
	MojErr assign(const MojBuffer& buf) { return buf.toByteVec(m_vec); }
	MojErr assign(const MojObject& obj, MojDbTextCollator* coll = NULL);
	MojErr increment();
	MojErr prepend(const MojDbKey& key);

	const ByteVec& byteVec() const { return m_vec; }
	ByteVec& byteVec() { return m_vec; }
	const MojByte* data() const { return m_vec.begin(); }
	bool empty() const { return m_vec.empty(); }
	MojSize size() const { return m_vec.size(); }
	int compare(const MojDbKey& rhs) const { return m_vec.compare(rhs.m_vec); }
	bool prefixOf(const MojDbKey& key) const;
	bool stringPrefixOf(const MojDbKey& key) const;

	MojDbKey& operator=(const ByteVec& rhs) { m_vec = rhs; return *this; }
	bool operator==(const MojDbKey& rhs) const { return m_vec == rhs.m_vec; }
	bool operator!=(const MojDbKey& rhs) const { return m_vec != rhs.m_vec; }
	bool operator<(const MojDbKey& rhs) const { return m_vec < rhs.m_vec; }
	bool operator<=(const MojDbKey& rhs) const { return m_vec <= rhs.m_vec; }
	bool operator>(const MojDbKey& rhs) const { return m_vec > rhs.m_vec; }
	bool operator>=(const MojDbKey& rhs) const { return m_vec >= rhs.m_vec; }

private:
	ByteVec m_vec;
};

class MojDbKeyRange
{
public:
	typedef enum {
		IdxLower,
		IdxUpper
	} Index;

	MojDbKeyRange(const MojDbKey& lowerKey, const MojDbKey& upperKey, MojUInt32 group);

	bool contains(const MojDbKey& key) const;
	bool contains(const MojDbKeyRange& range) const;
	const MojDbKey& key(MojSize idx) const { MojAssert(idx <= IdxUpper); return m_keys[idx]; }
	const MojDbKey& lowerKey() const { return key(IdxLower); }
	const MojDbKey& upperKey() const { return key(IdxUpper); }
	MojUInt32 group() const { return m_group; }

	MojDbKey& key(MojSize idx) { MojAssert(idx <= IdxUpper); return m_keys[idx]; }
	MojDbKey& lowerKey() { return key(IdxLower); }
	MojDbKey& upperKey() { return key(IdxUpper); }
	MojErr prepend(const MojDbKey& key);

private:
	MojDbKey m_keys[2];
	MojUInt32 m_group;
};

class MojDbKeyBuilder : private MojNoCopy
{
public:
	typedef MojSet<MojDbKey> KeySet;

	MojDbKeyBuilder() {}

	void clear() { m_stack.clear(); }
	MojErr push(const KeySet& vals);
	MojErr keys(KeySet& keysOut);

private:
	struct PropRec {
		PropRec(const KeySet& vals) : m_vals(vals) { reset(); }
		void reset() { m_iter = m_vals.begin(); }
		KeySet m_vals;
		KeySet::ConstIterator m_iter;
	};
	typedef MojVector<PropRec> PropStack;

	PropStack m_stack;
};

template<>
struct MojComp<MojDbKey>
{
	int operator()(const MojDbKey& val1, const MojDbKey& val2)
	{
		return val1.compare(val2);
	}
};

#endif /* MOJDBKEY_H_	 */
