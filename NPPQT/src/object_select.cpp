
/* File: was obj-ui.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * 						Jeff Greene, Diego Gonzalez
 *
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */


#include "src/object_select.h"

// Receives the number of the button pressed.
void ObjectSelectDialog::button_press(QString num_string)
{
    selected_button = num_string.toInt();

    this->accept();
}

// See if the user selected a button bia a keypress.
void ObjectSelectDialog::keyPressEvent(QKeyEvent* which_key)
{
    // Handle escape key
    if (which_key->key() == Qt::Key_Escape) {
        this->close();
        return;
    }

    QString key_pressed = which_key->text();

    //  Make sure we are dealing with a letter
    if (key_pressed.length() != 1 || !key_pressed.at(0).isLetter()) return;

    // Make it lowercase
    key_pressed = key_pressed.toLower();

    // Make sure we are dealing with the item name buttons and not the info buttons
    // or other future buttons
    key_pressed.append(") ");

    QWidget *tab = this->object_tabs->currentWidget(); // Search in the current tab
    QList<QPushButton *> buttons = tab->findChildren<QPushButton *>();
    for (int i = 0; i < buttons.size(); i++)
    {
        if (buttons.at(i)->text().startsWith(key_pressed))
        {
            buttons.at(i)->click();
            break;
        }
    }
}

// Receives the number of the button pressed.
void ObjectSelectDialog::help_press(QString num_string)
{
    int help_obj_value = num_string.toInt();

    int object_num = get_selected_object(help_obj_value);

    object_type *o_ptr;

    /* Inventory or floor */
    if (object_num >= 0)
        o_ptr = &inventory[object_num];
    else
        o_ptr = &o_list[0 - object_num];

    object_info_screen(o_ptr);
}

// Format the "equipment description", ex. "about body"
QString ObjectSelectDialog::add_equip_use(int slot)
{
    QString equip_add = QString ("%1") .arg(mention_use(slot));

    while (equip_add.length() < 14) equip_add.append(" ");
    equip_add.append(": ");

    return (equip_add);
}

void ObjectSelectDialog::track_longest_object_name(object_type *o_ptr, byte which_tab, int slot)
{
    QString o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    //Add description for equipment.
    if (which_tab == TAB_EQUIP)
    {
        o_name.prepend(add_equip_use(slot));
    }

    if (o_name.length() <= max_object_desc_length) return;
    max_object_desc_length = o_name.length();
}

QString ObjectSelectDialog::format_button_name(QChar char_index, object_type *o_ptr, byte which_tab, int slot)
{
    QString o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    //Add description for equipment.
    if (which_tab == TAB_EQUIP)
    {
        o_name.prepend(add_equip_use(slot));
    }


    while (o_name.length() < max_object_desc_length)o_name.append(" ");
    QString final_name = (QString("%1) %2") .arg(char_index) .arg(o_name));

    return (final_name);
}


void ObjectSelectDialog::floor_items_count(int mode, int sq_y, int sq_x)
{
    int this_o_idx, next_o_idx;

    // assume we aren't using the floor
    allow_floor = FALSE;

    if (!(mode & (USE_FLOOR))) return;

    // Wipe the vector
    floor_items.clear();

    /* Scan all objects in the grid */
    for (this_o_idx = dungeon_info[sq_y][sq_x].object_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        /* Get the object */
        object_type *o_ptr = &o_list[this_o_idx];

        /* Get the next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Verify item tester */
        if (!get_item_okay(0 - this_o_idx)) continue;

        /* Accept this item */
        floor_items.append(this_o_idx);
        allow_floor = TRUE;

        track_longest_object_name(o_ptr, TAB_FLOOR, this_o_idx);
    }
}

