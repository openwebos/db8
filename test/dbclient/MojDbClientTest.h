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


#ifndef MOJDBCLIENTTESTRUNNER_H_
#define MOJDBCLIENTTESTRUNNER_H_


#include "db/MojDbDefs.h"
#include "db/MojDbServiceClient.h"
#include "core/MojTestRunner.h"
#include "core/MojReactor.h"

class MojDbClientTestResultHandler;
class MojDbClientTestRunner: public MojTestRunner
{
private:
	void runTests();
};

class MojDbClientTest: public MojTestCase
{
public:
	MojDbClientTest();
	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr init();
	MojErr registerType();
	MojErr updateType();
	MojErr testPut();
	MojErr testMultiPut();
	MojErr testGet();
	MojErr testMultiGet();
	MojErr testDel();
	MojErr testMultiDel();
	MojErr testQueryDel();
	MojErr testMerge();
	MojErr testMultiMerge();
	MojErr testQueryMerge();
	MojErr testBasicFind();
	MojErr testBasicSearch();
	MojErr testUpdatedFind();
	MojErr testWatchFind();
	MojErr testWatch();
	MojErr testErrorHandling(bool multiResponse);
	MojErr testHdlrCancel();
	MojErr testCompact();
	MojErr testPurge();
	MojErr testPurgeStatus();
	MojErr testDeleteKind();
	MojErr testBatch();
	MojErr testPermissions();

	MojErr writeTestObj(const MojChar* json, MojObject* idOut = NULL);
	MojErr writeTestObj(const MojChar* json, bool kind, MojObject* idOut = NULL);
	MojErr getTestObj(MojObject id, MojObject& objOut);

	MojErr verifyFindResults(const MojDbClientTestResultHandler* handler,
			   			     MojObject idExpected, const MojChar* barExpected);
	MojErr delTestData();
	MojErr staleTestData(bool& resultOut);

	MojAutoPtr<MojService> m_service;
	MojAutoPtr<MojDbServiceClient> m_dbClient;
	MojAutoPtr<MojReactor> m_reactor;
};


#endif /* MOJDBCLIENTTESTRUNNER_H_ */
