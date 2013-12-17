/****************************************************************
* @@@LICENSE
*
* Copyright (c) 2014 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@
****************************************************************/

#include "SpamTest.h"
#include <array>

#include "utils.h"
#include "Runner.h"
#include "db-luna/MojDbLunaServiceApp.h"

using namespace std;

namespace
{
    constexpr const MojChar* TestKind = "{\"id\":\"Test:1\", \"owner\":\"com.foo.bar\", \"indexes\":[{\"name\":\"foo\",\"props\":[{\"name\":\"foo\",\"collate\":\"secondary\"}]}]}";
    constexpr const MojChar* TestObject = "{\"_kind\":\"Test:1\",\"foo\":\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\"}";
    constexpr unsigned int ThreadsCount = 100;

    constexpr const MojChar* ConfigFile = "'{ \"db\" : {\"path\" : \"/tmp/db8-spam-test1\", \"service_name\" : \"com.palm.testdb\" } }'";

    MojAtomicInt WrittenObjects = 0;
    const string Test1DatabasePath { "/tmp/db8-spam-test1"};
    const string Test2DatabasePath { "/tmp/db8-spam-test2"};

    volatile unsigned int IoErrorCount { 0 };
}

void SpamDatabaseSuite::SetUp()
{
    baseDir = to_string(time(static_cast<time_t>(0)));
    mountTmpFs();
}

void SpamDatabaseSuite::TearDown()
{
    string dbPathTest1 = Test1DatabasePath;
    string dbPathTest2 = Test2DatabasePath;

    EXPECT_TRUE(remove_directory(dbPathTest1));
    EXPECT_TRUE(remove_directory(dbPathTest2));
}


void SpamDatabaseSuite::mountTmpFs()
{
    //ASSERT_FALSE (geteuid()) << "Require root privileges to mount tmpfs dir";
    // lazy mount of tmpfs

    string command = "mountpoint -q " + Test1DatabasePath + " || (mkdir -p " + Test1DatabasePath + " && sudo mount -t tmpfs tmpfs " + Test1DatabasePath + " -o size=2M)";
    int ret = system(command.c_str());
    ASSERT_EQ (WEXITSTATUS(ret), 0) << command;

    command = "mountpoint -q " + Test2DatabasePath + " || (mkdir -p " + Test1DatabasePath + " && sudo mount -t tmpfs tmpfs " + Test2DatabasePath + " -o size=5M)";
    ret = system(command.c_str());
    ASSERT_EQ (WEXITSTATUS(ret), 0) << command;

    EXPECT_TRUE(remove_directory(Test1DatabasePath));
    EXPECT_TRUE(remove_directory(Test2DatabasePath));
}

MojErr threadFunction(void* dbPtr)
{
    MojDb* db = static_cast<MojDb*> (dbPtr);
    MojObject object;
    MojErrCheck(object.fromJson(TestObject));

    MojErr err;
    for (unsigned int i = 0; i < 100000; ++i) {
        err = db->put(object);
        // when not enough space, thread mark itself about DbFatalError and end own execution
        if (err == MojErrNone) {
            ++WrittenObjects;
        } else {
            MojString what;
            MojErrToString(err, what);

            // When no space, should return "No left space on device", and MojErrDBIo
            cerr << "Error ocupired: " << what.data() << endl;
            if (err == MojErrDbIO) {
                ++IoErrorCount;
            }

            return err;
        }
    }

    return MojErrNone;
}

template <typename C>
void createThreads(C& container, MojDb& db)
{
    for (unsigned int i = ThreadsCount; i > 0; --i) {
        MojThreadT thread;
        MojAssertNoErr(MojThreadCreate(thread, threadFunction, &db));

        container.push_back(thread);
    }
}
template <typename C>
void joinThreads(C& threadPool)
{
    for (auto i = threadPool.begin(); i != threadPool.end(); ++i) {
        MojErr err;
        MojAssertNoErr(MojThreadJoin(*i, err));
        //MojAssertNoErr(err);
        EXPECT_EQ(err, MojErrDbIO);  // when no space, all threads should end with MojErrDbFatal
    }
}

void SpamDatabaseSuite::copyDatabaseContent()
{
    string dbPathTest1 = Test1DatabasePath + "/" + baseDir;
    string dbPathTest2 = Test2DatabasePath + "/" + baseDir;

    string command = "cp -r " + dbPathTest1 + " " + Test2DatabasePath;
    int ret = system(command.c_str());
    ASSERT_EQ (WEXITSTATUS(ret), 0) << command;
}

void SpamDatabaseSuite::spamDatabase()
{
    const string path = Test1DatabasePath + "/" + baseDir;
    MojAssertNoErr( db.open(  path.c_str()) );

    // add kind
    MojObject obj;
    MojAssertNoErr( obj.fromJson(TestKind) );
    MojAssertNoErr( db.putKind(obj) );

    /*MojObject object;
    MojAssertNoErr(object.fromJson(TestObject));*/

    list<MojThreadT> threadPool;
    createThreads(threadPool, db);

    joinThreads(threadPool);

    MojExpectNoErr( db.close() );


}
void SpamDatabaseSuite::readSpammedDatabase()
{
    string dbPathTest2 = Test2DatabasePath + "/" + baseDir;
    MojAssertNoErr( db.open(dbPathTest2.c_str()) );

    // try to read all objects, that was successfully saved before space exceeded
    MojDbQuery query;
    MojDbCursor cursor;
    query.limit(MojUInt32Max);
    MojAssertNoErr( query.from(_T("Test:1")) );
    MojAssertNoErr( db.find(query, cursor) );

    MojInt32 count = 0;
    MojInt32 writtenObjects = WrittenObjects.value();
    bool found;
    for (;;) {
        MojObject obj;
        MojAssertNoErr( cursor.get(obj, found) );
        if (!found)
            break;
        ++count;
    }

    MojObject object;
    object.fromJson(TestObject);
    for (unsigned i = 0; i < 100; ++i) {
        MojErr err = db.put(object);
        ASSERT_EQ(err, MojErrNone) << "Put copied database failed";
    }

    MojExpectNoErr( cursor.close() );
    MojExpectNoErr( db.close() );
}

TEST_F(SpamDatabaseSuite, spamTempdb)
{
    spamDatabase();
    copyDatabaseContent();
    readSpammedDatabase();
}
