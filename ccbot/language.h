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
	CLanguage( const string &nCFGFile );
	~CLanguage( );

	void Replace( string &Text, const string &Key, const string &Value );

	string MessageQueueCleared(  );
	string YouDontHaveAccessToThatCommand( );
	string CommandTrigger( const string &trigger );
	string Ping( const string &user, const string &ping, const string &server );
	string CannotAccessPing(  );
	string CommandDisabled(  );
	string InvitationAccepted(  );
	string HasFollowingAccess( const string &access );
	string Version( const string &version );
	string ConnectingToBNET( const string &server );
	string ConnectedToBNET( const string &server );
	string DisconnectedFromBNET( const string &server );
	string LoggedInToBNET( const string &server );
	string ConnectingToBNETTimedOut( const string &server );
	string WelcomeMessageLine1( const string &channel, const string &user );
	string WelcomeMessageLine2( const string &channel, const string &user );
	string AnnounceGame( const string &user, const string &gamename );
	string SwearKick( const string &user, const string &swear );
	string UserAlreadySafelisted( const string &user );
	string UserSafelisted( const string &user );
	string ErrorSafelisting( const string &user );
	string AnnounceEnabled( const string &interval );
	string AnnounceDisabled( );
	string UserAlreadyBanned( const string &victim, const string &admin );
	string SuccesfullyBanned( const string &victim, const string &admin );
	string ErrorBanningUser( const string &victim );
	string UnbannedUser( const string &victim );
	string ErrorUnbanningUser( const string &victim );
	string GameAnnouncerEnabled(  );
	string GameAnnouncerDisabled( );
	string UpdatedClanList( );
	string ReceivedClanMembers( const string &count );
	string MustBeAClanMember( const string &user );
	string GreetingEnabled( );
	string GreetingDisabled( );
	string CFGReloaded( );
	string NotAllowedUsingSay( );
	string UnableToPartiallyMatchServer( );
	string Uptime( const string &user, const string &time );
	string GN8( const string &user );
	string UserIsSafelisted( const string &user );
	string UserNotSafelisted( const string &user );
	string ChangedRank( const string &user, const string &rank );
	string LockdownEnabled( const string &access );
	string LockdownDisabled( );
	string SetMOTD( const string &message );
	string SetTopic( const string &message );
	
	string ErrorAddingUser( const string &user, const string &server );	
	string UserHasAccess( const string &user, const string &access );
	string ThereAreNoUsers( const string &access );
	string ThereIsOneUser( const string &access );
	string ThereAreUsers( const string &number, const string &access );
	string UserHasNoAccessSet( const string &user );
	string DeletedUsersAccess( const string &access );
	string ErrorDeletingAccess( const string &access );
};

#endif
