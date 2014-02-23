#include <QtWidgets>
#include <QHash>
#include <QTextStream>
#include <QGraphicsItem>

#include "src/npp.h"
#include "src/qt_mainwindow.h"
#include "src/init.h"
#include "src/optionsdialog.h"
#include "src/birthdialog.h"
#include "src/dungeonbox.h"

static MainWindow *main_window = 0;

class MainWindowPrivate;

class DungeonGrid: public QGraphicsItem
{
public:
    DungeonGrid(int _x, int _y, MainWindowPrivate *_parent);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    void cellSizeChanged();

    MainWindowPrivate *parent;
    int x, y;
};

class DungeonCursor: public QGraphicsItem
{
public:
    MainWindowPrivate *parent;
    int x, y;

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    DungeonCursor(MainWindowPrivate *_parent);
    void moveTo(int y, int x);

    void cellSizeChanged();
};

class MainWindowPrivate
{
public:
    DungeonGrid *grids[MAX_DUNGEON_HGT][MAX_DUNGEON_WID];

    QGraphicsScene *scene;
    QGraphicsView *view;
    int font_hgt, font_wid;
    int tile_hgt, tile_wid;
    int cell_hgt, cell_wid;
    QFont font;
    bool do_pseudo_ascii;

    QPixmap blank_pix;
    // The key must me strings of the form "[row]x[col]"
    QHash<QString, QPixmap> tiles;
    QPixmap tile_map;

    DungeonCursor *cursor;

    MainWindowPrivate();
    void init_scene(QGraphicsScene *_scene, QGraphicsView *_view, QFont _font);
    void redraw();
    bool panel_contains(int y, int x);
    void set_font(QFont _font);
    void set_graphic_mode(int mode);
    void destroy_tiles();
    void rebuild_tile(QString key);
    void calculate_cell_size();
    QPixmap darken_pix(QPixmap src);
    QPixmap lighten_pix(QPixmap src);
    QPixmap colorize_pix(QPixmap src, QColor color);
    QPixmap gray_pix(QPixmap src);
    QPixmap pseudo_ascii(QChar chr, QColor color);
    void update_cursor();
};

void DungeonCursor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void DungeonGrid::cellSizeChanged()
{
    prepareGeometryChange();
}

void DungeonCursor::cellSizeChanged()
{
    prepareGeometryChange();
}

void DungeonGrid::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!character_dungeon) return;

    parent->cursor->setVisible(true);
    parent->cursor->moveTo(y, x);
    parent->view->viewport()->update(); // Hack -- Force full redraw, to eliminate bug in QT
}

void MainWindowPrivate::update_cursor()
{
    cursor->moveTo(p_ptr->py, p_ptr->px);
    cursor->setVisible(hilight_player);
}

DungeonCursor::DungeonCursor(MainWindowPrivate *_parent)
{
    parent = _parent;
    x = y = 0;
    setZValue(100);
}

QRectF DungeonCursor::boundingRect() const
{
    return QRectF(0, 0, parent->cell_wid, parent->cell_hgt);
}

void DungeonCursor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!character_dungeon) return;

    painter->save();
    painter->setPen(QColor("yellow"));
    painter->drawRect(0, 0, parent->cell_wid - 1, parent->cell_hgt - 1);
    if ((parent->cell_wid > 16) && (parent->cell_hgt > 16)){
        int z = 3;
        painter->drawRect(0, 0, z, z);
        painter->drawRect(parent->cell_wid - z - 1, 0, z, z);
        painter->drawRect(0, parent->cell_hgt - z - 1, z, z);
        painter->drawRect(parent->cell_wid - z - 1, parent->cell_hgt - z - 1, z, z);
    }
    painter->restore();
}

void DungeonCursor::moveTo(int _y, int _x)
{
    x = _x;
    y = _y;
    setPos(x * parent->cell_wid, y * parent->cell_hgt);
}

