#include "npp.h"

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
void get_stats(int stat_use[A_MAX])
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
