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


#ifndef MOJDBOBJECTITEM_H_
#define MOJDBOBJECTITEM_H_

#include "db/MojDbDefs.h"
#include "db/MojDbStorageEngine.h"

class MojDbObjectItem : public MojDbStorageItem
{
public:
	MojDbObjectItem(const MojObject& obj);

	virtual MojErr close();
	virtual MojErr kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine);
	virtual MojErr visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected = true) const;
	virtual const MojObject& id() const;
	virtual MojSize size() const;

	const MojObject& obj() const { return m_obj; }
	const MojDbKeyBuilder::KeySet sortKeys() const { return m_sortKeys; }
	void sortKeys(const MojDbKeyBuilder::KeySet& keys) { m_sortKeys = keys; }

private:
	MojObject m_obj;
	MojDbKeyBuilder::KeySet m_sortKeys;
};

#endif /* MOJDBOBJECTITEM_H_ */
