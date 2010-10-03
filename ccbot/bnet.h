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

#ifndef BNET_H
#define BNET_H

#include "bnetprotocol.h"

//
// CBNET
//

class CTCPClient;
class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CIncomingChatEvent;
class CUser;

class CBNET
{
public:
	CCCBot *m_CCBot;

	private:
	CTCPClient *m_Socket;						// the connection to battle.net
	CBNETProtocol *m_Protocol;					// battle.net protocol
	queue<CCommandPacket *> m_Packets;			// queue of incoming packets
	queue<BYTEARRAY> m_OutPackets;				// queue of outgoing packets to be sent (to prevent getting kicked for flooding)
	CBNCSUtilInterface *m_BNCSUtil;				// the interface to the bncsutil library (used for logging into battle.net)
	queue<string> m_ChatCommands;				// queue of chat commands waiting to be sent (to prevent getting kicked for flooding)
	vector<string> m_Admins;					// vector of cached admins
	vector<string> m_Lockdown;					// vector of tempbanned users whom will be unbanned later
	vector<string> m_Squelched;					// vector of squelched users, lowercase names
	multimap<string, string> m_SpamCache;		// cache used to store ( loweruser, lowermessage ) for detecting spam
	bool m_Exiting;								// set to true and this class will be deleted next update
	string m_Server;							// battle.net server to connect to
	string m_ServerAlias;						// alias to the connected battle.net
	string m_ServerIP;                          // battle.net server to connect to (the IP address so we don't have to resolve it every time we connect)
	string m_CDKeyROC;							// ROC CD key
	string m_CDKeyTFT;							// TFT CD key
	string m_CountryAbbrev;						// country abbreviation
	string m_Country;							// country
	string m_UserName;							// battle.net username
	string m_UserPassword;						// battle.net password
	string m_FirstChannel;						// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	string m_ClanRank;							// clan rank of bot's user
	string m_CurrentChannel;					// the current chat channel
	string m_RootAdmin;							// the root admin
	string m_HostbotName;						// hostbot's name if present
	string m_AnnounceMsg;						// text printed on X interval by the announce command
	char m_CommandTrigger;						// the character prefix to identify commands
	string m_CommandTriggerStr;					// the character prefix to identify commands
	unsigned char m_War3Version;				// custom warcraft 3 version for PvPGN users
	BYTEARRAY m_EXEVersion;						// custom exe version for PvPGN users
	BYTEARRAY m_EXEVersionHash;					// custom exe version hash for PvPGN users
	string m_PasswordHashType;					// password hash type for PvPGN users
	int m_AnnounceInterval;						// interval of two consecutive announce messages
	uint32_t m_AccessRequired;					// access required to join the channel when lockdown is on
	uint64_t m_Delay;							// delay of the next QueueChatCommand based on the length of the past one		
	uint64_t m_ClanDefaultAccess;				// default access a clan members has
	uint32_t m_MaxMessageLength;				// maximum message length for PvPGN users
	uint64_t m_NextConnectTime;					// GetTime when we should try connecting to battle.net next (after we get disconnected)
	uint64_t m_LastNullTime;					// GetTime when the last null packet was sent for detecting disconnects
	uint64_t m_LastGetClanTime;					// GetTime when the last SendGetClanList was send for updating the clan list
	uint64_t m_LastAnnounceTime;				// GetTime when the last Announce message was sent
	uint64_t m_LastRejoinTime;					// GetTime when the last /join was attempted
	uint64_t m_RejoinInterval;					// interval between two consecutive rejoins
	uint64_t m_LastInvitationTime;				// GetTime when the last SID_CLANINVITATION was recevied
	uint64_t m_LastChatCommandTicks;			// GetTicks when the last chat command was sent for the m_ChatCommands queue
	uint64_t m_LastOutPacketTicks;				// GetTicks when the last Packet was sent
	uint64_t m_LastSpamCacheCleaning;			// GetTicks when the last m_SpamCache was last cleared
	bool m_WaitingToConnect;					// if we're waiting to reconnect to battle.net after being disconnected
	bool m_LoggedIn;							// if we've logged into battle.net or not
	bool m_InChat;								// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	string m_LastKnown;							// who used the -join command last, gets deleted (set to "unknown user") after we get the packet
	string m_Removed;							// who gets removed from clan, gets deleted (set to "unknown user") after we get the packet
	string m_UsedRemove;						// who used the -remove command last, gets deleted (set to "unknown user") after we get the packet
	bool m_AntiSpam;							// set to true and will kick spammers
	bool m_Announce;							// bool variable if a announce message is active or not
	bool m_IsLockdown;							// hold True of False, depending on !LockDown
	bool m_ActiveInvitation;					// bool true if we got an SID_CLANINVITATION in the last 29 seconds
	bool m_ActiveCreation;						// bool true if we got an clan creation invitaiton in the last 29 seconds
	bool m_Rejoin;								// indicates that the bot is in the "wrong" channel
	bool m_ClanCommandsEnabled;					// bool true if bot is in a clan and the users can use clan commands
	bool m_AnnounceGames;						// set to true and every game joined by users from channel will be announced in channel
	bool m_BanChat;								// set to true and CHAT clients get kicked from channel
	bool m_SwearingKick;						// set to true and every message containing swears (contained in swears.cfg) will get the user kicked
	bool m_SelfJoin;							// set to true and !join command will be enabled
	bool m_GreetUsers;							// greet users on join
	string m_ClanTag;							// clan tag	

public:
	CBNET( CCCBot *nCCBot, string nServer, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, uint32_t nMaxMessageLength, string nClanTag, bool nGreetUsers, bool nSwearingKick, bool nAnnounceGames, bool nSelfJoin, bool nBanChat, uint32_t nClanDefaultAccess, string nHostbotname, bool nAntiSPam );
	~CBNET( );

