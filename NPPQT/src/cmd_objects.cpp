/*
 * File: cmd-obj.c
 * Purpose: Handle objects in various ways
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andrew Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *                       Jeff Greene, Diego Gonzalez
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
#include "src/npp.h"




// Miscellaneous hooks to support the universal object handler below
static bool obj_has_inscrip(object_type *o_ptr) {return (o_ptr->has_inscription());}
static bool obj_can_takeoff(object_type *o_ptr) {return (o_ptr->can_takeoff());}
static bool obj_can_study(object_type *o_ptr)   {return (o_ptr->can_study());}
static bool obj_can_cast(object_type *o_ptr)    {return (o_ptr->can_cast());}
static bool obj_is_scroll(object_type *o_ptr)   {return (o_ptr->is_scroll());}
static bool obj_is_wand(object_type *o_ptr)     {return (o_ptr->is_wand());}
static bool obj_is_rod(object_type *o_ptr)      {return (o_ptr->is_rod());}
static bool obj_is_food(object_type *o_ptr)     {return (o_ptr->is_food());}
static bool obj_is_potion(object_type *o_ptr)   {return (o_ptr->is_potion());}
static bool obj_is_staff(object_type *o_ptr)    {return (o_ptr->is_staff());}
static bool obj_is_ring(object_type *o_ptr)     {return (o_ptr->is_ring());}
static bool obj_is_ammo(object_type *o_ptr)     {return (o_ptr->is_ammo());}

/*** Examination ***/
cmd_arg obj_examine(object_type *o_ptr, cmd_arg args)
{
    (void)o_ptr;
    track_object(args.item);

    object_info_screen(o_ptr);

    args.verify = TRUE;
    return (args);
}

/*
 * Helper function to help spells that target traps (disarming, etc...)
 */
static bool is_trap_spell(byte spell_book, int spell)
{
    if (spell_book == TV_MAGIC_BOOK)
    {
        if (spell == SPELL_TRAP_DOOR_DESTRUCTION) return (TRUE);
    }
    else if (spell_book == TV_PRAYER_BOOK)
    {
        if (spell == PRAYER_UNBARRING_WAYS) return (TRUE);
    }
    else if (spell_book == TV_DRUID_BOOK)
    {
        if (spell == DRUID_TRAP_DOOR_DESTRUCTION) return (TRUE);

    }
    return (FALSE);
}

cmd_arg obj_cast(object_type *o_ptr, cmd_arg args)
{
    QString noun = cast_spell(MODE_SPELL_NOUN, cp_ptr->spell_book, 1, 0);
    bool trap_spell;

    /* Track the object kind */
    track_object(args.item);

    /* Ask for a spell */
    args.choice = get_spell_menu(o_ptr, BOOK_CAST);

    if (args.choice < 0)
    {
        if (args.choice == -2) message(QString("You don't know any %1s in that book.") .arg(noun));
        args.verify = FALSE;
        return (args);
    }

    trap_spell = is_trap_spell(cp_ptr->spell_book, args.choice);

    if (spell_needs_aim(cp_ptr->spell_book, args.choice))
    {
        if (!get_aim_dir(&args.direction, trap_spell))
        {
            // handle failure
            args.verify = FALSE;
            return (args);
        }
    }

    // Success
    args.verify = TRUE;
    return(args);
}

cmd_arg obj_drop(object_type *o_ptr, cmd_arg args)
{
    int amt = get_quantity("Please enter an amount to drop.", o_ptr->number);
    if (amt <= 0)
    {
        args.verify = FALSE;
        return (args);
    }

    args.verify = TRUE;
    args.number = amt;

    return (args);
}

/*
 * Study a book to gain a new spell
 */
static cmd_arg obj_study(object_type *o_ptr, cmd_arg args)
{
    /* Track the object kind */
    track_object(args.item);

    /* Mage -- Choose a spell to study */
    if (cp_ptr->flags & CF_CHOOSE_SPELLS)
    {
        args.choice = get_spell_menu(o_ptr, BOOK_STUDY);

        if (args.choice == -2)
        {
            message(QString("You cannot learn any spells from that book."));
            args.verify = FALSE;
            return(args);
        }
    }

    args.verify = TRUE;
    return(args);
}




