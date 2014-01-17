/* File: effects.c */

/*
 * Special lingering spell effects.
 *
 * Copyright (c) 2007
 * Diego Gonzalez and Jeff Greene
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
 * Determine if an effect can "absorb" a second effect
 *
 * See "effect_absorb()" for the actual "absorption" code.
 *
 */
static bool effect_similar(const effect_type *x_ptr, const effect_type *x2_ptr)
{
    /*Require identical coordinates*/
    if (x_ptr->x_cur_y != x2_ptr->x_cur_y)		return (FALSE);
    if (x_ptr->x_cur_x != x2_ptr->x_cur_x)		return (FALSE);

    /* Require certain types */
    if (x_ptr->x_type != x2_ptr->x_type)		return (FALSE);
    if (x_ptr->x_type == EFFECT_TRAP_SMART)		return (FALSE);
    if (x_ptr->x_type == EFFECT_TRAP_DUMB)		return (FALSE);
    if (x_ptr->x_type == EFFECT_TRAP_PLAYER)	return (FALSE);
    if (x_ptr->x_type == EFFECT_GLYPH)			return (FALSE);

    /* Require identical terrain types (this check also assures similar gf types) */
    if (x_ptr->x_f_idx != x2_ptr->x_f_idx)		return (FALSE);

    /* Require identical sources types */
    if (x_ptr->x_source != x2_ptr->x_source)	return (FALSE);

    /* They match, so they must be similar */
    return (TRUE);
}

/*
 * Allow one effect to "absorb" another, assuming they are similar.
 *
 * These assumptions are enforced by the "effect_similar()" code.
 */
static void effect_absorb(effect_type *x_ptr, const effect_type *x2_ptr)
{

    switch (x_ptr->x_type)
    {
        case EFFECT_PERMANENT_CLOUD:
        {
            /* Simply increase the power */
            x_ptr->x_power += x2_ptr->x_power;

            if (x_ptr->x_power > EFFECT_POWER_MAX) x_ptr->x_power = EFFECT_POWER_MAX;

            break;
        }
        case EFFECT_SHIMMERING_CLOUD:
        {
            u32b power =  (MAX(x_ptr->x_power,  1) * MAX(x_ptr->x_countdown,  1));
            u32b power2 = (MAX(x2_ptr->x_power, 1) * MAX(x2_ptr->x_countdown, 1));

            /* Add the countdown, and extract the power */
            int new_countdown = x_ptr->x_countdown + x2_ptr->x_countdown;
            if (new_countdown > MAX_UCHAR) new_countdown = MAX_UCHAR;
            x_ptr->x_countdown = (byte)new_countdown;

            x_ptr->x_power = ((power + power2) / (x_ptr->x_countdown));

            /* Calculate repeats */
            power =  (power * MAX(x_ptr->x_repeats,  1));
            power2 = (power * MAX(x_ptr->x_repeats,  1));

            /* Calc a new time to repeat */
            x_ptr->x_repeats = ((power + power2) / (x_ptr->x_countdown * x_ptr->x_power));

            if (x_ptr->x_power > EFFECT_POWER_MAX) x_ptr->x_power = EFFECT_POWER_MAX;

            break;

        }
        case EFFECT_LINGERING_CLOUD:
        {
            u32b power =  (MAX(x_ptr->x_power,  1) * MAX(x_ptr->x_countdown,  1));
            u32b power2 = (MAX(x2_ptr->x_power, 1) * MAX(x2_ptr->x_countdown, 1));

            /* Add the countdown, and extract the power */
            int new_countdown = x_ptr->x_countdown + x2_ptr->x_countdown;
            if (new_countdown > MAX_UCHAR) new_countdown = MAX_UCHAR;
            x_ptr->x_countdown = (byte)new_countdown;

            x_ptr->x_power = ((power + power2) / (x_ptr->x_countdown));

            if (x_ptr->x_power > EFFECT_POWER_MAX) x_ptr->x_power = EFFECT_POWER_MAX;

            break;
        }
        /* Oops! */
        default:
            pop_up_message_box(" Error: Unknown effect type.");

            break;
    }
}

