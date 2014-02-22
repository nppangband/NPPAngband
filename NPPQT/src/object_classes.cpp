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

ego_item_type::ego_item_type()
{
    ego_item_wipe();
}

void ego_item_type::ego_item_wipe()
{
    e_name.clear();
    e_text.clear();

    cost = e_flags1 = e_flags2 = e_flags3 = 0;
    level = rarity = rating = 0;

    for (int i = 0; i < EGO_TVALS_MAX; i++) {
        tval[i] = min_sval[i] = max_sval[i] = 0;
    }

    max_to_h = max_to_a = max_to_d = max_pval = 0;
    xtra = 0;
    everseen = squelch = 0;
}

object_type::object_type()
{
    object_wipe();
}

/*
 * Wipe all data in an object.
 * This function needs to have all variables in objct_type
 * This function does not clear the monster pointer from dungeon_type.
 * It should be not called directly on any object already mapped to
 *dungeon square or held by a monster.
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
void object_type::object_copy(object_type *j_ptr)
{
    k_idx = j_ptr->k_idx;
    iy = j_ptr->iy;
    ix = j_ptr->ix;
    tval = j_ptr->tval;
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
 * returns whether an object is aware (e.g. ID'd)
 */
bool object_type::is_aware()
{
    if (k_info[k_idx].aware) return (TRUE);
    return (FALSE);
}

/*
 * returns whether an object has been tried
 */
