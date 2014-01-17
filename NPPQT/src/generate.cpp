/* File: generate.cpp */

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

#include "src/npp.h"



/*
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines...  Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "vault.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors.  Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type.  The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).  This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the 33x11 panels)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  This can be so annoying that the player must set
 * a special option to enable "non-aligned" room generation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits".  The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon).  Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
 *
 * Notes for NPPAngband 0.2.0
 * The addition of smaller (arena) levels was causing the level generation routine
 * to lock up approximately once every 200-300 generated levels.  Occasionally a
 * level with only one or zero rooms is generated, and functions such as place_player_spot
 * would lock up in an infinite soop trying to find a suitable spot.  To fix this, it was
 * simpler to reject the level and have things start over, rather than try to
 * identify and fix the problem.  Code was adopted from EY Angband, which uses an array for
 * tracking the rooms, rather than just marking them in the dungeon floor array.  Notice all
 * of the "while (true) loops now have an escape mechanism, and many of the functions have
 * been changed to bool.  If they return false, the level is considered defective
 * (ex. 5000 chances to place the player fail), and level generation is re-started.
 * I generated over 50,000 levels without incident once I made these changes. JG
 */

/*
 * Dungeon generation values
 */

#define DUN_UNUSUAL_MORIA	300	/* Level/chance of unusual room */
#define DUN_UNUSUAL	180	/* Level/chance of unusual room */
#define DUN_DEST	35	/* 1/chance of having a destroyed level */
#define DUN_FRACTAL	25	/* 1/chance of having a fractal level */
#define SMALL_LEVEL	10	/* 1/chance of smaller size */
#define THEMED_LEVEL_CHANCE	75	/* 1/chance of being a themed level */
#define WILDERNESS_LEVEL_CHANCE	75 /* 1/chance of being a pseudo-wilderness level */
#define LABYRINTH_LEVEL_CHANCE	75 /* 1/chance of being a labrynth level */
#define GREATER_VAULT_LEVEL_CHANCE	150 /* 1/chance of being a greater vault level */

#define DUN_MAX_LAKES	3	/* Maximum number of lakes/rivers */
#define DUN_FEAT_RNG	2	/* Width of lake */

/* Maximum size of a fractal map */
#define MAX_FRACTAL_SIZE 65

/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND	10	/* Chance of random direction */
#define DUN_TUN_CHG	30	/* Chance of changing direction */
#define DUN_TUN_CON	15	/* Chance of extra tunneling */
#define DUN_TUN_PEN	25	/* Chance of doors at room entrances */
#define DUN_TUN_JCT	90	/* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_DEN	5	/* Density of streamers */
#define DUN_STR_RNG	2	/* Width of streamers */
#define DUN_STR_MAG	3	/* Number of magma streamers */
#define DUN_STR_MC	90	/* 1/chance of treasure per magma */
#define DUN_STR_QUA	2	/* Number of quartz streamers */
#define DUN_STR_QC	40	/* 1/chance of treasure per quartz */
#define DUN_STR_SAN	2	/* Number of sandstone streamers */
#define DUN_STR_SLV	40	/* Deepest level sandstone occurs instead of magma */
#define DUN_STR_GOL	20	/* 1/chance of rich mineral vein */
#define DUN_STR_GC	2	/* 1/chance of treasure per rich mineral vein */
#define DUN_STR_CRA	8	/* 1/chance of cracks through dungeon */
#define DUN_STR_CC	0	/* 1/chance of treasure per crack */

/*
 * Dungeon treausre allocation values
 */
#define DUN_AMT_ROOM	9	/* Amount of objects for rooms */
#define DUN_AMT_ITEM	3	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD	3	/* Amount of treasure for rooms/corridors */

#define DUN_AMT_ROOM_MORIA	7	/* Amount of objects for rooms */
#define DUN_AMT_ITEM_MORIA	2	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD_MORIA	2	/* Amount of treasure for rooms/corridors */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR		1	/* Hallway */
#define ALLOC_SET_ROOM		2	/* Room */
#define ALLOC_SET_BOTH		3	/* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1	/* Rubble */
#define ALLOC_TYP_TRAP		3	/* Trap */
#define ALLOC_TYP_GOLD		4	/* Gold */
#define ALLOC_TYP_OBJECT	5	/* Object */
#define ALLOC_TYP_CHEST		6	/* Object */
#define ALLOC_TYP_PARCHMENT 7	/* Special parchment, for collection quests */


