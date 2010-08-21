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
#include "bnetprotocol.h"

#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
 #include <process.h>
 #include "curses.h"
#else
 #include <sys/time.h>
 #include <curses.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

bool gCurses = false;
vector<string> gMainBuffer;
string gInputBuffer;
WINDOW *gMainWindow = NULL;
WINDOW *gBottomBorder = NULL;
WINDOW *gRightBorder = NULL;
WINDOW *gInputWindow = NULL;
WINDOW *gChannelWindow = NULL;
bool gMainWindowChanged = false;
bool gInputWindowChanged = false;
bool gChannelWindowChanged = false;

string gLogFile;
CCCBot *gCCBot = NULL;

uint32_t GetTime( )
{
	// return (uint32_t)( (GetTicks( ) / 1000) - gStartTime );
	return GetTicks( ) / 1000;
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

void CONSOLE_ChannelWindowChanged( )
{
	gChannelWindowChanged = true;
}

void CONSOLE_MainWindowChanged( )
{
	gMainWindowChanged = true;
}

void LOG_Print( string message )
{
	time_t Now;
	time( &Now );
    char datestr[100];
    strftime( datestr , 100, "%m%d%y", localtime( &Now ) );
    char str[80];
	char timestr[100];
    strftime( timestr , 100, "%H:%M:%S", localtime( &Now ) );

    if( !gLogFile.empty( ) )
    {
		ofstream Log;
                
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
			Log << "[" << timestr << "] " << message << endl;
            Log.close( );
		}
	}
}

void CONSOLE_Draw( )
{
	if( !gCurses )
		return;

	// draw main window

	if( gMainWindowChanged )
	{
		wclear( gMainWindow );
		wmove( gMainWindow, 0, 0 );

		for( vector<string> :: iterator i = gMainBuffer.begin( ); i != gMainBuffer.end( ); ++i )
		{
			for( string :: iterator j = (*i).begin( ); j != (*i).end( ); ++j )
				waddch( gMainWindow, *j );

			if( i != gMainBuffer.end( ) - 1 )
				waddch( gMainWindow, '\n' );
		}

		wrefresh( gMainWindow );
		gMainWindowChanged = false;
	}

	// draw input window

	if( gInputWindowChanged )
	{
		wclear( gInputWindow );
		wmove( gInputWindow, 0, 0 );

		for( string :: iterator i = gInputBuffer.begin( ); i != gInputBuffer.end( ); ++i )
			waddch( gInputWindow, *i );

		wrefresh( gInputWindow );
		gInputWindowChanged = false;
	}

	// draw channel window

	if( gChannelWindowChanged )
	{
		wclear( gChannelWindow );
		mvwaddnstr( gChannelWindow, 0, gCCBot->m_BNETs[0]->GetCurrentChannel( ).size( ) < 16 ? ( 16 - gCCBot->m_BNETs[0]->GetCurrentChannel( ).size( ) ) / 2 : 0, gCCBot->m_BNETs[0]->GetCurrentChannel( ).c_str( ), 16 );
		mvwhline( gChannelWindow, 1, 0, 0, 16 );
		int y = 2;

		// for( vector<string> :: iterator i = gChannelUsers.begin( ); i != gChannelUsers.end( ); ++i )
		
		for( map<string, CUser *> :: iterator i = gCCBot->m_BNETs[0]->m_Channel.begin( ); i != gCCBot->m_BNETs[0]->m_Channel.end( ); ++i )
		{
			mvwaddnstr( gChannelWindow, y, 0, (*i).second->GetUser( ).c_str( ), 16 );
			++y;

			if( y >= LINES - 3 )
				break;
		}

		wrefresh( gChannelWindow );
		gChannelWindowChanged = false;
	}
}

void CONSOLE_PrintNoCRLF( string message, bool log )
{
	gMainBuffer.push_back( message );

	if( gMainBuffer.size( ) > 100 )
		gMainBuffer.erase( gMainBuffer.begin( ) );

	gMainWindowChanged = true;
	CONSOLE_Draw( );

	if( log )
		LOG_Print( message );

	if( !gCurses )
		cout << message;
}

void CONSOLE_Print( string message )
{
	CONSOLE_PrintNoCRLF( message );

	if( !gCurses )
		cout << endl;
}

void DEBUG_Print( string message )
{
	CONSOLE_PrintNoCRLF( message, false );

	if( !gCurses )
		cout << endl;
}

