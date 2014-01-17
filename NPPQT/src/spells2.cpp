/* File: spells2.c */

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
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
    /* Healing needed */
    if (p_ptr->chp < p_ptr->mhp)
    {
        /* Gain hitpoints */
        p_ptr->chp += num;

        /* Enforce maximum */
        if (p_ptr->chp >= p_ptr->mhp)
        {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;
        }

        /* Redraw */
        p_ptr->redraw |= (PR_HP);

        /* Heal 0-4 */
        if (num < 5)
        {
            // TODO make message box msg_print("You feel a little better.");
        }

        /* Heal 5-14 */
        else if (num < 15)
        {
            // TODO make message box msg_print("You feel better.");
        }

        /* Heal 15-34 */
        else if (num < 35)
        {
            // TODO make message box msg_print("You feel much better.");
        }

        /* Heal 35+ */
        else
        {
            // TODO make message box msg_print("You feel very good.");
        }

        /* Notice */
        return (TRUE);
    }

    /* Ignore */
    return (FALSE);
}

