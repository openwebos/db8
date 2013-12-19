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


#ifndef MOJOS_H_
#define MOJOS_H_

#ifdef MOJ_HAVE_CTYPE_H
#	include <ctype.h>
#endif

#ifdef MOJ_HAVE_DIRENT_H
#	include <dirent.h>
#endif

#ifdef MOJ_HAVE_NETINET_IN_H
#	include <netinet/in.h>
#endif

#ifdef MOJ_HAVE_PTHREAD_H
#	include <pthread.h>
#endif

#ifdef MOJ_HAVE_SIGNAL_H
#	include <signal.h>
#endif

#ifdef MOJ_HAVE_STDIO_H
#	include <stdio.h>
#endif

#ifdef MOJ_HAVE_STDLIB_H
#	include <stdlib.h>
#endif

#ifdef MOJ_HAVE_STRING_H
#	include <string.h>
#endif

#ifdef MOJ_HAVE_SYS_FILE_H
#	include <sys/file.h>
#endif

#ifdef MOJ_HAVE_SYS_SELECT_H
#	include <sys/select.h>
#endif

#ifdef MOJ_HAVE_SYS_SOCKET_H
#	include <sys/socket.h>
#endif

#ifdef MOJ_HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif

#ifdef MOJ_HAVE_SYS_TIME_H
#	include <sys/time.h>
#endif

#ifdef MOJ_HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif

#ifdef MOJ_HAVE_SYS_UN_H
#	include <sys/un.h>
#endif

#ifdef MOJ_HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef MOJ_USE_DIR
	typedef DIR* MojDirT;
	const MojDirT MojInvalidDir = NULL;
#endif

#ifdef MOJ_USE_DIRENT
	typedef struct dirent MojDirentT;
#endif

#ifdef MOJ_USE_FD_SET
	typedef fd_set MojFdSetT;
#endif

#ifdef MOJ_USE_INT_FILE
	typedef int MojFileT;
	const MojFileT MojInvalidFile = -1;
#endif

#ifdef MOJ_USE_INT_SOCKET
	typedef int MojSockT;
	const MojSockT MojInvalidSock = -1;
#endif

#ifdef MOJ_USE_SOCKLEN_T
	typedef socklen_t MojSockLenT;
#endif

#ifdef MOJ_USE_MODE_T
	typedef mode_t MojModeT;
#endif

#ifdef MOJ_USE_SLASH_SEPARATOR
	const MojChar MojPathSeparator = _T('/');
#endif

#ifdef MOJ_USE_STRUCT_IOVEC
	typedef struct iovec MojIoVecT;
#endif

#ifdef MOJ_USE_STRUCT_SIGACTION
	typedef struct sigaction MojSigactionT;
#endif

#ifdef MOJ_USE_STRUCT_SOCKADDR
	typedef struct sockaddr MojSockAddrT;
#endif

#ifdef MOJ_USE_STRUCT_SOCKADDR_IN
	typedef struct sockaddr_in MojSockAddrInT;
#endif

#ifdef MOJ_USE_STRUCT_SOCKADDR_UN
	typedef struct sockaddr_un MojSockAddrUnT;
#endif

#ifdef MOJ_USE_STRUCT_RANDOM_DATA
#ifdef MOJ_USE_SRANDOM_R
	typedef struct random_data MojRandomDataT;
#else
	typedef struct { int padding;} MojRandomDataT;
#endif
#endif

#ifdef MOJ_USE_STRUCT_STAT
	typedef struct stat MojStatT;
#endif

#ifdef MOJ_USE_STRUCT_TIMESPEC
	typedef struct timespec MojTimespecT;
#endif

#ifdef MOJ_USE_STRUCT_TIMEVAL
	typedef struct timeval MojTimevalT;
#endif

#ifdef MOJ_USE_STRUCT_TM
	typedef struct tm MojTmT;
#endif

#ifdef MOJ_USE_TIME_T
    typedef time_t MojTimeT;
#endif

