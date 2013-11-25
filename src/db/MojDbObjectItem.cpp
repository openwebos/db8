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


#include "db/MojDbObjectItem.h"
#include "db/MojDb.h"

MojDbObjectItem::MojDbObjectItem(const MojObject& obj)
: m_obj(obj)
{
}

MojErr MojDbObjectItem::close()
{
	return MojErrNone;
}

MojErr MojDbObjectItem::kindId(MojString& kindIdOut, MojDbKindEngine& kindEngine)
{
	MojErr err = m_obj.getRequired(MojDb::KindKey, kindIdOut);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbObjectItem::visit(MojObjectVisitor& visitor, MojDbKindEngine& kindEngine, bool headerExpected) const
{
	MojErr err = m_obj.visit(visitor);
	MojErrCheck(err);

	return MojErrNone;
}

const MojObject& MojDbObjectItem::id() const
{
	MojObject::ConstIterator iter = m_obj.find(MojDb::IdKey);
	if (iter == m_obj.end()) {
		return MojObject::Undefined;
	} else {
		return *iter;
	}
}

MojSize MojDbObjectItem::size() const
{
	return m_obj.size();
}
