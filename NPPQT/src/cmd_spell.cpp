
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



/* Adjustment to minimum failure rates for wisdom/intelligence in moria */
int spell_failure_min_moria(int stat)
{


    int value = p_ptr->state.stat_use[stat];

    if (value > 117) 		return(0);
    else if (value > 107)	return(1);
    else if (value > 87)	return(2);
    else if (value > 67)	return(3);
    else if (value > 17)	return(4);
    else if (value > 14)	return(7);
    else if (value > 7)		return(10);
    else	return(25);
}



/*
 * Returns chance of failure for a spell
 */
s16b spell_chance(int spell)
{
    int chance, minfail;

    const magic_type *s_ptr;


    /* Paranoia -- must be literate */
    if (!cp_ptr->spell_book) return (100);

    /* Get the spell */
    s_ptr = &mp_ptr->info[spell];

    /* Extract the base spell failure rate */
    chance = s_ptr->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (p_ptr->lev - s_ptr->slevel);

    /* Reduce failure rate by INT/WIS adjustment */
    /* Extract the minimum failure rate */
    if (game_mode == GAME_NPPMORIA)
    {
        chance -= 3 * (stat_adj_moria(MORIA_SPELL_STAT)-1);
    }
    else chance -= adj_mag_stat[SPELL_STAT_SLOT];

    /* Not enough mana to cast */
    if (s_ptr->smana > p_ptr->csp)
    {
        chance += 5 * (s_ptr->smana - p_ptr->csp);
    }

    /* Extract the minimum failure rate */
    if (game_mode == GAME_NPPMORIA) minfail = spell_failure_min_moria(MORIA_SPELL_STAT);
    else minfail = adj_mag_fail[SPELL_STAT_SLOT];

    /* Non mage/priest characters never get better than 5 percent */
    if (!(cp_ptr->flags & CF_ZERO_FAIL))
    {
        if (minfail < 5) minfail = 5;
    }

    /* Priest prayer penalty for "edged" weapons (before minfail) */
    if (p_ptr->state.icky_wield)
    {
        chance += 25;
    }

    /* Minimum failure rate */
    if (chance < minfail) chance = minfail;

    /* Stunning makes spells harder (after minfail) */
    if (p_ptr->timed[TMD_STUN] > 50) chance += 25;
    else if (p_ptr->timed[TMD_STUN]) chance += 15;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    /* Return the chance */
    return (chance);
}


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


byte spells[SPELLS_PER_BOOK];
byte num_spells;
int spell_mode;
#define SPELL_DISP_ROW		0
#define SPELL_DISP_COL		6
#define SPELL_DISP_WIDTH	70
#define SPELL_DISP_HGT		15
#define SPELL_ERROR			-3
int spell_pick;

static byte update_spells(object_type *o_ptr)
{

    int i;

    byte count = 0;

    /* Make sure it is the right spellcasting realm. */
    if (cp_ptr->spell_book != o_ptr->tval) return (0);

    /* Clear everything to start */
    for (i = 0;  i < SPELLS_PER_BOOK; i++)
    {
        spells[i] = -1;
    }

    /* Extract spells */
    for (i = 0; i < SPELLS_PER_BOOK; i++)
    {
        int spell = get_spell_index(o_ptr->sval, i);

        /* Collect this spell */
        if (spell != -1) spells[count++] = spell;
    }


    return (count);
}



/**
 * Display list available spell possibilities.
 * Returns the actual spell to cast/study.
 * Returns -1 if the user hits escape.
 * Returns -2 if there are no legal choices.
 * Also handles browsing.
 * Assumes spellbook is appropriate for the spellcasting class
 */
int get_spell_menu(const object_type *o_ptr, int mode_dummy)
{
    int spell_pick;

    // TODO get spell pick

    return (spell_pick);
}


/*
 * Is the player capable of casting a spell?
 */
bool player_can_cast(void)
{
    if (!cp_ptr->spell_book)
    {
        message(QString("You cannot cast spells!"));
        return FALSE;
    }

    if (p_ptr->timed[TMD_BLIND] || no_light())
    {
        message(QString("You cannot see!"));
        return FALSE;
    }

    if (p_ptr->timed[TMD_CONFUSED])
    {
        message(QString("You are too confused!"));
        return FALSE;
    }

    return TRUE;
}


