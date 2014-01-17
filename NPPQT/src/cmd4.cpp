/* File: cmd4.c */

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

#include "npp.h"
#include <QFile>

/*
 * Create and return an empty file in writing mode to append notes.
 * It returns a copy of the file name in "path".
 * "max" must be the maximum size in bytes of "path".
 * Returns NULL on failure
 */
void create_notes_file(void)
{
    QString file_name = ("notes.txt");

    /* Build the filename */
    notes_file.setFileName(QString("%1%2" ) .arg(NPP_DIR_USER) .arg(file_name));

    notes_file.open(QIODevice::WriteOnly);

    QDataStream out_notes(&notes_file);

    return;
}

/*
 * Close and destroy the notes file. notes_file and notes_fname variables are cleared
 */
void delete_notes_file(void)
{
    notes_file.close();
    /* Close the notes file */
    if (notes_file.exists())
    {
        notes_file.remove();
    }
}


