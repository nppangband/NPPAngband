/* File: birth.cpp */

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

#include "npp.h"
#include "store.h"

#include <QTime>
#include <QDate>

/*
 * Cost of each "point" of a stat.
 */
static const int birth_stat_costs[18 + 1] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 4 };

static void get_bonuses(void)
{
    /* Fully healed */
    p_ptr->chp = p_ptr->mhp;

    /* Fully rested */
    p_ptr->csp = p_ptr->msp;
}

static void recalculate_stats(int *stats, int points_left)
{
    int i;

    /* Process stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Variable stat maxes */
        if (adult_maximize)
        {
            /* Reset stats */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = p_ptr->stat_birth[i] = stats[i];
        }

        /* Fixed stat maxes */
        else
        {
            /* Obtain a "bonus" for "race" and "class" */
            int bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

            /* Apply the racial/class bonuses */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = p_ptr->stat_birth[i] = modify_stat_value(stats[i], bonus);
        }
    }

    /* Gold is inversely proportional to cost */
    if (birth_money)
        p_ptr->au = 500;
    else
        p_ptr->au = 200 + (50 * points_left);

    p_ptr->au_birth = 200 + (50 * points_left);

    /* Update bonuses, hp, etc. */
    get_bonuses();
}

void reset_stats(int stats[A_MAX], int points_spent[A_MAX], int *points_left)
{
    int i;

    /* Calculate and signal initial stats and points totals. */
    *points_left = MAX_BIRTH_POINTS;

    for (i = 0; i < A_MAX; i++)
    {
        /* Initial stats are all 10 and costs are zero */
        stats[i] = 10;
        points_spent[i] = 0;
    }

    /* Use the new "birth stat" values to work out the "other"
       stat values (i.e. after modifiers) and tell the UI things have
       changed. */
    recalculate_stats(stats, *points_left);
}

bool buy_stat(int choice, int stats[A_MAX], int points_spent[A_MAX],
                     int *points_left)
{
    byte max_stat = (adult_maximize ? 18 : 17);

    /* Must be a valid stat, and have a "base" of below allowable max to be adjusted */
    if (!(choice >= A_MAX || choice < 0) &&	(stats[choice] < max_stat))
    {
        /* Get the cost of buying the extra point (beyond what
           it has already cost to get this far). */
        int stat_cost = birth_stat_costs[stats[choice] + 1];

        if (stat_cost <= *points_left)
        {
            stats[choice]++;
            points_spent[choice] += stat_cost;
            *points_left -= stat_cost;

            /* Recalculate everything that's changed because
               the stat has changed, and inform the UI. */
            recalculate_stats(stats, *points_left);

            return TRUE;
        }
    }

    /* Didn't adjust stat. */
    return FALSE;
}

bool sell_stat(int choice, int stats[A_MAX], int points_spent[A_MAX],
                      int *points_left)
{
    /* Must be a valid stat, and we can't "sell" stats below the base of 10. */
    if (!(choice >= A_MAX || choice < 0) && (stats[choice] > 10))
    {
        int stat_cost = birth_stat_costs[stats[choice]];

        stats[choice]--;
        points_spent[choice] -= stat_cost;
        *points_left += stat_cost;

        /* Recalculate everything that's changed because
           the stat has changed, and inform the UI. */
        recalculate_stats(stats, *points_left);

        return TRUE;
    }

    /* Didn't adjust stat. */
    return FALSE;
}

/*
 * This picks some reasonable starting values for stats based on the
 * current race/class combo, etc.  For now I'm disregarding concerns
 * about role-playing, etc, and using the simple outline from
 * http://angband.oook.cz/forum/showpost.php?p=17588&postcount=6:
 *
 * 0. buy base STR 17
 * 1. if possible buy adj DEX of 18/10
 * 2. spend up to half remaining points on each of spell-stat and con,
 *    but only up to max base of 16 unless a pure class
 *    [mage or priest or warrior]
 * 3. If there are any points left, spend as much as possible in order
 *    on DEX, non-spell-stat, CHR.
 */
