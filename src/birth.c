/*
 * File: birth.c
 * Purpose: Character creation
 *
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

#include "angband.h"
#include "cmds.h"
#include "game-event.h"
#include "game-cmd.h"
#include "ui-menu.h"

/*
 * Overview
 * ========
 * This file contains the game-mechanical part of the birth process.
 * To follow the code, start at player_birth towards the bottom of
 * the file - that is the only external entry point to the functions
 * defined here.
 *
 * Player (in the Angband sense of character) birth is modelled as a
 * a series of commands from the UI to the game to manipulate the
 * character and corresponding events to inform the UI of the outcomes
 * of these changes.
 *
 * The current aim of this section is that after any birth command
 * is carried out, the character should be left in a playable state.
 * In particular, this means that if a savefile is supplied, the
 * character will be set up according to the "quickstart" rules until
 * another race or class is chosen, or until the stats are reset by
 * the UI.
 *
 * Once the UI signals that the player is happy with the character, the
 * game does housekeeping to ensure the character is ready to start the
 * game (clearing the history log, making sure options are set, etc)
 * before returning control to the game proper.
 */


/*
 * Forward declare
 */
typedef struct birther /*lovely*/ birther; /*sometimes we think she's a dream*/



/*
 * A structure to hold "rolled" information, and any
 * other useful state for the birth process.
 *
 * XXX Demand Obama's birth certificate
 */
struct birther
{
	byte sex;
	byte race;
	byte pclass;

	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[A_MAX];

	char history[250];
};

static void set_moria_options(void)
{
	/* Paranoia */
	if (game_mode != GAME_NPPMORIA) return;

	/* Turn off options unused in Moria */
	auto_scum = FALSE;
	allow_themed_levels = FALSE;
	verify_leave_quest = FALSE;
	birth_maximize = adult_maximize = FALSE;
	birth_rand_artifacts = adult_rand_artifacts = FALSE;
	birth_force_small_lev = adult_force_small_lev = FALSE;
	birth_no_artifacts = adult_no_artifacts = FALSE;
	birth_simple_dungeons = adult_simple_dungeons = TRUE;
	birth_no_xtra_artifacts = adult_no_xtra_artifacts = TRUE;
	birth_no_store_services = adult_no_store_services = TRUE;
	birth_no_player_ghosts = adult_no_player_ghosts = TRUE;
	birth_no_quests = adult_no_quests = TRUE;
	birth_connected_stairs = adult_connected_stairs = FALSE;
	birth_preserve = adult_preserve = TRUE;

}

/*
 * Save the currently rolled data into the supplied 'player'.
 */
static void save_roller_data(birther *player)
{
	int i;

	/* Save the data */
	player->sex = p_ptr->psex;
	player->race = p_ptr->prace;
	player->pclass = p_ptr->pclass;
	player->age = p_ptr->age;
	player->wt = p_ptr->wt_birth;
	player->ht = p_ptr->ht_birth;
	player->sc = p_ptr->sc_birth;
	player->au = p_ptr->au_birth;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		player->stat[i] = p_ptr->stat_birth[i];
	}

	/* Save the history */
	my_strcpy(player->history, p_ptr->history, sizeof(player->history));
}


/*
 * Load stored player data from 'player' as the currently rolled data,
 * optionally placing the current data in 'prev_player' (if 'prev_player'
 * is non-NULL).
 *
 * It is perfectly legal to specify the same "birther" for both 'player'
 * and 'prev_player'.
 */
static void load_roller_data(birther *player, birther *prev_player)
{
	int i;

	/* The initialisation is just paranoia - structure assignment is
	   (perhaps) not strictly defined to work with uninitialised parts
	   of structures. */
	birther temp;
	WIPE(&temp, birther);

	/*** Save the current data if we'll need it later ***/
	if (prev_player)
		save_roller_data(&temp);

	/*** Load the previous data ***/

	/* Load the data */
	p_ptr->psex = player->sex;
	p_ptr->prace = player->race;
	p_ptr->pclass = player->pclass;
	p_ptr->age = player->age;
	p_ptr->wt = p_ptr->wt_birth = player->wt;
	p_ptr->ht = p_ptr->ht_birth = player->ht;
	p_ptr->sc = p_ptr->sc_birth = player->sc;
	p_ptr->au_birth = player->au;
	if (birth_money)
		p_ptr->au = 500;
	else
		p_ptr->au = p_ptr->au_birth;

	/* Load the stats */
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_max[i] = p_ptr->stat_cur[i] = p_ptr->stat_birth[i] = player->stat[i];
	}

	/* Load the history */
	my_strcpy(p_ptr->history, player->history, sizeof(p_ptr->history));


	/*** Save the current data if the caller is interested in it. ***/
	if (prev_player)
		*prev_player = temp;
}