MainWindowPrivate::MainWindowPrivate()
{
    view = 0;
    scene = 0;
    cursor = new DungeonCursor(this);
}

DungeonGrid::DungeonGrid(int _x, int _y, MainWindowPrivate *_parent)
{
    x = _x;
    y = _y;
    parent = _parent;
    setZValue(0);
}

QRectF DungeonGrid::boundingRect() const
{
    return QRectF(0, 0, parent->cell_wid, parent->cell_hgt);
}

void DungeonGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillRect(QRectF(0, 0, parent->cell_wid, parent->cell_hgt), Qt::black);

    if (!character_dungeon) return;

    dungeon_type *d_ptr = &dungeon_info[y][x];
    QChar square_char = d_ptr->dun_char;
    QColor square_color = d_ptr->dun_color;
    bool empty = true;
    u32b flags = 0;
    QString key2;
    qreal opacity = 1;
    bool do_shadow = false;

    flags = (d_ptr->ui_flags & (UI_LIGHT_BRIGHT | UI_LIGHT_DIM | UI_LIGHT_TORCH | UI_COSMIC_TORCH));

    // Draw visible monsters
    if (d_ptr->has_visible_monster())
    {
        square_char = d_ptr->monster_char;
        square_color = d_ptr->monster_color;

        empty = false;

        if (!parent->do_pseudo_ascii) key2 = d_ptr->monster_tile;
        else do_shadow = true;

        flags |= (d_ptr->ui_flags & UI_TRANSPARENT_MONSTER);
        opacity = 0.5;
    }
    // Draw effects
    else if (d_ptr->has_visible_effect())
    {
        square_char = d_ptr->effect_char;
        square_color = d_ptr->effect_color;

        empty = false;

        key2 = d_ptr->effect_tile;

        flags |= (d_ptr->ui_flags & UI_TRANSPARENT_EFFECT);
        opacity = 0.7;
    }
    // Draw objects
    else if (d_ptr->has_visible_object())
    {
        square_char = d_ptr->object_char;
        square_color = d_ptr->object_color;

        empty = false;

        key2 = d_ptr->object_tile;
    }

    bool done_bg = false;
    bool done_fg = false;

    painter->save();

    if (use_graphics) {
        // Draw background tile
        QString key1 = d_ptr->dun_tile;

        if (key1.length() > 0) {
            parent->rebuild_tile(key1);
            QPixmap pix = parent->tiles[key1];

            if (flags & UI_LIGHT_TORCH) {
                QColor color = QColor("yellow").darker(150);
                if (flags & UI_COSMIC_TORCH) color = QColor("cyan").darker(150);
                pix = parent->colorize_pix(pix, color);
            }
            else if (flags & UI_LIGHT_BRIGHT) {
                pix = parent->darken_pix(pix);
            }
            else if (flags & UI_LIGHT_DIM) {
                pix = parent->gray_pix(pix);
            }

            painter->drawPixmap(pix.rect(), pix, pix.rect());
            done_bg = true;

            // Draw foreground tile
            if (key2.length() > 0) {
               parent->rebuild_tile(key2);
               QPixmap pix = parent->tiles.value(key2);
               if (flags & (UI_TRANSPARENT_EFFECT | UI_TRANSPARENT_MONSTER)) {
                   painter->setOpacity(opacity);
               }
               painter->drawPixmap(pix.rect(), pix, pix.rect());
               painter->setOpacity(1);
               done_fg = true;
            }

            if (do_shadow) {
                QPixmap pix = parent->pseudo_ascii(square_char, square_color);
                painter->drawPixmap(pix.rect(), pix, pix.rect());
                done_fg = true;
            }
        }
    }

    if (!done_fg && (!empty || !done_bg)) {
        painter->setFont(parent->font);
        painter->setPen(square_color);
        painter->drawText(QRectF(0, 0, parent->cell_wid, parent->cell_hgt),
                          Qt::AlignCenter, QString(square_char));
    }

    // Draw a mark for visible artifacts
    if (d_ptr->has_visible_artifact()) {
        int s = 6;
        QPointF points[] = {
            QPointF(parent->cell_wid - s, parent->cell_hgt),
            QPointF(parent->cell_wid, parent->cell_hgt),
            QPointF(parent->cell_wid, parent->cell_hgt - s)
        };
        painter->setBrush(QColor("violet"));
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(points, 3);
    }

    painter->restore();
}

