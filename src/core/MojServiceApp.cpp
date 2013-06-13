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


#include "core/MojServiceApp.h"

MojServiceApp* MojServiceApp::s_instance = NULL;

MojServiceApp::MojServiceApp(MojUInt32 majorVersion, MojUInt32 minorVersion, const MojChar* versionString)
: MojApp(majorVersion, minorVersion, versionString),
  m_shutdown(false)
{
	if (s_instance == NULL)
		s_instance = this;
}

MojServiceApp::~MojServiceApp()
{
	if (s_instance == this) {
		s_instance = NULL;
	}
}

MojErr MojServiceApp::open()
{
	MojLogTrace(s_log);

	MojErr err = MojApp::open();
	MojErrCheck(err);

	// install signal handlers
	MojSigactionT sa;
	MojZero(&sa, sizeof(sa));
	// shutdown on SIGINT and SIGTERM
	sa.sa_handler = shutdownHandler;
	err = MojSigAction(MOJ_SIGINT, &sa, NULL);
	MojErrCheck(err);
	err = MojSigAction(MOJ_SIGTERM, &sa, NULL);
	MojErrCheck(err);
	// ignore SIGPIPE
	sa.sa_handler = MOJ_SIG_IGN;
	err = MojSigAction(MOJ_SIGPIPE, &sa, NULL);
	MojErrCheck(err);

	return MojErrNone;
}

void MojServiceApp::shutdown()
{
	MojLogTrace(s_log);

	m_shutdown = true;
}

void MojServiceApp::shutdownHandler(int sig)
{
	if (s_instance != NULL) {
		s_instance->shutdown();
	}
}
