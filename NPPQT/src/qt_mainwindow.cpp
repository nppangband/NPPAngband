#include <QtWidgets>

#include "src/npp.h"
#include "src/qt_mainwindow.h"
#include "src/init.h"
#include "src/optionsdialog.h"
#include "src/birthdialog.h"
#include "src/dungeonbox.h"

class MainWindowPrivate
{
public:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsSimpleTextItem *items[MAX_DUNGEON_HGT][MAX_DUNGEON_WID];

    void init_scene(QGraphicsScene *_scene, QGraphicsView *_view);
    void wipe();
    void redraw();
    void redraw_cell(int y, int x);
};

void MainWindowPrivate::init_scene(QGraphicsScene *_scene, QGraphicsView *_view)
{
    scene = _scene;
    view = _view;

    QFont font("Courier New"); // Use fixed size font. Right?
    QFontMetrics metrics(font);
    int hgt = metrics.height();
    int wid = metrics.width('M');

    QBrush brush(QColor("black"));
    scene->setBackgroundBrush(brush);

    for (int y = 0; y < MAX_DUNGEON_HGT; y++) {
        for (int x = 0; x < MAX_DUNGEON_WID; x++) {
            items[y][x] = scene->addSimpleText(QString(" "), font);
            items[y][x]->setPos(x * wid, y * hgt);
        }
    }
}

void MainWindowPrivate::wipe()
{
    for (int y = 0; y < MAX_DUNGEON_HGT; y++) {
        for (int x = 0; x < MAX_DUNGEON_WID; x++) {
            items[y][x]->setText(QString(" "));
        }
    }
}

void MainWindowPrivate::redraw()
{
    wipe();

    for (int y = 0; y < p_ptr->cur_map_hgt; y++) {
        for (int x = 0; x < p_ptr->cur_map_wid; x++) {
            light_spot(y, x);
            redraw_cell(y, x);
        }
    }
}

void MainWindowPrivate::redraw_cell(int y, int x)
{
    dungeon_type *d_ptr = &dungeon_info[y][x];
    QChar square_char = d_ptr->dun_char;
    QColor square_color = d_ptr->dun_color;
    if (d_ptr->has_monster())
    {
        square_char = d_ptr->monster_char;
        square_color = d_ptr->monster_color;
    }
    else if (d_ptr->has_effect())
    {
        square_char = d_ptr->effect_char;
        square_color = d_ptr->effect_color;
    }
    else if (d_ptr->has_object())
    {
        square_char = d_ptr->object_char;
        square_color = d_ptr->object_color;
    }

    items[y][x]->setText(QString(square_char));
    items[y][x]->setBrush(QBrush(square_color));
}

// The main function - intitalize the main window and set the menus.
MainWindow::MainWindow()
{
    setAttribute(Qt::WA_DeleteOnClose);

    priv = new MainWindowPrivate;

    dungeon_scene = new QGraphicsScene;
    graphics_view = new QGraphicsView(dungeon_scene);

    priv->init_scene(dungeon_scene, graphics_view);

    setCentralWidget(graphics_view);

    create_actions();
    update_file_menu_game_inactive();
    create_menus();
    create_toolbars();
    select_font();
    create_directories();
    create_signals();
    (void)statusBar();

    read_settings();
    set_map();

    setWindowFilePath(QString());
}



void MainWindow::setup_nppangband()
{
    game_mode = GAME_NPPANGBAND;

    setWindowTitle(tr("NPPAngband"));

    init_npp_games();
}

void MainWindow::setup_nppmoria()
{
    game_mode = GAME_NPPMORIA;

    setWindowTitle(tr("NPPMoria"));

    init_npp_games();
}

//  Support functions for the file menu.

void MainWindow::debug_dungeon()
{
    QDialog *dlg = new QDialog(this);

    DungeonBox *textbox = new DungeonBox;

    dlg->setLayout(new QVBoxLayout());

    dlg->layout()->addWidget(textbox);    

    wiz_light();

    pop_up_message_box(QString("Player: (%1,%2)").arg(p_ptr->py).arg(p_ptr->px));

    textbox->redraw();

    //dlg->resize(QSize(1024, 500));

    dlg->setWindowState(Qt::WindowMaximized);

    dlg->exec();
    delete dlg;
}

