
/*
 * File: object2.c
 * Purpose: Object list maintenance and other object utilities
 *
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
 * Returns TRUE if an object has some ego-powers that should be ignored if
 * the game does not want *full* knowledge of it.
*/
bool object_has_hidden_powers(const object_type *o_ptr)
{
    /* *identified* items are never hidden*/
    if (o_ptr->ident & (IDENT_MENTAL)) return (FALSE);

    /* Hack - Ignore chests */
    if (o_ptr->tval == TV_CHEST) return (FALSE);

    /* Analyze xtra1 */
    switch (o_ptr->xtra1)
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


/*Helper function for add extra flags. Adds the flags from xtra2*/
static u32b add_xtra2_flags(u32b xtra_flags, byte xtra_size, u32b xtra_base)
{
    byte i;

    u32b flag_check = 0x00000001L;

    u32b return_flag = 0;

    for (i = 0; i < xtra_size; i++)
    {
        /*Do we have this flag?*/
        if (xtra_flags & flag_check)
        {
            /*mark it*/
            return_flag |= xtra_base;
        }

        /*shift everything for the next check*/
        flag_check  = flag_check << 1;
        xtra_base  = xtra_base << 1;

    }

    return (return_flag);
}


/*
 * Obtain the "flags" for an item
 */
static void object_flags_aux(int mode, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native)
{
    object_kind *k_ptr;

    if (mode == OBJECT_FLAGS_KNOWN)
    {
        /* Clear */
        (*f1) = (*f2) = (*f3) = (*native) = 0L;

        /* Must be identified */
        if (!object_known_p(o_ptr)) return;
    }

    k_ptr = &k_info[o_ptr->k_idx];

    /* Base object */
    (*f1) = k_ptr->k_flags1;
    (*f2) = k_ptr->k_flags2;
    (*f3) = k_ptr->k_flags3;
    (*native) = k_ptr->k_native;

    if (mode == OBJECT_FLAGS_FULL)
    {
        /* Artifact */
        if (o_ptr->art_num)
        {
            artifact_type *a_ptr = &a_info[o_ptr->art_num];

            (*f1) = a_ptr->a_flags1;
            (*f2) = a_ptr->a_flags2;
            (*f3) = a_ptr->a_flags3;
            (*native) = a_ptr->a_native;
        }
    }

    /* Ego-item */
    if (o_ptr->ego_num)
    {
        ego_item_type *e_ptr = &e_info[o_ptr->ego_num];

        (*f1) |= e_ptr->e_flags1;
        (*f2) |= e_ptr->e_flags2;
        (*f3) |= e_ptr->e_flags3;
        (*native) |= e_ptr->e_native;
    }

    if (mode == OBJECT_FLAGS_KNOWN)
    {
        /* Obvious artifact flags */
        if (o_ptr->art_num)
        {
            artifact_type *a_ptr = &a_info[o_ptr->art_num];

            /* Obvious flags (pval) */
            (*f1) = (a_ptr->a_flags1 & (TR1_PVAL_MASK));
            (*f3) = (a_ptr->a_flags3 & (TR3_IGNORE_MASK));
        }
    }

    if (mode == OBJECT_FLAGS_KNOWN)
    {
        bool spoil = FALSE;

        /* Artifact, *ID'ed or spoiled */
        if ((o_ptr->art_num) && (spoil || (o_ptr->ident & IDENT_MENTAL)))
        {
            artifact_type *a_ptr = &a_info[o_ptr->art_num];

            (*f1) = a_ptr->a_flags1;
            (*f2) = a_ptr->a_flags2;
            (*f3) = a_ptr->a_flags3;
            (*native) = a_ptr->a_native;

        }

        /* Full knowledge for *identified* objects */
        if ((!(o_ptr->ident & IDENT_MENTAL)) &&
            (object_has_hidden_powers(o_ptr)))	return;

    }

    /*hack - chests use xtra1 to store the theme, don't give additional powers to chests*/
    if (o_ptr->tval == TV_CHEST) return;

    /* Extra powers */
    switch (o_ptr->xtra1)
    {

        case OBJECT_XTRA_STAT_SUSTAIN:
        {
            /* Flag 2 */
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_SUSTAIN,
                                        OBJECT_XTRA_BASE_SUSTAIN);
            break;
        }

        case OBJECT_XTRA_TYPE_HIGH_RESIST:
        {
            /* Flag 2 */
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_HIGH_RESIST,
                                        OBJECT_XTRA_BASE_HIGH_RESIST);
            break;
        }

        case OBJECT_XTRA_TYPE_POWER:
        {
            /* Flag 3 */
            (*f3) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_POWER,
                                        OBJECT_XTRA_BASE_POWER);
            break;
        }
        case OBJECT_XTRA_TYPE_IMMUNITY:
        {
            /* Flag 2 */
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_IMMUNITY,
                                        OBJECT_XTRA_BASE_IMMUNITY);
            break;
        }
        case OBJECT_XTRA_TYPE_STAT_ADD:
        {
            /* Flag 1 */
            (*f1) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_STAT_ADD,
                                        OBJECT_XTRA_BASE_STAT_ADD);
            /*Stat add Also sustains*/
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_SUSTAIN,
                                        OBJECT_XTRA_BASE_SUSTAIN);
            break;
        }
        case OBJECT_XTRA_TYPE_SLAY:
        {
            /* Flag 1 */
            (*f1) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_SLAY,
                                        OBJECT_XTRA_BASE_SLAY);
            break;
        }
        case OBJECT_XTRA_TYPE_KILL:
        {
            /* Flag 1 */
            (*f1) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_KILL,
                                        OBJECT_XTRA_BASE_KILL);
            break;
        }
        case OBJECT_XTRA_TYPE_BRAND:
        {
            /* Flag 1 */
            (*f1) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_BRAND,
                                        OBJECT_XTRA_BASE_BRAND);
            /*
             * elemental brands also provide the appropriate resist
             * Note that the OBJECT_XTRA_SIZE_LOW_RESIST is not used.  There
             * are only 4 base resists, but 5 base brands (+poison).  Hence the
             * OBJECT_XTRA_SIZE_BRAND used here is deliberate and not a bug.
             */
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_BRAND,
                                        OBJECT_XTRA_BASE_LOW_RESIST);

            break;
        }
        case OBJECT_XTRA_TYPE_LOW_RESIST:
        {
            /* Flag 2 */
            (*f2) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_LOW_RESIST,
                                        OBJECT_XTRA_BASE_LOW_RESIST);
            break;
        }
        case OBJECT_XTRA_TYPE_NATIVE:
        {
            /* Flag native */
            (*native) |= add_xtra2_flags(o_ptr->xtra2, OBJECT_XTRA_SIZE_NATIVE,
                                        OBJECT_XTRA_BASE_NATIVE);
            break;
        }
    }

    /*Now add the ignores for any xtra above*/
    if ((*f2) & (TR2_RES_ACID))	(*f3) |= TR3_IGNORE_ACID;
    if ((*f2) & (TR2_RES_ELEC))	(*f3) |= TR3_IGNORE_ELEC;
    if ((*f2) & (TR2_RES_FIRE))	(*f3) |= TR3_IGNORE_FIRE;
    if ((*f2) & (TR2_RES_COLD))	(*f3) |= TR3_IGNORE_COLD;
    if ((*native) & (TN1_NATIVE_LAVA | TN1_NATIVE_FIRE)) (*f3) |= TR3_IGNORE_FIRE;
    if ((*native) & (TN1_NATIVE_ICE)) (*f3) |= TR3_IGNORE_COLD;
    if ((*native) & (TN1_NATIVE_ACID)) (*f3) |= TR3_IGNORE_ACID;
}

