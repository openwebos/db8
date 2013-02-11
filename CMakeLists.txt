# @@@LICENSE
#
# Copyright (c) 2012-2013 Hewlett-Packard Development Company, L.P.
# Copyright (c) 2013 LG Electronics
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

cmake_minimum_required(VERSION 2.8.7)

project(db8 CXX)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC4)
webos_component(2 0 0)

include(FindPkgConfig)

# -- check for glib 2.0
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

# -- check for gthread 2.0
pkg_check_modules(GTHREAD2 REQUIRED gthread-2.0)
include_directories(${GTHREAD2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GTHREAD2_CFLAGS_OTHER})

# -- check for LS2 (internally depends on pmloglib)
pkg_check_modules(LS2 REQUIRED luna-service2)
include_directories(${LS2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LS2_CFLAGS_OTHER})

# -- check for ICU
find_library(ICU NAMES icuuc)
if(ICU STREQUAL "ICU-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU4C libaries. Please install.")
endif()

find_library(ICUI18N NAMES icui18n)
if(ICUI18N STREQUAL "ICUI18N-NOTFOUND")
	MESSAGE(FATAL_ERROR "Failed to find ICU-I18N libaries. Please install.")
endif()

# configure database backend.
include(${CMAKE_SOURCE_DIR}/src/db-luna/BackendBuildRules.cmake)

webos_build_system_bus_files()

# provide both db8 and mojodb pkgconfigs
webos_build_pkgconfig(files/pkgconfig/mojodb)
webos_build_pkgconfig(files/pkgconfig/db8)

webos_add_compiler_flags(ALL -g -O2 -Wall -fPIC -Wconversion -Werror
                         -Wno-unused-but-set-variable -Wno-unused-variable
                         -fno-exceptions -DNDEBUG -fexpensive-optimizations
                         -frename-registers -DMOJ_LINUX -fno-strict-aliasing
                         -DMOJ_INTERNAL -D_REENTRANT)
webos_add_linker_options(ALL --no-undefined)

# -- add local include paths
include_directories(inc)

# -- source for generating libmojocore.so
set(CORE_LIB_SOURCES
    src/core/MojApp.cpp
    src/core/MojBuffer.cpp
    src/core/MojDataSerialization.cpp
    src/core/MojDecimal.cpp
    src/core/MojEpollReactor.cpp
    src/core/MojErr.cpp
    src/core/MojFile.cpp
    src/core/MojGmainReactor.cpp
    src/core/MojHashBase.cpp
    src/core/MojJson.cpp
    src/core/MojList.cpp
    src/core/MojLog.cpp
    src/core/MojLogEngine.cpp
    src/core/MojMessageDispatcher.cpp
    src/core/MojObject.cpp
    src/core/MojObjectBuilder.cpp
    src/core/MojObjectFilter.cpp
    src/core/MojObjectSerialization.cpp
    src/core/MojOs.cpp
    src/core/MojPmLogAppender.cpp
    src/core/MojRbTreeBase.cpp
    src/core/MojRefCount.cpp
    src/core/MojSchema.cpp
    src/core/MojService.cpp
    src/core/MojServiceApp.cpp
    src/core/MojServiceMessage.cpp
    src/core/MojServiceRequest.cpp
    src/core/MojSignal.cpp
    src/core/MojSock.cpp
    src/core/MojString.cpp
    src/core/MojTestRunner.cpp
    src/core/MojThread.cpp
    src/core/MojTokenSet.cpp
    src/core/MojUtil.cpp
    )

add_library(mojocore SHARED ${CORE_LIB_SOURCES})
target_link_libraries(mojocore
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      ${LS2_LDFLAGS}
                      )
webos_build_library(TARGET mojocore NOHEADERS)

