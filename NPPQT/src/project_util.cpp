
/* File: project_util.cpp */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * 						Jeff Greene, Diego Gonzalez
 *
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.




/************************************************************************
 *                                                                      *
 *                           Projection types                           *
 *                                                                      *
 ************************************************************************/

#include "src/npp.h"

/*
 * Handle bolt spells.
 *
 * Bolts stop as soon as they hit a monster, whiz past missed targets, and
 * (almost) never affect items on the floor.
 */
bool project_bolt(int who, int rad, int y0, int x0, int y1, int x1, int dam,
                  int typ, u32b flg)
{
    /* Add the bolt bitflags */
    flg |= PROJECT_STOP | PROJECT_KILL | PROJECT_THRU | PROJECT_EFCT;

    /* Hurt the character unless he controls the spell */
    if (who != SOURCE_PLAYER) flg |= PROJECT_PLAY;

    /* Limit range */
    if ((rad > MAX_RANGE) || (rad <= 0)) rad = MAX_RANGE;

    /* Cast a bolt */
    return (project(who, rad, y0, x0, y1, x1, dam, typ, flg, 0, 0));
}

/*
 * Handle beam spells.
 *
 * Beams affect every grid they touch, go right through monsters, and
 * (almost) never affect items on the floor.
 */
bool project_beam(int who, int rad, int y0, int x0, int y1, int x1, int dam,
                  int typ, u32b flg)
{
    /* Add the beam bitflags */
    flg |= PROJECT_BEAM | PROJECT_KILL | PROJECT_THRU | PROJECT_EFCT;

    /* Hurt the character unless he controls the spell */
    if (who != SOURCE_PLAYER) flg |= (PROJECT_PLAY);

    /* Limit range */
    if ((rad > MAX_RANGE) || (rad <= 0)) rad = MAX_RANGE;

    /* Cast a beam */
    return (project(who, rad, y0, x0, y1, x1, dam, typ, flg, 0, 0));
}


/*
 * Handle ball spells.
 *
 * Balls act like bolt spells, except that they do not pass their target,
 * and explode when they hit a monster, a wall, their target, or the edge
 * of sight.  Within the explosion radius, they affect items on the floor.
 *
 * Balls may jump to the target, and have any source diameter (which affects
 * how quickly their damage falls off with distance from the center of the
 * explosion).
 */
bool project_ball(int who, int rad, int y0, int x0, int y1, int x1, int dam,
                  int typ, u32b flg, int source_diameter)
{
    /* Add the ball bitflags */
    flg |= PROJECT_BOOM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL |
        PROJECT_WALL | PROJECT_EFCT;

    /* Add the STOP flag if appropriate */
    if ((who == SOURCE_PLAYER) &&
        (!target_okay() || y1 != p_ptr->target_row || x1 != p_ptr->target_col))
    {
        flg |= (PROJECT_STOP);
    }

    /* Hurt the character unless he controls the spell */
    if (who != SOURCE_PLAYER) flg |= (PROJECT_PLAY);
    /*Hack - poison cloud poison spells have a lingering cloud */
    else if (typ == GF_POIS)
    {
        if (game_mode != GAME_NPPMORIA) flg |= (PROJECT_CLOUD);
    }

    /* Limit radius to nine (up to 256 grids affected) */
    if (rad > 9) rad = 9;

    /* Cast a ball */
    return (project(who, rad, y0, x0, y1, x1, dam, typ, flg,
                    0, source_diameter));
}

/*
 * Handle ball spells that explode immediately on the target.
 * Whether monsters or the player is hurt must be determined
 * by the code that calls this function.
 */
bool explosion(int who, int rad, int y0, int x0, int dam, int typ, u32b flg)
{
    /* Add the explosion bitflags */
    flg |= 	PROJECT_BOOM | PROJECT_GRID | PROJECT_JUMP | PROJECT_ITEM;

    /* Explode */
    return (project_ball(who, rad, y0, x0, y0, x0, dam, typ, flg, 0));
}

