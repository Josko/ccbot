=========================================
CCBot - Clan & Channel Bot - Version 1.02
=========================================

CCBot is a modified GHost++ bot (made by Varlock) to moderate both the channel and the clan it's located in. It's written in C++ and works on Windows and Linux (and possibly OS X with minor changes). It's meant to be small and powerful with almost no dependecies. Works on PvPGNs and B.NET. Use at your own risk.

Its source is freely available on http://code.google.com/p/ccbot while the GHost++ source is available at http://code.google.com/p/ghostplusplus

=============
Configuration
=============

CCBot is configured via the configuration files - ghost.cfg and language.cfg located in the "cfg" folder.
The program itself runs in console mode and does not take any console input (it outputs messages to the console for information purposes only).

***You need to edit ccbot.cfg before running CCBot***
***You need to edit ccbot.cfg before running CCBot***
***You need to edit ccbot.cfg before running CCBot***

There is also a swears.cfg file in the "cfg" folder in which you configure banned words and/or phrases.

=============
Console Input
=============

Input via console and the channel user list only work for the first B.NET connection. Commands via console work the same way as on B.NET - input starting with the
set command trigger will be executed while other input is sent to B.NET similar to chat bots. The console refresh might be flickery with a lot of activity (several
connections and high channel activity).

To chat via other connections - use !saybnets and !saybnet commands.

==============
Required Files
==============

If you want to be able to connect to battle.net:

-> "Game.dll" in your bot_war3path
-> "Storm.dll" in your bot_war3path
-> "war3.exe" in your bot_war3path

You also need Microsoft Visual C++ 2010 Redistributable Package (x86): http://www.microsoft.com/downloads/details.aspx?FamilyID=a7b7a05e-6de6-4d3a-a423-37bf0912db84&displaylang=en

===============
Clan Moderation
===============

GHost Chat also had built in Clan support, for most functions he needs to be at least a Shaman in the clan - also making him a chieftain is _never_ a good idea.

WARNING : All of the clan features could cause damage (by your account being banned, excessive traffic, etc.) if you use them while the bot is NOT a clan member. A measure
preventing this exists but there is always a risk.

=======================
How Access System Works
=======================

User access levels range from 0 to 10. Root admin has automatically access of 10 and it's encouraged not to give anyone such a high level of access.
You can change both user access level and commands access level to execute them.

Note: squelch keyword defines !squelch, !unsquelch and !squelchlist commands.

=============
How Bans Work
=============

Each ban is attached to a specific realm.
When you run the !addban or !ban commands on battle.net, the ban will be attached to the realm you ran the command on.

A person with the banned username won't be able to join the channel anymore. Please note that for bans the bot needs to have 
the appropriate flags - Channel Operator/Channel Admin/Shaman rank on some PvPGNs or Shaman rank on B.Net.

When you run the !delban or !unban commands all bans for that username will be deleted regardless of realm.
CC Bot considers a user to be banned if it finds a ban for that user on ANY of the defined realms.

===============
How Greets Work
===============

Greet system greets any non-banned user on join. It allows up to 2 lines for greet messages
and greet all together can be enabled disabled on start in ghost.cfg or in channel with
!greet off or !greet on.

In lagnuage.cfg you have for 2 line greets:

lang_0020 = /w $USER$ Welcome to the $CHANNEL$ channel! Enjoy your stay.
lang_0021 = Come again for more DotA games.

If you want to disable any of the lines without recompiling (sending 1 instead of 2 lines)
just shorten the line to only 1 or 0 characters. Example for sending 1 line greet:

lang_0020 = /w $USER$ Welcome to the $CHANNEL$ channel! Enjoy your stay.
lang_0021 = 

Because lang_0021 message length is lower then 1 - it won't be printed and sent to the server.

==============================
Using CCBot on Multiple Realms
==============================

DOESN'T SUPPORT ORIGINAL B.NET! But it is possible to make it work on B.Net but that's something very complicated for most users. While the Warden is down - it's again viable for this bot to work on BNET as well.

Here's how:

