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

#ifndef GHOSTDBSQLITE_H
#define GHOSTDBSQLITE_H

/**************
 *** SCHEMA ***
 **************

CREATE TABLE bans (
	id INTEGER PRIMARY KEY,
	server TEXT NOT NULL,
	name TEXT NOT NULL,
	date TEXT NOT NULL,
	admin TEXT NOT NULL,
	reason TEXT
)

CREATE TABLE safelist (
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	server TEXT NOT NULL DEFAULT ""
)

CREATE TABLE access (
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	server TEXT NOT NULL DEFAULT "",
	access INTEGER
)

CREATE TABLE config (
	name TEXT NOT NULL PRIMARY KEY,
	value TEXT NOT NULL
)

CREATE TABLE commands (
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	access INTEGER NOT NULL
)


 **************
 *** SCHEMA ***
 **************/

//
// CSQLITE3 (wrapper class)
//

class CSQLITE3
{
private:
	void *m_DB;
	bool m_Ready;
	vector<string> m_Row;

public:
	CSQLITE3( string filename );
	~CSQLITE3( );

	bool GetReady( )					{ return m_Ready; }
	vector<string> *GetRow( )				{ return &m_Row; }
	string GetError( );

	int Prepare( string query, void **Statement );
	int Step( void *Statement );
	int Finalize( void *Statement );
	int Reset( void *Statement );
	int ClearBindings( void *Statement );
	int Exec( string query );
	uint32_t LastRowID( );
};

//
// CGHostDBSQLite
//

class CGHostDBSQLite : public CGHostDB
{
private:
	string m_File;
	CSQLITE3 *m_DB;

	// we keep some prepared statements in memory rather than recreating them each function call
	// this is an optimization because preparing statements takes time
	// however it only pays off if you're going to be using the statement extremely often

	void *FromAddStmt;

public:
	CGHostDBSQLite( CConfig *CFG );
	virtual ~CGHostDBSQLite( );

	virtual void Upgrade1_2( );
	virtual void Upgrade2_3( );
	virtual void Upgrade3_4( );

	virtual bool Begin( );
	virtual bool Commit( );
	// safelist
	virtual uint32_t SafelistCount( string server );
	virtual bool SafelistCheck( string server, string user );
	virtual bool SafelistAdd( string server, string user );
	virtual bool SafelistRemove( string server, string user );
	// bans
	virtual uint32_t BanCount( string server );
	virtual CDBBan *BanCheck( string server, string user );
	virtual bool BanAdd( string server, string user, string admin, string reason );
	virtual bool BanRemove( string server, string user );
	// access
	virtual bool AccessSet( string server, string user, uint32_t access );
	virtual uint32_t AccessCheck( string server, string user );
	virtual uint32_t AccessCount( string server, uint32_t access );
	virtual bool AccessRemove( string user );
	// commands
	virtual uint32_t CommandAccess( string command );
	virtual bool CommandSetAccess( string command, uint32_t access );
	virtual vector<string> CommandList( uint32_t access );

};

#endif