/*
 * Handle monster-centered explosions.
 */
bool mon_explode(int who, int rad, int y0, int x0, int dam, int typ)
{
    return (project_ball(who, rad, y0, x0, y0, x0, dam, typ, 0L, 20));
}


/*
 * Handle arc spells.
 *
 * Arcs are a pie-shaped segment (with a width determined by "degrees")
 * of a explosion outwards from the source grid.  They are centered
 * along a line extending from the source towards the target.  -LM-
 *
 * Because all arcs start out as being one grid wide, arc spells with a
 * value for degrees of arc less than (roughly) 60 do not dissipate as
 * quickly.  In the extreme case where degrees of arc is 0, the arc is
 * actually a defined length beam, and loses no strength at all over the
 * ranges found in the game.
 *
 * Arcs affect items on the floor.
 */
bool project_arc(int who, int rad, int y0, int x0, int y1, int x1, int dam,
                 int typ, u32b flg, int degrees)
{
    /* Diameter of source of energy is normally, but not always, 20. */
    int source_diameter = 20;

    /* Radius of zero means no fixed limit. */
    if (rad == 0) rad = MAX_SIGHT;

    /* Calculate the effective diameter of the energy source, if necessary. */
    if (degrees < ARC_STANDARD_WIDTH)
    {
        if (degrees <= 9) source_diameter = rad * 10;
        else source_diameter = source_diameter * ARC_STANDARD_WIDTH / degrees;
    }

    /* If the arc has no spread, it's actually a beam */
    if (degrees <= 0)
    {
        /* Add the beam bitflags */
        flg |= (PROJECT_BEAM | PROJECT_KILL | PROJECT_THRU | PROJECT_WALL);

        source_diameter = 0;
    }

    /* If a full circle is asked for, we cast a ball spell. */
    else if (degrees >= 360)
    {
        /* Add the ball bitflags */
        flg |= PROJECT_STOP | PROJECT_BOOM | PROJECT_GRID |
               PROJECT_ITEM | PROJECT_KILL | PROJECT_WALL;

        source_diameter = 0;
    }

    /* Otherwise, we fire an arc */
    else
    {
        /* Add the arc bitflags */
        flg |= PROJECT_ARC  | PROJECT_BOOM | PROJECT_GRID |
               PROJECT_ITEM | PROJECT_KILL | PROJECT_THRU | PROJECT_EFCT;
    }

    /* Hurt the character unless he controls the spell */
    if (who != SOURCE_PLAYER) flg |= (PROJECT_PLAY);

    /* Cast an arc (or a ball) */
    return (project(who, rad, y0, x0, y1, x1, dam, typ, flg, degrees,
                    (byte)source_diameter));
}

/*
 * Handle starburst spells.
 *
 * Starbursts are randomized balls that use the same sort of code that
 * governs the shape of starburst rooms in the dungeon.  -LM-
 *
 * Starbursts always do full damage to every grid they affect:  however,
 * the chances of affecting grids drop off significantly towards the
 * edge of the starburst.  They always "jump" to their target and affect
 * items on the floor.
 */
bool project_star(int who, int rad, int y0, int x0, int dam, int typ, u32b flg)
{
    /* Add the star bitflags */
    flg |= PROJECT_STAR | PROJECT_BOOM | PROJECT_GRID | PROJECT_JUMP |
           PROJECT_ITEM | PROJECT_KILL | PROJECT_EFCT;

    /* Hurt the character unless he controls the spell */
    if (who != SOURCE_PLAYER) flg |= (PROJECT_PLAY);

    /* Cast a star */
    return (project(who, rad, y0, x0, y0, x0, dam, typ, flg, 0, 0));
}

/*
 * Handle target grids for projections under the control of
 * the character.  - Chris Wilde, Morgul
 */
