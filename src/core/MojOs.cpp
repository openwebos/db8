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


#include "core/MojCoreDefs.h"
#include "core/MojTime.h"

#ifdef MOJ_NEED_MEMRCHR
const MojChar* MojMemChrReverse(const MojChar* data, MojChar c, MojSize len)
{
	MojAssert(data || len == 0);
	if (data == NULL)
		return NULL;

	const MojChar* endPtr = data;
	const MojChar* p = endPtr + len - 1;
	while (p >= endPtr) {
		if (*p == c) {
			return p;
		}
		--p;
	}
	return NULL;
}
#endif /* MOJ_NEED_MEMRCHR */

#ifdef MOJ_USE_VPRINTF
MojErr MojPrintF(const MojChar* format, ...)
{
	MojAssert(format);

	va_list args;
	va_start (args, format);
	MojErr err = MojVPrintF(format, args);
	va_end(args);
	MojErrCheck(err);

	return MojErrNone;
}
#endif /* MOJ_USE_VPRINTF */

#ifdef MOJ_USE_VPRINTF
MojErr MojVPrintF(const MojChar* format, va_list args)
{
	MojAssert(format);

	int res = vprintf(format, args);
	if (res < 0)
		MojErrThrow(MojErrFormat);

	return MojErrNone;
}
#endif /* MOJ_USE_VPRINTF */

#ifdef MOJ_USE_VSNPRINTF
MojErr MojVsnPrintF(MojChar* buf, MojSize bufLen, MojSize& lenOut,
		const MojChar* format, va_list args)
{
	MojAssert(buf || bufLen == 0);
	MojAssert(format);

	lenOut = 0;
	int res = vsnprintf(buf, bufLen, format, args);
	if (res < 0) {
		if (buf)
			*buf = _T('\0');
		MojErrThrow(MojErrFormat);
	}
	lenOut = (MojSize) res;

	return MojErrNone;
}
#endif /* MOJ_USE_VSNPRINTF */

#ifdef MOJ_USE_CLOSEDIR
MojErr MojDirClose(MojDirT dir)
{
	MojAssert(dir);

	if (closedir(dir) < 0)
		MojErrThrowErrno(_T("closedir"));

	return MojErrNone;
}
#endif /* MOJ_USE_CLOSEDIR */

#ifdef MOJ_USE_OPENDIR
MojErr MojDirOpen(MojDirT& dirOut, const MojChar* path)
{
	MojAssert(path);

	dirOut = opendir(path);
	if (!dirOut)
		MojErrThrowErrno(_T("opendir"));

	return MojErrNone;
}
#endif /* MOJ_USE_OPENDIR */

#ifdef MOJ_USE_READDIR
MojErr MojDirRead(MojDirT dir, MojDirentT* entOut, bool& entReadOut)
{
	MojAssert(dir);

	entReadOut = false;
	MojDirentT* res = readdir(dir);
	if (res) {
		entReadOut = true;
		MojMemCpy(entOut, res, sizeof(*entOut));
	}

	return MojErrNone;
}
#endif /* MOJ_USE_READDIR */

#ifdef MOJ_USE_READDIR_R
MojErr MojDirRead(MojDirT dir, MojDirentT* entOut, bool& entReadOut)
{
	MojAssert(dir);

	entReadOut = false;
	// readdir_r directly returns errno val
	MojDirentT* res = NULL;
	MojErr err = static_cast<MojErr>(readdir_r(dir, entOut, &res));
	MojErrCheck(err);
	if (res)
		entReadOut = true;

	return MojErrNone;
}
#endif /* MOJ_USE_READDIR_R */

#ifdef MOJ_USE_FILE_CLOSE
MojErr MojFileClose(MojFileT file)
{
	MojAssert (file != MojInvalidFile);

	if (close(file) < 0)
		MojErrThrowErrno(_T("close"));

	return MojErrNone;
}
#endif /* MOJ_USE_FILE_CLOSE */

