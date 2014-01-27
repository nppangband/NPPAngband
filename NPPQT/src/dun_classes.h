#ifndef DUN_CLASSES_H
#define DUN_CLASSES_H

#include <QColor>
#include <QChar>

enum
{
    FLOOR_LIGHT_DIM = 0,
    FLOOR_LIGHT_NORMAL,
    FLOOR_LIGHT_BRIGHT,
};

enum
{
    OBJ_SYMBOL_NONE = 0,
    OBJ_SYMBOL_SQUELCH,
    OBJ_SYMBOL_PILE,
};

class dungeon_type
{
public:

    //  the 4 layers of the dungeon, the last 3 are indexes to lists which store the information
    s16b feat;
    s16b object_idx;
    u16b effect_idx;
    s16b monster_idx;

    u16b cave_info;
    byte special_lighting;
    bool dtrap;
    byte obj_special_symbol;

    //display information for the 4 layers
    QColor dun_color;
    QChar  dun_char;
    QColor object_color;
    QChar  object_char;
    QColor effect_color;
    QChar  effect_char;
    QColor monster_color;
    QChar  monster_char;

    bool has_object();
    bool has_effect();
    bool has_monster();

};

#endif // DUN_CLASSES_H
