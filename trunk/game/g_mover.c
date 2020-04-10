/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
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
===============================================================================

PUSHMOVE

===============================================================================
*/

void MatchTeam( gentity_t *teamLeader, int moverState, int time );

typedef struct {
    gentity_t   *ent;
    vec3_t  origin;
    vec3_t  angles;
    float   deltayaw;
} pushed_t;
pushed_t    pushed[MAX_GENTITIES], *pushed_p;


void ROT_SetMoverState( gentity_t *ent, moverState_t moverState, int time )
{
    vec3_t          delta;
    float           f;

    ent->s.apos.trTime = time;

    switch( moverState )
    {
    case MOVER_POS1:
        VectorCopy( ent->apos1, ent->s.apos.trBase );
        ent->s.apos.trType = TR_STATIONARY;
        break;

    case MOVER_POS2:
        VectorCopy( ent->apos2, ent->s.apos.trBase );
        ent->s.apos.trType = TR_STATIONARY;
        break;

    case MOVER_1TO2:
        VectorCopy( ent->apos1, ent->s.apos.trBase );
        VectorSubtract( ent->apos2, ent->apos1, delta );
        f = 1000.0 / ent->s.apos.trDuration;
        VectorScale( delta, f, ent->s.apos.trDelta );
        ent->s.apos.trType = TR_LINEAR_STOP;
        break;

    case MOVER_2TO1:
        VectorCopy( ent->apos2, ent->s.apos.trBase );
        VectorSubtract( ent->apos1, ent->apos2, delta );
        f = 1000.0 / ent->s.apos.trDuration;
        VectorScale( delta, f, ent->s.apos.trDelta );
        ent->s.apos.trType = TR_LINEAR_STOP;
        break;
    }

    BG_EvaluateTrajectory(&ent->s.apos, level.time, ent->r.currentAngles);
}

void Touch_2WayRotDoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace )
{
    vec3_t dir, angles;
    vec3_t forward, right;
    float dot;

    if ((ent->parent->moverState != MOVER_POS1) && (ent->parent->moverState != MOVER_POS2))
        return;
    if (!other->client)
        return;
    if (ent->parent->spawnflags & 16 && ent->parent->spawnflags != 604)
        return;
    // figure out which way the door should open
    VectorSubtract(ent->s.origin, other->client->ps.origin, forward);
    VectorNormalize(forward);

    // make sure we're facing the door
    AngleVectors(other->client->ps.viewangles, dir, NULL, NULL);
    //dot = DotProduct(dir, forward);
    //if (dot < 0.3)
    //  return;

    // LOCKED DOOR
    //if (ent->parent->spawnflags & 16)
    //  return;

    // if already open, just return to default position
    if ((ent->parent->moverState == MOVER_POS2)) {
        Use_BinaryMover(ent->parent, ent, other);
        return;
    }

    VectorSubtract(ent->parent->s.origin, ent->s.origin, dir);
    VectorNormalize(dir);
    vectoangles(dir, angles);
    AngleVectors(angles, NULL, right, NULL);

    // figure out we're roughly facing with or against it
    dot = DotProduct(forward, right);

    if (dot > 0.3) {
        VectorAdd(ent->parent->apos1, ent->apos2, ent->parent->apos2);
        Use_BinaryMover(ent->parent, ent, other);
    }
    else if (dot < -0.3) {
        VectorSubtract(ent->parent->apos1, ent->apos2, ent->parent->apos2);
        Use_BinaryMover(ent->parent, ent, other);
    }
}


void Think_Spawn2WayRotDoorTrigger(gentity_t *ent)
{
    gentity_t *other;
    vec3_t mins, maxs;
    int i, best;

    // find the bounds of everything on the team
    VectorCopy(ent->r.absmin, mins);
    VectorCopy(ent->r.absmax, maxs);

    for (other = ent->teamchain; other; other=other->teamchain){
        AddPointToBounds (other->r.absmin, mins, maxs);
        AddPointToBounds (other->r.absmax, mins, maxs);
    }

    // find the thinnest axis, which will be the one we expand
    best = 0;
    for (i = 1 ; i < 3 ; i++){
        if (maxs[i] - mins[i] < maxs[best] - mins[best])
            best = i;
    }

    maxs[best] += 120;
    mins[best] -= 120;

    // create a trigger with this size
    other = G_Spawn();
    other->classname = "door_trigger";
    VectorCopy (mins, other->r.mins);
    VectorCopy (maxs, other->r.maxs);
    other->parent = ent;
    other->r.contents = CONTENTS_TRIGGER;
    other->touch = Touch_2WayRotDoorTrigger;
    VectorCopy(ent->s.angles, other->apos2);

    // figure out the real origin of the door
    VectorAdd(mins, maxs, other->s.origin);
    VectorScale(other->s.origin, 0.5, other->s.origin);

    trap_LinkEntity (other);

    // remember the thinnest axis
    other->count = best;

    trap_LinkEntity (other);
    MatchTeam( ent, ent->moverState, level.time );
}

void Blocked_RotDoor( gentity_t *ent, gentity_t *other )
{
    // remove anything other than a client
    if (!other->client)
    {
        // except mission items
        if (other->s.eType == ET_ITEM && other->item->giType == IT_GAMETYPE)
            return;
        G_TempEntity(other->s.origin, EV_ITEM_POP);
        G_FreeEntity(other);
        return;
    }
    Use_BinaryMover(ent, ent, other);
}

/*
============
G_TestEntityPosition

============
*/
gentity_t   *G_TestEntityPosition( gentity_t *ent ) {
    trace_t tr;
    int     mask;

    if ( ent->clipmask ) {
        mask = ent->clipmask;
    } else {
        mask = MASK_SOLID;
    }
    if ( ent->client ) {
        trap_Trace( &tr, ent->client->ps.origin, ent->r.mins, ent->r.maxs, ent->client->ps.origin, ent->s.number, mask );
    } else {
        trap_Trace( &tr, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, ent->s.pos.trBase, ent->s.number, mask );
    }

    if (tr.startsolid)
        return &g_entities[ tr.entityNum ];

    return NULL;
}

/*
================
G_CreateRotationMatrix
================
*/
void G_CreateRotationMatrix(vec3_t angles, vec3_t matrix[3]) {
    AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
    VectorInverse(matrix[1]);
}

/*
================
G_TransposeMatrix
================
*/
void G_TransposeMatrix(vec3_t matrix[3], vec3_t transpose[3]) {
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            transpose[i][j] = matrix[j][i];
        }
    }
}

