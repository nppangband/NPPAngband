/*
 * File: squelch.c
 * Purpose: Item destruction
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * Diego Gonzalez, David Vestal, Pete Mack, Andrew Sidwell.
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
#include "src/squelch.h"

/*
 * Find the squelch type of the object, or TYPE_MAX if none
 */
static squelch_type_t squelch_type_of(object_type *o_ptr)
{
    size_t i;

    /* Find the appropriate squelch group */
    for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
    {
        if ((quality_mapping[i].tval == o_ptr->tval) &&
            (quality_mapping[i].min_sval <= o_ptr->sval) &&
            (quality_mapping[i].max_sval >= o_ptr->sval))
            return quality_mapping[i].squelch_type;
    }

    return PS_TYPE_MAX;
}

/* Convert the values returned by squelch_itemp to string */
QString squelch_to_label(int squelch)
{
    if (squelch == SQUELCH_YES) return ("(Squelched)");

    if (squelch == SQUELCH_FAILED) return ("(Squelch Failed)");

    return ("");
}


/*
 * Determines if an object is going to be squelched on identification.
 * Input:
 *  o_ptr   : This is a pointer to the object type being identified.
 *  feeling : This is the feeling of the object if it is being
 *            pseudoidentified or 0 if the object is being identified.
 *  fullid  : Is the object is being identified?
 *
 * Output: One of the three above values.
 */

int squelch_itemp(object_type *o_ptr, byte feelings, bool fullid)
{
    int num, result;
    byte feel;

    /* Default */
    result = SQUELCH_NO;

    /* Never squelch quest items */
    if (o_ptr->ident & IDENT_QUEST) return result;

    /* Squelch some ego items if known */
    if (fullid && (o_ptr->is_ego_item()) && (e_info[o_ptr->ego_num].squelch))
    {
        /* Squelch fails on inscribed objects */
        return ((!o_ptr->inscription.isEmpty()) ? SQUELCH_FAILED: SQUELCH_YES);
    }

    /* Check to see if the object is eligible for squelching on id. */
    num = squelch_type_of(o_ptr);

    /* Never squelched */
    if (num == PS_TYPE_MAX) return result;

    /*
     * Get the "feeling" of the object.  If the object is being identified
     * get the feeling returned by a heavy pseudoid.
     */
    feel = feelings;

    /* Handle fully identified objects */
    if (fullid)  feel = o_ptr->pseudo_heavy();

    /* Get result based on the feeling and the squelch_level */
    switch (squelch_level[num])
    {
        case SQUELCH_NONE:
        {
            return result;
        }

        case SQUELCH_CURSED:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_AVERAGE:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_GOOD_STRONG:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE) ||
                (feel==INSCRIP_GOOD_STRONG)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_GOOD_WEAK:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE) ||
                (feel==INSCRIP_GOOD_STRONG) ||
                (feel==INSCRIP_GOOD_WEAK)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_ALL:
        {
            result = SQUELCH_YES;
            break;
        }
    }

    /* Squelching will fail on an artifact or inscribed object */
    if ((result == SQUELCH_YES) && (o_ptr->is_artifact() || !o_ptr->inscription.isEmpty())) result = SQUELCH_FAILED;

    return result;
}

void rearrange_stack(int y, int x)
{
    s16b o_idx, next_o_idx;
    s16b first_bad_idx, first_good_idx, cur_bad_idx, cur_good_idx;

    object_type *o_ptr;

    bool sq_flag = FALSE;

    /* Initialize */
    first_bad_idx = 0;
    first_good_idx = 0;
    cur_bad_idx = 0;
    cur_good_idx = 0;

    /*go through all the objects*/
    for(o_idx = dungeon_info[y][x].object_idx; o_idx; o_idx = next_o_idx)
    {
        o_ptr = &(o_list[o_idx]);
        next_o_idx = o_ptr->next_o_idx;

        /*is it marked for squelching*/
        sq_flag = ((k_info[o_ptr->k_idx].squelch == SQUELCH_ALWAYS) &&
            (k_info[o_ptr->k_idx].aware));

        if (sq_flag)
        {
            if (first_bad_idx == 0)
            {
                first_bad_idx = o_idx;
                cur_bad_idx = o_idx;
            }

            else
            {
                o_list[cur_bad_idx].next_o_idx = o_idx;
                cur_bad_idx = o_idx;
            }
        }

        else

        {
            if (first_good_idx==0)
            {
                first_good_idx = o_idx;
                cur_good_idx = o_idx;
            }

            else
            {
                o_list[cur_good_idx].next_o_idx = o_idx;
                cur_good_idx = o_idx;
            }
        }
    }

    if (first_good_idx != 0)
    {
        dungeon_info[y][x].object_idx = first_good_idx;
        o_list[cur_good_idx].next_o_idx = first_bad_idx;
        o_list[cur_bad_idx].next_o_idx = 0;
    }

    else
    {
        dungeon_info[y][x].object_idx = first_bad_idx;
    }
}

/*
 * This performs the squelch, actually removing the item from the
 * game.  It returns 1 if the item was squelched, and 0 otherwise.
 * This return value is never actually used.
 */
int do_squelch_item(int squelch, int item, object_type *o_ptr)
{
    if (squelch != SQUELCH_YES) return 0;

    /*hack - never squelch quest items*/
    if (o_ptr->ident & IDENT_QUEST) return 0;

    if (item >= 0)
    {
        inven_item_increase(item, -o_ptr->number);
        inven_item_optimize(item);
    }

    else
    {
        floor_item_increase(0 - item, -o_ptr->number);
        floor_item_optimize(0 - item);
    }

    return 1;
}

bool squelch_item_ok(object_type *o_ptr)
{
    object_kind *k_ptr = k_ptr = &k_info[o_ptr->k_idx];

    /* Always delete "nothings" */
    if (!o_ptr->k_idx) return (TRUE);

    /* Ignore inscribed objects, artifacts , mimics or quest objects */
    if ((!o_ptr->inscription.isEmpty()) || (o_ptr->is_artifact()) || (o_ptr->ident & (IDENT_QUEST)) ||
        (o_ptr->mimic_r_idx))
    {
        return (FALSE);
    }

    /* Object kind is set to be always squelched */
    if ((k_ptr->squelch == SQUELCH_ALWAYS) && k_ptr->aware) return (TRUE);

    /* Apply quality squelch if possible */
    if (object_known_p(o_ptr) && (squelch_itemp(o_ptr, 0, TRUE) == SQUELCH_YES)) return TRUE;

    /* Don't squelch */
    return (FALSE);

}

/* Attempt to squelch every object in a pile. */
void do_squelch_pile(int y, int x)
{
    s16b o_idx, next_o_idx;
    object_type *o_ptr;

    for(o_idx = dungeon_info[y][x].object_idx; o_idx; o_idx = next_o_idx)
    {
        /* Get the object */
        o_ptr = &(o_list[o_idx]);

        /* Point to the next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Destroy the object? */
        if (squelch_item_ok(o_ptr))
        {
            /* Delete */
            delete_object_idx(o_idx);
        }
    }
}
