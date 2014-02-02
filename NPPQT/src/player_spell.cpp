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
 * Helper function for casting the "Prismatic Spray" spell.
 */
static void cast_prismatic_spray(int dir, int dam)
{
    int gftype;

    switch (randint(p_ptr->lev + 25) / 5)
    {
        /*
         * Should only be possible for level 50 and even then odds
         * should be 1/75 of this hapening. An easter egg type of
         * effect. :) -AR
         */
        case (15):
        {
            gftype = GF_CHAOS;
            dam *= 12;
            message("You conjure forth a massive torrent of raw chaos!");
            break;
        }
        case (14):
        {
            gftype = GF_INERTIA;
            dam *= 2;
            message("You conjure forth a torrent of inertia.");
            break;
        }
        case (13):
        {
            gftype = GF_GRAVITY;
            dam *= 2;
            message("You conjure forth a torrent of gravity.");
            break;
        }
        case (12):
        {
            gftype = GF_CONFUSION;
            dam *= 3;
            message("You conjure forth a torrent of confusion.");
            break;
        }
        case (11):
        {
            gftype = GF_FORCE;
            dam *= 3;
            message("You conjure forth a torrent of pure force.");
            break;
        }
        case (10):
        {
            gftype = GF_TIME;
            dam *= 3;
            message("You conjure forth a torrent of time.");
            break;
        }
        case (9):
        {
            gftype = GF_STATIC;
            dam *= 4;
            message("You conjure forth a torrent of anti-magic static.");
            break;
        }
        case (8):
        {
            gftype = GF_NEXUS;
            dam *= 4;
            message("You conjure forth a torrent of dimensional energy.");
            break;
        }
        case (7):
        {
            gftype = GF_DISENCHANT;
            dam *= 4;
            message("You conjure forth a torrent of disenchantment.");
            break;
        }
        case (6):
        {
            gftype = GF_SHARD;
            dam *= 4;
            message("You conjure forth a torrent of shrapnel.");
            break;
        }
        case (5):
        {
            gftype = GF_NETHER;
            dam *= 5;
            message("You conjure forth a torrent of nether.");
            break;
        }
        case (4):
        {
            gftype = GF_COLD;
            dam *= 5;
            message("You conjure forth a torrent of frost.");
            break;
        }
        case (3):
        {
            gftype = GF_FIRE;
            dam *= 5;
            message("You conjure forth a torrent of flames.");
            break;
        }
        case (2):
        {
            gftype = GF_ELEC;
            dam *= 5;
            message("You conjure forth a torrent of electricity.");
            break;
        }
        case (1):
        {
            gftype = GF_ACID;
            dam *= 5;
            message("You conjure forth a torrent of acid.");
            break;
        }
        default:
        {
            gftype = GF_POIS;
            dam *= 5;
            message("You conjure forth a torrent of poison.");
            break;
        }
    }

    // TOTO fire_arc(gftype, dir, dam, 0, 60);
}



static int beam_chance(void)
{
    int plev = p_ptr->lev;
    return ((cp_ptr->flags & CF_BEAM) ? plev : (plev / 2));
}




/* Report if a spell needs a target*/
bool spell_needs_aim(int tval, int spell)
{
    if (tval == TV_MAGIC_BOOK)
    {
        switch (spell)
        {
            case SPELL_MAGIC_MISSILE:
            case SPELL_STINKING_CLOUD:
            case SPELL_CONFUSE_MONSTER:
            case SPELL_SHOCK_WAVE:
            case SPELL_LIGHTNING_BOLT:
            case SPELL_FROST_BOLT:
            case SPELL_FROST_BALL:
            case SPELL_FIRE_BOLT:
            case SPELL_FIRE_BALL:
            case SPELL_TRAP_DOOR_DESTRUCTION:
            case SPELL_SLEEP_MONSTER:
            case SPELL_SPEAR_OF_LIGHT:
            case SPELL_ICE_BOLT:
            case SPELL_TURN_STONE_TO_MUD:
            case SPELL_PRISMATIC_SPRAY:
            case SPELL_POLYMORPH_OTHER:
            case SPELL_SHARD_STORM:
            case SPELL_SLOW_MONSTER:
            case SPELL_CALL_LIGHTNING:
            case SPELL_TELEPORT_OTHER:
            case SPELL_BEDLAM:
            case SPELL_WATER_BOLT:
            case SPELL_HURRICANE:
            case SPELL_CLOUD_KILL:
            case SPELL_ICE_STORM:
            case SPELL_PLASMA_BOLT:
            case SPELL_MANA_STORM:
            case SPELL_NOVA:
            case SPELL_REND_SOUL:
            case SPELL_RIFT:
            case SPELL_DARKNESS_STORM:
            case SPELL_METEOR_STORM:
            case SPELL_MANA_BOLT:
            case SPELL_WAIL_OF_THE_BANSHEE:
            {
                return TRUE;
            }

            default: return FALSE;
        }
    }

    else if (tval == TV_DRUID_BOOK)
    {
        switch (spell)
        {
            case DRUID_ACID_BOLT:
            case DRUID_POISON_CLOUD:
            case DRUID_TURN_STONE_TO_MUD:
            case DRUID_FROST_BEAM:
            case DRUID_TRAP_DOOR_DESTRUCTION:
            case DRUID_SPEAR_OF_LIGHT:
            case DRUID_FIRE_BEAM:
            case DRUID_STERILIZE:
            case DRUID_LIFE_DRAIN_BURST:
            case DRUID_FROST_BALL:
            case DRUID_FIRE_BALL:
            case DRUID_DRAIN_LIFE_ARC:
            case DRUID_SANDSTORM:
            case DRUID_CHANNEL_LIGHTNING:
            case DRUID_MASTER_ELEMENTS:
            case DRUID_STEAL_POWERS:
            {
                return TRUE;
            }
            default: return FALSE;
        }


    }
    else if (tval == TV_PRAYER_BOOK)
    {
        switch (spell)
        {
            case PRAYER_SHOCK_BOLT:
            case PRAYER_SCARE_MONSTER:
            case PRAYER_CONFUSE_MONSTER:
            case PRAYER_SUN_BEAM:
            case PRAYER_ORB_OF_DRAINING:
            case PRAYER_SUN_BURST:
            case PRAYER_UNBARRING_WAYS:
            case PRAYER_TELEPORT_OTHER:
            {
                return TRUE;
            }

            default: return FALSE;
        }
    }

    /*OOPS*/
    else return (FALSE);
}


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