/*** Object kind lookup functions ***/


/*
 * Find the index of the object_kind with the given tval and sval
 */
s16b lookup_kind(int tval, int sval)
{
    int k;

    /* Look for it */
    for (k = 1; k < z_info->k_max; k++)
    {
        object_kind *k_ptr = &k_info[k];

        /* Found a match */
        if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
    }

    /* Oops */
    pop_up_message_box(QString("No object (%1,%2)") .arg(tval)  .arg(sval));

    /* Oops */
    return (0);
}


/*
 * Obtain the "flags" for an item
 */
void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native)
{
    object_flags_aux(OBJECT_FLAGS_FULL, o_ptr, f1, f2, f3, native);
}

/*
 * Obtain the "flags" for an item which are known to the player
 */
void object_flags_known(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native)
{
    object_flags_aux(OBJECT_FLAGS_KNOWN, o_ptr, f1, f2, f3, native);
}




/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
bool wearable_p(const object_type *o_ptr)
{
    /* Valid "tval" codes */
    switch (o_ptr->tval)
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
 * Get and return the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop(void)
{
    int i;


    /* Initial allocation */
    if (o_max < z_info->o_max)
    {
        /* Get next space */
        i = o_max;

        /* Expand object array */
        o_max++;

        /* Count objects */
        o_cnt++;

        /* Use this object */
        return (i);
    }


    /* Recycle dead objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr;

        /* Get the object */
        o_ptr = &o_list[i];

        /* Skip live objects */
        if (o_ptr->k_idx) continue;

        /* Count objects */
        o_cnt++;

        /* Use this object */
        return (i);
    }

    /* Warn the player (except during dungeon creation) */
    if (character_dungeon) pop_up_message_box("Too many objects!");

    /* Oops */
    return (0);
}



/*
 * Wipe an object clean.
 */
void object_wipe(object_type *o_ptr)
{
    /* Wipe the structure */
    (void)WIPE(o_ptr, object_type);
}


/*
 * Prepare an object based on an existing object
 */
void object_copy(object_type *o_ptr, const object_type *j_ptr)
{
    /* Copy the object TODO - confirm if memset should be used in C++*/
    COPY(o_ptr, j_ptr, object_type);
}

/*
 * Returns the number of quiver units an object will consume when it's stored in the quiver.
 * Every 99 quiver units we consume an inventory slot
 */
int quiver_space_per_unit(const object_type *o_ptr)
{
    return (ammo_p(o_ptr) ? 1: 5);
}

/*
 * Save the size of the quiver.
 */
void save_quiver_size(void)
{
    int count, i;
    object_type *i_ptr;

    /*
     * Items in the quiver take up space which needs to be subtracted
     * from space available elsewhere.
     */
    count = 0;

    for (i = QUIVER_START; i < QUIVER_END; i++)
    {
        /* Get the item */
        i_ptr = &inventory[i];

        /* Ignore empty. */
        if (!i_ptr->k_idx) continue;

        /* Tally up missiles. */
        count += i_ptr->number * quiver_space_per_unit(i_ptr);
    }

    p_ptr->quiver_slots = (count + 98) / 99;
    p_ptr->quiver_remainder = count % 99;
}

