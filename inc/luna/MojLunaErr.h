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


#ifndef MOJLUNAERR_H_
#define MOJLUNAERR_H_

#include "luna/MojLunaDefs.h"
#include "core/MojNoCopy.h"

class MojLunaErr : private MojNoCopy
{
public:
	MojLunaErr() { LSErrorInit(&m_err); }
	~MojLunaErr() { LSErrorFree(&m_err); }

	MojErr toMojErr() const;
	const MojChar* message() const { return m_err.message; }
	const MojChar* file() const { return m_err.file; }
	int line() const { return m_err.line; }

	operator LSError*() { return &m_err; }

private:
	mutable LSError m_err;
};

#define MojLsErrCheck(RETVAL, LSERR) if (!(RETVAL)) MojErrThrowMsg((LSERR).toMojErr(), _T("luna-service: %s (%s:%d)"), (LSERR).message(), (LSERR).file(), (LSERR).line())
#define MojLsErrAccumulate(EACC, RETVAL, LSERR) if (!(RETVAL)) EACC = (LSERR).toMojErr()

#endif /* MOJLUNAERR_H_ */
