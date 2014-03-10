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

#include <QObject>

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
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_interactive_accept(int y, int x)
{
    object_type *o_ptr;

    /* Player grids are always interesting */
    if (dungeon_info[y][x].monster_idx < 0) return (TRUE);

    /* Handle hallucination */
    if (p_ptr->timed[TMD_IMAGE]) return (FALSE);

    /* Visible monsters */
    if (dungeon_info[y][x].monster_idx > 0)
    {
        monster_type *m_ptr = &mon_list[dungeon_info[y][x].monster_idx];

        /* Visible monsters */
        if (m_ptr->ml) return (TRUE);
    }

    /* Scan all objects in the grid */
    for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
    {
        /* Memorized object */
        if (o_ptr->marked) return (TRUE);
    }

    /* Interesting memorized features */
    /* Ignore unknown features */
    if (!(dungeon_info[y][x].cave_info & (CAVE_MARK))) return (FALSE);

    /* Find interesting effects */
    if (dungeon_info[y][x].effect_idx > 0)
    {
        /* Get the first effect */
        u16b x_idx = dungeon_info[y][x].effect_idx;

        /* Scan the effects on that grid */
        while (x_idx)
        {
            /* Get the effect data */
            effect_type *x_ptr = &x_list[x_idx];

            /* Point to the next effect */
            x_idx = x_ptr->next_x_idx;

            /* Ignore hidden effects */
            if (!(x_ptr->x_f_idx) ||
                (x_ptr->x_flags & (EF1_HIDDEN))) continue;

            /* We have an interesting effect */
            return (TRUE);
        }
    }

    /* Check grid type with dungeon capabilities */
    return ((*dun_cap->can_target_feature)(dungeon_info[y][x].feat));
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
            if (!target_set_interactive_accept(y, x)) continue;

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
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(int y1, int x1, int dy, int dx)
{
    int i, v;

    int x2, y2, x3, y3, x4, y4;

    int b_i = -1, b_v = 9999;

    /* Scan the locations */
    for (i = 0; i < temp_n; i++)
    {
        /* Point 2 */
        x2 = temp_x[i];
        y2 = temp_y[i];

        /* Directed distance */
        x3 = (x2 - x1);
        y3 = (y2 - y1);

        /* Verify quadrant */
        if (dx && (x3 * dx <= 0)) continue;
        if (dy && (y3 * dy <= 0)) continue;

        /* Absolute distance */
        x4 = ABS(x3);
        y4 = ABS(y3);

        /* Verify quadrant */
        if (dy && !dx && (x4 > y4)) continue;
        if (dx && !dy && (y4 > x4)) continue;

        /* Approximate Double Distance */
        v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

        /* Penalize location XXX XXX XXX */

        /* Track best */
        if ((b_i >= 0) && (v >= b_v)) continue;

        /* Track best */
        b_i = i; b_v = v;
    }

    /* Result */
    return (b_i);
}

/*
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_aim_dir()".
 *
 * Currently, when "flag" is true, that is, when
 * "interesting" grids are being used, and a directional key is used, we
 * only scroll by a single panel, in the direction requested, and check
 * for any interesting grids on that panel.  The "correct" solution would
 * actually involve scanning a larger set of grids, including ones in
 * panels which are adjacent to the one currently scanned, but this is
 * overkill for this function.  XXX XXX
 *
 * Hack -- targetting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 *
 * 'mode' is one of TARGET_LOOK or TARGET_KILL.
 * 'x' and 'y' are the initial position of the target to be highlighted,
 * or -1 if no location is specified.
 * Returns TRUE if a target has been successfully set, FALSE otherwise.
 */
bool target_set_interactive(int mode, int x, int y)
{
    int py = p_ptr->py;
    int px = p_ptr->px;

    int i, d, m, t, bd;
    int wid, hgt, help_prompt_loc;

    bool done = FALSE;
    bool flag = TRUE;
    bool help = FALSE;
    bool list_floor_objects = auto_display_lists;

    u16b path_n;
    u16b path_g[PATH_SIZE];
    u16b path_gx[PATH_SIZE];

    char info[80];

    /* These are used for displaying the path to the target */
    char path_char[MAX_RANGE];
    byte path_attr[MAX_RANGE];

    /* Temporarily turn off animate_flicker, must be re-set before exiting the function  */
    //bool temp_animate_flicker = animate_flicker;
    //animate_flicker = FALSE;

    color_message("Entering interactive mode", TERM_SKY_BLUE);

    /* If we haven't been given an initial location, start on the
       player. */
    if (x == -1 || y == -1)
    {
        x = p_ptr->px;
        y = p_ptr->py;
    }
    /* If we /have/ been given an initial location, make sure we
       honour it by going into "free targeting" mode. */
    else
    {
        flag = FALSE;
    }


    /* Cancel target */
    target_set_monster(0);

    /* health_track(0); */

      /* All grids are selectable */
    if (mode & (TARGET_GRID))
    {
        /* Disable other modes */
        mode &= ~(TARGET_LOOK | TARGET_KILL | TARGET_TRAP);

        /* Disable interesting grids */
        flag = FALSE;
    }

    /* Prepare the "temp" array */
    target_set_interactive_prepare(mode);

    /* Start near the player */
    m = 0;

    /* Interact */
    while (!done)
    {
        ui_toolbar_show(TOOLBAR_TARGETTING_INTERACTIVE);

        /* Interesting grids */
        if (flag && temp_n)
        {
            bool path_drawn = FALSE;
            int yy, xx;

            y = temp_y[m];
            x = temp_x[m];

            /* Dummy pointers to send to project_path */
            yy = y;
            xx = x;

            /* Allow targets for monsters....or traps, if applicable */
#if 0
            if (((cave_m_idx[y][x] > 0) && target_able(cave_m_idx[y][x])) ||
                ((mode & (TARGET_TRAP)) && target_able_trap(y, x)))
            {
                strcpy(info, "q,t,r,f,p,o,+,-,<dir>");
            }

            /* Dis-allow target */
            else
            {
                strcpy(info, "q,p,f,o,+,-,<dir>");
            }
#endif

            /* Adjust panel if needed */
            ui_ensure(y, x);

            /* Find the path. */
            path_n = project_path(path_g, path_gx, MAX_RANGE, py, px, &yy, &xx, PROJECT_THRU);

            /* Draw the path in "target" mode. If there is one */
            if ((mode & (TARGET_KILL)) && (dungeon_info[y][x].cave_info & (CAVE_FIRE)))
            {
                path_drawn = ui_draw_path(path_n, path_g, py, px, y, x);
            }

            ui_show_cursor(y, x);

            /* Describe and Prompt */
            //query = target_set_interactive_aux(y, x, mode, info, list_floor_objects);

            UserInput input = ui_get_input();

            /* Remove the path */
            if (path_drawn) ui_destroy_path();

            ui_show_cursor(-1, -1);

            /* Cancel tracking */
            /* health_track(0); */

            /* Assume no "direction" */
            d = 0;

            /* Analyze */
            switch (input.key)
            {
                case Qt::Key_Escape:
                case Qt::Key_Q:
                {
                    color_message(QObject::tr("Exiting interactive mode"), TERM_SKY_BLUE);
                    done = TRUE;
                    break;
                }

                case Qt::Key_Space:
                case Qt::Key_Asterisk:
                case Qt::Key_Plus:
                {
                    if (++m == temp_n) m = 0;
                    break;
                }

                case Qt::Key_Minus:
                {
                    if (m-- == 0)  m = temp_n - 1;
                    break;
                }

                case Qt::Key_P:
                {
                    /* Recenter around player */
                    ui_center(p_ptr->py, p_ptr->px);

                    y = py;
                    x = px;
                    break;
                }

                case Qt::Key_O:
                {
                    flag = FALSE;
                    break;
                }

                case Qt::Key_M:
                {
                    break;
                }

                /* If we click, move the target location to the click and
                   switch to "free targetting" mode by unsetting 'flag'.
                   This means we get some info about wherever we've picked. */
                case 0:
                {
                    x = input.x;
                    y = input.y;
                    flag = FALSE;
                    break;
                }

                case Qt::Key_T:
                case Qt::Key_5:
                case Qt::Key_0:
                case Qt::Key_Period:
                {
                    int m_idx = dungeon_info[y][x].monster_idx;

                    if ((m_idx > 0) && target_able(m_idx))
                    {
                        health_track(m_idx);
                        target_set_monster(m_idx);
                        done = TRUE;
                    }
                    else if ((mode & (TARGET_TRAP)) && target_able_trap(y, x))
                    {
                        target_set_location(y, x);
                        done = TRUE;
                    }
                    else if (mode & (TARGET_PROBE))
                    {
                        target_set_location(y, x);
                        done = TRUE;
                    }
                    else
                    {
                        message(QString("Illegal target!"));
                    }
                    break;
                }

                /*
                case 'g':
                {
                    cmd_insert(CMD_PATHFIND, y, x);
                    done = TRUE;
                    break;
                }
                */

                case Qt::Key_F:
                {
                    list_floor_objects = (!list_floor_objects);
                    break;
                }

#if 0
                case '?':
                {
                    help = !help;

                    /* Redraw main window */
                    p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
                    Term_clear();
                    handle_stuff();
                    if (!help)
                        prt("'?' - help", help_prompt_loc, 0);

                    break;
                }
#endif

                default:
                {
                    /* Extract direction */
                    d = target_dir(input);

                    /* Oops */
                    if (!d) message("Illegal command for target mode!");

                    break;
                }
            }

            /* Hack -- move around */
            if (d)
            {
                int old_y = temp_y[m];
                int old_x = temp_x[m];

                /* Find a new monster */
                i = target_pick(old_y, old_x, ddy[d], ddx[d]);

#if 0
                /* Scroll to find interesting grid */
                if (i < 0)
                {
                    int old_wy = Term->offset_y;
                    int old_wx = Term->offset_x;

                    /* Change if legal */
                    if (change_panel(d))
                    {
                        /* Recalculate interesting grids */
                        target_set_interactive_prepare(mode);

                        /* Find a new monster */
                        i = target_pick(old_y, old_x, ddy[d], ddx[d]);

                        /* Restore panel if needed */
                        if ((i < 0) && modify_panel(Term, old_wy, old_wx))
                        {
                            /* Recalculate interesting grids */
                            target_set_interactive_prepare(mode);
                        }

                        /* Handle stuff */
                        handle_stuff();
                    }
                }
#endif

                /* Use interesting grid if found */
                if (i >= 0) m = i;
            }
        }

        /* Arbitrary grids */
        else
        {
            bool path_drawn = FALSE;

            /* Dummy pointers to send to project_path */
            int yy = y;
            int xx = x;

#if 0
            /* Don't need this button any more */
            button_kill('o');

            /* Update help */
            if (help)
            {
                bool good_target = ((cave_m_idx[y][x] > 0) && target_able(cave_m_idx[y][x]));
                target_display_help(good_target, !(flag && temp_n));
            }

            /* Default prompt */
            if (!(mode & (TARGET_GRID)))
            {
                strcpy(info, "q,t,f,p,m,+,-,<dir>");
            }

            /* Disable monster selection */
            else
            {
                strcpy(info, "q,t,f.p,+,-,<dir>");
            }
#endif

            /* Find the path. */
            path_n = project_path(path_g, path_gx, MAX_RANGE, py, px, &yy, &xx, PROJECT_THRU);

            /* Draw the path in "target" mode. If there is one */
            if ((mode & (TARGET_KILL)) && (dungeon_info[y][x].cave_info & (CAVE_FIRE)))
            {
                /* Save target info */
                path_drawn = ui_draw_path(path_n, path_g, py, px, y, x);
            }

            //event_signal(EVENT_MOUSEBUTTONS);

            /* Describe and Prompt (enable "TARGET_LOOK") */
            //query = target_set_interactive_aux(y, x, (mode | TARGET_LOOK), info, list_floor_objects);

            ui_show_cursor(y, x);

            UserInput input = ui_get_input();

            /* Remove the path */
            if (path_drawn) ui_destroy_path();

            ui_show_cursor(-1, -1);

            /* Cancel tracking */
            /* health_track(0); */

            /* Assume no direction */
            d = 0;

            /* Analyze the keypress */
            switch (input.key)
            {
                case Qt::Key_Escape:
                case Qt::Key_Q:
                {
                    color_message(QObject::tr("Exiting interactive mode"), TERM_SKY_BLUE);
                    done = TRUE;
                    break;
                }

                case Qt::Key_Space:
                case Qt::Key_Asterisk:
                case Qt::Key_Plus:
                case Qt::Key_Minus:
                {
                    break;
                }

                case Qt::Key_P:
                {
                    /* Recenter around player */
                    ui_center(py, px);

                    y = py;
                    x = px;

                    break;
                }

                case Qt::Key_O:
                {
                    break;
                }

                case Qt::Key_M:
                {
                    /* Monster selection is disabled */
                    if (mode & (TARGET_GRID)) break;

                    flag = TRUE;

                    m = 0;
                    bd = 999;

                    /* Pick a nearby monster */
                    for (i = 0; i < temp_n; i++)
                    {
                        t = distance(y, x, temp_y[i], temp_x[i]);

                        /* Pick closest */
                        if (t < bd)
                        {
                            m = i;
                            bd = t;
                        }
                    }

                    /* Nothing interesting */
                    if (bd == 999) flag = FALSE;

                    break;
                }

                case 0:
                {

                    /* We only target if we click somewhere where the cursor
                       is already (i.e. a double-click without a time limit) */
                    if (input.x == x && input.y == y)
                    {
                        /* Make an attempt to target the monster on the given
                           square rather than the square itself (it seems this
                           is the more likely intention of clicking on a
                           monster). */
                        int m_idx = dungeon_info[y][x].monster_idx;

                        if ((m_idx > 0) && target_able(m_idx))
                        {
                            health_track(m_idx);
                            target_set_monster(m_idx);
                        }
                        else
                        {
                            /* There is no monster, or it isn't targettable,
                               so target the location instead. */
                            target_set_location(y, x);
                        }

                        done = TRUE;
                    }
                    else
                    {
                        /* Just move the cursor for now - another click will
                           target. */
                        x = input.x;
                        y = input.y;
                    }
                    break;
                }

                case Qt::Key_T:
                case Qt::Key_5:
                case Qt::Key_0:
                case Qt::Key_Period:
                {
                    target_set_location(y, x);
                    done = TRUE;
                    break;
                }

                /*
                case 'g':
                {
                    cmd_insert(CMD_PATHFIND, y, x);
                    done = TRUE;
                    break;
                }
                */

                case Qt::Key_F:
                {
                    list_floor_objects = (!list_floor_objects);
                }

#if 0
                case '?':
                {
                    help = !help;

                    /* Redraw main window */
                    p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
                    Term_clear();
                    handle_stuff();
                    if (!help)
                        prt("'?' - help.", help_prompt_loc, 0);

                    break;
                }
#endif

                default:
                {
                    /* Extract a direction */
                    d = target_dir(input);

                    /* Oops */
                    if (!d) message("Illegal command for target mode!");

                    break;
                }
            }

            /* Handle "direction" */
            if (d)
            {
                int dungeon_hgt = p_ptr->cur_map_hgt;
                int dungeon_wid = p_ptr->cur_map_wid;

                /* Move */
                x += ddx[d];
                y += ddy[d];

                /* Slide into legality */
                if (x >= dungeon_wid - 1) x--;
                else if (x <= 0) x++;

                /* Slide into legality */
                if (y >= dungeon_hgt - 1) y--;
                else if (y <= 0) y++;

                /* Adjust panel if needed */
                ui_ensure(y, x);
                if (true)
                {
                    /* Recalculate interesting grids */
                    target_set_interactive_prepare(mode);

                }
            }
        }
    }

    /* Forget */
    temp_n = 0;

#if 0
    /* Redraw as necessary */
    if (help)
    {
        p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
        Term_clear();
    }
    else
    {
        prt("", 0, 0);
        prt("", help_prompt_loc, 0);
        p_ptr->redraw |= (PR_DEPTH | PR_STATUS);
    }
#endif

    /* Recenter around player */
    ui_center(py, px);

    /* Re-set animate flicker */
    //animate_flicker = temp_animate_flicker;

    /* Failure to set target */
    if (!p_ptr->target_set) return (FALSE);

    /* Success */
    return (TRUE);
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
int target_dir(UserInput input)
{
    int d = 0;

    if (input.mode != INPUT_MODE_KEY) return 0;

    /* Already a direction? */
    if (!input.text.isEmpty() && input.text.at(0).isDigit())
    {
        d = input.text.mid(0, 1).toInt();
    }
    // handle arrow directions
    else if (input.key == Qt::Key_Left) {
        d = 4;
    }
    else if (input.key == Qt::Key_Up) {
        d = 8;
    }
    else if (input.key == Qt::Key_Right) {
        d = 6;
    }
    else if (input.key == Qt::Key_Down) {
        d = 2;
    }
    else
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

    color_message(QObject::tr("Entering targetting mode"), TERM_YELLOW);

    /* Hack -- auto-target if requested */
    if (use_old_target && target_okay() && !dir) dir = 5;

    /* Ask until satisfied */
    while (!dir)
    {
        ui_toolbar_show(TOOLBAR_TARGETTING);

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
            color_message(QObject::tr("Exiting targetting mode"), TERM_VIOLET);
            break;
        }

        /* Analyze */
        if (input.mode == INPUT_MODE_MOUSE) {
            /* Mouse aiming */
            if (target_set_interactive(TARGET_KILL, input.x, input.y)) dir = 5;
        }
        else if (input.text.compare("*") == 0) {
            /* Set new target, use target if legal */
            int mode = TARGET_KILL;
            if (target_trap) mode |= TARGET_TRAP;
            if (target_set_interactive(mode, -1, -1)) dir = 5;
        }
        else if (input.text.compare("c") == 0) {
            /* Set to closest target */
            if (target_set_closest(TARGET_KILL)) dir = 5;
        }
        else if (input.text.compare("t") == 0
                 || input.text.compare("5") == 0
                 || input.text.compare("0") == 0
                 || input.text.compare(".") == 0) {
            /* Use current target, if set and legal */
            if (target_okay()) dir = 5;
        }
        else {
            /* Possible direction */
            dir = target_dir(input);
        }

        /* Error */
        if (!dir) color_message("Illegal aim direction!", TERM_ORANGE);
    }

    ui_toolbar_hide(TOOLBAR_TARGETTING);

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
        message(QString("%1 is targeted.").arg(m_name));

    /* Set up target inQStringion */
    monster_race_track(m_ptr->r_idx);
    // TODO health_track(cave_m_idx[y][x]);
    target_set_monster(m_idx);

    /* Visual cue */
    // TODO draw it


    return TRUE;
}
