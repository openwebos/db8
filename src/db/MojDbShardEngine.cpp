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
#include "db/MojDbMediaLinkManager.h"
#include "core/MojDataSerialization.h"
#include <boost/crc.hpp>
#include <string>

using namespace std;



static const MojChar* const ShardInfoKind1Str =
    _T("{\"id\":\"ShardInfo1:1\",")
    _T("\"owner\":\"mojodb.admin\",")
    _T("\"indexes\":[ {\"name\":\"ShardId\",  \"props\":[ {\"name\":\"shardId\"} ]}, \
                      {\"name\":\"DeviceId\", \"props\":[ {\"name\":\"deviceId\"} ]}, \
                      {\"name\":\"IdBase64\", \"props\":[ {\"name\":\"idBase64\"} ]}, \
                      {\"name\":\"Active\",   \"props\":[ {\"name\":\"active\"} ]}\
                    ]}");

MojLogger MojDbShardEngine::s_log(_T("db.shardEngine"));

MojDbShardEngine::MojDbShardEngine(void)
    : m_pdmWatcher(this)
{
    MojLogTrace(s_log);
    mp_db = 0;
}

MojDbShardEngine::~MojDbShardEngine(void)
{
    MojLogTrace(s_log);
}

/**
 *
 */
MojErr MojDbShardEngine::init (MojDb* ip_db, MojDbReq &req)
{
    MojLogTrace(s_log);
    MojAssert(ip_db);
    MojErr err;
    MojObject obj;

    mp_db = ip_db;
    m_mediaLinkManager.reset(new MojDbMediaLinkManager());

    // add type
    err = obj.fromJson(ShardInfoKind1Str);
    MojErrCheck(err);
    err = mp_db->kindEngine()->putKind(obj, req, true); // add builtin kind
    MojErrCheck(err);

    return (MojErrNone);
}

/**
 * put a new shard description to db
 */
MojErr MojDbShardEngine::put (const ShardInfo& shardInfo)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);
    MojAssert(shardInfo.id);

    MojObject obj;
    MojString tmp_string;
    MojErr err;

    err = obj.putString(_T("_kind"), _T("ShardInfo1:1"));
    MojErrCheck(err);

    MojInt32 val = static_cast<MojInt32>(shardInfo.id);
    MojObject obj1(val); //keep numeric
    err = obj.put(_T("shardId"), obj1);
    MojErrCheck(err);

    MojObject obj2(shardInfo.active);
    err = obj.put(_T("active"), obj2);
    MojErrCheck(err);

    MojObject obj3(shardInfo.transient);
    err = obj.put(_T("transient"), obj3);
    MojErrCheck(err);

    if (!shardInfo.id_base64.empty()) {
        MojObject obj4(shardInfo.id_base64);
        err = obj.put(_T("idBase64"), obj4);
        MojErrCheck(err);
    } else {
        MojString shardIdBase64;
        err = MojDbShardEngine::convertId(shardInfo.id, shardIdBase64);
        MojErrCheck(err);

        MojObject obj4(shardIdBase64);
        err = obj.put(_T("idBase64"), obj4);
        MojErrCheck(err);
    }

    MojObject obj5(shardInfo.deviceId);
    err = obj.put(_T("deviceId"), obj5);
    MojErrCheck(err);

    MojObject obj6(shardInfo.deviceUri);
    err = obj.put(_T("deviceUri"), obj6);
    MojErrCheck(err);

    MojObject obj7(shardInfo.mountPath);
    err = obj.put(_T("mountPath"), obj7);
    MojErrCheck(err);

    MojObject obj8(shardInfo.deviceName);
    err = obj.put(_T("deviceName"), obj8);
    MojErrCheck(err);

    err = mp_db->put(obj);
    MojErrCheck(err);

    return MojErrNone;
}

/**
 * get shard description by id
 */
MojErr MojDbShardEngine::get (MojUInt32 shardId, ShardInfo& shardInfo, bool& found)
{
    MojErr err;
    MojAssert(mp_db);

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 id = static_cast<MojInt32>(shardId);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);

    MojObject obj(id);
    err = query.where(_T("shardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);
    MojErrCheck(err);

    err = cursor.get(dbObj, found);
    MojErrCheck(err);
    if (found)
    {
        err = dbObj.getRequired(_T("shardId"), shardInfo.id);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("idBase64"), shardInfo.id_base64);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("active"), shardInfo.active);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("transient"), shardInfo.transient);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceId"), shardInfo.deviceId);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceUri"), shardInfo.deviceUri);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("mountPath"), shardInfo.mountPath);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceName"), shardInfo.deviceName);
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * get list of all active shards
 */
