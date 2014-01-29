/*
 * File: prefs.c
 * Purpose: Pref file handling code
 *
 * Copyright (c) 2003 Takeshi Mogami, Robert Ruehlmann
 * Copyright (c) 2007 Pete Mack
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
#include "angband.h"
#include "z-file.h"
#include "cmds.h"
#include "ui-menu.h"
#include "game-cmd.h"


/*** Pref file saving code ***/



/*
 * Header and footer marker string for pref file dumps
 */
static cptr dump_separator = "#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#";



/*
 * Allow users to supply attr and char information in decimal, hexa-
 * decimal, octal, or even character form (the last of these three
 * being most familiar to many).  -LM-
 *
 * Also accept encoded tags for extended characters.
 */
static bool read_byte_or_char(char *zz, byte *a, char *c)
{
	*a = 0;  *c = '\0';

	/* First character is a digit or a '-' */
	if (my_isdigit(zz[0]) || (zz[0] == '-'))
	{
		/* Read as unsigned byte */
		*a = (byte)strtol(zz, NULL, 0);

		/* We are returning a byte */
		return (TRUE);
	}

	/* Usual case -- read it as a character */
	*c = zz[0];

	/* We are returning a char */
	return (FALSE);
}



/*
 * Remove old lines from pref files
 */
static void remove_old_dump(const char *cur_fname, const char *mark)
{
	bool between_marks = FALSE;
	bool changed = FALSE;

	char buf[1024];

	char start_line[1024];
	char end_line[1024];

	char new_fname[1024];

	ang_file *new_file;
	ang_file *cur_file;


	/* Format up some filenames */
	strnfmt(new_fname, sizeof(new_fname), "%s.new", cur_fname);

	/* Work out what we expect to find */
	strnfmt(start_line, sizeof(start_line), "%s begin %s",
			dump_separator, mark);
	strnfmt(end_line,   sizeof(end_line),   "%s end %s",
			dump_separator, mark);



	/* Open current file */
	cur_file = file_open(cur_fname, MODE_READ, FTYPE_TEXT);
	if (!cur_file) return;

	/* Open new file */
	new_file = file_open(new_fname, MODE_WRITE, FTYPE_TEXT);
	if (!new_file)
	{
		msg_format("Failed to create file %s", new_fname);
		return;
	}

	/* Loop for every line */
	while (file_getl(cur_file, buf, sizeof(buf)))
	{
		/* If we find the start line, turn on */
		if (!strcmp(buf, start_line))
		{
			between_marks = TRUE;
		}

		/* If we find the finish line, turn off */
		else if (!strcmp(buf, end_line))
		{
			between_marks = FALSE;
			changed = TRUE;
		}

		if (!between_marks)
		{
			/* Copy orginal line */
			file_putf(new_file, "%s\n", buf);
		}
	}

	/* Close files */
	file_close(cur_file);
	file_close(new_file);

	/* If there are changes, move things around */
	if (changed)
	{
		char old_fname[1024];
		strnfmt(old_fname, sizeof(old_fname), "%s.old", cur_fname);

		if (file_move(cur_fname, old_fname))
		{
			file_move(new_fname, cur_fname);
			file_delete(old_fname);
		}
	}

	/* Otherwise just destroy the new file */
	else
	{
		file_delete(new_fname);
	}
}


/*
 * Output the header of a pref-file dump
 */
static void pref_header(ang_file *fff, const char *mark)
{
	/* Start of dump */
	file_putf(fff, "%s begin %s\n", dump_separator, mark);

	file_putf(fff, "# *Warning!*  The lines below are an automatic dump.\n");
	file_putf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");
}

/*
 * Output the footer of a pref-file dump
 */
static void pref_footer(ang_file *fff, const char *mark)
{
	file_putf(fff, "# *Warning!*  The lines above are an automatic dump.\n");
	file_putf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");

	/* End of dump */
	file_putf(fff, "%s end %s\n", dump_separator, mark);
}