// Prepare to play a game of NPPAngband.
void MainWindow::start_game_nppangband()
{
    setup_nppangband();

    launch_birth();
}

// Prepare to play a game of NPPMoria.
void MainWindow::start_game_nppmoria()
{
    setup_nppmoria();

    launch_birth();
}

void MainWindow::open_current_savefile()
{
    // Let the user select the savefile
    QString file_name = QFileDialog::getOpenFileName(this, tr("Select a savefile"), NPP_DIR_SAVE, tr("NPP (*.npp)"));
    if (file_name.isEmpty()) return;

    load_file(file_name);
}

void MainWindow::save_character()
{
    if (current_savefile.isEmpty())
        save_character_as();
    else
        save_file(current_savefile);
}

void MainWindow::save_character_as()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select a savefile"), NPP_DIR_SAVE, tr("NPP (*.npp)"));
    if (fileName.isEmpty())
        return;

    save_file(fileName);
}

void MainWindow::save_and_close()
{

    save_character();

    set_current_savefile("");

    character_loaded = false;

    update_file_menu_game_inactive();

    // close game
    cleanup_npp_games();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!current_savefile.isEmpty() && character_loaded)
    {
        //save_character();
        //pop_up_message_box("Game saved");
    }
    write_settings();
    event->accept();
}

void MainWindow::open_recent_file()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        load_file(action->data().toString());
    }
}

void MainWindow::options_dialog()
{
    OptionsDialog *dlg = new OptionsDialog(this);
    dlg->exec();
    delete dlg;
}

void MainWindow::fontselect_dialog()
{
    bool selected;
    cur_font = QFontDialog::getFont( &selected, cur_font, this );

    if (selected)
    {
        //  Figure out - this sets the fonnt for everything setFont(cur_font);
        set_map();
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About NPPAngband and NPPMoria"),
            tr("<h2>NPPAngband and NPPMoria"
               "<p>Copyright (c) 2003-2015 Jeff Greene and Diego González.</h2>"

               "<p>For resources and links to places you can talk about the game, please see:"
               "<p>http://forum.nppangband.org/ -- the NPPAngband Forums"

               "<p>Based on Moria: (c) 1985 Robert Alan Koeneke and Umoria (c) 1989 James E. Wilson, David Grabiner,"
               "<p>Angband 2.6.2:   Alex Cutler, Andy Astrand, Sean Marsh, Geoff Hill, Charles Teague, Charles Swiger, "
               "Angband 2.7.0 - 2.8.5:   Ben Harrison 2.9.0 - 3.0.6: Robert Ruehlmann, "
               "Angband 3.0.7 - 3.4.0:  Andrew Sidwell"
               "<p>Oangband 0.6.0 Copyright 1998-2003 Leon Marrick, Bahman Rabii"
               "<p>EYAngband 0.5.2 By Eytan Zweig, UNAngband by Andrew Doull"
               "<p>Sangband 0.9.9 and Demoband by Leon Marrick"
               "<p>FAangband 1.6 by Nick McConnell"
               "<p>Please see copyright.txt for complete copyright and licensing restrictions."));
}

// Activates and de-activates certain file_menu commands when a game is started.
// Assumes create_actions has already been called.
void MainWindow::update_file_menu_game_active()
{
    new_game_nppangband->setEnabled(FALSE);
    new_game_nppmoria->setEnabled(FALSE);
    open_savefile->setEnabled(FALSE);
    save_cur_char->setEnabled(TRUE);
    save_cur_char_as->setEnabled(TRUE);
    close_cur_char->setEnabled(TRUE);

    for (int i = 0; i < MAX_RECENT_SAVEFILES; ++i)
    {
        recent_savefile_actions[i]->setEnabled(FALSE);
    }

    options_act->setEnabled(TRUE);
}

