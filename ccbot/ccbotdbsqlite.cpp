/*

   Copyright [2009] [Joško Nikolić]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   HEAVILY MODIFIED PROJECT BASED ON GHOST++: http://forum.codelain.com
   GHOST++ CODE IS PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#include "ccbot.h"
#include "util.h"
#include "config.h"
#include "ccbotdb.h"
#include "ccbotdbsqlite.h"
#include "sqlite3.h"

//
// CQSLITE3 (wrapper class)
//

CSQLITE3 :: CSQLITE3( string filename )
{
	m_Ready = true;

	if( sqlite3_open_v2( filename.c_str( ), (sqlite3 **)&m_DB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL ) != SQLITE_OK )
		m_Ready = false;
}

CSQLITE3 :: ~CSQLITE3( )
{
	sqlite3_close( (sqlite3 *)m_DB );
}

string CSQLITE3 :: GetError( )
{
	return sqlite3_errmsg( (sqlite3 *)m_DB );
}

int CSQLITE3 :: Prepare( string query, void **Statement )
{
	return sqlite3_prepare_v2( (sqlite3 *)m_DB, query.c_str( ), -1, (sqlite3_stmt **)Statement, NULL );
}

int CSQLITE3 :: Step( void *Statement )
{
	int RC = sqlite3_step( (sqlite3_stmt *)Statement );

	if( RC == SQLITE_ROW )
	{
		m_Row.clear( );

		for( int i = 0; i < sqlite3_column_count( (sqlite3_stmt *)Statement ); i++ )
		{
			char *ColumnText = (char *)sqlite3_column_text( (sqlite3_stmt *)Statement, i );

			if( ColumnText )
				m_Row.push_back( ColumnText );
			else
				m_Row.push_back( string( ) );
		}
	}

	return RC;
}

int CSQLITE3 :: Finalize( void *Statement )
{
	return sqlite3_finalize( (sqlite3_stmt *)Statement );
}

int CSQLITE3 :: Reset( void *Statement )
{
	return sqlite3_reset( (sqlite3_stmt *)Statement );
}

int CSQLITE3 :: Exec( string query )
{
	return sqlite3_exec( (sqlite3 *)m_DB, query.c_str( ), NULL, NULL, NULL );
}

//
// CCCBotDBSQLite
//

CCCBotDBSQLite :: CCCBotDBSQLite( CConfig *CFG ) : CCCBotDB( CFG )
{
	m_File = CFG->GetString( "db_sqlite3_file", "ccbot.dbs" );
	CONSOLE_Print( "[SQLITE3] opening database [" + m_File + "]" );
	m_DB = new CSQLITE3( m_File );

	if( !m_DB->GetReady( ) )
	{
		// setting m_HasError to true indicates there's been a critical error and we want CCBot to shutdown
		// this is okay here because we're in the constructor so we're not dropping any games or players

		CONSOLE_Print( string( "[SQLITE3] error opening database [" + m_File + "] - " ) + m_DB->GetError( ) );
		m_HasError = true;
		m_Error = "error opening database";
		return;
	}

	// find the schema number so we can determine whether we need to upgrade or not

	string SchemaNumber;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT value FROM config WHERE name=\"schema_number\"", (void **)&Statement );

	if( Statement )
	{
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
			SchemaNumber = (*m_DB->GetRow( ))[0];
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error getting schema number - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error getting schema number - " + m_DB->GetError( ) );

	if( SchemaNumber.empty( ) )
	{
		// couldn't find the schema number
		// unfortunately the very first schema didn't have a config table
		// so we might actually be looking at schema version 1 rather than an empty database
		// try to confirm this by looking for the admins table

		CONSOLE_Print( "[SQLITE3] couldn't find schema number, looking for bans table" );
		bool BansTable = false;
		m_DB->Prepare( "SELECT * FROM sqlite_master WHERE type=\"table\" AND name=\"bans\"", (void **)&Statement );

		if( Statement )
		{
			int RC = m_DB->Step( Statement );

			// we're just checking to see if the query returned a row, we don't need to check the row data itself

			if( RC == SQLITE_ROW )
				BansTable = true;
			else if( RC == SQLITE_ERROR )
				CONSOLE_Print( "[SQLITE3] error looking for bans table - " + m_DB->GetError( ) );

			m_DB->Finalize( Statement );
		}
		else
			CONSOLE_Print( "[SQLITE3] prepare error looking for bans table - " + m_DB->GetError( ) );

		if( BansTable )
		{
			// the bans table exists, assume we're looking at schema version 1

			CONSOLE_Print( "[SQLITE3] found bans table, assuming schema number [1]" );
			SchemaNumber = "1";
		}
		else
		{
			// the bans table doesn't exist, assume the database is empty
			// note to self: update the SchemaNumber and the database structure when making a new schema

			CONSOLE_Print( "[SQLITE3] couldn't find admins table, assuming database is empty" );
			SchemaNumber = "4";

			if( m_DB->Exec( "CREATE TABLE bans ( id INTEGER NOT NULL PRIMARY KEY, server TEXT NOT NULL, name TEXT NOT NULL, date TEXT NOT NULL, admin TEXT NOT NULL, reason TEXT )" ) != SQLITE_OK )
				CONSOLE_Print( "[SQLITE3] error creating bans table - " + m_DB->GetError( ) );

			if( m_DB->Exec( "CREATE TABLE safelist ( id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, server TEXT NOT NULL DEFAULT \"\" )" ) != SQLITE_OK )
				CONSOLE_Print( "[SQLITE3] error creating safelist table - " + m_DB->GetError( ) );

			if( m_DB->Exec( "CREATE TABLE config ( name TEXT NOT NULL PRIMARY KEY, value TEXT NOT NULL )" ) != SQLITE_OK )
				CONSOLE_Print( "[SQLITE3] error creating config table - " + m_DB->GetError( ) );
		
			if( m_DB->Exec( "CREATE TABLE access ( id INTEGER NOT NULL PRIMARY KEY, server TEXT NOT NULL, name TEXT NOT NULL, access INTEGER )" ) != SQLITE_OK )
				CONSOLE_Print( "[SQLITE3] error creating access table - " + m_DB->GetError( ) );

			if( m_DB->Exec( "CREATE TABLE commands ( id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL, access INTEGER NOT NULL )" ) != SQLITE_OK )
				CONSOLE_Print( "[SQLITE3] error creating access table - " + m_DB->GetError( ) );

			m_DB->Prepare( "INSERT INTO config VALUES ( \"schema_number\", ? )", (void **)&Statement );

			if( Statement )
			{
				sqlite3_bind_text( Statement, 1, SchemaNumber.c_str( ), -1, SQLITE_TRANSIENT );
				int RC = m_DB->Step( Statement );

				if( RC == SQLITE_ERROR )
					CONSOLE_Print( "[SQLITE3] error inserting schema number [" + SchemaNumber + "] - " + m_DB->GetError( ) );

				m_DB->Finalize( Statement );
			}
			else
				CONSOLE_Print( "[SQLITE3] prepare error inserting schema number [" + SchemaNumber + "] - " + m_DB->GetError( ) );			
		}
	}
	else
		CONSOLE_Print( "[SQLITE3] found schema number [" + SchemaNumber + "]" );

	if( SchemaNumber == "1" )
	{
		Upgrade1_2( );
		SchemaNumber = "2";
	}

	if( SchemaNumber == "2" )
	{
		Upgrade2_3( );
		SchemaNumber = "3";
	}
	if( SchemaNumber == "3" )
	{
		Upgrade3_4( );
		SchemaNumber = "4";
	}
}

CCCBotDBSQLite :: ~CCCBotDBSQLite( )
{
	if( FromAddStmt )
		m_DB->Finalize( FromAddStmt );

	CONSOLE_Print( "[SQLITE3] closing database [" + m_File + "]" );
	delete m_DB;
}

void CCCBotDBSQLite :: Upgrade1_2( )
{
	CONSOLE_Print( "[SQLITE3] schema upgrade v1 to v2 started" );

	// add new table config

	if( m_DB->Exec( "CREATE TABLE config ( name TEXT NOT NULL PRIMARY KEY, value TEXT NOT NULL )" ) != SQLITE_OK )
		CONSOLE_Print( "[SQLITE3] error creating config table - " + m_DB->GetError( ) );
	else
		CONSOLE_Print( "[SQLITE3] created config table" );

	sqlite3_stmt *Statement;
	m_DB->Prepare( "INSERT INTO config VALUES ( \"schema_number\", \"2\" )", (void **)&Statement );

	if( Statement )
	{
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			CONSOLE_Print( "[SQLITE3] inserted schema number [2]" );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error inserting schema number [2] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error inserting schema number [2] - " + m_DB->GetError( ) );

	CONSOLE_Print( "[SQLITE3] schema upgrade v1 to v2 finished" );
}

void CCCBotDBSQLite :: Upgrade2_3( )
{
	CONSOLE_Print( "[SQLITE3] schema upgrade v2 to v3 started" );

	// add table access
	if( m_DB->Exec( "CREATE TABLE access ( id INTEGER PRIMARY KEY, server TEXT NOT NULL, name TEXT NOT NULL, access INTEGER )" ) != SQLITE_OK )
			CONSOLE_Print( "[SQLITE3] error creating access table - " + m_DB->GetError( ) );
	else
			CONSOLE_Print( "[SQLITE3] added new table access" );

	// update schema number

	if( m_DB->Exec( "UPDATE config SET value=\"3\" where name=\"schema_number\"" ) != SQLITE_OK )
		CONSOLE_Print( "[SQLITE3] error updating schema number [3] - " + m_DB->GetError( ) );
	else
		CONSOLE_Print( "[SQLITE3] updated schema number [3]" );

	CONSOLE_Print( "[SQLITE3] schema upgrade v2 to v3 finished" );
}

void CCCBotDBSQLite :: Upgrade3_4( )
{
	CONSOLE_Print( "[SQLITE3] schema upgrade v3 to v4 started" );

	// add table access
	if( m_DB->Exec( "CREATE TABLE commands ( id INTEGER PRIMARY KEY, name TEXT NOT NULL, access INTEGER NOT NULL )" ) != SQLITE_OK )
			CONSOLE_Print( "[SQLITE3] error creating commands table - " + m_DB->GetError( ) );
	else
			CONSOLE_Print( "[SQLITE3] added new table commands" );

	// update schema number

	if( m_DB->Exec( "UPDATE config SET value=\"4\" where name=\"schema_number\"" ) != SQLITE_OK )
		CONSOLE_Print( "[SQLITE3] error updating schema number [4] - " + m_DB->GetError( ) );
	else
		CONSOLE_Print( "[SQLITE3] updated schema number [4]" );

	CONSOLE_Print( "[SQLITE3] schema upgrade v3 to v4 finished" );
}

bool CCCBotDBSQLite :: Begin( )
{
	return m_DB->Exec( "BEGIN TRANSACTION" ) == SQLITE_OK;
}

bool CCCBotDBSQLite :: Commit( )
{
	return m_DB->Exec( "COMMIT TRANSACTION" ) == SQLITE_OK;
}

uint32_t CCCBotDBSQLite :: SafelistCount( string server )
{
	uint32_t Count = 0;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT COUNT(*) FROM safelist WHERE server=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
			Count = UTIL_ToUInt32( ( *m_DB->GetRow( ) )[0] );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error counting safelist [" + server + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error counting safelist [" + server + "] - " + m_DB->GetError( ) );

	return Count;
}

bool CCCBotDBSQLite :: SafelistCheck( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool IsSafelisted = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT * FROM safelist WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		// we're just checking to see if the query returned a row, we don't need to check the row data itself

		if( RC == SQLITE_ROW )
			IsSafelisted = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking safelist [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking safelist [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return IsSafelisted;
}

bool CCCBotDBSQLite :: SafelistAdd( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "INSERT INTO safelist ( server, name ) VALUES ( ?, ? )", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			Success = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error adding safelisted user [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error adding safelisted user [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Success;
}

bool CCCBotDBSQLite :: SafelistRemove( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "DELETE FROM safelist WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			Success = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error removing safelisted user [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error safelisted user [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Success;
}

uint32_t CCCBotDBSQLite :: BanCount( string server )
{
	uint32_t Count = 0;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT COUNT(*) FROM bans WHERE server=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
			Count = sqlite3_column_int( Statement, 0 );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error counting bans [" + server + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error counting bans [" + server + "] - " + m_DB->GetError( ) );

	return Count;
}

CDBBan *CCCBotDBSQLite :: BanCheck( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	CDBBan *Ban = NULL;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT date, admin, reason FROM bans WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
		{
			vector<string> *Row = m_DB->GetRow( );

			if( Row->size( ) == 3 )
				Ban = new CDBBan( server, user, (*Row)[0], (*Row)[1], (*Row)[2] );
			else
				CONSOLE_Print( "[SQLITE3] error checking ban [" + server + " : " + user + "] - row doesn't have 5 columns" );
		}
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking ban [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking ban [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Ban;
}

bool CCCBotDBSQLite :: BanAdd( string server, string user, string admin, string reason )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "INSERT INTO bans ( server, name, date, admin, reason ) VALUES ( ?, ?, date('now'), ?, ? )", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 3, admin.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 4, reason.c_str( ), -1, SQLITE_TRANSIENT );

		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			Success = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error adding ban [" + server + " : " + user + " : " + admin + " : " + reason + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error adding ban [" + server + " : " + user + " : " + admin + " : " + reason + "] - " + m_DB->GetError( ) );

	return Success;
}

bool CCCBotDBSQLite :: BanRemove( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "DELETE FROM bans WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			Success = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error removing ban [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error removing ban [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Success;
}

bool CCCBotDBSQLite :: AccessSet( string server, string user, uint32_t access )
{	
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT access FROM access WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		// we're checking if the user has already has a set access, if it does we edit that record, if not insert one

		if( RC == SQLITE_ROW )
		{
			sqlite3_stmt *Statement;
			m_DB->Prepare( "UPDATE access SET access =? WHERE name =? AND server =?", (void **)&Statement );
			if( Statement )
			{
				sqlite3_bind_int( Statement, 1, access );				
				sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
				sqlite3_bind_text( Statement, 3, server.c_str( ), -1, SQLITE_TRANSIENT );
				int RC = m_DB->Step( Statement );

				if( RC == SQLITE_DONE )
					Success = true;
				else if( RC == SQLITE_ERROR )
					CONSOLE_Print( "[SQLITE3] error updating record in access table [" + server + " : " + user + " update to " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) + "]" );
			}			
		}
		else if ( RC == SQLITE_DONE )
		{
			m_DB->Prepare( "INSERT INTO access ( server, name, access ) VALUES ( ?, ?, ? )", (void **)&Statement );

			if( Statement )
			{
				sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
				sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
				sqlite3_bind_int( Statement, 3, access );
				int RC = m_DB->Step( Statement );

				if( RC == SQLITE_DONE )
					Success = true;
				else if( RC == SQLITE_ERROR )
					CONSOLE_Print( "[SQLITE3] error inserting access [" + server + " : " + user + " : " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );
			}
			else
			CONSOLE_Print( "[SQLITE3] prepare error inserting access [" + server + " : " + user + " : " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );

		}
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking access [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking access [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Success;
}

uint32_t CCCBotDBSQLite :: AccessCheck( string server, string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	uint32_t Access = 11;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT access FROM access WHERE server=? AND name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );
		sqlite3_bind_text( Statement, 2, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		// we're checking to see if the query returned a row then we extract the data (access level)

		if( RC == SQLITE_ROW )
			Access = UTIL_ToUInt32( (*m_DB->GetRow( ) )[0] );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking access [" + server + " : " + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking access [" + server + " : " + user + "] - " + m_DB->GetError( ) );

	return Access;
}

uint32_t CCCBotDBSQLite :: AccessCount( string server, uint32_t access )
{
	uint32_t Count = 0;
	sqlite3_stmt *Statement;
	if( access != 0 )
		m_DB->Prepare( "SELECT COUNT(*) FROM access WHERE server=? AND access=?", (void **)&Statement );
	else
		m_DB->Prepare( "SELECT COUNT(*) FROM access WHERE server=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, server.c_str( ), -1, SQLITE_TRANSIENT );

		if( access != 0 )
			sqlite3_bind_int( Statement, 2, access );

		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
			Count = sqlite3_column_int( Statement, 0 );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error counting access table [" + server + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error counting access table [" + server + "] - " + m_DB->GetError( ) );

	return Count;
}

bool CCCBotDBSQLite :: AccessRemove( string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "DELETE FROM access WHERE name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, user.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_DONE )
			Success = true;
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error removing access [" + user + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error removing access [" + user + "] - " + m_DB->GetError( ) );

	return Success;
}

uint32_t CCCBotDBSQLite :: CommandAccess( string command )
{
	transform( command.begin( ), command.end( ), command.begin( ), (int(*)(int))tolower );
	uint32_t Access = 11;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT access FROM commands WHERE name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, command.c_str( ), -1, SQLITE_TRANSIENT );

		int RC = m_DB->Step( Statement );

		if( RC == SQLITE_ROW )
			Access = UTIL_ToUInt32( (*m_DB->GetRow( ) )[0] );
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking access for [" + command + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking access for [" + command + "] - " + m_DB->GetError( ) );

	return Access;
}

bool CCCBotDBSQLite :: CommandSetAccess( string command, uint32_t access )
{	
	transform( command.begin( ), command.end( ), command.begin( ), (int(*)(int))tolower );
	bool Success = false;
	sqlite3_stmt *Statement;
	m_DB->Prepare( "SELECT access FROM commands WHERE name=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_text( Statement, 1, command.c_str( ), -1, SQLITE_TRANSIENT );
		int RC = m_DB->Step( Statement );

		// we're checking if the user has already has a set access, if it does we edit that record, if not insert one

		if( RC == SQLITE_ROW )
		{
			sqlite3_stmt *Statement;
			m_DB->Prepare( "UPDATE commands SET access =? WHERE name =?", (void **)&Statement );
			if( Statement )
			{
				sqlite3_bind_int( Statement, 1, access );				
				sqlite3_bind_text( Statement, 2, command.c_str( ), -1, SQLITE_TRANSIENT );
				int RC = m_DB->Step( Statement );

				if( RC == SQLITE_DONE )
					Success = true;
				else if( RC == SQLITE_ERROR )
					CONSOLE_Print( "[SQLITE3] error updating record in commands table [" + command + " update to " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) + "]" );
			}			
		}
		else if ( RC == SQLITE_DONE )
		{
			m_DB->Prepare( "INSERT INTO commands ( name, access ) VALUES ( ?, ? )", (void **)&Statement );

			if( Statement )
			{
				sqlite3_bind_text( Statement, 1, command.c_str( ), -1, SQLITE_TRANSIENT );
				sqlite3_bind_int( Statement, 2, access );
				int RC = m_DB->Step( Statement );

				if( RC == SQLITE_DONE )
					Success = true;
				else if( RC == SQLITE_ERROR )
					CONSOLE_Print( "[SQLITE3] error inserting command [" + command + " : " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );
			}
			else
			CONSOLE_Print( "[SQLITE3] prepare error inserting command [" + command + " : " + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );

		}
		else if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking access for [" + command + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking access for [" + command + "] - " + m_DB->GetError( ) );

	return Success;
}

vector<string> CCCBotDBSQLite :: CommandList( uint32_t access )
{
	vector<string> CommandList;
	sqlite3_stmt *Statement;	
	m_DB->Prepare( "SELECT name FROM commands WHERE access=?", (void **)&Statement );

	if( Statement )
	{
		sqlite3_bind_int( Statement, 1, access );
		int RC = m_DB->Step( Statement );

		// we're checking to see if the query returned a row then we extract the data

		while( RC == SQLITE_ROW )
		{
			CommandList.push_back( (*m_DB->GetRow( ))[0] );
			RC = m_DB->Step( Statement );
		}

		if( RC == SQLITE_ERROR )
			CONSOLE_Print( "[SQLITE3] error checking commands with access of [" + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );

		m_DB->Finalize( Statement );
	}
	else
		CONSOLE_Print( "[SQLITE3] prepare error checking commands with access of [" + UTIL_ToString( access ) + "] - " + m_DB->GetError( ) );	

	return CommandList;
}