#ifdef MOJ_USE_FLOCK
MojErr MojFileLock(MojFileT file, int op)
{
	MojAssert (file != MojInvalidFile && op != 0);

	if (flock(file, op) < 0)
		MojErrThrowErrno(_T("flock"));

	return MojErrNone;
}
#endif /* MOJ_USE_FLOCK */

#ifdef MOJ_USE_FILE_OPEN
MojErr MojFileOpen(MojFileT& fileOut, const MojChar* path, int flags, MojModeT mode)
{
	MojAssert (path);
	MojAssert (!(flags & O_CREAT) || mode);

	fileOut = open(path, flags, mode);
	if (fileOut == MojInvalidFile)
		MojErrThrowErrno(_T("open"));

	return MojErrNone;
}
#endif /* MOJ_USE_FILE_OPEN */

#ifdef MOJ_USE_FILE_READ
MojErr MojFileRead(MojFileT file, void* buf, MojSize bufSize, MojSize& sizeOut)
{
	MojAssert(buf || bufSize == 0);
	MojAssert(file != MojInvalidFile);

	ssize_t res = read(file, buf, bufSize);
	if (res < 0)
		MojErrThrowErrno(_T("read"));
	sizeOut = (MojSize) res;

	return MojErrNone;
}
#endif /* MOJ_USE_FILE_READ */

#ifdef MOJ_USE_FILE_WRITE
MojErr MojFileWrite(MojFileT file, const void* data, MojSize size, MojSize& sizeOut)
{
	MojAssert(data || size == 0);
	MojAssert(file != MojInvalidFile);

	ssize_t res = write(file, data, size);
	if (res < 0)
		MojErrThrowErrno(_T("write"));
	sizeOut = (MojSize) res;

	return MojErrNone;
}
#endif /* MOJ_USE_FILE_WRITE */

#ifdef MOJ_USE_MKDIR
MojErr MojMkDir(const MojChar* path, MojModeT mode)
{
	MojAssert(path);

	if (mkdir(path, mode) != 0)
		MojErrThrowErrno(_T("mkdir"));

	return MojErrNone;
}
#endif /* MOJ_USE_MKDIR */

#ifdef MOJ_USE_RMDIR
MojErr MojRmDir(const MojChar* path)
{
	MojAssert(path);

	if (rmdir(path) != 0)
		MojErrThrowErrno(_T("rmdir"));

	return MojErrNone;
}
#endif /* MOJ_USE_RMDIR */

#ifdef MOJ_USE_STAT
MojErr MojStat(const MojChar* path, MojStatT* buf)
{
	MojAssert(path && buf);

	if (stat(path, buf) != 0)
		MojErrThrowErrno(_T("stat"));

	return MojErrNone;
}
#endif /* MOJ_USE_STAT */

#ifdef MOJ_USE_UNLINK
MojErr MojUnlink(const MojChar* path)
{
	MojAssert(path);

	if (unlink(path) != 0)
		MojErrThrowErrno(_T("unlink"));

	return MojErrNone;
}
#endif /* MOJ_USE_UNLINK */