1.) When CCBot starts up it reads up to 9 sets of battle.net connection information from ccbot.cfg.
2.) A set of battle.net connection information contains the following keys:
 a.) *_server (required)
 b.) *_cdkeyroc
 c.) *_cdkeytft
 d.) *_username (required)
 e.) *_password (required)
 f.) *_firstchannel
 g.) *_rootadmin
 h.) *_commandtrigger
 i.) *_custom_war3version
 j.) *_custom_exeversion
 k.) *_custom_exeversionhash
 l.) *_custom_passwordhashtype
 m.) *_clantag
 n.) *_greetusers
 o.) *_swearingkick
 p.) *_selfjoin
 q.) *_announcegames
 r.) *_banchat
 s.) *_antispam
 t.) *_clandefaultaccess
 u.) *_hostbotname
3.) CCBot will search for battle.net connection information by replacing the "*" in each key above with "bnet_" then "bnet2_" then "bnet3_" and so on until "bnet9_".
 a.) Note that CCBot doesn't search for "bnet1_" for backwards compatibility reasons.
4.) If CCBot doesn't find a *_server key it stops searching for any further battle.net connection information.
5.) If any of the required keys are missing CCBot will skip that set of battle.net connection information and start searching for the next one.

========
Commands
========

In battle.net (via local chat or whisper at any time).

Parameters in angled brackets <like this> are required and parameters in square brackets [like this] are optional.

!addaccess <access <name>	sets the users access from 0-10
!countaccess <access>		counts how many people have # access
!checkaccess <name>		checks the users access
!delaccess <name>		sets the users access to 0
!addban <name> [reason]		add a new ban to the database for this realm
!ban                    	alias to !addban
!channel <name>          	change battle.net channel
!join <name>			alias to !channel
!countbans               	display the total number of bans for this realm
!delban <name>           	remove a ban from the database for all realms
!exit	                 	shutdown ccbot
!quit	                 	alias to !exit
!getclan                 	refresh the internal copy of the clan members list
!say <text>              	send <text> to battle.net as a chat command
!unban <name>            	alias to !delban
!version                 	display version information
!clanlist		 	lists the clan members by clan ranks (Peon, Grunt, Shaman, Chieftain - Recruits who are less then 7 days in the clan are considered Peons)
!shaman <name>			promotes a grunt or peon to shaman rank (bot needs chieftain rank for that - not recommended!)
!grunt <name>		 	promotes a peon to grunt rank (bot needs at least a Shaman rank for that)
!peon <name>		 	demotes a grunt to a peon rank (bot needs at least a Shaman rank for that)
!remove <name>		 	removes a peon/grunt from the clan (bot needs at least a Shaman rank for that)
!games <off>|<on>	 	announces the games users from the channel enter
!kick <name>		 	kicks the user from the channel
!squelch <name>		 	squelches a user
!ignore <name>			alias to !squelch
!unsquelch <name>	 	unsquelches a user
!unignore <name>		alias to !unsquelch
!clearqueue		 	clears the message queue
!cq			 	alias to !clearqueue
!announce <interval> <text>	prints the set text every X seconds in channel
!announce <off>                 turns off announce
!greet <off|on>			enables or disables the greet message(s) on user join
!squelchlist			lists names of squelched users
!sl				alias to !squelchlist
!addsafelist <name>		adds a user to the safelist
!adds <name>			alias to !addsafelist
!delsafelist <name>		removes a user from the safelist for all realms
!dels <name>			alias to !delsafelist
!countsafelist			display the total number of safelisted users on this realm
!counts				alias to !countsafelist
!checksafelist <name>		check if a user is safelisted on this realm
!checks <name>			alias to !checksafelist
!topic <message>		sets the current channel's topic to the specified message
!chanlist			shows the current users in the channel
!match <text>			tries to partially match the entered text to a user currently in the channel (used for testing)
!check <name>			checks the user if he is banned, if he in the clan, if yes - which rank and does he have admin access
!motd <text>			sets the clan MOTD to the set text
!reload				reloads the text CFG files and clan list
!uptime				shows the uptime of the bot
!online                  	lists current online clan members
!o			 	alias to !online
!slap <name>		 	slaps a user, prints random text
!spit <name>		 	spits a user, print random text
!join			 	sends a clan invite to the user who sent the command
!checkban <name>         	check if a user is banned on this realm
!gn8			 	prints a good night text
!invite <name>			invites a user to the clan
!command <name>			shows the access needed for the set command
!setcommand <access> <name> 	sets the access for the set command (use full names for commands - !clearqueue instead of !cq)
!accept				accepts a active clan invitation
!ping [name]			shows users ping (to server)
!restart			closes and starts the bot again - if you rename the binary this command won't work ( ccbot.exe filename for Windows, ccbot++ for Linux )
!lockdown <access>		activates lockdown mode - everyone with a lower access cannot enter
!lockdown off			deactivates lockdown mode and everyone banned during lockdown gets unbanned
!saybnets <text>		sends the chat command to every BNET we're connected to
!saybnet <bnet>	<text>		sends the chat command just to the specified BNET we're connected to
!chieftain <name>		transfers the chieftain position to the specified user
!rejoin				bot rejoins the current channel
!status				shows all servers the bot is connected to and if he's logged in or not
?trigger			shows the command trigger currently used

