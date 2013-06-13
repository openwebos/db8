Summary
-------
A userspace service that provides access to the openWebOS database

Description
-----------
DB8 is a userspace service that provides access to the webOS database.  Access to the database APIs is provided over the luna-service bus.  This initial release provides the infrastructure code to wrap a range of database engines.  The webOS team is currently implementing support for the LevelDB engine and updates will be posted here.

How to Build on Linux
=====================

## Building using OpenEmbedded 

Using the meta-webos layer for OpenEmbedded is the preferred method of building Open webOS components.

This allows your package to be installed into an Open webOS system, or as part of an Open webOS image.

### Building the latest "stable" version

Clone the repository at http://www.github.com/openwebos/build-webos and follow the instructions in that README to build Open webOS.

To build or rebuild a single Open webOS component, if your build-webos directory is ~/openwebos/build-webos, and you are wanting to rebuild the component called "db8", do:

    $ cd ~/openwebos/build-webos
    $ make cleanall-db8 db8

The resulting IPK package will be in your BUILD-[target-machine] directory, under deploy/ipk/[architecture], such as this example:

    ~/openwebos/build-webos/BUILD-qemux86/deploy/ipk/i586/db8_2.0.0-1.00-r5_i586.ipk

You can transfer this to your existing image, and install it by logging into the Open webOS system, and using:

    $ ipkg install /path/to/db8_2.0.0-1.00-r5_i586.ipk

Or you can create a completely new Open webOS image with:

    $ make webos-image

### Building your local clone

After successfully building the latest stable version, you may configure build-webos to build this component from your own local clone.

You can specify what directory to use as the local source inside the file "global-webos.conf" in your home directory, or within the file "webos-local.conf" within the build-webos directory, by adding the following:

    S_pn-[component-name] = "/path/to/component/source"

such as in this example:

    S_pn-db8 = "/home/user/openwebos/db8"

Then follow the instructions above to rebuild and install this package.

## Building for Open webOS Desktop

It is often desireable, for rapid iteration and testing purposes, to build a component for use within the Open webOS Desktop system.

### Building the latest "stable" version

Clone the repository at http://www.github.com/openwebos/build-desktop and follow the instructions in the README file.

### Building your local clone

First, follow the directions to build the latest "stable" version.

To build your local clone of a component, such as db8, instead of the "stable" version installed with the build-webos-desktop script:

* Open the build-webos-desktop.sh script with a text editor
* Locate the function build_component-name (i.e., build_db8)
* Change the line "cd $BASE/db8" to use the folder containing your clone, for example "cd ~/openwebos/db8"
* Close the text editor
* Remove the file ~/luna-desktop-binaries/component-name/luna-desktop-build*.stamp (<tt>~/luna-desktop-binaries/db8/luna-desktop-build*.stamp</tt>)
* Rebuild by running the build-webos-desktop.sh script again

Cautions:

* When you re-clone openwebos/build-desktop, you'll have to overwrite your changes and reapply them
* Components often advance in parallel with each other, so be prepared to keep your cloned repositories updated
* Fetch and rebase frequently

## Building Standalone (without webOS)

This component of webOS can be built as a standalone piece that does not depend upon the rest of the system. 

### Dependencies

Below are the tools and libraries (and their minimum versions) required to build this package:

* openwebos/cmake-modules-webos 1.0.0 RC2
* cmake (version required by openwebos/cmake-modules-webos)
* libdb-4.8-dev
* libicu-dev
* glib-2.0
* gthread-2.0
* openwebos/luna-service2

### Configure database backend

db8 support different database backends. To select db8 backend, set <tt>cmake</tt> variable <tt>WEBOS_DB8_BACKEND</tt> to one of supported database
backends. Currently support database backends are:

* leveldb
* berkeleydb

For example: 

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/openwebos -D WEBOS_DB8_BACKEND:STRING=leveldb ..

By default, db8 compiles with berkeleydb database backend.

### Enable unit tests

To build the unit tests for db8, specify a true value for `WEBOS_CONFIG_BUILD_TESTS` on the cmake command line. For example:

    $ cmake -D WEBOS_CONFIG_BUILD_TESTS:BOOL=TRUE

### Building Standalone

#### Using cmake

Once you have downloaded the source, execute the following to build it (after changing into the directory under which it was downloaded):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake ..
    $ make
    $ sudo make install

The directory under which the files are installed defaults to <tt>/usr/local/webos</tt>.
You can install them elsewhere by supplying a value for <tt>WEBOS_INSTALL_ROOT</tt> when invoking <tt>cmake</tt>. 

For example:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/openwebos ..
    $ make
    $ make install

will install the files in subdirectories of <tt>$HOME/projects/openwebos</tt>.

Specifying <tt>WEBOS_INSTALL_ROOT</tt> also causes <tt>pkg-config</tt> to look in that tree first before searching the standard locations.
You can specify additional directories to be searched prior to this one by setting the <tt>PKG_CONFIG_PATH</tt> environment variable.

To see all of the make targets that CMake has generated, issue:

    $ make help

### Debug build

To enable debug loggin and additional debug checks, set variables MOJ_DEBUG=1 and MOJ_DEBUG_LOGGIN=1

#### Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

# Copyright and License Information

All content, including all source code files and documentation files in this repository are:
 Copyright (c) 2009-2013 LG Electronics, Inc.

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