static void adjust_target(int dir, int y0, int x0, int *y1, int *x1)
{
    /* If no direction is given, and a target is, use the target. */
    if ((dir == 5) && target_okay())
    {
        *y1 = p_ptr->target_row;
        *x1 = p_ptr->target_col;
    }

    /* Otherwise, use the given direction */
    else
    {
        *y1 = y0 + MAX_RANGE * ddy[dir];
        *x1 = x0 + MAX_RANGE * ddx[dir];
    }
}



/*
 * Apply a "project()" directly to all monsters in view of a certain spot.
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 *
 * We are able to check LOS from either the character (in which case we
 * use line of fire for speed and accuracy), or from any given grid.
 *
 * To avoid misbehavior when monster deaths have side-effects,
 * this is done in two passes. -- JDL
 */
bool project_los(int y0, int x0, int dam, int typ)
{
    int i, d, x, y;

    u32b flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

    bool obvious = FALSE;

    int who;

    /* Determine whether we are using LOF or LOS */
    bool line_of_fire = FALSE;

    if ((y0 == p_ptr->py) && (x0 == p_ptr->px))
    {
        line_of_fire = TRUE;
        who = SOURCE_PLAYER;
    }
    else if (cave_monster_trap_bold(y0, x0)) who = SOURCE_TRAP;
    else if (cave_player_trap_bold(y0, x0))  who = SOURCE_EFFECT;
    else who = SOURCE_OTHER;

    /* Mark monsters in LOS */
    for (i = 1; i < mon_max; i++)
    {
        monster_type *m_ptr = &mon_list[i];

        /* Paranoia -- Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Location */
        y = m_ptr->fy;
        x = m_ptr->fx;

        /* Apply character-centered test */
        if (line_of_fire)
        {
            /* Require line of fire */
            if (!player_can_fire_bold(y, x)) continue;
        }

        /* Apply generic grid test */
        else
        {
            /* Get distance between source and monster */
            d = distance(y0, x0, y, x);

            /* LOS extends only to max sight range */
            if (d > MAX_RANGE) continue;

            /* Check LOS if not at grid or adjacent */
            if (d > 1)
            {
                /* Ignore if not in LOS */
                if (!los(y0, x0, y, x)) continue;
            }
        }

        /* Mark the monster */
        m_ptr->mflag |= (MFLAG_TEMP);
    }

    /* Affect all (nearby) monsters */
    for (i = 1; i < mon_max; i++)
    {
        monster_type *m_ptr = &mon_list[i];

        /* Skip unmarked monsters */
        if (!(m_ptr->mflag & (MFLAG_TEMP))) continue;

        /* Remove mark */
        m_ptr->mflag &= ~(MFLAG_TEMP);

        /* Jump directly to the monster */
        if (project_m(who, m_ptr->fy, m_ptr->fx, dam, typ, flg))
        {
            obvious = TRUE;
        }
    }

    /* Result */
    return (obvious);
}


/*
 * This routine clears the entire "temp" set.
 */
void clear_temp_array(void)
{
    int i;

    /* Apply flag changes */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* No longer in the array */
        dungeon_info[y][x].cave_info &= ~(CAVE_TEMP);
    }

    /* None left */
    temp_n = 0;
}


/*
 * Aux function -- see below
 */
