
/* File: was spells2.c */

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
 * Draw some projections in multi-hued colors.
 * -TY-, -EB-
 */
static byte mh_attr(void)
{
    switch (randint(8))
    {
        case 1:  return (TERM_RED);
        case 2:  return (TERM_GREEN);
        case 3:  return (TERM_BLUE);
        case 4:  return (TERM_YELLOW);
        case 5:  return (TERM_ORANGE);
        case 6:  return (TERM_L_RED);
        case 7:  return (TERM_L_GREEN);
        case 8:  return (TERM_L_BLUE);
    }

    return (TERM_WHITE);
}

static byte acid_color(void)
{
    switch (rand_int(3))
    {
        case 0: case 1: return (TERM_SLATE);
        case 2: return (TERM_L_DARK);
    }
    return (TERM_WHITE);
}

static byte elec_color(void)
{
    byte base;

    switch (rand_int(3))
    {
        case 0: case 1: base = (TERM_BLUE); break;
        default: base = (TERM_L_BLUE); break;
    }

    return (base);
}

static byte fire_color(void)
{
    byte base;

    switch (rand_int(3))
    {
        case 0: case 1: base = (TERM_RED); break;
        default: base = (TERM_L_RED); break;
    }

    return (base);
}

static byte cold_color(void)
{
    switch (rand_int(3))
    {
        case 0: case 1: return (TERM_WHITE);
        case 2: return (TERM_SNOW_WHITE);
    }
    return (TERM_WHITE);
}

static byte pois_color(void)
{
    switch (rand_int(3))
    {
        case 0: case 1: return (TERM_GREEN);
        case 2: return (TERM_L_GREEN);
    }
    return (TERM_WHITE);
}

static byte plasma_color(void)
{
    switch (rand_int(4))
    {
        case 0: case 1: return (TERM_WHITE);
        case 2: return (TERM_L_RED);
        case 3: return (TERM_YELLOW);
    }

    return (TERM_WHITE);
}

static byte disen_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: return (TERM_VIOLET);
        case 2: case 3: return (TERM_PURPLE);
        case 4: return (TERM_RASPBERRY);
    }

    return (TERM_WHITE);
}


static byte nexus_color(void)
{
    switch (rand_int(4))
    {
        case 0: case 1: return (TERM_VIOLET);
        case 2: return (TERM_RED);
        case 3: return (TERM_PURPLE);
    }

    return (TERM_WHITE);
}

static byte ice_color(void)
{
    switch (rand_int(3))
    {
        case 0: case 1: return (TERM_WHITE);
        case 2: return (TERM_L_BLUE);
    }

    return (TERM_WHITE);
}

static byte light_color(void)
{
    switch (rand_int(4))
    {
        case 0: case 1: case 2: return (TERM_YELLOW);
        case 3: return (TERM_ORANGE);
    }

    return (TERM_WHITE);
}

static byte confu_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: case 2: return (TERM_L_UMBER);
        case 3: return (TERM_UMBER);
        case 4: return (TERM_WHITE);
    }

    return (TERM_WHITE);
}

static byte spore_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: case 2: return (TERM_L_UMBER);
        case 3: return (TERM_UMBER);
        case 4: return (TERM_ORANGE);
    }

    return (TERM_WHITE);
}

static byte sound_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: case 2: return (TERM_YELLOW);
        case 3: return (TERM_GOLD);
        case 4: return (TERM_MAIZE);
    }

    return (TERM_WHITE);
}

static byte grav_color(void)
{
    switch (rand_int(4))
    {
        case 0: case 1: return (TERM_DARK);
        case 2: return (TERM_L_DARK);
        case 3: return (TERM_SLATE);
    }

    return (TERM_WHITE);
}

static byte iner_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: case 2: return (TERM_PINK);
        case 3: return (TERM_RASPBERRY);
        case 4: return (TERM_RED_RUST);
    }

    return (TERM_WHITE);
}

static byte meteor_color(void)
{
    switch (rand_int(6))
    {
        case 0: case 1: return (TERM_L_DARK);
        case 2: return (TERM_WHITE);
        case 3: return (TERM_RED);
        case 4: return (TERM_ORANGE);
        case 5: return (TERM_YELLOW);
    }

    return (TERM_WHITE);
}

static byte water_color(void)
{
    switch (rand_int(5))
    {
        case 0: case 1: case 2: return (TERM_SLATE);
        case 3: return (TERM_L_BLUE);
        case 4: return (TERM_SNOW_WHITE);
    }

    return (TERM_L_DARK);
}

static byte orb_color(void)
{
    switch (rand_int(4))
    {
        case 0: case 1: case 2: return (TERM_L_DARK);
        case 3: return (TERM_SLATE);
    }

    return (TERM_L_DARK);
}

static byte mana_color(void)
{
    byte base;

    switch (rand_int(2))
    {
        case 1: base = (TERM_BLUE); break;
        default: base = (TERM_RED); break;
    }

    return (base);
}