#ifdef MOJ_USE_SOCK_ACCEPT
MojErr MojSockAccept(MojSockT listenSock, MojSockT& sockOut, MojSockAddrT* addrOut, MojSockLenT* addrSizeOut)
{
	MojAssert(listenSock != MojInvalidSock);

	sockOut = accept(listenSock, addrOut, addrSizeOut);
	if (sockOut < 0)
		MojErrThrowErrno(_T("accept"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_ACCEPT */

#ifdef MOJ_USE_SOCK_BIND
MojErr MojSockBind(MojSockT sock, const MojSockAddrT* addr, MojSockLenT addrSize)
{
	MojAssert(sock != MojInvalidSock);

	if (bind(sock, addr, addrSize) < 0)
		MojErrThrowErrno(_T("bind"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_BIND */

#ifdef MOJ_USE_SOCK_CLOSE
MojErr MojSockClose(MojSockT sock)
{
	MojAssert(sock != MojInvalidSock);

	if (close(sock) < 0)
		MojErrThrowErrno(_T("close"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_CLOSE */

#ifdef MOJ_USE_SOCK_CONNECT
MojErr MojSockConnect(MojSockT sock, const MojSockAddrT* addr, MojSockLenT addrSize)
{
	MojAssert(sock != MojInvalidSock && addr && addrSize);

	if (connect(sock, addr, addrSize) < 0)
		MojErrThrowErrno(_T("connect"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_FCNTL */

#ifdef MOJ_USE_SOCK_LISTEN
MojErr MojSockListen(MojSockT sock, int backlog)
{
	MojAssert(sock != MojInvalidSock);

	if (listen(sock, backlog) < 0)
		MojErrThrowErrno(_T("listen"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_LISTEN */

#ifdef MOJ_USE_SOCK_SOCKET
MojErr MojSockOpen(MojSockT& sockOut, int domain, int type, int protocol)
{
	sockOut = socket(domain, type, protocol);
	if (sockOut == MojInvalidSock)
		MojErrThrowErrno(_T("socket"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_SOCKET */

#ifdef MOJ_USE_SOCK_RECV
MojErr MojSockRecv(MojSockT sock, void* buf, MojSize bufSize, MojSize& sizeOut, int flags)
{
	MojAssert(sock != MojInvalidSock && (buf || bufSize == 0));

	sizeOut = 0;
	ssize_t ret = recv(sock, buf, bufSize, flags);
	if (ret < 0)
		MojErrThrowErrno(_T("recv"));
	sizeOut = (MojSize) ret;

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_RECV */

#ifdef MOJ_USE_SOCK_SEND
MojErr MojSockSend(MojSockT sock, const void* data, MojSize size, MojSize& sizeOut, int flags)
{
	MojAssert(sock != MojInvalidSock && (data || size == 0));

	sizeOut = 0;
	ssize_t ret = send(sock, data, size, flags);
	if (ret < 0)
		MojErrThrowErrno(_T("send"));
	sizeOut = (MojSize) ret;

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_SEND */

#ifdef MOJ_USE_SOCK_FCNTL
MojErr MojSockSetNonblocking(MojSockT sock, bool val)
{
	MojAssert(sock != MojInvalidSock);

	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)
		MojErrThrowErrno(_T("fcntl"));
	if (val)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(sock, F_SETFL, flags) < 0)
		MojErrThrowErrno(_T("fcntl"));

	return MojErrNone;
}
#endif /* MOJ_USE_SOCK_FCNTL */

#ifdef MOJ_USE_PIPE
MojErr MojPipe(MojSockT fds[2])
{
	if (pipe(fds) < 0)
		MojErrThrowErrno(_T("pipe"));

	return MojErrNone;
}
#endif /* MOJ_USE_PIPE */

#ifdef MOJ_USE_SRANDOM_R
MojErr MojInitRandom(MojUInt32 seed, MojChar* stateBuf, MojSize stateBufLen, MojRandomDataT* buf)
{
	if (initstate_r(seed, stateBuf, stateBufLen, buf) < 0)
		MojErrThrowErrno(_T("initstate_r"));

	return MojErrNone;
}
#endif /* MOJ_USE_SRANDOM_R */

#ifdef MOJ_USE_SRANDOM
MojErr MojInitRandom(MojUInt32 seed, MojChar*, MojSize, MojRandomDataT*)
{
	srandom(seed);

	return MojErrNone;
}
#endif /* MOJ_USE_SRANDOM_R */

#ifdef MOJ_USE_RANDOM_R
MojErr MojRandom(MojRandomDataT* buf, MojUInt32* result)
{
	if (random_r(buf, (MojInt32*) result) < 0)
		MojErrThrowErrno(_T("random_r"));

	return MojErrNone;
}
#endif /* MOJ_USE_SRANDOM_R */

#ifdef MOJ_USE_RANDOM
MojErr MojRandom(MojRandomDataT*, MojUInt32* result)
{
	*result = random();

	return MojErrNone;
}
#endif /* MOJ_USE_SRANDOM_R */

#ifdef MOJ_USE_SELECT
MojErr MojSelect(int& nfdsOut, int nfds, MojFdSetT* readfds, MojFdSetT* writefds,
		MojFdSetT* exceptfds, const MojTime& timeout)
{
	MojTimevalT tv;
	timeout.toTimeval(&tv);
	int ret = select(nfds, readfds, writefds, exceptfds, &tv);
	if (ret < 0) {
		nfdsOut = 0;
		MojErrThrowErrno(_T("select"));
	}
	nfdsOut = ret;

	return MojErrNone;
}
#endif /* MOJ_USE_SELECT */

#ifdef MOJ_USE_SIGACTION
MojErr MojSigAction(int signum, const MojSigactionT* action, MojSigactionT* oldAction)
{
	if (sigaction(signum, action, oldAction) != 0)
		MojErrThrowErrno(_T("sigaction"));

	return MojErrNone;
}
#endif /* MOJ_USE_SIGACTION */

#ifdef MOJ_USE_NANOSLEEP
MojErr MojSleep(const MojTime& time)
{
	MojTimespecT ts;
	time.toTimespec(&ts);
	if (nanosleep(&ts, NULL) < 0)
		MojErrThrowErrno(_T("nanosleep"));

	return MojErrNone;
}
#endif /* MOJ_USE_NANOSLEEP */

#ifdef MOJ_USE_GETTIMEOFDAY
MojErr MojGetCurrentTime(MojTime& timeOut)
{
	MojTimevalT tv;
	int ret = gettimeofday(&tv, NULL);
	if(ret < 0)
		MojErrThrowErrno(_T("gettimeofday"));
	timeOut.fromTimeval(&tv);

	return MojErrNone;
}
#endif /* MOJ_USE_GETTIMEOFDAY */

#ifdef MOJ_USE_LOCALTIME_R
MojErr MojLocalTime(const MojTime& time, MojTmT& tmOut)
{
	time_t timet = (time_t) time.secs();
	if (localtime_r(&timet, &tmOut) == NULL)
		MojErrThrowErrno(_T("localtime_r"));

	return MojErrNone;
}
#endif /* MOJ_USE_LOCALTIME_R */

#ifdef MOJ_USE_PTHREADS
struct MojThreadParams
{
	MojThreadParams(MojThreadFn fn, void* arg) : m_fn(fn), m_arg(arg) {}
	MojThreadFn m_fn;
	void* m_arg;
};

static void* MojThreadStart(void* arg)
{
	MojAssert(arg);
	MojThreadParams* params = (MojThreadParams*) arg;
	MojThreadFn fn = params->m_fn;
	void* threadArg = params->m_arg;
	delete params;

	MojErr err = fn(threadArg);
	return (void*) (MojIntPtr) err;
}

MojErr MojThreadCreate(MojThreadT& threadOut, MojThreadFn fn, void* arg)
{
	MojThreadParams* params = new MojThreadParams(fn, arg);
	MojAllocCheck(params);
	return (MojErr) pthread_create(&threadOut, NULL, MojThreadStart, params);
}

MojErr MojThreadJoin(MojThreadT thread, MojErr& errOut)
{
	void* retVal = NULL;
	errOut = MojErrNone;
	MojErr err = (MojErr) pthread_join(thread, &retVal);
	MojErrCheck(err);
	errOut = (MojErr) (MojIntPtr) retVal;

	return MojErrNone;
}
#endif /* MOJ_USE_PTHREADS */
