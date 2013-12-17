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


#include "db/MojDbUtils.h"
#include "core/MojString.h"
#include "core/MojLogDb8.h"

struct MojDbCollationStrInfo {
	const MojChar* m_str;
	MojDbCollationStrength m_coll;
};

static MojDbCollationStrInfo s_collStrings[] = {
	{_T("primary"), MojDbCollationPrimary},
	{_T("secondary"), MojDbCollationSecondary},
	{_T("tertiary"), MojDbCollationTertiary},
	{_T("quaternary"), MojDbCollationQuaternary},
	{_T("identical"), MojDbCollationIdentical},
	{_T("invalid"), MojDbCollationInvalid}
};

const MojChar* MojDbUtils::collationToString(MojDbCollationStrength coll)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	const MojChar* str = NULL;
	MojDbCollationStrInfo* info = s_collStrings;
	do {
		str = info->m_str;
		if (info->m_coll == coll)
			break;
	} while ((info++)->m_coll != MojDbCollationInvalid);
	return str;
}

MojErr MojDbUtils::collationFromString(const MojString& str, MojDbCollationStrength& collOut)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	collOut = MojDbCollationInvalid;
	MojDbCollationStrInfo* info = s_collStrings;
	do {
		collOut = info->m_coll;
		if (str == info->m_str)
			break;
	} while ((info++)->m_coll != MojDbCollationInvalid);

	if (collOut == MojDbCollationInvalid)
		MojErrThrow(MojErrDbInvalidCollation);

	return MojErrNone;
}
