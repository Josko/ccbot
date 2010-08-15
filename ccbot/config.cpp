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

#include <stdlib.h>

//
// CConfig
//

CConfig :: CConfig( )
{

}

CConfig :: ~CConfig( )
{

}

void CConfig :: Read( string file )
{
	ifstream in;
	in.open( file );

	if( in.fail( ) )
	{
		CONSOLE_Print( "[CONFIG] warning - unable to read file [" + file + "]" );

		if( file == CFGFile )
			CreateConfig( );

		if( file == LanguageFile )
			CreateNewLanguage( );		
	}
	else
	{
		CONSOLE_Print( "[CONFIG] loading file [" + file + "]" );
		string Line;

		while( !in.eof( ) )
		{
			getline( in, Line );

			// ignore blank lines, newlines, comments

			if( Line.empty( ) || Line[0] == '#' || Line == "\n" )
				continue;

			string :: size_type Split = Line.find( "=" );

			if( Split == string :: npos )
				continue;

			string :: size_type KeyStart = Line.find_first_not_of( " " );
			string :: size_type KeyEnd = Line.find( " ", KeyStart );
			string :: size_type ValueStart = Line.find_first_not_of( " ", Split + 1 );
			string :: size_type ValueEnd = Line.size( );

			if( ValueStart != string :: npos )
				m_CFG[Line.substr( KeyStart, KeyEnd - KeyStart )] = Line.substr( ValueStart, ValueEnd - ValueStart );
		}

		in.close( );
	}
}

bool CConfig :: Exists( string key )
{
	return m_CFG.find( key ) != m_CFG.end( );
}

int CConfig :: GetInt( string key, int x )
{
	if( m_CFG.find( key ) == m_CFG.end( ) )
		return x;
	else
		return atoi( m_CFG[key].c_str( ) );
}

string CConfig :: GetString( string key, string x )
{
	if( m_CFG.find( key ) == m_CFG.end( ) )
		return x;
	else
		return m_CFG[key];
}

void CConfig :: CreateNewLanguage( )
{
	ofstream file( LanguageFile, ios :: app );

	if( file.fail( ) )
		CONSOLE_Print( "[CONFIG] error making a new language.cfg file" );
	else
	{
		CONSOLE_Print( "[CONFIG] creating a new lanuage.cfg with default values" );

		file << "lang_0001 = Message queue cleared...\n";
		file << "lang_0002 = Game announcer is [ON].\n";
		file << "lang_0003 = Game announcer is [OFF].\n";
		file << "lang_0004 = You don't have access to that command.\n";
		file << "lang_0005 = Updating bot's internal clan list from Battle.Net...\n";
		file << "lang_0006 = Received [$TOTAL$] clan members.\n";
		file << "lang_0007 = User [$USER$] must be a clan member to transfer ownership.\n";
		file << "lang_0008 = Greeting users on join is enabled.\n";
		file << "lang_0009 = Greeting users on join is disabled.\n";
		file << "lang_0010 = CFG settings and clan list reloaded.\n";
		file << "lang_0011 = Using B.net commands is not allowed via say command.\n";
		file << "lang_0012 = Unable to partially match with a server.\n";
		file << "lang_0013 = Clan and Channel Bot (CCBot) - Version $VERSION$\n";
		file << "lang_0014 = Connected to Battle.net server [$SERVER$].\n";
		file << "lang_0015 = Invalid password (attempt $ATTEMPT$).\n";
		file << "lang_0016 = Disconnected from battle.net server [$SERVER$].\n";
		file << "lang_0017 = Logged in.\n";
		file << "lang_0018 = Connecting to battle.net server [$SERVER$] timed out.\n";
		file << "lang_0019 = /w $USER$ Welcome to the $CHANNEL$ channel! Enjoy your stay.\n";
		file << "lang_0020 = \n";
		file << "lang_0021 = /kick $USER$ Undesirable word detected: $SWEAR$\n";
		file << "lang_0022 = $USER$ has joined a game named \"$GAMENAME$\".\n";
		file << "lang_0023 = Command trigger: $TRIGGER$\n";
		file << "lang_0024 = [$USER$] has a ping of $PING$ms on [$SERVER$].\n";
		file << "lang_0025 = Can only access pings from users in channel.\n";
		file << "lang_0026 = This feature has been disabled.\n";
		file << "lang_0027 = Invitation has been accepted.\n";
		file << "lang_0028 = With an access of [$ACCESS$] you have the following commands:\n";
		file << "lang_0029 = User [$USER$] is already safelisted.\n";
		file << "lang_0030 = [$USER$] added to the safelist.\n";
		file << "lang_0031 = Error adding [$USER$] to the safelist.\n";
		file << "lang_0032 = Announcing has been disabled.\n";
		file << "lang_0033 = Announcing has been enabled every [$INTERVAL$] seconds.\n";
		file << "lang_0034 = [$USER$] was already banned from the channel by $ADMIN$.\n";
		file << "lang_0035 = [$USER$] successfully banned from the channel by $ADMIN$.\n";
		file << "lang_0036 = Error banning [$USER$] from the channel.\n";
		file << "lang_0037 = [$USER$] has a uptime of : $TIME$\n";
		file << "lang_0038 = /me - $USER$ is going to bed and wishes everyone a good night!\n";
		file << "lang_0039 = User [$USER$] is safelisted.\n";
		file << "lang_0040 = User [$USER$] is not safelisted.\n";
		file << "lang_0041 = You have successfully changed rank of [$USER$] to $RANK$.\n";
		file << "lang_0042 = Lockdown mode engaged! Users with access lower then [$ACCESS$] cannot join.\n";
		file << "lang_0043 = Lockdown mode is deactivated! Anyone can join the channel now.\n";
		file << "lang_0044 = Clan MOTD set to [$MOTD$].\n";
		file << "lang_0045 = Channel topic set to [$TOPIC$].\n";
	}

	file.close( );
}

