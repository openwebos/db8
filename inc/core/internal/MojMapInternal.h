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


#ifndef MOJMAPINTERNAL_H_
#define MOJMAPINTERNAL_H_

template<>
struct MojComp<const MojMap<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> >
{
	int operator()(const MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>& val1, const MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>& val2)
	{
		return val1.compare(val2);
	}
};

template<>
struct MojComp<MojMap<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> >
: public MojComp<const MojMap<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> > {};

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> inline
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::ConstIterator::operator==(const Iterator& rhs) const
{
	return m_node == rhs.m_node;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP> inline
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::ConstIterator::operator!=(const Iterator& rhs) const
{
	return m_node != rhs.m_node;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
bool MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::get(const LookupType& key, ValueType& valOut) const
{
	ConstIterator iter = find(key);
	if (iter == end())
		return false;
	valOut = iter.value();
	return true;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
MojErr MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::put(const KeyType& key, const ValueType& val)
{
	MojAutoPtr<Node> node(new MapNode(key, val));
	MojAllocCheck(node.get());
	MojErr err = MojRbTreeBase::put(node, &key);
	MojErrCheck(err);

	return MojErrNone;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
int MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::compareFull(const Node* node1, const Node* node2) const
{
	MojAssert(node1 && node2);
	const MapNode* mapNode1 = static_cast<const MapNode*>(node1);
	const MapNode* mapNode2 = static_cast<const MapNode*>(node2);
	int comp = KCOMP()(mapNode1->m_key, mapNode2->m_key);
	if (comp == 0)
		comp = VCOMP()(mapNode1->m_val, mapNode2->m_val);
	return comp;
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
int MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::compareKey(const Node* node, const void* key) const
{
	MojAssert(node && key);
	return KCOMP()(static_cast<const MapNode*>(node)->m_key, *static_cast<const LookupType*>(key));
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
void MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::deleteNode(Node* node) const
{
	MojAssert(node);
	delete static_cast<MapNode*>(node);
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
MojRbTreeBase::Node* MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::cloneNode(const Node* node) const
{
	MojAssert(node);
	const MapNode* mapNode = static_cast<const MapNode*>(node);
	return new MapNode(mapNode->m_key, mapNode->m_val);
}

template<class KEY, class VAL, class LKEY, class KCOMP, class VCOMP>
const void* MojMap<KEY, VAL, LKEY, KCOMP, VCOMP>::key(const Node* node) const
{
	MojAssert(node);
	return &static_cast<const MapNode*>(node)->m_key;
}

#endif /* MOJMAPINTERNAL_H_ */
