/*
 * File: squelch.c
 * Purpose: Item destruction
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * Diego Gonzalez, David Vestal, Pete Mack, Andrew Sidwell.
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
#include "src/squelch.h"

/* Pseudo flag */
#define RBASE (TR2_RES_FIRE | TR2_RES_COLD | TR2_RES_ELEC | TR2_RES_ACID)

/* The amount of flag fields plus 1 */
#define MAX_GROUPS 5

/*
 * Create a string describing (some of) the known flags of the given object
 * If only_random_powers is TRUE the string will hold only a representation
 * of the random powers of ego-items
 */
QString format_object_flags(object_type *o_ptr, bool only_random_powers)
{
    u16b i;
    QString buf;

    u32b f1 = 0, f2 = 0, f3 = 0, native = 0;

    /* It's easier to handle flags in arrays. See later */
    u32b flags[MAX_GROUPS];

    /*
     * Object flags and their names
     */
    static struct
    {
        u32b flag;
        byte group;	/* The field (1 for flags1, 2 for flags2, 3 for flags3 and 4 for native) */
        QString name;
    } flag_info[] =
    {
        {TR2_IM_ACID,		2,	"ImAcid"},
        {TR2_IM_ELEC,		2,	"ImElec"},
        {TR2_IM_FIRE,		2,	"ImFire"},
        {TR2_IM_COLD,		2,	"ImCold"},
        {TR2_IM_POIS,		2,	"ImPois"},
        {RBASE,			2,	"RBase"},
        {TR2_RES_ACID,		2,	"RAcid"},
        {TR2_RES_ELEC,		2,	"RElec"},
        {TR2_RES_FIRE,		2,	"RFire"},
        {TR2_RES_COLD,		2,	"RCold"},
        {TR2_RES_POIS,		2,	"RPois"},
        {TR2_RES_FEAR,		2,	"RFear"},
        {TR2_RES_LIGHT,		2,	"RLite"},
        {TR2_RES_DARK,		2,	"RDark"},
        {TR2_RES_BLIND,		2,	"RBlind"},
        {TR2_RES_CONFU,		2,	"RConfu"},
        {TR2_RES_SOUND,		2,	"RSound"},
        {TR2_RES_SHARD,		2,	"RShard"},
        {TR2_RES_NEXUS,		2,	"RNexus"},
        {TR2_RES_NETHR,		2,	"RNethr"},
        {TR2_RES_CHAOS,		2,	"RChaos"},
        {TR2_RES_DISEN,		2,	"RDisen"},
        {TR3_SLOW_DIGEST,	3,	"S.Dig"},
        {TR3_FEATHER,		3,	"Feath"},
        {TR3_LIGHT,		3,	"PLite"},
        {TR3_REGEN,		3,	"Regen"},
        {TR3_TELEPATHY,		3,	"Telep"},
        {TR3_SEE_INVIS,		3,	"Invis"},
        {TR3_FREE_ACT,		3,	"FrAct"},
        {TR3_HOLD_LIFE,		3,	"HLife"},
        {TR1_STR,		1,	"STR"},
        {TR1_INT,		1,	"INT"},
        {TR1_WIS,		1,	"WIS"},
        {TR1_DEX,		1,	"DEX"},
        {TR1_CON,		1,	"CON"},
        {TR1_CHR,		1,	"CHR"},
        {TR2_SUST_STR,		2,	"SustSTR"},
        {TR2_SUST_INT,		2,	"SustINT"},
        {TR2_SUST_WIS,		2,	"SustWIS"},
        {TR2_SUST_DEX,		2,	"SustDEX"},
        {TR2_SUST_CON,		2,	"SustCON"},
        {TR2_SUST_CHR,		2,	"SustCHR"},
        {TR1_BRAND_ACID,	1,	"BrandAcid"},
        {TR1_BRAND_ELEC,	1,	"BrandElec"},
        {TR1_BRAND_FIRE,	1,	"BrandFire"},
        {TR1_BRAND_COLD,	1,	"BrandCold"},
        {TR1_BRAND_POIS,	1,	"BrandPois"},
        {TR1_SLAY_ANIMAL,	1,	"SlayAnimal"},
        {TR1_SLAY_EVIL,		1,	"SlayEvil"},
        {TR1_SLAY_UNDEAD,	1,	"SlayUndead"},
        {TR1_SLAY_DEMON,	1,	"SlayDemon"},
        {TR1_SLAY_ORC,		1,	"SlayOrc"},
        {TR1_SLAY_TROLL,	1,	"SlayTroll"},
        {TR1_SLAY_GIANT,	1,	"SlayGiant"},
        {TR1_SLAY_DRAGON,	1,	"SlayDragon"},
        {TR1_KILL_DRAGON,	1,	"KillDragon"},
        {TR1_KILL_DEMON,	1,	"KillDemon"},
        {TR1_KILL_UNDEAD,	1,	"KillUndead"},
        {TN1_NATIVE_LAVA,	4,	"NLava"},
        {TN1_NATIVE_ICE,	4,	"NIce"},
        {TN1_NATIVE_OIL,	4,	"NOil"},
        {TN1_NATIVE_FIRE,	4,	"NFire"},
        {TN1_NATIVE_SAND,	4,	"NSand"},
        {TN1_NATIVE_FOREST,	4,	"NForest"},
        {TN1_NATIVE_WATER,	4,	"NWater"},
        {TN1_NATIVE_ACID,	4,	"NAcid"},
        {TN1_NATIVE_MUD,	4,	"NMud"},
    };

    /* Get the known flags */
    object_flags_known(o_ptr, &f1, &f2, &f3, &native);

    /* Remove all the fixed flags if requested */
    if (only_random_powers)
    {
        /* Get the object kind */
        object_kind *k_ptr = &k_info[o_ptr->k_idx];

        /* Remove the flags */
        f1 &= ~(k_ptr->k_flags1);
        f2 &= ~(k_ptr->k_flags2);
        f3 &= ~(k_ptr->k_flags3);
        native &= ~(k_ptr->k_native);

        /* It's an artifact */
        if (o_ptr->art_num)
        {
            /* Get the artifact template */
            artifact_type *a_ptr = &a_info[o_ptr->art_num];

            /* Remove the flags */
            f1 &= ~(a_ptr->a_flags1);
            f2 &= ~(a_ptr->a_flags2);
            f3 &= ~(a_ptr->a_flags3);
            native &= ~(a_ptr->a_native);
        }

        /* It's an ego-item */
        if (o_ptr->ego_num)
        {
            /* Get the ego-item template */
            ego_item_type *e_ptr = &e_info[o_ptr->ego_num];

            /* Remove the flags */
            f1 &= ~(e_ptr->e_flags1);
            f2 &= ~(e_ptr->e_flags2);
            f3 &= ~(e_ptr->e_flags3);
            native &= ~(e_ptr->e_native);
        }
    }

    /* Make an array of flags with the individual flags */
    flags[0] = 0;
    flags[1] = f1;
    flags[2] = f2;
    flags[3] = f3;
    flags[4] = native;


    /* Start with an empty string */
    buf.clear();

    /* Scan the flags */
    for (i = 0; i < N_ELEMENTS(flag_info); i++)
    {
        /* Check presence of the flag */
        if ((flags[flag_info[i].group] & flag_info[i].flag) ==
            flag_info[i].flag)
        {
            /* Append a space if it's not the first flag we found */
            if (!buf.isNull())
            {
                buf.append(' ');
            }

            /* Append the flag name */
            buf.append(flag_info[i].name);

            /* Special cases in flags2 */
            if (flag_info[i].group == 2)
            {
                /* Analyze flag */
                switch (flag_info[i].flag)
                {
                    /* Immunities remove the respective resistances */
                    case TR2_IM_ACID: flags[2] &= ~(TR2_RES_ACID); break;
                    case TR2_IM_FIRE: flags[2] &= ~(TR2_RES_FIRE); break;
                    case TR2_IM_COLD: flags[2] &= ~(TR2_RES_COLD); break;
                    case TR2_IM_ELEC: flags[2] &= ~(TR2_RES_ELEC); break;
                    case TR2_IM_POIS: flags[2] &= ~(TR2_RES_POIS); break;
                    /* Special flag, RBASE, remove individual resistances */
                    case RBASE: flags[2] &= ~(RBASE); break;
                }
            }
        }
    }

    return (buf);
}


