
/* File: was cmd5.c */

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

#include <src/npp.h>


/*
 * Determine if a spell is "okay" for the player to cast or study
 * The spell must be legible, not forgotten, and also, to cast,
 * it must be known, and to study, it must not be known.
 */
bool spell_okay(int spell, bool known)
{
    const magic_type *s_ptr;

    /* Get the spell */
    s_ptr = &mp_ptr->info[spell];

    /* Spell is illegal */
    if (s_ptr->slevel > p_ptr->lev) return (FALSE);

    /* Spell is forgotten */
    if (p_ptr->spell_flags[spell] & PY_SPELL_FORGOTTEN)
    {
        /* Never okay */
        return (FALSE);
    }

    /* Spell is ironman */
    if (p_ptr->spell_flags[spell] & PY_SPELL_IRONMAN)
    {
        /* Never okay */
        return (FALSE);
    }

    /* Spell is learned */
    if (p_ptr->spell_flags[spell] & PY_SPELL_LEARNED)
    {

        /* Okay to cast, not to study */
        return (known);
    }

    /* Okay to study, not to cast */
    return (!known);
}

s16b get_spell_from_list(s16b book, s16b spell)
{
    int realm = get_player_spell_realm();

    if (game_mode == GAME_NPPMORIA)
    {
        /* Check bounds */
        if ((spell < 0) || (spell >= SPELLS_PER_BOOK)) return (-1);
        if ((book < 0) || (book >= BOOKS_PER_REALM_MORIA)) return (-1);

        if (realm == MAGE_REALM) return (spell_list_nppmoria_mage[book][spell]);
        if (realm == PRIEST_REALM) return (spell_list_nppmoria_priest[book][spell]);
    }
    else
    {
        /* Check bounds */
        if ((spell < 0) || (spell >= SPELLS_PER_BOOK)) return (-1);
        if ((book < 0) || (book >= BOOKS_PER_REALM_ANGBAND)) return (-1);

        if (realm == MAGE_REALM) return (spell_list_nppangband_mage[book][spell]);
        if (realm == PRIEST_REALM) return (spell_list_nppangband_priest[book][spell]);
        if (realm == DRUID_REALM) return (spell_list_nppangband_druid[book][spell]);
    }


    /* Whoops! */
    return (-1);
}


int get_spell_index(object_type *o_ptr, int index)
{
    return get_spell_from_list(o_ptr->sval,index);
}
