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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

#define MAX_DAMAGE_MESSAGE	32
#define RID			ROOM_INDEX_DATA
#define	SPELL( fun )		void fun( int sn, int strength, CHAR_DATA *ch, void *vo, int target_type )

/* command procedures needed */
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_release	);

RID	*get_random_room	args( ( CHAR_DATA *ch ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
int	find_spell_door	args( ( ROOM_INDEX_DATA *pRoom, char *arg, int sn ) );
int	get_random_door	args( ( ROOM_INDEX_DATA *pRoom ) );
int 	table_normal	args( ( int table, int strength ) );
int 	table_multiplier	args( ( int strength, int multiplier ) );
bool	break_weave	args( ( int char_strength, int weave_strength ) );
bool    owns_affect     args( ( CHAR_DATA *ch, bool fTied ) );
void	set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );
int     find_exit       args( ( CHAR_DATA *ch, char *arg ) );


/*
 * Local functions.
 */
bool	check_unconc	args( ( CHAR_DATA *ch ) );
void	show_flows	args( ( CHAR_DATA *ch, int sn, void *vo,
				int target_type ) );
void	spell_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim,int dam,
			        int dt,bool immune ) );
bool	spell_damage	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int dam_type ) );

/* Constants */
extern	char 	*	const   		dir_name	[];
extern			const	sh_int		rev_dir		[];

/* External variables */
extern	char	*	target_name;
extern	bool		successful_cast;

SPELL( spell_null )
{
    send_to_char( "That is not a weave.\n\r", ch );
    return;
}

SPELL( spell_flame_dart )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 1, strength );
    victim = (CHAR_DATA *) vo;

    act( "A small ball of flame streaks from $o hand towards $N.",
	ch, NULL, victim, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_FIRE );

    return;
}

SPELL( spell_stone_strike )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 3, strength );
    victim = (CHAR_DATA *) vo;

    act( "A small rock streaks from the ground toward from $N.",
	ch, NULL, victim, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_PIERCE );
    return;
}

SPELL( spell_wind_mace )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 5, strength );
    victim = (CHAR_DATA *) vo;

    act( "$n fall$% backwards slightly, as if hit.",
	victim, NULL, NULL, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_BASH );

    return;
}

SPELL( spell_flame_column )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 40, strength );
    victim = (CHAR_DATA *) vo;

    act( "A column of flame erupts around $N.",
	ch, NULL, victim, TO_ALL );

    if ( number_percent() < strength / 2 )
    {
	OBJ_DATA *obj_lose, *obj_next;
        for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;
            if ( number_bits(2) != 0 )
                continue;

            switch ( obj_lose->item_type )
            {
            default:             continue;
            case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
            case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
            case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
            }

            act( msg, victim, obj_lose, NULL, TO_CHAR );
            if (obj_lose->item_type == ITEM_CONTAINER)
            {
		OBJ_DATA *t_obj, *n_obj;
                /* save some of  the contents */
                for (t_obj = obj_lose->contains; t_obj != NULL; t_obj = n_obj)
                {
                    n_obj = t_obj->next_content;
                    obj_from_obj(t_obj);

                    if (number_bits(2) == 0 || ch->in_room == NULL)
                        extract_obj(t_obj);
                    else
                        obj_to_room(t_obj,ch->in_room);
                }
            }
            extract_obj( obj_lose );
        }
    }
    spell_damage( ch, victim, dam, sn, DAM_FIRE );

    return;
}

SPELL( spell_shield_from_true_source )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_TALENTED(ch, tn_shielding) )
	strength = strength * 125 / 80;

    if ( is_affected(victim, sn) )
    {
	act( "$N is already shielded from the True Source.", ch, NULL,
	    victim, TO_CHAR );
	return;
    }

    if ( IS_GRASPING(victim) )
    {
	int victim_strength;

	victim_strength = channel_strength( victim, POWER_ALL ) * 150 / 100;
	if ( victim_strength > strength )
	{
	    act( "You were not able to shield $N.", ch, NULL, victim,
		TO_CHAR );
	    return;
	}
    }

    if ( can_channel(victim, 1) )
    {
    act( "You have been shielded from the True Source!", ch, NULL,
	victim, TO_VICT );
    }

    act_channel( "You have shielded $N from the True Source!", ch, NULL,
	victim, TO_CHAR );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 45 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= AFF_SHIELDED;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( victim, &af );
    if ( owns_affect(victim, TRUE) )
	send_to_char( "Your maintained weaves fade, as you are shielded.\n\r", victim );
    die_weave( victim );
    drop_link( victim );
    if ( IS_GRASPING(victim) )
	do_release( victim, "" );
    return;
}


SPELL( spell_gag )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( is_affected(victim, sn) )
    {
	act( "$N has already been gagged.", ch, NULL,
	    victim, TO_CHAR );
	return;
    }

    act( "You have been gagged!", ch, NULL, victim, TO_VICT );
    act( "You have gagged $N!", ch, NULL, victim, TO_CHAR );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 15 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= AFF_GAG;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_char( victim, &af );
    return;
}

SPELL( spell_wrap )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( is_affected(victim, sn) )
    {
	act( "$N has already been wrapped.", ch, NULL,
	    victim, TO_CHAR );
	return;
    }

    act( "You have been wrapped!", ch, NULL, victim, TO_VICT );
    act( "You have wrapped $N!", ch, NULL, victim, TO_CHAR );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 15 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= AFF_WRAP;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_char( victim, &af );
    return;
}

SPELL( spell_cut_weave )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA		*victim;
    ROOM_INDEX_DATA	*vRoom;
    OBJ_DATA		*vObj;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    int vSn, weave_strength;
    bool found = FALSE;

    target_name = one_argument( target_name, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	act( "Which weave do you wish to cut?", ch, NULL, NULL, TO_CHAR );
	return;
    }

    if ( (vSn = skill_lookup( arg1 )) < 0 )
    {
        send_to_char( "There is no such weave as this.\n\r", ch );
        return;
    }

    if ( !str_cmp("here", arg2) )
    {
	vRoom = ch->in_room;
        for ( paf = vRoom->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
	    if ( paf->type != vSn )
		continue;

	    if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
		continue;

            if ( IS_TIED(paf) || paf->owner == NULL )   
                weave_strength = number_range( paf->strength * 3 / 4,paf->strength );
            else
                weave_strength = channel_strength( paf->owner, POWER_ALL );

	    found = TRUE;
	    if ( !break_weave(strength, weave_strength) )
	    {
		send_to_char( "You could not cut the weave.\n\r", ch );
		return;
	    }

	    act( "You feel your $t weave in $T being taken apart.",
		paf->owner, skill_table[vSn].name, vRoom->name, TO_CHAR );
	    if ( paf->owner != ch )
		send_to_char( "You slice through the weave.\n\r", ch );

	    affect_room_remove( vRoom, paf );
	}
	if ( !found )
	    act( "You cannot see that weave in $t.", ch, ch->in_room->name,
		NULL, TO_CHAR );
	return;
    }

    if ( (victim = get_char_room( ch, arg2)) != NULL )
    {
        for ( paf = victim->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
	    if ( paf->type != vSn )
		continue;

	    if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
		continue;

            if ( IS_TIED(paf) || paf->owner == NULL )   
                weave_strength = number_range( paf->strength * 3 / 4,paf->strength );
            else
                weave_strength = channel_strength( paf->owner, POWER_ALL );

	    found = TRUE;
	    if ( !break_weave(strength, weave_strength) )
	    {
		send_to_char( "You could not cut the weave.\n\r", ch );
		return;
	    }

	    if ( paf_next == NULL
	    ||   paf->type != paf_next->type
	    ||   paf_next->duration > 0 )
	    {
		if ( paf->type > 0 && skill_table[vSn].msg_off )
		{
		    send_to_char( skill_table[vSn].msg_off, victim );
		    send_to_char( "\n\r", victim );
		}
	    }

	    act( "You feel your $t weave on $N being taken apart.",
		paf->owner, skill_table[vSn].name, victim, TO_CHAR );
	    if ( paf->owner != ch )
		send_to_char( "You slice through the weave.\n\r", ch );

	    affect_remove( victim, paf );
	}
	if ( !found )
	    act( "You cannot see that weave on $N.", ch, NULL,
		victim, TO_CHAR );
	return;
    }

    if ( (vObj = get_obj_here( ch, arg2)) != NULL )
    {
        for ( paf = vObj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
	    if ( paf->type != vSn )
		continue;

	    if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
		continue;

            if ( IS_TIED(paf) || paf->owner == NULL )   
                weave_strength = number_range( paf->strength * 3 / 4,paf->strength );
            else
                weave_strength = channel_strength( paf->owner, POWER_ALL );

	    found = TRUE;
	    if ( !break_weave(strength, weave_strength) )
	    {
		send_to_char( "You could not cut the weave.\n\r", ch );
		return;
	    }

	    act( "You feel your $t weave on $P being taken apart.",
		paf->owner, skill_table[vSn].name, vObj, TO_CHAR );
	    if ( paf->owner != ch )
		send_to_char( "You slice through the weave.\n\r", ch );

	    affect_obj_remove( vObj, paf );
	}
	if ( !found )
	    act( "You cannot see that weave on $P.", ch, NULL,
		vObj, TO_CHAR );
	return;
    }
    send_to_char( "You cannot find that weave anywhere.\n\r", ch );
    return;
}

