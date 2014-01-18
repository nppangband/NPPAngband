/* File: feature.c */

/*
 * Copyright (c) 2006 Jeff Greene, Diego Gonzalez
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
 * Some dynamic features use a timer. Returns the value of that timer.
 * Every tick is equal to one call to process_dynamic_terrain.
 * Returns 0 if the feature isn't timed.
 */
static byte calculate_turn_count(u16b feat)
{
    /* Growing trees */
    if (feat == FEAT_FSOIL_DYNAMIC)
    {
        return (25 + rand_int(30));
    }

    /* Geysers */
    if (feat == FEAT_GEYSER)
    {
        return (20 + rand_int(50));
    }

    /* Sniper flowers */
    if (feat == FEAT_PUTRID_FLOWER)
    {
        return (15 + rand_int(20));
    }

    /* Silent watchers */
    if (feat == FEAT_SILENT_WATCHER)
    {
        return (5 + rand_int(10));
    }

    /* Default */
    return (0);
}

/*
 * Add a new grid to the dyna_g array given its coordinates.
 * Returns TRUE on success
 */
bool add_dynamic_terrain(byte y, byte x)
{
    int i;
    dynamic_grid_type *g_ptr;

    /* We don't have more space in dyna_g */
    if (dyna_cnt >= DYNA_MAX)
    {
        /*
         * Turn on dyna_full. From now on and until the generation of
         * a new level, the contents of dyna_g will be updated every
         * certain number of steps.
         */
        dyna_full = TRUE;

        /* Hack - Force the rebuild of dyna_g */
        dyna_center_y = 255;
        dyna_center_x = 255;

        /* Failure */
        return (FALSE);
    }

    /*
     * dyna_full is on, and the grid is too far away from the player.
     * Ignore that grid.
     */
    if (dyna_full && character_dungeon &&
        (distance(y, x, p_ptr->py, p_ptr->px) > MAX_SIGHT))
    {
        /* Failure */
        return (FALSE);
    }

    /* We can add the grid at the end of dyna_g without problems */
    if (dyna_next < DYNA_MAX)
    {
        i = dyna_next++;
    }
    /* We reached the end of dyna_g. Find a gap between entries */
    else
    {
        /* Scan dyna_g */
        for (i = 0; i < DYNA_MAX; i++)
        {
            /* We found an unused entry */
            if (!(dyna_g[i].flags & (DF1_OCCUPIED))) break;
        }
    }

    /* Get the new entry */
    g_ptr = &dyna_g[i];

    /* Fill in the grid info */
    g_ptr->y = y;
    g_ptr->x = x;
    g_ptr->flags = (DF1_OCCUPIED | DF1_NEW_BORN);
    g_ptr->counter = calculate_turn_count(cave_feat[y][x]);

    /* One grid more */
    ++dyna_cnt;

    /* Success */
    return (TRUE);
}


/*
 * Clear level_flag and then rescan the current level searching for elemental features.
 * Update level_flag for each element found.
 */
void update_level_flag(void)
{
    int y, x;

    /* Reset the global flags */
    level_flag = 0;

    /* Debug */
    if (cheat_room)
    {
        message("Updating level flags.");
        // TODO disturb(0, 0);
    }

    /* Scan the dungeon */
    for (y = 1; y < (p_ptr->cur_map_hgt - 1); y++)
    {
        for (x = 1; x < (p_ptr->cur_map_wid - 1); x++)
        {
            /* Cache the feature */
            u16b feat = cave_feat[y][x];

            /* Is it an elemental feature? */
            if (feat_ff3_match(feat, TERRAIN_MASK))
            {
                /* Update the global flags */
                level_flag |= get_level_flag(feat);
            }
        }
    }
}


/*
 * Return ONLY ONE of the LF1_* flags that represents the given feature.
 * Return 0 if there isn't none.
 */
u32b get_level_flag(u16b feat)
{
    /* Get the elemental flags */
    u32b element_flags = feat_ff3_match(feat, TERRAIN_MASK);

    /* Analyze the type of the flags */
    switch (element_flags)
    {
        /* Special case. Boiling mud (hybrid element) */
        case ELEMENT_BMUD:
        {
            /* This flag actually doesn't match any of the ELEMENT_* flags */
            return (LF1_BMUD);
        }
        /* Special case. Boiling water (hybrid element) */
        case ELEMENT_BWATER:
        {
            /* This flag actually doesn't match any of the ELEMENT_* flags */
            return (LF1_BWATER);
        }
        /* Just don't do anything for other flags */
        default:
        {
            return (element_flags);
        }

    }
}


/*
 * Return ALL the LF1_* flags that represents the nativity settings of the
 * given monster race.
 * Return 0 if there isn't none.
 */
u32b get_level_flag_from_race(monster_race *r_ptr)
{
    /* Get the native flags */
    u32b element_flags = (r_ptr->r_native & (TERRAIN_MASK));

    /* Special case. Boiling mud (hybrid element) */
    if ((element_flags & ELEMENT_BMUD) == (ELEMENT_BMUD))
    {
        /* Just add the pseudo flag */
        /* Note that LF1_LAVA and LF1_MUD still are in the flags */
        element_flags |= LF1_BMUD;
    }

    /* Special case. Boiling mud (hybrid element) */
    if ((element_flags & ELEMENT_BWATER) == (ELEMENT_BWATER))
    {
        /* Just add the pseudo flag */
        /* Note that LF1_LAVA and LF1_WATER still are in the flags */
        element_flags |= LF1_BWATER;
    }

    /* Done */
    return (element_flags);
}


/*
 * Paste the name of the element given in flag into buf.
 * max is the maximum size of buf.
 * flag must contain ONLY ONE of the LF1_* flags.
 */
QString describe_one_level_flag(u32b flag)
{
    /* Default name */
    QString name = "unknown";

    /* Analyze the flag */
    switch (flag)
    {
        case LF1_FIRE: name = "fire"; break;
        case LF1_ACID: name = "acid"; break;
        case LF1_WATER: name = "water"; break;
        case LF1_MUD: name = "mud"; break;
        case LF1_LAVA: name = "lava"; break;
        case LF1_ICE: name = "ice"; break;
        case LF1_FOREST: name = "forest"; break;
        case LF1_OIL: name = "oil"; break;
        case LF1_SAND: name = "sand"; break;
        case LF1_BWATER: name = "boiling water"; break;
        case LF1_BMUD: name = "boiling mud"; break;
    }

    /* Copy the name */
    return (name);
}


/*
 * Show several messages describing all the LF1_* flags contained in the
 * given set of flags
 */
void debug_all_level_flags(u32b all_flags)
{
    int i;
    u32b flag = 1;

    /* Parse the bits of the given flags */
    for (i = 0; i < 32; i++, flag <<= 1)
    {
        /* The current flags is present in the set? */
        if (all_flags & flag)
        {
            /* Get the name */
            QString buf = describe_one_level_flag(flag);

            /* Message */
            message(QString("The %s flag is present.") .arg(buf));
        }
    }
}


