/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics, Inc.
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


#ifndef MOJDBPERFINDEXTEST_H_
#define MOJDBPERFINDEXTEST_H_

#include "MojDbPerfTest.h"

class MojDbPerfIndexTest: public MojTestCase
{
public:
	MojDbPerfIndexTest();

	virtual MojErr run();
	virtual void cleanup();

private:
	MojErr _runPhaseTest (MojDb& db, MojUInt32 i_phase_number);
	MojErr _generateNewRecords (MojDb& db, MojUInt32 i_number_records);
	MojErr _performThreeQueries (MojDb& db);
	MojErr _delete10PersentsRecords (MojDb& db);
	MojErr _change10PersentsRecords (MojDb& db);

	inline MojErr _generateStreetAddress (MojString& o_string);
	inline MojErr _generateCityName (MojString& o_string);
	inline MojErr _generateStateName (MojString& o_string);
	inline MojErr _generateZipCode (MojString& o_string);
	inline void   _generateLimitCode (MojObject& o_object);
	inline MojErr _generateValidDate (MojString& o_string, time_t& o_value);
	inline MojErr _add3monthToDate (MojString& o_string, time_t& i_value);

	MojErr _displayMessage (const MojChar* format, ...);
};

#endif /* MOJDBPERFINDEXTEST_H_ */
