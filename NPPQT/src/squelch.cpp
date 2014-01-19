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
    for(o_idx = cave_o_idx[y][x]; o_idx; o_idx = next_o_idx)
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
        cave_o_idx[y][x] = first_good_idx;
        o_list[cur_good_idx].next_o_idx = first_bad_idx;
        o_list[cur_bad_idx].next_o_idx = 0;
    }

    else
    {
        cave_o_idx[y][x] = first_bad_idx;
    }
}

bool squelch_item_ok(const object_type *o_ptr)
{
    object_kind *k_ptr = k_ptr = &k_info[o_ptr->k_idx];

    /* Always delete "nothings" */
    if (!o_ptr->k_idx) return (TRUE);

    /* Ignore inscribed objects, artifacts , mimics or quest objects */
    if ((o_ptr->obj_note) || (artifact_p(o_ptr)) || (o_ptr->ident & (IDENT_QUEST)) ||
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

    for(o_idx = cave_o_idx[y][x]; o_idx; o_idx = next_o_idx)
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
