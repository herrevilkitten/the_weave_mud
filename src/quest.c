/***************************************************************************
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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"


void do_quest( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int position, old_con;

    if ( argument[0] == '\0' )
    {
	if ( IS_SET(ch->act, PLR_QUESTING) )
	{
	    send_to_char( "Okay, you are no longer part of the quest.\n\r", ch );
	    REMOVE_BIT(ch->act, PLR_QUESTING);
	    return;
	}
	send_to_char( "Okay, you are now part of the quest.\n\r", ch );
	SET_BIT(ch->act, PLR_QUESTING);
	return;
    }

    sprintf( buf, "[QUEST] %s: $7$T", FIRSTNAME(ch) );
    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( vch )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( !IS_SET(vch->act, PLR_QUESTING) )
		continue;

	    position		= vch->position;
	    old_con		= d->connected;
	    d->connected	= CON_PLAYING;
	    vch->position	= POS_STANDING;

	   act( buf, vch, NULL, argument, TO_CHAR );

	    vch->position	= position;
	    d->connected	= old_con;
	}
    }
    return;
}

/* Quest Item Calculation */
int quest_cost( OBJ_DATA *obj )
{
    int cost = 0;
    int i, mod;
    AFFECT_DATA *af;

    switch ( obj->item_type )
    {
	case ITEM_WEAPON:
	    if ( (obj->value[1] * obj->value[2]) > 60 )
		cost += ( (obj->value[1] * obj->value[2] - 60) * 6 );

	    if ( IS_SET(obj->value[4], WEAPON_FLAMING) )
		cost += 120;
	    if ( IS_SET(obj->value[4], WEAPON_FROST) )
		cost += 120;
	    if ( IS_SET(obj->value[4], WEAPON_SHARP) )
		cost += 90;

	    break;
	case ITEM_ARMOR:
	    for ( i = 0; i < 4; i++ )
		cost += (obj->value[i] * 2);
	    break;
	case ITEM_LIGHT:
	    if ( obj->value[2] == -1 || obj->value[2] == 999 )
		cost += 90;
	    break;
	case ITEM_CONTAINER:
	    if ( obj->value[0] == -1 )
		cost += 90;
	    break;
	case ITEM_DRINK_CON:
	    if ( obj->value[0] == -1 || obj->value[1] == -1 )
		cost += 90;
	    break;
	default:
	    break;
    }

    if ( obj->material == MAT_HEARTSTONE )
	cost += 240;
    if ( obj->material == MAT_DARKSILVER )
	cost += 180;

    if ( !obj->enchanted )
    {
	int mod;
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch( af->location )
	    {
		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_INT:
		case APPLY_WIS:
		case APPLY_CON:
		case APPLY_CHR:
		case APPLY_LUK:
		case APPLY_AGI:
		    cost += (15 * mod);
		    break;
		case APPLY_HIT:
		case APPLY_STAMINA:
		    cost += (4 * mod);
		    break;
		case APPLY_AC:
		    cost += (2 * mod);
		    break;
		case APPLY_HITROLL:
		case APPLY_DAMROLL:
		    cost += (30 * mod);
		    break;
		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		    cost += (15 * mod);
		    break;
		case APPLY_SPELL:
		    cost += 240;
		    break;
		case APPLY_SKILL:
		    cost += 60;
		    break;
		default:
		    break;
	    }
	}
    }
    for ( af = obj->affected; af != NULL; af = af->next )
    {
	mod = af->modifier;
	switch( af->location )
	{
	    case APPLY_STR:
	    case APPLY_DEX:
	    case APPLY_INT:
	    case APPLY_WIS:
	    case APPLY_CON:
	    case APPLY_CHR:
	    case APPLY_LUK:
	    case APPLY_AGI:
		cost += (15 * mod);
		break;
	    case APPLY_HIT:
	    case APPLY_STAMINA:
		cost += (4 * mod);
		break;
	    case APPLY_AC:
		cost += (2 * mod);
		break;
	    case APPLY_HITROLL:
	    case APPLY_DAMROLL:
		cost += (30 * mod);
		break;
	    case APPLY_SAVING_PARA:
	    case APPLY_SAVING_ROD:
	    case APPLY_SAVING_PETRI:
	    case APPLY_SAVING_BREATH:
	    case APPLY_SAVING_SPELL:
		cost += (15 * mod);
		break;
	    case APPLY_SPELL:
		cost += 240;
		break;
	    case APPLY_SKILL:
		cost += 60;
		break;
	    default:
		break;
	}
    }

    return( UMAX(1, cost / 30) );
}

void do_giveqp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int number;

    argument = one_argument( argument, arg );

    if ( (victim = get_char_world( ch, arg )) == NULL )
    {
	send_to_char( "They are not playing.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "You cannot give quest points to NPCs.\n\r", ch );
	return;
    }

    number = atoi( argument );

    if ( number < 1 )
    {
	send_to_char( "You must give at least 1 quest point.\n\r", ch );
	return;
    }

    victim->pcdata->qp += number;
    sprintf( buf, "You award %d quest points to $N.", number );
    act( buf, ch, NULL, victim, TO_CHAR );
    sprintf( buf, "$n awards you %d quest points.", number );
    act( buf, ch, NULL, victim, TO_VICT );
    return;
}

void do_takeqp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int number;

    argument = one_argument( argument, arg );

    if ( (victim = get_char_world( ch, arg )) == NULL )
    {
	send_to_char( "They are not playing.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "You cannot take quest points from NPCs.\n\r", ch );
	return;
    }

    number = atoi( argument );

    if ( number < 1 )
    {
	send_to_char( "You must take at least 1 quest point.\n\r", ch );
	return;
    }

    victim->pcdata->qp -= number;
    victim->pcdata->qp = UMAX( 0, victim->pcdata->qp );
    sprintf( buf, "You take %d quest points from $N.", number );
    act( buf, ch, NULL, victim, TO_CHAR );
    sprintf( buf, "$n takes %d quest points from you.", number );
    act( buf, ch, NULL, victim, TO_VICT );
    return;
}