cmd_arg obj_wield(object_type *o_ptr, cmd_arg args)
{
    args.slot = wield_slot(o_ptr);

    /* Usually if the slot is taken we'll just replace the item in the slot,
     * but in some cases we need to ask the user which slot they actually
     * want to replace */
    if (inventory[args.slot].k_idx)
    {
        if (o_ptr->is_ring())
        {
            QString q = "Replace which ring? ";
            QString s = "Error in obj_wield, please report";
            item_tester_hook = obj_is_ring;
            if (!get_item(&args.item, q, s, USE_EQUIP))
            {
                args.verify = FALSE;
                return (args);
            }
        }

        if (o_ptr->is_ammo() && !object_similar(&inventory[args.slot], o_ptr))
        {
            QString q = "Replace which ammunition? ";
            QString s = "Error in obj_wield, please report";
            item_tester_hook = obj_is_ammo;
            if (!get_item(&args.item, q, s, USE_QUIVER))
            {
                args.verify = FALSE;
                return (args);
            }
        }
    }

    /* Hack - Throwing weapons can be wielded in the quiver too. */
    /* Analyze object's inscription and verify the presence of "@v" */
    if (is_throwing_weapon(o_ptr) && !IS_QUIVER_SLOT(args.slot) && !o_ptr->inscription.isEmpty())
    {
        /* Get the note */
        QString note = o_ptr->inscription;

        if (note.contains("@v")) args.slot = QUIVER_START;
    }
    args.verify = TRUE;
    return (args);
}

/*** A universal object handler to avoid repetitive code ***/

/* Item "action" type */
typedef struct
{
    cmd_arg (*action)(object_type *, cmd_arg);
    bool needs_aim;
    QString desc;

    QString prompt;
    QString noop;

    bool (*filter)(object_type *o_ptr);
    int mode;
    bool (*prereq)(void);
} item_act_t;


/* All possible item actions */
static item_act_t item_actions[] =
{
    /* Not setting IS_HARMLESS for this one because it could cause a true
     * dangerous command to not be prompted, later.
     */

    /* ACTION_UNINSCRIBE */
    { NULL, FALSE, "uninscribe",
      "Un-inscribe which item? ", "You have nothing to un-inscribe.",
      obj_has_inscrip, (USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER), NULL },

    /* ACTION_INSCRIBE */
    { NULL, FALSE, "inscribe",
      "Inscribe which item? ", "You have nothing to inscribe.",
      NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS | USE_QUIVER), NULL },

    /* ACTION_EXAMINE */
    { obj_examine, FALSE, "examine",
    "Examine which item? ", "You have nothing to examine.",
    NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS | USE_QUIVER), NULL },

    /* ACTION_TAKEOFF */
    { NULL, FALSE, "takeoff",
    "Take off which item? ", "You are not wearing anything you can take off.",
    obj_can_takeoff, (USE_EQUIP | USE_QUIVER), NULL },

    /* ACTION_WIELD */
    { obj_wield, FALSE, "wield",
    "Wear/Wield which item? ", "You have nothing you can wear or wield.",
    obj_can_wear, (USE_INVEN | USE_FLOOR), NULL },

    /* ACTION_DROP */
    { obj_drop, FALSE, "drop",
    "Drop which item? ", "You have nothing to drop.",
    NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER), NULL },
      /* ACTION_BROWSE */
    /*** Spellbooks ***/
    { NULL, FALSE, "browse",
      "Browse which book? ", "You have no books that you can read.",
      obj_can_browse, (USE_INVEN | USE_FLOOR | IS_HARMLESS), NULL },

      /* ACTION_STUDY */
    { obj_study, FALSE, "study",
      "Study which book? ", "You have no books that you can study.",
      obj_can_study, (USE_INVEN | USE_FLOOR), player_can_study },

      /* ACTION_CAST */
    { obj_cast, FALSE, "cast",
      "Use which book? ", "You have no books that you can cast from.",
      obj_can_cast, (USE_INVEN | USE_FLOOR), player_can_cast },

    /* ACTION_USE_STAFF */
    { NULL,  TRUE, "use",
      "Use which staff? ", "You have no staff to use.",
      obj_is_staff, (USE_INVEN | USE_FLOOR | SHOW_FAIL), NULL },

      /* ACTION_AIM_WAND */
    { NULL, TRUE, "aim",
      "Aim which wand? ", "You have no wand to aim.",
      obj_is_wand, (USE_INVEN | USE_FLOOR | SHOW_FAIL), NULL },

      /* ACTION_ZAP_ROD */
    { NULL, TRUE, "zap",
      "Zap which rod? ", "You have no charged rods to zap.",
      obj_is_rod, (USE_INVEN | USE_FLOOR | SHOW_FAIL), NULL },

      /* ACTION_ACTIVATE */
    { NULL, TRUE, "activate",
      "Activate which item? ", "You have nothing to activate.",
      obj_is_activatable, (USE_EQUIP | SHOW_FAIL), NULL },

      /* ACTION_EAT_FOOD */
    { NULL, FALSE, "eat",
      "Eat which item? ", "You have nothing to eat.",
      obj_is_food, (USE_INVEN | USE_FLOOR), NULL },

      /* ACTION_QUAFF_POTION */
    { NULL, FALSE, "quaff",
      "Quaff which potion? ", "You have no potions to quaff.",
      obj_is_potion, (USE_INVEN | USE_FLOOR), NULL },

      /* ACTION_READ_SCROLL */
    { NULL, TRUE, "read",
      "Read which scroll? ", "You have no scrolls to read.",
      obj_is_scroll, (USE_INVEN | USE_FLOOR), player_can_read },

    /* ACTION_REFILL */
   { NULL, FALSE, "refill",
    "Refuel with what fuel source? ", "You have nothing to refuel with.",
    obj_can_refill, (USE_INVEN | USE_FLOOR), NULL },

};