QPixmap MainWindowPrivate::gray_pix(QPixmap src)
{
    QImage img = src.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor col = QColor(img.pixel(x, y)).darker();
            int gray = qGray(col.rgb());
            img.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap MainWindowPrivate::colorize_pix(QPixmap src, QColor color)
{
    QImage img = src.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_HardLight);
    p.fillRect(img.rect(), color);
    QPixmap pix = QPixmap::fromImage(img);
    return pix;
}

QPixmap MainWindowPrivate::darken_pix(QPixmap src)
{
    QImage img = src.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_HardLight);
    p.fillRect(img.rect(), QColor("#444"));
    QPixmap pix = QPixmap::fromImage(img);
    return pix;    
}

QPixmap MainWindowPrivate::lighten_pix(QPixmap src)
{
    QImage img = src.toImage();
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_HardLight);
    p.fillRect(img.rect(), QColor("#999"));
    QPixmap pix = QPixmap::fromImage(img);
    return pix;
}

void MainWindowPrivate::destroy_tiles()
{
    tiles.clear();
}

void MainWindowPrivate::calculate_cell_size()
{
    cell_hgt = font_hgt;
    if (tile_hgt > cell_hgt) cell_hgt = tile_hgt;

    cell_wid = font_wid;
    if (tile_wid > cell_wid) cell_wid = tile_wid;

    for (int y = 0; y < MAX_DUNGEON_HGT; y++) {
        for (int x = 0; x < MAX_DUNGEON_WID; x++) {
            grids[y][x]->cellSizeChanged();
            grids[y][x]->setPos(x * cell_wid, y * cell_hgt);
        }
    }

    cursor->cellSizeChanged();
}

void MainWindowPrivate::set_graphic_mode(int mode)
{
    int hgt, wid;
    QString fname;

    switch (mode) {
    case GRAPHICS_DAVID_GERVAIS:
        hgt = 32;
        wid = 32;
        fname.append("32x32.png");
        break;    
    default:
        hgt = 0;
        wid = 0;
        break;
    }

    if (fname.length() > 0) {
        fname.prepend(NPP_DIR_GRAF);
        QPixmap pix = QPixmap(fname);
        if (pix.isNull()) {
            pop_up_message_box(QString("Can't load tiles"));
            return;
        }
        tile_hgt = hgt;
        tile_wid = wid;
        calculate_cell_size();
        destroy_tiles();
        tile_map = pix;
        init_graphics();
    }
    // Go to text mode
    else {
        tile_hgt = hgt;
        tile_wid = wid;
        calculate_cell_size();
        destroy_tiles();
        tile_map = blank_pix;
        clear_graphics();   
    }

    use_graphics = mode;    
}

// Tile creation on demmand
void MainWindowPrivate::rebuild_tile(QString key)
{
    // Already created
    if (tiles.contains(key)) return;

    QList<QString> coords = key.split("x");
    if (coords.size() != 2) return;
    int x = coords.at(1).toInt();
    int y = coords.at(0).toInt();
    // Grab a chunk from the tile map
    QPixmap pix = tile_map.copy(x * tile_wid, y * tile_hgt, tile_wid, tile_hgt);
    // Scale if necessary
    if ((cell_wid != tile_wid) || (cell_hgt != tile_hgt)) {
        pix = pix.scaled(QSize(cell_wid, cell_hgt));
    }
    tiles.insert(key, pix);
}

