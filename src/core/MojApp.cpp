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


#include "core/MojApp.h"
#include "core/MojLogDb8.h"

//this line is required for compatibility with mojomail-pop
MojLogger MojApp::s_log(_T("core.app"));

MojApp::MojApp(MojUInt32 majorVersion, MojUInt32 minorVersion, const MojChar* versionString)
: m_runMode(ModeDefault),
  m_errDisplayed(false),
  m_majorVersion(majorVersion),
  m_minorVersion(minorVersion),
  m_versionString(versionString)
{
}

MojApp::~MojApp()
{
}

int MojApp::main(int argc, MojChar** argv)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = init();
	if (err != MojErrNone) {
		(void) displayErr(err, _T("error initializing"));
	}
	MojErrCheck(err);
	err = handleCommandLine(argc, argv);
	if (err != MojErrNone) {
		m_runMode = ModeHelp;
		(void) displayErr(err, _T("error processing arguments"));
	}
	MojErrCatchAll(err);

	switch (m_runMode) {
	case ModeDefault: {
		err = configure(m_conf);
		if (err != MojErrNone) {
			(void) displayErr(err, _T("error configuring"));
		}
		MojErrCheck(err);
		err = open();
		if (err != MojErrNone) {
			(void) displayErr(err, _T("error opening"));
		}
		MojErrCheck(err);
		MojAutoCloser<MojApp> closer(this);
		err = run();
		MojErrCheck(err);
		closer.release();
		err = close();
		MojErrCheck(err);
		break;
	}
	case ModeHelp: {
		err = displayUsage();
		MojErrCheck(err);
		err = displayOptions();
		MojErrCheck(err);
		break;
	}
	case ModeLoggers: {
		break;
	}
	case ModeVersion: {
		err = displayVersion();
		MojErrCheck(err);
		break;
	}
	default:
		MojAssertNotReached();
	}
	return MojErrNone;
}

MojErr MojApp::configure(const MojObject& conf)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	//MojErr err = MojLogEngine::instance()->configure(conf, m_name);
	//MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::init()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	MojErr err = registerOption(&MojApp::handleConfig, _T("-c"), _T("Path to config file or configuration literal."), true);
	MojErrCheck(err);
	err = registerOption(&MojApp::handleHelp, _T("-h"), _T("Print this message and exit."));
	MojErrCheck(err);
	err = registerOption(&MojApp::handleLoggers, _T("-l"), _T("Print names of registered loggers and exit."));
	MojErrCheck(err);
	err = registerOption(&MojApp::handleVersion, _T("-v"), _T("Print version and exit."));
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::open()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return MojErrNone;
}

MojErr MojApp::close()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return MojErrNone;
}

MojErr MojApp::run()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

	return MojErrNone;
}

MojErr MojApp::registerOption(OptionHandler handler, const MojChar* opt, const MojChar* help, bool argRequired)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(handler && opt && help);

	MojString optStr;
	MojErr err = optStr.assign(opt);
	MojErrCheck(err);
	MojString helpStr;
	err = helpStr.assign(help);
	MojErrCheck(err);
	Option option(handler, helpStr, argRequired);
	err = m_optMap.put(optStr, option);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::displayMessage(const MojChar* format, ...)
{
	va_list args;
	va_start (args, format);
	MojErr err = MojVPrintF(format, args);
	va_end(args);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::displayErr(const MojChar* prefix, const MojChar* errText)
{
	if (!m_errDisplayed) {
		MojErr err = displayMessage(_T("%s: %s -- %s\n"), name().data(), prefix, errText);
		MojErrCheck(err);
		m_errDisplayed = true;
	}
	return MojErrNone;
}

MojErr MojApp::displayErr(MojErr errDisplay, const MojChar* message)
{
	MojString errStr;
	MojErr err = MojErrToString(errDisplay, errStr);
	MojErrCheck(err);
	err = displayErr(message, errStr);
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::displayUsage()
{
	MojErr err = displayMessage(_T("usage: %s [OPTIONS]\n"), name().data());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::displayOptions()
{
	MojString usage;
	MojErr err = usage.assign(_T("\nOptions:\n"));
	MojErrCheck(err);
	for (OptHandlerMap::ConstIterator i = m_optMap.begin();
	     i != m_optMap.end(); ++i) {
		MojErr err = usage.appendFormat(_T("  %-8s %s\n"), i.key().data(), i->m_help.data());
		MojErrCheck(err);
	}
	err = displayMessage(_T("%s"), usage.data());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::displayVersion()
{
	MojErr err = MojErrNone;
	MojString version;
	if (m_versionString){
		err = version.assign(m_versionString);
		MojErrCheck(err);
	} else {
		err = version.format(_T("%d.%d"), m_majorVersion, m_minorVersion);
		MojErrCheck(err);
	}
	err = displayMessage(_T("%s %s\nCopyright (c) 2009-2013 LG Electronics, Inc.\n"), name().data(), version.data());
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::handleCommandLine(int argc, MojChar** argv)
{
	// set name
	if (argc > 0) {
		MojErr err = m_name.assign(MojFileNameFromPath(argv[0]));
		MojErrCheck(err);
	}
	// handle opts and args
	StringVec args;
	for (int i = 1; i < argc; ++i) {
		MojString arg;
		MojErr err = arg.assign(argv[i]);
		MojErrCheck(err);
		if (arg.startsWith(_T("-"))) {
			// find handler
			OptHandlerMap::ConstIterator mapIter = m_optMap.find(arg);
			if (mapIter == m_optMap.end()) {
				(void) displayErr(_T("invalid option"), arg);
				MojErrThrow(MojErrInvalidArg);
			}
			// find value
			MojString val;
			int valIdx = i + 1;
			if (valIdx < argc && argv[valIdx][0] != _T('-')) {
				err = val.assign(argv[valIdx]);
				MojErrCheck(err);
				++i;
			} else if (mapIter->m_arg) {
				(void) displayErr(_T("option requires an argument"), arg);
				MojErrThrow(MojErrInvalidArg);
			}
			OptionHandler handler = mapIter->m_handler;
			err = (this->*handler)(arg, val);
			MojErrCheck(err);
		} else {
			err = args.push(arg);
			MojErrCheck(err);
		}
	}
	if (m_runMode == ModeDefault) {
		MojErr err = handleArgs(args);
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojApp::handleArgs(const StringVec& args)
{
	return MojErrNone;
}

MojErr MojApp::handleConfig(const MojString& opt, const MojString& val)
{
	MojErr err = loadConfig(val);
	if (err != MojErrNone) {
		(void) displayErr(err, _T("error loading conf file"));
	}
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojApp::handleHelp(const MojString& opt, const MojString& val)
{
	m_runMode = ModeHelp;
	return MojErrNone;
}

MojErr MojApp::handleLoggers(const MojString& opt, const MojString& val)
{
	m_runMode = ModeLoggers;
	return MojErrNone;
}

MojErr MojApp::handleVersion(const MojString& opt, const MojString& val)
{
	m_runMode = ModeVersion;
	return MojErrNone;
}

MojErr MojApp::loadConfig(const MojChar* arg)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
	MojAssert(arg);

	if (arg[0] == '{') {
		MojErr err = m_conf.fromJson(arg);
		MojErrCheck(err);
	} else {
		MojString confStr;
		MojErr err = MojFileToString(arg, confStr);
		MojErrCheck(err);
		err = m_conf.fromJson(confStr);
		MojErrCheck(err);
	}

	return MojErrNone;
}


