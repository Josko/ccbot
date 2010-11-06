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
#include "socket.h"
#include "util.h"
#include "language.h"
#include "commandpacket.h"
#include "ccbotdb.h"
#include "bnetprotocol.h"
#include "bnet.h"

#include <cstdio>
#include <cstdlib>

#ifdef WIN32
	#include <time.h>
#endif

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{
	unsigned char Output = BNET;

	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );	
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	string User = chatEvent->GetUser( );
	string lowerUser = User;
	transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );
	string Message = chatEvent->GetMessage( );
	
	unsigned char Access = m_CCBot->m_DB->AccessCheck( m_Server, User );

	if( Access == 255 )
		Access = 0;

	m_ClanCommandsEnabled = IsClanMember( m_UserName );	

	if( Event == CBNETProtocol :: EID_WHISPER )
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "][" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_TALK )
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + ":" + m_CurrentChannel + "][" + User + "] " + Message );
	else if( Event == CBNETProtocol :: EID_EMOTE )
			CONSOLE_Print( "[EMOTE: " + m_ServerAlias + ":" + m_CurrentChannel + "][" + User + "] " + Message );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK || Event == CBNETProtocol :: EID_EMOTE || Event == CBNETProtocol :: CONSOLE_INPUT )
	{

		if( Event == CBNETProtocol :: CONSOLE_INPUT )
			Output = CONSOLE;

		// Anti-Spam
		// TODO: improve this to be more robust and efficient

		if( m_AntiSpam && !Message.empty( ) && Message[0] != m_CommandTrigger[0] && Access < 5 && IsInChannel( User ) && User != m_UserName )
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
				SendChatCommand( "/kick " + User + " Anti-Spam®", Output );
		}

		// Swearing kick

		if ( m_SwearingKick && !Message.empty( ) && !Match( User, m_HostbotName ) && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) && IsInChannel( User ) && Access < 5 ) 
		{
			string message = Message;
			transform( message.begin( ), message.end( ), message.begin( ), (int(*)(int))tolower );

			for( uint32_t i = 0; i < m_CCBot->m_SwearList.size( ); ++i )
			{
				if( message.find( m_CCBot->m_SwearList[i] ) != string :: npos )
					QueueChatCommand( m_CCBot->m_Language->SwearKick( User, m_CCBot->m_SwearList[i] ), Output );
			}
		}
		
		// ?TRIGGER
		
		if( Match( Message, "?trigger" ) )
		{
			QueueChatCommand( m_CCBot->m_Language->CommandTrigger( m_CommandTrigger ), User, Whisper, Output );	
		}		
		else if( !Message.empty( ) && Message[0] == m_CommandTrigger[0] && Event != CBNETProtocol :: EID_EMOTE )
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
			

			/********************************
			************ COMMANDS ***********
			********************************/

			//
			// !ACCEPT
			//

			if( Command == "accept" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "accept" ) && m_ActiveInvitation )
			{
				m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATIONRESPONSE( m_Protocol->GetClanTag( ), m_Protocol->GetInviter( ), true ) );
				QueueChatCommand( m_CCBot->m_Language->InvitationAccepted( ), User, Whisper, Output );
				SendGetClanList( );
				m_ActiveInvitation = false;
			}

			//
			// !ACCESS
			//

			else if( Command == "access" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "access" ) )
			{
				SendChatCommand( "/w " + User + " " + m_CCBot->m_Language->HasFollowingAccess( UTIL_ToString( Access ) ), Output );
				
				for( unsigned char n = 0; n <= Access; ++n )
				{
					string Commands;

					vector<string> m_CommandList = m_CCBot->m_DB->CommandList( n );

					for( vector<string> :: iterator i = m_CommandList.begin( ); i != m_CommandList.end( ); ++i )
						Commands = Commands + m_CommandTrigger + (*i) + ", ";

					Commands = Commands.substr( 0, Commands.size( ) -2 );

					if( m_CommandList.size ( ) )
						SendChatCommand( "/w " + User + " [" + UTIL_ToString( n ) + "]: " + Commands, Output );
					else
						break;
				}
			}

			//
			// !ADDSAFELIST
			// !ADDS
			//
				
			else if( ( Command == "addsafelist" || Command == "adds" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "addsafelist" ) )
			{					
				if( m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
					QueueChatCommand( m_CCBot->m_Language->UserAlreadySafelisted( Payload ), User, Whisper, Output );
				else
				{
					if( m_CCBot->m_DB->SafelistAdd( m_Server, Payload ) )
						QueueChatCommand( m_CCBot->m_Language->UserSafelisted( Payload ), User, Whisper, Output );
					else
						QueueChatCommand( m_CCBot->m_Language->ErrorSafelisting( Payload ), User, Whisper, Output );
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

				if( ( UTIL_ToInt32( Interval ) > 0 ) && AnnounceMessage.size( ) )
				{
					m_Announce = true;
					m_AnnounceMsg = AnnounceMessage;
					m_AnnounceInterval = UTIL_ToInt32( Interval );

					if( m_AnnounceInterval < 3 )
						m_AnnounceInterval = 4;

					QueueChatCommand( m_CCBot->m_Language->AnnounceEnabled( Interval ), User, Whisper, Output );				
				}
				else if( Interval == "off" && m_Announce == true )
				{
					m_Announce = false;
					QueueChatCommand( m_CCBot->m_Language->AnnounceDisabled( ), User, Whisper, Output );
				}
			}

			//
			// !BAN
			// !ADDBAN
			//

			else if( ( Command == "ban" || Command == "addban" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "ban" ) )
			{
				string Victim, Reason;
				stringstream SS;
				SS << Payload;
				SS >> Victim;
					
				if( Access >= m_CCBot->m_DB->AccessCheck( m_Server, Victim ) && !IsClanShaman( Victim ) && !IsClanChieftain( Victim ) )			
				{
					QueueChatCommand( "/ban " + Payload, BNET );
					m_CCBot->m_DB->AccessSet( m_Server, Victim, 0 );

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
						QueueChatCommand( m_CCBot->m_Language->UserAlreadyBanned( Victim, Ban->GetAdmin( ) ), User, Whisper, Output );
						delete Ban;
					}
					else
					{
						if( m_CCBot->m_DB->BanAdd( m_Server, Victim, User, Reason ) )
							QueueChatCommand( m_CCBot->m_Language->SuccesfullyBanned( Victim, User ), User, Whisper, Output );		
						else
							QueueChatCommand( m_CCBot->m_Language->ErrorBanningUser( Victim ), User, Whisper, Output );			
					}					
				}
				else
					QueueChatCommand( "Cannot ban higher clan ranks and people with higher access.", User, Whisper, Output );
			}
				
			//
			// !CHECK
			//

			else if( Command == "check" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "checkban" ) )
			{
				CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, Payload );
				int access = m_CCBot->m_DB->AccessCheck( m_Server, Payload );

				if( access == 11 )
					access = 0;

				if( IsClanMember( Payload ) )
				{
					if( IsClanPeon( Payload ) )
					{							
						if( Ban )							
							QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( access ) + "] access and banned from channel.", User, Whisper, Output );
						else
							QueueChatCommand( Payload + " is in clan, rank Peon, with [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
					}
					else if( IsClanGrunt( Payload ) )
					{
						if( Ban )
							QueueChatCommand( Payload + " is in clan, rank Grunt, with [" + UTIL_ToString( access ) + "] access and is banned from channel.", User, Whisper, Output );
						else
							QueueChatCommand( Payload + " is in clan, rank Grunt with [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
					}
					else if( IsClanShaman( Payload ) )
						QueueChatCommand( Payload + " is in clan, rank Shaman, with [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
					else if( IsClanChieftain( Payload ) )
						QueueChatCommand( Payload + " is in clan, rank Chieftain, with [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
				}
				else
				{
					if( !Ban )
						QueueChatCommand( Payload + " is not a clan member and has [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
					else
						QueueChatCommand( Payload + " is banned from the channel, not a clan member and has [" + UTIL_ToString( access ) + "] access.", User, Whisper, Output );
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

				unsigned char NewAccess;
				string UserName;
				stringstream SS;
				SS << Payload;
				SS >> NewAccess;

				if( SS.fail( ) || SS.eof( ) )
				{						
					QueueChatCommand( "Bad input - use form " + m_CommandTrigger + "setaccess <access 0-10> <name>", User, Whisper, Output );
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
						QueueChatCommand( "Cannot raise your own access.", User, Whisper, Output );
						CanSetAccess = false;
					}
					if( NewAccess >= Access && !Match( UserName, User ) && NewAccess != 10 )
					{
						QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper, Output );
						CanSetAccess = false;
					}
					if( OldAccess > NewAccess && !Match( UserName, User ) && OldAccess >= Access )
					{
						CanSetAccess = false;
						QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper, Output );
					}
					if( OldAccess > Access && !Match( UserName, User )  )
					{
						CanSetAccess = false;
						QueueChatCommand( "Cannot set access level higher or equal then your own.", User, Whisper, Output );
					}					
					
					if( CanSetAccess )
					{
						if( m_CCBot->m_DB->AccessSet( m_Server, UserName, NewAccess ) )
							QueueChatCommand( "Added user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper, Output );
						else
							QueueChatCommand( "Error adding user [" + UserName + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper, Output );
					}
				}					
			}
				
			//
			// !CHECKACCESS
			//

			else if( Command == "checkaccess" && Access >= m_CCBot->m_DB->CommandAccess( "checkaccess" ) )
			{
				if( Payload.empty( ) )
					Payload = User;

				if( m_CCBot->m_DB->AccessCheck( m_Server, Payload ) != 11 )
					QueueChatCommand( "User [" + Payload + "] has access of [" + UTIL_ToString( m_CCBot->m_DB->AccessCheck( m_Server, Payload ) ) + "].", User, Whisper, Output );
				else
					QueueChatCommand( "User [" + Payload + "] has access of [0].", User, Whisper, Output );
			}

			//
			// !COUNTACCESS
			//

			else if( Command == "countaccess" && Access >= m_CCBot->m_DB->CommandAccess( "countaccess" ) )
			{
				if( Payload.empty( ) )
					Payload = "0";
				else if( UTIL_ToInt32( Payload ) > 10 )
					Payload = "10";

				uint32_t Count = m_CCBot->m_DB->AccessCount( m_Server, UTIL_ToInt32( Payload ) );
			
				if( Count == 0 )
					QueueChatCommand( "There are no users with access of [" + Payload + "]", User, Whisper, Output );
				else if( Count == 1 )
					QueueChatCommand( "There is only one user with access of [" + Payload + "]", User, Whisper, Output );
				else
					QueueChatCommand( "There are [" + UTIL_ToString( Count ) + "] users with access [" + Payload + "]", User, Whisper, Output );
			}

			//
			// !DELACCESS
			//
				
			else if( Command == "delaccess" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "delaccess" ) )
			{
				if( !m_CCBot->m_DB->AccessCheck( m_Server, Payload ) )
					QueueChatCommand( "User has no set access.", User, Whisper, Output );
				else
				{
					if( m_CCBot->m_DB->AccessRemove( Payload ) )
						QueueChatCommand( "Deleted user [" + Payload + "]'s set access." , User, Whisper, Output );
					else
						QueueChatCommand( "Error deleting user [" + Payload + "]'s access.", User, Whisper, Output );
				}					
			}

			//
			// !CHECKSAFELIST
			// !CHECKS
			//

			if( ( Command == "checksafelist" || Command == "checks" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "checksafelist" ) )
			{
				if( m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
					QueueChatCommand( m_CCBot->m_Language->UserIsSafelisted( Payload ), User, Whisper, Output );
				else
					QueueChatCommand( m_CCBot->m_Language->UserNotSafelisted( Payload ), User, Whisper, Output );
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
					QueueChatCommand( "Access needed for the [" + Payload + "] command is [" + UTIL_ToString( m_CCBot->m_DB->CommandAccess( Payload ) ) + "]", User, Whisper, Output );
				else
					QueueChatCommand( "Error checking access for [" + Payload + "] command, doesn't exist.", User, Whisper, Output );
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
					QueueChatCommand( "Bad input - use form " + m_CommandTrigger + "setcommand <access 0-10> <command>", User, Whisper, Output );
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
							QueueChatCommand( "Updated [" + command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper, Output );
						else
							QueueChatCommand( "Error updating access for [" + command + "] with access of [" + UTIL_ToString( NewAccess ) + "]", User, Whisper, Output );
					}
					else
						QueueChatCommand( "Unable to set access for [" + command + "] because it doesn't exist.", User, Whisper, Output );
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

				QueueChatCommand( m_CCBot->m_Language->MessageQueueCleared( ), User, Whisper, Output );
			}

			//
			// !COUNTBANS
			//

			else if( Command == "countbans" && Access >= m_CCBot->m_DB->CommandAccess( "countbans" ) )
			{
				uint32_t Count = m_CCBot->m_DB->BanCount( m_Server );

				if( Count == 0 )
					QueueChatCommand( "There are no banned users from the channel.", User, Whisper, Output );
				else if( Count == 1 )
					QueueChatCommand( "There is one banned user from the channel.", User, Whisper, Output );
				else
					QueueChatCommand( "There are " + UTIL_ToString( Count ) + " banned users from the channel.", User, Whisper, Output );
			}					

			//
			// !COUNTSAFELIST
			// !COUNTS
			//

			else if( ( Command == "countsafelist" || Command == "counts" ) && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "countsafelist" ) )
			{
				uint32_t Count = m_CCBot->m_DB->SafelistCount( m_Server );

				if( Count == 0 )
					QueueChatCommand( "There are no safelisted users.", User, Whisper, Output );
				else if( Count == 1 )
					QueueChatCommand( "There is one safelisted user.", User, Whisper, Output );
				else
					QueueChatCommand( "There are " + UTIL_ToString( Count ) + " users safelisted.", User, Whisper, Output );
			}

			//
			// !CHANNEL
			//

			else if( ( Command == "channel" || Command == "join" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "channel" ) )
			{
				QueueChatCommand( "/join " + Payload, BNET );
				m_CurrentChannel = Payload;
			}

			//
			// !REJOIN
			//

			else if( Command == "rejoin" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "channel" ) )
			{
				QueueChatCommand( "/rejoin", BNET );
			}

			//
			// !CHANLIST
			//
								
			else if( Command == "chanlist" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "chanlist" ) )
			{
				string tempText;
					
				for( map<string, CUser *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); ++i )					
					tempText = tempText + (*i).second->GetUser( ) + ", ";					

				if( tempText.length( ) >= 2 )
					tempText = tempText.substr( 0, tempText.length( ) - 2 );

				QueueChatCommand( "Users in channel [" + UTIL_ToString( m_Channel.size( ) ) + "]: " + tempText + ".", User, Whisper, Output );			
			}

			//
			// !CLANLIST
			//

			else if( Command == "clanlist" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "clanlist" ) && m_ClanCommandsEnabled )
			{
				string Chieftains = "Chieftains: ", Shamans = "Shamans: ", Grunts = "Grunts: ", Peons = "Peons: ";

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

				if( Chieftains.size( ) > 12 )
				{
					if( Chieftains.size( ) > 179 )
					{
						SendChatCommand( "/w " + User + " " + Chieftains.substr( 0, 179 ), Output );
						SendChatCommand( "/w " + User + " " + Chieftains.substr( 179, 180 ), Output );				
					}
					else
						SendChatCommand( "/w " + User + " " + Chieftains.substr( 0, Chieftains.size( )-2 ), Output );
				}

				if( Shamans.size( ) > 9 )
				{
					if( Shamans.size( ) > 179 )
					{
						SendChatCommand( "/w " + User + " " + Shamans.substr( 0, 179 ), Output );
						SendChatCommand( "/w " + User + " " + Shamans.substr( 179, 180 ), Output );				
					}
					else
						SendChatCommand( "/w " + User + " " + Shamans.substr( 0, Shamans.size( )-2), Output );
				}

				if( Grunts.size( ) > 8 )
				{
					if( Grunts.size( ) > 179 )
					{
						SendChatCommand( "/w " + User + " " + Grunts.substr( 0, 179 ), Output );
						SendChatCommand( "/w " + User + " " + Grunts.substr( 179, 180 ), Output );				
					}
					else
						SendChatCommand( "/w " + User + " " + Grunts.substr( 0, Grunts.size( )-2), Output );
				}

				if( Peons.size( ) > 7 )
				{
					if( Peons.size( ) > 179 )
					{
						SendChatCommand( "/w " + User + " " + Peons.substr( 0, 179 ), Output );
						SendChatCommand( "/w " + User + " " + Peons.substr( 179, 180 ), Output );
					}
					else
						SendChatCommand( "/w " + User + " " + Peons.substr( 0, Peons.size( )-2), Output );
				}
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
					QueueChatCommand( "[" + Victim + "] successfully unbanned from the channel by " + User + ".", User, Whisper, Output );
					QueueChatCommand( "/unban " + Victim, BNET );
				}
				else
					QueueChatCommand( "Error unbanning [" + Victim + "] from the channel.", User, Whisper, Output );
			}

			//
			// !DELSAFELIST
			// !DELS
			//				

			else if( ( Command == "delsafelist" || Command == "dels" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "delsafelist" ) )
			{					
				if( !m_CCBot->m_DB->SafelistCheck( m_Server, Payload ) )
					QueueChatCommand( "User " + Payload + " is not safelisted.", User, Whisper, Output );
				else
				{
					if( m_CCBot->m_DB->SafelistRemove( m_Server, Payload ) )
						QueueChatCommand( "User " + Payload + " is deleted from safelist.", User, Whisper, Output );
					else
						QueueChatCommand( "Error deleting user " + Payload + " from the safelist.", User, Whisper, Output );
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
					QueueChatCommand( m_CCBot->m_Language->GameAnnouncerEnabled( ), User, Whisper, Output );
					m_AnnounceGames = true;
				}
				else if( Match( Payload, "off" ) )
				{
					QueueChatCommand( m_CCBot->m_Language->GameAnnouncerDisabled( ), User, Whisper, Output );
					m_AnnounceGames = false;
				}
			}				

			//
			// !GETCLAN
			//

			else if( Command == "getclan" && Access >= m_CCBot->m_DB->CommandAccess( "getclan" ) && m_ClanCommandsEnabled )
			{
				SendGetClanList( );
				QueueChatCommand( m_CCBot->m_Language->UpdatedClanList( ), User, Whisper, Output );
				QueueWhisperCommand( m_CCBot->m_Language->ReceivedClanMembers( UTIL_ToString( m_Clans.size( ) ) ), User, Output );
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
					QueueChatCommand( m_CCBot->m_Language->MustBeAClanMember( Payload ), User, Whisper, Output );
			}	

			//
			// !GREET
			//

			else if( Command == "greet"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "greet" ) )
			{	
				if( Match( Payload, "off" ) )
				{
					QueueChatCommand( m_CCBot->m_Language->GreetingDisabled( ), User, Whisper, Output );
					m_GreetUsers = false;
				}
				else if( Match( Payload, "on" ) )
				{
					QueueChatCommand( m_CCBot->m_Language->GreetingEnabled( ), User, Whisper, Output );
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
					QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Grunt" ), User, Whisper, Output );
				}
				else
					QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper, Output );
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
				bool Found = false;

				if ( !( Victim = GetUserFromNamePartial( Victim ) ).empty( ) )
					Found = true;

				if( Found && !IsClanShaman( Victim ) && !IsClanChieftain( Victim ) && !m_CCBot->m_DB->SafelistCheck( m_Server, Victim )  )
				{	
					if( !SS.eof( ) )
					{
							getline( SS, Reason );
							string :: size_type Start = Reason.find_first_not_of( " " );
	
							if( Start != string :: npos )
								Reason = Reason.substr( Start );
					}
						
					ImmediateChatCommand( "/kick " + Victim + " " + Reason, BNET );
				
				}
				else if ( !Found )
					QueueChatCommand( "Can kick only users in channel.", User, Whisper, Output );
				else
					QueueChatCommand( "Clan shamans, chieftains and safelisted users cannot be kicked.", User, Whisper, Output );
			}
			
			//
			// !LOCKDOWN
			//
				
			else if( Command == "lockdown" && !Match( "off", Payload ) && Access >= m_CCBot->m_DB->CommandAccess( "lockdown" ) )
			{
				if( Payload.empty( ) )
					m_AccessRequired = Access;
				else
				{	
					stringstream SS;
					SS << Payload;
					SS >> m_AccessRequired;
				}
				m_IsLockdown = true;
				QueueChatCommand( m_CCBot->m_Language->LockdownEnabled( UTIL_ToString( m_AccessRequired ) ), User, Whisper, Output );	
			}
				
			//
			// !LOCKDOWN OFF
			//
				
			else if( Command == "lockdown" && Match( "off", Payload ) && m_IsLockdown && Access >= m_CCBot->m_DB->CommandAccess( "lockdown" ) )
			{
				m_IsLockdown = false;
				QueueChatCommand( m_CCBot->m_Language->LockdownDisabled( ), User, Whisper, Output );

				for( vector<string> :: iterator i = m_Lockdown.begin( ); i != m_Lockdown.end( ); ++i )
					ImmediateChatCommand( "/unban " + *i, BNET );

				m_Lockdown.clear( );
			}

			//
			// !MOTD (clan message of the day)
			//

			else if( Command == "motd" && Access >= m_CCBot->m_DB->CommandAccess( "motd" ) && m_ClanCommandsEnabled )
			{					
				m_Socket->PutBytes( m_Protocol->SEND_SID_CLANSETMOTD( Payload ) );
				QueueChatCommand( m_CCBot->m_Language->SetMOTD( Payload ), User, Whisper, Output );
			}
				
			//
			// !PEON
			//

			else if( Command == "peon"  && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "peon" ) && m_ClanCommandsEnabled )
			{
				if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
				{
					SendClanChangeRank( Payload, CBNETProtocol :: CLAN_PARTIAL_MEMBER );
					QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Peon" ), User, Whisper, Output );
				}
				else
					QueueChatCommand( "Bot's account needs to be at least Shaman to perform this action.", User, Whisper, Output );
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
					QueueChatCommand( "Removing Chieftains and Shamans is prohibited.", User, Whisper, Output );
			}

			//
			// !RELOAD
			//

			else if( Command == "reload" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "reload" ) )
			{
				SendGetClanList( );
				m_CCBot->UpdateSwearList( );					
				m_CCBot->ReloadConfigs( );
				QueueChatCommand( m_CCBot->m_Language->CFGReloaded( ), User, Whisper, Output );
			}

			//
			// !SAY
			//

			else if( Command == "say" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "say" ) )
			{
				if( Access > 8 || Payload[0] != '/' )
					QueueChatCommand( Payload, BNET );
				else
					QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper, Output );					
			}

			//
			// !SAYBNET
			//

			else if( Command == "saybnet" && Access >= m_CCBot->m_DB-> CommandAccess( "say" ) && Payload.find( " " ) != string :: npos )
			{
				string :: size_type CommandStart = Payload.find( " " );
				string server = Payload.substr( 0, CommandStart );
				string command = Payload.substr( CommandStart + 1 );

				vector<CBNET *> :: iterator i = m_CCBot->GetServerFromNamePartial( server );
				
				if( i != m_CCBot->m_BNETs.end( ) )
				{
					if( Access > 8 || command[0] != '/' )						
						(*i)->QueueChatCommand( command, BNET );						
					else if( Payload[0] == '/' )						
						QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper, Output );
				}
				else
					QueueChatCommand( m_CCBot->m_Language->UnableToPartiallyMatchServer( ), User, Whisper, Output );
			}

			//
			// !SAYBNETS
			//
	
			else if( Command == "saybnets" && !Payload.empty( ) && Access >= m_CCBot->m_DB-> CommandAccess( "say" ) )
			{
				for( vector<CBNET *> :: iterator i = m_CCBot->m_BNETs.begin( ); i != m_CCBot->m_BNETs.end( ); ++i )
				{
					if( Access > 8 || Payload[0] != '/' )
						(*i)->QueueChatCommand( Payload, BNET );
					else if( (*i)->GetServer( ) == m_Server )
						QueueChatCommand( m_CCBot->m_Language->NotAllowedUsingSay( ), User, Whisper, Output );
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
					QueueChatCommand( m_CCBot->m_Language->ChangedRank( Payload, "Shaman" ), User, Whisper, Output );
				}
				else
					QueueChatCommand( "Bot's account needs to be Chieftain to perform this action.", User, Whisper, Output );
			}

			//
			// !SQUELCH
			// !IGNORE
			//

			else if( ( Command == "squelch" || Command == "ignore" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
			{
				if( GetSquelched( Payload ) == m_Squelched.end( ) )
				{
					QueueChatCommand( "/ignore " + Payload, BNET );
					QueueChatCommand( "Ignoring user " + Payload + ".", User, Whisper, Output );
					transform( Payload.begin( ), Payload.end( ), Payload.begin( ), (int(*)(int))tolower );
					m_Squelched.push_back( string ( Payload ) );
				}
				else
					QueueChatCommand( "User [" + Payload + "] is already ignored.", User, Whisper, Output );						
			}

			//
			// !SQUELCHLIST
			// !SL
			//

			else if( ( Command == "squelchlist" || Command == "sl" ) && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
			{
				if ( m_Squelched.size( ) )
				{
					string users;
					
					for( unsigned int i = 1; i <= m_Squelched.size( ); ++i )
					{
						if( m_Squelched[i].size( ) > 0 )
						{
							if( i != m_Squelched.size( ) )
								users = users + m_Squelched[i] + ", ";
							else
								users = users + m_Squelched[i];
						}
					}
					QueueChatCommand( "Squelched users: " + users + ".", User, Whisper, Output );
				}
				else
					QueueChatCommand( "There are no squelched users." , User, Whisper, Output );					
			}

			//
			// !TOPIC
			//

			else if( Command == "topic" && Access >= m_CCBot->m_DB->CommandAccess( "topic" ) )
			{					
				QueueChatCommand( "/topic " + m_CurrentChannel + " \"" + Payload + "\"", BNET );
				QueueChatCommand( m_CCBot->m_Language->SetTopic( Payload ), User, Whisper, Output );
			}
				
			//
			// !UNSQUELCH
			// !UNIGNORE
			//

			else if( ( Command == "unsquelch" || Command == "unignore" ) && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "squelch" ) )
			{
				if( GetSquelched( Payload ) != m_Squelched.end( ) )
				{
					m_Squelched.erase( GetSquelched( Payload ) );
					QueueChatCommand( "Unignoring user " + Payload + ".", User, Whisper, Output );
				}
				else
					QueueChatCommand( "User " + Payload + " is not ignored.", User, Whisper, Output );
			}

			//
			// !UPTIME
			//
				
			else if( Command == "uptime" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "uptime" ) )
			{
				string date;
				uint64_t time = GetTime( ) - m_CCBot->m_Uptime;

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
				date += UTIL_ToString( (uint32_t) time ) + "s";
					QueueChatCommand( m_CCBot->m_Language->Uptime( m_UserName, date ), User, Whisper, Output );
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
						QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " by " + Ban->GetAdmin( ) + ".", User, Whisper, Output );
					else
						QueueChatCommand( "User [" + Payload + "] was banned on " + Ban->GetDate( ) + " because \"" + Ban->GetReason( ) + "\" by " + Ban->GetAdmin( ) + ".", User, Whisper, Output );
					delete Ban;
					Ban = NULL;
				}
				else
					QueueChatCommand( "User [" + Payload + "] is not banned.", User, Whisper, Output );
			}

			//
			// !GN8
			//
				
			else if( Command == "gn8" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "gn8" ) )
				QueueChatCommand( m_CCBot->m_Language->GN8( User ), BNET ); 

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
						QueueChatCommand( "Clan invitation was sent.", User, Whisper, Output );
					}
					else
						QueueChatCommand( "Bot needs to have at least a Shaman rank to invite.", User, Whisper, Output );
				}
				else
					QueueChatCommand( "You can't invite people already in clan!", User, Whisper, Output );
			}						
				
			//
			// !JOINCLAN 
			// !JOIN
			//
				
			else if( ( Command == "joinclan" || Command == "join" ) && Payload.empty( ) && !IsClanMember( User ) && IsInChannel( User ) && m_ClanCommandsEnabled )
			{
				if( m_SelfJoin )
				{
					if( IsClanShaman( m_UserName ) || IsClanChieftain( m_UserName ) )
					{
						m_LastKnown = User;
						m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( User ) );
						SendGetClanList( );
					}
				}
				else
					QueueChatCommand( m_CCBot->m_Language->CommandDisabled( ), User, Whisper, Output );
			}

			//
			// !ONLINE 
			// !O
			//
				
			else if( ( Command == "online" || Command == "o" ) && Access >= m_CCBot->m_DB->CommandAccess( "online" ) && m_ClanCommandsEnabled )
			{
				string Online;
				uint32_t OnlineNumber = 0;				
				
				for( unsigned int i = 1; i <= m_Clans.size( ); ++i ) 
				{
					if( m_Clans[i]->GetStatus( ) == "Online" && !Match( m_Clans[i]->GetName( ), m_UserName ) && !Match( m_Clans[i]->GetName( ), m_HostbotName ) )
					{
						if( i != m_Clans.size( ) )
							Online = Online + m_Clans[i]->GetName( ) + ", ";
						else
							Online = Online + m_Clans[i]->GetName( );
							
						++OnlineNumber;
					}
				}

				if( OnlineNumber == 0 )
					SendChatCommand( "/w " + User + " " + "There are no " + m_ClanTag + " members online.", Output );
				else if( Online.size( ) < 156 ) 
					SendChatCommand( "/w " + User + " " + m_ClanTag + " members online [" + UTIL_ToString(OnlineNumber) + "]: " + Online, Output );
				else
				{
					SendChatCommand( "/w " + User + " " + m_ClanTag + " members online [" + UTIL_ToString(OnlineNumber) + "]: " + Online.substr( 0,155 ), Output );
					SendChatCommand( "/w " + User + " " + Online.substr( 155, Online.size( )-155 ), Output );
				}
			}
 
			//
			// !SLAP
			//

			else if ( Command == "slap" && !Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "slap" ) )
			{
				if ( GetUserFromNamePartial( Payload ).size( ) )
					Payload = GetUserFromNamePartial( Payload );

				if( Payload == User )					
					Payload = "himself";

				srand( time( 0 ) );					
				int Random = ( rand( ) % 6 );					

				switch( Random )
				{
					case 0: QueueChatCommand( User + " slaps " + Payload + " around a bit with a large trout.", BNET );
						break;

					case 1: QueueChatCommand( User + " slaps " + Payload + " around a bit with a pink Macintosh.", BNET );
						break;

					case 2: QueueChatCommand( User + " throws a Playstation 3 at " + Payload + ".", BNET );
						break;

					case 3: QueueChatCommand( User + " drives a car over " + Payload + ".", BNET );
						break;

					case 4: QueueChatCommand( User + " finds " + Payload + " on uglypeople.com.", BNET );
						break;

					case 5: Random = rand( ); QueueChatCommand( User + " searches google.com for \"goatsex + " + Payload + "\". " + UTIL_ToString( Random ) + " hits WEEE!", BNET );	
						break; 
				}
			}
			
			//
			// !SPIT
			//

			else if ( Command == "spit" && !Payload.empty( ) && IsInChannel( User ) && Access >= m_CCBot->m_DB->CommandAccess( "spit" ) )
			{
				if ( GetUserFromNamePartial( Payload ).size( ) )
					Payload = GetUserFromNamePartial( Payload );

				srand( time( 0 ) );  
				int Random = ( rand( ) % 6 );

				switch( Random )
				{
					case 0: QueueChatCommand( User + " spits " + Payload + " but misses completely!", BNET );
						break;

					case 1: QueueChatCommand( User + " spits " + Payload + " and hits him in the eye!", BNET );
						break;

					case 2: QueueChatCommand( User + " spits " + Payload + " and hits him in his mouth!", BNET );
						break;

					case 3: QueueChatCommand( User + " spits " + Payload + " and nails it into his ear!", BNET );
						break;

					case 4: QueueChatCommand( User + " spits " + Payload + " but he accidentaly hits himself!", BNET );
						break;

					case 5: QueueChatCommand( Payload + " counterspits " + User + ". Lame.", BNET );
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

				if ( GetUserFromNamePartial( Victim ).size( ) )
					Victim = GetUserFromNamePartial( Victim );

				QueueChatCommand( User + " serves " + Victim + " a " + Object + ".", BNET );				 
			}

			//
			// !STATUS
			//

			else if ( Command == "status" && Payload.empty( ) && Access >= m_CCBot->m_DB->CommandAccess( "status" ) )
			{
				string message = "Status: ";

		       		for( vector<CBNET *> :: iterator i = m_CCBot->m_BNETs.begin( ); i != m_CCBot->m_BNETs.end( ); ++i )
				{
					message += (*i)->GetUserName( ) + ( (*i)->GetLoggedIn( ) ? " [Online], " : " [Offline], " );
				}

				QueueChatCommand( message.substr( 0, message.size( ) - 2 ) + ".", User, Whisper, Output );						 
			}				

			//
			// !VERSION
			//

			else if( Command == "version" && Payload.empty( ) )					
					QueueChatCommand( m_CCBot->m_Language->Version( m_CCBot->m_Version ), User, Whisper, Output );

			//
			// !PING
			//

			else if( Command == "ping" && Access >= m_CCBot->m_DB->CommandAccess( "ping" ) )
			{
				if( Payload.empty( ) )
					Payload = User;			
				else if( GetUserFromNamePartial( Payload ).size( ) )
					Payload = GetUserFromNamePartial( Payload );

				CUser *Result = GetUserByName( Payload );

				if( Result )
					QueueChatCommand( m_CCBot->m_Language->Ping( Payload, UTIL_ToString( Result->GetPing( ) ) , m_ServerAlias), User, Whisper, Output );
				else
					QueueChatCommand( m_CCBot->m_Language->CannotAccessPing( ), User, Whisper, Output );
			}	
				
			//
			// !RESTART
			//

			else if( Command == "restart" && Access >= m_CCBot->m_DB->CommandAccess( "restart" ) )
			{
				m_CCBot->Restart( );	
			}				
		}		
	}

	else if( Event == CBNETProtocol :: EID_INFO )
	{
		CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );

		if( m_AnnounceGames && m_Channel.size( ) >= 2 )		
		{
			if( Message.find( "is using Warcraft III Frozen Throne and is currently in  game" ) != string :: npos || Message.find( "is using Warcraft III Frozen Throne and is currently in private game" ) != string :: npos || Message.find( "is using Warcraft III The Frozen Throne in game" ) != string :: npos)
			{
				string user = Message.substr( 0, Message.find_first_of(" ") );

				if( !Match( user, m_HostbotName ) )
				{			
					Message = Message.substr( Message.find_first_of("\"") + 1, Message.find_last_of("\"") - Message.find_first_of("\"") - 1 );				
					QueueChatCommand( m_CCBot->m_Language->AnnounceGame( user, Message ), BNET );
				}
			}			
		}							
	}

	else if( Event == CBNETProtocol :: EID_JOIN )
	{
		// check if the user joined has been banned if yes ban him again and send him the reason the first time he tries to join
		CDBBan *Ban = m_CCBot->m_DB->BanCheck( m_Server, User );

		if( Ban )
		{
			string Info = "by " + Ban->GetAdmin( );
			if( !Ban->GetReason( ).empty( ) )
				Info += ": " + Ban->GetReason( );
			Info += " on " + Ban->GetDate( );
			SendChatCommand( "/ban " + User + " " + Info, BNET );

			delete Ban;
		}
		else
		{	
			// if there is a lockdown and the person can't join do not add it to the channel list and greet him - that's why we use bool CanJoinChannel
			if( m_IsLockdown )
			{
				if( m_AccessRequired > Access && !Match( User, m_HostbotName ) )
				{
					SendChatCommand( "/ban " + User + " Lockdown: only users with " + UTIL_ToString( m_AccessRequired ) + "+ access can join", BNET );

					if( GetLockdown( User ) == m_Lockdown.end( ) )
						m_Lockdown.push_back( User );

					return;
				}				
			}

			// only greet in originally set channel
			
			if( Match( m_CurrentChannel, m_FirstChannel ) && m_GreetUsers && !Match( User, m_HostbotName ) ) 
			{
				ImmediateChatCommand( m_CCBot->m_Language->WelcomeMessageLine1( m_FirstChannel, User ), BNET );
				QueueChatCommand( m_CCBot->m_Language->WelcomeMessageLine2( m_FirstChannel, User ), BNET );
			}

			CUser *user = GetUserByName( User );
			if( !user )
				user = new CUser( User, chatEvent->GetPing( ), chatEvent->GetUserFlags( ) );

			if( Message.size( ) >= 14 )
			{
				reverse( Message.begin( ), Message.end( ) );		
				user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );			
			}

			if( m_BanChat && Message == "TAHC" && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) )
			{
				ImmediateChatCommand( "/kick " + User + " Chat clients are forbidden to enter the channel", BNET );
			}

			m_Channel[lowerUser] = user;
			
			if ( m_CCBot->m_DB->AccessCheck( m_Server, User ) == 11 && IsClanMember( User ) && m_ClanDefaultAccess > 0 )
			{					
				m_CCBot->m_DB->AccessSet( m_Server, User, m_ClanDefaultAccess );
				CONSOLE_Print( "[ACCESS: " + m_ServerAlias + "] Setting [" + User + "] access to clan default of " + UTIL_ToString( m_ClanDefaultAccess ) ); 
			}

			CONSOLE_ChannelWindowChanged( );
			CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + ":" + m_CurrentChannel + "] user [" + User + "] joined the channel." );					
		}		
	}

	else if( Event == CBNETProtocol :: EID_LEAVE )
	{
		if ( m_AnnounceGames && !Match( m_HostbotName, User ) )
			SendChatCommandHidden( "/whois " + User, BNET );

		for( map<string, CUser *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); ++i )
		{
			if( i->first == lowerUser )
			{
				delete i->second;
				m_Channel.erase( i );
			}
		}

		CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + ":" + m_CurrentChannel + "] user [" + User + "] left the channel." );
		CONSOLE_ChannelWindowChanged( );
	}

	else if( Event == CBNETProtocol :: EID_ERROR )
	{
		CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );
		
		if( Message == "You are banned from that channel." )
			m_RejoinInterval = 300;
		else if( Message == "Your message quota has been exceeded!" )
			m_Delay = m_Delay + 1500;
	}

	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		CONSOLE_Print( "[CHANNEL: " + m_ServerAlias + "] Joining channel [" + Message + "]" );

		if( !Match( Message, m_CurrentChannel ) )
			m_Rejoin = true;
		else
		{
			m_Rejoin = false;
			m_RejoinInterval = 15;
		}

		for( map<string, CUser *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); ++i )
			delete i->second;

		m_Channel.clear( );
		CONSOLE_ChannelWindowChanged( );		
	}

	else if( Event == CBNETProtocol :: EID_SHOWUSER )
	{
		CUser *user = GetUserByName( lowerUser );

		if( !user )
			user = new CUser( User, chatEvent->GetPing( ), chatEvent->GetUserFlags( ) );

		if( Message.size( ) >= 14 )
		{
			reverse( Message.begin( ), Message.end( ) ); 
			user->SetClan( Message.substr( 0, Message.find_first_of(" ") ) );			
		}

		if( m_BanChat && Message == "TAHC" && !m_CCBot->m_DB->SafelistCheck( m_Server, User ) )
		{
			// this method ("CHAT" client checking) works exclusively on PvPGNs as BNET banned Telnet clients
			ImmediateChatCommand( "/kick " + User + " Chat clients are banned from channel", BNET );
		}

		m_Channel[lowerUser] = user;
		CONSOLE_ChannelWindowChanged( );
	}
	
	CONSOLE_MainWindowChanged( );
}