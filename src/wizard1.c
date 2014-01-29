/* File: wizard1.c */

/*
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

#include "angband.h"
#include "cmds.h"


#ifdef ALLOW_SPOILERS


/*
 * The spoiler file being created
 */
static ang_file *fh = NULL;


/*
 * Write out `n' of the character `c' to the spoiler file
 */
static void spoiler_out_n_chars(int n, char c)
{
	while (--n >= 0) file_writec(fh, c);
}


/*
 * Write out `n' blank lines to the spoiler file
 */
static void spoiler_blanklines(int n)
{
	spoiler_out_n_chars(n, '\n');
}


/*
 * Write a line to the spoiler file and then "underline" it with hypens
 */
static void spoiler_underline(cptr str, char c)
{
	text_out(str);
	text_out("\n");
	spoiler_out_n_chars(strlen(str), c);
	text_out("\n");
}


/*
 * Item Spoilers by Ben Harrison (benh@phial.com)
 */


/*
 * The basic items categorized by type
 */
static const grouper group_item[] =
{
	{ TV_SHOT,		"Ammo" },
	{ TV_ARROW,		  NULL },
	{ TV_BOLT,		  NULL },

	{ TV_BOW,		"Bows" },

	{ TV_SWORD,		"Weapons" },
	{ TV_POLEARM,	  NULL },
	{ TV_HAFTED,	  NULL },
	{ TV_DIGGING,	  NULL },

	{ TV_SOFT_ARMOR,	"Armour (Body)" },
	{ TV_HARD_ARMOR,	  NULL },
	{ TV_DRAG_ARMOR,	  NULL },

	{ TV_CLOAK,		"Armour (Misc)" },
	{ TV_SHIELD,	  NULL },
	{ TV_DRAG_SHIELD, NULL },
	{ TV_HELM,		  NULL },
	{ TV_CROWN,		  NULL },
	{ TV_GLOVES,	  NULL },
	{ TV_BOOTS,		  NULL },

	{ TV_AMULET,	"Amulets" },
	{ TV_RING,		"Rings" },

	{ TV_SCROLL,	"Scrolls" },
	{ TV_POTION,	"Potions" },
	{ TV_FOOD,		"Food" },

	{ TV_ROD,		"Rods" },
	{ TV_WAND,		"Wands" },
	{ TV_STAFF,		"Staffs" },

	{ TV_MAGIC_BOOK,	"Books (Mage)" },
	{ TV_PRAYER_BOOK,	"Books (Priest)" },
	{ TV_DRUID_BOOK,	"Books (Druid)" },

	{ TV_CHEST,		"Chests" },

	{ TV_SPIKE,		"Various" },
	{ TV_LIGHT,		  NULL },
	{ TV_FLASK,		  NULL },
	{ TV_JUNK,		  NULL },
	{ TV_BOTTLE,	  NULL },
	{ TV_SKELETON,	  NULL },

	{ 0, "" }
};


/*
 * Describe the kind
 */
static void kind_info(char *buf, size_t buf_len,
					  char *dam, size_t dam_len,
					  char *wgt, size_t wgt_len,
					  int *lev, s32b *val, int k)
{
	object_kind *k_ptr;

	object_type object_type_body;

	/* Get local object */
	object_type *i_ptr = &object_type_body;

	/* Prepare a fake item */
	object_prep(i_ptr, k);

	/* Obtain the "kind" info */
	k_ptr = &k_info[i_ptr->k_idx];

	/* Cancel bonuses */
	i_ptr->pval = 0;
	i_ptr->to_a = 0;
	i_ptr->to_h = 0;
	i_ptr->to_d = 0;

	/* Level */
	(*lev) = k_ptr->k_level;

	/* Identify the object and get the squelch setting */
	identify_object(i_ptr, TRUE);

	/* Value */
	(*val) = object_value(i_ptr);

	/* Description (too brief) */
	if (buf)
		object_desc(buf, buf_len, i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Weight */
	if (wgt)
		strnfmt(wgt, wgt_len, "%3d.%d",
				i_ptr->weight / 10, i_ptr->weight % 10);

	/* Hack */
	if (!dam)
		return;

	/* Misc info */
	dam[0] = '\0';

	/* Damage */
	switch (i_ptr->tval)
	{
		/* Bows */
		case TV_BOW:
		{
			break;
		}

		/* Ammo */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			strnfmt(dam, dam_len, "%dd%d", i_ptr->dd, i_ptr->ds);
			break;
		}

		/* Weapons */
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			strnfmt(dam, dam_len, "%dd%d", i_ptr->dd, i_ptr->ds);
			break;
		}

		/* Armour */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			strnfmt(dam, dam_len, "%d", i_ptr->ac);
			break;
		}
	}
}


