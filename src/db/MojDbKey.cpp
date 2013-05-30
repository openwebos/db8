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


#include "db/MojDbKey.h"
#include "db/MojDbTextCollator.h"
#include "core/MojObjectSerialization.h"

MojErr MojDbKey::assign(const MojObject& obj, MojDbTextCollator* coll)
{
	if (coll && obj.type() == MojObject::TypeString) {
		MojString text;
		MojErr err = obj.stringValue(text);
		MojErrCheck(err);
		err = coll->sortKey(text, *this);
		MojErrCheck(err);
	} else {
		MojErr err = obj.toBytes(m_vec);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbKey::increment()
{
	ByteVec::Iterator iter;
	MojErr err = m_vec.end(iter);
	MojErrCheck(err);
	while (iter > m_vec.begin()) {
		if (*(--iter) < G_MAXUINT8) {
			++(*iter);
			break;
		}
		m_vec.pop();
	}
	return MojErrNone;
}

MojErr MojDbKey::prepend(const MojDbKey& key)
{
	MojErr err = m_vec.insert(0, key.m_vec.begin(), key.m_vec.end());
	MojErrCheck(err);

	return MojErrNone;
}

bool MojDbKey::prefixOf(const MojDbKey& key) const
{
	return (key.size() >= size() && MojMemCmp(data(), key.data(), size()) == 0);
}

bool MojDbKey::stringPrefixOf(const MojDbKey& key) const
{
	MojAssert(size() > 0);
	return (key.size() >= size() && MojMemCmp(data(), key.data(), size() - 1) == 0);
}

MojDbKeyRange::MojDbKeyRange(const MojDbKey& lowerKey, const MojDbKey& upperKey, guint32 group)
: m_group(group)
{
	m_keys[IdxLower] = lowerKey;
	m_keys[IdxUpper] = upperKey;
}

bool MojDbKeyRange::contains(const MojDbKey& key) const
{
	const MojDbKey& lk = lowerKey();
	const MojDbKey& uk = upperKey();
	if ((lk.empty() || key >= lk) && (uk.empty() || key < uk)) {
		return true;
	}
	return false;
}

bool MojDbKeyRange::contains(const MojDbKeyRange& range) const
{
	const MojDbKey& uk = upperKey();
	if (contains(range.lowerKey()) &&
		(uk.empty() || range.upperKey() <= uk))
		return true;
	return false;
}

MojErr MojDbKeyRange::prepend(const MojDbKey& key)
{
	MojErr err = lowerKey().prepend(key);
	MojErrCheck(err);
	err = upperKey().prepend(key);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKeyBuilder::push(const KeySet& vals)
{
	MojErr err = m_stack.push(PropRec(vals));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbKeyBuilder::keys(KeySet& keysOut)
{
	keysOut.clear();
	if (m_stack.empty())
		return MojErrNone;

	// reset iters
	PropStack::Iterator pos;
	MojErr err = m_stack.begin(pos);
	MojErrCheck(err);
	for (PropStack::Iterator i = pos; i != m_stack.end(); ++i)
		i->reset();
	// create set of all combinations containing one value from each property.
	// we do this iteratively, using our vector of values as a stack.
	PropStack::Iterator last = pos + m_stack.size() - 1;
	for(;;) {
		if (pos->m_iter == pos->m_vals.end()) {
			// if we reached the end of the first rec, we're done
			if (pos == m_stack.begin())
				break;
			// reset iter on current rec
			pos->reset();
			// pop back up one rec
			--pos;
			// advance the iter in the now-current rec
			++(pos->m_iter);
		} else if (pos == last) {
			// walk up the stack and create a key
			MojDbKey key;
			MojDbKey::ByteVec& vec = key.byteVec();
			for (PropStack::ConstIterator i = m_stack.begin();
				 i != m_stack.end();
				 ++i) {
				const MojDbKey::ByteVec& stackVec = i->m_iter->byteVec();
				err = vec.append(stackVec.begin(), stackVec.end());
				MojErrCheck(err);
			}
			// add key to output set
			err = keysOut.put(key);
			MojErrCheck(err);
			// advance iter in current rec
			++(pos->m_iter);
		} else {
			// push
			++pos;
		}
	}
	return MojErrNone;
}
