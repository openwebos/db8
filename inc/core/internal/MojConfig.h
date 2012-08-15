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


#ifndef MOJCONFIG_H_
#define MOJCONFIG_H_

// arch
#if !(defined(MOJ_ARM) || defined(MOJ_X86))
#	if (__arm__)
#		define MOJ_ARM
#	elif (__i386__ || __x86_64__)
#		define MOJ_X86
#	endif
#endif

#if (defined(MOJ_X86) && defined(MOJ_ARM))
#   error Invalid arch!
#endif

// byte order
#if (defined(MOJ_X86) || defined(MOJ_ARM))
#	define MOJ_LITTLE_ENDIAN
#endif

// memory alignment
#ifdef MOJ_X86
#	define MOJ_UNALIGNED_MEM_ACCESS
#endif

#ifdef MOJ_LINUX
#	include "core/internal/MojConfigLinux.h"
#elif MOJ_MAC
#	include "core/internal/MojConfigMac.h"
#elif MOJ_WIN
#	include "core/internal/MojConfigWin.h"
#endif

#endif /* MOJCONFIG_H_ */
