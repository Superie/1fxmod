// Copyright (C) 2001-2002 Raven Software
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "boe_local.h"

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

char	*ConcatArgs( int start );

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			Com_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	m[0] = m[1] = m[2] = m[3] = 0;

	i = 0;
	p = from;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
		{
			break;		// free spot
		}
	}

	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			Com_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
	{
		ipFilters[i].compare = 0xffffffffu;
	}

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		Com_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		Com_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			Com_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	Com_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		Com_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			Com_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			Com_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			Com_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			Com_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			Com_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			Com_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			Com_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			Com_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			Com_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			Com_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			Com_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			Com_Printf("ET_GRAPPLE          ");
			break;
		default:
			Com_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			Com_Printf("%s", check->classname);
		}
		Com_Printf("\n");
	}
}


void Svcmd_ExtendTime_f (void) 
{
	char str[MAX_TOKEN_CHARS];
	int	 time;

	if ( trap_Argc() < 2 ) 
	{
		Com_Printf("Usage:  extendtime <minutes>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	time = atoi(str);
	level.timeExtension += time;

	G_LogPrintf ( "timelimit extended by %d minutes\n", time );

	trap_SendServerCommand( -1, va("print \"timelimit extended by %d minutes\n\"", time) );
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			Com_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	Com_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void Svcmd_ForceTeam_f( void ) 
{
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) 
	{
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str, NULL );
}

/*
===================
Svcmd_CancelVote_f

cancels the vote in progress
===================
*/
void Svcmd_CancelVote_f ( void )
{
	level.voteTime = 0;

	trap_SetConfigstring( CS_VOTE_TIME, "" );	

	trap_SendServerCommand( -1, "print \"Vote cancelled by admin.\n\"" );
}

/*
=================
ConsoleCommand
=================
*/
qboolean ConsoleCommand( void ) 
{
	char cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) 
	{
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) 
	{
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "cancelvote" ) == 0 )
	{
		Svcmd_CancelVote_f();
		return qtrue;
	}

#ifdef _SOF2_BOTS

	if (Q_stricmp (cmd, "addbot") == 0) 
	{
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) 
	{
		Svcmd_BotList_f();
		return qtrue;
	}

#endif

	if (Q_stricmp (cmd, "addip") == 0) 
	{
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) 
	{
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) 
	{
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if (Q_stricmp (cmd, "gametype_restart" ) == 0 )
	{
		G_ResetGametype ( );
		return qtrue;
	}

	if (Q_stricmp (cmd, "extendtime" ) == 0 )
	{
		Svcmd_ExtendTime_f();
		return qtrue;
	}
	// Boe!Man 3/30/10
	if (Q_stricmp (cmd, "addbadmin" ) == 0 )
	{
		Boe_Add_bAdmin_f(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "addadmin" ) == 0 )
	{
		Boe_Add_Admin_f(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "addsadmin" ) == 0 )
	{
		Boe_Add_sAdmin_f(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "addclan" ) == 0 )
	{
		Boe_Add_Clan_Member(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "removeclan" ) == 0 )
	{
		Boe_Remove_Clan_Member(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "clanvsall" ) == 0 )
	{
		RPM_Clan_Vs_All(NULL);
		return qtrue;
	}
	if (Q_stricmp (cmd, "ban" ) == 0 )
	{
		Boe_Ban_f(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "runover" ) == 0 )
	{
		Boe_Runover(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "twist" ) == 0 )
	{
		Boe_Twist(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "untwist" ) == 0 )
	{
		Boe_unTwist(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "respawn" ) == 0 )
	{
		Boe_Respawn(1, NULL, qfalse);
		return qtrue;
	}
	if ((Q_stricmp (cmd, "uppercut" ) == 0 )  || (Q_stricmp (cmd, "uc" ) == 0 ))
	{
		Boe_Uppercut(1, NULL, qfalse);
		return qtrue;
	}
	if ((Q_stricmp (cmd, "removeadmin" ) == 0))
	{
		Boe_Remove_Admin_f(1, NULL, qfalse);
		return qtrue;
	}
	if ((Q_stricmp (cmd, "flash" ) == 0))
	{
		Boe_Flash(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "plant" ) == 0 )
	{
		Boe_Plant(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "unplant" ) == 0 )
	{
		Boe_unPlant(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "pop" ) == 0 )
	{
		Boe_pop(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "burn" ) == 0 )
	{
		Boe_Burn(1, NULL, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "mute" ) == 0 )
	{
		Boe_Mute(1, NULL, qtrue, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "unmute" ) == 0 )
	{
		Boe_Mute(1, NULL, qfalse, qfalse);
		return qtrue;
	}
	if (Q_stricmp (cmd, "strip" ) == 0 )
	{
		Boe_Strip(1, NULL, qfalse);
		return qtrue;
	}
	// Henk 28/01/10 -> Add eventeams for rcon
	if ((Q_stricmp (cmd, "eventeams" ) == 0 ))
	{
		EvenTeams(NULL);
		return qtrue;
	}
	if ((Q_stricmp (cmd, "swapteams" ) == 0 ))
	{
		Boe_SwapTeams(NULL);
		return qtrue;
	}

	if ((Q_stricmp (cmd, "lock" ) == 0 ))
	{
		RPM_lockTeam(NULL, qtrue, ConcatArgs(1) );
		return qtrue;
	}
	if (g_dedicated.integer) 
	{
		if (Q_stricmp (cmd, "say") == 0) 
		{
			trap_SendServerCommand( -1, va("chat -1 \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}

		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("chat -1 \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

