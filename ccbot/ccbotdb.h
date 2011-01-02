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

#ifndef CCBOTDB_H
#define CCBOTDB_H

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
	CSQLITE3( const string &filename );
	~CSQLITE3( );

	bool GetReady( )					{ return m_Ready; }
	vector<string> *GetRow( )				{ return &m_Row; }
	string GetError( );

	int Prepare( const string &query, void **Statement );
	int Step( void *Statement );
	int Finalize( void *Statement );
	int Reset( void *Statement );
	int Exec( const string &query );
};

//
// CCCBotDB
//

class CDBBan;

class CCCBotDB
{
private:
	string m_File;
	CSQLITE3 *m_DB;
        
protected:
	bool m_HasError;
	string m_Error;

public:
	CCCBotDB( CConfig *CFG );
	~CCCBotDB( );

        bool HasError( )		{ return m_HasError; }
	string GetError( )		{ return m_Error; }

	void Upgrade1_2( );
	void Upgrade2_3( );
	void Upgrade3_4( );

	bool Begin( );
	bool Commit( );

	// safelist
	uint32_t SafelistCount( const string &server );
	bool SafelistCheck( const string &server, string &user );
	bool SafelistAdd( const string &server, string &user );
	bool SafelistRemove( const string &server, string &user );

	// bans
	uint32_t BanCount( const string &server );
	CDBBan *BanCheck( const string &server, string user );
	bool BanAdd( const string &server, string &user, const string &admin, const string &reason );
	bool BanRemove( const string &server, string &user );

	// access
	bool AccessSet( const string &server, string &user, unsigned char access );
	unsigned char AccessCheck( const string &server, string &user );
	uint32_t AccessCount( const string &server, unsigned char access );
	bool AccessRemove( string &user );

	// commands
	unsigned char CommandAccess( const string &command );
	bool CommandSetAccess( const string &command, unsigned char access );
	vector<string> CommandList( unsigned char access );

};

//
// CDBBan
//

class CDBBan
{
private:
	string m_Server;
	string m_Name;
	string m_Date;
	string m_Admin;
	string m_Reason;

public:
	CDBBan( const string &nServer, const string &nName, const string &nDate, const string &nAdmin, const string &nReason );
	~CDBBan( );

	string GetServer( )		{ return m_Server; }
	string GetName( )		{ return m_Name; }
	string GetDate( )		{ return m_Date; }
	string GetAdmin( )		{ return m_Admin; }
	string GetReason( )		{ return m_Reason; }
};
#endif
