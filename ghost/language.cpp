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
#include "language.h"

//
// CLanguage
//

CLanguage :: CLanguage( string nCFGFile )
{
	m_CFG = new CConfig( );
	m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
	delete m_CFG;
}

void CLanguage :: Replace( string &Text, string Key, string Value )
{
	// don't allow any infinite loops

	if( Value.find( Key ) != string :: npos )
		return;

	string :: size_type KeyStart = Text.find( Key );

	while( KeyStart != string :: npos )
	{
		Text.replace( KeyStart, Key.size( ), Value );
		KeyStart = Text.find( Key );
	}
}

string CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0004", "lang_0004" );
}

string CLanguage :: Version( string version )
{
	string Out = m_CFG->GetString( "lang_0013", "lang_0013" );
	Replace( Out, "$VERSION$", version );
	return Out;
}

string CLanguage :: ConnectingToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0014", "lang_0014" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ConnectedToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0015", "lang_0015" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: DisconnectedFromBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0016", "lang_0016" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: LoggedInToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0017", "lang_0017" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ConnectingToBNETTimedOut( string server )
{
	string Out = m_CFG->GetString( "lang_0018", "lang_0018" );
	Replace( Out, "$SERVER$", server );
	return Out;
}
string CLanguage :: WelcomeMessageLine1( string channel, string user )
{
	string Out = m_CFG->GetString( "lang_0019", "lang_0019" );
	Replace( Out, "$CHANNEL$", channel );
	Replace( Out, "$USER$", user );
	return Out;
}
string CLanguage :: WelcomeMessageLine2( string channel, string user )
{
	string Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	Replace( Out, "$CHANNEL$", channel );
	Replace( Out, "$USER$", user );
	return Out;
}
string CLanguage :: SwearKick( string user, string swear )
{
	string Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$SWEAR$", swear );
	return Out;
}
string CLanguage :: AnnounceGame( string user, string gamename )
{
	string Out = m_CFG->GetString( "lang_0022", "lang_0022" );
	Replace(Out, "$USER$", user );
	Replace(Out, "$GAMENAME$", gamename );
	return Out;
}