/* List matching up to item_actions[] */
typedef enum
{
    ACTION_UNINSCRIBE = 0,
    ACTION_INSCRIBE,
    ACTION_EXAMINE,
    ACTION_TAKEOFF,
    ACTION_WIELD,
    ACTION_DROP,

    ACTION_BROWSE,
    ACTION_STUDY,
    ACTION_CAST,

    ACTION_USE_STAFF,
    ACTION_AIM_WAND,
    ACTION_ZAP_ROD,
    ACTION_ACTIVATE,
    ACTION_EAT_FOOD,
    ACTION_QUAFF_POTION,
    ACTION_READ_SCROLL,
    ACTION_REFILL
} item_act;

static bool trap_related_object(object_type *o_ptr)
{
    if (o_ptr->is_wand() && o_ptr->is_aware())
    {
        if ((o_ptr->sval == SV_WAND_DISARMING) ||
            (o_ptr->sval == SV_WAND_TRAP_DOOR_DEST)) return (TRUE);
    }
    else if (o_ptr->is_rod() && o_ptr->is_aware())
    {
        if (o_ptr->sval == SV_ROD_DISARMING) return (TRUE);
    }

    return (FALSE);
}

/*** Old-style noun-verb functions ***/


/*
 * Generic "select item action" function
 */
static cmd_arg select_item(item_act act)
{
    object_type *o_ptr;

    QString q, s;

    cmd_arg args;
    args.wipe();

    if (item_actions[act].prereq)
    {
        if (!item_actions[act].prereq())
        {
            args.verify = FALSE;
            return (args);
        }
    }

    /* Don't allow activation of swap weapons */
    if (adult_swap_weapons)
    {
        if (act == ACTION_ACTIVATE)  item_tester_swap = TRUE;
    }

    /* Get item */
    q = item_actions[act].prompt;
    s = item_actions[act].noop;
    item_tester_hook = item_actions[act].filter;

    if (!get_item(&args.item, q, s, item_actions[act].mode))
    {
        args.verify = FALSE;
        return (args);
    }

    /* Get the item */
    o_ptr = object_from_item_idx(args.item);

    /* Execute the item command */
    if (item_actions[act].action != NULL)
        args = item_actions[act].action(o_ptr, args);
    else if (item_actions[act].needs_aim && obj_needs_aim(o_ptr))
    {
        bool trap_related = trap_related_object(o_ptr);
        if (!get_aim_dir(&args.direction, trap_related))
        {
            args.verify = FALSE;
            return (args);
        }
    }

    // Success
    args.verify = TRUE;
    return(args);
}