SPELL( spell_minor_heal )
{
    CHAR_DATA *victim;
    int heal;

    heal = table_normal( 4, strength );
    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 

    cure_condition( victim, BODY_BLIND, 5 );
    cure_condition( victim, BODY_POISON, 5 );
    cure_condition( victim, BODY_DISEASE, 5 );

    heal = UMIN( heal, victim->max_hit - victim->hit );
    gain_condition( victim, COND_FULL, -2 );

    lose_stamina( ch, heal / 8, FALSE, TRUE );
    lose_stamina( victim, heal / 4, FALSE, TRUE );

    if ( gain_health( victim, heal, FALSE ) )
	act( "$n look$% a little better.", victim, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_heal )
{
    CHAR_DATA *victim;
    int heal;

    heal = table_normal( 8, strength );
    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 

    cure_condition( victim, BODY_BLIND, 10 );
    cure_condition( victim, BODY_POISON, 10 );
    cure_condition( victim, BODY_DISEASE, 10 );

    heal = UMIN( heal, victim->max_hit - victim->hit );
    gain_condition( victim, COND_FULL, -3 );

    lose_stamina( ch, heal / 8, FALSE, TRUE );
    lose_stamina( victim, heal / 4, FALSE, TRUE );

    if ( gain_health( victim, heal, FALSE ) )
	act( "$n look$% better.", victim, NULL, NULL, TO_ALL );
    return;
}


SPELL( spell_major_heal )
{
    CHAR_DATA *victim;
    int heal;

    heal = table_normal( 16, strength );
    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 

    cure_condition( victim, BODY_BLIND, 25 );
    cure_condition( victim, BODY_POISON, 25 );
    cure_condition( victim, BODY_DISEASE, 25 );

    heal = UMIN( heal, victim->max_hit - victim->hit );
    gain_condition( victim, COND_FULL, -4 );

    lose_stamina( ch, heal / 8, FALSE, TRUE );
    lose_stamina( victim, heal / 4, FALSE, TRUE );

    if ( gain_health( victim, heal, FALSE ) )
	act( "$n look$% very much better.", victim, NULL, NULL, TO_ALL );

    gain_health( victim, heal, FALSE );
    return;
}

SPELL( spell_refreshment )
{
    CHAR_DATA *victim;
    int heal;

    heal = table_normal( 2, strength );
    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 
    gain_condition( victim, COND_FULL, -1 );

    lose_stamina( ch, heal / 4, FALSE, TRUE );
    if ( gain_stamina( victim, heal, FALSE ) )
	act( "$n look$% more rested.", victim, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_cure_blindness )
{
    CHAR_DATA *victim;

    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL );
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 4, FALSE, TRUE );
    gain_condition( victim, COND_FULL, -1 );

    if ( cure_condition( victim, BODY_BLIND, strength ) )
	act( "The blackness in your eyes goes away, allowing you to see again.",
	    ch, NULL, victim, TO_VICT );
    return;
}

SPELL( spell_cure_disease )
{
    CHAR_DATA *victim;

    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 4, FALSE, TRUE );
    gain_condition( victim, COND_FULL, -1 );

    if ( cure_condition( victim, BODY_DISEASE, strength ) )
	act( "$n look$% less ill.", victim, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_cure_poison )
{
    CHAR_DATA *victim;

    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL );
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 4, FALSE, TRUE );
    gain_condition( victim, COND_FULL, -1 );

    if ( cure_condition( victim, BODY_POISON, strength ) )
	act( "The burning in your veins goes away.", ch, NULL, victim, TO_VICT );
    return;
}

SPELL( spell_bone_knitting )
{
    CHAR_DATA *victim;

    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL );
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 4, FALSE, TRUE );
    gain_condition( victim, COND_FULL, -1 );

    if ( cure_condition(victim, BODY_RIGHT_LEG, strength) )
    {
	act( "$o right leg seems more straight now.", victim, NULL,
	    NULL, TO_ALL );
	return;
    }
    if ( cure_condition(victim, BODY_LEFT_LEG, strength) )
    {
	act( "$o left leg seems more straight now.", victim, NULL,
	    NULL, TO_ALL );
	return;
    }
    if ( cure_condition(victim, BODY_RIGHT_ARM, strength) )
    {
	act( "$o right arm seems more straight now.", victim, NULL,
	    NULL, TO_ALL );
	return;
    }
    if ( cure_condition(victim, BODY_LEFT_ARM, strength) )
    {
	act( "$o left arm seems more straight now.", victim, NULL,
	    NULL, TO_ALL );
	return;
    }

    send_to_char( "You were not able to Heal any bones.\n\r", ch );
    return;
}

SPELL( spell_clot )
{
    CHAR_DATA *victim;

    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 4, FALSE, TRUE );
    gain_condition( victim, COND_FULL, -1 );

    if ( cure_condition(victim, BODY_BLEEDING, strength) )
    {
	act( "$o bleeding has stopped.", victim, NULL, NULL, TO_ALL );
	return;
    }

    send_to_char( "You were not able to Heal any bleeding.\n\r", ch );
    return;
}