/*
 * Adjust a stat by an amount.
 *
 * This just uses "modify_stat_value()" unless "maximize" mode is false,
 * and a positive bonus is being applied, in which case, a special hack
 * is used.
 */
static int adjust_stat(int value, int amount)
{
	/* Negative amounts or maximize mode */
	if ((amount < 0) || adult_maximize)
	{
		return (modify_stat_value(value, amount));
	}

	/* Special hack */
	else
	{
		int i;

		/* Apply reward */
		for (i = 0; i < amount; i++)
		{
			if (value < 18)
			{
				value++;
			}
			else if (value < 18+70)
			{
				value += randint1(15) + 5;
			}
			else if (value < 18+90)
			{
				value += randint1(6) + 2;
			}
			else if (value < 18+100)
			{
				value++;
			}
		}
	}

	/* Return the result */
	return (value);
}

/*
 * Roll for a character's stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(int stat_use[A_MAX])
{
	int i, j;

	int bonus;

	int dice[18];


	/* Roll and verify some stats */
	while (TRUE)
	{
		/* Roll some dice */
		for (j = i = 0; i < 18; i++)
		{
			/* Roll the dice */
			dice[i] = randint1(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 42) && (j < 54)) break;
	}

	/* Roll the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

		/* Save that value */
		p_ptr->stat_max[i] = j;

		/* Obtain a "bonus" for "race" and "class" */
		bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

		/* Variable stat maxes */
		if (adult_maximize)
		{
			/* Start fully healed */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i];

			/* Efficiency -- Apply the racial/class bonuses */
			stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
		}

		/* Fixed stat maxes */
		else
		{
			/* Apply the bonus to the stat (somewhat randomly) */
			stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}

		p_ptr->stat_birth[i] = p_ptr->stat_max[i];
	}
}


static void roll_hp(void)
{
	int i, j, min_value, max_value;

	/* Minimum hitpoints at highest level */
	min_value = (z_info->max_level * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += z_info->max_level;

	/* Maximum hitpoints at highest level */
	max_value = (z_info->max_level * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += z_info->max_level;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < z_info->max_level; i++)
		{
			j = randint1(p_ptr->hitdie);
			p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (p_ptr->player_hp[z_info->max_level-1] < min_value) continue;
		if (p_ptr->player_hp[z_info->max_level-1] > max_value) continue;

		/* Acceptable */
		break;
	}
}


static void get_bonuses(void)
{
	/* Calculate the bonuses and hitpoints */
	p_ptr->update |= (PU_BONUS | PU_HP);

	/* Update stuff */
	update_stuff();

	/* Fully healed */
	p_ptr->chp = p_ptr->mhp;

	/* Fully rested */
	p_ptr->csp = p_ptr->msp;
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(void)
{
	int i, chart, roll, social_class;


	/* Clear the previous history strings */
	p_ptr->history[0] = '\0';

	/* Initial social class */
	social_class = randint1(4);

	/* Starting place */
	chart = rp_ptr->hist;

	/* Process the history */
	while (chart)
	{
		/* Start over */
		i = 0;

		/* Roll for nobility */
		roll = randint1(100);

		/* Get the proper entry in the table */
		while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

		/* Get the textual history */
		my_strcat(p_ptr->history, (h_text + h_info[i].h_text), sizeof(p_ptr->history));

		/* Add a space */
		my_strcat(p_ptr->history, " ", sizeof(p_ptr->history));

		/* Add in the social class */
		social_class += (int)(h_info[i].bonus) - 50;

		/* Enter the next chart */
		chart = h_info[i].next;
	}

	/* Verify social class */
	if (social_class > 75) social_class = 75;
	else if (social_class < 1) social_class = 1;

	/* Save the social class */
	p_ptr->sc = p_ptr->sc_birth = social_class;
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
	/* Calculate the age */
	p_ptr->age = rp_ptr->b_age + randint1(rp_ptr->m_age);

	/* Calculate the height/weight for males */
	if (p_ptr->psex == SEX_MALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->m_b_wt, rp_ptr->m_m_wt);
	}

	/* Calculate the height/weight for females */
	else if (p_ptr->psex == SEX_FEMALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->f_b_ht, rp_ptr->f_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->f_b_wt, rp_ptr->f_m_wt);
	}
}