void generate_stats(int stats[A_MAX], int points_spent[A_MAX],
                           int *points_left)
{
    int step = 0;
    int maxed[A_MAX] = { 0 };
    bool pure = FALSE;

    /* Determine whether the class is "pure" */
    if (cp_ptr->spell_book == 0 || cp_ptr-> max_attacks < 5)
    {
        pure = TRUE;
    }

    while (*points_left && step >= 0)
    {
        switch (step)
        {
            /* Buy base STR 17 */
            case 0:
            {
                if (!maxed[A_STR] && stats[A_STR] < 17)
                {
                    if (!buy_stat(A_STR, stats, points_spent, points_left))
                        maxed[A_STR] = TRUE;
                }
                else
                {
                    step++;
                }

                break;
            }

            /* Try and buy adj DEX of 18/10 */
            case 1:
            {
                if (!maxed[A_DEX] && p_ptr->state.stat_top[A_DEX] < 18+10)
                {
                    if (!buy_stat(A_DEX, stats, points_spent, points_left))
                        maxed[A_DEX] = TRUE;
                }
                else
                {
                    step++;
                }

                break;
            }

            /* If we can't get 18/10 dex, sell it back. */
            case 2:
            {
                if (p_ptr->state.stat_top[A_DEX] < 18+10)
                {
                    while (stats[A_DEX] > 10)
                        sell_stat(A_DEX, stats, points_spent, points_left);

                    maxed[A_DEX] = FALSE;
                }

                step++;

                break;
            }

            /*
             * Spend up to half remaining points on each of spell-stat and
             * con, but only up to max base of 16 unless a pure class
             * [mage or priest or warrior]
             */
            case 3:
            {
                int points_trigger = *points_left / 2;

                if (cp_ptr->spell_book)
                {
                    int spell_stat;

                    if (cp_ptr->spell_book == TV_MAGIC_BOOK) spell_stat = A_INT;
                    else if (cp_ptr->spell_book == TV_PRAYER_BOOK) spell_stat = A_WIS;
                    /*ugly hack for the druid and ranger class, which have 2 spell stats*/
                    else if (stats[A_INT] > stats[A_WIS]) spell_stat = A_WIS;
                    else /*(stats[A_WIS] >= stats[A_INT])*/ spell_stat = A_INT;



                    while (!maxed[spell_stat] &&
                           (pure || stats[spell_stat] < 16) &&
                           points_spent[spell_stat] < points_trigger)
                    {
                        if (!buy_stat(spell_stat, stats, points_spent,
                                      points_left))
                        {
                            maxed[spell_stat] = TRUE;
                        }

                        if (points_spent[spell_stat] > points_trigger)
                        {
                            sell_stat(spell_stat, stats, points_spent,
                                      points_left);
                            maxed[spell_stat] = TRUE;
                        }
                    }
                }

                while (!maxed[A_CON] &&
                       (pure || stats[A_CON] < 16) &&
                       points_spent[A_CON] < points_trigger)
                {
                    if (!buy_stat(A_CON, stats, points_spent,points_left))
                    {
                        maxed[A_CON] = TRUE;
                    }

                    if (points_spent[A_CON] > points_trigger)
                    {
                        sell_stat(A_CON, stats, points_spent, points_left);
                        maxed[A_CON] = TRUE;
                    }
                }

                step++;
                break;
            }

            /*
             * If there are any points left, spend as much as possible in
             * order on DEX, non-spell-stat, CHR.
             */
            case 4:
            {
                int next_stat;

                if (!maxed[A_DEX])
                {
                    next_stat = A_DEX;
                }
                else if (!maxed[A_INT] && cp_ptr->spell_book != TV_MAGIC_BOOK)
                {
                    next_stat = A_INT;
                }
                else if (!maxed[A_WIS] && cp_ptr->spell_book != TV_PRAYER_BOOK)
                {
                    next_stat = A_WIS;
                }
                else if (!maxed[A_CHR])
                {
                    next_stat = A_CHR;
                }
                else
                {
                    step++;
                    break;
                }

                /* Buy until we can't buy any more. */
                while (buy_stat(next_stat, stats, points_spent, points_left));
                maxed[next_stat] = TRUE;

                break;
            }

            default:
            {
                step = -1;
                break;
            }
        }
    }
}

