// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_autodl.c - "dirtrav" auto download exploit memory patch.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void Patch_autoDownloadDetour   (char *message, int *clientNum, char *mapString);

/*
==================
Patch_autoDownloadDetour

The actual code of where we check what file the client tries to download.
==================
*/

void Patch_autoDownloadDetour(char *message, int *clientNum, char *mapString)
{
    if (!strstr(mapString, ".pk3")){
        G_LogPrintf("Auto D/L protection: ^1HACK ATTEMPT: ^7client %d tries to download \"%s\"\n", clientNum, mapString);
        *mapString = '\0';
    }
    #ifdef _DEBUG
    else{
        Com_Printf("Auto D/L: Allowed client %d to download \"%s\"\n", clientNum, mapString);
    }
    #endif
}

/*
==================
Patch_autoDownloadExploit

Writes the jump to the detour.
==================
*/

void Patch_autoDownloadExploit()
{
    #ifdef _WIN32
    #ifdef _GOLD
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x00471552, qfalse);
    #else
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x0046EF20, qfalse);
    #endif // _GOLD
    #elif __linux__
    #ifdef _GOLD
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x0804d9f1, qfalse);
    #else
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x0804b185, qfalse);
    #endif // _GOLD
    #endif // _WIN32
}
