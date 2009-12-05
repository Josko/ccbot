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

#ifndef BNETPROTOCOL_H
#define BNETPROTOCOL_H

//
// CBNETProtocol
//

#define BNET_HEADER_CONSTANT 255

class CIncomingChatEvent;
class CIncomingClanList;

class CBNETProtocol
{
public:
	enum Protocol {
		SID_NULL				= 0,	// 0x0
		SID_STOPADV				= 2,	// 0x2
		SID_GETADVLISTEX			= 9,	// 0x9
		SID_ENTERCHAT				= 10,	// 0xA
		SID_JOINCHANNEL				= 12,	// 0xC
		SID_CHATCOMMAND				= 14,	// 0xE
		SID_CHATEVENT				= 15,	// 0xF
		SID_FLOODDETECTED			= 19,	// 0x13
		SID_CHECKAD				= 21,	// 0x15
		SID_STARTADVEX3				= 28,	// 0x1C
		SID_DISPLAYAD				= 33,	// 0x21
		SID_NOTIFYJOIN				= 34,	// 0x22
		SID_PING				= 37,	// 0x25
		SID_LOGONRESPONSE			= 41,	// 0x29
		SID_NETGAMEPORT				= 69,	// 0x45
		SID_AUTH_INFO				= 80,	// 0x50
		SID_AUTH_CHECK				= 81,	// 0x51
		SID_AUTH_ACCOUNTLOGON			= 83,	// 0x53
		SID_AUTH_ACCOUNTLOGONPROOF		= 84,	// 0x54
		SID_FRIENDSLIST				= 101,	// 0x65
		SID_FRIENDSUPDATE			= 102,	// 0x66
		SID_CLANCREATIONINVITATION		= 114,	// 0x72
		SID_CLANMAKECHIEFTAIN			= 116,	// 0x74
		SID_CLANINVITATION 			= 119,  // 0x77
		SID_CLANREMOVEMEMBER			= 120,  // 0x78
		SID_CLANINVITATIONRESPONSE		= 121,  // 0x79
		SID_CLANCHANGERANK			= 122,	// 0x7A
		SID_CLANSETMOTD				= 123,	// 0x7B
		SID_CLANMEMBERLIST			= 125,	// 0x7D
		SID_CLANMEMBERSTATUSCHANGE		= 127	// 0x7F		
	};

	enum KeyResult {
		KR_GOOD					= 0,
		KR_OLD_GAME_VERSION			= 256,
		KR_INVALID_VERSION			= 257,
		KR_ROC_KEY_IN_USE			= 513,
		KR_TFT_KEY_IN_USE			= 529
	};

	enum IncomingChatEvent {
		EID_SHOWUSER				= 1,	// received when you join a channel (includes users in the channel and their information)
		EID_JOIN				= 2,	// received when someone joins the channel you're currently in
		EID_LEAVE				= 3,	// received when someone leaves the channel you're currently in
		EID_WHISPER				= 4,	// received a whisper message
		EID_TALK				= 5,	// received when someone talks in the channel you're currently in
		EID_BROADCAST				= 6,	// server broadcast
		EID_CHANNEL				= 7,	// received when you join a channel (includes the channel's name, flags)
		EID_USERFLAGS				= 9,	// user flags updates
		EID_WHISPERSENT				= 10,	// sent a whisper message
		EID_CHANNELFULL				= 13,	// channel is full
		EID_CHANNELDOESNOTEXIST			= 14,	// channel does not exist
		EID_CHANNELRESTRICTED			= 15,	// channel is restricted
		EID_INFO				= 18,	// broadcast/information message
		EID_ERROR				= 19,	// error message
		EID_EMOTE				= 23	// emote
	};

	enum RankCode {
		CLAN_INITIATE = 0,			// 0x00 First week member
		CLAN_PARTIAL_MEMBER = 1,		// 0x01 Peon
		CLAN_MEMBER = 2,			// 0x02 Grunt
		CLAN_OFFICER = 3,			// 0x03 Shaman
		CLAN_LEADER = 4				// 0x04 Chieftain
	};

