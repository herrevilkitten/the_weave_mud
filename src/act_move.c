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
#include "merc.h"
#include "mem.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_release	);
DECLARE_DO_FUN(do_stand		);

int     count_users     args( (OBJ_DATA *obj) );


char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 4, 4
};

const	int	door_flag	[]		=
{
    1, 2, 4, 8, 16, 32
};



/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void	add_explore( );

bool match_flag( CHAR_DATA *ch, CHAR_DATA *guard, int flag )
{
    if ( IS_SET(flag, GUARD_GUILD)
    &&   ch->guild == guard->guild )
	return TRUE;

    if ( IS_SET(flag, GUARD_RACE)
    &&   ch->race == guard->race )
	return TRUE;

    if ( IS_SET(flag, GUARD_CLASS)
    &&   ch->class == guard->class )
	return TRUE;

    if ( IS_SET(flag, GUARD_VNUM)
    &&   IS_NPC(ch)
    &&   ch->pIndexData->vnum == guard->pIndexData->vnum )
	return TRUE;

    if ( IS_SET(flag, GUARD_IMMORTAL)
    &&   IS_IMMORTAL(ch) )
	return TRUE;

    if ( IS_SET(flag, GUARD_NEWBIE)
    &&   ch->level <= 5 )
	return TRUE;

    return FALSE;
}

bool check_exit_guard( CHAR_DATA *ch, CHAR_DATA *guard, int door )
{
    GUARD_DATA *gd;
    int dir;

    dir = door_flag[door];

    for ( gd = guard->pIndexData->guard; gd; gd = gd->next )
    {
	ROOM_INDEX_DATA *pRoom;
	TEXT_DATA *text;

	if ( !IS_SET(gd->exit, dir) )
	    continue;

	if ( !IS_SET(gd->flag, GUARD_EXIT) )
	    continue;

	if ( IS_SET(gd->flag, GUARD_DENY)
	&&   !match_flag(ch, guard, gd->allow) )
	{
	    act( "$N block$^ $o way.", ch, NULL, guard, TO_ALL );
	    return TRUE;
	}

	if ( IS_SET(gd->flag, GUARD_ATTACK)
	&&   !match_flag(ch, guard, gd->allow) )
	{
	    act( "$N block$^ $o way, then attack$^!", ch,
		NULL, guard, TO_ALL );
	    multi_hit( guard, ch, TYPE_UNDEFINED );
	    return TRUE;
	}

	if ( IS_SET(gd->flag, GUARD_MOVE)
	&&   !match_flag(ch, guard, gd->allow)
	&&   (pRoom = get_room_index( gd->room )) != NULL )
	{
	    act( "$N move$^ $n to another place.", ch, NULL, guard,
		TO_ALL );
	    return TRUE;
	}

	if ( IS_SET(gd->flag, GUARD_MESSAGE)
	&&   match_flag(ch, guard, gd->allow)
	&&   (text = get_text_index( gd->message )) != NULL )
	{
	    act( "$n say$% '$t'", guard, text->text, NULL, TO_ALL );
	    return FALSE;
	}
    }
    return FALSE;
}

bool check_entrance_guard( CHAR_DATA *ch, CHAR_DATA *guard, int door )
{
    GUARD_DATA *gd;
    int dir;

    dir = door_flag[door];

    for ( gd = guard->pIndexData->guard; gd; gd = gd->next )
    {
	TEXT_DATA *text;

	if ( !IS_SET(gd->exit, dir) )
	    continue;
	if ( !IS_SET(gd->flag, GUARD_ENTRANCE) )
	    continue;

	if ( IS_SET(gd->flag, GUARD_ATTACK)
	&&   !match_flag(ch, guard, gd->allow) )
	{
	    act( "$N attack$^ $o!", ch, NULL, guard, TO_ALL );
	    multi_hit( guard, ch, TYPE_UNDEFINED );
	    return TRUE;
	}

	if ( IS_SET(gd->flag, GUARD_MESSAGE)
	&&   match_flag(ch, guard, gd->allow)
	&&   (text = get_text_index( gd->message )) != NULL )
	{
	    act( "$n say$% '$t'", guard, text->text, NULL, TO_ALL );
	    return FALSE;
	}
    }
    return FALSE;
}

