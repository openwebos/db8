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


#include "MojDbPerfTest.h"
#include "db/MojDb.h"
#include "core/MojTime.h"

#include <time.h>
#include <stdlib.h>

const MojChar* MojDbPerfTest::s_lastNames[] = {
	_T("Smith"), _T("Johnson"), _T("Williams"), _T("Jones"), _T("Brown"),
	_T("Davis"), _T("Miller"), _T("Wilson"), _T("Moore"), _T("Taylor"),
	_T("Anderson"), _T("Thomas"), _T("Jackson"), _T("White"), _T("Harris"),
	_T("Martin"), _T("Thompson"), _T("Garcia"), _T("Martinez"), _T("Robinson"),
	_T("Clark"), _T("Rodriguez"), _T("Lewis"), _T("Lee"), _T("Walker"),
	_T("Hall"), _T("Allen"), _T("Young"), _T("Hernandez"), _T("King"),
	_T("Wright"), _T("Lopez"), _T("Hill"), _T("Scott"), _T("Green"),
	_T("Adams"), _T("Baker"), _T("Gonzalez"), _T("Nelson"), _T("Carter"),
	_T("Mitchell"), _T("Perez"), _T("Roberts"), _T("Turner"), _T("Phillips"),
	_T("Campbell"), _T("Parker"), _T("Evans"), _T("Edwards"), _T("Collins"), NULL
};

const MojChar* MojDbPerfTest::s_firstNames[] = {
	_T("James"), _T("Mary"), _T("John"), _T("Patricia"), _T("Robert"),
	_T("Linda"), _T("Michael"), _T("Barbara"), _T("William"), _T("Elizabeth"),
	_T("David"), _T("Jennifer"), _T("Richard"), _T("Maria"), _T("Charles"),
	_T("Susan"), _T("Joseph"), _T("Margaret"), _T("Thomas"), _T("Dorothy"),
	_T("Christopher"), _T("Lisa"), _T("Daniel"), _T("Nancy"), _T("Paul"),
	_T("Karen"), _T("Mark"), _T("Betty"), _T("Donald"), _T("Helen"),
	_T("George"), _T("Sandra"), _T("Kenneth"), _T("Donna"), _T("Steven"),
	_T("Carol"), _T("Edward"), _T("Ruth"), _T("Brian"), _T("Sharon"),
	_T("Ronald"), _T("Michelle"), _T("Anthony"), _T("Laura"), _T("Kevin"),
	_T("Sarah"), _T("Jason"), _T("Kimberly"), _T("Jeff"), _T("Deborah"), NULL
};