/*
 * Get the player's starting money
 */
static void get_money(int stat_use[A_MAX])
{
	if (birth_money)
	{
		p_ptr->au_birth = 200;
		p_ptr->au = 500;
	}
	else
	{
		p_ptr->au = p_ptr->au_birth = 200;
	}
}


/*
 * Clear all the global "character" data
 */
static void player_wipe(void)
{
	int i;

	/* Wipe the player */
	(void)WIPE(p_ptr, player_type);

	/* Clear the inventory */
	for (i = 0; i < ALL_INVEN_TOTAL; i++)
		object_wipe(&inventory[i]);

	/* Start with no artifacts made yet */
	for (i = 0; i < z_info->art_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		a_ptr->a_cur_num = 0;
	}

	/* Wipe the quest */
	guild_quest_wipe(TRUE);

	/* Reset the fixed quests */
	for (i = 0; i < z_info->q_max; i++)
	{
		quest_type *q_ptr = &q_info[i];

		/* Reset level */
		if (quest_fixed(q_ptr))
		{
			q_ptr->q_flags = 0L;
			q_ptr->q_num_killed = 0;
		}
	}

	/* Reset the "objects" */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Reset "tried" */
		k_ptr->tried = FALSE;

		/* Reset "aware" */
		k_ptr->aware = FALSE;
	}

	/* Reset the "monsters" */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Hack -- Reset the counter */
		r_ptr->cur_num = 0;

		/* Hack -- Reset the max counter */
		r_ptr->max_num = 100;

		/* Hack -- Reset the max counter */
		if (r_ptr->flags1 & (RF1_UNIQUE)) r_ptr->max_num = 1;

		/* Clear player kills */
		l_ptr->pkills = 0;
	}

	/*No current player ghosts*/
	player_ghost_num = -1;
	ghost_r_idx = 0;

	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;

	/*re-set the altered inventory counter*/
	allow_altered_inventory = FALSE;
	altered_inventory_counter = 0;

	/* None of the spells have been learned yet */
	for (i = 0; i < PY_MAX_SPELLS; i++) p_ptr->spell_order[i] = 99;
}


/**
 * Try to wield everything in the inventory.
 */
static void wield_all(void)
{
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	int slot;
	int item;
	int num;
	bool is_ammo;


	/* Scan through the slots backwards */
	for (item = INVEN_PACK - 1; item >= 0; item--)
	{
		o_ptr = &inventory[item];
		is_ammo = obj_is_ammo(o_ptr);

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Make sure we can wield it */
		slot = wield_slot(o_ptr);
		if (slot < INVEN_WIELD) continue;
		i_ptr = &inventory[slot];

		/* Make sure that there's an available slot */
		if (is_ammo)
		{
			if (i_ptr->k_idx && !object_similar(o_ptr, i_ptr)) continue;
		}
		else
		{
			if (i_ptr->k_idx) continue;
		}

		/* Figure out how much of the item we'll be wielding */
		num = is_ammo ? o_ptr->number : 1;

		/* Get local object */
		i_ptr = &object_type_body;
		object_copy(i_ptr, o_ptr);

		/* Modify quantity */
		i_ptr->number = num;

		/* Decrease the item (from the pack) */
		inven_item_increase(item, -num);
		inven_item_optimize(item);

		/* Get the wield slot */
		o_ptr = &inventory[slot];

		/* Wear the new stuff */
		object_copy(o_ptr, i_ptr);

	}

	save_quiver_size();

	return;
}