void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *guard;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    AFFECT_DATA *paf, *paf_next;
    EXIT_DATA *pexit;
    DESCRIPTOR_DATA *d;
    int count_p = 0;

    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CONFUSED) )
	door = number_range(0, 5);

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||	 !can_see_room(ch,pexit->u1.to_room))
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	return;
    }

    if ( CHARM_SET(ch)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room
    &&   (!IS_NPC(ch->master) && ch->master->desc != NULL) )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room )&&(ch->level != IMPLEMENTOR) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }


    /* Check for guardian mobs */
    if ( !IS_NPC(ch) )
    {
	for ( guard = ch->in_room->people; guard; guard = guard->next_in_room )
 	{
	    if ( guard == ch )
		continue;
	    if ( !IS_NPC(guard) )
		continue;

	    if ( guard->memory
	    &&   IS_SET(guard->memory->reaction, MEM_CUSTOMER) )
	    {
		act( "$N grin$^. 'Ahhhh.. $n, my friend, come, buy many, many things.'",
		    ch, NULL, guard, TO_ALL );
		free_mem_data(guard->memory);
		guard->memory = NULL;
	    }

	    if ( guard->pIndexData->guard == NULL )
		continue;

	    if ( check_exit_guard( ch, guard, door) )
		return;
	}
    }

    for ( paf = in_room->affected; paf; paf = paf->next )
    {
	if ( paf->type == gsn_wind_barrier )
	{
	    if ( !check_stat(ch, STAT_STR, -12) )
	    {
		act( "$n tries to leave $twards, but is held back by a wall of air.",
		    ch, dir_name[door], NULL, TO_ROOM );
		send_to_char_new( ch, "You try to leave %swards, but are held back a wall of air.\n\r", dir_name[door] );
		return;
	    }
	    act( "$n struggle$% with fierce winds, but finally make$% it through.",
		ch, NULL, NULL, TO_ALL );
	}
	if ( paf->type == gsn_earth_barrier )
	{
	    act( "$n tries to leave $twards, but is blocked by piles of earth.",
		ch, dir_name[door], NULL, TO_ROOM );
	    send_to_char_new( ch, "You try to leave %swards, but are blocked by piles of earth.\n\r", dir_name[door] );
	    return;
	}
	if ( paf->type == gsn_fire_wall && paf->modifier == door )
	{
	    act( "$n scream$% in pain as $e $i burned by flames.", ch,
		NULL, NULL, TO_ALL );
	    if ( spell_damage(ch, ch, dice(1, 10), gsn_fire_wall, DAM_FIRE) )
		return;
	}
	if ( paf->type == gsn_ice_wall && paf->modifier == door )
	{
	    act( "$n $i blocked by a large wall of ice.", ch, NULL, NULL,
		TO_ALL );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->position != POS_MOUNTED )
    {
	int move;

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
	    {
		send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

        if (( in_room->sector_type == SECT_WATER_SWIM  
        ||    to_room->sector_type == SECT_WATER_SWIM )
        &&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	/* Check for skill and boat */
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }

	    if ( !found && !check_skill(ch, gsn_swimming, 100, TRUE) )
	    {
		send_to_char( "You gasp for breath as you flounder around in the water.\n\r", ch );
		move += dice( 1, 5 ) + 5;
	    }
	}


	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

	if ( in_room->sector_type == SECT_FOREST
	&&   to_room->sector_type == SECT_FOREST )
	{
	    if ( check_skill(ch, gsn_forestry, 75, TRUE) )
	    {
		move /= 2;
		check_improve(ch, gsn_forestry, TRUE, 2);
	    }
	    else
		check_improve(ch, gsn_forestry, FALSE, 3);
	}

        move /= 2;  /* i.e. the average */
	if ( ch->guild == guild_lookup("warder") )
	    move /= 2;

	if ( check_skill(ch, gsn_hardiness, 50, TRUE) )
	    move = move * 2 / 3;

	move = UMAX(1, move);
	if ( IS_SET(ch->body, BODY_RIGHT_LEG) )
	{
	    move *= 3;
	    lose_health(ch, 1, TRUE);
	    act( "$n scream$% in pain as $e drag$% $o right leg along the ground.",
		ch, NULL, NULL, TO_ALL );
	}
	if ( IS_SET(ch->body, BODY_LEFT_LEG) )
	{
	    move *= 3;
	    lose_health(ch, 1, TRUE);
	    act( "$n scream$% in pain as $e drag$% $o left leg along the ground.",
		ch, NULL, NULL, TO_ALL );
	}

	if ( !lose_stamina(ch, move, TRUE, FALSE) && ch->fighting  != NULL )
	{
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

	WAIT_STATE( ch, 1 );
    }
    else if ( !IS_NPC(ch) && ch->position == POS_MOUNTED )
    {
	int move;
	CHAR_DATA *mount;

	if ( ch->mount == NULL )
	{
	    bug( "Move_char: Invalid mount.", 0 );
	    return;
	}

	mount = ch->mount;

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(mount, AFF_FLYING) && !IS_IMMORTAL(ch))
	    {
		send_to_char( "Your mount can't fly.\n\r", ch );
		return;
	    }
	}

        if (( in_room->sector_type == SECT_WATER_SWIM  
        ||    to_room->sector_type == SECT_WATER_SWIM )
        &&    !IS_AFFECTED(mount,AFF_FLYING))
	{
	/* Check for skill and boat */
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = mount->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }

	    if ( !found && !check_skill(ch, gsn_swimming, 100, TRUE) )
	    {
		send_to_char( "Your mount gasps for breath as it flounders around in the water.\n\r", ch );
		move += dice( 1, 5 ) + 5;
	    }
	}


	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(mount,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = mount->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char( "Your mount needs a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move = move / 2 +
	       dice( 2, 3 ) +
	       UMAX( 10, 100 / (get_skill( ch, gsn_riding ) + 1) );
	check_skill(ch, gsn_riding, 95, FALSE);

	move = UMAX(move, 1);
	if ( IS_SET(ch->body, BODY_RIGHT_LEG) )
	{
	    move *= 3;
	    lose_health(ch, 1, TRUE);
	    act( "$n scream$% in pain as $e drag$% $o right leg along the ground.",
		ch, NULL, NULL, TO_ALL );
	}
	if ( IS_SET(ch->body, BODY_LEFT_LEG) )
	{
	    move *= 3;
	    lose_health(ch, 1, TRUE);
	    act( "$n scream$% in pain as $e drag$% $o left leg along the ground.",
		ch, NULL, NULL, TO_ALL );
	}

	if ( !lose_stamina(mount, move, TRUE, FALSE) )
	{
	    send_to_char( "Your mount is too exhausted.\n\r", ch );
	    return;
	}

	WAIT_STATE( ch, 1 );
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
	if ( ch->mount != NULL && ch->position == POS_MOUNTED &&
	     ch->in_room == ch->mount->in_room )
	    act( "$n rides $T.", ch, NULL, dir_name[door], TO_ROOM );
	else if ( ch->mount == NULL && IS_AFFECTED(ch, AFF_FLYING) )
	    act( "$n flies $T.", ch, NULL, dir_name[door], TO_ROOM );
	else if ( ch->mount == NULL && (in_room->sector_type == SECT_WATER_SWIM
        ||    to_room->sector_type == SECT_WATER_SWIM) )
	    act( "$n swims $T.", ch, NULL, dir_name[door], TO_ROOM );
	else
	    act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
    }

    char_from_room( ch );
    char_to_room( ch, to_room );

/*    if ( !IS_NPC(ch)
    &&   !IS_IMMORTAL(ch)
    &&   IS_SET(ch->in_room->room_flags, ROOM_EXPLORE) )
    {
	int xp;
	REMOVE_BIT(ch->in_room->room_flags, ROOM_EXPLORE);
	for ( d = descriptor_list; d; d = d->next )
	    if ( d->connected == CON_PLAYING )
		count_p++;
	num_explore--;
	xp = count_p * 2 / UMAX(1, ch->in_room->area->nplayer);
	send_to_char( "Your explorations have been experiencing.\n\r", ch );
	gain_exp( ch, number_range(xp * 3, xp * 6) );
	add_explore( );
    }
*/
    for ( paf = to_room->affected; paf; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->type == gsn_set_snare )
	{
	    paf->strength -= number_range(1, 3);
	    if ( number_percent() > paf->strength )
	    {
		paf->duration = 0;
		continue;
	    }

	    if ( !check_stat(ch, STAT_DEX, -12) )
	    {
		int dam;
		dam = number_range( paf->strength / 4, paf->strength / 2 );
		if ( number_percent() > paf->strength )
		    paf->duration = UMAX(0, paf->duration - 1);
		if ( spell_damage(ch, ch, dam, gsn_set_snare, DAM_HARM) )
		    return;
	    }
	}

	if ( paf->type == gsn_wind_barrier )
	{
	    if ( !check_stat(ch, STAT_STR, -12) )
	    {
		char_from_room( ch );
		char_to_room( ch, in_room );
		act( "$n tries to leave $twards, but is held back by a wall of air.",
		    ch, dir_name[door], NULL, TO_ROOM );
		send_to_char_new( ch, "You try to leave %swards, but are held back a wall of air.\n\r", dir_name[door] );
		return;
	    }
	    act( "$n struggle$% with fierce winds, but finally make$% it through.",
		ch, NULL, NULL, TO_ALL );
	}
	if ( paf->type == gsn_earth_barrier )
	{
	    char_from_room( ch );
	    char_to_room( ch, in_room );
	    act( "$n tries to leave $twards, but is blocked by piles of earth.",
		ch, dir_name[door], NULL, TO_ROOM );
	    send_to_char_new( ch, "You try to leave %swards, but are blocked by piles of earth.\n\r", dir_name[door] );
	    return;
	}
	if ( paf->type == gsn_fire_wall && paf->modifier == rev_dir[door] )
	{
	    act( "$n scream$% in pain as $e $i burned by flames.", ch,
		NULL, NULL, TO_ALL );
	    if ( spell_damage(ch, ch, dice(1, 10), gsn_fire_wall, DAM_FIRE) )
		return;
	}
	if ( paf->type == gsn_ice_wall && paf->modifier == rev_dir[door] )
	{
	    act( "$n $i blocked by a large wall of ice.", ch, NULL, NULL,
		TO_ALL );
	    return;
	}
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
	if ( ch->mount != NULL && ch->position == POS_MOUNTED &&
	     ch->in_room == ch->mount->in_room )
	    act( "$n rides in.", ch, NULL, NULL, TO_ROOM );
	else if ( ch->mount == NULL && IS_AFFECTED(ch, AFF_FLYING) )
	    act( "$n flies in.", ch, NULL, NULL, TO_ROOM );
	else if ( ch->mount == NULL && (in_room->sector_type == SECT_WATER_SWIM
        ||    to_room->sector_type == SECT_WATER_SWIM) )
	    act( "$n swims in.", ch, NULL, NULL, TO_ROOM );
	else
	    act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Inviso moves ... not.  ( Same with Shapeshifted -- Joker )
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
        affect_strip( ch, gsn_invis );
        affect_strip( ch, gsn_mass_invis );
        REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
        act( "$n snaps into existence.", ch, NULL, NULL, TO_ROOM );
    }

    do_look( ch, "moved" );

    /* Check for guardian mobs */
    if ( !IS_NPC(ch) )
    {
	for ( guard = ch->in_room->people; guard; guard = guard->next_in_room )
 	{
	    if ( guard == ch )
		continue;
	    if ( !IS_NPC(guard) )
		continue;
	    if ( guard->pIndexData->guard == NULL )
		continue;

	    if ( check_entrance_guard( ch, guard, door) )
		return;
	}
    }

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch
	&&   CHARM_SET(fch)
	&&   fch->position < POS_STANDING
	&&   !IS_AFFECTED(fch, AFF_WRAP) )
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING
	&&   !IS_AFFECTED(fch, AFF_WRAP) )
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		return;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, door, TRUE );
	}
    }

    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_CAPTURED) )
    {
	send_to_char( "You can't go anywhere all tied up.\n\r", ch );
	return;
    }

    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}


