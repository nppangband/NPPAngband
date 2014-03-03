/* File: cmd3.cpp */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *                    Jeff Greene, Diego Gonzalez
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

#include "src/npp.h"

static int quiver_wield(int item, object_type *o_ptr)
{
    int slot = 0;
    int num;
    QString o_name;

    object_type object_type_body;
    object_type *i_ptr;

    /* was: num = get_quantity(NULL, o_ptr->number);*/
    num = o_ptr->number;

    /* Cancel */
    if (num <= 0) return 0;

    /* Check free space */
    if (!quiver_carry_okay(o_ptr, num, item))
    {
        message(QString("Your quiver needs more space."));
        return 0;
    }

    /* Get local object */
    i_ptr = &object_type_body;

    /* Copy to the local object */
    i_ptr->object_copy(o_ptr);

    /* Modify quantity */
    i_ptr->number = num;

    /* No longer in use */
    i_ptr->obj_in_use = FALSE;

    /* Describe the result */
    o_name = object_desc(i_ptr, ODESC_PREFIX | ODESC_FULL);

    /*Mark it to go in the quiver */
    o_ptr->ident |= (IDENT_QUIVER);

    slot = wield_slot_ammo(o_ptr);

    /* Handle errors (paranoia) */
    if (slot == QUIVER_END)
    {
        /* No space. */
        message(QString("Your quiver is full."));
        return 0;
    }

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -num);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -num);
        floor_item_optimize(0 - item);
    }

    /* Get the wield slot */
    o_ptr = &inventory[slot];

    /* Describe the result */
    o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Take off existing item */
    if (o_ptr->k_idx)
    {
        /* Take off existing item */
        o_ptr->number += i_ptr->number;
    }

    /* Wear the new stuff */
    else o_ptr->object_copy(i_ptr);

    /* Describe the result */
    o_name = object_desc(i_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Message */
    sound(MSG_WIELD);
    message(QString("You have readied %1 (%2).") .arg(o_name) .arg(index_to_label(slot)));

    /* Cursed! */
    if (o_ptr->is_cursed())
    {
        /* Warn the player */
        sound(MSG_CURSED);
        message(QString("Oops! It feels deathly cold!"));

        /* Remove special inscription, if any */
        if (o_ptr->discount >= INSCRIP_NULL) o_ptr->discount = 0;

        /* Sense the object if allowed */
        if (o_ptr->discount == 0) o_ptr->discount = INSCRIP_CURSED;

        /* The object has been "sensed" */
        o_ptr->ident |= (IDENT_SENSE);
    }

    slot = sort_quiver(slot);

    /* Save quiver size */
    save_quiver_size();

    /* See if we have to overflow the pack */
    pack_overflow();

    /* Recalculate bonuses */
    p_ptr->notice |= (PN_SORT_QUIVER);
    p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA | PU_NATIVE);
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);

    /* Reorder the quiver and return the perhaps modified slot */
    return (slot);
}


/*
 * Wield or wear a single item from the pack or floor.
 * Does not process the player energy.
 */
void wield_item(object_type *o_ptr, int item, int slot)
{
    object_type object_type_body;
    object_type *i_ptr = &object_type_body;

    QString fmt;
    QString o_name;

    /* Describe the result */
    o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Ammo goes in quiver slots, which have special rules. */
    if (IS_QUIVER_SLOT(slot))
    {
        /* Put the ammo in the quiver */
        slot = quiver_wield(item, o_ptr);

        // Succcess
        return;
    }

    /* Hack - Don't wield mimic objects */
    if (o_ptr->mimic_r_idx) return;

    /* Get local object */
    i_ptr = &object_type_body;

    /* Obtain local object */
    i_ptr->object_copy(o_ptr);

    /* Modify quantity */
    i_ptr->number = 1;

    /* No longer in use */
    i_ptr->obj_in_use = FALSE;

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);

    }

    /* Get the wield slot */
    o_ptr = &inventory[slot];

    /* Take off existing item */
    if (o_ptr->k_idx)
    {
        /* Take off existing item */
        (void)inven_takeoff(slot, 255);
    }

    /* Wear the new stuff */
    o_ptr->object_copy(i_ptr);

    /* Increment the equip counter by hand */
    p_ptr->equip_cnt++;

    /* Autoinscribe if the object was on the floor */
    if (item < 0) apply_autoinscription(o_ptr);

    /* Describe the result */
    o_name = object_desc(o_ptr, ODESC_PREFIX | ODESC_FULL);

    /* Where is the item now */
    if ((slot == INVEN_WIELD) || (slot == INVEN_BOW))
    {
        if (o_ptr->is_bow()) fmt = (QString("You are shooting with %1 (%2).") .arg(o_name) .arg(index_to_label(slot)));
        else fmt = (QString("You are wielding %1 (%2).") .arg(o_name) .arg(index_to_label(slot)));

    }
    else if (slot == INVEN_LIGHT) fmt = (QString("Your light source is %1 (%2).") .arg(o_name) .arg(index_to_label(slot)));

    else fmt = (QString("You are wearing %1 (%2).") .arg(o_name) .arg(index_to_label(slot)));

    /* Message */
    sound(MSG_WIELD);
    message(fmt);

    /* Cursed! */
    if (o_ptr->is_cursed())
    {
        /* Warn the player */
        sound(MSG_CURSED);
        message(QString("Oops! It feels deathly cold!"));

        /* Remove special inscription, if any */
        if (o_ptr->discount >= INSCRIP_NULL) o_ptr->discount = 0;

        /* Sense the object if allowed */
        if (o_ptr->discount == 0) o_ptr->discount = INSCRIP_CURSED;

        /* The object has been "sensed" */
        o_ptr->ident |= (IDENT_SENSE);
    }

    /* Save quiver size */
    save_quiver_size();

    /* See if we have to overflow the pack */
    pack_overflow();

    /* Recalculate bonuses, torch, mana */
    p_ptr->notice |= (PN_SORT_QUIVER);
    p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA | PU_NATIVE);
    p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_ITEMLIST);
}



