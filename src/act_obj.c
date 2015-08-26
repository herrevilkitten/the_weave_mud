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
#include "mem.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_release	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_wake		);

void    raw_kill        args(( CHAR_DATA *victim, CHAR_DATA *ch, int dt ));

/*
 * Local functions.
 */
#define CD CHAR_DATA
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( ( CHAR_DATA *ch ) );
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
#undef	CD

void	figure_carrying	args( ( CHAR_DATA *ch ) );

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owned || obj->owned == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owned))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];
    int sn;

    sn = skill_lookup( "ward object" );

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }


    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            if (gch->on == obj)
            {
                act("$N appears to be using $p.",
                    ch,obj,gch,TO_CHAR);
                return;
            }  
    }   

    if ( container != NULL )
    {
    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE) && obj->timer)
	    obj->timer = 0;	
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	if ( !check_skill(ch, gsn_pilfer, 95, TRUE) )
	    act( "$n gets $p.", ch, obj, container, TO_ROOM );

	obj_from_room( obj );
    }

    if ( is_obj_affected(obj, sn) )
    {
	AFFECT_DATA *paf;
	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->type == sn
	    &&   paf->owner != ch )
	    {
		int strength;
		strength = paf->strength;
		act( "$p suddenly bursts into flame for a second.",
		    ch, obj, NULL, TO_ALL );
		spell_damage( ch, ch, strength, sn, DAM_FIRE );
		break;
	    }
	    paf->duration--;
	    paf->strength--;
	}
    }

    if ( IS_SET(obj->extra_flags, ITEM_DONATE_ROT) )
    {
	obj->timer = 0;
	REMOVE_BIT( obj->extra_flags, ITEM_DONATE_ROT );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->gold += obj->value[0];
        if (IS_SET(ch->act,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if ( is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && obj->value[0] > 1)
	  {
	    sprintf(buffer,"%d",obj->value[0]);
	    do_split(ch, buffer);	
	  }
        }
 
	figure_carrying( ch );
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	if ( !str_cmp(arg2, "from") )
	    argument = one_argument( argument, arg2 );

	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

/*
	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}
*/

	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] && container->value[0] != -1 )
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}
	
	if (container->pIndexData->vnum == OBJ_VNUM_PIT 
	&&  !CAN_WEAR(container,ITEM_TAKE)) {
	    if (obj->timer)
	    {
	    	send_to_char( "Only permanent items may go in the pit.\n\r",ch);
	    	return;
	    }
	    else
	        obj->timer = number_range(100,200);
        }

	obj_from_char( obj );
	obj_to_obj( obj, container );
	act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
