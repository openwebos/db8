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


#ifndef MOJDBCURSORTXNTEST_H_
#define MOJDBCURSORTXNTEST_H_

#include "MojDbPerfTest.h"

class MojDbCursorTxnTest: public MojTestCase
{
public:
    MojDbCursorTxnTest();

    virtual MojErr run();
    virtual void cleanup();

private:

    MojErr _execDbAndTxn ( MojDb& db );
    MojErr _execTxnAndDeleted ( MojDb& db );
    MojErr _execTxnAndAdd ( MojDb& db );

    MojErr _registerKind ( MojDb& db, const MojChar* kindStr );
    MojErr _assignValue ( MojObject& obj, const MojChar* kindId, const MojChar* key, MojInt32 value );
    MojInt32 _printCursorValues ( MojDbCursor& cursor );


    MojErr _displayMessage ( const MojChar* format, ... );
};

#endif /* MOJDBCURSORTXNTEST_H_ */