SPELL( spell_probe )
{
    CHAR_DATA *victim;
    int percent = 100;
    char buf[MAX_STRING_LENGTH];
        
    victim = (CHAR_DATA *) vo;
    if ( victim->position > POS_SLEEPING )
	act( "$n shiver$% slightly.", victim, NULL, NULL, TO_ALL ); 
    lose_stamina( victim, UMAX(1, stamina_cost( ch, sn )) / 10, FALSE, TRUE );

    if ( is_affected(victim, skill_lookup( "poison" ))
    ||   IS_AFFECTED(victim, AFF_POISON) )
	act( "$N seems poisoned.", ch, NULL, victim, TO_CHAR );

    if ( is_affected(victim, skill_lookup( "disease" ))
    ||   IS_AFFECTED(victim, AFF_PLAGUE) )
	act( "$N seems sick.", ch, NULL, victim, TO_CHAR );

    if ( is_affected(victim, skill_lookup( "blindness" ))
    ||   IS_AFFECTED(victim, AFF_BLIND) )
	act( "$N seems blind.", ch, NULL, victim, TO_CHAR );

    if ( is_affected(victim, skill_lookup( "haste" ))
    ||   IS_AFFECTED(victim, AFF_HASTE) )
	act( "$N's metabolism seems much faster.",
	    ch, NULL, victim, TO_CHAR );

    if ( is_affected(victim, skill_lookup( "slow" ))
    ||   IS_AFFECTED(victim, AFF_SLOW) )
	act( "$N's metabolism seems much slower.",
	    ch, NULL, victim, TO_CHAR ); 

    if ( IS_SET(victim->body, BODY_RIGHT_LEG) )
	act( "$N's right leg is broken.", ch, NULL, victim, TO_CHAR );
    if ( IS_SET(victim->body, BODY_LEFT_LEG) )
	act( "$N's left leg is broken.", ch, NULL, victim, TO_CHAR );
    if ( IS_SET(victim->body, BODY_RIGHT_ARM) )
	act( "$N's right arm is broken.", ch, NULL, victim, TO_CHAR );
    if ( IS_SET(victim->body, BODY_LEFT_ARM) )
	act( "$N's left arm is broken.", ch, NULL, victim, TO_CHAR );
    if ( IS_SET(victim->body, BODY_BLEEDING) )
	act( "$N is bleeding badly.", ch, NULL, victim, TO_CHAR );

    if ( victim->max_hit != 0 )
	percent = victim->hit * 100 / victim->max_hit;

    sprintf( buf, "%s", PERS(victim, ch) );

    if (percent >= 100)
	strcat( buf, " is in `2excellent condition`n and");
    else if (percent >= 90)
	strcat( buf, " has a `2few scratches`n and");
    else if (percent >= 75)
	strcat( buf," has some small wounds and bruises and");
    else if (percent >=  50)
	strcat( buf, " has quite a few wounds and");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches and");
    else if (percent >= 15)
	strcat ( buf, " looks `3pretty hurt`n and");
    else if (percent >= 0 )
	strcat (buf, " is in `3awful condition`n and");
    else
	strcat(buf, " is `3bleeding to death`n and");

    percent = 100;
    if ( victim->max_stamina != 0 )
	percent = victim->stamina * 100 / victim->max_stamina;

    if (percent >= 100)
	strcat( buf, " is `2very rested.`n\n\r");
    else if (percent >= 90) 
	strcat( buf, " is `2quite rested.`n\n\r");  
    else if (percent >= 75)
	strcat( buf," is showing some signs of fatigue.\n\r");
    else if (percent >=  50)
	strcat( buf, " is looking pretty tired.\n\r");
    else if (percent >= 30)
	strcat( buf, " is looking very tired.\n\r");   
    else if (percent >= 15)
	strcat ( buf, " seems `3fatigued.`n\n\r");   
    else if (percent >= 0 )
	strcat (buf, " looks `3exhausted.`n\n\r");
    else
	strcat(buf, " is `3on the verge of collapsing.`n\n\r");

    send_to_char( buf, ch  );
    return;
}

    
SPELL( spell_skim )
{
    OBJ_DATA *portal;
    ROOM_INDEX_DATA *from_room, *to_room;
    void *vobj;
        
    from_room = ch->in_room;
    to_room = (ROOM_INDEX_DATA *) vo;
    
    if ( to_room != NULL && (!can_see_room( ch, to_room )
    ||   !can_see_room( ch, from_room )
    ||   IS_SET( to_room->room_flags, ROOM_SAFE )
    ||   IS_SET( to_room->room_flags, ROOM_PRIVATE )
    ||   IS_SET( to_room->room_flags, ROOM_SOLITARY )
    ||   IS_SET( to_room->room_flags, ROOM_NO_RECALL )
    ||   IS_SET( from_room->room_flags,ROOM_NO_RECALL )
    ||   IS_SET( to_room->room_flags, ROOM_WARDED )
    ||   IS_SET( to_room->room_flags, ROOM_STEDDING )
    ||	 !IS_SET( to_room->area->area_flags, AREA_COMPLETE )
    ||	 is_room_affected( to_room, skill_lookup("ward room") )) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + strength / 25;
    portal->value[3] = (to_room != NULL) ? to_room->vnum : -1;
    portal->owner = ch;
    SET_BIT(portal->value[2], GATE_DESTROY);
    SET_BIT(portal->extra_flags, ITEM_CHANNELED);

    vobj = (void *) portal;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    obj_to_room(portal,from_room);

    act("$p snaps into existence.",ch,portal,NULL,TO_ALL);
}

SPELL( spell_travel )
{
    OBJ_DATA *portal, *portal_match;
    ROOM_INDEX_DATA *to_room, *from_room;
    void *vobj;

    from_room = ch->in_room;
    to_room = (ROOM_INDEX_DATA *) vo;

    if ( to_room != NULL && (!can_see_room( ch,to_room )
    ||   !can_see_room( ch,from_room )
    ||   IS_SET( to_room->room_flags, ROOM_SAFE )
    ||   IS_SET( to_room->room_flags, ROOM_PRIVATE )
    ||   IS_SET( to_room->room_flags, ROOM_SOLITARY )
    ||   IS_SET( to_room->room_flags, ROOM_NO_RECALL )
    ||   IS_SET( from_room->room_flags,ROOM_NO_RECALL )
    ||   IS_SET( to_room->room_flags, ROOM_WARDED ) 
    ||   IS_SET( to_room->room_flags, ROOM_STEDDING )
    ||	 !IS_SET( to_room->area->area_flags, AREA_COMPLETE )
    ||	 is_room_affected( to_room, skill_lookup("ward room") )) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( to_room == NULL )
        to_room = get_random_room( ch );
 
    /* portal one */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + strength / 30; 
    portal->value[3] = to_room->vnum;
    portal->owner = ch;
    SET_BIT(portal->value[2], GATE_DESTROY);
    SET_BIT(portal->extra_flags, ITEM_CHANNELED);

    obj_to_room(portal,from_room);
 
    act("$p snaps into existence.", ch, portal, NULL, TO_ALL);
  
    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;
    
    /* portal two */
    portal_match = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal_match->timer = 1 + strength / 15;
    portal_match->value[3] = from_room->vnum;  
    portal_match->owner = ch;
    SET_BIT(portal_match->value[2], GATE_DESTROY);

    portal_match->match = portal;
    portal->match = portal_match;
    obj_to_room(portal_match,to_room);

    vobj = (void *) portal;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    if (to_room->people != NULL)
        act("$p snaps into existence.", to_room->people, portal,
	    NULL, TO_ALL);
}
    
SPELL( spell_reveal_invisible )
{
    CHAR_DATA *victim;
    AFFECT_DATA *paf, *paf_next;

    victim = (CHAR_DATA *) vo;
 
    if ( !IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !is_affected(victim, gsn_invis) )
    {
        send_to_char( "Nothing happens.\n\r", ch );
        return;
    }

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == gsn_invis
	&&   break_weave(strength, paf->strength) )
	{
	    affect_strip( ch, gsn_invis );
	    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    break;
	}
    }
    return;
}       
        
SPELL( spell_ward_room )
{
    AFFECT_DATA af;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 25 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= AFF_WARDED;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act_channel( "The room is traced in blue for a second.", ch, NULL, NULL,
	TO_ALL );
    return;
}       
        
SPELL( spell_ward_person )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already warded.\n\r",ch);
        else
          act("$N is already warded.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_normal( strength, 25 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= AFF_WARDED;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_char( victim, &af );
    act_channel( "A blue outline surrounds $N for a second.", ch, NULL,
	victim, TO_ALL );
    return;
}
    
SPELL( spell_create_flame_sword )
{
    OBJ_DATA *sword;
    void *vobj;

    if ( get_eq_char(ch, WEAR_WIELD) != NULL )
    {
        send_to_char( "You may not create a flame sword while wielding a weapon.\n\r", ch );
        return;
    }

    sword = create_object( get_obj_index(4223), 0 );
    sword->timer = UMAX( 1, strength / 15 );
    sword->value[1] = URANGE( 1, strength / 10, 10 );
    sword->value[2] = URANGE( 1, strength / 12, 12 );
    sword->owner = ch;
    SET_BIT(sword->extra_flags, ITEM_CHANNELED);
    obj_to_char( sword, ch );
    equip_char( ch, sword, WEAR_WIELD );

    vobj = (void *) sword;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    act( "A sword of flame forms in $o hand.", ch, NULL, NULL, TO_ALL );
    return;
}   

