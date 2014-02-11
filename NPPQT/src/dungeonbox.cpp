#include "dungeonbox.h"
#include "npp.h"

DungeonBox::DungeonBox(QWidget *parent) :
    QPlainTextEdit(parent)
{
    // Any fixed-width font works
    QFont font = QFont("Courier New");
    this->setFont(font);
}

void DungeonBox::keyPressEvent(QKeyEvent *e)
{
    int dir = 0;
    int key = e->key();

    switch (key) {
    case Qt::Key_Left: dir = 4; break;
    case Qt::Key_Right: dir = 6; break;
    case Qt::Key_Up: dir = 8; break;
    case Qt::Key_Down: dir = 2; break;
    }

    if (dir > 0) {
        move_player(dir, 0);
        this->redraw();
    }
}

void DungeonBox::redraw()
{
    this->clear();

    for (int y = 0; y < p_ptr->cur_map_hgt; y++) {
        QString line = QString("");
        for (int x = 0; x < p_ptr->cur_map_wid; x++) {
            light_spot(y, x);
            dungeon_type *d_ptr = &(dungeon_info[y][x]);
            QChar chr = d_ptr->dun_char;
            if (d_ptr->has_monster()) {
                chr = d_ptr->monster_char;
            }
            else if (d_ptr->has_object()) {
                chr = d_ptr->object_char;
            }
            line.append(chr);
        }
        line.append(QChar::CarriageReturn);
        this->appendPlainText(line);
    }
}
