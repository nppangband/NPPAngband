#ifndef FUNCTION_DECLARATIONS_H
#define FUNCTION_DECLARATIONS_H

#include "src/structures.h"
#include <QString>
#include <QFile>

//was externs.h


//calcs.c
extern byte calc_energy_gain(byte speed);

//cave.cpp
extern int distance(int y1, int x1, int y2, int x2);
extern bool generic_los(int y1, int x1, int y2, int x2, u16b flg);
extern bool no_light(void);
extern bool cave_valid_bold(int y, int x);
extern byte multi_hued_attr(monster_race *r_ptr);
extern bool feat_supports_lighting(u16b feat);
extern bool dtrap_edge(int y, int x);
extern void map_info(s16b y, s16b x);
extern int vinfo_init(void);
extern void forget_view(void);
extern void update_view(void);
extern void update_flows(bool full);
extern void wiz_light(void);
extern void wiz_dark(void);
extern void town_illuminate(bool daytime);
extern void update_los_proj_move(int y, int x);
extern void cave_alter_feat(int y, int x, int action);
extern void cave_set_feat(int y, int x, u16b feat);
extern int  project_path(u16b *path_g, u16b * path_gx, int range, int y1, int x1, int *y2, int *x2, u32b flg);
extern byte projectable(int y1, int x1, int y2, int x2, u32b flg);
extern void scatter(int *yp, int *xp, int y, int x, int d, int m);
extern void health_track(int m_idx);
extern void track_object(int item);
extern void track_object_kind(int k_idx);
extern void monster_race_track(int r_idx);
extern void feature_kind_track(int f_idx);
extern void disturb(int stop_search, int unused_flag);

// cmd4.cpp
extern void create_notes_file(void);
extern void delete_notes_file(void);

// cmd_spell.cpp
extern bool spell_okay(int spell, bool known);
extern s16b get_spell_from_list(s16b book, s16b spell);
extern int get_spell_index(int sval, int index);

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



/* init2.c */
extern void init_npp_games(void);
extern void cleanup_npp_games(void);
extern void flavor_init(void);

/* load.c */
extern bool load_player(void);
extern bool load_gamemode(void);

//mon_classes.cpp
extern void lore_treasure(int m_idx, int num_item, int num_gold);

//mon_ranged_attack.cpp
extern int get_dam(monster_race *r_ptr, int attack);
extern int get_breath_dam(s16b hit_points, int gf_type, bool powerful);
extern int get_ball_beam_dam(int m_idx, monster_race *r_ptr, int attack, int gf_type, bool powerful);

/* object_desc.c */
extern QString strip_name(int k_idx);
extern QString object_desc(object_type *o_ptr, byte mode);
extern QString object_desc_spoil(object_type *o_ptr);



/* object-make.cpp */
extern s16b get_obj_num(int level);
extern void object_prep(object_type *o_ptr, int k_idx);
extern s16b charge_wand(object_type *o_ptr, int percent);
extern s16b charge_staff(object_type *o_ptr, int percent);
extern void object_into_artifact(object_type *o_ptr, artifact_type *a_ptr);
extern void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great, bool interesting);
extern void object_quantities(object_type *j_ptr);
extern bool make_object(object_type *j_ptr, bool good, bool great, int objecttype, bool interesting);
extern bool prep_store_object(int storetype);
extern bool prep_object_theme(int themetype);
extern int get_object_mimic_k_idx(const monster_race *r_ptr);
extern bool make_gold(object_type *j_ptr);
extern void place_object(int y, int x, bool good, bool great, int droptype);
extern bool place_quest_artifact(int y, int x);
extern void place_gold(int y, int x);
extern int  get_coin_type(const monster_race *r_ptr);
extern void steal_object_from_monster(int y, int x);
extern void acquirement(int y1, int x1, int num, bool great);
extern void create_food(void);