/*
 * Save autoinscription data to a pref file.
 */
static void autoinsc_dump(ang_file *fff)
{
	int i;
	if (!inscriptions) return;

	file_putf(fff, "# Autoinscription settings\n");
	file_putf(fff, "# B:item kind:inscription\n\n");

	for (i = 0; i < inscriptionsCount; i++)
	{
		object_kind *k_ptr = &k_info[inscriptions[i].kindIdx];

		file_putf(fff, "# Autoinscription for %s\n", k_name + k_ptr->name);
		file_putf(fff, "B:%d:%s\n\n", inscriptions[i].kindIdx,
		        quark_str(inscriptions[i].inscriptionIdx));
	}

	file_putf(fff, "\n");
}

/*
 * Write all current options to a user preference file.
 */
void option_dump(ang_file *fff)
{
	int i, j;

	/* Dump options (skip cheat, adult, score) */
	for (i = 0; i < OPT_CHEAT; i++)
	{
		const char *name = option_name(i);
		if (!name) continue;

		/* Comment */
		file_putf(fff, "# Option '%s'\n", option_desc(i));

		/* Dump the option */
		if (op_ptr->opt[i])
			file_putf(fff, "Y:%s\n", name);
		else
			file_putf(fff, "X:%s\n", name);

		/* Skip a line */
		file_putf(fff, "\n");
	}

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < (int)N_ELEMENTS(window_flag_desc); j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			file_putf(fff, "# Window '%s', Flag '%s'\n",
				angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (op_ptr->window_flag[i] & (1L << j))
				file_putf(fff, "W:%d:%d:1\n", i, j);
			else
				file_putf(fff, "W:%d:%d:0\n", i, j);

			/* Skip a line */
			file_putf(fff, "\n");
		}
	}

	autoinsc_dump(fff);
}



#ifdef ALLOW_MACROS

/*
 * Append all current macros to the given file
 */
void macro_dump(ang_file *fff)
{
	int i;
	char buf[1024];

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		file_putf(fff, "# Macro '%d'\n", i);

		/* Extract the macro action */
		ascii_to_text(buf, sizeof(buf), macro__act[i]);
		file_putf(fff, "A:%s\n", buf);

		/* Extract the macro pattern */
		ascii_to_text(buf, sizeof(buf), macro__pat[i]);
		file_putf(fff, "P:%s\n", buf);

		file_putf(fff, "\n");
	}
}



/*
 * Hack -- Append all keymaps to the given file.
 *
 * Hack -- We only append the keymaps for the "active" mode.
 */
void keymap_dump(ang_file *fff)
{
	size_t i;
	int mode;
	char buf[1024];

	if (rogue_like_commands)
		mode = KEYMAP_MODE_ROGUE;
	else
		mode = KEYMAP_MODE_ORIG;

	for (i = 0; i < N_ELEMENTS(keymap_act[mode]); i++)
	{
		char key[2] = "?";
		cptr act;

		/* Loop up the keymap */
		act = keymap_act[mode][i];

		/* Skip empty keymaps */
		if (!act) continue;

		/* Encode the action */
		ascii_to_text(buf, sizeof(buf), act);

		/* Dump the keymap action */
		file_putf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = i;

		/* Encode the key */
		ascii_to_text(buf, sizeof(buf), key);

		/* Dump the keymap pattern */
		file_putf(fff, "C:%d:%s\n", mode, buf);

		/* Skip a line */
		file_putf(fff, "\n");
	}

}


#endif



/* Dump monsters */
void dump_monsters(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		byte attr = r_ptr->x_attr;
		byte chr = r_ptr->x_char;

		/* Skip non-entries */
		if (!r_ptr->r_speed) continue;

		file_putf(fff, "# Monster: %s\n", r_ptr->name_full);
		file_putf(fff, "R:%d:0x%02X:0x%02X\n", i, attr, chr);
	}
}

