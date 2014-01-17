/* File: cave.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *                    Jeff Greene, Diego Gonzalez
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
 */

#include "src/npp.h"

/*
 * Approximate distance between two points.
 *
 * When either the X or Y component dwarfs the other component,
 * this function is almost perfect, and otherwise, it tends to
 * over-estimate about one grid per fifteen grids of distance.
 *
 * Algorithm: hypot(dy,dx) = max(dy,dx) + min(dy,dx) / 2
 */
int distance(int y1, int x1, int y2, int x2)
{
    int ay, ax;

    /* Find the absolute y/x distance components */
    ay = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    ax = (x1 > x2) ? (x1 - x2) : (x2 - x1);

    /* Hack -- approximate the distance */
    return ((ay > ax) ? (ay + (ax>>1)) : (ax + (ay>>1)));
}


/*
 * Set or unset the CAVE_LOS, CAVE_PROJECT and CAVE_MOVE flags of the given
 * grid depending on the terrain feature and effects contained in such grid.
 * This function is very IMPORTANT. It must be called whenever the content of
 * a grid changes (cave_set_feat_aux), we add/remove effects or we load a
 * savefile.
 *
 * CAVE_LOS controls the CAVE_VIEW and CAVE_SEEN flags.
 * CAVE_PROJECT controls the CAVE_FIRE flag.
 * CAVE_MOVE controls monster/player movement and noise flows.
 *
 * Note on CAVE_MOVE. This flag is actually equal to FF1_MOVE plus effects.
 * Some places in the sources still need to check the presence of the FF1_MOVE
 * flag sans effects. You'll see checks for CAVE_MOVE or FF1_MOVE depending
 * on the context.
 */
void update_los_proj_move(int y, int x)
{
    feature_type *f_ptr = &f_info[cave_feat[y][x]];
    u16b x_idx;
    u32b mask = 0;
    u32b old_flags;

    /* Save the old flags */
    old_flags = cave_info[y][x];

    /* Paranoia */
    old_flags &= (CAVE_LOS | CAVE_PROJECT | CAVE_MOVE);

    /* Turn off those flags by default */
    cave_info[y][x] &= ~(CAVE_LOS | CAVE_PROJECT | CAVE_MOVE);

    /* Feature allows LOS */
    if (_feat_ff1_match(f_ptr, FF1_LOS)) mask |= (CAVE_LOS);

    /* Feature allows projections */
    if (_feat_ff1_match(f_ptr, FF1_PROJECT)) mask |= (CAVE_PROJECT);

    /* Feature allows movement */
    if (_feat_ff1_match(f_ptr, FF1_MOVE)) mask |= (CAVE_MOVE);

    /* Check effects if necessary */
    x_idx = cave_x_idx[y][x];

    /* Traverse the effects applied to that grid */
    while (x_idx)
    {
        /* Get the effect */
        effect_type *x_ptr = &x_list[x_idx];

        /* Get the next effect (for the next iteration) */
        x_idx = x_ptr->next_x_idx;

        /* Ignore certain effects */
        if (!(x_ptr->x_f_idx) || (x_ptr->x_flags & (EF1_HIDDEN))) continue;

        /* Get the pseudo-feature associated to the effect */
        f_ptr = &f_info[x_ptr->x_f_idx];

        /* Disable los if necessary */
        if (!_feat_ff1_match(f_ptr, FF1_LOS)) mask &= ~(CAVE_LOS);

        /* Disable projections if necessary */
        if (!_feat_ff1_match(f_ptr, FF1_PROJECT)) mask &= ~(CAVE_PROJECT);

        /* Disable movement if necessary */
        if (!_feat_ff1_match(f_ptr, FF1_MOVE)) mask &= ~(CAVE_MOVE);
    }

    /* Turn on the collected flags */
    cave_info[y][x] |= mask;

    /* Request view update if necessary */
    if ((old_flags & (CAVE_LOS | CAVE_PROJECT)) !=
        (mask & (CAVE_LOS | CAVE_PROJECT)))
    {
        /* Update the visuals */
        p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
    }

    /* Request flow update if necessary */
    if ((old_flags & (CAVE_MOVE)) != (mask & (CAVE_MOVE)))
    {
        /* Full update of the flows */
        p_ptr->update |= (PU_FLOW_DOORS | PU_FLOW_NO_DOORS);
    }
}

