#ifndef SQUELCH_H
#define SQUELCH_H

#include <src/structures.h>

const size_t squelch_size = SQUELCH_BYTES;

/*
 * These are the base types for quelching on identification.
 * Some of the tvls are combined by ini_tv_to_type to make this
 * list a little more reasonable.
 */

#define TYPE_AMMO    1
#define TYPE_BOW     2
#define TYPE_WEAPON1 3
#define TYPE_WEAPON2 4
#define TYPE_BODY    5
#define TYPE_CLOAK   6
#define TYPE_SHIELD  7
#define TYPE_HELM    8
#define TYPE_GLOVES  9
#define TYPE_BOOTS   10
#define TYPE_RING    11
#define TYPE_STAFF   12
#define TYPE_WAND    13
#define TYPE_ROD     14
#define TYPE_SCROLL  15
#define TYPE_POTION  16
#define TYPE_AMULET  17
#define TYPE_BOOK    18
#define TYPE_FOOD    19
#define TYPE_MISC    20
#define TYPE_MAX	 TYPE_MISC

#define MAXTV_TO_TYPE  100
/*
 * This (admittedly hacky) stores the mapping from tval to typeval
 * and is reinitialized every time do_cmd_squelch is called.  This
 * can certainly be done more cleanly.
 */
static int tv_to_type[MAXTV_TO_TYPE];
static bool seen_type[TYPE_MAX];

/*
 * List of kinds of item, for pseudo-id squelch.
 */
typedef enum
{
    PS_TYPE_WEAPON_SHARP,
    PS_TYPE_WEAPON_BLUNT,
    PS_TYPE_EQUIP_RARE,
    PS_TYPE_BOW,
    PS_TYPE_MISSILE_SLING,
    PS_TYPE_MISSILE_BOW,
    PS_TYPE_MISSILE_XBOW,
    PS_TYPE_ARMOR_ROBE,
    PS_TYPE_ARMOR_BODY,
    PS_TYPE_ARMOR_DRAGON,
    PS_TYPE_CLOAK,
    PS_TYPE_SHIELD,
    PS_TYPE_HELMS,
    PS_TYPE_CROWNS,
    PS_TYPE_GLOVES,
    PS_TYPE_BOOTS,
    PS_TYPE_DIGGER,
    PS_TYPE_RING,
    PS_TYPE_AMULET,
    PS_TYPE_LIGHT,

    PS_TYPE_MAX
} squelch_type_t;

typedef struct
{
    squelch_type_t squelch_type;
    int tval;
    int min_sval;
    int max_sval;
} quality_squelch_struct;

/*
 * SVAL Ranges for the quality squelch.
 * Note "Rare" items now have their own category.
 * As long as they come first in the list, before the
 * "0 to SV_UNKNOWN" category, they will be grouped
 * with the rare items.
 */
static quality_squelch_struct quality_mapping[] =
{
    { PS_TYPE_EQUIP_RARE,		TV_SWORD,	SV_BLADE_OF_CHAOS,	SV_BLADE_OF_CHAOS },
    { PS_TYPE_WEAPON_SHARP,	TV_SWORD,	0,		SV_UNKNOWN },
    { PS_TYPE_EQUIP_RARE,		TV_POLEARM,	SV_SCYTHE_OF_SLICING,	SV_SCYTHE_OF_SLICING },
    { PS_TYPE_WEAPON_SHARP,	TV_POLEARM,	0,		SV_UNKNOWN },
    { PS_TYPE_EQUIP_RARE,		TV_HAFTED,	SV_MACE_OF_DISRUPTION,	SV_GROND },
    { PS_TYPE_WEAPON_BLUNT,	TV_HAFTED,	0,		SV_UNKNOWN },
    { PS_TYPE_BOW,				TV_BOW,		0,		SV_UNKNOWN },
    { PS_TYPE_MISSILE_SLING,	TV_SHOT,	0,		SV_UNKNOWN },
    { PS_TYPE_MISSILE_BOW,		TV_ARROW,	0,		SV_UNKNOWN },
    { PS_TYPE_MISSILE_XBOW,	TV_BOLT,	0,		SV_UNKNOWN },
    { PS_TYPE_ARMOR_ROBE,		TV_SOFT_ARMOR,	SV_ROBE,	SV_ROBE },
    { PS_TYPE_ARMOR_BODY,		TV_SOFT_ARMOR,	0,		SV_UNKNOWN },
    { PS_TYPE_EQUIP_RARE,		TV_HARD_ARMOR,	SV_MITHRIL_CHAIN_MAIL,	SV_ADAMANTITE_PLATE_MAIL },
    { PS_TYPE_ARMOR_BODY,		TV_HARD_ARMOR,	0,		SV_UNKNOWN },
    { PS_TYPE_ARMOR_DRAGON,	TV_DRAG_ARMOR,	0, 	SV_UNKNOWN},
    { PS_TYPE_ARMOR_DRAGON,	TV_DRAG_SHIELD,	0, 	SV_UNKNOWN},
    { PS_TYPE_EQUIP_RARE,		TV_CLOAK,	SV_SHADOW_CLOAK, 	SV_SHADOW_CLOAK },
    { PS_TYPE_CLOAK,			TV_CLOAK,	0, 		SV_UNKNOWN },
    { PS_TYPE_EQUIP_RARE,		TV_SHIELD,	SV_SHIELD_OF_DEFLECTION, 	SV_SHIELD_OF_DEFLECTION },
    { PS_TYPE_SHIELD,			TV_SHIELD,	0,		SV_UNKNOWN },
    { PS_TYPE_HELMS,			TV_HELM,	0,		SV_UNKNOWN },
    { PS_TYPE_CROWNS,			TV_CROWN,	0,		SV_UNKNOWN },
    { PS_TYPE_GLOVES,			TV_GLOVES,	0,		SV_UNKNOWN },
    { PS_TYPE_BOOTS,			TV_BOOTS,	0,		SV_UNKNOWN },
    { PS_TYPE_DIGGER,			TV_DIGGING,	0,		SV_UNKNOWN },
    { PS_TYPE_RING,			TV_RING,	0,		SV_UNKNOWN },
    { PS_TYPE_AMULET,			TV_AMULET,	0,		SV_UNKNOWN },
    { PS_TYPE_LIGHT, 			TV_LIGHT, 	0,		SV_UNKNOWN },
};

