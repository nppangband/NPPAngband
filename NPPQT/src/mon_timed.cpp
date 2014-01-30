/* File: monster1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "src/npp.h"




typedef struct
{
  int message_begin;
  int message_end;
  int message_increase;
  u32b flag_resist;
} mon_timed_effect;

/*
 * Monster timed effects.  Notice this code assumes the monster resist
 * is in the third set of flags.
 * '0' means no message.
 */

static mon_timed_effect effects[] =
{
    /*TMD_MON_SLEEP*/
    {MON_MSG_FALL_ASLEEP, MON_MSG_WAKES_UP, FALSE, RF3_NO_SLEEP},
    /*TMD_MON_STUN*/
    {MON_MSG_DAZED, MON_MSG_NOT_DAZED, MON_MSG_MORE_DAZED, RF3_NO_STUN },
    /*TMD_MON_CONF*/
    {MON_MSG_CONFUSED, MON_MSG_NOT_CONFUSED, MON_MSG_MORE_CONFUSED, RF3_NO_CONF },
    /*TMD_MON_FEAR*/
    {MON_MSG_FLEE_IN_TERROR, MON_MSG_NOT_AFRAID, MON_MSG_MORE_AFRAID, RF3_NO_FEAR },
    /*TMD_MON_SLOW*/
    {MON_MSG_SLOWED, MON_SNG_NOT_SLOWED, MON_MSG_MORE_SLOWED, RF3_NO_SLOW  },
    /*TMD_MON_FAST*/
    {MON_MSG_HASTED, MON_MSG_NOT_HASTED, MON_MSG_MORE_HASTED, 0L  },

};

static int charisma_adjustment(const monster_race *r_ptr)
{

    /*stupid or brainless monsters aren't affected by player charisma*/
    if (r_ptr->flags2 & (RF2_STUPID | RF2_EMPTY_MIND)) return (0);

    /*weird monsters are rarely affected by player charisma*/
    if ((r_ptr->flags2 & (RF2_WEIRD_MIND)) && (!one_in_(10))) return (0);

    /*charisma applies*/
    return (adj_chr_charm[p_ptr->state.stat_ind[A_CHR]]);
}

#define ZERO_RESIST		0
#define HALF_RESIST		1
#define FULL_RESIST		2


/*
 * Helper function for mon_set_timed.  This determined if the monster
 * Successfully resisted the effect.  Also marks the lore for any
 * appropriate resists.
 */
static int mon_resist_effect(int m_idx, int idx, u16b flag)
{
    mon_timed_effect *effect = &effects[idx];
    int resisted = ZERO_RESIST;
    int resist_chance;
    monster_type *m_ptr = &mon_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* Hasting never fails */
    if (idx == MON_TMD_FAST) return (ZERO_RESIST);

    /* Some effects are marked to never fail */
    if (flag & (MON_TMD_FLG_NOFAIL)) return (ZERO_RESIST);

    /* Stupid, weird, or empty monsters aren't affected by some effects*/
    if (r_ptr->flags2 & (RF2_STUPID | RF2_EMPTY_MIND | RF2_WEIRD_MIND))
    {
        if (idx == MON_TMD_CONF) return (FULL_RESIST);
        if (idx == MON_TMD_SLEEP) return (FULL_RESIST);
    }

    /* Calculate the chance of the monster resisting. */
    if (flag & (MON_TMD_MON_SOURCE))
    {
        resist_chance = r_ptr->level;
    }
    else
    {
        resist_chance = r_ptr->level + 25 - p_ptr->lev / 5;
        resist_chance -= charisma_adjustment(r_ptr);
    }

    /* Monsters who resist get half the duration, at most */
    if (r_ptr->flags3 & (effect->flag_resist))
    {
        resisted = HALF_RESIST;

        /* Mark the lore */
        if (flag & MON_TMD_FLG_SEEN) l_ptr->r_l_flags3 |= effect->flag_resist;

        /* 2 changes to resist */
        if (randint0(100) < resist_chance) return (FULL_RESIST);
        if (randint0(100) < resist_chance) return (FULL_RESIST);

    }

    /* Uniques are doubly hard to affect */
    if (r_ptr->flags1 & RF1_UNIQUE)
    {
        resisted = HALF_RESIST;
        if (randint0(100) < resist_chance) return (FULL_RESIST);
    }

    /* Monsters with specific breaths and undead get an extra chance at resisting at stunning*/
    if ((idx == MON_TMD_STUN) &&
        ((r_ptr->flags4 & (RF4_BRTH_SOUND | RF4_BRTH_FORCE)) || (monster_nonliving(r_ptr))))
    {
        resisted = HALF_RESIST;

        if ((randint0(100) < resist_chance))
        {
            /* Add the lore */
            if (flag & MON_TMD_FLG_SEEN)
            {
                if (r_ptr->flags4 & (RF4_BRTH_SOUND))
                {
                    l_ptr->r_l_flags4 |= RF4_BRTH_SOUND;
                }
                if (r_ptr->flags4 & (RF4_BRTH_FORCE))
                {
                    l_ptr->r_l_flags4 |= RF4_BRTH_FORCE;
                }
            }

            return (FULL_RESIST);
        }
    }

    /* Monsters with specific breaths get an extra chance at resisting confusion*/
    if ((idx == MON_TMD_CONF) &&
        (r_ptr->flags4 & (RF4_BRTH_CONFU | RF4_BRTH_CHAOS)))
    {

        resisted = HALF_RESIST;

        if ((randint0(100) < resist_chance))
        {
            /* Add the lore */
            if (flag & MON_TMD_FLG_SEEN)
            {
                if (r_ptr->flags4 & (RF4_BRTH_CONFU))
                {
                    l_ptr->r_l_flags4 |= RF4_BRTH_CONFU;
                }
                if (r_ptr->flags4 & (RF4_BRTH_CHAOS))
                {
                    l_ptr->r_l_flags4 |= RF4_BRTH_CHAOS;
                }
            }
            return (FULL_RESIST);
        }
    }

    /* Very difficult to make non-living creatures sleep */
    if ((idx == MON_TMD_SLEEP) &&  (monster_nonliving(r_ptr)))
    {
        resisted = HALF_RESIST;

        if ((randint0(100) < resist_chance)) return (FULL_RESIST);
    }

    /* Inertia breathers are highly resistant to slowing*/
    if ((idx == MON_TMD_SLOW) && (r_ptr->flags4 & (RF4_BRTH_INER)))
    {
        resisted = HALF_RESIST;

        if ((randint0(100) < resist_chance))
        {
            /* Add the lore */
            if (flag & MON_TMD_FLG_SEEN)
            {
                l_ptr->r_l_flags4 |= RF4_BRTH_INER;
            }

            return (FULL_RESIST);

        }
    }

    return (resisted);
}