// Activates and de-activates certain file_menu commands when a game is ended.
// Assumes create_actions has already been called.
void MainWindow::update_file_menu_game_inactive()
{
    new_game_nppangband->setEnabled(TRUE);
    new_game_nppmoria->setEnabled(TRUE);
    open_savefile->setEnabled(TRUE);
    save_cur_char->setEnabled(FALSE);
    save_cur_char_as->setEnabled(FALSE);
    close_cur_char->setEnabled(FALSE);

    for (int i = 0; i < MAX_RECENT_SAVEFILES; ++i)
    {
        recent_savefile_actions[i]->setEnabled(TRUE);
    }

    options_act->setEnabled(FALSE);
}


//  Set's up all the QActions that will be added to the menu bar.  These are later added by create_menus.
void MainWindow::create_actions()
{
    new_game_nppangband = new QAction(tr("New Game - NPPAngband"), this);
    new_game_nppangband->setStatusTip(tr("Start a new game of NPPAngband."));
    new_game_nppangband->setIcon(QIcon(":/icons/lib/icons/New_game_NPPAngband.png"));
    new_game_nppangband->setShortcut(tr("Ctrl+A"));
    connect(new_game_nppangband, SIGNAL(triggered()), this, SLOT(start_game_nppangband()));

    new_game_nppmoria = new QAction(tr("New Game - NPPMoria"), this);
    new_game_nppmoria->setStatusTip(tr("Start a new game of NPPMoria."));
    new_game_nppmoria->setIcon(QIcon(":/icons/lib/icons/New_Game_NPPMoria.png"));
    new_game_nppmoria->setShortcut(tr("Ctrl+R"));
    connect(new_game_nppmoria, SIGNAL(triggered()), this, SLOT(start_game_nppmoria()));

    open_savefile = new QAction(tr("Open Savefile"), this);
    open_savefile->setShortcut(tr("Ctrl+F"));
    open_savefile->setIcon(QIcon(":/icons/lib/icons/open_savefile.png"));
    open_savefile->setStatusTip(tr("Open an existing savefile."));
    connect(open_savefile, SIGNAL(triggered()), this, SLOT(open_current_savefile()));

    save_cur_char = new QAction(tr("Save Character"), this);
    save_cur_char->setShortcut(tr("Ctrl+S"));
    save_cur_char->setIcon(QIcon(":/icons/lib/icons/save.png"));
    save_cur_char->setStatusTip(tr("Save current character."));
    connect(save_cur_char, SIGNAL(triggered()), this, SLOT(save_character()));

    save_cur_char_as = new QAction(tr("Save Character As"), this);
    save_cur_char_as->setShortcut(tr("Ctrl+W"));
    save_cur_char_as->setIcon(QIcon(":/icons/lib/icons/save_as.png"));
    save_cur_char_as->setStatusTip(tr("Save current character to new file."));
    connect(save_cur_char_as, SIGNAL(triggered()), this, SLOT(save_character_as()));

    close_cur_char = new QAction(tr("Save And Close"), this);
    close_cur_char->setShortcut(tr("Ctrl+X"));
    close_cur_char->setIcon(QIcon(":/icons/lib/icons/close_game.png"));
    close_cur_char->setStatusTip(tr("Save and close current character."));
    connect(close_cur_char, SIGNAL(triggered()), this, SLOT(save_and_close()));

    exit_npp = new QAction(tr("Exit Game"), this);
    exit_npp->setShortcut(tr("Ctrl+Q"));
    exit_npp->setIcon(QIcon(":/icons/lib/icons/Exit.png"));
    exit_npp->setStatusTip(tr("Exit the application.  Save any open character."));
    connect(exit_npp, SIGNAL(triggered()), this, SLOT(close()));


    for (int i = 0; i < MAX_RECENT_SAVEFILES; ++i)
    {
        recent_savefile_actions[i] = new QAction(this);
        recent_savefile_actions[i]->setVisible(false);
        connect(recent_savefile_actions[i], SIGNAL(triggered()),
                this, SLOT(open_recent_file()));
    }

    options_act = new QAction(tr("Options"), this);
    options_act->setStatusTip(tr("Change the game options."));
    options_act->setIcon(QIcon(":/icons/lib/icons/options.png"));
    connect(options_act, SIGNAL(triggered()), this, SLOT(options_dialog()));

    bigtile_act = new QAction(tr("Use Bigtile"), this);
    bigtile_act->setCheckable(true);
    bigtile_act->setChecked(use_bigtile);
    bigtile_act->setStatusTip(tr("Doubles the width of each dungeon square."));
    connect(bigtile_act, SIGNAL(changed()), this, SLOT(set_map()));

    fontselect_act = new QAction(tr("Fonts"), this);
    fontselect_act->setStatusTip(tr("Change the window font or font size."));
    connect(fontselect_act, SIGNAL(triggered()), this, SLOT(fontselect_dialog()));


    about_act = new QAction(tr("&About"), this);
    about_act->setStatusTip(tr("Show the application's About box"));
    connect(about_act, SIGNAL(triggered()), this, SLOT(about()));

    about_Qt_act = new QAction(tr("About &Qt"), this);
    about_Qt_act->setStatusTip(tr("Show the Qt library's About box"));
    connect(about_Qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

//  Set's up many of the keystrokes and commands used during the game.
void MainWindow::create_signals()
{
    // currently empty
}


//Actually add the QActions intialized in create_actions to the menu
void MainWindow::create_menus()
{

    //File section of top menu.
    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(new_game_nppangband);
    file_menu->addAction(new_game_nppmoria);
    file_menu->addAction(open_savefile);
    separator_act = file_menu->addSeparator();
    file_menu->addAction(save_cur_char);
    file_menu->addAction(save_cur_char_as);
    file_menu->addAction(close_cur_char);
    separator_act = file_menu->addSeparator();
    file_menu->addAction(exit_npp);
    separator_act = file_menu->addSeparator();    
    for (int i = 0; i < MAX_RECENT_SAVEFILES; ++i)
        file_menu->addAction(recent_savefile_actions[i]);
    separator_act = file_menu->addSeparator();

    update_recent_savefiles();

    menuBar()->addSeparator();

    settings = menuBar()->addMenu(tr("&Settings"));
    settings->addAction(options_act);
    settings->addAction(fontselect_act);
    settings->addAction(bigtile_act);

    // Help section of top menu.
    help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(about_act);
    help_menu->addAction(about_Qt_act);
}

// Create the toolbars
void MainWindow::create_toolbars()
{
    file_toolbar = addToolBar(tr("&File"));

    file_toolbar->addAction(new_game_nppangband);
    file_toolbar->addAction(new_game_nppmoria);
    file_toolbar->addAction(open_savefile);    
    file_toolbar->addSeparator();
    file_toolbar->addAction(save_cur_char);
    file_toolbar->addAction(save_cur_char_as);
    file_toolbar->addAction(close_cur_char);
    file_toolbar->addAction(options_act);
    file_toolbar->addSeparator();
    file_toolbar->addAction(exit_npp);
}

// Just find an initial font to start the game
// User preferences will be saved with the game.
void MainWindow::select_font()
{
    bool have_font = FALSE;

    foreach (QString family, font_database.families())
    {
        if (font_database.isFixedPitch(family))
        {
            font_database.addApplicationFont(family);
            if (have_font) continue;
            cur_font = QFont(family);
            have_font = TRUE;
        }
    }

    cur_font.setPointSize(12);
    //  Figure out - this sets the font for everything setFont(cur_font);
}



// Read and write the game settings.
// Every entry in write-settings should ahve a corresponding entry in read_settings.
void MainWindow::read_settings()
{
    QSettings settings("NPPGames", "NPPQT");

    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    recent_savefiles = settings.value("recentFiles").toStringList();
    bool bigtile_setting = settings.value("set_bigtile", TRUE).toBool();
    bigtile_act->setChecked((bigtile_setting));

    QString load_font = settings.value("current_font", cur_font ).toString();
    cur_font.fromString(load_font);
    restoreState(settings.value("window_state").toByteArray());


    update_recent_savefiles();
}

void MainWindow::write_settings()
{
    QSettings settings("NPPGames", "NPPQT");

    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("recentFiles", recent_savefiles);
    settings.setValue("set_bigtile", bigtile_act->isChecked());
    settings.setValue("current_font", cur_font.toString());
    settings.setValue("window_state", saveState());

}


void MainWindow::load_file(const QString &file_name)
{    
    if (!file_name.isEmpty())
    {
        set_current_savefile(file_name);

        //make sure we have a valid game_mode
        game_mode = GAME_MODE_UNDEFINED;
        load_gamemode();
        if (game_mode == GAME_MODE_UNDEFINED) return;

        // Initialize game then load savefile
        if (game_mode == GAME_NPPANGBAND) setup_nppangband();
        else if (game_mode == GAME_NPPMORIA) setup_nppmoria();

        if (load_player())
        {
            //update_file_menu_game_active();
            statusBar()->showMessage(tr("File loaded"), 2000);

            if (!character_loaded) {
                save_prev_character();
                launch_birth(true);
            }
            else {
                update_file_menu_game_active();
                launch_game();
                //debug_dungeon();
                //screen_redraw();
                priv->redraw();
            }
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Recent Files"), tr("Cannot read file %1").arg(file_name));
        return;
    }
}

void MainWindow::launch_birth(bool quick_start)
{
    BirthDialog *dlg = new BirthDialog(this);
    dlg->set_quick_start(quick_start);
    if (dlg->run()) {                
        update_file_menu_game_active();
        launch_game();
        save_character();
        //debug_dungeon();
        //screen_redraw();
        priv->redraw();
    } else {
        cleanup_npp_games();
        character_loaded = false;
        current_savefile.clear();
    }
    delete dlg;
}

void MainWindow::save_file(const QString &file_name)
{
    set_current_savefile(file_name);

    if (!save_player())
    {
        QMessageBox::warning(this, tr("Recent Files"), tr("Cannot write file %1").arg(file_name));
        return;
    }

    statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::set_current_savefile(const QString &file_name)
{
    current_savefile = file_name;
    setWindowModified(false);

    QString shownName = "Untitled";
    if (!current_savefile.isEmpty()) {
        shownName = stripped_name(current_savefile);
        recent_savefiles.removeAll(current_savefile);
        recent_savefiles.prepend(current_savefile);
        update_recent_savefiles();
    }
}

// Update the 5 most recently played savefile list.
void MainWindow::update_recent_savefiles()
{
    QMutableStringListIterator i(recent_savefiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MAX_RECENT_SAVEFILES; ++j)
    {
        if (j < recent_savefiles.count())
        {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(stripped_name(recent_savefiles[j]));
            recent_savefile_actions[j]->setText(text);
            recent_savefile_actions[j]->setData(recent_savefiles[j]);
            recent_savefile_actions[j]->setVisible(true);
        }
        else
        {
            recent_savefile_actions[j]->setVisible(false);
        }
    }
    separator_act->setVisible(!recent_savefiles.isEmpty());
}

QString MainWindow::stripped_name(const QString &full_file_name)
{
    return QFileInfo(full_file_name).fileName();
}


// Overloaded function to ensure that set_map is called every time the window is re-sized
void MainWindow::resizeEvent (QResizeEvent *event)
{
    (void) event;
    set_map();
}

// Write a single colored character on a designated square
void MainWindow::write_colored_text(QChar letter, QColor color, s16b y, s16b x)
{
    QPainter painter(this);

    // Paranoia
    if (!panel_contains (y, x)) return;

    // Get the coordinates
    s16b pixel_y = (y - first_y) * square_height;
    s16b pixel_x = (y - first_x) * square_width;

    painter.setPen(color);
    painter.drawText(QPoint(pixel_x, pixel_y), letter);
}

// Write a single colored character on a designated square
void MainWindow::display_square(s16b y, s16b x)
{
    dungeon_type *d_ptr = &dungeon_info[y][x];
    QChar square_char = d_ptr->dun_char;
    QColor square_color = d_ptr->dun_color;
    if (d_ptr->has_monster())
    {
        square_char = d_ptr->monster_char;
        square_color = d_ptr->monster_color;
    }
    else if (d_ptr->has_effect())
    {
        square_char = d_ptr->effect_char;
        square_color = d_ptr->effect_color;
    }
    else if (d_ptr->has_object())
    {
        square_char = d_ptr->object_char;
        square_color = d_ptr->object_color;
    }
    write_colored_text(square_char, square_color, y, x);
}

void MainWindow::screen_wipe()
{
    QPainter painter(this);
    QColor dark;
    QRect window_size;

    window_size.setTopLeft(QPoint(0,0));
    window_size.setBottomRight(QPoint(graphics_view->geometry().width(), graphics_view->geometry().height()));
    dark.setRgb(0,0,0,255);
    painter.fillRect(window_size, dark);
}

// Complete screen redraw
void MainWindow::screen_redraw()
{
    screen_wipe();

    for (int y = 0; y < last_y; y++)
    {
        s32b screen_y = y + first_y;

        for (int x =0; x < last_x; x++)
        {
            s32b screen_x = x + first_x;

            light_spot(screen_y, screen_x);
            display_square(screen_y, screen_x);
        }
    }
}

// determine of a dungeon square is onscreen at present
bool MainWindow::panel_contains(s16b y, s16b x)
{
    if (first_x > x) return (FALSE);
    if (last_x < x)  return (FALSE);
    if (first_y > y) return (FALSE);
    if (last_y < y)  return (FALSE);
    return (TRUE);
}

// Try to center the onscreen map around the player.
// should be followed by a total screen redraw
void MainWindow::set_onscreen_dungeon_boundries()
{
    /*
     * First find the upper left boundries
     */
    first_y = p_ptr->py - (window_height / 2);
    if (first_y < 0) first_y = 0;
    first_x = p_ptr->px - (window_width / 2);
    if (first_x < 0) first_x = 0;

    // Now find the lower right boundries
    last_y = first_y + window_height;
    last_x = first_x + window_width;

    // Verify the top and bottom boundries of the dungeon.
    if (last_y > p_ptr->cur_map_hgt)
    {
        last_y = p_ptr->cur_map_hgt - 1;
        first_y = last_y - p_ptr->cur_map_hgt;

        //Maybe the screen is higher than the dungeon
        if (first_y < 0)
        {
            first_y = 0;
        }
    }

    // Verify the top and bottom boundries of the dungeon.
    if (last_y >= p_ptr->cur_map_hgt)
    {
        last_y = p_ptr->cur_map_hgt - 1;
        first_y = last_y - p_ptr->cur_map_hgt;

        //Maybe the screen is higher than the dungeon
        if (first_y < 0)
        {
            first_y = 0;
        }
    }

    // Verify the left and right boundries of the dungeon.
    if (last_x >= p_ptr->cur_map_wid)
    {
        last_x = p_ptr->cur_map_wid - 1;
        first_x = last_x - p_ptr->cur_map_wid;

        //Maybe the screen is wider than the dungeon
        if (first_x < 0)
        {
            first_x = 0;
        }
    }
}

/*
 *Set up the dungeon map according to the curren screen size
 * Should be called every time the map or font is re-sized.
 * or any other action that affects the main widget's size or dimensions.
 */
void MainWindow::set_map()
{
    QFontMetrics font_metrics(cur_font);

    if (bigtile_act->isChecked()) use_bigtile = TRUE;
    else use_bigtile = FALSE;

    window_height = graphics_view->geometry().height(); // in pixels
    window_width = graphics_view->geometry().width();  // in pixels

    square_height = font_metrics.height(); // in pixels
    square_width = font_metrics.width('X');   // in pixels

    screen_num_rows = window_height / square_height;
    screen_num_columns = window_width / square_width;

    // TODO factor in bigscreen.
    //bool use_bigtile;
}
