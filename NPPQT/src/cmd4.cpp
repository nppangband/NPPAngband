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

#include "src/npp.h"
#include <QFile>


/*
 * Array of feeling strings
 */
static QString do_cmd_feeling_text[LEV_THEME_HEAD] =
{
    "Looks like any other level.",
    "You feel there is something special about this level.",
    "You have a superb feeling about this level.",
    "You have an excellent feeling...",
    "You have a very good feeling...",
    "You have a good feeling...",
    "You feel strangely lucky...",
    "You feel your luck is turning...",
    "You like the look of this place...",
    "This level can't be all bad...",
    "What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
    bool is_quest_level = quest_check(p_ptr->depth);

    /* No sensing things in Moria */
    if (game_mode == GAME_NPPMORIA) return;

    /* No useful feeling in town */
    if (!p_ptr->depth)
    {
        message(QString("Looks like a typical town."));
        return;
    }

    /* No useful feelings until enough time has passed */
    if (!do_feeling)
    {
        message(QString("You are still uncertain about this level..."));
        return;
    }

    if (p_ptr->dungeon_type == DUNGEON_TYPE_WILDERNESS)
    {
        if (is_quest_level) 	message(QString("You have entered a wilderness level on the verge of destruction!."));
        else 					message(QString("You have entered an area of near pristine wilderness."));
    }

    else if (p_ptr->dungeon_type == DUNGEON_TYPE_GREATER_VAULT)
    {
        message(QString("You have discovered a gigantic vault of great treasures guarded by dangerous creatures."));
    }

    else if (p_ptr->dungeon_type == DUNGEON_TYPE_LABYRINTH)
    {
        if (is_quest_level) message(QString("You have entered a tiny, closely guarded labyrinth."));
        else message(QString("You have entered a complex labyrinth of dungeon hallways."));
    }

    else if (p_ptr->dungeon_type == DUNGEON_TYPE_ARENA)
    {
        message(QString("You are in an arena fighting for your life."));
    }

    /* Verify the feeling */
    else if (feeling >= LEV_THEME_HEAD)
    {

        /*print out a message about a themed level*/
        QString note;
        QString mon_theme;


        note = (QString("You have entered "));
        mon_theme = (QString(feeling_themed_level[feeling - LEV_THEME_HEAD]));
        QChar first = mon_theme[0];
        if (is_a_vowel(first)) note.append(QString("an "));
        else note.append(QString("a "));

        note.append(QString("%1 stronghold.") .arg(mon_theme));

        message(note);
    }

    /* Display the feeling */
    else message(QString(do_cmd_feeling_text[feeling]));

    /* Redraw the feeling indicator */
    p_ptr->redraw |= (PR_FEELING);
}



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