MojErr MojDbShardEngine::getAllActive (std::list<ShardInfo>& shardInfoList, MojUInt32& count)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);

    MojErr err;

    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(true);

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("active"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);
    MojErrCheck(err);

    count = 0;
    shardInfoList.clear();

    while (true)
    {
        bool found;
        MojObject dbObj;

        err = cursor.get(dbObj, found);
        MojErrCheck(err);
        if (!found)
            break;

        ShardInfo shardInfo;

        err = dbObj.getRequired(_T("shardId"), shardInfo.id);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("transient"), shardInfo.transient);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("idBase64"), shardInfo.id_base64);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceId"), shardInfo.deviceId);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceUri"), shardInfo.deviceUri);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("mountPath"), shardInfo.mountPath);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceName"), shardInfo.deviceName);
        MojErrCheck(err);

        shardInfoList.push_back(shardInfo);
        ++count;
    }

    return MojErrNone;
}

/**
 * update shardInfo
 */
MojErr MojDbShardEngine::update (const ShardInfo& i_shardInfo)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);
    MojErr err;

    MojDbQuery query;
    MojObject obj(static_cast<MojInt32>(i_shardInfo.id));
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("shardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    MojObject update;
    MojUInt32 count = 0;

    MojObject obj2(i_shardInfo.active);
    err = update.put(_T("active"), obj2);
    MojErrCheck(err);

    err = update.put(_T("transient"), i_shardInfo.transient);
    MojErrCheck(err);

    MojObject obj4(i_shardInfo.id_base64);
    err = update.put(_T("idBase64"), obj4);
    MojErrCheck(err);

    MojObject obj5(i_shardInfo.deviceId);
    err = update.put(_T("deviceId"), obj5);
    MojErrCheck(err);

    MojObject obj6(i_shardInfo.deviceUri);
    err = update.put(_T("deviceUri"), obj6);
    MojErrCheck(err);

    MojObject obj7(i_shardInfo.mountPath);
    err = update.put(_T("mountPath"), obj7);
    MojErrCheck(err);

    MojObject obj8(i_shardInfo.deviceName);
    err = update.put(_T("deviceName"), obj8);
    MojErrCheck(err);

    err = mp_db->merge(query, update, count);
    MojErrCheck(err);

    if (count == 0) {
        return MojErrDbObjectNotFound;
    }

    return err;
}


MojErr MojDbShardEngine::getByDeviceUuid (const MojString& deviceUuid, ShardInfo& shardInfo, bool& found)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);
    MojErr err;

    //get record from db, extract id
    MojDbQuery query;
    MojDbCursor cursor;
    MojObject obj(deviceUuid);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);
    err = query.where(_T("deviceId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);
    MojErrCheck(err);

    err = cursor.get(dbObj, found);
    MojErrCheck(err);

    if (found)
    {
        err = dbObj.getRequired(_T("shardId"), shardInfo.id);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("idBase64"), shardInfo.id_base64);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("active"), shardInfo.active);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("transient"), shardInfo.transient);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceId"), shardInfo.deviceId);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceUri"), shardInfo.deviceUri);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("mountPath"), shardInfo.mountPath);
        MojErrCheck(err);

        err = dbObj.getRequired(_T("deviceName"), shardInfo.deviceName);
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * get shard id
 * ------------
 * search within db for i_deviceId, return id if found
 * else
 * allocate a new id
 */
MojErr MojDbShardEngine::getShardId (const MojString& deviceUuid, MojUInt32& shardId)
{
    MojLogTrace(s_log);
    MojAssert(mp_db);
    MojErr err;
    ShardInfo shardInfo;
    bool found;

    err = getByDeviceUuid(deviceUuid, shardInfo, found);
    MojErrCheck(err);
    if (found)  {
        shardId = shardInfo.id;
        MojLogDebug(s_log, _T("Shard id for device %s is %d"), deviceUuid.data(), shardId);
    } else {
        MojLogDebug(s_log, _T("Shard id for device %s not found, generating it"), deviceUuid.data());
        err = allocateId(deviceUuid, shardId);
        MojErrCheck(err);
    }

    return MojErrNone;
}

/**
 * compute a new shard id
 */
MojErr MojDbShardEngine::allocateId (const MojString& deviceUuid, MojUInt32& shardId)
{
    MojLogTrace(s_log);

    MojErr err;
    MojUInt32 id;
    MojUInt32 calc_id;
    MojUInt32 prefix = 1;
    bool found = false;

    err = computeId(deviceUuid, calc_id);
    MojErrCheck(err);

    do
    {
        id = calc_id | (prefix * 0x01000000);

        //check the table to see if this ID already exists
        err = isIdExist(id, found);
        MojErrCheck(err);

        if (found) {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> [%x] exist already, prefix = %u\n"), id, prefix);
            prefix++;
        } else {
            MojAssert(id);

            shardId = id;
            break;  // exit from loop
        }

        if (prefix == 128)
        {
            MojLogWarning(MojDbShardEngine::s_log, _T("id generation -> next iteration\n"));
            prefix = 1;
            computeId(deviceUuid, calc_id); //next iteration
        }
    }
    while (!found);

    return err;
}

