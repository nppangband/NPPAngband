/* File: dun_classes.c */

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

//Verify if the dungeon square has an object on it
bool dungeon_type::has_object()
{
    if (object_idx) return (TRUE);
    return (FALSE);
}

//Verify if the dungeon square has an effect on it
bool dungeon_type::has_effect()
{
    if (effect_idx) return (TRUE);
    return (FALSE);
}

//Verify if the dungeon square has a monster object on it
bool dungeon_type::has_monster()
{
    if (monster_idx) return (TRUE);
    return (FALSE);
}


// Manually wipe the effect.
void effect_type::effect_wipe()
{
    x_type = 0;
    x_f_idx = 0;
    x_cur_y = 0;
    x_cur_x = 0;
    x_countdown = 0;
    x_repeats = 0;
    x_power = 0;
    x_source = 0;
    x_flags = 0;
    next_x_idx = 0;
    x_r_idx = 0;
}
