/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * 						Jeff Greene, Diego Gonzalez, Anssi Ramela
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
 * Return the player realm for the spell_list table.  Assumes player is a spellcaster.
 * We don't return any error because this value is going to be looked up in a table,
 * & would cause the game to crash
 */
int get_player_spell_realm(void)
{
    /* Mage or priest spells? */
    if (cp_ptr->spell_book == TV_MAGIC_BOOK) 	return (MAGE_REALM);
    if (cp_ptr->spell_book == TV_PRAYER_BOOK)	return (PRIEST_REALM);
    /*TV_DRUID_BOOK*/           				return (DRUID_REALM);
}

QString get_spell_name(int tval, int spell)
{
    if (tval == TV_MAGIC_BOOK) return ("mage");
    // TODO return do_mage_spell(MODE_SPELL_NAME, spell,0);
    else if (tval == TV_PRAYER_BOOK)return ("priest");

    // TODO return do_priest_prayer(MODE_SPELL_NAME, spell, 0);
    /*TV_DRUID_BOOK*/
    else return ("druid");
    // TODO else return do_druid_incantation(MODE_SPELL_NAME, spell,0);
}
