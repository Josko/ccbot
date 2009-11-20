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
#include "language.h"
#include "socket.h"
#include "ccbotdb.h"
#include "ccbotdbsqlite.h"
#include "bnet.h"
#include <time.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

time_t gStartTime;
string gLogFile;
CCCBot *gCCBot = NULL;

uint32_t GetTime( )
{
	return (uint32_t)( (GetTicks( ) / 1000) - gStartTime );
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return GetTickCount( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

void SignalCatcher( int signal )
{
	// signal( SIGABRT, SignalCatcher );
	// signal( SIGINT, SignalCatcher );

	CONSOLE_Print( "[!!!] caught signal, shutting down" );

	if( gCCBot )
	{
		if( gCCBot->m_Exiting )
			exit( 1 );
		else
			gCCBot->m_Exiting = true;
	}
	else
		exit( 1 );
}

//
// main
//

int main( int argc, char **argv )
{
#ifdef WIN32
		string CFGFile = "cfg\\ccbot.cfg";
#else
		string CFGFile = "cfg/ccbot.cfg";
#endif

	if( argc > 1 && argv[1] )
		CFGFile = argv[1];

	// read config file

	CConfig CFG;
	CFG.Read( CFGFile );
	gLogFile = CFG.GetString( "bot_log", string( ) );

	// print something for logging purposes

	CONSOLE_Print( "[CCBOT] starting up" );

	// catch SIGABRT and SIGINT

	signal( SIGABRT, SignalCatcher );
	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

	// initialize the start time
	gStartTime = 0;
	gStartTime = GetTime( );

#ifdef WIN32

	// increase process priority

	CONSOLE_Print( "[CCBOT] setting process priority to \"above normal\"" );
	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );

	// initialize winsock

	CONSOLE_Print( "[CCBOT] starting winsock" );
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[CCBOT] error starting winsock" );
		return 1;
	}
#endif

	// initialize ccbot

	gCCBot = new CCCBot( &CFG );

	while( 1 )
	{
		// block for 50ms on all sockets - if you intend to perform any timed actions more frequently you should change this
		// that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

		if( gCCBot->Update( 10000 ) )
		break;
	}

	// shutdown ghost

	CONSOLE_Print( "[CCBOT] shutting down" );
	delete gCCBot;
	gCCBot = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[CCBOT] shutting down winsock" );
	WSACleanup( );
#endif

	return 0;
}

void CONSOLE_Print( string message )
{
        cout << message << endl;

        // logging

        if( !gLogFile.empty( ) )
        {
                ofstream Log;
                time_t Now;
                time( &Now );
                char datestr[100];
                strftime( datestr , 100, "%m%d%y", localtime( &Now ) );
                char str[80];
#ifdef WIN32
                strcpy(str, "logs\\");
#else
                strcpy(str, "logs/");
#endif

                strcat(str, datestr);
                strcat(str, ".log");
                Log.open(str, ios :: app );

                if( !Log.fail( ) )
                {
                        char timestr[100];
                        strftime( timestr , 100, "%H:%M:%S", localtime( &Now ) );

                        // erase the newline

                        Log << "[" << timestr << "] " << message << endl;
                        Log.close( );
                }
        }
}

void DEBUG_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}

//
// CCBot
//

