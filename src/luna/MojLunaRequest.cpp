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


#include "luna/MojLunaRequest.h"

MojLunaRequest::MojLunaRequest(MojService* service)
: MojServiceRequest(service),
  m_onPublic(false),
  m_cancelled(false)
{
	MojAssert(service);
}

MojLunaRequest::MojLunaRequest(MojService* service, bool onPublic)
: MojServiceRequest(service),
  m_onPublic(onPublic),
  m_cancelled(false)
{
	MojAssert(service);
}

MojLunaRequest::MojLunaRequest(MojService* service, bool onPublic,
	const MojString& requester)
: MojServiceRequest(service),
  m_requester(requester),
  m_onPublic(onPublic),
  m_cancelled(false)
{
	MojAssert(service);
}

