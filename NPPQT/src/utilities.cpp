#include "src/npp.h"
#include "src/qt_mainwindow.h"
#include <QMessageBox>

typedef struct letters_and_numbers letters_and_numbers;

struct letters_and_numbers
{
    QChar let;
    int num;
};

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


// There is probably a better way with QChar, but I can't find it.
int letter_to_number (QChar let)
{
   // Make sure we are dealing with lowercase letters.
    let.toLower();
    if (!let.isLower()) return 0;

    for (int i = 0; i < 26; i++)
    {
        letters_and_numbers *ln_ptr =& lowercase_and_numbers[i];
        if (ln_ptr->let == let) return (ln_ptr->num);
    }

    /* all else - just return zero*/
    return 0;
}

// There is probably a better way with QChar, but I can't find it.
QChar number_to_letter (int num)
{
    for (int i = 0; i < 26; i++)
    {
        letters_and_numbers *ln_ptr =& lowercase_and_numbers[i];
        if (ln_ptr->num == num) return (ln_ptr->let);
    }

    /* all else - just return zero*/
    return 0;

    /* all else - Paranoia*/
    return ('a');
}

bool is_a_vowel(QChar single_letter)
{
    // Make sure we are dealing with lowercase letters.
     single_letter.toLower();
     if (!single_letter.isLower()) return FALSE;

    if (single_letter == 'a') return TRUE;
    if (single_letter == 'e') return TRUE;
    if (single_letter == 'i') return TRUE;
    if (single_letter == 'o') return TRUE;
    if (single_letter == 'u') return TRUE;

    return (FALSE);
}

void pop_up_message_box(QString message)
{
    QMessageBox msg_box;

    msg_box.setText(message);
    msg_box.exec();
}