/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(void)
{
	int i;
	const start_item *e_ptr;
	object_type *i_ptr;
	object_type object_type_body;


	/* Hack -- Give the player his equipment */
	for (i = 0; i < MAX_START_ITEMS; i++)
	{
		/* Access the item */
		e_ptr = &(cp_ptr->start_items[i]);

		/* Get local object */
		i_ptr = &object_type_body;

		/* Hack	-- Give the player an object */
		if (e_ptr->tval > 0)
		{
			/* Get the object_kind */
			int k_idx = lookup_kind(e_ptr->tval, e_ptr->sval);

			/* Valid item? */
			if (!k_idx) continue;

			/* Prepare the item */
			object_prep(i_ptr, k_idx);
			i_ptr->number = (byte)rand_range(e_ptr->min, e_ptr->max);

			object_aware(i_ptr);
			object_known(i_ptr);
			(void)inven_carry(i_ptr);
			k_info[k_idx].everseen = TRUE;

			apply_autoinscription(i_ptr);

			/* Remember history */
			object_history(i_ptr, ORIGIN_BIRTH, 0);
		}
	}

	/* Hack -- give the player hardcoded equipment XXX */

	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some food */
	object_prep(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
	i_ptr->number = (byte)rand_range(3, 7);
	object_aware(i_ptr);
	object_known(i_ptr);
	k_info[i_ptr->k_idx].everseen = TRUE;
	apply_autoinscription(i_ptr);
	/* Remember history */
	object_history(i_ptr, ORIGIN_BIRTH, 0);
	(void)inven_carry(i_ptr);

	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some torches */
	object_prep(i_ptr, lookup_kind(TV_LIGHT, SV_LIGHT_TORCH));
	apply_magic(i_ptr, 0, FALSE, FALSE, FALSE, FALSE);
	i_ptr->number = (byte)rand_range(3, 7);
	object_aware(i_ptr);
	object_known(i_ptr);
	k_info[i_ptr->k_idx].everseen = TRUE;
	apply_autoinscription(i_ptr);
	/* Remember history */
	object_history(i_ptr, ORIGIN_BIRTH, 0);
	(void)inven_carry(i_ptr);

	/* Now try wielding everything */
	wield_all();
}


/*
 * Cost of each "point" of a stat.
 */
static const int birth_stat_costs[18 + 1] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 4 };

/* It was feasible to get base 17 in 3 stats with the autoroller */
#define MAX_BIRTH_POINTS 24 /* 3 * (1+1+1+1+1+1+2) */


static void recalculate_stats(int *stats, int points_left)
{
	int i;

	/* Process stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Variable stat maxes */
		if (adult_maximize)
		{
			/* Reset stats */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = p_ptr->stat_birth[i] = stats[i];
		}

		/* Fixed stat maxes */
		else
		{
			/* Obtain a "bonus" for "race" and "class" */
			int bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

			/* Apply the racial/class bonuses */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] =
				modify_stat_value(stats[i], bonus);
		}
	}

	/* Gold is inversely proportional to cost */
	if (birth_money)
		p_ptr->au = 500;
	else
		p_ptr->au = 200 + (50 * points_left);

	p_ptr->au_birth = 200 + (50 * points_left);

	/* Update bonuses, hp, etc. */
	get_bonuses();

	/* Tell the UI about all this stuff that's changed. */
	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);
}

static void reset_stats(int stats[A_MAX], int points_spent[A_MAX], int *points_left)
{
	int i;

	/* Calculate and signal initial stats and points totals. */
	*points_left = MAX_BIRTH_POINTS;

	for (i = 0; i < A_MAX; i++)
	{
		/* Initial stats are all 10 and costs are zero */
		stats[i] = 10;
		points_spent[i] = 0;
	}

	/* Use the new "birth stat" values to work out the "other"
	   stat values (i.e. after modifiers) and tell the UI things have
	   changed. */
	recalculate_stats(stats, *points_left);
	event_signal_birthpoints(points_spent, *points_left);
}