void do_givexp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int number;

    argument = one_argument( argument, arg );

    if ( (victim = get_char_world( ch, arg )) == NULL )
    {
	send_to_char( "They are not playing.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "You cannot give experience points to NPCs.\n\r", ch );
	return;
    }

    number = atoi( argument );

    if ( number < 1 || number > 250 )
    {
	send_to_char( "You can only give between 1 and 250 experience points.\n\r", ch );
	return;
    }

    sprintf( buf, "You award %d experience point%s to $N.", number,
	number != 1 ? "s" : "" );
    act( buf, ch, NULL, victim, TO_CHAR );
    sprintf( buf, "$n awards you %d experience point%s.", number,
	number != 1 ? "s" : "" );
    act( buf, ch, NULL, victim, TO_VICT );
    gain_exp( victim, number );
    return;
}

void do_takexp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int number;

    argument = one_argument( argument, arg );

    if ( (victim = get_char_world( ch, arg )) == NULL )
    {
	send_to_char( "They are not playing.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "You cannot take experience points from NPCs.\n\r", ch );
	return;
    }

    number = atoi( argument );

    if ( number < 1 || number > 250 )
    {
	send_to_char( "You can only take between 1 and 250 experience points.\n\r", ch );
	return;
    }

    sprintf( buf, "You take %d experience point%s from $N.", number,
	number != 1 ? "s" : "" );
    act( buf, ch, NULL, victim, TO_CHAR );
    sprintf( buf, "$n takes %d experience point%s from you.", number,
	number != 1 ? "s" : "" );
    act( buf, ch, NULL, victim, TO_VICT );
    return;
}


int item_level( OBJ_DATA *obj )
{
    int cost = 0;
    int i, mod;
    AFFECT_DATA *af;

    switch ( obj->item_type )
    {
	case ITEM_WEAPON:
	    cost += (obj->value[1] * obj->value[2]) * 5 / 4;
	    cost *= 30;

	    if ( IS_SET(obj->value[4], WEAPON_FLAMING) )
		cost += 90;
	    if ( IS_SET(obj->value[4], WEAPON_FROST) )
		cost += 90;
	    if ( IS_SET(obj->value[4], WEAPON_SHARP) )
		cost += 60;

	    break;
	case ITEM_ARMOR:
	    for ( i = 0; i < 4; i++ )
		cost += obj->value[i];
	    cost = cost * 100 / 120;
	    cost *= 30;
	    break;
	case ITEM_LIGHT:
	    if ( obj->value[2] == -1 || obj->value[2] == 999 )
		cost = 3000;
	    else
		cost = UMIN( 99, obj->value[2] / 2 ) * 30;
	    break;
	case ITEM_CONTAINER:
	    if ( obj->value[0] == -1 )
		cost = 3000;
	    else
		cost = UMIN( 99, obj->value[0] / 4 ) * 30;
	    break;
	case ITEM_DRINK_CON:
	    if ( obj->value[0] == -1 || obj->value[1] == -1 )
		cost = 3000;
	    else
		cost = UMIN( 99, obj->value[0] / 3 ) * 30;
	    break;
	default:
	    cost = obj->level * 30;
	    break;
    }

    if ( obj->material == MAT_HEARTSTONE )
	cost += 240;
    if ( obj->material == MAT_DARKSILVER )
	cost += 180;

    if ( !obj->enchanted )
    {
	int mod;
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch( af->location )
	    {
		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_INT:
		case APPLY_WIS:
		case APPLY_CON:
		case APPLY_CHR:
		case APPLY_LUK:
		case APPLY_AGI:
		    cost += (15 * mod);
		    break;
		case APPLY_HIT:
		case APPLY_STAMINA:
		    cost += (4 * mod);
		    break;
		case APPLY_AC:
		    cost += (2 * mod);
		    break;
		case APPLY_HITROLL:
		case APPLY_DAMROLL:
		    cost += (30 * mod);
		    break;
		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		    cost += (15 * mod);
		    break;
		case APPLY_SPELL:
		    cost += 240;
		    break;
		case APPLY_SKILL:
		    cost += 60;
		    break;
		default:
		    break;
	    }
	}
    }
    for ( af = obj->affected; af != NULL; af = af->next )
    {
	mod = af->modifier;
	switch( af->location )
	{
	    case APPLY_STR:
	    case APPLY_DEX:
	    case APPLY_INT:
	    case APPLY_WIS:
	    case APPLY_CON:
	    case APPLY_CHR:
	    case APPLY_LUK:
		cost += (15 * mod);
		break;
	    case APPLY_HIT:
	    case APPLY_STAMINA:
		cost += (4 * mod);
		break;
	    case APPLY_AC:
		cost += (2 * mod);
		break;
	    case APPLY_HITROLL:
	    case APPLY_DAMROLL:
		cost += (30 * mod);
		break;
	    case APPLY_SAVING_PARA:
	    case APPLY_SAVING_ROD:
	    case APPLY_SAVING_PETRI:
	    case APPLY_SAVING_BREATH:
	    case APPLY_SAVING_SPELL:
		cost += (15 * mod);
		break;
	    case APPLY_SPELL:
		cost += 240;
		break;
	    case APPLY_SKILL:
		cost += 60;
		break;
	    default:
		break;
	}
    }

    return( UMAX(1, cost / 30) );
}
