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
#include "util.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "commandpacket.h"
#include "ghostdb.h"
#include "bncsutilinterface.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <time.h>
	#include <process.h>
#endif

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{

	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	string User = chatEvent->GetUser( );
	string lowerUser = User;
	transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );
	string Message = chatEvent->GetMessage( );
	uint32_t Ping = chatEvent->GetPing( );
	uint32_t UserFlags = chatEvent->GetUserFlags( );
	uint32_t Access = m_GHost->m_DB->AccessCheck( m_Server, User );
	if( Access == 11 )
		Access = 0;
	m_ClanCommandsEnabled = IsClanMember( m_UserName );

	if( Event == CBNETProtocol :: EID_WHISPER )
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "] [" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_TALK )
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + "] [" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_EMOTE )
			CONSOLE_Print( "[EMOTE: " + m_ServerAlias + "] [" + User + "] " + Message );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK || Event == CBNETProtocol :: EID_EMOTE )
	{	
		
		if( !Message.empty( ) && Message[0] == m_CommandTrigger && Event != CBNETProtocol :: EID_EMOTE)
		{
			// extract the command trigger, the command, and the payload
			// e.g. "!say hello world" -> command: "say", payload: "hello world"

			string Command;
			string Payload;
			string :: size_type PayloadStart = Message.find( " " );

			if( PayloadStart != string :: npos )
			{
				Command = Message.substr( 1, PayloadStart - 1 );
				Payload = Message.substr( PayloadStart + 1 );
			}
			else
				Command = Message.substr( 1 );

			transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

			

				/**********************
				******* COMMANDS ******
				**********************/

				//
				// !TEST
				//

				if( Command == "test" && !Payload.empty( ) && Access >= 9 )
				{
					CChannel *Result = GetUserByName( Payload );

					if( Result && !Result->GetClan( ).empty( ) )
						QueueChatCommand( "User is in a uber-clan with a clan tag of: " + Result->GetClan( ), User, Whisper );
					else if( Result && Result->GetClan( ).empty( ) )
						QueueChatCommand( "User is lame and doesn't have a clan.", User, Whisper );
					else
						QueueChatCommand( "User not in channel.", User, Whisper );
					
				}			
				
				//
				// !ACCEPT
				//

				if( Command == "accept" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "accept" ) && !IsClanMember( m_UserName ) )
				{
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATIONRESPONSE( m_InvitationClanTag, m_InvitationInviter, true ) );
					QueueChatCommand( "Invitation has been accepted if it's still active.", User, Whisper );
					m_DeclineInvitation = false;
				}

				//
				// !ACCESS
				//

				if( Command == "access" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "access" ) )
				{
					SendChatCommand( "/w " + User +" With an access of [" + UTIL_ToString( Access ) + "] you have the following commands:" );
					string Commands;

					for( int i = 0; i <= Access; i++)
					{
						vector<string> m_CommandList = m_GHost->m_DB->CommandList( i );

						for( vector<string> :: iterator it = m_CommandList.begin( ); it != m_CommandList.end( ); it++ )
							Commands = Commands + m_CommandTrigger + (*it) + ", ";

						Commands = Commands.substr( 0, Commands.size( ) -2 );

						if( m_CommandList.size ( ) >= 1 )
							SendChatCommand( "/w " + User + " [" + UTIL_ToString( i ) + "]: " + Commands );

						Commands.erase( );
					}			
				}

				//
				// !ADDSAFELIST
				// !ADDS
				//
				

				if( Command == "addsafelist" || Command == "adds" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "addsafelist" ) )
				{
					
						if( m_GHost->m_DB->SafelistCheck( m_Server, Payload ) )
							QueueChatCommand( "User is already safelisted.", User, Whisper );
						else
						{
							if( m_GHost->m_DB->SafelistAdd( m_Server, Payload ) )
								QueueChatCommand( Payload + " added to the safelist.", User, Whisper );
							else
								QueueChatCommand( "Error adding " + Payload + " to the safelist.", User, Whisper );
						}
										
				}
				
				//
				// !ANNOUNCE
				//

				if( Command == "announce" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "announce" ) )
				{					
					string Interval;
					string AnnounceMessage;
					stringstream SS;
					SS << Payload;
					SS >> Interval;

					if( !SS.eof( ) )
					{
						getline( SS, AnnounceMessage );
						string :: size_type Start = AnnounceMessage.find_first_not_of( " " );

						if( Start != string :: npos )
							AnnounceMessage = AnnounceMessage.substr( Start );
					}

					if( ( UTIL_ToInt16( Interval ) > 0 ) && ( AnnounceMessage.length() > 1 ) )
					{
						m_Announce = true;
						m_AnnounceMsg = AnnounceMessage;	
						m_AnnounceInterval = UTIL_ToInt16( Interval );
						QueueChatCommand( "Announcing has been enabled every " + Interval + " seconds.", User, Whisper );				
					}
					else if( Interval == "off" && m_Announce == true )
					{
						m_Announce = false;
						QueueChatCommand( "Announcing has been disabled.", User, Whisper );
					}

				}

				//
				// !BAN
				// !ADDBAN
				//

				if( ( Command == "ban" || Command == "addban" )&& !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "ban" ) )
				{
					
					string Victim;
					string Reason;
					stringstream SS;
					SS << Payload;
					SS >> Victim;
					
					if( !IsClanShaman( Victim ) && !IsClanChieftain( Victim ) )					
					{
						QueueChatCommand( "/ban " + Payload );

						if( !SS.eof( ) )
						{
							getline( SS, Reason );
							string :: size_type Start = Reason.find_first_not_of( " " );
	
							if( Start != string :: npos )
								Reason = Reason.substr( Start );
						}
	
						CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, Victim );
	
						if( Ban )
						{
							QueueChatCommand( "[" + Victim + "] was already banned from the channel by " + Ban->GetAdmin( ) + "." , User, Whisper );
							delete Ban;
							Ban = NULL;
						}
						else
						{
							if( m_GHost->m_DB->BanAdd( m_Server, Victim, User, Reason ) )
								QueueChatCommand( "[" + Victim + "] successfully banned from the channel by " + User + ".", User, Whisper );
							else
								QueueChatCommand( "Error banning [" + Victim + "] from the channel.", User, Whisper );
						}
					}
				}
				
				//
				// !CHECK
				//

				if( Command == "check" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "checkban" ) )
				{
					CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, Payload );
					Access = m_GHost->m_DB->AccessCheck( m_Server, Payload );
					if( Access == 11 )
						Access = 0;

					if( IsClanMember( Payload ) )
					{
						if( IsClanPeon( Payload ) )							
							if( Ban )							
								QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( Access ) + "] access and banned from channel.", User, Whisper );		
							else
								QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );

						if( IsClanGrunt( Payload ) )
							if( Ban )
								QueueChatCommand( Payload + " is in clan, rank Grunt, with [" + UTIL_ToString( Access ) + "] access and is banned from channel.", User, Whisper );		
							else
								QueueChatCommand( Payload + " is in clan, rank Grunt with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );					

						if( IsClanShaman( Payload ) )
								QueueChatCommand( Payload + " is in clan, rank Shaman, with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );

						if( IsClanChieftain( Payload ) )
							QueueChatCommand( Payload + " is in clan, rank Chieftain, with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );
					}
					else
					{
							if( !Ban )
								QueueChatCommand( Payload + " is not a clan member and has [" + UTIL_ToString( Access ) + "] access.", User, Whisper );
							else
								QueueChatCommand( Payload + " is banned from the channel, not a clan member and has [" + UTIL_ToString( Access ) + "] access.", User, Whisper );
					}
				}
		
				//
				// !SETACCESS
				//

				if( Command == "setaccess" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "setaccess" ) )
				{
					bool CanSetAccess = true;

					// extract the username and the access which we are setting
					// ie. !setaccess Panda 8 -> username "Panda" , access "8" 

					uint32_t NewAccess;
					string UserName;
					stringstream SS;
					SS << Payload;
					SS >> NewAccess;

					if( SS.fail( ) || SS.eof( ) )
					{
						string Response = "Bad input - use form  setaccess <access 0-10> <name>";
						Response[21] = m_CommandTrigger;
						QueueChatCommand( Response, User, Whisper );

					}
					else
					{
						getline( SS, UserName );
						string :: size_type Start = UserName.find_first_not_of( " " );

						if( Start != string :: npos )
							UserName = UserName.substr( Start );

						uint32_t OldAccess = m_GHost->m_DB->AccessCheck( m_Server, UserName );

						if( NewAccess > 10 )
							NewAccess = 10;							

						if( Match( User, UserName ) && NewAccess > Access )
						{
							QueueChatCommand( "Cannot raise your own access.", User, Whisper );
							CanSetAccess = false;
						}
						if( NewAccess >= Access && !Match( UserName, User ) && NewAccess != 10 )
						{
							QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper );
							CanSetAccess = false;
						}
						if( OldAccess > NewAccess && !Match( UserName, User ) && OldAccess >= Access )
						{
							CanSetAccess = false;
							QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper );
						}
						if( OldAccess > Access && !Match( UserName, User )  )
						{
							CanSetAccess = false;
							QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper );
						}					
					
						if( CanSetAccess )
						{
							if( m_GHost->m_DB->AccessSet( m_Server, UserName, NewAccess ) )
								QueueChatCommand( "Added user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
							else
								QueueChatCommand( "Error adding user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
						}
					}					
				}
				
				//
				// !CHECKACCESS
				//

				if( Command == "checkaccess" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "checkaccess" ) )
				{
					if( m_GHost->m_DB->AccessCheck( m_Server, Payload ) != 11 )
						QueueChatCommand("User [" + Payload + "] has access of [" + UTIL_ToString( m_GHost->m_DB->AccessCheck( m_Server, Payload ) ) + "]", User, Whisper );
					else
						QueueChatCommand("User [" + Payload + "] has access of [" + UTIL_ToString( 0 ) + "]", User, Whisper );
				}

				//
				// !COUNTACCESS
				//

				if( Command == "countaccess" && Access >= m_GHost->m_DB->CommandAccess( "countaccess" ) )
				{
				

					if( Payload.size( ) == 0 )
						Payload = "0";
					else if( UTIL_ToInt32( Payload ) > 10 )
						Payload = "10";

					uint32_t Count = m_GHost->m_DB->AccessCount( m_Server, UTIL_ToInt32( Payload ) );

			
					if( Count == 0 )
						QueueChatCommand( "There are no users with access of [" + Payload + "]", User, Whisper );
					else if( Count == 1 )
						QueueChatCommand( "There is only one user with access of [" + Payload + "]", User, Whisper );
					else
						QueueChatCommand( "There are [" + UTIL_ToString( Count ) + "] users with access [" + Payload + "]", User, Whisper );
				}

				//
				// !DELACCESS
				//
				// TO DO: !DELUSER from database
				
				if( Command == "delaccess" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "delaccess" ) )
				{
					if( !m_GHost->m_DB->AccessCheck( m_Server, Payload ) )
						QueueChatCommand( "User has no set access - the default 0 access remains.", User, Whisper );
					else
					{
						if( m_GHost->m_DB->AccessRemove( Payload ) )
							QueueChatCommand( "Deleted user [" + Payload + "]'s set access and returning it to default 0." , User, Whisper );
						else
							QueueChatCommand( "Error deleting user [" + Payload + "]'s access.", User, Whisper );
					}
					
				}

				//
				// !CHECKSAFELIST
				// !CHECKS
				//

				if( Command == "checksafelist" || Command == "checks" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "checksafelist" ) )
				{
					
						if( m_GHost->m_DB->SafelistCheck( m_Server, Payload ) )
							QueueChatCommand( "User [" + Payload + "] is safelisted.", User, Whisper );
						else
							QueueChatCommand( "User [" + Payload + "] is not safelisted.", User, Whisper );
				}
				
				//
				// !COMMAND
				//	

				if( Command == "command" && !Payload.empty( ) && ( Access >= m_GHost->m_DB->CommandAccess( "command" ) ) )
				{
					transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );

					bool Exists = false;

					for( map<string, uint32_t> :: iterator i = m_GHost->m_Commands.begin( ); i != m_GHost->m_Commands.end( ); i++ )
						if( (*i).first == Payload )
						{
							Exists = true;
						}

					if( Exists )
						QueueChatCommand( "Access needed for the [" + Payload + "] command is [" + UTIL_ToString( m_GHost->m_DB->CommandAccess( Payload ) ) + "]", User, Whisper );
					else
						QueueChatCommand( "Error checking access for [" + Payload + "] command, doesn't exist.", User, Whisper );
				}

				//
				// !SETCOMMAND
				//	

				if( Command == "setcommand" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "setcommand" ) )
				{
					bool Exists = false;

					uint32_t NewAccess;
					string Command;
					stringstream SS;
					SS << Payload;
					SS >> NewAccess;

					if( SS.fail( ) || SS.eof( ) )
					{
						string Response = "Bad input - use form  setcommand <access 0-10> <command>";
						Response[21] = m_CommandTrigger;
						QueueChatCommand( Response, User, Whisper );

					}
					else
					{
						getline( SS, Command );
						string :: size_type Start = Command.find_first_not_of( " " );

						if( Start != string :: npos )
							Command = Command.substr( Start );

						if( NewAccess > 10 )
							NewAccess = 10;
					
						for( map<string, uint32_t> :: iterator i = m_GHost->m_Commands.begin( ); i != m_GHost->m_Commands.end( ); i++ )
							if( (*i).first == Command )
								Exists = true;
										
						if( Exists )
						{
							if( m_GHost->m_DB->CommandSetAccess( Command, NewAccess ) )
								QueueChatCommand( "Updated [" + Command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
							else
								QueueChatCommand( "Error updating access for [" + Command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
						}
						else
							QueueChatCommand( "Unable to set access for [" + Command + "] because it doesn't exist.", User, Whisper );
					}
				}

				//
				// !CLEARQUEUE
				// !CQ
				//

				if( ( Command == "clearqueue" ) || ( Command == "cq" ) && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "clearqueue" ) )
				{
					while( !m_ChatCommands.empty( ) )
						m_ChatCommands.pop( );

					QueueChatCommand( "Message queue cleared...", User, Whisper );
				}

				//
				// !COUNTBANS
				//

				if( Command == "countbans" && Access >= m_GHost->m_DB->CommandAccess( "countbans" ) )
				{
					uint32_t Count = m_GHost->m_DB->BanCount( m_Server );

					if( Count == 0 )
						QueueChatCommand( "There are no banned users from the channel.", User, Whisper );
					else if( Count == 1 )
						QueueChatCommand( "There is one banned user from the channel.", User, Whisper );
					else
						QueueChatCommand( "There are " + UTIL_ToString( Count ) + " banned users from the channel.", User, Whisper );
				}					

				//
				// !COUNTSAFELIST
				// !COUNTS
				//

				if( ( Command == "countsafelist" ) || ( Command == "counts" ) && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "countsafelist" ) )
				{
					uint32_t Count = m_GHost->m_DB->SafelistCount( m_Server );

					if( Count == 0 )
						QueueChatCommand( "There are no safelisted users.", User, Whisper );
					else if( Count == 1 )
						QueueChatCommand( "There is one safelisted user.", User, Whisper );
					else
						QueueChatCommand( "There are " + UTIL_ToString( Count ) + " users safelisted.", User, Whisper );
				}

				//
				// !CHANNEL
				//

				if( Command == "channel" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "channel" ) )
				{
					QueueChatCommand( "/join " + Payload );
					m_CurrentChannel = Payload;
				}

				//
				// !CHANLIST
				//
				
				
				if( Command == "chanlist" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "chanlist" ) && m_ClanCommandsEnabled )
				{
					string tempText;
					
					for( map<string, CChannel *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); i++ )					
						tempText = tempText + (*i).second->GetUser( ) + ", ";			
					

					if( tempText.length( ) >= 2 )
						tempText = tempText.substr( 0, tempText.length( ) - 2 );

					QueueChatCommand( "Users in channel:  " + tempText + ".", User, Whisper );					

				}

				//
				// !CLANLIST
				//

				if( Command == "clanlist" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "clanlist" ) && m_ClanCommandsEnabled )
				{
					string ClanList1;
					string ClanList2;
					string Chieftains;
					string Shamans;
					string Grunts;
					string Peons;

					for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
					{
						if( (*i)->GetRank( ) == "Recruit" )
								Peons = Peons + (*i)->GetName( ) + ", ";
						if( (*i)->GetRank( ) == "Peon" )
								Peons = Peons + (*i)->GetName( ) + ", ";
						if( (*i)->GetRank( ) == "Grunt" )
								Grunts = Grunts + (*i)->GetName( ) + ", ";
						if( (*i)->GetRank( ) == "Shaman" )
								Shamans = Shamans + (*i)->GetName( ) + ", ";
						if( (*i)->GetRank( ) == "Chieftain" )
								Chieftains = Chieftains + (*i)->GetName( ) + ", ";
					}

					if( Chieftains.size( ) > 179 )
					{
						ClanList1 = Chieftains.substr( 0, 179 );
						ClanList2 = Chieftains.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Chieftains: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );
				
					}
					else
						if( Chieftains.size() >= 2 )
						SendChatCommand( "/w " + User + " Chieftains: " + Chieftains.substr( 0, Chieftains.size( )-2 ) );

					if( Shamans.size( ) > 179 )
					{
						ClanList1 = Shamans.substr( 0, 179 );
						ClanList2 = Shamans.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Shamans: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );
				
					}
					else
						if( Shamans.size() >= 2 )
						SendChatCommand( "/w " + User + " Shamans: " + Shamans.substr( 0, Shamans.size( )-2) );

					if( Grunts.size( ) > 179 )
					{
						ClanList1 = Grunts.substr( 0, 179 );
						ClanList2 = Grunts.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Grunts: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );
				
					}
					else
						if( Grunts.size() >= 2 )
						SendChatCommand( "/w " + User + " Grunts: " + Grunts.substr( 0, Grunts.size( )-2) );

					if( Peons.size( ) > 179 )
					{
						ClanList1 = Peons.substr( 0, 179 );
						ClanList2 = Peons.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Peons: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );
				
					}
					else
						if( Peons.size() >= 2 )
						SendChatCommand( "/w " + User + " Peons: " + Peons.substr( 0, Peons.size( )-2) );

				}

				//
				// !DELBAN
				// !UNBAN
				//

				if( ( Command == "delban" || Command == "unban" ) && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "unban" ) )
				{
					string Victim;
					string Reason;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if( m_GHost->m_DB->BanRemove( m_Server, Victim ) )
						{
						QueueChatCommand( "[" + Victim + "] successfully unbanned from the channel by " + User + ".", User, Whisper );
						QueueChatCommand( "/unban " + Victim );
						}
					else
						QueueChatCommand( "Error unbanning [" + Victim + "] from the channel.", User, Whisper );
				}

				//
				// !DELSAFELIST
				// !DELS
				//
				

				if( Command == "delsafelist" || Command == "dels" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "delsafelist" ) )
				{
					
					
						if( !m_GHost->m_DB->SafelistCheck( m_Server, Payload ) )
							QueueChatCommand( "User " + Payload + " is not safelisted.", User, Whisper );
						else
						{
							if( m_GHost->m_DB->SafelistRemove( m_Server, Payload ) )
								QueueChatCommand( "User " + Payload + " is deleted from safelist.", User, Whisper );
							else
								QueueChatCommand( "Error deleting user " + Payload + " from the safelist.", User, Whisper );
						}
				}

				//
				// !EXIT
				// !QUIT
				//

				if( Command == "exit" || Command == "quit" && Access >= m_GHost->m_DB->CommandAccess( "exit" ) )
					m_Exiting = true;

				//
				// !GAMES
				//

				if( Command == "games"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "games" ) )
				{
					if( Payload == "on" )
					{
						QueueChatCommand( "Game announcer is ON.", User, Whisper );
						m_AnnounceGames = true;
					}
					else if( Payload == "off" )
					{
						QueueChatCommand( "Game announcer is OFF.", User, Whisper );
						m_AnnounceGames = false;
					}
				}
				

				//
				// !GETCLAN
				//

				if( Command == "getclan" && Access >= m_GHost->m_DB->CommandAccess( "getclan" ) && m_ClanCommandsEnabled )
				{
					SendGetClanList( );
					QueueChatCommand( "Updating bot's internal clan list from Battle.Net...", User, Whisper );
					QueueWhisperCommand( "Received [" + UTIL_ToString( m_Clans.size( ) ) + "] clan members.", User );
				}	

				//
				// !GREET
				//

				if( Command == "greet"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "greet" ) )
				{
					
					if( Payload == "off" )
					{
						QueueChatCommand( "Greet users on join is disabled.", User, Whisper );
						m_GreetUsers = false;
					}
					else if( Payload == "on" )
					{
						QueueChatCommand( "Greet users on join is enabled.", User, Whisper );
						m_GreetUsers = true;
					}
				}	 				

				//
				// !GRUNT				//

				if( Command == "grunt"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "grunt" ) && m_ClanCommandsEnabled )
				{
					if(  IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_MEMBER );
						SendGetClanList( );
						CONSOLE_Print( "[CLAN: " +  m_ServerAlias + "] changing " + Payload + " to status grunt done by " + User );
						QueueChatCommand( "You have successfully changed rank of " + Payload + " to Grunt.", User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper );
				}			

				//
				// !KICK
				//

				if( Command == "kick" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "kick" ) )
				{
					string Victim;
					string Reason;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if ( GetUserFromNamePartial( Victim ).size( ) >= 1 )
							Victim = GetUserFromNamePartial( Victim );
					if( !IsClanShaman( Victim ) && !IsClanChieftain( Victim ) && !m_GHost->m_DB->SafelistCheck( m_Server, Payload ) && GetUserFromNamePartial( Victim ).size( ) >= 1  )
					{	
						if( !SS.eof( ) )
						{
								getline( SS, Reason );
								string :: size_type Start = Reason.find_first_not_of( " " );
	
								if( Start != string :: npos )
									Reason = Reason.substr( Start );
						}
						
						ImmediateChatCommand( "/kick " + Victim + " " + Reason );
						
					}
					else if ( GetUserFromNamePartial( Victim ).size( ) == 0 )
						QueueChatCommand( "Can kick only users in channel.", User, Whisper );
					else
						QueueChatCommand( "Clan shamans, chieftains and safelisted users cannot be kicked.", User, Whisper );
				}
			
				//
				// !LOCKDOWN
				//
				
				if( Command == "lockdown" && !Match( "off", Payload) && Access >= m_GHost->m_DB->CommandAccess( "lockdown" ) )
				{
					m_AccessRequired = UTIL_ToInt32( Payload );
					m_Lockdown = true;
					QueueChatCommand( "Lockdown mode engaged! Users with access lower then [" + Payload + "] will be tempBanned", User, Whisper );
	
				}
				
				//
				// !LOCKDOWN OFF
				//
				
				if( Command == "lockdown" && Match( "off", Payload) && m_Lockdown && Access >= m_GHost->m_DB->CommandAccess( "lockdown" ) )
				{
					m_Lockdown = false;
					QueueChatCommand( "Lockdown mode is deactivated! Anyone can join the channel now.", User, Whisper );
					// unban everyone before deleting the users stored
					for( vector<string> :: iterator i = LockdownNames.begin( ); i != LockdownNames.end( ); i++ )
						ImmediateChatCommand( "/unban " + *i );
					LockdownNames.clear( );
				}

				//
				// !MATCH
				// (command for testing purposes)
				//

				if( Command == "match" && !Payload.empty( ) && Access >= 8 )
				{			

					if( GetUserFromNamePartial( Payload ).size( ) >= 2 )
						QueueChatCommand( "Match found: " + GetUserFromNamePartial( Payload ) + ".", User, Whisper );
					else 					
						QueueChatCommand( "No matches or more then one match found.", User, Whisper );
						
				}

				//
				// !MOTD (clan message of the day)
				//

				if( Command == "motd" && Access >= m_GHost->m_DB->CommandAccess( "motd" ) && m_ClanCommandsEnabled )
				{
					
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANSETMOTD( Payload ) );
					QueueChatCommand( "Clan MOTD set to: \"" + Payload + "\".", User, Whisper );
					CONSOLE_Print("[CLAN: " + m_ServerAlias + "] clan MOTD changed by [" + User + "] to [" + Payload + "].");

				}
				
				//
				// !PEON				//
				if( Command == "peon"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "peon" ) && m_ClanCommandsEnabled )
				{
					if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_PARTIAL_MEMBER );
						CONSOLE_Print( "[CLAN: " +  m_ServerAlias + "] changing " + Payload + " to status peon done by " + User );
						QueueChatCommand( "You have successfully changed rank of " + Payload + " to Peon.", User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper );
				}				
				
				//
				// !REMOVE (from clan)
				//

				if( Command == "remove"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "remove" ) && m_ClanCommandsEnabled )
				{
					if( IsClanChieftain( Payload ) || IsClanShaman( Payload ) )
					{
						m_Socket->PutBytes( m_Protocol->SEND_SID_CLANREMOVEMEMBER( Payload ) );
						m_Removed = Payload;
						m_UsedRemove = User;
						CONSOLE_Print( "[GHOST] attempting to kick " + Payload + " from " + m_ClanTag + " by " + User + "." );
					}
						QueueChatCommand( "Removing Chieftains and shamans is prohibited.", User, Whisper );
				}

				//
				// !RELOAD
				//

				if( Command == "reload" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "reload" ) )
				{
					m_GHost->UpdateSwearList( );
					SendGetClanList( );
					QueueChatCommand( "CFG settings and clan list reloaded.", User, Whisper );
				}

				//
				// !SAY
				//

				if( Command == "say" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "say" ) )
				{
					if( Access > 8 )
						QueueChatCommand( Payload );
					else
					{
						if( Payload.find( "/" ) == string :: npos )
							QueueChatCommand( Payload );
						else
							QueueChatCommand( "Using / is not allowed via say command.", User, Whisper );
					}
				}

				//
				// !SHAMAN				//
				if( Command == "shaman"  && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "shaman" ) && m_ClanCommandsEnabled )
				{
					if( IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_OFFICER );
						CONSOLE_Print( "[CLAN: " +  m_ServerAlias + "] changing " + Payload + " to status shaman done by " + User );
						QueueChatCommand( "You have successfully changed rank of " + Payload + " to Shaman.", User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be Chieftain to perform this action.", User, Whisper );
				}

				//
				// !SQUELCH
				// !IGNORE
				//

				if( Command == "squelch" || Command == "ignore" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "squelch" ) )
				{
					if( !IsAlreadySquelched( Payload ) )
					{
						QueueChatCommand( "/ignore " + Payload );
						QueueChatCommand( "Ignoring user " + Payload + ".", User, Whisper );
						transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
						SquelchedUsers.push_back( string ( Payload ) );
					}
					else
						QueueChatCommand( "User [" + Payload + "] is already ignored.", User, Whisper );
						
				}

				//
				// !SQUELCHLIST
				// !SL
				//

				if( Command == "squelchlist" || Command == "sl" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "squelch" ) )
				{					
					string tempText;

					for( vector<string> :: iterator i = SquelchedUsers.begin( ); i != SquelchedUsers.end( ); i++ )
						if( (*i).size( ) > 0 )
							tempText = tempText + (*i) + ", ";					
				
					if ( tempText.size( ) == 0 || SquelchedUsers.size( ) == 0 )
						QueueChatCommand( "There are no squelched users." , User, Whisper );
					else if( tempText.size( ) >= 2 )
						QueueChatCommand( "Squelched users: " + tempText.substr( 0, tempText.size( )-2 ) + ".", User, Whisper );
				}

				//
				// !TOPIC
				//

				if( Command == "topic" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "topic" ) )
				{					
					QueueChatCommand( "/topic " + m_CurrentChannel + " \"" + Payload + "\"" );
					QueueChatCommand( "Channel topic set to [" + Payload + "].", User, Whisper );

				}				
				//
				// !UNSQUELCH
				// !UNIGNORE
				//

				if( Command == "unsquelch" || Command == "unignore" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "squelch" ) )
				{
					if( IsAlreadySquelched( Payload ) )
					{
						QueueChatCommand( "/unignore " + Payload );						

						for( vector<string> :: iterator i = SquelchedUsers.begin( ); i != SquelchedUsers.end( ); i++ )
							if( Match( (*i), Payload ) )
							{
								(*i) = "";
							}

						QueueChatCommand( "Unignoring user " + Payload + ".", User, Whisper );
					}
					else
						QueueChatCommand( "User " + Payload + " is not ignored.", User, Whisper );
				}

				//
				// !UPTIME
				//
				
				if( Command == "uptime" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "uptime" ) )
				{
					string date;
					uint32_t time = GetTime( ) - m_GHost->m_Uptime;

					if( time >= 86400 )
					{
						uint32_t days = time / 86400;
						time = time - ( days * 86400 );
						date = UTIL_ToString( days ) + "d ";
					}
					if( time >= 3600 )
					{
						uint32_t hours = time / 3600;
						time = time - ( hours * 3600 );
						date += UTIL_ToString( hours ) + "h ";
					}
					if( time >= 60 )
					{
						uint32_t minutes = time / 60;
						time = time - ( minutes * 60 );
						date += UTIL_ToString( minutes ) + "m ";
					}
					date += UTIL_ToString( time ) + "s";

					QueueChatCommand( m_UserName + " has a uptime of : " + date, User, Whisper );
				}		

					
				//
				// !CHECKBAN
				//

				if( Command == "checkban" && Access >= m_GHost->m_DB->CommandAccess( "checkban" ) )
				{
					if ( Payload.empty( ) )
					User = Payload;
					CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, Payload );
					
					if( Ban )
					{	
						string GetReasonString = Ban->GetReason( );
						if ( GetReasonString.empty( ) )
							QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " by " + Ban->GetAdmin( ) + ".", User, Whisper );
						else
							QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " because \"" + GetReasonString + "\" by " + Ban->GetAdmin( ) + ".", User, Whisper );
						delete Ban;
						Ban = NULL;
					}
					else
						QueueChatCommand( "User [" + Payload + "] is not banned.", User, Whisper );
				}

				//
				// !GN8
				//
				
				if( Command == "gn8" && Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "gn8" ) )
					QueueChatCommand( "/me - " + User + " is going to bed and wishes everyone a good night!" ); 

				//
				// !INVITE
				//

				if( Command == "invite" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "invite" )  && m_ClanCommandsEnabled && IsInChannel( Payload ) )
				{

					if ( !IsClanMember( Payload ) )
					{
						if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
						{
							m_LastKnown = Payload;
						
							if( m_LoggedIn )
							m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( Payload ) );							
								QueueChatCommand( "Clan invitation was sent.", User, Whisper );

						}
						else
					   	QueueChatCommand( "Bot needs to have at least a shaman rank to invite.", User, Whisper );
					}
					else
					   QueueChatCommand( "You can't invite people already in clan!", User, Whisper );
                                      	
				}						
				
				//
				// !JOINCLAN 
				// !JOIN
				//
				
				if( ( Command == "joinclan" || Command == "join" ) && Payload.empty( ) && !IsClanMember( User ) && IsInChannel( User ) && Access >= 0  && m_ClanCommandsEnabled )
				{

					if( m_SelfJoin && IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						m_LastKnown = User;

						if( m_LoggedIn )
							m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( User ) );

						SendGetClanList( );
					}
					else
						QueueChatCommand( "This feature has been disabled.", User, Whisper );           
                                      	
				}

				//
				// !ONLINE 
				// !O
				//
				
				if( Command == "online" || Command == "o" && Access >= m_GHost->m_DB->CommandAccess( "online" ) && m_ClanCommandsEnabled )
				{
					string Online;
					uint32_t OnlineNumber = 0;

					for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
						if( (*i)->GetStatus( ) == "Online" && !Match( (*i)->GetName( ), m_UserName ) )
						{
							Online = Online + (*i)->GetName( ) + ", ";
							OnlineNumber++;
						}

					if( OnlineNumber == 0)
						SendChatCommand( "/w " + User + " " + "There are no " + m_ClanTag + " members online." );
					else if( Online.size() < 3 )
					{
						SendChatCommand( "/w " + User + " " + m_ClanTag + " members online [" + UTIL_ToString(OnlineNumber) + "]: " + Online.substr(0,3) );

					}
					else
					{					
						if( Online.size( ) < 156 ) 
							SendChatCommand( "/w " + User + " " + m_ClanTag + " members online [" + UTIL_ToString(OnlineNumber) + "]: " + Online.substr(0,Online.size( )-2) );
						else
						{
							SendChatCommand( "/w " + User + " " + m_ClanTag + " members online [" + UTIL_ToString(OnlineNumber) + "]: " + Online.substr(0,155) );
							SendChatCommand( "/w " + User + " " + Online.substr(155,Online.size( )-157) );
						}
					}
					
				}
 
				//
				// !SLAP
				//

				if ( Command == "slap" && !Payload.empty( ) && IsInChannel( User ) && Access >= m_GHost->m_DB->CommandAccess( "slap" ) )
				{
					string Victim;
					string Object;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if( !SS.eof( ) )
					{
						getline( SS, Object );
						string :: size_type Start = Object.find_first_not_of( " " );

						if( Start != string :: npos )
							Object = Object.substr( Start );
					}

					if ( GetUserFromNamePartial( Victim ).size( ) >= 1 )
						Victim = GetUserFromNamePartial( Victim );

					
					int RandomNumber = 5;
					srand((unsigned)time(0));					
					RandomNumber = (rand()%8);
					
					if ( Victim != User )
					{
					
					  if ( RandomNumber == 0 )
					    QueueChatCommand( User + " slaps " + Victim + " with a large trout." );
					  else if ( RandomNumber == 1 )
						QueueChatCommand( User + " slaps " + Victim + " with a pink Macintosh." );
					  else if ( RandomNumber == 2 )
						QueueChatCommand( User + " throws a Playstation 3 at " + Victim + "." );
					  else if ( RandomNumber == 3 )
						QueueChatCommand( User + " drives a car over " + Victim + "." );
					  else if ( RandomNumber == 4 )
						QueueChatCommand( User + " steals " + Victim + "'s cookies. mwahahah!" );
					  else if ( RandomNumber == 5 )	
					  {
						int Goatsex = rand();
						string s;
						stringstream out;
						out << Goatsex;
						s = out.str();
						QueueChatCommand( User + " searches yahoo.com for goatsex + " + Victim + ". " + s + " hits WEEE!" );	
					  }
					  else if ( RandomNumber == 6 )
						QueueChatCommand( User + " burns " + Victim + "'s house." );
					  else if ( RandomNumber == 7 )
						QueueChatCommand( User + " finds " + Victim + "'s picture on uglypeople.com." );
					}
					else
					{
						if ( RandomNumber == 0 )
					    QueueChatCommand( User + " slaps himself with a large trout." );
					  else if ( RandomNumber == 1 )
						QueueChatCommand( User + " slaps himself with a pink Macintosh." );
					  else if ( RandomNumber == 2 )
						QueueChatCommand( User + " throws a Playstation 3 at himself." );
					  else if ( RandomNumber == 3 )
						QueueChatCommand( User + " drives a car over himself." );
					  else if ( RandomNumber == 4 )
						QueueChatCommand( User + " steals his cookies. mwahahah!" );
					  else if ( RandomNumber == 5 )	
					  {
						int Goatsex = rand() / 10000;
						string s;
						stringstream out;
						out << Goatsex;
						s = out.str();
						QueueChatCommand( User + " searches yahoo.com for goatsex + " + User + ". " + s + " hits WEEE!" );	
					  }
					  else if ( RandomNumber == 6 )
						QueueChatCommand( User + " burns his house." );
					  else if ( RandomNumber == 7 )
						QueueChatCommand( User + " finds his picture on uglypeople.com." );
					}

					  
				}
				
				//
				// !SPIT
				//

				if ( Command == "spit" && !Payload.empty( ) && IsInChannel( User ) && Access >= m_GHost->m_DB->CommandAccess( "spit" ) )
				{
					if ( GetUserFromNamePartial( Payload ).size( ) >= 1 )
						Payload = GetUserFromNamePartial( Payload );

					int RandomNumber = 7;
					srand((unsigned)time(0));  
					RandomNumber = (rand()%6);     
					  if ( RandomNumber == 0 )
					    QueueChatCommand( User + " spits " + Payload + " but misses completely!" );
					  else if ( RandomNumber == 1 )
						QueueChatCommand( User + " spits " + Payload + " and hits him in the eye!" );
					  else if ( RandomNumber == 2 )
						QueueChatCommand( User + " spits " + Payload + " and hits him in his mouth!" );
					  else if ( RandomNumber == 3 )
						QueueChatCommand( User + " spits " + Payload + " and nails it into his ear!" );
					  else if ( RandomNumber == 4 )
						QueueChatCommand( User + " spits " + Payload + " but he accidentaly hits himself!" );
					  else if ( RandomNumber == 5 )
						QueueChatCommand( Payload + " counterspits " + User + ". Lame." );
				}

				//
				// !SERVE
				//

				if ( Command == "serve" && !Payload.empty( ) && Access >= m_GHost->m_DB->CommandAccess( "serve" ) )
				{
			       		string Victim;
					string Object;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if( !SS.eof( ) )
					{
						getline( SS, Object );
						string :: size_type Start = Object.find_first_not_of( " " );

						if( Start != string :: npos )
							Object = Object.substr( Start );
					}

					if ( GetUserFromNamePartial( Victim ).size( ) >= 1 )
						Victim = GetUserFromNamePartial( Victim );
					if ( Object == "shit" && !IsRootAdmin( User ) )
					{
						  QueueChatCommand( "/kick " + User + " Abusing the power of the Shit." );
					}
					else
						  QueueChatCommand( User + " serves " + Victim + " a " + Object + "." );				 
				}				

				//
				// !VERSION
				//

				if( Command == "version" && Payload.empty( ) && Access >= 0 )					
						QueueChatCommand( m_GHost->m_Language->Version( m_GHost->m_Version ), User, Whisper );

				//
				// !PING
				//

				if( Command == "ping" && Access >= m_GHost->m_DB->CommandAccess( "ping" ) )
				{
					if( Payload.empty( ) )
						Payload = User;			
					else if( GetUserFromNamePartial( Payload ).size( ) >= 2 )
						Payload = GetUserFromNamePartial( Payload );

					CChannel *Result = GetUserByName( Payload );

					if( Result )
						QueueChatCommand( Payload + " has a ping of " + UTIL_ToString( Result->GetPing( ) ) + "ms on " + m_ServerAlias + ".", User, Whisper );
					else
						QueueChatCommand( "Can only access pings from users in channel.", User, Whisper );
				}

