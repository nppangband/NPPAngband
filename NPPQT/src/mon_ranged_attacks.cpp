
/* File: was melee1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * 						Leon Marrick, Bahman Rabbi, Diego Gonzalez, Jeff Greene
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


#include "npp.h"

/*********************************************************************/
/*                                                                   */
/*                      Monster Ranged Attacks                       */
/*                                                                   */
/*********************************************************************/


/*
 * Using an input value for average damage, and another that controls
 * variability, return the actual base damage of a monster's attack
 * spell.  The larger the value for "control", the less likely the damage
 * will vary greatly.
 */
int get_dam(monster_race *r_ptr, int attack)
{
    int dam = 0;
    int spread;

    int control, av_dam;

    byte power = r_ptr->spell_power;

    /* Determine mana cost */
    if (attack >= 224) return (FALSE);
    else if (attack >= 192)
    {
        av_dam = power * spell_info_RF7[attack-192][COL_SPELL_DAM_MULT];
        av_dam /= MAX(1, spell_info_RF7[attack-192][COL_SPELL_DAM_DIV]);
        control = spell_info_RF7[attack-192][COL_SPELL_DAM_VAR];
    }
    else if (attack >= 160)
    {
        av_dam = power * spell_info_RF6[attack-160][COL_SPELL_DAM_MULT];
        av_dam /= MAX(1, spell_info_RF6[attack-160][COL_SPELL_DAM_DIV]);
        control = spell_info_RF6[attack-160][COL_SPELL_DAM_VAR];
    }
    else if (attack >= 128)
    {
        av_dam = power * spell_info_RF5[attack-128][COL_SPELL_DAM_MULT];
        av_dam /= MAX(1, spell_info_RF5[attack-128][COL_SPELL_DAM_DIV]);
        control = spell_info_RF5[attack-128][COL_SPELL_DAM_VAR];
    }
    else if (attack >=  96)
    {
        av_dam = power * spell_info_RF4[attack- 96][COL_SPELL_DAM_MULT];
        av_dam /= MAX(1, spell_info_RF4[attack- 96][COL_SPELL_DAM_DIV]);
        control = spell_info_RF4[attack- 96][COL_SPELL_DAM_VAR];
    }
    else return (FALSE);

    /*Hack - handle Wound spell differently */
    if (attack == 160+20)
    {
        if (power > 75) av_dam = 225 + power - 75;
    }


    /*No point in going through this to return 0*/
    if (av_dam < 1) return (FALSE);

    /* Damage may never differ by more than 50% from the average */
    if (control < 4) control = 4;

    /*
     * Get the allowable spread (two standard deviations, or 100,
     * whichever is less).
     */
    spread = MIN(100, av_dam * 2 / control);

    /* Loop until damage is within the allowable spread */
    while (TRUE)
    {
        /* Randomize damage (average, standard deviation) */
        dam = Rand_normal(av_dam, div_round(av_dam, control));

        /* Forbid too great a variation */
        if (dam > av_dam + spread) continue;
        if (dam < av_dam - spread) continue;

        /* Accept */
        break;
    }

    /* Return randomized damage */
    return (dam);
}

/*
 * Return the max damage by GF type.  It is important to
 * also have a corresponding damage in get_breath_dam and
 * get_ball_dam below, or a damage of zero will be returned.
 */
static int get_max_dam(int gf_type, bool powerful)
{
    switch (gf_type)
    {
        case GF_ACID:
        case GF_ELEC:
        case GF_FIRE:
        case GF_COLD:
        case GF_ICE:
        {
            return (1600);
        }

        case GF_POIS: return (800);

        case GF_PLASMA:
        case GF_INERTIA_NPP: case GF_FORCE:
        case GF_TIME:
        {
            if (powerful) return (400);
            else return (150);
        }

        case GF_LIGHT:
        case GF_DARK:
        case GF_CONFUSION:
        case GF_HOLY_ORB:
        {
            return (400);
        }

        case GF_SOUND:
        {
            if (powerful) return (500);
            else return (150);
        }

        case GF_SHARD:
        case GF_LAVA:
        case GF_CHAOS:
        case GF_DISENCHANT:
        case GF_METEOR:
        case GF_WATER:
        {
            return (500);
        }

        case GF_GRAVITY:
        {
            if (powerful) return (300);
            else return (150);
        }

        case GF_NEXUS: 	return (450);

        case GF_NETHER:	return (550);

        case GF_MANA:
        {
            if (powerful) return (400);
            else return (250);
        }
    }

    /* Return breath damage */
    return (0);
}


/*
 * Get the breath damage by GF type  It is important to
 * also have a corresponding max damage by GF type in get_max_dam above,
 * or a damage of zero will be returned.
 */
int get_breath_dam(s16b hit_points, int gf_type, bool powerful)
{
    int dam;
    int max_dam = get_max_dam(gf_type, powerful);

    switch (gf_type)
    {
        case GF_ACID:
        case GF_ELEC:
        case GF_FIRE:
        case GF_COLD:
        case GF_POIS:
        case GF_LAVA:
        case GF_TIME:
        case GF_MANA:
        case GF_HOLY_ORB:
        {
            dam = hit_points / 3;
            break;
        }

        case GF_PLASMA:
        case GF_FORCE:
        {
            if (powerful) dam = hit_points / 3;
            else dam = hit_points / 6;
            break;
        }
        case GF_LIGHT:
        case GF_DARK:
        case GF_CONFUSION:
        case GF_SHARD:
        case GF_NEXUS:
        case GF_NETHER:
        case GF_CHAOS:
        case GF_DISENCHANT:
        case GF_METEOR:
        {
            dam = hit_points / 6;
            break;
        }

        case GF_WATER:
        {
            dam = hit_points / 4;
            break;
        }

        case GF_SOUND:
        {
            if (powerful) dam = hit_points / 4;
            else dam = hit_points / 9;
            break;
        }

        case GF_INERTIA_NPP: case GF_GRAVITY:
        {
            if (powerful) 	dam = hit_points / 4;
            else 			dam = hit_points / 8;
            break;
        }

        /*Whoops!*/
        default: return (FALSE);
    }

    /* Return breath damage, don't exceed max damage */
    if (max_dam <= dam) return (max_dam);
    return (dam);
}

/*
 * Get the ball damage by GF type  It is important to
 * also have a corresponding max damage by GF type in get_max_dam above,
 * or a damage of zero will be returned.
 *
 * An m_idx of (-1) means we want damage based solely on the monster race, and
 * to not factor in the HP of the specific monster.
 *
 */

int get_ball_beam_dam(int m_idx, monster_race *r_ptr, int attack, int gf_type, bool powerful)
{
    int dam = get_dam(r_ptr, attack);
    int max_dam = get_max_dam(gf_type, powerful);

    /* Factor in the hp of the specific monster if called for */
    if (m_idx > 0)
    {
        monster_type *m_ptr = &mon_list[m_idx];

        /* Paranoia - make sure it is a real monster */
        if (m_ptr->r_idx)
        {
            int breath_dam = get_breath_dam(m_ptr->hp, gf_type, powerful);

            /*
             * Average the damage of a fully healthy creature, and the current monster, so
             * ball (breath) spells lose some power, but not as much as plain breath spells.
             */
            dam = (breath_dam + dam) / 2;
        }
    }

    /* Return breath damage, don't exceed max damage */
    if (max_dam <= dam) return (max_dam);
    return (dam);
}