/*
 * Create a spoiler file for items
 */
static void spoil_obj_desc(cptr fname)
{
	int i, k, s, t, n = 0;

	u16b who[200];

	char buf[1024];

	char wgt[80];
	char dam[80];

	cptr format = "%-51s  %7s%6s%4s%9s\n";

	/* We use either ASCII or system-specific encoding */
 	int encoding = (xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

	/* Open the file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Header */
	file_putf(fh, "Spoiler File -- Basic Items (%s)\n\n\n", VERSION_STRING);

	/* More Header */
	file_putf(fh, format, "Description", "Dam/AC", "Wgt", "Lev", "Cost");
	file_putf(fh, format, "----------------------------------------",
	        "------", "---", "---", "----");

	/* List the groups */
	for (i = 0; TRUE; i++)
	{
		/* Write out the group title */
		if (group_item[i].name)
		{
			/* Hack -- bubble-sort by cost and then level */
			for (s = 0; s < n - 1; s++)
			{
				for (t = 0; t < n - 1; t++)
				{
					int i1 = t;
					int i2 = t + 1;

					int e1;
					int e2;

					s32b t1;
					s32b t2;

					kind_info(NULL, 0, NULL, 0, NULL, 0, &e1, &t1, who[i1]);
					kind_info(NULL, 0, NULL, 0, NULL, 0, &e2, &t2, who[i2]);

					if ((t1 > t2) || ((t1 == t2) && (e1 > e2)))
					{
						int tmp = who[i1];
						who[i1] = who[i2];
						who[i2] = tmp;
					}
				}
			}

			/* Spoil each item */
			for (s = 0; s < n; s++)
			{
				int e;
				s32b v;

				/* Describe the kind */
				kind_info(buf, sizeof(buf), dam, sizeof(dam), wgt, sizeof(wgt), &e, &v, who[s]);

				/* Dump it */
				x_file_putf(fh, encoding, "  %-51s%7s%6s%4d%9ld\n",
							buf, dam, wgt, e, (long)(v));
			}

			/* Start a new set */
			n = 0;

			/* Notice the end */
			if (!group_item[i].tval) break;

			/* Start a new set */
			x_file_putf(fh, encoding, "\n\n%s\n\n", group_item[i].name);
		}

		/* Get legal item types */
		for (k = 1; k < z_info->k_max; k++)
		{
			object_kind *k_ptr = &k_info[k];

			/* Skip wrong tval's */
			if (k_ptr->tval != group_item[i].tval) continue;

			/* Hack -- Skip instant-artifacts */
			if (k_ptr->k_flags3 & (TR3_INSTA_ART)) continue;

			/* Save the index */
			who[n++] = k;
		}
	}

	/* Check for errors */
	if (!file_close(fh))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Artifact Spoilers by: randy@PICARD.tamu.edu (Randy Hutson)
 *
 * (Mostly) rewritten in 2002 by Andrew Sidwell and Robert Ruehlmann.
 */


/*
 * The artifacts categorized by type
 */
static const grouper group_artifact[] =
{
	{ TV_SWORD,			"Edged Weapons" },
	{ TV_POLEARM,		"Polearms" },
	{ TV_HAFTED,		"Hafted Weapons" },
	{ TV_BOW,			"Bows" },
	{ TV_DIGGING,		"Diggers" },

	{ TV_SOFT_ARMOR,	"Body Armor" },
	{ TV_HARD_ARMOR,	NULL },
	{ TV_DRAG_ARMOR,	NULL },

	{ TV_CLOAK,			"Cloaks" },
	{ TV_SHIELD,		"Shields" },
	{ TV_DRAG_SHIELD,	NULL },
	{ TV_HELM,			"Helms/Crowns" },
	{ TV_CROWN,			NULL },
	{ TV_GLOVES,		"Gloves" },
	{ TV_BOOTS,			"Boots" },

	{ TV_LIGHT,			"Light Sources" },
	{ TV_AMULET,		"Amulets" },
	{ TV_RING,			"Rings" },

	{ 0, NULL }
};


/*
 * Create a spoiler file for artifacts
 */
static void spoil_artifact(cptr fname)
{
	int i, j;

	object_type *i_ptr;
	object_type object_type_body;

	char buf[1024];

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump to the spoiler file */
	text_out_hook = text_out_to_file;
	text_out_file = fh;

	/* Set object_info_out() hook */
	object_info_out_flags = object_flags;

	/* Dump the header */
	spoiler_underline(format("Artifact Spoilers for %s %s",
							 VERSION_MODE_NAME, VERSION_STRING), '=');

	/* List the artifacts by tval */
	for (i = 0; group_artifact[i].tval; i++)
	{
		/* Write out the group title */
		if (group_artifact[i].name)
		{
			spoiler_blanklines(2);
			spoiler_underline(group_artifact[i].name, '=');
			spoiler_blanklines(1);
		}

		/* Now search through all of the artifacts */
		for (j = 1; j < z_info->art_max; ++j)
		{
			artifact_type *a_ptr = &a_info[j];
			char buf[80];

			/* We only want objects in the current group */
			if (a_ptr->tval != group_artifact[i].tval) continue;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(i_ptr);

			/* Attempt to "forge" the artifact */
			if (!make_fake_artifact(i_ptr, (byte)j)) continue;

			object_aware(i_ptr);
			object_known(i_ptr);
			i_ptr->ident |= (IDENT_MENTAL);

			/* Grab artifact name */
			object_desc(buf, sizeof(buf), i_ptr, ODESC_PREFIX | ODESC_FULL);

			/* Print name and underline */
			spoiler_underline(buf, '-');

			identify_object(i_ptr, TRUE);

			/* Write out the artifact description to the spoiler file */
			object_info_out(i_ptr, FALSE);

			/*
			 * Determine the minimum depth an artifact can appear, its rarity,
			 * its weight, and its value in gold pieces.
			 */
			text_out("\nMin Level %u, Rarity %u, %d.%d lbs\n",
				a_ptr->a_level, a_ptr->a_rarity, (a_ptr->weight / 10),
				(a_ptr->weight % 10));

			/* Terminate the entry */
			spoiler_blanklines(2);
		}
	}

	/* Check for errors */
	if (!file_close(fh))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Create a spoiler file for monsters
 */
static void spoil_mon_desc(cptr fname)
{
	int i, n = 0;

	char buf[1024];

	char nam[80];
	char lev[80];
	char rar[80];
	char spd[80];
	char ac[80];
	char hp[80];
	char exp[80];

	u16b *who;
	u16b why = 2;

	/* We use either ascii or system-specific encoding */
 	int encoding = (xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */
	x_file_putf(fh, encoding, "Monster Spoilers for %s Version %s\n",
			VERSION_MODE_NAME, VERSION_STRING);
	x_file_putf(fh, encoding, "------------------------------------------\n\n");

	/* Dump the header */
	x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s %12.12s\n",
	        "Name", "Lev", "Rar", "Spd", "Hp", "Ac", "Visual Info");
	x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s %12.12s\n",
	        "----", "---", "---", "---", "--", "--", "------------");

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monsters (except the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->r_speed) who[n++] = (u16b)i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of monsters */
	ang_sort(who, &why, n);

	/* Scan again */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[who[i]];

		cptr name = (r_ptr->name_full);

		/* Get the "name" */
		if (r_ptr->flags1 & (RF1_QUESTOR))
		{
			strnfmt(nam, sizeof(nam), "[Q] %s", name);
		}
		else if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			strnfmt(nam, sizeof(nam), "[U] %s", name);
		}
		else
		{
			strnfmt(nam, sizeof(nam), "The %s", name);
		}

		/* Level */
		strnfmt(lev, sizeof(lev), "%d", r_ptr->level);

		/* Rarity */
		strnfmt(rar, sizeof(rar), "%d", r_ptr->rarity);

		/* Speed */
		if (calc_energy_gain(r_ptr->r_speed) >= STANDARD_ENERGY_GAIN)
			strnfmt(spd, sizeof(spd), "+%d", (calc_energy_gain(r_ptr->r_speed) - STANDARD_ENERGY_GAIN));
		else
			strnfmt(spd, sizeof(spd), "-%d", (STANDARD_ENERGY_GAIN - calc_energy_gain(r_ptr->r_speed)));

		/* Armor Class */
		strnfmt(ac, sizeof(ac), "%d", r_ptr->ac);

		/* Hitpoints */
		strnfmt(hp, sizeof(hp), "hp dice %d, hp sides %d", r_ptr->hdice,r_ptr->hside);

		/* Experience */
		strnfmt(exp, sizeof(exp), "%ld", (long)(r_ptr->mexp));

		/* Hack -- use visual instead */
		strnfmt(exp, sizeof(exp), "%s '%c'", attr_to_text(r_ptr->d_attr), r_ptr->d_char);

		/* Dump the info */
		x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s %12.12s\n",
					nam, lev, rar, spd, hp, ac, exp);
	}

	/* End it */
	file_putf(fh, "\n");

	/* Free the "who" array */
	FREE(who);

	/* Check for errors */
	if (!file_close(fh))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Worked */
	msg_print("Successfully created a spoiler file.");
}


