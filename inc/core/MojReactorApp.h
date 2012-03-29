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


#ifndef MOJREACTORAPP_H_
#define MOJREACTORAPP_H_

#include "core/MojCoreDefs.h"
#include "core/MojServiceApp.h"

template<class REACTOR>
class MojReactorApp : public MojServiceApp
{
public:
	virtual MojErr init();
	virtual MojErr run();
	virtual void shutdown();

protected:
	typedef REACTOR Reactor;

	MojReactorApp(MojUInt32 majorVersion = MajorVersion, MojUInt32 minorVersion = MinorVersion, const MojChar* versionString = NULL)
	: MojServiceApp(majorVersion, minorVersion, versionString) {}

	Reactor m_reactor;
};

#include "core/internal/MojReactorAppInternal.h"

#endif /* MOJREACTORAPP_H_ */