void MainWindowPrivate::set_font(QFont _font)
{
    font = _font;
    QFontMetrics metrics(font);
    font_hgt = metrics.height();
    font_wid = metrics.width('M');

    calculate_cell_size();

    destroy_tiles();
}

void MainWindowPrivate::init_scene(QGraphicsScene *_scene, QGraphicsView *_view,
                                   QFont _font)
{
    scene = _scene;
    view = _view;

    do_pseudo_ascii = false;

    font = _font;
    QFontMetrics metrics(font);

    font_hgt = metrics.height();
    font_wid = metrics.width('M');
    tile_hgt = tile_wid = 0;
    cell_hgt = cell_wid = 0;

    QBrush brush(QColor("black"));
    scene->setBackgroundBrush(brush);

    blank_pix = QPixmap(1, 1);
    blank_pix.fill(QColor("black"));

    for (int y = 0; y < MAX_DUNGEON_HGT; y++) {
        for (int x = 0; x < MAX_DUNGEON_WID; x++) {
            grids[y][x] = new DungeonGrid(x, y, this);
            scene->addItem(grids[y][x]);
        }
    }

    scene->addItem(cursor);

    view->update();
}

void MainWindowPrivate::redraw()
{    
    // Important. No dungeon yet
    if (!character_dungeon) {
        if (view) view->update();
        return;
    }

    // TODO REMOVE THIS
    wiz_light();

    // Adjust scrollbars
    view->setSceneRect(0, 0, p_ptr->cur_map_wid * cell_wid, p_ptr->cur_map_hgt * cell_hgt);

    for (int y = 0; y < p_ptr->cur_map_hgt; y++) {
        for (int x = 0; x < p_ptr->cur_map_wid; x++) {
            light_spot(y, x);
        }
    }

    //ui_center(p_ptr->py, p_ptr->px);
    update_cursor();
    view->viewport()->update(); // Hack -- Force full redraw
}

bool MainWindowPrivate::panel_contains(int y, int x)
{
    QPolygonF pol = view->mapToScene(view->viewport()->geometry());
    // We test top-left and bottom-right corners of the cell
    QPointF point1(x * cell_wid, y * cell_hgt);
    QPointF point2(x * cell_wid + cell_wid, y * cell_hgt + cell_hgt);
    return pol.containsPoint(point1, Qt::OddEvenFill) && pol.containsPoint(point2, Qt::OddEvenFill);
}

QPixmap MainWindowPrivate::pseudo_ascii(QChar chr, QColor color)
{
    QImage img(cell_wid, cell_hgt, QImage::Format_ARGB32);
    // Fill with transparent color
    for (int x = 0; x < cell_wid; x++) {
        for (int y = 0; y < cell_hgt; y++) {
            img.setPixel(x, y, QColor(0, 0, 0, 0).rgba());
        }
    }

    QPainter p(&img);
    p.setPen(color);
    p.setFont(font);
    // Draw the text once to get the shape of the letter plus antialiasing
    p.drawText(img.rect(), Qt::AlignCenter, QString(chr));

    // Mark colored grids
    bool marks[img.width()][img.height()];
    for (int x = 0; x < cell_wid; x++) {
        for (int y = 0; y < cell_hgt; y++) {
            QRgb pixel = img.pixel(x, y);
            if (qAlpha(pixel) > 0) {
                marks[x][y] = true;
            }
            else {
                marks[x][y] = false;
            }
        }
    }

    // Surround with black. Note that all concerning grids are burned, even marked ones
    for (int x = 0; x < cell_wid; x++) {
        for (int y = 0; y < cell_hgt; y++) {
            if (!marks[x][y]) continue;
            for (int y1 = y - 1; y1 <= y + 1; y1++) {
                for (int x1 = x - 1; x1 <= x + 1; x1++) {
                    if (!img.rect().contains(x1, y1, false)) continue;
                    img.setPixel(x1, y1, qRgba(0, 0, 0, 255));
                }
            }
        }
    }

    // Draw the text again so the antialiasing pixels blend with black properly
    p.drawText(img.rect(), Qt::AlignCenter, QString(chr));

    return QPixmap::fromImage(img);
}