#if defined(SIG_IGN) && !defined(MOJ_SIG_IGN)
#	define MOJ_SIG_IGN	SIG_IGN
#endif
#if defined(SIGINT) && !defined(MOJ_SIGINT)
#	define MOJ_SIGINT SIGINT
#endif
#if defined(SIGPIPE) && !defined(MOJ_SIGPIPE)
#	define MOJ_SIGPIPE SIGPIPE
#endif
#if defined(SIGTERM) && !defined(MOJ_SIGTERM)
#	define MOJ_SIGTERM SIGTERM
#endif

// file open flags
#if defined(O_APPEND) && !defined(MOJ_O_APPEND)
#	define MOJ_O_APPEND O_APPEND
#endif
#if defined(O_CREAT) && !defined(MOJ_O_CREAT)
#	define MOJ_O_CREAT O_CREAT
#endif
#if defined(O_RDONLY) && !defined(MOJ_O_RDONLY)
#	define MOJ_O_RDONLY O_RDONLY
#endif
#if defined(O_RDWR) && !defined(MOJ_O_RDWR)
#	define MOJ_O_RDWR O_RDWR
#endif
#if defined(O_RDRW) && !defined(MOJ_O_RDRW)
#	define MOJ_O_RDRW O_RDRW
#endif
#if defined(O_WRONLY) && !defined(MOJ_O_WRONLY)
#	define MOJ_O_WRONLY O_WRONLY
#endif
#if defined(O_TRUNC) && !defined(MOJ_O_TRUNC)
#	define MOJ_O_TRUNC O_TRUNC
#endif

// file modes
#if defined(S_IRUSR) && !defined(MOJ_S_IRUSR)
#	define MOJ_S_IRUSR S_IRUSR
#endif
#if defined(S_IWUSR) && !defined(MOJ_S_IWUSR)
#	define MOJ_S_IWUSR S_IWUSR
#endif
#if defined(S_IXUSR) && !defined(MOJ_S_IXUSR)
#	define MOJ_S_IXUSR S_IXUSR
#endif
#if defined(S_IRWXU) && !defined(MOJ_S_IRWXU)
#	define MOJ_S_IRWXU S_IRWXU
#endif

// special files
#if defined(STDOUT_FILENO) && !defined(MojStdOutFile)
#	define MojStdOutFile	STDOUT_FILENO
#endif
#if defined(STDERR_FILENO) && !defined(MojStdErrFile)
#	define MojStdErrFile	STDERR_FILENO
#endif

// protocol families
#if defined(PF_LOCAL) && !defined(MOJ_PF_LOCAL)
#	define MOJ_PF_LOCAL PF_LOCAL
#endif
#if defined(PF_INET) && !defined(MOJ_PF_INET)
#	define MOJ_PF_INET PF_INET
#endif

// protocols
#if defined(SOCK_STREAM) && !defined(MOJ_SOCK_STREAM)
#	define MOJ_SOCK_STREAM SOCK_STREAM
#endif

#ifdef MOJ_USE_PTHREADS
	typedef pthread_t MojThreadT;
	typedef pthread_t MojThreadIdT;
	typedef pthread_mutex_t MojThreadMutexT;
	typedef pthread_cond_t MojThreadCondT;
	typedef pthread_rwlock_t MojThreadRwLockT;
	typedef pthread_key_t MojThreadKeyT;
	static const MojThreadT MojInvalidThread = (MojThreadT)-1;
	static const MojThreadIdT MojInvalidThreadId = (MojThreadIdT)-1;
	static const MojThreadKeyT MojInvalidThreadKey = (MojThreadKeyT) -1;
#endif

typedef struct { MojInt32 val; } MojAtomicT;
typedef MojErr (*MojThreadFn)(void* args);
typedef void (MojDestructorFn)(void* val);

