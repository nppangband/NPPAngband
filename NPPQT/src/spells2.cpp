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
            message("You feel a little better.");
        }

        /* Heal 5-14 */
        else if (num < 15)
        {
            message("You feel better.");
        }

        /* Heal 15-34 */
        else if (num < 35)
        {
            message("You feel much better.");
        }

        /* Heal 35+ */
        else
        {
            message("You feel very good.");
        }

        /* Notice */
        return (TRUE);
    }

    /* Ignore */
    return (FALSE);
}

/*
 * Array of stat "descriptions"
 */
static QString desc_stat_pos[A_MAX] =
{
    "strong",
    "smart",
    "wise",
    "dextrous",
    "healthy",
    "cute"
};


/*
 * Array of stat "descriptions"
 */
static QString desc_stat_neg[A_MAX] =
{
    "weak",
    "stupid",
    "naive",
    "clumsy",
    "sickly",
    "ugly"
};

/*
 * Array of stat "descriptions"
 */
static QString moria_desc_stat_sus_pos[A_MAX] =
{
    "You feel weaker for a moment, but it passes.",
    "You have trouble thinking clearly.  But your mind quickly clears.",
    "Your wisdom is sustained.",
    "You feel clumsy for a moment, but it passes.",
    "Your body resists the effects of the disease.",
    "Your skin starts to itch, but instantly feels better."
};

/*
 * Array of stat "descriptions"
 */
static QString moria_desc_stat_dec_pos[A_MAX] =
{
    "You feel weaker.",
    "You have trouble thinking clearly.",
    "Your wisdom is drained.",
    "You feel more clumsy.",
    "Your health is damaged!",
    "Your skin starts to itch."
};

/*
 * Increase a stat by one randomized level
 *
 * Most code will "restore" a stat before calling this function,
 * in particular, stat potions will always restore the stat and
 * then increase the fully restored value.
 */
bool inc_stat(int stat)
{
    int value, gain;

    /* Then augment the current/max stat */
    value = p_ptr->stat_cur[stat];

    /* Cannot go above 18/100 */
    if (value < 18+100)
    {
        /* Gain one (sometimes two) points */
        if (value < 18)
        {
            gain = ((rand_int(100) < 75) ? 1 : 2);
            value += gain;
        }

        /* Gain 1/6 to 1/3 of distance to 18/100 */
        else if (value < 18+98)
        {
            /* Approximate gain value */
            gain = (((18+100) - value) / 2 + 3) / 2;

            /* Paranoia */
            if (gain < 1) gain = 1;

            /* Apply the bonus */
            value += randint(gain) + gain / 2;

            /* Maximal value */
            if (value > 18+99) value = 18 + 99;
        }

        /* Gain one point at a time */
        else
        {
            value++;
        }

        /* Save the new value */
        p_ptr->stat_cur[stat] = value;

        /* Bring up the maximum too */
        if (value > p_ptr->stat_max[stat])
        {
            p_ptr->stat_max[stat] = value;
        }

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to gain */
    return (FALSE);
}



/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool dec_stat(int stat, int amount,bool permanent)
{
    int cur, max, loss, same, res = FALSE;


    /* Get the current value */
    cur = p_ptr->stat_cur[stat];
    max = p_ptr->stat_max[stat];

    /* Note when the values are identical */
    same = (cur == max);

    /* Damage "current" value */
    if (cur > 3)
    {
        /* Handle "low" values */
        if (cur <= 18)
        {
            if (amount > 90) cur--;
            if (amount > 50) cur--;
            if (amount > 20) cur--;
            cur--;
        }

        /* Handle "high" values */
        else
        {
            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((cur-18) / 2 + 1) / 2 + 1);

            /* Paranoia */
            if (loss < 1) loss = 1;

            /* Randomize the loss */
            loss = ((randint(loss) + loss) * amount) / 100;

            /* Maximal loss */
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            cur = cur - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (cur < 18) cur = (amount <= 20) ? 18 : 17;
        }

        /* Prevent illegal values */
        if (cur < 3) cur = 3;

        /* Something happened */
        if (cur != p_ptr->stat_cur[stat]) res = TRUE;
    }

    /* Damage "max" value */
    if (permanent && (max > 3))
    {
        /* Handle "low" values */
        if (max <= 18)
        {
            if (amount > 90) max--;
            if (amount > 50) max--;
            if (amount > 20) max--;
            max--;
        }

        /* Handle "high" values */
        else
        {
            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((max-18) / 2 + 1) / 2 + 1);
            if (loss < 1) loss = 1;
            loss = ((randint(loss) + loss) * amount) / 100;
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            max = max - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (max < 18) max = (amount <= 20) ? 18 : 17;
        }

        /* Hack -- keep it clean */
        if (same || (max < cur)) max = cur;

        /* Something happened */
        if (max != p_ptr->stat_max[stat]) res = TRUE;
    }

    /* Apply changes */
    if (res)
    {
        /* Actually set the stat to its new value. */
        p_ptr->stat_cur[stat] = cur;
        p_ptr->stat_max[stat] = max;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);
    }

    /* Done */
    return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
    /* Restore if needed */
    if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
    {
        /* Restore */
        p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to restore */
    return (FALSE);
}


