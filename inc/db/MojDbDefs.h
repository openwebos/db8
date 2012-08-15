/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJDBDEFS_H_
#define MOJDBDEFS_H_

#include "core/MojCoreDefs.h"

class MojDb;
class MojDbBatch;
class MojDbClient;
class MojDbCursor;
class MojDbIndex;
class MojDbKey;
class MojDbKeyRange;
class MojDbKind;
class MojDbKindEngine;
class MojDbObjectItem;
class MojDbPermissionEngine;
class MojDbPropExtractor;
class MojDbPutHandler;
class MojDbQuery;
class MojDbQueryFilter;
class MojDbQueryPlan;
class MojDbQuotaEngine;
class MojDbReq;
class MojDbSearchCursor;
class MojDbServiceClient;
class MojDbServiceClientBatch;
class MojDbServiceHandler;
class MojDbTextCollator;
class MojDbTextTokenizer;
class MojDbWatcher;

class MojDbStorageCursor;
class MojDbStorageDatabase;
class MojDbStorageEngine;
class MojDbStorageIndex;
class MojDbStorageItem;
class MojDbStorageQuery;
class MojDbStorageSeq;
class MojDbStorageTxn;

enum MojDbOp {
	OpNone 		= 0,
	OpUndefined = (1 << 0),
	OpCreate 	= (1 << 1),
	OpRead 		= (1 << 2),
	OpUpdate 	= (1 << 3),
	OpDelete 	= (1 << 4),
	OpExtend 	= (1 << 5),
	OpKindUpdate = (1 << 6)
};

enum MojDbCollationStrength {
	MojDbCollationInvalid,
	MojDbCollationPrimary,
	MojDbCollationSecondary,
	MojDbCollationTertiary,
	MojDbCollationQuaternary,
	MojDbCollationIdentical
};

#endif /* MOJDBDEFS_H_ */
