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


#ifdef MOJ_USE_PMLOG

#include "core/MojPmLogAppender.h"
#include "core/MojServiceApp.h"
#include "PmLogLib.h"

MojPmLogAppender::MojPmLogAppender()
{
}

MojPmLogAppender::~MojPmLogAppender()
{
}

MojErr MojPmLogAppender::append(MojLogger::Level level, MojLogger* logger, const MojChar* format, va_list args)
{
	MojAssertNoLog(format);

	static const int s_pmlogLevels[] = {
		kPmLogLevel_Debug,
		kPmLogLevel_Debug,
		kPmLogLevel_Info,
		kPmLogLevel_Notice,
		kPmLogLevel_Warning,
		kPmLogLevel_Error,
		kPmLogLevel_Critical
	};
	MojAssertNoLog(sizeof(s_pmlogLevels) / sizeof(int) == MojLogger::LevelMax);

	int pmlogLevel = kPmLogLevel_Debug;
	if (level >= 0 && level < MojLogger::LevelMax)
		pmlogLevel = s_pmlogLevels[level];

	PmLogContext context = kPmLogGlobalContext;
	if (logger) {
		context = (PmLogContext) logger->data();
		if (!context) {
			PmLogErr err = PmLogGetContext(logger->name(), &context);
			if (err == kPmLogErr_None) {
				logger->setData((void *)context);
			}
		}
	}

	PmLogVPrint(context, pmlogLevel, format, args);

	return MojErrNone;
}

MojErr MojServiceApp::initPmLog()
{
	MojAutoPtr<MojLogAppender> appender(new MojPmLogAppender);
	MojAllocCheck(appender.get());
	MojLogEngine::instance()->appender(appender);

	return MojErrNone;
}

#endif // MOJ_USE_PMLOG

