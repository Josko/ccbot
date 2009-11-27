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
	in.open( file.c_str( ) );

	if( in.fail( ) )
	{
		CONSOLE_Print( "[CONFIG] warning - unable to read file [" + file + "]" );

		if( file == LanguageFile )
			CreateNewLanguage( );

		if( file == CFGFile )
			CreateConfig( );
	}
	else
	{
		CONSOLE_Print( "[CONFIG] loading file [" + file + "]" );
		string Line;

		while( !in.eof( ) )
		{
			getline( in, Line );

			// ignore blank lines and comments

			if( Line.empty( ) || Line[0] == '#' )
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


	string line, filestr = LanguageFile;

	ofstream file( filestr.c_str( ), ios :: app );

	if( file.fail( ) )
		CONSOLE_Print( "[CONFIG] error making a new language.cfg file" );
	else
	{
		CONSOLE_Print( "[CONFIG] creating a new lanuage.cfg with default values" );
		file << "" << endl;
	}

	file.close( );
	file.clear( );
}

void CConfig :: CreateConfig( )
{


	string line, filestr = CFGFile;

	ifstream file( filestr.c_str( ) );

	if( file.fail( ) )
	{		
		ofstream file( filestr.c_str( ), ios :: app );

		if( file.fail( ) )
			CONSOLE_Print( "[CONFIG] error making ccbot.cfg" );
		else
		{
			CONSOLE_Print( "[CONFIG] creating a new, blank ccbot.cfg file" );

			file << "# This file is used to configure CCBot options" << endl;
			file << "" << endl;
			file << "# Starting here are the global variables which are valid for every bnet" << endl;
			file << "" << endl;
			file << "bot_log = logs\\" << endl;
			file << "bot_language = language.cfg" << endl;
			file << "bot_war3path = C:\\Program Files\\Warcraft III\\" << endl;			
			file << "tcp_nodelay = 0" << endl;
			file << "db_type = sqlite3" << endl;
			file << "db_sqlite3_file = ccbot.dbs" << endl;
			file << "" << endl;
			file << "# Starting here are the per bnet variables" << endl;
			file << "" << endl;
			file << "bnet_server = server.eurobattle.net" << endl;
			file << "bnet_cdkeyroc = FFFFFFFFFFFFFFFFFFFFFFFFFF" << endl;
			file << "bnet_cdkeytft = FFFFFFFFFFFFFFFFFFFFFFFFFF" << endl;
			file << "bnet_username = <bots bnet account name>" << endl;
			file << "bnet_password = <bots bnet password>" << endl;
			file << "bnet_firstchannel = <first channel the bot joins>" << endl;
			file << "bnet_rootadmin = <your bnet name>" << endl;
			file << "bnet_commandtrigger = !" << endl;
			file << "bnet_antispam = 1" << endl;	
			file << "bnet_greetusers = 0" << endl;
			file << "bnet_swearingkick = 1" << endl;
			file << "bnet_selfjoin = 0" << endl;
			file << "bnet_announcegames = 1" << endl;
			file << "bnet_banchat = 0" << endl;
			file << "" << endl;
			file << "# These variables are blank if not used" << endl;
			file << "" << endl;
			file << "bnet_clantag = " << endl;
			file << "bnet_clandefaultaccess = " << endl;
			file << "bnet_hostbotname = " << endl;
			file << "bnet_countryabbrev = HRV" << endl;
			file << "bnet_country = Croatia" << endl;
			file << "" << endl;
			file << "# These variables are used when connecting to a PvPGN - how to fill them can be found on forum.codelain.com and eurobattle.net" << endl;
			file << "" << endl;		
			file << "bnet_custom_war3version = 24" << endl;
			file << "bnet_custom_exeversion = " << endl;
			file << "bnet_custom_exeversionhash = " << endl;
			file << "bnet_custom_passwordhashtype = pvpgn" << endl;
			file << "" << endl;
			file << "# You can add more servers (upto 9) by using \"bnet2_\" prefix instead of \"bnet_\" and so on" << endl;
			file << "" << endl;

			CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
			CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
			CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
			CONSOLE_Print( "[CONFIG] EDIT ccbot.cfg IN THE cfg FOLDER AND RESTART CCBOT!" );
		}
	}
	file.close( );
	file.clear( );
}
