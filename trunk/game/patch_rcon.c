// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_rcon.c - Memory patch that enables logging from RCON.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void Patch_rconLogging(const char *message, char *ip, char *command);

/*
==================
Patch_rconLogging

The actual code of where we check what command the client tries to execute.
==================
*/

void Patch_rconLogging(const char *message, char *ip, char *command)
{
    char        ip2[MAX_IP];
    char        *p;
    qboolean    validAttempt = qtrue;

    // Check if the attempt was valid.
    if (strstr(message, "Bad rcon from")){
        validAttempt = qfalse;
    }

    if (validAttempt && g_enableRconLog.integer){
        p = strstr(ip, ":");
        strncpy(ip2, ip, (int)(p - ip));

        G_LogRcon("%-40sby %s", command, ip);
    }

    Com_Printf(message, ip, command);
}

/*
==================
Patch_rconLog

Writes the jump to the detour.
==================
*/

void Patch_rconLog()
{
    #ifdef _WIN32
    #ifdef _GOLD
    Patch_detourAddress("RCON log patch", (long)&Patch_rconLogging, 0x0047908a, qfalse);
    #elif _DEMO
    Patch_detourAddress("RCON log patch", (long)&Patch_rconLogging, 0x004743f4, qfalse);
    #else
    Patch_detourAddress("RCON log patch", (long)&Patch_rconLogging, 0x0047666a, qfalse);
    #endif // _GOLD, _DEMO or Full
    #endif // _WIN32

    #ifdef __linux__
    #ifdef _GOLD
    Patch_detourAddress("RCON log patch (1/2)", (long)&Patch_rconLogging, 0x0805876b, qfalse);
    Patch_detourAddress("RCON log patch (2/2)", (long)&Patch_rconLogging, 0x08058719, qfalse);
    #else
    Patch_detourAddress("RCON log patch (1/2)", (long)&Patch_rconLogging, 0x08054def, qfalse);
    Patch_detourAddress("RCON log patch (2/2)", (long)&Patch_rconLogging, 0x08054d9d, qfalse);
    #endif // _GOLD
    #endif // __linux__
}