typedef struct
{
    int enum_val;
    const char *name;
} quality_name_struct;

static quality_name_struct quality_choices[PS_TYPE_MAX] =
{
    { PS_TYPE_WEAPON_SHARP,	"Sharp Melee Weapons" },
    { PS_TYPE_WEAPON_BLUNT,	"Blunt Melee Weapons" },
    { PS_TYPE_EQUIP_RARE,		"Rare Equipment" },
    { PS_TYPE_BOW,				"Missile launchers" },
    { PS_TYPE_MISSILE_SLING,	"Shots and Pebbles" },
    { PS_TYPE_MISSILE_BOW,		"Arrows" },
    { PS_TYPE_MISSILE_XBOW,	"Bolts" },
    { PS_TYPE_ARMOR_ROBE,		"Robes" },
    { PS_TYPE_ARMOR_BODY,		"Body Armor" },
    { PS_TYPE_ARMOR_DRAGON,	"Dragon Armor/Shields" },
    { PS_TYPE_CLOAK,			"Cloaks" },
    { PS_TYPE_SHIELD,			"Shields" },
    { PS_TYPE_HELMS,			"Helms" },
    { PS_TYPE_CROWNS,			"Crowns" },
    { PS_TYPE_GLOVES,			"Gloves" },
    { PS_TYPE_BOOTS,			"Boots" },
    { PS_TYPE_DIGGER,			"Diggers" },
    { PS_TYPE_RING,			"Rings" },
    { PS_TYPE_AMULET,			"Amulets" },
    { PS_TYPE_LIGHT, 			"Lights" },
};


/* Categories for sval-dependent squelch. */
static tval_desc tvals[] =
{
    {TYPE_AMMO, 	"Missiles"},
    {TYPE_BOW, 		"Missile Launchers"},
    {TYPE_WEAPON1, 	"Weapons (Swords)"},
    {TYPE_WEAPON2, 	"Weapons (Non Swords)"},
    {TYPE_BODY, 	"Body Armor"},
    {TYPE_CLOAK, 	"Cloaks"},
    {TYPE_SHIELD, 	"Shields"},
    {TYPE_HELM, 	"Helmets"},
    {TYPE_GLOVES, 	"Gloves"},
    {TYPE_BOOTS, 	"Boots"},
    {TYPE_AMULET, 	"Amulets"},
    {TYPE_RING, 	"Rings"},
    {TYPE_STAFF, 	"Staves"},
    {TYPE_WAND, 	"Wands"},
    {TYPE_ROD, 		"Rods"},
    {TYPE_SCROLL, 	"Scrolls"},
    {TYPE_POTION, 	"Potions"},
    {TYPE_BOOK, 	"Magic Books"},
    {TYPE_FOOD, 	"Food Items"},
    {TYPE_MISC, 	"Miscellaneous"},
};

/*
 * The names for the various kinds of quality
 */
static quality_name_struct quality_values[SQUELCH_MAX] =
{
    { SQUELCH_NONE,		"none" },
    { SQUELCH_CURSED,	"squelch cursed" },
    { SQUELCH_AVERAGE,	"squelch cursed and average" },
    { SQUELCH_GOOD_STRONG,	"squelch good, average, and cursed" },
    { SQUELCH_GOOD_WEAK,	"squelch good pseudo-id, average, and cursed" },
    { SQUELCH_ALL,	"squelch all but artifacts" },
};

/*
 * menu struct for differentiating aware from unaware squelch
 */
typedef struct
{
    s16b idx;
    bool aware;
} squelch_choice;



#endif // SQUELCH_H
