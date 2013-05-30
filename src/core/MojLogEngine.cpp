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


#include "core/MojLogEngine.h"
#include "core/MojObject.h"
#include "core/MojTime.h"

#ifdef MOJ_HAVE_SYSLOG_H
#	include "syslog.h"
#endif

#ifdef MOJ_USE_PMLOG
#	include "core/MojPmLogAppender.h"
#endif

const MojChar* const MojLogEngine::AppenderKey = _T("appender");
const MojChar* const MojLogEngine::DefaultKey = _T("default");
const MojChar* const MojLogEngine::ErrorKey = _T("error");
const MojChar* const MojLogEngine::LevelsKey = _T("levels");
const MojChar* const MojLogEngine::LogKey = _T("log");
const MojChar* const MojLogEngine::PathKey = _T("path");
const MojChar* const MojLogEngine::TypeKey = _T("type");

MojFileAppender::MojFileAppender()
: m_close(false),
  m_file(MojInvalidFile)
{
}

MojFileAppender::~MojFileAppender()
{
	(void) close();
}

void MojFileAppender::open(MojFileT file)
{
	MojAssertNoLog(m_file == MojInvalidFile && m_close == false);
	m_file = file;
}

MojErr MojFileAppender::open(const MojChar* path)
{
	MojAssertNoLog(m_file == MojInvalidFile);
	MojAssertNoLog(path);

	MojErr err = MojFileOpen(m_file, path,
			MOJ_O_CREAT | MOJ_O_APPEND | MOJ_O_WRONLY,
			MOJ_S_IRUSR | MOJ_S_IWUSR);
	MojErrCheckNoLog(err);
	m_close = true;

	return MojErrNone;
}

MojErr MojFileAppender::close()
{
	MojErr err = MojErrNone;
	if (m_close) {
		MojErr errClose = MojFileClose(m_file);
		MojErrAccumulateNoLog(err, errClose);
		m_close = false;
		m_file = MojInvalidFile;
	}
	return err;
}

MojErr MojFileAppender::append(MojLogger::Level level, MojLogger* logger, const MojChar* format, va_list args)
{
	MojAssertNoLog(format);
	MojAssertNoLog(m_file != MojInvalidFile);

	// get current time
	MojTime time = 0;
	MojErr err = MojGetCurrentTime(time);
	MojErrCheckNoLog(err);
	MojTmT tm;
	err = MojLocalTime(time, tm);
	MojErrCheckNoLog(err);

	// format message
	err =  m_buf.format(_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] [%p] [%s]"),
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, time.millisecsPart(),
			(void*) (gintptr) MojThreadCurrentId(),
			MojLogger::stringFromLevel(level));
	MojErrCheckNoLog(err);
	if (logger) {
		err = m_buf.appendFormat(_T(" [%s]: "), logger->name());
		MojErrCheckNoLog(err);
	} else {
		err = m_buf.append(_T(": "));
		MojErrCheckNoLog(err);
	}
	err =  m_buf.appendVFormat(format, args);
	MojErrCheckNoLog(err);
	err = m_buf.append(_T('\n'));
	MojErrCheckNoLog(err);

	// append to file
	const guint8* begin = (const guint8*) m_buf.begin();
	const guint8* end = (const guint8*) m_buf.end();
	while (begin < end) {
		gsize written = 0;
		err = MojFileWrite(m_file, begin, end - begin, written);
		MojErrCheckNoLog(err);
		begin += written;
	}
	return MojErrNone;
}

#ifdef MOJ_USE_SYSLOG
MojSyslogAppender::MojSyslogAppender()
{
}

MojSyslogAppender::~MojSyslogAppender()
{
	close();
}

void MojSyslogAppender::open(const MojChar* name)
{
	openlog(name, LOG_CONS | LOG_PID, LOG_USER);
}

void MojSyslogAppender::close()
{
	closelog();
}

