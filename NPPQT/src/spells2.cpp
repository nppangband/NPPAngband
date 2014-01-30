/* File: spells2.c */

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
 * Increase players hit points, notice effects
 */
bool hp_player(int num)
{
    /* Healing needed */
    if (p_ptr->chp < p_ptr->mhp)
    {
        /* Gain hitpoints */
        p_ptr->chp += num;

        /* Enforce maximum */
        if (p_ptr->chp >= p_ptr->mhp)
        {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;
        }

        /* Redraw */
        p_ptr->redraw |= (PR_HP);

        /* Heal 0-4 */
        if (num < 5)
        {
            message("You feel a little better.");
        }

        /* Heal 5-14 */
        else if (num < 15)
        {
            message("You feel better.");
        }

        /* Heal 15-34 */
        else if (num < 35)
        {
            message("You feel much better.");
        }

        /* Heal 35+ */
        else
        {
            message("You feel very good.");
        }

        /* Notice */
        return (TRUE);
    }

    /* Ignore */
    return (FALSE);
}

/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "light_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_light(void)
{
    int i;


    /* Apply flag changes */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* No longer in the array */
        dungeon_info[y][x].cave_info &= ~(CAVE_TEMP);

        /* Perma-Lite */
        dungeon_info[y][x].cave_info |= (CAVE_GLOW);
    }

    /* Fully update the visuals */
    p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

    /* Update stuff */
    // TODO update_stuff();

    /* Process the grids */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* Redraw the grid */
        light_spot(y, x);

        /* Process affected monsters */
        if (dungeon_info[y][x].monster_idx > 0)
        {
            int chance = 25;

            int m_idx = dungeon_info[y][x].monster_idx;
            monster_type *m_ptr = &mon_list[m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Stupid monsters rarely wake up */
            if (r_ptr->flags2 & (RF2_STUPID)) chance = 10;

            /* Smart monsters always wake up */
            if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

            /* Sometimes monsters wake up */
            if ((m_ptr->m_timed[MON_TMD_SLEEP]) && (rand_int(100) < chance))
            {
                /* Wake up! */
                // TODO    wake_monster_attack(m_ptr, MON_TMD_FLG_NOTIFY);

                /*possibly update the monster health bar*/
                if ((p_ptr->health_who == m_idx)  || (m_ptr->sidebar))
                    p_ptr->redraw |= (PR_HEALTH);
            }
        }

    }

    /* None left */
    temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlight_room()"
 */
static void cave_temp_room_unlite(void)
{
    int i;

    /* Apply flag changes */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* No longer in the array */
        dungeon_info[y][x].cave_info &= ~(CAVE_TEMP);

        /* Turn off the light */
        dungeon_info[y][x].cave_info &= ~(CAVE_GLOW);

        /* Hack -- Forget "boring" grids */
        if (!(dungeon_info[y][x].cave_info & (CAVE_HALO)) &&
            !cave_ff1_match(y, x, FF1_REMEMBER) &&
            (!cave_any_trap_bold(y, x) ||
                (x_list[dungeon_info[y][x].effect_idx].x_flags &
                (EF1_HIDDEN))))
        {
            /* Forget the grid */
            dungeon_info[y][x].cave_info &= ~(CAVE_MARK);
        }
    }

    /* Fully update the visuals */
    p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

    /* Update stuff */
    //TODO update_stuff();

    /* Process the grids */
    for (i = 0; i < temp_n; i++)
    {
        int y = temp_y[i];
        int x = temp_x[i];

        /* Redraw the grid */
        light_spot(y, x);


    }

    /* None left */
    temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
    /* Avoid infinite recursion */
    if (dungeon_info[y][x].cave_info & (CAVE_TEMP)) return;

    /* Do not "leave" the current room */
    if (!(dungeon_info[y][x].cave_info & (CAVE_ROOM))) return;

    /* Paranoia -- verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid as "seen" */
    dungeon_info[y][x].cave_info |= (CAVE_TEMP);

    /* Add it to the "seen" set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void light_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get lit, but stop light */
        if (!cave_project_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, lite them all up at once */
    cave_temp_room_light();

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);

}