/*
 * Adjust a stat by an amount.
 *
 * This just uses "modify_stat_value()" unless "maximize" mode is false,
 * and a positive bonus is being applied, in which case, a special hack
 * is used.
 */
static int adjust_stat(int value, int amount)
{
    /* Negative amounts or maximize mode */
    if ((amount < 0) || adult_maximize)
    {
        return (modify_stat_value(value, amount));
    }

    /* Special hack */
    else
    {
        int i;

        /* Apply reward */
        for (i = 0; i < amount; i++)
        {
            if (value < 18)
            {
                value++;
            }
            else if (value < 18+70)
            {
                value += randint1(15) + 5;
            }
            else if (value < 18+90)
            {
                value += randint1(6) + 2;
            }
            else if (value < 18+100)
            {
                value++;
            }
        }
    }

    /* Return the result */
    return (value);
}

/*
 * Roll for a character's stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(int stat_use[A_MAX])
{
    int i, j;

    int bonus;

    int dice[18];

    /* Roll and verify some stats */
    while (TRUE)
    {
        /* Roll some dice */
        for (j = i = 0; i < 18; i++)
        {
            /* Roll the dice */
            dice[i] = randint1(3 + i % 3);

            /* Collect the maximum */
            j += dice[i];
        }

        /* Verify totals */
        if ((j > 42) && (j < 54)) break;
    }

    /* Roll the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Extract 5 + 1d3 + 1d4 + 1d5 */
        j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

        /* Save that value */
        p_ptr->stat_max[i] = j;

        /* Obtain a "bonus" for "race" and "class" */
        bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

        /* Variable stat maxes */
        if (adult_maximize)
        {
            /* Start fully healed */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i];

            /* Efficiency -- Apply the racial/class bonuses */
            stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
        }

        /* Fixed stat maxes */
        else
        {
            /* Apply the bonus to the stat (somewhat randomly) */
            stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus);

            /* Save the resulting stat maximum */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
        }

        p_ptr->stat_birth[i] = p_ptr->stat_max[i];
    }
}


static void set_moria_options(void)
{
    /* Paranoia */
    if (game_mode != GAME_NPPMORIA) return;

    /* Turn off options unused in Moria */
    auto_scum = FALSE;
    allow_themed_levels = FALSE;
    verify_leave_quest = FALSE;
    birth_maximize = adult_maximize = FALSE;
    birth_rand_artifacts = adult_rand_artifacts = FALSE;
    birth_force_small_lev = adult_force_small_lev = FALSE;
    birth_no_artifacts = adult_no_artifacts = FALSE;
    birth_simple_dungeons = adult_simple_dungeons = TRUE;
    birth_swap_weapons = adult_swap_weapons = TRUE;
    birth_no_xtra_artifacts = adult_no_xtra_artifacts = TRUE;
    birth_no_store_services = adult_no_store_services = TRUE;
    birth_no_player_ghosts = adult_no_player_ghosts = TRUE;
    birth_no_quests = adult_no_quests = TRUE;
    birth_connected_stairs = adult_connected_stairs = FALSE;
    birth_preserve = adult_preserve = TRUE;
}