/*
 * Maximum numbers of rooms along each axis (currently 6x18)
 */

#define MAX_ROOMS_ROW	(MAX_DUNGEON_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(MAX_DUNGEON_WID / BLOCK_WID)

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	110
#define DOOR_MAX	200
#define WALL_MAX	500
#define TUNN_MAX	900


/*
 * These flags control the construction of starburst rooms and lakes
 */
#define STAR_BURST_ROOM		0x00000001	/* Mark grids with CAVE_ROOM */
#define STAR_BURST_LIGHT	0x00000002	/* Mark grids with CAVE_GLOW */
#define STAR_BURST_CLOVER	0x00000004	/* Allow cloverleaf rooms */
#define STAR_BURST_RAW_FLOOR	0x00000008	/* Floor overwrites dungeon */
#define STAR_BURST_RAW_EDGE	0x00000010	/* Edge overwrites dungeon */


bool allow_uniques;


/*
 * Maximal number of room types
 */
#define ROOM_MAX	15
#define ROOM_MIN	2

/*
 * Maximum distance between rooms
 */
#define MAX_RANGE_TO_ROOM 15


/*
 * Room type information
 */

typedef struct room_data room_data;

struct room_data
{
    /* Required size in blocks */
    s16b dy1, dy2, dx1, dx2;

    /* Hack -- minimum level */
    s16b level;
};


/*
 * Structure to hold all "dungeon generation" data
 */

typedef struct dun_data dun_data;

struct dun_data
{
    /* Array of centers of rooms */
    int cent_n;
    coord cent[CENT_MAX];

    /* Array of possible door locations */
    int door_n;
    coord door[DOOR_MAX];

    /* Array of wall piercing locations */
    int wall_n;
    coord wall[WALL_MAX];

    /* Array of tunnel grids */
    int tunn_n;
    coord tunn[TUNN_MAX];

    /* Number of blocks along each axis */
    int row_rooms;
    int col_rooms;

    /* Array of which blocks are used */
    bool room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

    /* Hack -- there is a pit/nest on this level */
    bool crowded;
};

/*
 * Dungeon generation data -- see "cave_gen()"
 */
static dun_data *dun;

/*
 * Allow escorts
 */
static bool can_place_escorts_true(s16b r_idx)
{
    // Unused vaiable
    (void)(r_idx);

    return (TRUE);
}

/*
 * Allow escorts
 */
static bool can_place_escorts_false(s16b r_idx)
{
    // Unused vaiable
    (void)(r_idx);

    return (FALSE);
}

/*
 * Player in rooms
 */
static bool can_place_player_in_rooms_false(void)
{
    return (FALSE);
}


/*
 * Player in rooms
 */
static bool can_place_player_in_rooms_true(void)
{
    return (TRUE);
}


/*
 * Valid location for stairs
 */
static bool can_place_stairs_default(int y, int x)
{
    return (cave_naked_bold(y, x) ? TRUE: FALSE);
}


/*
 * Adjust the number of stairs in a level
 */
static int adjust_stairs_number_default(int initial_amount)
{
    /* Smaller levels don't need that many stairs, but there are a minimum of 4 rooms */
    if (dun->cent_n > 0)
    {
        if (dun->cent_n <= 4) initial_amount = 1;
        else if (initial_amount > (dun->cent_n / 2)) initial_amount = dun->cent_n / 2;
    }

    return (initial_amount);
}


/*
 * Fog in rooms
 */
static bool can_place_fog_in_rooms_default(void)
{
    return (one_in_(3));
}


/*
 * Fog in rooms
 */
static bool can_place_fog_in_rooms_true(void)
{
    return (TRUE);
}


/*
 * Fog in rooms
 */
static bool can_place_fog_in_rooms_false(void)
{
    return (TRUE);
}


/*
 * Feature is interesting for the look command
 */
static bool can_target_feature_default(int f_idx)
{
    return (feat_ff1_match(f_idx, FF1_NOTICE) ? TRUE: FALSE);
}


/*
 * Dungeon can be transformed
 */
static bool can_be_transformed_true(void)
{
    return (TRUE);
}