/* Dump objects */
void dump_objects(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		byte attr = k_ptr->x_attr;
		byte chr = k_ptr->x_char;

		/* Skip non-entries */
		if (!k_ptr->name) continue;

		file_putf(fff, "# Object: %s\n", (k_name + k_ptr->name));
		file_putf(fff, "K:%d:0x%02X:0x%02X\n", i, attr, chr);
	}
}

/* Dump features */
void dump_features(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];
		byte attr = f_ptr->x_attr;
		byte chr = f_ptr->x_char;

		/* Skip non-entries */
		if (!f_ptr->name) continue;

		file_putf(fff, "# Terrain: %s\n", (f_name + f_ptr->name));
		file_putf(fff, "F:%d:0x%02X:0x%02X\n", i, attr, chr);
	}
}

/* Dump flavors */
void dump_flavors(ang_file *fff)
{
	int i;

	for (i = 1; i < z_info->flavor_max; i++)
	{
		flavor_type *x_ptr = &flavor_info[i];
		byte attr = x_ptr->x_attr;
		byte chr = x_ptr->x_char;

		file_putf(fff, "# Item flavor: %s\n", (flavor_text + x_ptr->text));
		file_putf(fff, "L:%d:0x%02X:0x%02X\n\n", i, attr, chr);
	}
}




/**
 * Save a set of preferences to file, overwriting any old preferences with the
 * same title.
 *
 * \param path is the filename to dump to
 * \param dump is a pointer to the function that does the writing to file
 * \param title is the name of this set of preferences
 *
 * \returns TRUE on success, FALSE otherwise.
 */
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title)
{
	ang_file *fff;

	/* Remove old macros */
	remove_old_dump(path, title);

	fff = file_open(path, MODE_APPEND, FTYPE_TEXT);
	if (!fff) return FALSE;

	/* Append the header */
	pref_header(fff, title);
	file_putf(fff, "\n\n");
	file_putf(fff, "# %s definitions\n\n", strstr(title, " "));

	dump(fff);

	file_putf(fff, "\n\n\n");
	pref_footer(fff, title);
	file_close(fff);

	return TRUE;
}





/*** Pref file parsing code ***/

/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens)
{
	int i = 0;

	char *s = buf;


	/* Process */
	while (i < num - 1)
	{
		char *t;

		/* Scan the string */
		for (t = s; *t; t++)
		{
			/* Found a delimiter */
			if ((*t == ':') || (*t == '/')) break;

			/* Handle single quotes */
			if (*t == '\'')
			{
				/* Advance */
				t++;

				/* Handle backslash */
				if (*t == '\\') t++;

				/* Require a character */
				if (!*t) break;

				/* Advance */
				t++;

				/* Hack -- Require a close quote */
				if (*t != '\'') *t = '\'';
			}

			/* Handle back-slash */
			if (*t == '\\') t++;
		}

		/* Nothing left */
		if (!*t) break;

		/* Nuke and advance */
		*t++ = '\0';

		/* Save the token */
		tokens[i++] = s;

		/* Advance */
		s = t;
	}

	/* Save the token */
	tokens[i++] = s;

	/* Number found */
	return (i);
}


/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Specify the attr/char values for "monsters" by race index.
 *   R:<num>:<a>/<c>
 *
 * Specify the attr/char values for "objects" by kind index.
 *   K:<num>:<a>/<c>
 *
 * Specify the attr/char values for "features" by feature index.
 *   F:<num>:<a>/<c>
 *
 * Specify the attr/char values for "special" things.
 *   S:<num>:<a>/<c>
 *
 * Specify the attribute values for inventory "objects" by kind tval.
 *   E:<tv>:<a>
 *
 * Define a macro action, given an encoded macro action.
 *   A:<str>
 *
 * Create a macro, given an encoded macro trigger.
 *   P:<str>
 *
 * Create a keymap, given an encoded keymap trigger.
 *   C:<num>:<str>
 *
 * Turn an option off, given its name.
 *   X:<str>
 *
 * Turn an option on, given its name.
 *   Y:<str>
 *
 * Turn a window flag on or off, given a window, flag, and value.
 *   W:<win>:<flag>:<value>
 *
 * Specify visual information, given an index, and some data.
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify colors for message-types.
 *   M:<type>:<attr>
 *
 * Specify the attr/char values for "flavors" by flavors index.
 *   L:<num>:<a>/<c>
 */