// Make the tab for the widget
void ObjectSelectDialog::build_floor_tab()
{
    QVBoxLayout *vlay = new QVBoxLayout;

    QWidget *aux = new QWidget;
    vlay->addWidget(aux);

    vlay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QGridLayout *object_layout = new QGridLayout;
    aux->setLayout(object_layout);

    // Only column 0 can resize
    object_layout->setColumnStretch(0, 100);

    // Give each one titles
    QLabel *object_header = new QLabel("Floor Items");
    QLabel *weight_header = new QLabel("Weight");
    QLabel *help_header = new QLabel("info");
    object_header->setAlignment(Qt::AlignLeft);
    weight_header->setAlignment(Qt::AlignRight);
    help_header->setAlignment(Qt::AlignHCenter);

    // Add the headers
    object_layout->addWidget(object_header, 0, 0);
    object_layout->addWidget(weight_header, 0, 1);
    object_layout->addWidget(help_header, 0, 2);


    // Make a button for each object.
    for (int i = 0; i < floor_items.size(); i++)
    {
        // Make the label.
        QChar which_char = number_to_letter(i);
        object_type *o_ptr = &o_list[floor_items[i]];

        // Make the button for the object.
        QString button_name = format_button_name(which_char, o_ptr, TAB_FLOOR, floor_items[i]);
        QString text_num = QString::number(num_buttons);
        QPushButton *button = new QPushButton(text_num);
        button->setText(button_name);
        button->setStyleSheet("Text-align:left");

        // Let the button tell us the number button that was clicked
        connect(button, SIGNAL(clicked()), button_values, SLOT(map()));
        button_values->setMapping(button, text_num);

        // Add a weight button
        QLabel *weight_label = new QLabel(format_object_weight(o_ptr));
        weight_label->setAlignment(Qt::AlignRight);

        // Add a help button to put up a description of the object.
        QPushButton *help_button = new QPushButton(text_num);
        help_button->setIcon(QIcon(":/icons/lib/icons/help.png"));
        help_button->setText(QString(""));
        connect(help_button, SIGNAL(clicked()), help_values, SLOT(map()));
        help_values->setMapping(help_button, text_num);

        // Add both buttons to the appropriate layout.
        object_layout->addWidget(button, (i+1), 0);
        object_layout->addWidget(weight_label, (i+1), 1);
        weight_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        object_layout->addWidget(help_button, (i+1), 2);

        // Keep track of the number of buttons.
        num_buttons++;
    }

    floor_tab->setLayout(vlay);
}

void ObjectSelectDialog::inven_items_count(int mode)
{
    // assume we aren't using the inventory
    allow_inven = FALSE;

    if (!(mode & (USE_INVEN))) return;

    // Wipe the vector
    inven_items.clear();

    /* Scan all objects in the inventory */
    for (int i = 0; i < (INVEN_WIELD - 1); i++)
    {

        /* Verify item tester */
        if (!get_item_okay(i)) continue;

        /* Accept this item */
        inven_items.append(i);
        allow_inven = TRUE;

        /* Get the object */
        object_type *o_ptr = &inventory[i];

        track_longest_object_name(o_ptr, TAB_INVEN, i);
    }
}