/*
 * Monster spoilers originally by: smchorse@ringer.cs.utsa.edu (Shawn McHorse)
 */


/*
 * Create a spoiler file for monsters (-SHAWN-)
 */
static void spoil_mon_info(cptr fname)
{
	char buf[1024];
	int i, n;
	u16b why = 2;
	u16b *who;
	int count = 0;

	/* Open the file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump to the spoiler file */
	text_out_hook = text_out_to_file;
	text_out_file = fh;

	/* Dump the header */
	text_out("Monster Spoilers for %s Version %s\n",
			VERSION_MODE_NAME, VERSION_STRING);
	text_out("------------------------------------------\n\n");

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monsters */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->r_speed) who[count++] = (u16b)i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of monsters */
	ang_sort(who, &why, count);

	/* List all monsters in order (except the ghost). */
	for (n = 0; n < count; n++)
	{
		int r_idx = who[n];
		monster_race *r_ptr = &r_info[r_idx];

		/* Prefix */
		if (r_ptr->flags1 & RF1_QUESTOR)
		{
			text_out("[Q] ");
		}
		else if (r_ptr->flags1 & RF1_UNIQUE)
		{
			text_out("[U] ");
		}
		else
		{
			text_out("The ");
		}

		/* Name */
		text_out("%s  (", (r_ptr->name_full));	/* ---)--- */

		/* Color */
		text_out(attr_to_text(r_ptr->d_attr));

		/* Symbol --(-- */
		text_out(" '%c')\n", r_ptr->d_char);

		/* Indent */
		text_out("=== ");

		/* Number */
		text_out("Num:%d  ", r_idx);

		/* Level */
		text_out("Lev:%d  ", r_ptr->level);

		/* Rarity */
		text_out("Rar:%d  ", r_ptr->rarity);

		/* Speed */
		if (calc_energy_gain(r_ptr->r_speed) >= STANDARD_ENERGY_GAIN)
		{
			text_out("Spd:+%d  ", (calc_energy_gain(r_ptr->r_speed) - STANDARD_ENERGY_GAIN));
		}
		else
		{
			text_out("Spd:-%d  ", (STANDARD_ENERGY_GAIN - calc_energy_gain(r_ptr->r_speed)));
		}

		/* Hitpoints */
		text_out("Hp:dice %d sides %d", r_ptr->hdice, r_ptr->hside);

		/* Armor Class */
		text_out("Ac:%d  ", r_ptr->ac);

		/* Experience */
		text_out("Exp:%ld\n", (long)(r_ptr->mexp));

		/* Describe */
		describe_monster(r_idx, TRUE);

		/* Terminate the entry */
		text_out("\n");
	}

	/* Free the "who" array */
	FREE(who);

	/* Check for errors */
	if (!file_close(fh))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	msg_print("Successfully created a spoiler file.");
}


