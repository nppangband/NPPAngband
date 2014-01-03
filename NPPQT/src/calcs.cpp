/* File: calcs.c */

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


/* return energy gain for a player or monster */
byte calc_energy_gain(byte speed)
{
    if (game_mode == GAME_NPPMORIA)
    {
        /* Boundry control */
        if (speed < NPPMORIA_LOWEST_SPEED) speed = NPPMORIA_LOWEST_SPEED;
        else if (speed > NPPMORIA_MAX_SPEED) speed = NPPMORIA_MAX_SPEED;

        return (extract_energy_nppmoria[speed - NPPMORIA_LOWEST_SPEED]);
    }

    if (speed > 199) speed = 199;

    return (extract_energy_nppangband[speed]);
}