static bool buy_stat(int choice, int stats[A_MAX], int points_spent[A_MAX],
					 int *points_left)
{
	/* Must be a valid stat, and have a "base" of below 18 to be adjusted */
	if (!(choice >= A_MAX || choice < 0) &&	(stats[choice] < 18))
	{
		/* Get the cost of buying the extra point (beyond what
		   it has already cost to get this far). */
		int stat_cost = birth_stat_costs[stats[choice] + 1];

		if (stat_cost <= *points_left)
		{
			stats[choice]++;
			points_spent[choice] += stat_cost;
			*points_left -= stat_cost;

			/* Tell the UI the new points situation. */
			event_signal_birthpoints(points_spent, *points_left);

			/* Recalculate everything that's changed because
			   the stat has changed, and inform the UI. */
			recalculate_stats(stats, *points_left);

			return TRUE;
		}
	}

	/* Didn't adjust stat. */
	return FALSE;
}


static bool sell_stat(int choice, int stats[A_MAX], int points_spent[A_MAX],
					  int *points_left)
{
	/* Must be a valid stat, and we can't "sell" stats below the base of 10. */
	if (!(choice >= A_MAX || choice < 0) && (stats[choice] > 10))
	{
		int stat_cost = birth_stat_costs[stats[choice]];

		stats[choice]--;
		points_spent[choice] -= stat_cost;
		*points_left += stat_cost;

		/* Tell the UI the new points situation. */
		event_signal_birthpoints(points_spent, *points_left);

		/* Recalculate everything that's changed because
		   the stat has changed, and inform the UI. */
		recalculate_stats(stats, *points_left);

		return TRUE;
	}

	/* Didn't adjust stat. */
	return FALSE;
}


/*
 * This picks some reasonable starting values for stats based on the
 * current race/class combo, etc.  For now I'm disregarding concerns
 * about role-playing, etc, and using the simple outline from
 * http://angband.oook.cz/forum/showpost.php?p=17588&postcount=6:
 *
 * 0. buy base STR 17
 * 1. if possible buy adj DEX of 18/10
 * 2. spend up to half remaining points on each of spell-stat and con,
 *    but only up to max base of 16 unless a pure class
 *    [mage or priest or warrior]
 * 3. If there are any points left, spend as much as possible in order
 *    on DEX, non-spell-stat, CHR.
 */
static void generate_stats(int stats[A_MAX], int points_spent[A_MAX],
						   int *points_left)
{
	int step = 0;
	int maxed[A_MAX] = { 0 };
	bool pure = FALSE;

	/* Determine whether the class is "pure" */
	if (cp_ptr->spell_book == 0 || cp_ptr-> max_attacks < 5)
	{
		pure = TRUE;
	}

	while (*points_left && step >= 0)
	{
		switch (step)
		{
			/* Buy base STR 17 */
			case 0:
			{
				if (!maxed[A_STR] && stats[A_STR] < 17)
				{
					if (!buy_stat(A_STR, stats, points_spent, points_left))
						maxed[A_STR] = TRUE;
				}
				else
				{
					step++;
				}

				break;
			}

			/* Try and buy adj DEX of 18/10 */
			case 1:
			{
				if (!maxed[A_DEX] && p_ptr->state.stat_top[A_DEX] < 18+10)
				{
					if (!buy_stat(A_DEX, stats, points_spent, points_left))
						maxed[A_DEX] = TRUE;
				}
				else
				{
					step++;
				}

				break;
			}

			/* If we can't get 18/10 dex, sell it back. */
			case 2:
			{
				if (p_ptr->state.stat_top[A_DEX] < 18+10)
				{
					while (stats[A_DEX] > 10)
						sell_stat(A_DEX, stats, points_spent, points_left);

					maxed[A_DEX] = FALSE;
				}

				step++;

				break;
			}

			/*
			 * Spend up to half remaining points on each of spell-stat and
			 * con, but only up to max base of 16 unless a pure class
			 * [mage or priest or warrior]
			 */
			case 3:
			{
				int points_trigger = *points_left / 2;

				if (cp_ptr->spell_book)
				{
					int spell_stat;

					if (cp_ptr->spell_book == TV_MAGIC_BOOK) spell_stat = A_INT;
					else if (cp_ptr->spell_book == TV_PRAYER_BOOK) spell_stat = A_WIS;
					/*ugly hack for the druid and ranger class, which have 2 spell stats*/
					else if (stats[A_INT] > stats[A_WIS]) spell_stat = A_WIS;
					else /*(stats[A_WIS] >= stats[A_INT])*/ spell_stat = A_INT;



					while (!maxed[spell_stat] &&
						   (pure || stats[spell_stat] < 16) &&
						   points_spent[spell_stat] < points_trigger)
					{
						if (!buy_stat(spell_stat, stats, points_spent,
									  points_left))
						{
							maxed[spell_stat] = TRUE;
						}

						if (points_spent[spell_stat] > points_trigger)
						{
							sell_stat(spell_stat, stats, points_spent,
									  points_left);
							maxed[spell_stat] = TRUE;
						}
					}
				}

				while (!maxed[A_CON] &&
					   (pure || stats[A_CON] < 16) &&
					   points_spent[A_CON] < points_trigger)
				{
					if (!buy_stat(A_CON, stats, points_spent,points_left))
					{
						maxed[A_CON] = TRUE;
					}

					if (points_spent[A_CON] > points_trigger)
					{
						sell_stat(A_CON, stats, points_spent, points_left);
						maxed[A_CON] = TRUE;
					}
				}

				step++;
				break;
			}

			/*
			 * If there are any points left, spend as much as possible in
			 * order on DEX, non-spell-stat, CHR.
			 */
			case 4:
			{
				int next_stat;

				if (!maxed[A_DEX])
				{
					next_stat = A_DEX;
				}
				else if (!maxed[A_INT] && cp_ptr->spell_book != TV_MAGIC_BOOK)
				{
					next_stat = A_INT;
				}
				else if (!maxed[A_WIS] && cp_ptr->spell_book != TV_PRAYER_BOOK)
				{
					next_stat = A_WIS;
				}
				else if (!maxed[A_CHR])
				{
					next_stat = A_CHR;
				}
				else
				{
					step++;
					break;
				}

				/* Buy until we can't buy any more. */
				while (buy_stat(next_stat, stats, points_spent, points_left));
				maxed[next_stat] = TRUE;

				break;
			}

			default:
			{
				step = -1;
				break;
			}
		}
	}
}


