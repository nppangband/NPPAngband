/* File: store.c */

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
#include "src/store.h"


/*** Constants and definitions ***/

/* Easy names for the elements of the 'scr_places' arrays. */
enum
{
    LOC_PRICE = 0,
    LOC_OWNER,
    LOC_HEADER,
    LOC_ITEMS_START,
    LOC_ITEMS_END,
    LOC_MORE,
    LOC_HELP_CLEAR,
    LOC_HELP_PROMPT,
    LOC_AU,
    LOC_WEIGHT,
    LOC_CUR_QUEST1,
    LOC_CUR_QUEST2,
    LOC_GUILD_REP,

    LOC_MAX
};

/* Places for the various things displayed onscreen */
static unsigned int scr_places_x[LOC_MAX];
static unsigned int scr_places_y[LOC_MAX];


/* State flags */
#define STORE_GOLD_CHANGE      0x01
#define STORE_FRAME_CHANGE     0x02

#define STORE_SHOW_HELP        0x04

#define STORE_MAX_ITEM			99


/* Compound flag for the initial display of a store */
#define STORE_INIT_CHANGE		(STORE_FRAME_CHANGE | STORE_GOLD_CHANGE)


/** Variables to maintain state ***/

/* Flags for the display */
static u16b store_flags;

static int services_min;
static int services_max;
static int quests_min;
static int quests_max;




/*List of various store services allowed*/
/*
 * Timed effects
 */
enum
{
    SERVICE_ENCHANT_ARMOR	= 0,
    SERVICE_ENCHANT_TO_HIT,
    SERVICE_ENCHANT_TO_DAM,
    SERVICE_ELEM_BRAND_WEAP,
    SERVICE_ELEM_BRAND_AMMO,
    SERVICE_RECHARGING,
    SERVICE_IDENTIFY,
    SERVICE_IDENTIFY_FULLY,
    SERVICE_CURE_CRITICAL,
    SERVICE_RESTORE_LIFE_LEVELS,
    SERVICE_REMOVE_CURSE,
    SERVICE_REMOVE_HEAVY_CURSE,
    SERVICE_RESTORE_STAT,
    SERVICE_INCREASE_STAT,
    SERVICE_CREATE_RANDART,
    SERVICE_PROBE_QUEST_MON,
    SERVICE_BUY_HEALING_POTION,
    SERVICE_BUY_LIFE_POTION,
    SERVICE_BUY_SCROLL_BANISHMENT,
    SERVICE_FIREPROOF_BOOK,
    SERVICE_QUEST_DEFER_REWARD,
    SERVICE_ABANDON_QUEST,
    SERVICE_QUEST_REWARD_RANDART,
    SERVICE_QUEST_REWARD_INC_HP,
    SERVICE_QUEST_REWARD_INC_STAT,
    SERVICE_QUEST_REWARD_AUGMENTATION,


    STORE_SERVICE_MAX
};

#define QUEST_REWARD_HEAD	SERVICE_QUEST_DEFER_REWARD
#define QUEST_REWARD_TAIL	SERVICE_QUEST_REWARD_AUGMENTATION

