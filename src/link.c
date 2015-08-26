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
#include "merc.h"
#include "interp.h"
#include "mem.h"


void do_link( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	if ( !IS_SET(ch->affected_by_2, AFF_LINK) )
	{
	    send_to_char( "You are not part of a circle.\n\r", ch );
	    return;
	}

	leader = ch->leader ? ch->leader : ch;
	send_to_char_new( ch, "%s's circle:\n\r", PERS(leader,ch) );

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	{
	    if ( is_same_group(gch, ch)
	    &&   IS_SET(gch->affected_by_2, AFF_LINK) )
	    {
		char *color_str;
		char stamina_bar[34];
		int val = 100;
		int i;

		val = UMAX((ch->max_stamina ? ch->stamina * 100 / ch->max_stamina : 100), 0);

		if ( val >= 50 )
		    color_str = "`2";
		else if ( val >= 25 )
		    color_str = "`B`6";
		else
		    color_str = "`3";

		for ( i = 0; i < val / 3; i++ )
		    stamina_bar[i] = '#';
		for ( ; i < 33; i++ )
		    stamina_bar[i] = ' ';
		stamina_bar[33] = '\0';

		sprintf( buf, "[%6s] %-20.20s [%s%s`n] stamina\n\r",
		    leader == gch ? "LEADER" : "",
		    FIRSTNAME(gch),
		    color_str, stamina_bar );

		send_to_char( buf, ch );
	    }
	}
	send_to_char_new( ch, "Women: %2d Men: %2d\r\n",
	    link_count(leader, SEX_FEMALE),
	    link_count(leader, SEX_MALE) );
	return;
    }

    if ( !IS_GRASPING(ch) )
    {
	send_to_char( "You must be grasping the one power to form a circle.\n\r", ch );
	return;
    }

    if ( !IS_LINKING(ch)
    &&   (ch->leader == NULL || ch->leader == ch) )
    {
	send_to_char( "You are now part of the circle.\r\n", ch );
	SET_BIT(ch->affected_by_2, AFF_LINK);
    }
	
    if ( str_cmp(arg, "all") )
    {
        if ( (victim = get_char_room( ch, arg )) == NULL )
        {
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
        }

	if ( ch == victim )
	{
	    send_to_char( "You are already in the circle.\r\n", ch );
	    return;
	}

	if ( !is_same_group(ch, victim) )
	{
	    act( "$N is not part of your group.", ch, NULL, victim,
		TO_ALL );
	    return;
	}

        if ( ch->master != NULL || (ch->leader != NULL && ch->leader != ch) )
        {
	    send_to_char( "But you are not a group leader!\n\r", ch );
	    return;
        }

        if ( (victim->master != ch && ch != victim)
	||   IS_SET(victim->affected_by, AFF_STALK) )
        {
	    act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	    return;
        }

	if ( !IS_GRASPING(victim) )
	{
	    act( "$N is not grasping the one power.", ch, NULL,
		victim, TO_CHAR );
	    return;
	}

	if ( IS_LINKING(victim) )
	{
	    act( "$N is already in a circle.", ch, NULL, victim, TO_CHAR );
	    return;
	}

	if ( !can_join_link(ch, victim) )
	{
	    act( "$N cannot join your circle.", ch, NULL, victim, TO_CHAR );
	    return;
	}

	link_stamina( ch, victim, TRUE );
        act( "$N joins $n's circle.", ch, NULL, victim, TO_NOTVICT );
        act( "You join $n's circle.", ch, NULL, victim, TO_VICT    );
        act( "$N joins your circle.", ch, NULL, victim, TO_CHAR    );
	SET_BIT(victim->affected_by_2, AFF_LINK);
    }
    else
    {
	CHAR_DATA *vch;
        if ( ch->master != NULL || (ch->leader != NULL && ch->leader != ch) )
        {
	    send_to_char( "But you are following someone else!\n\r", ch );
	    return;
        }

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
	    if ( vch->master == ch
	    &&   vch != ch 
	    &&   is_same_group(vch, ch)
	    &&   !IS_SET(vch->affected_by, AFF_STALK)
	    &&   can_join_link(ch, vch)
	    &&   !IS_LINKING(vch)
	    &&   IS_GRASPING(vch) )
	    {
		vch->leader = ch;
		link_stamina( ch, vch, TRUE );
		act( "$N joins $n's circle.", ch, NULL, vch, TO_NOTVICT );
		act( "You join $n's circle.", ch, NULL, vch, TO_VICT    );
		act( "$N joins your circle.", ch, NULL, vch, TO_CHAR    );
		SET_BIT(vch->affected_by_2, AFF_LINK);
	    }
	}
    }

    return;
}