/*
================
G_RotatePoint
================
*/
void G_RotatePoint(vec3_t point, vec3_t matrix[3]) {
    vec3_t tvec;

    VectorCopy(point, tvec);
    point[0] = DotProduct(matrix[0], tvec);
    point[1] = DotProduct(matrix[1], tvec);
    point[2] = DotProduct(matrix[2], tvec);
}

/*
==================
G_TryPushingEntity

Returns qfalse if the move is blocked
==================
*/
qboolean G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove )
{
    vec3_t      matrix[3], transpose[3];
    vec3_t      org, org2, move2;
    gentity_t   *block;

    // EF_MOVER_STOP will just stop when contacting another entity
    // instead of pushing it, but entities can still ride on top of it
    if ( ( pusher->s.eFlags & EF_MOVER_STOP ) && check->s.groundEntityNum != pusher->s.number )
    {
        return qfalse;
    }

    // save off the old position
    if (pushed_p > &pushed[MAX_GENTITIES])
    {
        Com_Error(ERR_FATAL, "pushed_p > &pushed[MAX_GENTITIES]");
    }

    pushed_p->ent = check;
    VectorCopy (check->s.pos.trBase, pushed_p->origin);
    VectorCopy (check->s.apos.trBase, pushed_p->angles);
    if ( check->client )
    {
        pushed_p->deltayaw = check->client->ps.delta_angles[YAW];
        VectorCopy (check->client->ps.origin, pushed_p->origin);
    }

    pushed_p++;

    // try moving the contacted entity
    // figure movement due to the pusher's amove
    G_CreateRotationMatrix( amove, transpose );
    G_TransposeMatrix( transpose, matrix );
    if ( check->client )
    {
        VectorSubtract (check->client->ps.origin, pusher->r.currentOrigin, org);
    }
    else
    {
        VectorSubtract (check->s.pos.trBase, pusher->r.currentOrigin, org);
    }

    VectorCopy( org, org2 );
    G_RotatePoint( org2, matrix );
    VectorSubtract (org2, org, move2);

    // add movement
    VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);
    VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);
    if ( check->client )
    {
        VectorAdd (check->client->ps.origin, move, check->client->ps.origin);
        VectorAdd (check->client->ps.origin, move2, check->client->ps.origin);
        // make sure the client's view rotates when on a rotating mover
        check->client->ps.delta_angles[YAW] += ANGLE2SHORT(amove[YAW]);
    }

    // may have pushed them off an edge
    if ( check->s.groundEntityNum != pusher->s.number )
    {
        check->s.groundEntityNum = -1;
    }

    block = G_TestEntityPosition( check );
    if (!block)
    {
        // pushed ok
        if ( check->client )
        {
            VectorCopy( check->client->ps.origin, check->r.currentOrigin );
        }
        else
        {
            VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
        }

        trap_LinkEntity (check);
        return qtrue;
    }

    // if it is ok to leave in the old position, do it
    // this is only relevent for riding entities, not pushed
    // Sliding trapdoors can cause this.
    VectorCopy( (pushed_p-1)->origin, check->s.pos.trBase);
    if ( check->client )
    {
        VectorCopy( (pushed_p-1)->origin, check->client->ps.origin);
    }

    VectorCopy( (pushed_p-1)->angles, check->s.apos.trBase );
    block = G_TestEntityPosition (check);
    if ( !block )
    {
        check->s.groundEntityNum = -1;
        pushed_p--;
        return qtrue;
    }

    // blocked
    return qfalse;
}


void G_ExplodeMissile( gentity_t *ent );

/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If qfalse is returned, *obstacle will be the blocking entity
============
*/
qboolean G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle ) {
    int         i, e;
    gentity_t   *check;
    vec3_t      mins, maxs;
    pushed_t    *p;
    int         entityList[MAX_GENTITIES];
    int         listedEntities;
    vec3_t      totalMins, totalMaxs;

    *obstacle = NULL;


    // mins/maxs are the bounds at the destination
    // totalMins / totalMaxs are the bounds for the entire move
    if ( pusher->r.currentAngles[0] || pusher->r.currentAngles[1] || pusher->r.currentAngles[2]
        || amove[0] || amove[1] || amove[2] )
    {
        float       radius;

        radius = RadiusFromBounds( pusher->r.mins, pusher->r.maxs );
        for ( i = 0 ; i < 3 ; i++ )
        {
            mins[i] = pusher->r.currentOrigin[i] + move[i] - radius;
            maxs[i] = pusher->r.currentOrigin[i] + move[i] + radius;
            totalMins[i] = mins[i] - move[i];
            totalMaxs[i] = maxs[i] - move[i];
        }

        // If only yaw then dontbother with the z axis
        if ( !pusher->r.currentAngles[PITCH] && !pusher->r.currentAngles[ROLL] )
        {
            mins[2] = pusher->r.absmin[i] + move[2];
            maxs[2] = pusher->r.absmax[i] + move[2];

            totalMins[2] = pusher->r.absmin[2];
            totalMaxs[2] = pusher->r.absmax[2];

            if ( move[2] > 0 )
            {
                totalMaxs[2] += move[i];
            }
            else
            {
                totalMins[2] += move[i];
            }
        }
    }
    else
    {
        for (i=0 ; i<3 ; i++) {
            mins[i] = pusher->r.absmin[i] + move[i];
            maxs[i] = pusher->r.absmax[i] + move[i];
        }

        VectorCopy( pusher->r.absmin, totalMins );
        VectorCopy( pusher->r.absmax, totalMaxs );
        for (i=0 ; i<3 ; i++) {
            if ( move[i] > 0 ) {
                totalMaxs[i] += move[i];
            } else {
                totalMins[i] += move[i];
            }
        }
    }

    // unlink the pusher so we don't get it in the entityList
    trap_UnlinkEntity( pusher );

    listedEntities = trap_EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES );

    // move the pusher to it's final position
    VectorAdd( pusher->r.currentOrigin, move, pusher->r.currentOrigin );
    VectorAdd( pusher->r.currentAngles, amove, pusher->r.currentAngles );
    trap_LinkEntity( pusher );

    // see if any solid entities are inside the final position
    for ( e = 0 ; e < listedEntities ; e++ ) {
        check = &g_entities[ entityList[ e ] ];

        // Dont bother with corpses for now
        if ( check->s.eType == ET_BODY )
        {
            continue;
        }

        // only push items and players
        if ( check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && !check->physicsObject ) {
            continue;
        }

        // if the entity is standing on the pusher, it will definitely be moved
        if ( check->s.groundEntityNum != pusher->s.number ) {
            // see if the ent needs to be tested
            if ( check->r.absmin[0] >= maxs[0]
            || check->r.absmin[1] >= maxs[1]
            || check->r.absmin[2] >= maxs[2]
            || check->r.absmax[0] <= mins[0]
            || check->r.absmax[1] <= mins[1]
            || check->r.absmax[2] <= mins[2] ) {
                continue;
            }
            // see if the ent's bbox is inside the pusher's final position
            // this does allow a fast moving object to pass through a thin entity...
            if (!G_TestEntityPosition (check)) {
                continue;
            }
        }

        // the entity needs to be pushed
        if ( G_TryPushingEntity( check, pusher, move, amove ) )
        {
            continue;
        }

        // If being blocked by an item just let it pass through it and get rid of the item
        if ( check->s.eType == ET_ITEM )
        {
            // If the item is a gametype item, it should respawn properly.
            if (check->item && check->item->giType == IT_GAMETYPE){
                trap_GT_SendEvent(GTEV_ITEM_STUCK, level.time,
                    check->item->quantity, 0, 0, 0, 0);
            }else{
                G_FreeEntity ( check );
            }

            continue;
        }

        // the move was blocked an entity

        // bobbing entities are instant-kill and never get blocked
        if ( pusher->s.pos.trType == TR_SINE || pusher->s.apos.trType == TR_SINE )
        {
            G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH, HL_NONE );
            continue;
        }


        // save off the obstacle so we can call the block function (crush, etc)
        *obstacle = check;

        // move back any entities we already moved
        // go backwards, so if the same entity was pushed
        // twice, it goes back to the original position
        for ( p=pushed_p-1 ; p>=pushed ; p-- ) {
            VectorCopy (p->origin, p->ent->s.pos.trBase);
            VectorCopy (p->angles, p->ent->s.apos.trBase);
            if ( p->ent->client ) {
                p->ent->client->ps.delta_angles[YAW] = p->deltayaw;
                VectorCopy (p->origin, p->ent->client->ps.origin);
            }
            trap_LinkEntity (p->ent);
        }
        return qfalse;
    }

    return qtrue;
}