==========================
Compiling CCBot on Windows
==========================

1. Download Visual C++ 2010 Express Edition at http://www.microsoft.com/express/
2. Open ccbot.sln
3. Choose Release version
4. Build

Notes:

An already compiled BNSCUtil.dll is supplied but BNSCUtil project and source are still here. Users just need to compile
the ccbot project and replace the current binary if they do not experience any problem with the current libs, if problems occur try compiling all of the librares yourself, then CCBot then try again.


========================
Compiling CCBot on Linux
========================

You will need a few libraries, all of which are installed, if you have apt, with following command:

1. sudo apt-get install build-essential m4 libgmp3-dev libncurses5-dev

Then, assuming you extracted this archive to your home dir (~):

2. cd ~/ccbot/bncsutil/src/bncsutil/
3. make
4. sudo make install

Once it's built you can continue:

5. cd ~/ccbot/ccbot/
6. make
(ignore the sqlite3.c warnings)

The binary will be located in ~/ccbot/ccbot.

======================
Running CCBot on Linux
======================

You will need to copy ~/ccbot/bncsutil/src/bncsutil/libbncutil.so to /usr/local/lib/ or otherwise set LD_LIBRARY_PATH so it can find the bncsutil library. (Install bncsutil)
You will also need to copy game.dll, Storm.dll, and war3.exe from a valid Warcraft III installation to the location specified in your ccbot.cfg 
Then when in the CCBot binary directory just type in terminal "./ccbot++" without the quotes, to run it.

=========
CHANGELOG

Version 1.02 ( 6.11.2010. )

- “logs” folder renamed into “log”
- create “log” and “cfg” folders if necessary
- bot_log is now a boolean ( 0 = false; 1 = true )
- Logging can be disabled during runtime now (set it to 0 in .cfg then use !reload or !restart)
- Fixed compilation problems on Windows
- Fixed g++ warnings on Linux
- Small tweak for better SQLite3 performance
- Using init. lists in constructors for better startup/!restart performance
- Removed some unecessary variables
- Overall better performance
- Fixed a bug found in the !ban command
- Updated SQLite3 to 3.7.3 (from 3.7.0.1)

Version 1.01 ( 03.10.2010. )

- Final fix to the swears.cfg
- Time is now a 64-bit unsigned int (no more overflow after 30+ days)
- Fixed an segmentation fault on exit
- Fixed Linux compatibility
- Other misc. fixes

Version 1.00 ( 22.08.2010. )

- Fixed a bug with ping
- Fixed a bug with swears.cfg not loading
- Fixed a bug with swears.cfg not generating when not present
- Couple of miscellanous fixes
- Updated SQLite to version 3.7.0.1
- Fixed a bug with clan members and CHAT clients (telnet/bots)
- Cannot ban people with higher access and clan shamans & chieftains
- Ban sets the victims access to 0
- People with access < 5 are affected by anti-spam (up from 3)
- !checkaccess returns your access if you don't supply a user
- Fixed the "fix" for rejoining the channel when you're banned
- Fixed a major problem (100% CPU usage/crash)
- Fixed a minor memory leak
- Game announcer uses a "better" command (/whois intead of /whereis) and added a check for BNET pub response ("is using Warcraft III The Frozen Throne in game")

