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

# the RPATH to be used when installing
set(CMAKE_INSTALL_RPATH "")

set (DB_CLIENT_TEST_SOURCES
     MojDbClientTest.cpp
)

add_executable(test_dbclient ${DB_CLIENT_TEST_SOURCES})
target_link_libraries(test_dbclient
                      mojocore
                      mojodb
                      mojoluna
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/test/dbclient/test_dbclient DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)