/*
 * Put a copy of "src" into "dest". Expand a replaces some special patterns found in "src"
 * with short descriptions of the known object flags. These patterns are:
 * :all:
 * 	Put all the known flags
 * :random:
 *	Put only the known random powers of ego-items
 */
static QString expand_inscription(object_type *o_ptr)
{
    QString dest;
    QString src = o_ptr->inscription;

    dest.clear();

    if (!src.contains(':'))
    /* Regular characters */
    {
        /* Just copy them */
        dest = src;
    }
    /* Found the start of a pattern */
    else while (src.contains(':'))
    {
        QString temp;
        /* The supported patterns */
        QString pattern_list[2] = {":all:", ":random:"};
        /* Info of each pattern */
        bool mode_list[] = {FALSE, TRUE};
        int found = -1;
        u16b k;

        /* Scan the pattern list */
        for (k = 0; (k < N_ELEMENTS(pattern_list)) && (found == -1); k++)
        {
            /* Is it this pattern? */
            if (src.contains(pattern_list[k]))
            {
                /* Yes */
                found = k;
            }
        }

        /* Found a pattern match */
        if (found != -1)
        {
            /* Get the flag text. Note use of the mode list */
            dest.append(format_object_flags(o_ptr, mode_list[found]));
        }

        src[src.indexOf(':')] = ' ';
    }

    return (dest);
}