int find_exit( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
	return -1;

    return door;
}


int find_spell_door( ROOM_INDEX_DATA *pRoom, char *arg, int sn )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    AFFECT_DATA *paf;
    int door, return_door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) return_door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) return_door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) return_door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) return_door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) return_door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) return_door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = pRoom->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword )
	    &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
		return_door = door;
	}
	return -1;
    }

    for ( paf = pRoom->affected; paf; paf = paf->next )
    {
	if ( paf->type == gsn_earth_barrier )
	{
	    if ( number_percent() >= 45
 	    &&   sn == skill_lookup("earth wave") )
		affect_room_remove( pRoom, paf );
	    return -1;
	}

	if ( paf->type == gsn_wind_barrier
	&&   number_percent() >= 90 )
	    return -1;

	if ( paf->type == gsn_ice_wall
	&&   paf->modifier == return_door )
	{
	    if ( number_percent() >= 5
	    &&   sn == skill_lookup("flame wave") )
		affect_room_remove( pRoom, paf );
	    return -1;
	}

	if ( paf->type == gsn_fire_wall
	&&   paf->modifier == return_door
	&&   sn == skill_lookup("frost wave") )
	{
	    if ( number_percent() >= 20 )
		affect_room_remove( pRoom, paf );
	    return -1;
	}
    }

    if ( (pexit = pRoom->exit[return_door]) == NULL )
	return -1;

    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	return -1;

    if ( (to_room = pexit->u1.to_room) == NULL )
	return -1;

    for ( paf = to_room->affected; paf; paf = paf->next )
    {
	if ( paf->type == gsn_earth_barrier )
	{
	    if ( number_percent() >= 45
 	    &&   sn == skill_lookup("earth wave") )
		affect_room_remove( to_room, paf );
	    return -1;
	}

	if ( paf->type == gsn_wind_barrier
	&&   number_percent() >= 90 )
	    return -1;

	if ( paf->type == gsn_ice_wall
	&&   paf->modifier == rev_dir[return_door] )
	{
	    if ( number_percent() >= 5
	    &&   sn == skill_lookup("flame wave") )
		affect_room_remove( to_room, paf );
	    return -1;
	}

	if ( paf->type == gsn_fire_wall
	&&   paf->modifier == rev_dir[return_door]
	&&   sn == skill_lookup("frost wave") )
	{
	    if ( number_percent() >= 20 )
		affect_room_remove( to_room, paf );
	    return -1;
	}
    }

    if ( IS_SET(to_room->room_flags, ROOM_SAFE) )
	return -1;

    return return_door;
}