/*** Utility bits and bobs ***/

/*
 * Check to see if the player can use a rod/wand/staff/activatable object.
 */
static int check_devices(object_type *o_ptr)
{
    int fail;
    QString msg;
    QString what = NULL;

    /* Get the right string */
    switch (o_ptr->tval)
    {
        case TV_ROD:   {msg = "zap the rod";   break;}
        case TV_WAND:  {msg = "use the wand";  what = "wand";  break;}
        case TV_STAFF: {msg = "use the staff"; what = "staff"; break;}
        default:       {msg = "activate it";  break;}
    }

    /* Figure out how hard the item is to use */
    fail = get_use_device_chance(o_ptr);

    /* Roll for usage */
    if (randint1(1000) < fail)
    {
        message(QString("You failed to %1 properly.") .arg(msg));
        return FALSE;
    }

    /* Notice empty staffs */
    if (!what.isEmpty() && o_ptr->pval <= 0)
    {
        message(QString("The %1 has no charges left.") .arg(msg));
        o_ptr->ident |= (IDENT_EMPTY);
        p_ptr->notice |= (PN_COMBINE | PN_REORDER);
        p_ptr->window |= (PW_INVEN);

        return FALSE;
    }

    return TRUE;
}


/*** Inscriptions ***/
void command_uninscribe(cmd_arg args)
{
    bool check_autoinscribe = FALSE;
    object_type *o_ptr = object_from_item_idx(args.item);
    object_kind *k_ptr = &k_info[o_ptr->k_idx];

    // Command cancelled
    if(!args.verify) return;

    if (!item_is_available(args.item, NULL, USE_FLOOR | USE_INVEN | USE_EQUIP | USE_QUIVER))
    {
        pop_up_message_box("You do not have that item to uninscribe.");
        return;
    }

    if (!o_ptr->has_inscription())
    {
        pop_up_message_box("That item had no inscription to remove.");
        return;
    }

    // See if the inscriptions match
    if (!k_ptr->autoinscribe.isEmpty())
    {
        if (operator==(k_ptr->autoinscribe, o_ptr->inscription)) check_autoinscribe = TRUE;
    }

    /* Remove the inscription */
    o_ptr->inscription.clear();

    message(QString("Inscription removed."));

    /*The object kind has an autoinscription*/
    if (check_autoinscribe)
    {
        QString tmp_val;
        QString o_name2;

        /*make a fake object so we can give a proper message*/
        object_type *i_ptr;
        object_type object_type_body;

        /* Get local object */
        i_ptr = &object_type_body;

        /* Wipe the object */
        i_ptr->object_wipe();

        /* Create the object */
        object_prep(i_ptr, o_ptr->k_idx);

        /*now describe with correct amount*/
        o_name2 = object_desc(i_ptr, ODESC_PLURAL | ODESC_FULL);

        /* Prompt */
        tmp_val = (QString("Remove automatic inscription for %1?") .arg(o_name2));

        /* Clear Auto-Inscribe if they want that */
        if (get_check(tmp_val)) k_ptr->autoinscribe.clear();
    }

    p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);
}

/*
 * Remove inscription
 */
void do_cmd_uninscribe(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_UNINSCRIBE);

    command_uninscribe(args);
}