/*
 * Dungeon can be transformed
 */
static bool can_be_transformed_false(void)
{
    return (FALSE);
}


/*
 * Non native monsters in elemental terrain
 */
static bool can_place_non_native_monsters_false(void)
{
    return (FALSE);
}


/*
 * Non native monsters in elemental terrain
 */
static bool can_place_non_native_monsters_true(void)
{
    return (TRUE);
}


/*
 * Allow repopulation of monsters on level
 */
static bool allow_level_repopulation_true(void)
{
    return (TRUE);
}


/*
 * Disallow repopulation of monsters on level
 */
static bool allow_level_repopulation_false(void)
{
    return (FALSE);
}


/*
 * Only allow summoners to be relocated from the current level
 */
static bool limited_level_summoning_true(void)
{
    return (TRUE);
}


/*
 * Summoned monsters can be appear from nowhere
 */
static bool limited_level_summoning_false(void)
{
    return (FALSE);
}


/*
 * Only allow summoners to be relocated from the current level
 */
static bool allow_monster_multiply_true(void)
{
    return (TRUE);
}


/*
 * Summoned monsters can be appear from nowhere
 */
static bool allow_monster_multiply_false(void)
{
    return (FALSE);
}


/* Allow breeders to slowly spread */
static bool allow_monster_multiply_quarter(void)
{
    if (one_in_(4)) return (TRUE);
    return (FALSE);
}


/*
 * Earthquakes and destruction are prevented.
 */
static bool prevent_destruction_true(void)
{
    return (TRUE);
}


/*
 * Earthquakes and destruction are allowed.
 */
static bool prevent_destruction_false(void)
{
    return (FALSE);
}


/*
 * Earthquakes and destruction are allowed, except in town.
 */
static bool prevent_destruction_default(void)
{
    if (!p_ptr->depth) return (TRUE);

    return (FALSE);
}


/*
 * Monsters in level
 */
static int get_monster_count_default(void)
{
    return (MIN_M_ALLOC_LEVEL);
}


/*
 * Objects in rooms
 */
static int get_object_count_default(void)
{
    return (Rand_normal(DUN_AMT_ROOM, 3));
}


/*
 * Objects in rooms
 */
static int get_object_count_zero(void)
{
    return (0);
}


/*
 * Gold in both rooms and corridors
 */
static int get_gold_count_default(void)
{
    return (Rand_normal(DUN_AMT_GOLD, 3));
}


/*
 * Gold in both rooms and corridors
 */
static int get_gold_count_zero(void)
{
    return (0);
}


/*
 * objects in both rooms and corridors
 */
static int get_extra_object_count_default(void)
{
    return (Rand_normal(DUN_AMT_ITEM, 3));
}


/*
 * Dungeon capabilities for classic levels
 */
static dungeon_capabilities_type dun_cap_body_default =
{
    can_place_escorts_true,
    can_place_player_in_rooms_false,
    can_place_stairs_default,
    adjust_stairs_number_default,
    can_place_fog_in_rooms_default,
    can_target_feature_default,
    can_be_transformed_true,
    can_place_non_native_monsters_false,
    allow_level_repopulation_true,
    limited_level_summoning_false,
    allow_monster_multiply_true,
    prevent_destruction_default,
    get_monster_count_default,
    get_object_count_default,
    get_gold_count_default,
    get_extra_object_count_default,
};

/*
 * Monsters in level
 */
static int get_monster_count_moria(void)
{
    int alloc_level = (p_ptr->depth / 3);

    /* Boundary Control */
    if (alloc_level < 2) alloc_level = 2;
    else if (alloc_level > 10) alloc_level = 10;

    alloc_level += MIN_M_ALLOC_LEVEL;

    return (alloc_level);
}


/*
 * Objects in rooms
 */
static int get_object_count_moria(void)
{
    return (Rand_normal(DUN_AMT_ROOM_MORIA, 3));
}


/*
 * Gold in both rooms and corridors
 */
static int get_gold_count_moria(void)
{
    return (Rand_normal(DUN_AMT_GOLD_MORIA, 3));
}

/*
 * objects in both rooms and corridors
 */
static int get_extra_object_count_moria(void)
{
    int alloc_level = (p_ptr->depth / 3);

    /* Boundary Control */
    if (alloc_level < 2) alloc_level = 2;
    else if (alloc_level > 10) alloc_level = 10;

    return (DUN_AMT_ITEM_MORIA + randint1(alloc_level));
}


