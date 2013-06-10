# @@@LICENSE
#
# Copyright (c) 2013 LG Electronics, Inc.
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


project(gtest_mojocore C CXX)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_definitions(-Wall -std=c++0x -Wno-deprecated -pthread)

add_executable(${PROJECT_NAME}
               Runner.cpp
               AtomicIntTest.cpp
               )

target_link_libraries(${PROJECT_NAME}
                      mojocore
                      )

target_link_libraries(${PROJECT_NAME} gtest gtest_main)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)
