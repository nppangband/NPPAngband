#ifndef UTILITIES_H
#define UTILITIES_H

#include <QVector>

typedef struct letters_and_numbers letters_and_numbers;
typedef struct message_type message_type;

struct letters_and_numbers
{
    QChar let;
    int num;
};

struct message_type
{
    QColor msg_color;
    QString message;
    s32b message_turn;
};

extern QVector<message_type> message_list;

static letters_and_numbers lowercase_and_numbers[26] =
{
    { 'a', 0},
    { 'b', 1},
    { 'c', 2},
    { 'd', 3},
    { 'e', 4},
    { 'f', 5},
    { 'g', 6},
    { 'h', 7},
    { 'i', 8},
    { 'j', 9},
    { 'k', 10},
    { 'l', 11},
    { 'm', 12},
    { 'n', 13},
    { 'o', 14},
    { 'p', 15},
    { 'q', 16},
    { 'r', 17},
    { 's', 18},
    { 't', 19},
    { 'u', 20},
    { 'v', 21},
    { 'w', 22},
    { 'x', 23},
    { 'y', 24},
    { 'z', 25}
};


QString likert(int x, int y, byte *attr);
QString _num(int n);


#endif // UTILITIES_H