void ObjectSelectDialog::build_inven_tab()
{
    QVBoxLayout *vlay = new QVBoxLayout;

    QWidget *aux = new QWidget;
    vlay->addWidget(aux);

    vlay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QGridLayout *object_layout = new QGridLayout;
    aux->setLayout(object_layout);

    object_layout->setColumnStretch(0, 100);

    // Give each one titles
    QLabel *object_header = new QLabel("Inventory Items");
    QLabel *weight_header = new QLabel("Weight");
    QLabel *help_header = new QLabel("info");
    object_header->setAlignment(Qt::AlignLeft);
    weight_header->setAlignment(Qt::AlignRight);
    help_header->setAlignment(Qt::AlignHCenter);

    // Add the headers
    object_layout->addWidget(object_header, 0, 0);
    object_layout->addWidget(weight_header, 0, 1);
    object_layout->addWidget(help_header, 0, 2);

    // Make a button, a weight label, and a help button for each object.
    for (int i = 0; i < inven_items.size(); i++)
    {
        QChar which_char = number_to_letter(inven_items[i]);
        object_type *o_ptr = &inventory[inven_items[i]];

        // Make the button for the object.
        QString button_name = format_button_name(which_char, o_ptr, TAB_INVEN, inven_items[i]);
        QString text_num = QString::number(num_buttons);
        QPushButton *button = new QPushButton(text_num);
        button->setText(button_name);
        button->setStyleSheet("Text-align:left");

        // Let the button tell us the number button that was clicked
        connect(button, SIGNAL(clicked()), button_values, SLOT(map()));
        button_values->setMapping(button, text_num);

        // Add a weight button
        QLabel *weight_label = new QLabel(format_object_weight(o_ptr));
        weight_label->setAlignment(Qt::AlignRight);

        // Add a help button to put up a description of the object.
        QPushButton *help_button = new QPushButton(text_num);
        help_button->setIcon(QIcon(":/icons/lib/icons/help.png"));
        help_button->setText(QString(""));
        connect(help_button, SIGNAL(clicked()), help_values, SLOT(map()));
        help_values->setMapping(help_button, text_num);

        // Add both buttons to the appropriate layout.
        object_layout->addWidget(button, (i+1), 0);
        object_layout->addWidget(weight_label, (i+1), 1);
        weight_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        object_layout->addWidget(help_button, (i+1), 2);

        // Keep track of the number of buttons.
        num_buttons++;
    }

    inven_tab->setLayout(vlay);
}

void ObjectSelectDialog::equip_items_count(int mode)
{
    // assume we aren't using any equipment
    allow_equip = FALSE;

    // Wipe the vector
    equip_items.clear();

    if (!(mode & (USE_EQUIP))) return;

    /* Scan all pieces of equipment */
    for (int i = INVEN_WIELD; i < QUIVER_START; i++)
    {
        /* Verify item tester */
        if (!get_item_okay(i)) continue;

        /* Accept this item */
        equip_items.append(i);
        allow_equip = TRUE;

        /* Get the object */
        object_type *o_ptr = &inventory[i];

        track_longest_object_name(o_ptr, TAB_EQUIP, i);
    }
}

void ObjectSelectDialog::build_equip_tab()
{
    QVBoxLayout *vlay = new QVBoxLayout;

    QWidget *aux = new QWidget;
    vlay->addWidget(aux);

    vlay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QGridLayout *object_layout = new QGridLayout;
    aux->setLayout(object_layout);

    object_layout->setColumnStretch(0, 100);

    // Give each one titles
    QLabel *object_header = new QLabel("Equipment Items");
    QLabel *weight_header = new QLabel("Weight");
    QLabel *help_header = new QLabel("info");
    object_header->setAlignment(Qt::AlignLeft);
    weight_header->setAlignment(Qt::AlignRight);
    help_header->setAlignment(Qt::AlignHCenter);

    // Add the headers
    object_layout->addWidget(object_header, 0, 0);
    object_layout->addWidget(weight_header, 0, 1);
    object_layout->addWidget(help_header, 0, 2);

    // Make a button for each object.
    for (int i = 0; i < equip_items.size(); i++)
    {
        QChar which_char = number_to_letter(equip_items[i]-INVEN_WIELD);
        object_type *o_ptr = &inventory[equip_items[i]];

        // Make the button for the object.
        QString button_name = format_button_name(which_char, o_ptr, TAB_EQUIP, equip_items[i]);
        QString text_num = QString::number(num_buttons);
        QPushButton *button = new QPushButton(text_num);
        button->setText(button_name);
        button->setStyleSheet("Text-align:left");

        // Let the button tell us the number button that was clicked
        connect(button, SIGNAL(clicked()), button_values, SLOT(map()));
        button_values->setMapping(button, text_num);

        // Add a weight button
        QLabel *weight_label = new QLabel(format_object_weight(o_ptr));
        weight_label->setAlignment(Qt::AlignRight);

        // Add a help button to put up a description of the object.
        QPushButton *help_button = new QPushButton(text_num);
        help_button->setIcon(QIcon(":/icons/lib/icons/help.png"));
        help_button->setText(QString(""));
        connect(help_button, SIGNAL(clicked()), help_values, SLOT(map()));
        help_values->setMapping(help_button, text_num);

        // Add both buttons to the appropriate layout.
        object_layout->addWidget(button, (i+1), 0);
        object_layout->addWidget(weight_label, (i+1), 1);
        weight_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        object_layout->addWidget(help_button, (i+1), 2);

        // Keep track of the number of buttons.
        num_buttons++;
    }

    equip_tab->setLayout(vlay);
}