SPELL( spell_create_air_sword )
{
    OBJ_DATA *sword;
    void *vobj;

    if ( get_eq_char(ch, WEAR_WIELD) != NULL )
    {
        send_to_char( "You may not create an air sword while wielding a weapon.\n\r", ch );
        return;
    }

    sword = create_object( get_obj_index(4224), 0 );
    sword->timer = UMAX( 1, strength / 15 );
    sword->value[1] = URANGE( 1, strength / 10, 10 );
    sword->value[2] = URANGE( 1, strength / 12, 12 );
    sword->owner = ch;
    SET_BIT(sword->extra_flags, ITEM_CHANNELED);
    obj_to_char( sword, ch );
    equip_char( ch, sword, WEAR_WIELD );

    vobj = (void *) sword;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    act( "An irridescent blue sword forms in $o hand.", ch, NULL, NULL,
	TO_ALL );
    return;
}

 
SPELL( spell_flame_wave )
{
    ROOM_INDEX_DATA *pRoom;   
    CHAR_DATA *vch, *vch_next;
    int door = 0;
    int rooms = 0;
    int dam;

    pRoom = ch->in_room;    
    while ( door != -1 && rooms < strength / 8 )
    {
	if ( rooms == 0 )
	{
	    act( "Waves of flame flow forward from $o outstretched arms.",
		ch, NULL, NULL, TO_ALL );
	}
        rooms += number_range( 1, 2 );
        for ( vch = pRoom->people; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next_in_room;

            if ( is_safe_spell(ch, vch, TRUE) )
		continue;

	    if ( ch == vch )
		continue;

            /* Oh .. poor SOB */
            dam = table_normal( 31, strength );
	    if ( !spell_damage(ch, vch, dam, sn, DAM_FIRE) )
	    {
		if ( number_percent() >= 65 )
		{
		    add_fight_list( vch );
		    vch->hunting = ch;
		}
	    }
        }
        strength -= number_fuzzy( 2 );
        door = find_spell_door( pRoom, target_name, sn );
        if ( door != -1 )
	{
	    if ( pRoom->people != NULL )
		act( "Waves of flame flow out $twards.", pRoom->people,
		    dir_name[door], NULL, TO_ALL );
            pRoom = pRoom->exit[door]->u1.to_room;
	    if ( pRoom->people != NULL )
		act( "Waves of flame flow in from $twards.", pRoom->people,
		    dir_name[rev_dir[door]], NULL, TO_ALL );
	}
    }    
    return;
}


SPELL( spell_earth_wave )
{
    ROOM_INDEX_DATA *pRoom;   
    CHAR_DATA *vch, *vch_next;
    int door = 0;
    int rooms = 0;
    int dam;
    int chance;
    
    pRoom = ch->in_room;    
    while ( door != -1 && rooms < strength / 8 )
    {
        door = find_spell_door( pRoom, target_name, sn );
	if ( pRoom->people != NULL )
	{
	    if ( door == -1 )
		act( "The ground rumbles.", pRoom->people, NULL, NULL,
		   TO_ALL );
	    else
		act( "The ground rumbles and churns $twards.", pRoom->people,
		    dir_name[door], NULL, TO_ALL );
	}
	rooms += number_range( 1, 2 );
	for ( vch = pRoom->people; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next_in_room;
	    chance = strength * 3 / 4;

            if ( is_safe_spell(ch, vch, TRUE) )
                continue;

	    if ( ch == vch )
		continue;

	    chance -= vch->carry_weight / 20;
	    chance -= get_curr_stat(vch,STAT_DEX) * 3/4;

	    if (IS_SET(vch->off_flags,OFF_FAST))
	        chance -= 10;

	    if ( number_percent() < chance 
	    &&   vch != ch )
	    {
		act( "$n tumble$% to the ground.", vch, NULL, NULL,
		    TO_ALL );
		if ( vch->position == POS_MOUNTED )
		{
		    vch->mount->rider = NULL;
		    vch->mount = NULL;
		}
		WAIT_STATE(vch, 2 );
		vch->position = POS_SITTING;

            	/* Oh .. poor SOB */
		dam = table_normal( 23, strength );
		if ( IS_AFFECTED(vch, AFF_FLYING) )
		    dam = 0;

		if ( !spell_damage(ch, vch, dam, sn, DAM_BASH) )
		{
		    if ( number_percent() >= 65 )
		    {
			add_fight_list( vch );
			vch->hunting = ch;
		    }
		    if ( door != -1
		    &&   vch->position < POS_STANDING )
		    {
			/* Earth wave pushes 'em along */
			act( "The rolling waves of earth push $n $twards.", vch,
			    dir_name[door], NULL, TO_ALL );
			move_char( vch, door, FALSE );
		    }
	        }
	    }
	}
	strength -= number_fuzzy( 2 );
	if ( door != -1 )
	{
	    if ( pRoom->people != NULL )
		act( "The ground churns, moving $twards.",
		    pRoom->people, dir_name[door], NULL, TO_ALL );
	    pRoom = pRoom->exit[door]->u1.to_room;
	}
    }    
    return;
}

SPELL( spell_windstorm )
{
    CHAR_DATA *vch, *vch_next;
    int door = 0;
    int rooms = 0;
    int dam;
    int chance;

    act( "Violent winds swirl about the room!", ch, NULL, NULL, TO_ALL );
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
	door = -1;
	chance = strength * 4 / 5;

        if ( is_safe_spell(ch, vch, TRUE)
        &&   number_percent() < 5 + rooms )
            continue;

	chance -= vch->carry_weight / 20;
	chance -= get_curr_stat(vch,STAT_STR) * 3/4;

	if (IS_SET(vch->off_flags,OFF_FAST))
	    chance -= 10;

	if ( number_percent() < chance 
	&&   vch != ch )
	{
	    act( "The winds batter you!", vch, NULL, NULL, TO_CHAR );
	    WAIT_STATE(vch, 2 );
	    vch->position = POS_SITTING;

            /* Oh .. poor SOB */
	    dam = table_normal( 26, strength );
	    if ( !spell_damage(ch, vch, dam, sn, DAM_BASH) )
	    {
		if ( number_percent() < 50 )
		    door = get_random_door( ch->in_room );

		if ( door != -1 
		&&   vch->position < POS_STANDING )
		{
		    act( "The winds toss $n $twards.", vch,
			dir_name[door], NULL, TO_ALL );
		    move_char( vch, door, FALSE );
		}
	    }
	}
    }
    return;
}

SPELL( spell_fireball )
{
    CHAR_DATA *vch, *vch_next;
    int dam;

    dam = table_normal( 14, strength );

    act( "$n open$% $s hand and a gush of flame spews forth.", ch, NULL,
	NULL, TO_ALL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	if ( is_safe_spell(ch, vch, TRUE) )
	    continue;

	spell_damage( ch, vch, dam, sn, DAM_FIRE );
    }
    return;
}

SPELL( spell_air_shield )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

    vch = (CHAR_DATA *) vo;

    if ( is_affected(vch, sn) )
    {
	act( "Flows of Air are already protecting $N.", ch, NULL, vch,
	    TO_CHAR );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 33 );
    af.location		= APPLY_AC;
    af.modifier		= table_multiplier( strength, 55 );
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

/* Air Armor - new version of sanct :) */
SPELL( spell_air_armor )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

    vch = (CHAR_DATA *) vo;

    if ( is_affected(vch, sn) )
    {
	act( "$N is already surrounded by armor of Air.", ch, NULL, vch,
	    TO_CHAR );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 12 );
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector	= AFF_AIR_ARMOR;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

SPELL( spell_wind_spear )
{
    CHAR_DATA *vch;
    int dam;

    vch = (CHAR_DATA *) vo;

    act( "$n scream$%, as something rends a hole in $s skin!", vch,
	NULL, NULL, TO_ALL );
    dam = table_normal( 12, strength );
    spell_damage( ch, vch, dam, sn, DAM_PIERCE );
    return;
}

