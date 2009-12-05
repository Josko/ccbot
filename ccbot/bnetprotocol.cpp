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
#include "bnetprotocol.h"

CBNETProtocol :: CBNETProtocol( )
{
	unsigned char ClientToken[] = { 220, 1, 203, 7 };
	m_ClientToken = UTIL_CreateByteArray( ClientToken, 4 );
}

CBNETProtocol :: ~CBNETProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

bool CBNETProtocol :: RECEIVE_SID_NULL( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_NULL" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

bool CBNETProtocol :: RECEIVE_SID_ENTERCHAT( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_ENTERCHAT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated string	-> UniqueName

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		m_UniqueName = UTIL_ExtractCString( data, 4 );
		return true;
	}

	return false;
}

CIncomingChatEvent *CBNETProtocol :: RECEIVE_SID_CHATEVENT( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CHATEVENT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> EventID
	// 4 bytes					-> User Flags
	// 4 bytes					-> Ping
	// 12 bytes					-> ???
	// null terminated string	-> User
	// null terminated string	-> Message

	if( ValidateLength( data ) && data.size( ) >= 29 )
	{
		BYTEARRAY EventID = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		BYTEARRAY UserFlags = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 );
		BYTEARRAY Ping = BYTEARRAY( data.begin( ) + 12, data.begin( ) + 16 );
		BYTEARRAY User = UTIL_ExtractCString( data, 28 ); 
		BYTEARRAY Message = UTIL_ExtractCString( data, User.size( ) + 29 );

		switch( UTIL_ByteArrayToUInt32( EventID, false ) )
		{
			case CBNETProtocol :: EID_SHOWUSER:
			case CBNETProtocol :: EID_JOIN:
			case CBNETProtocol :: EID_LEAVE:
			case CBNETProtocol :: EID_WHISPER:
			case CBNETProtocol :: EID_TALK:
			case CBNETProtocol :: EID_BROADCAST:
			case CBNETProtocol :: EID_CHANNEL:
			case CBNETProtocol :: EID_USERFLAGS:
			case CBNETProtocol :: EID_WHISPERSENT:
			case CBNETProtocol :: EID_CHANNELFULL:
			case CBNETProtocol :: EID_CHANNELDOESNOTEXIST:
			case CBNETProtocol :: EID_CHANNELRESTRICTED:
			case CBNETProtocol :: EID_INFO:
			case CBNETProtocol :: EID_ERROR:
			case CBNETProtocol :: EID_EMOTE:
				return new CIncomingChatEvent(	(CBNETProtocol :: IncomingChatEvent)UTIL_ByteArrayToUInt32( EventID, false ),
													UTIL_ByteArrayToUInt32( UserFlags, false ),
													UTIL_ByteArrayToUInt32( Ping, false ),
													string( User.begin( ), User.end( ) ),
													string( Message.begin( ), Message.end( ) ) );
		}

	}

	return NULL;
}

