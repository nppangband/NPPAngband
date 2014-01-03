#include <src/init.h>
#include <src/npp.h>

#include <QMainWindow>
#include <QLabel>
#include <src/qt_mainwindow.h>

// was init.2

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

/*
 * This file is used to initialize various variables and arrays for the
 * NPPAngband or NPPMoria game.
 *
 * Most of the arrays for Angband are built from "template" files in
 * the "lib/edit" directory.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collectthe
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 */

// Functional pointer and variables used only in the init_xxx.files
// Specify which information to be parsed
int (*parse_which_info)(QString line_info);

/*
 * Error tracking
 */
int last_idx;
int prev_lev;
int shop_idx;
int owner_idx;
int cur_title;
int cur_equip;


/*
 * Standard error message text
 */
static QString err_str[PARSE_ERROR_MAX] =
{
    NULL,
    "parse error",
    "invalid flag specification",
    "invalid number of items (0-99)",
    "invalid spell frequency",
    "invalid random value",
    "missing colon",
    "missing field",
    "missing record header",
    "non-sequential records",
    "value not a number",
    "obsolete file",
    "value out of bounds",
    "out of memory",
    "too few entries",
    "too many entries",
    "undefined directive",
    "unrecognised blow",
    "unrecognised tval name",
    "unrecognised sval name",
    "vault too big",
    "too many allocations",
    "too many arguments",
    "too many non sequential quests",
    "name too long",
};

/* We are going to play NPPAngband*/
static void init_nppangband(void)
{
    z_info->max_level = 50;
    z_info->max_titles = z_info->max_level  / 5;
}

/* We are going to play NPPMoria*/
static void init_nppmoria(void)
{
    z_info->max_level = z_info->max_titles = PY_MAX_LEVEL_MORIA;
}

void quit_npp_games(QString message)
{
    QMessageBox msg_box;
    msg_box.setText(message);
    msg_box.setInformativeText(QString(QObject::tr("NPPGames will close now.")));
    msg_box.exec();
}

/*
 * Display a parser error message.
 */
static void display_parse_error(QString filename, int err, QString buf, int error_line)
{

    QString add_on;


    //  Make a message box, display it, and then quit.
    add_on = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

    pop_up_message_box(QString("Error at line %1 of '%2.txt'. %3 ").arg(error_line) .arg(filename) .arg(buf));

    /* Force Quit */
    quit_npp_games(QObject::tr("exiting NPP Games"));
}

//Initialize the various directories used by NPP
void create_directories()
{
    NPP_DIR_BASE = QDir::currentPath();
    NPP_DIR_BASE.append ("/NPPQT");
    NPP_DIR_EDIT = NPP_DIR_HELP = NPP_DIR_ICON = NPP_DIR_PREF = NPP_DIR_SAVE = NPP_DIR_USER = NPP_DIR_BASE;
    NPP_DIR_EDIT.append ("/lib/edit/");
    NPP_DIR_HELP.append ("/lib/help/");
    NPP_DIR_ICON.append ("/lib/icons/");
    NPP_DIR_PREF.append ("/lib/pref/");
    NPP_DIR_SAVE.append ("/lib/save/");
    NPP_DIR_SAVE.append ("/lib/user/");
}


static int read_edit_file(QString file_name)
{
    QFile edit_file;

    int err = 0;

    int error_line = 0;

    // Keep track of the last idx we have read so we can point to the correct entry.
    last_idx = -1;

    file_name.append(".txt");

    /* Build the filename */
    edit_file.setFileName(QString("%1%2" ) .arg(NPP_DIR_EDIT) .arg(file_name));

    //pop_up_message_box((QString(QObject::tr(" starting edit_file %1")) .arg(file_name)));

    if (!edit_file.exists())
    {
        quit_npp_games(QString(QObject::tr("Cannot find '%1.txt' file.")) .arg(file_name));
        return (1);
    }

    if (!edit_file.open(QIODevice::ReadOnly))
    {
        quit_npp_games(QString(QObject::tr("Cannot open '%1.txt' file.")) .arg(file_name));
        return(1);
    }

    QTextStream reading(&edit_file);

    /* Parse */
    while (!reading.atEnd())
    {
        error_line++;

        QString file_line = reading.readLine();

         if (error_line > 6525)
         {
             pop_up_message_box(QString("file_line is %1 name is %2").arg(file_line) .arg(file_name));

             parse_which_info = parse_r_info;
         }

        err = parse_which_info(file_line);

        if (err)
        {
             display_parse_error(file_name, err, file_line, error_line);
             //An error has occurrred.

             break;
        }
    }

    //pop_up_message_box((QString(QObject::tr("finishing with line is %1 file-name is %2")).arg(error_line) .arg(file_name)));

    // close the file
    edit_file.close();

    return (err);
}