/*
 * create a monster trap, currently used from a scroll
 */
bool make_monster_trap(void)
{
    int y, x, dir;

    /* Get a direction */
    // TODO if (!get_rep_dir(&dir)) return (FALSE);

    /* Get location */
    y = p_ptr->py + ddy[dir];
    x = p_ptr->px + ddx[dir];

    /*check for an empty floor, monsters, and objects*/
    if (!cave_trappable_bold(y, x) || dungeon_info[y][x].monster_idx)
    {
        message(QString("You can only make a trap on an empty floor space."));

        return(FALSE);
    }

    /*set the trap*/
    py_set_trap(y, x);

    return(TRUE);
}



/*
 * Rogues may set traps, or they can be set from a scroll.
 * Only one such trap may exist at any one time,
 * but an old trap can be disarmed to free up equipment for a new trap.
 * -LM-
 */
void py_set_trap(int y, int x)
{

    if (p_ptr->timed[TMD_BLIND] || no_light())
    {
        message(QString("You can not see to set a trap."));
        return;
    }

    if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE])
    {
        message(QString("You are too confused."));
        return;
    }

    /* Set the trap, and draw it. */
    set_effect_trap_player(FEAT_MTRAP_BASE, y, x);

    /* Remember the location */
    dungeon_info[y][x].cave_info |= (CAVE_MARK);

    /* Notify the player. */
    message(QString("You set a monster trap."));

    light_spot(y, x);

    /*make the monsters who saw wary*/
    (void)project_los(y, x, 0, GF_MAKE_WARY);

    /* Increment the number of monster traps. */
    num_trap_on_level++;

}


/*
 * Choose advanced monster trap type
 */
static bool choose_mtrap(int *choice)
{
    int num;

    char c;

    bool done=FALSE;

    /* TODO select the mtrap

    prt("        Choose an advanced monster trap (ESC to cancel):", 1, 8);

    num = 1 + (p_ptr->lev / 6);

                  prt("    a) Sturdy Trap          - (less likely to break)", 2, 8);
    if (num >= 2) prt("    b) Slowing Trap         - (slows monsters)", 3, 8);
    if (num >= 3) prt("    c) Confusion Trap       - (confuses monsters)", 4, 8);
    if (num >= 4) prt("    d) Poison Gas Trap      - (creates a toxic cloud)", 5, 8);
    if (num >= 5) prt("    e) Life Draining Trap   - (Hurts living monsters)", 6, 8);
    if (num >= 6) prt("    f) Lightning Trap       - (shoots a lightning bolt)", 7, 8);
    if (num >= 7) prt("    g) Explosive Trap       - (causes area damage)", 8, 8);
    if (num >= 8) prt("    h) Portal Trap          - (teleports monsters)", 9, 8);
    if (num >= 9) prt("    i) Dispel Monsters Trap - (hurt all monsters in area)", 10, 8);

    */



    /* Return */
    return (TRUE);
}


/*
 * Turn a basic monster trap into an advanced one -BR-
 */
bool py_modify_trap(int y, int x)
{
    int trap_choice = 0;

    if (p_ptr->timed[TMD_BLIND] || no_light())
    {
        message(QString("You can not see to modify your trap."));
        return (FALSE);
    }

    /*no modifying traps on top of monsters*/
    if (dungeon_info[y][x].monster_idx > 0)
    {
        message(QString("There is a creature in the way."));
        return (FALSE);
    }

    if (!(choose_mtrap(&trap_choice))) return (FALSE);

    /* Set the trap, and draw it. */
    x_list[dungeon_info[y][x].effect_idx].x_f_idx = FEAT_MTRAP_BASE + 1 + trap_choice;

    /*check if player did not modify trap*/
    if (x_list[dungeon_info[y][x].effect_idx].x_f_idx == FEAT_MTRAP_BASE) return(FALSE);

    /*Mark it*/
    light_spot(y,x);

    /* Notify the player. */
    message(QString("You modify the monster trap."));

    /*make the monsters who saw wary*/
    (void)project_los(y, x, 0, GF_MAKE_WARY);

    return (TRUE);
}