/*	    &&   can_drop_obj( ch, obj ) */
	    &&   (get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] || container->value[0] == -1) )
	    {
	    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	    	&&  !CAN_WEAR(obj, ITEM_TAKE) ) {
	    	    if (obj->timer)
	    	        continue;
	    	    else
	    	    	obj->timer = number_range(100,200);
                }
		obj_from_char( obj );
		obj_to_obj( obj, container );
		act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	     str_cmp( arg, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that many coins.\n\r", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}

	obj_to_room( create_money( amount ), ch->in_room );
	act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	figure_carrying( ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}


	argument = one_argument( argument, arg2 );
	if ( !str_cmp(arg2, "to") )
	    argument = one_argument( argument, arg2 );

	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	sprintf(buf,"$n gives you %d gold.",amount);
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d gold.",amount);
	act( buf, ch, NULL, victim, TO_CHAR    );
	figure_carrying( ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( !str_cmp(arg2, "to") )
	argument = one_argument( argument, arg2 );

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( IS_NPC(victim)
    &&   !IS_SENTIENT(victim)
    &&   !IS_SET(victim->act, ACT_PET) )
    {
	send_to_char( "It really doesn't know what to do with it.\n\r", ch );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    if ( IS_NPC(victim) )
    {
	act( "$N look$^ at $n suspiciously, then hands $p back.",
	    ch, obj, victim, TO_ALL );
	return;
    }
    obj_from_char( obj );
    obj_to_char( obj, victim );
    return;
}



void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fobj;
    bool found;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    found = FALSE;
    if ( arg2[0] == '\0' )
    {
	for ( fobj = ch->in_room->contents; fobj != NULL; fobj = fobj->next_content )
	{
	    if ( fobj->item_type == ITEM_FOUNTAIN )
	    {
		found = TRUE;
		break;
	    }
	}

	if ( !found )
	{
	    send_to_char( "There is no fountain here.\n\r", ch );
	    return;
	}

	if ( obj->value[1] != 0 && obj->value[2] != 0 )
	{
	    send_to_char( "There's already another liquid in it.\n\r", ch );
	    return;
	}

	if ( obj->value[1] >= obj->value[0] )
	{
	    send_to_char( "Your container is already full.\n\r", ch );
	    return;
	}

	act( "You fill $p.", ch, obj, NULL, TO_CHAR );
	obj->value[2] = 0;
	obj->value[1] = obj->value[0];
	return;
    }

    fobj = get_obj_here( ch, arg2 );

    if ( fobj == NULL )
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    if ( fobj->item_type != ITEM_FOUNTAIN &&
	 fobj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You cannot fill your container from that.\n\r", ch );
	return;
    }

    if ( fobj->item_type == ITEM_FOUNTAIN )
    {
	if ( obj->value[1] != 0 && obj->value[2] != 0 )
	{
	    send_to_char( "There's already another liquid in it.\n\r", ch );
	    return;
        }
    }
    else
    {
	if ( obj->value[1] != 0 && obj->value[2] != fobj->value[2] )
	{
	    send_to_char( "There's already another liquid in it.\n\r", ch );
	    return;
        }

	if ( fobj->value[1] == 0 )
	{
	    act( "$p is empty.", ch, fobj, NULL, TO_CHAR );
	    return;
	}
    }

    if ( (obj->value[1] >= obj->value[0]) || obj->value[1] == -1 ||
	 fobj->value[0] == -1 )
    {
	send_to_char( "Your container is already full.\n\r", ch );
	return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    if ( fobj->item_type == ITEM_FOUNTAIN )
    {
	obj->value[1] = obj->value[0];
	obj->value[2] = 0;
    }
    else
    {
	obj->value[2] = fobj->value[2];
	if ( fobj->value[0] != -1 && fobj->value[1] != -1 )
	{
	    obj->value[1] = UMIN( obj->value[0],
			          (fobj->value[1] - obj->value[0]) );
	}
	else
	    obj->value[1] = obj->value[0];
    }
    return;
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( (obj = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You are not carrying that.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_FOUNTAIN &&
	 obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You cannot empty that.\n\r", ch );
	return;
    }

    act( "You empty $p.", ch, obj, NULL, TO_CHAR );
    obj->value[1] = 0;
    return;
}



void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = 48;
	act( "$n drinks from $p.", ch, obj, NULL, TO_ROOM );
	send_to_char( "You are no longer thirsty.\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 && obj->value[0] != -1 && 
	     obj->value[1] != -1 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	amount = number_range(3, 10);
	if ( obj->value[1] != -1 && obj->value[0] != -1 )
	    amount = UMIN(amount, obj->value[1]);
	
	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	    send_to_char( "You are full.\n\r", ch );
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );
	
	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned ! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );
	    af.type      = gsn_poison;
	    af.strength	 = number_fuzzy(amount); 
	    af.duration  = 3 * amount;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    af.bitvector_2 = 0;  
	    af.owner     = NULL;
	    af.flags     = AFFECT_NOTCHANNEL;
	    affect_join( ch, &af );
	}
	
	if ( obj->value[0] != -1 && obj->value[1] != -1 )
	    obj->value[1] -= amount;
	break;
    }

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.type      = gsn_poison;
	    af.strength  = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    af.bitvector_2 = 0;  
	    af.owner     = NULL;
	    af.flags     = AFFECT_NOTCHANNEL;
	    affect_join( ch, &af );
	}
	break;

    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if (get_eq_char (ch, WEAR_SECONDARY) != NULL
    &&  iWear == WEAR_WIELD )
    {
	act( "You must remove $p before you can wield a weapon in your primary hand.", ch,
	    get_eq_char(ch, WEAR_SECONDARY), NULL, TO_CHAR );
	return FALSE;
    }

    if ( ch->fighting != NULL
    &&   (iWear == WEAR_BODY
    ||	  iWear == WEAR_LEGS
    ||    iWear == WEAR_ARMS
    ||    iWear == WEAR_FEET) )
    {
	act( "You cannot remove $p in the heat of combat!", ch,
	    get_eq_char(ch, iWear), NULL, TO_CHAR );
	return FALSE;
    }

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    int count;
    count = hand_count( ch );