/* Indicates which store offers the service*/
static byte service_store[STORE_SERVICE_MAX] =
{
    STORE_ARMOR,		/*  SERVICE_ENCHANT_ARMOR   	*/
    STORE_WEAPON,		/*  SERVICE_ENCHANT_TO_HIT   	*/
    STORE_WEAPON,		/*  SERVICE_ENCHANT_TO_DAM   	*/
    STORE_WEAPON,		/*  SERVICE_ELEM_BRAND_WEAP   	*/
    STORE_WEAPON,		/*  SERVICE_ELEM_BRAND_AMMO   	*/
    STORE_MAGIC, 		/*  SERVICE_RECHARGING   		*/
    STORE_MAGIC, 		/*  SERVICE_IDENTIFY   			*/
    STORE_MAGIC, 		/*  SERVICE_IDENTIFY_FULLY		*/
    STORE_TEMPLE, 		/*  SERVICE_CURE_CRITICAL  		*/
    STORE_TEMPLE, 		/*  SERVICE_RESTORE_LIFE_LEVELS	*/
    STORE_TEMPLE, 		/*  SERVICE_REMOVE_CURSE   		*/
    STORE_TEMPLE, 		/*  SERVICE_REMOVE_HEAVY_CURSE	*/
    STORE_ALCHEMY, 		/*  SERVICE_RESTORE_STAT   		*/
    STORE_ALCHEMY, 		/*  SERVICE_INCREASE_STAT   	*/
    STORE_GUILD,		/*  SERVICE_CREATE_RANDART   	*/
    STORE_GUILD,		/*	SERVICE_PROBE_QUEST_MON		*/
    STORE_TEMPLE,		/*	SERVICE_BUY_HEALING_POTION	*/
    STORE_TEMPLE,		/*	SERVICE_BUY_LIFE_POTION		*/
    STORE_MAGIC,		/*	SERVICE_BUY_SCROLL_BANISHMENT	*/
    STORE_BOOKSHOP,		/*	SERVICE_FIREPROOF_BOOK		*/
    STORE_GUILD,		/*	SERVICE_QUEST_DEFER_REWARD	*/
    STORE_GUILD,		/*	SERVICE_ABANDON_QUEST		*/
    STORE_GUILD,		/*	SERVICE_QUEST_REWARD_RANDART	*/
    STORE_GUILD,		/*	SERVICE_QUEST_REWARD_INC_HP		*/
    STORE_GUILD,		/*	SERVICE_QUEST_REWARD_INC_STAT	*/
    STORE_GUILD			/*	SERVICE_QUEST_REWARD_AUGMENTATION	*/

};

/* Indicates the base price of the service*/
static u32b service_price[STORE_SERVICE_MAX] =
{
    125,				/*  SERVICE_ENCHANT_ARMOR   	*/
    125,				/*  SERVICE_ENCHANT_TO_HIT   	*/
    125,				/*  SERVICE_ENCHANT_TO_DAM   	*/
    35000,				/*  SERVICE_ELEM_BRAND_WEAP   	*/
    17500,				/*  SERVICE_ELEM_BRAND_AMMO   	*/
    175, 				/*  SERVICE_RECHARGING   		*/
    75, 				/*  SERVICE_IDENTIFY   			*/
    4500,		 		/*  SERVICE_IDENTIFY_FULLY		*/
    75, 				/*  SERVICE_CURE_CRITICAL  		*/
    1000, 				/*  SERVICE_RESTORE_LIFE_LEVELS	*/
    300, 				/*  SERVICE_REMOVE_CURSE   		*/
    15000, 				/*  SERVICE_REMOVE_HEAVY_CURSE	*/
    700, 				/*  SERVICE_RESTORE_STAT   		*/
    37500L, 			/*  SERVICE_INCREASE_STAT   	*/
    750000L,			/*  SERVICE_CREATE_RANDART   	*/
    150,				/*	SERVICE_PROBE_QUEST_MON		*/
    20000L,				/*	SERVICE_BUY_HEALING_POTION	*/
    125000L,			/*	SERVICE_BUY_LIFE_POTION		*/
    125000L,			/*	SERVICE_BUY_SCROLL_BANISHMENT	*/
    100000L,			/*  SERVICE_FIREPROOF_BOOK 		*/
    0,					/*	SERVICE_QUEST_DEFER_REWARD	*/
    0,					/*  SERVICE_ABANDON_QUEST 		*/
    0,					/*	SERVICE_QUEST_REWARD_RANDART	*/
    0,					/*	SERVICE_QUEST_REWARD_INC_HP		*/
    0,					/*	SERVICE_QUEST_REWARD_INC_STAT	*/
    0					/*	SERVICE_QUEST_REWARD_AUGMENTATION	*/

};

/*
 * Indicates the base price of the service.  [v] means the price varies depending on the item
 */