void cave_temp_mark(int y, int x, bool room)
{
    if(!in_bounds_fully(y, x)) return;

    /* Avoid infinite recursion */
    if (dungeon_info[y][x].cave_info & (CAVE_TEMP)) return;

    /* Option -- do not leave the current room */
    if ((room) && (!(dungeon_info[y][x].cave_info & (CAVE_ROOM)))) return;

    /* Verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid */
    dungeon_info[y][x].cave_info |= (CAVE_TEMP);

    /* Add it to the marked set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}

/*
 * Mark the nearby area with CAVE_TEMP flags.  Allow limited range.
 */
void spread_cave_temp(int y1, int x1, int range, bool room, bool pass_walls)
{
    int i, y, x;

    /* Add the initial grid */
    cave_temp_mark(y1, x1, room);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get marked, but stop further spread, unless pass_walls is TRUE */
        if (!pass_walls && !cave_project_bold(y, x)) continue;

        /* Note limited range (note:  we spread out one grid further) */
        if ((range) && (distance(y1, x1, y, x) >= range)) continue;

        /* Spread adjacent */
        cave_temp_mark(y + 1, x, room);
        cave_temp_mark(y - 1, x, room);
        cave_temp_mark(y, x + 1, room);
        cave_temp_mark(y, x - 1, room);

        /* Spread diagonal */
        cave_temp_mark(y + 1, x + 1, room);
        cave_temp_mark(y - 1, x - 1, room);
        cave_temp_mark(y - 1, x + 1, room);
        cave_temp_mark(y + 1, x - 1, room);
    }
}



/*
 * Character casts a special-purpose bolt or beam spell.
 */
bool fire_bolt_beam_special(int typ, int dir, int dam, int rad, u32b flg)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* This is a beam spell */
    if (flg & (PROJECT_BEAM))
    {
        /* Cast a beam */
        return (project_beam(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam,
                            typ, flg));
    }

    /* This is a bolt spell */
    else
    {
        /* Cast a bolt */
        return (project_bolt(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam,
                                    typ, flg));
    }
}

/*
 * Player casts a orb spell that creates an effect, but does not affect anything else.
 */
bool fire_effect_orb(int typ, int dir, int dam, int rad)
{
    int y1, x1;
    u32b flg = 0L;

    /* Add the ball bitflags */
    flg |= (PROJECT_BOOM | PROJECT_WALL | PROJECT_EFCT | PROJECT_CLOUD);

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Add the STOP flag if appropriate */
    if (!target_okay() || y1 != p_ptr->target_row || x1 != p_ptr->target_col)
    {
        flg |= (PROJECT_STOP);
    }

    /* Limit radius to nine (up to 256 grids affected) */
    if (rad > 9) rad = 9;

    /* Cast a ball */
    return (project(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ, flg, 0L, 10 + rad * 10));
}

/*
 * Character casts a (simple) ball spell.
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast a (simple) ball */
    return (project_ball(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ,
                         0L, 0));
}

/*
 * Character casts an orb spell (a ball that loses no strength out
 * from the origin).
 */
bool fire_orb(int typ, int dir, int dam, int rad)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast an orb */
    return (project_ball(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ,
                         0L, 10 + rad * 10));
}

/*
 * Character casts a ball spell with a specified source diameter, that
 * jumps to the target, or does various other special things.
 */
bool fire_ball_special(int typ, int dir, int dam, int rad, u32b flg,
                       int source_diameter)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast a ball with specified source diameter */
    return (project_ball(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ,
                         flg, source_diameter));
}

/*
 * Character casts an arc spell.
 */
bool fire_arc_special(int typ, int dir, int dam, int rad, int degrees, u32b flg)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast an arc */
    return (project_arc(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ,
                        flg, degrees));
}

/*
 * Character casts an arc spell.
 */
bool fire_arc(int typ, int dir, int dam, int rad, int degrees)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast an arc */
    return (project_arc(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam, typ,
                        0L, degrees));
}

/*
 * Character casts a star-shaped spell.
 */
bool fire_star(int typ, int dam, int rad, u32b flg)
{
    /* Cast a star */
    return (project_star(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, dam, typ, flg));
}


/*
 * Fire a number of bolts, beams, or arcs that start in semi-random grids
 * near the character, and head in totally random directions.  The larger
 * the number of grids in the area of fire, and the more monsters inhabit
 * those grids, the more effective this spell is.
 * -LM-
 */