int get_random_door( ROOM_INDEX_DATA *pRoom )
{
    EXIT_DATA *pexit;
    int door;

    door = number_range( 0, 5 );

    if ( ( pexit = pRoom->exit[door] ) == NULL )
	return -1;

    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	return -1;

    return door;
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;
    bool pDoor = FALSE;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( arg2[0] != '\0' && str_cmp(arg2, "door") )
	pDoor = TRUE;

    if ( (obj = get_obj_here( ch, arg )) != NULL && !pDoor )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( pDoor )
	strcpy( arg, arg2 );

    if ( (door = find_door( ch, arg )) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
 	    return;
	}
    }

    if ( !check_skill(ch, gsn_pick_lock, 100, TRUE) )
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	check_improve(ch,gsn_pick_lock,TRUE,2);
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}

void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
    char arg[MAX_INPUT_LENGTH];

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if (arg[0] != '\0')
    {
        if (ch->position == POS_FIGHTING)
        {
            send_to_char("Maybe you should finish fighting first?\n\r",ch);
            return;
        }
        obj = get_obj_list(ch,arg,ch->in_room->contents);
        if (obj == NULL)
        {
            send_to_char("You don't see that here.\n\r",ch);
            return;
        }   
        if (obj->item_type != ITEM_FURNITURE  
        ||  (!IS_SET(obj->value[2],STAND_AT)
        &&   !IS_SET(obj->value[2],STAND_ON)
        &&   !IS_SET(obj->value[2],STAND_IN)))
        {
            send_to_char("You can't seem to find a place to stand.\n\r",ch);
            return;
        }
        if (ch->on != obj && count_users(obj) >= obj->value[0])
        {
            act_new("There's no room to stand on $p.",
                ch,obj,NULL,TO_CHAR,POS_DEAD);
            return;
        }
        ch->on = obj;
    }
            
    switch ( ch->position )
    {
    case POS_SLEEPING:
        if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "You can't wake up!\n\r", ch ); return; }
            
        if (obj == NULL)
        {
            send_to_char( "You wake and stand up.\n\r", ch );  
            act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {
           act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
           act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
        }   
        else if (IS_SET(obj->value[2],STAND_ON))
        {
            act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_STANDING;
        do_look(ch,"moved");
        break;

    case POS_RECLINING:
        if (obj == NULL)
        {
            act( "$n stand$% up.", ch, NULL, NULL, TO_ALL );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {
           act("$n stand$% at $p.",ch,obj,NULL,TO_ALL);
        }   
        else if (IS_SET(obj->value[2],STAND_ON))
        {
            act("$n stand$% on $p.",ch,obj,NULL,TO_ALL);
        }
        else
        {
            act("$n stand$% in $p.",ch,obj,NULL,TO_ALL);
        }
        ch->position = POS_STANDING;
	break;

    case POS_RESTING: case POS_SITTING:
        if (obj == NULL)
        {   
            send_to_char( "You stand up.\n\r", ch );
            act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {   
            act("You stand at $p.",ch,obj,NULL,TO_CHAR);
            act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],STAND_ON))
        {
            act("You stand on $p.",ch,obj,NULL,TO_CHAR);
            act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act("You stand in $p.",ch,obj,NULL,TO_CHAR);
            act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_STANDING;
        break;
        
    case POS_STANDING:
        send_to_char( "You are already standing.\n\r", ch );
        break;
         
    case POS_FIGHTING:
        send_to_char( "You are already fighting!\n\r", ch );
        break;
    }
         
    return; 
}
            
            
         
void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
        send_to_char("You are already fighting!\n\r",ch);
        return;
    }
        
    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
        obj = get_obj_list(ch,argument,ch->in_room->contents);
        if (obj == NULL)
        {   
            send_to_char("You don't see that here.\n\r",ch);
            return;
        }
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (!IS_SET(obj->item_type,ITEM_FURNITURE)
        ||  (!IS_SET(obj->value[2],REST_ON)
        &&   !IS_SET(obj->value[2],REST_IN)
        &&   !IS_SET(obj->value[2],REST_AT)))
        {
            send_to_char("You can't rest on that.\n\r",ch);
            return;
        }
            
        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
            act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            return;
        }

        ch->on = obj;
    }
        
    switch ( ch->position )
    {
    case POS_SLEEPING:
        if (IS_AFFECTED(ch,AFF_SLEEP))
        {
            send_to_char("You can't wake up!\n\r",ch);
            return;
        }   
        
        if (obj == NULL)
        {
            send_to_char( "You wake up and start resting.\n\r", ch );
            act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_AT))
        {
            act_new("You wake up and rest at $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_RESTING;
        break;

    case POS_RECLINING:
        if (obj == NULL)
        {
            act( "$n rest$% up.", ch, NULL, NULL, TO_ALL );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {
           act("$n rest$% at $p.",ch,obj,NULL,TO_ALL);
        }   
        else if (IS_SET(obj->value[2],STAND_ON))
        {
            act("$n rest$% on $p.",ch,obj,NULL,TO_ALL);
        }
        else
        {
            act("$n rest$% in $p.",ch,obj,NULL,TO_ALL);
        }
        ch->position = POS_RESTING;
	break;
                    
    case POS_RESTING:
        send_to_char( "You are already resting.\n\r", ch );
        break;
         
    case POS_STANDING:
        if (obj == NULL)
        {
            send_to_char( "You rest.\n\r", ch );
            act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
        }
        else if (IS_SET(obj->value[2],REST_AT))   
        {
            act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
            act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {           
            act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
            act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act("You rest in $p.",ch,obj,NULL,TO_CHAR);
            act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_RESTING;
        break;
        
    case POS_SITTING:
        if (obj == NULL)
        {
            send_to_char("You rest.\n\r",ch);
            act("$n rests.",ch,NULL,NULL,TO_ROOM);
        }           
        else if (IS_SET(obj->value[2],REST_AT))
        {
            act("You rest at $p.",ch,obj,NULL,TO_CHAR);
            act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act("You rest on $p.",ch,obj,NULL,TO_CHAR);
            act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act("You rest in $p.",ch,obj,NULL,TO_CHAR);
            act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
        ch->position = POS_RESTING;
        break;      
    }            
    return;
}
        
         
void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    if (ch->position == POS_FIGHTING)
    {
        send_to_char("Maybe you should finish this fight first?\n\r",ch);
        return;
    }
        
    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {       
        obj = get_obj_list(ch,argument,ch->in_room->contents);
        if (obj == NULL)
        {
            send_to_char("You don't see that here.\n\r",ch);
            return;
        }
    }
    else obj = ch->on;
    
    if (obj != NULL)
    {
        if (!IS_SET(obj->item_type,ITEM_FURNITURE)
        ||  (!IS_SET(obj->value[2],SIT_ON)
        &&   !IS_SET(obj->value[2],SIT_IN)
        &&   !IS_SET(obj->value[2],SIT_AT)))
        {
            send_to_char("You can't sit on that.\n\r",ch);
            return;
        }
         
        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
            act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            return;
        }
    
        ch->on = obj;
    }
    switch (ch->position)
    {
        case POS_SLEEPING:
            if (IS_AFFECTED(ch,AFF_SLEEP))  
            {
                send_to_char("You can't wake up!\n\r",ch);
                return;
            }
         
            if (obj == NULL)
            {
                send_to_char( "You wake and sit up.\n\r", ch );
                act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
                act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
                act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
                act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
                act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
                act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
                act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }
                
            ch->position = POS_SITTING;
            break;

    case POS_RECLINING:
        if (obj == NULL)
        {
            act( "$n sit$% up.", ch, NULL, NULL, TO_ALL );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {
           act("$n sit$% at $p.",ch,obj,NULL,TO_ALL);
        }   
        else if (IS_SET(obj->value[2],STAND_ON))
        {
            act("$n sit$% on $p.",ch,obj,NULL,TO_ALL);
        }
        else
        {
            act("$n sit$% in $p.",ch,obj,NULL,TO_ALL);
        }
        ch->position = POS_SITTING;
	break;

        case POS_RESTING:
            if (obj == NULL)
                send_to_char("You stop resting.\n\r",ch);
            else if (IS_SET(obj->value[2],SIT_AT))
            {
                act("You sit at $p.",ch,obj,NULL,TO_CHAR);
                act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
            }
             
            else if (IS_SET(obj->value[2],SIT_ON))
            {
                act("You sit on $p.",ch,obj,NULL,TO_CHAR);
                act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
            }
            ch->position = POS_SITTING;
            break;
        case POS_SITTING:
            send_to_char("You are already sitting down.\n\r",ch);
            break;
        case POS_STANDING:
            if (obj == NULL)
            {
                send_to_char("You sit down.\n\r",ch);
                act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
                act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
                act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
                act("You sit on $p.",ch,obj,NULL,TO_CHAR);
                act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
            }
            else  
            {
                act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
                act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
            }
            ch->position = POS_SITTING;
            break;
    }
    return;  
}
                

void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
        send_to_char( "You are already sleeping.\n\r", ch );   
        break;
             
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
        if (argument[0] == '\0' && ch->on == NULL)
        {
            send_to_char( "You go to sleep.\n\r", ch );
            act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    if ( IS_GRASPING(ch) )
		do_release( ch, "" );
            ch->position = POS_SLEEPING;
        }
        else  /* find an object and sleep on it */
        {       
            if (argument[0] == '\0')
                obj = ch->on;
            else
                obj = get_obj_list( ch, argument,  ch->in_room->contents );
        
            if (obj == NULL)
            {
                send_to_char("You don't see that here.\n\r",ch);
                return;
            }
            if (obj->item_type != ITEM_FURNITURE
            ||  (!IS_SET(obj->value[2],SLEEP_ON)
            &&   !IS_SET(obj->value[2],SLEEP_IN)
            &&   !IS_SET(obj->value[2],SLEEP_AT)))
            {
                send_to_char("You can't sleep on that!\n\r",ch);
                return;
            }
                
            if (ch->on != obj && count_users(obj) >= obj->value[0])
            {
                act_new("There is no room on $p for you.",
                    ch,obj,NULL,TO_CHAR,POS_DEAD);
                return;
            }
                
            ch->on = obj;
            if (IS_SET(obj->value[2],SLEEP_AT)) 
            {
                act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
                act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SLEEP_ON))
            {
                act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
                act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
                act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
                act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
            }
	    if ( IS_GRASPING(ch) )
		do_release( ch, "" );
            ch->position = POS_SLEEPING;
        }
        break;
    case POS_RECLINING:
        if (obj == NULL)
        {
	    send_to_char( "You go to sleep.\n\r", ch );
            act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
            ch->on = NULL;
        }
        else if (IS_SET(obj->value[2],STAND_AT))
        {
	    send_to_char( "You go to sleep.\n\r", ch );
            act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
        }   
        else if (IS_SET(obj->value[2],STAND_ON))
        {
	    send_to_char( "You go to sleep.\n\r", ch );
            act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    send_to_char( "You go to sleep.\n\r", ch );
            act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
        }
	if ( IS_GRASPING(ch) )
	    do_release( ch, "" );
        ch->position = POS_SLEEPING;
	break;
             
    case POS_FIGHTING:
        send_to_char( "You are already fighting!\n\r", ch );
        break;
    }

    return;
}