/*
 * Set a timed monster event to 'v'.  Give messages if the right flags are set.
 * Check if the monster is able to resist the spell.  Mark the lore
 * Note much this code assumes the monster resistances are in the
 * r_ptr>flags3 set.
 * Returns TRUE if the monster was affected
 * Return FALSE if the monster was unaffected.
 */
static bool mon_set_timed(int m_idx, int idx, int v, u16b flag)
{
    mon_timed_effect *effect = &effects[idx];
    monster_type *m_ptr = &mon_list[m_idx];

    QString m_name;
    int m_note = FALSE;

    int resisted;

    m_note = 0;

    /* Get monster name*/
    m_name = monster_desc(m_ptr, 0);

    /* No change */
    if (m_ptr->m_timed[idx] == v) return FALSE;

    /* Turning off, usually mention */
    if (v == 0)
    {
        m_note = effect->message_end;

        flag |= MON_TMD_FLG_NOTIFY;
    }

    /* Turning on, usually mention */
    else if (m_ptr->m_timed[idx] == 0)
    {

        flag |= MON_TMD_FLG_NOTIFY;

        m_note = effect->message_begin;
    }
    /* Different message for increases, but don't automatically mention. */
    else if (v > m_ptr->m_timed[idx])
    {
        m_note = effect->message_increase;
    }

    /* Determine if the monster resisted or not */
    resisted = mon_resist_effect(m_idx, idx, flag);

    if (resisted == FULL_RESIST)
    {
        m_note = MON_MSG_UNAFFECTED;
    }

    /* Cut the increase duration in half */
    else if (resisted == HALF_RESIST)
    {
        int change = v - m_ptr->m_timed[idx];

        m_note = MON_MSG_RESIST_SOMEWHAT;

        /* Paranoia - make sure it is an increase that can be cut in half */
        if (change > 1)
        {
            change /= 2;
            v = m_ptr->m_timed[idx] + change;
        }
    }

    /* set the JUST_SCARED flag */
    else if (idx == MON_TMD_FEAR)
    {
        if (v > m_ptr->m_timed[idx]) m_ptr->mflag |= (MFLAG_JUST_SCARED);
    }

    /* Apply the value, unless they fully resisted */
    if (resisted != FULL_RESIST)
    {
        m_ptr->m_timed[idx] = v;
    }

    /*possibly update the monster health bar*/
    if ((p_ptr->health_who == m_idx)  || (m_ptr->sidebar))p_ptr->redraw |= (PR_HEALTH);

    if ((idx == MON_TMD_FAST) || (idx == MON_TMD_SLOW))
    {
         calc_monster_speed(m_ptr->fy, m_ptr->fx);
    }
    /* Just waking up, clear all other effects */
    else if ((idx == MON_TMD_SLEEP) && (v == 0))
    {
        m_ptr->m_timed[MON_TMD_CONF] = 0;
        m_ptr->m_timed[MON_TMD_STUN] = 0;
        m_ptr->m_timed[MON_TMD_FEAR] = 0;
        m_ptr->m_timed[MON_TMD_SLOW] = 0;
        m_ptr->m_timed[MON_TMD_FAST] = 0;
        calc_monster_speed(m_ptr->fy, m_ptr->fx);
    }

    /* Update the visuals, as appropriate. */
    p_ptr->redraw |= (PR_MONLIST);

    /* Return result without any messages */
    if ((flag & (MON_TMD_FLG_NOMESSAGE)) || (!m_note) ||
        (!(flag & (MON_TMD_FLG_SEEN))) ||
        (!(flag & (MON_TMD_FLG_NOTIFY))))
    {
        /* Return a boolean result */
        if (resisted == FULL_RESIST) return FALSE;
        return (TRUE);
    }

    /* Finally, handle the message */
    add_monster_message(m_name, m_idx, m_note);

    /* Return a boolean result */
    if (resisted == FULL_RESIST) return FALSE;
    return (TRUE);
}

