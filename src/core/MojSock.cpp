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


#include "core/MojSock.h"

MojSockAddr::MojSockAddr()
: m_size(0)
{
	MojZero(&m_u, sizeof(m_u));
}

void MojSockAddr::fromAddr(MojSockAddrT* addr, MojSockLenT size)
{
	MojAssert(size <= sizeof(m_u));
	m_size = size;
	MojMemCpy(&m_u, addr, size);
}

MojErr MojSockAddr::fromPath(const MojChar* path)
{
	MojAssert(path);

	MojSockAddrUnT addr;
	MojZero(&addr, sizeof(addr));

	if (MojStrLen(path) > (sizeof(addr.sun_path) - 1))
		MojErrThrow(MojErrPathTooLong);
	MojStrCpy(addr.sun_path, path);

	addr.sun_family = MOJ_PF_LOCAL;
	fromAddr((MojSockAddrT*) &addr, sizeof(addr));

	return MojErrNone;
}

MojErr MojSockAddr::fromHostPort(const MojChar* host, MojUInt32 port)
{
	MojAssert(host);

	return MojErrNone;
}

MojErr MojSock::close()
{
	MojErr err = MojErrNone;
	if (m_sock != MojInvalidSock) {
		MojErr errClose = MojSockClose(m_sock);
		MojErrAccumulate(err, errClose);
		m_sock = MojInvalidSock;
	}
	return err;
}