void do_recline( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if ( is_affected(ch, skill_lookup("wrap")) )
    {
	send_to_char( "You are wrapped in flows of air.  You cannot move.\n\r", ch );
	return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	act( "$n wakes up.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You wake up.", ch );
	ch->position = POS_RECLINING;
	break;
    case POS_RECLINING:
	send_to_char( "You are already reclining.\n\r", ch );
	return;
             
    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
        if (argument[0] == '\0' && ch->on == NULL)
        {
	    act( "$n lie$% down.", ch, NULL, NULL, TO_ALL );
            ch->position = POS_RECLINING;
        }
        else  /* find an object and recline on it */
        {       
            if (argument[0] == '\0')
                obj = ch->on;
            else
                obj = get_obj_list( ch, argument,  ch->in_room->contents );
        
            if (obj == NULL)
            {
                send_to_char("You don't see that here.\n\r",ch);
                return;
            }
            if (obj->item_type != ITEM_FURNITURE
            ||  (!IS_SET(obj->value[2],SLEEP_ON)
            &&   !IS_SET(obj->value[2],SLEEP_IN)
            &&   !IS_SET(obj->value[2],SLEEP_AT)))
            {
                send_to_char("You can't recline on that!\n\r",ch);
                return;
            }
                
            if (ch->on != obj && count_users(obj) >= obj->value[0])
            {
                act_new("There is no room on $p for you.",
                    ch,obj,NULL,TO_CHAR,POS_DEAD);
                return;
            }
                
            ch->on = obj;
            if (IS_SET(obj->value[2],SLEEP_AT)) 
            {
		act( "$n lie$% down at $p",ch,obj,NULL,TO_ALL);
            }
            else if (IS_SET(obj->value[2],SLEEP_ON))
            {
		act( "$n lie$% down on $p",ch,obj,NULL,TO_ALL);
            }
            else
            {
		act( "$n lie$% down in $p",ch,obj,NULL,TO_ALL);
            }
            ch->position = POS_RECLINING;
        }
        break;
             
    case POS_FIGHTING:
        send_to_char( "You are already fighting!\n\r", ch );
        break;
    }

    return;
}
             