errr process_pref_file_command(char *buf)
{
	long i, j, n1, n2, sq;

 	char c;
 	byte a;

	char *zz[16];


	/* Skip "empty" lines */
	if (!buf[0]) return (0);

	/* Skip "blank" lines */
	if (isspace((unsigned char)buf[0])) return (0);

	/* Skip comments */
	if (buf[0] == '#') return (0);

	/* Paranoia */
	/* if (strlen(buf) >= 1024) return (1); */

	/* Require "?:*" format */
	if (buf[1] != ':') return (1);


	/* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
	if (buf[0] == 'R')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			monster_race *r_ptr;
			i = strtol(zz[0], NULL, 0);
			if ((i < 0) || (i >= (long)z_info->r_max)) return (1);
			r_ptr = &r_info[i];

 			/* Get monster color */
 			if (read_byte_or_char(zz[1], &a, &c))
 			      r_ptr->x_attr = a;
 			else  r_ptr->x_attr = (byte)color_char_to_attr(c);

 			/* Get monster symbol */
 			if (read_byte_or_char(zz[2], &a, &c))
 			      r_ptr->x_char = (char)a;
 			else  r_ptr->x_char = c;

			return (0);
		}
	}

	/* Process "B:<k_idx>:inscription */
	else if (buf[0] == 'B')
	{
		if(2 == tokenize(buf + 2, 2, zz))
		{
			add_autoinscription(strtol(zz[0], NULL, 0), zz[1]);
			return (0);
		}
	}

	/* Process "Q:<idx>:<tval>:<sval>:<y|n>"  -- squelch bits   */
	/* and     "Q:<idx>:<val>"                -- squelch levels */
	/* and     "Q:<val>"                      -- auto_destroy   */
	else if (buf[0] == 'Q')
	{
		i = tokenize(buf+2, 4, zz);
		if (i == 2)
		{
			n1 = strtol(zz[0], NULL, 0);
			n2 = strtol(zz[1], NULL, 0);
			squelch_level[n1] = n2;
			return(0);
		}
		else if (i == 4)
		{
			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			sq = strtol(zz[3], NULL, 0);
			if ((k_info[i].tval == n1) && (k_info[i].sval == n2))
			{
				k_info[i].squelch = sq;
				return(0);
			}
			else
			{
				for (i = 1; i < z_info->k_max; i++)
				{
					if ((k_info[i].tval == n1) && (k_info[i].sval == n2))
					{
						k_info[i].squelch = sq;
						return(0);
					}
				}
			}
		}
	}


	/* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
	else if (buf[0] == 'K')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			object_kind *k_ptr;
			i = strtol(zz[0], NULL, 0);
			if ((i < 0) || (i >= (long)z_info->k_max)) return (1);
			k_ptr = &k_info[i];

 			/* Get object kind color */
 			if (read_byte_or_char(zz[1], &a, &c))
 			      k_ptr->x_attr = a;
 			else  k_ptr->x_attr = (byte)color_char_to_attr(c);

 			/* Get object kind symbol */
 			if (read_byte_or_char(zz[2], &a, &c))
 			      k_ptr->x_char = (char)a;
 			else  k_ptr->x_char = c;

			return (0);
		}
	}


	/* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
	else if (buf[0] == 'F')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			int attr = -1;

			feature_type *f_ptr;
			i = strtol(zz[0], NULL, 0);
			if ((i < 0) || (i >= (long)z_info->f_max)) return (1);
			f_ptr = &f_info[i];

 			/* Get feature color */

			/* Color string has more than one character */
			if (zz[1][0] && zz[1][1])
			{
				/* Try to get a shade */
				attr = color_text_to_attr(zz[1]);
			}

			/* Got one? */
			if (attr >= 0)
			{
				f_ptr->x_attr = attr;
			}
			/* Try to parse single character or number */
			else
			{

 				if (read_byte_or_char(zz[1], &a, &c))
 			      	f_ptr->x_attr = a;
 				else  f_ptr->x_attr = (byte)color_char_to_attr(c);
			}

 			/* Get feature symbol */
 			if (read_byte_or_char(zz[2], &a, &c))
 			      f_ptr->x_char = (char)a;
 			else  f_ptr->x_char = c;

			return (0);
		}
	}


	/* Process "L:<num>:<a>/<c>" -- attr/char for flavors */
	else if (buf[0] == 'L')
	{
		if (tokenize(buf+2, 3, zz) == 3)
		{
			flavor_type *flavor_ptr;
			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if ((i < 0) || (i >= (long)z_info->flavor_max)) return (1);
			flavor_ptr = &flavor_info[i];
			if (n1) flavor_ptr->x_attr = (byte)n1;
			if (n2) flavor_ptr->x_char = (char)n2;
			return (0);
		}
	}

	/* Process "S:<num>:<a>:<c>:<a>:<c>" -- attr/char for ball spells and bolt spells */
	else if (buf[0] == 'S')
	{

		if (tokenize(buf+2, 5, zz) == 5)
		{
			long n3, n4;

			i = strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			n3 = strtol(zz[3], NULL, 0);
			n4 = strtol(zz[4], NULL, 0);
			if ((i < 0) || (i > (long)MAX_COLOR_USED)) return (1);
			color_to_attr[TILE_BALL_INFO][i] = (byte)n1;
			color_to_char[TILE_BALL_INFO][i] = (char)n2;
			color_to_attr[TILE_BOLT_INFO][i] = (byte)n3;
			color_to_char[TILE_BOLT_INFO][i] = (char)n4;
			return (0);
		}
	}

	/* Process "E:<tv>:<a>" -- attribute for inventory objects */
	else if (buf[0] == 'E')
	{
		if (tokenize(buf+2, 2, zz) == 2)
		{
			i = strtol(zz[0], NULL, 0) % 128;
 			n1 = strtol(zz[1], NULL, 0);
 			if ((i < 0) || (i >= (long)N_ELEMENTS(tval_to_attr))) return (1);
 			if (n1) tval_to_attr[i] = (byte)n1;
			return (0);
		}
	}


	/* Process "A:<str>" -- save an "action" for later */
	else if (buf[0] == 'A')
	{
		text_to_ascii(macro_buffer, sizeof(macro_buffer), buf+2);
		return (0);
	}

	/* Process "P:<str>" -- create macro */
	else if (buf[0] == 'P')
	{
		char tmp[1024];
		text_to_ascii(tmp, sizeof(tmp), buf+2);
		macro_add(tmp, macro_buffer);
		return (0);
	}

	/* Process "C:<num>:<str>" -- create keymap */
	else if (buf[0] == 'C')
	{
		long mode;

		char tmp[1024];

		if (tokenize(buf+2, 2, zz) != 2) return (1);

		mode = strtol(zz[0], NULL, 0);
		if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);

		text_to_ascii(tmp, sizeof(tmp), zz[1]);
		if (!tmp[0] || tmp[1]) return (1);
		i = (long)tmp[0];

		string_free(keymap_act[mode][i]);

		keymap_act[mode][i] = string_make(macro_buffer);

		return (0);
	}


	/* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
	else if (buf[0] == 'V')
	{
		if (tokenize(buf+2, 5, zz) == 5)
		{
			i = strtol(zz[0], NULL, 0);
			if ((i < 0) || (i >= 256)) return (1);
			angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
			angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
			angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
			angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
			return (0);
		}
	}

	/* set macro trigger names and a template */
	/* Process "T:<trigger>:<keycode>:<shift-keycode>" */
	/* Process "T:<template>:<modifier chr>:<modifier name>:..." */
	else if (buf[0] == 'T')
	{
		int tok;

		tok = tokenize(buf + 2, MAX_MACRO_MOD + 2, zz);

		/* Trigger template */
		if (tok >= 4)
		{
			int i;
			int num;

			/* Free existing macro triggers and trigger template */
			macro_trigger_free();

			/* Clear template done */
			if (*zz[0] == '\0') return 0;

			/* Count modifier-characters */
			num = strlen(zz[1]);

			/* One modifier-character per modifier */
			if (num + 2 != tok) return 1;

			/* Macro template */
			macro_template = string_make(zz[0]);

			/* Modifier chars */
			macro_modifier_chr = string_make(zz[1]);

			/* Modifier names */
			for (i = 0; i < num; i++)
			{
				macro_modifier_name[i] = string_make(zz[2+i]);
			}
		}
		/* Macro trigger */
		else if (tok >= 2)
		{
			char *buf;
			cptr s;
			char *t;

			if (max_macrotrigger >= MAX_MACRO_TRIGGER)
			{
				msg_print("Too many macro triggers!");
				return 1;
			}

			/* Buffer for the trigger name */
			buf = C_ZNEW(strlen(zz[0]) + 1, char);

			/* Simulate strcpy() and skip the '\' escape character */
			s = zz[0];
			t = buf;

			while (*s)
			{
				if ('\\' == *s) s++;
				*t++ = *s++;
			}

			/* Terminate the trigger name */
			*t = '\0';

			/* Store the trigger name */
			macro_trigger_name[max_macrotrigger] = string_make(buf);

			/* Free the buffer */
			FREE(buf);

			/* Normal keycode */
			macro_trigger_keycode[0][max_macrotrigger] = string_make(zz[1]);

			/* Special shifted keycode */
			if (tok == 3)
			{
				macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[2]);
			}
			/* Shifted keycode is the same as the normal keycode */
			else
			{
				macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[1]);
			}

			/* Count triggers */
			max_macrotrigger++;
		}

		return 0;
	}


	/* Process "X:<str>" -- turn option off */
	else if (buf[0] == 'X')
	{
		/* Check non-adult options */
		for (i = 0; i < OPT_ADULT; i++)
		{
			if (options[i].name && streq(options[i].name, buf + 2))
			{
				op_ptr->opt[i] = FALSE;
				return (0);
			}
		}

		/* Ignore unknown options */
		return (0);

	}

	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'Y')
	{
		/* Check non-adult options */
		for (i = 0; i < OPT_ADULT; i++)
		{
			if (options[i].name && streq(options[i].name, buf + 2))
			{
				op_ptr->opt[i] = TRUE;
				return (0);
			}
		}

		/* Ignore unknown options */
		return (0);

	}


	/* Process "W:<win>:<flag>:<value>" -- window flags */
	else if (buf[0] == 'W')
	{
		long win, flag, value;

		if (tokenize(buf + 2, 3, zz) == 3)
		{
			win = strtol(zz[0], NULL, 0);
			flag = strtol(zz[1], NULL, 0);
			value = strtol(zz[2], NULL, 0);

			/* Ignore illegal windows */
			/* Hack -- Ignore the main window */
			if ((win <= 0) || (win >= ANGBAND_TERM_MAX)) return (1);

			/* Ignore illegal flags */
			if ((flag < 0) || (flag >= 32)) return (1);

			/* Require a real flag */
			if (window_flag_desc[flag])
			{
				if (value)
				{
					/* Turn flag on */
					op_ptr->window_flag[win] |= (1L << flag);
				}
				else
				{
					/* Turn flag off */
					op_ptr->window_flag[win] &= ~(1L << flag);
				}
			}

			/* Success */
			return (0);
		}
	}


	/* Process "M:<type>:<attr>" -- colors for message-types */
	else if (buf[0] == 'M')
	{
		if (tokenize(buf+2, 2, zz) == 2)
		{
			byte color;
			/* Get tval */
 			j = strtol(zz[0], NULL, 0);

 			if ((j < 0) || (j >= MSG_MAX)) return (1);

 			/* Handle 'r', or '4', or '0x04' for "red" */
 			if (read_byte_or_char(zz[1], &a, &c))   color = a;
 			else color = (byte)color_char_to_attr(c);

			message_color_define((MessageType)j, color);

			/* Success */
			return (0);
		}
	}


	/* Failure */
	return (1);
}




