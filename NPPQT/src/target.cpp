/*
 * File: target.c
 * Purpose: Targeting code
 *
 * Copyright (c) 1997-2007 Angband contributors, Jeff Greene, Diego Gonzalez
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
 * Determine if a trap makes a reasonable target
 */
static bool target_able_trap(int y, int x)
{
    /* Must be on line of fire */
    if (!player_can_fire_bold(y, x)) return (FALSE);

    /* Only player traps allowed. Ignore monster traps and glyphs */
    if (!cave_player_trap_bold(y, x)) return (FALSE);

    /* Ignore hidden traps */
    if (x_list[dungeon_info[y][x].effect_idx].x_flags & (EF1_HIDDEN)) return (FALSE);

    /* Known player traps are okay */
    return (TRUE);
}

/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
bool target_able(int m_idx)
{

    monster_type *m_ptr;

    /* No monster */
    if (m_idx <= 0) return (FALSE);

    /* Get monster */
    m_ptr = &mon_list[m_idx];

    /* Monster must be alive */
    if (!m_ptr->r_idx) return (FALSE);

    /* Monster must be visible */
    if (!m_ptr->ml) return (FALSE);

    /* Monster must be projectable */
    if (!m_ptr->project) return (FALSE);

    /* Walls protect monsters */
    if (!cave_project_bold(m_ptr->fy, m_ptr->fx) &&
        !cave_passable_bold(m_ptr->fy, m_ptr->fx)) return (FALSE);

    /* Hack -- no targeting hallucinations */
    if (p_ptr->timed[TMD_IMAGE]) return (FALSE);

    /* Hack -- Never target trappers XXX XXX XXX */
    /* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

    /* Hidden monsters cannot be targets */
    if (m_ptr->mflag & (MFLAG_HIDE)) return (FALSE);

    /* Assume okay */
    return (TRUE);
}

/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
static bool ang_sort_comp_distance(const void *u, const void *v, int a, int b)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    byte *x = (byte*)(u);
    byte *y = (byte*)(v);

    int da, db, kx, ky;

    /* Absolute distance components */
    kx = x[a]; kx -= px; kx = ABS(kx);
    ky = y[a]; ky -= py; ky = ABS(ky);

    /* Approximate Double Distance to the first point */
    da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

    /* Absolute distance components */
    kx = x[b]; kx -= px; kx = ABS(kx);
    ky = y[b]; ky -= py; ky = ABS(ky);

    /* Approximate Double Distance to the first point */
    db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

    /* Compare the distances */
    return (da <= db);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static void ang_sort_swap_distance(void *u, void *v, int a, int b)
{
    byte *x = (byte*)(u);
    byte *y = (byte*)(v);

    byte temp;

    /* Swap "x" */
    temp = x[a];
    x[a] = x[b];
    x[b] = temp;

    /* Swap "y" */
    temp = y[a];
    y[a] = y[b];
    y[b] = temp;
}

/*
 * Prepare the "temp" array for "target_interactive_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_interactive_prepare(int mode)
{
    int y, x;

    bool expand_look = (mode & (TARGET_LOOK)) ? TRUE : FALSE;

    /* Reset "temp" array */
    clear_temp_array();

    QRect vis = visible_dungeon();

    /* Scan the current panel */
    for (y = vis.y(); y < vis.y() + vis.height(); y++)
    {
        for (x = vis.x(); x < vis.x() + vis.width(); x++)
        {
            bool do_continue = FALSE;

            /* Check overflow */
            if (temp_n >= TEMP_MAX) continue;

            /* Check bounds */
            if (!in_bounds_fully(y, x)) continue;

            /* Require line of sight, unless "look" is "expanded" */
            if (!player_has_los_bold(y, x) && (!expand_look)) continue;

            /* Require "interesting" contents */
            //if (!target_set_interactive_accept(y, x)) continue;

            /* Special mode */
            if (mode & (TARGET_KILL))
            {
                /* Must contain a monster */
                if (!(dungeon_info[y][x].monster_idx > 0)) do_continue = TRUE;

                /* Must be a targettable monster */
                if (!target_able(dungeon_info[y][x].monster_idx)) do_continue = TRUE;
            }

            /* Don't continue on the trap exception, or if probing. */
            if ((mode & (TARGET_TRAP)) && target_able_trap(y, x)) do_continue = FALSE;
            else if (mode & (TARGET_PROBE)) do_continue = FALSE;

            if (do_continue) continue;

            /*
             * Hack - don't go over redundant elemental terrain \
             * (since we have large lakes and pools of the same terrain)
             */
            if ((p_ptr->target_row > 0) || (p_ptr->target_col > 0))
            {
                if (dungeon_info[p_ptr->target_row][p_ptr->target_col].feat == dungeon_info[y][x].feat)
                {
                    if (cave_ff3_match(y, x, TERRAIN_MASK)) continue;
                }
            }

            /* Save the location */
            temp_x[temp_n] = x;
            temp_y[temp_n] = y;
            temp_n++;
        }
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_distance;
    ang_sort_swap = ang_sort_swap_distance;

    /* Sort the positions */
    ang_sort(temp_x, temp_y, temp_n);
}