SPELL( spell_create_spring )
{
    OBJ_DATA *spring;
    void *vobj;

    switch (ch->in_room->sector_type)
    {
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	    send_to_char( "There is water all around you.\n\r", ch );
	    return;
	case SECT_UNUSED:
	case SECT_AIR:
	case SECT_CITY:
	case SECT_INSIDE:
	    send_to_char( "You cannot draw forth water here.\n\r", ch );
	    return;
    }

    if ( !IS_SET(ch->in_room->resources, RES_WATER) )
    {
	strength = strength / 2;
	if ( number_percent() > strength )
	{
	    send_to_char( "You do not find any water to draw up.\n\r", ch );
	    return;
	}
    }
    REMOVE_BIT(ch->in_room->resources, RES_WATER);

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = strength / 5;
    spring->owner = ch;
    SET_BIT(spring->extra_flags, ITEM_CHANNELED);
    obj_to_room( spring, ch->in_room );

    vobj = (void *) spring;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    act_channel( "$n begin$% to draw up water from the ground.", ch, NULL,
	NULL, TO_ALL );
    act( "A small spring begins to bubble and flow.", ch, NULL, NULL,
	TO_ALL );
    return;
}

SPELL( spell_create_water )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "It is unable to hold water.\n\r", ch );
        return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
        send_to_char( "It contains some other liquid.\n\r", ch );
        return;
    }

    water = UMIN(
                strength / 8 * (weather_info.sky >= SKY_RAINING ? 4 : 2),
                obj->value[0] - obj->value[1]
                );

    act_channel( "$n draw$% water from the air, and directs it into $p.",
	ch, obj, NULL, TO_ALL );

    if ( water > 0 )
    {
        obj->value[2] = LIQ_WATER;
        obj->value[1] += water;
        if ( !is_name( "water", obj->name ) )
        {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "%s water", obj->name );
            free_string( obj->name );
            obj->name = str_dup( buf );
        }
        act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }
    return;
}

SPELL( spell_lightning_bolt )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	successful_cast = FALSE;
	return;
    }

    dam = table_normal( 16, strength );

    act( "$n raise$% $s hand and a bolt of lightning falls from the sky onto $N.",
	ch, NULL, victim, TO_ALL );

    spell_damage( ch, victim, dam, sn, DAM_LIGHTNING );
    return;
}

SPELL( spell_invisibility )
{
    CHAR_DATA *vch = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(vch, AFF_INVISIBLE) )
        return;

    act( "$n snap$% out of existence.", vch, NULL, NULL, TO_ALL );
    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 25 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= AFF_INVISIBLE;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}
 
SPELL( spell_mass_invisibility )
{
    CHAR_DATA *gch;
    AFFECT_DATA af;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
            continue;
        act( "$n snap$% out of existence.", gch, NULL, NULL, TO_ALL );
	af.type		= sn;
	af.strength	= strength;
	af.duration	= table_multiplier( strength, 12 );
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_INVISIBLE;
	af.bitvector_2	= 0;
	af.owner	= AFF_OWNER(ch);
	SET_SEX( af, ch );
	affect_to_char( gch, &af );
    }
    return;
}

SPELL( spell_ice_ball )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = table_normal( 4, strength );

    act( "A ball of ice forms in the air and flies at $N.",
	ch, NULL, victim, TO_ALL );

    spell_damage( ch, victim, dam, sn, DAM_BASH );
    return;
}

SPELL( spell_sleep )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
        return;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= URANGE( 1, table_normal(strength, 40), 10 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= AFF_SLEEP;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
        send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
        act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
        victim->position = POS_SLEEPING;
    }
    if ( IS_GRASPING(victim) )
	do_release( victim, "" );
    return;
}


SPELL( spell_ward_object )
{
    AFFECT_DATA af;
    OBJ_DATA *vobj;

    vobj = (OBJ_DATA *) vo;
    affect_from_index( vobj );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 10 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_obj( vobj, &af );
    return;
}

SPELL( spell_earthquake )
{
    CHAR_DATA *vch;  
    CHAR_DATA *vch_next;
    int dam;

    dam = table_normal( 13, strength );

    act( "The earth trembles beneath your feet!", ch, NULL, NULL, TO_ALL );
    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;
        if ( vch->in_room == NULL )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( vch != ch
	    &&   !is_safe_spell(ch, vch, TRUE) )
	    {
		if ( IS_AFFECTED(vch, AFF_FLYING) )
		    spell_damage( ch, vch, 0, sn, DAM_BASH );
		else
		    spell_damage( ch, vch, dam, sn, DAM_BASH );
	    }
	    continue;   
        }
    
        if ( vch->in_room->area == ch->in_room->area )
            send_to_char( "The earth trembles and shivers.\n\r", vch );
    }     
    return;
}

SPELL( spell_shape_change )
{
    AFFECT_DATA af;

    if ( IS_NPC(ch) )
	return;

    if ( target_name[0] == '\0' )
    {
	send_to_char( "What form do you wish to appear as?\n\r", ch );
	return;
    }

    act("$o form blurs and changes into something else.",
	ch, NULL, NULL, TO_ALL );

    remove_shape( ch );
    ch->pcdata->new_name	= str_dup( target_name );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 25 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= AFF_SHAPE_CHANGE;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( ch, &af );
    return;
}

SPELL( spell_wind_push )
{
    CHAR_DATA *vch, *vch_next;
    char *argument;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int door = 0;
    int chance;

    argument = one_argument( target_name, arg1 );
    one_argument( argument, arg2 );

    if ( (vch = get_char_room( ch, arg1 )) != NULL )
    {
	chance = strength * 3 / 2 ;

	chance -= vch->carry_weight / 30;
	chance -= get_curr_stat(vch,STAT_STR) * 2/3;

	if (IS_SET(vch->off_flags,OFF_FAST))
	    chance -= 10;

	if ( number_percent() > chance )
	{
	    send_to_char( "Your winds were not strong enough.\n\r", ch );
	    return;
	}

	if ( (door = find_spell_door(ch->in_room, arg2, sn)) == -1 )
	{
	    send_to_char( "There is no way out in that direction.\n\r", ch );
	    return;
	}
	act( "The winds rise suddenly, pushing $N $t.", ch, dir_name[door],
	    vch, TO_ALL );
	move_char( vch, door, FALSE );
	return;
    }

    act( "The winds rise suddenly.", ch, NULL, NULL, TO_ALL );
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
	door = -1;
	chance = strength * 3 / 5;

        if ( is_safe_spell(ch, vch, TRUE) )
            continue;

	chance -= vch->carry_weight / 20;
	chance -= get_curr_stat(vch,STAT_STR) * 3/4;

	if (IS_SET(vch->off_flags,OFF_FAST))
	    chance -= 10;

	if ( number_percent() < chance )
	    door = get_random_door( ch->in_room );

	if ( door != -1 
	&&   vch->position < POS_STANDING )
	{
	    act( "The winds push $n $twards.", vch,
		dir_name[door], NULL, TO_ALL );
	    move_char( vch, door, FALSE );
	}
    }
    return;
}

SPELL( spell_blindness )
{
    CHAR_DATA *vch = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   if ( IS_AFFECTED(vch, AFF_BLIND) )
	return;

    send_to_char( "You can't see!  You've been blinded!\n\r", vch );
    act( "$n stumbles around blindly.", vch, NULL, NULL, TO_ROOM );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_normal( strength, 10 );
    af.modifier		= -4;
    af.location		= APPLY_HITROLL;
    af.bitvector	= AFF_BLIND;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

SPELL( spell_poison )
{
    CHAR_DATA *vch = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(vch, AFF_POISON) )
	return;

    send_to_char( "You are suddenly very ill.\n\r", vch );
    act( "$n is suddenly ill.", vch, NULL, NULL, TO_ROOM );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 15 );
    af.modifier		= table_multiplier( strength, 8 ) * -1;
    af.location		= APPLY_STR;
    af.bitvector	= AFF_POISON;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

