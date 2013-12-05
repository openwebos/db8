/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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
* LICENSE@@@ */


#include "db/MojDb.h"
#include "core/MojUtil.h"
#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#else
#error "Database Engine doesn't set. See README.txt"
#endif

enum DATA_TYPE {
    dataTypeKind,
    dataTypePermission,
    dataTypeObject,
};

//DBGen

MojErr genDbImage(const MojChar* dirPath, MojDb& db, DATA_TYPE type)
{
    // open dir
    MojString entryPath;
    MojDirT dir = MojInvalidDir;
    MojErr err = MojDirOpen(dir, dirPath);
    MojErrCheck(err);

    //process files from directory path
    for(;;){
        bool entRead = false;
        MojDirentT ent;
        err = MojDirRead(dir, &ent, entRead);
        MojErrGoto(err, Done);
        if (!entRead)
            break;

        if (MojStrCmp(ent.d_name, _T(".")) == 0 ||MojStrCmp(ent.d_name, _T("..")) == 0)
            continue;

        entryPath.clear();
        entryPath.append(dirPath);
        entryPath.append(ent.d_name);
        MojStatT stat;
        MojErr err = MojStat(entryPath.data(), &stat);
        MojErrGoto(err, Done);
        if (stat.st_mode & S_IFREG){
            if (type == dataTypeObject) {
                MojUInt32 count;
                err = db.load(entryPath.data(), count);
            } else {
                MojString inputStr;
                MojObject inputObj;

                err = MojFileToString(entryPath.data(), inputStr);
                MojErrGoto(err, Done);
                err = inputObj.fromJson(inputStr);
                MojErrGoto(err, Done);
                if (type == dataTypeKind) {
                    err = db.putKind(inputObj);
                } else if (type == dataTypePermission) {
                // permission files form with array object
                    MojObject::ArrayIterator begin;
                    err = inputObj.arrayBegin(begin);
                    MojErrGoto(err, Done);
                    MojObject::ConstArrayIterator end = inputObj.arrayEnd();
                    err = db.putPermissions(begin, end);
                }
            }
            MojErrGoto(err, Done);
        } else {
            // there are must be regular files.
            err = MojErrUnknown;
            MojErrGoto(err, Done);
        }
    }
Done:
    if (dir != MojInvalidDir)
        (void) MojDirClose(dir);

    return err;
}

int main(int argc, char**argv)
{
#ifdef MOJ_USE_BDB
    MojDbStorageEngine::setEngineFactory(new MojDbBerkeleyFactory());
#elif MOJ_USE_LDB
    MojDbStorageEngine::setEngineFactory(new MojDbLevelFactory());
#endif
    if (argc < 3) {
        LOG_ERROR(MSGID_DB_ERROR, 0, "Invalid arg, This program need two args(input and output path)");
        return -1;
    }

    typedef struct {
        const MojChar* dirName;
        DATA_TYPE type;
    } DBGEN_T;
    static const DBGEN_T dbGenType[] = {{_T("data/"),dataTypeObject},{_T("permissions/"),dataTypePermission}, {_T("kinds/"),dataTypeKind} };

    MojDb db;
    MojString dirName;
    MojErr err = db.open(argv[2]);
    MojErrCheck(err);

    int cnt = sizeof(dbGenType) / sizeof(dbGenType[0]);

    /* find each sub dir(1.kind -> 2.permissions -> 3.data) */
    while(--cnt >= 0)
    {
        dirName.clear();
        dirName.append(argv[1]);
        dirName.append(dbGenType[cnt].dirName);

        err = genDbImage(dirName.data(), db, dbGenType[cnt].type);
        if (err != MojErrNone) {
            MojString str;
            MojErrToString(err, str);
            LOG_ERROR(MSGID_DB_ERROR, 0, "(%s) error occured during generating DB image", str.data());
            db.drop(argv[2]);
            break;
        }
    }
    db.close();

    return 0;
}
