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


project(gtest_db_media C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../db)

add_definitions(-Wall -Wno-deprecated -pthread --std=c++0x)

foreach(filename ${DB_BACKEND_WRAPPER_SOURCES})
    set(DB_BACKEND_WRAPPER_SOURCES_CPP ${DB_BACKEND_WRAPPER_SOURCES_CPP} "${CMAKE_SOURCE_DIR}/${filename}")
endforeach ()

add_executable(${PROJECT_NAME}
               MediaTest.cpp
               Runner.cpp
               ${DB_BACKEND_WRAPPER_SOURCES_CPP})

target_link_libraries(${PROJECT_NAME}
                      mojocore
                      mojodb
                      mojoluna
                      ${DB_BACKEND_LIB})

target_link_libraries(${PROJECT_NAME} gtest gtest_main)

foreach (backend ${WEBOS_DB8_BACKEND})
    add_test(NAME ${backend}/${test} COMMAND env MOJODB_ENGINE=${backend} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
endforeach ()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ)

