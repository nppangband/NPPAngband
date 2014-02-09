#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QDir>
#include <QPainter>
#include <QImage>
#include <QFontDatabase>

#define MAX_RECENT_SAVEFILES    5

class QAction;
class QMenu;
class QGraphicsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow();



protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);


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


    // Functions to make sure the available menu commands are appropriate to the situation.
    //  For example, make the save game command unanavailable when no savefile is open.
    void update_file_menu_game_active();
    void update_file_menu_game_inactive();


    // Set up the games mapping
    void set_map();


private:

    // The editable part of the main window.
    QGraphicsView *graphics_view;

    void setup_nppangband();
    void setup_nppmoria();
    void launch_birth(bool quick_start = false);


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
    QAction *fontselect_act;


    // Holds the actual commands for the help menu.
    QAction *about_act;
    QAction *about_Qt_act;
    QAction *separator_act;

    // information about the main window
    qreal window_height; // in pixels
    qreal window_width;  // in pixels
    QFontDatabase font_database;
    QFont cur_font;
    int square_height;
    int square_width;
    int screen_num_rows;
    int screen_num_columns;
    int top_x;
    int top_y;
    int bottom_x;
    int bottom_y;
    bool use_bigtile;



};

#endif
