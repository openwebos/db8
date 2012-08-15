/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#include "MojLogTest.h"
#include "core/MojLogEngine.h"

class TestAppender : public MojLogAppender
{
public:
	struct Rec
	{
		MojLogger::Level m_level;
		MojString m_logger;
		MojString m_msg;
	};

	virtual MojErr append(MojLogger::Level level, MojLogger* logger, const MojChar* format, va_list args)
	{
		Rec rec;
		rec.m_level = level;
		MojErr err = rec.m_logger.assign(logger->name());
		MojTestErrCheck(err);
		err = rec.m_msg.vformat(format, args);
		MojTestErrCheck(err);
		err = m_recs.push(rec);
		MojTestErrCheck(err);

		return MojErrNone;
	}

	MojVector<Rec> m_recs;
};

MojLogTest::MojLogTest()
: MojTestCase(_T("MojLog"))
{
}

MojErr MojLogTest::run()
{
	MojLogEngine engine;
	TestAppender* appender = new TestAppender;
	MojTestAssert(appender);
	MojAutoPtr<MojLogAppender> appenderPtr(appender);
	engine.appender(appenderPtr);

	MojLogger* defaultLogger = engine.defaultLogger();
	MojLogger* errorLogger = engine.errorLogger();
	defaultLogger->log(MojLogger::LevelDebug, _T("hello %d"), 1);
	MojTestAssert(appender->m_recs.empty());
	defaultLogger->log(MojLogger::LevelNotice, _T("hello %d"), 2);
	MojTestAssert(appender->m_recs.size() == 1);
	const TestAppender::Rec* rec = &appender->m_recs.back();
	MojTestAssert(rec->m_logger == _T("default") && rec->m_level == MojLogger::LevelNotice);
	MojTestAssert(rec->m_msg == _T("hello 2"));

	defaultLogger->level(MojLogger::LevelDebug);
	defaultLogger->log(MojLogger::LevelDebug, _T("hello %d"), 3);
	MojTestAssert(appender->m_recs.size() == 2);
	rec = &appender->m_recs.back();
	MojTestAssert(rec->m_logger == _T("default") && rec->m_level == MojLogger::LevelDebug);
	MojTestAssert(rec->m_msg == _T("hello 3"));
	errorLogger->log(MojLogger::LevelDebug, _T("hello %d"), 4);
	MojTestAssert(appender->m_recs.size() == 2);

	MojLogger a(_T("a"), &engine);
	MojTestAssert(a.level() == MojLogger::LevelDebug);
	a.log(MojLogger::LevelDebug, _T("hello %d"), 5);
	MojTestAssert(appender->m_recs.size() == 3);

	MojObject conf;
	MojErr err = conf.fromJson(
		_T("{\"log\":{\"levels\":{\"a\":\"info\",\"ab\":\"critical\",\"a.b\":\"error\",\"a.b.d\":\"debug\",\"c.d\":\"warning\",\"f\":\"notice\"}}}")
		);
	MojTestErrCheck(err);
	err = engine.configure(conf, _T("test"));
	MojTestErrCheck(err);

	MojLogger ab(_T("ab"), &engine);
	MojLogger a_b(_T("a.b"), &engine);
	MojLogger a_b_d(_T("a.b.d"), &engine);
	MojLogger a_c(_T("a.c"), &engine);
	MojLogger a_b_c(_T("a.b.c"), &engine);
	MojLogger c(_T("c"), &engine);
	MojLogger c_d(_T("c.d"), &engine);
	MojLogger f(_T("f"), &engine);

	MojTestAssert(a.level() == MojLogger::LevelInfo);
	MojTestAssert(ab.level() == MojLogger::LevelCritical);
	MojTestAssert(a_b.level() == MojLogger::LevelError);
	MojTestAssert(a_b_d.level() == MojLogger::LevelDebug);
	MojTestAssert(a_c.level() == MojLogger::LevelInfo);
	MojTestAssert(a_b_c.level() == MojLogger::LevelError);
	MojTestAssert(c.level() == MojLogger::LevelDefault);
	MojTestAssert(c_d.level() == MojLogger::LevelWarning);
	MojTestAssert(f.level() == MojLogger::LevelNotice);

	conf.clear(MojObject::TypeObject);
	err = engine.configure(conf, _T("test"));
	MojTestErrCheck(err);

	MojTestAssert(a.level() == MojLogger::LevelDefault);
	MojTestAssert(ab.level() == MojLogger::LevelDefault);
	MojTestAssert(a_b.level() == MojLogger::LevelDefault);
	MojTestAssert(a_b_d.level() == MojLogger::LevelDefault);
	MojTestAssert(a_c.level() == MojLogger::LevelDefault);
	MojTestAssert(a_b_c.level() == MojLogger::LevelDefault);
	MojTestAssert(c.level() == MojLogger::LevelDefault);
	MojTestAssert(c_d.level() == MojLogger::LevelDefault);
	MojTestAssert(f.level() == MojLogger::LevelDefault);

	err = conf.fromJson(
		_T("{\"log\":{\"levels\":{\"default\":\"warning\"}}}")
		);
	MojTestErrCheck(err);
	err = engine.configure(conf, _T("test"));
	MojTestErrCheck(err);

	MojTestAssert(a.level() == MojLogger::LevelWarning);
	MojTestAssert(ab.level() == MojLogger::LevelWarning);
	MojTestAssert(a_b.level() == MojLogger::LevelWarning);
	MojTestAssert(a_b_d.level() == MojLogger::LevelWarning);
	MojTestAssert(a_c.level() == MojLogger::LevelWarning);
	MojTestAssert(a_b_c.level() == MojLogger::LevelWarning);
	MojTestAssert(c.level() == MojLogger::LevelWarning);
	MojTestAssert(c_d.level() == MojLogger::LevelWarning);
	MojTestAssert(f.level() == MojLogger::LevelWarning);

	return MojErrNone;
}