MojErr MojDbShardEngine::isIdExist (MojUInt32 shardId, bool& found)
{
    MojErr err;
    MojAssert(mp_db);

    MojDbQuery query;
    MojDbCursor cursor;
    MojInt32 val = static_cast<MojInt32>(shardId);
    MojObject obj(val);
    MojObject dbObj;

    err = query.from(_T("ShardInfo1:1"));
    MojErrCheck(err);

    err = query.where(_T("shardId"), MojDbQuery::OpEq, obj);
    MojErrCheck(err);

    err = mp_db->find(query, cursor);
    MojErrCheck(err);

    err = cursor.get(dbObj, found);
    MojErrCheck(err);

    err = cursor.close();
    MojErrCheck(err);

    return MojErrNone;
}

MojErr MojDbShardEngine::computeId (const MojString& mediaUuid, MojUInt32& sharId)
{
    MojAssert(!mediaUuid.empty());

    std::string str = mediaUuid.data();

    //Create a 24 bit hash of the string
    boost::crc_32_type result;
    result.process_bytes(str.data(), str.length());
    MojInt32 code = result.checksum();
    result.reset();

    //Prefix the 24 bit hash with 0x01 to create a 32 bit unique shard ID
    sharId = code & 0xFFFFFF;

    return MojErrNone;
}

MojDbShardEngine::Watcher::Watcher(MojDbShardEngine* shardEngine)
    : m_shardEngine(shardEngine),
    m_pdmSlot(this, &Watcher::handleShardInfoSlot)
{
}

MojErr MojDbShardEngine::Watcher::handleShardInfoSlot(ShardInfo pdmShardInfo)
{
    MojLogTrace(s_log);
    MojLogDebug(s_log, "Shard engine notified about new shard");
    MojAssert(m_shardEngine->m_mediaLinkManager.get());

    MojErr err;
    bool found;
    ShardInfo databaseShardInfo;

    // Inside shardInfo we have only filled deviceId deviceUri mountPath MojString deviceName;

    err = m_shardEngine->getByDeviceUuid(pdmShardInfo.deviceId, databaseShardInfo, found);
    MojErrCheck(err);

    if (found) {    // shard already registered in database
        databaseShardInfo.deviceUri = pdmShardInfo.deviceUri;
        databaseShardInfo.deviceName = pdmShardInfo.deviceName;
        databaseShardInfo.mountPath = pdmShardInfo.mountPath;
        databaseShardInfo.active = pdmShardInfo.active;

        err = m_shardEngine->update(databaseShardInfo);
        MojErrCheck(err);
    } else {        // not found in database
        err = m_shardEngine->allocateId(pdmShardInfo.deviceId, databaseShardInfo.id);
        MojErrCheck(err);
        MojLogDebug(s_log, _T("shardEngine for device %s generated shard id: %d"), databaseShardInfo.deviceId.data(), databaseShardInfo.id);

        databaseShardInfo.deviceId = pdmShardInfo.deviceId;
        databaseShardInfo.deviceUri = pdmShardInfo.deviceUri;
        databaseShardInfo.deviceName = pdmShardInfo.deviceName;
        databaseShardInfo.mountPath = pdmShardInfo.mountPath;
        databaseShardInfo.active = pdmShardInfo.active;
        // databaseShardInfo.transient = true; what setup by default?

        err = m_shardEngine->put(databaseShardInfo);
        MojErrCheck(err);
    }
    MojLogDebug(s_log, _T("updated shard info"));

    MojLogDebug(s_log, _T("Run softlink logic"));
    if (databaseShardInfo.active) {  // inseted media
        m_shardEngine->m_mediaLinkManager->createLink(databaseShardInfo);
    } else {     // removed media
        m_shardEngine->m_mediaLinkManager->removeLink(databaseShardInfo);
    }

    return MojErrNone;
}

MojErr MojDbShardEngine::convertId (const MojUInt32 i_id, MojString& o_id_base64)
{
    MojErr err = MojErrNone;
    MojBuffer buf;
    MojDataWriter writer(buf);

    err = writer.writeUInt32(i_id);
    MojErrCheck(err);

    MojVector<MojByte> byteVec;
    err = buf.toByteVec(byteVec);
    MojErrCheck(err);
    MojString str;
    err = o_id_base64.base64Encode(byteVec, false);
    MojErrCheck(err);

    return err;
}

MojErr MojDbShardEngine::convertId (const MojString& i_id_base64, MojUInt32& o_id)
{
    MojErr err = MojErrNone;
    MojVector<MojByte> idVec;
    err = i_id_base64.base64Decode(idVec);
    MojErrCheck(err);

    // extract first 32bits of _id as shard id in native order
    MojDataReader reader(idVec.begin(), idVec.size());

    err = reader.readUInt32(o_id);
    MojErrCheck(err);

    return err;
}
