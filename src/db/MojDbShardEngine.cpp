/* @@@LICENSE
*
* Copyright (c) 2013 LG Electronics, Inc.
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

#include "db/MojDbShardEngine.h"
#include "db/MojDb.h"
#include "db/MojDbServiceDefs.h"
#include <boost/crc.hpp>
#include <string>

using namespace std;

static const MojChar* const ShardInfoKind1Str =
    _T("{\"id\":\"ShardInfo1:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"Info1\", \"props\":[ {\"name\":\"ShardId\"},{\"name\":\"Active\"},{\"name\":\"Transient\"},{\"name\":\"IdBase64\"}, \
                                                       {\"name\":\"deviceId\"},{\"name\":\"deviceUri\"},{\"name\":\"mountPath\"},{\"name\":\"deviceName\"} ]}, \
                      {\"name\":\"Info2\", \"props\":[ {\"name\":\"mountPath\"},{\"name\":\"ShardId\"} ]}, \
                    ]}");

MojLogger MojDbShardEngine::s_log(_T("db.shardEngine"));

MojDbShardEngine::MojDbShardEngine(void)
{
    MojLogTrace(s_log);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    mp_db = nullptr;
#else
    mp_db = NULL;
#endif
}

MojDbShardEngine::~MojDbShardEngine(void)
{
    MojLogTrace(s_log);
}

/**
 *
 */
MojErr MojDbShardEngine::init (MojDb* ip_db)
{
    MojErr err;
    MojObject obj;

    mp_db = ip_db;

    // add type
    err = obj.fromJson(ShardInfoKind1Str);
    MojErrCheck(err);
    err = mp_db->putKind(obj);
    MojErrCheck(err);

    return (MojErrNone);
}

/**
 * put a new shard description to db
 */
MojErr MojDbShardEngine::put (ShardInfo& i_info)
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojObject obj;
    MojString tmp_string;

    MojErr err = obj.putString(_T("_kind"), _T("ShardInfo1:1"));
    MojErrCheck(err);

    MojInt32 val = static_cast<MojInt32>(i_info.id);
    MojObject obj1(val); //keep numeric
    err = obj.put(_T("ShardId"), obj1);
    MojErrCheck(err);

    MojObject obj2(i_info.active);
    err = obj.put(_T("Active"), obj2);
    MojErrCheck(err);

    MojObject obj3(i_info.transient);
    err = obj.put(_T("Transient"), obj3);
    MojErrCheck(err);

    MojObject obj4(i_info.id_base64);
    err = obj.put(_T("IdBase64"), obj4);
    MojErrCheck(err);

    MojObject obj5(i_info.deviceId);
    err = obj.put(_T("deviceId"), obj5);
    MojErrCheck(err);

    MojObject obj6(i_info.deviceUri);
    err = obj.put(_T("deviceUri"), obj6);
    MojErrCheck(err);

    MojObject obj7(i_info.mountPath);
    err = obj.put(_T("mountPath"), obj7);
    MojErrCheck(err);

    MojObject obj8(i_info.deviceName);
    err = obj.put(_T("deviceName"), obj8);
    MojErrCheck(err);

    err = mp_db->put(obj);
    MojErrCheck(err);
    obj.clear();

    return err;
}

/**
 * get shard description by id
 */
MojErr MojDbShardEngine::get (MojUInt32 i_id, ShardInfo& o_info)
{
    MojString str;
    str.assign("");
    return(_get(i_id, str, o_info));
}

/**
 * get shard description by id
 */
MojErr MojDbShardEngine::get (MojString& i_id_base64, ShardInfo& o_info)
{
    return(_get(0, i_id_base64, o_info));
}

/**
 * get correspond shard id using path to device
 */
MojErr MojDbShardEngine::getIdForPath (MojString& i_path, MojUInt32& o_id)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    if ((i_path.length() == 1) && (i_path.last() == '/' ))
    {
        o_id = 0; //main db id
        return MojErrNone;
    }

    MojString path = i_path;
    //trim tailing symbol '/' if present
    if ( (i_path.length() > 2) && (i_path.last() == '/' ) )
    {
        i_path.substring(0, i_path.length() - 2, path);
    }

    //get record from db, extract id and return
    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(path);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("mountPath"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if (found)
        {
            MojInt32 value;
            err = dbObj.get(_T("ShardId"), value, found);
            MojErrCheck(err);

            if (found)
                o_id = static_cast<MojUInt32>(value);
            else
                err = MojErrNotFound;
        }

        cursor.close();
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}

