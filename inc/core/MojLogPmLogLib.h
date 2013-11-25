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


#ifndef MOJLOGPMLOGLIB_H_
#define MOJLOGPMLOGLIB_H_

#include "PmLogLib.h"

/* Logging ********
 * The parameters needed are
 * msgid - unique message id
 * kvcount - count for key-value pairs
 * ... - key-value pairs and free text. key-value pairs are formed using PMLOGKS or PMLOGKFV e.g.)
 * LOG_CRITICAL(msgid, 2, PMLOGKS("key1", "value1"), PMLOGKFV("key2", "%d", value2), "free text message");
 */
#define LOG_CRITICAL(msgid, kvcount, ...) \
PmLogCritical(getactivitymanagercontext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_ERROR(msgid, kvcount, ...) \
PmLogError(getactivitymanagercontext(), msgid, kvcount,##__VA_ARGS__)

#define LOG_WARNING(msgid, kvcount, ...) \
PmLogWarning(getactivitymanagercontext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_INFO(msgid, kvcount, ...) \
PmLogInfo(getactivitymanagercontext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_DEBUG(...) \
PmLogDebug(getactivitymanagercontext(), ##__VA_ARGS__)

#define LOG_TRACE(...) \
PMLOG_TRACE(__VA_ARGS__);

#define MSGID_ERROR_CALL               "ERROR_CALL"
#define MSGID_MESSAGE_CALL             "MESSAGE_CALL"
#define MSGID_LUNA_SERVICE_DB_OPEN     "LUNA_SERVICE_DB_OPEN"
#define MSGID_LUNA_ERROR_RESPONSE      "LUNA_ERROR_RESPONSE"
#define MSGID_LEVEL_DB_ENGINE_ERROR    "LEVEL_DB_ENGINE_ERROR"
#define MSGID_DB_ADMIN_ERROR           "DB_ADMIN_ERROR"
#define MSGID_DB_KIND_ENGINE_ERROR     "DB_KIND_ENGINE_ERROR"
#define MSGID_DB_SERVICE_ERROR         "DB_SERVICE_ERROR"
#define MSGID_DB_ERROR                 "DB_ERROR"
#define MSGID_MOJ_SERVICE_WARNING      "MOJ_SERVICE_WARNING"
#define MSGID_DB_BERKLEY_TXN_WARNING   "DB_BERKLEY_TXN_WARNING"
#define MSGID_LUNA_SERVICE_WARNING     "LUNA_SERVICE_WARNING"
#define MSGID_LEVEL_DB_WARNING         "LEVEL_DB_WARNING"
#define MSGID_MOJ_DB_WARNING           "MOJ_DB_WARNING"
#define MOJ_DB_KIND_WARNING            "DB_KIND_WARNING"
#define MSGID_MOJ_DB_CURSOR_WARNING    "MOJ_DB_CURSOR_WARNING"
#define MSGID_MOJ_DB_ADMIN_WARNING     "MOJ_DB_ADMIN_WARNING"
#define MSGID_MOJ_DB_INDEX_WARNING     "MOJ_DB_INDEX_WARNING"
#define MSGID_MOJ_DB_KIND_WARNING      "MOJ_DB_KIND_WARNING"
#define MSGID_MOJ_DB_MEDIALINK_WARNING "MOJ_DB_MEDIALINK_WARNING"
#define MSGID_MOJ_DB_SERVICE_WARNING   "MOJ_DB_SERVICE_WARNING"
#define MSGID_DB_SHARDENGINE_WARNING   "DB_SHARDENGINE_WARNING"

extern PmLogContext getactivitymanagercontext();

#endif /* MOJLOGPMLOGLIB_H_ */