/*
 * Create a spoiler file for monsters (-SHAWN-)
 */
static void spoil_features(cptr fname)
{
	char buf[1024];
	int i, n;
	u16b why = 2;
	u16b *who;
	int count = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* Open the file */
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump to the spoiler file */
	text_out_hook = text_out_to_file;
	text_out_file = fh;

	/* Dump the header */
	strnfmt(buf, sizeof(buf), "Feature Spoilers for %s Version %s\n",
			VERSION_MODE_NAME, VERSION_STRING);
	text_out(buf);
	text_out("------------------------------------------\n\n");

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->f_max, u16b);

	/* Scan the features */
	for (i = 1; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Use that feature */
		if (f_ptr->name) who[count++] = (u16b)i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of features */
	ang_sort(who, &why, count);

	/* List all features in order. */
	for (n = 0; n < count; n++)
	{
		int f_idx = who[n];
		feature_type *f_ptr = &f_info[f_idx];

		/*Output the name*/
		feature_desc(buf, sizeof(buf), f_idx, TRUE, FALSE);

		/* Name */
		strnfmt(buf, sizeof(buf), "%s  (", (f_name + f_ptr->name));	/* ---)--- */
		text_out(buf);

		/* Color */
		text_out(attr_to_text(f_ptr->d_attr));

		/* Symbol --(-- */
		sprintf(buf, " '%c')\n", f_ptr->d_char);
		text_out(buf);

		/* Indent and Number */
		sprintf(buf, "=== Num:%d  ", f_idx);
		text_out(buf);

		/* Describe */
		describe_feature(f_idx, TRUE);

		/* Terminate the entry */
		text_out("\n");
	}

	/* Free the "who" array */
	FREE(who);

	/* Check for errors */
	if (!file_close(fh))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	msg_print("Successfully created a spoiler file.");
}