void CONSOLE_Resize( )
{
	if( !gCurses )
		return;

	wresize( gMainWindow, LINES - 3, COLS - 17 );
	wresize( gBottomBorder, 1, COLS );
	wresize( gRightBorder, LINES - 3, 1 );
	wresize( gInputWindow, 2, COLS );
	wresize( gChannelWindow, LINES - 3, 16 );
	// mvwin( gMainWindow, 0, 0 );
	mvwin( gBottomBorder, LINES - 3, 0 );
	mvwin( gRightBorder, 0, COLS - 17 );
	mvwin( gInputWindow, LINES - 2, 0 );
	mvwin( gChannelWindow, 0, COLS - 16 );
	mvwhline( gBottomBorder, 0, 0, 0, COLS );
	mvwvline( gRightBorder, 0, 0, 0, LINES );
	wrefresh( gBottomBorder );
	wrefresh( gRightBorder );
	gMainWindowChanged = true;
	gInputWindowChanged = true;
	gChannelWindowChanged = true;
	CONSOLE_Draw( );
}

//
// main
//

int main( )
{
	// read config file

	CConfig CFG;
	CFG.Read( CFGFile );
	gLogFile = CFG.GetString( "bot_log", string( ) );	

	// catch SIGABRT and SIGINT

	signal( SIGABRT, SignalCatcher );
	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

	// initialize curses

	gCurses = true;
	initscr( );
#ifdef WIN32
	resize_term( 28, 97 );
#endif
	clear( );
	noecho( );
	cbreak( );
	gMainWindow = newwin( LINES - 3, COLS - 17, 0, 0 );
	gBottomBorder = newwin( 1, COLS, LINES - 3, 0 );
	gRightBorder = newwin( LINES - 3, 1, 0, COLS - 17 );
	gInputWindow = newwin( 2, COLS, LINES - 2, 0 );
	gChannelWindow = newwin( LINES - 3, 16, 0, COLS - 16 );
	mvwhline( gBottomBorder, 0, 0, 0, COLS );
	mvwvline( gRightBorder, 0, 0, 0, LINES );
	wrefresh( gBottomBorder );
	wrefresh( gRightBorder );
	scrollok( gMainWindow, TRUE );
	keypad( gInputWindow, TRUE );
	scrollok( gInputWindow, TRUE );
	CONSOLE_Draw( );
	nodelay( gInputWindow, TRUE );

	// print something for logging purposes

	CONSOLE_Print( "[CCBOT] starting up" );

#ifdef WIN32

	// increase process priority

	CONSOLE_Print( "[CCBOT] setting process priority to \"high\"" );
	SetPriorityClass( GetCurrentProcess( ), HIGH_PRIORITY_CLASS );

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

	while( true )
	{
		// block for 100ms on all sockets - if you intend to perform any timed actions more frequently you should change this
		// that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

		if( gCCBot->Update( 10000 ) )
			break;

		bool Quit = false;
		int c = wgetch( gInputWindow );
		
		while( c != ERR )
		{ 
			if( c == 8 || c == 127 || c == KEY_BACKSPACE || c == KEY_DC )
			{
				// Backspace, Delete

				if( !gInputBuffer.empty( ) )
					gInputBuffer.erase( gInputBuffer.size( ) - 1, 1 );
			}
			else if( c == 9 )
			{
				// Tab
			}
#ifdef WIN32
			else if( c == 10 || c == 13 || c == PADENTER )
#else
			else if( c == 10 || c == 13 )
#endif
			{
				// CR, LF
				// process input buffer now

				string Command = gInputBuffer;

				if( Command[0] == gCCBot->m_BNETs[0]->GetCommandTrigger( ) )
				{
					CONSOLE_Print( "[CONSOLE] " + Command );
					
					CIncomingChatEvent event = CIncomingChatEvent( CBNETProtocol :: CONSOLE_INPUT, 0, 0, gCCBot->m_BNETs[0]->GetRootAdmin( ), Command );
					gCCBot->m_BNETs[0]->ProcessChatEvent( &event );
				}
				else
					gCCBot->m_BNETs[0]->QueueChatCommand( Command, BNET );				

				gInputBuffer.clear( );
			}
#ifdef WIN32
			else if( c == 22 )
			{
				// Paste

				char *clipboard = NULL;
				long length = 0;

				if( PDC_getclipboard( &clipboard, &length ) == PDC_CLIP_SUCCESS )
				{
					gInputBuffer += string( clipboard, length );
					PDC_freeclipboard( clipboard );
				}
			}
#endif
			else if( c == 27 )
			{
				// Escape button

				gInputBuffer.clear( );
			}
			else if( c >= 32 && c <= 255 )
			{
				// Printable characters

				gInputBuffer.push_back( c );
			}
#ifdef WIN32
			else if( c == PADSLASH )
				gInputBuffer.push_back( '/' );
			else if( c == PADSTAR )
				gInputBuffer.push_back( '*' );
			else if( c == PADMINUS )
				gInputBuffer.push_back( '-' );
			else if( c == PADPLUS )
				gInputBuffer.push_back( '+' );
#endif
			else if( c == KEY_RESIZE )
				CONSOLE_Resize( );

			// clamp input buffer size

			if( gInputBuffer.size( ) > 200 )
				gInputBuffer.erase( 200 );

			c = wgetch( gInputWindow );
			gInputWindowChanged = true;
		}

		CONSOLE_Draw( );

		if( Quit )
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

	// shutdown curses

	endwin( );

	return 0;
}

//
// CCBot
//

CCCBot :: CCCBot( CConfig *CFG ) : m_Version( "1.00" )
{
	m_DB = new CCCBotDBSQLite( CFG );
	m_Exiting = false;
	m_Language = new CLanguage( LanguageFile );
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)

	for( uint32_t i = 1; i < 10; ++i )
	{
		string Prefix;

		if( i == 1 )
			Prefix = "bnet_";
		else
			Prefix = "bnet" + UTIL_ToString( i ) + "_";

		string Server = CFG->GetString( Prefix + "server", string( ) );
		transform( Server.begin( ), Server.end( ), Server.begin( ), (int(*)(int))tolower );
		string CDKeyROC = CFG->GetString( Prefix + "cdkeyroc", string( ) );
		string CDKeyTFT = CFG->GetString( Prefix + "cdkeytft", string( ) );
		string CountryAbbrev = CFG->GetString( Prefix + "countryabbrev", "FIN" );
		string Country = CFG->GetString( Prefix + "country", "Finland" );
		string UserName = CFG->GetString( Prefix + "username", string( ) );
		string UserPassword = CFG->GetString( Prefix + "password", string( ) );
		string FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
		string RootAdmin = CFG->GetString( Prefix + "rootadmin", string( ) );
		string BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "!" );

		string ClanTag = CFG->GetString( Prefix + "clantag", "" );
		string HostbotName = CFG->GetString( Prefix + "hostbotname", "" );
		bool AntiSpam = CFG->GetInt( Prefix + "antispam", 0 ) == 0 ? false : true;
		bool GreetUsers = CFG->GetInt( Prefix + "greetusers", 0 ) == 0 ? false : true;
		bool SwearingKick = CFG->GetInt( Prefix + "swearingkick", 0 ) == 0 ? false : true;
		bool AnnounceGames = CFG->GetInt( Prefix + "announcegames", 0 ) == 0 ? false : true;
		bool SelfJoin = CFG->GetInt( Prefix + "selfjoin", 0 ) == 0 ? false : true;
		bool BanChat = CFG->GetInt( Prefix + "banchat", 0 ) == 0 ? false : true;		
		unsigned int ClanDefaultAccess = CFG->GetInt( Prefix + "clanmembersdefaultaccess", 4 );

		if( ClanDefaultAccess > 9 )
			ClanDefaultAccess = 9;

		unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 24 );
		BYTEARRAY EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", string( ) ), 4 );
		BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", string( ) ), 4 );
		string PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", string( ) );
		uint32_t MaxMessageLength = CFG->GetInt( Prefix + "custom_maxmessagelength", 200 );

		if( Server.empty( ) )
			break;

		CONSOLE_Print( "[CCBOT] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );
		m_BNETs.push_back( new CBNET( this, Server, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger[0], War3Version, EXEVersion, EXEVersionHash, PasswordHashType, MaxMessageLength, ClanTag, GreetUsers, SwearingKick, AnnounceGames, SelfJoin, BanChat, ClanDefaultAccess, HostbotName, AntiSpam ) );

	}

	if( m_BNETs.empty( ) )
		CONSOLE_Print( "[CCBOT] warning - no battle.net connections found in config file" );

	CONSOLE_Print( "[CCBOT] Channel && Clan Bot - " + m_Version + ", based on GHost++" );

	// Update the swears.cfg file
	UpdateSwearList( );
	
	// Check for the default access system values
	UpdateCommandAccess( );

	m_Uptime = GetTime( );
}

