/* @@@LICENSE
*
*      Copyright (c) 2009-2014 LG Electronics, Inc.
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


#include "MojDbDumpLoadTest.h"
#include "db/MojDb.h"
#include "core/MojUtil.h"

namespace {
	MojString sandboxFileName(const MojChar *name)
	{
		MojString path;
		MojErr err = path.format("%s/%s", MojDbTestDir, name);
		assert(err == MojErrNone);
		return path;
	}
}

static const MojChar* const MojLoadTestFileName = _T("loadtest.json");
static const MojChar* const MojDumpTestFileName = _T("dumptest.json");
static const MojChar* const MojTestStr =
	_T("{\"_id\":\"_kinds/LoadTest:1\",\"_kind\":\"Kind:1\",\"id\":\"LoadTest:1\",\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\"}]},{\"name\":\"barfoo\",\"props\":[{\"name\":\"bar\"},{\"name\":\"foo\"}]}]}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}")
	_T("{\"_kind\":\"LoadTest:1\",\"foo\":\"hello\",\"bar\":\"world\"}");

MojDbDumpLoadTest::MojDbDumpLoadTest()
: MojTestCase(_T("MojDbDumpLoad"))
{
}

MojErr MojDbDumpLoadTest::run()
{
	MojDb db;

	// open
	MojErr err = db.open(MojDbTestDir);
	MojTestErrCheck(err);

	// load
	err = MojFileFromString(sandboxFileName(MojLoadTestFileName), MojTestStr);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = db.load(sandboxFileName(MojLoadTestFileName), count);
	MojTestErrCheck(err);
	MojTestAssert(count == 11);
	err = checkCount(db);
	MojTestErrCheck(err);

	// dump
	count = 0;
	err = db.dump(sandboxFileName(MojDumpTestFileName), count);
	MojTestErrCheck(err);
	MojTestAssert(count == 11);

	// del and purge
	MojString id;
	err = id.assign(_T("LoadTest:1"));
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(id, found);
	MojTestErrCheck(err);
	MojTestAssert(found);
	err = db.purge(count, 0);
	MojTestErrCheck(err);

	// load again
	err = db.load(sandboxFileName(MojDumpTestFileName), count);
	MojTestErrCheck(err);
	MojTestAssert(count == 12);
	err = checkCount(db);
	MojTestErrCheck(err);

	// analyze
	MojObject analysis;
	err = db.stats(analysis);
	MojErrCheck(err);

	err = db.close();
	MojTestErrCheck(err);

	return MojErrNone;
}



MojErr MojDbDumpLoadTest::checkCount(MojDb& db)
{
	MojDbQuery query;
	MojErr err = query.from(_T("LoadTest:1"));
	MojTestErrCheck(err);
	MojDbCursor cursor;
	err = db.find(query, cursor);
	MojTestErrCheck(err);
	MojUInt32 count = 0;
	err = cursor.count(count);
	MojTestErrCheck(err);
	MojTestAssert(count == 10);

	return MojErrNone;
}

void MojDbDumpLoadTest::cleanup()
{
	(void) MojUnlink(sandboxFileName(MojLoadTestFileName));
	(void) MojUnlink(sandboxFileName(MojDumpTestFileName));
	(void) MojRmDirRecursive(MojDbTestDir);
}