/*
 * Set the target to a monster (or nobody)
 */
void target_set_monster(int m_idx)
{
    /* Acceptable target */
    if ((m_idx > 0) && target_able(m_idx))
    {
        monster_type *m_ptr = &mon_list[m_idx];

        /* Save target info */
        p_ptr->target_set = TRUE;
        p_ptr->target_who = m_idx;
        p_ptr->target_row = m_ptr->fy;
        p_ptr->target_col = m_ptr->fx;
    }

    /* Clear target */
    else
    {
        /* Reset target info */
        p_ptr->target_set = FALSE;
        p_ptr->target_who = 0;
        p_ptr->target_row = 0;
        p_ptr->target_col = 0;
    }

    p_ptr->redraw |= (PR_HEALTH);
}


/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool target_okay(void)
{
    /* No target */
    if (!p_ptr->target_set) return (FALSE);

    /* Accept "location" targets */
    if (p_ptr->target_who == 0) return (TRUE);

    /* Check "monster" targets */
    if (p_ptr->target_who > 0)
    {
        int m_idx = p_ptr->target_who;

        /* Accept reasonable targets */
        if (target_able(m_idx))
        {
            monster_type *m_ptr = &mon_list[m_idx];

            /* Get the monster location */
            p_ptr->target_row = m_ptr->fy;
            p_ptr->target_col = m_ptr->fx;

            /* Good target */
            return (TRUE);
        }
    }

    /* Assume no target */
    return (FALSE);
}



/*
 * Set the target to a location
 */
void target_set_location(int y, int x)
{
    /* Legal target */
    if (in_bounds_fully(y, x))
    {
        /* Save target info */
        p_ptr->target_set = TRUE;
        p_ptr->target_who = 0;
        p_ptr->target_row = y;
        p_ptr->target_col = x;
    }

    /* Clear target */
    else
    {
        /* Reset target info */
        p_ptr->target_set = FALSE;
        p_ptr->target_who = 0;
        p_ptr->target_row = 0;
        p_ptr->target_col = 0;
    }

    p_ptr->redraw |= (PR_HEALTH);
}



/*
 * Extract a direction (or zero) from a character
 */
int target_dir(QChar ch)
{
    int d = 0;

    int mode;

    QString act;

    QString s;


    /* Already a direction? */
    if (ch.isDigit())
    {
        d = letter_to_number(ch);
    }
    // TODO handle arrow directions else if (isarrow(ch))
    {

    }
    //else
    {
        // TODO handle keystroke commands
    }

    /* Paranoia */
    if (d == 5) d = 0;

    /* Return direction */
    return (d);
}