static byte fog_color(void)
{
    switch (rand_int(5))
    {
        case 0: return (TERM_VIOLET);
        case 1: return (TERM_PURPLE);
    }

    return (TERM_SLATE);
}

static byte bwater_color(void)
{
    if (one_in_(3)) return (TERM_L_BLUE);

    return (TERM_WHITE);
}


static byte lava_color(void)
{
    if (one_in_(4)) return (TERM_L_RED);

    return (TERM_RED);
}

static byte smoke_color(void)
{
    if (one_in_(4)) return (TERM_L_RED);

    return (TERM_L_DARK);
}

/*
 * Return a color to use for the bolt/ball spells
 */
byte gf_color(int type)
{
    /* Analyze */
    switch (type)
    {
        case GF_MISSILE:	return (TERM_VIOLET);
        case GF_ACID:		return (acid_color());
        case GF_ELEC:		return (elec_color());
        case GF_ELEC_BURST:	return (elec_color());
        case GF_FIRE:		return (fire_color());
        case GF_COLD:		return (cold_color());
        case GF_POIS:		return (pois_color());
        case GF_HOLY_ORB:	return (orb_color());
        case GF_MANA:		return (mana_color());
        case GF_STATIC:		return (TERM_WHITE);
        case GF_ARROW:		return (TERM_WHITE);
        case GF_WATER:		return (water_color());
        case GF_EXTINGUISH:	return (TERM_BLUE);
        case GF_CLEAR_AIR:	return (TERM_WHITE);
        case GF_NETHER:		return (TERM_L_GREEN);
        case GF_CHAOS:		return (mh_attr());
        case GF_DISENCHANT:	return (disen_color());
        case GF_STERILIZE:	return (TERM_YELLOW);
        case GF_NEXUS:		return (nexus_color());
        case GF_CONFUSION:	return (confu_color());
        case GF_SOUND:		return (sound_color());
        case GF_SPORE:		return (spore_color());
        case GF_SHARD:		return (TERM_UMBER);
        case GF_FORCE:		return (TERM_UMBER);
        case GF_KILL_WALL:	return (TERM_COPPER);
        case GF_KILL_TRAP:	return (TERM_L_BLUE);
        case GF_KILL_DOOR:	return (TERM_SILVER);
        case GF_MAKE_WALL:	return (TERM_EARTH_YELLOW);
        case GF_MAKE_DOOR:	return (TERM_COPPER);
        case GF_MAKE_TRAP:	return (TERM_GOLD);
        case GF_AWAY_UNDEAD:return (TERM_ORANGE_PEEL);
        case GF_AWAY_EVIL:	return (TERM_SNOW_WHITE);
        case GF_AWAY_ALL:	return (TERM_JUNGLE_GREEN);
        case GF_TURN_UNDEAD:return (TERM_MAHAGONY);
        case GF_TURN_EVIL:	return (TERM_MAIZE);
        case GF_TURN_ALL:	return (TERM_RED_RUST);
        case GF_DISP_UNDEAD:return (TERM_TAUPE);
        case GF_DISP_EVIL:	return (TERM_SKY_BLUE);
        case GF_DISP_ALL:	return (TERM_PURPLE);
        case GF_MAKE_WARY:	return (TERM_MAIZE);
        case GF_OLD_CLONE:	return (TERM_BLUE);
        case GF_OLD_POLY:	return (mh_attr());
        case GF_OLD_HEAL:	return (TERM_SNOW_WHITE);
        case GF_OLD_SPEED:	return (TERM_ORANGE);
        case GF_OLD_SLOW:	return (TERM_PINK);
        case GF_OLD_CONF:	return (confu_color());
        case GF_OLD_SLEEP:	return (TERM_L_DARK);
        case GF_INERTIA:	return (iner_color());
        case GF_GRAVITY:	return (grav_color());
        case GF_TIME:		return (TERM_L_BLUE);
        case GF_LIGHT_WEAK:	return (light_color());
        case GF_LIGHT:		return (light_color());
        case GF_DARK_WEAK:	return (TERM_L_DARK);
        case GF_DARK:		return (TERM_L_DARK);
        case GF_PLASMA:		return (plasma_color());
        case GF_METEOR:		return (meteor_color());
        case GF_ICE:		return (ice_color());
        case GF_MASS_IDENTIFY:	return (TERM_WHITE);
        case GF_SMOKE:		return (smoke_color());
        case GF_FOG:		return (fog_color());
        case GF_SAND:		return (TERM_YELLOW);
        case GF_BMUD:		return (TERM_ORANGE);
        case GF_BWATER:		return (bwater_color());
        case GF_LAVA:		return (lava_color());
        case GF_LIFE_DRAIN:  return (TERM_VIOLET);
    }

    /* Standard "color" */
    return (TERM_WHITE);
}