/**
 * get list of all active shards
 */
MojErr MojDbShardEngine::getAllActive (std::list<ShardInfo>& o_list, MojUInt32& o_count)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(true);
    MojObject dbObj;
    ShardInfo shard_info;
    bool flag;
    MojInt32 value_id;
    MojUInt32 count = 0;
    MojString value;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("Active"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found = true;
        while (found)
        {
            err = cursor.get(dbObj, found);
            MojErrCheck(err);

            if (!found)
                break;

            //Shard id
            err = dbObj.get(_T("ShardId"), value_id, found);
            MojErrCheck(err);

            if (found)
                shard_info.id = static_cast<MojUInt32>(value_id);
            else
            {
                break;
            }

            //Transient
            found = dbObj.get(_T("Transient"), flag);
            MojErrCheck(err);

            if (found)
                shard_info.transient = flag;
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'Transient' for record with id [%x]\n"), shard_info.id);
            }

            //IdBase64
            err = dbObj.get(_T("IdBase64"), value, found);
            MojErrCheck(err);

            if (found)
                shard_info.id_base64.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'IdBase64' for record with id [%x]\n"), shard_info.id);
            }

            //deviceId
            err = dbObj.get(_T("deviceId"), value, found);
            MojErrCheck(err);

            if (found)
                shard_info.deviceId.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceId' for record with id [%x]\n"), shard_info.id);
            }

            //deviceUri
            err = dbObj.get(_T("deviceUri"), value, found);
            MojErrCheck(err);

            if (found)
                shard_info.deviceUri.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceUri' for record with id [%x]\n"), shard_info.id);
            }

            //mountPath
            err = dbObj.get(_T("mountPath"), value, found);
            MojErrCheck(err);

            if (found)
                shard_info.mountPath.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'mountPath' for record with id [%x]\n"), shard_info.id);
            }

            //deviceName
            err = dbObj.get(_T("deviceName"), value, found);
            MojErrCheck(err);

            if (found)
                shard_info.deviceName.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceName' for record with id [%x]\n"), shard_info.id);
            }

            o_list.push_back(shard_info);
           ++count;
        }

        cursor.close();
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    o_count = count;
    return err;
}

/**
 * set shard activity
 */
MojErr MojDbShardEngine::setActivity (MojUInt32 i_id, bool i_isActive)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 val = static_cast<MojInt32>(i_id);
    MojObject obj(val);
    MojObject dbObj;
    MojUInt32 count = 0;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("ShardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        cursor.close();

        if (found)
        {
            MojObject update;
            err = update.put(_T("Active"), i_isActive);
            MojErrCheck(err);
            err = mp_db->merge(query, update, count);
            MojErrCheck(err);
        }
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}

/**
 * set transient
 */
MojErr MojDbShardEngine::setTransient (MojUInt32 i_id, bool i_isTransient)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 val = static_cast<MojInt32>(i_id);
    MojObject obj(val);
    MojObject dbObj;
    MojUInt32 count = 0;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("ShardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        cursor.close();

        if (found)
        {
            MojObject update;
            err = update.put(_T("Transient"), i_isTransient);
            MojErrCheck(err);
            err = mp_db->merge(query, update, count);
            MojErrCheck(err);
        }
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}

/**
 * set transient
 */
MojErr MojDbShardEngine::setTransient (MojString& i_id_base64, bool i_isTransient)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(i_id_base64);
    MojObject dbObj;
    MojUInt32 count = 0;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("IdBase64"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        cursor.close();

        if (found)
        {
            MojObject update;
            err = update.put(_T("Transient"), i_isTransient);
            MojErrCheck(err);
            err = mp_db->merge(query, update, count);
            MojErrCheck(err);
        }
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}

/**
 * compute a new shard id
 */
MojErr MojDbShardEngine::computeShardId (MojString& i_media, MojUInt32& o_id)
{
    MojErr err = MojErrNone;
    MojUInt32 id;
    MojUInt32 calc_id;
    MojUInt32 prefix = 1;
    bool found = false;

    if (!_computeId(i_media, calc_id))
        return MojErrInvalidArg;

    do
    {
        id = calc_id | (prefix * 0x01000000);

        //check the table to see if this ID already exists
        err = isIdExist(id);
        if (err == MojErrExists)
        {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> [%x] exist already, prefix = %u\n"), id, prefix);
            prefix++;
        }
        else
        {
            if (err == MojErrNotFound)
            {
                o_id = id;
                found = true;
                break;
            }
            else
                return err;
        }

        if (prefix == 128)
        {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> next iteration\n"));
            prefix = 1;
            _computeId(i_media, calc_id); //next iteration
        }
    }
    while (!found);

    return err;
}

