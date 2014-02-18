/* File: was dungeon.c */

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
#include "src/store.h"


/*
 * Sense the inventory
 */
static void sense_inventory(void)
{
    int i;

    int plev = p_ptr->lev;

    bool heavy = ((cp_ptr->flags & CF_PSEUDO_ID_HEAVY) ? TRUE : FALSE);

    int feel;

    object_type *o_ptr;

    QString o_name;


    /*** Check for "sensing" ***/

    /* No sensing when confused */
    if (p_ptr->timed[TMD_CONFUSED]) return;

    if (cp_ptr->flags & CF_PSEUDO_ID_IMPROV)
    {
        if (0 != rand_int(cp_ptr->sense_base / (plev * plev + cp_ptr->sense_div)))
            return;
    }
    else
    {
        if (0 != rand_int(cp_ptr->sense_base / (plev + cp_ptr->sense_div)))
            return;
    }


    /*** Sense everything ***/

    /* Check everything */
    for (i = 0; i < ALL_INVEN_TOTAL; i++)
    {

        int squelch = SQUELCH_NO;

        o_ptr = &inventory[i];

        /* Skip empty slots */
        if (!o_ptr->k_idx) continue;

        /* Sensing the swap weapon is kind of cheating */
        if (adult_swap_weapons && (i == INVEN_SWAP_WEAPON))	continue;

        /* Skip non-sense machines */
        if (!o_ptr->can_be_pseudo_ided()) continue;

        /* It already has a discount or special inscription */
        if ((o_ptr->discount > 0) &&
            (o_ptr->discount != INSCRIP_INDESTRUCTIBLE)) continue;

        /* It has already been sensed, do not sense it again */
        if (o_ptr->ident & (IDENT_SENSE)) continue;

        /* It is known, no information needed */
        if (object_known_p(o_ptr)) continue;

        /* 80% failure on inventory items */
        if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

        /* Indestructible objects are either excellent or terrible */
        if (o_ptr->discount == INSCRIP_INDESTRUCTIBLE)
            heavy = TRUE;

        /* Check for a feeling */
        feel = (heavy ? o_ptr->pseudo_heavy() : o_ptr->pseudo_light());

        /* Skip non-feelings */
        if (!feel) continue;

        /* Squelch it? */
        if (i < INVEN_WIELD)
        {
            squelch = squelch_itemp(o_ptr, feel, FALSE);
        }

        /* Stop everything */
        disturb(0, 0);

        /* Get an object description */
        o_name = object_desc(o_ptr, ODESC_FULL);

        /* Message (equipment) */

        sound(MSG_PSEUDOID);
        if (i >= INVEN_WIELD)
        {
            message(QString("You feel the %1 (%2) you are %3 %4 %5...") .arg(o_name)
                       .arg(index_to_label(i)) .arg(describe_use(i))
                       .arg((o_ptr->number == 1) ? "is" : "are") .arg(inscrip_text[feel - INSCRIP_NULL]));
        }

        /* Message (inventory) */
        else
        {
            message(QString("You feel the %1 (%2) in your pack %3 %4...  %5") .arg(o_name)
                    .arg(index_to_label(i))
                    .arg((o_ptr->number == 1) ? "is" : "are") .arg(inscrip_text[feel - INSCRIP_NULL])
                    .arg(squelch_to_label(squelch)));
        }

        /* Sense the object */
        o_ptr->discount = feel;

        if (feel == INSCRIP_AVERAGE)
        {
            /* Identify it fully */
            object_aware(o_ptr);
            object_known(o_ptr);
        }

        else
        {
            /* The object has been "sensed" */
            o_ptr->ident |= (IDENT_SENSE);
        }

        /* Squelch it if necessary */
        do_squelch_item(squelch, i, o_ptr);

        /* Combine / Reorder the pack (later) */
        p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

        /* Redraw stuff */
        p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
    }
}



/*
 * Regenerate hit points
 */
static void regenhp(int percent)
{
    s32b new_chp, new_chp_frac;
    int old_chp;

    /* Save the old hitpoints */
    old_chp = p_ptr->chp;

    /* Extract the new hitpoints */
    new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
    p_ptr->chp += (s16b)(new_chp >> 16);	/* div 65536 */

    /* check for overflow */
    if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
    new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
    if (new_chp_frac >= 0x10000L)
    {
        p_ptr->chp_frac = (u16b)(new_chp_frac - 0x10000L);
        p_ptr->chp++;
    }
    else
    {
        p_ptr->chp_frac = (u16b)new_chp_frac;
    }

    /* Fully healed */
    if (p_ptr->chp >= p_ptr->mhp)
    {
        p_ptr->chp = p_ptr->mhp;
        p_ptr->chp_frac = 0;
    }

    /* Notice changes */
    if (old_chp != p_ptr->chp)
    {
        /* Redraw */
        p_ptr->redraw |= (PR_HP);
    }
}