/*
    if ( !eq_is_free(obj, ch) )
    {
	send_to_char( "You are already wearing something there.\n\r", ch );
	return;
    }
*/
    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using two weapons.\n\r", ch );
            return;
        }

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS) )
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight( obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield )
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch) && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (get_eq_char(ch,WEAR_SHIELD) != NULL ||
	     get_eq_char(ch,WEAR_SECONDARY) != NULL) )
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
        else
            act("You don't even know which is end is up on $p.",
                ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	OBJ_DATA *weapon;

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot hold an item while using two weapons.\n\r", ch );
            return;
        }

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS) )
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( !str_cmp(argument, "all") )
    {
	bool found = FALSE;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
	    if ( obj->wear_loc > -1
	    &&   !IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	    {
		found = TRUE;
		unequip_char( ch, obj );
	    }
	}

	if (found != TRUE)
	    send_to_char( "You are not wearing anything.\n\r", ch );

	return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc == WEAR_WIELD && get_eq_char( ch, WEAR_SECONDARY))
    {
	send_to_char( "You must remove your secondary weapon before your primary weapon.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int gold;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to the Creator, who graciously declines.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "The Creator appreciates your offer and may accept it later.\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
	   send_to_char(
	     "The Creator wouldn't like that.\n\r",ch);
	   return;
        }
    }


    if ( !CAN_WEAR(obj, ITEM_TAKE))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            if (gch->on == obj)
            {
                act("$N appears to be using $p.",
                    ch,obj,gch,TO_CHAR);
                return;
            }
    }


    gold = UMAX(1,obj->level * 2);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	gold = UMIN(gold,obj->cost);

    if (gold == 1)
        send_to_char(
	    "The Creator gives you one gold coin for your sacrifice.\n\r", ch );
    else
    {
	sprintf(buf,"The Creator gives you %d gold coins for your sacrifice.\n\r",
		gold);
	send_to_char(buf,ch);
    }
    
    ch->gold += gold;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && gold > 1)
	{
	    sprintf(buffer,"%d",gold);
	    do_split(ch,buffer);	
	}
    }

    act( "$n sacrifices $p to the Creator.", ch, obj, NULL, TO_ROOM );
    wiznet("$N sends up $p as a burnt offering.",
           ch,obj,WIZ_SACCING,0,0);
    figure_carrying( ch );

    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    int value;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Use what?\n\r", ch );
	return;
    }

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You do not have that.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can use only medicinal potions, salves, and pills.\n\r", ch );
	return;
    }

    vch = ch;

    if ( arg2[0] != '\0' )
    {
	if ( (vch = get_char_room( ch, arg2 )) == NULL )
	{
	    send_to_char( "You don't see them here.\n\r", ch );
	    return;
	}

	if ( vch->position > POS_SLEEPING
	&&   !IS_AFFECTED(vch, AFF_WRAP)
	&&   !IS_AFFECTED_2(vch, AFF_CAPTURED) )
	{
	    act( "$N is alive and kicking and can take $s medicine by $mself.",
		ch, NULL, vch, TO_CHAR );
	    return;
	}
    }


    if ( ch == vch )
    {
	switch( potion_table[obj->value[0]].type )
	{
	    default:
		act( "$n quaff$% $p.", vch, obj, NULL, TO_ALL );
		break;
	    case POTION_DRINK:
		act( "$n quaff$% $p.", vch, obj, NULL, TO_ALL );
		break;
	    case POTION_BALM:
		act( "$n rub$% $p on $s body.", vch, obj, NULL, TO_ALL );
		break;
	    case POTION_EAT:
		act( "$n swallow$% $p.", vch, obj, NULL, TO_ALL );
		break;
	}
    }
    else
    {
	switch( potion_table[obj->value[0]].type )
	{
	    default:
		act( "$n force$% $N to quaff $p.", ch, obj, vch, TO_ALL );
		break;
	    case POTION_DRINK:
		act( "$n force$% $N to quaff $p.", ch, obj, vch, TO_ALL );
		break;
	    case POTION_BALM:
		act( "$n rubs $p on $O body.", ch, obj, vch, TO_ALL );
		break;
	    case POTION_EAT:
		act( "$n force$% $N to swallow $p.", ch, obj, vch, TO_ALL );
		break;
	}
    }

    value = potion_table[obj->value[0]].affect;
    if ( value & HERB_HEAL )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff,
			     2 * potion_table[obj->value[0]].diff );
	gain_health( vch, heal, FALSE );
	send_to_char( "You feel better.\n\r", vch );
    }

    if ( value & HERB_CURE_POISON )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff / 2,
			     potion_table[obj->value[0]].diff * 3 / 2 );
	cure_condition( vch, BODY_POISON, heal );
	send_to_char( "The burning in your veins goes away.\n\r", vch );
    }

    if ( value & HERB_CURE_DISEASE )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff / 2,
			     potion_table[obj->value[0]].diff * 3 / 2 );
	cure_condition( vch, BODY_DISEASE, heal );
	send_to_char( "You feel less nauseous.\n\r", vch );
    }

    if ( value & HERB_REFRESH )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff / 2,
			     potion_table[obj->value[0]].diff * 3 / 2 );
	gain_stamina( vch, heal, FALSE );
	send_to_char( "You feel less tired.\n\r", vch );
    }

    if ( value & HERB_SLEEP )
    {
	AFFECT_DATA af;
	int strength;

	strength = number_range( potion_table[obj->value[0]].diff / 2,
			         potion_table[obj->value[0]].diff * 3 / 2 );

	af.type		= skill_lookup( "sleep" );
	af.strength	= strength;
	af.duration	= strength / 10;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_SLEEP;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;
	affect_join( vch, &af );

	if ( IS_AWAKE(vch) )
	{
	    send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", vch );
	    act( "$n goes to sleep.", vch, NULL, NULL, TO_ROOM );
	    vch->position = POS_SLEEPING;
	}

	if ( IS_GRASPING(vch) )
	    do_release( vch, "" );
    }

    if ( value & HERB_HURT )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff,
			     2 * potion_table[obj->value[0]].diff );
	lose_health( vch, heal, TRUE );
	send_to_char( "This mixture burns!\n\r", vch );

	if ( ch->position == POS_DEAD )
	{
	    sprintf( log_buf, "%s took some bad medicine in %s [room %d]",
		(IS_NPC(vch) ? vch->short_descr : vch->name),
		vch->in_room->name, vch->in_room->vnum);

	    if (IS_NPC(vch))
		wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
	    else
		wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
             
	    if ( !IS_NPC(vch) )
	    {
		int loss;
		char buf[MAX_STRING_LENGTH];
                
		switch (vch->level)
		{
		    default:
			loss = -(exp_per_level(vch, vch->pcdata->points));
			break;
                    case 1:
                    case 2:
                        loss = 0;
                        break;
                    case 3:
                        loss = -100;
                        break;
                    case 4:   
                        loss = -200;
                        break;
                    case 5:
                        loss = -400;
                        break;
                    case 6:
                        loss = -800;
                        break;
                    case 7:   
                        loss = -1600;
                        break;
                }
                sprintf( buf, "You have lost %d experience points.\n\r",
                    loss * -1 );
                send_to_char( buf, vch );
                gain_exp( vch, loss );
            }
            raw_kill( vch, vch, DAM_POISON );
	}
    }

    if ( value & HERB_POISON )
    {
	AFFECT_DATA af;
	int strength;

	strength = number_range( potion_table[obj->value[0]].diff / 2,
			         potion_table[obj->value[0]].diff * 3 / 2 );

	af.type		= skill_lookup( "poison" );
	af.strength	= strength;
	af.duration	= 1 + strength / 3;
	af.location	= APPLY_STR;
	af.modifier	= (strength / 10) * -1;
	af.bitvector	= AFF_POISON;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;
	affect_join( vch, &af );

	send_to_char( "You are suddenly very ill.\n\r", vch );
	act( "$n is suddenly ill.", vch, NULL, NULL, TO_ROOM );
    }

    if ( value & HERB_DISEASE )
    {
	AFFECT_DATA af;
	int strength;

	strength = number_range( potion_table[obj->value[0]].diff / 2,
			         potion_table[obj->value[0]].diff * 3 / 2 );

	af.type		= skill_lookup( "poison" );
	af.strength	= strength;
	af.duration	= strength * 3 / 4;
	af.location	= APPLY_STR;
	af.modifier	= (strength / 6) * -1;
	af.bitvector	= AFF_PLAGUE;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;
	affect_join( vch, &af );

	act( "$n scream$% as sores break out over $s body.", vch, NULL,
	    NULL, TO_ALL );
    }

    if ( value & HERB_DRAIN )
    {
	int heal;

	heal = number_range( potion_table[obj->value[0]].diff / 2,
			     potion_table[obj->value[0]].diff * 3 / 2 );
	send_to_char( "You feel more tired now.\n\r", vch );
	lose_stamina( vch, heal, TRUE, TRUE );
    }

    if ( value & HERB_STOP_CHANNEL )
    {
	if ( can_channel(vch, 1)
	||   IS_AFFECTED_2(vch, AFF_STOP_CHANNEL) )
	{
	    AFFECT_DATA af;
	    int strength;

	    strength = number_range( potion_table[obj->value[0]].diff / 2,
			             potion_table[obj->value[0]].diff * 3 / 2 );

	    af.type		= skill_lookup( "stop channeling" );
	    af.strength		= strength;
	    af.duration		= 1 + strength / 4;
	    af.location		= APPLY_NONE;
	    af.modifier		= 0;
	    af.bitvector	= 0;
	    af.bitvector_2	= AFF_STOP_CHANNEL;
	    af.owner		= NULL;
	    af.flags		= AFFECT_NOTCHANNEL;
	    affect_to_char( vch, &af );

	    send_to_char( "You seem to have some trouble touching the True Source.\n\r", vch );
	    if ( IS_GRASPING(vch) )
		do_release( vch, "" );

	}
    }
	    


    if ( --obj->value[2] <= 0 )
	extract_obj( obj );
    else if ( obj->value[2] == 1 )
    {
	switch( potion_table[obj->value[0]].type )
	{
	    default:
		break;
	    case POTION_DRINK:
		free_string( obj->name );
		free_string( obj->short_descr );
		free_string( obj->description );

		obj->name = str_dup( "vial potion mixture" );
		obj->short_descr = str_dup( "an almost empty vial of some mixture" );
		obj->description = str_dup( "A glass vial filled with a tiny bit liquid mixture is here." );
		break;	
	    case POTION_BALM:
		free_string( obj->name );
		free_string( obj->short_descr );
		free_string( obj->description );

		obj->name = str_dup( "vial balm salve" );
		obj->short_descr = str_dup( "an almost empty vial of balm" );
		obj->description = str_dup( "A glass vial filled with only a tiny bit of balm is here." );
		break;
	    case POTION_EAT:
		free_string( obj->name );
		free_string( obj->short_descr );
		free_string( obj->description );

		obj->name = str_dup( "pill" );
		obj->short_descr = str_dup( "a single odd pill" );
		obj->description = str_dup( "A small and odd pill is lying here." );
		break;	
	}
    }
    return;
}