void MojAtomicInit(MojAtomicT* a, MojInt32 i);
void MojAtomicDestroy(MojAtomicT* a);
MojInt32 MojAtomicGet(const MojAtomicT* a);
void MojAtomicSet(MojAtomicT* a, MojInt32 i);
MojInt32 MojAtomicIncrement(MojAtomicT* a);
MojInt32 MojAtomicDecrement(MojAtomicT* a);

MojErr MojErrno();
const MojChar* MojStrError(int err, MojChar* buf, MojSize bufLen);
void MojDebugBreak();

void MojAbort();
void MojFree(void* p);
void* MojMalloc(MojSize size);
void* MojRealloc(void* p, MojSize size);
int MojRand(unsigned int* seed);

void MojZero(void* dest, MojSize size);
const MojChar* MojMemChr(const MojChar* data, MojChar c, MojSize len);
const MojChar* MojMemChrReverse(const MojChar* data, MojChar c, MojSize len);
int MojMemCmp(const void* p1, const void* p2, MojSize size);
void* MojMemCpy(void* dest, const void* src, MojSize size);
void* MojMemMove(void* dest, const void* src, MojSize size);

const MojChar* MojStrChr(const MojChar* str, MojChar c);
const MojChar* MojStrChrReverse(const MojChar* str, MojChar c);
const MojChar* MojStrStr(const MojChar* haystack, const MojChar* needle);
int MojStrCaseCmp(const MojChar* str1, const MojChar* str2);
int MojStrNCaseCmp(const MojChar* str1, const MojChar* str2);
int MojStrCmp(const MojChar* str1, const MojChar* str2);
int MojStrNCmp(const MojChar* str1, const MojChar* str2, MojSize len);
MojChar* MojStrCpy(MojChar* strDest, const MojChar* strSrc);
MojChar* MojStrNCpy(MojChar* strDest, const MojChar* strSrc, MojSize n);
MojSize MojStrLen(const MojChar* str);
MojDouble MojStrToDouble(const MojChar* str, const MojChar** end);
MojInt64 MojStrToInt64(const MojChar* str, const MojChar** end, int base);

bool MojIsAlNum(MojChar c);
bool MojIsSpace(MojChar c);
bool MojIsDigit(MojChar c);
bool MojIsHexDigit(MojChar c);
MojChar MojToLower(MojChar c);
MojChar MojToUpper(MojChar c);

MojErr MojPrintF(const MojChar* format, ...) MOJ_FORMAT_ATTR((printf, 1, 2));
MojErr MojVPrintF(const MojChar* format, va_list args) MOJ_FORMAT_ATTR((printf, 1, 0));
MojErr MojVsnPrintF(MojChar* buf, MojSize bufLen, MojSize& lenOut,
		const MojChar* format, va_list args)  MOJ_FORMAT_ATTR((printf, 4, 0));

MojErr MojDirClose(MojDirT dir);
MojErr MojDirOpen(MojDirT& dirOut, const MojChar* path);
MojErr MojDirRead(MojDirT dir, MojDirentT* entOut, bool& entReadOut);

MojErr MojFileClose(MojFileT file);
MojErr MojFileLock(MojFileT file, int op);
MojErr MojFileOpen(MojFileT& fileOut, const MojChar* path, int flags, MojModeT mode = 0);
MojErr MojFileRead(MojFileT file, void* buf, MojSize bufSize, MojSize& sizeOut);
MojErr MojFileWrite(MojFileT file, const void* data, MojSize size, MojSize& sizeOut);
MojErr MojFileSync(MojFileT file);
MojErr MojFileRename(const MojChar* oldName, const MojChar* newName);

const MojChar* MojMkTemp(MojChar* name);
MojErr MojMkDir(const MojChar* path, MojModeT mode);
MojErr MojRmDir(const MojChar* path);
MojErr MojStat(const MojChar* path, MojStatT* buf);
MojErr MojUnlink(const MojChar* path);
MojErr MojSymlink(const MojChar* src, const MojChar* dst);