/*
 * Is the player capable of studying?
 */
bool player_can_study(void)
{
    if (!player_can_cast())
        return FALSE;

    if (!p_ptr->new_spells)
    {
        QString p = cast_spell(MODE_SPELL_NOUN, cp_ptr->spell_book, 1, 0);
        message(QString("You cannot learn any new %1s!") .arg(p));
        return FALSE;
    }

    return TRUE;
}


/*
 * Check if the given spell is in the given book.
 */
static bool spell_in_book(int spell, int book)
{
    int i;
    object_type *o_ptr = object_from_item_idx(book);

    for (i = 0; i < SPELLS_PER_BOOK; i++)
    {
        if (spell == get_spell_index(o_ptr->sval, i)) return TRUE;
    }

    return FALSE;
}


/*
 * Gain a specific spell, specified by spell number (for mages).
 */
void do_cmd_study_spell(int code, cmd_arg args[])
{
    int spell = args[0].choice;

    int item_list[INVEN_TOTAL + MAX_FLOOR_STACK];
    int item_num;
    int i;

    /* Check the player can study at all atm */
    if (!player_can_study())
        return;

    /* Check that the player can actually learn the nominated spell. */
    item_tester_hook = obj_can_browse;
    item_num = scan_items(item_list, N_ELEMENTS(item_list), (USE_INVEN | USE_FLOOR));

    /* Check through all available books */
    for (i = 0; i < item_num; i++)
    {
        if (spell_in_book(spell, item_list[i]))
        {
            if (spell_okay(spell, FALSE))
            {
                /* Spell is in an available book, and player is capable. */
                spell_learn(spell);
                process_player_energy(BASE_ENERGY_MOVE);
            }
            else
            {
                /* Spell is present, but player incapable. */
                message(QString("You cannot learn that spell."));
            }

            return;
        }
    }
}


/*
 * See if we can cast or study from a book
 */
bool player_can_use_book(const object_type *o_ptr, bool known)
{
    int i;

    /* Check the player can study at all, and the book is the right type */
    if (!cp_ptr->spell_book) return FALSE;
    if (p_ptr->timed[TMD_BLIND] || no_light()) return FALSE;
    if (p_ptr->timed[TMD_CONFUSED]) return FALSE;
    if (o_ptr->tval != cp_ptr->spell_book) return (FALSE);

    /* Extract spells */
    for (i = 0; i < SPELLS_PER_BOOK; i++)
    {
        int s = get_spell_index(o_ptr->sval, i);

        /* Skip non-OK spells */
        if (s == -1) continue;
        if (!spell_okay(s, known)) continue;

        /* We found a spell to study/cast */
        return (TRUE);
    }

    /* No suitable spells */
    return (FALSE);
}


/*
 * Gain a random spell from the given book (for priests)
 */
void do_cmd_study_book(int code, cmd_arg args[])
{
    int book = args[0].item;
    object_type *o_ptr = object_from_item_idx(book);

    int spell = -1;
    int i, k = 0;

    QString p = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

    /* Check the player can study at all atm */
    if (!player_can_study())
        return;

    /* Check that the player has access to the nominated spell book. */
    if (!item_is_available(book, obj_can_browse, (USE_INVEN | USE_FLOOR)))
    {
        message(QString("That item is not within your reach."));
        return;
    }

    /* Extract spells */
    for (i = 0; i < SPELLS_PER_BOOK; i++)
    {
        int s = get_spell_index(o_ptr->sval, i);

        /* Skip non-OK spells */
        if (s == -1) continue;
        if (!spell_okay(s, FALSE)) continue;

        /* Apply the randomizer */
        if ((++k > 1) && (randint0(k) != 0)) continue;

        /* Track it */
        spell = s;
    }

    if (spell < 0)
    {
        message(QString("You cannot learn any %1s in that book.") .arg(p));
    }
    else
    {
        /* Remember we have used this book */
        object_kind *k_ptr = &k_info[o_ptr->k_idx];
        k_ptr->tried = TRUE;

        spell_learn(spell);
        process_player_energy(BASE_ENERGY_MOVE);
    }
}


