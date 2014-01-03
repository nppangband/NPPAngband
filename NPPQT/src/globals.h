#ifndef GLOBALS_H
#define GLOBALS_H

#include "src/structures.h"
#include <QString>
#include <QFile>

//was externs.h

extern byte game_mode;

extern QString current_savefile;
extern QFile notes_file;


//Various game arrays and lists
extern maxima *z_info;
extern object_type *o_list;
extern monster_type *mon_list;
extern monster_lore *l_list;
extern effect_type *x_list;
extern const player_sex *sp_ptr;
extern const player_race *rp_ptr;
extern player_class *cp_ptr;
extern player_magic *mp_ptr;
extern player_other *op_ptr;
extern player_type *p_ptr;
extern vault_type *v_info;
extern feature_type *f_info;
extern feature_lore *f_l_list;
extern object_kind *k_info;
extern ghost_template *t_info;
extern artifact_type *a_info;
extern artifact_lore *a_l_list;
extern ego_item_type *e_info;
extern monster_race *r_info;
extern player_race *p_info;
extern player_class *c_info;
extern hist_type *h_info;
extern owner_type *b_info;
extern byte *g_info;
extern flavor_type *flavor_info;
extern quest_type *q_info;
extern names_type *n_info;
extern store_type *store;
extern object_type *inventory;
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern s16b alloc_ego_size;
extern alloc_entry *alloc_ego_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern s16b alloc_feat_size;
extern alloc_entry *alloc_feat_table;

extern option_entry options[OPT_MAX];

//Various directories used by NPP
extern QString NPP_DIR_BASE;
extern QString NPP_DIR_EDIT;
extern QString NPP_DIR_HELP;
extern QString NPP_DIR_ICON;
extern QString NPP_DIR_PREF;
extern QString NPP_DIR_SAVE;
extern QString NPP_DIR_USER;

// Monser race messages
extern monster_race_message *mon_msg;
extern monster_message_history *mon_message_hist;
extern u16b size_mon_msg;
extern u16b size_mon_hist;

//monster movement information
extern move_moment_type *mon_moment_info;

// Arrays for cave information
extern int view_n;
extern u16b *view_g;
extern int fire_n;
extern u16b *fire_g;
extern int temp_n;
extern u16b *temp_g;
extern byte *temp_y;
extern byte *temp_x;
extern u16b (*cave_info)[256];
extern byte (*cave_feat)[MAX_DUNGEON_WID];
extern s16b (*cave_o_idx)[MAX_DUNGEON_WID];
extern s16b (*cave_m_idx)[MAX_DUNGEON_WID];
extern s16b (*cave_x_idx)[MAX_DUNGEON_WID];
extern u32b mon_power_ave[MAX_DEPTH_ALL][CREATURE_TYPE_MAX];

extern dynamic_grid_type *dyna_g;
extern u16b dyna_cnt;
extern u16b dyna_next;
extern bool dyna_full;
extern byte dyna_center_y;
extern byte dyna_center_x;

extern u16b cave_cost[MAX_FLOWS][MAX_DUNGEON_HGT][MAX_DUNGEON_WID];
extern int cost_at_center[MAX_FLOWS];


#endif // GLOBALS_H
