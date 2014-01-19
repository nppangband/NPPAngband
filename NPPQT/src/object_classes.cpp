/* File: object_classes.cpp */

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
 * Wipe all data in an object.
 * This function needs to have all variables in objct_type
 */

void object_type::object_wipe()
{
    k_idx = iy = ix = 0;
    pval = sval = tval = 0;
    discount = number = weight = 0;
    art_num = ego_num = xtra1 = xtra2 = 0;
    to_h = to_d = to_a = ac = dd = ds = 0;
    timeout = discount = ident = 0;
    marked = next_o_idx = held_m_idx = 0;
    origin_nature = origin_dlvl = origin_r_idx = mimic_r_idx = 0;
    obj_in_use = FALSE;
    inscription.clear();
    origin_m_name.clear();
}

// Copy object safely without using memset.
void object_type::object_copy (object_type *j_ptr)
{
    k_idx = j_ptr->k_idx;
    iy = j_ptr->iy;
    ix = j_ptr->ix;
    pval = j_ptr->pval;
    sval = j_ptr->sval;
    discount = j_ptr->discount;
    number = j_ptr->number;
    weight = j_ptr->weight;
    art_num = j_ptr->art_num;
    ego_num = j_ptr->ego_num;
    xtra1 = j_ptr->xtra1;
    xtra2 = j_ptr->xtra2;
    to_h = j_ptr->to_h;
    to_d = j_ptr->to_d;
    to_a = j_ptr->to_a;
    ac = j_ptr->ac;
    dd = j_ptr->dd;
    ds = j_ptr->ds;
    timeout = j_ptr->timeout;
    discount = j_ptr->discount;
    ident = j_ptr->ident;
    marked = j_ptr->marked;
    next_o_idx = j_ptr->next_o_idx;
    held_m_idx = j_ptr->held_m_idx;
    origin_nature = j_ptr->origin_nature;
    origin_dlvl = j_ptr->origin_dlvl;
    origin_r_idx = j_ptr->origin_r_idx;
    mimic_r_idx = j_ptr->mimic_r_idx;
    obj_in_use = j_ptr->obj_in_use;
    inscription = j_ptr->inscription;
    origin_m_name = j_ptr->origin_m_name;
}

/*
 * Returns TRUE if an object has some ego-powers that should be ignored if
 * the game does not want *full* knowledge of it.
*/
bool object_type::has_hidden_powers()
{
    /* *identified* items are never hidden*/
    if (ident & (IDENT_MENTAL)) return (FALSE);

    /* Hack - Ignore chests */
    if (tval == TV_CHEST) return (FALSE);

    /* Analyze xtra1 */
    switch (xtra1)
    {
        case OBJECT_XTRA_STAT_SUSTAIN:
        case OBJECT_XTRA_TYPE_HIGH_RESIST:
        case OBJECT_XTRA_TYPE_POWER:
        case OBJECT_XTRA_TYPE_IMMUNITY:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
bool object_type::is_wearable()
{
    /* Valid "tval" codes */
    switch (tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_DRAG_SHIELD:
        case TV_LIGHT:
        case TV_AMULET:
        case TV_RING: return (TRUE);
    }

    /* Nope */
    return (FALSE);
}

/*
 * returns whether an object counts as "known" due to EASY_KNOW status
 */
bool object_type::is_easy_know()
{
    object_kind *k_ptr = &k_info[k_idx];

    if (k_ptr->aware && (k_ptr->k_flags3 & TR3_EASY_KNOW)) return (TRUE);

    return (FALSE);
}


/*
 * returns whether an object should be treated as fully known (e.g. ID'd)
 */
bool object_type::is_known()
{
    if (is_easy_know()) return (TRUE);
    if (ident & (IDENT_KNOWN)) return (TRUE);
    if (ident & (IDENT_STORE)) return (TRUE);
    return (FALSE);
}

/*
 * returns whether the player is aware of the object's flavour
 */
bool object_type::is_flavor_known()
{
    return k_info[k_idx].aware;
}

/*
 * Returns TRUE if this object can be pseudo-ided.
 */
bool object_type::can_be_pseudo_ided()
{
    /* Valid "tval" codes */
    switch (tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_DRAG_SHIELD:
        {
            return (TRUE);
        }
        case TV_LIGHT:
        {
            if (game_mode == GAME_NPPMORIA) return (FALSE);

            if (sval == SV_LIGHT_LANTERN)
            return (TRUE);
            break;
        }
    }
    return (FALSE);
}