void ObjectSelectDialog::quiver_items_count(int mode)
{
    // assume we aren't using the quiver
    allow_quiver = FALSE;

    // Wipe the vector
    quiver_items.clear();

    if (!(mode & (USE_QUIVER))) return;

    /* Scan all objects in the quiver */
    for (int i = QUIVER_START; i < QUIVER_END; i++)
    {

        /* Verify item tester */
        if (!get_item_okay(i)) continue;

        /* Accept this item */
        quiver_items.append(i);
        allow_quiver = TRUE;

        /* Get the object */
        object_type *o_ptr = &inventory[i];

        track_longest_object_name(o_ptr, TAB_QUIVER, i);
    }

    return;
}

void ObjectSelectDialog::build_quiver_tab()
{
    QVBoxLayout *vlay = new QVBoxLayout;

    QWidget *aux = new QWidget;
    vlay->addWidget(aux);

    vlay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QGridLayout *object_layout = new QGridLayout;
    aux->setLayout(object_layout);

    object_layout->setColumnStretch(0, 100);

    // Give each one titles
    QLabel *object_header = new QLabel("Quiver Items");
    QLabel *weight_header = new QLabel("Weight");
    QLabel *help_header = new QLabel("info");
    object_header->setAlignment(Qt::AlignLeft);
    weight_header->setAlignment(Qt::AlignRight);
    help_header->setAlignment(Qt::AlignHCenter);

    // Add the headers
    object_layout->addWidget(object_header, 0, 0);
    object_layout->addWidget(weight_header, 0, 1);
    object_layout->addWidget(help_header, 0, 2);

    // Make a button for each object.
    for (int i = 0; i < quiver_items.size(); i++)
    {
        QChar which_char = number_to_letter(quiver_items[i]-QUIVER_START);
        object_type *o_ptr = &inventory[quiver_items[i]];

        // Make the button for the object.
        QString button_name = format_button_name(which_char, o_ptr, TAB_QUIVER, quiver_items[i]);
        QString text_num = QString::number(num_buttons);
        QPushButton *button = new QPushButton(text_num);
        button->setText(button_name);
        button->setStyleSheet("Text-align:left");

        // Let the button tell us the number button that was clicked
        connect(button, SIGNAL(clicked()), button_values, SLOT(map()));
        button_values->setMapping(button, text_num);

        // Add a weight button
        QLabel *weight_label = new QLabel(format_object_weight(o_ptr));
        weight_label->setAlignment(Qt::AlignRight);

        // Add a help button to put up a description of the object.
        QPushButton *help_button = new QPushButton(text_num);
        help_button->setIcon(QIcon(":/icons/lib/icons/help.png"));
        help_button->setText(QString(""));
        connect(help_button, SIGNAL(clicked()), help_values, SLOT(map()));
        help_values->setMapping(help_button, text_num);

        // Add both buttons to the appropriate layout.
        object_layout->addWidget(button, (i+1), 0);
        object_layout->addWidget(weight_label, (i+1), 1);
        weight_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        object_layout->addWidget(help_button, (i+1), 2);

        // Keep track of the number of buttons.
        num_buttons++;
    }

    quiver_tab->setLayout(vlay);
}