static void roll_hp(void)
{
    int i, j, min_value, max_value;

    /* Minimum hitpoints at highest level */
    min_value = (z_info->max_level * (p_ptr->hitdie - 1) * 3) / 8;
    min_value += z_info->max_level;

    /* Maximum hitpoints at highest level */
    max_value = (z_info->max_level * (p_ptr->hitdie - 1) * 5) / 8;
    max_value += z_info->max_level;

    /* Roll out the hitpoints */
    while (TRUE)
    {
        /* Roll the hitpoint values */
        for (i = 1; i < z_info->max_level; i++)
        {
            j = randint1(p_ptr->hitdie);
            p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (p_ptr->player_hp[z_info->max_level-1] < min_value) continue;
        if (p_ptr->player_hp[z_info->max_level-1] > max_value) continue;

        /* Acceptable */
        break;
    }
}

/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(void)
{
    int i, chart, roll, social_class;

    /* Clear the previous history strings */
    p_ptr->history.clear();

    /* Initial social class */
    social_class = randint1(4);

    /* Starting place */
    chart = rp_ptr->hist;

    /* Process the history */
    while (chart)
    {
        /* Start over */
        i = 0;

        /* Roll for nobility */
        roll = randint1(100);

        /* Get the proper entry in the table */
        while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

        /* Get the textual history */
        p_ptr->history.append(h_info[i].h_text);

        /* Add a space */
        p_ptr->history.append(' ');

        /* Add in the social class */
        social_class += (int)(h_info[i].bonus) - 50;

        /* Enter the next chart */
        chart = h_info[i].next;
    }

    /* Verify social class */
    if (social_class > 75) social_class = 75;
    else if (social_class < 1) social_class = 1;

    /* Save the social class */
    p_ptr->sc = p_ptr->sc_birth = social_class;
}

/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
    /* Calculate the age */
    p_ptr->age = rp_ptr->b_age + randint1(rp_ptr->m_age);

    /* Calculate the height/weight for males */
    if (p_ptr->psex == SEX_MALE)
    {
        p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
        p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->m_b_wt, rp_ptr->m_m_wt);
    }

    /* Calculate the height/weight for females */
    else if (p_ptr->psex == SEX_FEMALE)
    {
        p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->f_b_ht, rp_ptr->f_m_ht);
        p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->f_b_wt, rp_ptr->f_m_wt);
    }
}

/*
 * Get the player's starting money
 */
static void get_money(int stat_use[A_MAX])
{
    if (birth_money)
    {
        p_ptr->au_birth = 200;
        p_ptr->au = 500;
    }
    else
    {
        p_ptr->au = p_ptr->au_birth = 200;
    }
}

/*
 * This fleshes out a full player based on the choices currently made,
 * and so is called whenever things like race or class are chosen.
 */
void generate_player()
{
    /* Set sex according to p_ptr->sex */
    sp_ptr = &sex_info[p_ptr->psex];

    /* Set class according to p_ptr->class */
    cp_ptr = &c_info[p_ptr->pclass];
    mp_ptr = &cp_ptr->spells;

    /* Set race according to p_ptr->race */
    rp_ptr = &p_info[p_ptr->prace];

    /* Level 1 */
    p_ptr->max_lev = p_ptr->lev = 1;

    /* Experience factor */
    p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

    /* Hitdice */
    p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

    /* Initial hitpoints */
    p_ptr->mhp = p_ptr->hitdie;

    /* Pre-calculate level 1 hitdice */
    p_ptr->player_hp[0] = p_ptr->hitdie;

    /* Roll for age/height/weight */
    get_ahw();

    get_history();
}

void roll_player(int stats[A_MAX])
{
    /* Get a new character */
    get_stats(stats);

    /* Roll for gold */
    get_money(stats);

    /* Update stats with bonuses, etc. */
    get_bonuses();

    /* There's no real need to do this here, but it's tradition. */
    get_ahw();
    get_history();
}

/**
 * Try to wield everything in the inventory.
 */
