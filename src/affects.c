/**************************************************************************r
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "mem.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return	);
DECLARE_DO_FUN(do_stand		);

AFFECT_DATA *		affect_free;



/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	apply_spell	args( ( CHAR_DATA *ch, int mod ) );

/*
 *   AFFECT FUNCTIONS
 */

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	SET_BIT( ch->affected_by, paf->bitvector );
	SET_BIT( ch->affected_by_2, paf->bitvector_2 );
    }
    else
    {
	REMOVE_BIT( ch->affected_by, paf->bitvector );
	REMOVE_BIT( ch->affected_by_2, paf->bitvector_2 );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_CHR:           ch->mod_stat[STAT_CHR]	+= mod;	break;
    case APPLY_LUK:           ch->mod_stat[STAT_LUK]	+= mod;	break;
    case APPLY_AGI:           ch->mod_stat[STAT_AGI]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_HIT:		ch->max_hit		+= mod;	break;
    case APPLY_STAMINA:		ch->max_stamina		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SPELL:
	apply_spell( ch, mod );
	break;

    case APPLY_SKILL:
	if ( !IS_NPC(ch) )
	{
	    if ( mod >= 0 )
	    {
	        if ( ch->pcdata->skill_mod[slot_lookup(mod)] > 0 )
		    ch->pcdata->skill_mod[slot_lookup(mod)] += 5;
	    }
	    else
	    {
	        if ( ch->pcdata->skill_mod[slot_lookup(abs(mod))] > 0 )
		    ch->pcdata->skill_mod[slot_lookup(abs(mod))] -= 5;
	    }
	}
	break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > str_app[get_curr_stat(ch,STAT_STR)].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}

void apply_spell( CHAR_DATA *ch, int mod )
{
    if ( mod > 0 )
    {
	if ( slot_lookup(mod) == gsn_haste )
	{
	    if ( !IS_SET(ch->affected_by, AFF_HASTE) )
	    {
		SET_BIT( ch->affected_by, AFF_HASTE );
		act("$n begins moving more quickly.",ch,NULL,NULL,TO_ROOM);
		send_to_char( "You begins moving more quickly.\n\r", ch );
	    }
	}
	else if ( slot_lookup(mod) == gsn_invis )
	{
	    if ( !IS_SET( ch->affected_by, AFF_INVISIBLE) )
	    {
		SET_BIT( ch->affected_by, AFF_INVISIBLE );
   		act( "$n snaps out of existence.", ch, NULL,NULL, TO_ROOM );
    		send_to_char( "You snap out of existence.\n\r", ch );
	    }
	}
	else if ( slot_lookup(mod) == gsn_air_armor )
	{
	    if ( !IS_SET(ch->affected_by, AFF_AIR_ARMOR) )
	    {
		SET_BIT( ch->affected_by, AFF_AIR_ARMOR );
    		send_to_char( "Flows of air harden around you, forming armor.\n\r", ch );
	    }
	}
	else if ( slot_lookup(mod) == gsn_ward_person )
	{
	    if ( !IS_SET(ch->affected_by, AFF_WARDED) )
	    {
		SET_BIT( ch->affected_by, AFF_WARDED );
		send_to_char( "Blue fills your vision for a moment.\n\r",ch );
	    }
	}
	else
	    bug("Apply_spell: unusable slot number %d.", mod );
    }
    else
    {
	if ( slot_lookup(abs( mod )) == gsn_haste )
	{
	    if ( IS_SET(ch->affected_by, AFF_HASTE) )
	    {
		REMOVE_BIT( ch->affected_by, AFF_HASTE );
		act("$n slows down and moves more normally.",ch,NULL,NULL,TO_ROOM);
		send_to_char( "You feel yourself slowing down.\n\r", ch );
	    }
	}
	else if ( slot_lookup(abs( mod )) == gsn_invis )
	{
	    if ( IS_SET(ch->affected_by, AFF_INVISIBLE) )
	    {
	    	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    	act( "$n snaps into view.", ch, NULL, NULL, TO_ROOM );
	    	send_to_char( "You snap into view.\n\r", ch );
	    }
	}
	else if ( slot_lookup(abs( mod )) == gsn_air_armor )
	{
	    if ( IS_SET(ch->affected_by, AFF_AIR_ARMOR) )
	    {
		REMOVE_BIT( ch->affected_by, AFF_AIR_ARMOR );
	    	send_to_char( "Your armor of air dissipates.\n\r", ch );
	    }
	}
	else if ( slot_lookup(abs( mod )) == gsn_ward_person )
	{
	    if ( IS_SET(ch->affected_by, AFF_WARDED) )
	    {
		REMOVE_BIT( ch->affected_by, AFF_WARDED );
		send_to_char( "You suddenly feel less safe.\n\r", ch );
	    }
	}
	else
	    bug("Reset_char: unusable slot number %d.", mod );
    }
    return;
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;
    void 	*vo;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    vo			= (void *) ch;
    add_weave_list( vo, NODE_WEAVE_CHAR );

    affect_modify( ch, paf_new, TRUE );
    return;
}

/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type && paf_old->flags == paf->flags )
	{
	    paf->strength = (paf->strength += paf_old->strength) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}