/*
 * Lose a "point"
 */
bool do_dec_stat(int stat)
{
    bool sust = FALSE;

    /* Get the "sustain" */
    switch (stat)
    {
        case A_STR: if (p_ptr->state.sustain_str) sust = TRUE; break;
        case A_INT: if (p_ptr->state.sustain_int) sust = TRUE; break;
        case A_WIS: if (p_ptr->state.sustain_wis) sust = TRUE; break;
        case A_DEX: if (p_ptr->state.sustain_dex) sust = TRUE; break;
        case A_CON: if (p_ptr->state.sustain_con) sust = TRUE; break;
        case A_CHR: if (p_ptr->state.sustain_chr) sust = TRUE; break;
    }

    /* Sustain */
    if (sust)
    {
        /* Message */
        if (game_mode == GAME_NPPMORIA) message(QString("%1") .arg(moria_desc_stat_sus_pos[stat]));
        else message(QString("You feel very %1 for a moment, but the feeling passes.") .arg(desc_stat_neg[stat]));

        /* Notice effect */
        return (TRUE);
    }

    /* Attempt to reduce the stat */
    if (dec_stat(stat, 10, FALSE))
    {
        /* Message */
        if (game_mode == GAME_NPPMORIA) message(QString("%1") .arg(moria_desc_stat_dec_pos[stat]));
                else message(QString("You feel very %1.") .arg(desc_stat_neg[stat]));

        /* Notice effect */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}

/*
 * Array of stat "descriptions"
 */
static QString moria_desc_stat_res_pos[A_MAX] =
{
    "You feel your strength returning.",
    "Your head spins a moment.",
    "You feel your wisdom returning.",
    "You feel more dextrous.",
    "You feel your health returning.",
    "You feel your looks returning."
};



/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat)
{
    /* Attempt to increase */
    if (res_stat(stat))
    {
        /* Message */
        if (game_mode == GAME_NPPMORIA) message(QString("%1") .arg(moria_desc_stat_res_pos[stat]));
        else message(QString("You feel less %1.") .arg(desc_stat_neg[stat]));

        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}

/*
 * Array of stat "descriptions"
 */
static QString moria_desc_stat_inc_pos[A_MAX] =
{
    "Wow!  What bulging muscles!",
    "Aren't you brilliant!",
    "You suddenly have a profound thought!",
    "You feel more limber!",
    "You feel tingly for a moment.",
    "Gee, ain't you cute!"
};



/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
    bool res;

    /* Restore stat first */
    res = res_stat(stat);

    /* Attempt to increase */
    if (inc_stat(stat))
    {
        /* Message */
        if (game_mode == GAME_NPPMORIA) message(QString("%s") .arg(moria_desc_stat_inc_pos[stat]));
        else message(QString("You feel very %s!") .arg(desc_stat_pos[stat]));

        /* Notice */
        return (TRUE);
    }

    /* Restoration worked */
    if (res)
    {
        /* Message */
        message(QString("You feel less %1.") .arg(desc_stat_neg[stat]));

        /* Notice */
        return (TRUE);
    }

    /* Nothing obvious */
    return (FALSE);
}