/*
 * Initialize the "z_info" array
 */
static int init_z_info(void)
{
    int err;

    QString file_name;

    z_info = C_ZNEW(1, maxima);

    if (game_mode == GAME_NPPANGBAND) file_name = "limits";
    else file_name = "m_limits";  /* game_mode == NPPMORIA */

    parse_which_info = parse_z_info;

    err = read_edit_file(file_name);

    return (err);
}


/*
 * Initialize the "f_info" array
 */
static int init_f_info(void)
{
    int err;

    QString file_name;

    f_info = C_ZNEW(z_info->f_max, feature_type);

    file_name = "terrain";

    parse_which_info = parse_f_info;

    err = read_edit_file(file_name);

    return (err);
}


/*
 * Initialize the "k_info" array
 */
static int init_k_info(void)
{
    int err;

    QString file_name;

    k_info = C_ZNEW(z_info->k_max, object_kind);

    if (game_mode == GAME_NPPANGBAND) file_name = "object";
    else file_name = "m_object";  /* game_mode == NPPMORIA */

    parse_which_info = parse_k_info;

    err = read_edit_file(file_name);

    return (err);
}

/*
 * Initialize the "t_info" array
 */
static int init_t_info(void)
{
    int err;

    QString file_name;

    t_info = C_ZNEW(z_info->ghost_template_max, ghost_template);

    file_name = "player_ghost";

    parse_which_info = parse_t_info;

    err = read_edit_file(file_name);

    return (err);
}




/*
 * Initialize the "a_info" array
 */
