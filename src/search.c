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
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "olc.h"

bool    check_break     args( ( OBJ_DATA *obj, int damage ) );

void do_search( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i;
    int luck;

    luck = luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Search for herbs, lumber, gems, or ore?\n\r", ch );
	return;
    }

    if ( !str_prefix(argument, "herbs") )
    {
	if ( ch->in_room == NULL )
	    return;

	switch (ch->in_room->sector_type)
	{
	    default:
		send_to_char( "This is not the right environment to find herbs.\n\r", ch );
		return;
	    case SECT_WATER_SWIM:
	    case SECT_FOREST:
	    case SECT_MOUNTAIN:
	    case SECT_FIELD:
	    case SECT_SWAMP:
	    case SECT_DESERT:
	    case SECT_HILLS:
		break;
	}

	WAIT_STATE(ch, 6);
	for ( i = 0; herb_table[i].herb != HERB_NONE; i++ )
	{
	    int chance;

	    if ( herb_table[i].terrain != ch->in_room->sector_type )
		continue;

	    chance = herb_table[i].diff;

	    if ( IS_SET(ch->in_room->resources, RES_HERB) )
		chance = chance * 2 / 3;

	    if ( !IS_NPC(ch)
	    &&   ch->pcdata->talent[tn_herbalism] )
		chance = chance * 2 / 3;

	    if ( check_skill(ch, gsn_herbalism, 50, TRUE)
	    &&   number_percent() > chance )
	    {
		obj = create_object( get_obj_index(herb_table[i].vnum), 1 );

		act( "$n look$% around, and pick$% up $p.", ch, obj,
		    NULL, TO_ALL );
		obj_to_char( obj, ch );
		if ( number_bits(2) != 0 )
		{
		    REMOVE_BIT(ch->in_room->resources, RES_HERB);
		    num_herb--;
		}
		check_improve(ch, gsn_herbalism, TRUE, 4);
		return;
	    }
	}
	act( "$n looks around, but doesn't find anything.", ch, NULL, NULL,
	    TO_ROOM );
	send_to_char( "You look around, but don't find anything.\n\r", ch );
	return;
    }


    if ( !str_prefix(argument, "lumber") )
    {
	int chance;

	if ( ch->in_room == NULL )
	    return;

	switch (ch->in_room->sector_type)
	{
	    default:
		send_to_char( "This is not the right environment to find lumber.\n\r", ch );
		return;
	    case SECT_FOREST:
		chance = 70;
		break;
	    case SECT_SWAMP:
		chance = 85;
		break;
	    case SECT_HILLS:
		chance = 75;
		break;
	    case SECT_MOUNTAIN:
		chance = 95;
		break;
	}

	if ( (obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	||   obj->item_type != ITEM_WEAPON
	||   obj->value[0] != WEAPON_AXE )
	{
	    send_to_char( "You need an axe to cut down trees.\n\r", ch);
	    return;
	}
	WAIT_STATE(ch, 6);

	if ( IS_SET(ch->in_room->resources, RES_LUMBER) )
	    chance = chance * 2 / 3;

	if ( check_skill(ch, gsn_forestry, 50, TRUE)
	&&   number_percent() > chance )
	{
	    obj = create_object( get_obj_index(OBJ_VNUM_LUMBER), 1 );

	    act( "$n look$% around, chooses a good tree, and cuts it down for lumber.", ch, obj,
		NULL, TO_ALL );
	    obj_to_char( obj, ch );
	    if ( number_bits(2) != 0 )
	    {
		REMOVE_BIT(ch->in_room->resources, RES_LUMBER);
		num_lumber--;
	    }
	    check_improve(ch, gsn_forestry, TRUE, 4);
	    return;
	}
	act( "$n looks around, but doesn't find anything.", ch, NULL, NULL,
	    TO_ROOM );
	send_to_char( "You look around, but don't find anything.\n\r", ch );
	return;
    }


    if ( !str_prefix(argument, "ore") )
    {
	int chance;

	if ( ch->in_room == NULL )
	    return;

	switch (ch->in_room->sector_type)
	{
	    default:
		send_to_char( "This is not the right environment to find ore.\n\r", ch );
		return;
	    case SECT_HILLS:
		chance = 90;
		break;
	    case SECT_DESERT:
		chance = 98;
		break;
	    case SECT_MOUNTAIN:
		chance = 85;
		break;
	}
	if ( (obj = get_eq_char(ch, WEAR_HOLD)) == NULL
	||    obj->pIndexData->vnum != OBJ_VNUM_PICK )
	{
	    send_to_char( "You need to hold pick and shovels to mine.\n\r", ch );
	    return;
	}
	WAIT_STATE(ch, 6);

	check_break( obj, number_range(10, 30) );
	if ( IS_SET(ch->in_room->resources, RES_ORE) )
	    chance = chance * 2 / 3;

	if ( check_skill(ch, gsn_mining, 66, TRUE)
	&&   number_percent() > chance )
	{
	    int vnum;
	    switch( number_bits(4) )
	    {
		default:
		    send_to_char( "You find no ore here.\n\r", ch );
		    return;
		case 0:
		    vnum = OBJ_VNUM_GOLD;
		    break;
		case 1:
		case 2:
		    vnum = OBJ_VNUM_SILVER;
		    break;
		case 3:
		case 4:
		case 5:
		    vnum = OBJ_VNUM_ELECTRUM;
		    break;
		case 6:
		case 7:
		case 8:
		case 9:
		    vnum = OBJ_VNUM_IRON;
		    break;
		case 10:
		case 11:
		case 12:
		case 13:
		    vnum = OBJ_VNUM_COPPER;
		    break;
	    }

	    obj = create_object( get_obj_index(vnum), 1 );

	    act( "$n look$% around, and begins mining in the earth.", ch,
		obj, NULL, TO_ALL );
	    obj_to_char( obj, ch );
	    if ( number_bits(2) != 0 )
	    {
		REMOVE_BIT(ch->in_room->resources, RES_ORE);
		num_ore--;
	    }
	    act( "$n dig$% up $p.", ch, obj, NULL, TO_ALL );
	    check_improve(ch, gsn_mining, TRUE, 4);
	    return;
	}
	act( "$n looks around, but doesn't find anything.", ch, NULL, NULL,
	    TO_ROOM );
	send_to_char( "You look around, but don't find anything.\n\r", ch );
	return;
    }


    if ( !str_prefix(argument, "gems") )
    {
	int chance;

	if ( ch->in_room == NULL )
	    return;

	switch (ch->in_room->sector_type)
	{
	    default:
		send_to_char( "This is not the right environment to find gems.\n\r", ch );
		return;
	    case SECT_HILLS:
		chance = 97;
		break;
	    case SECT_DESERT:
		chance = 95;
		break;
	    case SECT_MOUNTAIN:
		chance = 90;
		break;
	}
	if ( (obj = get_eq_char(ch, WEAR_HOLD)) == NULL
	||    obj->pIndexData->vnum != OBJ_VNUM_PICK )
	{
	    send_to_char( "You need to hold pick and shovels to mine.\n\r", ch );
	    return;
	}
	WAIT_STATE(ch, 6);

	check_break( obj, number_range(10, 30) );
	if ( IS_SET(ch->in_room->resources, RES_GEMS) )
	    chance = chance * 2 / 3;

	if ( check_skill(ch, gsn_mining, 66, TRUE)
	&&   number_percent() > chance )
	{
	    int vnum;
	    switch( number_bits(4) )
	    {
		default:
		    send_to_char( "You find no gems here.\n\r", ch );
		    return;
		case 0:
		    vnum = OBJ_VNUM_DIAMOND;
		    break;
		case 1:
		case 2:
		    vnum = OBJ_VNUM_OPAL;
		    break;
		case 3:
		case 4:
		    vnum = OBJ_VNUM_EMERALD;
		    break;
		case 5:
		case 6:
		case 7:
		    vnum = OBJ_VNUM_RUBY;
		    break;
		case 8:
		case 9:
		case 10:
		    vnum = OBJ_VNUM_SAPPHIRE;
		    break;
		case 11:
		case 12:
		    vnum = OBJ_VNUM_OTHER_GEM;
		    break;

	    }

	    obj = create_object( get_obj_index(vnum), 1 );

	    act( "$n look$% around, and begins mining in the earth.", ch,
		obj, NULL, TO_ALL );
	    obj_to_char( obj, ch );
	    if ( IS_SET(ch->in_room->resources, RES_GEMS)
	    &&   number_bits(2) != 0 )
	    {
		REMOVE_BIT(ch->in_room->resources, RES_GEMS);
		num_gem--;
	    }
	    act( "$n dig$% up $p.", ch, obj, NULL, TO_ALL );
	    check_improve(ch, gsn_mining, TRUE, 4);
	    return;
	}
	act( "$n looks around, but doesn't find anything.", ch, NULL, NULL,
	    TO_ROOM );
	send_to_char( "You look around, but don't find anything.\n\r", ch );
	return;
    }

    send_to_char( "Search for herbs, lumber, gems, or ore?\n\r", ch );
    return;
}