/*
 * Dungeon capabilities for classic levels
 */
static dungeon_capabilities_type dun_cap_body_moria =
{
    can_place_escorts_false,
    can_place_player_in_rooms_false,
    can_place_stairs_default,
    adjust_stairs_number_default,
    can_place_fog_in_rooms_false,
    can_target_feature_default,
    can_be_transformed_false,
    can_place_non_native_monsters_false,
    allow_level_repopulation_true,
    limited_level_summoning_false,
    allow_monster_multiply_true,
    prevent_destruction_default,
    get_monster_count_moria,
    get_object_count_moria,
    get_gold_count_moria,
    get_extra_object_count_moria,
};



/*
 * Allow escorts
 */
static bool can_place_escorts_wild(s16b r_idx)
{
    u32b flags1 = r_info[r_idx].flags1;

    /* Uniques are allowed */
    if (flags1 & (RF1_UNIQUE)) return (TRUE);

    /* Monsters with escorts are allowed only part of the time */
    if (flags1 & (RF1_FRIEND | RF1_FRIENDS | RF1_ESCORT | RF1_ESCORTS)) return (one_in_(10));

    /* Default */
    return (FALSE);
}


/*
 * Valid location for stairs
 */
static bool can_place_stairs_wild(int y, int x)
{
    return (cave_plain_bold(y, x) ? TRUE: FALSE);
}


/*
 * Adjust the number of stairs in a level
 */
static int adjust_stairs_number_unchanged(int initial_amount)
{
    return (initial_amount);
}


/*
 * Feature is interesting for the look command
 */
static bool can_target_feature_wild(int f_idx)
{
    /* Only stairs and doors */
    return (feat_ff1_match(f_idx, FF1_STAIRS | FF1_DOOR) ? TRUE: FALSE);
}


/*
 * Monsters in level
 */
static int get_monster_count_wild(void)
{
    int count = 0;
    int y, x;
    u32b ff1 = (FF1_MOVE | FF1_PLACE);

    /* Count the grids that allow monsters in them */
    for (y = 0; y < p_ptr->cur_map_hgt; y++)
    {
        for (x = 0; x < p_ptr->cur_map_wid; x++)
        {
            /* Found one? Increment the count */
            if (cave_ff1_match(y, x, ff1) == ff1) ++count;
        }
    }

    /* Calculate the monster ratio */
    count = ((15 * count) / 1000);

    /* Paranoia */
    return (MAX(MIN_M_ALLOC_LEVEL, count));
}


/*
 * Objects in rooms
 */
static int get_object_count_wild(void)
{
    return (Rand_normal(90, 20));
}


/*
 * Gold in both rooms and corridors
 */
static int get_gold_count_wild(void)
{
    return (Rand_normal(30, 10));
}


/*
 * Objects in both rooms and corridors
 */
static int get_extra_object_count_zero(void)
{
    return (0);
}


/*
 * Dungeon capabilities for wilderness levels
 */
static dungeon_capabilities_type dun_cap_body_wild =
{
    can_place_escorts_wild,
    can_place_player_in_rooms_true,
    can_place_stairs_wild,
    adjust_stairs_number_unchanged,
    can_place_fog_in_rooms_true,
    can_target_feature_wild,
    can_be_transformed_false,
    can_place_non_native_monsters_true,
    allow_level_repopulation_false,
    limited_level_summoning_true,
    allow_monster_multiply_quarter,
    prevent_destruction_true,
    get_monster_count_wild,
    get_object_count_wild,
    get_gold_count_wild,
    get_extra_object_count_zero,
};


/*
 * Monsters in level
 */
static int get_monster_count_labyrinth(void)
{
    int count = 0;
    int y, x;
    u32b ff1 = (FF1_MOVE | FF1_PLACE);

    /* Count the grids that allow monsters in them */
    for (y = 0; y < p_ptr->cur_map_hgt; y++)
    {
        for (x = 0; x < p_ptr->cur_map_wid; x++)
        {
            /* Found one? Increment the count */
            if (cave_ff1_match(y, x, ff1) == ff1) ++count;
        }
    }

    /* Calculate the monster ratio */
    count = ((25 * count) / 1000);

    /* Paranoia */
    return (MAX(MIN_M_ALLOC_LEVEL, count));
}