// determine which tab to start with
byte ObjectSelectDialog::find_starting_tab(int mode)
{
    byte starting_tab = 0;

    // First figure out which starting tab we want.

    /* Hack -- Start on quiver if shooting or throwing */
    if ((mode & (QUIVER_FIRST)) && allow_quiver)
    {
        starting_tab = TAB_QUIVER;
    }
    /* Hack -- Start on equipment if requested */
    else if ((mode == (USE_EQUIP)) && allow_equip)
    {
        starting_tab = TAB_EQUIP;
    }

    /* Use inventory if allowed. */
    else if (allow_inven)
    {
        starting_tab = TAB_INVEN;
    }

    /* Use equipment if allowed */
    else if (allow_equip)
    {
        starting_tab = TAB_EQUIP;
    }

    /* Use floor if allowed */
    else if (allow_floor)
    {
        starting_tab = TAB_FLOOR;
    }
    /* Hack -- Use (empty) inventory if no other choices available. */
    else
    {
        starting_tab = TAB_INVEN;
    }

    // Now find out which tab that is in the dialog box.
    for (int i = 0; i < tab_order.size(); i++)
    {
        // Found it.
        if (tab_order[i] == starting_tab) return (i);
    }

    // Oops.
    return (0);
}

// Return the index off the actual object selected.
int ObjectSelectDialog::get_selected_object(int num_tracker)
{
    object_found = TRUE;

    /*
     * Go through each array and figure out which button was selected.
     * based on the order in which the buttons were created.
     */

    // Floor first
    if (num_tracker < floor_items.size())
    {
        // Floor items are returned as negative items.
        return (0 - floor_items[num_tracker]);
    }
    else num_tracker -= floor_items.size();

    // Inventory next
    if (num_tracker < inven_items.size())
    {
        return (inven_items[num_tracker]);
    }
    else num_tracker -= inven_items.size();

    // Now try equipment
    if (num_tracker < equip_items.size())
    {
        return (equip_items[num_tracker]);
    }
    else num_tracker -= equip_items.size();

    // must be the quiver
    if (num_tracker < quiver_items.size())
    {
        return (quiver_items[num_tracker]);
    }

    object_found = FALSE;
    // This should never happen.  Paranoia
    return (0);
}

void ObjectSelectDialog::on_dialog_buttons_pressed(QAbstractButton *)
{
    this->reject();
}

ObjectSelectDialog::ObjectSelectDialog(int *item, QString prompt, int mode, bool *success, bool *cancelled, int sq_y, int sq_x)
{
    object_tabs = new QTabWidget;
    floor_tab = new QWidget;
    inven_tab = new QWidget;
    equip_tab = new QWidget;
    quiver_tab = new QWidget;

    main_prompt = new QLabel(QString("<b><big>%1</big></b>") .arg(prompt));
    main_prompt->setAlignment(Qt::AlignCenter);

    button_values = new QSignalMapper(this);
    help_values = new QSignalMapper(this);
    connect(button_values, SIGNAL(mapped(QString)), this, SLOT(button_press(QString)));
    connect(help_values, SIGNAL(mapped(QString)), this, SLOT(help_press(QString)));

    // Start with a clean slate
    tab_order.clear();
    num_buttons = 0;
    max_object_desc_length = 0;

    // First, find the eligible objects
    floor_items_count(mode, sq_y, sq_x);
    inven_items_count(mode);
    equip_items_count(mode);
    quiver_items_count(mode);

    // Handle no available objects.
    if (!allow_floor && !allow_inven && !allow_equip && !allow_quiver)
    {
        /* Cancel p_ptr->command_see */
        p_ptr->command_see = FALSE;

        /* Report failure */
        *success = FALSE;

        /* Done here */
        return;
    }

    // formatting detail
    max_object_desc_length += 2;

    // Build, then add the tabs as necessary
    if (allow_floor)
    {
        build_floor_tab();
        object_tabs->addTab(floor_tab, "&Floor Items");
        tab_order.append(TAB_FLOOR);
    }
    if (allow_inven)
    {
        build_inven_tab();
        object_tabs->addTab(inven_tab, "&Inventory");
        tab_order.append(TAB_INVEN);
    }
    if (allow_equip)
    {
        build_equip_tab();
        object_tabs->addTab(equip_tab, "&Equipment");
        tab_order.append(TAB_EQUIP);
    }
    if (allow_quiver)
    {
        build_quiver_tab();
        object_tabs->addTab(quiver_tab, "&Quiver");
        tab_order.append(TAB_QUIVER);
    }

    buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(clicked(QAbstractButton*)), this,
            SLOT(on_dialog_buttons_pressed(QAbstractButton*)));

    // Figure out which tab should appear first.
    object_tabs->setTabEnabled(find_starting_tab(mode), TRUE);

    QVBoxLayout *main_layout = new QVBoxLayout;

    main_layout->addWidget(main_prompt);
    main_layout->addWidget(object_tabs);
    main_layout->addWidget(buttons);
    setLayout(main_layout);
    setWindowTitle(tr("Object Selection Menu"));

    if (!this->exec())
    {
        *cancelled = TRUE;
        *success = FALSE;
    }
    else
    {
        *item = get_selected_object(selected_button);
        *success = object_found;
    }
}