CCCBot :: ~CCCBot( )
{
	delete m_DB;
	delete m_Language;

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        	delete *i;
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

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
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
		// we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection)
		// select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time

		MILLISLEEP( 100 );
	}

	bool BNETExit = false;

	// update battle.net connections

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
	{
		if( (*i)->Update( &fd, &send_fd ) )
			BNETExit = true;
	}

	return m_Exiting || BNETExit;
}

void CCCBot :: Restart ( )
{
	// shutdown ghost

	CONSOLE_Print( "[CCBOT] restarting" );
	delete gCCBot;
	gCCBot = NULL;	

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[CCBOT] shutting down winsock" );
	WSACleanup( );

	_spawnl( _P_OVERLAY, "ccbot.exe", "ccbot.exe", NULL );
#else		
	execl( "ccbot++", "ccbot++", NULL );				
#endif	

}

void CCCBot :: ReloadConfigs( )
{
	CConfig CFG;
	CFG.Read( CFGFile );
	SetConfigs( &CFG );
}

void CCCBot :: SetConfigs( CConfig *CFG )
{
	// this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections

	m_Language = new CLanguage( LanguageFile );
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );
}

void CCCBot :: UpdateCommandAccess( )
{
	// load default access values and insert them into database if a access number hasn't already been set
	// format: m_Commands["<command keyword>"] = <value> (must be higher or equal 0 and lower then 10)
	
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
	m_Commands[ "chieftain" ] = 9;
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
	m_Commands[ "rejoin" ] = 4;
	m_Commands[ "restart" ] = 9;
	m_Commands[ "remove" ] = 9;
	m_Commands[ "reload" ] = 4;
	m_Commands[ "say" ] = 6;
	m_Commands[ "setaccess" ] = 8;
	m_Commands[ "setcommand" ] = 9;
	m_Commands[ "shaman" ] = 9;
	m_Commands[ "slap" ] = 0;
	m_Commands[ "spit" ] = 0;
	m_Commands[ "status" ] = 2;
	m_Commands[ "squelch" ] = 5;
	m_Commands[ "topic" ] = 5;
	m_Commands[ "unban" ] = 7;
	m_Commands[ "uptime" ] = 1;	

	for( map<string, uint32_t> :: iterator i = m_Commands.begin( ); i != m_Commands.end( ); ++i )
	{
		if( m_DB->CommandAccess( (*i).first ) == 11 )
		{
			m_DB->CommandSetAccess( (*i).first, (*i).second );
			CONSOLE_Print( "[ACCESS] no value found for command [" + (*i).first + "] - setting default value of [" + UTIL_ToString( (*i).second ) + "]" );			
		}
	}
}