# -- source for generating libmojodb.so
set(DB_LIB_SOURCES
    src/db/MojDb.cpp
    src/db/MojDbAdmin.cpp
    src/db/MojDbClient.cpp
    src/db/MojDbCursor.cpp
    src/db/MojDbExtractor.cpp
    src/db/MojDbIdGenerator.cpp
    src/db/MojDbIndex.cpp
    src/db/MojDbIsamQuery.cpp
    src/db/MojDbKey.cpp
    src/db/MojDbKind.cpp
    src/db/MojDbKindEngine.cpp
    src/db/MojDbKindState.cpp
    src/db/MojDbObjectHeader.cpp
    src/db/MojDbObjectItem.cpp
    src/db/MojDbPermissionEngine.cpp
    src/db/MojDbPutHandler.cpp
    src/db/MojDbQuery.cpp
    src/db/MojDbQueryFilter.cpp
    src/db/MojDbQueryPlan.cpp
    src/db/MojDbQuotaEngine.cpp
    src/db/MojDbReq.cpp
    src/db/MojDbRevisionSet.cpp
    src/db/MojDbSearchCursor.cpp
    src/db/MojDbServiceClient.cpp
    src/db/MojDbServiceDefs.cpp
    src/db/MojDbServiceHandler.cpp
    src/db/MojDbServiceHandlerBase.cpp
    src/db/MojDbServiceSchemas.cpp
    src/db/MojDbStorageEngine.cpp
    src/db/MojDbTextCollator.cpp
    src/db/MojDbTextTokenizer.cpp
    src/db/MojDbTextUtils.cpp
    src/db/MojDbUtils.cpp
    src/db/MojDbWatcher.cpp
    src/db/MojDbServiceHandlerInternal.cpp
    )

add_library(mojodb SHARED ${DB_LIB_SOURCES})
target_link_libraries(mojodb
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      mojocore
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
                      )
webos_build_library(TARGET mojodb NOHEADERS)

# -- source for generating libmojoluna.so
set(LUNA_LIB_SOURCES
    src/luna/MojLunaErr.cpp
    src/luna/MojLunaMessage.cpp
    src/luna/MojLunaRequest.cpp
    src/luna/MojLunaService.cpp
    )

add_library(mojoluna SHARED ${LUNA_LIB_SOURCES})
target_link_libraries(mojoluna
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      mojocore
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
                      )
webos_build_library(TARGET mojoluna NOHEADERS)

# -- source for generating mojodb-luna
set(LUNA_BIN_SOURCES src/db-luna/MojDbLunaServiceApp.cpp)

include_directories (${DB_BACKEND_INCLUDES})
add_executable(mojodb-luna ${LUNA_BIN_SOURCES} ${DB_BACKEND_WRAPPER_SOURCES})
target_link_libraries(mojodb-luna
                      ${GLIB2_LDFLAGS}
                      ${GTHREAD2_LDFLAGS}
                      mojocore
                      mojodb
                      mojoluna
                      ${DB_BACKEND_LIB}
                      ${LS2_LDFLAGS}
                      ${ICU}
                      ${ICUI18N}
)

webos_add_compiler_flags(ALL ${DB_BACKEND_WRAPPER_CFLAGS})

webos_build_system_bus_files()

# specifying files/launch allows creating upstart scripts
# of all files named as <name>.in into upstart/name
# (see cmake_webos)
webos_build_daemon(NAME mojodb-luna LAUNCH files/launch/)

# extras
install(FILES src/db-luna/mojodb.conf DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR})
install(FILES src/db-luna/backup-com.palm.db DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/backup)
install(FILES src/db-luna/activity-com.palm.db.purge.json DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/activities/com.palm.db)
install(FILES src/db-luna/activity-com.palm.db.space.json DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/activities/com.palm.db)

install(DIRECTORY inc/ DESTINATION ${WEBOS_INSTALL_INCLUDEDIR}/mojodb/)

if (WEBOS_CONFIG_BUILD_TESTS)
	add_subdirectory(test/core)
	add_subdirectory(test/db)
	add_subdirectory(test/dbclient)
	add_subdirectory(test/luna-service)
endif()