void command_inscribe(cmd_arg args)
{
    object_type *o_ptr = object_from_item_idx(args.item);
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    bool check_autoinscribe = FALSE;

    QString o_name;
    QString new_inscription;

    // Command cancelled
    if(!args.verify) return;

    if (!item_is_available(args.item, NULL, USE_FLOOR | USE_INVEN | USE_EQUIP | USE_QUIVER))
    {
        pop_up_message_box("You do not have that item to inscribe.");
        return;
    }

    /* Describe the activity */
    o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Message */
    QString tmp2 = (QString("Inscribing %1.") .arg(o_name));

    new_inscription = get_string("Please enter an inscription", QString(tmp2), o_ptr->inscription);

    // Nothing inscribed
    if (new_inscription.isEmpty()) return;

    if (!o_ptr->is_artifact() && operator!=(k_ptr->autoinscribe, new_inscription)) check_autoinscribe = TRUE;

    if (check_autoinscribe)
    {
        /* Get a new inscription */
        QString tmp_val;
        QString o_name2;

        /*make a fake object so we can give a proper message*/
        object_type *i_ptr;
        object_type object_type_body;

        /* Get local object */
        i_ptr = &object_type_body;

        /* Wipe the object */
        i_ptr->object_wipe();

        /* Create the object */
        object_prep(i_ptr, o_ptr->k_idx);

        /*now describe with correct amount*/
        o_name2 = object_desc(i_ptr, ODESC_FULL | ODESC_PLURAL);

        /* Prompt */
        tmp_val = (QString("Automatically inscribe all %1 with %2?") .arg(o_name2) .arg(new_inscription));

        /* Auto-Inscribe if they want that */
        if (get_check(tmp_val)) k_ptr->autoinscribe = new_inscription;
    }

    /* Save the inscription */
    o_ptr->inscription = new_inscription;

    /* Combine / Reorder the pack */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);

}

/*
 * Add inscription
 */
void do_cmd_inscribe(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_INSCRIBE);

    command_inscribe(args);
}


//Inspect an item
void do_cmd_observe(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    select_item(ACTION_EXAMINE);
}

/*** Taking off/putting on ***/

//Actually take off the item.

bool command_takeoff(cmd_arg args)
{
    // Command cancelled
    if(!args.verify) return (FALSE);

    if (!item_is_available(args.item, NULL, USE_EQUIP | USE_QUIVER))
    {
        pop_up_message_box("You are not wielding that item.");
        return (FALSE);
    }

    if (!obj_can_takeoff(object_from_item_idx(args.item)))
    {
        message(QString("You cannot take off that item."));
        return (FALSE);
    }

    (void)inven_takeoff(args.item, 255);
    pack_overflow();
    process_player_energy(BASE_ENERGY_MOVE / 2);
    return (TRUE);
}

/*
 * Take off an item (keyboard command)
 */
void do_cmd_takeoff(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_TAKEOFF);

    (void)command_takeoff(args);
}

// Handle wielding the item.
bool command_wield(cmd_arg args)
{
    object_type *equip_o_ptr;
    QString o_name;

    // Command cancelled
    if(!args.verify) return (FALSE);

    object_type *o_ptr = object_from_item_idx(args.item);

    if (!item_is_available(args.item, NULL, USE_INVEN | USE_FLOOR))
    {
        pop_up_message_box("You do not have that item to wield.");
        return (FALSE);
    }

    /* Check the slot */
    if (!slot_can_wield_item(args.slot, o_ptr))
    {
        o_name = object_desc(o_ptr,  ODESC_PREFIX | ODESC_FULL);

        pop_up_message_box("You cannot wield that item there.");
        return (FALSE);
    }

    /*Hack - don't allow quest items to be worn*/
    if(o_ptr->ident & (IDENT_QUEST))
    {
        pop_up_message_box("You cannot wield quest items.");
        return (FALSE);
    }

    /* Hack - Throwing weapons can be wielded in the quiver too. */
    /* Analyze object's inscription and verify the presence of "@v" */
    if (is_throwing_weapon(o_ptr) && !IS_QUIVER_SLOT(args.slot) && !o_ptr->inscription.isEmpty())
    {
        if (o_ptr->inscription.contains("@v")) args.slot = QUIVER_START;
    }

    equip_o_ptr = &inventory[args.slot];

    /* If the slot is open, wield and be done */
    if (!equip_o_ptr->k_idx)
    {
        wield_item(o_ptr, args.item, args.slot);
        process_player_energy(BASE_ENERGY_MOVE);
        return (TRUE);
    }

    /* If the slot is in the quiver and objects can be combined */
    if (((obj_is_ammo(equip_o_ptr)) || (is_throwing_weapon(o_ptr)))
                          && object_similar(equip_o_ptr, o_ptr))
    {
        wield_item(o_ptr, args.item, args.slot);
        process_player_energy(BASE_ENERGY_MOVE);
        return (TRUE);
    }

    /* Prevent wielding into a cursed slot */
    if (equip_o_ptr->is_cursed())
    {
        o_name = object_desc(equip_o_ptr,  ODESC_BASE);
        pop_up_message_box(QString("The %1 you are %2 appears to be cursed.") .arg(o_name) .arg(describe_use(args.slot)));
        return(FALSE);
    }

    /* "!t" checks for taking off */
    if(equip_o_ptr->inscription.contains("!t"))
    {
        /* Prompt */
        o_name = object_desc(equip_o_ptr,  ODESC_PREFIX | ODESC_FULL);

        /* Forget it */
        if (!get_check(QString("Really take off %1? ") .arg(o_name))) return(FALSE);
    }

    wield_item(o_ptr, args.item, args.slot);
    process_player_energy(BASE_ENERGY_MOVE);
    return (TRUE);
}