SPELL( spell_disease )
{
    CHAR_DATA *vch = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(vch, AFF_PLAGUE) )
	return;

    act( "$n scream$% as sores break out over $s body.", vch, NULL, NULL,
	TO_ALL );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 10 );
    af.modifier		= table_multiplier( strength, 20 ) * -1;
    af.location		= APPLY_STR;
    af.bitvector	= AFF_PLAGUE;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

SPELL( spell_illusion )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];   
    char buf3[MAX_STRING_LENGTH];   
    CHAR_DATA *vch;

    sprintf( buf1, "%s\n\r", target_name );
    sprintf( buf2, "$n makes an illusion . . . %s", target_name );
    sprintf( buf3, "$N makes an illusion . . . %s\n\r", target_name );
    buf1[0] = UPPER(buf1[0]);

    wiznet(buf3,ch,NULL,WIZ_SECURE,0,0);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if ( can_channel(vch, 1) && TRUE_SEX(vch) == TRUE_SEX(ch) )
	    act( buf2, ch, NULL, vch, TO_VICT );
	else
	    send_to_char( buf1, vch );
    }
    send_to_char( buf1, ch );
    return;
}

SPELL( spell_ventriloquate )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];   
    char buf3[MAX_STRING_LENGTH];   
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    
    target_name = one_argument( target_name, speaker );
    
    sprintf( buf1, "%s says '%s'.\n\r", speaker, target_name);
    sprintf( buf2, "$n makes %s say '%s'.\n\r", speaker, target_name);
    sprintf( buf3, "$N makes %s say '%s'.\n\r", speaker, target_name);
    buf1[0] = UPPER(buf1[0]);

    wiznet(buf3,ch,NULL,WIZ_SECURE,0,0);
    
    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if ( !is_name( speaker, vch->name ) )
	{
	    if ( can_channel(vch, 1) && TRUE_SEX(vch) == TRUE_SEX(ch) )
		act( buf2, ch, NULL, vch, TO_VICT );
	    else
		send_to_char( buf1, vch );
	}
    }
    send_to_char( buf1, ch ); 
    return;
}

SPELL( spell_charm )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }
    
    if ( CHARM_SET(ch)
    ||   CHARM_SET(victim)
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   check_stat(victim, STAT_WIS, 2) )
	return;

    if (ch->leader != NULL)
    {
	CHAR_DATA *ldr = ch->leader;
	while (ldr != NULL)
	{
	    if (ldr == victim)
	    {
		stop_follower(ch);
		break;
	    }
	    ldr = ldr->leader;
	}
    }

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( number_fuzzy(strength), 12 );
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector	= AFF_CHARM;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_char( victim, &af );
    if ( ch != victim )
	act("$N looks at $n with adoring eyes.",ch,NULL,victim,TO_ALL);
    return;
}

SPELL( spell_lash )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 2, strength );
    victim = (CHAR_DATA *) vo;

    act( "$N scream$% in pain as something lashes at $M.",
	ch, NULL, victim, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_SLASH );

    return;
}

SPELL( spell_rock_shower )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 6, strength );
    victim = (CHAR_DATA *) vo;

    act( "An explosion of large stones burst forth into the air and showers down upon $N.",
	victim, NULL, NULL, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_BASH );

    return;
}

SPELL( spell_air_blade )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 8, strength );
    victim = (CHAR_DATA *) vo;

    act( "$n falter$% slightly as something makes a large gash on $m.",
	victim, NULL, NULL, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_BASH );

    return;
}

SPELL( spell_steam )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 10, strength );
    victim = (CHAR_DATA *) vo;

    act( "Bouts of steam form around $N, scalding $M.", ch, NULL, victim,
	TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_FIRE );

    return;
}

SPELL( spell_control_weather )
{

    if ( !str_prefix( target_name, "better" ) )
	weather_info.change += dice( strength / 3, 4 );
    else if ( !str_prefix( target_name, "worse" ) )
	weather_info.change -= dice( strength / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}
    
SPELL( spell_awareness )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_AWARENESS) )
    {
	if (victim == ch)
	    send_to_char("You are already as alert as you can be.\n\r",ch);
	else
	    act( "$N can already as alert as $E can be.", ch, NULL,
		victim, TO_CHAR );
	return;
    }
    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 75 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= AFF_AWARENESS;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX(af, ch);
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

SPELL( spell_detect_invisible )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
	if (victim == ch)
	    send_to_char("You can already see invisible.\n\r",ch);
	else
	    act("$N can already see invisible things.",ch,NULL,
		victim,TO_CHAR);
	return;
    }
    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 90 );
    af.modifier		= 0;
    af.location		= APPLY_NONE;
    af.bitvector	= AFF_DETECT_INVIS;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX(af, ch);
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

SPELL( spell_strengthen )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, sn) )
    {
	if ( victim == ch )
	    send_to_char( "You are already as strong as you can get!\n\r", ch );
	else
	    act( "$N can't get any stronger.", ch, NULL, victim, TO_CHAR );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 75 );
    af.location 	= APPLY_STR;
    af.modifier		= 1 + (strength >= 25) + (strength >= 65) + (strength >= 85);
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( victim, &af );
    act("$o muscles surge with heightened power.", victim, NULL, NULL,
	TO_ALL);
    return;
}

SPELL( spell_haste )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)  
	    send_to_char( "You can't move any faster!\n\r", ch );
	else
	    act( "$N is already moving as fast as $e can.",
		ch, NULL, victim, TO_CHAR );
	return;
    }
    af.type		= sn;
    af.strength		= strength;
    if (victim == ch)
	af.duration	= table_multiplier( strength, 30 );
    else
	af.duration	= table_multiplier( strength, 15 );
    af.location		= APPLY_AGI;
    af.modifier		= 1 + (strength >= 25) + (strength >= 65) + (strength >= 85);
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( victim, &af );
    af.location		= APPLY_DEX;
    affect_to_char( victim, &af );
    act("$n seem$% to be moving more quickly.", victim, NULL, NULL, TO_ALL);
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

SPELL( spell_lightning_storm )
{
    AFFECT_DATA af;

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	successful_cast = FALSE;
	return;
    }

    if ( is_room_affected(ch->in_room, gsn_lightning_storm) )
    {
	send_to_char( "It is already storming here.\n\r", ch );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 33 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "The clouds begin to darken overhead.", ch, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_hail_storm )
{
    AFFECT_DATA af;

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	successful_cast = FALSE;
	return;
    }

    if ( is_room_affected(ch->in_room, gsn_hail_storm) )
    {
	send_to_char( "It is already hailing here.\n\r", ch );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 12 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "The clouds begin to darken overhead.", ch, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_slow )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW) )
    {
	if (victim == ch)  
	    send_to_char( "You can't move any slower!\n\r", ch );
	else
	    act( "$N is already moving as slow as $e can.",
		ch, NULL, victim, TO_CHAR );
	return;
    }
    af.type		= sn;
    af.strength		= strength;
    if (victim == ch)
	af.duration	= table_multiplier( strength, 30 );
    else
	af.duration	= table_multiplier( strength, 15 );
    af.location		= APPLY_AGI;
    af.modifier		= 1 + (strength >= 20) + (strength >= 45) + (strength >= 65);
    af.modifier		= -1 * af.modifier;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( victim, &af );
    af.location		= APPLY_DEX;
    affect_to_char( victim, &af );
    act("$n seem$% to be moving more slowly.", victim, NULL, NULL, TO_ALL);
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

SPELL( spell_weaken )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    if ( is_affected(victim, sn) )
    {
	if ( victim == ch )
	    send_to_char( "You are already as weak as you can get!\n\r", ch );
	else
	    act( "$N can't get any weaker.", ch, NULL, victim, TO_CHAR );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 75 );
    af.location 	= APPLY_STR;
    af.modifier		= 1 + (strength >= 25) + (strength >= 65) + (strength >= 85);
    af.modifier		= -1 * af.modifier;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( victim, &af );
    act("$o muscles seem flabbier.", victim, NULL, NULL,
	TO_ALL);
    return;
}

