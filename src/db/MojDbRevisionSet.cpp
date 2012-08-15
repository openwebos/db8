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


#include "db/MojDbRevisionSet.h"
#include "db/MojDb.h"

const MojChar* const MojDbRevisionSet::PropsKey = _T("props");
const MojChar* const MojDbRevisionSet::NameKey = _T("name");

MojLogger MojDbRevisionSet::s_log(_T("db.revset"));

MojDbRevisionSet::MojDbRevisionSet()
{
}

MojErr MojDbRevisionSet::fromObject(const MojObject& obj)
{
	MojLogTrace(s_log);

	// get name
	MojErr err = obj.getRequired(NameKey, m_name);
	MojErrCheck(err);

	// add props
	MojObject props;
	err = obj.getRequired(PropsKey, props);
	MojErrCheck(err);
	MojObject propObj;
	MojSize i = 0;
	while (props.at(i++, propObj)) {
		err = addProp(propObj);
		MojErrCheck(err);
	}
	if (m_props.empty()) {
		MojErrThrow(MojErrDbInvalidRevisionSet);
	}
	return MojErrNone;
}

MojErr MojDbRevisionSet::addProp(const MojObject& propObj)
{
	MojLogTrace(s_log);

	// and create extractor
	MojRefCountedPtr<MojDbPropExtractor> extractor(new MojDbPropExtractor);
	MojAllocCheck(extractor.get());
	MojErr err = extractor->fromObject(propObj, _T(""));
	MojErrCheck(err);
	err = m_props.push(extractor);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbRevisionSet::update(MojObject* newObj, const MojObject* oldObj) const
{
	MojAssert(newObj);

	if (!oldObj) {
		MojErr err = updateRev(*newObj);
		MojErrCheck(err);
	} else {
		MojErr err = diff(*newObj, *oldObj);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbRevisionSet::diff(MojObject& newObj, const MojObject& oldObj) const
{
	KeySet newVals;
	KeySet oldVals;
	for (PropVec::ConstIterator i = m_props.begin(); i != m_props.end(); ++i) {
		// get new vals
		newVals.clear();
		MojErr err = (*i)->vals(newObj, newVals);
		MojErrCheck(err);
		// get old vals
		oldVals.clear();
		err = (*i)->vals(oldObj, oldVals);
		MojErrCheck(err);
		// diff them
		if (newVals != oldVals) {
			err = updateRev(newObj);
			MojErrCheck(err);
			break;
		}
	}
	return MojErrNone;
}

MojErr MojDbRevisionSet::updateRev(MojObject& obj) const
{
	MojObject rev;
	MojErr err = obj.getRequired(MojDb::RevKey, rev);
	MojErrCheck(err);
	err = obj.put(m_name, rev);
	MojErrCheck(err);

	return MojErrNone;
}