/*
 * Hack -- allow user to "prevent" certain choices.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_allow(int item, bool is_harmless)
{
    object_type *o_ptr;

    /* Inventory or floor */
    if (item >= 0)
        o_ptr = &inventory[item];
    else
        o_ptr = &o_list[0 - item];

    // TODO - work out a system to verify objects

    /* Allow it */
    return (TRUE);
}




/**
 *
 * This code was taken from FAAngband v1.6, Modified for NPP
 *
 * FAAngband notes are below:
 *
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * Any of these are displayed (even if no acceptable items are in that
 * location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * If 'all squelched items' are selected we set cp to ALL_SQUELCHED and return
 * TRUE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven/floor
 * listings.  It is equal to USE_INVEN or USE_EQUIP or USE_FLOOR, except
 * when this function is first called, when it is equal to zero, which will
 * cause it to be set to USE_INVEN.
 *
 * This function has been largely rewritten for FAangband 0.3.2 using
 * Pete Mack's menu code.
 *
 * Assumes the item is on the player square
 */
bool get_item(int *cp, QString pmt, QString str, int mode)
{
    bool success = FALSE;
    bool cancelled = FALSE;

    /* No item selected */
    *cp = 0;

    /* Go to menu */
    ObjectSelectDialog(cp, pmt, mode, &success, &cancelled, p_ptr->py, p_ptr->px);

    /* Check validity */
    if (success)
    {
        if (!get_item_allow(*cp, FALSE))
        {
            success = FALSE;
        }
    }

    /* Hack -- Cancel "display" */
    p_ptr->command_see = FALSE;

    /* Forget the item_tester_tval restriction */
    item_tester_tval = 0;

    /* Forget the item_tester_hook restriction */
    item_tester_hook = NULL;

    /* Forget the item tester_swap restriction */
    item_tester_swap = FALSE;

    /* Make sure the equipment/inventory windows are up to date */
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP);

    /* Warning if needed */
    if (!success && !cancelled && !str.isEmpty()) message(str);

    /* Result */
    return (success);
}

/*
* Same notes as above for get_item, except this is used for squares other than the one the player is on.
* This can be used on any square on the map, but it is intended for
* disarming opening chests on adjacent squares.
*/
bool get_item_beside(int *cp, QString pmt, QString str, int sq_y, int sq_x)
{
    bool success = FALSE;
    bool cancelled = FALSE;

    /* No item selected */
    *cp = 0;

    /* Paranoia */
    if (!in_bounds_fully(sq_y, sq_x)) success = FALSE;

    /* Go to menu */
    ObjectSelectDialog(cp, pmt, (USE_FLOOR), &success, &cancelled, sq_y, sq_x);

    /* Check validity */
    if (success)
    {
        if (!get_item_allow(*cp, TRUE))
        {
            success = FALSE;
        }
    }

    /* Hack -- Cancel "display" */
    p_ptr->command_see = FALSE;

    /* Forget the item_tester_tval restriction */
    item_tester_tval = 0;

    /* Forger the item tester_swap restriction */
    item_tester_swap = FALSE;

    /* Forget the item_tester_hook restriction */
    item_tester_hook = NULL;

    /* Warning if needed */
    if (!success && !cancelled && !str.isEmpty()) message(str);

    /* Result */
    return (success);
}
