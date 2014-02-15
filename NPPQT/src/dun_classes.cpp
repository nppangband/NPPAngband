/* File: dun_classes.c */

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

//Verify if the dungeon square has an object on it
bool dungeon_type::has_object()
{
    if (object_idx) return (TRUE);
    return (FALSE);
}

//Verify if the dungeon square has an effect on it
bool dungeon_type::has_effect()
{
    if (effect_idx) return (TRUE);
    return (FALSE);
}

//Verify if the dungeon square has a monster object on it
bool dungeon_type::has_monster()
{
    if (monster_idx) return (TRUE);
    return (FALSE);
}

effect_type::effect_type()
{
    effect_wipe();
}

/*
 * Wipe the effect_type class.
 * This function shoudld be used instead of WIPE command.
 * This function does not clear the effect pointer from dungeon_type.
 * It should be called directly only under unusual circumstances.
 */
void effect_type::effect_wipe()
{
    x_type = 0;
    x_f_idx = 0;
    x_cur_y = 0;
    x_cur_x = 0;
    x_countdown = 0;
    x_repeats = 0;
    x_power = 0;
    x_source = 0;
    x_flags = 0;
    next_x_idx = 0;
    x_r_idx = 0;
}


dungeon_type::dungeon_type()
{
    dungeon_square_wipe();
}

/*
 * Wipe the dungeon_type class.
 * This function shoudld be used instead of WIPE command.
 * All variables in dungeon_type should be re-set in this function.
 * This function does not clear the object, effect, and monster lists.
 * It should not be called without first removing any
 * monsters, effects, or objects from their respective lists.
 */
void dungeon_type::dungeon_square_wipe()
{
    feat = effect_idx = monster_idx = 0;
    effect_idx = cave_info = 0;
    special_lighting = obj_special_symbol = 0;
    dtrap = FALSE;
    dun_color.setRgb(0,0,0,0);
    dun_char = '\0';
    object_color.setRgb(0,0,0,0);
    object_char = '\0';
    effect_color.setRgb(0,0,0,0);
    effect_char = '\0';
    monster_color.setRgb(0,0,0,0);
    monster_char = '\0';
}


feature_type::feature_type()
{
    feature_wipe();
}

void feature_type::feature_wipe()
{
    f_name = QString("");
    f_text = QString("");

    f_mimic = f_edge = f_flags1 = f_flags2 = f_flags3 = 0;
    f_level = f_rarity = priority = defaults = f_power = unused = 0;

    for (int i = 0; i < MAX_FEAT_STATES; i++) {
        state[i].fs_action = state[i].fs_power = state[i].fs_result = 0;
    }

    color_num = 0;
    d_color = QColor("black");
    d_char = QChar(' ');

    f_everseen = false;

    x_damage = x_gf_type = x_timeout_rand = x_timeout_set = 0;

    dam_non_native = native_energy_move = non_native_energy_move = native_to_hit_adj =
            non_native_to_hit_adj = f_stealth_adj = 0;
}

vault_type::vault_type()
{
    vault_wipe();
}

void vault_type::vault_wipe()
{
    vault_name.clear();
    vault_text.clear();
    typ = rat = hgt = wid = 0;
}

feature_lore::feature_lore()
{
    feature_lore_wipe();
}


/*
 * Wipe the feature_lore class.
 * This function shoudld be used instead of WIPE command.
 * All variables in feature should be re-set in this function.
 */
void feature_lore::feature_lore_wipe()
{
    f_l_sights = 0;
    f_l_flags1 = f_l_flags2 = f_l_flags3 = 0;
    f_l_defaults = f_l_power = f_l_dam_non_native = f_l_native_moves = 0;
    f_l_non_native_moves = f_l_native_to_hit_adj = f_l_non_native_to_hit_adj = f_l_stealth_adj = 0;
    for (int i = 0; i < MAX_FEAT_STATES; i++)  f_l_state[i] = 0;
}


dynamic_grid_type::dynamic_grid_type()
{
    dynamic_grid_wipe();
}


/*
 * Wipe the feature_lore class.
 * This function shoudld be used instead of WIPE command.
 * All variables in feature should be re-set in this function.
 */
void dynamic_grid_type::dynamic_grid_wipe()
{
    y = x = flags = counter = 0;
}

void reset_dungeon_info()
{
    for (int y = 0; y < MAX_DUNGEON_HGT; y++) {
        for (int x = 0; x < MAX_DUNGEON_WID; x++) {
            dungeon_info[y][x].dungeon_square_wipe();
        }
    }
}
