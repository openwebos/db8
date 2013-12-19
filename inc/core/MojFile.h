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


#ifndef MOJFILE_H_
#define MOJFILE_H_

#include "core/MojCoreDefs.h"

class MojFile {
public:
	static const MojSize MojFileBufSize = 2048;

	MojFile(MojFileT file = MojInvalidFile) : m_file(file) {}
	~MojFile() { (void) close(); }

	MojErr close();
	MojErr lock(int op) { return MojFileLock(m_file, op); }
	bool open() { return m_file != MojInvalidFile; }
	MojErr open(const MojChar* path, int flags, MojModeT mode = 0) { MojAssert(m_file == MojInvalidFile); return MojFileOpen(m_file, path, flags, mode); }
	MojErr readString(MojString& strOut);
	MojErr read(void* buf, MojSize bufSize, MojSize& sizeOut) { return MojFileRead(m_file, buf, bufSize, sizeOut); }
	MojErr writeString(const MojChar* data, MojSize& sizeOut);
	MojErr write(const void* data, MojSize size, MojSize& sizeOut) { return MojFileWrite(m_file, data, size, sizeOut); }
    MojErr sync() {return MojFileSync(m_file);}

private:
	MojFileT m_file;
};

#endif /* MOJFILE_H_ */