/*
 * Place the effect in a stack of effects.
 */
static void place_effect_idx(int x_idx, int y, int x)
{
    /* Get this effect */
    effect_type *x_ptr = &x_list[x_idx];

    /*
     * Handle next_x_idx.  Traps (and glyphs) always have first priority.
     */
    if (cave_any_trap_bold(y, x))
    {
        x_ptr->next_x_idx = x_list[cave_x_idx[y][x]].next_x_idx;
        x_list[cave_x_idx[y][x]].next_x_idx = x_idx;
    }
    else
    {
        x_ptr->next_x_idx = cave_x_idx[y][x];
        cave_x_idx[y][x] = x_idx;
    }

    /* Update some CAVE_* flags */
    update_los_proj_move(y, x);

    /* Redraw grid if necessary */
    if (character_dungeon && !(x_ptr->x_flags & (EF1_HIDDEN)))
    {
        //TODO redraw the square light_spot(y, x);
    }
}




/*
 * Should always be called only after x_pop();
 */
void effect_prep(int x_idx, byte type, u16b f_idx, byte y, byte x, byte countdown,
                            byte repeats, u16b power, s16b source, u16b flags)
{
    int this_x_idx, next_x_idx;

    /* Get this effect */
    effect_type *x_ptr = &x_list[x_idx];

    /* Wipe it */
    effect_wipe(x_ptr);

    /*Fill in the data*/
    x_ptr->x_type = type;
    x_ptr->x_f_idx = f_idx;
    x_ptr->x_cur_y = y;
    x_ptr->x_cur_x = x;
    x_ptr->x_countdown = countdown;
    x_ptr->x_repeats = repeats;
    x_ptr->x_power = power;
    x_ptr->x_source = source;
    x_ptr->x_flags = flags;

    /* Scan objects in that grid for combination */
    for (this_x_idx = cave_x_idx[y][x]; this_x_idx; this_x_idx = next_x_idx)
    {
        effect_type *x2_ptr;

        /* Get the object */
        x2_ptr = &x_list[this_x_idx];

        /* Get the next object */
        next_x_idx = x2_ptr->next_x_idx;

        /* Check for combination */
        if (effect_similar(x2_ptr, x_ptr))
        {
            /* Combine the items */
            effect_absorb(x2_ptr, x_ptr);

            /*We don't need it anymore*/
            effect_wipe(x_ptr);

            /* We didn't use the slot. */
            x_cnt--;

            /* We used the last slot */
            if ((x_idx + 1) == x_max) x_max--;

            /* Result */
            return;
        }

    }

    /*Put it in the proper order*/
    place_effect_idx(x_idx, y, x);
}

/*
 * Wipe the current effect.
 * This function should be called whenever an effect is terminated.
 */
void effect_wipe(effect_type *x_ptr)
{
    /* Wipe it */
    WIPE(x_ptr, effect_type);

}

/*
 * Get and return the index of a "free" effect.  Assumes
 * an effect will be created.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b x_pop(void)
{
    int i;

    /* First attempt to recycle dead effects */
    for (i = 1; i < x_max; i++)
    {
        effect_type *x_ptr;

        /* Get the effect */
        x_ptr = &x_list[i];

        /* Skip live effect */
        if (x_ptr->x_type) continue;

        /* Count effects */
        x_cnt++;

        /* Use this effects */
        return (i);
    }

    /*
     * We didn't find an empty slot
     * Initial allocation
     */
    if (x_max < z_info->x_max)
    {
        /* Get next space */
        i = x_max;

        /* Expand effect array */
        x_max++;

        /* Count effects */
        x_cnt++;

        /* Use this effect */
        return (i);
    }

    /* Oops */
    return (0);
}