/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( gentity_t *ent ) {
    vec3_t      move, amove;
    gentity_t   *part, *obstacle;
    vec3_t      origin, angles;

    obstacle = NULL;

    // make sure all team slaves can move before commiting
    // any moves or calling any think functions
    // if the move is blocked, all moved objects will be backed out
    pushed_p = pushed;
    for (part = ent ; part ; part=part->teamchain) {
        // get current position
        BG_EvaluateTrajectory( &part->s.pos, level.time, origin );
        BG_EvaluateTrajectory( &part->s.apos, level.time, angles );
        VectorSubtract( origin, part->r.currentOrigin, move );
        VectorSubtract( angles, part->r.currentAngles, amove );
        if ( !G_MoverPush( part, move, amove, &obstacle ) ) {
            break;  // move was blocked
        }
    }

    if (part) {
        // go back to the previous position
        for ( part = ent ; part ; part = part->teamchain ) {
            part->s.pos.trTime += level.time - level.previousTime;
            part->s.apos.trTime += level.time - level.previousTime;
            BG_EvaluateTrajectory( &part->s.pos, level.time, part->r.currentOrigin );
            BG_EvaluateTrajectory( &part->s.apos, level.time, part->r.currentAngles );
            trap_LinkEntity( part );
        }

        // if the pusher has a "blocked" function, call it
        if (ent->blocked) {
            ent->blocked( ent, obstacle );
        }
        return;
    }

    // the move succeeded
    for ( part = ent ; part ; part = part->teamchain ) {
        // call the reached function if time is at or past end point
        if ( part->s.pos.trType == TR_LINEAR_STOP ) {
            if ( level.time >= part->s.pos.trTime + part->s.pos.trDuration ) {
                if ( part->reached ) {
                    part->reached( part );
                }
            }
        }
        ///RxCxW - 03.13.07 - 08:02pm #rotating
        if ( part->s.apos.trType == TR_LINEAR_STOP ) {
            if ( level.time >= part->s.apos.trTime + part->s.apos.trDuration ) {
                if ( part->reached ) {
                    part->reached( part );
                }
            }
        }
        ///End  - 03.13.07 - 08:02pm
    }
}

/*
================
G_RunMover

================
*/
void G_RunMover( gentity_t *ent ) {
    // if not a team captain, don't do anything, because
    // the captain will handle everything
    if ( ent->flags & FL_TEAMSLAVE ) {
        return;
    }

    // if stationary at one of the positions, don't move anything
    if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) {
        G_MoverTeam( ent );
    }

    // check think function
    G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