void CConfig :: CreateConfig( )
{
	ofstream file( CFGFile );

	if( file.fail( ) )
		CONSOLE_Print( "[CONFIG] error making a new ccbot.cfg file" );
	else
	{		
		CONSOLE_Print( "[CONFIG] creating a new, blank ccbot.cfg file" );

		file << "# This file is used to configure CCBot options\n" << endl;
		file << "# Starting here are the global variables which are valid for every bnet\n" << endl;
		file << "bot_log = logs\\" << endl;
		file << "bot_language = language.cfg" << endl;
		file << "bot_war3path = C:\\Program Files\\Warcraft III\\" << endl;			
		file << "tcp_nodelay = 0" << endl;
		file << "db_type = sqlite3" << endl;
		file << "db_sqlite3_file = ccbot.dbs\n" << endl;
		file << "# These variables are needed\n" << endl;
		file << "bnet_server = server.eurobattle.net" << endl;
		file << "bnet_cdkeyroc = FFFFFFFFFFFFFFFFFFFFFFFFFF" << endl;
		file << "bnet_cdkeytft = FFFFFFFFFFFFFFFFFFFFFFFFFF" << endl;
		file << "bnet_username = <bots bnet account name>" << endl;
		file << "bnet_password = <bots bnet password>" << endl;
		file << "bnet_firstchannel = <first channel the bot joins>" << endl;
		file << "bnet_rootadmin = <your bnet name>" << endl;
		file << "bnet_commandtrigger = !\n" << endl;
		file << "# These variables are optional\n" << endl;
		file << "bnet_clantag = " << endl;
		file << "bnet_clandefaultaccess = " << endl;
		file << "bnet_hostbotname = " << endl;
		file << "bnet_countryabbrev = HRV" << endl;
		file << "bnet_country = Croatia" << endl;
		file << "bnet_antispam = 0" << endl;	
		file << "bnet_greetusers = 0" << endl;
		file << "bnet_swearingkick = 0" << endl;
		file << "bnet_selfjoin = 0" << endl;
		file << "bnet_announcegames = 0" << endl;
		file << "bnet_banchat = 0\n" << endl;
		file << "# These variables are used when connecting to a PvPGN - how to fill them can be found on forum.codelain.com and eurobattle.net\n" << endl;	
		file << "bnet_custom_war3version = 24" << endl;
		file << "bnet_custom_exeversion = " << endl;
		file << "bnet_custom_exeversionhash = " << endl;
		file << "bnet_custom_passwordhashtype = pvpgn\n" << endl;
		file << "# You can add more servers (upto 9) by using \"bnet2_\" prefix instead of \"bnet_\" and so on\n" << endl;

		CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
		CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
		CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
		CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
	}

	file.close( );
}
