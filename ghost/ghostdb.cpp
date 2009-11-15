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

#include "ghost.h"
#include "config.h"
#include "ghostdb.h"

//
// CGHostDB
//

CGHostDB :: CGHostDB( CConfig *CFG )
{
	m_HasError = false;
}

CGHostDB :: ~CGHostDB( )
{

}

bool CGHostDB :: Begin( )
{
	return false;
}

bool CGHostDB :: Commit( )
{
	return false;
}

uint32_t CGHostDB :: SafelistCount( string server )
{
	return 0;
}

bool CGHostDB :: SafelistCheck( string server, string user )
{
	return false;
}

bool CGHostDB :: SafelistAdd( string server, string user )
{
	return false;
}

bool CGHostDB :: SafelistRemove( string server, string user )
{
	return false;
}
uint32_t CGHostDB :: BanCount( string server )
{
	return 0;
}

CDBBan *CGHostDB :: BanCheck( string server, string user )
{
	return NULL;
}

bool CGHostDB :: BanAdd( string server, string user, string admin, string reason )
{
	return false;
}

bool CGHostDB :: BanRemove( string server, string user )
{
	return false;
}

bool CGHostDB :: AccessSet( string server, string user, uint32_t access )
{
	return false;
}

uint32_t CGHostDB :: AccessCheck( string server, string user )
{
	return 11;
}

uint32_t CGHostDB :: AccessCount( string server, uint32_t access )
{
	return 0;
}

bool CGHostDB :: AccessRemove( string user )
{
	return false;
}

uint32_t CGHostDB :: CommandAccess( string command )
{
	return 11;
}

bool CGHostDB :: CommandSetAccess( string command, uint32_t access )
{
	return false;
}

vector<string> CGHostDB :: CommandList( uint32_t access )
{
	return vector<string>( );
}

//
// CDBBan
//

CDBBan :: CDBBan( string nServer, string nName, string nDate, string nAdmin, string nReason )
{
	m_Server = nServer;
	m_Name = nName;
	m_Date = nDate;
	m_Admin = nAdmin;
	m_Reason = nReason;
}

CDBBan :: ~CDBBan( )
{

}

