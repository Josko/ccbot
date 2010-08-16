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

uint32_t CCCBotDB :: SafelistCount( string server )
{
	return 0;
}

bool CCCBotDB :: SafelistCheck( string server, string user )
{
	return false;
}

bool CCCBotDB :: SafelistAdd( string server, string user )
{
	return false;
}

bool CCCBotDB :: SafelistRemove( string server, string user )
{
	return false;
}
uint32_t CCCBotDB :: BanCount( string server )
{
	return 0;
}

CDBBan *CCCBotDB :: BanCheck( string server, string user )
{
	return NULL;
}

bool CCCBotDB :: BanAdd( string server, string user, string admin, string reason )
{
	return false;
}

bool CCCBotDB :: BanRemove( string server, string user )
{
	return false;
}

bool CCCBotDB :: AccessSet( string server, string user, uint32_t access )
{
	return false;
}

uint32_t CCCBotDB :: AccessCheck( string server, string user )
{
	return 11;
}

uint32_t CCCBotDB :: AccessCount( string server, uint32_t access )
{
	return 0;
}

bool CCCBotDB :: AccessRemove( string user )
{
	return false;
}

uint32_t CCCBotDB :: CommandAccess( string command )
{
	return 11;
}

bool CCCBotDB :: CommandSetAccess( string command, uint32_t access )
{
	return false;
}

vector<string> CCCBotDB :: CommandList( uint32_t access )
{
	return vector<string>( );
}

//
// CDBBan
//

CDBBan :: CDBBan( string nServer, string nName, string nDate, string nAdmin, string nReason ) : m_Server( nServer ), m_Name( nName ), m_Date( nDate ), m_Admin( nAdmin ), m_Reason( nReason )
{

}

CDBBan :: ~CDBBan( )
{

}