/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static cptr process_pref_file_expr(char **sp, char *fp)
{
	cptr v;

	char *b;
	char *s;

	char b1 = '[';
	char b2 = ']';

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace((unsigned char)*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == b1)
	{
		const char *p;
		const char *t;

		/* Skip b1 */
		s++;

		/* First */
		t = process_pref_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) >= 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) <= 0)) v = "0";
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != b2) v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';
	}

	/* Other */
	else
	{
		/* Accept all printables except spaces and brackets */
		while (my_isprint((unsigned char)*s) && !strchr(" []", *s)) ++s;

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			/* System */
			if (streq(b+1, "SYS"))
			{
				v = ANGBAND_SYS;
			}

			/* Graphics */
			else if (streq(b+1, "GRAF"))
			{
				v = ANGBAND_GRAF;
			}

			/* Race */
			else if (streq(b+1, "RACE"))
			{
				v = p_name + rp_ptr->name;
			}

			/* Class */
			else if (streq(b+1, "CLASS"))
			{
				v = c_name + cp_ptr->name;
			}

			/* Player */
			else if (streq(b+1, "PLAYER"))
			{
				v = op_ptr->base_name;
			}

			/* Game version */
			else if (streq(b+1, "VERSION"))
			{
				v = VERSION_STRING;
			}
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;

	/* Save */
	(*sp) = s;

	/* Result */
	return (v);
}