/*
 * Cast a spell from a book
 */
void do_cmd_cast(int code, cmd_arg args[])
{
    int spell = args[0].choice;
    int dir = args[1].direction;

    int chance;

    QString noun = cast_spell(MODE_SPELL_NOUN, cp_ptr->spell_book, 1, 0);
    QString verb = cast_spell(MODE_SPELL_VERB, cp_ptr->spell_book, 1, 0);

    const magic_type *s_ptr;

    /* Get the spell */
    s_ptr = &mp_ptr->info[spell];

    /* Verify insufficient mana */
    if (s_ptr->smana > p_ptr->csp)
    {
        /* Warning */
        message(QString("You do not have enough mana to %1 this %2.") .arg(verb) .arg(noun));

        /* Verify */
        if (!get_check("Attempt it anyway? ")) return;
    }

    /* Spell failure chance */
    chance = spell_chance(spell);

    /* Failed spell */
    if (rand_int(100) < chance)
    {
        message(QString("You failed to get the spell off!"));
    }

    /* Process spell */
    else
    {
        /* Cast the spell */
        sound(MSG_SPELL);

        if (cast_spell(MODE_SPELL_CAST, cp_ptr->spell_book, spell, dir) == NULL) return;

        /* A spell was cast */
        if (!(p_ptr->spell_flags[spell] & PY_SPELL_WORKED))
        {
            int e = s_ptr->sexp;

            /* The spell worked */
            p_ptr->spell_flags[spell] |= PY_SPELL_WORKED;

            /* Gain experience */
            gain_exp(e * s_ptr->slevel);

            /* Redraw object recall */
            p_ptr->redraw |= (PR_OBJECT);
        }
    }



    /* Sufficient mana */
    if (s_ptr->smana <= p_ptr->csp)
    {
        /* Use some mana */
        p_ptr->csp -= s_ptr->smana;
    }

    /* Over-exert the player */
    else
    {
        int oops = s_ptr->smana - p_ptr->csp;

        /* No mana left */
        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        /* Message */
        message(QString("You faint from the effort!"));

        /* Hack -- Bypass free action */
        (void)inc_timed(TMD_PARALYZED, randint1(5 * oops + 1), TRUE);

        /* Damage CON (possibly permanently) */
        if (rand_int(100) < 50)
        {
            bool perm = (rand_int(100) < 25);

            /* Message */
            message(QString("You have damaged your health!"));

            /* Reduce constitution */
            (void)dec_stat(A_CON, 15 + randint(10), perm);
        }
    }

    /* Redraw mana */
    p_ptr->redraw |= (PR_MANA);

    /* Take a turn */
    process_player_energy(BASE_ENERGY_MOVE);
}


/*
 * Learn the specified spell.
 */
void spell_learn(int spell)
{
    int i;
   QString p = cast_spell(MODE_SPELL_NOUN, cp_ptr->spell_book, 1, 0);

    /* Learn the spell */
    p_ptr->spell_flags[spell] |= PY_SPELL_LEARNED;

    /* Find the next open entry in "spell_order[]" */
    for (i = 0; i < PY_MAX_SPELLS; i++)
    {
        /* Stop at the first empty space */
        if (p_ptr->spell_order[i] == 99) break;
    }

    /* Add the spell to the known list */
    p_ptr->spell_order[i] = spell;

    /* Mention the result */
    //message_format(MSG_STUDY, 0, "You have learned the %s of %s.", p, get_spell_name(cp_ptr->spell_book, spell));
    message(QString("You have learned the %1 of %2.") .arg(p) .arg(get_spell_name(cp_ptr->spell_book, spell)));

    /* One less spell available */
    p_ptr->new_spells--;

    /* Message if needed */
    if (p_ptr->new_spells)
    {
        if (p_ptr->new_spells > 1) p.append("s");

        /* Message */
        message(QString("You can learn %1 more %2.") .arg(p_ptr->new_spells) .arg(p));
    }

    /* Redraw Study Status */
    p_ptr->redraw |= (PR_STUDY | PR_OBJECT);
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


int get_spell_index(int sval, int index)
{
    return get_spell_from_list(sval,index);
}