/*
 * This fleshes out a full player based on the choices currently made,
 * and so is called whenever things like race or class are chosen.
 */
static void generate_player(void)
{
	/* Set sex according to p_ptr->sex */
	sp_ptr = &sex_info[p_ptr->psex];

	/* Set class according to p_ptr->class */
	cp_ptr = &c_info[p_ptr->pclass];
	mp_ptr = &cp_ptr->spells;

	/* Set race according to p_ptr->race */
	rp_ptr = &p_info[p_ptr->prace];

	/* Level 1 */
	p_ptr->max_lev = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

	/* Initial hitpoints */
	p_ptr->mhp = p_ptr->hitdie;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Roll for age/height/weight */
	get_ahw();

	get_history();
}


/* Reset everything back to how it would be on loading the game. */
static void do_birth_reset(bool use_quickstart, birther *quickstart_prev)
{
	player_wipe();

	/* If there's quickstart data, we use it to set default
	   character choices. */
	if (use_quickstart && quickstart_prev)
		load_roller_data(quickstart_prev, NULL);

	generate_player();

	/* Update stats with bonuses, etc. */
	get_bonuses();
}


/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth(bool quickstart_allowed)
{
	int i, n;
	game_command cmd = { CMD_NULL, 0, {{0}} };

	int stats[A_MAX];
	int points_spent[A_MAX];
	int points_left;
	char *buf;
	int success;

	bool rolled_stats = FALSE;

	/*
	 * The last character displayed, to allow the user to flick between two.
	 * We rely on prev.age being zero to determine whether there is a stored
	 * character or not, so initialise it here.
	 */
	birther prev = { 0, 0, 0, 0, 0, 0, 0, 0, {0}, "" };

	/*
	 * If quickstart is allowed, we store the old character in this,
	 * to allow for it to be reloaded if we step back that far in the
	 * birth process.
	 */
	birther quickstart_prev = {0, 0, 0, 0, 0, 0, 0, 0, {0}, "" };

	/* Turn off many options for Moria */
	if (game_mode == GAME_NPPMORIA) set_moria_options();

	/*
	 * If there's a quickstart character, store it for later use.
	 * If not, default to whatever the first of the choices is.
	 */
	if (quickstart_allowed)
		save_roller_data(&quickstart_prev);
	else
	{
		p_ptr->psex = 0;
		p_ptr->pclass = 0;
		p_ptr->prace = 0;
		generate_player();
	}

	reset_stats(stats, points_spent, &points_left);
	do_birth_reset(quickstart_allowed, &quickstart_prev);

	/* Handle incrementing name suffix */
	buf = find_roman_suffix_start(op_ptr->full_name);

	if (buf)
	{
		/* Try to increment the Roman suffix */
		success = int_to_roman((roman_to_int(buf) + 1), buf,
			(sizeof(op_ptr->full_name) - (buf -
			(char *)&op_ptr->full_name)));

		if (!success) msg_print("Sorry, could not deal with suffix");
	}

	/* We're ready to start the interactive birth process. */
	event_signal_flag(EVENT_ENTER_BIRTH, quickstart_allowed);
	event_signal(EVENT_INIT_STATUSLINE);

	/*
	 * Loop around until the UI tells us we have an acceptable character.
	 * Note that it is possible to quit from inside this loop.
	 */
	while (cmd.command != CMD_ACCEPT_CHARACTER)
	{
		/* Grab a command from the queue - we're happy to wait for it. */
		if (cmd_get(CMD_BIRTH, &cmd, TRUE) != 0) continue;

		if (cmd.command == CMD_BIRTH_RESET)
		{
			reset_stats(stats, points_spent, &points_left);
			do_birth_reset(quickstart_allowed, &quickstart_prev);
			rolled_stats = FALSE;
		}
		else if (cmd.command == CMD_CHOOSE_SEX)
		{
			p_ptr->psex = cmd.args[0].choice;
			generate_player();
		}
		else if (cmd.command == CMD_CHOOSE_RACE)
		{
			p_ptr->prace = cmd.args[0].choice;

			generate_player();

			reset_stats(stats, points_spent, &points_left);

			generate_stats(stats, points_spent, &points_left);

			rolled_stats = FALSE;
		}
		else if (cmd.command == CMD_CHOOSE_CLASS)
		{
			p_ptr->pclass = cmd.args[0].choice;
			generate_player();

			reset_stats(stats, points_spent, &points_left);
			generate_stats(stats, points_spent, &points_left);
			rolled_stats = FALSE;
		}
		else if (cmd.command == CMD_CHOOSE_OPTIONS)
		{
			if (cmd.args[0].choice == 1)
			{
				do_cmd_options_aux((void*)3, "Birth (difficulty) options");
			}
		}
		else if (cmd.command == CMD_BUY_STAT)
		{
			/* .choice is the stat to buy */
			if (!rolled_stats)
				buy_stat(cmd.args[0].choice, stats, points_spent, &points_left);
		}
		else if (cmd.command == CMD_SELL_STAT)
		{
			/* .choice is the stat to sell */
			if (!rolled_stats)
				sell_stat(cmd.args[0].choice, stats, points_spent, &points_left);
		}
		else if (cmd.command == CMD_RESET_STATS)
		{
			/* .choice is whether to regen stats */
			reset_stats(stats, points_spent, &points_left);

			if (cmd.args[0].choice)
				generate_stats(stats, points_spent, &points_left);

			rolled_stats = FALSE;
		}
		else if (cmd.command == CMD_ROLL_STATS)
		{
			int i;

			save_roller_data(&prev);

			/* Get a new character */
			get_stats(stats);

			/* Roll for gold */
			get_money(stats);

			/* Update stats with bonuses, etc. */
			get_bonuses();

			/* There's no real need to do this here, but it's tradition. */
			get_ahw();
			get_history();

			event_signal(EVENT_GOLD);
			event_signal(EVENT_AC);
			event_signal(EVENT_HP);
			event_signal(EVENT_STATS);

			/* Give the UI some dummy info about the points situation. */
			points_left = 0;
			for (i = 0; i < A_MAX; i++)
			{
				points_spent[i] = 0;
			}

			event_signal_birthpoints(points_spent, points_left);

			/* Lock out buying and selling of stats based on rolled stats. */
			rolled_stats = TRUE;
		}
		else if (cmd.command == CMD_PREV_STATS)
		{
			/* Only switch to the stored "previous"
			   character if we've actually got one to load. */
			if (prev.age)
			{
				load_roller_data(&prev, &prev);
				get_bonuses();
			}

			event_signal(EVENT_GOLD);
			event_signal(EVENT_AC);
			event_signal(EVENT_HP);
			event_signal(EVENT_STATS);
		}
		else if (cmd.command == CMD_NAME_CHOICE)
		{
			/* Set player name */
			my_strcpy(op_ptr->full_name, cmd.args[0].string,
					  sizeof(op_ptr->full_name));

			string_free((void *) cmd.args[0].string);

			/* Don't change savefile name.  If the UI
			   wants it changed, they can do it. XXX (Good idea?) */
			process_player_name(FALSE);
		}
		/* Various not-specific-to-birth commands. */
		else if (cmd.command == CMD_HELP)
		{
			char buf[80];

			strnfmt(buf, sizeof(buf), "birth.txt");
			screen_save();
			show_file(buf, NULL, 0, 0);
			screen_load();
		}
		else if (cmd.command == CMD_QUIT)
		{
			quit(NULL);
		}
	}

	roll_hp();

	/* Set adult options from birth options */
	for (i = OPT_BIRTH; i < OPT_CHEAT; i++)
	{
		op_ptr->opt[OPT_ADULT + (i - OPT_BIRTH)] = op_ptr->opt[i];
	}

	/* Reset score options from cheat options */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
	{
		op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = op_ptr->opt[i];
	}


	/*Re-set the squelch settings.  Spellbooks are never_pickup by default. */
	for (i = 0; i < z_info->k_max; i++)
	{
		/* Analyze the item type */
		switch (k_info[i].tval)
		{
			case TV_MAGIC_BOOK:
			case TV_PRAYER_BOOK:
			case TV_DRUID_BOOK:
			{
				k_info[i].squelch = NO_SQUELCH_NEVER_PICKUP;

				break;
			}
			default:
			{
				k_info[i].squelch = SQUELCH_NEVER;

				break;
			}
		}
	}

	/*Clear the squelch bytes*/
	for (i = 0; i < SQUELCH_BYTES; i++)
	{
		squelch_level[i] = SQUELCH_NONE;
	}
	/* Clear the ego-item squelching flags */
	for (i = 0; i < z_info->e_max; i++)
	{
		e_info[i].squelch = FALSE;
	}

	/* Make a note file if that option is set */
	if (adult_take_notes)
	{

		/* Variables */
		char long_day[25];
		time_t ct = time((time_t*)0);

		/* Open the file (notes_file and notes_fname are global) */
		create_notes_file();

		if (!notes_file) quit("Can't create the notes file");

		/* Get date */
		(void)strftime(long_day, 25, "%m/%d/%Y at %I:%M %p", localtime(&ct));

		/* Add in "character start" information */
		file_putf(notes_file, "{{full_character_name}} the %s %s\n",
								p_name + rp_ptr->name,
								c_name + cp_ptr->name);
		file_putf(notes_file, "Began the quest to kill Morgoth on %s\n",long_day);
		file_putf(notes_file, "============================================================\n");
		file_putf(notes_file, "                   CHAR.  \n");
		file_putf(notes_file, "|   TURN  | DEPTH |LEVEL| EVENT\n");
		file_putf(notes_file, "============================================================\n");

		/* Paranoia. Remove the notes from memory */
		file_flush(notes_file);
 	}

	/* Note player birth in the message recall */
	message_add(" ", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add("====================", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add(" ", MSG_GENERIC);

	/* Reset message prompt (i.e. no extraneous -more-s) */
	msg_flag = TRUE;

	/* Hack -- outfit the player */
	if (!birth_money) player_outfit();

	/* Shops */
	for (n = 0; n < MAX_STORES; n++)
	{
		/* Initialize */
		store_init(n);

		/* Ignore home */
		if ((n == STORE_HOME) || (n ==STORE_GUILD)) continue;

		/* Maintain the shop (ten times) */
		for (i = 0; i < 10; i++) store_maint(n);

	}

	/* Now we're really done.. */
	event_signal(EVENT_LEAVE_BIRTH);
	event_signal(EVENT_REMOVE_STATUSLINE);
}

