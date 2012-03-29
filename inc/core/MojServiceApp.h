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


#ifndef MOJSERVICEAPP_H_
#define MOJSERVICEAPP_H_

#include "core/MojCoreDefs.h"
#include "core/MojApp.h"

class MojServiceApp : public MojApp
{
public:
	virtual ~MojServiceApp();

protected:
	MojServiceApp(MojUInt32 majorVersion = MajorVersion, MojUInt32 minorVersion = MinorVersion, const MojChar* versionString = NULL);
	virtual MojErr open();
	virtual void shutdown();
	MojErr initPmLog();

	bool m_shutdown;

private:
	static void shutdownHandler(int sig);
	static MojServiceApp* s_instance;

	MojString m_name;
};

#endif /* MOJSERVICEAPP_H_ */
