/* File: was monster2.c */

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
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "CHEST/GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(int m_idx, int num_item, int num_gold)
{
    monster_type *m_ptr = &mon_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* Note the number of things dropped */
    if (num_item > l_ptr->drop_item) l_ptr->drop_item = num_item;
    if (num_gold > l_ptr->drop_gold) l_ptr->drop_gold = num_gold;


    /* Hack -- memorize the good/great flags */
    if (r_ptr->flags1 & (RF1_DROP_CHEST)) l_ptr->r_l_flags1 |= (RF1_DROP_CHEST);
    if (r_ptr->flags1 & (RF1_DROP_GOOD)) l_ptr->r_l_flags1 |= (RF1_DROP_GOOD);
    if (r_ptr->flags1 & (RF1_DROP_GREAT)) l_ptr->r_l_flags1 |= (RF1_DROP_GREAT);


    /* Update monster recall window */
    if (p_ptr->monster_race_idx == m_ptr->r_idx)
    {
        /* Window stuff */
        p_ptr->redraw |= (PR_MONSTER);
    }
}


