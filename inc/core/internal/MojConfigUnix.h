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


#ifndef MOJCONFIGUNIX_H_
#define MOJCONFIGUNIX_H_

// HEADERS
#define MOJ_HAVE_ASSERT_H
#define MOJ_HAVE_CTYPE_H
#define MOJ_HAVE_DIRENT_H
#define MOJ_HAVE_ERRNO_H
#define MOJ_HAVE_LIMITS_H
#define MOJ_HAVE_NETINET_IN_H
#define MOJ_HAVE_PTHREAD_H
#define MOJ_HAVE_SIGNAL_H
#define MOJ_HAVE_STDARG_H
#define MOJ_HAVE_STDDEF_H
#define MOJ_HAVE_STDIO_H
#define MOJ_HAVE_STDINT_H
#define MOJ_HAVE_STDLIB_H
#define MOJ_HAVE_STRING_H
#define MOJ_HAVE_SYSLOG_H
#define MOJ_HAVE_SYS_FILE_H
#define MOJ_HAVE_SYS_SELECT_H
#define MOJ_HAVE_SYS_SOCKET_H
#define MOJ_HAVE_SYS_STAT_H
#define MOJ_HAVE_SYS_TIME_H
#define MOJ_HAVE_SYS_TYPES_H
#define MOJ_HAVE_SYS_UN_H
#define MOJ_HAVE_UNISTD_H

// TYPES/FEATURES
#define MOJ_USE_DIR
#define MOJ_USE_DIRENT
#define MOJ_USE_FD_SET
#define MOJ_USE_INT_FILE
#define MOJ_USE_INT_SOCKET
#define MOJ_USE_INTPTR_T
#define MOJ_USE_LOCALTIME_R
#define MOJ_USE_LONG_LONG
#define MOJ_USE_MODE_T
#define MOJ_USE_PTHREADS
#define MOJ_USE_SIZE_T
#define MOJ_USE_SOCKLEN_T
#define MOJ_USE_STRUCT_IOVEC
#define MOJ_USE_STRUCT_RANDOM_DATA
#define MOJ_USE_STRUCT_SIGACTION
#define MOJ_USE_STRUCT_SOCKADDR
#define MOJ_USE_STRUCT_SOCKADDR_IN
#define MOJ_USE_STRUCT_SOCKADDR_UN
#define MOJ_USE_STRUCT_STAT
#define MOJ_USE_STRUCT_TIMESPEC
#define MOJ_USE_STRUCT_TIMEVAL
#define MOJ_USE_STRUCT_TM

// CONSTANTS
#define MOJ_USE_NAME_MAX
#define MOJ_USE_PATH_MAX
#define MOJ_USE_SLASH_SEPARATOR

// CHARACTER ENCODING
#define MOJ_ENCODING_UTF8

// FUNCTIONS
#define MOJ_NEED_DEBUGBREAK

#define MOJ_USE_ABORT
#define MOJ_USE_ASSERT
#define MOJ_USE_BZERO
#define MOJ_USE_CLOSEDIR
#define MOJ_USE_ERRNO
#define MOJ_USE_ISALNUM
#define MOJ_USE_ISDIGIT
#define MOJ_USE_ISSPACE
#define MOJ_USE_ISXDIGIT
#define MOJ_USE_FILE_OPEN
#define MOJ_USE_FILE_CLOSE
#define MOJ_USE_FILE_READ
#define MOJ_USE_FILE_WRITE
#define MOJ_USE_FLOCK
#define MOJ_USE_FREE
#define MOJ_USE_GETTIMEOFDAY
#define MOJ_USE_GLIB
#define MOJ_USE_MALLOC
#define MOJ_USE_MEMCHR
#define MOJ_USE_MEMCMP
#define MOJ_USE_MEMCPY
#define MOJ_USE_MEMMOVE
#define MOJ_USE_MKDIR
#define MOJ_USE_NANOSLEEP
#define MOJ_USE_OPENDIR
#define MOJ_USE_PIPE
#define MOJ_USE_RAND_R
#define MOJ_USE_REALLOC
#define MOJ_USE_RMDIR
#define MOJ_USE_SELECT
#define MOJ_USE_SIGACTION
#define MOJ_USE_SOCK_ACCEPT
#define MOJ_USE_SOCK_BIND
#define MOJ_USE_SOCK_CLOSE
#define MOJ_USE_SOCK_CONNECT
#define MOJ_USE_SOCK_FCNTL
#define MOJ_USE_SOCK_LISTEN
#define MOJ_USE_SOCK_RECV
#define MOJ_USE_SOCK_SEND
#define MOJ_USE_SOCK_SOCKET
#define MOJ_USE_STAT
#define MOJ_USE_STRCASECMP
#define MOJ_USE_STRCHR
#define MOJ_USE_STRCMP
#define MOJ_USE_STRCPY
#define MOJ_USE_STRLEN
#define MOJ_USE_STRNCASECMP
#define MOJ_USE_STRNCMP
#define MOJ_USE_STRNCPY
#define MOJ_USE_STRRCHR
#define MOJ_USE_STRSTR
#define MOJ_USE_STRTOD
#define MOJ_USE_STRTOLL
#define MOJ_USE_SYSLOG
#define MOJ_USE_TIME_T
#define MOJ_USE_TOLOWER
#define MOJ_USE_TOUPPER
#define MOJ_USE_UNLINK
#define MOJ_USE_SYMLINK
#define MOJ_USE_VPRINTF
#define MOJ_USE_VSNPRINTF

// ATTRIBUTES
#define MOJ_USE_FORMAT_ATTR

#endif /* MOJCONFIGUNIX_H_ */
