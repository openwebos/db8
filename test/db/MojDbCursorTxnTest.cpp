/* @@@LICENSE
*
* Copyright (c) 2009-2013 LG Electronics, Inc.
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

#include <stdlib.h>
#include "db/MojDbStorageEngine.h"

#include "MojDbCursorTxnTest.h"
#include "db/MojDb.h"

#ifdef MOJ_USE_BDB
#include "db-luna/MojDbBerkeleyFactory.h"
#include "db-luna/MojDbBerkeleyEngine.h"
#elif MOJ_USE_LDB
#include "db-luna/leveldb/MojDbLevelFactory.h"
#include "db-luna/leveldb/MojDbLevelEngine.h"
#else
#error "Doesn't specified database type. See macro MOJ_USE_BDB and MOJ_USE_LDB"
#endif

static const MojChar* const TestKind1Str =
    _T ( "{\"id\":\"Test1:1\"," )
    _T ( "\"owner\":\"mojodb.admin\"," )
    _T ( "\"indexes\":[{\"name\":\"idxtest1\",\"props\":[{\"name\":\"data1\"}]} ]}" );

static const MojChar* const TestKind2Str =
    _T ( "{\"id\":\"Test2:1\"," )
    _T ( "\"owner\":\"mojodb.admin\"," )
    _T ( "\"indexes\":[{\"name\":\"idxtest2\",\"props\":[{\"name\":\"data1\"}]} ]}" );

static const MojChar* const TestKind3Str =
    _T ( "{\"id\":\"Test3:1\"," )
    _T ( "\"owner\":\"mojodb.admin\"," )
    _T ( "\"indexes\":[{\"name\":\"idxtest3\",\"props\":[{\"name\":\"data1\"}]} ]}" );

MojDbCursorTxnTest::MojDbCursorTxnTest()
    : MojTestCase ( _T ( "MojDbCursorTxnTest" ) )
{
}

MojErr MojDbCursorTxnTest::run()
{
#ifdef MOJ_USE_BDB
    MojDbStorageEngine::setEngineFactory ( new MojDbBerkeleyFactory );
#elif MOJ_USE_LDB
    MojDbStorageEngine::setEngineFactory ( new MojDbLevelFactory );
#else
#error "Not defined engine type"
#endif
    MojDb db;
    // open
    MojErr err = db.open ( MojDbTestDir );
    MojTestErrCheck ( err );

    err = _execDbAndTxn ( db );
    MojTestErrCheck ( err );

    err = _execTxnAndDeleted ( db );
    MojTestErrCheck ( err );

    err = _execTxnAndAdd ( db );
    MojTestErrCheck ( err );

    err = db.close();
    MojTestErrCheck ( err );

    return MojErrNone;
}

MojErr MojDbCursorTxnTest::_registerKind ( MojDb& db, const MojChar* kindStr )
{
    MojObject obj;
    MojErr err = obj.fromJson ( kindStr );
    MojTestErrCheck ( err );
    err = db.putKind ( obj );
    MojTestErrCheck ( err );

    return err;
}

MojErr MojDbCursorTxnTest::_assignValue ( MojObject& obj, const MojChar* kindId, const MojChar* key, MojInt32 value )
{
    MojErr err = obj.putString ( MojDb::KindKey, kindId );
    MojTestErrCheck ( err );
    err = obj.put ( key, value );
    MojTestErrCheck ( err );

    return err;
}

MojInt32 MojDbCursorTxnTest::_printCursorValues ( MojDbCursor& cursor )
{
    MojInt32 count = 0;
    bool found;
    MojObject obj;
    for ( ;; )
    {
        found = false;
        MojErr err = cursor.get ( obj, found );
        MojTestErrCheck ( err );
        if ( !found )
            break;

        // print all
        MojInt32 value;
        MojObject vobject;
        err = obj.get ( _T ( "data1" ), value, found );

        if ( found )
        {
            _displayMessage ( _T ( "%d " ), value );
        }

        ++count;
    }

    return count;
}

/*
 * records exist in db and into txn
 */