/*
 * Permanently gain a "point" in a stat
 */
void do_perm_stat_boost(int stat)
{
    /* Restore stat first */
    (void)res_stat(stat);

    /* Increase stat*/
    p_ptr->stat_quest_add[stat]++;

    /* Message */
    message(QString("You feel very %1!") .arg(desc_stat_pos[stat]));

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redisplay the stats later */
    p_ptr->redraw |= (PR_STATS);
}

/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
    int i;

    /* Simply identify and know every item */
    for (i = 0; i < ALL_INVEN_TOTAL; i++)
    {
        object_type *o_ptr = &inventory[i];

        bool aware = FALSE;

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Remember awareness */
        if (o_ptr->is_aware()) aware = TRUE;

        /* Aware and Known */
        object_aware(o_ptr);
        object_known(o_ptr);

        if (!aware) apply_autoinscription(o_ptr);
    }

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS | PU_NATIVE);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP);

}

/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "light_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_light(void)
{
    int i;


    /* Apply flag changes */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* No longer in the array */
        dungeon_info[y][x].cave_info &= ~(CAVE_TEMP);

        /* Perma-Lite */
        dungeon_info[y][x].cave_info |= (CAVE_GLOW);
    }

    /* Fully update the visuals */
    p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

    /* Update stuff */
    update_stuff();

    /* Process the grids */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* Redraw the grid */
        light_spot(y, x);

        /* Process affected monsters */
        if (dungeon_info[y][x].monster_idx > 0)
        {
            int chance = 25;

            int m_idx = dungeon_info[y][x].monster_idx;
            monster_type *m_ptr = &mon_list[m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Stupid monsters rarely wake up */
            if (r_ptr->flags2 & (RF2_STUPID)) chance = 10;

            /* Smart monsters always wake up */
            if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

            /* Sometimes monsters wake up */
            if ((m_ptr->m_timed[MON_TMD_SLEEP]) && (rand_int(100) < chance))
            {
                /* Wake up! */
                wake_monster_attack(m_ptr, MON_TMD_FLG_NOTIFY);

                /*possibly update the monster health bar*/
                if ((p_ptr->health_who == m_idx)  || (m_ptr->sidebar))
                    p_ptr->redraw |= (PR_HEALTH);
            }
        }

    }

    /* None left */
    temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlight_room()"
 */
static void cave_temp_room_unlite(void)
{
    int i;

    /* Apply flag changes */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* No longer in the array */
        dungeon_info[y][x].cave_info &= ~(CAVE_TEMP);

        /* Turn off the light */
        dungeon_info[y][x].cave_info &= ~(CAVE_GLOW);

        /* Hack -- Forget "boring" grids */
        if (!(dungeon_info[y][x].cave_info & (CAVE_HALO)) &&
            !cave_ff1_match(y, x, FF1_REMEMBER) &&
            (!cave_any_trap_bold(y, x) ||
                (x_list[dungeon_info[y][x].effect_idx].x_flags &
                (EF1_HIDDEN))))
        {
            /* Forget the grid */
            dungeon_info[y][x].cave_info &= ~(CAVE_MARK);
        }
    }

    /* Fully update the visuals */
    p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

    /* Update stuff */
    update_stuff();

    /* Process the grids */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* Redraw the grid */
        light_spot(y, x);


    }

    /* None left */
    temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
    /* Avoid infinite recursion */
    if (dungeon_info[y][x].cave_info & (CAVE_TEMP)) return;

    /* Do not "leave" the current room */
    if (!(dungeon_info[y][x].cave_info & (CAVE_ROOM))) return;

    /* Paranoia -- verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid as "seen" */
    dungeon_info[y][x].cave_info |= (CAVE_TEMP);

    /* Add it to the "seen" set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void light_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get lit, but stop light */
        if (!cave_project_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, lite them all up at once */
    cave_temp_room_light();

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);

}


/*
 * Darken all rooms containing the given location
 */
