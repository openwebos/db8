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


#ifndef MOJLUNAREQUEST_H_
#define MOJLUNAREQUEST_H_

#include "MojLunaDefs.h"
#include "core/MojJson.h"
#include "core/MojServiceRequest.h"

class MojLunaRequest : public MojServiceRequest
{
public:
	MojLunaRequest(MojService* service);
	MojLunaRequest(MojService* service, bool onPublic);
	MojLunaRequest(MojService* service, bool onPublic, const MojString&
		requester);

	bool onPublic() const { return m_onPublic; }
	bool isProxyRequest() const { return !m_requester.empty(); }
	bool cancelled() const { return m_cancelled; }
	const MojChar* payload() const { return m_writer.json(); }
	const MojChar* getRequester() const { return m_requester.data(); }
	virtual MojObjectVisitor& writer() { return m_writer; }

private:
	friend class MojLunaService;

	void cancelled(bool val) { m_cancelled = val; }

	MojJsonWriter m_writer;

	MojString m_requester;

	bool m_onPublic;
	bool m_cancelled;
};

#endif /* MOJLUNAREQUEST_H_ */