/*
 * Returns the current autoinscription.
 */
QString get_autoinscription(s16b kindIdx)
{

    object_kind *k_ptr = &k_info[kindIdx];

    if (k_ptr->autoinscribe.isEmpty()) return (NULL);

    return (k_ptr->autoinscribe);
}

/*Put the autoinscription on an object*/
void apply_autoinscription(object_type *o_ptr)
{
    QString o_name;
    QString note = get_autoinscription(o_ptr->k_idx);

    /* Don't inscribe unaware objects */
    if (!o_ptr->is_aware())
    {
        return;
    }

    /* Bugfix - Don't replace existing inscriptions */
    if (!o_ptr->inscription.isEmpty())
    {
        return;
    }

    /* We have an autoinscription */
    if (!note.isEmpty())
    {
        /* Process special text patterns */
        note.append(expand_inscription(o_ptr));
    }
    /* Check the addition of smart autoinscriptions for ego-items */
    else if (o_ptr->is_ego_item() && o_ptr->is_known() &&
        (k_info[o_ptr->k_idx].squelch != SQUELCH_ALWAYS) &&
        (squelch_itemp(o_ptr, 0, TRUE) != SQUELCH_YES))
    {
        /* Describe the random powers */
        note = format_object_flags(o_ptr, TRUE);

    }

    /* No autoinscription. Done */
    if (note.isEmpty()) return;

    /* Get the object name before adding the inscription */
    o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Assign the new inscription */
    o_ptr->inscription = note;

    /* Show a message */
    message(QString("You autoinscribe %1.") .arg(o_name));

    return;
}


void autoinscribe_ground(void)
{
    int py = p_ptr->py;
    int px = p_ptr->px;
    s16b this_o_idx, next_o_idx = 0;

    /* Scan the pile of objects */
    for (this_o_idx = dungeon_info[py][px].object_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        /* Get the next object */
        next_o_idx = o_list[this_o_idx].next_o_idx;

        /* Apply an autoinscription */
        apply_autoinscription(&o_list[this_o_idx]);
    }
}