bool CBNETProtocol :: RECEIVE_SID_FLOODDETECTED( BYTEARRAY data )
{
	DEBUG_Print( "RECEIVED SID_FLOODDETECTED" );
	DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	if( ValidateLength( data ) && data.size( ) == 4 )
		return true;

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_CHECKAD( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CHECKAD" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

BYTEARRAY CBNETProtocol :: RECEIVE_SID_PING( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_PING" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Ping

	if( ValidateLength( data ) && data.size( ) >= 8 )
		return BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

	return BYTEARRAY( );
}

bool CBNETProtocol :: RECEIVE_SID_LOGONRESPONSE( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_LOGONRESPONSE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY Status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( Status, false ) == 1 )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_INFO( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_INFO" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> LogonType
	// 4 bytes					-> ServerToken
	// 4 bytes					-> ???
	// 8 bytes					-> MPQFileTime
	// null terminated string			-> IX86VerFileName
	// null terminated string			-> ValueStringFormula

	if( ValidateLength( data ) && data.size( ) >= 25 )
	{
		m_LogonType = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		m_ServerToken = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 );
		m_MPQFileTime = BYTEARRAY( data.begin( ) + 16, data.begin( ) + 24 );
		m_IX86VerFileName = UTIL_ExtractCString( data, 24 );
		m_ValueStringFormula = UTIL_ExtractCString( data, m_IX86VerFileName.size( ) + 25 );
		return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_CHECK( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_CHECK" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> KeyState
	// null terminated string		-> KeyStateDescription

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		m_KeyState = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		m_KeyStateDescription = UTIL_ExtractCString( data, 8 );

		if( UTIL_ByteArrayToUInt32( m_KeyState, false ) == KR_GOOD )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGON( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status
	// if( Status == 0 )
	// 32 bytes					-> Salt
	// 32 bytes					-> ServerPublicKey

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( status, false ) == 0 && data.size( ) >= 72 )
		{
			m_Salt = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 40 );
			m_ServerPublicKey = BYTEARRAY( data.begin( ) + 40, data.begin( ) + 72 );
			return true;
		}
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY Status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( Status, false ) == 0 )
			return true;
	}

	return false;
}

int CBNETProtocol :: RECEIVE_SID_CLANINVITATION( BYTEARRAY data )
{

	// DEBUG_Print( "RECEIVED SID_CLANINVITATION" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Cookie
	// 1 byte					-> Result	

	if( ValidateLength( data ) && data.size( ) == 9 )
	{
		return data[8];
	}

	return -1;

}

int CBNETProtocol :: RECEIVE_SID_CLANREMOVEMEMBER( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANREMOVEMEMBER" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Cookie
	// 1 byte					-> Result	


	if( ValidateLength( data ) && data.size( ) == 9 )
	{
		return data[8];
	}

	return -1;
}

vector<CIncomingClanList *> CBNETProtocol :: RECEIVE_SID_CLANMEMBERLIST( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMEMBERLIST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> ???
	// 1 byte					-> Total
	// for( 1 .. Total )
	// null term string				-> Name
	// 1 byte					-> Rank
	// 1 byte					-> Status
	// null term string				-> Location

	vector<CIncomingClanList *> ClanList;

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		unsigned int i = 9;
		unsigned char Total = data[8];
		
		while( Total > 0 )
		{
			Total--;

			if( data.size( ) < i + 1 )
				break;

			BYTEARRAY Name = UTIL_ExtractCString( data, i );
			i += Name.size( ) + 1;

			if( data.size( ) < i + 3 )
				break;

			unsigned char Rank = data[i];
			unsigned char Status = data[i + 1];
			i += 2;

			// in the original VB source the location string is read but discarded, so that's what I do here

			BYTEARRAY Location = UTIL_ExtractCString( data, i );
			i += Location.size( ) + 1;
			ClanList.push_back( new CIncomingClanList(	string( Name.begin( ), Name.end( ) ),
														Rank,
														Status ) );
		}
	}

	return ClanList;
}

CIncomingClanList *CBNETProtocol :: RECEIVE_SID_CLANMEMBERSTATUSCHANGE( BYTEARRAY data )
{
	DEBUG_Print( "RECEIVED SID_CLANMEMBERSTATUSCHANGE" );
	DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated string			-> Name
	// 1 byte					-> Rank
	// 1 byte					-> Status
	// null terminated string			-> Location

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		BYTEARRAY Name = UTIL_ExtractCString( data, 4 );

		if( data.size( ) >= Name.size( ) + 7 )
		{
			unsigned char Rank = data[Name.size( ) + 5];
			unsigned char Status = data[Name.size( ) + 6];

			// in the original VB source the location string is read but discarded, so that's what I do here

			BYTEARRAY Location = UTIL_ExtractCString( data, Name.size( ) + 7 );
			return new CIncomingClanList(	string( Name.begin( ), Name.end( ) ),
											Rank,
											Status );
		}
	}

	return NULL;
}

