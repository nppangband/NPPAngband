
/* File: dungeon.c */

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
 * Change dungeon level.
 * Aside from setting the player depth at the beginning of the game,
 * this should be the only place where a player depth is actually
 * changed.
 */
void dungeon_change_level(int dlev)
{
    /* Handle lost greater vaults */
    if (g_vault_name[0] != '\0')
    {
        if (adult_take_notes)
        {
            QString note = "Left the level without entering the ";
            note.append(g_vault_name);

            // TODO do_cmd_note(note, p_ptr->depth);
        }
    }

    /* New depth */
    p_ptr->depth = dlev;

    /* Leaving */
    p_ptr->leaving = TRUE;

    /* Save the game when we arrive on the new level. */
    p_ptr->autosave = TRUE;

    p_ptr->redraw |= (PR_DEPTH | PR_QUEST_ST | PR_FEELING);
}

/*
 * Process the player terrain damage
 * This function can kill the player, so all calls to this function should be able to handle this.
 */
void process_player_terrain_damage(void)
{
    /* No damage to take terrain */
    if (p_ptr->cumulative_terrain_damage > 0)
    {
        /*
         * IMPORTANT: we divide cumulative damage by 10
         * to get a value nearly equal to "dam_non_native" at
         * normal speed (1 player turn every 10 game turns)
         */
        int dam = p_ptr->cumulative_terrain_damage / 10;

        QString name,  kb_str;

        /* Get the feature */
        int feat = dungeon_info[p_ptr->py][p_ptr->px].feat;

        /* Uncomment this if you want a damage cap */
        /*dam = MIN(dam, f_info[feat].dam_non_native);*/

        /* Get the feature name */
        name = feature_desc(feat, TRUE, TRUE);

        /* Format the killer string */
        kb_str = "standing in ";
        kb_str.append(name);

        /* Take the hit */
        // TODO take_terrain_hit(dam, feat, kb_str);

        /* Reset terrain damage */
        p_ptr->cumulative_terrain_damage = 0;

    }
}

