#include "src/npp.h"
#include "src/qt_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>

/*
 * Returns a "rating" of x depending on y, and sets "attr" to the
 * corresponding "attribute".
 */
QString likert(int x, int y, byte *attr)
{
    /* Paranoia */
    if (y <= 0) y = 1;

    /* Negative value */
    if (x < 0)
    {
        *attr = TERM_RED;
        return ("Very Bad");
    }

    /* Analyze the value */
    switch ((x / y))
    {
        case 0:
        case 1:
        {
            *attr = TERM_RED;
            return ("Bad");
        }
        case 2:
        {
            *attr = TERM_RED;
            return ("Poor");
        }
        case 3:
        case 4:
        {
            *attr = TERM_YELLOW;
            return ("Fair");
        }
        case 5:
        {
            *attr = TERM_YELLOW;
            return ("Good");
        }
        case 6:
        {
            *attr = TERM_YELLOW;
            return ("Very Good");
        }
        case 7:
        case 8:
        {
            *attr = TERM_L_GREEN;
            return ("Excellent");
        }
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        {
            *attr = TERM_L_GREEN;
            return ("Superb");
        }
        case 14:
        case 15:
        case 16:
        case 17:
        {
            *attr = TERM_L_GREEN;
            return ("Heroic");
        }
        default:
        {
            *attr = TERM_L_GREEN;
            return ("Legendary");
        }
    }
}

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
// Capitilize the first character in a string.
QString capitilize_first(QString line)
{
    // Paranoia
    if (line.isEmpty()) return (line);
    QChar first = line[0];
    first.toUpper();
    line[0] = first;
    return (line);

}

void pop_up_message_box(QString message, QMessageBox::Icon the_icon)
{
    QMessageBox msg_box;
    msg_box.setIcon(the_icon);
    msg_box.setText(message);
    msg_box.exec();
}


// Post a question to the player and wait for a yes/no response.
// return TRUE/FALSE;
bool get_check(QString question)
{
    QMessageBox get_check;
    // get_check.setParent();  TODO - set as parent of main window
    get_check.setWindowTitle("Please respond yes or no");
    get_check.setText(question);
    get_check.setStandardButtons(QMessageBox::Yes);
    get_check.setStandardButtons(QMessageBox::No);
    if (get_check.exec() == QMessageBox::Yes) return (TRUE);
    else return (FALSE);
}

// Post a question to the player and wait for player to enter a string.
// returns the string;
QString get_string(QString question)
{

    bool ok;
    QString text = QInputDialog::getText(0, "Please enter a response", question, QLineEdit::Normal,"", &ok);

    if (!ok) return NULL;

    return (text);
}



QColor add_preset_color(int which_color)
{
    QColor color;

    color.setRgb(preset_colors[which_color].red, preset_colors[which_color].green, preset_colors[which_color].blue, 255);

    return (color);
}

/*
 *  Add a message - assume the color of white
 *This should be the only function to add to the message list, to make sure
 *it never gets larger than 250 messages
 */
static void add_message_to_vector(QString msg, QColor which_color)
{
    message_type message_body;
    message_type *msg_ptr = &message_body;

    // First make sure the message list gets no greater than 200
    while (message_list.size() >= 200)
    {
        message_list.QVector::erase(message_list.end());
    }

    // Default is a while message
    msg_ptr->msg_color = which_color;

    msg_ptr->message = msg;
    msg_ptr->message_turn = turn;

    // Add the message at the beginning of the list
    message_list.prepend(message_body);
}

/*
 *  Add a message with a preset color
 */
void color_message(QString msg, int which_color)
{
    QColor msg_color;

    //Paranoia
    if (which_color > MAX_COLORS) which_color = TERM_WHITE;

    // Default is a while message
    msg_color = defined_colors[which_color];

    add_message_to_vector(msg, msg_color);
}

//  Add a message - assume the color of white
void message(QString msg)
{
    add_message_to_vector(msg, add_preset_color(TERM_WHITE));
}

//  Add a message with any 24 bit color
void custom_color_message(QString msg, byte red, byte green, byte blue)
{
    QColor msg_color;

    msg_color.setRgb(red, green, blue, 255);

    add_message_to_vector(msg, msg_color);
}

static bool repeat_prev_allowed;

void cmd_enable_repeat(void)
{
    repeat_prev_allowed = TRUE;
}

void cmd_disable_repeat(void)
{
    repeat_prev_allowed = FALSE;
}



/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
static void ang_sort_aux(void *u, void *v, int p, int q)
{
    int z, a, b;

    /* Done sort */
    if (p >= q) return;

    /* Pivot */
    z = p;

    /* Begin */
    a = p;
    b = q;

    /* Partition */
    while (TRUE)
    {
        /* Slide i2 */
        while (!(*ang_sort_comp)(u, v, b, z)) b--;

        /* Slide i1 */
        while (!(*ang_sort_comp)(u, v, z, a)) a++;

        /* Done partition */
        if (a >= b) break;

        /* Swap */
        (*ang_sort_swap)(u, v, a, b);

        /* Advance */
        a++, b--;
    }

    /* Recurse left side */
    ang_sort_aux(u, v, p, b);

    /* Recurse right side */
    ang_sort_aux(u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(void *u, void *v, int n)
{
    /* Sort the array */
    ang_sort_aux(u, v, 0, n-1);
}


// Write a note to the notes file
void write_note(QString note, s16b depth)
{
    (void)note;
    (void)depth;
    // TODO write note
}

QString get_player_title(void)
{
    if (game_mode == GAME_NPPMORIA)
    {
        return (cp_ptr->cl_title[p_ptr->lev-1]);
    }

    return (cp_ptr->cl_title[(p_ptr->lev - 1) / 5]);
}