/*
 * Regenerate mana points
 */
static void regenmana(int percent)
{
    s32b new_mana, new_mana_frac;
    int old_csp;

    old_csp = p_ptr->csp;
    new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
    p_ptr->csp += (s16b)(new_mana >> 16);	/* div 65536 */
    /* check for overflow */
    if ((p_ptr->csp < 0) && (old_csp > 0))
    {
        p_ptr->csp = MAX_SHORT;
    }
    new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
    if (new_mana_frac >= 0x10000L)
    {
        p_ptr->csp_frac = (u16b)(new_mana_frac - 0x10000L);
        p_ptr->csp++;
    }
    else
    {
        p_ptr->csp_frac = (u16b)new_mana_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->csp >= p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }

    /* Redraw mana */
    if (old_csp != p_ptr->csp)
    {
        /* Redraw */
        p_ptr->redraw |= (PR_MANA);
    }
}

/*
 * Give the monsters terrain damage (once per 10 game turns)
 */
static void monster_terrain_damage(void)
{
    int i;

    /* Regenerate everyone */
    for (i = 1; i < mon_max; i++)
    {
        /* Check the i'th monster */
        monster_type *m_ptr = &mon_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];
        /* Get the feature */
        u16b feat = dungeon_info[m_ptr->fy][m_ptr->fx].monster_idx;
        feature_type *f_ptr = &f_info[feat];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /* Monsters in non-native terrain take damage, and isn't flying */
        if ((f_ptr->dam_non_native > 0) &&
            !is_monster_native(m_ptr->fy, m_ptr->fx, r_ptr) &&
            !MONSTER_CAN_FLY(m_ptr, feat))
        {
            int gf_type = 0;

            u16b tmd_flag = (MON_TMD_FLG_NOTIFY);

            if (m_ptr->ml) tmd_flag |= MON_TMD_FLG_SEEN;

            /* Wake it up */
            mon_clear_timed(i, MON_TMD_SLEEP, tmd_flag);

            /*If we saw this, count this in the lore*/
            if (m_ptr->ml)
            {
                feature_lore *f_l_ptr = &f_l_list[feat];

                /*Count the number of times this damage has been felt*/
                if (f_l_ptr->f_l_dam_non_native < MAX_UCHAR) f_l_ptr->f_l_dam_non_native++;
            }

            get_spell_type_from_feature(feat, &gf_type);

            /* Hack - quest monsters shouldn't take damage from terrain */
            if (m_ptr->mflag & (MFLAG_QUEST))
            {
                teleport_away(i, 2);
                continue;
            }

            /*Take damage*/
            (void)project_m(SOURCE_OTHER, m_ptr->fy, m_ptr->fx, f_ptr->dam_non_native, gf_type, 0L);

            /* Hack - if the monster isn't visible or in line-of-sight, move it to safety */
            if ((!m_ptr->ml) && (!m_ptr->project)) teleport_away(i, 2);
        }
    }
}


/*
 * Regenerate the monsters (once per 100 game turns)
 */
static void regen_monsters(void)
{
    int i, frac;

    int smooth = (turn / 100) % 100;

    /* Regenerate everyone */
    for (i = 1; i < mon_max; i++)
    {
        /* Check the i'th monster */
        monster_type *m_ptr = &mon_list[i];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Skip dead monsters */
        if (!m_ptr->r_idx) continue;

        /*
         * Hack -- in order to avoid a monster of 200 hitpoints having twice
         * the regeneration of one with 199, and because we shouldn't randomize
         * things (since we don't randomize character regeneration), we use
         * current turn to smooth things out.
         */

        /* Regenerate mana, if needed */
        if (m_ptr->mana != r_ptr->mana)
        {
            frac = (r_ptr->mana + smooth) / 100;

            /* Minimal regeneration rate */
            if (!frac) frac = 1;

            /* Regenerate */
            m_ptr->mana += frac;

            /* Do not over-regenerate */
            if (m_ptr->mana > r_ptr->mana) m_ptr->mana = r_ptr->mana;

            /* Fully healed -> flag minimum range for recalculation */
            if (m_ptr->mana == r_ptr->mana) m_ptr->min_range = 0;
        }

        /* Allow hp regeneration, if needed. */
        if (m_ptr->hp != m_ptr->maxhp)
        {
            frac = (m_ptr->maxhp + smooth) / 100;

            /* Some monsters regenerate quickly */
            if (r_ptr->flags2 & (RF2_REGENERATE)) frac *= 2;

            /* Minimal regeneration rate */
            if (!frac) frac = 1;

            /* Regenerate */
            m_ptr->hp += frac;

            /* Do not over-regenerate */
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

            /* Fully healed -> flag minimum range for recalculation */
            if (m_ptr->hp == m_ptr->maxhp) m_ptr->min_range = 0;
        }

    }
}