void fire_storm(int who, int typ0, int y0, int x0, int dam, int rad, int len,
    byte projection, bool lingering)
{
    int i, j;
    int y, x = 0, y1, x1, last_y, last_x;
    int dir;
    int typ;
    long num_missiles;
    int choice;
    monster_type *m_ptr;

    /* Save standard delay */
    int std_delay = op_ptr->delay_factor;

    /* Array of grids (max radius is 20) */
    u16b grid[1681];

    /* Grid count */
    int grid_count = 0;

    /* Array of monsters to hurt (indexes, initial HPs) */
    s16b mon_array[100][2];

    /* Monster count */
    int mon_count = 0;

    /* Allow spell graphics to accumulate */
    u32b flg = (lingering ? PROJECT_NO_REDRAW : 0L);


    /* We can't handle a radius of more than 20 */
    if (rad > 20) rad = 20;

    /* Very little delay while projecting each missile */
    op_ptr->delay_factor = (std_delay + 1) / 2;


    /* Build up an array of all nearby projectable grids */
    for (y = y0 - rad; y <= y0 + rad; y++)
    {
        for (x = x0 - rad; x <= x0 + rad; x++)
        {
            /* Stay legal */
            if (!in_bounds(y, x)) continue;

            /* Require that grid be projectable */
            if (projectable(y0, x0, y, x, PROJECT_NONE))
            {
                /* Convert location to a grid, save and count it */
                grid[grid_count++] = GRID(y, x);
            }
        }
    }


    /* Scan the monster list */
    for (i = 0; i < mon_max; i++)
    {
        /* Get this monster */
        m_ptr = &mon_list[i];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Skip monsters not in LOF (or LOS), and if too far away */
        if ((y0 == p_ptr->py) && (x0 == p_ptr->px))
        {
            if (!player_can_fire_bold(m_ptr->fy, m_ptr->fx)) continue;
            if (m_ptr->cdis > rad) continue;
        }
        else
        {
            if (!los(y0, x0, m_ptr->fy, m_ptr->fx)) continue;
            if (distance(y0, x0, m_ptr->fy, m_ptr->fx) > rad) continue;
        }

        /* Store this monster and its current HPs */
        if (mon_count < 100)
        {
            mon_array[mon_count][0] = i;
            mon_array[mon_count][1] = m_ptr->hp;
            mon_count++;
        }
    }


    /* Calculate the minimum number of missiles */
    num_missiles = MAX(1L, grid_count / 8);

    /* Handle each missile in turn */
    for (i = 0;; i++)
    {
        /* Limit -- never fire more than num_missiles * 8 */
        if (i > num_missiles * 8) break;

        /* We've used up our guaranteed missiles */
        if (i >= num_missiles)
        {
            /* Assume we stop */
            bool stop = TRUE;

            /* Keep firing until all monsters have been hurt */
            for (j = 0; j < mon_count; j++)
            {
                /* Get this monster */
                m_ptr = &mon_list[mon_array[j][0]];

                /* Skip dead monsters */
                if (!m_ptr->r_idx) continue;

                /* Skip monsters with HPs < initial value */
                if (m_ptr->hp < mon_array[j][1]) continue;

                /* This monster hasn't been hurt - keep firing */
                stop = FALSE;
                break;
            }

            /* Stop if all monsters have been hurt */
            if (stop) break;
        }


        /* Start with a very far away location */
        last_y = -255;
        last_x = -255;

        /* Bias for closer grids */
        for (j = 0; j < 3; j++)
        {
            /* Choose a grid at random */
            choice = rand_int(grid_count);

            /* Get the coordinates */
            y = GRID_Y(grid[choice]);
            x = GRID_X(grid[choice]);

            /* Save if less than previous distance */
            if (distance(y, x, y0, x0) < distance(last_x, last_x, y0, x0))
            {
                /* Save these coordinates */
                last_y = y;
                last_x = x;
            }
        }

        /* No movement */
        dir = 5;

        /* Get any direction other than 5 */
        while (dir == 5) dir = randint(9);

        /* Get target grid */
        y1 = last_y + ddy[dir];
        x1 = last_x + ddx[dir];


        /* Allow wizardly projection types */
        if (typ0 == -1)
        {
            choice = rand_int(12);

            if      (choice ==  1) typ = GF_FIRE;
            else if (choice ==  2) typ = GF_COLD;
            else if (choice ==  3) typ = GF_ACID;
            else if (choice ==  4) typ = GF_ELEC;
            else if (choice ==  5) typ = GF_POIS;
            else if (choice ==  6) typ = GF_LIGHT;
            else if (choice ==  7) typ = GF_DARK;
            else if (choice ==  8) typ = GF_NEXUS;
            else if (choice ==  9) typ = GF_CONFUSION;
            else if (choice == 10) typ = GF_SOUND;
            else if (choice == 11) typ = GF_SHARD;
            else                   typ = GF_CHAOS;
        }

        /* Allow light, darkness, and confusion */
        else if (typ0 == -2)
        {
            choice = rand_int(3);

            if      (choice == 1) typ = GF_LIGHT;
            else if (choice == 2) typ = GF_DARK;
            else                  typ = GF_CONFUSION;
        }

        /* Use given projection */
        else
        {
            typ = typ0;
        }

        /* Fire a projection using the calculated data */
        if (projection == 0)
        {
            (void)project_bolt(who, len, last_y, last_x, y1, x1, dam, typ, flg);
        }
        else if (projection == 1)
        {
            (void)project_beam(who, len, last_y, last_x, y1, x1, dam, typ, flg);
        }
        else if (projection == 2)
        {
            /* Used for the "Prismatic Armageddon" spell */
            (void)project_arc(who, rand_range(len - 1, len + 1), last_y, last_x,
                y1, x1, dam, typ, flg, rand_range(40, 55));
        }
        else if (projection == 3)
        {
            (void)project_ball(who, rad, y1, x1, y1, x1, dam, typ, flg, 0);
        }
    }

    /* We allowed spell graphics to accumulate */
    if (lingering)
    {

        // TODO int ty = Term->offset_y + SCREEN_HGT;
       // TODO  int tx = Term->offset_x + SCREEN_WID;

        /* Clear all lingering spell effects on screen XXX */
        // TODO for (y = Term->offset_y; y < ty; y++)
        {
            // TODO for (y = Term->offset_x; x < tx; x++)
            {
                // TODO light_spot(y, x);
            }
        }
    }

    /* Restore standard delay */
    op_ptr->delay_factor = std_delay;
}