CCCBot :: CCCBot( CConfig *CFG )
{
	m_DB = new CCCBotDBSQLite( CFG );
	m_Exiting = false;
	m_Version = "0.27";
#ifdef WIN32
	m_Language = new CLanguage( "cfg\\language.cfg" );
#else
	m_Language = new CLanguage( "cfg/language.cfg" );
#endif
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );
	string BotCommandTrigger = CFG->GetString( "bot_commandtrigger", "!" );
	tcp_nodelay = CFG->GetInt( "tcp_nodelay", 0 ) == 0 ? false : true;	
	

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)

	for( uint32_t i = 1; i < 10; i++ )
	{
		string Prefix;

		if( i == 1 )
			Prefix = "bnet_";
		else
			Prefix = "bnet" + UTIL_ToString( i ) + "_";

		string Server = CFG->GetString( Prefix + "server", string( ) );
		string CDKeyROC = CFG->GetString( Prefix + "cdkeyroc", string( ) );
		string CDKeyTFT = CFG->GetString( Prefix + "cdkeytft", string( ) );
		string CountryAbbrev = CFG->GetString( Prefix + "countryabbrev", "RU" );
		string Country = CFG->GetString( Prefix + "country", "Russia" );
		string UserName = CFG->GetString( Prefix + "username", string( ) );
		string UserPassword = CFG->GetString( Prefix + "password", string( ) );
		string FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
		string RootAdmin = CFG->GetString( Prefix + "rootadmin", string( ) );
		string BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "-" );

		string ClanTag = CFG->GetString( Prefix + "clantag", "" );
		string HostbotName = CFG->GetString( Prefix + "hostbotname", "" );
		bool GreetUsers = CFG->GetInt( Prefix + "greetusers", 0 ) == 0 ? false : true;
		bool SwearingKick = CFG->GetInt( Prefix + "swearingkick", 0 ) == 0 ? false : true;
		bool AnnounceGames = CFG->GetInt( Prefix + "announcegames", 0 ) == 0 ? false : true;
		bool SelfJoin = CFG->GetInt( Prefix + "selfjoin", 0 ) == 0 ? false : true;
		bool BanChat = CFG->GetInt( Prefix + "banchat", 0 ) == 0 ? false : true;		
		uint32_t ClanDefaultAccess = CFG->GetInt( Prefix + "clanmembersdefaultaccess", 4 );
		if( ClanDefaultAccess > 9)
			ClanDefaultAccess = 9;

		if( BNETCommandTrigger.empty( ) )
			BNETCommandTrigger = "!";

		unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 24 );
		BYTEARRAY EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", string( ) ), 4 );
		BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", string( ) ), 4 );
		string PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", string( ) );
		uint32_t MaxMessageLength = CFG->GetInt( Prefix + "custom_maxmessagelength", 200 );

		if( Server.empty( ) )
			break;

		CONSOLE_Print( "[CCBOT] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );
		m_BNETs.push_back( new CBNET( this, Server, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger[0], War3Version, EXEVersion, EXEVersionHash, PasswordHashType, MaxMessageLength, ClanTag, GreetUsers, SwearingKick, AnnounceGames, SelfJoin, BanChat, ClanDefaultAccess, HostbotName ) );

	}

	if( m_BNETs.empty( ) )
		CONSOLE_Print( "[CCBOT] warning - no battle.net connections found in config file" );


	CONSOLE_Print( "[CCBOT] Channel && Clan Bot - " + m_Version + " - by h4x0rz88" );

	// Update the .txt configuration file(s)
	UpdateSwearList( );	
	
	// Check for the default access system values
	UpdateCommandAccess( );

	m_Uptime = GetTime( );
}

CCCBot :: ~CCCBot( )
{
	delete m_DB;
	delete m_Language;
	
}

bool CCCBot :: Update( long usecBlock )
{
	// todotodo: do we really want to shutdown if there's a database error? is there any way to recover from this?

	if( m_DB->HasError( ) )
	{
		CONSOLE_Print( "[CCBOT] database error - " + m_DB->GetError( ) );
		return true;
	}

	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	// all battle.net sockets

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		NumFDs += (*i)->SetFD( &fd, &send_fd, &nfds );

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

#ifdef WIN32
	select( 1, &fd, &send_fd, NULL, &tv );
#else
	select( nfds + 1, &fd, &send_fd, NULL, &tv );
#endif

	if( NumFDs == 0 )
	{
		// we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection and there aren't any games running)
		// select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time

		MILLISLEEP( 70 );
	}

	bool BNETExit = false;

	// update battle.net connections

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( (*i)->Update( &fd, &send_fd ) )
			BNETExit = true;
	}

	return m_Exiting || BNETExit;
}

void CCCBot :: UpdateSwearList( )
{
#ifdef WIN32
	string line, filestr = "cfg\\swears.cfg";
#else
	string line, filestr = "cfg/swears.cfg";
#endif

	ifstream file( filestr.c_str( ) );

	if( !file.fail( ) )
	{
		m_SwearList.clear( );

		while( !file.eof( ) )
		{
			getline( file, line );
			if( !line.empty( ) )
			{
				if( find( m_SwearList.begin( ), m_SwearList.end( ), line ) != m_SwearList.end( ) && line.substr(0,1) != "#" )
					CONSOLE_Print( "[CONFIG] duplicate swear: " + line );			
				else if( line.substr(0,1) != "#" )
					m_SwearList.push_back( line );
			}
		}
		CONSOLE_Print( "[CONFIG] updated swear list file (" + UTIL_ToString( m_SwearList.size( ) ) + ")" );
	}
	else
	{		
		ofstream file( filestr.c_str( ), ios :: app );

		if( file.fail( ) )
			CONSOLE_Print( "[CONFIG] error updating swears from swear list" );
		else
		{
			CONSOLE_Print( "[CONFIG] creating a new, blank swears.txt file" );
			file << "#############################################################################################################" << endl;
			file << "### THIS FILE CONTAINS ALL BANNED PHRASES AND WORDS! WRITE EVERYTHING HERE IN lowercase OR IT WONT WORK! ####" << endl;
			file << "#############################################################################################################" << endl;
			file << "# setting the # character on the first position will comment out your line" << endl;

		}
	}
	file.close( );
	file.clear( );
}