void unlight_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get dark, but stop darkness */
        if (!cave_project_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, darken them all at once */
    cave_temp_room_unlite();

}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool light_area(int dam, int rad)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    u32b flg = PROJECT_BOOM | PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->timed[TMD_BLIND])
    {
        message("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    // TODO (void)project(SOURCE_PLAYER, rad, py, px, py, px, dam, GF_LIGHT_WEAK, flg, 0, 0);

    /* Lite up the room */
    light_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlight_area(int dam, int rad)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    u32b flg = PROJECT_BOOM | PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->timed[TMD_BLIND])
    {
        message("Darkness surrounds you.");
    }

    /* Hook into the "project()" function */
    //TODO (void)project(SOURCE_PLAYER, rad, py, px, py, px, dam, GF_DARK_WEAK, flg, 0, 0);

    /* Lite up the room */
    unlight_room(py, px);

    /* Assume seen */
    return (TRUE);
}

/*
 * Get a spell type (GF_*) for the given terrain feature.
 * The spell type is stored in gf_type.
 * A description of the effect on an object is stored in action.
 * Both gf_type and action can be NULL.
 */
QString get_spell_type_from_feature(int f_idx, int *gf_type)
{
    /* Get the element flags */
    u32b element = feat_ff3_match(f_idx, TERRAIN_MASK);
    u16b i;
    QString desc;


    /*
     * Spell information for each element type.
     */
    static struct
    {
        u32b element;
        int gf_type;
        QString action;
    } spell_info[] =
    {
        {ELEMENT_BWATER, GF_BWATER, "burns"},
        {ELEMENT_BMUD, GF_BMUD, "burns"},
        {ELEMENT_LAVA, GF_LAVA, "burns"},
        {ELEMENT_FIRE, GF_FIRE, "burns"},
        {ELEMENT_ICE, GF_ICE, "freezes"},
        {ELEMENT_WATER, GF_WATER, "hurts"},
        {ELEMENT_SAND, GF_SAND, "hurts"},
        {ELEMENT_ACID, GF_ACID, "hurts"}
    };

    /* Save default spell type */
    *gf_type = GF_MANA;

    /* Save default action */
    desc = ("hurts");

    /* Find the element in the list */
    for (i = 0; i < N_ELEMENTS(spell_info); i++)
    {
        /* Found the element? */
        if (spell_info[i].element == element)
        {
            /* Save the spell type */
            *gf_type = spell_info[i].gf_type;

            /* Save the action */
            desc = (spell_info[i].action);
        }
    }

    return (desc);
}

/*
 * Return TRUE if the player is immune to the effects of the given
 * spell type
 */
bool is_player_immune(int gf_type)
{
    /* Check immunities */
    switch (gf_type)
    {
        case GF_ACID:
        {
            if (p_ptr->state.immune_acid) return (TRUE);

            break;
        }
        case GF_ICE:
        case GF_COLD:
        {
            if (p_ptr->state.immune_cold) return (TRUE);

            break;
        }
        case GF_FIRE:
        {
            if (p_ptr->state.immune_fire) return (TRUE);

            break;
        }
        case GF_ELEC:
        {
            if (p_ptr->state.immune_elec) return (TRUE);

            break;
        }
        case GF_POIS:
        {
            if (p_ptr->state.immune_pois) return (TRUE);

            break;
        }
    }

    return (FALSE);
}

/*
 * Hack -- Removes curse from an object.
 */
void uncurse_object(object_type *o_ptr)
{
    /* Uncurse it */
    o_ptr->ident &= ~(IDENT_CURSED);

    /* Remove special inscription, if any */
    if (o_ptr->discount >= INSCRIP_NULL) o_ptr->discount = 0;

    /* Take note if allowed */
    if (o_ptr->discount == 0) o_ptr->discount = INSCRIP_UNCURSED;

    /* The object has been "sensed" */
    o_ptr->ident |= (IDENT_SENSE);
}


/*
 * Removes curses from items in inventory.
 *
 * \param heavy removes heavy curses if true
 *
 * \returns number of items uncursed
 */