/*
 * Wield or wear an item - (keyboard command)
 */
void do_cmd_wield()
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_WIELD);

    (void)command_wield(args);
}

bool command_drop(cmd_arg args)
{
    int item = args.item;
    object_type *o_ptr = object_from_item_idx(item);
    int amt = args.number;

    // Command cancelled
    if(!args.verify) return (FALSE);

    if (!item_is_available(item, NULL, USE_INVEN | USE_EQUIP | USE_QUIVER))
    {
        message(QString("You do not have that item to drop."));
        return (FALSE);
    }

    /* Hack -- Cannot remove cursed items */
    if ((item >= INVEN_WIELD) && o_ptr->is_cursed())
    {
        pop_up_message_box("Hmmm, it seems to be cursed.");
        return (FALSE);
    }

    /* Cursed quiver */
    else if (IS_QUIVER_SLOT(item) && p_ptr->state.cursed_quiver)
    {
        /* Oops */
        pop_up_message_box("Your quiver is cursed!");

        /* Nope */
        return(FALSE);
    }

    inven_drop(item, amt);
    process_player_energy(BASE_ENERGY_MOVE / 2);

    return (TRUE);
}

/*
 * Drop an item
 */
void do_cmd_drop(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_DROP);

    (void) command_drop(args);
}

static void refill_lamp(object_type *j_ptr, object_type *o_ptr, int item)
{

    /* Refuel from a lantern */
    if (o_ptr->sval == SV_LIGHT_LANTERN)
    {
        j_ptr->timeout += o_ptr->timeout;
    }
    /* Refuel from a flask */
    else
    {
        j_ptr->timeout += o_ptr->pval;
    }

    /* Message */
    message(QString("You fuel your lamp."));

    /* Comment */
    if (j_ptr->timeout >= FUEL_LAMP)
    {
        j_ptr->timeout = FUEL_LAMP;
        message(QString("Your lamp is full."));
    }

    /* Refilled from a lantern */
    if (o_ptr->sval == SV_LIGHT_LANTERN)
    {

        /* Unstack if necessary */
        if (o_ptr->number > 1)
        {
            object_type *i_ptr;
            object_type object_type_body;

            /* Get local object */
            i_ptr = &object_type_body;

            /* Obtain a local object */
            i_ptr->object_copy(o_ptr);

            /* Modify quantity */
            i_ptr->number = 1;

            /* Remove fuel */
            i_ptr->timeout = 0;

            /* Unstack the used item */
            o_ptr->number--;

            /* Carry or drop */
            if (item >= 0)
                item = inven_carry(i_ptr);
            else
                drop_near(i_ptr, 0, p_ptr->py, p_ptr->px);
        }

        /* Empty a single lantern */
        else
        {
            /* No more fuel */
            o_ptr->timeout = 0;
        }

        /* Combine / Reorder the pack (later) */
        p_ptr->update |= (PU_BONUS);
        p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

        /* Redraw stuff */
        p_ptr->redraw |= (PR_INVEN | PR_ITEMLIST);
    }

    /* Refilled from a flask */
    else
    {
        /* Decrease the item (from the pack) */
        if (item >= 0)
        {
            inven_item_increase(item, -1);
            inven_item_describe(item);
            inven_item_optimize(item);
        }

        /* Decrease the item (from the floor) */
        else
        {
            floor_item_increase(0 - item, -1);
            floor_item_describe(0 - item);
            floor_item_optimize(0 - item);
        }
    }

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_EQUIP);
}


