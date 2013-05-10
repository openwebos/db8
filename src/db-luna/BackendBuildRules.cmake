# @@@LICENSE
#
# Copyright (c) 2013 Hewlett-Packard Development Company, L.P.
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

# This CMakeLists.txt contains build instructions for database backend.
# This script MUST provide such variables:
# DB_BACKEND_INCLUDES           - includes for database headers (like leveldb/db.h)
# DB_BACKEND_LIB                - requre libraries for database (like -lleveldb)
# DB_BACKEND_WRAPPER_SOURCES    - wrapper cpp files inside this directory (like MojDbBerkeleyEngine.cpp)
# Optional variable:
# DB_BACKEND_WRAPPER_CFLAGS     - compiller flags

set(WEBOS_DB8_BACKEND "berkeleydb" CACHE STRING "Backend to use with DB8")

message (STATUS "Use database frontend: ${WEBOS_DB8_BACKEND}")

if (WEBOS_DB8_BACKEND STREQUAL "berkeleydb")
    # -- check for BerkeleyDB
    # (add an alternate standard path in case building BDB locally: does not override)
    find_library(BDB NAMES db-4.8 PATH /usr/local/BerkeleyDB.4.8/lib)
    if(BDB STREQUAL "BDB-NOTFOUND")
        MESSAGE(FATAL_ERROR "Failed to find BerkleyDB libaries. Please install.")
    endif()

    find_path(BDB_INC db.h
              PATHS /usr/local/BerkeleyDB.4.8/include
              PATH_SUFFIXES db4.8)
    if(BDB_INC STREQUAL "BDB_INC-NOTFOUND")
        MESSAGE(FATAL_ERROR "Failed to find BerkleyDB includes. Please install.")
    endif()

    set (DB_BACKEND_INCLUDES "${BDB_INC}")
    set (DB_BACKEND_LIB "${BDB}")

    set(DB_BACKEND_WRAPPER_SOURCES
        src/db-luna/MojDbBerkeleyEngine.cpp
        src/db-luna/MojDbBerkeleyFactory.cpp
        src/db-luna/MojDbBerkeleyQuery.cpp
    )

    set (DB_BACKEND_WRAPPER_CFLAGS "-DMOJ_USE_BDB")
elseif (WEBOS_DB8_BACKEND STREQUAL "leveldb")

    # -- check for LevelDB backend
    find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
    if(LDB STREQUAL "LDB-NOTFOUND")
        MESSAGE(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
    endif()

    set (DB_BACKEND_INCLUDES ${WEBOS_INSTALL_ROOT}/../leveldb/leveldb-1.9.0/include)
    set (DB_BACKEND_LIB "${LDB}")

    set(DB_BACKEND_WRAPPER_SOURCES
        src/db-luna/MojDbLevelEngine.cpp
        src/db-luna/MojDbLevelFactory.cpp
        src/db-luna/MojDbLevelQuery.cpp
        src/db-luna/MojDbLevelTxn.cpp
        src/db-luna/MojDbLevelSeq.cpp
   )

    set (DB_BACKEND_WRAPPER_CFLAGS "-DMOJ_USE_LDB")
else () 
    message(FATAL_ERROR "WEBOS_DB8_BACKEND: unsuported value '${WEBOS_DB8_BACKEND}'")
endif ()

# -- check for errors
if(NOT DEFINED DB_BACKEND_LIB)
        message(FATAL_ERROR "Not set DB_BACKEND_LIB in ${CMAKE_SOURCE_DIR}/src/db-luna/CMakeLists.txt")
endif()
if (NOT DEFINED DB_BACKEND_INCLUDES)
        message (FATAL_ERROR "Not set DB_BACKEND_INCLUDES in ${CMAKE_SOURCE_DIR}/src/db-luna/CMakeLists.txt")
endif()
if (NOT DEFINED DB_BACKEND_WRAPPER_SOURCES)
        message (FATAL_ERROR "Not set DB_BACKEND_WRAPPER_SOURCES in ${CMAKE_SOURCE_DIR}/src/db-luna/CMakeLists.txt")
endif()
if (NOT DEFINED DB_BACKEND_WRAPPER_CFLAGS)
        message (FATAL_ERROR "Not set DB_BACKEND_WRAPPER_CFLAGS in ${CMAKE_SOURCE_DIR}/src/db-luna/CMakeLists.txt")
endif()