	enum UserFlags {
		FLAG_BLIZZARD = 1,			// Blizzard representative - 0x01
		FLAG_CHANNELOPERATOR = 2,		// Channel operator - 0x02
		FLAG_SPEAKER = 4,			// Speaker - 0x04
		FLAG_ADMINISTRATOR = 8,			// B.Net administrator - 0x08
		FLAG_NOUDP = 16,			// No UDP - 0x10
	};

	BYTEARRAY m_ClanTag;
	BYTEARRAY m_ClanName;
	BYTEARRAY m_Inviter;
	BYTEARRAY m_InviterStr;
	BYTEARRAY m_ClanCreationTag;
	BYTEARRAY m_ClanCreator;
	BYTEARRAY m_ClanCreationName;

private:
	BYTEARRAY m_ClientToken;			// set in constructor
	BYTEARRAY m_LogonType;				// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ServerToken;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_MPQFileTime;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_IX86VerFileName;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ValueStringFormula;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_KeyState;				// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_KeyStateDescription;		// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_Salt;				// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_ServerPublicKey;			// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_UniqueName;				// set in RECEIVE_SID_ENTERCHAT

public:
	CBNETProtocol( );
	~CBNETProtocol( );

	BYTEARRAY GetClientToken( )			{ return m_ClientToken; }
	BYTEARRAY GetLogonType( )			{ return m_LogonType; }
	BYTEARRAY GetServerToken( )			{ return m_ServerToken; }
	BYTEARRAY GetMPQFileTime( )			{ return m_MPQFileTime; }
	BYTEARRAY GetIX86VerFileName( )			{ return m_IX86VerFileName; }
	string GetIX86VerFileNameString( )		{ return string( m_IX86VerFileName.begin( ), m_IX86VerFileName.end( ) ); }
	BYTEARRAY GetValueStringFormula( )		{ return m_ValueStringFormula; }
	string GetValueStringFormulaString( )		{ return string( m_ValueStringFormula.begin( ), m_ValueStringFormula.end( ) ); }
	BYTEARRAY GetKeyState( )			{ return m_KeyState; }
	string GetKeyStateDescription( )		{ return string( m_KeyStateDescription.begin( ), m_KeyStateDescription.end( ) ); }
	BYTEARRAY GetSalt( )				{ return m_Salt; }
	BYTEARRAY GetServerPublicKey( )			{ return m_ServerPublicKey; }
	BYTEARRAY GetUniqueName( )			{ return m_UniqueName; }
	string GetInviterStr( )				{ return string( m_InviterStr.begin( ), m_InviterStr.end( ) ); }
	BYTEARRAY GetClanTag( )				{ return m_ClanTag; }
	BYTEARRAY GetInviter( )				{ return m_Inviter; }
	string GetClanName( )				{ return string( m_ClanName.begin( ), m_ClanName.end( ) ); }
	BYTEARRAY GetClanCreationTag( )			{ return m_ClanCreationTag; }
	string GetClanCreationName( )			{ return string( m_ClanCreationName.begin( ), m_ClanCreationName.end( ) ); }
	BYTEARRAY GetClanCreator( )			{ return m_ClanCreator; }
	string GetClanCreatorStr( )			{ return string( m_ClanCreator.begin( ), m_ClanCreator.end( ) ); }

	// receive functions

