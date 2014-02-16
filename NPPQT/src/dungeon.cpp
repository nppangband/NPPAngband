
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

            write_note(note, p_ptr->depth);
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
 * Remove the ironman ego_items of the probability tables.
 */
static void remove_ironman_ego_items(void)
{
    s16b i;

    alloc_entry *table = alloc_ego_table;

    /* Go through "normal" ego-item types */
    for (i = 0; i < alloc_ego_size; i++)
    {
        ego_item_type *e_ptr = &e_info[table[i].index];

        /*
         * Mega-hack - Allow fireproof books if store services
         * are disabled
         */
        if ((table[i].index == EGO_FIREPROOF) &&
            adult_no_store_services) continue;

        /* Ignore ironman ego-item types */
        if (e_ptr->e_flags3 & TR3_IRONMAN_ONLY)
        {
            /*No chance to be created normally*/
            table[i].prob1 = 0;
            table[i].prob2 = 0;
            table[i].prob3 = 0;
        }
    }
}


/*
 * Remove the ironman items of the probability tables.
 */
static void remove_ironman_items(void)
{
    s16b i;

    alloc_entry *table = alloc_kind_table;

    /* Go through "normal" object types */
    for (i = 0; i < alloc_kind_size; i++)
    {
        object_kind *k_ptr = &k_info[table[i].index];

        /* Ignore ironman object types */
        if (k_ptr->k_flags3 & TR3_IRONMAN_ONLY)
        {
            /*No chance to be generated normally*/
            table[i].prob1 = 0;
            table[i].prob2 = 0;
            table[i].prob3 = 0;

            /*
             * Hack - don't let the player cast the spells from
             * an ironman_only book.  Note this can be a quest item, so it can be tried.
             */
            if ((k_ptr->tval == cp_ptr->spell_book) && (k_ptr->tried == FALSE))
            {
                byte j;

                /* Extract spells */
                for (j = 0; j < SPELLS_PER_BOOK; j++)
                {

                    s16b spell = get_spell_from_list(k_ptr->sval, j);

                    /*skip blank spell slots*/
                    if (spell == -1) continue;

                    /* Don't count Ironman Spells. */
                    p_ptr->spell_flags[spell] |= PY_SPELL_IRONMAN;

                }
            }
        }
    }
}

#if 0
/*
 * Process some user pref files
 */
static void process_some_user_pref_files(void)
{
    char buf[1024];

    /* Process the "user.prf" file */
    (void)process_pref_file("user.prf");

    /* Process the "user.scb" autoinscriptions file */
    (void)process_pref_file("user.scb");

    /* Process the "classes.prf" file */
    (void)process_pref_file("classes.prf");

    /* Process the "races.prf" file */
    (void)process_pref_file("races.prf");

    /* Get the "PLAYER.prf" filename */
    (void)strnfmt(buf, sizeof(buf), "%s.prf", op_ptr->base_name);

    /* Process the "PLAYER.prf" file */
    (void)process_pref_file(buf);
}
#endif

void launch_game()
{
    /* Hack -- Enter wizard mode */
    //if (arg_wizard && enter_wizard_mode()) p_ptr->wizard = TRUE;

    /* Flavor the objects */
    flavor_init();

    /* Process some user pref files */
    //process_some_user_pref_files();

    /* Remove ironman ego-items if needed */
    if (!adult_ironman && !adult_no_stores)
    {
        remove_ironman_items();
        remove_ironman_ego_items();
    }

    /* Generate a dungeon level if needed */
    if (!character_dungeon) generate_cave();

    /* Character is now "complete" */
    character_generated = TRUE;

    /* Start with normal object generation mode */
    object_generation_mode = OB_GEN_MODE_NORMAL;

    /* Start with the item_tester_swap variable as false */
    item_tester_swap = FALSE;

    /* Start playing */
    p_ptr->playing = TRUE;

    /* Save not required yet. */
    p_ptr->autosave = FALSE;

    /* Hack -- Enforce "delayed death" */
    if (p_ptr->chp < 0) p_ptr->is_dead = TRUE;
}
