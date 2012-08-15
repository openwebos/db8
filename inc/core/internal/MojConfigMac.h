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


#ifndef MOJCONFIGMAC_H_
#define MOJCONFIGMAC_H_

#define MOJ_USE_READDIR_R
#define MOJ_USE_POSIX_STRERROR_R
#define MOJ_USE_RANDOM
#define MOJ_USE_SRANDOM

#define MOJ_NEED_ATOMIC_INIT
#define MOJ_NEED_ATOMIC_DESTROY
#define MOJ_NEED_ATOMIC_GET
#define MOJ_NEED_ATOMIC_SET

#define MOJ_NEED_MEMRCHR

#include "core/internal/MojConfigUnix.h"

#endif /* MOJCONFIGMAC_H_ */