bool CBNETProtocol :: RECEIVE_SID_CLANINVITATIONRESPONSE( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANINVITATIONRESPONSE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Cookie
	// 4 bytes					-> Clan Tag (Reversed)
	// x bytes (null terminated string)		-> Clan Name
	// x bytes (null terminated string)		-> Inviter Name

	if( ValidateLength( data ) && data.size( ) >= 14  )
	{
		m_ClanTag = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 ); 
		m_ClanName = UTIL_ExtractCString( data, 12 );
		m_Inviter = BYTEARRAY( data.begin( ) + 12 + m_ClanName.size( ), data.end( ) - 1 );
		m_InviterStr = UTIL_ExtractCString( data, m_ClanName.size( ) + 13 );

		return true;
	}	

	return false;
}

int CBNETProtocol :: RECEIVE_SID_CLANMAKECHIEFTAIN( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMAKECHIEFTAIN" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Cookie
	// 1 byte					-> Status

	if( ValidateLength( data ) && data.size( ) == 9  )
	{
		return data[8];
	}
	
	return false;
}

bool CBNETProtocol :: RECEIVE_SID_CLANCREATIONINVITATION( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANCREATIONINVITATION" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Cookie
	// 4 bytes					-> Clan Tag (Reversed)
	// x bytes (null terminated string)		-> Clan Name
	// x bytes (null terminated string)		-> Inviter Name
	// 1 byte					-> Number of users invited
	// x bytes (list of strings)			-> List of users invited

	if( ValidateLength( data ) && data.size( ) >= 16  )
	{
		m_ClanCreationTag = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 );
		m_ClanCreationName = UTIL_ExtractCString( data, 13 );
		m_ClanCreator = UTIL_ExtractCString( data, m_ClanCreationName.size( ) + 14  );

		return true;
	}
	
	return false;
}

////////////////////
// SEND FUNCTIONS //
////////////////////

