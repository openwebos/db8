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


#include "db/MojDbServiceDefs.h"
#include "core/MojService.h"

// property keys
const MojChar* const MojDbServiceDefs::BytesKey = _T("maxTempBytes");
const MojChar* const MojDbServiceDefs::CallerKey = _T("caller");
const MojChar* const MojDbServiceDefs::CountKey = _T("count");
const MojChar* const MojDbServiceDefs::CountryCodeKey = _T("countryCode");
const MojChar* const MojDbServiceDefs::CreateKey = _T("create");
const MojChar* const MojDbServiceDefs::DeleteKey = _T("delete");
const MojChar* const MojDbServiceDefs::DeletedRevKey = _T("deletedRev");
const MojChar* const MojDbServiceDefs::DevicesKey = _T("devices");
const MojChar* const MojDbServiceDefs::DescriptionKey = _T("description");
const MojChar* const MojDbServiceDefs::DirKey = _T("tempDir");
const MojChar* const MojDbServiceDefs::ExtendKey = _T("extend");
const MojChar* const MojDbServiceDefs::FilesKey = _T("files");
const MojChar* const MojDbServiceDefs::FiredKey = _T("fired");
const MojChar* const MojDbServiceDefs::FullKey = _T("full");
const MojChar* const MojDbServiceDefs::HasMoreKey = _T("hasMore");
const MojChar* const MojDbServiceDefs::IdKey = _T("id");
const MojChar* const MojDbServiceDefs::IdsKey = _T("ids");
const MojChar* const MojDbServiceDefs::IgnoreMissingKey = _T("ignoreMissing");
const MojChar* const MojDbServiceDefs::IncludeDeletedKey = _T("incDel");
const MojChar* const MojDbServiceDefs::IncrementalKey = _T("incrementalKey");
const MojChar* const MojDbServiceDefs::KeysKey = _T("keys");
const MojChar* const MojDbServiceDefs::KindKey = _T("kind");
const MojChar* const MojDbServiceDefs::KindNameKey = _T("incrementalKey");
const MojChar* const MojDbServiceDefs::LanguageCodeKey = _T("languageCode");
const MojChar* const MojDbServiceDefs::LocaleKey = _T("locale");
const MojChar* const MojDbServiceDefs::QueryKey = _T("query");
const MojChar* const MojDbServiceDefs::MethodKey = _T("method");
const MojChar* const MojDbServiceDefs::NextKey = _T("next");
const MojChar* const MojDbServiceDefs::ObjectKey = _T("object");
const MojChar* const MojDbServiceDefs::ObjectsKey = _T("objects");
const MojChar* const MojDbServiceDefs::OperationsKey = _T("operations");
const MojChar* const MojDbServiceDefs::OwnerKey = _T("owner");
const MojChar* const MojDbServiceDefs::ParamsKey = _T("params");
const MojChar* const MojDbServiceDefs::PathKey = _T("path");
const MojChar* const MojDbServiceDefs::PermissionsKey = _T("permissions");
const MojChar* const MojDbServiceDefs::PostBackupKey = _T("postBackup");
const MojChar* const MojDbServiceDefs::PostRestoreKey = _T("postRestore");
const MojChar* const MojDbServiceDefs::PreBackupKey = _T("preBackup");
const MojChar* const MojDbServiceDefs::PreRestoreKey = _T("preRestore");
const MojChar* const MojDbServiceDefs::PrevKey = _T("prev");
const MojChar* const MojDbServiceDefs::ProceedKey = _T("proceed");
const MojChar* const MojDbServiceDefs::PropsKey = _T("props");
const MojChar* const MojDbServiceDefs::PurgeKey = _T("purge");
const MojChar* const MojDbServiceDefs::QuotasKey = _T("quotas");
const MojChar* const MojDbServiceDefs::ReadKey = _T("read");
const MojChar* const MojDbServiceDefs::ResponsesKey = _T("responses");
const MojChar* const MojDbServiceDefs::ResultsKey = _T("results");
const MojChar* const MojDbServiceDefs::RevKey = _T("rev");
const MojChar* const MojDbServiceDefs::ShardIdKey = _T("shardId");
const MojChar* const MojDbServiceDefs::SizeKey = _T("size");
const MojChar* const MojDbServiceDefs::ServiceKey = _T("service");
const MojChar* const MojDbServiceDefs::SubscribeKey = _T("subscribe");
const MojChar* const MojDbServiceDefs::TypeKey = _T("type");
const MojChar* const MojDbServiceDefs::UpdateKey = _T("update");
const MojChar* const MojDbServiceDefs::UsedKey = _T("used");
const MojChar* const MojDbServiceDefs::VersionKey = _T("version");
const MojChar* const MojDbServiceDefs::VerifyKey = _T("verify");
const MojChar* const MojDbServiceDefs::WarningsKey = _T("warnings");
const MojChar* const MojDbServiceDefs::WatchKey = _T("watch");
const MojChar* const MojDbServiceDefs::WindowKey = _T("window");
const MojChar* const MojDbServiceDefs::ListActiveMediaKey = _T("listActiveMedia");
const MojChar* const MojDbServiceDefs::ShardInfoKey = _T("shardInfo");
const MojChar* const MojDbServiceDefs::ShardKindKey = _T("shardKind");
const MojChar* const MojDbServiceDefs::SetShardModeKey = _T("setShardMode");
const MojChar* const MojDbServiceDefs::RegisterMediaKey = _T("registerMedia");
const MojChar* const MojDbServiceDefs::MediaKey = _T("media");
const MojChar* const MojDbServiceDefs::TransientKey = _T("transient");
// property values
const MojChar* const MojDbServiceDefs::AllowValue = _T("allow");
const MojChar* const MojDbServiceDefs::DenyValue = _T("deny");
// method names
const MojChar* const MojDbServiceDefs::BatchMethod = _T("batch");
const MojChar* const MojDbServiceDefs::CompactMethod = _T("compact");
const MojChar* const MojDbServiceDefs::DelMethod = _T("del");
const MojChar* const MojDbServiceDefs::DelKindMethod = _T("delKind");
const MojChar* const MojDbServiceDefs::DumpMethod = _T("dump");
const MojChar* const MojDbServiceDefs::FindMethod = _T("find");
const MojChar* const MojDbServiceDefs::GetMethod = _T("get");
const MojChar* const MojDbServiceDefs::GetPermissionsMethod = _T("getPermissions");
const MojChar* const MojDbServiceDefs::LoadMethod = _T("load");
const MojChar* const MojDbServiceDefs::MergeMethod = _T("merge");
const MojChar* const MojDbServiceDefs::MergePutMethod = _T("mergePut");
const MojChar* const MojDbServiceDefs::PostBackupMethod = _T("postBackup");
const MojChar* const MojDbServiceDefs::PostRestoreMethod = _T("postRestore");
const MojChar* const MojDbServiceDefs::PreBackupMethod = _T("preBackup");
const MojChar* const MojDbServiceDefs::PreRestoreMethod = _T("preRestore");
const MojChar* const MojDbServiceDefs::PurgeMethod = _T("purge");
const MojChar* const MojDbServiceDefs::PurgeStatusMethod = _T("purgeStatus");
const MojChar* const MojDbServiceDefs::PutMethod = _T("put");
const MojChar* const MojDbServiceDefs::PutKindMethod = _T("putKind");
const MojChar* const MojDbServiceDefs::PutPermissionsMethod = _T("putPermissions");
const MojChar* const MojDbServiceDefs::PutQuotasMethod = _T("putQuotas");
const MojChar* const MojDbServiceDefs::QuotaStatsMethod = _T("quotaStats");
const MojChar* const MojDbServiceDefs::ReserveIdsMethod = _T("reserveIds");
const MojChar* const MojDbServiceDefs::ScheduledPurgeMethod = _T("scheduledPurge");
const MojChar* const MojDbServiceDefs::SearchMethod = _T("search");
const MojChar* const MojDbServiceDefs::SpaceCheckMethod = _T("spaceCheck");
const MojChar* const MojDbServiceDefs::ScheduledSpaceCheckMethod = _T("scheduledSpaceCheck");
const MojChar* const MojDbServiceDefs::StatsMethod = _T("stats");
const MojChar* const MojDbServiceDefs::WatchMethod = _T("watch");
const MojChar* const MojDbServiceDefs::QuotaCheckMethod = _T("quotaCheck");
const MojChar* const MojDbServiceDefs::ListActiveMediaMethod = _T("listActiveMedia");
const MojChar* const MojDbServiceDefs::ShardInfoMethod = _T("shardInfo");
const MojChar* const MojDbServiceDefs::ShardKindMethod = _T("shardKind");
const MojChar* const MojDbServiceDefs::SetShardModeMethod = _T("setShardMode");
const MojChar* const MojDbServiceDefs::RegisterMediaMethod = _T("registerMedia");
// service name
const MojChar* const MojDbServiceDefs::Category = MojService::DefaultCategory;
const MojChar* const MojDbServiceDefs::InternalCategory = _T("/internal");
const MojChar* const MojDbServiceDefs::ServiceName = _T("com.palm.db");
const MojChar* const MojDbServiceDefs::TempServiceName = _T("com.palm.tempdb");
const MojChar* const MojDbServiceDefs::MediaServiceName = _T("com.webos.mediadb");
const MojChar* const MojDbServiceDefs::PDMServiceName = _T("com.webos.service.attachedstoragemanager");
const MojChar* const MojDbServiceDefs::PDMClientName = _T("com.palm.mojodbclient");
const MojChar* const MojDbServiceDefs::DefaultMediaLinkPath = _T("/var/run/db8/mountpoints");