static int remove_curse_aux(bool heavy)
{
    int i, cnt = 0;

    /* Attempt to uncurse items being worn */
    for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
    {
        u32b f1, f2, f3, fn;

        object_type *o_ptr = &inventory[i];
        QString o_name;

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Uncursed already */
        if (!o_ptr->is_cursed()) continue;

        /* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3, &fn);

        /* Heavily Cursed Items need a special spell */
        if (!heavy && (f3 & (TR3_HEAVY_CURSE))) continue;

        /* Perma-Cursed Items can NEVER be uncursed */
        if (f3 & (TR3_PERMA_CURSE)) continue;

        /* Uncurse the object */
        uncurse_object(o_ptr);

        o_name = object_desc(o_ptr, ODESC_BASE);

        message(QString("The curse on your %1 is broken!") .arg(o_name));

        /* Recalculate the bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Count the uncursings */
        cnt++;
    }
    /* Combine and re-order the pack - and redraw stuff */
    if (cnt)
    {
        p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
        p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);
    }

    else message("Nothing happens.");

    /* Return "something uncursed" */
    return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(bool heavy)
{
    return (remove_curse_aux(heavy));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
    return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level(void)
{
    /* Restore experience */
    if (p_ptr->exp < p_ptr->max_exp)
    {
        /* Message */
        message("You feel your life energies returning.");

        /* Restore the experience */
        p_ptr->exp = p_ptr->max_exp;

        /* Check the experience */
        // TODO check_experience();

        /* Did something */
        return (TRUE);
    }

    /* No effect */
    return (FALSE);
}

void identify_object(object_type *o_ptr, bool star_ident)
{
    /* Identify it */
    object_aware(o_ptr);
    object_known(o_ptr);

    /* Apply an autoinscription, if necessary */
    apply_autoinscription(o_ptr);

    /*   *identify the item if called for*/
    if (star_ident)
    {
        /* Mark the item as fully known */
        o_ptr->ident |= (IDENT_MENTAL);
    }

    p_ptr->redraw |= (PR_ITEMLIST);
}


/*
 * Execute some common code of the identify spells.
 * "item" is used to print the slot occupied by an object in equip/inven.
 * ANY negative value assigned to "item" can be used for specifying an object
 * on the floor (they don't have a slot, example: the code used to handle
 * GF_MASS_IDENTIFY in project_o).
 * It returns the value returned by squelch_itemp.
 * The object is NOT squelched here.
 */
int do_ident_item(int item, object_type *o_ptr)
{
    QString o_name;
    int squelch = SQUELCH_NO;
    byte color = TERM_WHITE;

    /* In Moria, mark the item as fully known */
    if (game_mode == GAME_NPPMORIA) o_ptr->ident |= (IDENT_MENTAL);

    /* Identify it */
    object_aware(o_ptr);
    object_known(o_ptr);

    /* Apply an autoinscription, if necessary */
    apply_autoinscription(o_ptr);

    /* Squelch it? */
    if (item < INVEN_WIELD) squelch = squelch_itemp(o_ptr, 0, TRUE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS | PU_NATIVE);

    p_ptr->redraw |= (PR_EXP | PR_STATS | PR_INVEN | PR_EQUIP | PR_ITEMLIST);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

    /* Description */
    o_name  = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Display independent of cursedness */
    if(o_ptr->art_num)
        color = TERM_BLUE;
    else if (o_ptr->ego_num)
        color = TERM_GREEN;

    /* Describe */
    if (item >= INVEN_WIELD)
    {
        color_message(QString("%^1: %2 (%3).") .arg(describe_use(item)) .arg(o_name) .arg(index_to_label(item)), color);
    }
    else if (item >= 0)
    {
        color_message(QString("In your pack: %1 (%2).  %3") .arg(o_name) .arg(index_to_label(item)) .arg(squelch_to_label(squelch)), color);
    }
    else
    {
        color_message(QString("On the ground: %1.  %2") .arg(o_name)  .arg(squelch_to_label(squelch)), color);
    }

    /*
     * If the item was an artifact, and if the auto-note is selected,
     * write a message.
     */
        if ((adult_take_notes) && o_ptr->is_artifact() && (o_ptr->xtra1 >= 1))
    {
        int artifact_depth;
        QString note;
        QString shorter_desc;

        /* Get a shorter description to fit the notes file */
        shorter_desc = object_desc(o_ptr, ODESC_PREFIX | ODESC_BASE);

        /* Build note and write */
        note = (QString("Found %1") .arg(shorter_desc));

        /* Record the depth where the artifact was created */
        artifact_depth = o_ptr->xtra1;

        write_note(note, artifact_depth);

        /*
         * Mark item creation depth 0, which will indicate the artifact
         * has been previously identified.  This prevents an artifact
         * from showing up on the notes list twice ifthe artifact had
         * been previously identified.  JG
         */
        o_ptr->xtra1 = 0 ;
    }

    /* Check if the object is an artifact known from a previous game */
    if (!(o_ptr->ident & (IDENT_MENTAL)) && ARTIFACT_EASY_MENTAL(o_ptr)
        && a_l_list[o_ptr->art_num].was_fully_identified)
    {
        /* Message */
        color_message("You are already familiar with this artifact.", TERM_BLUE);

        /* Fully identify the artifact for free */
        o_ptr->ident |= (IDENT_MENTAL);
    }

    return (squelch);
}

static bool item_tester_unknown(object_type *o_ptr)
{
    if (object_known_p(o_ptr))
        return FALSE;
    else
        return TRUE;
}


static bool item_tester_unknown_star(object_type *o_ptr)
{
    if (o_ptr->ident & IDENT_MENTAL)
        return FALSE;
    else
        return TRUE;
}

/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell(void)
{
    int item;

    int squelch;

    object_type *o_ptr;

    QString q, s;

    /* Only un-id'ed items */
    item_tester_hook = item_tester_unknown;

    /* Get an item */
    q = "Identify which item? ";
    s = "You have nothing to identify.";
    // TODO if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER))) return (FALSE);

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Identify the object and get squelch setting */
    squelch = do_ident_item(item, o_ptr);

    /* Now squelch it if needed */
    do_squelch_item(squelch, item, o_ptr);

    /* Something happened */
    return (TRUE);
}



/*
 * Fully "identify" an object in the inventory
 *
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully(void)
{
    int item;

    int squelch;

    object_type *o_ptr;

    QString q, s;

    /* Only un-*id*'ed items */
    item_tester_hook = item_tester_unknown_star;

    /* Get an item */
    q = "Identify which item? ";
    s = "You have nothing to identify.";
    // TODO if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER))) return (FALSE);

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Mark the item as fully known */
    o_ptr->ident |= (IDENT_MENTAL);

    /* Identify the object and get the squelch setting */
    squelch = do_ident_item(item, o_ptr);

    /* Now squelch it if needed */
    if (squelch == SQUELCH_YES)
    {
        do_squelch_item(squelch, item, o_ptr);
    }

    else
    {
        /* Describe it fully */
        // TODO object_info_screen(o_ptr);
    }

    /* Check for easy mental feature (artifacts) */
    if (ARTIFACT_EASY_MENTAL(o_ptr))
    {
        artifact_lore *a_l_ptr = &a_l_list[o_ptr->art_num];

        /* Message, keep commented out for now */

        /* Remember that we *identified* this artifact */
        a_l_ptr->was_fully_identified = TRUE;
    }

    /* Success */
    return (TRUE);
}


