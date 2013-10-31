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

set (CORE_TEST_SOURCES
     MojCoreTestRunner.cpp
     MojAtomicIntTest.cpp
     MojAutoPtrTest.cpp
     MojBufferTest.cpp
     MojDataSerializationTest.cpp
     MojDecimalTest.cpp
     MojErrTest.cpp
     MojHashMapTest.cpp
     MojJsonTest.cpp
     MojListTest.cpp
     MojLogTest.cpp
     MojMapTest.cpp
     MojMessageDispatcherTest.cpp
     MojObjectFilterTest.cpp
     MojObjectSerializationTest.cpp
     MojObjectTest.cpp
     MojReactorTest.cpp
     MojRefCountTest.cpp
     MojSchemaTest.cpp
     MojSetTest.cpp
     MojSignalTest.cpp
     MojStringTest.cpp
     MojThreadTest.cpp
     MojTimeTest.cpp
     MojUtilTest.cpp
     MojVectorTest.cpp
)

add_executable(test_core ${CORE_TEST_SOURCES})
target_link_libraries(test_core mojocore)

install(PROGRAMS ${CMAKE_BINARY_DIR}/test/core/test_core DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests)