void do_unlink( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( !IS_LINKING(ch)
    ||   (ch->leader != NULL && ch->leader != ch) )
    {
	send_to_char( "You are not leading a circle.\r\n", ch );
	return;
    }
	
    if ( arg[0] == '\0' || !str_cmp(arg, "all") )
    {
	CHAR_DATA *vch;
        if ( ch->master != NULL || (ch->leader != NULL && ch->leader != ch) )
        {
	    send_to_char( "But you are following someone else!\n\r", ch );
	    return;
        }

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
	    if ( (vch->master == ch
	    ||    vch->leader == ch)
	    &&   is_same_group(vch, ch)
	    &&   IS_LINKING(vch) )
	    {
		link_stamina( ch, vch, FALSE );
		send_to_char( "The circle is dissolved.\r\n", vch );
		REMOVE_BIT(vch->affected_by_2, AFF_LINK);
	    }
	}
    }
    else
    {
        if ( (victim = get_char_room( ch, arg )) == NULL )
        {
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
        }

	if ( ch == victim )
	{
	    send_to_char( "You cannot remove yourself from the circle without removing everyone.\r\n", ch );
	    return;
	}

	if ( ch->master != NULL || (ch->leader != NULL && ch->leader != ch) )
	{
	    send_to_char( "But you are not a group leader!\n\r", ch );
	    return;
	}

	if ( (victim->master != ch && ch != victim)
	||   IS_SET(victim->affected_by, AFF_STALK) )
	{
	    act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	    return;
	}

	if ( !IS_LINKING(victim) )
	{
	    act( "$N is not part of the circle.", ch, NULL, victim, TO_CHAR );
	    return;
	}

	link_stamina( ch, victim, FALSE );
	act( "$N is removed $n's circle.", ch, NULL, victim, TO_NOTVICT );
	act( "You are removed from $n's circle.", ch, NULL, victim, TO_VICT );
	act( "$N is removed from your circle.", ch, NULL, victim, TO_CHAR );
	REMOVE_BIT(victim->affected_by_2, AFF_LINK);
    }

    return;
}

void link_stamina( CHAR_DATA *leader, CHAR_DATA *ch, bool fGain )
{
    AFFECT_DATA af;
    int value;
    int old_percent;

    if ( !leader || !ch )
	return;

    if ( leader == ch )
	return;

    value		= ch->max_stamina / 5;
    if ( fGain == FALSE )
	value = -value;

    old_percent		= leader->max_stamina ?
			      leader->stamina * 100 / leader->max_stamina : 100;

    af.type		= gsn_linking;
    af.strength		= 0;
    af.duration		= -1;
    af.modifier		= value;
    af.location		= APPLY_STAMINA;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= NULL;
    af.flags		= AFFECT_LINKING|AFFECT_NOTCHANNEL;

    affect_join( leader, &af );

    if ( fGain == FALSE )
	leader->stamina = leader->max_stamina * old_percent / 100;
    else
	leader->stamina = UMAX(leader->max_stamina, leader->stamina + value);

    return;
}

int link_count( CHAR_DATA *ch, int sex )
{
    int count = 0;
    CHAR_DATA *gch;

    if ( !IS_GRASPING(ch)
    ||   !IS_LINKING(ch) )
	return 0;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( !IS_GRASPING(gch)
	||   !IS_LINKING(gch)
	||   TRUE_SEX(gch) != sex
	||   !is_same_group(ch, gch)
	||   (gch->leader != NULL && gch->leader != ch) )
	    continue;
	count++;
    }

    return count;
}

bool can_join_link( CHAR_DATA *ch, CHAR_DATA *join )
{
    int male, female, total;

    male   = link_count( ch, SEX_MALE );
    female = link_count( ch, SEX_FEMALE );
    total  = male + female;

    if ( total == 71 && TRUE_SEX(ch) != SEX_MALE )
	return FALSE;

    if ( female == 0 && TRUE_SEX(join) != SEX_FEMALE )
	return FALSE;

    if ( male > 2 || female > 2 )
	if ( male >= female && TRUE_SEX(join) == SEX_MALE )
	    return FALSE;

    if ( female / 13 > male && TRUE_SEX(join) == SEX_FEMALE )
	return FALSE;

    if ( female == 66 && TRUE_SEX(join) == SEX_FEMALE )
	return FALSE;

    if ( male == 6 && TRUE_SEX(join) == SEX_MALE )
	return FALSE;
    return TRUE;
}


void drop_link( CHAR_DATA *ch )
{
    CHAR_DATA *leader;

    if ( !IS_AFFECTED_2(ch, AFF_LINK) )
	return;

    leader = ch->leader ? ch->leader : ch;
    REMOVE_BIT( ch->affected_by_2, AFF_LINK );
    link_stamina( leader, ch, FALSE );
    do_release( ch, "" );

    return;
}
