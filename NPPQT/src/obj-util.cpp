/*
* File: obj-util.cpp
* Purpose: Object list maintenance and other object utilities
*
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
 * Find the index of the object_kind with the given tval and sval
 */
s16b lookup_kind(int tval, int sval)
{
    int k;

    /* Look for it */
    for (k = 1; k < z_info->k_max; k++)
    {
        object_kind *k_ptr = &k_info[k];

        /* Found a match */
        if ((k_ptr->tval == tval) && (k_ptr->sval == sval)) return (k);
    }

    /* Oops */
    pop_up_message_box(QString(QObject::tr("No object (%1,%2)")) .arg(tval) .arg(sval));

    /* Oops */
    return (0);
}