MojErr MojSockAccept(MojSockT listenSock, MojSockT& sockOut, MojSockAddrT* addrOut = NULL, MojSockLenT* addrSizeOut = NULL);
MojErr MojSockBind(MojSockT sock, const MojSockAddrT* addr, MojSockLenT addrSize);
MojErr MojSockClose(MojSockT sock);
MojErr MojSockConnect(MojSockT sock, const MojSockAddrT* addr, MojSockLenT addrSize);
MojErr MojSockListen(MojSockT sock, int backlog);
MojErr MojSockOpen(MojSockT& sockOut, int domain, int type, int protocol = 0);
MojErr MojSockRecv(MojSockT sock, void* buf, MojSize bufSize, MojSize& sizeOut, int flags = 0);
MojErr MojSockSend(MojSockT sock, const void* data, MojSize size, MojSize& sizeOut, int flags = 0);
MojErr MojSockSetNonblocking(MojSockT sock, bool val);

MojErr MojPipe(MojSockT fds[2]);
MojErr MojInitRandom(MojUInt32 seed, MojChar* stateBuf, MojSize stateBufLen, MojRandomDataT* buf);
MojErr MojRandom(MojRandomDataT* buf, MojUInt32* result);
MojErr MojSelect(int& nfdsOut, int nfds, MojFdSetT* readfds, MojFdSetT* writefds,
		MojFdSetT* exceptfds, const MojTime& timeout);
MojErr MojSigAction(int signum, const MojSigactionT* action, MojSigactionT* oldAction);
MojErr MojSleep(const MojTime& time);

MojUInt16 MojUInt16ToBigEndian(MojUInt16 val);
MojUInt32 MojUInt32ToBigEndian(MojUInt32 val);
MojInt64 MojInt64ToBigEndian(MojInt64 val);

MojErr MojGetCurrentTime(MojTime& timeOut);
MojErr MojLocalTime(const MojTime& time, MojTmT& tmOut);

MojErr MojThreadCreate(MojThreadT& threadOut, MojThreadFn fn, void* arg);
MojErr MojThreadJoin(MojThreadT thread, MojErr& errOut);
MojErr MojThreadDetach(MojThreadT thread);
MojErr MojThreadYield();
MojThreadT MojThreadCurrent();
MojThreadIdT MojThreadCurrentId();

MojErr MojThreadMutexInit(MojThreadMutexT* mutex);
MojErr MojThreadMutexDestroy(MojThreadMutexT* mutex);
MojErr MojThreadMutexLock(MojThreadMutexT* mutex);
MojErr MojThreadMutexTryLock(MojThreadMutexT* mutex);
MojErr MojThreadMutexUnlock(MojThreadMutexT* mutex);

MojErr MojThreadCondInit(MojThreadCondT* cond);
MojErr MojThreadCondDestroy(MojThreadCondT* cond);
MojErr MojThreadCondSignal(MojThreadCondT* cond);
MojErr MojThreadCondBroadcast(MojThreadCondT* cond);
MojErr MojThreadCondWait(MojThreadCondT* cond, MojThreadMutexT* mutex);

MojErr MojThreadRwLockInit(MojThreadRwLockT* lock);
MojErr MojThreadRwLockDestroy(MojThreadRwLockT* lock);
MojErr MojThreadRwLockReadLock(MojThreadRwLockT* lock);
MojErr MojThreadRwLockTryReadLock(MojThreadRwLockT* lock);
MojErr MojThreadRwLockWriteLock(MojThreadRwLockT* lock);
MojErr MojThreadRwLockTryWriteLock(MojThreadRwLockT* lock);
MojErr MojThreadRwLockUnlock(MojThreadRwLockT* lock);

MojErr MojThreadKeyCreate(MojThreadKeyT& keyOut, MojDestructorFn destructor = NULL);
MojErr MojThreadKeyDelete(MojThreadKeyT key);
MojErr MojThreadSetSpecific(MojThreadKeyT key, void* val);
void* MojThreadGetSpecific(MojThreadKeyT key);

#include "core/internal/MojOsInternal.h"

#endif /* MOJOS_H_ */
