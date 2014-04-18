# @@@LICENSE
#
# Copyright (c) 2013-2014 LG Electronics, Inc.
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

set(WEBOS_DB8_BACKEND "leveldb" CACHE STRING "Backend(s) to use with DB8")

foreach (backend ${WEBOS_DB8_BACKEND})
	message (STATUS "Use database frontend: ${backend}")

	if (backend STREQUAL "berkeleydb")
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

		set (DB_BACKEND_INCLUDES ${DB_BACKEND_INCLUDES} ${BDB_INC})
		set (DB_BACKEND_LIB ${DB_BACKEND_LIB} ${BDB})

		set(DB_BACKEND_WRAPPER_SOURCES
			${DB_BACKEND_WRAPPER_SOURCES}
			src/db-luna/MojDbBerkeleyEngine.cpp
			src/db-luna/MojDbBerkeleyFactory.cpp
			src/db-luna/MojDbBerkeleyQuery.cpp
		)

		set (DB_BACKEND_WRAPPER_CFLAGS "${DB_BACKEND_WRAPPER_CFLAGS} -DMOJ_USE_BDB")
	elseif (backend STREQUAL "leveldb")

		# -- check for LevelDB backend
		find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
		if(LDB STREQUAL "LDB-NOTFOUND")
			MESSAGE(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
		endif()

		set (DB_BACKEND_INCLUDES ${DB_BACKEND_INCLUDES} ${WEBOS_INSTALL_ROOT}/include)
		set (DB_BACKEND_LIB ${DB_BACKEND_LIB} ${LDB})

		set(DB_BACKEND_WRAPPER_SOURCES
			${DB_BACKEND_WRAPPER_SOURCES}
			src/db-luna/leveldb/defs.cpp
			src/db-luna/leveldb/MojDbLevelEngine.cpp
			src/db-luna/leveldb/MojDbLevelFactory.cpp
			src/db-luna/leveldb/MojDbLevelDatabase.cpp
			src/db-luna/leveldb/MojDbLevelQuery.cpp
			src/db-luna/leveldb/MojDbLevelTxn.cpp
			src/db-luna/leveldb/MojDbLevelSeq.cpp
			src/db-luna/leveldb/MojDbLevelCursor.cpp
			src/db-luna/leveldb/MojDbLevelEnv.cpp
			src/db-luna/leveldb/MojDbLevelIndex.cpp
			src/db-luna/leveldb/MojDbLevelItem.cpp
			src/db-luna/leveldb/MojDbLevelTxnIterator.cpp
			src/db-luna/leveldb/MojDbLevelIterator.cpp
			src/db-luna/leveldb/MojDbLevelContainerIterator.cpp
	   )

		set (DB_BACKEND_WRAPPER_CFLAGS "${DB_BACKEND_WRAPPER_CFLAGS} -DMOJ_USE_LDB")
	elseif (backend STREQUAL "sandwich")

		# -- check for LevelDB backend
		find_library(LDB NAMES leveldb ${WEBOS_INSTALL_ROOT}/lib)
		if(LDB STREQUAL "LDB-NOTFOUND")
			MESSAGE(FATAL_ERROR "Failed to find LevelDB libaries. Please install.")
		endif()

		set (DB_BACKEND_INCLUDES ${DB_BACKEND_INCLUDES} ${WEBOS_INSTALL_ROOT}/include)
		set (DB_BACKEND_LIB ${DB_BACKEND_LIB} ${LDB})

		set(DB_BACKEND_WRAPPER_SOURCES
			${DB_BACKEND_WRAPPER_SOURCES}
			src/storage-sandwich/MojDbSandwichEngine.cpp
			src/storage-sandwich/MojDbSandwichFactory.cpp
			src/storage-sandwich/MojDbSandwichDatabase.cpp
			src/storage-sandwich/MojDbSandwichQuery.cpp
			src/storage-sandwich/MojDbSandwichTxn.cpp
			src/storage-sandwich/MojDbSandwichSeq.cpp
			src/storage-sandwich/MojDbSandwichCursor.cpp
			src/storage-sandwich/MojDbSandwichEnv.cpp
			src/storage-sandwich/MojDbSandwichIndex.cpp
			src/storage-sandwich/MojDbSandwichItem.cpp
			src/storage-sandwich/MojDbSandwichIterator.cpp
			src/storage-sandwich/MojDbSandwichContainerIterator.cpp
		)

		set (DB_BACKEND_WRAPPER_CFLAGS "${DB_BACKEND_WRAPPER_CFLAGS} -I${CMAKE_SOURCE_DIR}/src/storage-sandwich -DMOJ_USE_SANDWICH")
	else ()
		message(FATAL_ERROR "WEBOS_DB8_BACKEND: unsuported value '${backend}'")
	endif ()
endforeach ()

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
