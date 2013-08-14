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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "core/MojLogEngine.h"
#include "core/MojLog.h"

const MojChar* const MojLogger::s_levelNames[] = {
	_T("trace"),
	_T("debug"),
	_T("info"),
	_T("notice"),
	_T("warning"),
	_T("error"),
	_T("critical"),
	_T("none")
};

MojLogger::MojLogger(const MojChar* name, MojLogEngine* engine)
: m_engine(engine),
  m_name(name),
  m_level(LevelDefault),
  m_data(NULL)
{
	MojAssertNoLog(name);
	if (m_engine == NULL)
		m_engine = MojLogEngine::instance();
	m_engine->addLogger(this);
#if defined(USE_PMLOG)
    PmLogGetContext(m_name, &m_context);
#endif  // USE_PMLOG
}

MojLogger::~MojLogger()
{
	if (m_entry.inList()) {
		m_engine->removeLogger(this);
	}
}

void MojLogger::log(Level level, const MojChar* format, ...)
{
	MojAssertNoLog(format);
	MojAssertNoLog(m_engine);

	if (level >= m_level) {
		va_list args;
		va_start(args, format);
		m_engine->log(level, this, format, args);
		va_end(args);
	}
}

void MojLogger::vlog(Level level, const MojChar* format, va_list args)
{
	MojAssertNoLog(format);
	MojAssertNoLog(m_engine);

	if (level >= m_level) {
		m_engine->log(level, this, format, args);
	}
}

const MojChar* MojLogger::stringFromLevel(Level level)
{
	if (level > LevelMax)
		return _T("unknown");
	return s_levelNames[level];
}

MojErr MojLogger::levelFromString(const MojChar* str, Level& levelOut)
{
	MojAssertNoLog(str);

	for (int i = 0; i <= LevelMax; ++i) {
		if (!MojStrCmp(str, s_levelNames[i])) {
			levelOut = (Level) i;
			return MojErrNone;
		}
	}
	MojErrThrowMsg(MojErrLogLevelNotFound, _T("log: level not found: '%s'"), str);
}

#if defined(USE_PMLOG)
char * MojLogger::fmtUnique(char *dest, const char *pFile, int32_t lineNbr)
{
    const char *pStart = strrchr (pFile, '/');
    gchar *str = g_ascii_strup((pStart ? pStart + 1 : pFile), MOJLOG_UNIQUE_MAX - 6);
    char *ptr = strchr(str, '.');
    if (ptr) *ptr = '\0'; // trim off file extensions
    snprintf (dest, MOJLOG_UNIQUE_MAX, "%s#%d", str, lineNbr);
    g_free (str);
    return dest;
}
#endif

MojLogTracer::MojLogTracer(MojLogger& logger, const MojChar* function, const MojChar* file, int line)
: m_logger(logger),
  m_function(function),
  m_level(indentLevel(1))
{
	MojAssertNoLog(function && file);
	logger.log(MojLogger::LevelTrace, _T("%*s-> %s (%s:%d)"),
			m_level * IndentSpaces, _T(""), function, MojFileNameFromPath(file), line);
}

MojLogTracer::~MojLogTracer()
{
	m_logger.log(MojLogger::LevelTrace, _T("%*s<- %s"),
			m_level * IndentSpaces, _T(""), m_function);
	indentLevel(-1);
}

int MojLogTracer::indentLevel(int inc)
{
	static MojThreadLocalValue<int, MojThreadLocalValueZeroCtor<int> > s_level;
	int *level = NULL;
	MojErr err = s_level.get(level);
	MojErrCatchAllNoLog(err) {
		return 0;
	}
	int curVal = *level;
	*level += inc;
	return curVal;
}