/*
 * Re-charge a staff or wand, and remove the identification
 * If there is a chance for the re-charge to fail, that
 * should be checked before this function is called.
 */
void recharge_staff_wand(object_type *o_ptr, int percent)
{
    int recharge_amount;

    if (o_ptr->tval == TV_WAND) recharge_amount = charge_wand(o_ptr, percent);
    else if (o_ptr->tval == TV_STAFF) recharge_amount = charge_staff(o_ptr, percent);
    /* Paranoia */
    else return;

    /* Handle stacks of wands/staves, with diminishing returns */
    if (o_ptr->number > 1)
    {
        if (o_ptr->tval == TV_WAND) recharge_amount += charge_wand(o_ptr, (percent * 4 / 10));
        else if (o_ptr->tval == TV_STAFF) recharge_amount += charge_staff(o_ptr, (percent * 4 / 10));
    }

    /* Little increase for a stack greater than two */
    if (o_ptr->number > 2) recharge_amount += (o_ptr->number - 2);

    /* Recharge the wand or staff. */
    o_ptr->pval += recharge_amount;

    /* *Identified* items keep the knowledge about the charges */
    if (!(o_ptr->ident & IDENT_MENTAL))
    {
        /* We no longer "know" the item */
        o_ptr->ident &= ~(IDENT_KNOWN);
    }

    /* Hack -- we no longer think the item is empty */
    o_ptr->ident &= ~(IDENT_EMPTY);

}


