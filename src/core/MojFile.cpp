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


#include "core/MojFile.h"
#include "core/MojString.h"

MojErr MojFile::close()
{
	MojErr err = MojErrNone;
	if (m_file != MojInvalidFile) {
		MojErr errClose = MojFileClose(m_file);
		MojErrAccumulate(err, errClose);
		m_file = MojInvalidFile;
	}
	return err;
}

MojErr MojFile::readString(MojString& strOut)
{
	MojString str;
	MojChar buf[MojFileBufSize];
	gsize bytesRead = 0;
	do {
		MojErr err = read(buf, sizeof(buf), bytesRead);
		MojErrCheck(err);
		err = str.append(buf, bytesRead);
		MojErrCheck(err);
	} while (bytesRead > 0);
	str.swap(strOut);

	return MojErrNone;
}

MojErr MojFile::writeString(const MojChar* data, gsize& sizeOut)
{
	gsize len = MojStrLen(data) * sizeof(MojChar);
	const guint8* bytes = (const guint8*)data;
	while (len > 0) {
		gsize written = 0;
		MojErr err = write(bytes, len, written);
		MojErrCheck(err);
		len -= written;
		bytes += written;
		sizeOut += written;
	}

	return MojErrNone;
}

