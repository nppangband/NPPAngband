/* File: was melee2.c */

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
 */

#include "src/npp.h"

/*
 * Can the monster exist in this grid?
 *
 * Because this function is designed for use in monster placement and
 * generation as well as movement, it cannot accept monster-specific
 * data, but must rely solely on racial information.
 */
bool cave_exist_mon(const monster_race *r_ptr, int y, int x, bool occupied_ok, bool damage_ok, bool can_dig)
{
    feature_type *f_ptr;

    /* Check Bounds */
    if (!in_bounds(y, x)) return (FALSE);

    /* Check location */
    f_ptr = &f_info[dungeon_info[y][x].feat];

    /* The grid is already occupied. */
    if (dungeon_info[y][x].has_monster())
    {
        if (!occupied_ok) return (FALSE);
    }

    /* Glyphs -- must break first */
    if (cave_player_glyph_bold(y, x)) return (FALSE);

    /* Permanent walls are never OK */
    if (_feat_ff1_match(f_ptr, FF1_MOVE | FF1_PERMANENT) ==
        (FF1_PERMANENT)) return (FALSE);

    /*** Check passability of various features. ***/

    /* Feature is a wall */
    if (!cave_passable_bold(y, x))
    {
        /* Monster isn't allowed to enter */
        if (!can_dig) return (FALSE);

        /* Handle creatures who can go through walls */
        if ((r_ptr->flags2 & (RF2_KILL_WALL)) ||
            (r_ptr->flags2 & (RF2_PASS_WALL)))
        {
            /* Monster is not going there by choice */
            if (damage_ok) return (TRUE);

            /* Check to see if monster wants to go there */
            if (cave_no_dam_for_mon(y, x, r_ptr)) return (TRUE);

            else return (FALSE);

        }
        else return (FALSE);
    }

    /* Monster is not going there by choice */
    if (damage_ok) return (TRUE);

    /* Check to see if monster wants to go there */
    if (cave_no_dam_for_mon(y, x, r_ptr)) return (TRUE);

    /* Flying monsters can pass through dangerous terrain */
    if (r_ptr->flags3 & (RF3_FLYING)) return (TRUE);

    /*Monster will be damaged going there*/
    return (FALSE);

}