#ifdef WIN32
				//
				// !RESTART /just for Windows
				//		
				if( Command == "restart" && Access >= m_GHost->m_DB->CommandAccess( "restart" ) )
					_spawnl(_P_OVERLAY,"ghost.exe","ghost.exe",NULL);
#endif	
			
		}
		
		if ( m_SwearingKick ) 
		{
			transform( Message.begin( ), Message.end( ), Message.begin( ), (int(*)(int))tolower );

			// only kick non-root admin users, non-admins and non-safelisted users when not using the unban,ban and kick commands
			if( !Message.empty( ) && ( Message[0] != m_CommandTrigger ) && ( Message.find( "unban" ) == string :: npos ) && ( Message.find( "ban" ) == string :: npos ) && ( Message.find( "kick" ) == string :: npos ) && !Match( User, m_HostbotName ) &&  !m_GHost->m_DB->SafelistCheck( m_Server, User ) && IsInChannel( User ) )
			{	

				for( int i = 0; i < m_GHost->m_SwearList.size( ); i++)
					if( Message.find( m_GHost->m_SwearList[i] ) != string :: npos )
						QueueChatCommand( m_GHost->m_Language->SwearKick( User, m_GHost->m_SwearList[i] ) );							
			}
		}

	}

	else if( Event == CBNETProtocol :: EID_INFO )
	{
		
                if ( m_AnnounceGames )
		{			
			if( Message.find("private game") != string::npos || Message.find("in  game") != string::npos )
			{	
				string LastUser = Message.substr( 0, Message.find_first_of(" ") );

				if( !Match( LastUser, m_HostbotName ) )
				{			
					Message = Message.substr( Message.find_first_of("\"")+1, Message.find_last_of("\"") - Message.find_first_of("\"") - 1 );				
					QueueChatCommand( m_GHost->m_Language->AnnounceGame( LastUser, Message ) );
				}			
			}
			else
				CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );
		}
		else
			CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );	
					
	}

	else if( Event == CBNETProtocol :: EID_JOIN )
	{
		bool CanJoinChannel = false;	

		CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + "] user [" + User + "] joined the channel." );

		// check if the user joined has been banned if yes ban him again and send him the reason the first time he tries to join
		CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, User );
		if( Ban )
		{
			SendChatCommand( "/ban " + User );
			string GetReasonString = Ban->GetReason( );

			if ( GetReasonString.empty( ) )
				QueueWhisperCommand( "You were banned on " + Ban->GetDate( ) + " by " + Ban->GetAdmin( ), User );
			else
				QueueWhisperCommand( "You were banned on " + Ban->GetDate( ) + " because \"" + GetReasonString + "\" by " + Ban->GetAdmin( ), User );

			delete Ban;
			Ban = NULL;
		}
		else
		{	
			CanJoinChannel = true;
	
			// if there is a lockdown and the person can't join do not add it to the channel list and greet him - that's why we use bool CanJoinChannel
			if( m_Lockdown )
			{
				if( m_AccessRequired > Access && !Match( User, m_HostbotName ) )
				{
					SendChatCommand( "/ban " + User + " Lockdown: only users with " + UTIL_ToString( m_AccessRequired ) + "+ access can join" );
					if( !IsInLockdownNames( User ) )
						LockdownNames.push_back( string( User ) );
					CanJoinChannel = false;
				}
			}

			if( m_BanChat && UserFlags == 16 && !m_GHost->m_DB->SafelistCheck( m_Server, User ) )
				ImmediateChatCommand( "/kick " + User + " Chat clients are banned from channel" );
			
			
			// only greet in originally set channel, not in others, and only users able to join
			if( Match( m_CurrentChannel, m_FirstChannel ) && CanJoinChannel && m_GreetUsers && !Match( User, m_HostbotName ) ) 
			{ 
				
				string Line1 = m_GHost->m_Language->WelcomeMessageLine1( m_FirstChannel, User );
				string Line2 = m_GHost->m_Language->WelcomeMessageLine2( m_FirstChannel, User );

				if( Line1.size( ) > 2 )
					ImmediateChatCommand( Line1 );
				if( Line2.size( ) > 2 )
					QueueChatCommand( Line2 );					
				
			}

			if ( CanJoinChannel )
			{
				CChannel *user = GetUserByName( User );
				if( !user )
					user = new CChannel( User );
				user->SetPing( Ping );
				user->SetUserFlags( UserFlags );

				if( Message.size( ) >= 14 )
				{
					reverse( Message.begin( ), Message.end( ) );		
					user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );			
				}

				m_Channel[lowerUser] = user;
				
				if ( Access == 0 )				
					if( IsClanMember( User ) )
					{
						m_GHost->m_DB->AccessSet( m_Server, User, m_ClanDefaultAccess );
						CONSOLE_Print( "[ACCESS: " + m_ServerAlias + "] Setting [" + User + "] access to clan default of " + UTIL_ToString( m_ClanDefaultAccess ) ); 
					}													
			}			
		}		
	}

	else if( Event == CBNETProtocol :: EID_LEAVE )
	{
		CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + "] user [" + User + "] left the channel." );

		if ( m_AnnounceGames && !Match( m_HostbotName, User ) )
			SendChatCommandHidden( "/whereis " + User );

		for( map<string,  CChannel *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); i++ )		
			if( (*i).first == lowerUser )
				m_Channel.erase( i );			
	}

	else if( Event == CBNETProtocol :: EID_ERROR )
	{
		CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );	
	}

	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		if( !Match( Message, m_CurrentChannel ) )
			m_Rejoin = true;
		else
			m_Rejoin = false;

		m_Channel.clear( );
	}

	else if( Event == CBNETProtocol :: EID_SHOWUSER )
	{

		CChannel *user = GetUserByName( lowerUser );
		if( !user )
			user = new CChannel( User );
		user->SetPing( Ping );
		user->SetUserFlags( UserFlags );
		if( Message.size( ) >= 14 )
		{
			reverse( Message.begin( ), Message.end( ) ); 
			user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );
			
		}
		m_Channel[lowerUser] = user;
	}
	
}