===============
SetMoverState
===============
*/
void SetMoverState( gentity_t *ent, moverState_t moverState, int time ) {
    vec3_t          delta;
    float           f;

    ent->moverState = moverState;

    // KRIS 13/03/2003 2:36PM
    // if we're supposed to rotate, then rotate... duh!
    if (ent->apos2[0] || ent->apos2[1] || ent->apos2[2])
    {
        ROT_SetMoverState( ent, moverState, time );
        trap_LinkEntity( ent );
        return;
    }
    // KRIS

    ent->s.pos.trTime = time;
    switch( moverState ) {
    case MOVER_POS1:
        VectorCopy( ent->pos1, ent->s.pos.trBase );
        ent->s.pos.trType = TR_STATIONARY;
        break;
    case MOVER_POS2:
        VectorCopy( ent->pos2, ent->s.pos.trBase );
        ent->s.pos.trType = TR_STATIONARY;
        break;
    case MOVER_1TO2:
        VectorCopy( ent->pos1, ent->s.pos.trBase );
        VectorSubtract( ent->pos2, ent->pos1, delta );
        f = 1000.0 / ent->s.pos.trDuration;
        VectorScale( delta, f, ent->s.pos.trDelta );
        ent->s.pos.trType = TR_LINEAR_STOP;
        break;
    case MOVER_2TO1:
        VectorCopy( ent->pos2, ent->s.pos.trBase );
        VectorSubtract( ent->pos1, ent->pos2, delta );
        f = 1000.0 / ent->s.pos.trDuration;
        VectorScale( delta, f, ent->s.pos.trDelta );
        ent->s.pos.trType = TR_LINEAR_STOP;
        break;
    }
    BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
    trap_LinkEntity( ent );
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( gentity_t *teamLeader, int moverState, int time ) {
    gentity_t       *slave;

    for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {
        SetMoverState( slave, (moverState_t)moverState, time );
    }
}



/*
================
ReturnToPos1
================
*/
void ReturnToPos1( gentity_t *ent ) {
    MatchTeam( ent, MOVER_2TO1, level.time );

    // looping sound
    ent->s.loopSound = ent->soundLoop;

    // starting sound
    if ( ent->sound2to1 ) {
        G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
    }
}


/*
================
Reached_BinaryMover
================
*/
void Reached_BinaryMover( gentity_t *ent )
{
    // stop the looping sound
    ent->s.loopSound = ent->soundLoop;

    if ( ent->moverState == MOVER_1TO2 )
    {
        // reached pos2
        SetMoverState( ent, MOVER_POS2, level.time );

        // play sound
        if ( ent->soundPos2 )
        {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos2 );
        }

        // return to pos1 after a delay
        ent->think = ReturnToPos1;
        ent->nextthink = level.time + ent->wait;

        // fire targets
        if ( !ent->activator )
        {
            ent->activator = ent;
        }

        G_UseTargets( ent, ent->activator );
    }
    else if ( ent->moverState == MOVER_2TO1 )
    {
        // reached pos1
        SetMoverState( ent, MOVER_POS1, level.time );

        // play sound
        if ( ent->soundPos1 )
        {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->soundPos1 );
        }

        // close areaportals
        if ( ent->teammaster == ent || !ent->teammaster )
        {
            trap_AdjustAreaPortalState( ent, qfalse );
        }
    }
    else
    {
        Com_Error(ERR_FATAL, "Reached_BinaryMover: bad moverState");
    }
}

/*
================
Use_BinaryMover
================
*/
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
    int     total;
    int     partial;

    ///Skip doors that are locked
    if (ent->spawnflags & 16 && ent->spawnflags != 604)
        return;

    // only the master should be used
    if ( ent->flags & FL_TEAMSLAVE )
    {
        Use_BinaryMover( ent->teammaster, other, activator );
        return;
    }

    ent->activator = activator;

    if ( ent->moverState == MOVER_POS1 )
    {
        // start moving 50 msec later, becase if this was player
        // triggered, level.time hasn't been advanced yet
        MatchTeam( ent, MOVER_1TO2, level.time + 50 );

        // starting sound
        if ( ent->sound1to2 )
        {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
        }

        // looping sound
        ent->s.loopSound = ent->soundLoop;

        // open areaportal
        if ( ent->teammaster == ent || !ent->teammaster )
        {
            trap_AdjustAreaPortalState( ent, qtrue );
        }
        return;
    }

    // if all the way up, just delay before coming down
    if ( ent->moverState == MOVER_POS2 )
    {
        if(ent->spawnflags == 604)
            ent->nextthink = -1;
        else
        ent->nextthink = level.time + ent->wait;
        return;
    }

    // only partway down before reversing
    if ( ent->moverState == MOVER_2TO1 )
    {
        total = ent->s.pos.trDuration;
        partial = level.time - ent->s.pos.trTime;
        if ( partial > total )
        {
            partial = total;
        }

        MatchTeam( ent, MOVER_1TO2, level.time - ( total - partial ) );

        if ( ent->sound1to2 )
        {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2 );
        }
        return;
    }

    // only partway up before reversing
    if ( ent->moverState == MOVER_1TO2 )
    {
        total = ent->s.pos.trDuration;
        partial = level.time - ent->s.pos.trTime;
        if ( partial > total )
        {
            partial = total;
        }

        MatchTeam( ent, MOVER_2TO1, level.time - ( total - partial ) );

        if ( ent->sound2to1 )
        {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1 );
        }
        return;
    }
}

/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMover( gentity_t *ent ) {
    vec3_t      move;
    float       distance;
    char        *sound;

    // if the "model2" key is set, use a seperate model
    // for drawing, but clip against the brushes
    if ( ent->model2 ) {
        ent->s.modelindex2 = G_ModelIndex( ent->model2 );
    }

    // if the "loopsound" key is set, use a constant looping sound when moving
    if ( G_SpawnString( "noise", "100", &sound ) ) {
        ent->s.loopSound = G_SoundIndex( sound, qtrue );
    }

    ent->use = Use_BinaryMover;
    ent->reached = Reached_BinaryMover;

    ent->moverState = MOVER_POS1;
    ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    ent->s.eType = ET_MOVER;
    VectorCopy (ent->pos1, ent->r.currentOrigin);
    trap_LinkEntity (ent);

    ent->s.pos.trType = TR_STATIONARY;
    VectorCopy( ent->pos1, ent->s.pos.trBase );

    // calculate time to reach second position from speed
    VectorSubtract( ent->pos2, ent->pos1, move );
    distance = VectorLength( move );
    if ( ! ent->speed ) {
        ent->speed = 100;
    }
    VectorScale( move, ent->speed, ent->s.pos.trDelta );
    ent->s.pos.trDuration = distance * 1000 / ent->speed;
    if ( ent->s.pos.trDuration <= 0 ) {
        ent->s.pos.trDuration = 1;
    }
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
void Blocked_Door( gentity_t *ent, gentity_t *other )
{
    // remove anything other than a client
    if ( !other->client )
    {
        // except mission items
        if( other->s.eType == ET_ITEM && other->item->giType == IT_GAMETYPE )
        {
            return;
        }

        G_TempEntity( other->s.origin, EV_ITEM_POP );
        G_FreeEntity( other );
        return;
    }

    if ( ent->damage )
    {
        G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH, HL_NONE );
    }

    if ( ent->spawnflags & 4 )
    {
        {
            Use_BinaryMover( ent, ent, other );
            return;
        }
        // crushers don't reverse
        return;
    }

    // reverse direction
    Use_BinaryMover( ent, ent, other );
}

