/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#ifndef LANGUAGE_H
#define LANGUAGE_H

//
// CLanguage
//

class CLanguage
{
private:
	CConfig *m_CFG;

public:
	CLanguage( string nCFGFile );
	~CLanguage( );

	void Replace( string &Text, string Key, string Value );

	string YouDontHaveAccessToThatCommand( );
	string ErrorAddingUser( string user, string server );
	string UnbannedUser( string victim );
	string ErrorUnbanningUser( string victim );
	string UserAlreadyBanned( string victim, string admin );
	string SuccesfullyBanned( string victim, string admin );
	string ErrorBanningUser( string victim );
	string Version( string version );
	string ConnectingToBNET( string server );
	string ConnectedToBNET( string server );
	string DisconnectedFromBNET( string server );
	string LoggedInToBNET( string server );
	string ConnectingToBNETTimedOut( string server );
	string WelcomeMessageLine1( string channel, string user );
	string WelcomeMessageLine2( string channel, string user );
	string AnnounceGame( string user, string gamename );
	string SwearKick( string user, string swear );
	string InvitationHasBeenAccepted( );
	string UserHasBeenAlreadySafelisted( string user );
	string UserHasBeenAddedToSafelist( string user );	
	string AnnounceEnabled( string user, string interval );
	string AnnounceDisabled( );
	string UserHasAccess( string user, string access );
	string ThereAreNoUsers( string access );
	string ThereIsOneUser( string access );
	string ThereAreUsers( string number, string access );
	string UserHasNoAccessSet( string user );
	string DeletedUsersAccess( string access );
	string ErrorDeletingAccess( string access );
	string UserSafelisted( string number, string access );
	string UserNotSafelisted( string number, string access );
	string LockdownEnabled( string number, string access );
	string LockdownDisabled( string number, string access );

};

#endif
