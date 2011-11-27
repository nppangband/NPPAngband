/* File: use_obj.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

#ifdef  EXCESS CODE

/* nothing left */


		if (evt.type == EVT_NONE) my_strcat(message, "none", sizeof(message));
		if (evt.type == EVT_ESCAPE) my_strcat(message, " escape", sizeof(message));
		if (evt.type == EVT_KBRD) my_strcat(message, " keyboard", sizeof(message));
		if (evt.type == EVT_MOUSE) my_strcat(message, " mouse", sizeof(message));
		if (evt.type == EVT_BACK) my_strcat(message, " back", sizeof(message));
		if (evt.type == EVT_MOVE) my_strcat(message, " move", sizeof(message));
		if (evt.type == EVT_SELECT) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_BUTTON) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_CMD) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_OK) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_REFRESH) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_RESIZE) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_AGAIN) my_strcat(message, " none", sizeof(message));
		if (evt.type == EVT_STOP) my_strcat(message, " none", sizeof(message));
		my_strcat(message, format(" type is %d", evt.type), sizeof(message)); */

		if ((mode == BOOK_CAST) && (FALSE))
		{
			/* check for a keyboard selection */
			if (evt.type == EVT_KBRD)
			{
				int i = A2I(evt.key);

				if (evt.type == EVT_KBRD)

				/* spell choice */
				if ((i >= 0) || (i < num))
				{
					if (spell_okay(spells[i], TRUE)) pick = spells[i];
				}
			}

			else if (evt.type == EVT_SELECT)
			{
				if ((evt.key == '\r') || (old_cursor == cursor))
				{
					/* We have a good spell */
					if (spell_okay(spell, TRUE)) pick = spells[cursor];
					else error = TRUE;
				}
			}
			else if (evt.type == EVT_BUTTON)
			{
				switch (evt.key)
				{
					case 'G':
					{
						if (spell_okay(spell, FALSE))
						{
							spell_learn(spell);
							handle_stuff();
						}
						else error = TRUE;
						break;
					}
					case 'M':
					{
						if (spell_okay(spell, TRUE)) pick = spell;
						else error = (TRUE);
						break;
					}
					case '?':
					{
						show_file("magic.txt#magic", NULL, 0, 0);
						break;
					}
					default:
					{
						int i = A2I(evt.key);
						/* Totally Illegal */
						if ((i < 0) || (i >= num))
						{
							error = TRUE;
						}
						else pick = spells[i];
						break;
					}
				}
			}
		}

		else if (mode == BOOK_STUDY)
		{
			if ((evt.type == EVT_SELECT)  && (FALSE))
			{
				if (evt.key == '\r')
				{
					/* We have a good spell */
					if (spell_okay(spell, FALSE)) pick = spells[cursor];
					else error = TRUE;
					break;
				}
			}
			else if ((evt.type == EVT_BUTTON) && (FALSE))
			{
				switch (evt.key)
				{
					case '\r':
					case 'G':
					{
						/* We have a good spell */
						if (spell_okay(spell, FALSE)) pick = spells[cursor];
						else error = TRUE;
						break;
					}
					case '?':
					{
						show_file("magic.txt#magic", NULL, 0, 0);
						break;
					}
					default:
				{
						error = TRUE;
						break;
					}
				}
			}
		}

		else if (mode == BOOK_BROWSE)
		{

			if (evt.type == EVT_BUTTON)
			{
				switch (evt.key)
				{
					case 'G':
					{
						if (spell_okay(spell, FALSE))
						{
							spell_learn(spell);
							handle_stuff();
						}
						else error = TRUE;
						break;
					}

					case '?':	{show_file("magic.txt#magic", NULL, 0, 0); break;}
					default:  	break;
				}
			}
		}

		if (mode == BOOK_STUDY)
				{
					if ((evt.type == EVT_SELECT)  && (FALSE))
					{
						if (evt.key == '\r')
						{
							/* We have a good spell */
							if (spell_okay(spell, FALSE)) pick = spells[cursor];
							else error = TRUE;
							break;
						}
					}
					else if ((evt.type == EVT_BUTTON) && (FALSE))
					{
						switch (evt.key)
						{
							case '\r':
							case 'G':
							{
								/* We have a good spell */
								if (spell_okay(spell, FALSE)) pick = spells[cursor];
								else error = TRUE;
								break;
							}
							case '?':
							{
								show_file("magic.txt#magic", NULL, 0, 0);
								break;
							}
							default:
						{
								error = TRUE;
								break;
							}
						}
					}
				}

				else if (mode == BOOK_BROWSE)
				{

					if (evt.type == EVT_BUTTON)
					{
						switch (evt.key)
						{
							case 'G':
							{
								if (spell_okay(spell, FALSE))
								{
									spell_learn(spell);
									handle_stuff();
								}
								else error = TRUE;
								break;
							}

							case '?':	{show_file("magic.txt#magic", NULL, 0, 0); break;}
							default:  	break;
						}
					}
				}

#endif EXCESS CODE