/*
 * Fire beams in random directions.
 */
bool beam_burst(int y, int x, int typ, int num, int dam)
{
    int i, yy, xx;

    bool notice = FALSE;

    int old_delay = op_ptr->delay_factor;

    /* Require legal centerpoint */
    if (!in_bounds_fully(y, x)) return (FALSE);


    /* Hack -- lower delay factor */
    if (op_ptr->delay_factor)
    {
        op_ptr->delay_factor = (op_ptr->delay_factor + 1) / 2;
    }

    /* Fire beams in all directions */
    for (i = 0; i < num; i++)
    {
        /* Get a totally random grid within six grids from current position */
        yy = rand_spread(y, 6);
        xx = rand_spread(x, 6);

        /* Fire a beam of (strong) light towards it */
        if (project(-1, 0, y, x, yy, xx, dam, typ,
            PROJECT_BEAM | PROJECT_KILL | PROJECT_EFCT, 0, 0)) notice = TRUE;
    }

    /* Restore standard delay */
    op_ptr->delay_factor = old_delay;

    /* Return "anything noticed" */
    return (notice);
}

/*
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
bool fire_swarm(int num, int typ, int dir, int dam, int rad)
{
    bool noticed = FALSE;

    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    while (num--)
    {
        /* Analyze the "dir" and the "target".  Hurt items on floor. */
        if (project_ball(SOURCE_PLAYER, rad, p_ptr->py, p_ptr->px, y1, x1, dam,
                typ, 0L, 0)) noticed = TRUE;
    }

    return noticed;
}