- First stable version, ready for general usage - version changed to 1.00

Version 0.33 ( 15.08.2010. )

- Major optimisations
- Language.cfg creation if one not present already
- Fixed the annoying welcome message bug

Version 0.32 ( 12.03.2010. )

- Minimalistic terminal "GUI" using curses library
- Added pdcurses, edited makefile, ghost.vcproj and updated readme
- Removed (now) obsolete input thread
- Some bug fixes and visual improvements
- Added !join alias to !channel command
- Better Eurobattle.net quota handling (in case of a warning increase the delay for the next message)
- Removed pthreads

Version 0.31 ( 03.02.2009. )

- Removed libircclient from the project
- Updated version to 0.31
- Fixed a bug in !uptime caused by latest changes
- Updated SQLite
- Other minor changes
- Some console output changes
- Changed the process priority to High (from Above Normal)
- Fixed an error in language.cfg
- Updated the readme, fixed some typos
- Tidied the source a bit and fixed/removed some comments

Version 0.30 ( 14.12.2009. )

- Changed Anti-Spam to be stricter depending on number of people in the channel
- Added !rejoin command
- Fixed a few responses
- Fixed !saybnets
- Ban Chat will kick the chat users already in channel when connecting
- Fixed a bug in !setaccess when setting higher access to a user with 0 access
- Fixed a bug in clan creation
- Fixed a bug in !restart where zombie connections would be left
- Edited !topic so you can make a blank topic
- !saybnet now uses partial matching
- Added !status
- Specify the channel on user join, leave, emote and talk
- Return BNET errors on connecting (such as CD key banned, etc.)
- Minimum interval on !announce is 4 seconds
- Added ?trigger
- Fixed a bug in !lockdown
- More language.cfg support
- Fixed a bug in !ban
- Made ?trigger command case insensitive
- Made arguments like "on" and "off" in commands such as !games, !greet, !lockdown case insensitive

Version 0.29 ( 03.12.2009. )

- Minor tweaks to clan invitation responses and clan creation responses
- Fixed a bug with stdin
- Added RoC support (just leave the TFT key line empty in ccbot.cfg)
- Minor tweaks to Anti-Spam
- Improved CHAT client recognition
- Added !Chieftain command
- Fixed a fatal error caused when you just inputed a space into stdin
- Added timestamps to console output
- Updated SQLite from 3.6.16 to 3.6.21
- Minor changes to the Makefile
- Updated ms_stdint.h
- Fixed an issue with CTRL + D (eof) in stdin input
- Overall minor fixes, optimisations etc.

Version 0.28 ( 20.11.2009. )

- Refined some parts of the code
- Removed arguments when running ccbot since .cfg file placement is fixed
- Print a few messages when creating a new ccbot.cfg so the user doesn't forget to edit it and restart the bot
- Renamed ccbot Linux binary to ccbot++
- Added input via stdin for Linux and Windows ( OS X untested )
- Fixed a bug where for !chanlist the bot needed to be in a clan
- !saybnet now uses partial matching
- Fixed a flaw in logic in !remove
- !accept works only if there is an active invitation
- Few bugs fixed in !lockdown and !kick
- Added Anti-spam
- Minor code changes and tweaks

Version 0.27 ( 13.11.2009. )