/*
================
Touch_DoorTriggerSpectator
================
*/
static void Touch_DoorTriggerSpectator( gentity_t *ent, gentity_t *other, trace_t *trace ) {
    int i, axis;
    vec3_t origin, dir, angles;

    axis = ent->count;
    VectorClear(dir);
    if (fabs(other->s.origin[axis] - ent->r.absmax[axis]) <
        fabs(other->s.origin[axis] - ent->r.absmin[axis])) {
        origin[axis] = ent->r.absmin[axis] - 10;
        dir[axis] = -1;
    }
    else {
        origin[axis] = ent->r.absmax[axis] + 10;
        dir[axis] = 1;
    }
    for (i = 0; i < 3; i++) {
        if (i == axis) continue;
        origin[i] = (ent->r.absmin[i] + ent->r.absmax[i]) * 0.5;
    }
    vectoangles(dir, angles);
    TeleportPlayer(other, origin, angles, qfalse );
}

/*
================
Touch_DoorTrigger
================
*/
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace )
{
    if ( !other->client )
        return;
    if(G_IsClientSpectating ( other->client ))
        return;
    /// if the door is not open and not opening
    if ( ent->parent->moverState != MOVER_1TO2 && ent->parent->moverState != MOVER_POS2
        /*&&    ent->parent->moverState != ROTATOR_1TO2 && ent->parent->moverState != ROTATOR_POS2*/)
        Use_BinaryMover( ent->parent, ent, other );
    //if ( other->client && G_IsClientSpectating ( other->client ) )
    //  return;
    //if ( ent->parent->moverState != MOVER_1TO2 && ent->parent->moverState != ROTATOR_1TO2 )
    //  Use_BinaryMover( ent->parent, ent, other );
}

/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTrigger( gentity_t *ent )
{
    gentity_t   *other;
    vec3_t      mins, maxs;
    int         i, best;

    // set all of the slaves as shootable
    for ( other = ent ; other ; other = other->teamchain ) {
        other->takedamage = qtrue;
    }

    // find the bounds of everything on the team
    VectorCopy (ent->r.absmin, mins);
    VectorCopy (ent->r.absmax, maxs);

    for (other = ent->teamchain ; other ; other=other->teamchain) {
        AddPointToBounds (other->r.absmin, mins, maxs);
        AddPointToBounds (other->r.absmax, mins, maxs);
    }

    // find the thinnest axis, which will be the one we expand
    best = 0;
    for ( i = 1 ; i < 3 ; i++ ) {
        if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
            best = i;
        }
    }
    maxs[best] += 120;
    mins[best] -= 120;

    // create a trigger with this size
    other = G_Spawn ();
    other->classname = "door_trigger";
    VectorCopy (mins, other->r.mins);
    VectorCopy (maxs, other->r.maxs);
    other->parent = ent;
    other->r.contents = CONTENTS_TRIGGER;
    other->touch = Touch_DoorTrigger;
    // remember the thinnest axis
    other->count = best;
    trap_LinkEntity (other);

    MatchTeam( ent, ent->moverState, level.time );
}

void Think_MatchTeam( gentity_t *ent ) {
    MatchTeam( ent, ent->moverState, level.time );
}


/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER
TOGGLE      wait in both the start and end states for a trigger event.
START_OPEN  the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER   monsters will not trigger this door

"model2"    .md3 model to also draw
"angle"     determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"     movement speed (100 default)
"wait"      wait before returning (3 default, -1 = never return)
"lip"       lip remaining at end of move (8 default)
"dmg"       damage to inflict when blocked (2 default)
"health"    if set, the door must be shot open
*/
void SP_func_door (gentity_t *ent) {
    vec3_t  abs_movedir;
    float   distance;
    vec3_t  size;
    float   lip;

    #ifndef _DEMO
    ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/doors/dr1_strt.wav", qtrue);
    ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/doors/dr1_end.wav", qtrue);
    #endif // not _DEMO

    ent->blocked = Blocked_Door;

    // default speed of 400
    if (!ent->speed)
        ent->speed = 400;

    // default wait of 2 seconds
    if (!ent->wait)
        ent->wait = 2;
    ent->wait *= 1000;

    // default lip of 8 units
    G_SpawnFloat( "lip", "8", &lip );

    // default damage of 2 points
    G_SpawnInt( "dmg", "2", &ent->damage );

    VectorCopy( ent->s.angles, ent->savedAngles );

    // first position at start
    VectorCopy( ent->s.origin, ent->pos1 );

    // calculate second position
    trap_SetBrushModel( ent, ent->model );
    G_SetMovedir (ent->s.angles, ent->movedir);
    abs_movedir[0] = fabs(ent->movedir[0]);
    abs_movedir[1] = fabs(ent->movedir[1]);
    abs_movedir[2] = fabs(ent->movedir[2]);
    VectorSubtract( ent->r.maxs, ent->r.mins, size );
    distance = DotProduct( abs_movedir, size ) - lip;
    VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

    // if "start_open", reverse position 1 and 2
    if ( ent->spawnflags & 1 ) {
        vec3_t  temp;

        VectorCopy( ent->pos2, temp );
        VectorCopy( ent->s.origin, ent->pos2 );
        VectorCopy( temp, ent->pos1 );
    }

    InitMover( ent );

    ent->nextthink = level.time + FRAMETIME;

    if ( ! (ent->flags & FL_TEAMSLAVE ) ) {
        int health;

        G_SpawnInt( "health", "0", &health );
        if ( health ) {
            ent->takedamage = qtrue;
        }
        if ( ent->targetname || health ) {
            // non touch/shoot doors
            ent->think = Think_MatchTeam;
        } else {
            ent->think = Think_SpawnNewDoorTrigger;
        }
    }

    if ( ent->teammaster == ent || !ent->teammaster ) {
        if(ent->damage == -999){
            trap_AdjustAreaPortalState( ent, qtrue );
            return;
        }
    }
}