bool object_type::is_tried()
{
    if (k_info[k_idx].tried) return (TRUE);
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

/*
 * Artifacts use the "art_num" field
 */
bool object_type::is_artifact()
{
    if (art_num) return (TRUE);
    return (FALSE);
}


/**
 * \returns whether the object is known to be an artifact
 */
bool object_type::is_known_artifact()
{

    if (is_artifact() && was_sensed()) return (TRUE);
    if (ident & (IDENT_INDESTRUCT)) return (TRUE);
    if (ident & (INSCRIP_TERRIBLE)) return (TRUE);
    if (ident & (INSCRIP_SPECIAL)) return (TRUE);
    return (FALSE);
}

/*
 * Artifacts use the "ego_num" field
 */
bool object_type::is_ego_item()
{
    if (ego_num) return (TRUE);
    return (FALSE);
}

/*
 * Returns TRUE if this object is cursed.
 */
bool object_type::is_cursed()
{
    if (ident & (IDENT_CURSED)) return (TRUE);
    return (FALSE);
}

/*
 * Returns TRUE if this object is broken.
 */
bool object_type::is_broken()
{
    if (ident & (IDENT_BROKEN)) return (TRUE);
    return (FALSE);
}

/**
 * \returns whether the object has been sensed with pseudo-ID
 */
bool object_type::was_sensed()
{
    if (ident & (IDENT_SENSE)) return (TRUE);
    return (FALSE);
}

/*
 * Is this a spellbook?
 */
bool object_type::is_spellbook()
{
    if (tval == TV_MAGIC_BOOK) return (TRUE);
    if (tval == TV_PRAYER_BOOK) return (TRUE);
    if (tval == TV_DRUID_BOOK) return (TRUE);

    return (FALSE);
}

/* Basic tval testers */
bool object_type::is_shovel()       { return tval == TV_DIGGING;}
bool object_type::is_bow()          { return tval == TV_BOW; }
bool object_type::is_staff()        { return tval == TV_STAFF; }
bool object_type::is_wand()         { return tval == TV_WAND; }
bool object_type::is_rod()          { return tval == TV_ROD; }
bool object_type::is_potion()       { return tval == TV_POTION; }
bool object_type::is_scroll()       { return tval == TV_SCROLL; }
bool object_type::is_parchment()    { return tval == TV_PARCHMENT; }
bool object_type::is_food()         { return tval == TV_FOOD; }
bool object_type::is_light()        { return tval == TV_LIGHT; }
bool object_type::is_ring()         { return tval == TV_RING; }
bool object_type::is_amulet()       { return tval == TV_AMULET; }
bool object_type::is_jewlery()
{
    if (is_amulet())    return (TRUE);
    if (is_ring())      return (TRUE);
    return (FALSE);
}
bool object_type::is_chest()        { return tval == TV_CHEST; }

/**
 * \returns whether the object can be filled with oil
 */
bool object_type::is_fuelable_lite()
{
    if (tval != TV_LIGHT)   return (FALSE);
    if (is_artifact())      return (FALSE);
    return (TRUE);
}

/**
 * \returns whether the object is ammunition
 */
bool object_type::is_ammo()
{
    if (tval == TV_BOLT)   return (TRUE);
    if (tval == TV_ARROW)   return (TRUE);
    if (tval == TV_SHOT)   return (TRUE);
    return (FALSE);
}

/*
 * Determine whether an object is a weapon
 */
bool object_type::is_weapon()
{
    switch (tval)
    {
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING:
            return TRUE;
        default:
            return FALSE;
    }
}


/*
 * Determine if an object has charges
 */
bool object_type::can_zap()
{
    object_kind *k_ptr = &k_info[k_idx];

    if (!is_rod()) return FALSE;

    if (timeout > (pval - k_ptr->pval)) return (FALSE);

    return (TRUE);
}

/*
 * Determine if an object can be browsed (spellbook)
 */
bool object_type::can_browse()
{
    if (tval != cp_ptr->spell_book) return (FALSE);
    return TRUE;
}

/*
 * Determine if an object is a spelbook with spells that can be studied
 */
bool object_type::can_study()
{
    int i;
    if (tval != cp_ptr->spell_book) return FALSE;

    for (i = 0;  i < SPELLS_PER_BOOK; i++)
    {
        int spell = get_spell_index(sval, i);

        /* Not a spell */
        if (spell == -1) continue;

        /* Is there a spell we can learn? */
        if (spell_okay(spell, FALSE)) return (TRUE);
    }
    return (FALSE);

}

/*
 * Determine if an object is a spellbook with spells that can be cast
 */
bool object_type::can_cast()
{
    int i;
    if (tval != cp_ptr->spell_book) return FALSE;

    for (i = 0;  i < SPELLS_PER_BOOK; i++)
    {
        int spell = get_spell_index(sval, i);

        /* Not a spell */
        if (spell == -1) continue;

        /* Is there a spell we can learn? */
        if (spell_okay(spell, TRUE)) return (TRUE);
    }
    return (FALSE);
}


/*
 * Can only take off non-cursed items
 */
bool object_type::can_takeoff()
{
    return (!is_cursed());
}


/*
 * Can has inscrip pls
 */
bool object_type::has_inscription()
{
    return (!inscription.isEmpty());
}

/*
 * Determine if an object has charges
 */
bool object_type::has_charges()
{
    if (!is_wand() && !is_staff()) return (FALSE);

    if (pval <= 0) return (FALSE);

    return (TRUE);
}

/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
s16b object_type::pseudo_heavy()
{
    /* Artifacts */
    if (is_artifact())
    {
        /* Cursed/Broken */
        if (is_cursed() || is_broken()) return (INSCRIP_TERRIBLE);

        /* Normal */
        return (INSCRIP_SPECIAL);
    }

    /* Ego-Items */
    if (is_ego_item())
    {
        /* Cursed/Broken */
        if (is_cursed() || is_broken()) return (INSCRIP_WORTHLESS);

        /* Normal */
        return (INSCRIP_EXCELLENT);
    }

    /* Cursed items */
    if (is_cursed()) return (INSCRIP_CURSED);

    /* Broken items */
    if (is_broken()) return (INSCRIP_BROKEN);

    /* Good "armor" bonus */
    if (to_a > 0) return (INSCRIP_GOOD_STRONG);

    /* Good "weapon" bonus */
    if (to_h + to_d > 0) return (INSCRIP_GOOD_STRONG);

    /* Default to "average" */
    return (INSCRIP_AVERAGE);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
s16b object_type::pseudo_light()
{
    /* Cursed items (all of them) */
    if (is_cursed()) return (INSCRIP_CURSED);

    /* Broken items (all of them) */
    if (is_broken()) return (INSCRIP_BROKEN);

    /* Artifacts -- except cursed/broken ones */
    if (is_artifact()) return (INSCRIP_GOOD_WEAK);

    /* Ego-Items -- except cursed/broken ones */
    if (is_ego_item()) return (INSCRIP_GOOD_WEAK);

    /* Good armor bonus */
    if (to_a > 0) return (INSCRIP_GOOD_WEAK);

    /* Good weapon bonuses */
    if (to_h + to_d > 0) return (INSCRIP_GOOD_WEAK);

    /* Default to "average" */
    return (INSCRIP_AVERAGE);
}

/*
 * Hack -- Removes curse from an object.
 */
void object_type::uncurse()
{
    /* Uncurse it */
    ident &= ~(IDENT_CURSED);

    /* Remove special inscription, if any */
    if (discount >= INSCRIP_NULL) discount = 0;

    /* Take note if allowed */
    if (discount == 0) discount = INSCRIP_UNCURSED;

    /* The object has been "sensed" */
    ident |= (IDENT_SENSE);
}

/*
 * Hack -- Removes curse from an object.
 */
byte object_type::object_color()
{
    return (k_info[k_idx].color_num);
}


object_kind::object_kind()
{
    object_kind_wipe();
}

void object_kind::object_kind_wipe()
{
    k_name.clear();
    k_text.clear();

    tval = sval = pval = 0;
    to_h = to_d = to_a = 0;
    ac = dd = ds = weight = cost = 0;
    k_flags1 = k_flags2 = k_flags3 = 0;
    effect = 0;
    C_WIPE(locale, 4, byte);
    C_WIPE(chance, 4, byte);
    k_level = extra = 0;
    color_num = 0;
    d_color = QColor("black");
    d_char = QChar(' ');
    autoinscribe.clear();
    flavor = squelch = 0;
    aware = tried = everseen = 0;
    tile_32x32_y = tile_32x32_x = tile_8x8_y = tile_8x8_x = 0
    tile_id.clear();

}

artifact_type::artifact_type()
{
    artifact_wipe();
}

/*
 * Wipe all data in an randart.
 * This function needs to have all variables in artifact_type
 * This function does not clear the object that contains the randart.
 * It should be not called directly on any artifact already mapped to an object.
 */
void artifact_type::artifact_wipe()
{
    a_name.clear();
    a_text.clear();
    sval = tval = 0;
    to_h = to_d = to_a = ac = 0;
    dd = ds = 0;
    weight = 0;
    cost = 0;
    a_flags1 = a_flags2 = a_flags3 = a_native = 0;
    a_level = a_rarity = a_cur_num = a_max_num = activation = 0;
    time = randtime = 0;
}


flavor_type::flavor_type()
{
    flavor_wipe();
}

void flavor_type::flavor_wipe()
{
    text.clear();
    tval = sval = color_num = 0;
    d_color = QColor("black");
    d_char = QChar(' ');
    tile_32x32_y = tile_32x32_x = tile_8x8_y = tile_8x8_x = 0;
    tile_id.clear();
}