//obj_util.cpp
extern void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native);
extern void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *native);
extern QChar index_to_label(int i);
extern s16b wield_slot_ammo(object_type *o_ptr);
extern s16b wield_slot(object_type *o_ptr);
extern bool ammo_inscribed_for_quiver(object_type *o_ptr);
extern bool weapon_inscribed_for_quiver(object_type *o_ptr);
extern bool slot_can_wield_item(int slot, object_type *o_ptr);
extern QString mention_use(int i);
extern QString describe_use(int i);
extern bool item_tester_okay(object_type *o_ptr, int obj_num);
extern int scan_floor(int *items, int size, int y, int x, int mode);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern int count_floor_items(int y, int x, bool pickup_only);
extern object_type* get_first_object(int y, int x);
extern object_type* get_next_object(object_type *o_ptr);
extern int get_obj_num_prep(void);
extern bool is_blessed(object_type *o_ptr);
extern s32b object_value(object_type *o_ptr);
extern bool object_similar(object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern void object_copy_amt(object_type *dst, object_type *src, int amt);
extern s16b floor_carry(int y, int x, object_type *j_ptr);
extern bool drop_near(object_type *j_ptr, int chance, int y, int x);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern int get_tag_num(int o_idx, QChar cmd, byte *tag_num);
extern int quiver_space_per_unit(const object_type *o_ptr);
extern void save_quiver_size(void);
extern int compare_ammo(int slot1, int slot2);
extern byte quiver_get_group(object_type *o_ptr);
extern int sort_quiver(int slot);
extern void open_quiver_slot(int slot);
extern bool quiver_carry_okay(object_type *o_ptr, int num, int item);
extern void inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool inven_carry_okay(object_type *o_ptr);
extern bool quiver_stack_okay(object_type *o_ptr);
extern bool inven_stack_okay(object_type *o_ptr, int set_limit);
extern s16b quiver_carry(object_type *o_ptr);
extern s16b inven_carry(object_type *o_ptr);
extern s16b inven_takeoff(int item, int amt);
extern void inven_drop(int item, int amt);
extern void combine_pack(void);
extern void combine_quiver(void);
extern void reorder_pack(void);
extern int get_use_device_chance(object_type *o_ptr);
extern void distribute_charges(object_type *o_ptr, object_type *i_ptr, int amt);
extern void reduce_charges(object_type *o_ptr, int amt);
extern unsigned check_for_inscrip(object_type *o_ptr, QString inscrip);
extern s16b lookup_kind(int tval, int sval);
extern void display_object_idx_recall(s16b o_idx);
extern void display_object_kind_recall(s16b k_idx);
extern bool obj_can_refill(object_type *o_ptr);
extern bool obj_is_openable_chest(object_type *o_ptr);
extern bool chest_requires_disarming(object_type *o_ptr);
extern bool ammo_can_fire(object_type *o_ptr, int item);
extern bool has_correct_ammo(void);
extern bool obj_has_charges(object_type *o_ptr);
extern object_type *object_from_item_idx(int item);
extern bool obj_needs_aim(object_type *o_ptr);
extern bool obj_is_activatable(object_type *o_ptr);
extern bool obj_can_activate(object_type *o_ptr);
extern bool get_item_okay(int item);
extern int scan_items(int *item_list, size_t item_list_max, int mode);
extern bool item_is_available(int item, bool (*tester)(object_type *), int mode);
extern bool is_throwing_weapon(object_type *o_ptr);
extern bool pack_is_full(void);
extern bool pack_is_overfull(void);
extern void pack_overflow(void);
extern void object_known(object_type *o_ptr);
extern void object_aware(object_type *o_ptr);
extern void object_tried(object_type *o_ptr);
extern void object_history(object_type *o_ptr, byte origin, s16b r_idx);
extern void stack_histories(object_type *o_ptr, const object_type *j_ptr);
extern void expand_inscription(const object_type *o_ptr, const char *src, char dest[], int max);



//player_ghost.cpp
extern void prepare_ghost_name(void);
extern bool prepare_ghost(int r_idx);
extern void ghost_challenge(void);
extern void remove_player_ghost(void);
extern void delete_player_ghost_entry(void);
extern void add_player_ghost_entry(void);
extern void load_player_ghost_file(void);
extern void save_player_ghost_file(void);

// player_spell.cpp
extern int get_player_spell_realm(void);

// projection.cpp
extern byte gf_color(int type);

/* randart.c */
extern QString make_random_name(byte min_length, byte max_length);
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
extern int squelch_itemp(object_type *o_ptr, byte feeling, bool fullid);
extern void rearrange_stack(int y, int x);
extern bool squelch_item_ok(object_type *o_ptr);
extern void do_squelch_pile(int y, int x);

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
extern QString inscrip_text[MAX_INSCRIP];
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
extern bool get_check(QString question);
extern QString get_string(QString question);
extern QColor add_preset_color(int which_color);
extern void message(QString msg);
extern void color_message(QString msg, int which_color);
extern void custom_color_message(QString msg, byte red, byte green, byte blue);
extern void cmd_enable_repeat(void);
extern void cmd_disable_repeat(void);


#endif // FUNCTION_DECLARATIONS_H
