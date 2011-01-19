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
#ifndef CCBOT_H
#define CCBOT_H

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <string>
#include <vector>

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

// time

uint64_t GetTime( );		// seconds since January 1, 1970 usually
uint64_t GetTicks( );		// milliseconds since computer startup usually, overflows after ~50 days

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
 #define CFGFile "cfg\\ccbot.cfg"
 #define LanguageFile "cfg\\language.cfg"
 #define SwearsFile "cfg\\swears.cfg"
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
 #define CFGFile "cfg/ccbot.cfg"
 #define LanguageFile "cfg/language.cfg"
 #define SwearsFile "cfg/swears.cfg"
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

void LOG_Print( const string &message );
void CONSOLE_PrintNoCRLF( const string &message, bool log = true );
void CONSOLE_Print( const string &message );
void DEBUG_Print( const string &message );
void CONSOLE_Draw( );
void CONSOLE_Resize( );
void CONSOLE_ChannelWindowChanged( );
void CONSOLE_MainWindowChanged( );

//
// CCCBot
//

class CBNET;
class CCCBotDB;
class CLanguage;
class CConfig;

class CCCBot
{
public:

	vector<CBNET *> m_BNETs;			// all our battle.net connections (there can be more than one)
	CCCBotDB *m_DB;						// database
	CLanguage *m_Language;				// language
	bool m_Exiting;						// set to true to force ghost to shutdown next update (used by SignalCatcher)
	bool m_Enabled;						// set to false to prevent new games from being created
	string m_Version;					// CCBot version string
	uint64_t m_Uptime;					// uptime value
	map<string, unsigned char> m_Commands;		// map of every command keyword and default access value
	string m_LanguageFile;				// config value: language file
	string m_Warcraft3Path;				// config value: Warcraft 3 path
	string m_CFGPath;					// config value:path to txt files
	vector<string> m_SwearList;			// vector of words for swear kicking

	CCCBot( CConfig *CFG );
	~CCCBot( );

	// processing functions

	bool Update( );	
	void UpdateCommandAccess( );

	// other functions
	
	void SetConfigs( CConfig *CFG );
	void ReloadConfigs( );
	void UpdateSwearList( );
	vector<CBNET *> :: iterator GetServerFromNamePartial( string name );
};

#endif
