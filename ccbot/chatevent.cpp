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
#include "commandpacket.h"
#include "ccbotdb.h"
#include "bncsutilinterface.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <time.h>
#endif

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{

	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	string User = chatEvent->GetUser( );
	string lowerUser = User;
	transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );
	string Message = chatEvent->GetMessage( );
	uint32_t Access = m_CCBot->m_DB->AccessCheck( m_Server, User );

	if( Access == 11 )
		Access = 0;

	m_ClanCommandsEnabled = IsClanMember( m_UserName );	

	if( Event == CBNETProtocol :: EID_WHISPER )
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "][" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_TALK )
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + ":" + m_CurrentChannel + "][" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_EMOTE )
			CONSOLE_Print( "[EMOTE: " + m_ServerAlias + ":" + m_CurrentChannel + "][" + User + "] " + Message );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK || Event == CBNETProtocol :: EID_EMOTE )
	{

		// Anti-Spam

		if( m_AntiSpam && !Message.empty( ) && Message[0] != m_CommandTrigger && Access < 3 && IsInChannel( User ) && User != m_UserName )
		{			
			string message = Message;
			transform( message.begin( ), message.end( ), message.begin( ), (int(*)(int))tolower );
			uint32_t SpamCacheSize = m_Channel.size( ) * 3;

			if( m_SpamCache.size( ) > SpamCacheSize )
				m_SpamCache.erase( m_SpamCache.begin( ) );

			m_SpamCache.insert( pair<string, string>( lowerUser, message ) );

			int mesgmatches = 0;
			int nickmatches = 0;

			for( multimap<string, string> :: iterator i = m_SpamCache.begin( ); i != m_SpamCache.end( ); ++i )
			{
				if( (*i).first == lowerUser )
					++nickmatches;

				if( (*i).second.find( message ) )
					++mesgmatches;
			}

			if( mesgmatches > 2 || nickmatches > 3 )
				SendChatCommand( "/kick " + User + " Anti-Spam®" );
		}

		// Swearing kick

		if ( m_SwearingKick && !Message.empty( ) && !Match( User, m_HostbotName ) && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) && IsInChannel( User ) && Access < 5 ) 
		{
			string message = Message;
			transform( message.begin( ), message.end( ), message.begin( ), (int(*)(int))tolower );

			for( uint32_t i = 0; i < m_CCBot->m_SwearList.size( ); ++i )
			{
				if( message.find( m_CCBot->m_SwearList[i] ) != string :: npos )
					QueueChatCommand( m_CCBot->m_Language->SwearKick( User, m_CCBot->m_SwearList[i] ) );
			}
		}
		
		// ?TRIGGER
		
		if( Match( Message, "?trigger" ) && Event != CBNETProtocol :: EID_EMOTE)
		{
			QueueChatCommand( m_CCBot->m_Language->CommandTrigger( m_CommandTriggerStr ), User, Whisper );	
		}
		
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
				// !ACCEPT
				//

				if( Command == "accept" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "accept" ) && m_ActiveInvitation )
				{
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATIONRESPONSE( m_Protocol->GetClanTag( ), m_Protocol->GetInviter( ), true ) );
					QueueChatCommand( m_CCBot->m_Language->InvitationAccepted( ), User, Whisper );
					SendGetClanList( );
					m_ActiveInvitation = false;
				}

				//
				// !ACCESS
				//

				else if( Command == "access" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "access" ) )
				{
					SendChatCommand( "/w " + User + " " + m_CCBot->m_Language->HasFollowingAccess( UTIL_ToString( Access ) ) );
					string Commands;

					for( uint32_t i = 0; i <= Access; ++i )
					{
						vector<string> m_CommandList = m_CCBot->m_DB->CommandList( i );

						for( vector<string> :: iterator it = m_CommandList.begin( ); it != m_CommandList.end( ); ++it )
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
				

				else if( ( Command == "addsafelist" || Command == "adds" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "addsafelist" ) )
				{					
					if( m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
						QueueChatCommand( m_CCBot->m_Language->UserAlreadySafelisted( Payload ), User, Whisper );
					else
					{
						if( m_CCBot->m_DB->SafelistAdd( m_Server, Payload ) )
							QueueChatCommand( m_CCBot->m_Language->UserSafelisted( Payload ), User, Whisper );
						else
							QueueChatCommand( m_CCBot->m_Language->ErrorSafelisting( Payload ), User, Whisper );
					}					
				}
				
				//
				// !ANNOUNCE
				//

				else if( Command == "announce" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "announce" ) )
				{					
					string Interval, AnnounceMessage;
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

					if( ( UTIL_ToInt32( Interval ) > 0 ) && ( AnnounceMessage.length( ) > 1 ) )
					{
						if( UTIL_ToInt32( Interval ) < 3 )
							Interval = "4";

						m_Announce = true;
						m_AnnounceMsg = AnnounceMessage;	
						m_AnnounceInterval = UTIL_ToInt32( Interval );
						QueueChatCommand( m_CCBot->m_Language->AnnounceEnabled( Interval ), User, Whisper );				
					}
					else if( Interval == "off" && m_Announce == true )
					{
						m_Announce = false;
						QueueChatCommand( m_CCBot->m_Language->AnnounceDisabled( ), User, Whisper );
					}
				}

				//
				// !BAN
				// !ADDBAN
				//

				else if( ( Command == "ban" || Command == "addban" )&& !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "ban" ) )
				{
					
					string Victim, Reason;
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
	
						CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, Victim );
	
						if( Ban )
						{
							QueueChatCommand( m_CCBot->m_Language->UserAlreadyBanned( Victim, Ban->GetAdmin( ) ), User, Whisper );
							delete Ban;
							Ban = NULL;
						}
						else
						{
							if( m_CCBot->m_DB->BanAdd( m_Server, Victim, User, Reason ) )
								QueueChatCommand( m_CCBot->m_Language->SuccesfullyBanned( Victim, User ), User, Whisper );
							else
								QueueChatCommand( m_CCBot->m_Language->ErrorBanningUser( Victim ), User, Whisper );
						}
					}
				}
				
				//
				// !CHECK
				//

				else if( Command == "check" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "checkban" ) )
				{
					CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, Payload );
					Access = m_CCBot->m_DB->AccessCheck( m_Server, Payload );
					if( Access == 11 )
						Access = 0;

					if( IsClanMember( Payload ) )
					{
						if( IsClanPeon( Payload ) )
						{							
							if( Ban )							
								QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( Access ) + "] access and banned from channel.", User, Whisper );		
							else
								QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );
						}
						else if( IsClanGrunt( Payload ) )
						{
							if( Ban )
								QueueChatCommand( Payload + " is in clan, rank Grunt, with [" + UTIL_ToString( Access ) + "] access and is banned from channel.", User, Whisper );		
							else
								QueueChatCommand( Payload + " is in clan, rank Grunt with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );					
						}
						else if( IsClanShaman( Payload ) )
								QueueChatCommand( Payload + " is in clan, rank Shaman, with [" + UTIL_ToString( Access ) + "] access.", User, Whisper );

						else if( IsClanChieftain( Payload ) )
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

				else if( Command == "setaccess" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "setaccess" ) )
				{
					bool CanSetAccess = true;

					// extract the username and the access which we are setting
					// ie. !setaccess 8 Panda -> username "Panda" , access "8" 

					uint32_t NewAccess;
					string UserName;
					stringstream SS;
					SS << Payload;
					SS >> NewAccess;

					if( SS.fail( ) || SS.eof( ) )
					{						
						QueueChatCommand( "Bad input - use form " + m_CommandTriggerStr + "setaccess <access 0-10> <name>", User, Whisper );
					}
					else
					{
						getline( SS, UserName );
						string :: size_type Start = UserName.find_first_not_of( " " );

						if( Start != string :: npos )
							UserName = UserName.substr( Start );

						uint32_t OldAccess = m_CCBot->m_DB->AccessCheck( m_Server, UserName );

						if( NewAccess > 10 )
							NewAccess = 10;
						if( OldAccess == 11 )
							OldAccess = 0;							

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
							if( m_CCBot->m_DB->AccessSet( m_Server, UserName, NewAccess ) )
								QueueChatCommand( "Added user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
							else
								QueueChatCommand( "Error adding user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
						}
					}					
				}
				
				//
				// !CHECKACCESS
				//

				else if( Command == "checkaccess" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "checkaccess" ) )
				{
					if( m_CCBot->m_DB->AccessCheck( m_Server, Payload ) != 11 )
						QueueChatCommand( "User [" + Payload + "] has access of [" + UTIL_ToString( m_CCBot->m_DB->AccessCheck( m_Server, Payload ) ) + "]", User, Whisper );
					else
						QueueChatCommand( "User [" + Payload + "] has access of [" + UTIL_ToString( 0 ) + "]", User, Whisper );
				}

				//
				// !COUNTACCESS
				//

				else if( Command == "countaccess" && Access >= m_CCBot->m_DB->CommandAccess( "countaccess" ) )
				{
					if( Payload.size( ) == 0 )
						Payload = "0";
					else if( UTIL_ToInt32( Payload ) > 10 )
						Payload = "10";

					uint32_t Count = m_CCBot->m_DB->AccessCount( m_Server, UTIL_ToInt32( Payload ) );

			
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
				
				else if( Command == "delaccess" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "delaccess" ) )
				{
					if( !m_CCBot->m_DB->AccessCheck( m_Server, Payload ) )
						QueueChatCommand( "User has no set access.", User, Whisper );
					else
					{
						if( m_CCBot->m_DB->AccessRemove( Payload ) )
							QueueChatCommand( "Deleted user [" + Payload + "]'s set access." , User, Whisper );
						else
							QueueChatCommand( "Error deleting user [" + Payload + "]'s access.", User, Whisper );
					}					
				}

				//
				// !CHECKSAFELIST
				// !CHECKS
				//

				if( ( Command == "checksafelist" || Command == "checks" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "checksafelist" ) )
				{
					if( m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
						QueueChatCommand( m_CCBot->m_Language->UserIsSafelisted( Payload ), User, Whisper );
					else
						QueueChatCommand( m_CCBot->m_Language->UserNotSafelisted( Payload ), User, Whisper );
				}
				
				//
				// !COMMAND
				//	

				else if( Command == "command" && !Payload.empty( ) && ( Access >= m_CCBot->m_DB->CommandAccess( "command" ) ) )
				{
					transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );

					bool Exists = false;

					for( map<string, uint32_t> :: iterator i = m_CCBot->m_Commands.begin( ); i != m_CCBot->m_Commands.end( ); ++i )
					{
						if( (*i).first == Payload )
							Exists = true;						
					}
					if( Exists )
						QueueChatCommand( "Access needed for the [" + Payload + "] command is [" + UTIL_ToString( m_CCBot->m_DB->CommandAccess( Payload ) ) + "]", User, Whisper );
					else
						QueueChatCommand( "Error checking access for [" + Payload + "] command, doesn't exist.", User, Whisper );
				}

				//
				// !SETCOMMAND
				//	

				else if( Command == "setcommand" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "setcommand" ) )
				{
					bool Exists = false;

					uint32_t NewAccess;
					string command;
					stringstream SS;
					SS << Payload;
					SS >> NewAccess;

					if( SS.fail( ) || SS.eof( ) )
					{				
						QueueChatCommand( "Bad input - use form " + m_CommandTriggerStr + "setcommand <access 0-10> <command>", User, Whisper );
					}
					else
					{
						getline( SS, command );
						string :: size_type Start = command.find_first_not_of( " " );

						if( Start != string :: npos )
							command = command.substr( Start );

						if( NewAccess > 10 )
							NewAccess = 10;
					
						for( map<string, uint32_t> :: iterator i = m_CCBot->m_Commands.begin( ); i != m_CCBot->m_Commands.end( ); ++i )
							if( (*i).first == command )
								Exists = true;
										
						if( Exists )
						{
							if( m_CCBot->m_DB->CommandSetAccess( command, NewAccess ) )
								QueueChatCommand( "Updated [" + command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
							else
								QueueChatCommand( "Error updating access for [" + command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper );
						}
						else
							QueueChatCommand( "Unable to set access for [" + command + "] because it doesn't exist.", User, Whisper );
					}
				}

				//
				// !CLEARQUEUE
				// !CQ
				//

				else if( ( Command == "clearqueue" ||  Command == "cq" ) && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "clearqueue" ) )
				{
					while( !m_ChatCommands.empty( ) )
						m_ChatCommands.pop( );

					QueueChatCommand( m_CCBot->m_Language->MessageQueueCleared( ), User, Whisper );
				}

				//
				// !COUNTBANS
				//

				else if( Command == "countbans" && Access >= m_CCBot->m_DB->CommandAccess( "countbans" ) )
				{
					uint32_t Count = m_CCBot->m_DB->BanCount( m_Server );

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

				else if( ( Command == "countsafelist" || Command == "counts" ) && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "countsafelist" ) )
				{
					uint32_t Count = m_CCBot->m_DB->SafelistCount( m_Server );

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

				else if( Command == "channel" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "channel" ) )
				{
					QueueChatCommand( "/join " + Payload );
					m_CurrentChannel = Payload;
				}

				//
				// !REJOIN
				//

				else if( Command == "rejoin" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "channel" ) )
				{
					QueueChatCommand( "/rejoin" );
				}

				//
				// !CHANLIST
				//
								
				else if( Command == "chanlist" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "chanlist" ) )
				{
					string tempText;
					
					for( map<string, CChannel *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); ++i )					
						tempText = tempText + (*i).second->GetUser( ) + ", ";					

					if( tempText.length( ) >= 2 )
						tempText = tempText.substr( 0, tempText.length( ) - 2 );

					QueueChatCommand( "Users in channel [" + UTIL_ToString( m_Channel.size( ) ) + "]: " + tempText + ".", User, Whisper );			
				}

				//
				// !CLANLIST
				//

				else if( Command == "clanlist" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "clanlist" ) && m_ClanCommandsEnabled )
				{
					string ClanList1, ClanList2, Chieftains, Shamans, Grunts, Peons;

					for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); ++i )
					{
						if( (*i)->GetRank( ) == "Recruit" )
								Peons = Peons + (*i)->GetName( ) + ", ";
						else if( (*i)->GetRank( ) == "Peon" )
								Peons = Peons + (*i)->GetName( ) + ", ";
						else if( (*i)->GetRank( ) == "Grunt" )
								Grunts = Grunts + (*i)->GetName( ) + ", ";
						else if( (*i)->GetRank( ) == "Shaman" )
								Shamans = Shamans + (*i)->GetName( ) + ", ";
						else if( (*i)->GetRank( ) == "Chieftain" )
								Chieftains = Chieftains + (*i)->GetName( ) + ", ";
					}

					if( Chieftains.size( ) > 179 )
					{
						ClanList1 = Chieftains.substr( 0, 179 );
						ClanList2 = Chieftains.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Chieftains: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );				
					}
					else if( Chieftains.size() >= 2 )
						SendChatCommand( "/w " + User + " Chieftains: " + Chieftains.substr( 0, Chieftains.size( )-2 ) );

					if( Shamans.size( ) > 179 )
					{
						ClanList1 = Shamans.substr( 0, 179 );
						ClanList2 = Shamans.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Shamans: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );				
					}
					else if( Shamans.size() >= 2 )
						SendChatCommand( "/w " + User + " Shamans: " + Shamans.substr( 0, Shamans.size( )-2) );

					if( Grunts.size( ) > 179 )
					{
						ClanList1 = Grunts.substr( 0, 179 );
						ClanList2 = Grunts.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Grunts: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );				
					}
					else if( Grunts.size() >= 2 )
						SendChatCommand( "/w " + User + " Grunts: " + Grunts.substr( 0, Grunts.size( )-2) );

					if( Peons.size( ) > 179 )
					{
						ClanList1 = Peons.substr( 0, 179 );
						ClanList2 = Peons.substr( 179, 180 );
						SendChatCommand( "/w " + User + " Peons: " + ClanList1 );
						SendChatCommand( "/w " + User + " " + ClanList2 );
					}
					else if( Peons.size() >= 2 )
						SendChatCommand( "/w " + User + " Peons: " + Peons.substr( 0, Peons.size( )-2) );
				}

				//
				// !DELBAN
				// !UNBAN
				//

				else if( ( Command == "delban" || Command == "unban" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "unban" ) )
				{
					string Victim;
					string Reason;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if( m_CCBot->m_DB->BanRemove( m_Server, Victim ) )
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

				else if( ( Command == "delsafelist" || Command == "dels" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "delsafelist" ) )
				{					

					if( !m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
						QueueChatCommand( "User " + Payload + " is not safelisted.", User, Whisper );
					else
					{
						if( m_CCBot->m_DB->SafelistRemove( m_Server, Payload ) )
							QueueChatCommand( "User " + Payload + " is deleted from safelist.", User, Whisper );
						else
							QueueChatCommand( "Error deleting user " + Payload + " from the safelist.", User, Whisper );
					}
				}

				//
				// !EXIT
				// !QUIT
				//

				else if( ( Command == "exit" || Command == "quit" ) && Access >= m_CCBot->m_DB->CommandAccess( "exit" ) )
				{
					m_Exiting = true;
				}

				//
				// !GAMES
				//

				else if( Command == "games"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "games" ) )
				{
					if( Match( Payload, "on" ) )
					{
						QueueChatCommand( m_CCBot->m_Language->GameAnnouncerEnabled( ), User, Whisper );
						m_AnnounceGames = true;
					}
					else if( Match( Payload, "off" ) )
					{
						QueueChatCommand( m_CCBot->m_Language->GameAnnouncerDisabled( ), User, Whisper );
						m_AnnounceGames = false;
					}
				}				

				//
				// !GETCLAN
				//

				else if( Command == "getclan" && Access >= m_CCBot->m_DB->CommandAccess( "getclan" ) && m_ClanCommandsEnabled )
				{
					SendGetClanList( );
					QueueChatCommand( m_CCBot->m_Language->UpdatedClanList( ), User, Whisper );
					QueueWhisperCommand( m_CCBot->m_Language->ReceivedClanMembers( UTIL_ToString( m_Clans.size( ) ) ), User );
				}

				//
				// !CHIEFTAIN
				//

				else if( Command == "chieftain" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "chieftain" ) && IsClanChieftain( m_UserName ) && m_ClanCommandsEnabled )
				{
					if( IsClanMember( Payload ) )
					{
						m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMAKECHIEFTAIN( Payload ) );
						m_LastKnown = Payload;
					}
					else
						QueueChatCommand( m_CCBot->m_Language->MustBeAClanMember( Payload ), User, Whisper );
				}	

				//
				// !GREET
				//

				else if( Command == "greet"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "greet" ) )
				{
					
					if( Match( Payload, "off" ) )
					{
						QueueChatCommand( m_CCBot->m_Language->GreetingDisabled( ), User, Whisper );
						m_GreetUsers = false;
					}
					else if( Match( Payload, "on" ) )
					{
						QueueChatCommand( m_CCBot->m_Language->GreetingEnabled( ), User, Whisper );
						m_GreetUsers = true;
					}
				}	 				

				//
				// !GRUNT
				//

				else if( Command == "grunt"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "grunt" ) && m_ClanCommandsEnabled )
				{
					if(  IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_MEMBER );
						SendGetClanList( );
						QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Grunt" ), User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper );
				}			

				//
				// !KICK
				//

				else if( Command == "kick" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "kick" ) )
				{
					string Victim;
					string Reason;
					stringstream SS;
					SS << Payload;
					SS >> Victim;

					if ( GetUserFromNamePartial( Victim ).size( ) >= 1 )
							Victim = GetUserFromNamePartial( Victim );

					if( !IsClanShaman( Victim ) && !IsClanChieftain( Victim ) && !m_CCBot->m_DB->SafelistCheck( m_Server, Victim ) && GetUserFromNamePartial( Victim ).size( ) >= 1  )
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
				
				else if( Command == "lockdown" && !Match( "off", Payload) && Access >= m_CCBot->m_DB->CommandAccess( "lockdown" ) )
				{
					if( Payload.empty( ) )
						m_AccessRequired = Access;
					else
					{	
						stringstream SS;
						SS << Payload;
						SS >> m_AccessRequired;
					}
					m_Lockdown = true;
					QueueChatCommand( m_CCBot->m_Language->LockdownEnabled( UTIL_ToString( m_AccessRequired ) ), User, Whisper );	
				}
				
				//
				// !LOCKDOWN OFF
				//
				
				else if( Command == "lockdown" && Match( "off", Payload ) && m_Lockdown && Access >= m_CCBot->m_DB->CommandAccess( "lockdown" ) )
				{
					m_Lockdown = false;
					QueueChatCommand( m_CCBot->m_Language->LockdownDisabled( ), User, Whisper );

					for( vector<string> :: iterator i = LockdownNames.begin( ); i != LockdownNames.end( ); ++i )
						ImmediateChatCommand( "/unban " + *i );

					LockdownNames.clear( );
				}

				//
				// !MOTD (clan message of the day)
				//

				else if( Command == "motd" && Access >= m_CCBot->m_DB->CommandAccess( "motd" ) && m_ClanCommandsEnabled )
				{					
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANSETMOTD( Payload ) );
					QueueChatCommand( m_CCBot->m_Language->SetMOTD( Payload ), User, Whisper );
				}
				
				//
				// !PEON
				//

				else if( Command == "peon"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "peon" ) && m_ClanCommandsEnabled )
				{
					if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_PARTIAL_MEMBER );
						QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Peon" ), User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper );
				}				
				
				//
				// !REMOVE (from clan)
				//

				else if( Command == "remove"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "remove" ) && m_ClanCommandsEnabled )
				{
					if( !IsClanChieftain( Payload ) && !IsClanShaman( Payload ) )
					{
						m_Socket->PutBytes( m_Protocol->SEND_SID_CLANREMOVEMEMBER( Payload ) );
						m_Removed = Payload;
						m_UsedRemove = User;
					}
					else
						QueueChatCommand( "Removing Chieftains and Shamans is prohibited.", User, Whisper );
				}

				//
				// !RELOAD
				//

				else if( Command == "reload" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "reload" ) )
				{
					SendGetClanList( );
					m_CCBot->UpdateSwearList( );					
					m_CCBot->ReloadConfigs( );
					QueueChatCommand( m_CCBot->m_Language->CFGReloaded( ), User, Whisper );
				}

				//
				// !SAY
				//

				else if( Command == "say" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "say" ) )
				{
					if( Access > 8 )
						QueueChatCommand( Payload );
					else if( Payload[0] != '/' )
						QueueChatCommand( Payload );
					else
						QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper );					
				}

				//
				// !SAYBNET
				//

				else if( Command == "saybnet" && !Payload.empty( ) && Access >= m_CCBot->m_DB-> CommandAccess( "say" ) )
				{
					if( Payload.find( " " ) != string :: npos )
					{
						string :: size_type CommandStart = Payload.find( " " );
						string server = Payload.substr( 0, CommandStart );
						string command = Payload.substr( CommandStart + 1 );

						if( m_CCBot->GetServerFromNamePartial( server ).size( ) > 0 )
							server = m_CCBot->GetServerFromNamePartial( server );
						else
							QueueChatCommand( m_CCBot->m_Language->UnableToPartiallyMatchServer( ) );
	
						for( vector<CBNET *> :: iterator i = m_CCBot->m_BNETs.begin( ); i != m_CCBot->m_BNETs.end( ); ++i )
						{
							if( Access > 8 && Match( (*i)->GetServer( ), server ) )
							{
								(*i)->QueueChatCommand( command );
							}
							else if( command[0] != '/' && Match( (*i)->GetServer( ), server ) )
							{
								(*i)->QueueChatCommand( command );
							}
							else if( (*i)->GetServer( ) == m_Server && Payload[0] == '/' )
							{
								QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper );
							}
						}
					}
				}

				//
				// !SAYBNETS
				//

				else if( Command == "saybnets" && !Payload.empty( ) && Access >= m_CCBot->m_DB-> CommandAccess( "say" ) )
				{
					for( vector<CBNET *> :: iterator i = m_CCBot->m_BNETs.begin( ); i != m_CCBot->m_BNETs.end( ); ++i )
					{
						if( Access > 8 )
						{
							(*i)->QueueChatCommand( Payload );
						}
						else if( Payload[0] != '/' )
						{
							(*i)->QueueChatCommand( Payload );
						}
						else if( (*i)->GetServer( ) == m_Server )
						{
							QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper );
						}
					}
				}

				//
				// !SHAMAN
				//

				else if( Command == "shaman"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "shaman" ) && m_ClanCommandsEnabled )
				{
					if( IsClanChieftain( m_UserName ) )
					{
						SendClanChangeRank( Payload, CBNETProtocol :: CLAN_OFFICER );
						QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Shaman" ), User, Whisper );
					}
					else
						QueueChatCommand( "Bot's account needs to be Chieftain to perform this action.", User, Whisper );
				}

				//
				// !SQUELCH
				// !IGNORE
				//

				else if( ( Command == "squelch" || Command == "ignore" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
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

				else if( ( Command == "squelchlist" || Command == "sl" ) && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
				{					
					string tempText;

					for( vector<string> :: iterator i = SquelchedUsers.begin( ); i != SquelchedUsers.end( ); ++i )
					{
						if( (*i).size( ) > 0 )
							tempText = tempText + (*i) + ", ";					
					}

					if ( tempText.size( ) == 0 || SquelchedUsers.size( ) == 0 )
						QueueChatCommand( "There are no squelched users." , User, Whisper );
					else if( tempText.size( ) >= 2 )
						QueueChatCommand( "Squelched users: " + tempText.substr( 0, tempText.size( )-2 ) + ".", User, Whisper );
				}

				//
				// !TOPIC
				//

				else if( Command == "topic" && Access >= m_CCBot->m_DB->CommandAccess( "topic" ) )
				{					
					QueueChatCommand( "/topic " + m_CurrentChannel + " \"" + Payload + "\"" );
					QueueChatCommand( m_CCBot->m_Language->SetTopic( Payload ), User, Whisper );
				}
				
				//
				// !UNSQUELCH
				// !UNIGNORE
				//

				else if( ( Command == "unsquelch" || Command == "unignore" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
				{
					if( IsAlreadySquelched( Payload ) )
					{
						QueueChatCommand( "/unignore " + Payload );						

						for( vector<string> :: iterator i = SquelchedUsers.begin( ); i != SquelchedUsers.end( ); ++i )
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
				
				else if( Command == "uptime" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "uptime" ) )
				{
					string date;
					uint32_t time = GetTime( ) - m_CCBot->m_Uptime;

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

					QueueChatCommand( m_CCBot->m_Language->Uptime( m_UserName, date ), User, Whisper );
				}
					
				//
				// !CHECKBAN
				//

				else if( Command == "checkban" && Access >= m_CCBot->m_DB->CommandAccess( "checkban" ) )
				{
					if( Payload.empty( ) )
					User = Payload;
					CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, Payload );
					
					if( Ban )
					{	
						if( Ban->GetReason( ).empty( ) )
							QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " by " + Ban->GetAdmin( ) + ".", User, Whisper );
						else
							QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " because \"" + Ban->GetReason( ) + "\" by " + Ban->GetAdmin( ) + ".", User, Whisper );
						delete Ban;
						Ban = NULL;
					}
					else
						QueueChatCommand( "User [" + Payload + "] is not banned.", User, Whisper );
				}

				//
				// !GN8
				//
				
				else if( Command == "gn8" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "gn8" ) )
					QueueChatCommand( m_CCBot->m_Language->GN8( User ) ); 

				//
				// !INVITE
				//

				else if( Command == "invite" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "invite" )  && m_ClanCommandsEnabled && IsInChannel( Payload ) )
				{

					if( !IsClanMember( Payload ) )
					{
						if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
						{
							m_LastKnown = GetUserFromNamePartial( Payload );
						
							m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( Payload ) );							
							QueueChatCommand( "Clan invitation was sent.", User, Whisper );
						}
						else
					   		QueueChatCommand( "Bot needs to have at least a Shaman rank to invite.", User, Whisper );
					}
					else
						QueueChatCommand( "You can't invite people already in clan!", User, Whisper );                                      	
				}						
				
				//
				// !JOINCLAN 
				// !JOIN
				//
				
				else if( ( Command == "joinclan" || Command == "join" ) && Payload.empty( ) && !IsClanMember( User ) && IsInChannel( User ) && Access >= 0  && m_ClanCommandsEnabled )
				{
					if( m_SelfJoin )
					{
						if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
						{
							m_LastKnown = User;

							if( m_LoggedIn )
								m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( User ) );

							SendGetClanList( );
						}
					}
					else
						QueueChatCommand( m_CCBot->m_Language->CommandDisabled( ), User, Whisper ); 	
				}

				//
				// !ONLINE 
				// !O
				//
				
				else if( ( Command == "online" || Command == "o" ) && Access >= m_CCBot->m_DB->CommandAccess( "online" ) && m_ClanCommandsEnabled )
				{
					string Online;
					uint32_t OnlineNumber = 0;

					for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); ++i )
					{
						if( (*i)->GetStatus( ) == "Online" && !Match( (*i)->GetName( ), m_UserName ) && !Match( (*i)->GetName( ), m_HostbotName ) )
						{
							Online = Online + (*i)->GetName( ) + ", ";
							++OnlineNumber;
						}
					}

					if( OnlineNumber == 0 )
						SendChatCommand( "/w " + User + " " + "There are no " + m_ClanTag + " members online." );
					else if( Online.size( ) < 3 )
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

				else if ( Command == "slap" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "slap" ) )
				{

					if ( GetUserFromNamePartial( Payload ).size( ) >= 1 )
						Payload = GetUserFromNamePartial( Payload );

					if( Payload == User )					
						Payload = "himself";

					srand( ( unsigned )time( 0 ) );					
					int Random = ( rand( ) % 6 );					

					switch( Random )
					{
						case 0: QueueChatCommand( User + " slaps " + Payload + " around a bit with a large trout." );
							break;

						case 1: QueueChatCommand( User + " slaps " + Payload + " around a bit with a pink Macintosh." );
							break;

						case 2: QueueChatCommand( User + " throws a Playstation 3 at " + Payload + "." );
							break;

						case 3: QueueChatCommand( User + " drives a car over " + Payload + "." );
							break;

						case 4: QueueChatCommand( User + " finds " + Payload + " on uglypeople.com." );
							break;

						case 5: Random = rand( );
							string s;
							stringstream out;
							out << Random;
							s = out.str();
							QueueChatCommand( User + " searches google.com for \"goatsex + " + Payload + "\". " + s + " hits WEEE!" );	
							break; 
					}
				}
				
				//
				// !SPIT
				//

				else if ( Command == "spit" && !Payload.empty( ) && IsInChannel( User ) && Access >= m_CCBot->m_DB->CommandAccess( "spit" ) )
				{
					if ( GetUserFromNamePartial( Payload ).size( ) >= 1 )
						Payload = GetUserFromNamePartial( Payload );

					srand( ( unsigned )time( 0 ) );  
					int Random = ( rand( ) % 6 );

					switch( Random )
					{
						case 0: QueueChatCommand( User + " spits " + Payload + " but misses completely!" );
							break;

						case 1: QueueChatCommand( User + " spits " + Payload + " and hits him in the eye!" );
							break;

						case 2: QueueChatCommand( User + " spits " + Payload + " and hits him in his mouth!" );
							break;

						case 3: QueueChatCommand( User + " spits " + Payload + " and nails it into his ear!" );
							break;

						case 4: QueueChatCommand( User + " spits " + Payload + " but he accidentaly hits himself!" );
							break;

						case 5: QueueChatCommand( Payload + " counterspits " + User + ". Lame." );
							break;
					}
				}

				//
				// !SERVE
				//

				else if ( Command == "serve" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "serve" ) )
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
				// !STATUS
				//

				else if ( Command == "status" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "status" ) )
				{
					string message = "Status: ";

			       		for( vector<CBNET *> :: iterator i = m_CCBot->m_BNETs.begin( ); i != m_CCBot->m_BNETs.end( ); ++i )
					{
						if( (*i)->GetLoggedIn( ) == true )
							message += (*i)->GetUserName( ) + " [Online], ";
						else
							message += (*i)->GetUserName( ) + " [Offline], ";
					}

					QueueChatCommand( message.substr( 0, message.size( ) - 2 ) + ".", User, Whisper );						 
				}				

				//
				// !VERSION
				//

				else if( Command == "version" && Payload.empty( ) )					
						QueueChatCommand( m_CCBot->m_Language->Version( m_CCBot->m_Version ), User, Whisper );

				//
				// !PING
				//

				else if( Command == "ping" && Access >= m_CCBot->m_DB->CommandAccess( "ping" ) )
				{
					if( Payload.empty( ) )
						Payload = User;			
					else if( GetUserFromNamePartial( Payload ).size( ) >= 1 )
						Payload = GetUserFromNamePartial( Payload );

					CChannel *Result = GetUserByName( Payload );

					if( Result )
						QueueChatCommand( m_CCBot->m_Language->Ping( Payload, UTIL_ToString( Result->GetPing( ) ) , m_ServerAlias), User, Whisper );
					else
						QueueChatCommand( m_CCBot->m_Language->CannotAccessPing( ), User, Whisper );
				}
				
				//
				// !RESTART
				//

				else if( Command == "restart" && ( Access >= m_CCBot->m_DB->CommandAccess( "restart" ) || IsRootAdmin( User ) ) )
				{
					m_CCBot->Restart( );	
				}				
		}		
	}

	else if( Event == CBNETProtocol :: EID_INFO )
	{
		CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );

		if( m_AnnounceGames )		
		{
			if( Message.find("private game") != string::npos || Message.find("in  game") != string::npos )
			{
				string user = Message.substr( 0, Message.find_first_of(" ") );

				if( !Match( user, m_HostbotName ) )
				{			
					Message = Message.substr( Message.find_first_of("\"") + 1, Message.find_last_of("\"") - Message.find_first_of("\"") - 1 );				
					QueueChatCommand( m_CCBot->m_Language->AnnounceGame( user, Message ) );
				}
			}			
		}							
	}

	else if( Event == CBNETProtocol :: EID_JOIN )
	{
		bool CanJoinChannel = false;		

		// check if the user joined has been banned if yes ban him again and send him the reason the first time he tries to join
		CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, User );

		if( Ban )
		{
			string Info = "by " + Ban->GetAdmin( );
			if( !Ban->GetReason( ).empty( ) )
				Info += ": " + Ban->GetReason( );
			Info += " on " + Ban->GetDate( );
			SendChatCommand( "/ban " + User + " " + Info );

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
					CanJoinChannel = false;
					SendChatCommand( "/ban " + User + " Lockdown: only users with " + UTIL_ToString( m_AccessRequired ) + "+ access can join" );
					if( !IsInLockdownNames( User ) )
						LockdownNames.push_back( User );					
				}
			}							
			
			// only greet in originally set channel, not in others, and only users that are able to join
			if( Match( m_CurrentChannel, m_FirstChannel ) && CanJoinChannel && m_GreetUsers && !Match( User, m_HostbotName ) ) 
			{				
				string Line1 = m_CCBot->m_Language->WelcomeMessageLine1( m_FirstChannel, User );
				string Line2 = m_CCBot->m_Language->WelcomeMessageLine2( m_FirstChannel, User );

				if( Line1.size( ) > 1 )
					ImmediateChatCommand( Line1 );
				if( Line2.size( ) > 1 )
					QueueChatCommand( Line2 );				
			}

			if ( CanJoinChannel )
			{
				CChannel *user = GetUserByName( User );
				if( !user )
					user = new CChannel( User );
				user->SetPing( chatEvent->GetPing( ) );
				user->SetUserFlags( chatEvent->GetUserFlags( ) );

				if( Message.size( ) >= 14 )
				{
					reverse( Message.begin( ), Message.end( ) );		
					user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );			
				}
				else if( m_BanChat && Message == "TAHC" && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) )
				{
					ImmediateChatCommand( "/kick " + User + " Chat clients are forbidden to enter the channel" );
				}

				m_Channel[lowerUser] = user;
				
				if ( m_CCBot->m_DB->AccessCheck( m_Server, User ) == 11 && IsClanMember( User ) )
				{					
					m_CCBot->m_DB->AccessSet( m_Server, User, m_ClanDefaultAccess );
					CONSOLE_Print( "[ACCESS: " + m_ServerAlias + "] Setting [" + User + "] access to clan default of " + UTIL_ToString( m_ClanDefaultAccess ) ); 
				}

				CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + ":" + m_CurrentChannel + "] user [" + User + "] joined the channel." );													
			}			
		}		
	}

	else if( Event == CBNETProtocol :: EID_LEAVE )
	{
		CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + ":" + m_CurrentChannel + "] user [" + User + "] left the channel." );

		if ( m_AnnounceGames && !Match( m_HostbotName, User ) )
			SendChatCommandHidden( "/whereis " + User );

		for( map<string,  CChannel *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); ++i )		
			if( (*i).first == lowerUser )
				m_Channel.erase( i );			
	}

	else if( Event == CBNETProtocol :: EID_ERROR )
	{
		CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );
		
		if( Message == "You are banned from that channel." )
			m_RejoinInterval = 300;
	}

	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		if( !Match( Message, m_CurrentChannel ) )
			m_Rejoin = true;
		else
		{
			m_Rejoin = false;
			m_RejoinInterval = 15;
		}

		m_Channel.clear( );
	}

	else if( Event == CBNETProtocol :: EID_SHOWUSER )
	{
		CChannel *user = GetUserByName( lowerUser );
		if( !user )
			user = new CChannel( User );
		user->SetPing( chatEvent->GetPing( ) );
		user->SetUserFlags( chatEvent->GetUserFlags( ) );

		if( Message.size( ) >= 14 )
		{
			reverse( Message.begin( ), Message.end( ) ); 
			user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );			
		}
		else if( m_BanChat && Message == "TAHC" && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) )
		{
			ImmediateChatCommand( "/kick " + User + " Chat clients are banned from channel" );
		}

		m_Channel[lowerUser] = user;
	}
}
