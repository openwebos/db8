# @@@LICENSE
#
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

include_directories(${CMAKE_SOURCE_DIR}/inc)
set(DB_BACKEND_WRAPPER_SOURCES_CPP "")
foreach(filename ${DB_BACKEND_WRAPPER_SOURCES})
    set(DB_BACKEND_WRAPPER_SOURCES_CPP ${DB_BACKEND_WRAPPER_SOURCES_CPP} "${CMAKE_SOURCE_DIR}/${filename}")
endforeach ()

# --------------------------------
# dbcore target
# ---------------------------------
set (DB_TEST_SOURCES
     MojDbTestRunner.cpp
     MojDbBulkTest.cpp
     MojDbConcurrencyTest.cpp
     MojDbCrudTest.cpp
     MojDbDumpLoadTest.cpp
     MojDbIndexTest.cpp
     MojDbKindTest.cpp
     MojDbLocaleTest.cpp
     MojDbPermissionTest.cpp
     MojDbPurgeTest.cpp
     MojDbQueryTest.cpp
     MojDbQueryFilterTest.cpp
     MojDbQuotaTest.cpp
     MojDbRevTest.cpp
     MojDbRevisionSetTest.cpp
     MojDbSearchTest.cpp
     MojDbTextCollatorTest.cpp
     MojDbTextTokenizerTest.cpp
     MojDbWatchTest.cpp
     MojDbTestStorageEngine.cpp
)

add_executable(test_db_core ${DB_TEST_SOURCES} ${DB_BACKEND_WRAPPER_SOURCES_CPP})
target_link_libraries(test_db_core
                      mojocore
                      mojodb
                      ${DB_BACKEND_LIB}
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/test/db/test_db_core DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)

# --------------------------------
# performance test target
# ---------------------------------
set (DB_PERF_TEST_SOURCES
     MojDbPerfTestRunner.cpp
     MojDbPerfTest.cpp
     MojDbPerfCreateTest.cpp
     MojDbPerfDeleteTest.cpp
     MojDbPerfReadTest.cpp
     MojDbPerfUpdateTest.cpp
)

add_executable(test_db_performance ${DB_PERF_TEST_SOURCES} ${DB_BACKEND_WRAPPER_SOURCES_CPP})
target_link_libraries(test_db_performance
                      mojocore
                      mojodb
                      ${DB_BACKEND_LIB}
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/test/db/test_db_performance DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)

# --------------------------------
# stress test target
# ---------------------------------
set (DB_STRESS_TEST_SOURCES
     MojDbStressTest.cpp
)

add_executable(test_db_stress ${DB_STRESS_TEST_SOURCES} ${DB_BACKEND_WRAPPER_SOURCES_CPP})
target_link_libraries(test_db_stress
                      mojocore
                      mojodb
                      mojoluna
                      ${DB_BACKEND_LIB}
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/test/db/test_db_stress DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)

