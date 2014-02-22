/* File: cmd3.cpp */

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
 * create a monster trap, currently used from a scroll
 */
bool make_monster_trap(void)
{
    int y, x, dir;

    /* Get a direction */
    // TODO if (!get_rep_dir(&dir)) return (FALSE);

    /* Get location */
    y = p_ptr->py + ddy[dir];
    x = p_ptr->px + ddx[dir];

    /*check for an empty floor, monsters, and objects*/
    if (!cave_trappable_bold(y, x) || dungeon_info[y][x].monster_idx)
    {
        message(QString("You can only make a trap on an empty floor space."));

        return(FALSE);
    }

    /*set the trap*/
    py_set_trap(y, x);

    return(TRUE);
}



/*
 * Rogues may set traps, or they can be set from a scroll.
 * Only one such trap may exist at any one time,
 * but an old trap can be disarmed to free up equipment for a new trap.
 * -LM-
 */
void py_set_trap(int y, int x)
{

    if (p_ptr->timed[TMD_BLIND] || no_light())
    {
        message(QString("You can not see to set a trap."));
        return;
    }

    if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE])
    {
        message(QString("You are too confused."));
        return;
    }

    /* Set the trap, and draw it. */
    set_effect_trap_player(FEAT_MTRAP_BASE, y, x);

    /* Remember the location */
    dungeon_info[y][x].cave_info |= (CAVE_MARK);

    /* Notify the player. */
    message(QString("You set a monster trap."));

    light_spot(y, x);

    /*make the monsters who saw wary*/
    (void)project_los(y, x, 0, GF_MAKE_WARY);

    /* Increment the number of monster traps. */
    num_trap_on_level++;

}


/*
 * Choose advanced monster trap type
 */
static bool choose_mtrap(int *choice)
{
    int num;

    char c;

    bool done=FALSE;

    /* TODO select the mtrap

    prt("        Choose an advanced monster trap (ESC to cancel):", 1, 8);

    num = 1 + (p_ptr->lev / 6);

                  prt("    a) Sturdy Trap          - (less likely to break)", 2, 8);
    if (num >= 2) prt("    b) Slowing Trap         - (slows monsters)", 3, 8);
    if (num >= 3) prt("    c) Confusion Trap       - (confuses monsters)", 4, 8);
    if (num >= 4) prt("    d) Poison Gas Trap      - (creates a toxic cloud)", 5, 8);
    if (num >= 5) prt("    e) Life Draining Trap   - (Hurts living monsters)", 6, 8);
    if (num >= 6) prt("    f) Lightning Trap       - (shoots a lightning bolt)", 7, 8);
    if (num >= 7) prt("    g) Explosive Trap       - (causes area damage)", 8, 8);
    if (num >= 8) prt("    h) Portal Trap          - (teleports monsters)", 9, 8);
    if (num >= 9) prt("    i) Dispel Monsters Trap - (hurt all monsters in area)", 10, 8);

    */



    /* Return */
    return (TRUE);
}


/*
 * Turn a basic monster trap into an advanced one -BR-
 */
bool py_modify_trap(int y, int x)
{
    int trap_choice = 0;

    if (p_ptr->timed[TMD_BLIND] || no_light())
    {
        message(QString("You can not see to modify your trap."));
        return (FALSE);
    }

    /*no modifying traps on top of monsters*/
    if (dungeon_info[y][x].monster_idx > 0)
    {
        message(QString("There is a creature in the way."));
        return (FALSE);
    }

    if (!(choose_mtrap(&trap_choice))) return (FALSE);

    /* Set the trap, and draw it. */
    x_list[dungeon_info[y][x].effect_idx].x_f_idx = FEAT_MTRAP_BASE + 1 + trap_choice;

    /*check if player did not modify trap*/
    if (x_list[dungeon_info[y][x].effect_idx].x_f_idx == FEAT_MTRAP_BASE) return(FALSE);

    /*Mark it*/
    light_spot(y,x);

    /* Notify the player. */
    message(QString("You modify the monster trap."));

    /*make the monsters who saw wary*/
    (void)project_los(y, x, 0, GF_MAKE_WARY);

    return (TRUE);
}