/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;
    void *vo;

    paf_new = new_affect();
    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    vo			= (void *) obj;
    add_weave_list( vo, NODE_WEAVE_OBJ );

    return;
}

void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;
    void *vo;

    paf_new = new_affect();
    *paf_new		= *paf;
    paf_new->next	= room->affected;
    room->affected	= paf_new;

    vo			= (void *) room;
    add_weave_list( vo, NODE_WEAVE_ROOM );

    return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    if ( paf->type == gsn_shape_change
    ||   paf->type == gsn_disguise )
    {

	if ( !IS_NPC(ch) )
	{
	    free_string( ch->pcdata->new_name );
	    free_string( ch->pcdata->new_last );
	    free_string( ch->pcdata->new_title );
	    free_string( ch->pcdata->new_desc );
	    ch->pcdata->new_name = NULL;
	    ch->pcdata->new_last = NULL;
	    ch->pcdata->new_title = NULL;
	    ch->pcdata->new_desc = NULL;
	}
    }

    if ( paf->type == skill_lookup("set limb") )
    {
	if ( cure_condition(ch, BODY_RIGHT_LEG, paf->strength) )
	    act( "$o right leg seems more straight now.", ch, NULL,
		NULL, TO_ALL );
	else if ( cure_condition(ch, BODY_LEFT_LEG, paf->strength) )
	    act( "$o left leg seems more straight now.", ch, NULL,
		NULL, TO_ALL );
	else if ( cure_condition(ch, BODY_RIGHT_ARM, paf->strength) )
	    act( "$o right arm seems more straight now.", ch, NULL,
		NULL, TO_ALL );
	else if ( cure_condition(ch, BODY_LEFT_ARM, paf->strength) )
	    act( "$o left arm seems more straight now.", ch, NULL,
		NULL, TO_ALL );
    }

    if ( paf == ch->affected )
    {
	ch->affected	= paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    if ( ch->affected == NULL )
    {
	void 	*vo;
	vo	= (void *) ch;
	rem_weave_list( vo, NODE_WEAVE_CHAR );
    }

    paf->next	= affect_free;
    affect_free	= paf->next;
    return;
}

void affect_obj_remove( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    if ( obj->affected == NULL )
    {
        bug( "Affect_obj_remove: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_obj_remove: cannot find paf.", 0 );
            return;
        }
    }

    if ( obj->affected == NULL )
    {
	void 	*vo;
	vo	= (void *) obj;
	rem_weave_list( obj, NODE_WEAVE_OBJ );
    }

    paf->next   = affect_free;
    affect_free = paf->next;
    return;
}

void affect_room_remove( ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    if ( room->affected == NULL )
    {
        bug( "Affect_room_remove: no affect.", 0 );
        return;
    }

    if ( paf == room->affected )
    {
        room->affected	= paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = room->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_room_remove: cannot find paf.", 0 );
            return;
        }
    }

    if ( room->affected == NULL )
    {
	void 	*vo;
	vo	= (void *) room;
	rem_weave_list( room, NODE_WEAVE_ROOM );
    }

    if ( room->people && skill_table[paf->type].msg_off )
	act( skill_table[paf->type].msg_off, room->people, NULL, NULL, TO_ALL );


    paf->next   = affect_free;
    affect_free = paf->next;
    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_obj_strip( OBJ_DATA *obj, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = obj->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_obj_remove( obj, paf );
    }

    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_room_strip( ROOM_INDEX_DATA *room, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = room->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_room_remove( room, paf );
    }

    return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    if ( ch == NULL )
    {
	bug( "Is_affected: NULL ch.", 0 );
	return FALSE;
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Return true if an obj is affected by a spell.
 */
bool is_obj_affected( OBJ_DATA *obj, int sn )
{
    AFFECT_DATA *paf;

    if ( obj == NULL )
    {
	bug( "Is_obj_affected: NULL obj.", 0 );
	return FALSE;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}


bool is_room_affected( ROOM_INDEX_DATA *vRoom, int sn )
{
    AFFECT_DATA *paf;

    if ( vRoom == NULL )
    {
	bug( "Is_room_affected: NULL room.", 0 );
	return FALSE;
    }

    for ( paf = vRoom->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

int affect_lookup( int vnum )
{
    if ( vnum == 4223 )
	return 25;

    if ( vnum == 4224 )
	return 26;

    if ( vnum == 22 )
	return 34;

    if ( vnum == 4251 )
	return 21;

    if ( vnum == 21 )
	return 69;

    return 0;
}

void affect_from_index( OBJ_DATA *obj )
{
    AFFECT_DATA *paf, *af_new;

    obj->enchanted	= TRUE;
    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
    {
	af_new		= new_affect( );
	af_new->next	= obj->affected;
	obj->affected	= af_new;

	af_new->type		= paf->type;
	af_new->strength	= paf->strength;
	af_new->duration	= paf->duration;
	af_new->location	= paf->location;
	af_new->modifier	= paf->modifier;
	af_new->bitvector	= paf->bitvector;
	af_new->bitvector_2	= paf->bitvector_2;
	af_new->owner		= NULL;
	af_new->flags		= paf->flags;
    }
    return;
}
