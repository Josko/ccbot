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

//
// CCCBotDB
//
class CDBBan;
class CCCBotDB

{
protected:
	bool m_HasError;
	string m_Error;

public:
	CCCBotDB( CConfig *CFG );
	virtual ~CCCBotDB( );

	bool HasError( )		{ return m_HasError; }
	string GetError( )		{ return m_Error; }

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

	// command
	virtual uint32_t CommandAccess( string command );
	virtual bool CommandSetAccess( string command, uint32_t access );
	virtual vector<string> CommandList( uint32_t access );
	
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
	CDBBan( string nServer, string nName, string nDate, string nAdmin, string nReason );
	~CDBBan( );

	string GetServer( )		{ return m_Server; }
	string GetName( )		{ return m_Name; }
	string GetDate( )		{ return m_Date; }
	string GetAdmin( )		{ return m_Admin; }
	string GetReason( )		{ return m_Reason; }
};
#endif