	bool RECEIVE_SID_NULL( BYTEARRAY data );
	bool RECEIVE_SID_ENTERCHAT( BYTEARRAY data );
	CIncomingChatEvent *RECEIVE_SID_CHATEVENT( BYTEARRAY data );
	bool RECEIVE_SID_FLOODDETECTED( BYTEARRAY data );
	bool RECEIVE_SID_CHECKAD( BYTEARRAY data );
	BYTEARRAY RECEIVE_SID_PING( BYTEARRAY data );
	bool RECEIVE_SID_LOGONRESPONSE( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_INFO( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_CHECK( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGON( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data );
	vector<CIncomingClanList *> RECEIVE_SID_CLANMEMBERLIST( BYTEARRAY data );
	CIncomingClanList *RECEIVE_SID_CLANMEMBERSTATUSCHANGE( BYTEARRAY data );
	int RECEIVE_SID_CLANINVITATION( BYTEARRAY data );
	int RECEIVE_SID_CLANREMOVEMEMBER( BYTEARRAY data );
	bool RECEIVE_SID_CLANINVITATIONRESPONSE( BYTEARRAY data );
	int RECEIVE_SID_CLANMAKECHIEFTAIN( BYTEARRAY data );
	bool RECEIVE_SID_CLANCREATIONINVITATION( BYTEARRAY data );
	

	// send functions

	BYTEARRAY SEND_PROTOCOL_INITIALIZE_SELECTOR( );
	BYTEARRAY SEND_SID_NULL( );
	BYTEARRAY SEND_SID_ENTERCHAT( );
	BYTEARRAY SEND_SID_JOINCHANNEL( string channel );
	BYTEARRAY SEND_SID_CHATCOMMAND( string command );
	BYTEARRAY SEND_SID_CHECKAD( );
	BYTEARRAY SEND_SID_PING( BYTEARRAY pingValue );
	BYTEARRAY SEND_SID_LOGONRESPONSE( BYTEARRAY clientToken, BYTEARRAY serverToken, BYTEARRAY passwordHash, string accountName );
	BYTEARRAY SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, string countryAbbrev, string country );
	BYTEARRAY SEND_SID_AUTH_CHECK( BYTEARRAY clientToken, BYTEARRAY exeVersion, BYTEARRAY exeVersionHash, BYTEARRAY keyInfoROC, BYTEARRAY keyInfoTFT, string exeInfo, string keyOwnerName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGON( BYTEARRAY clientPublicKey, string accountName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY clientPasswordProof );
	BYTEARRAY SEND_SID_FRIENDSLIST( );
	BYTEARRAY SEND_SID_CLANMEMBERLIST( );
	BYTEARRAY SEND_SID_CLANINVITATION( string username );
	BYTEARRAY SEND_SID_CLANCHANGERANK( string username, CBNETProtocol :: RankCode rank );
	BYTEARRAY SEND_SID_CLANREMOVEMEMBER( string username );
	BYTEARRAY SEND_SID_CLANSETMOTD( string message );
	BYTEARRAY SEND_SID_CLANINVITATIONRESPONSE( BYTEARRAY clantag, BYTEARRAY inviter, bool response );
	BYTEARRAY SEND_SID_CLANMAKECHIEFTAIN( string username );
	BYTEARRAY SEND_SID_CLANCREATIONINVITATION( BYTEARRAY data, BYTEARRAY inviter );

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

//
// CIncomingChatEvent
//

class CIncomingChatEvent
{
private:
	CBNETProtocol :: IncomingChatEvent m_ChatEvent;
	uint32_t m_UserFlags;
	uint32_t m_Ping;
	string m_User;
	string m_Message;

public:
	CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, uint32_t nUserFlags, uint32_t nPing, string nUser, string nMessage );
	~CIncomingChatEvent( );

	CBNETProtocol :: IncomingChatEvent GetChatEvent( )	{ return m_ChatEvent; }
	uint32_t GetUserFlags( )				{ return m_UserFlags; }
	uint32_t GetPing( )					{ return m_Ping; }
	string GetUser( )					{ return m_User; }
	string GetMessage( )					{ return m_Message; }
};


//
// CIncomingClanList
//

class CIncomingClanList
{
private:
	string m_Name;
	unsigned char m_Rank;
	unsigned char m_Status;

public:
	CIncomingClanList( string nName, unsigned char nRank, unsigned char nStatus );
	~CIncomingClanList( );

	string GetName( )					{ return m_Name; }
	string GetRank( );
	string GetStatus( );
	string GetDescription( );
};

#endif