SPELL( spell_earth_shield )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

    vch = (CHAR_DATA *) vo;

    if ( is_affected(vch, sn) )
    {
	act( "Flows of Air and Earth are already protecting $N.", ch, NULL,
	    vch, TO_CHAR );
	return;
    }

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 33 );
    af.location		= APPLY_AC;
    af.modifier		= table_multiplier( strength, 65 );
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );
    return;
}

SPELL( spell_frost_wave )
{
    ROOM_INDEX_DATA *pRoom;   
    CHAR_DATA *vch, *vch_next;
    int door = 0;
    int rooms = 0;
    int dam;

    pRoom = ch->in_room;    
    while ( door != -1 && rooms < strength / 10 )
    {
	if ( rooms == 0 )
	{
	    act( "The temperature drops and wind begins blowing snow and ice around.",
		ch, NULL, NULL, TO_ALL );
	}
        rooms += number_range( 1, 2 );
        for ( vch = pRoom->people; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next_in_room;

            if ( is_safe_spell(ch, vch, TRUE) )
                continue;

	    if ( vch == ch )
		continue;
      
            /* Oh .. poor SOB */
            dam = table_normal( 28, strength );
	    if (!spell_damage(ch, vch, dam, sn, DAM_COLD) )
	    {
		if ( number_percent() >= 65 )
		{
		    add_fight_list( vch );
		    vch->hunting = ch;
		}
	    }
        }
        strength -= number_fuzzy( 2 );
        door = find_spell_door( pRoom, target_name, sn );
        if ( door != -1 )
	{
	    if ( pRoom->people != NULL )
		act( "Piles of snow and shards of ice blow out $twards.", pRoom->people,
		    dir_name[door], NULL, TO_ALL );
            pRoom = pRoom->exit[door]->u1.to_room;
	    if ( pRoom->people != NULL )
		act( "Piles of snow and shards of ice blow in from $twards.", pRoom->people,
		    dir_name[rev_dir[door]], NULL, TO_ALL );
	}
    }    
    return;
}

SPELL( spell_detect_ability )
{
    const char * strengths[13] =
    {
	"none", "almost none", "a trickle of", "very little", "weak",
	"nominal", "moderate", "some", "significant",
	"strong", "very strong", "great", "legendary"
    };
    CHAR_DATA *victim;
    int channel, i;
    char buf[MAX_STRING_LENGTH];
    const char	*	power_name[] =
    {
	"earth", "air", "fire", "water","spirit"
    };
            
    victim = (CHAR_DATA *) vo;
    for ( i = 0; i < 5; i++ )
    {
	channel = victim->channel_max[i];
	sprintf( buf, "$N has " );
	if ( IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL)
	||   channel < 1
	||   TRUE_SEX(ch) != TRUE_SEX(victim) )
	    strcat( buf, "no" );
	else
	    strcat( buf, strengths[channel / 10 + 1] );
	strcat( buf, " strength in " );
	strcat( buf, power_name[i] );
	strcat( buf, "." );
	act( buf, ch, NULL, victim, TO_CHAR );
    }
    return;
}

SPELL( spell_hide_channeling )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED_2(ch, AFF_HIDE_CHANNEL) )
        return;

    act_channel( "The glow of $t no longer surrounds $n.", ch,
	SOURCE(ch), NULL, TO_ALL );

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 20 );
    af.location		= APPLY_NONE;
    af.modifier		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_HIDE_CHANNEL;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( ch, &af );
    return;
}

SPELL( spell_flame_burst )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 33, strength );
    victim = (CHAR_DATA *) vo;

    act( "$N scream$^ in pain as flames burst around $M.",
	ch, NULL, victim, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_FIRE );

    return;
}

SPELL( spell_air_hammer )
{
    CHAR_DATA *vch;
    int dam, chance, mod;

    vch = (CHAR_DATA *) vo;
    dam = table_normal( 20, strength );
    chance = 80;

    mod = 1 - (strength >= 20) - (strength >= 50);
    chance += ( 10 * (strength >= 20) ) + ( 10 * (strength >= 50) );

    if ( check_stat(vch, STAT_CON, mod) )
	chance -= 20;

    act( "$N fall$% slightly, as if hit on the head.", ch, NULL, vch, TO_ALL );
    if ( !spell_damage(ch, vch, dam, sn, DAM_BASH) )
    {
	if ( number_percent() <= chance
	&&   ch->fighting == NULL )
	{
	    AFFECT_DATA af;
	    af.type		= sn;
	    af.strength		= strength;
	    af.duration		= 3;
	    af.location		= APPLY_NONE;
	    af.modifier		= 0;
	    af.bitvector	= AFF_SLEEP;
	    af.bitvector_2	= 0;
	    af.owner		= NULL;
	    af.flags		= AFFECT_NOTCHANNEL;
	    affect_join( vch, &af );

	    if ( IS_AWAKE(vch) )
	    {
		act( "$n collapse$% in a heap, unconscious.", vch,
		    NULL, NULL, TO_ALL );
		vch->position = POS_SLEEPING;
	    }
	    stop_fighting( vch, TRUE );
	    stop_fighting( ch, TRUE );
	}
	else
	    set_fighting( vch, ch );	   
    }
    return;
}

SPELL( spell_frostbite )
{
    CHAR_DATA *vch;
    int dam;

    vch = (CHAR_DATA *) vo;
    dam = table_normal( 18, strength );

    act( "Chilling air freezes $N.", ch, NULL, vch, TO_ALL );
    WAIT_STATE( vch, 3 );
    spell_damage( ch, vch, dam, sn, DAM_COLD );
    return;
}

SPELL( spell_earth_rending )
{
    CHAR_DATA *vch;
    int dam, chance, mod;

    vch = (CHAR_DATA *) vo;
    dam = table_normal( 18, strength );

    chance = 80 + (10 * (strength > 20)) + (10 * (strength > 50));
    mod    = 0  - (strength > 25) - (strength > 60);

    act( "The ground under $N rises, tossing $M to the side.", ch, NULL,
	vch, TO_ALL );
    WAIT_STATE( vch, 5 );

    if ( check_stat(ch, STAT_DEX, mod) )
	chance = chance * 2 / 3;

    if ( number_percent() <= chance )
    {
	act( "$E fall$^ heavily to the earth.", ch, NULL, vch, TO_ALL );
	vch->position = POS_SITTING;
    }
    spell_damage( ch, vch, dam, sn, DAM_BASH );
    return;
}

