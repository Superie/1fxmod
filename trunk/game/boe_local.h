// Copyright (C) 2010 - Boe!Man.
//
// boe_local.h - Local definitions from boe_*.c files are stored here.

//==================================================================

// Boe!Man 3/30/10
#ifdef Q3_VM
#define INF_VERSION_STRING "0.56t"
#else
#define INF_VERSION_STRING "0.56t-dll"
#endif
#define INF_STRING "1fx. Mod"
#define INF_VERSION_STRING_COLORED "^71fx^1. ^3Mod"
#define INF_VERSION_DATE "1/14/11"
#define TEST_VERSION "Developed by ^GBoe!Man ^7& ^6Henkie\n^1Running a Test version of the Mod\n\n"
#define STABLE_VERSION "Developed by ^GBoe!Man ^7& ^6Henkie\n1fx.uk.to ^3| ^7i3D.net\n\n"

// 1fx. Mod
// Developed by: Boe!Man & Henkie
// Codename: BoeMODV2
// Modname: 1fx. Mod (Originally: 1fx. Infiltration)
// Project started: 3/30/10
//
// --- VERSION LOG BELOW
//
// Version		Status of Release				Date
// ----------------------------------------------------------------
// 0.01			Alpha							3/30/10
// 0.02			Alpha							3/31/10
// 0.05			Beta							4/4/10
// 0.06			Beta							4/6/10
// 0.10			Beta							4/7/10
// 0.11a		1st Beta						4/8/10
// 0.11b		2nd Beta						4/8/10
// 0.15a		Release Candidate				4/12/10
// 0.15b		Release							4/14/10
// 0.16			Beta							4/15/10
// 0.17			Beta							4/16/10
// 0.18			Beta							4/17/10
// 0.19			Beta							4/18/10
// 0.20			Release							4/20/10
// 0.21			Release							4/22/10
// 0.22			Release							4/23/10
// 0.25a		Release							5/3/10
// 0.25b		Release							5/3/10
// 0.30t		Beta							5/7/10
// 0.30			Release							6/2/10
// 0.31			Release							6/3/10
// 0.40t		Beta							6/11/10
// 0.40t		Release Candidate				7/24/10
// 0.40			Release							8/2/10
// 0.41t		Beta							8/22/10
// 0.41			Release							8/25/10
// 0.42t		Beta							9/1/10
// 0.42			Release							9/3/10
// 0.50t		Beta							9/9/10
// 0.50			Release							9/17/10
// 0.51t		Alpha							9/18/10
// 0.51t		Beta							9/19/10
// 0.51t		1st Release Candidate			9/26/10
// 0.51t		2nd Release Candidate			9/30/10
// 0.51			Release							10/7/10
// 0.52t		Beta							10/16/10
// 0.52			Release							10/25/10
// 0.53			Release							11/2/10
// 0.54t		Beta							11/21/10
// 0.54t		Release Candidate				12/13/10
// 0.54			Release							12/19/10
// 0.55t		Beta							12/23/10
// 0.55			Release							12/31/10
// 0.56t		Alpha							1/1/11
// 0.56t		1st Beta						1/8/11
// 0.56t		2nd Beta						1/14/11

//===================================================================

/*
Boe_log CVAR.
-------------
This CVAR is used for the debug of several functions, 
writing a simple string to the games.log file, so the we can notice
where the bug would be. This CVAR gets used often in new or updated
functions and enabled at release, or when frequent crashes appear.
Note you can select multiple options by simply putting multiple
options in the CVAR itself (as it actually searches for the option
within the string, example: you want to watch option 1 and
3, so you update the CVAR to either '13' or '31').

You have the option to NOT compile this function , so in final 
releases this CVAR can be left out. To exclude this function,
simply leave out (-D_BOE_DBG).

-------------
For '1' in the boe_log CVAR:
1 = RPM Awards.
2 = Auto restart (Kill server).
3 = Server messages.
4 = Remove Admin.
5 = Auto even teams.
6 = ClientUserInfoChanged.
7 = ClientConnect.
8 = ClientBegin.
9 = player_die.

For '2' in the boe_log CVAR:
NOTE: This log is primarily based around compMode.
1 = RPM_Tcmd.
2 = RPM_TeamInfo.
3 = G_InitGame (g_compMode check).
4 = ExitLevel.
5 = CheckExitRules.
6 = G_ResetGametype (RT_NONE check).
7 = Boe_calcMatchScores.
8 = Boe_About.

*/