/*
 * Increase the timed effect `idx` by `v`.
 */
bool mon_inc_timed(int m_idx, int idx, int v, u16b flag)
{
    monster_type *m_ptr = &mon_list[m_idx];

    /* Ignore dead monsters */
    if (!m_ptr->r_idx) return FALSE;

    if (v < 0) return (FALSE);

    /* Check we have a valid effect */
    if ((idx < 0) || (idx > MON_TMD_MAX)) return FALSE;

    /* mark if seen */
    if (m_ptr->ml) flag |= MON_TMD_FLG_SEEN;

    /* Hasting never fails */
    if (idx == MON_TMD_FAST) flag |= MON_TMD_FLG_NOFAIL;

    /* Can't prolong sleep of sleeping monsters */
    if ((idx == MON_TMD_SLEEP) &&
        (m_ptr->m_timed[MON_TMD_SLEEP])) return FALSE;

    /* Make it last for a mimimum # of turns if it is a new effect */
    if ((!m_ptr->m_timed[idx]) && (v < 2)) v = 2;

    /* New counter amount */
    v = m_ptr->m_timed[idx] + v;

    /* Boundry Control */
    if (v > 10000) v = 10000;

    return mon_set_timed(m_idx, idx, v, flag);
}

/*
 * Decrease the timed effect `idx` by `v`.
 */
bool mon_dec_timed(int m_idx, int idx, int v, u16b flag)
{
    monster_type *m_ptr = &mon_list[m_idx];

    /* Ignore dead monsters */
    if (!m_ptr->r_idx) return FALSE;

    if (v < 0) return (FALSE);

    /* Check we have a valid effect */
    if ((idx < 0) || (idx > MON_TMD_MAX)) return FALSE;

    /* mark if seen */
    if (m_ptr->ml) flag |= MON_TMD_FLG_SEEN;

    /* Decreasing is never resisted */
    flag |= MON_TMD_FLG_NOFAIL;

    /* New counter amount */
    v = m_ptr->m_timed[idx] - v;

    /* Use clear function if appropriate */
    if (v < 0) return (mon_clear_timed(m_idx, idx, flag));

    return mon_set_timed(m_idx, idx, v, flag);
}

/**
 * Clear the timed effect `idx`.
 */
bool mon_clear_timed(int m_idx, int idx, u16b flag)
{
    monster_type *m_ptr = &mon_list[m_idx];

    /* Ignore dead monsters */
    if (!m_ptr->r_idx) return FALSE;

    if (!m_ptr->m_timed[idx]) return FALSE;

    /* mark if seen */
    if (m_ptr->ml) flag |= MON_TMD_FLG_SEEN;

    /* Monster is no longer desperate */
    if (idx == MON_TMD_FEAR)
    {
        m_ptr->mflag &= ~(MFLAG_DESPERATE);
    }

    /* Clearing never fails */
    flag |= MON_TMD_FLG_NOFAIL;

    /* Check we have a valid effect */
    if ((idx < 0) || (idx > MON_TMD_MAX)) return (FALSE);

    return mon_set_timed(m_idx, idx, 0, flag);
}

/* Helper function to wake monsters who are asleep */
void wake_monster_attack(monster_type *m_ptr, u16b flag)
{
    int m_idx = dungeon_info[m_ptr->fx][m_ptr->fx].monster_idx;

    /* Already Awake */
    if (!m_ptr->m_timed[MON_TMD_SLEEP]) return;

    /* Disturb the monster */
    mon_clear_timed(m_idx, MON_TMD_SLEEP, flag);

    /* Make the monster a little slow to wake up */
    if (m_ptr->m_energy > BASE_ENERGY_MOVE /2) m_ptr->m_energy = BASE_ENERGY_MOVE /2;
}