int dir_transitions[10][10] =
{
    /* 0-> */ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
    /* 1-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 2-> */ { 0, 0, 2, 0, 1, 0, 3, 0, 5, 0 },
    /* 3-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 4-> */ { 0, 0, 1, 0, 4, 0, 5, 0, 7, 0 },
    /* 5-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 6-> */ { 0, 0, 3, 0, 5, 0, 6, 0, 9, 0 },
    /* 7-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* 8-> */ { 0, 0, 5, 0, 7, 0, 9, 0, 8, 0 },
    /* 9-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};




/*
 * Get an "aiming direction" (1,2,3,4,6,7,8,9 or 5) from the user.
 *
 * Return TRUE if a direction was chosen, otherwise return FALSE.
 *
 * The direction "5" is special, and means "use current target".
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", if it is set.
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Currently this function applies confusion directly.
 */
bool get_aim_dir(int *dp, bool target_trap)
{
    /* Global direction */
    int dir = p_ptr->command_dir;

    QString p;

    /* Initialize */
    (*dp) = 0;

    /* Hack -- auto-target if requested */
    if (use_old_target && target_okay() && !dir) dir = 5;

    /* Ask until satisfied */
    while (!dir)
    {
        /* Choose a prompt */
        if (!target_okay())
            p = "Direction ('*' or <click> to target, 'c' for closest, Escape to cancel)? ";
        else
            p = "Direction ('5' for target, '*' or <click> to re-target, Escape to cancel)? ";

        /* Get a command (or Cancel) */
        UserInput input = ui_get_input();

        // Paranoia
        if (input.mode == INPUT_MODE_NONE) break;

        if (input.key == Qt::Key_Escape) {
            pop_up_message_box("Goint out from targetting");
            break;
        }

        int ke = 0;

        /* Analyze */
        switch (input.key)  // TODO
        {
            /* Mouse aiming */
            case 0: // TODO
            {

                //TODO handle mouseclick target

                break;
            }

            /* Set new target, use target if legal */
            case '*':
            {
                int mode = TARGET_KILL;
                if (target_trap) mode |= TARGET_TRAP;
                // TODO handle targetingif (target_set_interactive(mode, -1, -1)) dir = 5;
                break;
            }

            /* Set to closest target */
            case Qt::Key_C:
            {
                if (target_set_closest(TARGET_KILL)) dir = 5;
                break;
            }

            /* Use current target, if set and legal */
            case 't':
            case '5':
            case '0':
            case '.':
            {
                if (target_okay()) dir = 5;
                break;
            }

            /* Possible direction */
            default:
            {
                int keypresses_handled = 0;
                QChar fake_char = 'c'; //TODO

                while (ke != 0) //TODO
                {
                    int this_dir;

                    /* XXX Ideally show and move the cursor here to indicate
                       the currently "Pending" direction. XXX */
                    this_dir = target_dir(fake_char);

                    if (this_dir)
                    {
                        dir = dir_transitions[dir][this_dir];
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        /* Error */
        if (!dir) pop_up_message_box("Illegal aim direction!");
    }

    /* No direction */
    if (!dir) return (FALSE);

    /* Save the direction */
    p_ptr->command_dir = dir;

    /* Check for confusion */
    if (p_ptr->timed[TMD_CONFUSED])
    {
        /* Random direction */
        dir = ddd[randint0(8)];
    }

    /* Notice confusion */
    if (p_ptr->command_dir != dir)
    {
        /* Warn the user */
        message(QString("You are confused."));
    }

    /* Save direction */
    (*dp) = dir;

    /* A "valid" direction was entered */
    return (TRUE);
}


bool target_set_closest(int mode)
{
    int y, x, m_idx;
    monster_type *m_ptr;
    QString m_name;
    bool visibility;

    /* Cancel old target */
    target_set_monster(0);

    /* Get ready to do targetting */
    target_set_interactive_prepare(mode);

    /* If nothing was prepared, then return */
    if (temp_n < 1)
    {
        message(QString("No Available Target."));
        return FALSE;
    }

    /* Find the first monster in the queue */
    y = temp_y[0];
    x = temp_x[0];
    m_idx = dungeon_info[y][x].monster_idx;

    /* Target the monster, if possible */
    if ((m_idx <= 0) || !target_able(m_idx))
    {
        message(QString("No Available Target."));
        return FALSE;
    }

    /* Target the monster */
    m_ptr = &mon_list[m_idx];
    m_name = monster_desc(m_ptr, 0x00);
    if (!(mode & TARGET_QUIET))
        message(QString("%^1 is targeted.") .arg(m_name));

    /* Set up target inQStringion */
    monster_race_track(m_ptr->r_idx);
    // TODO health_track(cave_m_idx[y][x]);
    target_set_monster(m_idx);

    /* Visual cue */
    // TODO draw it


    return TRUE;
}
