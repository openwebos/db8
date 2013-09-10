/* @@@LICENSE
*
*  Copyright (c) 2009-2013 LG Electronics, Inc.
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


#include "db/MojDbServiceHandler.h"

#define MOJ_QUERY_UNCLOSED_SCHEMA \
	_T("{\"type\":\"object\",") \
	 _T("\"properties\":{") \
		 _T("\"select\":{\"type\":\"array\",\"optional\":true,\"items\":{\"type\":\"string\"}},") \
		 _T("\"from\":{\"type\":\"string\"},") \
		 _T("\"where\":{\"type\":\"array\",\"optional\":true,\"items\":{") \
			_T("\"type\":\"object\",") \
			 _T("\"properties\":{") \
				 _T("\"prop\":{\"type\":\"string\"},") \
				 _T("\"op\":{\"type\":\"string\",\"enum\":[\"<\",\"<=\",\"=\",\">=\",\">\",\"!=\",\"%\",\"?\"]},") \
				 _T("\"val\":{}},") \
                 _T("\"collate\":{\"type\":\"string\",\"optional\":true,\"enum\":[\"default\",\"primary\",\"secondary\",\"tertiary\",\"quaternary\",\"identical\"]}}},") \
		 _T("\"filter\":{\"type\":\"array\",\"optional\":true,\"items\":{") \
			_T("\"type\":\"object\",") \
			 _T("\"properties\":{") \
				 _T("\"prop\":{\"type\":\"string\"},") \
				 _T("\"op\":{\"type\":\"string\",\"enum\":[\"<\",\"<=\",\"=\",\">=\",\">\",\"!=\"]},") \
				 _T("\"val\":{}}}},") \
		 _T("\"orderBy\":{\"type\":\"string\",\"optional\":true},") \
		 _T("\"distinct\":{\"type\":\"string\",\"optional\":true},") \
		 _T("\"desc\":{\"type\":\"boolean\",\"optional\":true},") \
		 _T("\"incDel\":{\"type\":\"boolean\",\"optional\":true},") \
		 _T("\"limit\":{\"type\":\"integer\",\"optional\":true,\"minimum\":0,\"maximum\":500},") \
		 _T("\"page\":{\"type\":\"string\",\"optional\":true}},") \
	 _T("\"additionalProperties\":false")

#define MOJ_QUERY_SCHEMA \
	MOJ_QUERY_UNCLOSED_SCHEMA _T("}")

#define MOJ_OPTIONAL_QUERY_SCHEMA \
	MOJ_QUERY_UNCLOSED_SCHEMA _T(",\"optional\":true}")

#define MOJ_DEL_SCHEMA \
	_T("{\"type\":\"object\",") \
	  _T("\"properties\":{") \
		 _T("\"query\":") MOJ_OPTIONAL_QUERY_SCHEMA _T(",") \
		 _T("\"ids\":{\"type\":\"array\",\"optional\":true},") \
		 _T("\"purge\":{\"type\":\"boolean\",\"optional\":true}},") \
	  _T("\"additionalProperties\":false}")

#define MOJ_FIND_SCHEMA \
	_T("{\"type\":\"object\",") \
	 _T("\"properties\":{") \
		 _T("\"query\":") MOJ_QUERY_SCHEMA _T(",") \
		 _T("\"count\":{\"type\":\"boolean\",\"optional\":true},") \
		 _T("\"watch\":{\"type\":\"boolean\",\"optional\":true},") \
		 _T("\"subscribe\":{\"type\":\"boolean\",\"optional\":true}},") \
	 _T("\"additionalProperties\":false}")

#define MOJ_GET_SCHEMA 	\
	_T("{\"type\":\"object\",") \
	 _T("\"properties\":{\"ids\":{\"type\":\"array\"}},") \
	 _T("\"additionalProperties\":false}")

#define MOJ_MERGE_SCHEMA \
	_T("{\"type\":\"object\",") \
	  _T("\"properties\":{") \
		 _T("\"query\":") MOJ_OPTIONAL_QUERY_SCHEMA _T(",") \
		 _T("\"objects\":{\"type\":\"array\",\"items\":{\"type\":\"object\"},\"optional\":true},") \
 		 _T("\"ignoreMissing\":{\"type\":\"boolean\",\"optional\":true},") \
		 _T("\"props\":{\"type\":\"object\",\"optional\":true,\"requires\":\"query\"}},") \
	  _T("\"additionalProperties\":false}")

#define MOJ_PUT_SCHEMA \
	_T("{\"type\":\"object\",") \
	  _T("\"properties\":{") \
         _T("\"shardId\":{\"type\":\"string\",\"optional\":true},") \
		 _T("\"objects\":{\"type\":\"array\",\"items\":{\"type\":\"object\"}}},") \
	  _T("\"additionalProperties\":false}")

const MojChar* const MojDbServiceHandler::BatchSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{\"operations\":{\"type\":\"array\",\"items\":{\"type\":[")
		 _T("{\"type\":\"object\",\"properties\":{\"method\":{\"enum\":[\"del\"]},\"params\":") MOJ_DEL_SCHEMA _T("}},")
		 _T("{\"type\":\"object\",\"properties\":{\"method\":{\"enum\":[\"find\",\"search\"]},\"params\":") MOJ_FIND_SCHEMA _T("}},")
		 _T("{\"type\":\"object\",\"properties\":{\"method\":{\"enum\":[\"get\"]},\"params\":") MOJ_GET_SCHEMA _T("}},")
		 _T("{\"type\":\"object\",\"properties\":{\"method\":{\"enum\":[\"merge\"]},\"params\":") MOJ_MERGE_SCHEMA _T("}},")
		 _T("{\"type\":\"object\",\"properties\":{\"method\":{\"enum\":[\"put\"]},\"params\":") MOJ_PUT_SCHEMA _T("}}]}}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::StatsSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"kind\":{\"type\":\"string\",\"optional\":true},")
		 _T("\"verify\":{\"type\":\"boolean\",\"optional\":true}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::CompactSchema = StatsSchema;

const MojChar* const MojDbServiceHandler::DelSchema = MOJ_DEL_SCHEMA;

const MojChar* const MojDbServiceHandler::DelKindSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{\"id\":{\"type\":\"string\"}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::DumpSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"path\":{\"type\":\"string\"},")
		 _T("\"incDel\":{\"type\":\"boolean\",\"optional\":true}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::FindSchema = MOJ_FIND_SCHEMA;

const MojChar* const MojDbServiceHandler::GetSchema = MOJ_GET_SCHEMA;

const MojChar* const MojDbServiceHandler::LoadSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"path\":{\"type\":\"string\"}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::MergeSchema = MOJ_MERGE_SCHEMA;

const MojChar* const MojDbServiceHandler::PurgeSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"window\":{\"type\":\"integer\",\"optional\":true,\"minimum\":0}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::PurgeStatusSchema =
	_T("{\"type\":\"object\",")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::PutSchema = MOJ_PUT_SCHEMA;

const MojChar* const MojDbServiceHandler::PutKindSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"id\":{\"type\":\"string\",\"minimum\":3},")
		 _T("\"owner\":{\"type\":\"string\",\"minimum\":1},")
		 _T("\"sync\":{\"type\":\"boolean\",\"optional\":true},")
		 _T("\"extends\":{\"type\":\"array\",\"optional\":true,\"items\":{\"type\":\"string\",\"minimum\":1}},")
		 _T("\"schema\":{\"type\":\"object\",\"optional\":true},")
		 _T("\"indexes\":{\"type\":\"array\",\"optional\":true,\"items\":{")
			_T("\"type\":\"object\",")
			_T("\"properties\":{")
				 _T("\"name\":{\"type\":\"string\",\"minimum\":1},")
 				 _T("\"incDel\":{\"type\":\"boolean\",\"optional\":true},")
				 _T("\"props\":{\"type\":\"array\",\"items\":{")
					 _T("\"type\":\"object\",")
					 _T("\"properties\":{")
						 _T("\"name\":{\"type\":\"string\",\"minimum\":1},")
 						 _T("\"type\":{\"type\":\"string\",\"optional\":true,\"enum\":[\"single\",\"multi\"]},")
                         _T("\"collate\":{\"type\":\"string\",\"optional\":true,\"enum\":[\"default\",\"primary\",\"secondary\",\"tertiary\",\"quaternary\",\"identical\"]},")
						 _T("\"tokenize\":{\"type\":\"string\",\"optional\":true,\"enum\":[\"none\",\"default\",\"all\"]},")
						 _T("\"default\":{\"optional\":true},")
						 _T("\"include\":{\"type\":\"array\",\"optional\":true,\"items\":{")
							 _T("\"name\":{\"type\":\"string\",\"minimum\":1},")
							 _T("\"tokenize\":{\"type\":\"string\",\"optional\":true,\"enum\":[\"none\",\"default\",\"all\"]}}}}}}}}},")
		_T("\"revSets\":{\"type\":\"array\",\"optional\":true,\"items\":{")
			_T("\"type\":\"object\",")
			_T("\"properties\":{")
				 _T("\"props\":{\"type\":\"array\",\"items\":{")
					 _T("\"type\":\"object\",")
					 _T("\"properties\":{")
						 _T("\"name\":{\"type\":\"string\",\"minimum\":1}}}}}}}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::PutPermissionsSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"permissions\":{\"type\":\"array\",\"items\":{")
			_T("\"type\":\"object\",")
			_T("\"properties\":{")
				 _T("\"type\":{\"type\":\"string\"},")
 				 _T("\"caller\":{\"type\":\"string\"},")
				 _T("\"object\":{\"type\":\"string\"},")
 				 _T("\"operations\":{\"type\":\"object\",")
				 	_T("\"additionalProperties\":false,")
				 	_T("\"properties\":{")
				 		_T("\"read\":{\"enum\":[\"allow\",\"deny\"]},")
				 		_T("\"update\":{\"enum\":[\"allow\",\"deny\"]},")
				 		_T("\"create\":{\"enum\":[\"allow\",\"deny\"]},")
				 		_T("\"delete\":{\"enum\":[\"allow\",\"deny\"]},")
				 		_T("\"extend\":{\"enum\":[\"allow\",\"deny\"]}")
				  _T("}}}}}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::PutQuotasSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"quotas\":{\"type\":\"array\",\"items\":{")
			_T("\"type\":\"object\",")
			_T("\"properties\":{")
				 _T("\"owner\":{\"type\":\"string\"},")
 				 _T("\"size\":{\"type\":\"integer\"}}}}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::QuotaStatsSchema = StatsSchema;

const MojChar* const MojDbServiceHandler::ReserveIdsSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"count\":{\"type\":\"integer\",\"minimum\":0}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::SearchSchema = MojDbServiceHandler::FindSchema;

const MojChar* const MojDbServiceHandler::WatchSchema =
	_T("{\"type\":\"object\",")
	 _T("\"properties\":{")
		 _T("\"query\":") MOJ_QUERY_SCHEMA _T(",")
  		 _T("\"subscribe\":{\"type\":\"boolean\",\"optional\":true}},")
	 _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::ListActiveMediaSchema = MojDbServiceHandler::PurgeStatusSchema;

const MojChar* const MojDbServiceHandler::ShardInfoSchema =
    _T("{\"type\":\"object\",")
     _T("\"properties\":{")
         _T("\"shardId\":{\"type\":\"string\"}},")
     _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::ShardKindSchema =
    _T("{\"type\":\"object\",")
     _T("\"properties\":{")
         _T("\"shardId\":{\"type\":\"string\"},")
         _T("\"kind\":{\"type\":\"string\"}},")
     _T("\"additionalProperties\":false}");

const MojChar* const MojDbServiceHandler::SetShardModeSchema =
    _T("{\"type\":\"object\",")
     _T("\"properties\":{")
         _T("\"shardId\":{\"type\":\"string\"},")
         _T("\"transient\":{\"type\":\"boolean\"}},")
     _T("\"additionalProperties\":false}");