MojErr MojDbShardEngine::isIdExist (MojUInt32 i_id)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 val = static_cast<MojInt32>(i_id);
    MojObject obj(val);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("ShardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        cursor.close();
        return (found ? MojErrExists : MojErrNotFound);
    }

    return err;
}

MojErr MojDbShardEngine::isIdExist (MojString& i_id_base64)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(i_id_base64);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("IdBase64"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        cursor.close();
        return (found ? MojErrExists : MojErrNotFound);
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}

bool MojDbShardEngine::_computeId (MojString& i_media, MojUInt32& o_id)
{
    if(i_media.length() == 0)
        return false;

    std::string str = i_media.data();

    //Create a 24 bit hash of the string
    static boost::crc_32_type result;
    result.process_bytes(str.data(), str.length());
    MojInt32 code = result.checksum();

    //Prefix the 24 bit hash with 0x01 to create a 32 bit unique shard ID
    o_id = code & 0xFFFFFF;

    return true;
}

MojErr MojDbShardEngine::_get (MojUInt32 i_id, MojString& i_id_base64, ShardInfo& o_info)
{
    MojErr err = MojErrNone;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    if(mp_db == nullptr)
        return MojErrNotInitialized;
#else
    if(mp_db == NULL)
        return MojErrNotInitialized;
#endif

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 id = static_cast<MojInt32>(i_id);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);

    if (i_id_base64.length() > 0) //switch
    {
        MojObject obj(i_id_base64);
        o_info.id_base64.assign(i_id_base64);
        err = query.where(_T("IdBase64"), MojDbQuery::OpEq, obj);
    }
    else
    {
        MojObject obj(id);
        o_info.id = i_id;
        err = query.where(_T("ShardId"), MojDbQuery::OpEq, obj);
    }

    MojErrCheck(err);
    err = mp_db->find(query, cursor);

    if (err == MojErrNone)
    {
        bool found;
        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if (found)
        {
            bool flag;
            MojString value;

            if (i_id_base64.length() > 0) //switch
            {
                //Shard id
                err = dbObj.get(_T("ShardId"), id, found);
                MojErrCheck(err);

                if (found)
                    o_info.id = static_cast<MojUInt32>(id);
                else
                {
                    MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'ShardId' for record with id [%s]\n"), i_id_base64.data());
                }
            }
            else
            {
                //IdBase64
                err = dbObj.get(_T("IdBase64"), value, found);
                MojErrCheck(err);

                if (found)
                    o_info.id_base64.assign(value);
                else
                {
                    MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'IdBase64' for record with id [%x]\n"), o_info.id);
                }
            }

            //Active
            found = dbObj.get(_T("Active"), flag);
            MojErrCheck(err);

            if (found)
                o_info.active = flag;
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'Active' for record with id [%x]\n"), o_info.id);
            }

            //Transient
            found = dbObj.get(_T("Transient"), flag);
            MojErrCheck(err);

            if (found)
                o_info.transient = flag;
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'Transient' for record with id [%x]\n"), o_info.id);
            }

            //deviceId
            err = dbObj.get(_T("deviceId"), value, found);
            MojErrCheck(err);

            if (found)
                o_info.deviceId.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceId' for record with id [%x]\n"), o_info.id);
            }

            //deviceUri
            err = dbObj.get(_T("deviceUri"), value, found);
            MojErrCheck(err);

            if (found)
                o_info.deviceUri.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceUri' for record with id [%x]\n"), o_info.id);
            }

            //mountPath
            err = dbObj.get(_T("mountPath"), value, found);
            MojErrCheck(err);

            if (found)
                o_info.mountPath.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'mountPath' for record with id [%x]\n"), o_info.id);
            }

            //deviceName
            err = dbObj.get(_T("deviceName"), value, found);
            MojErrCheck(err);

            if (found)
                o_info.deviceName.assign(value);
            else
            {
                MojLogWarning(MojDbShardEngine::s_log, _T("not found field 'deviceName' for record with id [%x]\n"), o_info.id);
            }
        }

        cursor.close();
    }
    else
    {
        err = MojErrDbObjectNotFound;
    }

    return err;
}