/*
 * Character casts a bolt spell.
 */
bool fire_bolt(int typ, int dir, int dam)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast a bolt */
    return (project_bolt(SOURCE_PLAYER, MAX_RANGE, p_ptr->py, p_ptr->px, y1, x1, dam,
                         typ, 0L));
}

/*
 * Character casts a beam spell.
 */
bool fire_beam(int typ, int dir, int dam, u32b flg)
{
    int y1, x1;

    /* Get target */
    adjust_target(dir, p_ptr->py, p_ptr->px, &y1, &x1);

    /* Cast a beam */
    return (project_beam(-1, MAX_RANGE, p_ptr->py, p_ptr->px, y1, x1, dam,
                         typ, flg));
}


/*
 * Cast a bolt or a beam spell
 */
bool fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (rand_int(100) < prob)
    {
        return (fire_beam(typ, dir, dam, 0L));
    }
    else
    {
        return (fire_bolt(typ, dir, dam));
    }
}


/*
 * Cast a chain of beams of the given type, jumping through nearby monsters. Monsters
 * must be inside the player's LOF.
 * max_hits is the maximum number of beams that will be casted.
 * decrement is a percentage of damage that is decreased with every beam. Use 0
 * to keep the damage constant.
 * Return TRUE if at least one monster was hit.
 */
bool beam_chain(int gf_type, int dam, int max_hits, int decrement)
{
    int yy, xx;

    /* Must be from from player's location */
    int py = p_ptr->py;
    int px = p_ptr->px;

    int y = py;
    int x = px;

    int i, k;

    /* The indexes in mon_list of the reached monsters */
    u16b *monsters;
    u16b *hits;
    u16b n = 0;

    bool flag = FALSE;

    hits = C_ZNEW(mon_max, u16b);
    monsters = C_ZNEW(mon_max, u16b);

    /* Cast max_hits beams */
    for (i = 0; i < max_hits; i++)
    {
        int m_idx;
        u16b m = 0;

        /* Scan monsters as potential targets */

        for (m_idx = 0; m_idx < mon_max; m_idx++)
        {
            monster_type *m_ptr = &mon_list[m_idx];

            /* Paranoia -- Skip dead monsters */
            if (!m_ptr->r_idx) continue;

            /* It must be visible */
            if (!m_ptr->ml) continue;

            /* Get monster coordinates */
            yy = m_ptr->fy;
            xx = m_ptr->fx;

            /* It must be in the line of fire of the player */
            if(!m_ptr->project) continue;

            /* It must be in the line of fire of the previous location */
            if(!projectable(y, x, yy, xx, PROJECT_NONE)) continue;

            /* It must be close enough to the previous location */
            if (distance(y, x, yy, xx) > MAX_RANGE) continue;

            /* Find the monster in the list */
            for (k = 0; k < n; k++)
            {
                /* Found. Stop */
                if (hits[k] == m_idx) break;
            }

            /* If the monster was found in the list we just ignore it */
            if (k < n) continue;

            /* Mark the monster as a possible candidate */
            monsters[m++] = m_idx;
        }

        /* No monsters. Done */
        if (!m) break;

        /* Select a random monster from the list */
        m_idx = monsters[rand_int(m)];

        /* Get its location */
        yy = mon_list[m_idx].fy;
        xx = mon_list[m_idx].fx;

        /* Remember the monster */
        hits[n++] = m_idx;

        /* Cast the beam */
        project(SOURCE_PLAYER, 0, y, x, yy, xx, dam, gf_type,
            (PROJECT_KILL | PROJECT_BEAM), 0, 0);

        /* Success */
        flag = TRUE;

        /* Make the next beam weaker */
        dam -= (decrement * dam) / 100;

        /* No damage. Done */
        if (dam < 1) break;

        /* Remember the last location */
        y = yy;
        x = xx;

    }

    FREE(hits);
    FREE(monsters);

    return (flag);
}