void autoinscribe_pack(void)
{
    int i;

    for (i = INVEN_PACK; i > 0; i--)
    {
        /* Skip empty items */
        if(!inventory[i].k_idx) continue;

        apply_autoinscription(&inventory[i]);
    }
}

/*
 * Find the squelch type of the object, or TYPE_MAX if none
 */
static squelch_type_t squelch_type_of(object_type *o_ptr)
{
    size_t i;

    /* Find the appropriate squelch group */
    for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
    {
        if ((quality_mapping[i].tval == o_ptr->tval) &&
            (quality_mapping[i].min_sval <= o_ptr->sval) &&
            (quality_mapping[i].max_sval >= o_ptr->sval))
            return quality_mapping[i].squelch_type;
    }

    return PS_TYPE_MAX;
}

/* Convert the values returned by squelch_itemp to string */
QString squelch_to_label(int squelch)
{
    if (squelch == SQUELCH_YES) return ("(Squelched)");

    if (squelch == SQUELCH_FAILED) return ("(Squelch Failed)");

    return ("");
}


/*
 * Determines if an object is going to be squelched on identification.
 * Input:
 *  o_ptr   : This is a pointer to the object type being identified.
 *  feeling : This is the feeling of the object if it is being
 *            pseudoidentified or 0 if the object is being identified.
 *  fullid  : Is the object is being identified?
 *
 * Output: One of the three above values.
 */

int squelch_itemp(object_type *o_ptr, byte feelings, bool fullid)
{
    int num, result;
    byte feel;

    /* Default */
    result = SQUELCH_NO;

    /* Never squelch quest items */
    if (o_ptr->ident & IDENT_QUEST) return result;

    /* Squelch some ego items if known */
    if (fullid && (o_ptr->is_ego_item()) && (e_info[o_ptr->ego_num].squelch))
    {
        /* Squelch fails on inscribed objects */
        return ((!o_ptr->inscription.isEmpty()) ? SQUELCH_FAILED: SQUELCH_YES);
    }

    /* Check to see if the object is eligible for squelching on id. */
    num = squelch_type_of(o_ptr);

    /* Never squelched */
    if (num == PS_TYPE_MAX) return result;

    /*
     * Get the "feeling" of the object.  If the object is being identified
     * get the feeling returned by a heavy pseudoid.
     */
    feel = feelings;

    /* Handle fully identified objects */
    if (fullid)  feel = o_ptr->pseudo_heavy();

    /* Get result based on the feeling and the squelch_level */
    switch (squelch_level[num])
    {
        case SQUELCH_NONE:
        {
            return result;
        }

        case SQUELCH_CURSED:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_AVERAGE:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_GOOD_STRONG:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE) ||
                (feel==INSCRIP_GOOD_STRONG)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_GOOD_WEAK:
        {
            result = (((feel==INSCRIP_BROKEN) ||
                (feel==INSCRIP_TERRIBLE) ||
                (feel==INSCRIP_WORTHLESS) ||
                (feel==INSCRIP_CURSED) ||
                (feel==INSCRIP_AVERAGE) ||
                (feel==INSCRIP_GOOD_STRONG) ||
                (feel==INSCRIP_GOOD_WEAK)) ? SQUELCH_YES : SQUELCH_NO);
            break;
        }

        case SQUELCH_ALL:
        {
            result = SQUELCH_YES;
            break;
        }
    }

    /* Squelching will fail on an artifact or inscribed object */
    if ((result == SQUELCH_YES) && (o_ptr->is_artifact() || !o_ptr->inscription.isEmpty())) result = SQUELCH_FAILED;

    return result;
}