/*
 * Used by the "enchant" function (chance of failure)
 */
static const int enchant_table[ENCHANT_MAX + 1] =
{
    0, 20,  50, 150, 300,
    450, 600, 750,
    900, 990
};

/*
 * Enchant an item
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
    int i, chance, prob;

    bool res = FALSE;

    bool a = o_ptr->is_artifact();

    u32b f1, f2, f3, fn;

    /* Extract the flags */
    object_flags(o_ptr, &f1, &f2, &f3, &fn);

    /* Large piles resist enchantment */
    prob = o_ptr->number * 100;

    /* Missiles are easy to enchant */
    if ((o_ptr->tval == TV_BOLT) ||
        (o_ptr->tval == TV_ARROW) ||
        (o_ptr->tval == TV_SHOT))
    {
        prob = prob / 20;
    }

    /* Try "n" times */
    for (i=0; i<n; i++)
    {
        /* Hack -- Roll for pile resistance */
        if ((prob > 100) && (rand_int(prob) >= 100)) continue;

        /* Enchant to hit */
        if (eflag & (ENCH_TOHIT))
        {
            if (o_ptr->to_h < 0) chance = 0;
            else if (o_ptr->to_h > ENCHANT_MAX) chance = 1000;
            else chance = enchant_table[o_ptr->to_h];

            /* Attempt to enchant */
            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                res = TRUE;

                /* Enchant */
                o_ptr->to_h++;

                /* Break curse */
                if (o_ptr->is_cursed() &&
                    (!(f3 & (TR3_PERMA_CURSE))) &&
                    (o_ptr->to_h >= 0) && (rand_int(100) < 25))
                {
                    message("The curse is broken!");

                    /* Uncurse the object */
                    o_ptr->uncurse();
                }
            }
        }

        /* Enchant to damage */
        if (eflag & (ENCH_TODAM))
        {
            if (o_ptr->to_d < 0) chance = 0;
            else if (o_ptr->to_d > ENCHANT_MAX) chance = 1000;
            else chance = enchant_table[o_ptr->to_d];

            /* Attempt to enchant */
            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                res = TRUE;

                /* Enchant */
                o_ptr->to_d++;

                /* Break curse */
                if (o_ptr->is_cursed() &&
                    (!(f3 & (TR3_PERMA_CURSE))) &&
                    (o_ptr->to_d >= 0) && (rand_int(100) < 25))
                {
                    message("The curse is broken!");

                    /* Uncurse the object */
                    o_ptr->uncurse();
                }
            }
        }

        /* Enchant to armor class */
        if (eflag & (ENCH_TOAC))
        {
            if (o_ptr->to_a < 0) chance = 0;
            else if (o_ptr->to_a > ENCHANT_MAX) chance = 1000;
            else chance = enchant_table[o_ptr->to_a];

            /* Attempt to enchant */
            if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
            {
                res = TRUE;

                /* Enchant */
                o_ptr->to_a++;

                /* Break curse */
                if (o_ptr->is_cursed() &&
                    (!(f3 & (TR3_PERMA_CURSE))) &&
                    (o_ptr->to_a >= 0) && (rand_int(100) < 25))
                {
                    message("The curse is broken!");

                    /* Uncurse the object */
                    o_ptr->uncurse();
                }
            }
        }
    }

    /* Failure */
    if (!res) return (FALSE);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine / Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP  | PR_ITEMLIST);

    /* Success */
    return (TRUE);
}