void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, "" ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    victim->position = POS_STANDING;
    act_new( "$n wake$% $N.", ch, NULL, victim, TO_ALL, POS_SLEEPING );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int luck;

    if ( IS_AFFECTED(ch, AFF_SNEAK)
    ||   is_affected(ch, gsn_sneak) )
    {
	send_to_char( "You stop sneaking.\n\r", ch );
	affect_strip( ch, gsn_sneak );
	REMOVE_BIT(ch->affected_by, AFF_SNEAK );
	return;
    }

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( check_skill(ch, gsn_sneak, 100, TRUE) )
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.type		= gsn_sneak;
	af.strength	= ch->level; 
	af.duration	= ch->level;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_SNEAK;
	af.bitvector_2	= 0;  
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    affect_strip ( ch, gsn_shape_change );
    affect_strip ( ch, gsn_disguise );
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    REMOVE_BIT( ch->affected_by, AFF_SHAPE_CHANGE );
    ch->hide_type = HIDE_NONE;
    ch->hide = NULL;

    if ( !IS_NPC(ch) )
    {
	free_string( ch->pcdata->new_name );
	ch->pcdata->new_name = NULL;
	free_string( ch->pcdata->new_desc );
	free_string( ch->pcdata->new_title );
	free_string( ch->pcdata->new_last );
	ch->pcdata->new_desc  = str_dup( "" );
	ch->pcdata->new_title = str_dup( "" );
	ch->pcdata->new_last  = str_dup( "" );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].roll_mod;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }
  
    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->level > 10 && !IS_IMMORTAL(ch) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
    {
	send_to_char( "The Creator has forsaken you.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = 40 + ch->level;

	if ( number_percent() < 80 * skill / 100 )
	{
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );	
    }

    ch->stamina /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "moved" );
    
    if (ch->pet != NULL)
	do_recall(ch->pet,"");

    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 3;

    if ( !str_prefix( argument, "strength" ) )
    {
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_prefix( argument, "intelligence" ) )
    {
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_prefix( argument, "wisdom" ) )
    {
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_prefix( argument, "dexterity" ) )
    {
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_prefix( argument, "constitution" ) )
    {
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_prefix( argument, "charisma" ) )
    {
	stat	    = STAT_CHR;
	pOutput     = "charisma";
    }

    else if ( !str_prefix( argument, "luck" ) )
    {
	stat	    = STAT_LUK;
	pOutput     = "luck";
    }

    else if ( !str_prefix( argument, "agility" ) )
    {
	stat	    = STAT_AGI;
	pOutput     = "agility";
    }

    else if ( !str_prefix(argument, "hp" ) )
	cost = 1;

    else if ( !str_prefix(argument, "stamina" ) )
	cost = 1;

    else
    {
	sprintf( buf, "It will cost you 3 trains to improve stats and 1 train to improve hit points or stamina.\n\r" );
	send_to_char( buf, ch );
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	if ( ch->perm_stat[STAT_CHR] < get_max_train(ch,STAT_CHR))  
	    strcat( buf, " cha" );
	if ( ch->perm_stat[STAT_LUK] < get_max_train(ch,STAT_LUK))  
	    strcat( buf, " luc" );
	if ( ch->perm_stat[STAT_AGI] < get_max_train(ch,STAT_AGI))  
	    strcat( buf, " agi" );
	strcat( buf, " hp stamina");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_prefix(argument,"hp") )
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        return;
    }
 
    if (!str_prefix(argument,"stamina") && stat == -1)
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_stamina += 10;
        ch->max_stamina += 10;
        ch->stamina += 10;
        act( "Your endurance increases!",ch,NULL,NULL,TO_CHAR);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    return;
}