MojErr MojSyslogAppender::append(MojLogger::Level level, MojLogger* logger, const MojChar* format, va_list args)
{
	MojAssertNoLog(format);

	static const int s_syslogLevels[] = {
		LOG_DEBUG,
		LOG_DEBUG,
		LOG_INFO,
		LOG_NOTICE,
		LOG_WARNING,
		LOG_ERR,
		LOG_CRIT
	};
	MojAssertNoLog(sizeof(s_syslogLevels) / sizeof(int) == MojLogger::LevelMax);

	int syslogLevel = LOG_DEBUG;
	if (level >= 0 && level < MojLogger::LevelMax)
		syslogLevel = s_syslogLevels[level];

	m_buf.clear();
	if (logger) {
		MojErr err = m_buf.appendFormat(_T("[%s] "), logger->name());
		MojErrCheckNoLog(err);
	}
	MojErr err = m_buf.appendVFormat(format, args);
	MojErrCheckNoLog(err);

	(void) syslog(syslogLevel, "%s", m_buf.data());

	return MojErrNone;
}
#endif // MOJ_USE_SYSLOG

MojLogEngine::MojLogEngine()
: m_defaultLogger(DefaultKey, this),
  m_errorLogger(ErrorKey, this),
  m_appender(&m_defaultAppender),
  m_logThread(MojInvalidThreadId)
{
	m_defaultAppender.open(MojStdOutFile);
}

MojLogEngine::~MojLogEngine()
{
	MojThreadGuard guard(m_mutex);
	m_loggers.clear();
}

MojLogEngine* MojLogEngine::instance()
{
	static MojLogEngine s_engine;
	return &s_engine;
}

