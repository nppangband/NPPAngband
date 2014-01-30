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

class effect_type
{
public:

    byte x_type;            /* Effect Type */

    u16b x_f_idx;           /* Effect Feature IDX */

    byte x_cur_y;			/* Current y location, or countdown_base */
    byte x_cur_x;			/* Current x location, or countdown_rand */

    byte x_countdown;       /* Number of turns effect has left */
    byte x_repeats;			/* Number of times the effect repeats*/

    u16b x_power;           /* Strength of effect */

    s16b x_source;          /* Source of effect - THIS MUST BE THE RACE of the monster, not the mon_idx of the creature. */

    u16b x_flags;           /* Effect "memory" bitflags */

    s16b next_x_idx;		/* Idx of next effect at this square. */

    s16b x_r_idx;           /* Some monster race index. Used for inscriptions */

    void effect_wipe();
};


#endif // DUN_CLASSES_H