/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
void Touch_Plat( gentity_t *ent, gentity_t *other, trace_t *trace ) {
    if ( !other->client || other->client->ps.stats[STAT_HEALTH] <= 0 ) {
        return;
    }

    // delay return-to-pos1 by one second
    if ( ent->moverState == MOVER_POS2 ) {
        ent->nextthink = level.time + 1000;
    }
}

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
void Touch_PlatCenterTrigger(gentity_t *ent, gentity_t *other, trace_t *trace ) {
    if ( !other->client ) {
        return;
    }

    if ( ent->parent->moverState == MOVER_POS1 ) {
        Use_BinaryMover( ent->parent, ent, other );
    }
}


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger( gentity_t *ent ) {
    gentity_t   *trigger;
    vec3_t  tmin, tmax;

    // the middle trigger will be a thin trigger just
    // above the starting position
    trigger = G_Spawn();
    trigger->classname = "plat_trigger";
    trigger->touch = Touch_PlatCenterTrigger;
    trigger->r.contents = CONTENTS_TRIGGER;
    trigger->parent = ent;

    tmin[0] = ent->pos1[0] + ent->r.mins[0] + 33;
    tmin[1] = ent->pos1[1] + ent->r.mins[1] + 33;
    tmin[2] = ent->pos1[2] + ent->r.mins[2];

    tmax[0] = ent->pos1[0] + ent->r.maxs[0] - 33;
    tmax[1] = ent->pos1[1] + ent->r.maxs[1] - 33;
    tmax[2] = ent->pos1[2] + ent->r.maxs[2] + 8;

    if ( tmax[0] <= tmin[0] ) {
        tmin[0] = ent->pos1[0] + (ent->r.mins[0] + ent->r.maxs[0]) *0.5;
        tmax[0] = tmin[0] + 1;
    }
    if ( tmax[1] <= tmin[1] ) {
        tmin[1] = ent->pos1[1] + (ent->r.mins[1] + ent->r.maxs[1]) *0.5;
        tmax[1] = tmin[1] + 1;
    }

    VectorCopy (tmin, trigger->r.mins);
    VectorCopy (tmax, trigger->r.maxs);

    trap_LinkEntity (trigger);
}


/*QUAKED func_plat (0 .5 .8) ?
Plats are always drawn in the extended position so they will light correctly.

"lip"       default 8, protrusion above rest position
"height"    total height of movement, defaults to model height
"speed"     overrides default 200.
"dmg"       overrides default 2
"model2"    .md3 model to also draw
*/
void SP_func_plat (gentity_t *ent) {
    float       lip, height;

    #ifndef _DEMO
    ent->sound1to2 = ent->sound2to1 = G_SoundIndex("sound/movers/plats/pt1_strt.wav", qtrue);
    ent->soundPos1 = ent->soundPos2 = G_SoundIndex("sound/movers/plats/pt1_end.wav", qtrue);
    #endif // not _DEMO

    VectorClear (ent->s.angles);

    G_SpawnFloat( "speed", "200", &ent->speed );
    G_SpawnInt( "dmg", "2", &ent->damage );
    G_SpawnFloat( "wait", "1", &ent->wait );
    G_SpawnFloat( "lip", "8", &lip );

    ent->wait = 1000;

    // create second position
    trap_SetBrushModel( ent, ent->model );

    if ( !G_SpawnFloat( "height", "0", &height ) ) {
        height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;
    }

    // pos1 is the rest (bottom) position, pos2 is the top
    VectorCopy( ent->s.origin, ent->pos2 );
    VectorCopy( ent->pos2, ent->pos1 );
    ent->pos1[2] -= height;

    InitMover( ent );

    // touch function keeps the plat from returning while
    // a live player is standing on it
    ent->touch = Touch_Plat;

    ent->blocked = Blocked_Door;

    ent->parent = ent;  // so it can be treated as a door

    // spawn the trigger if one hasn't been custom made
    if ( !ent->targetname ) {
        SpawnPlatTrigger(ent);
    }
}


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace ) {
    if ( !other->client ) {
        return;
    }

    if ( ent->moverState == MOVER_POS1 ) {
        Use_BinaryMover( ent, other, other );
    }
}


/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"model2"    .md3 model to also draw
"angle"     determines the opening direction
"target"    all entities with a matching targetname will be used
"speed"     override the default 40 speed
"wait"      override the default 1 second wait (-1 = never return)
"lip"       override the default 4 pixel lip remaining at end of move
"health"    if set, the button must be killed instead of touched
*/
void SP_func_button( gentity_t *ent ) {
    vec3_t      abs_movedir;
    float       distance;
    vec3_t      size;
    float       lip;

    #ifndef _DEMO
    ent->sound1to2 = G_SoundIndex("sound/movers/switches/butn2.wav", qtrue);
    #endif // not _DEMO

    if ( !ent->speed ) {
        ent->speed = 40;
    }

    if ( !ent->wait ) {
        ent->wait = 1;
    }
    ent->wait *= 1000;

    // first position
    VectorCopy( ent->s.origin, ent->pos1 );

    // calculate second position
    trap_SetBrushModel( ent, ent->model );

    G_SpawnFloat( "lip", "4", &lip );

    G_SetMovedir( ent->s.angles, ent->movedir );
    abs_movedir[0] = fabs(ent->movedir[0]);
    abs_movedir[1] = fabs(ent->movedir[1]);
    abs_movedir[2] = fabs(ent->movedir[2]);
    VectorSubtract( ent->r.maxs, ent->r.mins, size );
    distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
    VectorMA (ent->pos1, distance, ent->movedir, ent->pos2);

    if (ent->health) {
        // shootable button
        ent->takedamage = qtrue;
    } else {
        // touchable button
        ent->touch = Touch_Button;
    }

    InitMover( ent );
}



/*
===============================================================================

TRAIN

===============================================================================
*/


#define TRAIN_START_ON          1
#define TRAIN_TOGGLE            2
#define TRAIN_BLOCK_STOPS       4
#define PATH_CORNER_INNER_LOOP  8
#define TRAIN_ALT_DAMAGE        16

/*
===============
Think_BeginMoving

The wait time at a corner has completed, so start moving again
===============
*/
void Think_BeginMoving( gentity_t *ent ) {
    ent->s.pos.trTime = level.time;
    ent->s.pos.trType = TR_LINEAR_STOP;
}

/*
===============
Blocked_Train

Workaround for models that don't do damage if applied on a func_train.
===============
*/
void Blocked_Train(gentity_t *ent, gentity_t *other)
{
    // Do damage to the player that blocks.
    if (ent->damage)
    {
        G_Damage(other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH, HL_NONE);
    }
}