static int init_a_info(void)
{
    int err;

    QString file_name;

    a_info = C_ZNEW(z_info->art_max, artifact_type);

    if (game_mode == GAME_NPPANGBAND) file_name = "artifact";
    else file_name = "m_artifact";  /* game_mode == NPPMORIA */

    parse_which_info = parse_a_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize the "e_info" array
 */
static int init_e_info(void)
{
    int err;

    QString file_name;

    e_info = C_ZNEW(z_info->e_max, ego_item_type);

    if (game_mode == GAME_NPPANGBAND) file_name = "ego_item";
    else file_name = "m_ego_item";  /* game_mode == NPPMORIA */

    parse_which_info = parse_e_info;

    err = read_edit_file(file_name);

    return (err);
}


/*
 * Initialize the "r_info" array
 */
static int init_r_info(void)
{
    int err;

    QString file_name;

    r_info = C_ZNEW(z_info->r_max, monster_race);

    if (game_mode == GAME_NPPANGBAND) file_name = "monster";
    else file_name = "m_monster";  /* game_mode == NPPMORIA */

    parse_which_info = parse_r_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize the "v_info" array
 */
static int init_v_info(void)
{
    int err;

    QString file_name;

    v_info = C_ZNEW(z_info->v_max, vault_type);

    file_name = "vault";

    parse_which_info = parse_v_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize the "p_info" array
 */
static int init_p_info(void)
{
    int err;

    QString file_name;

    p_info = C_ZNEW(z_info->p_max, player_race);

    if (game_mode == GAME_NPPANGBAND) file_name = "p_race";
    else file_name = "m_p_race";  /* game_mode == NPPMORIA */

    parse_which_info = parse_p_info;

    err = read_edit_file(file_name);

    return (err);
}


/*
 * Initialize the "c_info" array
 */
static int init_c_info(void)
{
    int err;

    cur_title = 0;
    cur_equip = 0;

    QString file_name;

    c_info = C_ZNEW(z_info->c_max, player_class);

    if (game_mode == GAME_NPPANGBAND) file_name = "p_class";
    else file_name = "m_p_class";  /* game_mode == NPPMORIA */

    parse_which_info = parse_c_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize the "h_info" array
 */
static int init_h_info(void)
{
    int err;

    QString file_name;

    h_info = C_ZNEW(z_info->h_max, hist_type);

    if (game_mode == GAME_NPPANGBAND) file_name = "p_hist";
    else file_name = "m_p_hist";  /* game_mode == NPPMORIA */

    parse_which_info = parse_h_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize the "b_info" array
 */
static int init_b_info(void)
{
    int err;

    shop_idx = 0;
    owner_idx = 0;

    QString file_name;

    b_info = C_ZNEW((z_info->b_max * MAX_STORES), owner_type);

    if (game_mode == GAME_NPPANGBAND) file_name = "shop_own";
    else file_name = "m_shop_own";  /* game_mode == NPPMORIA */

    parse_which_info = parse_b_info;

    err = read_edit_file(file_name);

    return (err);
}


/*
 * Initialize the "q_info" array
 */
static int init_q_info(void)
{
    int err;

    QString file_name;

    q_info = C_ZNEW(z_info->q_max, quest_type);

    if (game_mode == GAME_NPPANGBAND) file_name = "quest";
    else file_name = "m_quest";  /* game_mode == NPPMORIA */

    parse_which_info = parse_q_info;

    err = read_edit_file(file_name);

    // Make sure the quests are appearing in the right order.
    prev_lev = 0;

    return (err);
}

/*
 * Initialize the "n_info" structure
 */
static int init_n_info(void)
{
  int err;

  QString file_name;

  n_info = C_ZNEW(1, names_type);

  file_name = "names";

  parse_which_info = parse_n_info;

  err = read_edit_file(file_name);

  return (err);
}



/*
 * Initialize the "flavor_info" array
 */
static int init_flavor_info(void)
{
    int err;

    QString file_name;

    flavor_info = C_ZNEW(z_info->flavor_max, flavor_type);

    file_name = "flavor";

    parse_which_info = parse_flavor_info;

    err = read_edit_file(file_name);

    return (err);
}



/*
 * Initialize some other arrays
 */
static int init_other(void)
{
    int i;

    /* Array of grids */
    view_g = C_ZNEW(VIEW_MAX, u16b);

    /* Array of grids */
    temp_g = C_ZNEW(TEMP_MAX, u16b);

    /* Hack -- use some memory twice */
    temp_y = ((byte*)(temp_g)) + 0;
    temp_x = ((byte*)(temp_g)) + TEMP_MAX;

    /* Array of grids */
    fire_g = C_ZNEW(VIEW_MAX, u16b);

    /* has_LIGHT patch causes both temp_g and temp_x/y to be used
       in targetting mode: can't use the same memory any more. */
    temp_y = C_ZNEW(TEMP_MAX, byte);
    temp_x = C_ZNEW(TEMP_MAX, byte);

    /* Array of dynamic grids */
    dyna_g = C_ZNEW(DYNA_MAX, dynamic_grid_type);

    /* Array of stacked monster messages */
    mon_msg = C_ZNEW(MAX_STORED_MON_MSG, monster_race_message);
    mon_message_hist = C_ZNEW(MAX_STORED_MON_CODES, monster_message_history);

    /* Prepare monster movement array*/
    mon_moment_info = C_ZNEW(z_info->m_max, move_moment_type);

    /*** Prepare dungeon arrays ***/

    /* Padded into array */
    cave_info = C_ZNEW(MAX_DUNGEON_HGT, u16b_256);

    /* Feature array */
    cave_feat = C_ZNEW(MAX_DUNGEON_HGT, byte_wid);

    /* Entity arrays */
    cave_o_idx = C_ZNEW(MAX_DUNGEON_HGT, s16b_wid);
    cave_m_idx = C_ZNEW(MAX_DUNGEON_HGT, s16b_wid);
    cave_x_idx = C_ZNEW(MAX_DUNGEON_HGT, s16b_wid);


    /*start with cost at center 0*/
    for (i = 0; i < MAX_FLOWS; i++)
    {
        cost_at_center[i] = 0;
    }


    /*** Prepare "vinfo" array ***/

    /* Used by "update_view()" */
    // (void)vinfo_init();  add in cave.later


    /*** Prepare entity arrays ***/

    /* Objects */
    o_list = C_ZNEW(z_info->o_max, object_type);

    /* Monsters */
    mon_list = C_ZNEW(z_info->m_max, monster_type);

    /* Effects */
    x_list = C_ZNEW(z_info->x_max, effect_type);


    /*** Prepare mosnter lore array ***/

    /* Lore */
    l_list = C_ZNEW(z_info->r_max, monster_lore);

    /*** Prepare terrain lore array ***/

    /* Lore */
    f_l_list = C_ZNEW(z_info->f_max, feature_lore);

    /*** Prepare artifact lore array ***/

    /* Lore */
    a_l_list = C_ZNEW(z_info->art_max, artifact_lore);

    /*** Prepare the inventory ***/

    /* Allocate it */
    inventory = C_ZNEW(ALL_INVEN_TOTAL, object_type);

    /*** Prepare the stores ***/

    /* Allocate the stores */
    store = C_ZNEW(MAX_STORES, store_type);

    /* Fill in each store */
    for (i = 0; i < MAX_STORES; i++)
    {

        /* Get the store */
        store_type *st_ptr = &store[i];

        /* Assume full stock */
        st_ptr->stock_size = STORE_INVEN_MAX;

        /* Allocate the stock */
        st_ptr->stock = C_ZNEW(st_ptr->stock_size, object_type);

    }

    /*** Prepare the options ***/

    /* Initialize the options */
    for (i = 0; i < OPT_MAX; i++)
    {
        /* Default value */
        op_ptr->opt[i] = options[i].normal;
    }

    /* Initialize the window flags */
    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        /* Assume no flags */
        op_ptr->window_flag[i] = 0L;
    }

    /*Clear the update flags*/
    p_ptr->notice = 0L;
    p_ptr->update = 0L;
    p_ptr->redraw = 0L;

    /* Success */
    return (0);
}



/*
 * Initialize some other arrays
 */
static int init_alloc(void)
{
    int i, j;

    object_kind *k_ptr;

    feature_type *f_ptr;

    monster_race *r_ptr;

    ego_item_type *e_ptr;

    alloc_entry *table;

    s16b num[MAX_DEPTH_ALL];

    s16b aux[MAX_DEPTH_ALL];


    /*** Analyze object allocation info ***/

    /* Clear the "aux" array */
    (void)C_WIPE(aux, MAX_DEPTH_ALL, s16b);

    /* Clear the "num" array */
    (void)C_WIPE(num, MAX_DEPTH_ALL, s16b);

    /* Size of "alloc_kind_table" */
    alloc_kind_size = 0;

    /* Scan the objects */
    for (i = 1; i < z_info->k_max; i++)
    {
        k_ptr = &k_info[i];

        /* Scan allocation pairs */
        for (j = 0; j < 4; j++)
        {
            /* Count the "legal" entries */
            if (k_ptr->chance[j])
            {
                /* Count the entries */
                alloc_kind_size++;

                /* Group by level */
                num[k_ptr->locale[j]]++;
            }
        }
    }

    /* Collect the level indexes */
    for (i = 1; i < MAX_DEPTH_ALL; i++)
    {
        /* Group by level */
        num[i] += num[i-1];
    }

    /* Paranoia */
    if (!num[0]) quit_npp_games(QObject::tr("No town objects!"));


    /*** Initialize object allocation info ***/

    /* Allocate the alloc_kind_table */
    alloc_kind_table = C_ZNEW(alloc_kind_size, alloc_entry);

    /* Get the table entry */
    table = alloc_kind_table;

    /* Scan the objects */
    for (i = 1; i < z_info->k_max; i++)
    {
        k_ptr = &k_info[i];

        /* Scan allocation pairs */
        for (j = 0; j < 4; j++)
        {
            /* Count the "legal" entries */
            if (k_ptr->chance[j])
            {
                int p, x, y, z;

                /* Extract the base level */
                x = k_ptr->locale[j];

                /* Extract the base probability */
                p = (100 / k_ptr->chance[j]);

                /* Skip entries preceding our locale */
                y = (x > 0) ? num[x-1] : 0;

                /* Skip previous entries at this locale */
                z = y + aux[x];

                /* Load the entry */
                table[z].index = i;
                table[z].level = x;
                table[z].prob1 = p;
                table[z].prob2 = p;
                table[z].prob3 = p;

                /* Another entry complete for this locale */
                aux[x]++;
            }
        }
    }

    /*** Analyze feature allocation info ***/

    /* Clear the "aux" array */
    (void)C_WIPE(&aux, MAX_DEPTH_ALL, s16b);

    /* Clear the "num" array */
    (void)C_WIPE(&num, MAX_DEPTH_ALL, s16b);

    /* Size of "alloc_feat_table" */
    alloc_feat_size = 0;

    /* Scan the features */
    for (i = 1; i < z_info->f_max; i++)
    {
        /* Get the i'th race */
        f_ptr = &f_info[i];

        /* Legal features */
        if (f_ptr->f_rarity)
        {
            /* Count the entries */
            alloc_feat_size++;

            /* Group by level */
            num[f_ptr->f_level]++;
        }
    }

    /* Collect the level indexes */
    for (i = 1; i < MAX_DEPTH_ALL; i++)
    {
        /* Group by level */
        num[i] += num[i-1];
    }

    /* Paranoia - not really necessary */
    if (!num[0]) quit_npp_games(QObject::tr("No town features!"));

    /*** Initialize feature allocation info ***/

    /* Allocate the alloc_feat_table */
    alloc_feat_table = C_ZNEW(alloc_feat_size, alloc_entry);

    /* Get the table entry */
    table = alloc_feat_table;

    /* Scan the features */
    for (i = 1; i < z_info->f_max; i++)
    {
        /* Get the i'th feature */
        f_ptr = &f_info[i];

        /* Count valid pairs */
        if (f_ptr->f_rarity)
        {
            int p, x, y, z;

            /* Extract the base level */
            x = f_ptr->f_level;

            /* Extract the base probability */
            p = (100 / f_ptr->f_rarity);

            /* Skip entries preceding our locale */
            y = (x > 0) ? num[x-1] : 0;

            /* Skip previous entries at this locale */
            z = y + aux[x];

            /* Load the entry */
            table[z].index = i;
            table[z].level = x;
            table[z].prob1 = p;
            table[z].prob2 = p;
            table[z].prob3 = p;

            /* Another entry complete for this locale */
            aux[x]++;
        }
    }

    /*** Analyze monster allocation info ***/

    /* Clear the "aux" array */
    (void)C_WIPE(aux, MAX_DEPTH_ALL, s16b);

    /* Clear the "num" array */
    (void)C_WIPE(num, MAX_DEPTH_ALL, s16b);

    /* Size of "alloc_race_table" */
    alloc_race_size = 0;

    /* Scan the monsters*/
    for (i = 1; i < z_info->r_max; i++)
    {
        /* Get the i'th race */
        r_ptr = &r_info[i];

        /* Legal monsters */
        if (r_ptr->rarity)
        {
            /* Count the entries */
            alloc_race_size++;

            /* Group by level */
            num[r_ptr->level]++;
        }
    }

    /* Collect the level indexes */
    for (i = 1; i < MAX_DEPTH_ALL; i++)
    {
        /* Group by level */
        num[i] += num[i-1];
    }

    /* Paranoia */
    if (!num[0]) quit_npp_games(QObject::tr("No town monsters!"));


    /*** Initialize monster allocation info ***/

    /* Allocate the alloc_race_table */
    alloc_race_table = C_ZNEW(alloc_race_size, alloc_entry);

    /* Get the table entry */
    table = alloc_race_table;

    /* Scan the monsters*/
    for (i = 1; i < z_info->r_max; i++)
    {
        /* Get the i'th race */
        r_ptr = &r_info[i];

        /* Count valid pairs */
        if (r_ptr->rarity)
        {
            int p, x, y, z;

            /* Extract the base level */
            x = r_ptr->level;

            /* Extract the base probability */
            p = (100 / r_ptr->rarity);

            /* Skip entries preceding our locale */
            y = (x > 0) ? num[x-1] : 0;

            /* Skip previous entries at this locale */
            z = y + aux[x];

            /* Load the entry */
            table[z].index = i;
            table[z].level = x;
            table[z].prob1 = p;
            table[z].prob2 = p;
            table[z].prob3 = p;

            /* Another entry complete for this locale */
            aux[x]++;
        }
    }

    /*** Analyze ego_item allocation info ***/

    /* Clear the "aux" array */
    (void)C_WIPE(aux, MAX_DEPTH_ALL, s16b);

    /* Clear the "num" array */
    (void)C_WIPE(num, MAX_DEPTH_ALL, s16b);

    /* Size of "alloc_ego_table" */
    alloc_ego_size = 0;

    /* Scan the ego items */
    for (i = 1; i < z_info->e_max; i++)
    {
        /* Get the i'th ego item */
        e_ptr = &e_info[i];

        /* Legal items */
        if (e_ptr->rarity)
        {
            /* Count the entries */
            alloc_ego_size++;

            /* Group by level */
            num[e_ptr->level]++;
        }
    }

    /* Collect the level indexes */
    for (i = 1; i < MAX_DEPTH_ALL; i++)
    {
        /* Group by level */
        num[i] += num[i-1];
    }

    /*** Initialize ego-item allocation info ***/

    /* Allocate the alloc_ego_table */
    alloc_ego_table = C_ZNEW(alloc_ego_size, alloc_entry);

    /* Get the table entry */
    table = alloc_ego_table;

    /* Scan the ego-items */
    for (i = 1; i < z_info->e_max; i++)
    {
        /* Get the i'th ego item */
        e_ptr = &e_info[i];

        /* Count valid pairs */
        if (e_ptr->rarity)
        {
            int p, x, y, z;

            /* Extract the base level */
            x = e_ptr->level;

            /* Extract the base probability */
            p = (100 / e_ptr->rarity);

            /* Skip entries preceding our locale */
            y = (x > 0) ? num[x-1] : 0;

            /* Skip previous entries at this locale */
            z = y + aux[x];

            /* Load the entry */
            table[z].index = i;
            table[z].level = x;
            table[z].prob1 = p;
            table[z].prob2 = p;
            table[z].prob3 = p;

            /* Another entry complete for this locale */
            aux[x]++;
        }
    }

    /* Success */
    return (0);
}

/*
 * Hack -- main Angband initialization entry point
 *
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialize all internal arrays, and
 * load the basic "user pref files".
 *
 * Be very careful to keep track of the order in which things
 * are initialized, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialized until the
 * end of this function, when the default "user pref files"
 * are loaded and "Term_xtra(TERM_XTRA_REACT,0)" is called.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 *
 * Note that various things are initialized by this function,
 * including everything that was once done by "init_some_arrays".
 *
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 *
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 *
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
void init_npp_games(void)
{

    QLabel status_update;
    status_update.setText (QString("Starting game"));
    status_update.show();

    /*** Initialize some arrays ***/

    /* Initialize size info */
    status_update.setText (QString(QObject::tr("Initializing array sizes...")));
    if (init_z_info()) quit_npp_games(QObject::tr("Cannot initialize sizes"));

    if (game_mode == GAME_NPPANGBAND) init_nppangband();
    else if (game_mode == GAME_NPPMORIA) init_nppmoria();
    else return;  //Something would have to be wrong with the setup for this to happen.

    /* Initialize feature info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (features)")));
    if (init_f_info()) quit_npp_games(QObject::tr("Cannot initialize features"));

    /* Initialize object info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (objects)")));
    if (init_k_info()) quit_npp_games(QObject::tr("Cannot initialize objects"));

    /* Initialize object info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (ghosts)")));
    if (init_t_info()) quit_npp_games(QObject::tr("Cannot initialize ghosts"));

    /* Initialize artifact info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (artifacts)")));
    if (init_a_info()) quit_npp_games(QObject::tr("Cannot initialize artifacts"));

    /* Initialize ego-item info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (ego-items)")));
    if (init_e_info()) quit_npp_games(QObject::tr("Cannot initialize ego-items"));

    /* Initialize monster info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (monsters)")));
    if (init_r_info()) quit_npp_games(QObject::tr("Cannot initialize monsters"));

    /* Initialize feature info */
    status_update.setText (QString(QObject::tr("Initializing arrays... vaults)")));
    if (init_v_info()) quit_npp_games(QObject::tr("Cannot initialize vaults"));

    /* Initialize history info */
    status_update.setText (QString(QObject::tr("Initializing arrays... histories)")));
    if (init_h_info()) quit_npp_games(QObject::tr("Cannot initialize histories"));

    /* Initialize race info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (races)")));
    if (init_p_info()) quit_npp_games(QObject::tr("Cannot initialize races"));

    /* Initialize class info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (classes)")));
    if (init_c_info()) quit_npp_games(QObject::tr("Cannot initialize classes"));

    /* Initialize owner info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (owners)")));
    if (init_b_info()) quit_npp_games(QObject::tr("Cannot initialize owners"));

    /* Initialize flavor info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (flavors)")));
    if (init_flavor_info()) quit_npp_games(QObject::tr("Cannot initialize flavors"));

    /* Initialize quests info */
    status_update.setText (QString(QObject::tr("Initializing arrays... (quests)")));
    if (init_q_info()) quit_npp_games(QObject::tr("Cannot initialize quests"));

    /* Initialize some other arrays */
    status_update.setText (QString(QObject::tr("Initializing arrays... (other)")));
    if (init_other()) quit_npp_games(QObject::tr("Cannot initialize other stuff"));

    /* Initialize some other arrays */
    status_update.setText (QString(QObject::tr("Initializing arrays... (alloc)")));
    if (init_alloc()) quit_npp_games(QObject::tr("Cannot initialize alloc stuff"));

    /*** Load default user pref files ***/


    /* Initialize randart tables info */
    status_update.setText (QString(QObject::tr("Initializing Random Artifact Tables...]")));
    if (init_n_info()) quit_npp_games(QObject::tr("Cannot initialize random name generator list"));

    /*Build the randart probability tables based on the standard Artifact Set*/
    build_randart_tables();

    /* Done */
    status_update.setText (QString(QObject::tr("Initialization complete")));

    status_update.hide();
}


void cleanup_npp_games(void)
{
    int i;

    delete_notes_file();

    /* Free the allocation tables */
    FREE(alloc_ego_table);
    FREE(alloc_feat_table);
    FREE(alloc_race_table);
    FREE(alloc_kind_table);

    if (store)
    {
        /* Free the store inventories */
        for (i = 0; i < MAX_STORES; i++)
        {
            /* Get the store */
            store_type *st_ptr = &store[i];

            /* Free the store inventory */
            FREE(st_ptr->stock);
        }
    }

    /* Free the stores */
    FREE(store);

    /* Free the player inventory */
    FREE(inventory);

    // Free the various edit file arrays
    FREE(z_info);
    FREE(f_info);
    FREE(k_info);
    FREE(t_info);
    FREE(a_info);
    FREE(e_info);
    FREE(r_info);
    FREE(v_info);
    FREE(p_info);
    FREE(c_info);
    FREE(h_info);
    FREE(b_info);
    FREE(q_info);
    FREE(n_info);
    FREE(flavor_info);


    /* Free the lore, monster, effects, and object lists */
    FREE(l_list);
    FREE(f_l_list);
    FREE(a_l_list);
    FREE(mon_list);
    FREE(o_list);
    FREE(x_list);

    /* Free the cave */
    FREE(cave_o_idx);
    FREE(cave_m_idx);
    FREE(cave_x_idx);
    FREE(cave_feat);
    FREE(cave_info);

    /* Prepare monster movement array*/
    FREE(mon_moment_info);

    /* Free the "update_view()" array */
    FREE(view_g);

    /* Free the other "update_view()" array */
    FREE(fire_g);

    /* Free the temp array */
    FREE(temp_g);
    FREE(temp_y);
    FREE(temp_x);

    /* Free the dynamic features array */
    FREE(dyna_g);

    /* Free the stacked monster messages */
    FREE(mon_msg);
    FREE(mon_message_hist);

    /*free the randart arrays*/
    free_randart_tables();
}

