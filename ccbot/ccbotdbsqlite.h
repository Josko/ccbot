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

#ifndef CCBOTDBSQLITE_H
#define CCBOTDBSQLITE_H

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
// CCCBotDBSQLite
//

class CCCBotDBSQLite : public CCCBotDB
{
private:
	string m_File;
	CSQLITE3 *m_DB;

public:
	CCCBotDBSQLite( CConfig *CFG );
	virtual ~CCCBotDBSQLite( );

	virtual void Upgrade1_2( );
	virtual void Upgrade2_3( );
	virtual void Upgrade3_4( );

	virtual bool Begin( );
	virtual bool Commit( );

	// safelist
	virtual uint32_t SafelistCount( const string &server );
	virtual bool SafelistCheck( const string &server, string &user );
	virtual bool SafelistAdd( const string &server, string &user );
	virtual bool SafelistRemove( const string &server, string &user );

	// bans
	virtual uint32_t BanCount( const string &server );
	virtual CDBBan *BanCheck( const string &server, string &user );
	virtual bool BanAdd( const string &server, string &user, const string &admin, const string &reason );
	virtual bool BanRemove( const string &server, string &user );

	// access
	virtual bool AccessSet( const string &server, string &user, unsigned char access );
	virtual unsigned char AccessCheck( const string &server, string &user );
	virtual uint32_t AccessCount( const string &server, unsigned char access );
	virtual bool AccessRemove( string &user );

	// commands
	virtual unsigned char CommandAccess( const string &command );
	virtual bool CommandSetAccess( const string &command, unsigned char access );
	virtual vector<string> CommandList( unsigned char access );

};

#endif