/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 * "all" must be FALSE when only one object of a stack is recharged.
 */
static void recharged_notice(object_type *o_ptr, bool all)
{
    QString o_name;

    QString s;

    if (!notify_recharge) return;

    /* Not the right  inscription */
    if (!o_ptr->inscription.contains("!!")) return;

    /* Describe (briefly) */
    o_name = object_desc(o_ptr, ODESC_BASE);

    /*Disturb the player*/
    disturb(0, 0);

    /* Notify the player */
    if (o_ptr->number > 1)
    {
        if (all)
        {
           message(QString("Your %1 are recharged.") .arg(o_name));
        }
        else
        {
           message(QString("One of your %1 has recharged.") .arg(o_name));
        }
    }

    /*artifacts*/
    else if (o_ptr->art_num)
    {
       message(QString("The %1 has recharged.") .arg(o_name));
    }

    /*single, non-artifact items*/
    else message(QString("Your %1 has recharged.") .arg(o_name));
}

static void process_mimics(void)
{
    s16b i;
    int dist;
    int obj_y, obj_x;
    int chance;

    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];
        monster_race *r_ptr;

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Only work with the mimic objects */
        if (!o_ptr->mimic_r_idx) continue;

        r_ptr = &r_info[o_ptr->mimic_r_idx];

        /* Determine object location */
        /* Held by a monster */
        if (o_ptr->held_m_idx)
        {
            monster_type *m_ptr;

            /* Get the monster */
            m_ptr = &mon_list[o_ptr->held_m_idx];

            /* Get the location */
            obj_y = m_ptr->fy;
            obj_x = m_ptr->fx;
        }
        /* On the ground */
        else
        {
            obj_y = o_ptr->iy;
            obj_x = o_ptr->ix;
        }

        /*
         * If the mimic can't cast, wait until the player is right next to it to come out of hiding
         * Hack - make an exception for creeping coins (so pits/nests are still dangerous)
         *
         */
        if ((!r_ptr->freq_ranged) && (o_ptr->tval != TV_GOLD))
        {
            if ((ABS(obj_y - p_ptr->py) <= 1)  && (ABS(obj_x - p_ptr->px) <= 1))
            {
                reveal_mimic(i, o_ptr->marked);
            }
            continue;
        }

        /* get the distance to player */
        dist = distance(obj_y, obj_x, p_ptr->py, p_ptr->px);

        /* Must be in line of fire from the player */
        if (!player_can_fire_bold(obj_y, obj_x)) continue;

        /* paranoia */
        if (dist > MAX_SIGHT) continue;

        /* Chance to be revealed gets bigger as the player gets closer */
        chance = (MAX_SIGHT - dist) * (MAX_SIGHT/4)  + 10;

        /* Reveal the mimic if test is passed*/
        if (randint0(MAX_SIGHT * 5) < chance)
        {
            reveal_mimic(i, o_ptr->marked);
        }
    }
}


/*
 * Recharge activatable objects in the player's equipment
 * and rods in the inventory and on the ground.
 */