/*
===============
Reached_Train
===============
*/
void Reached_Train( gentity_t *ent ) {
    gentity_t       *next;
    float           speed;
    vec3_t          move;
    float           length;

    // copy the apropriate values
    next = ent->nextTrain;
    if ( !next || !next->nextTrain ) {
        return;     // just stop
    }

    // fire all other targets
    G_UseTargets( next, NULL );

    // set the new trajectory
    if (ent->nextTrain == ent){
        ent->nextTrain = ent->loopCorner;
    }else{
        ent->nextTrain = next->nextTrain;
    }
    VectorCopy( next->s.origin, ent->pos1 );
    VectorCopy( next->nextTrain->s.origin, ent->pos2 );

    // if the path_corner has a speed, use that
    if ( next->speed ) {
        speed = next->speed;
    } else {
        // otherwise use the train's speed
        speed = ent->speed;
    }
    if ( speed < 1 ) {
        speed = 1;
    }

    // calculate duration
    VectorSubtract( ent->pos2, ent->pos1, move );
    length = VectorLength( move );

    ent->s.pos.trDuration = length * 1000 / speed;

    // looping sound
    ent->s.loopSound = next->soundLoop;

    // start it going
    SetMoverState( ent, MOVER_1TO2, level.time );

    // if there is a "wait" value on the target, don't start moving yet
    if ( next->wait ) {
        ent->nextthink = level.time + next->wait * 1000;
        ent->think = Think_BeginMoving;
        ent->s.pos.trType = TR_STATIONARY;
    }
}


/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
void Think_SetupTrainTargets( gentity_t *ent ) {
    gentity_t       *path, *next, *start;
    qboolean        secondTime = qfalse;

    ent->nextTrain = G_Find( NULL, FOFS(targetname), ent->target );
    if ( !ent->nextTrain )
    {
        Com_Printf( "func_train at %s with an unfound target\n",
            vtos(ent->r.absmin) );
        return;
    }

    start = NULL;
    for ( path = ent->nextTrain ; path != start ; path = next ) {
        if ( !start ) {
            start = path;
        }

        if ( !path->target ) {
            Com_Printf( "Train corner at %s without a target\n",
                vtos(path->s.origin) );
            return;
        }

        // find a path_corner among the targets
        // there may also be other targets that get fired when the corner
        // is reached
        next = NULL;
        do {
            next = G_Find( next, FOFS(targetname), path->target );
            if ( !next ) {
                Com_Printf( "Train corner at %s without a target path_corner\n",
                    vtos(path->s.origin) );
                return;
            }

            if (next->spawnflags & PATH_CORNER_INNER_LOOP){
                if (!ent->loopCorner){
                    ent->loopCorner = next;
                }else if(!secondTime){
                    secondTime = qtrue;
                }
            }
        } while ( strcmp( next->classname, "path_corner" ) );

        if (secondTime){
            path->nextTrain = ent;
            break;
        }

        path->nextTrain = next;
    }

    // start the train moving from the first corner
    Reached_Train( ent );
}



/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner( gentity_t *self ) {
    if ( !self->targetname ) {
        Com_Printf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
        G_FreeEntity( self );
        return;
    }
    // path corners don't need to be linked in
}



/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"    .md3 model to also draw
"speed"     default 100
"dmg"       default 2
"noise"     looping sound to play when the train is in motion
"target"    next path corner
*/
void SP_func_train (gentity_t *self) {
    VectorClear (self->s.angles);

    if (self->spawnflags & TRAIN_BLOCK_STOPS) {
        self->damage = 0;
    } else {
        if (!self->damage) {
            self->damage = 2;
        }
    }

    if ( !self->speed ) {
        self->speed = 100;
    }

    if ( !self->target ) {
        Com_Printf ("func_train without a target at %s\n", vtos(self->r.absmin));
        G_FreeEntity( self );
        return;
    }

    trap_SetBrushModel( self, self->model );
    InitMover( self );

    self->reached = Reached_Train;

    // start trains on the second frame, to make sure their targets have had
    // a chance to spawn
    self->nextthink = level.time + FRAMETIME;
    self->think = Think_SetupTrainTargets;

    // FIXME BOE: Not efficient? Other ways of checking this?
    // See if the user specified to have alternate damage (damage by models that don't do damage for whatever reason).
    if (self->damage && self->spawnflags & TRAIN_ALT_DAMAGE){
        self->blocked = Blocked_Train;
    }
}

/*
===============================================================================

STATIC

===============================================================================
*/


/*QUAKED func_static (0 .5 .8) ?
A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"    .md3 model to also draw
*/
void SP_func_static( gentity_t *ent ) {
    trap_SetBrushModel( ent, ent->model );
    VectorCopy(ent->s.origin, ent->pos1);
    VectorCopy(ent->s.angles, ent->r.currentAngles);
    VectorCopy(ent->s.angles, ent->s.apos.trBase );
    InitMover( ent );
    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );

    if (level.mBSPInstanceDepth)
    {   // this means that this guy will never be updated, moved, changed, etc.
        ent->s.eFlags = EF_PERMANENT;
    }
}

void G_WallUse(gentity_t *self, gentity_t *other, gentity_t *activator)
{
    if ( self->r.linked )
    {
        trap_UnlinkEntity ( self );
    }
    else
    {
        trap_LinkEntity ( self );
    }
}

/*QUAKED func_wall (0 .5 .8) ? START_OFF
A func wall can be turned off and on by targetting it.
*/
void SP_func_wall ( gentity_t* ent )
{
    // Right now func walls are not supported in multiplayer
    if ( RMG.integer )
    {
        trap_UnlinkEntity ( ent );
        return;
    }

    if(level.mBSPInstanceDepth && !ent->spawnflags){
        // Boe!Man 2/15/16: Take the parent (misc_bsp) their spawnflags if we don't have any.
        ent->spawnflags = level.mSpawnFlags;
    }

    trap_SetBrushModel( ent, ent->model );

    InitMover( ent );

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
    VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

    #ifdef _GOLD
    ent->s.eType = ET_WALL;
    #else
    ent->s.eType = ET_MOVER;
    #endif // _GOLD

    ent->use = G_WallUse;

    if ( !(ent->spawnflags & 1 ) )
    {
        trap_LinkEntity( ent );
    }
}

/*
===============================================================================

ROTATING

===============================================================================
*/


/*QUAKED func_rotating (0 .5 .8) ? START_ON - X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"model2"    .md3 model to also draw
"speed"     determines how fast it moves; default value is 100.
"dmg"       damage to inflict when blocked (2 default)
*/
void SP_func_rotating (gentity_t *ent) {
    if ( !ent->speed ) {
        ent->speed = 100;
    }

    // set the axis of rotation
    ent->s.apos.trType = TR_LINEAR;
    if ( ent->spawnflags & 4 ) {
        ent->s.apos.trDelta[2] = ent->speed;
    } else if ( ent->spawnflags & 8 ) {
        ent->s.apos.trDelta[0] = ent->speed;
    } else {
        ent->s.apos.trDelta[1] = ent->speed;
    }

    if (!ent->damage) {
        ent->damage = 2;
    }

    if(!strstr(ent->model, "none")){
    trap_SetBrushModel( ent, ent->model );
    }else
        ent->s.modelindex = G_ModelIndex("models/objects/Armory/virus.md3");

    InitMover( ent );

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
    VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

    trap_LinkEntity( ent );
}


