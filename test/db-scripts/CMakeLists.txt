# @@@LICENSE
#
# Copyright (c) 2014 LG Electronics, Inc.
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

install(FILES readme
        DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests/db-scripts/
        PERMISSIONS OWNER_WRITE OWNER_READ)

install(FILES test_quota.bash
        DESTINATION ${WEBOS_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/tests/db-scripts/
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