- Almost every config value is now per server
- Moved ghost.cfg and language.cfg to the cfg folder
- Increased timings on clan refreshing, autorejoining channel and detecting disconnects
- Fixed and removed some obsolete code and namings
- Made !access, !clanlist, !online and ban on join - instant (overriding queue)
- Started to make the bot more customizable in messages - still no visible effect tho
- Updated squelch related commands, fixed a lot of bugs in them
- Changed some default command access
- Chieftains and Shamans cannot be !removed from clan now
- Rewrote game announcing
- Rewrote !setaccess, !setcommand
- Instead of just "storm.dll" the bot searches for "Storm.dll" too - important only on Linux systems
- Fixed the timing issue on sending "accept"s or "decline"s to clan invitations (when creating a clan and normal invitations)
- Swear kicking now kicks users who aren't safelisted and have access lower then 5
- Added !saybnets, !saybnet
- !reload now reloads all CFGs (but only easily reloadable variables, for most of them a full CCBot restart is needed)
- Much renaming in the source and filenames
- Create a new ccbot.cfg with the default values if a existing one isn't found

Version 0.26 ( 10.11.2009. )

- Various minor optimisations and fixes and aesthetics changes
- Serious bug fixed when searching for a user
- Every clan command is now disabled when bot is not in a clan to prevent bad situations
- Added !test for testing purposes - safe to use but no real value in every day usage
- Removed StormLib and zlib (and libbz2-dev on Linux) dependencies
- Edited readme.txt, makefile and ghost.vcproj
- Updated SQLite to SQLite 3.6.16
- Increase priority on Windows to "Above Normal"
- Added !restart for Windows only, DO NOT CHANGE NAME ghost.exe TO ANYTHING ELSE
- Renamed swears.txt to swears.cfg
- If no swears.cfg file found - make a new one with the default info in it
- Made the bot Apple's OS X compliant - but no OS X PCs to test if it can compile there
- Finished !access
- Optimised channel users information saving, accessing, sorting and their related commands/procedures/functions
- Changed !lockdown - tempBans if the user joining has a lower access then set
- Bot recognizes clan creation invites and automatically accepts them (this means you can make a clan with 1 War3 client nad 9 accounts on 1 bot)

Version 0.25 ( 09.11.2009. )

- Major changes to the channel users information saving, accessing, sorting and their related commands/procedures/functions
- Added !ping
- Major optimisation of default command names and command access structure
- Modified ghost.vcproj for VS++, Windows users
- Forced (bot's) clan rank checking in commands like !grunt, !peon, !shaman
- Removed bot's own name in !online command output
- Fixed a bug in partial name matching
- Fixed !getclan returning the wrong number of clan members
- Optimisation to many clan commands
- Removed !channelinfo
- Rewrote some packet processing protocols (SID_REMOVEMEMBER and SID_CLANINVITATION)
- Fixed a issue with !online when used on multiple realms
- Fixed a minor bug with !lockdown and storing banned users
- Disabled Nagle's algorithm - currently can't unset it in ghost.cfg
- Fixed a bug in !kick
- Fixed some bugs when connecting two accounts to the same battle.net/PvPGN (this isn't recommended)
- Send a clan invitation decline if we don't accept it within 29 seconds
- If not logged in don't send refresh clanlist or rejoin if in "wrong" channel
- Major optimisation to BYTEARRAY appending and creation

Version 0.24 ( 08.11.2009. )

- Major work on access system - old admin system completely removed and everything related
- Access settings stored in database table "commands"
- New table but same schema number (4) - which could cause problems to newer databses but it's easy to fix
- All commands are given default access values if there are no entries for them
- Added !command, !setcommand, !access, !accept
- Fixed a major bug and a couple of minor ones
- Added a server alias so that server.eurobattle.net shows as Eurobattle.net on the console messages
- Added a config option bot_banchat - when set to 1 it kicks telnet and chat clients
- Added protocols for receiving clan invitations and accepting received clan invitations
- Message is sent to the channel when receiving a clan invitation
- Using [WHISPER] tag on console print on sent whispers
- Fixed a few missing and misspelled default values in the access system
- Fixed a variable error used in ghost.cfg
- Lines in swear.txt are ignored if on the 1st position, start, of the line is a # character
- Moved all of the ProcessChatEvent to chatevent.cpp - to reduce time of recompiling and easier manipulation
- Edited the Makefile and ghost.vcproj files
- Bot tested on FAWKZBNET - added alias
- Made country abbreviation and name RU and Russia respectively - just for gags
- Rank check on !join and !invite - bot MUST be a shaman or chieftain