/*
 * Open the "user pref file" and parse it.
 */
static errr process_pref_file_aux(cptr name)
{
	ang_file *fp;

	char buf[1024];
	char old[1024];

	int line = -1;

	errr err = 0;

	bool bypass = FALSE;


	/* Open the file */
	fp = file_open(name, MODE_READ, FTYPE_TEXT);
	if (!fp) return (-1);


	/* Process the file */
	while (file_getl(fp, buf, sizeof(buf)))
	{
		/* Count lines */
		line++;

		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace((unsigned char) buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;


		/* Save a copy */
		my_strcpy(old, buf, sizeof(old));

		/* Process "?:<expr>" */
		if ((buf[0] == '?') && (buf[1] == ':'))
		{
			char f;
			cptr v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_pref_file_expr(&s, &f);

			/* Set flag */
			bypass = (streq(v, "0") ? TRUE : FALSE);

			/* Continue */
			continue;
		}

		/* Apply conditionals */
		if (bypass) continue;


		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			(void)process_pref_file(buf + 2);

			/* Continue */
			continue;
		}

		/* Process the line */
		err = process_pref_file_command(buf);

		/* Oops */
		if (err) break;
	}

	/* Error */
	if (err)
	{
		/* Print error message */
		/* ToDo: Add better error messages */
		msg_format("Error %d in line %d of file '%s'.", err, line, name);
		msg_format("Parsing '%s'", old);
		message_flush();
	}

	/* Close the file */
	file_close(fp);

	/* Result */
	return (err);
}



/*
 * Process the "user pref file" with the given name
 *
 * See the functions above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
	char buf[1024];

	errr err = 0;


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_PREF, name);

	/* Process the pref file */
	err = process_pref_file_aux(buf);

	/* Stop at parser errors, but not at non-existing file */
	if (err < 1)
	{
		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

		/* Process the pref file */
		err = process_pref_file_aux(buf);

	}

	/* Result */
	return (err);
}