BYTEARRAY CBNETProtocol :: SEND_PROTOCOL_INITIALIZE_SELECTOR( )
{
	BYTEARRAY packet;
	packet.push_back( 1 );

	// DEBUG_Print( "SENT PROTOCOL_INITIALIZE_SELECTOR" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_NULL( )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_NULL );					// SID_NULL
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_NULL" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_ENTERCHAT( )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_ENTERCHAT );				// SID_ENTERCHAT
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// Account Name is NULL on Warcraft III/The Frozen Throne
	packet.push_back( 0 );						// Stat String is NULL on CDKEY'd products
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_ENTERCHAT" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_JOINCHANNEL( string channel )
{
	unsigned char NoCreateJoin[]	= { 2, 0, 0, 0 };
	unsigned char FirstJoin[]	= { 1, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
	packet.push_back( SID_JOINCHANNEL );					// SID_JOINCHANNEL
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( 0 );							// packet length will be assigned later

	if( channel.size( ) > 0 )
		UTIL_AppendByteArray( packet, NoCreateJoin, 4 );		// flags for no create join
	else
		UTIL_AppendByteArray( packet, FirstJoin, 4 );			// flags for first join

	UTIL_AppendByteArrayFast( packet, channel );
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_JOINCHANNEL" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CHATCOMMAND( string command )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CHATCOMMAND );		// SID_CHATCOMMAND
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArrayFast( packet, command );	// Message
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_CHATCOMMAND" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CHECKAD( )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CHECKAD );		// SID_CHECKAD
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendByteArray( packet, Zeros, 4 );	// ???
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_CHECKAD" );
	// DEBUG_Print( packet );
	return packet;
}
BYTEARRAY CBNETProtocol :: SEND_SID_PING( BYTEARRAY pingValue )
{
	BYTEARRAY packet;

	if( pingValue.size( ) == 4 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
		packet.push_back( SID_PING );			// SID_PING
		packet.push_back( 0 );				// packet length will be assigned later
		packet.push_back( 0 );				// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, pingValue );	// Ping Value
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_PING" );

	// DEBUG_Print( "SENT SID_PING" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_LOGONRESPONSE( BYTEARRAY clientToken, BYTEARRAY serverToken, BYTEARRAY passwordHash, string accountName )
{
	// todotodo: check that the passed BYTEARRAY sizes are correct (don't know what they should be right now so I can't do this today)

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_LOGONRESPONSE );				// SID_LOGONRESPONSE
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArrayFast( packet, clientToken );		// Client Token
	UTIL_AppendByteArrayFast( packet, serverToken );		// Server Token
	UTIL_AppendByteArrayFast( packet, passwordHash );		// Password Hash
	UTIL_AppendByteArrayFast( packet, accountName );		// Account Name
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_LOGONRESPONSE" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, string countryAbbrev, string country )
{
	unsigned char ProtocolID[]		= {   0,   0,   0,   0 };
	unsigned char PlatformID[]		= {  54,  56,  88,  73 };	// "IX86"
	unsigned char ProductID_ROC[]		= {  51,  82,  65,  87 };	// "WAR3"
	unsigned char ProductID_TFT[]		= {  80,  88,  51,  87 };	// "W3XP"
	unsigned char Version[]			= { ver,   0,   0,   0 };
	unsigned char Language[]		= {  83,  85, 110, 101 };	// "enUS"
	unsigned char LocalIP[]			= { 127,   0,   0,   1 };
	unsigned char TimeZoneBias[]		= { 108, 253, 255, 255 };
	unsigned char LocaleID[]		= {   9,  12,   0,   0 };
	unsigned char LanguageID[]		= {   9,   4,   0,   0 };	// 0x0409 English (United States)

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
	packet.push_back( SID_AUTH_INFO );					// SID_AUTH_INFO
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( 0 );							// packet length will be assigned later
	UTIL_AppendByteArray( packet, ProtocolID, 4 );				// Protocol ID
	UTIL_AppendByteArray( packet, PlatformID, 4 );				// Platform ID

	if( TFT )
		UTIL_AppendByteArray( packet, ProductID_TFT, 4 );		// Product ID (TFT)
	else
		UTIL_AppendByteArray( packet, ProductID_ROC, 4 );		// Product ID (ROC)

	UTIL_AppendByteArray( packet, Version, 4 );				// Version
	UTIL_AppendByteArray( packet, Language, 4 );				// Language
	UTIL_AppendByteArray( packet, LocalIP, 4 );				// Local IP for NAT compatibility
	UTIL_AppendByteArray( packet, TimeZoneBias, 4 );			// Time Zone Bias
	UTIL_AppendByteArray( packet, LocaleID, 4 );				// Locale ID
	UTIL_AppendByteArray( packet, LanguageID, 4 );				// Language ID
	UTIL_AppendByteArrayFast( packet, countryAbbrev );			// Country Abbreviation
	UTIL_AppendByteArrayFast( packet, country );				// Country
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_AUTH_INFO" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_CHECK( BYTEARRAY clientToken, BYTEARRAY exeVersion, BYTEARRAY exeVersionHash, BYTEARRAY keyInfoROC, BYTEARRAY keyInfoTFT, string exeInfo, string keyOwnerName )
{
	uint32_t NumKeys = 0;
	uint32_t UsingSpawn = 0;

	if( keyInfoTFT.empty( ) )
		NumKeys = 1;
	else
		NumKeys = 2;

	BYTEARRAY packet;

	if( clientToken.size( ) == 4 && exeVersion.size( ) == 4 && exeVersionHash.size( ) == 4 && keyInfoROC.size( ) == 36 && ( keyInfoTFT.empty( ) || keyInfoTFT.size( ) == 36 ) )
	{
		packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
		packet.push_back( SID_AUTH_CHECK );				// SID_AUTH_CHECK
		packet.push_back( 0 );						// packet length will be assigned later
		packet.push_back( 0 );						// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientToken );		// Client Token
		UTIL_AppendByteArrayFast( packet, exeVersion );			// EXE Version
		UTIL_AppendByteArrayFast( packet, exeVersionHash );		// EXE Version Hash
		UTIL_AppendByteArray( packet, NumKeys, false );			// number of keys in this packet
		UTIL_AppendByteArray( packet, UsingSpawn, false );		// boolean Using Spawn (32 bit)
		UTIL_AppendByteArrayFast( packet, keyInfoROC );			// ROC Key Info

		if( !keyInfoTFT.empty( ) )
			UTIL_AppendByteArrayFast( packet, keyInfoTFT );		// TFT Key Info

		UTIL_AppendByteArrayFast( packet, exeInfo );			// EXE Info
		UTIL_AppendByteArrayFast( packet, keyOwnerName );		// CD Key Owner Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_CHECK" );

	// DEBUG_Print( "SENT SID_AUTH_CHECK" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGON( BYTEARRAY clientPublicKey, string accountName )
{
	BYTEARRAY packet;

	if( clientPublicKey.size( ) == 32 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGON );			// SID_AUTH_ACCOUNTLOGON
		packet.push_back( 0 );						// packet length will be assigned later
		packet.push_back( 0 );						// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientPublicKey );		// Client Key
		UTIL_AppendByteArrayFast( packet, accountName );		// Account Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY clientPasswordProof )
{
	BYTEARRAY packet;

	if( clientPasswordProof.size( ) == 20 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGONPROOF );			// SID_AUTH_ACCOUNTLOGONPROOF
		packet.push_back( 0 );						// packet length will be assigned later
		packet.push_back( 0 );						// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientPasswordProof );	// Client Password Proof
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( packet );
	return packet;
}
	
BYTEARRAY CBNETProtocol :: SEND_SID_CLANINVITATION( string username )
{
	unsigned char Cookie[] = { 0, 1, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANINVITATION );		// SID_CLANINVITATION
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4);	// Cookie: 00 01 00 00
	UTIL_AppendByteArrayFast( packet, username );	// Target name			
	AssignLength( packet );				// Assign packet length

	// DEBUG_Print( "SENT SID_CLANINVITATION" );
	// DEBUG_Print( packet );

	return packet;	
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANREMOVEMEMBER( string username )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANREMOVEMEMBER );	// SID_CLANREMOVEMEMBER 
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4);	// Cookie: 00 00 00 00
	UTIL_AppendByteArrayFast( packet, username );	// Target name			
	AssignLength( packet );				// Assign packet length

	// DEBUG_Print( "SENT SID_CLANREMOVEMEMBER" );
	// DEBUG_Print( packet );

	return packet;	
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANCHANGERANK( string username,  CBNETProtocol :: RankCode rank )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );       // BNET header constant
	packet.push_back( SID_CLANCHANGERANK );         // SID_CLANCHANGERANK
	packet.push_back( 0 );                          // packet length will be assigned later
	packet.push_back( 0 );                          // packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );      // Cookie: 00 01 00 00
	UTIL_AppendByteArrayFast( packet, username );
	packet.push_back( rank );
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_CLANCHANGERANK" );
	// DEBUG_Print( packet );

	return packet;

}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANMEMBERLIST( )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANMEMBERLIST );		// SID_CLANMEMBERLIST
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	AssignLength( packet );

	// DEBUG_Print( "SENT SID_CLANMEMBERLIST" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANSETMOTD( string message )
{
	unsigned char Cookie[] = { 11, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANSETMOTD );		// SID_CLANSETMOTD
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArrayFast( packet, message );	// message
	AssignLength( packet );

	// DEBUG_Print( "SEND SID_CLANMOTD" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANINVITATIONRESPONSE( BYTEARRAY clantag, BYTEARRAY inviter, bool response )
{
	unsigned char Cookie[] = { 0, 11, 0 };
	unsigned char Accept[] = { 0, 6 };
	unsigned char Decline[] = { 0, 4 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
	packet.push_back( SID_CLANINVITATIONRESPONSE );		// CLANINVITATIONRESPONSE
	packet.push_back( 0 );					// packet length will be assigned later
	packet.push_back( 0 );					// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 3 );		// cookie
	UTIL_AppendByteArrayFast( packet, clantag );		// clan tag
	UTIL_AppendByteArrayFast( packet, inviter );		// inviter
	if( response )
		UTIL_AppendByteArray( packet, Accept, 2 );	// response - 0x06: Accept
	else
		UTIL_AppendByteArray( packet, Decline, 2 );	// response - 0x04: Decline
	AssignLength( packet );					// assign length

	// DEBUG_Print( "SEND SID_CLANINVITATIONRESPONSE" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANMAKECHIEFTAIN( string username )
{
	unsigned char Cookie[] = { 1, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
	packet.push_back( SID_CLANMAKECHIEFTAIN );		// CLANINVITATIONRESPONSE
	packet.push_back( 0 );					// packet length will be assigned later
	packet.push_back( 0 );					// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );		// cookie
	UTIL_AppendByteArrayFast( packet, username );		// clan tag
	AssignLength( packet );					// assign length

	DEBUG_Print( "SEND SID_CLANMAKECHIEFTAIN" );
	DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANCREATIONINVITATION( BYTEARRAY clantag, BYTEARRAY inviter )
{
	unsigned char Cookie[] = { 2, 0, 0, 0 };
	unsigned char Accept[] = { 0, 6 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANCREATIONINVITATION );	// CLANCREATIONINVITATION
	packet.push_back( 0 );				// packet length will be assigned later
	packet.push_back( 0 );				// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArrayFast( packet, clantag );	// clan tag
	UTIL_AppendByteArrayFast( packet, inviter );	// inviter	
	UTIL_AppendByteArray( packet, Accept, 2 );	// response - 0x06: Accept
	AssignLength( packet );				// assign length

	// DEBUG_Print( "SEND SID_CLANCREATIONINVITATION" );
	// DEBUG_Print( packet );
	return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CBNETProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), false );
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNETProtocol :: ValidateLength( BYTEARRAY &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	uint16_t Length;
	BYTEARRAY LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[2] );
		LengthBytes.push_back( content[3] );
		Length = UTIL_ByteArrayToUInt16( LengthBytes, false );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}

//
// CIncomingChatEvent
//

CIncomingChatEvent :: CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, uint32_t nUserFlags, uint32_t nPing, string nUser, string nMessage )
{
	m_ChatEvent = nChatEvent;
	m_UserFlags = nUserFlags;
	m_Ping = nPing;
	m_User = nUser;
	m_Message = nMessage;
}

CIncomingChatEvent :: ~CIncomingChatEvent( )
{

}
//
// CIncomingClanList
//

CIncomingClanList :: CIncomingClanList( string nName, unsigned char nRank, unsigned char nStatus )
{
	m_Name = nName;
	m_Rank = nRank;
	m_Status = nStatus;
}

CIncomingClanList :: ~CIncomingClanList( )
{

}

string CIncomingClanList :: GetRank( )
{
	switch( m_Rank )
	{
		case 0: return "Recruit";
		case 1: return "Peon";
		case 2: return "Grunt";
		case 3: return "Shaman";
		case 4: return "Chieftain";
	}

	return "Rank Unknown";
}

string CIncomingClanList :: GetStatus( )
{
	if( m_Status == 0 )
		return "Offline";
	else
		return "Online";
}

string CIncomingClanList :: GetDescription( )
{
	string Description;
	Description += GetName( ) + "\n";
	Description += GetStatus( ) + "\n";
	Description += GetRank( ) + "\n\n";
	return Description;
}