void rearrange_stack(int y, int x)
{
    s16b o_idx, next_o_idx;
    s16b first_bad_idx, first_good_idx, cur_bad_idx, cur_good_idx;

    object_type *o_ptr;

    bool sq_flag = FALSE;

    /* Initialize */
    first_bad_idx = 0;
    first_good_idx = 0;
    cur_bad_idx = 0;
    cur_good_idx = 0;

    /*go through all the objects*/
    for(o_idx = dungeon_info[y][x].object_idx; o_idx; o_idx = next_o_idx)
    {
        o_ptr = &(o_list[o_idx]);
        next_o_idx = o_ptr->next_o_idx;

        /*is it marked for squelching*/
        sq_flag = ((k_info[o_ptr->k_idx].squelch == SQUELCH_ALWAYS) &&
            (k_info[o_ptr->k_idx].aware));

        if (sq_flag)
        {
            if (first_bad_idx == 0)
            {
                first_bad_idx = o_idx;
                cur_bad_idx = o_idx;
            }

            else
            {
                o_list[cur_bad_idx].next_o_idx = o_idx;
                cur_bad_idx = o_idx;
            }
        }

        else

        {
            if (first_good_idx==0)
            {
                first_good_idx = o_idx;
                cur_good_idx = o_idx;
            }

            else
            {
                o_list[cur_good_idx].next_o_idx = o_idx;
                cur_good_idx = o_idx;
            }
        }
    }

    if (first_good_idx != 0)
    {
        dungeon_info[y][x].object_idx = first_good_idx;
        o_list[cur_good_idx].next_o_idx = first_bad_idx;
        o_list[cur_bad_idx].next_o_idx = 0;
    }

    else
    {
        dungeon_info[y][x].object_idx = first_bad_idx;
    }
}

/*
 * This performs the squelch, actually removing the item from the
 * game.  It returns 1 if the item was squelched, and 0 otherwise.
 * This return value is never actually used.
 */
int do_squelch_item(int squelch, int item, object_type *o_ptr)
{
    if (squelch != SQUELCH_YES) return 0;

    /*hack - never squelch quest items*/
    if (o_ptr->ident & IDENT_QUEST) return 0;

    if (item >= 0)
    {
        inven_item_increase(item, -o_ptr->number);
        inven_item_optimize(item);
    }

    else
    {
        floor_item_increase(0 - item, -o_ptr->number);
        floor_item_optimize(0 - item);
    }

    return 1;
}

bool squelch_item_ok(object_type *o_ptr)
{
    object_kind *k_ptr = k_ptr = &k_info[o_ptr->k_idx];

    /* Always delete "nothings" */
    if (!o_ptr->k_idx) return (TRUE);

    /* Ignore inscribed objects, artifacts , mimics or quest objects */
    if ((!o_ptr->inscription.isEmpty()) || (o_ptr->is_artifact()) || (o_ptr->ident & (IDENT_QUEST)) ||
        (o_ptr->mimic_r_idx))
    {
        return (FALSE);
    }

    /* Object kind is set to be always squelched */
    if ((k_ptr->squelch == SQUELCH_ALWAYS) && k_ptr->aware) return (TRUE);

    /* Apply quality squelch if possible */
    if (object_known_p(o_ptr) && (squelch_itemp(o_ptr, 0, TRUE) == SQUELCH_YES)) return TRUE;

    /* Don't squelch */
    return (FALSE);

}

/* Attempt to squelch every object in a pile. */
void do_squelch_pile(int y, int x)
{
    s16b o_idx, next_o_idx;
    object_type *o_ptr;

    for(o_idx = dungeon_info[y][x].object_idx; o_idx; o_idx = next_o_idx)
    {
        /* Get the object */
        o_ptr = &(o_list[o_idx]);

        /* Point to the next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Destroy the object? */
        if (squelch_item_ok(o_ptr))
        {
            /* Delete */
            delete_object_idx(o_idx);
        }
    }
}
