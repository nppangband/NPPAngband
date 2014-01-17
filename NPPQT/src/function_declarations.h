#ifndef FUNCTION_DECLARATIONS_H
#define FUNCTION_DECLARATIONS_H

#include "src/structures.h"
#include <QString>
#include <QFile>

//was externs.h


//calcs.c
extern byte calc_energy_gain(byte speed);

//cave.cpp
extern void update_los_proj_move(int y, int x);
extern int distance(int y1, int x1, int y2, int x2);

// cmd4.cpp
extern void create_notes_file(void);
extern void delete_notes_file(void);

/* effect.cpp */
extern void effect_prep(int x_idx, byte type, u16b f_idx, byte y, byte x, byte countdown,
                            byte repeats, u16b power, s16b source, u16b flags);
extern void effect_wipe(effect_type *x_ptr);
extern s16b x_pop(void);

//feature.cpp
extern bool add_dynamic_terrain(byte y, byte x);
extern void update_level_flag(void);
extern u32b get_level_flag(u16b feat);
extern u32b get_level_flag_from_race(monster_race *r_ptr);
extern QString describe_one_level_flag(u32b flag);
extern void debug_all_level_flags(u32b flags);

//generate.cpp
extern void set_dungeon_type(u16b dungeon_type);

//globals.cpp
extern s16b num_repro;
extern s16b object_level;
extern s16b monster_level;
extern byte object_generation_mode;

/* init2.c */
extern void init_npp_games(void);
extern void cleanup_npp_games(void);

/* load.c */
extern bool load_player(void);
extern bool load_gamemode(void);

//mon_ranged_attack.cpp
extern int get_dam(monster_race *r_ptr, int attack);
extern int get_breath_dam(s16b hit_points, int gf_type, bool powerful);
extern int get_ball_beam_dam(int m_idx, monster_race *r_ptr, int attack, int gf_type, bool powerful);

//obj_util.cpp
extern bool object_has_hidden_powers(const object_type *o_ptr);
extern void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native);
extern bool wearable_p(const object_type *o_ptr);
extern s16b lookup_kind(int tval, int sval);
extern s16b o_pop(void);
extern void object_wipe(object_type *o_ptr);
extern void object_copy(object_type *o_ptr, const object_type *j_ptr);
extern int quiver_space_per_unit(const object_type *o_ptr);
extern void save_quiver_size(void);

//player_ghost.cpp
extern void prepare_ghost_name(void);
extern bool prepare_ghost(int r_idx);
extern void ghost_challenge(void);
extern void remove_player_ghost(void);
extern void delete_player_ghost_entry(void);
extern void add_player_ghost_entry(void);
extern void load_player_ghost_file(void);
extern void save_player_ghost_file(void);

/* randart.c */
extern void make_random_name(QString *random_name, byte min, byte max);
extern s32b artifact_power(int a_idx);
extern void build_randart_tables(void);
extern void free_randart_tables(void);
extern int do_randart(u32b randart_seed, bool full);
extern bool make_one_randart(object_type *o_ptr, int art_power, bool namechoice);
extern void make_quest_artifact(int lev);
extern void create_quest_artifact(object_type *o_ptr);
extern void artifact_wipe(int a_idx, bool quest_art_wipe);
extern bool can_be_randart(const object_type *o_ptr);

/* save.c */
extern bool save_player(void);

/* spells2.c */
extern bool hp_player(int num);

//squelch.cpp
extern void rearrange_stack(int y, int x);

//tables.cpp
extern const byte moria_class_level_adj[MORIA_MAX_CLASS][MORIA_MAX_LEV_ADJ];
extern const byte moria_blows_table[MORIA_MAX_STR_ADJ][MORIA_MAX_DEX_ADJ];
extern const s16b ddd[9];
extern const s16b ddx[10];
extern const s16b ddy[10];
extern const s16b ddx_ddd[9];
extern const s16b ddy_ddd[9];
extern const QChar hexsym[16];
extern const int adj_mag_study[];
extern const int adj_mag_mana[];
extern const byte adj_mag_fail[];
extern const int adj_mag_stat[];
extern const byte adj_chr_gold[];
extern const s16b adj_chr_charm[];
extern const byte adj_int_dev[];
extern const byte adj_wis_sav[];
extern const byte adj_dex_dis[];
extern const byte adj_int_dis[];
extern const byte adj_dex_ta[];
extern const byte adj_str_td[];
extern const byte adj_dex_th[];
extern const byte adj_str_th[];
extern const byte adj_str_wgt[];
extern const byte adj_str_hold[];
extern const byte adj_str_dig[];
extern const byte adj_str_blow[];
extern const byte adj_dex_blow[];
extern const byte adj_dex_safe[];
extern const byte adj_con_fix[];
extern const int adj_con_mhp[];
extern const byte blows_table[12][12];
extern const byte extract_energy_nppmoria[6];
extern const byte extract_energy_nppangband[200];
extern const s32b player_exp_nppangband[PY_MAX_LEVEL];
extern const s32b player_exp_nppmoria[PY_MAX_LEVEL_MORIA];
extern const player_sex sex_info[MAX_SEXES];
extern const s16b spell_list_nppmoria_mage[BOOKS_PER_REALM_MORIA][SPELLS_PER_BOOK];
extern const s16b spell_list_nppmoria_priest[BOOKS_PER_REALM_MORIA][SPELLS_PER_BOOK];
extern const s16b spell_list_nppangband_mage[BOOKS_PER_REALM_ANGBAND][SPELLS_PER_BOOK];
extern const s16b spell_list_nppangband_priest[BOOKS_PER_REALM_ANGBAND][SPELLS_PER_BOOK];
extern const s16b spell_list_nppangband_druid[BOOKS_PER_REALM_ANGBAND][SPELLS_PER_BOOK];
extern QString feeling_themed_level[LEV_THEME_TAIL];
extern const byte chest_traps[64];
extern QString color_names[16];
extern QString stat_names[A_MAX];
extern QString stat_names_reduced[A_MAX];
extern QString stat_names_full[A_MAX];
extern QString window_flag_desc[32];
extern option_entry options[OPT_MAX];
extern byte spell_info_RF4[32][5];
extern byte spell_info_RF5[32][5];
extern byte spell_info_RF6[32][5];
extern byte spell_info_RF7[32][5];
extern byte spell_desire_RF4[32][8];
extern byte spell_desire_RF5[32][8];
extern byte spell_desire_RF6[32][8];
extern byte spell_desire_RF7[32][8];
extern const colors_preset preset_colors[MAX_COLORS];


// Utilities.cpp
extern int letter_to_number (QChar let);
extern QChar number_to_letter (int num);
extern bool is_a_vowel(QChar single_letter);
extern void pop_up_message_box(QString message);



#endif // FUNCTION_DECLARATIONS_H