// The main function - intitalize the main window and set the menus.
MainWindow::MainWindow()
{
    // Store a reference for public functions (panel_contains and others)
    if (!main_window) main_window = this;

    setAttribute(Qt::WA_DeleteOnClose);

    priv = new MainWindowPrivate;

    dungeon_scene = new QGraphicsScene;
    graphics_view = new QGraphicsView(dungeon_scene);
    //graphics_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

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
    priv->init_scene(dungeon_scene, graphics_view, cur_font);
    priv->set_graphic_mode(use_graphics);

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

    character_loaded = character_dungeon = character_generated = character_icky = false;

    update_file_menu_game_inactive();

    // close game
    cleanup_npp_games();

    priv->destroy_tiles();
    priv->redraw();
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
    ui_redraw_all();
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
        priv->set_font(cur_font);
        priv->redraw();
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

    ascii_mode_act = new QAction(tr("Ascii graphics"), this);
    ascii_mode_act->setStatusTip(tr("Set the graphics to ascii mode."));
    connect(ascii_mode_act, SIGNAL(triggered()), this, SLOT(set_ascii()));

    dvg_mode_act = new QAction(tr("David Gervais tiles"), this);
    dvg_mode_act->setStatusTip(tr("Set the graphics to David Gervais tiles mode."));
    connect(dvg_mode_act, SIGNAL(triggered()), this, SLOT(set_dvg()));

    pseudo_ascii_act = new QAction(tr("Pseudo-Ascii monsters"), this);
    pseudo_ascii_act->setCheckable(true);
    pseudo_ascii_act->setChecked(false);
    pseudo_ascii_act->setStatusTip(tr("Set the monsters graphics to pseudo-ascii."));
    connect(pseudo_ascii_act, SIGNAL(changed()), this, SLOT(set_pseudo_ascii()));

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

void MainWindow::set_dvg()
{
    //popup1("hola");
    main_window->priv->set_graphic_mode(GRAPHICS_DAVID_GERVAIS);
    ui_redraw_all();
}

void MainWindow::set_ascii()
{
    main_window->priv->set_graphic_mode(0);
    ui_redraw_all();
}

void MainWindow::set_pseudo_ascii()
{
    main_window->priv->do_pseudo_ascii = pseudo_ascii_act->isChecked();
    ui_redraw_all();
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
    settings->addAction(ascii_mode_act);
    settings->addAction(dvg_mode_act);
    settings->addAction(pseudo_ascii_act);
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
    priv->do_pseudo_ascii = settings.value("pseudo_ascii", false).toBool();
    pseudo_ascii_act->setChecked(priv->do_pseudo_ascii);
    use_graphics = settings.value("use_graphics", 0).toInt();

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
    settings.setValue("pseudo_ascii", priv->do_pseudo_ascii);
    settings.setValue("use_graphics", use_graphics);
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
    if (!panel_contains(y, x)) return;

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
bool panel_contains(int y, int x)
{
    return main_window->priv->panel_contains(y, x);
}

void ui_ensure(int y, int x)
{
    MainWindowPrivate *priv = main_window->priv;
    main_window->priv->view->ensureVisible(QRectF(x * priv->cell_wid, y * priv->cell_hgt, priv->cell_wid, priv->cell_hgt));
}

void ui_center(int y, int x)
{
    MainWindowPrivate *priv = main_window->priv;
    main_window->priv->view->centerOn(x * priv->cell_wid, y * priv->cell_hgt);
}

void ui_redraw_grid(int y, int x)
{
    DungeonGrid *g_ptr = main_window->priv->grids[y][x];
    g_ptr->setVisible(true);
    g_ptr->update(g_ptr->boundingRect());
}

void ui_redraw_all()
{
    main_window->priv->redraw();
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