MojErr MojDbCursorTxnTest::_execDbAndTxn ( MojDb& db )
{
    MojByte success = 1;
    MojObject obj;
    MojDbQuery query;

    _displayMessage ( _T ( "\n\tTest1: store sequence-> 1 2 3" ) );

    _registerKind ( db, TestKind1Str );

    // make query
    MojErr err = query.from ( _T ( "Test1:1" ) );
    MojTestErrCheck ( err );

    //define objects
    MojObject obj1;
    _assignValue ( obj1, _T ( "Test1:1" ), _T ( "data1" ), 1 );

    MojObject obj2;
    _assignValue ( obj2, _T ( "Test1:1" ), _T ( "data1" ), 2 );

    MojObject obj3;
    _assignValue ( obj3, _T ( "Test1:1" ), _T ( "data1" ), 3 );

    //put within transaction
    MojDbReq req;
    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    err = db.put ( obj1, MojDb::FlagNone, req );
    MojTestErrCheck ( err );
    err = db.put ( obj2, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    req.end ( true );

    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    err = db.put ( obj3, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    MojDbCursor cursor;
    err = query.where ( _T ( "data1" ), MojDbQuery::OpNotEq, 0 );
    MojTestErrCheck ( err );

    err = db.find ( query, cursor, req );
    MojTestErrCheck ( err );

    _displayMessage ( _T ( "\n\tread sequence-> " ) );

    if ( _printCursorValues ( cursor ) != 3 )
    {
        _displayMessage ( _T ( ", ==> Failed\n" ) );
        success = 0;
    }
    else
        _displayMessage ( _T ( ", ==> Ok\n" ) );

    cursor.close();
    req.end ( true );

    return ( success == 1 ) ? MojErrNone : MojErrTestFailed;
}

MojErr MojDbCursorTxnTest::_execTxnAndDeleted ( MojDb& db )
{
    MojByte success = 1;
    MojObject obj;
    MojDbQuery query;

    _displayMessage ( _T ( "\n\tTest2: store sequence-> 1 2 3" ) );

    _registerKind ( db, TestKind2Str );

    // make query
    MojErr err = query.from ( _T ( "Test2:1" ) );
    MojTestErrCheck ( err );

    //define objects
    MojObject obj1;
    _assignValue ( obj1, _T ( "Test2:1" ), _T ( "data1" ), 1 );

    MojObject obj2;
    _assignValue ( obj2, _T ( "Test2:1" ), _T ( "data1" ), 2 );

    MojObject obj3;
    _assignValue ( obj3, _T ( "Test2:1" ), _T ( "data1" ), 3 );

    MojDbReq req;
    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    err = db.put ( obj1, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    MojObject id1;
    bool res = obj1.get ( MojDb::IdKey, id1 );
    MojTestAssert ( res );

    req.end ( true );

    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    err = db.put ( obj2, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    err = db.put ( obj3, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    MojDbCursor cursor;
    err = query.where ( _T ( "data1" ), MojDbQuery::OpNotEq, 0 );
    MojTestErrCheck ( err );

    err = db.find ( query, cursor, req );
    MojTestErrCheck ( err );

    _displayMessage ( _T ( ", delete 1" ) );

    bool found;
    //delete obj1
    err = db.del ( id1, found, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    _displayMessage ( _T ( "\n\tread sequence-> " ) );

    if ( _printCursorValues ( cursor ) != 2 )
    {
        _displayMessage ( _T ( ", ==> Failed\n" ) );
        success = 0;
    }
    else
        _displayMessage ( _T ( ", ==> Ok\n" ) );

    cursor.close();
    req.end ( true );

    return ( success == 1 ) ? MojErrNone : MojErrTestFailed;
}

MojErr MojDbCursorTxnTest::_execTxnAndAdd ( MojDb& db )
{
    MojByte success = 1;
    MojObject obj;
    MojDbQuery query;

    _displayMessage ( _T ( "\n\tTest3: store sequence-> 1 2" ) );

    _registerKind ( db, TestKind3Str );

    // make query
    MojErr err = query.from ( _T ( "Test3:1" ) );
    MojTestErrCheck ( err );

    //define objects
    MojObject obj1;
    _assignValue ( obj1, _T ( "Test3:1" ), _T ( "data1" ), 1 );

    MojObject obj2;
    _assignValue ( obj2, _T ( "Test3:1" ), _T ( "data1" ), 2 );

    MojDbReq req;
    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    err = db.put ( obj1, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    req.end ( true );

    err = req.begin ( &db, false );
    MojTestErrCheck ( err );

    MojDbCursor cursor;
    err = query.where ( _T ( "data1" ), MojDbQuery::OpNotEq, 0 );
    MojTestErrCheck ( err );

    err = db.find ( query, cursor, req );
    MojTestErrCheck ( err );

    err = db.put ( obj2, MojDb::FlagNone, req );
    MojTestErrCheck ( err );

    _displayMessage ( _T ( "\n\tread sequence-> " ) );

    if ( _printCursorValues ( cursor ) != 2 )
    {
        _displayMessage ( _T ( ", ==> Failed\n" ) );
        success = 0;
    }
    else
        _displayMessage ( _T ( ", ==> Ok\n" ) );

    cursor.close();
    req.end ( true );

    return ( success == 1 ) ? MojErrNone : MojErrTestFailed;
}

void MojDbCursorTxnTest::cleanup()
{
    ( void ) MojRmDirRecursive ( MojDbTestDir );
}

MojErr MojDbCursorTxnTest::_displayMessage ( const MojChar* format, ... )
{
    va_list args;
    va_start ( args, format );
    MojErr err = MojVPrintF ( format, args );
    va_end ( args );
    MojErrCheck ( err );

    return MojErrNone;
}