static QString service_names[STORE_SERVICE_MAX] =
{
    "Enchant armor [price varies]",				/*  SERVICE_ENCHANT_ARMOR   	*/
    "Enchant weapon to-hit [price varies]",			/*  SERVICE_ENCHANT_TO_HIT   	*/
    "Enchant weapon to-dam [price varies]",			/*  SERVICE_ENCHANT_TO_DAM   	*/
    "Elemental Brand a weapon [price varies]",	/*  SERVICE_ELEM_BRAND_WEAP   	*/
    "Elemental brand some ammunition[price varies]",	/*  SERVICE_ELEM_BRAND_AMMO */
    "Recharge item [price varies]",				/*  SERVICE_RECHARGING   		*/
    "Identify item",							/*  SERVICE_IDENTIFY   			*/
    "*Identify* item",							/*  SERVICE_IDENTIFY_FULLY		*/
    "Cure Critical Wounds",	 					/*  SERVICE_CURE_CRITICAL  		*/
    "Restore Life Levels",						/*  SERVICE_RESTORE_LIFE_LEVELS	*/
    "Remove curse", 							/*  SERVICE_REMOVE_CURSE   		*/
    "Remove *curse*", 							/*  SERVICE_REMOVE_HEAVY_CURSE	*/
    "Restore stat", 							/*  SERVICE_RESTORE_STAT   		*/
    "Increase stat", 							/*  SERVICE_INCREASE_STAT   	*/
    "Create Artifact[price varies]",			/*  SERVICE_CREATE_RANDART   	*/
    "Probe a Quest Monster[price varies]",		/*	SERVICE_PROBE_QUEST_MON		*/
    "Purchase Potion of Healing",				/*	SERVICE_BUY_HEALING_POTION	*/
    "Purchase Potion of Life",					/*	SERVICE_BUY_LIFE_POTION		*/
    "Purchase Scroll of Mass Banishment",		/*	SERVICE_BUY_SCROLL_BANISHMENT	*/
    "Make Spell Book Fireproof[price varies]",	/*  SERVICE_FIREPROOF_BOOK */
    "Defer Quest Reward",						/*	SERVICE_QUEST_DEFER_REWARD	*/
    "Abandon Your Quest",						/*  SERVICE_ABANDON_QUEST 		*/
    "Create Artifact Quest Reward",				/*	SERVICE_QUEST_REWARD_RANDART	*/
    "Permanent Hit Point Increase Reward",		/*	SERVICE_QUEST_REWARD_INC_HP		*/
    "Permanent Stat Increase Reward",			/*	SERVICE_QUEST_REWARD_INC_STAT	*/
    "Permanent Stats Augmentation Reward"		/*	SERVICE_QUEST_REWARD_AUGMENTATION	*/
};


static byte services_offered[STORE_SERVICE_MAX];
static byte quests_offered[QUEST_SLOT_MAX];

/* Quest Titles*/
static QString quest_title[QUEST_SLOT_MAX] =
{
    "Monster or Unique Quest",	/* QUEST_MONSTER*/
    "Guardian Quest",			/* QUEST_GUARDIAN */
    "Pit or Nest Quest",		/* QUEST_PIT*/
    "Wilderness Quest",			/* QUEST_WILDERNESS */
    "Level Quest",				/* QUEST_THEMED_LEVEL*/
    "Vault Quest",				/* QUEST_VAULT*/
    "Arena Quest",				/* QUEST_ARENA_LEVEL */
    "Labyrinth Quest",			/* QUEST_LABYRINTH_LEVEL */
    "Greater Vault Quest"		/* QUEST_SLOT_GREATER_VAULT */
};

/*
 * Delete an object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 */
void store_delete_index(int st, int what)
{
    int num;
    object_type *o_ptr;

    store_type *st_ptr = &store[st];

    /* Paranoia */
    if (st_ptr->stock_num <= 0) return;

    /* keep certain items */

    /* Get the object */
    o_ptr = &st_ptr->stock[what];

    /* Determine how many objects are in the slot */
    num = o_ptr->number;

    /* Some stores keep large amounts of certain objects in stock objects*/
    /*
    if ((st != STORE_B_MARKET) && (keep_in_stock(o_ptr, st)))
    {
        if (o_ptr->number > 60) num = num / 2;
        return;
    }
    */

    /* Deal with stacks */
    if (num > 1)
    {
        /* Special behaviour for arrows, bolts &tc. */
        switch (o_ptr->tval)
        {
            case TV_SPIKE:
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
            {
                /* 50% of the time, destroy the entire stack */
                if (randint0(100) < 50 || num < 10)
                    num = o_ptr->number;

                /* 50% of the time, reduce the size to a multiple of 5 */
                else
                    num = randint1(num / 5) * 5 + (num % 5);

                break;
            }

            default:
            {
                /* 50% of the time, destroy a single object */
                if (randint0(100) < 50) num = 1;

                /* 25% of the time, destroy half the objects */
                else if (randint0(100) < 50) num = (num + 1) / 2;

                /* 25% of the time, destroy all objects */
                else num = o_ptr->number;

                /* Hack -- decrement the total charges of staves and wands. */
                if (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND)
                {
                    o_ptr->pval -= num * o_ptr->pval / o_ptr->number;
                }
            }
        }

    }

    /*Wipe the randart if necessary*/
    if (o_ptr->art_num) artifact_wipe(o_ptr->art_num, FALSE);

    /* Delete the item */
    store_item_increase(st, what, -num);
    store_item_optimize(st, what);
}


