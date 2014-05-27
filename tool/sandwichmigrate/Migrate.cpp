/* @@@LICENSE
*
*      Copyright (c) 2014 LG Electronics, Inc.
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
#include <iostream>
#include <deque>
#include <vector>
#include <string>

#include <core/MojOs.h>
#include <leveldb/txn_db.hpp>
#include <leveldb/bottom_db.hpp>
#include "leveldb/sandwich_db.hpp"

MojErr doMigrate(MojObject& confObj)
{
    int idx = 0;
    std::deque<std::string> parts = {"indexes.ldb", "indexIds.db", "kinds.db", "objects.db", "seq.ldb", "UsageDbName"};

    MojErr err;
    int result = 0;
    MojObject dbConf;
    err = confObj.getRequired("db", dbConf);
    MojErrCheck(err);

    MojObject dbPath;
    err = dbConf.getRequired("path", dbPath);
    MojErrCheck(err);

    MojString dbName;
    MojString path;
    err = dbPath.stringValue(path);
    MojErrCheck(err);

    MojString checkFolder;
    err = checkFolder.append(path);
    MojErrCheck(err);
    err = checkFolder.append("/indexIds.db");
    MojErrCheck(err);
    MojStatT stat;
    err = MojStat(checkFolder, &stat);
    if (err == MojErrNotFound)
    {
        MojErrThrowMsg(MojErrNone, _T("Database is already converted"));
    }

    MojSize pos = path.rfind('/');
    path.substring(pos, path.length()-pos, dbName);

    MojString lockFile;
    lockFile.append(path);
    lockFile.append("/migration.lock");
    if(!access(lockFile.data(), F_OK))
    {
        MojErrThrowMsg(MojErrExists, _T("Database is locked"));
    }

    //check - valid old database
    for (const auto &part : parts)
    {
        leveldb::BottomDB db;
        std::string ldbPpath(path.data());
        ldbPpath += "/";
        auto s = db.Open((ldbPpath + part).c_str());
        if (!s.ok())
        {
            MojErrThrowMsg(MojErrDbIO, _T("Failed to open source database %s:%s"), (ldbPpath + part).c_str(), s.ToString().c_str());
        }
    }

    MojString bkPath;
    bkPath.append(WEBOS_PERSISTENTSTORAGEDIR "/db8/migration");
    bkPath.append(dbName);

    MojString execCommand;
    //remove temp
    execCommand.clear();
    execCommand.append("rm -r ");
    execCommand.append(bkPath);
    system(execCommand.data());

    //create temporary folder
    execCommand.clear();
    execCommand.append("mkdir -p ");
    execCommand.append(bkPath);
    result = system(execCommand.data());
    MojErrCheck(result);

    //create file identifer for identification any troubles during process
    FILE* pFile = fopen(lockFile.data(), "a");
    if(!pFile)
        return MojErrAccessDenied;
    fclose(pFile);

    typedef leveldb::SandwichDB<leveldb::TxnDB<leveldb::BottomDB>> BackendDbTxn;
    typedef leveldb::SandwichDB<leveldb::BottomDB> BackendDb;
    {
        BackendDb cdb;
        cdb->options.create_if_missing = true;
        cdb->options.error_if_exists = true;
        auto s = cdb->Open(bkPath.data());
        if (!s.ok())
        {
            MojErrThrowMsg(MojErrDbIO, _T("Failed to open destination database %s:%s"), bkPath.data(), s.ToString().c_str());
        }
        BackendDbTxn txnDb = cdb.ref<leveldb::TxnDB>();

        size_t batchSize = 0;
        for (const auto &part : parts)
        {
            leveldb::BottomDB db;
            std::string ldbPpath(path.data());
            ldbPpath += "/";
            s = db.Open((ldbPpath + part).c_str());
            if (!s.ok())
            {
                MojErrThrowMsg(MojErrDbIO, _T("Failed to open source database %s:%s"), (ldbPpath + part).c_str(), s.ToString().c_str());
            }
            auto pdb = txnDb.use(parts[idx++]);
            decltype(db)::Walker it { db };
            for (it.SeekToFirst(); it.Valid(); it.Next())
            {
                pdb.Put(it.key(), it.value());
                batchSize += it.key().size();
                batchSize += it.value().size();
                if(batchSize >= 1024*1024)
                {
                    txnDb->commit();
                    batchSize = 0;
                }
            }
        }
        txnDb->commit();
    }

    //remove old-database
    execCommand.clear();
    execCommand.append("rm -r ");
    execCommand.append(path);
    execCommand.append("/*/");
    result = system(execCommand.data());
    MojErrCheck(result);

    //copy
    execCommand.clear();
    execCommand.append("cp -rf ");
    execCommand.append(bkPath);
    execCommand.append("/* ");
    execCommand.append(path);
    result = system(execCommand.data());
    MojErrCheck(result);

    //remove temp
    execCommand.clear();
    execCommand.append("rm -r ");
    execCommand.append(bkPath);
    result = system(execCommand.data());
    MojErrCheck(result);

    return MojErrNone;
}

int main(int argc, char**argv)
{
	if (argc != 2) {
		LOG_ERROR(MSGID_DB_ERROR, 0, "Invalid arg, This program need args(config file path)");
		return -1;
	}

	MojObject confObj;

	MojErr err;

	MojString confStr;
	err = MojFileToString(argv[1], confStr);
	if (err != MojErrNone)
	{
		LOG_ERROR(MSGID_DB_ERROR, 0, "Can't read configuration file");
		return 0;
	}
	err = confObj.fromJson(confStr);
	if (err != MojErrNone)
	{
		LOG_ERROR(MSGID_DB_ERROR, 0, "Can't read configuration file");
		return 0;
	}

	err = doMigrate(confObj);
	if (err != MojErrNone)
	{
		LOG_ERROR(MSGID_DB_ERROR, 0, "Can't convert database");
		return 0;
	}

	return 0;
}

