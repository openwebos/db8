/* @@@LICENSE
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2013 LG Electronics, Inc.
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


#ifndef MOJDBLUNASERVICEAPP_H_
#define MOJDBLUNASERVICEAPP_H_

#include "db/MojDbDefs.h"
#include "db/MojDbServiceHandlerInternal.h"
#include "db/MojDbServiceHandler.h"
#include "core/MojReactorApp.h"
#include "core/MojGmainReactor.h"
#include "core/MojMessageDispatcher.h"
#include "luna/MojLunaService.h"

class MojDbEnv;

class MojDbLunaServiceApp : public MojReactorApp<MojGmainReactor>
{
public:
	MojDbLunaServiceApp();
	virtual ~MojDbLunaServiceApp();
	virtual MojErr configure(const MojObject& conf);
	virtual MojErr init();
	virtual MojErr open();
	virtual MojErr close();

private:
	static const MojChar* const VersionString;
	static const MojChar* const MainDir;
	static const MojChar* const TempDir;
	static const MojChar* const TempStateDir;
	static const MojChar* const TempInitStateFile;
	static const MojInt32 NumThreads = 3;

	typedef MojReactorApp<MojGmainReactor> Base;

	class DbService
	{
	public:
		DbService(MojMessageDispatcher& dispatcher);
		MojErr init(MojReactor& reactor);
		MojErr open(MojGmainReactor& reactor, MojDbEnv* env,
					const MojChar* serviceName, const MojChar* baseDir, const MojChar* subDir);
		MojErr openDb(MojDbEnv* env, const MojChar* baseDir, const MojChar* subDir);
		MojErr close();

		MojDb& db() { return m_db; }
		MojLunaService& service() { return m_service; }

	private:
		MojDb m_db;
		MojLunaService m_service;
		MojRefCountedPtr<MojDbServiceHandler> m_handler;
	};

	MojErr dropTemp();
	virtual MojErr handleArgs(const StringVec& args);
	virtual MojErr displayUsage();

	MojString m_dbDir;
	DbService m_mainService;
	DbService m_tempService;
	MojMessageDispatcher m_dispatcher;
	MojRefCountedPtr<MojDbEnv> m_env;
	MojRefCountedPtr<MojDbServiceHandlerInternal> m_internalHandler;
};

#endif /* MOJDBLUNASERVICEAPP_H_ */
