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

string CLanguage :: MessageQueueCleared(  )
{
	return m_CFG->GetString( "lang_0001", "lang_0001" );
}

string CLanguage :: GameAnnouncerEnabled(  )
{
	return m_CFG->GetString( "lang_0002", "lang_0002" );
}

string CLanguage :: GameAnnouncerDisabled(  )
{
	return m_CFG->GetString( "lang_0003", "lang_0003" );
}

string CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0004", "lang_0004" );
}

string CLanguage :: UpdatedClanList( )
{
	return m_CFG->GetString( "lang_0005", "lang_0005" );
}

string CLanguage :: ReceivedClanMembers( string count )
{
	string Out = m_CFG->GetString( "lang_0006", "lang_0006" );
	Replace( Out, "$TOTAL$", count );
	return Out;
}

string CLanguage :: MustBeAClanMember( string user )
{
	string Out = m_CFG->GetString( "lang_0007", "lang_0007" );
	Replace( Out, "$USER$", user );
	return Out;
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

string CLanguage :: CommandTrigger( string trigger )
{
	string Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	Replace(Out, "$TRIGGER$", trigger );
	return Out;
}

string CLanguage :: Ping( string user, string ping, string server )
{
	string Out = m_CFG->GetString( "lang_0024", "lang_0024" );
	Replace(Out, "$USER$", user );
	Replace(Out, "$PING$", ping );
	Replace(Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: CannotAccessPing(  )
{
	return m_CFG->GetString( "lang_0025", "lang_0025" );
}

string CLanguage :: CommandDisabled(  )
{
	return m_CFG->GetString( "lang_0026", "lang_0026" );
}

string CLanguage :: InvitationAccepted(  )
{
	return m_CFG->GetString( "lang_0027", "lang_0027" );
}

string CLanguage :: HasFollowingAccess( string access )
{
	string Out = m_CFG->GetString( "lang_0028", "lang_0028" );
	Replace(Out, "$ACCESS$", access );
	return Out;
}

string CLanguage :: UserAlreadySafelisted( string user )
{
	string Out = m_CFG->GetString( "lang_0029", "lang_0029" );
	Replace(Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserSafelisted( string user )
{
	string Out = m_CFG->GetString( "lang_0030", "lang_0030" );
	Replace(Out, "$USER$", user );
	return Out;
}

string CLanguage :: ErrorSafelisting( string user )
{
	string Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	Replace(Out, "$USER$", user );
	return Out;
}

string CLanguage :: AnnounceDisabled( )
{
	return m_CFG->GetString( "lang_0032", "lang_0032" );
}

string CLanguage :: AnnounceEnabled( string interval )
{
	string Out = m_CFG->GetString( "lang_0033", "lang_0033" );
	Replace(Out, "$INTERVAL$", interval );
	return Out;
}

string CLanguage :: UserAlreadyBanned( string victim, string admin )
{
	string Out = m_CFG->GetString( "lang_0034", "lang_0034" );
	Replace(Out, "$USER$", victim );
	Replace(Out, "$ADMIN$", admin );
	return Out;
}

string CLanguage :: SuccesfullyBanned( string victim, string admin )
{
	string Out = m_CFG->GetString( "lang_0035", "lang_0035" );
	Replace(Out, "$USER$", victim );
	Replace(Out, "$ADMIN$", admin );
	return Out;
}

string CLanguage :: ErrorBanningUser( string victim )
{
	string Out = m_CFG->GetString( "lang_0036", "lang_0036" );
	Replace(Out, "$USER$", victim );
	return Out;
}