/*
 * Darken all rooms containing the given location
 */
void unlight_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < temp_n; i++)
    {
        x = temp_x[i], y = temp_y[i];

        /* Walls get dark, but stop darkness */
        if (!cave_project_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, darken them all at once */
    cave_temp_room_unlite();

}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool light_area(int dam, int rad)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    u32b flg = PROJECT_BOOM | PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->timed[TMD_BLIND])
    {
        message("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    // TODO (void)project(SOURCE_PLAYER, rad, py, px, py, px, dam, GF_LIGHT_WEAK, flg, 0, 0);

    /* Lite up the room */
    light_room(py, px);

    /* Assume seen */
    return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlight_area(int dam, int rad)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    u32b flg = PROJECT_BOOM | PROJECT_GRID | PROJECT_KILL;

    /* Hack -- Message */
    if (!p_ptr->timed[TMD_BLIND])
    {
        message("Darkness surrounds you.");
    }

    /* Hook into the "project()" function */
    //TODO (void)project(SOURCE_PLAYER, rad, py, px, py, px, dam, GF_DARK_WEAK, flg, 0, 0);

    /* Lite up the room */
    unlight_room(py, px);

    /* Assume seen */
    return (TRUE);
}

/*
 * Get a spell type (GF_*) for the given terrain feature.
 * The spell type is stored in gf_type.
 * A description of the effect on an object is stored in action.
 * Both gf_type and action can be NULL.
 */
QString get_spell_type_from_feature(int f_idx, int *gf_type)
{
    /* Get the element flags */
    u32b element = feat_ff3_match(f_idx, TERRAIN_MASK);
    u16b i;
    QString desc;


    /*
     * Spell information for each element type.
     */
    static struct
    {
        u32b element;
        int gf_type;
        QString action;
    } spell_info[] =
    {
        {ELEMENT_BWATER, GF_BWATER, "burns"},
        {ELEMENT_BMUD, GF_BMUD, "burns"},
        {ELEMENT_LAVA, GF_LAVA, "burns"},
        {ELEMENT_FIRE, GF_FIRE, "burns"},
        {ELEMENT_ICE, GF_ICE, "freezes"},
        {ELEMENT_WATER, GF_WATER, "hurts"},
        {ELEMENT_SAND, GF_SAND, "hurts"},
        {ELEMENT_ACID, GF_ACID, "hurts"}
    };

    /* Save default spell type */
    *gf_type = GF_MANA;

    /* Save default action */
    desc = ("hurts");

    /* Find the element in the list */
    for (i = 0; i < N_ELEMENTS(spell_info); i++)
    {
        /* Found the element? */
        if (spell_info[i].element == element)
        {
            /* Save the spell type */
            *gf_type = spell_info[i].gf_type;

            /* Save the action */
            desc = (spell_info[i].action);
        }
    }

    return (desc);
}

/*
 * Return TRUE if the player is immune to the effects of the given
 * spell type
 */
bool is_player_immune(int gf_type)
{
    /* Check immunities */
    switch (gf_type)
    {
        case GF_ACID:
        {
            if (p_ptr->state.immune_acid) return (TRUE);

            break;
        }
        case GF_ICE:
        case GF_COLD:
        {
            if (p_ptr->state.immune_cold) return (TRUE);

            break;
        }
        case GF_FIRE:
        {
            if (p_ptr->state.immune_fire) return (TRUE);

            break;
        }
        case GF_ELEC:
        {
            if (p_ptr->state.immune_elec) return (TRUE);

            break;
        }
        case GF_POIS:
        {
            if (p_ptr->state.immune_pois) return (TRUE);

            break;
        }
    }

    return (FALSE);
}