/*
===============================================================================

BOBBING

===============================================================================
*/


/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS
Normally bobs on the Z axis
"model2"    .md3 model to also draw
"height"    amplitude of bob (32 default)
"speed"     seconds to complete a bob cycle (4 default)
"phase"     the 0.0 to 1.0 offset in the cycle to start at
"dmg"       damage to inflict when blocked (2 default)
*/
void SP_func_bobbing (gentity_t *ent) {
    float       height;
    float       phase;

    G_SpawnFloat( "speed", "4", &ent->speed );
    G_SpawnFloat( "height", "32", &height );
    G_SpawnInt( "dmg", "2", &ent->damage );
    G_SpawnFloat( "phase", "0", &phase );

    trap_SetBrushModel( ent, ent->model );
    InitMover( ent );

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );

    VectorCopy( ent->s.angles, ent->s.apos.trBase );

    ent->s.pos.trDuration = ent->speed * 1000;
    ent->s.pos.trTime = ent->s.pos.trDuration * phase;
    ent->s.pos.trType = TR_SINE;

    // set the axis of bobbing
    if ( ent->spawnflags & 1 ) {
        ent->s.pos.trDelta[0] = height;
    } else if ( ent->spawnflags & 2 ) {
        ent->s.pos.trDelta[1] = height;
    } else {
        ent->s.pos.trDelta[2] = height;
    }
}

/*
===============================================================================

PENDULUM

===============================================================================
*/


/*QUAKED func_pendulum (0 .5 .8) ?
You need to have an origin brush as part of this entity.
Pendulums always swing north / south on unrotated models.  Add an angles field to the model to allow rotation in other directions.
Pendulum frequency is a physical constant based on the length of the beam and gravity.
"model2"    .md3 model to also draw
"speed"     the number of degrees each way the pendulum swings, (30 default)
"phase"     the 0.0 to 1.0 offset in the cycle to start at
"dmg"       damage to inflict when blocked (2 default)
*/
void SP_func_pendulum(gentity_t *ent) {
    float       freq;
    float       length;
    float       phase;
    float       speed;

    G_SpawnFloat( "speed", "30", &speed );
    G_SpawnInt( "dmg", "2", &ent->damage );
    G_SpawnFloat( "phase", "0", &phase );

    trap_SetBrushModel( ent, ent->model );

    // find pendulum length
    length = fabs( ent->r.mins[2] );
    if ( length < 8 ) {
        length = 8;
    }

    freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity.value / ( 3 * length ) );

    ent->s.pos.trDuration = ( 1000 / freq );

    InitMover( ent );

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );

    VectorCopy( ent->s.angles, ent->s.apos.trBase );

    ent->s.apos.trDuration = 1000 / freq;
    ent->s.apos.trTime = ent->s.apos.trDuration * phase;
    ent->s.apos.trType = TR_SINE;
    ent->s.apos.trDelta[2] = speed;
}

/*
===============================================================================

GLASS

===============================================================================
*/

void G_ResetGlass ( void )
{
    gentity_t* ent;
    ent = NULL;
    while ( NULL != (ent = G_Find ( ent, FOFS(classname), "func_glass" ) ) )
    {
        trap_LinkEntity ( ent );
    }
}

void G_GlassDie ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod, int hitLocation, vec3_t hitDir )
{
    gentity_t *te;
    vec3_t dif;

    dif[0] = (self->r.absmax[0]+self->r.absmin[0])/2;
    dif[1] = (self->r.absmax[1]+self->r.absmin[1])/2;
    dif[2] = (self->r.absmax[2]+self->r.absmin[2])/2;

    // Make the client shatter the glass
    te = G_TempEntity( dif, EV_GLASS_SHATTER );
    te->s.eventParm = self->s.number;
    VectorCopy(self->r.maxs, te->s.origin);
    VectorCopy(self->r.mins, te->s.angles);

    // Only unlink the entity so we can reset it later
    trap_UnlinkEntity(self);
}

void G_GlassUse(gentity_t *self, gentity_t *other, gentity_t *activator)
{
    G_GlassDie(self, other, other, 100, MOD_UNKNOWN, HL_NONE, vec3_origin );
}

/*QUAKED func_glass (0 .5 .8) ?
Breakable glass
*/
void SP_func_glass( gentity_t *ent )
{
    trap_SetBrushModel( ent, ent->model );
    InitMover( ent );

    ent->r.svFlags = SVF_GLASS_BRUSH;

    VectorCopy( ent->s.origin, ent->s.pos.trBase );
    VectorCopy( ent->s.origin, ent->r.currentOrigin );
    if (!ent->health)
    {
        ent->health = 1;
    }

    ent->moverState = MOVER_POS1;

    ent->takedamage = qtrue;

    ent->die = G_GlassDie;
    ent->use = G_GlassUse;
}

void SP_func_door_rotating (gentity_t *ent)
{
    int i, best, b;

    SP_func_door (ent);

    if (VectorCompare(ent->apos2, vec3_origin))
        VectorSet(ent->apos2, 0, 90, 0);

    // figure out
    for (best = 0, i = 0; i < 3; i++){
        // record angle change for later use (ie. 2-way doors)
        ent->s.angles[i] = abs(ent->apos2[i]);
        ent->apos1[i] = ent->s.apos.trBase[i];
        b = abs(ent->apos2[i]);
        ent->apos2[i] += ent->apos1[i];
        if (b > best)
                best = b;
    }

    ent->s.apos.trDuration = best * 1000 / abs(ent->speed);
    VectorCopy(ent->s.apos.trBase, ent->r.currentAngles);
    ent->blocked = Blocked_RotDoor;
    trap_LinkEntity(ent);

    if ( ent->teammaster == ent || !ent->teammaster ) {
        if(ent->damage == -999){
            trap_AdjustAreaPortalState( ent, qtrue );
            return;
        }
    }
    // if a team slave, we're done
    if (ent->flags & FL_TEAMSLAVE)
        return;

    if (ent->targetname)
        ent->think = Think_MatchTeam;
    else
        ent->think = Think_Spawn2WayRotDoorTrigger;
}