/*
 * Objects in rooms
 */
static int get_object_count_labyrinth(void)
{
    return (Rand_normal(40, 10));
}


/*
 * Dungeon capabilities for labyrinth levels
 */
static dungeon_capabilities_type dun_cap_body_labyrinth =
{
    can_place_escorts_true,
    can_place_player_in_rooms_true,
    can_place_stairs_default,
    adjust_stairs_number_unchanged,
    can_place_fog_in_rooms_false,
    can_target_feature_default,
    can_be_transformed_false,
    can_place_non_native_monsters_true,
    allow_level_repopulation_false,
    limited_level_summoning_false,
    allow_monster_multiply_quarter,
    prevent_destruction_true,
    get_monster_count_labyrinth,
    get_object_count_labyrinth,
    get_gold_count_wild,
    get_extra_object_count_zero,
};


/*
 * Dungeon capabilities for arena levels
 */
static dungeon_capabilities_type dun_cap_body_arena =
{
    can_place_escorts_true,
    can_place_player_in_rooms_false,
    can_place_stairs_default,
    adjust_stairs_number_unchanged,
    can_place_fog_in_rooms_false,
    can_target_feature_default,
    can_be_transformed_false,
    can_place_non_native_monsters_true,
    allow_level_repopulation_false,
    limited_level_summoning_true,
    allow_monster_multiply_false,
    prevent_destruction_true,
    get_monster_count_labyrinth,
    get_object_count_zero,
    get_gold_count_zero,
    get_extra_object_count_zero,
};


/*
 * Dungeon capabilities for wilderness levels
 */
static dungeon_capabilities_type dun_cap_body_themed_level =
{
    can_place_escorts_true,
    can_place_player_in_rooms_false,
    can_place_stairs_default,
    adjust_stairs_number_default,
    can_place_fog_in_rooms_true,
    can_target_feature_default,
    can_be_transformed_true,
    can_place_non_native_monsters_true,
    allow_level_repopulation_false,
    limited_level_summoning_true,
    allow_monster_multiply_false,
    prevent_destruction_false,
    get_monster_count_labyrinth,
    get_object_count_zero,
    get_gold_count_zero,
    get_extra_object_count_zero,
};


/*
 * Dungeon capabilities for wilderness levels
 */
static dungeon_capabilities_type dun_cap_body_greater_vault =
{
    can_place_escorts_true,
    can_place_player_in_rooms_false,
    can_place_stairs_default,
    adjust_stairs_number_default,
    can_place_fog_in_rooms_false,
    can_target_feature_default,
    can_be_transformed_false,
    can_place_non_native_monsters_true,
    allow_level_repopulation_false,
    limited_level_summoning_false,
    allow_monster_multiply_quarter,
    prevent_destruction_true,
    get_monster_count_labyrinth,
    get_object_count_zero,
    get_gold_count_zero,
    get_extra_object_count_zero,
};


/*
 * Get the proper dungeon capabilities based on the given dungeon type
 */
void set_dungeon_type(u16b dungeon_type)
{
    /* Remember the type */
    p_ptr->dungeon_type = dungeon_type;

    /* Get the capabilities */
    switch (dungeon_type)
    {
        /* Special rules for wilderness levels */
        case DUNGEON_TYPE_WILDERNESS:
        {
            dun_cap = &dun_cap_body_wild;
            break;
        }
        /* Special rules for wilderness levels */
        case DUNGEON_TYPE_LABYRINTH:
        {
            dun_cap = &dun_cap_body_labyrinth;
            break;
        }
        case DUNGEON_TYPE_THEMED_LEVEL:
        {
            dun_cap = &dun_cap_body_themed_level;
            break;
        }
        case DUNGEON_TYPE_ARENA:
        {
            dun_cap = &dun_cap_body_arena;
            break;
        }
        case DUNGEON_TYPE_GREATER_VAULT:
        {
            dun_cap = &dun_cap_body_greater_vault;
            break;
        }
        /* Classic dungeons */
        default:
        {
            if (game_mode == GAME_NPPMORIA) dun_cap = &dun_cap_body_moria;

            else dun_cap = &dun_cap_body_default;
        }
    }
}