static void refuel_torch(object_type *j_ptr, object_type *o_ptr, int item)
{
    /* Refuel */
    j_ptr->timeout += o_ptr->timeout + 5;

    /* Message */
    message(QString("You combine the torches."));

    /* Over-fuel message */
    if (j_ptr->timeout >= FUEL_TORCH)
    {
        j_ptr->timeout = FUEL_TORCH;
        message(QString("Your torch is fully fueled."));
    }

    /* Refuel message */
    else
    {
        message(QString("Your torch glows more brightly."));
    }

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_EQUIP);
}


/*
 * Lanterns can be filled with oil on the dungeon's floor.
 * Returns TRUE if the amount of fuel could be filled in this way.
 */
static bool do_cmd_refill_lamp_from_terrain(void)
{
    object_type *j_ptr;

    /* Get player coordinates */
    int y = p_ptr->py;
    int x = p_ptr->px;

    /* Assume that the oil can be dry */
    bool can_dry = TRUE;

    /* Check presence of oil */
    if (!(cave_ff3_match(y, x, FF3_OIL) && cave_passable_bold(y, x))) return (FALSE);

    /* Ask the user if he/she wants to use the oil on the grid */
    if (!get_check("Do you want to refill your lamp from the oil on the floor? ")) return (FALSE);

    /* Get the lantern */
    j_ptr = &inventory[INVEN_LIGHT];

    /* Increase fuel amount (not too much) */
    j_ptr->timeout += (200 + rand_int(3) * 50);

    /* Increase fuel amount even more if there is more oil on that grid */
    if (cave_ff2_match(y, x, FF2_DEEP)) j_ptr->timeout += (100 + rand_int(2) * 50);

    /* Message */
    message(QString("You fuel your lamp."));

    /* Comment */
    if (j_ptr->timeout >= FUEL_LAMP)
    {
        /* Some oil was unused */
        if (j_ptr->timeout > FUEL_LAMP)
        {
            /* Set oil to the max. value */
            j_ptr->timeout = FUEL_LAMP;

            /* Give the remaining oil to the grid */
            can_dry = FALSE;
        }

        /* Message */
        message(QString("Your lamp is full."));
    }

    /* Sometimes we remove the oil grid to stop oil-abuse */
    if (can_dry && !f_info[FEAT_EARTH].f_name.length() && one_in_(7))
    {
        /* Transform to earth */
        cave_set_feat(y, x, FEAT_EARTH);

        /* Message */
        message(QString("The oil patch dries."));
    }

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);

    /* Window stuff */
    p_ptr->redraw |= (PR_EQUIP);

    /* Success */
    return (TRUE);
}

/*
 * Refill the players lamp, or restock his torches
 */
void command_refuel(cmd_arg args)
{
    object_type *j_ptr = &inventory[INVEN_LIGHT];

    int item = args.item;
    object_type *o_ptr = object_from_item_idx(item);

    if (!item_is_available(item, NULL, USE_INVEN | USE_FLOOR))
    {
        pop_up_message_box("You do not have that item to refill with it.");
        return;
    }

    /* It is nothing */
    if (j_ptr->tval != TV_LIGHT)
    {
        pop_up_message_box("You are not wielding a light.");
        return;
    }

    /* It's a lamp */
    else if (j_ptr->sval == SV_LIGHT_LANTERN)
    {
        /* First we check if the lamp can be filled from the oil on the floor */
        if (!do_cmd_refill_lamp_from_terrain())
        {
            refill_lamp(j_ptr, o_ptr, item);
        }
    }

    /* It's a torch */
    else if (j_ptr->sval == SV_LIGHT_TORCH)
        refuel_torch(j_ptr, o_ptr, item);


    process_player_energy(BASE_ENERGY_MOVE / 2);
}