	vector<CIncomingClanList *> m_Clans;		// vector of clan members
	map<string, CUser *> m_Channel;				// map of users in channel

	bool GetExiting( )					{ return m_Exiting; }
	string GetServer( )					{ return m_Server; }
	string GetCDKeyROC( )				{ return m_CDKeyROC; }
	string GetCDKeyTFT( )				{ return m_CDKeyTFT; }
	string GetUserName( )				{ return m_UserName; }
	string GetUserPassword( )			{ return m_UserPassword; }
	string GetFirstChannel( )			{ return m_FirstChannel; }
	string GetCurrentChannel( )			{ return m_CurrentChannel; }
	string GetRootAdmin( )				{ return m_RootAdmin; }
	string GetClanTag( )				{ return m_ClanTag; }
	string GetHostBotName( )			{ return m_HostbotName; }
	char GetCommandTrigger( )			{ return m_CommandTrigger; }
	BYTEARRAY GetEXEVersion( )			{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )		{ return m_EXEVersionHash; }
	string GetPasswordHashType( )		{ return m_PasswordHashType; }
	bool GetLoggedIn( )					{ return m_LoggedIn; }
	bool GetInChat( )					{ return m_InChat; }

	vector<string> :: iterator GetSquelched( string name );
	vector<string> :: iterator GetLockdown( string name );

	// processing functions

	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	inline void ExtractPackets( );
	inline void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );

	// functions to send packets to battle.net

	void SendChatCommand( string chatCommand, int destination );
	void SendChatCommandHidden( string chatCommand, int destination );
	void SendGetClanList( );
	void SendClanChangeRank( string accountName, CBNETProtocol::RankCode rank );
	
	// other functions

	void QueueChatCommand( string chatCommand, int destination );
	void QueueChatCommand( string chatCommand, string user, bool whisper, int destination );
	void QueueWhisperCommand( string chatCommand, string user, int destination );
	void ImmediateChatCommand( string chatCommand, int destination );
	void ImmediateChatCommand( string chatCommand, string user, bool whisper, int destination );

	bool IsRootAdmin( string name );	
	bool IsInChannel( string name );
	bool IsClanPeon( string name );
	bool IsClanGrunt( string name );
	bool IsClanShaman( string name );
	bool IsClanChieftain( string name );
	bool IsClanMember( string name );

	string GetUserFromNamePartial( string name );
	CUser *GetUserByName( string name );

	bool Match( string string1, string string2 )
	{
		transform( string1.begin( ), string1.end( ), string1.begin( ), (int(*)(int))tolower );
		transform( string2.begin( ), string2.end( ), string2.begin( ), (int(*)(int))tolower );

		return string1 == string2;
	};
};

//
// CUser
//

class CUser
{
private:
	string m_User;
	int m_Ping;
	uint32_t m_UserFlags;
	string m_Clan;
public:
	CUser( string nUser, int nPing, uint32_t nUserFlags );
	~CUser( );

	string GetUser( )							{ return m_User; }
	int GetPing( )								{ return m_Ping; }
	uint32_t GetUserFlags( )					{ return m_UserFlags; }
	string GetClan( )							{ return m_Clan; }
	
	void SetPing( int nPing )					{ m_Ping = nPing; }
	void SetUserFlags( uint32_t nUserFlags )	{ m_UserFlags = nUserFlags; }
	void SetClan( string nClan )				{ m_Clan = nClan; }
};

#endif