/*
 * Brand weapons (or ammo)
 *
 * Turns the (non-magical) object into an ego-item of 'brand_type'.
 */
bool brand_object(object_type *o_ptr, byte brand_type, bool do_enchant)
{
    /* you can never modify artifacts / ego-items */
    /* you can never modify broken / cursed items */
    if ((o_ptr->k_idx) && !o_ptr->is_artifact() && !o_ptr->is_ego_item() && !o_ptr->is_broken() && !o_ptr->is_cursed())
    {
        QString act = "magical";
        QString o_name = object_desc(o_ptr, ODESC_BASE);

        /*Handle weapons differently than ammo*/
        if (wield_slot(o_ptr) == INVEN_WIELD)
        {
            /* Brand the object */
            o_ptr->ego_num = EGO_BRAND_ELEMENTS;

            o_ptr->xtra1 = OBJECT_XTRA_TYPE_BRAND;
            o_ptr->xtra2 = 1 << brand_type;
        }
        else
        {

            /* Brand the object */
            o_ptr->ego_num = brand_type;
        }

        switch (brand_type)
        {
            case BRAND_OFFSET_FLAME:
            case EGO_AMMO_FLAME:
            {
                act = "fiery";
                break;
            }
            case BRAND_OFFSET_FROST:
            case EGO_AMMO_FROST:
            {
                act = "frosty";
                break;
            }
            case BRAND_OFFSET_VENOM:
            case EGO_AMMO_VENOM:
            {
                act = "sickly";
                break;
            }
        }

        /* Describe */
        message(QString("A %1 aura surrounds the %1.") .arg(act) .arg(o_name));

        /* Combine / Reorder the pack (later) */
        p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

        /* Window stuff */
        p_ptr->redraw |= (PR_INVEN | PR_EQUIP  | PR_ITEMLIST);

        /* Enchant */
        if (do_enchant) enchant(o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);

        /* Hack - Identify it */
        object_aware(o_ptr);
        object_known(o_ptr);

        return (TRUE);
    }
    else
    {
        message("The Branding failed.");
    }

    return (FALSE);
}

/*
 * Brand the current weapon
 */
bool brand_weapon(bool enchant)
{
    object_type *o_ptr;
    byte brand_type;

    o_ptr = &inventory[INVEN_WIELD];

    /* Handle swap weapons */
    if (!o_ptr->is_weapon())
    {
        return (FALSE);
    }

    /* Select a brand */
    if (one_in_(3))
        brand_type = BRAND_OFFSET_FLAME;
    else
        brand_type = BRAND_OFFSET_FROST;

    /* Brand the weapon */
    return (brand_object(o_ptr, brand_type, enchant));
}

/*
 * Brand some (non-magical) ammo
 */
bool brand_ammo(bool enchant)
{
    int item;
    object_type *o_ptr;
    QString q, s;
    byte brand_type;

    /* Only accept ammo */
    item_tester_hook = item_tester_hook_ammo;

    /* Get an item */
    q = "Brand which kind of ammunition? ";
    s = "You have nothing to brand.";
    // TODO if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER | QUIVER_FIRST))) return (FALSE);

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Select the brand */
    if (one_in_(3))
        brand_type = EGO_AMMO_FLAME;
    else if (one_in_(2))
        brand_type = EGO_AMMO_FROST;
    else brand_type = EGO_AMMO_VENOM;

    /* Brand the ammo */
    return (brand_object(o_ptr, brand_type, enchant));

}


/*
 * Enchant some (non-magical) bolts
 */
bool brand_bolts(bool enchant)
{
    int item;
    object_type *o_ptr;
    QString q, s;


    /* Restrict choices to bolts */
    item_tester_tval = TV_BOLT;

    /* Get an item */
    q = "Brand which bolts? ";
    s = "You have no bolts to brand.";
    // TODO if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER | QUIVER_FIRST))) return (FALSE);

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    /* Brand the bolts */
    return (brand_object(o_ptr, EGO_AMMO_FLAME, enchant));

}



