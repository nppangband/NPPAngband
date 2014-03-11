/* File: was cmd2.c */

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
 * Check if action permissible here.
 */
bool do_cmd_test(int y, int x, int action, bool do_message)
{
    u32b bitzero = 0x01;
    u32b flag;

    QString act = "";

    QString here = ((p_ptr->px == x ) && (p_ptr->py == y)) ? "here": "there";

    feature_type *f_ptr;

    /* Must have knowledge */
    if (!(dungeon_info[y][x].cave_info & (CAVE_MARK)))
    {
        /* Message */
        if (do_message) message(QString("You see nothing %1.") .arg(here));

        /* Nope */
        return (FALSE);
    }

    /* Get memorised feature */
    f_ptr = &f_info[dungeon_info[y][x].feat];

    switch (action)
    {
        case FS_SECRET:							break;
        case FS_OPEN:	act = " to open";		break;
        case FS_CLOSE:	act = " to close";		break;
        case FS_BASH:	act = " to bash";		break;
        case FS_SPIKE:	act = " to spike";		break;
        case FS_TUNNEL:	act = " to tunnel";		break;
        case FS_FLOOR:	act = " to set a trap on";	break;
        default: break;
    }

    if (action < FS_FLAGS2)
    {
        flag = bitzero << (action - FS_FLAGS1);
        if (!(f_ptr->f_flags1 & flag))
        {
            if (do_message) message(QString("You see nothing %1%2.") .arg(here) .arg(act));
            return (FALSE);
        }
    }

    else if (action < FS_FLAGS3)
    {
        flag = bitzero << (action - FS_FLAGS2);
        if (!(f_ptr->f_flags2 & flag))
        {
            if (do_message) message(QString("You see nothing %1%2.") .arg(here) .arg(act));
            return (FALSE);
        }
    }

    else if (action < FS_FLAGS_END)
    {
        flag = bitzero << (action - FS_FLAGS3);
        if (!(f_ptr->f_flags2 & flag))
        {
            if (do_message) message(QString("You see nothing %1%2.") .arg(here) .arg(act));
            return (FALSE);
        }
    }

    return (TRUE);
}


/*
 * Go up one level
 */
void do_cmd_go_up(void)
{
    QString out_val;
    byte quest;

    int decrease = 0;

    // Paranoia
    if (!p_ptr->playing) return;

    feature_type *f_ptr= &f_info[dungeon_info[p_ptr->py][p_ptr->px].feat];

    /* Verify stairs */
    if (!cave_up_stairs(p_ptr->py, p_ptr->px))
    {
        message(QString("I see no up staircase here."));

        return;
    }

    /* Ironman */
    if (adult_ironman)
    {
        message(QString("Nothing happens!"));
        return;
    }

    /* Verify leaving normal quest level */
    if ((verify_leave_quest) && quest_might_fail_if_leave_level())
    {
        out_val = "Really risk failing your quest? ";
        if (!get_check(out_val)) return;
    }

    /* Verify leaving normal quest level */
    if ((verify_leave_quest) && quest_shall_fail_if_leave_level())
    {
        out_val = "Really fail your quest? ";
        if (!get_check(out_val)) return;
    }

    /* Success */
    message(QString("You enter a maze of up staircases."));
    if (game_mode == GAME_NPPMORIA) message(QString("You pass through a one-way door."));

    /* Create a way back */
    if (adult_connected_stairs) p_ptr->create_stair = FEAT_MORE;

    /* New depth */
    decrease++;

    /*find out of entering a quest level (unusual going up)*/
    quest = quest_check(p_ptr->depth);

    /*go up another level if it is a shaft*/
    if ((f_ptr->f_flags2 & (FF2_SHAFT)) &&
        (!quest) && (p_ptr->depth > 0))
    {
        decrease++;

        /* Create a way back (usually) */
        if (adult_connected_stairs) p_ptr->create_stair = FEAT_MORE_SHAFT;
    }

    /* Change level */
    dungeon_change_level(p_ptr->depth - decrease);

    process_player_energy(BASE_ENERGY_MOVE);
}


/*
 * Go down one level
 */
void do_cmd_go_down(void)
{
    byte quest;
    QString out_val;

    int increase = 0;

    // Paranoia
    if (!p_ptr->playing) return;

    feature_type *f_ptr= &f_info[dungeon_info[p_ptr->py][p_ptr->px].feat];

    /*find out if entering a quest level*/
    quest = quest_check(p_ptr->depth);

    /* Verify stairs */
    if (!cave_down_stairs(p_ptr->py, p_ptr->px))
    {
        message(QString("I see no down staircase here."));
        return;
    }

    /* Verify leaving normal quest level */
    if ((verify_leave_quest) && quest_might_fail_if_leave_level())
    {
        out_val = "Really risk failing your quest? ";
        if (!get_check(out_val)) return;
    }

    /* Verify leaving normal quest level */
    if ((verify_leave_quest) && quest_shall_fail_if_leave_level())
    {
        out_val = "Really fail your quest? ";
        if (!get_check(out_val)) return;
    }

    /* Success */
    message(QString("You enter a maze of down staircases."));
    if (game_mode == GAME_NPPMORIA) message(QString("You pass through a one-way door."));

    /* Create a way back (usually) */
    if (adult_connected_stairs) p_ptr->create_stair = FEAT_LESS;

    /* New level */
    increase++;

    /*find out if entering a quest level*/
    quest = quest_check(p_ptr->depth);

    /* Go down a shaft if allowed */
    if ((f_ptr->f_flags2 & (FF2_SHAFT)) &&
        (!quest) && (p_ptr->depth < MAX_DEPTH - 1))
    {
        increase++;

        /* Create a way back (usually) */
        if (adult_connected_stairs) p_ptr->create_stair = FEAT_LESS_SHAFT;
    }

    /* Change level */
    dungeon_change_level(p_ptr->depth + increase);

    process_player_energy(BASE_ENERGY_MOVE);
}
