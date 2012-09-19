Summary
-------
A userspace service that provides access to the openWebOS database

Description
-----------
DB8 is a userspace service that provides access to the webOS database.  Access to the database APIs is provided over the luna-service bus.  This initial release provides the infrastructure code to wrap a range of database engines.  The webOS team is currently implementing support for the LevelDB engine and updates will be posted here.

# Build Instructions 

Please make sure you have BerkeleyDB installed on your system:

        sudo apt-get install libdb4.8-dev libicu-dev

Next, cd to the folder where you have downloaded db8 and execute the following instructions:

        make -f Makefile.Ubuntu.Release install

This will create the binary mojodb-luna under the folder release-linux-x86 along with libraries libmojocore.so, libmojodb.so and libmojoluna.so

# Copyright and License Information

All content, including all source code files and documentation files in this repository are:
 Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.

All content, including all source code files and documentation files in this repository are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# end Copyright and License Information