/*
 * Create Spoiler files
 */
void do_cmd_spoilers(void)
{
	char ch;

	/* Save screen */
	screen_save();


	/* Interact */
	while (TRUE)
	{
		/* Clear screen */
		Term_clear();

		/* Info */
		prt("Create a spoiler file.", 2, 0);

		/* Prompt for a file */
		prt("(1) Brief Object Info (obj-desc.spo)", 5, 5);
		prt("(2) Brief Artifact Info (artifact.spo)", 6, 5);
		prt("(3) Brief Monster Info (mon-desc.spo)", 7, 5);
		prt("(4) Full Monster Info (mon-info.spo)", 8, 5);
		prt("(5) Full Feature Info (feature.spo)", 9, 5);

		/* Prompt */
		prt("Command: ", 12, 0);

		/* Get a choice */
		ch = inkey();

		/* Escape */
		if (ch == ESCAPE)
		{
			break;
		}

		/* Option (1) */
		else if (ch == '1')
		{
			spoil_obj_desc("obj-desc.spo");
		}

		/* Option (2) */
		else if (ch == '2')
		{
			spoil_artifact("artifact.spo");
		}

		/* Option (3) */
		else if (ch == '3')
		{
			spoil_mon_desc("mon-desc.spo");
		}

		/* Option (4) */
		else if (ch == '4')
		{
			spoil_mon_info("mon-info.spo");
		}

		/* Option (4) */
		else if (ch == '5')
		{
			spoil_features("feature.spo");
		}

		/* Oops */
		else
		{
			bell("Illegal command for spoilers!");
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif


/*
 * Hack -- Create a "forged" artifact
 */
bool make_fake_artifact(object_type *o_ptr, byte art_num)
{
	int i;

	artifact_type *a_ptr = &a_info[art_num];

	/* Ignore "empty" artifacts */
	if (a_ptr->tval + a_ptr->sval == 0) return FALSE;

	/* Get the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return (FALSE);

	/* Create the artifact */
	object_prep(o_ptr, i);

	/* Save the name */
	o_ptr->art_num = art_num;

	/* Extract the fields */
	o_ptr->pval = a_ptr->pval;
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;

	/*identify it*/
	object_known(o_ptr);

	/*make it a store item*/
	o_ptr->ident |= IDENT_STORE;

	/* Hack -- extract the "cursed" flag */
	if (a_ptr->a_flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);

	/* Success */
	return (TRUE);
}