void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position == POS_FIGHTING)
    {
	send_to_char("You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );

    if ( ch->level + 5 < victim->level
    ||   victim->position == POS_FIGHTING
    ||   !IS_NPC(victim)
    || ( check_skill(ch, gsn_steal, 100 - percent, TRUE) ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }
	if ( !IS_AWAKE(victim) )
	    do_wake(victim, "" );
	if ( IS_AWAKE(victim) )
	    do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
                sprintf(buf,"$N tried to steal from %s.",victim->name);
                wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) )
    {
	int amount;

	amount = victim->gold * number_range(1, 10) / 100;
	if ( amount <= 0 )
	{
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
	sprintf( buf, "Bingo!  You got %d gold coins.\n\r", amount );
	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	figure_carrying( ch );
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   is_guild_eq(obj->pIndexData->vnum) )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper)
	&& (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	char buf[MAX_STRING_LENGTH];
	int time;
	time = pShop->open_hour - time_info.hour;
	sprintf( buf, "Sorry, I am closed.  Try coming back in %d hour%s.",
	    time, time != 1 ? "s" : "" );
	do_say( keeper, buf );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	char buf[MAX_STRING_LENGTH];
	int time;
	time = 23 - time_info.hour + pShop->open_hour;
	sprintf( buf, "Sorry, I am closed.  Try coming back in %d hour%s.",
	    time, time != 1 ? "s" : "" );
	do_say( keeper, buf );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
            {
                cost = 0;
                break;
            }
	}
    }

    if ( IS_SET(obj->extra_flags, ITEM_QUEST)
    ||   is_guild_eq(obj->pIndexData->vnum) )
	return 0;

    switch(obj->material)
    {
	default:
		break;
	case MAT_SILVER:
		cost = cost * 150 / 100;
		break;
	case MAT_GOLD:
		cost = cost * 225 / 100;
		break;
	case MAT_DARKSILVER:
		cost = cost * 500 / 100;
		break;
	case MAT_HEARTSTONE:
		cost = cost * 2000 / 100;
		break;
	case MAT_SAPPHIRE:
		cost = cost * 140 / 100;
		break;
	case MAT_EMERALD:
		cost = cost * 170 / 100;
		break;
	case MAT_RUBY:
		cost = cost * 140 / 100;
		break;
	case MAT_OPAL:
		cost = cost * 155 / 100;
		break;
	case MAT_SILK:
		cost = cost * 130 / 100;
		break;
	case MAT_GEM_OTHER:
		cost = cost * 125 / 100;
		break;
	case MAT_DIAMOND:
		cost = cost * 180 / 100;
		break;
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    int cost,roll;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 20 * pet->level * pet->level * (IS_SET( pet->act, ACT_MOUNT ) ? 3 : 1);

	if ( ch->gold < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );    
	    return;
	}

	/* haggle */
	roll = number_percent();
	if ( check_skill(ch, gsn_haggle, 100, TRUE) )
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}
    	if ( !IS_NPC(ch) && check_skill(ch, skill_lookup("appraising"), 75, FALSE) )
    	{
	    send_to_char( "Your appraising eye helps you greatly.\n\r", ch );
	    cost = cost * 85 / 100;
            check_improve( ch, skill_lookup("appraising"), TRUE, 3 );
    	}

	ch->gold		-= cost;
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(ch->act, PLR_BOUGHT_PET);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	figure_carrying( ch );
	return;
    }
    else if ( IS_SET(ch->in_room->room_flags, ROOM_QUEST_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA *keeper;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);
	if ( ( keeper = find_keeper( ch ) ) == NULL )
            return;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad quest shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	obj         = get_obj_list( ch, arg, ch->in_room->contents );
	ch->in_room = in_room;

	if ( obj == NULL || !IS_SET(obj->extra_flags, ITEM_QUEST) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

 	cost = obj->pIndexData->qp;

	if ( ch->pcdata->qp < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );    
	    return;
	}

	act( "$n tells you 'Enjoy $p.'",keeper, obj, ch, TO_VICT );
	ch->reply = keeper;
	ch->pcdata->qp		-= cost;
	obj			= create_object( obj->pIndexData, ch->level );
	SET_BIT(obj->extra_flags, ITEM_NODROP );
	SET_BIT(obj->extra_flags, ITEM_QUEST );

	obj_to_char( obj, ch );
	return;
    }
    else
    {
/*

===========================================================================
This snippet was written by Erwin S. Andreasen, 4u2@aabc.dk. You may use
this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://login.dknet.dk/~ea/

If you do not have WWW access, try ftp'ing to login.dknet.dk and examine
the /pub/ea directory.
===========================================================================


Multiple object buy routine
---------------------------

Last update:  Aug 20, 1995

Should work on : MERC2.2 ROM2.3

Fixed since last update:
 None

Know bugs and limitations yet to be fixed:
 None?

Comments:

 This change to do_buy allows the player to type e.g. buy 10 bread and get all
 the 10 pieces of bread at once.

 The code does check if the item is an item sold to the shopkeeper he only has
 one of.

For ROM 2.3, you have to uncomment a single line near the beginning.
*/

/* Insert this in the lower part of the do_buy routine, after the pet code */

	char arg[MAX_INPUT_LENGTH]; /* Uncomment for ROM 2.3 */

        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost;
        char arg2[MAX_INPUT_LENGTH]; /* 2nd argument */
        int item_count = 1;          /* default: buy only 1 item */

    argument = one_argument (argument, arg); /* get another argument, if any */
    argument = one_argument (argument, arg2); /* get another argument, if any */

    if (arg2[0]) /* more than one argument specified? then arg2[0] <> 0 */
    {
        /* check if first of the 2 args really IS a number */

        if (!is_number(arg))
        {
            send_to_char ("Syntax for BUY is: BUY [number] <item>\n\r\"number\" is an optional number of items to buy.\n\r",ch);
            return;
        }

        item_count = atoi (arg); /* first argument is the optional count */
	if ( item_count >= 30 )
	{
	    send_to_char( "Don't be silly.  Try to keep it under 30 items.\n\r", ch );
	    return;
	}
        strcpy (arg,arg2);       /* copy the item name to its right spot */
    }

    if ( (keeper = find_keeper( ch )) == NULL ) /* is there a shopkeeper here? */
            return;

/* find the pointer to the object */
        obj  = get_obj_carry( keeper, arg );
        cost = get_cost( keeper, obj, TRUE );

    if ( cost <= 0 || !can_see_obj( ch, obj ) ) /* cant buy what you cant see */
        {
            act( "$n tells you 'I don't sell that -- try 'list''.",keeper, NULL, ch, TO_VICT );
            ch->reply = keeper;
            return; 
        }

/* check for valid positive numeric value entered */
    if (item_count < 1 )
    {
	send_to_char ("You must buy at least one!\n\r",ch);
	return;
    }

/* can the character afford it ? */
    if ( ch->gold < (cost * item_count) )
    {
        if (item_count == 1) /* he only wanted to buy one */
        {
            act( "$n tells you 'You can't afford to buy $p'.",keeper, obj, ch, TO_VICT );
        }
        else
        {
            char buf[MAX_STRING_LENGTH]; /* temp buffer */
            if ( (ch->gold / cost) > 0) /* how many CAN he afford? */
                sprintf (buf, "$n tells you 'You can only afford %ld of those!", (ch->gold / cost));
            else /* not even a single one! what a bum! */
                sprintf (buf, "$n tells you '%s? You must be kidding - you can't even afford a single one, let alone %d!'",capitalize(obj->short_descr), item_count);

            act(buf,keeper, obj, ch, TO_VICT );
            ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
            return;
        }

        ch->reply = keeper;
        return;
    }

/* can the character carry more items? */
    if ( ch->carry_number + (get_obj_number(obj)*item_count) > can_carry_n( ch ) )
        {
            send_to_char( "You can't carry that many items.\n\r", ch );
            return;
        }

/* can the character carry more weight? */
    if ( ch->carry_weight + item_count*get_obj_weight(obj) > can_carry_w( ch ) )
        {
            send_to_char( "You can't carry that much weight.\n\r", ch );
            return;
        }

/* check for objects sold to the keeper */
    if ( (item_count > 1) && !IS_SET (obj->extra_flags,ITEM_INVENTORY))
    {
        act( "$n tells you 'Sorry - $p is something I have only one of'.",keeper, obj, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

/* change this to reflect multiple items bought */
    if (item_count == 1)
    {
        act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
        act( "You buy $p.", ch, obj, NULL, TO_CHAR );
    }
    else /* inform of multiple item purchase */
    {
        char buf[MAX_STRING_LENGTH]; /* temporary buffer */

/* "buys 5 * a piece of bread" seems to be the easiest and least gramatically incorerct solution. */
        sprintf (buf, "$n buys $p (x %d).", item_count);
        act (buf, ch, obj, NULL, TO_ROOM); /* to char self */
        sprintf (buf, "You buy $p (x %d).", item_count);
        act(buf, ch, obj, NULL, TO_CHAR ); /* to others */
    }

    ch->gold     -= cost*item_count;
    keeper->gold += cost*item_count;

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) /* 'permanent' item */
    {
        /* item_count of items */
        for ( ; item_count > 0; item_count--) /* create item_count objects */
        {
            obj = create_object( obj->pIndexData, obj->level );
            obj_to_char( obj, ch );
        }
    }
    else /* single item */
    {
        obj_from_char( obj );
        obj_to_char( obj, ch );
    }

    if ( cost * item_count >= number_range(3, 7) * number_range(20, 60)
    &&   keeper->memory == NULL )
    {
	act( "$n smile%^ at $N greedily and say$^ 'Come back anytime.'",
	    keeper, NULL, ch, TO_ALL );
	keeper->memory			= new_mem_data();
	keeper->memory->reaction	= MEM_CUSTOMER;
	keeper->memory->id		= ch->id;
	keeper->memory->when		= current_time;
    }

    figure_carrying( ch );
    return;
    } /* else */
} /* do_buy */