SPELL( spell_light_ball )
{
    OBJ_DATA *light;
    void *vobj;

    if ( get_eq_char(ch, WEAR_LIGHT) != NULL )
    {
        send_to_char( "You may not create a ball of light while using a light.\n\r", ch );
        return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    SET_BIT(light->extra_flags, ITEM_CHANNELED);
    light->timer = strength / 4;
    light->owner = ch;
    obj_to_char( light, ch );
    equip_char( ch, light, WEAR_LIGHT );

    vobj = (void *) light;
    add_weave_list( vobj, NODE_WEAVE_CREATE );

    act( "$p begins hovering near $n.",
	ch, light, NULL, TO_ALL );
    return;
}       

SPELL( spell_light_storm )
{
    CHAR_DATA *vch, *vch_next;

    act( "Rays of light fly out from in front of $n.", ch, NULL,
	NULL, TO_ALL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	AFFECT_DATA af;
	vch_next = vch->next_in_room;
	if ( is_safe_spell(ch, vch, TRUE) )
	    continue;

   	if ( IS_AFFECTED(vch, AFF_BLIND) )
	    continue;

	if ( check_stat(vch, STAT_AGI, 1) )
	    continue;

    	af.type			= sn;
    	af.strength		= strength;
    	af.duration		= 1;
    	af.modifier		= -3;
    	af.location		= APPLY_HITROLL;
    	af.bitvector		= AFF_BLIND;
    	af.bitvector_2		= 0;
    	af.owner		= NULL;
    	af.flags		= AFFECT_NOTCHANNEL;
    	affect_to_char( vch, &af );
	spell_damage( ch, vch, 0, sn, DAM_LIGHT );
    }
    return;
}

SPELL( spell_cause_pain )
{
    CHAR_DATA *victim;
    int dam;

    dam = table_normal( 6, strength );
    victim = (CHAR_DATA *) vo;

    act( "$n place$% $s hand on $N and $E collapses in pain.",
	ch, NULL, victim, TO_ALL );
    spell_damage( ch, victim, dam, sn, DAM_MENTAL );
    lose_stamina( victim, dam, FALSE, TRUE );
    break_con( victim );

    return;
}

SPELL( spell_delve )
{
    char arg[MAX_INPUT_LENGTH];
    int fRestrict = 0;

    if ( ch->in_room == NULL )
	return;

    target_name = one_argument( target_name, arg );
    if ( target_name[0] != '\0' )
    {
	if ( !str_prefix(target_name, "ores") )
	{
	    fRestrict = 1;
	    strength  += 25;
	}

	if ( !str_prefix(target_name, "gems") )
	{
	    fRestrict = 2;
	    strength  += 25;
	}
    }

    if ( number_percent() >= strength )
    {
	send_to_char( "You don't detect anything.\n\r", ch );
	return;
    }

    if ( (fRestrict == 1
    ||    fRestrict == 0)
    &&   IS_SET(ch->in_room->resources, RES_ORE) )
    {
	send_to_char( "This place is rich in ore.\n\r", ch );
	return;
    }

    if ( (fRestrict == 2
    ||    fRestrict == 0)
    &&   IS_SET(ch->in_room->resources, RES_GEMS) )
    {
	send_to_char( "This place is rich in gems.\n\r", ch );
	return;
    }

    send_to_char( "You don't detect anything.\n\r", ch );
    return;
}


SPELL( spell_wind_barrier )
{
    AFFECT_DATA af;

    if ( is_room_affected(ch->in_room, gsn_wind_barrier) )
    {
	send_to_char( "The winds are already swirling about the room.\n\r", ch );
	return;
    }

    if ( !IS_OUTSIDE(ch) )
	strength = strength / 2;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 16 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "Winds begin to swirl about the room.", ch, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_earth_barrier )
{
    AFFECT_DATA af;

    if ( is_room_affected(ch->in_room, sn) )
    {
	send_to_char( "Walls of earth already surround the room.\n\r", ch );
	return;
    }

    if ( IS_OUTSIDE(ch) )
	strength = strength / 2;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 12 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "Earth begins to pile up around the room.", ch, NULL, NULL,
	TO_ALL );
    return;
}


SPELL( spell_fire_wall )
{
    ROOM_INDEX_DATA *room;
    int door;
    AFFECT_DATA *paf;
    AFFECT_DATA af;
    char arg[MAX_INPUT_LENGTH];

    room = ch->in_room;

    one_argument( target_name, arg );

    if ( (door = find_exit(ch, arg)) == -1 )
    {
	send_to_char( "There is no way out in that direction.\n\r", ch );
	return;
    }

    if ( !IS_SET(room->exit[door]->rs_flags, EX_ISDOOR) )
    {
	send_to_char( "You can only create walls of fire within doorways.\n\r", ch );
	return;
    }

    for ( paf = room->affected; paf; paf = paf->next )
    {
	if ( paf->type == sn && paf->modifier == door )
	{
	    send_to_char( "A wall of fire already exists there.\n\r", ch );
	    return;
	}
    }

    if ( IS_OUTSIDE(ch) )
	strength = strength / 2;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 12 );
    af.modifier		= door;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "A wall of fire bursts up in the $tward exit.", ch,
	dir_name[door], NULL, TO_ALL );
    return;
}


SPELL( spell_ice_wall )
{
    ROOM_INDEX_DATA *room;
    int door;
    AFFECT_DATA *paf;
    AFFECT_DATA af;
    char arg[MAX_INPUT_LENGTH];

    room = ch->in_room;

    one_argument( target_name, arg );

    if ( (door = find_exit(ch, arg)) == -1 )
    {
	send_to_char( "There is no way out in that direction.\n\r", ch );
	return;
    }

    if ( !IS_SET(room->exit[door]->rs_flags, EX_ISDOOR) )
    {
	send_to_char( "You can only create walls of fire within doorways.\n\r", ch );
	return;
    }

    for ( paf = room->affected; paf; paf = paf->next )
    {
	if ( paf->type == sn && paf->modifier == door )
	{
	    send_to_char( "A wall of ice already exists there.\n\r", ch );
	    return;
	}
    }

    if ( !IS_OUTSIDE(ch) )
	strength = strength / 2;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 16 );
    af.modifier		= door;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= 0;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );

    affect_to_room( ch->in_room, &af );

    act( "A wall of ice slowly builds up in the $tward exit.", ch,
	dir_name[door], NULL, TO_ALL );
    return;
}


SPELL( spell_balefire )
{
    CHAR_DATA *vch;

    vch = (CHAR_DATA *) vo;
    act( "$n raise$% $o hand and point$% at $N.", ch, NULL, vch, TO_ALL );
    act( "A beam of liquid fire strikes $n and $e is ripped from the pattern!", vch, NULL, NULL, TO_ALL );
    return;
}


SPELL( spell_confusion )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

    vch = (CHAR_DATA *) vo;
    if ( IS_AFFECTED_2(vch, AFF_CONFUSED) )
    {
	act( "$N is confused enough as it is.", ch, NULL, vch, TO_CHAR );
	return;
    }

    if ( !check_stat(vch, STAT_WIS, -12) )
	return;

    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( strength, 12 );
    af.modifier		= table_multiplier( strength, 7 ) * -1;
    af.location		= APPLY_HITROLL;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_CONFUSED;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_to_char( vch, &af );

    act( "$n seem$% very confused.", vch, NULL, NULL, TO_ALL );
    return;
}

SPELL( spell_net_of_pain )
{
    AFFECT_DATA af;
    CHAR_DATA *vch;

    vch = (CHAR_DATA *) vo;
    af.type		= sn;
    af.strength		= strength;
    af.duration		= table_multiplier( number_fuzzy(strength), 15 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_PAIN;
    af.owner		= AFF_OWNER(ch);
    af.flags		= 0;
    SET_SEX( af, ch );
    affect_join( vch, &af );

    act( "$o mouth opens wide in a silent scream of pain.", vch, NULL,
	NULL, TO_ALL );
    return;
};

SPELL( spell_sever_from_true_source )
{
    CHAR_DATA *victim;
    int victim_strength = 0;

    victim = (CHAR_DATA *) vo;

    if ( !can_channel(victim, 1) )
    {
	act( "$N cannot channel.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( IS_AFFECTED_2(victim, AFF_STILL) )
    {
	act( "$N is already severed from the True Source.", ch, NULL,
	    victim, TO_CHAR );
	return;
    }

    if ( IS_GRASPING(victim) )
	victim_strength = victim_strength * 150 / 100;
    if ( IS_LINKING(victim) )
	victim_strength = victim_strength * 125 / 100;

    if ( number_range(victim_strength / 2, victim_strength)
    >=   number_range(strength / 2, strength) )
    {
	act( "You were not able to sever $N.", ch, NULL, victim, TO_CHAR );
	return;
    }

    act( "`3You have been severed from the True Source!`n", ch, NULL,
	victim, TO_VICT );
    act_channel( "`3You have severed $N from the True Source!`n", ch,
	NULL, victim, TO_CHAR );

    SET_BIT(victim->affected_by_2, AFF_STILL);
    if ( owns_affect(victim, TRUE) )
	send_to_char( "Your maintained weaves fade, as you are severed.\n\r", victim );
    die_weave( victim );
    drop_link( victim );
    if ( IS_GRASPING(victim) )
	do_release( victim, "" );
    return;
}
