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


#ifndef MOJHASHMAPINTERNAL_H_
#define MOJHASHMAPINTERNAL_H_

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
bool MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::get(const LookupType& key, ValueType& valOut) const
{
	ConstIterator iter = find(key);
	if (iter == end())
		return false;
	valOut = iter.value();
	return true;
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
MojErr MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::put(const KeyType& key, const ValueType& val)
{
	MojSize idx = 0;
	Node* node = NULL;
	MojErr err = MojHashBase::find(&key, node, &idx);
	MojErrCheck(err);
	if (node) {
		static_cast<MapNode*>(node)->m_val = val;
	} else {
		MojAutoPtr<Node> node(new MapNode(key, val));
		MojAllocCheck(node.get());
		err = MojHashBase::put(node, idx, &key);
		MojErrCheck(err);
	}

	return MojErrNone;
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
bool MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::compareFull(const Node* node1, const Node* node2) const
{
	MojAssert(node1 && node2);
	const MapNode* mapNode1 = static_cast<const MapNode*>(node1);
	const MapNode* mapNode2 = static_cast<const MapNode*>(node2);
	if (KEQ()(mapNode1->m_key, mapNode2->m_key))
		return VEQ()(mapNode1->m_val, mapNode2->m_val);
	return false;
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
bool MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::compareKey(const Node* node, const void* key) const
{
	MojAssert(node && key);
	return KEQ()(static_cast<const MapNode*>(node)->m_key, *static_cast<const LookupType*>(key));
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
void MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::deleteNode(Node* node) const
{
	MojAssert(node);
	delete static_cast<MapNode*>(node);
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
MojHashBase::Node* MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::cloneNode(const Node* node) const
{
	MojAssert(node);
	const MapNode* mapNode = static_cast<const MapNode*>(node);
	return new MapNode(mapNode->m_key, mapNode->m_val);
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
const void* MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::key(const Node* node) const
{
	MojAssert(node);
	return &static_cast<const MapNode*>(node)->m_key;
}

template<class KEY, class VAL, class LKEY, class HASH, class KEQ, class VEQ>
MojSize MojHashMap<KEY, VAL, LKEY, HASH, KEQ, VEQ>::hash(const void* key) const
{
	MojAssert(key);
	return HASH()(*static_cast<const LookupType*>(key));
}

#endif /* MOJHASHMAPINTERNAL_H_ */