void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\n\r", ch );
		}
		sprintf( buf, "%8d - %s\n\r",
		    20 * pet->level * pet->level * 
		    (IS_SET( pet->act, ACT_MOUNT ) ? 3 : 1),
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return;
    }
    else if ( IS_SET(ch->in_room->room_flags, ROOM_QUEST_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	OBJ_DATA *obj;
	bool found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad quest shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( obj = pRoomIndexNext->contents; obj; obj = obj->next_content )
	{
	    if ( IS_SET(obj->extra_flags, ITEM_QUEST) )
	    {
		char arg[MAX_INPUT_LENGTH];

		one_argument( obj->name, arg );

		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Quest items for sale:\n\r", ch );
		}
		sprintf( buf, "%8d - %s (%s)\n\r",
		    obj->pIndexData->qp,
		    obj->short_descr,
		    arg );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of quest items right now.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( (keeper = find_keeper( ch )) == NULL )
	    return;

	if ( IS_SET(keeper->act, ACT_FIXER) )
	    send_to_char( "I charge 95 coins per unit of damage to repair items.\n\r\n\r", ch );

        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj(ch, obj)
	    &&   (cost = get_cost( keeper, obj, TRUE )) > 0 
	    &&   (arg[0] == '\0'  
 	    ||    is_name( arg,obj->name )) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Price] Item\n\r", ch );
		}

		sprintf( buf, "[%5d] %s.\n\r",
		    cost, obj->short_descr);
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    /* won't buy rotting goods */
    if ( obj->timer || ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if ( check_skill(ch, gsn_haggle, 100, TRUE) )
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
/*	cost = UMIN(cost,keeper->gold);*/
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    if ( !IS_NPC(ch) && check_skill(ch, skill_lookup("appraising"), 75, TRUE) )
    {
	send_to_char( "Your appraising eye helps you greatly.\n\r", ch );
	cost = cost * 115 / 100;
        cost = UMAX(cost,115 * get_cost(keeper,obj,TRUE) / 100);
/*	cost = UMIN(cost,keeper->gold);	*/
        check_improve( ch, skill_lookup("appraising"), TRUE, 3 );
    }
    sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost;
/*    keeper->gold -= cost;	Keepers have infinite hold :) */
    if ( keeper->gold < 0 )
	keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	obj->timer = number_range(50,100);
	obj_to_char( obj, keeper );
    }
    figure_carrying( ch );
    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( IS_SET(keeper->act, ACT_FIXER) )
    {
	if ( obj->condition < 100 && obj->condition >= 0 )
	{
	    cost = (100 - obj->condition) * 85;
	    sprintf( buf, "$n tells you 'I would repair $p for %d gold coins'.", cost );
	    act( buf, keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	}
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}


void do_mount( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Mount what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_SET(victim->act, ACT_MOUNT) )
    {
	send_to_char( "You cannot mount that.\n\r", ch );
	return;
    }

    if ( victim->rider != NULL )
    {
	act( "$N is already being mounted by someone.", ch, NULL, victim, 
	    TO_CHAR );
	return;
    }

    if ( ch->pet != victim )
    {
	send_to_char( "It's owner may not appreciate that.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Trying to mount yourself really isn't too effective.\n\r", ch );
        return;
    }

    ch->mount = victim;
    victim->rider = ch;
    ch->position = POS_MOUNTED;
    act( "$n mounts $N.", ch, NULL, victim, TO_ROOM );
    act( "You mount $N.", ch, NULL, victim, TO_CHAR );
}

void do_dismount( CHAR_DATA *ch, char *argument )
{
    if ( ch->mount == NULL )
    {
	send_to_char( "You're not mounting anything right now.\n\r", ch );
	return;
    }

    act( "$n dismounts $N.", ch, NULL, ch->mount, TO_ROOM );
    act( "You dismount $N.", ch, NULL, ch->mount, TO_CHAR );
    ch->mount->rider = NULL;
    ch->mount = NULL;
    ch->position = POS_STANDING;
}

void do_use( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    argument = one_argument( argument, arg );

    if ( (obj = get_obj_here( ch, arg )) == NULL )
    {
	send_to_char( "You have don't see that here.\n\r", ch );
	return;
    }

    if ( obj->pIndexData->use_fun == NULL )
    {
	send_to_char( "You have no idea how to use that.\n\r", ch );
	return;
    }

    (*obj->pIndexData->use_fun)( obj, ch, argument );
    return;
}


void do_donate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *pit;
    ROOM_INDEX_DATA *pit_room;
    bool found = FALSE;
    int gold = 0;
    int vnum;
    int pit_vnum;

    one_argument( argument, arg );

    if ( (obj = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You are not carrying that.\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum >= 4600 && ch->in_room->vnum <= 4999 )
    {
	vnum		= 4999;
	pit_vnum	= 4701;
    }
    else
    {
	vnum		= 674;
	pit_vnum	= OBJ_VNUM_PIT;
    }

    if ( (pit_room = get_room_index( vnum )) == NULL )
    {
	send_to_char( "The donation pit has disappeared .. \n\r", ch );
	return;
    }

    for ( pit = pit_room->contents; pit; pit = pit->next_content )
    {
	if ( pit->pIndexData->vnum == pit_vnum )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "The donation chest has disappeared .. \n\r", ch );
	return;
    }

    if ( is_guild_eq(obj->pIndexData->vnum)
    ||   IS_SET(obj->extra_flags, ITEM_QUEST)
    ||   obj->item_type == ITEM_CORPSE_PC
    ||   obj->item_type == ITEM_CORPSE_NPC )
    {
	send_to_char( "You cannot donate this.\n\r", ch );
	return;
    }

    if ( IS_SET(obj->extra_flags, ITEM_NODROP)
    ||   IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	send_to_char( "You cannot let go of that!\n\r", ch );
	return;
    }

    if ( !IS_SET(obj->extra_flags, ITEM_DONATE) )
    {
	act( "$n donates $p, and is rewarded.", ch, obj, NULL, TO_ROOM );
	gold = UMAX(1,obj->level / 2);

	gold = UMIN(gold,obj->cost);

	if (gold == 1)
            send_to_char(
		"The Creator gives you one gold coin for your donation.\n\r",
		ch );
	else
	{
	    sprintf(buf,"The Creator gives you %d gold coins for your donation.\n\r",
		gold);
	    send_to_char(buf,ch);
	}
    
	ch->gold += gold;
	SET_BIT( obj->extra_flags, ITEM_DONATE );
    }
    else
	act( "$n donates $p.", ch, obj, NULL, TO_ROOM );

    if ( pit_room->people != NULL )
	act( "$p glows brightly for a second.", pit_room->people, pit,
	    NULL, TO_ALL );

    obj_from_char( obj );
    obj_to_obj( obj, pit );
    return;
}