void CCCBot :: ReloadConfigs( )
{
#ifdef WIN32
		string CFGFile = "cfg\\ccbot.cfg";
#else
		string CFGFile = "cfg/ccbot.cfg";
#endif
	CConfig CFG;
	CFG.Read( CFGFile );
	SetConfigs( &CFG );
}

void CCCBot :: SetConfigs( CConfig *CFG )
{
	// this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections
	// it just set the easily reloadable values

#ifdef WIN32
	m_Language = new CLanguage( "cfg\\language.cfg" );
#else
	m_Language = new CLanguage( "cfg/language.cfg" );
#endif
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );
	tcp_nodelay = CFG->GetInt( "tcp_nodelay", 0 ) == 0 ? false : true;	
}

void CCCBot :: UpdateCommandAccess( )
{
	// load default access values and insert them into database if a access number hasn't already been set
	// format: m_Commands["<command keyword>"] = <value> (must be higher or equal 0!!!)
	
	m_Commands[ "accept" ] = 9;
	m_Commands[ "access" ] = 0;
	m_Commands[ "addsafelist" ] = 6;
	m_Commands[ "announce" ] = 6;
	m_Commands[ "ban" ] = 7;
	m_Commands[ "chanlist" ] = 3;
	m_Commands[ "channel" ] = 5;
	m_Commands[ "checkaccess" ] = 5;
	m_Commands[ "checkban" ] = 3;
	m_Commands[ "checksafelist" ] = 5;
	m_Commands[ "clanlist" ] = 1;
	m_Commands[ "clearqueue" ] = 3;
	m_Commands[ "command" ] = 7;
	m_Commands[ "countaccess" ] = 7;
	m_Commands[ "countbans" ] = 5;
	m_Commands[ "countsafelist" ] = 5;
	m_Commands[ "delaccess" ] = 8;
	m_Commands[ "delsafelist" ] = 6;
	m_Commands[ "exit" ] = 9;
	m_Commands[ "games" ] = 4;
	m_Commands[ "getclan" ] = 2;
	m_Commands[ "gn8" ] = 1;
	m_Commands[ "greet" ] = 6;
	m_Commands[ "grunt" ] = 7;
	m_Commands[ "invite" ] = 2;
	m_Commands[ "kick" ] = 5;
	m_Commands[ "lockdown" ] = 8;
	m_Commands[ "motd" ] = 5;
	m_Commands[ "online" ] = 1;
	m_Commands[ "peon" ] = 7;
	m_Commands[ "ping" ] = 0;
	m_Commands[ "reload" ] = 4;
	m_Commands[ "remove" ] = 9;
	m_Commands[ "say" ] = 6;
	m_Commands[ "setaccess" ] = 8;
	m_Commands[ "setcommand" ] = 9;
	m_Commands[ "shaman" ] = 9;
	m_Commands[ "slap" ] = 0;
	m_Commands[ "spit" ] = 0;
	m_Commands[ "squelch" ] = 5;
	m_Commands[ "topic" ] = 5;
	m_Commands[ "unban" ] = 7;
	m_Commands[ "uptime" ] = 1;

#ifdef WIN32
	m_Commands[ "restart" ] = 9;
#endif


	for( map<string, uint32_t> :: iterator i = m_Commands.begin( ); i != m_Commands.end( ); i++ )
		if( m_DB->CommandAccess( (*i).first ) == 11 )
		{
			m_DB->CommandSetAccess( (*i).first, (*i).second );
			CONSOLE_Print( "[ACCESS] no value found for command [" + (*i).first + "] - setting default value of [" + UTIL_ToString( (*i).second ) + "]" );			
		}
		
}

void CCCBot :: EventBNETConnecting( CBNET *bnet )
{

}

void CCCBot :: EventBNETConnected( CBNET *bnet )
{
	
}

void CCCBot :: EventBNETDisconnected( CBNET *bnet )
{
		
}

void CCCBot :: EventBNETLoggedIn( CBNET *bnet )
{
		
}

void CCCBot :: EventBNETConnectTimedOut( CBNET *bnet )
{

}