/*** Utilities ***/

/*
 * Return the owner struct for the given store.
 */
static owner_type *store_owner(int st)
{
    store_type *st_ptr = &store[st];
    return &b_info[(st * z_info->b_max) + st_ptr->owner];
}


/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[randint0(N_ELEMENTS(x))]


/*
 * Shopkeeper welcome messages.
 *
 * The shopkeeper's name must come first, then the character's name.
 */
static const char *comment_welcome[] =
{
    "",
    "%s nods to you.",
    "%s says hello.",
    "%s: \"See anything you like, adventurer?\"",
    "%s: \"How may I help you, %s?\"",
    "%s: \"Welcome back, %s.\"",
    "%s: \"A pleasure to see you again, %s.\"",
    "%s: \"How may I be of assistance, good %s?\"",
    "%s: \"You do honour to my humble store, noble %s.\"",
    "%s: \"I and my family are entirely at your service, glorious %s.\""
};

/*
 * Messages for reacting to purchase prices.
 */
static const char *comment_worthless[] =
{
    "Arrgghh!",
    "You bastard!",
    "You hear someone sobbing...",
    "The shopkeeper howls in agony!",
    "The shopkeeper wails in anguish!",
    "The shopkeeper mutters in disgust."
};

static const char *comment_bad[] =
{
    "Damn!",
    "You fiend!",
    "The shopkeeper curses at you.",
    "The shopkeeper glares at you."
};

static const char *comment_accept[] =
{
    "Okay.",
    "Fine.",
    "Accepted!",
    "Agreed!",
    "Done!",
    "Taken!"
};

static const char *comment_good[] =
{
    "Cool!",
    "You've made my day!",
    "The shopkeeper sniggers.",
    "The shopkeeper giggles.",
    "The shopkeeper laughs loudly."
};

static const char *comment_great[] =
{
    "Yipee!",
    "I think I'll retire!",
    "The shopkeeper jumps for joy.",
    "The shopkeeper smiles gleefully.",
    "Wow.  I'm going to name my new villa in your honour."
};

/*
 * Increase, by a 'num', the number of an item 'item' in store 'st'.
 * This can result in zero items.
 */
void store_item_increase(int st, int item, int num)
{
    int cnt;
    object_type *o_ptr;

    store_type *st_ptr = &store[st];

    /* Get the object */
    o_ptr = &st_ptr->stock[item];

    /* Verify the number */
    cnt = o_ptr->number + num;
    if (cnt > STORE_MAX_ITEM) cnt = STORE_MAX_ITEM;
    else if (cnt < 0) cnt = 0;
    num = cnt - o_ptr->number;

    /* Save the new number */
    o_ptr->number += num;

    /* Hack - don't let the store be bought out of items that are always in stock run out */
    /*
    if (keep_in_stock(o_ptr, st))
    {
        if (st != STORE_HOME) o_ptr->number = STORE_MAX_ITEM;
    }
    */
}


/*
 * Remove a slot if it is empty, in store 'st'.
 */
void store_item_optimize(int st, int item)
{
    int j;
    object_type *o_ptr;

    store_type *st_ptr = &store[st];

    /* Get the object */
    o_ptr = &st_ptr->stock[item];

    /* Must exist */
    if (!o_ptr->k_idx) return;

    /* Must have no items */
    if (o_ptr->number) return;

    /* One less object */
    st_ptr->stock_num--;

    /* Slide everyone */
    for (j = item; j < st_ptr->stock_num; j++)
    {
        st_ptr->stock[j] = st_ptr->stock[j + 1];
    }

    /* Nuke the final slot */
    //object_wipe(&st_ptr->stock[j]);
}

