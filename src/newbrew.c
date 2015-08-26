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

int random_potion( );


/* Original Code by Todd Lair.                                        */
/* Improvements and Modification by Jason Huang (huangjac@netcom.com).*/
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void do_brew ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *result;
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *vial, *vobj, *vobj_next;
    int potion, skill, luck, i, poison = 0;
    bool iFound = FALSE;

    if ( !IS_NPC( ch )                                                  
	&& ch->level < skill_table[gsn_brew].skill_level[ch->class] )
    {                                          
	send_to_char( "You do not know how to brew things.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Brew what?\n\r", ch );
	return;
    }

    /* Do we have a vial to brew potions? */

    if ( (vial = get_eq_char( ch, WEAR_HOLD )) == NULL
    ||   vial->item_type != ITEM_VIAL )
    {
	send_to_char( "You must have a vial to brew things.\n\r", ch );
	return;
    }

    if ( (potion = potion_lookup( arg )) == 0 )
    {
	send_to_char( "You have never heard of such a mixture.\n\r", ch );
	return;
    }

    act( "$n begin$% working with some ingredients, mixing things.",
	ch, NULL, NULL, TO_ALL );
    WAIT_STATE( ch, UMAX(2, potion_table[potion].diff / 10) );

    skill = get_skill(ch, gsn_brew) - potion_table[potion].diff / 5;
    luck  = luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod / 2;

    skill = UMIN(skill + luck, 97);

    /* remove ingredients from inventory */
    for ( i = 0; i < 20; i++ )
    {
	int j;

	j = ( 1 << i );
	if ( potion_table[potion].ingredients & j )
	{
	    iFound = FALSE;
	    for ( vobj = ch->carrying; vobj; vobj = vobj->next_content )
	    {
		if ( vobj->item_type == ITEM_INGREDIENT
		&&   vobj->value[0]  == j ) 
		{
		    iFound = TRUE;
		    break;
		}
	    }
	    if ( iFound == FALSE )
	    {
		send_to_char( "You seem to be missing an ingredient.\n\r", ch );
		return;
	    }
	}
    }

    for ( i = 0; i < 20; i++ )
    {
	int j;

	j = ( 1 << i );
	if ( potion_table[potion].ingredients & j )
	{
	    iFound = FALSE;
	    for ( vobj = ch->carrying; vobj; vobj = vobj_next )
	    {
		vobj_next = vobj->next_content;

		if ( vobj->item_type == ITEM_INGREDIENT
		&&   vobj->value[0]  == j ) 
		{
		    act( "You add $p to the mixture.", ch, vobj, NULL,
			TO_CHAR );
		    if ( vobj->value[3] )
			poison++;
		    extract_obj( vobj );
		    break;
		}
	    }
	}
    }

    act( "$n finishes the mixture, looking at it discernly.", ch, NULL,
	NULL, TO_ROOM );
    act( "You finish the mixture, and look at it discernly.", ch, NULL,
	NULL, TO_CHAR );

    if ( number_percent() >= skill )
    {
	potion = random_potion( );
	
	extract_obj( vial );
	result = create_object( get_obj_index(4230), 1 );
	result->value[0] = potion;
	result->value[1] = number_range( 2, 5 );
	result->value[2] = result->value[1];
	result->item_type = ITEM_POTION;

	switch( potion_table[potion].type )
	{
	    default:
		break;
	    case POTION_DRINK:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "vial potion mixture" );
		result->short_descr = str_dup( "a vial of some mixture" );
		result->description = str_dup( "A glass vial filled with some liquid mixture is here." );
		break;	
	    case POTION_BALM:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "vial balm salve" );
		result->short_descr = str_dup( "a vial of balm" );
		result->description = str_dup( "A glass vial filled a balm is here." );
		break;	
	    case POTION_EAT:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "pills" );
		result->short_descr = str_dup( "some odd pills" );
		result->description = str_dup( "Some small and odd pills are lying here." );
		break;	
	}
    }
    else
    {
	extract_obj( vial );
	result = create_object( get_obj_index(4230), 1 );
	result->value[0] = potion;
	result->value[1] = number_range( 2, 5 );
	result->value[2] = result->value[1];
	result->item_type = ITEM_POTION;

	switch( potion_table[potion].type )
	{
	    default:
		break;
	    case POTION_DRINK:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "vial potion mixture" );
		result->short_descr = str_dup( "a vial of some mixture" );
		result->description = str_dup( "A glass vial filled with some liquid mixture is here." );
		break;	
	    case POTION_BALM:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "vial balm salve" );
		result->short_descr = str_dup( "a vial of balm" );
		result->description = str_dup( "A glass vial filled a balm is here." );
		break;	
	    case POTION_EAT:
		free_string( result->name );
		free_string( result->short_descr );
		free_string( result->description );

		result->name = str_dup( "pills" );
		result->short_descr = str_dup( "some odd pills" );
		result->description = str_dup( "Some small and odd pills are lying here." );
		break;	
	}
    }
    if ( number_bits(1) < poison )
	result->value[3] = TRUE;
    obj_to_char( result, ch );
    return;
}



int potion_lookup (const char *name)
{
    int potion;

    for ( potion = 0; potion_table[potion].name != NULL; potion++)
    {
        if (LOWER(name[0]) == LOWER(potion_table[potion].name[0])
        &&  !str_prefix( name, potion_table[potion].name))
            return( potion );
    }
    
    return 0;
}       



int random_potion( )
{
    int count, i = 0;

    for ( count = 0; potion_table[i].name != NULL; count++ )

    i = number_range( 1, count );
    return i;
}



char *potion_name( sh_int num )
{
    return( potion_table[num].name );
}