static void wield_all(void)
{
    object_type *o_ptr;
    object_type *i_ptr;
    object_type object_type_body;

    int slot;
    int item;
    int num;
    bool is_ammo;

    /* Scan through the slots backwards */
    for (item = INVEN_PACK - 1; item >= 0; item--)
    {
        o_ptr = &inventory[item];
        is_ammo = o_ptr->is_ammo();

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Make sure we can wield it */
        slot = wield_slot(o_ptr);
        if (slot < INVEN_WIELD) continue;
        i_ptr = &inventory[slot];

        /* Make sure that there's an available slot */
        if (is_ammo)
        {
            if (i_ptr->k_idx && !object_similar(o_ptr, i_ptr)) continue;
        }
        else
        {
            if (i_ptr->k_idx) continue;
        }

        /* Figure out how much of the item we'll be wielding */
        num = is_ammo ? o_ptr->number : 1;

        /* Get local object */
        i_ptr = &object_type_body;
        i_ptr->object_copy(o_ptr);

        /* Modify quantity */
        i_ptr->number = num;

        /* Decrease the item (from the pack) */
        inven_item_increase(item, -num);
        inven_item_optimize(item);

        /* Get the wield slot */
        o_ptr = &inventory[slot];

        /* Wear the new stuff */
        o_ptr->object_copy(i_ptr);
    }

    save_quiver_size();

    return;
}