void do_balance ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    sprintf( buf, "You have %ld coins in the bank.\n\r", ch->bank );
    send_to_char( buf, ch );
    return;
}

void do_deposit ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
        return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
        sprintf( buf, "But you are not in a bank.\n\r" );
        send_to_char( buf, ch );
        return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
        if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
            break;
    }

    if ( !banker )
    {
        sprintf( buf, "The banker is currently not available.\n\r" );
        send_to_char( buf, ch );
        return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        sprintf( buf, "How much gold do you wish to deposit?\n\r" );
        send_to_char( buf, ch );
        return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->gold + 1) )
    {
        sprintf( buf, "%s, you do not have %d gold coins.", ch->name, amnt );
        do_say( banker, buf );
        return;
    }

    ch->bank += amnt;
    ch->gold -= amnt;
    sprintf( buf, "%s, your account now contains %ld coins, after depositing %d coins.", PERS(ch, banker), ch->bank, amnt );
    do_say( banker, buf );
    figure_carrying( ch );
    return;
}

void do_withdraw ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
        return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
        sprintf( buf, "But you are not in a bank.\n\r" );
        send_to_char( buf, ch );
        return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
        if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
            break;
    }

    if ( !banker )
    {
         sprintf( buf, "The banker is currently not available.\n\r" );
         send_to_char( buf, ch );
         return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
         sprintf( buf, "How much gold do you wish to withdraw?\n\r" );
         send_to_char( buf, ch );
         return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->bank + 1) )
    {
        sprintf( buf, "%s, you do not have %d gold coins in the bank.", ch->name, amnt );
        do_say( banker, buf );
        return;
    }

    ch->gold += amnt;
    ch->bank -= amnt;
    sprintf( buf, "%s, your account now contains %ld coins, after withdrawing %d coins.", PERS(ch, banker), ch->bank, amnt );
    do_say( banker, buf );
    figure_carrying( ch );
    return;
}

void do_toss( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->gold < 1 )
    {
	send_to_char( "You must have at least one coin to toss.\r\n", ch );
	return;
    }

    act( "$n flip$% a coin high into the air.", ch, NULL, NULL, TO_ALL );
    sprintf( buf, "The coin comes down with %s showing.",
	number_bits(1) == 0 ? "heads" : "tails" );
    act( buf, ch, NULL, NULL, TO_ALL );
    return;
};
