/*
===========================================================================
Copyright (C) 2004 - 2005, Ryan Powell (RPM)
Copyright (C) 2006 - 2007, GodOrDevil
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "g_local.h"
#include "boe_local.h"

/*
=============
G_ref_cmd
=============
*/
void G_ref_cmd( gentity_t *ent)
{
    char    arg1[MAX_STRING_TOKENS];
    char    arg2[MAX_STRING_TOKENS];

    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
    {
        trap_SendServerCommand( ent-g_entities, va("print \" \n ^3%-5s %-16s %-13s Explanation\n\"", "Lvl", "Commands", "Arguments"));
        trap_SendServerCommand( ent-g_entities, "print \" ----------------------------------------------------------\n\"");
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-7s %-26s ^7[^3Enter the Referee password^7]\n\"", "]", "password"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Info of the specified team^7]\n\"", "]", "i", "info", "<team>"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-26s ^7[^3Ready up the teams^7]\n\"", "]", "rt", "ready"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Lock a team^7]\n\"", "]", "l", "lock", "<team>"));
        // Boe: Hieronder mot nog.. Updated soonish.
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Reset a team their settings^7]\n\"", "]", "re", "reset", "<team>"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Kick a player^7]\n\"", "]", "k", "kick", "<id> <reason>"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Sends a message to Referees^7]\n\"", "]", "rch", "chat", "<message>"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-12s %-13s ^7[^3Sends a message to all players^7]\n\"", "]", "rt", "reftalk", "<message>"));
        trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7%-3s %-3s %-26s ^7[^3Pauses/resumes the game^7]\n\"", "]", "pa", "pause"));
        return;
    }

    if (!g_compMode.integer) {
        G_printInfoMessage(ent,
            "Server must be in Competetion Mode to become a Referee.");
        return;
    }


    if (!Q_stricmp ( arg1, "password" ))
    {
        if(ent->client->sess.referee)
        {
            G_printInfoMessage(ent, "You are already a Referee.");
            return;
        }

        if(ent->client->sess.admin)
        {
            G_printInfoMessage(ent, "You are already an Admin.");
            return;
        }

        if(!Q_stricmp ( g_refpassword.string, "none" )
            || !Q_stricmp ( g_refpassword.string, "\0" ))
        {
            G_printInfoMessage(ent,
                "This server does not have a Referee password.");

            return;
        }
        if(!Q_stricmp ( g_refpassword.string, arg2 ))
        {
            ent->client->sess.referee = 1;
            G_Broadcast(
                va("%s\nis now a \\Referee", ent->client->pers.netname),
                BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1,
                va("print\"^3[Referee Action] ^7%s is now a Referee.\n\"",
                ent->client->pers.cleanName));
            return;
        }
        else
        {
            G_printInfoMessage(ent, "Incorrect Referee password entered.");
            return;
        }
    }
}
