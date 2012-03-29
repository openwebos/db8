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


#ifndef MOJAPP_H_
#define MOJAPP_H_

#include "core/MojCoreDefs.h"
#include "core/MojAutoPtr.h"
#include "core/MojMap.h"
#include "core/MojObject.h"
#include "core/MojString.h"
#include "core/MojVector.h"

class MojApp : private MojNoCopy
{
public:
	static const MojUInt32 MajorVersion = 1;
	static const MojUInt32 MinorVersion = 0;

	virtual ~MojApp();
	const MojString& name() const { return m_name; }

	virtual int main(int argc, MojChar** argv);
	virtual MojErr configure(const MojObject& conf);
	virtual MojErr init();
	virtual MojErr open();
	virtual MojErr close();
	virtual MojErr run();

protected:
	typedef MojErr (MojApp::*OptionHandler)(const MojString& name, const MojString& val);
	typedef MojVector<MojString> StringVec;

	MojApp(MojUInt32 majorVersion = MajorVersion, MojUInt32 minorVersion = MinorVersion, const MojChar* versionString = NULL);
	MojErr registerOption(OptionHandler handler, const MojChar* opt, const MojChar* help, bool argRequired = false);

	virtual MojErr displayMessage(const MojChar* format, ...) MOJ_FORMAT_ATTR((printf, 2, 3));
	virtual MojErr displayErr(const MojChar* prefix, const MojChar* errToDisplay);
	virtual MojErr displayErr(MojErr errToDisplay, const MojChar* message);
	virtual MojErr displayLoggers();
	virtual MojErr displayUsage();
	virtual MojErr displayOptions();
	virtual MojErr displayVersion();
	virtual MojErr handleCommandLine(int argc, MojChar** argv);
	virtual MojErr handleArgs(const StringVec& args);

protected:
	static MojLogger s_log;

private:
	enum RunMode {
		ModeDefault,
		ModeHelp,
		ModeLoggers,
		ModeVersion
	};
	struct Option {
		Option(OptionHandler handler, const MojString& help, bool argRequired)
		: m_handler(handler), m_help(help), m_arg(argRequired) {}
		bool operator<(const Option& rhs) const { return m_help < rhs.m_help; }

		OptionHandler m_handler;
		MojString m_help;
		bool m_arg;
	};
	typedef MojMap<MojString, Option, const MojChar*> OptHandlerMap;

	MojErr handleConfig(const MojString& name, const MojString& val);
	MojErr handleHelp(const MojString& name, const MojString& val);
	MojErr handleLoggers(const MojString& name, const MojString& val);
	MojErr handleVersion(const MojString& name, const MojString& val);
	MojErr loadConfig(const MojChar* path);

	MojString m_name;
	OptHandlerMap m_optMap;
	MojObject m_conf;
	RunMode m_runMode;
	bool m_errDisplayed;
	MojUInt32 m_majorVersion;
	MojUInt32 m_minorVersion;
	const MojChar* m_versionString;
};

#endif /* MOJAPP_H_ */
