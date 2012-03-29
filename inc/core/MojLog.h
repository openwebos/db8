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


#ifndef MOJLOG_H_
#define MOJLOG_H_

#include "core/MojListEntry.h"

#ifdef MOJ_HAVE_STDARG_H
#	include <stdarg.h>
#endif

#if defined(MOJ_DEBUG) || defined(MOJ_DEBUG_LOGGING)
#	define MojLogDebug(LOGGER, ...)		(LOGGER).log(MojLogger::LevelDebug, __VA_ARGS__)
#	define MojLogTrace(LOGGER)			MojLogTracer __MOJTRACER(LOGGER, _T(__PRETTY_FUNCTION__), _T(__FILE__), __LINE__)
#else
#	define MojLogDebug(LOGGER, ...)
#	define MojLogTrace(LOGGER)
#endif

#define MojLogInfo(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelInfo, __VA_ARGS__)
#define MojLogNotice(LOGGER, ...)		(LOGGER).log(MojLogger::LevelNotice, __VA_ARGS__)
#define MojLogWarning(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelWarning, __VA_ARGS__)
#define MojLogError(LOGGER, ...) 		(LOGGER).log(MojLogger::LevelError, __VA_ARGS__)
#define MojLogCritical(LOGGER, ...) 	(LOGGER).log(MojLogger::LevelCritical, __VA_ARGS__)

class MojLogger : private MojNoCopy
{
public:
	enum Level {
		LevelTrace,
		LevelDebug,
		LevelInfo,
		LevelNotice,
		LevelWarning,
		LevelError,
		LevelCritical,
		LevelNone,
		LevelDefault = LevelNotice,
		LevelMax = LevelNone
	};

	MojLogger(const MojChar* name, MojLogEngine* engine = NULL);
	~MojLogger();

	Level level() const { return m_level; }
	const MojChar* name() const { return m_name; }

	void level(Level level) { m_level = level; }
	void log(Level level, const MojChar* format, ...) MOJ_FORMAT_ATTR((printf, 3, 4));
	void vlog(Level level, const MojChar* format, va_list args) MOJ_FORMAT_ATTR((printf, 3, 0));

	static const MojChar* stringFromLevel(Level level);
	static MojErr levelFromString(const MojChar* str, Level& levelOut);

	void *data() const { return m_data; }
	void setData(void *data) { m_data = data; }

private:
	friend class MojLogEngine;

	MojListEntry m_entry;
	MojLogEngine* m_engine;
	const MojChar* m_name;
	Level m_level;
	void *m_data;

	static const MojChar* const s_levelNames[];
};

class MojLogTracer : public MojNoCopy
{
public:
	MojLogTracer(MojLogger& logger, const MojChar* function, const MojChar* file, int line);
	~MojLogTracer();

private:
	static const int IndentSpaces = 2;
	int indentLevel(int inc);

	MojLogger& m_logger;
	const MojChar* m_function;
	int m_level;
};

#endif /* MOJLOG_H_ */