static void recharge_objects(void)
{
    int i;
    int j = 0;

    object_type *o_ptr;
    object_kind *k_ptr;

    /*** Recharge equipment ***/
    for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
    {
        /* Get the object */
        o_ptr = &inventory[i];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Skip the swap weapon */
        if ((adult_swap_weapons) && (i == INVEN_SWAP_WEAPON)) continue;

        /* Recharge activatable objects */
        if (o_ptr->timeout > 0 && !o_ptr->is_fuelable_lite())
        {
            /* Recharge */
            o_ptr->timeout--;

            /* Notice changes */
            if (!(o_ptr->timeout))
            {
                /* Update window */
                j++;

                /* Message if item is recharged, if inscribed !! */
                if (!(o_ptr->timeout)) recharged_notice(o_ptr, TRUE);
            }
        }
    }

    /* Notice changes */
    if (j)
    {
        /* Redraw stuff */
        p_ptr->redraw |= (PR_EQUIP);
    }

    /* Recharge rods */
    for (j = 0, i = 0; i < INVEN_PACK; i++)
    {
        o_ptr = &inventory[i];
        k_ptr = &k_info[o_ptr->k_idx];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Examine all charging rods or stacks of charging rods. */
        if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
        {
            /* Determine how many rods are charging. */
            s16b temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

            if (temp > o_ptr->number) temp = o_ptr->number;

            /* Decrease timeout by that number. */
            o_ptr->timeout -= temp;

            /* Boundary control. */
            if (o_ptr->timeout < 0) o_ptr->timeout = 0;

            /* Update if any rods are recharged */
            if (temp > (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval)
            {
                /* Update window */
                j++;

            /* Message if whole stack is recharged, if inscribed !! */
            if (!(o_ptr->timeout)) recharged_notice(o_ptr, TRUE);

            /* Message if first rod in the stack is recharged, if inscribed !! */
            else if (temp == o_ptr->number) recharged_notice(o_ptr, FALSE);

            }

        }
    }

    /* Notice changes */
    if (j)
    {
        /* Combine pack */
        p_ptr->notice |= (PN_COMBINE);

        /* Redraw stuff */
        p_ptr->redraw |= (PR_INVEN | PR_ITEMLIST);
    }

    /*** Recharge the ground ***/
    for (i = 1; i < o_max; i++)
    {
        /* Get the object */
        o_ptr = &o_list[i];

        /* Skip non-objects */
        if (!o_ptr->k_idx) continue;

        /* Examine all charging rods or stacks of charging rods. */
        if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
        {
            s16b temp;

            k_ptr = &k_info[o_ptr->k_idx];

            /* Determine how many rods are charging. */
            temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

            if (temp > o_ptr->number) temp = o_ptr->number;

            /* Decrease timeout by that number. */
            o_ptr->timeout -= temp;

            /* Boundary control. */
            if (o_ptr->timeout < 0) o_ptr->timeout = 0;

        }
    }

    /*re-charge rods and artifacts in the home*/
    for (i = 0; i < MAX_INVENTORY_HOME	; ++i)
    {
        store_type *st_ptr = &store[STORE_HOME];

        /* Object */
        o_ptr = &st_ptr->stock[i];
        k_ptr = &k_info[o_ptr->k_idx];

        /* Skip empty objects */
        if (!o_ptr->k_idx) continue;

        /* Examine all charging rods or stacks of charging rods. */
        if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
        {
            /* Determine how many rods are charging. */
            s16b temp = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

            if (temp > o_ptr->number) temp = o_ptr->number;

            /* Decrease timeout by that number. */
            o_ptr->timeout -= temp;

            /* Boundary control. */
            if (o_ptr->timeout < 0) o_ptr->timeout = 0;

        }
        else if ((o_ptr->art_num) && (o_ptr->timeout))
        {
            /* Decrease timeout by that number. */
            o_ptr->timeout--;

            /* Boundary control, paranoia. */
            if (o_ptr->timeout < 0) o_ptr->timeout = 0;
        }
    }
}


static void play_ambient_sound(void)
{
    /* Town sound */
    if (p_ptr->depth == 0)
    {
        /* Hack - is it daytime or nighttime? */
        if (turn % (10L * TOWN_DAWN) < TOWN_DAWN / 2)
        {
            /* It's day. */
            sound(MSG_AMBIENT_DAY);
        }
        else
        {
            /* It's night. */
            sound(MSG_AMBIENT_NITE);
        }

    }

    /* Dungeon level 1-20 */
    else if (p_ptr->depth <= 20)
    {
        sound(MSG_AMBIENT_DNG1);
    }

    /* Dungeon level 21-40 */
    else if (p_ptr->depth <= 40)
    {
        sound(MSG_AMBIENT_DNG2);
    }

    /* Dungeon level 41-60 */
    else if (p_ptr->depth <= 60)
    {
        sound(MSG_AMBIENT_DNG3);
    }

    /* Dungeon level 61-80 */
    else if (p_ptr->depth <= 80)
    {
        sound(MSG_AMBIENT_DNG4);
    }

    /* Dungeon level 80- */
    else
    {
        sound(MSG_AMBIENT_DNG5);
    }
}


/*
 * This function randomly extinguish fires near the player location
 */
static void put_out_fires(void)
{
    u16b feat;
    int y1, y2;
    int x1, x2;

    /* Debug message */
    if (cheat_room)
    {
        color_message(QString("Putting out fires."), MSG_NOTICE);
        disturb(0, 0);
    }

    /* Get the bottom-right corner of a rectangle centered on the player */
    y2 = MIN(p_ptr->cur_map_hgt - 2, p_ptr->py + MAX_SIGHT);
    x2 = MIN(p_ptr->cur_map_wid - 2, p_ptr->px + MAX_SIGHT);

    /* Traverse the rectangle */
    for (y1 = MAX(1, p_ptr->py - MAX_SIGHT); y1 <= y2; y1++)
    {
        for (x1 = MAX(1, p_ptr->px - MAX_SIGHT); x1 <= x2; x1++)
        {
            /* Get the feature */
            feat = dungeon_info[y1][x1].feat;

            /* Must be in the line of fire (to avoid abuses) */
            if (!player_can_fire_bold(y1, x1)) continue;

            /* Must be a fire */
            if (!feat_ff3_match(feat, ELEMENT_FIRE)) continue;

            /* Must be sensitive to cold  */
            if (!feat_ff2_match(feat, FF2_HURT_COLD)) continue;

            /* Get the new feature */
            feat = feat_state(feat, FS_HURT_COLD);

            /* The fire is burning oil, ignore */
            if (feat_ff3_match(feat, ELEMENT_OIL)) continue;

            /* Randomness */
            if (!one_in_(20)) continue;

            /* Extinguish the fire */
            cave_set_feat(y1, x1, feat);
        }
    }

    /* Rescan the element flags of the level */
    update_level_flag();
}


/*
 * Helper for process_world -- decrement p_ptr->timed[] fields.
 */
static void decrease_timeouts(void)
{
    int adjust = (adj_con_fix[p_ptr->state.stat_ind[A_CON]] + 1);
    int i;

    /* Decrement all effects that can be done simply */
    for (i = 0; i < TMD_MAX; i++)
    {
        int decr = 1;
        if (!p_ptr->timed[i])
            continue;

        switch (i)
        {
            case TMD_CUT:
            {
                /* Hack -- check for truly "mortal" wound */
                decr = (p_ptr->timed[i] > 1000) ? 0 : adjust;
                break;
            }

            case TMD_POISONED:
            case TMD_STUN:
            {
                decr = adjust;
                break;
            }
        }
        /* Decrement the effect */
        dec_timed(i, decr, FALSE);
    }

    return;
}


/*
 * Handle certain things once every 10 game turns
 */
void process_world(void)
{
    int i;

    int regen_amount;

    int feat;

    object_type *o_ptr;

    /* We decrease noise slightly every game turn */
    total_wakeup_chance -= 400;

    /* But the character always makes some noise */
    if (total_wakeup_chance < p_ptr->base_wakeup_chance)
        total_wakeup_chance = p_ptr->base_wakeup_chance;

    /* Every 10 game turns */
    if (turn % 10) return;

    /*** Update quests ***/
    if (guild_quest_active())
    {
        quest_type *q_ptr = &q_info[GUILD_QUEST_SLOT];

        /* Check for failure */
        if (guild_quest_level() != p_ptr->depth)
        {
            if (one_in_(20))
            {
                if (!(turn % QUEST_TURNS))
                {
                    if (quest_might_fail_now())
                    {
                        quest_fail();
                    }
                }
            }
        }
        /* We are on the quest level */
        else
        {
            if (q_ptr->q_type == QUEST_ARENA_LEVEL) 		process_arena_quest();
            else if (q_ptr->q_type == QUEST_LABYRINTH) 		process_labyrinth_quest();
            else if (q_ptr->q_type == QUEST_WILDERNESS)		process_wilderness_quest();
            else if (q_ptr->q_type == QUEST_GREATER_VAULT)	process_greater_vault_quest();
            else if (!(turn % QUEST_TURNS)) process_guild_quests();
        }
    }

    /* Play an ambient sound at regular intervals. */
    if (!(turn % ((10L * TOWN_DAWN) / 4)))
    {
        play_ambient_sound();
    }

    /*** Handle the "town" (stores and sunshine) ***/

    /* While in town */
    if (!p_ptr->depth)
    {
        /* Hack -- Daybreak/Nighfall in town */
        if (!(turn % ((10L * TOWN_DAWN) / 2)))
        {
            bool dawn;

            /* Check for dawn */
            dawn = (!(turn % (10L * TOWN_DAWN)));

            /* Day breaks */
            if (dawn)
            {
                /* Message */
                message(QString("The sun has risen."));
            }

            /* Night falls */
            else
            {
                /* Message */
                message(QString("The sun has fallen."));
            }

            /* Illuminate */
            town_illuminate(dawn);
        }
    }

    /* While in the dungeon */
    else
    {
        /*** Update the Stores ***/

        /* Update each store once a day (while in dungeon) */
        if (!(turn % (10L * STORE_TURNS)))
        {

            int n;

            /* Message */
            if (cheat_xtra) message(QString("Updating Shops..."));

            /* Maintain each shop (except home and guild) */
            for (n = 0; n < MAX_STORES; n++)
            {
                /* Skip the home */
                if (n == STORE_HOME) continue;
                if (n == STORE_GUILD)  continue;

                /* Maintain */
                store_maint(n);
            }

            /* Sometimes, shuffle the shop-keepers */
            if (one_in_(STORE_SHUFFLE))
            {

                /* Message */
                if (cheat_xtra) message(QString("Shuffling a Shopkeeper..."));

                /* Pick a random shop (except home and guild) */
                while (1)
                {
                    n = rand_int(MAX_STORES);
                    if ((n != STORE_HOME) && (n != STORE_GUILD)) break;
                }

                /* Shuffle it */
                store_shuffle(n);
            }

            /* Message */
            if (cheat_xtra) message(QString("Done."));
        }
    }

    /*** Process the monsters ***/

    /* Check for creature generation */
    if (one_in_(MAX_M_ALLOC_CHANCE))
    {
        /*
         * Make a new monster where it is allowed
         */
        if ((*dun_cap->allow_level_repopulation)())
        {
            (void)alloc_monster(MAX_SIGHT + 5, (MPLACE_SLEEP | MPLACE_GROUP | MPLACE_NO_MIMIC | MPLACE_NO_GHOST));
        }
    }

    /* Occasionally have the ghost give a challenge */
    if (turn % 500)
    {
        if (one_in_(50)) ghost_challenge();
    }

    /* Put out fire if necessary */
    if ((level_flag & (LF1_FIRE)) && !(turn % 1000)) put_out_fires();

    /* Hack -- Check for terrain damage */
    monster_terrain_damage();

    /* Hack -- Check for creature regeneration */
    if (!(turn % 100)) regen_monsters();

    /* Process effects */
    process_effects();

    /* Process dynamic dungeon grids */
    process_dynamic_terrain();

    /* Show stacked monster messages */
    notice_stuff();

    /*** Damage over Time ***/

    /* Get the feature */
    feat = dungeon_info[p_ptr->py][p_ptr->px].feat;

    /* If paralyzed, we drown in deep */
    if ((p_ptr->timed[TMD_PARALYZED] || (p_ptr->timed[TMD_STUN] >= 100)) &&
        feat_ff2_match(feat, FF2_DEEP))
    {
        /* Calculate damage */
        int dam = damroll(4, 6);

        /* Don't kill the player, just hurt him/her */
        if (dam <= p_ptr->chp)
        {

            /* Get the feature name */
            QString name = feature_desc(feat, TRUE, TRUE);

            message(QString("You are drowning in %1!") .arg(name));

            /* Apply the blow */
            take_hit(dam, "drowning");
        }
    }

    /* Take damage from poison */
    if (p_ptr->timed[TMD_POISONED])
    {
        /* Take damage */
        if(!(p_ptr->state.immune_pois))take_hit(1, "poison");
    }

    /* Take damage from cuts */
    if (p_ptr->timed[TMD_CUT])
    {
        /* Mortal wound or Deep Gash */
        if (p_ptr->timed[TMD_CUT] > 200)
        {
            i = 3;
        }

        /* Severe cut */
        else if (p_ptr->timed[TMD_CUT] > 100)
        {
            i = 2;
        }

        /* Other cuts */
        else
        {
            i = 1;
        }

        /* Take damage */
        take_hit(i, "a fatal wound");
    }

    /* Don't bother with the rest if the player is dead. */
    if (p_ptr->is_dead) return;

    /*** Check the Food, and Regenerate ***/

    /* Digest normally */
    if (p_ptr->food < PY_FOOD_MAX)
    {
        /* Every 100 game turns */
        if (!(turn % 100))
        {
            /* Basic digestion rate based on speed */
            i = calc_energy_gain(p_ptr->state.p_speed) * 2;

            /* Regeneration takes more food */
            if (p_ptr->state.regenerate) i += 30;

            /* Slow digestion takes less food */
            if (p_ptr->state.slow_digest) i -= 10;

            /* Minimal digestion */
            if (i < 1) i = 1;

            /* Digest some food */
            (void)set_food(p_ptr->food - i);
        }
    }

    /* Digest quickly when gorged */
    else
    {
        /* Digest a lot of food */
        (void)set_food(p_ptr->food - 100);
    }

    /* Starve to death (slowly) */
    if (p_ptr->food < PY_FOOD_STARVE)
    {
        /* Calculate damage */
        i = (PY_FOOD_STARVE - p_ptr->food) / 10;

        /* Take damage */
        take_hit(i, "starvation");
    }

    /* Default regeneration */
    regen_amount = PY_REGEN_NORMAL;

    /* Getting Weak */
    if (p_ptr->food < PY_FOOD_WEAK)
    {
        /* Lower regeneration */
        if (p_ptr->food < PY_FOOD_STARVE)
        {
            regen_amount = 0;
        }
        else if (p_ptr->food < PY_FOOD_FAINT)
        {
            regen_amount = PY_REGEN_FAINT;
        }
        else
        {
            regen_amount = PY_REGEN_WEAK;
        }

        /* Getting Faint */
        if (p_ptr->food < PY_FOOD_FAINT)
        {
            /* Faint occasionally */
            if (!p_ptr->timed[TMD_PARALYZED] && (rand_int(100) < 10))
            {
                /* Message */
                message(QString("You faint from the lack of food."));
                disturb(1, 0);

                /* Hack -- faint (bypass free action) */
                (void)inc_timed(TMD_PARALYZED, 1 + rand_int(5), TRUE);
            }
        }
    }

    /* Regeneration ability */
    if (p_ptr->state.regenerate)
    {
        regen_amount = regen_amount * 2;
    }

    /* Searching or Resting */
    if (p_ptr->searching || p_ptr->resting)
    {
        regen_amount = regen_amount * 2;
    }

    /* Regenerate the mana */
    if (p_ptr->csp < p_ptr->msp)
    {
        regenmana(regen_amount);
    }

    /* Various things interfere with healing */
    if (p_ptr->timed[TMD_PARALYZED]) regen_amount = 0;
    if ((p_ptr->timed[TMD_POISONED]) && (!(p_ptr->state.immune_pois))) regen_amount = 0;
    if (p_ptr->timed[TMD_STUN]) regen_amount = 0;
    if (p_ptr->timed[TMD_CUT]) regen_amount = 0;

    /* Regenerate Hit Points if needed */
    if (p_ptr->chp < p_ptr->mhp)
    {
        regenhp(regen_amount);
    }

    /*** Timeout Various Things ***/

    decrease_timeouts();

    /* Warn about flying */
    if (p_ptr->timed[TMD_FLYING])
    {
        if ((p_ptr->timed[TMD_FLYING] <= 3) && (p_ptr->timed[TMD_FLYING] > 0) &&
            (!p_ptr->state.ffall || ((f_info[dungeon_info[p_ptr->py][p_ptr->px].feat].dam_non_native > 0) &&
             !is_player_native(p_ptr->py, p_ptr->px))))
        {
            color_message(QString("You are about to stop flying."), MSG_LOSING_FLYING);

            disturb(0, 0);

        }
    }

    /*Temporary Native Flags*/

    /* Native to Lava */
    if (p_ptr->timed[TMD_NAT_LAVA])
    {
        if ((p_ptr->timed[TMD_NAT_LAVA]) && (p_ptr->timed[TMD_NAT_LAVA] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_LAVA))
            {
                color_message(QString("You are about to lose nativity to lava."), MSG_LOSING_NATIVITY);

                disturb(0, 0);

            }
        }

    }

    /* Native to Oil */
    if (p_ptr->timed[TMD_NAT_OIL])
    {
        if ((p_ptr->timed[TMD_NAT_OIL]) && (p_ptr->timed[TMD_NAT_OIL] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_OIL))
            {
                color_message(QString("You are about to lose nativity to oil."), MSG_LOSING_NATIVITY);

                disturb(0, 0);

            }
        }
    }

    /* Native to Sand */
    if (p_ptr->timed[TMD_NAT_SAND])
    {
        if ((p_ptr->timed[TMD_NAT_SAND]) && (p_ptr->timed[TMD_NAT_SAND] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_SAND))
            {
                color_message(QString("You are about to lose nativity to sand."), MSG_LOSING_NATIVITY);

                disturb(0, 0);
            }
        }
    }

    /* Native to Forest */
    if (p_ptr->timed[TMD_NAT_TREE])
    {
        if ((p_ptr->timed[TMD_NAT_TREE]) && (p_ptr->timed[TMD_NAT_TREE] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_FOREST))
            {
                color_message(QString("You are about to lose nativity to forest."), MSG_LOSING_NATIVITY);

                disturb(0, 0);
            }
        }
    }

    /* Native to Water */
    if (p_ptr->timed[TMD_NAT_WATER])
    {
        if ((p_ptr->timed[TMD_NAT_WATER]) && (p_ptr->timed[TMD_NAT_WATER] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_WATER))
            {
                color_message(QString("You are about to lose nativity to water."), MSG_LOSING_NATIVITY);

                disturb(0, 0);
            }
        }
    }

    /* Native to Mud */
    if (p_ptr->timed[TMD_NAT_MUD])
    {
        if ((p_ptr->timed[TMD_NAT_MUD]) && (p_ptr->timed[TMD_NAT_MUD] < 5))
        {
            if (cave_ff3_match(p_ptr->py, p_ptr->px, FF3_MUD))
            {
                color_message(QString("You are about to lose nativity to mud."), MSG_LOSING_NATIVITY);

                disturb(0, 0);
            }
        }
    }

    /* Animate trees if necessary */
    if (p_ptr->timed[TMD_CALL_HOURNS] > 0) call_huorns();

    /*** Process Light ***/

    /* Check for light being wielded */
    o_ptr = &inventory[INVEN_LIGHT];

    /* Burn some fuel in the current lite */
    if (o_ptr->tval == TV_LIGHT)
    {
        /* Hack -- Use some fuel (except on artifacts) */
        if (!o_ptr->is_artifact() && (o_ptr->timeout > 0) &&
            !(dungeon_info[p_ptr->py][p_ptr->px].cave_info & (CAVE_GLOW | CAVE_HALO)))
        {
            /* Decrease life-span */
            o_ptr->timeout--;

            /* Hack -- notice interesting fuel steps */
            if ((o_ptr->timeout < 100) || (!(o_ptr->timeout % 100)))
            {
                /* Redraw stuff */
                p_ptr->redraw |= (PR_EQUIP);
            }

            /* Hack -- Special treatment when blind */
            if (p_ptr->timed[TMD_BLIND])
            {
                /* Hack -- save some light for later */
                if (o_ptr->timeout == 0) o_ptr->timeout++;
            }

            /* The light is now out */
            else if (o_ptr->timeout == 0)
            {
                disturb(0, 0);
                message(QString("Your light has gone out!"));
            }

            /* The light is getting dim */
            else if ((o_ptr->timeout < 100) && (!(o_ptr->timeout % 10)))
            {
                disturb(0, 0);
                message(QString("Your light is growing faint."));
            }
        }
    }


    /* Calculate torch radius */
    p_ptr->update |= (PU_TORCH);

    /*** Process Inventory ***/

    /* Handle experience draining */
    if (p_ptr->state.exp_drain)
    {
        if ((rand_int(100) < 10) && (p_ptr->exp > 0))
        {
            p_ptr->exp--;
            p_ptr->max_exp--;
            check_experience();
        }
    }

    /* Process mimic objects */
    process_mimics();

    /* Recharge activatable objects and rods */
    recharge_objects();

    /* Feel the inventory */
    sense_inventory();


    /*** Process Objects ***/

    /*** Involuntary Movement ***/

    /* Mega-Hack -- Random teleportation XXX XXX XXX */
    if ((p_ptr->state.teleport) && (rand_int(100) < 1))
    {
        /* Teleport player */
        teleport_player(40, FALSE);
    }

    /* Delayed Word-of-Recall */
    if (p_ptr->word_recall)
    {
        /* Count down towards recall */
        p_ptr->word_recall--;

        /* Activate the recall */
        if (!p_ptr->word_recall)
        {
            /* Disturbing! */
            disturb(0, 0);

            /* Sound */
            sound(MSG_TPLEVEL);

            /* Determine the level */
            if (p_ptr->depth)
            {
                message(QString("You feel yourself yanked upwards!"));

                /* Go to the town. */
                dungeon_change_level(0);
            }
            else
            {
                /* New depth */
                int new_depth = p_ptr->recall_depth;
                if (new_depth < 1) new_depth = 1;

                message(QString("You feel yourself yanked downwards!"));

                dungeon_change_level(new_depth);
            }
        }
    }

    /* Delayed level feelings */
    if ((p_ptr->depth) && (!p_ptr->leaving) && (!do_feeling) && (!(turn % 100)))
    {
        int chance;

        /*players notice arena levels almost instantly */
        if (p_ptr->dungeon_type== DUNGEON_TYPE_ARENA) chance = 2;

        /*players notice wilderness and labyrinth levels almost as quickly */
        else if (p_ptr->dungeon_type== DUNGEON_TYPE_WILDERNESS) chance = 10;
        else if (p_ptr->dungeon_type== DUNGEON_TYPE_LABYRINTH) chance = 10;
        else if (p_ptr->dungeon_type== DUNGEON_TYPE_GREATER_VAULT) chance = 10;

        /* Players notice themed levels quickly as well */
        else if (p_ptr->dungeon_type >= DUNGEON_TYPE_THEMED_LEVEL) chance = 20;

        else chance = 40;

        /* After sufficient time, can learn about the level */
        if ((rand_int(chance) < p_ptr->state.skills[SKILL_SEARCH]) &&
            (rand_int(chance) < p_ptr->state.skills[SKILL_SEARCH]))
        {
            /* Now have a feeling */
            do_feeling = TRUE;

            /* Announce feeling */
            do_cmd_feeling();

            /* Update the level indicator */
            p_ptr->redraw |= (PR_DEPTH | PR_FEELING);

            /* Disturb */
            disturb(0, 0);
        }
    }

    /* Notice stuff */
    notice_stuff();

}
