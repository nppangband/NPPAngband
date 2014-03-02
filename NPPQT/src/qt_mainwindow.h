#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QDir>
#include <QPainter>
#include <QImage>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QActionGroup>
#include <QEventLoop>
#include <src/defines.h>

#define UI_MODE_DEFAULT 0
#define UI_MODE_TARGETTING 1

#define MAX_RECENT_SAVEFILES    5

class QAction;
class QMenu;
class QGraphicsView;
class QGraphicsScene;
class DungeonGrid;
class DungeonCursor;

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    int ui_mode;
    QPoint target;
    QEventLoop ev_loop;

    DungeonGrid *grids[MAX_DUNGEON_HGT][MAX_DUNGEON_WID];

    int font_hgt, font_wid;
    int tile_hgt, tile_wid;
    int cell_hgt, cell_wid;
    bool do_pseudo_ascii;

    QPixmap blank_pix;
    // The key must me strings of the form "[row]x[col]"
    QHash<QString, QPixmap> tiles;
    QPixmap tile_map;

    DungeonCursor *cursor;

    MainWindowPrivate *priv;

    MainWindow();

    QPoint get_target(u32b flags);
    void init_scene();
    void set_font(QFont newFont);
    void calculate_cell_size();
    void destroy_tiles();
    void set_graphic_mode(int mode);
    void redraw();
    void update_cursor();
    void force_redraw();
    bool panel_contains(int y, int x);

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent* which_key);
    bool eventFilter(QObject *obj, QEvent *event);


private slots:

    // Functions called from signals from the top menu.

    void start_game_nppangband();
    void start_game_nppmoria();
    void open_current_savefile();
    void save_character();
    void save_character_as();
    void save_and_close();
    void open_recent_file();
    void about();
    void options_dialog();
    void fontselect_dialog();

    void slot_find_player();
    void slot_redraw();
    void slot_zoom_out();
    void slot_zoom_in();
    void slot_something();
    void slot_finish_bolt();

    // Functions to make sure the available menu commands are appropriate to the situation.
    //  For example, make the save game command unanavailable when no savefile is open.
    void update_file_menu_game_active();
    void update_file_menu_game_inactive();

    // Graphics
    void set_dvg();
    void set_old_tiles();
    void set_ascii();
    void set_pseudo_ascii();


private:

    // The editable part of the main window.
    QGraphicsView *graphics_view;
    QGraphicsScene *dungeon_scene;

    void setup_nppangband();
    void setup_nppmoria();
    void launch_birth(bool quick_start = false);

    void debug_dungeon();

    // Functions that initialize the file menu of the main window.
    void create_actions();
    void create_menus();
    void create_toolbars();
    void select_font();

    // Set up many of the game commands
    void create_signals();


    // Remember the game settings
    void read_settings();
    void write_settings();    

    //Functions and variables that handle opening and saving files, as well as maintain the
    //  5 most recent savefile list.
    void load_file(const QString &file_name);
    void save_file(const QString &file_name);
    void set_current_savefile(const QString &file_name);
    void update_recent_savefiles();
    QString stripped_name(const QString &full_file_name);
    QStringList recent_savefiles;
    QAction *recent_savefile_actions[MAX_RECENT_SAVEFILES];



    //  Holds the actual commands for the file menu and toolbar.
    QMenu *file_menu;
    QMenu *recent_files_menu;
    QMenu *settings;
    QMenu *help_menu;
    QToolBar *file_toolbar;
    QAction *new_game_nppangband;
    QAction *new_game_nppmoria;
    QAction *open_savefile;
    QAction *save_cur_char;
    QAction *save_cur_char_as;
    QAction *close_cur_char;
    QAction *exit_npp;

    //Command for the settings menu
    QAction *options_act;
    QAction *bigtile_act;
    QAction *ascii_mode_act;
    QAction *dvg_mode_act;
    QAction *old_tiles_act;
    QAction *pseudo_ascii_act;
    QAction *fontselect_act;


    // Holds the actual commands for the help menu.
    QAction *about_act;
    QAction *about_Qt_act;
    QAction *separator_act;

    // information about the main window
    QFontDatabase font_database;
    QFont cur_font;
    bool use_bigtile;

    QActionGroup *multipliers;
};

#endif