//==============================================
// boe_admcmds.c
//==============================================

// Boe!Man 3/30/10
void	Boe_adm_f ( gentity_t *ent );
int		Boe_ClientNumFromArg (gentity_t *ent, int argNum, const char* usage, const char* type, qboolean aliveOnly, qboolean otheradmins, qboolean shortCmd);

extern	vmCvar_t	g_kick;
//extern	admCmd_t AdminCommands[124];

extern  vmCvar_t	g_adminfile;
extern	vmCvar_t	g_addbadmin;
extern	vmCvar_t	g_addadmin;
extern	vmCvar_t	g_addsadmin;

void	Boe_FileError (gentity_t * ent, const char *file);
void	Boe_id (int);
int		Boe_AddToList(const char *string, const char* file, const char* type, gentity_t* ent);
int		Boe_Remove_from_list( char *key, const char *file, const char* type, gentity_t *ent, qboolean unban, qboolean removeAdmin, qboolean scoreCheck);
void	Boe_Unban(gentity_t *adm, char *ip, qboolean subnet);

void	Boe_subnetBan(int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Remove_Admin_f(int argNum, gentity_t *adm, qboolean shortCmd);

extern	vmCvar_t	g_ban;
extern	vmCvar_t	g_subnetban;
extern	vmCvar_t	g_subnetbanlist;
extern	vmCvar_t	g_banlist;
extern	vmCvar_t	g_banlist1;
extern	vmCvar_t	g_removeadmin;
extern	vmCvar_t	g_broadcast;

extern	vmCvar_t	server_color1;
extern	vmCvar_t	server_color2;
extern	vmCvar_t	server_color3;
extern	vmCvar_t	server_color4;
extern	vmCvar_t	server_color5;
extern	vmCvar_t	server_color6;

void	Boe_Add_bAdmin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_Add_Admin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_Add_sAdmin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_BanList(int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Ban_f (int argNum, gentity_t *adm, qboolean shortCmd);				// Prototype warnings.
void	Boe_Uppercut (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_uppercut;

void	Boe_Twist (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_unTwist (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_twist;
void	Boe_Respawn (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_respawn;
extern	vmCvar_t	g_runover;
void	Boe_Runover (int argNum, gentity_t *adm, qboolean shortCmd);

void	G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback );	// Prototype warnings.

extern	vmCvar_t	g_maprestart;

void	Boe_Flash(int argNum, gentity_t *ent, qboolean shortCmd);
extern	vmCvar_t	g_flash;

void	Boe_unPlant (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Plant (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_plant;

void	Boe_pop (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Burn (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Broadcast(int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Mute (int argNum, gentity_t *adm, qboolean mute, qboolean shortCmd);
void	Boe_Strip (int argNum, gentity_t *adm, qboolean shortCmd);
void	Adm_ForceTeam(int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_pop;
extern	vmCvar_t	g_burn;
extern	vmCvar_t	g_mute;
extern	vmCvar_t	g_strip;
extern	vmCvar_t	g_eventeams;

//extern	vmCvar_t	g_333;

extern	vmCvar_t	g_forceteam;

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 );
gentity_t *NV_projectile (gentity_t *ent, vec3_t start, vec3_t dir, int weapon, int damage);
extern	vmCvar_t	g_maxIPConnections;

// Boe!Man 4/1/10
extern	vmCvar_t	server_redteamprefix;
extern	vmCvar_t	server_blueteamprefix;

// Boe!Man 4/5/10
// EMPTY

// Boe!Man 4/15/10
extern	vmCvar_t	g_nolower;
extern	vmCvar_t	g_nades;
extern	vmCvar_t	g_sl;
extern	vmCvar_t	g_tl;
extern	vmCvar_t	g_ri;
extern	vmCvar_t	g_respawninterval;
extern	vmCvar_t	g_damage;
extern	vmCvar_t	g_gr;
extern	vmCvar_t	g_clanvsall;
extern	vmCvar_t	g_swapteams;
extern	vmCvar_t	g_lock;
extern	vmCvar_t	g_clan;

// Boe!Man 12/7/10
void Boe_SubnetBanlist (int argNum, gentity_t *adm, qboolean shortCmd);

//==============================================
// boe_utils.c
//==============================================

// Boe!Man 3/30/10
extern  vmCvar_t	server_motd1;
extern  vmCvar_t	server_motd2;
extern  vmCvar_t	server_motd3;
extern  vmCvar_t	server_motd4;
extern  vmCvar_t	server_motd5;
extern  vmCvar_t	server_motd6;

char	*Boe_StatColor(int stat);
int		Boe_FindClosestTeammate(gentity_t *ent, qboolean bot);
int		Boe_FindClosestEnemy(gentity_t *ent, qboolean bot);
void	Boe_ParseChatSounds (void);
void	Boe_Tokens(gentity_t *ent, char *chatText, int mode, qboolean CheckSounds);

extern	chatSounds_t chatSounds[MAX_BOE_CHATS];

void	Boe_ClientSound (gentity_t *ent, int soundIndex);
void	Boe_GlobalSound (int soundIndex);

void	Boe_Display_Sounds( gentity_t *ent );
void	Boe_Display_Sounds2( gentity_t *ent );
void	Boe_Display_ExtraSounds (gentity_t *ent);

extern	vmCvar_t	server_color1;
extern	vmCvar_t	server_color2;
extern	vmCvar_t	server_color3;
extern	vmCvar_t	server_color4;
extern	vmCvar_t	server_color5;
extern	vmCvar_t	server_color6;

extern  int			Preloaded;

void	Boe_GlobalSound (int soundIndex);

void	Boe_Motd (gentity_t *ent);

void	HENK_COUNTRY(gentity_t *ent);
qboolean CheckIP(gentity_t *ent);

void	Boe_Players (gentity_t *ent);

void	Boe_Print_File (gentity_t *ent, char *file, qboolean clonecheckstats, int idnum);

void	Boe_Stats( gentity_t *ent );

void	QDECL Boe_adminLog	(const char *text, ... );

// Boe!Man 4/2/10: Crash log.
void	QDECL Boe_crashLog	(const char *text);

// Henk 28/01/10 -> Add eventeams
void	EvenTeams (gentity_t *adm, qboolean aet);

void	Boe_SwapTeams(gentity_t *adm);

extern	vmCvar_t	server_badminprefix;
extern	vmCvar_t	server_adminprefix;
extern	vmCvar_t	server_sadminprefix;
extern	vmCvar_t	server_acprefix;
extern	vmCvar_t	server_caprefix;
extern	vmCvar_t	server_ccprefix;
extern	vmCvar_t	server_starprefix;

extern	vmCvar_t	Owner;
extern	vmCvar_t	Clan;
extern	vmCvar_t	ClanURL;

// Henk 19/04/10 -> Fix all prototypes warning(Yes im bored)
int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck, qboolean cloneCheck);
void G_UpdateOutfitting ( int clientNum );
void G_DropGametypeItems ( gentity_t* self, int delayPickup );
int trap_RealTime( qtime_t *qtime );
qboolean BG_InitWeaponStats(void);
void Boe_About( gentity_t *ent );
void Boe_dev_f ( gentity_t *ent );
void Boe_Add_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd);
void Boe_Remove_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd);
void RPM_Clan_Vs_All(gentity_t *adm);
void Preload(void);

extern  vmCvar_t	g_disablelower;

char *GetReason(void);

// Boe!Man 5/9/10
char	*Boe_BarStat(int stat);
void	Boe_Kick(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_XMute(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_UnMute(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_MapRestart(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NoLower(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NoNades(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_TimeLimit(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_ScoreLimit(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_RespawnInterval(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_RealDamage(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NormalDamage(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_GametypeRestart(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_Third(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_CompMode(int argNum, gentity_t *ent, qboolean ShortCmd);
void	RemoveFence(void);
void	SpawnFence(int arg);
qboolean henk_isdigit(char c);
qboolean henk_ischar(char c);
void RPM_Awards(void);
void G_AdjustClientBBox(gentity_t *other);
void G_AdjustClientBBoxs(void);
void G_UndoAdjustedClientBBox(gentity_t *other);
void G_UndoAdjustedClientBBoxs(void);
void G_SetClientPreLeaningBBox(gentity_t *ent);
void G_SetClientLeaningBBox(gentity_t *ent);

#define BBOX_LEAN_OFFSET	0.65f
#define LB_MAXZ				-3
#define LB_MINZ				54
#define DUCKED_LB_MINZ		25
#define PLB_MAXZ			-39

// Boe!Man 6/2/10
extern	vmCvar_t	g_autoeventeams;
extern	vmCvar_t	server_msgInterval;
extern	vmCvar_t	server_enableServerMsgs;
extern	vmCvar_t	server_message1;
extern	vmCvar_t	server_message2;
extern	vmCvar_t	server_message3;
extern	vmCvar_t	server_message4;
extern	vmCvar_t	server_message5;
extern	vmCvar_t	server_msgDelay;

// Boe!Man 10/16/10
extern	vmCvar_t	g_adminspec;

// Boe!Man 11/16/10: Default scrim settings.
extern	vmCvar_t	g_autoLockSpec;
extern	vmCvar_t	g_autoSwapTeams;
extern	vmCvar_t	g_matchTimeLimit;
extern	vmCvar_t	g_matchScoreLimit;
extern	vmCvar_t	g_matchDisableSounds;

extern vmCvar_t	cm_enabled;
extern vmCvar_t	cm_sl;
extern vmCvar_t	cm_tl;
extern vmCvar_t	cm_slock;
extern vmCvar_t	cm_aswap;
extern vmCvar_t	cm_dsounds;
extern vmCvar_t	cm_oldsl;
extern vmCvar_t	cm_oldtl;
extern vmCvar_t	cm_sr;
extern vmCvar_t	cm_sb;

// Boe!Man 11/18/10
void Boe_calcMatchScores (void);

// Boe!Man 12/13/10
extern vmCvar_t	g_aliasCheck;

// Boe!Man 1/8/10
void Boe_serverMsg (void);

// 1/14/10: Custom commands by Henk
void	SpawnCage(vec3_t org, gentity_t *ent );
// Henk 23/01/10 -> Add BoxNade and StripHiders
void	SpawnBox(vec3_t org);
void	StripHiders(void);
void	SetupOutfitting(void);
// Henk 19/02/10 -> Add Flares
void	Effect(vec3_t org, char *name, qboolean rpg);

// Henk 20/02/10 -> Add UpdateScores()
void	UpdateScores(void);
void	ShowScores(void);
int		Henk_GetScore(void);

char	*vtocs( const vec3_t v );

// Henk 14/01/11 -> Add EvenTeams_HS and all other shit
void EvenTeams_HS(gentity_t *adm, qboolean aet);
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
gentity_t* G_RealSpawnGametypeItem ( gitem_t* item, vec3_t origin, vec3_t angles, qboolean dropped );
void RandomRPGM4(void);
void G_RespawnClients ( qboolean force, team_t team, qboolean fullRestart );
