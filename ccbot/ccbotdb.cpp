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
#include "config.h"
#include "ccbotdb.h"

//
// CCCBotDB
//

CCCBotDB :: CCCBotDB( CConfig *CFG )
{
	m_HasError = false;
}

CCCBotDB :: ~CCCBotDB( )
{

}

bool CCCBotDB :: Begin( )
{
	return false;
}

bool CCCBotDB :: Commit( )
{
	return false;
}

uint32_t CCCBotDB :: SafelistCount( const string &server )
{
	return 0;
}

bool CCCBotDB :: SafelistCheck( const string &server, string &user )
{
	return false;
}

bool CCCBotDB :: SafelistAdd( const string &server, string &user )
{
	return false;
}

bool CCCBotDB :: SafelistRemove( const string &server, string &user )
{
	return false;
}
uint32_t CCCBotDB :: BanCount( const string &server )
{
	return 0;
}

CDBBan *CCCBotDB :: BanCheck( const string &server, string &user )
{
	return NULL;
}

bool CCCBotDB :: BanAdd( const string &server, string &user, const string &admin, const string &reason )
{
	return false;
}

bool CCCBotDB :: BanRemove( const string &server, string &user )
{
	return false;
}

bool CCCBotDB :: AccessSet( const string &server, string &user, unsigned char access )
{
	return false;
}

unsigned char CCCBotDB :: AccessCheck( const string &server, string &user )
{
	return 255;
}

uint32_t CCCBotDB :: AccessCount( const string &server, unsigned char access )
{
	return 0;
}

bool CCCBotDB :: AccessRemove( string &user )
{
	return false;
}

unsigned char CCCBotDB :: CommandAccess( const string &command )
{
	return 255;
}

bool CCCBotDB :: CommandSetAccess( const string &command, unsigned char access )
{
	return false;
}

vector<string> CCCBotDB :: CommandList( unsigned char access )
{
	return vector<string>( );
}

//
// CDBBan
//

CDBBan :: CDBBan( const string &nServer, const string &nName, const string &nDate, const string &nAdmin, const string &nReason ) : m_Server( nServer ), m_Name( nName ), m_Date( nDate ), m_Admin( nAdmin ), m_Reason( nReason )
{

}

CDBBan :: ~CDBBan( )
{

}