/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(void)
{
    int i;
    const start_item *e_ptr;
    object_type *i_ptr;
    object_type object_type_body;

    /* Hack -- Give the player his equipment */
    for (i = 0; i < MAX_START_ITEMS; i++)
    {
        /* Access the item */
        e_ptr = &(cp_ptr->start_items[i]);

        /* Get local object */
        i_ptr = &object_type_body;

        /* Hack	-- Give the player an object */
        if (e_ptr->tval > 0)
        {
            /* Get the object_kind */
            int k_idx = lookup_kind(e_ptr->tval, e_ptr->sval);

            /* Valid item? */
            if (!k_idx) continue;

            /* Prepare the item */
            object_prep(i_ptr, k_idx);
            i_ptr->number = (byte)rand_range(e_ptr->min, e_ptr->max);

            object_aware(i_ptr);
            object_known(i_ptr);
            (void)inven_carry(i_ptr);
            k_info[k_idx].everseen = TRUE;

            apply_autoinscription(i_ptr);

            /* Remember history */
            object_history(i_ptr, ORIGIN_BIRTH, 0);
        }
    }

    /* Hack -- give the player hardcoded equipment XXX */

    /* Get local object */
    i_ptr = &object_type_body;

    /* Hack -- Give the player some food */
    object_prep(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
    i_ptr->number = (byte)rand_range(3, 7);
    object_aware(i_ptr);
    object_known(i_ptr);
    k_info[i_ptr->k_idx].everseen = TRUE;
    apply_autoinscription(i_ptr);
    /* Remember history */
    object_history(i_ptr, ORIGIN_BIRTH, 0);
    (void)inven_carry(i_ptr);

    /* Get local object */
    i_ptr = &object_type_body;

    /* Hack -- Give the player some torches */
    object_prep(i_ptr, lookup_kind(TV_LIGHT, SV_LIGHT_TORCH));
    apply_magic(i_ptr, 0, FALSE, FALSE, FALSE, FALSE);
    i_ptr->number = (byte)rand_range(3, 7);
    object_aware(i_ptr);
    object_known(i_ptr);
    k_info[i_ptr->k_idx].everseen = TRUE;
    apply_autoinscription(i_ptr);
    /* Remember history */
    object_history(i_ptr, ORIGIN_BIRTH, 0);
    (void)inven_carry(i_ptr);

    /* Now try wielding everything */
    wield_all();
}

/*
 * Clear all the global "character" data
 */
static void player_wipe(void)
{
    int i;

    /* Wipe the player */
    p_ptr->player_type_wipe();

    /* Clear the inventory */
    for (i = 0; i < ALL_INVEN_TOTAL; i++) {
        inventory[i].object_wipe();
    }

    /* Start with no artifacts made yet */
    for (i = 0; i < z_info->art_max; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        a_ptr->a_cur_num = 0;
    }

    /* Wipe the quest */
    guild_quest_wipe(TRUE);

    /* Reset the fixed quests */
    for (i = 0; i < z_info->q_max; i++)
    {
        quest_type *q_ptr = &q_info[i];

        /* Reset level */
        if (quest_fixed(q_ptr))
        {
            q_ptr->q_flags = 0L;
            q_ptr->q_num_killed = 0;
        }
    }

    /* Reset the "objects" */
    for (i = 1; i < z_info->k_max; i++)
    {
        object_kind *k_ptr = &k_info[i];

        /* Reset "tried" */
        k_ptr->tried = FALSE;

        /* Reset "aware" */
        k_ptr->aware = FALSE;
    }

    /* Reset the "monsters" */
    for (i = 1; i < z_info->r_max; i++)
    {
        monster_race *r_ptr = &r_info[i];
        monster_lore *l_ptr = &l_list[i];

        /* Hack -- Reset the counter */
        r_ptr->cur_num = 0;

        /* Hack -- Reset the max counter */
        r_ptr->max_num = 100;

        /* Hack -- Reset the max counter */
        if (r_ptr->flags1 & (RF1_UNIQUE)) r_ptr->max_num = 1;

        /* Clear player kills */
        l_ptr->pkills = 0;
    }

    /*No current player ghosts*/
    player_ghost_num = -1;
    ghost_r_idx = 0;

    /* Hack -- Well fed player */
    p_ptr->food = PY_FOOD_FULL - 1;

    /*re-set the altered inventory counter*/
    allow_altered_inventory = FALSE;
    altered_inventory_counter = 0;

    /* None of the spells have been learned yet */
    for (i = 0; i < PY_MAX_SPELLS; i++) p_ptr->spell_order[i] = 99;
}

void finish_birth()
{
    int i, n;

    // Hit points
    roll_hp();

    /* Set adult options from birth options */
    for (i = OPT_BIRTH; i < OPT_CHEAT; i++)
    {
        op_ptr->opt[OPT_ADULT + (i - OPT_BIRTH)] = op_ptr->opt[i];
    }

    /* Reset score options from cheat options */
    for (i = OPT_CHEAT; i < OPT_ADULT; i++)
    {
        op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = op_ptr->opt[i];
    }

    /*Re-set the squelch settings.  Spellbooks are never_pickup by default. */
    for (i = 0; i < z_info->k_max; i++)
    {
        /* Analyze the item type */
        switch (k_info[i].tval)
        {
            case TV_MAGIC_BOOK:
            case TV_PRAYER_BOOK:
            case TV_DRUID_BOOK:
            {
                k_info[i].squelch = NO_SQUELCH_NEVER_PICKUP;

                break;
            }
            default:
            {
                k_info[i].squelch = SQUELCH_NEVER;

                break;
            }
        }
    }

    /* Clear the squelch bytes */
    for (i = 0; i < SQUELCH_BYTES; i++)
    {
        squelch_level[i] = SQUELCH_NONE;
    }

    /* Clear the ego-item squelching flags */
    for (i = 0; i < z_info->e_max; i++)
    {
        e_info[i].squelch = false;
    }

    /* Write a note, if that option is on */
    if (adult_take_notes)
    {
        QDate today = QDate::currentDate();
        QTime right_now = QTime::currentTime();
        QString long_day;
        QString final_monster = "Morgoth, Lord of Darkness";

        if (game_mode == GAME_NPPMORIA) final_monster = "The Balrog of Moria";

        /* Get time */
        long_day = QString("%1 at %2") .arg(today.toString() .arg(right_now.toString()));

        write_note(QString("%1 the %2 %3 began the quest to kill %4 on %5.")
                   .arg(op_ptr->full_name) .arg(p_info[p_ptr->prace].pr_name) .arg(c_info[p_ptr->pclass].cl_name)
                   .arg(final_monster) .arg(long_day), p_ptr->depth);
    }

    /* Hack -- outfit the player */
    if (!birth_money) player_outfit();

    /* Shops */
    for (n = 0; n < MAX_STORES; n++)
    {
        /* Initialize */
        store_init(n);

        /* Ignore home */
        if ((n == STORE_HOME) || (n == STORE_GUILD)) continue;

        /* Maintain the shop (ten times) */
        for (i = 0; i < 10; i++) store_maint(n);
    }

    /* Start in town */
    p_ptr->depth = 0;

    /* Hack -- seed for flavors */
    seed_flavor = rand_int(0x10000000);

    /* Hack -- seed for town layout */
    seed_town = rand_int(0x10000000);

    /* Hack -- seed for random artifacts */
    seed_randart = rand_int(0x10000000);

    seed_ghost = rand_int(0x10000000);

    /* Randomize the artifacts */
    if (adult_rand_artifacts)
    {
        do_randart(seed_randart, TRUE);
    }

    /* Hack -- enter the world */
    turn = 1;

    p_ptr->p_turn = 0;

    quest_indicator_timer = 0;
    quest_indicator_complete = FALSE;
}

void init_birth()
{
    player_wipe();

    /* Turn off many options for Moria */
    if (game_mode == GAME_NPPMORIA) set_moria_options();

    character_loaded = character_dungeon = character_generated = false;
}

/*
 * A class to hold "rolled" information, and any
 * other useful state for the birth process.
 */
class Birther
{
public:
    Birther();

    byte mode;

    QString full_name;

    byte p_sex;
    byte p_race;
    byte p_class;

    s16b age;
    s16b wt;
    s16b ht;
    s16b sc;

    s32b au;

    s16b stat[A_MAX];

    QString history;

    void save();
    void load();
};

Birther::Birther()
{
    p_sex = p_race = p_class = age = wt = ht = sc = au = mode = 0;
    C_WIPE(stat, A_MAX, s16b);
    history.clear();
}

void Birther::save()
{
    int i;

    mode = game_mode;

    /* Save the data */
    p_sex = p_ptr->psex;
    p_race = p_ptr->prace;
    p_class = p_ptr->pclass;
    age = p_ptr->age;
    wt = p_ptr->wt_birth;
    ht = p_ptr->ht_birth;
    sc = p_ptr->sc_birth;
    au = p_ptr->au_birth;

    /* Save the stats */
    for (i = 0; i < A_MAX; i++)
    {
        stat[i] = p_ptr->stat_birth[i];
    }

    history = p_ptr->history;

    full_name = op_ptr->full_name;
}

void Birther::load()
{
    if (!has_prev_character()) return;

    /* Load the data */
    p_ptr->psex = p_sex;
    p_ptr->prace = p_race;
    p_ptr->pclass = p_class;
    p_ptr->age = age;
    p_ptr->wt = p_ptr->wt_birth = wt;
    p_ptr->ht = p_ptr->ht_birth = ht;
    p_ptr->sc = p_ptr->sc_birth = sc;
    p_ptr->au_birth = au;
    if (birth_money)
        p_ptr->au = 500;
    else
        p_ptr->au = p_ptr->au_birth;

    /* Load the stats */
    for (int i = 0; i < A_MAX; i++)
    {
        p_ptr->stat_max[i] = p_ptr->stat_cur[i] = p_ptr->stat_birth[i] = stat[i];
    }

    p_ptr->history = history;
    op_ptr->full_name = full_name;
}

static Birther _birther;

bool has_prev_character()
{
    return _birther.mode == game_mode;
}

void save_prev_character()
{
    _birther.save();
}

void load_prev_character()
{
    _birther.load();
}