void CCCBot :: UpdateSwearList( )
{
	ifstream file;
	file.open( SwearsFile, ios :: app );

	if( !file.fail( ) )
	{
		m_SwearList.clear( );
		string line;

		while( !file.eof( ) )
		{
			getline( file, line );

			if( !line.empty( ) )
			{
				if( find( m_SwearList.begin( ), m_SwearList.end( ), line ) == m_SwearList.end( ) && line.substr(0,1) != "#" ) 
				{
					transform( line.begin( ), line.end( ), line.begin( ), (int(*)(int))tolower );
					m_SwearList.push_back( line );
				}					
			}
		}

		file.close( );

		if( m_SwearList.size( ) )
			CONSOLE_Print( "[CONFIG] updated swear list file (" + UTIL_ToString( m_SwearList.size( ) ) + ")" );
		else
		{
			ofstream new_file;
			new_file.open( SwearsFile );

			if( !new_file.fail( ) )
			{
				CONSOLE_Print( "[CONFIG] creating a new, blank swears.txt file" );

				new_file << "#########################################################" << endl;
				new_file << "### THIS FILE CONTAINS ALL BANNED PHRASES AND WORDS! ####" << endl;
				new_file << "#########################################################" << endl;
				new_file << "# setting the # character on the first position will comment out your line" << endl;
			}

			new_file.close( );
		}
	}	
}

vector<CBNET *> :: iterator CCCBot :: GetServerFromNamePartial( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	int Matches = 0;
	vector<CBNET *> :: iterator it = m_BNETs.end( );

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
	{
			if( (*i)->GetServer( ).find( name ) != string :: npos )
			{
				++Matches;
				it = i;

				if( (*i)->GetServer( ) == name )
					return i;
			}		
	}

	return it;
}