void do_push( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    int door;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' && arg2[0] == '\0' )
    {
	send_to_char( "Push whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( ch == victim )
    {
	send_to_char( "Pushing yourself doesn't do much.\n\r", ch );
	return;
    }

	 if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
    else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
    else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
    else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
    else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
    else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
    else
    {
	act( "In which direction would you like to push $N?", ch, NULL,
	    victim, TO_CHAR );
	return;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "There is no way $t from here.", ch, dir_name[door], NULL, TO_CHAR );
	return;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	act( "The way $t seems closed.", ch, dir_name[door], NULL, TO_CHAR );
	return;
    }

    if ( number_fuzzy(get_curr_stat( ch, STAT_STR )) <=
	 number_fuzzy(get_curr_stat( victim, STAT_STR )) +
	 (IS_SET( victim->act, ACT_SENTINEL ) ? 3 : 0) )
    {
	act( "You just can't seem to push $N around.", ch, NULL, victim, TO_CHAR );
	act( "You chuckle as $n tries to push you.", ch, NULL, victim, TO_VICT );
	act( "$n futilely tries pushing $N around.", ch, NULL, victim, TO_NOTVICT );
	return;
    }

    act( "You push $N $t.", ch, dir_name[door], victim, TO_CHAR );
    act( "$n pushes you $t.", ch, dir_name[door], victim, TO_VICT );
    act( "$n pushes $N $t.", ch, dir_name[door], victim,  TO_NOTVICT );

    move_char( victim, door, FALSE );
}



void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *vobj;
    EXIT_DATA *pexit;
    int door;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' && arg2[0] == '\0' )
    {
	send_to_char( "Drag whom or what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	if ( ch == victim )
	{
	    send_to_char( "Dragging yourself doesn't do much.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
	else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
	else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
	else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
	else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
	else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
	else
	{
	    act( "In which direction would you like to drag $N?", ch, NULL,
		victim, TO_CHAR );
	    return;
	}

	if ( ( pexit = ch->in_room->exit[door] ) == NULL )
	{
	    act( "There is no way $t from here.", ch, dir_name[door],
		NULL, TO_CHAR );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The way $t seems closed.", ch, dir_name[door], NULL, TO_CHAR );
	    return;
	}

	if ( number_fuzzy(get_curr_stat( ch, STAT_STR )) <=
	    (number_fuzzy( get_curr_stat(victim, STAT_STR) ) +
	     number_fuzzy( 2 )) +
	 (IS_SET( victim->act, ACT_SENTINEL ) ? 3 : 0) )

	{
	    act( "You just can't seem to drag $N around.", ch, NULL,
		victim, TO_CHAR );
	    act( "You chuckle as $n tries to drag you.", ch, NULL,
		victim, TO_VICT );
	    act( "$n futilely tries dragging $N around.", ch, NULL,
		victim, TO_NOTVICT );
	    return;
	}

	act( "You drag $N $t.", ch, dir_name[door], victim, TO_CHAR );
	act( "$n drags you $t.", ch, dir_name[door], victim, TO_VICT );
	act( "$n drags $N $t.", ch, dir_name[door], victim,  TO_NOTVICT );

	move_char( ch, door, FALSE );
	move_char( victim, door, FALSE );

	act( "$n drags $N in.", ch, NULL, victim,  TO_NOTVICT );
	return;
    }
    else if ( ( vobj = get_obj_here( ch, arg1 ) ) != NULL )
    {

	if ( vobj->item_type == ITEM_FURNITURE
	&&   count_users(vobj) > 0 )
	{
	    send_to_char( "You can't drag that with people using it.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
	else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
	else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
	else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
	else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
	else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
	else
	{
	    act( "In which direction would you like to drag $N?", ch, NULL,
		victim, TO_CHAR );
	    return;
	}

	if ( ( pexit = ch->in_room->exit[door] ) == NULL )
 	{
	    act( "There is no way $t from here.", ch, dir_name[door],
		NULL, TO_CHAR );
	    return;
	}

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The way $t seems closed.", ch, dir_name[door], NULL, TO_CHAR );
	    return;
	}

	if ( (str_app[number_fuzzy(get_curr_stat( ch, STAT_STR ))].carry <=
	    get_obj_weight(vobj)) ||
	    !IS_SET(vobj->wear_flags, ITEM_TAKE) )
	{
	    act( "You just can't seem to drag $P.", ch, NULL,
		vobj, TO_CHAR );
	    act( "$n futilely tries dragging $P.", ch, NULL,
		vobj, TO_ROOM );
	    return;
	}

	act( "You drag $P $t.", ch, dir_name[door], vobj, TO_CHAR );
	act( "$n drag $P $t.", ch, dir_name[door], vobj,  TO_ROOM );

	obj_from_room( vobj );
	move_char( ch, door, FALSE );
	obj_to_room( vobj, ch->in_room );

	act( "You drag $P in.", ch, NULL, vobj,  TO_CHAR );
	act( "$n drags $P in.", ch, NULL, vobj,  TO_NOTVICT );
	return;
    }

    send_to_char( "You don't see that here.\n\r", ch );
}

void do_knock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int door;

    one_argument( argument, arg );

    if ( (door = find_door( ch, arg )) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }

	act( "$n knocks on the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		 act( "Someone knocks on the $d.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
	return;
    }
    return;
}

void do_snare( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int chance = 65;
    int time;

    time = number_fuzzy( skill_table[gsn_set_snare].rating[ch->class] ) * 10;

    switch(ch->in_room->sector_type)
    {
	default:
		send_to_char( "You may not set a trap here.\n\r", ch );
		free_action( ch );
		return;
	case SECT_INSIDE:
		time++;
		chance -= 15;
		break;
	case SECT_CITY:
		time++;
		chance -= 35;
		break;
	case SECT_FOREST:
		time -= 4;
		chance += 50;
		break;
	case SECT_FIELD:
		time--;
		chance += 5;
		break;
	case SECT_HILLS:
		time -= 3;
		chance += 20;
		break;
	case SECT_MOUNTAIN:
		time -= 2;
		chance += 15;
		break;
	case SECT_DESERT:
		chance += 5;
		break;
	case SECT_UNDERGROUND:
		time++;
		chance += 20;
		break;
	case SECT_SWAMP:
		time -= 2;
		chance += 5;
		break;
    }

    if ( --ch->action_timer < 0 )
    {
	if ( !ch->in_room )
	{
	    free_action( ch );
	    ch->action_timer = 0;
	    return;
	}

	act( "$n begin$% working on something in the room.", ch, NULL,
	    NULL, TO_ALL );
	new_action( ch, do_snare, time, "", NULL );
	return;
    }
    else if ( ch->action_timer > 0 )
	return;

    free_action( ch );
    act( "$n complete$% working on something in the room.", ch,
	NULL, NULL, TO_ALL );
    if ( !check_skill(ch, gsn_set_snare, chance, TRUE) )
    {
	send_to_char( "You slip and ruin the trap.\n\r", ch );
	return;
    }
    send_to_char( "You deftly place a trap in the room.\n\r", ch );
    

    af.type             = gsn_set_snare;
    af.strength         = number_fuzzy(0) * number_range(1, 10)
			+ get_skill(ch, gsn_set_snare);
    af.duration         = number_fuzzy(0) * number_range(1, 3)
			+ get_skill(ch, gsn_set_snare) / 10;
    af.modifier         = 0;
    af.location         = 0;
    af.bitvector        = 0;
    af.bitvector_2      = 0;
    af.owner            = NULL;
    af.flags            = 0;

    affect_to_room(ch->in_room, &af); 
    return;
}

ROOM_INDEX_DATA  *get_explore_room( )
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( number_range( 0, 65535 ) );
        if ( room != NULL )
	{
	    if ( !room_is_private(room)   
	    &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
	    &&   !IS_SET(room->room_flags, ROOM_SOLITARY)
	    &&   !IS_SET(room->room_flags, ROOM_SAFE)
	    &&   room->sector_type != SECT_INSIDE
	    &&   room->sector_type != SECT_CITY
	    &&   room->sector_type != SECT_AIR
	    &&   room->sector_type != SECT_UNUSED
	    &&   room->sector_type != SECT_WATER_SWIM
	    &&   room->sector_type != SECT_WATER_NOSWIM
	    &&   !IS_SET(room->area->area_flags, AREA_COMPLETE) )
		break;
	}
    }

    return room;
}


void add_explore( )
{
    DESCRIPTOR_DATA *d;
    AREA_DATA *a;
    ROOM_INDEX_DATA *room;
    int count_p = 0;
    int count_a = 0;

    for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING )
	    count_p++;
    for ( a = area_first; a; a = a->next )
	if ( IS_SET(a->area_flags, AREA_COMPLETE) )
	    count_a++;
    if ( num_explore >= count_a * count_p / 15 )
	return;

    room = get_explore_room( );
    if ( !room )
	return;

    num_explore++;
    SET_BIT(room->room_flags, ROOM_EXPLORE);
    return;
}