const MojChar* const MojDbPerfTest::MojPerfSmKindId = _T("SmKind:1");
const MojChar* const MojDbPerfTest::MojPerfSmKindStr =
	_T("{\"id\":\"SmKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first\",\"props\":[{\"name\":\"first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedKindId = _T("MedKind:1");
const MojChar* const MojDbPerfTest::MojPerfMedKindStr =
	_T("{\"id\":\"MedKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first\",\"props\":[{\"name\":\"first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgKindId = _T("LgKind:1");
const MojChar* const MojDbPerfTest::MojPerfLgKindStr =
	_T("{\"id\":\"LgKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first\",\"props\":[{\"name\":\"first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedNestedKindId = _T("MedNestedKind:1");
const MojChar* const MojDbPerfTest::MojPerfMedNestedKindStr =
	_T("{\"id\":\"MedNestedKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"name_first\",\"props\":[{\"name\":\"name.first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgNestedKindId = _T("LgNestedKind:1");
const MojChar* const MojDbPerfTest::MojPerfLgNestedKindStr =
	_T("{\"id\":\"LgNestedKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"med_name_first\",\"props\":[{\"name\":\"med.name.first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedArrayKindId = _T("MedArrayKind:1");
const MojChar* const MojDbPerfTest::MojPerfMedArrayKindStr =
	_T("{\"id\":\"MedArrayKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"names\",\"props\":[{\"name\":\"names\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgArrayKindId = _T("LgArrayKind:1");
const MojChar* const MojDbPerfTest::MojPerfLgArrayKindStr =
	_T("{\"id\":\"LgArrayKind:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"names\",\"props\":[{\"name\":\"names\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfSmKind2Id = _T("SmKind2:1");
const MojChar* const MojDbPerfTest::MojPerfSmKind2Str =
	_T("{\"id\":\"SmKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first_last\",\"props\":[{\"name\":\"first\"},{\"name\":\"last\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedKind2Id = _T("MedKind2:1");
const MojChar* const MojDbPerfTest::MojPerfMedKind2Str =
	_T("{\"id\":\"MedKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first_last\",\"props\":[{\"name\":\"first\"},{\"name\":\"last\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgKind2Id = _T("LgKind2:1");
const MojChar* const MojDbPerfTest::MojPerfLgKind2Str =
	_T("{\"id\":\"LgKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first_last\",\"props\":[{\"name\":\"first\"},{\"name\":\"last\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedNestedKind2Id = _T("MedNestedKind2:1");
const MojChar* const MojDbPerfTest::MojPerfMedNestedKind2Str =
	_T("{\"id\":\"MedNestedKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first_last\", \"props\":[{\"name\":\"name.first\"},{\"name\":\"name.last\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgNestedKind2Id = _T("LgNestedKind2:1");
const MojChar* const MojDbPerfTest::MojPerfLgNestedKind2Str =
	_T("{\"id\":\"LgNestedKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"first_last\",\"props\":[{\"name\":\"med.name.first\"},{\"name\":\"med.name.last\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfMedArrayKind2Id = _T("MedArrayKind2:1");
const MojChar* const MojDbPerfTest::MojPerfMedArrayKind2Str =
	_T("{\"id\":\"MedArrayKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"names_first\",\"props\":[{\"name\":\"names\"},{\"name\":\"first\"}]}]}");
const MojChar* const MojDbPerfTest::MojPerfLgArrayKind2Id = _T("LgArrayKind2:1");
const MojChar* const MojDbPerfTest::MojPerfLgArrayKind2Str =
	_T("{\"id\":\"LgArrayKind2:1\",")
	_T("\"owner\":\"mojodb.admin\",")
	_T("\"indexes\":[{\"name\":\"names_first\",\"props\":[{\"name\":\"names\"},{\"name\":\"first\"}]}]}");

const MojChar* const MojDbPerfTest::MojPerfSmKindExtraIndex =
		_T("{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfMedKindExtraIndex =
		_T("{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfLgKindExtraIndex =
		_T("{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfMedNestedKindExtraIndex =
                _T("{\"name\":\"name_timestamp\",\"props\":[{\"name\":\"name.timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfLgNestedKindExtraIndex =
                _T("{\"name\":\"med_name_timestamp\",\"props\":[{\"name\":\"med.name.timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfMedArrayKindExtraIndex =
                _T("{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}");
const MojChar* const MojDbPerfTest::MojPerfLgArrayKindExtraIndex =
                _T("{\"name\":\"timestamp\",\"props\":[{\"name\":\"timestamp\"}]}");

MojDbPerfTest::MojDbPerfTest(const MojChar* name)
: MojTestCase(name), m_lazySync(false)
{
    m_lazySync = getenv("lazySync") ? true : false;
}


MojErr MojDbPerfTest::putKinds(MojDb& db, MojUInt64& putKindTime)
{
	MojObject kind1;
	MojErr err = kind1.fromJson(MojPerfSmKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind1);
	MojTestErrCheck(err);

	MojObject kind2;
	err = kind2.fromJson(MojPerfMedKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind2);
	MojTestErrCheck(err);

	MojObject kind3;
	err = kind3.fromJson(MojPerfLgKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind3);
	MojTestErrCheck(err);

	MojObject kind4;
	err = kind4.fromJson(MojPerfMedNestedKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind4);
	MojTestErrCheck(err);

	MojObject kind5;
	err = kind5.fromJson(MojPerfLgNestedKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind5);
	MojTestErrCheck(err);

	MojObject kind6;
	err = kind6.fromJson(MojPerfMedArrayKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind6);
	MojTestErrCheck(err);

	MojObject kind7;
	err = kind7.fromJson(MojPerfLgArrayKindStr);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind7);
	MojTestErrCheck(err);

	MojObject kind8;
	err = kind8.fromJson(MojPerfSmKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind8);
	MojTestErrCheck(err);

	MojObject kind9;
	err = kind9.fromJson(MojPerfMedKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind9);
	MojTestErrCheck(err);

	MojObject kind10;
	err = kind10.fromJson(MojPerfLgKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind10);
	MojTestErrCheck(err);

	MojObject kind11;
	err = kind11.fromJson(MojPerfMedNestedKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind11);
	MojTestErrCheck(err);

	MojObject kind12;
	err = kind12.fromJson(MojPerfLgNestedKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind12);
	MojTestErrCheck(err);

	MojObject kind13;
	err = kind13.fromJson(MojPerfMedArrayKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind13);
	MojTestErrCheck(err);

	MojObject kind14;
	err = kind14.fromJson(MojPerfLgArrayKind2Str);
	MojTestErrCheck(err);
	err = timePutKind(db, putKindTime, kind14);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::delKinds(MojDb& db)
{
	MojString kind1Id;
	MojErr err = kind1Id.assign(MojPerfSmKindId);
	MojTestErrCheck(err);
	bool found = false;
	err = db.delKind(kind1Id, found);
	MojTestErrCheck(err);

	MojString kind2Id;
   err = kind2Id.assign(MojPerfMedKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind2Id, found);
	MojTestErrCheck(err);

	MojString kind3Id;
	err = kind3Id.assign(MojPerfLgKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind3Id, found);
	MojTestErrCheck(err);

	MojString kind4Id;
	err = kind4Id.assign(MojPerfMedNestedKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind4Id, found);
	MojTestErrCheck(err);

	MojString kind5Id;
	err = kind5Id.assign(MojPerfLgNestedKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind5Id, found);
	MojTestErrCheck(err);

	MojString kind6Id;
	err = kind6Id.assign(MojPerfMedArrayKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind6Id, found);
	MojTestErrCheck(err);

	MojString kind7Id;
	err = kind7Id.assign(MojPerfLgArrayKindId);
	MojTestErrCheck(err);
	err = db.delKind(kind7Id, found);
	MojTestErrCheck(err);

	MojString kind8Id;
	err = kind8Id.assign(MojPerfSmKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind8Id, found);
	MojTestErrCheck(err);

	MojString kind9Id;
	err = kind9Id.assign(MojPerfMedKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind9Id, found);
	MojTestErrCheck(err);

	MojString kind10Id;
	err = kind10Id.assign(MojPerfLgKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind10Id, found);
	MojTestErrCheck(err);

	MojString kind11Id;
	err = kind11Id.assign(MojPerfMedNestedKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind11Id, found);
	MojTestErrCheck(err);

	MojString kind12Id;
	err = kind12Id.assign(MojPerfLgNestedKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind12Id, found);
	MojTestErrCheck(err);

	MojString kind13Id;
	err = kind13Id.assign(MojPerfMedArrayKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind13Id, found);
	MojTestErrCheck(err);

	MojString kind14Id;
	err = kind14Id.assign(MojPerfLgArrayKind2Id);
	MojTestErrCheck(err);
	err = db.delKind(kind14Id, found);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::timePutKind(MojDb& db, MojUInt64& putKindTime, MojObject kind)
{
	timespec startTime;
	startTime.tv_nsec = 0;
	startTime.tv_sec = 0;
	timespec endTime;
	endTime.tv_nsec = 0;
	endTime.tv_sec = 0;
	clock_gettime(CLOCK_REALTIME, &startTime);
	MojErr err = db.putKind(kind);
	MojTestErrCheck(err);
	clock_gettime(CLOCK_REALTIME, &endTime);
	putKindTime += timeDiff (startTime, endTime);

	return MojErrNone;
}


MojErr MojDbPerfTest::createSmallObj(MojObject& obj, MojUInt64 i)
{
	MojErr err = obj.putString(_T("first"), s_firstNames[i % 50]);
	MojTestErrCheck(err);
	err = obj.putString(_T("last"), s_lastNames[i % 50]);
	MojTestErrCheck(err);
	MojTime time;
	err = MojGetCurrentTime(time);
	MojTestErrCheck(err);
	err = obj.put(_T("timestamp"), time.millisecs());
	MojTestErrCheck(err);
	err = obj.putInt(_T("flags"), i);
	MojTestErrCheck(err);
	err = obj.putBool(_T("unread"), (i % 2 == 0));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createMedObj(MojObject& obj, MojUInt64 i)
{
	MojErr err = createSmallObj(obj, i);
	MojTestErrCheck(err);
	err = obj.putInt(_T("accountId"), i*10000);
	MojTestErrCheck(err);
	err = obj.putString(_T("displayName"), s_lastNames[i % 50]);
	MojTestErrCheck(err);
	err = obj.putString(_T("url"), _T("http://www.google.com"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("count"), i * 3);
	MojTestErrCheck(err);
	err = obj.putString(_T("reminder"), _T("you owe me money"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createLargeObj(MojObject& obj, MojUInt64 i)
{
	MojErr err = createMedObj(obj, i);
	MojTestErrCheck(err);
	err = obj.putString(_T("summary"), _T("a quick brown fox jumps over the lazy dog"));
	MojTestErrCheck(err);
	err = obj.putString(_T("subject"), _T("The five boxing wizards jump quickly"));
	MojTestErrCheck(err);
	err = obj.putString(_T("three"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("four"), i * 4);
	MojTestErrCheck(err);
	err = obj.putString(_T("five"), _T("FIVE"));
	MojTestErrCheck(err);
	err = obj.putBool(_T("six"), (i % 6) == 0);
	MojTestErrCheck(err);
	err = obj.putString(_T("seven"), _T("hello"));
	MojTestErrCheck(err);
	err = obj.putString(_T("eight"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("nine"), i * 9);
	MojTestErrCheck(err);
	err = obj.putString(_T("ten"), _T("TEN"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createMedNestedObj(MojObject& obj, MojUInt64 i)
{
	MojObject name;
	MojErr err = createSmallObj(name, i);
	MojTestErrCheck(err);

	err = obj.put(_T("name"), name);
	MojTestErrCheck(err);
	err = obj.putInt(_T("accountId"), i*10000);
	MojTestErrCheck(err);
	err = obj.putString(_T("displayName"), s_lastNames[i % 50]);
	MojTestErrCheck(err);
	err = obj.putString(_T("url"), _T("http://www.google.com"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("count"), i * 3);
	MojTestErrCheck(err);
	err = obj.putString(_T("reminder"), _T("you owe me money"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createLargeNestedObj(MojObject& obj, MojUInt64 i)
{
	MojObject med;
	MojErr err = createMedNestedObj(med, i);
	MojTestErrCheck(err);

	err = obj.put(_T("med"), med);
	MojTestErrCheck(err);
	err = obj.putString(_T("summary"), _T("a quick brown fox jumps over the lazy dog"));
	MojTestErrCheck(err);
	err = obj.putString(_T("subject"), _T("The five boxing wizards jump quickly"));
	MojTestErrCheck(err);
	err = obj.putString(_T("three"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("four"), i * 4);
	MojTestErrCheck(err);
	err = obj.putString(_T("five"), _T("FIVE"));
	MojTestErrCheck(err);
	err = obj.putBool(_T("six"), (i % 6) == 0);
	MojTestErrCheck(err);
	err = obj.putString(_T("seven"), _T("hello"));
	MojTestErrCheck(err);
	err = obj.putString(_T("eight"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("nine"), i * 9);
	MojTestErrCheck(err);
	err = obj.putString(_T("ten"), _T("TEN"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createMedArrayObj(MojObject& obj, MojUInt64 i)
{
	MojErr err = createSmallObj(obj, i);
	MojTestErrCheck(err);

	err = obj.putInt(_T("accountId"), i*10000);
	MojTestErrCheck(err);
	err = obj.putString(_T("displayName"), s_lastNames[i % 50]);
	MojTestErrCheck(err);
	err = obj.putString(_T("url"), _T("http://www.google.com"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("count"), i * 3);
	MojTestErrCheck(err);
	err = obj.putString(_T("reminder"), _T("you owe me money"));
	MojTestErrCheck(err);

	MojObject array;
	MojUInt64 j = i;
	for(; j < (i + 5); j++) {
		err = array.pushString(s_firstNames[j % 50]);
		MojTestErrCheck(err);
	}

	err = obj.put(_T("names"), array);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::createLargeArrayObj(MojObject& obj, MojUInt64 i)
{
	MojErr err = createMedArrayObj(obj, i);
	MojTestErrCheck(err);

	err = obj.putString(_T("summary"), _T("a quick brown fox jumps over the lazy dog"));
	MojTestErrCheck(err);
	err = obj.putString(_T("subject"), _T("The five boxing wizards jump quickly"));
	MojTestErrCheck(err);
	err = obj.putString(_T("three"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("four"), i * 4);
	MojTestErrCheck(err);
	err = obj.putString(_T("five"), _T("FIVE"));
	MojTestErrCheck(err);
	err = obj.putBool(_T("six"), (i % 6) == 0);
	MojTestErrCheck(err);
	err = obj.putString(_T("seven"), _T("hello"));
	MojTestErrCheck(err);
	err = obj.putString(_T("eight"), _T("THREE"));
	MojTestErrCheck(err);
	err = obj.putInt(_T("nine"), i * 9);
	MojTestErrCheck(err);
	err = obj.putString(_T("ten"), _T("TEN"));
	MojTestErrCheck(err);

	MojObject array;
	MojUInt64 j = i;
	for(; j < (i + 5); j++) {
		err = array.pushString(s_lastNames[j % 50]);
		MojTestErrCheck(err);
	}

	err = obj.put(_T("lastNames"), array);
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbPerfTest::fileWrite(MojFile& file, MojString buf)
{
	MojSize size;
	return file.writeString(buf, size);
}

MojUInt64 MojDbPerfTest::timeDiff(timespec start, timespec end)
{
	timespec temp;
	temp.tv_nsec = 0;
	temp.tv_sec = 0;
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp.tv_sec * 1000000000 + temp.tv_nsec;
}

MojObject MojDbPerfTest::lazySyncConfig() const
{
    MojObject conf;
    conf.fromJson(_T("{ \"db\": { \"sync\": 2 } }"));
    return (conf);
}