MojErr MojLogEngine::loggerNames(StringSet& setOut)
{
	setOut.clear();
	MojThreadGuard guard(m_mutex);
	for (LoggerList::ConstIterator i = m_loggers.begin(); i != m_loggers.end(); ++i) {
		MojString name;
		MojErr err = name.assign((*i)->name());
		MojErrCheck(err);
		err = setOut.put(name);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojLogEngine::configure(const MojObject& conf, const MojChar* name)
{
	MojAssert(name);

	MojErr err = MojErrNone;
	MojLogger::Level defaultLevel = MojLogger::LevelDefault;
	MojAutoPtr<MojLogAppender> appender;
	LevelMap levels;

	// validate entire configuration before we apply any changes
	MojObject logConf;
	if (conf.get(LogKey, logConf)) {
		// appender
		MojObject obj;
		if (logConf.get(AppenderKey, obj)) {
			err = createAppender(obj, name, appender);
			MojErrCheck(err);
		}
		// levels
		if (logConf.get(LevelsKey, obj)) {
			// configure loggers
			MojString str;
			for (MojObject::ConstIterator i = obj.begin(); i != obj.end(); ++i) {
				MojLogger::Level level = MojLogger::LevelNone;
				err = i.value().stringValue(str);
				MojErrCheck(err);
				err = MojLogger::levelFromString(str, level);
				MojErrCheck(err);
				if (i.key() == DefaultKey) {
					defaultLevel = level;
				} else {
					err = levels.put(i.key(), level);
					MojErrCheck(err);
				}
			}
		}
	}

	// apply changes
	MojThreadGuard guard(m_mutex);
	// update name
	err = m_name.assign(name);
	MojErrCheck(err);
	// update appender
	updateAppender(appender);
	// update levels
	m_levels = levels;
	m_defaultLogger.level(defaultLevel);
	updateLoggers();

	return MojErrNone;
}

void MojLogEngine::reset(const MojLogger::Level level)
{
	MojThreadGuard guard(m_mutex);

	m_levels.clear();
	m_defaultLogger.level(level);
	updateLoggers();
}

void MojLogEngine::appender(MojAutoPtr<MojLogAppender> appender)
{
	MojThreadGuard guard(m_mutex);
	updateAppender(appender);
}

void MojLogEngine::log(MojLogger::Level level, MojLogger* logger, const MojChar* format, va_list args)
{
	MojAssertNoLog(logger && format);
	MojAssert(m_appender);

	// prevent recursive logging of errors that occur during logging
	MojThreadIdT self = MojThreadCurrentId();
	if (m_logThread == self)
		return;

	// suppress logger name for traces and error macros
	if (level == MojLogger::LevelTrace || logger == &m_errorLogger)
		logger = NULL;

	MojThreadGuard guard(m_mutex);
	MojAssertNoLog(m_logThread == MojInvalidThreadId);
	m_logThread = self;
	(void) m_appender->append(level, logger, format, args);
	m_logThread = MojInvalidThreadId;
}

void MojLogEngine::addLogger(MojLogger* logger)
{
	MojAssertNoLog(logger);

	MojThreadGuard guard(m_mutex);
	m_loggers.pushBack(logger);
	updateLoggerLevel(logger);
}

void MojLogEngine::removeLogger(MojLogger* logger)
{
	MojAssertNoLog(logger);

	MojThreadGuard guard(m_mutex);
	m_loggers.erase(logger);
}

void MojLogEngine::updateAppender(MojAutoPtr<MojLogAppender> appender)
{
	MojAssertMutexLocked(m_mutex);

	if (appender.get()) {
		m_configuredAppender = appender;
		m_appender = m_configuredAppender.get();
	} else {
		m_configuredAppender.reset();
		m_appender = &m_defaultAppender;
	}
}

void MojLogEngine::updateLoggerLevel(MojLogger* logger)
{
	MojAssertMutexLocked(m_mutex);
	MojAssert(logger);

	bool matched = false;
	const MojChar* loggerName = logger->name();
	gsize loggerLen = MojStrLen(loggerName);

	for (LevelMap::ConstIterator i = m_levels.begin(); i != m_levels.end(); ++i) {
		const MojChar* confName = i.key().data();
		gsize confLen = i.key().length();

		if (loggerLen >= confLen &&
			!MojStrNCmp(loggerName, confName, confLen) &&
			(loggerName[confLen] == _T('\0') || loggerName[confLen] == _T('.')))
		{
			// update the level, but keep iterating because there may be a longer match
			logger->level(i.value());
			matched = true;
		}
	}
	if (!matched) {
		// default to default
		logger->level(m_defaultLogger.level());
	}
}

void MojLogEngine::updateLoggers()
{
	for (LoggerList::Iterator i = m_loggers.begin(); i != m_loggers.end(); ++i) {
		updateLoggerLevel(*i);
	}
}

MojErr MojLogEngine::createAppender(const MojObject& conf, const MojChar* name, MojAutoPtr<MojLogAppender>& appenderOut)
{
	MojString type;
	MojErr err = conf.getRequired(TypeKey, type);
	MojErrCheck(err);

	// TODO: use some sort of factory registry to do this
	if (type == _T("file")) {
		MojString path;
		err = conf.getRequired(PathKey, path);
		MojErrCheck(err);
		MojAutoPtr<MojFileAppender> appender(new MojFileAppender());
		MojAllocCheck(appender.get());
		err = appender->open(path.data());
		MojErrCheck(err);
		appenderOut = appender;
	} else if (type == _T("stderr")) {
		MojAutoPtr<MojFileAppender> appender(new MojFileAppender());
		MojAllocCheck(appender.get());
		appender->open(MojStdErrFile);
		appenderOut = appender;
	} else if (type == _T("stdout")) {
		MojAutoPtr<MojFileAppender> appender(new MojFileAppender());
		MojAllocCheck(appender.get());
		appender->open(MojStdOutFile);
		appenderOut = appender;
	}
#ifdef MOJ_USE_SYSLOG
	else if (type == _T("syslog")) {
		MojAutoPtr<MojSyslogAppender> appender(new MojSyslogAppender());
		MojAllocCheck(appender.get());
		appender->open(name);
		appenderOut = appender;
	}
#endif // MOJ_USE_SYSLOG
#ifdef MOJ_USE_PMLOG
	else if (type == _T("pmlog")) {
		MojAutoPtr<MojPmLogAppender> appender(new MojPmLogAppender());
		MojAllocCheck(appender.get());
		appenderOut = appender;
	}
#endif // MOJ_USE_PMLOG
	else {
		MojErrThrowMsg(MojErrLogAppenderNotFound, _T("log: appender not found '%s'"), type.data());
	}
	return MojErrNone;
}