/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refuel(void)
{
    // Paranoia
    if (!p_ptr->playing) return;

    cmd_arg args = select_item(ACTION_REFILL);

    (void) command_refuel(args);

}

static void swap_weapons(void)
{
    object_type *o_ptr = &inventory[INVEN_MAIN_WEAPON];
    object_type *j_ptr = &inventory[INVEN_SWAP_WEAPON];
    object_type object_type_body;
    object_type *i_ptr = & object_type_body;
    QString o_name;
    QString act;

    /* Not holding anything */
    if ((!o_ptr->k_idx) && (!j_ptr->k_idx))
    {
        message(QString("But you are wielding no weapons."));
        return;
    }

    /* Can't swap because of a cursed weapon */
    if (o_ptr->is_cursed())
    {
        o_name = object_desc(o_ptr,  ODESC_BASE);
        message(QString("The %1 you are %2 appears to be cursed.") .arg(o_name) .arg(describe_use(INVEN_MAIN_WEAPON)));
        return;
    }

    /* Give the player a message for teh item they are taking off */
    if (o_ptr->k_idx)
    {
        /* The player took off a bow */
        if (o_ptr->is_bow())
        {
            act = "You were shooting";
        }

        /* Took off weapon */
        else act = "You were wielding";

        /* Describe the object */
        o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);
        message(QString("%1 %2 (%3).") .arg(act) .arg(o_name) .arg(index_to_label(INVEN_SWAP_WEAPON)));
    }

    /* Make a record of the primary weapon, and wipe it */
    i_ptr->object_copy(o_ptr);
    o_ptr->object_wipe();

    /* Insert the swap weapon if there is one */
    if (j_ptr->k_idx)
    {
        wield_item(j_ptr, INVEN_SWAP_WEAPON, INVEN_MAIN_WEAPON);
    }

    /* if a previous weapon, place it in the secondary weapon slot */
    if (i_ptr->k_idx)
    {
        j_ptr->object_copy(i_ptr);
    }

    /* Recalculate bonuses, torch, mana */
    p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA | PU_NATIVE);
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);

    process_player_energy(BASE_ENERGY_MOVE);
}

/* Search the backpack for a weapon with @x and wield it*/
static void wield_swap_weapon(void)
{
    int i;

    object_type *o_ptr = &inventory[INVEN_MAIN_WEAPON];

    /* Can't swap because of a cursed weapon */
    if (o_ptr->is_cursed())
    {
        QString o_name;

        o_name = object_desc(o_ptr,  ODESC_BASE);
        message(QString("The %1 you are %2 appears to be cursed.") .arg(o_name) .arg(describe_use(INVEN_MAIN_WEAPON)));
        return;
    }

    /* Check every object */
    for (i = 0; i < INVEN_MAX_PACK; ++i)
    {
        o_ptr = &inventory[i];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Skip empty inscriptions */
        if (o_ptr->inscription.isEmpty()) continue;

        if(!o_ptr->is_weapon()) continue;

        /* Look for '@x' */
        if(!o_ptr->inscription.contains("@x")) continue;

        /* Wield it */
        wield_item(o_ptr, i, INVEN_MAIN_WEAPON);

        /* Recalculate bonuses, torch, mana */
        p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA | PU_NATIVE);
        p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);

        /* We are done */
        process_player_energy(BASE_ENERGY_MOVE);
        return;
    }

    /* Didn't find anything */
    message(QString("Please inscribe a weapon with '@x' in order to swap it."));
}

/*
 * Depending on game options, either swap weapons between the main weapon
 * slot and the swap weapon slot, or search for weapon with the @x inscription and wield it
 */
void do_cmd_swap_weapon()
{
    if (adult_swap_weapons) swap_weapons();
    else wield_swap_weapon();
}

