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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "mem.h"

#define MAX_DAMAGE_MESSAGE 32

/* command procedures needed */
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_shout		);
DECLARE_DO_FUN(do_wear		);


/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune, int location ) );
void	spell_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch, int dt ) );
void	make_pk_corpse	args( ( CHAR_DATA *ch, int dt ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void	riposte_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, CHAR_DATA *ch, int dt ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	group_heal	args( ( CHAR_DATA *ch ) );
bool	check_break	args( ( OBJ_DATA *obj, int damage ) );

int	absorb_damage( CHAR_DATA *ch, int dam, int dam_type, int location );
int	armor_weight( CHAR_DATA *ch );
int	hit_loc( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
int	get_required( CHAR_DATA *ch, int sn, int weapon );
char	*flag_string               args ( ( const struct flag_type *flag_table,
                                         int bits ) );
int	luck_bonus( CHAR_DATA *ch );
int	roll_damage( CHAR_DATA *ch, OBJ_DATA *wield, sh_int sn, sh_int weapon );
OBJ_DATA *find_item_room( CHAR_DATA *ch, int type );


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    NODE_DATA *node;
    NODE_DATA *node_next;
    CHAR_DATA *ch;
    CHAR_DATA *victim;

    for ( node = fight_list; node != NULL; node = node_next )
    {
	int carry, enc;
	node_next	= node->next;

	if ( node->data_type != NODE_FIGHT )
	    continue;

	ch = (CHAR_DATA *) node->data;

	if ( ch->fighting == NULL
	&&   ch->hunting  == NULL )
	{
	    rem_fight_list( ch );
	    continue;
	}

        /*
         * Hunting mobs.
         */
        if ( IS_NPC(ch)
        &&   ch->fighting == NULL
        &&   IS_AWAKE(ch)
        &&   ch->hunting )
	{
	    hunt_victim(ch);
	    continue;
	}

	if ( !IS_NPC(ch) && ch->wait > 0 
	&&   !IS_SET(ch->comm, COMM_NOTICK) )
	{
	    int i;
	    send_to_char( "Time left until next action: ", ch );
	    for ( i = 0; i < ch->wait; i++ )
		send_to_char( "`3`B**`n ", ch );
	    send_to_char( "\n\r", ch );
	}

	if ( --ch->fight_timer > 0 )
	{
	    if ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_NOTICK) )
	    {
		int i;
		send_to_char( "Time left until next attack: ", ch );
		for ( i = 0; i < ch->fight_timer; i++ )
		    send_to_char( "`B##`n ", ch );
		send_to_char( "\n\r", ch );
	    }
	    continue;
	}

	ch->fight_timer = 4;

	if ( check_skill(ch, gsn_flash_strike, 50, TRUE) )
	{
	    ch->fight_timer--;
	    check_improve( ch, gsn_flash_strike, TRUE, 3 );
	}
	if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	    ch->fight_timer--;
	if ( IS_AFFECTED(ch, AFF_SLOW) )
	    ch->fight_timer++;

	carry = ch->carry_weight * 100 / can_carry_w( ch );
	if ( carry >= 100 )
	    enc = 300; 
	else if ( carry >= 90 )
	    enc = 200;
	else if ( carry >= 75 )
	    enc = 150;
	else if ( carry >= 60 )
	    enc = 125;   
	else
	    enc = 100;
	ch->fight_timer = ch->fight_timer * enc
			* agi_app[get_curr_stat(ch, STAT_AGI)].aff
			/ 10000;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( !IS_NPC(ch) )
	{
	    if ( IS_SET(ch->comm, COMM_NOSPAM) )
	    {
		send_to_char_new( ch, "Hits: %d  Misses: %d\n\r",
		    ch->pcdata->hits,
		    ch->pcdata->misses );
	    }
	    ch->pcdata->hits = 0;
	    ch->pcdata->misses = 0;
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    if (ch->in_room == NULL)
	return;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	if ( rch == ch )
	    continue;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || CHARM_SET(ch))
	    {
		if ( (( !IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST) )
		||     CHARM_SET( rch )) 
		&&   is_same_group(ch,rch) )
		{
		    act( "You rush to help $N!", rch, NULL, ch, TO_CHAR );
		    act( "$n rushes to $N's aid.", rch, NULL, ch, TO_NOTVICT );
		    act( "$n rushes to your aid.", rch, NULL, ch, TO_VICT );
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		}
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !CHARM_SET(ch) )	
	    {
		if ( !IS_SENTIENT(ch) )
		    continue;

		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   ( IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE) )

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM))

		||   (is_name( "guard", ch->name ) 
		   && IS_SET(rch->off_flags,ASSIST_GUARD))

		||  (rch->guild == ch->guild
		   && IS_SET(rch->off_flags, ASSIST_GUILD)) )

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    /* no attacks for stunnies -- just a check */
    if ( ch->position < POS_RECLINING )
	return;

    if ( ch->position < POS_FIGHTING
    &&   ch->wait == 0 )
    {
	act_fight( "$n struggle$% to $s feet and once again join$% the battle.", ch, NULL, NULL, TO_ALL );
	ch->position = POS_FIGHTING;
	return;
    }

    if ( IS_NPC(ch) )
    {
	mob_hit( ch, victim, dt );
	return;
    }

    one_hit( ch, victim, dt, FALSE );
    if ( ch->fighting != victim )
	return;

    if ( dt == gsn_backstab )
	return;

    if ( !can_channel(ch, 1) )
    {
	if ( number_percent() >= 80 )
	{
	    act_fight( "Your timing is just right, and you swing at $N again.", ch, NULL,
		victim, TO_CHAR );
	    one_hit( ch, victim, dt, FALSE );
	    if ( ch->fighting != victim )
		return;
	}
    }
    else
    {
	if ( number_percent() >= 98 )
	{
	    act_fight( "Your timing is just right, and you swing at $N again.", ch, NULL,
		victim, TO_CHAR );
	    one_hit( ch, victim, dt, FALSE );
	    if ( ch->fighting != victim )
		return;
	}
    }

    if ( get_eq_char(ch, WEAR_SECONDARY) )
        one_hit( ch, victim, dt, TRUE );

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int number;
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *obj;

    one_hit( ch, victim, dt, FALSE );
    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ( vch != victim && vch->fighting == ch )
		one_hit( ch, vch, dt, FALSE );
	}
        if ( ch->fighting != victim )
            return;
    }

    if ( get_eq_char(ch, WEAR_SECONDARY) )
        one_hit( ch, victim, dt, TRUE );

    if (ch->fighting != victim || dt == gsn_backstab)
	return;

    /* oh boy!  Fun stuff! */
    if (ch->wait > 0)
	return;

    /* check for items on the ground */
    if ( ch
    &&   number_bits(4) < 4
    &&   IS_SENTIENT(ch)
    &&   get_eq_char(ch, ITEM_WIELD) == NULL
    &&   (obj = find_item_room(ch, ITEM_WEAPON)) )
    {
	WAIT_STATE(ch, 1);
	get_obj( ch, obj, NULL );
	do_wear( ch, obj->name );
    }

    /* now for the skills */
    number = number_range( 0, 7 );
    switch( number ) 
    {
    case (0) :
	if ( IS_SET(ch->off_flags, OFF_BASH) )
	    do_bash( ch, "" );
	break;

    case (1) :
	if ( IS_SET(ch->off_flags, OFF_BERSERK)
	&&   !IS_AFFECTED(ch, AFF_BERSERK) )
	    do_berserk( ch, "" );
	break;

    case (2) :
	if ( IS_SET(ch->off_flags, OFF_DISARM) 
	||   (get_weapon_sn( ch ) != gsn_hand_to_hand 
	&&    ( IS_SET(ch->act, ACT_WARRIOR)
   	||      IS_SET(ch->act, ACT_ROGUE) )) ) 
	    do_disarm( ch, "" );
	break;

    case (3) :
	if ( IS_SET(ch->off_flags, OFF_KICK) )
	    do_kick( ch, "" );
	break;

    case (4) :
	if ( IS_SET(ch->off_flags, OFF_KICK_DIRT) )
	    do_dirt( ch, "" );
	break;

    case (5) :
/*	if (IS_SET(ch->off_flags,OFF_TAIL))
	     do_tail(ch,"") */ ;
	break; 

    case (6) :
	if ( IS_SET(ch->off_flags, OFF_TRIP)
	||   IS_SET(ch->act, ACT_ROGUE) ) 
	    do_trip( ch, "" );
	break;

    case (7) :
/*	if (IS_SET(ch->off_flags,OFF_CRUSH))
	     do_crush(ch,"") */ ;
	break;
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    CHAR_DATA *vch;
    OBJ_DATA *wield;
    bool has_critical, has_fumble, otherFight = FALSE;
    int dam;
    int roll;
    int sn;
    int required;
    int dam_type;
    int stamina, stamina_v;
    int weapon;
    int skill;

    sn = -1;
    has_critical = FALSE;
    has_fumble = FALSE;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if ( secondary )
	wield = get_eq_char( ch, WEAR_SECONDARY );
    else
	wield = get_eq_char( ch, WEAR_WIELD );

    if ( ch->stamina > 0 )
    {
	int loss;
	loss = number_range( 2, 6 );
	if ( check_skill(ch, gsn_endurance, 50, TRUE) )
	    loss /= 2;
	if ( check_skill(ch, gsn_hardiness, 50, TRUE) )
	    loss = loss * 2 / 3;
	loss = UMIN( loss, ch->stamina );
	lose_stamina( ch, loss, TRUE, FALSE );
    }

    /* Get current stamina percentage */
    stamina	= stamina_status( ch );
    stamina_v	= stamina_status( victim );

    /* Okay .. get the weapon skill */
    if ( secondary )
	sn = get_secondary_sn( ch );
    else
	sn = get_weapon_sn( ch );

    if ( dt == TYPE_UNDEFINED )
    {   
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    
	if ( wield != NULL && IS_SET(wield->extra_flags, ITEM_RUINED) )
	    dt = TYPE_HIT + ch->dam_type;
    }

    if ( dt < TYPE_HIT )
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;    
    if ( dam_type == -1 )
	dam_type = DAM_BASH;

    /* Got the sn, get the weapon */
    if ( wield != NULL
    &&   wield->item_type == ITEM_WEAPON )
	weapon = wield->value[0];
    else
	weapon = 10;

    /* Modify the to hit requirement based on weapon skill
       and do a d100 roll */
    required = get_required( ch, sn, weapon );
    roll = number_percent( );

    if ( roll <= weapon_table[weapon].fumble )
	has_fumble = TRUE;

    if ( roll >= weapon_table[weapon].critical )
	has_critical = TRUE;

    /* Okay, now modify it based on:
	Dexterity OB
	Weapon Bonus
	Flash Strike
	Opponent's Armor Weight
	Victim's Position
	Ambidexterity Bonus
       and
	Dexterity DB
	Parry Bonus
	Acrobatics Bonus
	Stamina Percentage
	Secondary Weapon (and Ambidexterity)
    */

    roll += dex_app[get_curr_stat(ch, STAT_DEX)].off_bonus / 2;
    roll += ch->hitroll;
    if ( check_skill(ch, gsn_flash_strike, 50, TRUE) )
    {
	check_improve( ch, gsn_flash_strike, TRUE, 4 );
	roll += 25;
    }
    else
	check_improve( ch, gsn_flash_strike, FALSE, 4 );
    roll += armor_weight( victim );
    if ( !IS_AWAKE(victim) )
	roll += 50;
    else if ( victim->position < POS_FIGHTING )
	roll += 5 * ( POS_FIGHTING - victim->position );
    if ( check_skill(ch, gsn_ambidexterity, 50, TRUE) )
    {
	int skill;

	skill = get_skill( ch, gsn_ambidexterity );
	roll += number_range( skill / 8, skill / 4 );
    }
    roll += class_table[ch->class].off_bonus;
    if ( victim->wait > 0 )
	roll += (victim->wait * 3 / 2) + 1;
    if ( can_channel(victim, 1) )
	roll += 10;
    if ( sn == gsn_hand_to_hand )
    {
	if ( check_skill(ch, gsn_martial_arts, 50, TRUE) )
	{
	    int skill;

	    skill = get_skill( ch, gsn_martial_arts );
	    roll += UMAX( 1, skill / 4 );
	}
    }
    else if ( sn != -1 )
    {
	if ( check_skill(ch, gsn_weapon_prof, 50, TRUE) )
	{
	    int skill;

	    skill = get_skill( ch, gsn_weapon_prof );
	    check_improve(ch, gsn_weapon_prof, TRUE, 4);
	    roll += UMAX( 1, skill / 4 );
	}
	if ( check_skill(ch, gsn_spear_dancing, 33, TRUE) )
	{
	    int skill;

	    skill = get_skill( ch, gsn_spear_dancing );
	    check_improve(ch, gsn_spear_dancing, TRUE, 4);
	    roll += UMAX( 1, skill / 3 );
	}
    }
    roll += luck_bonus( ch );
    if ( wield && wield->item_type == ITEM_WEAPON )
	roll += number_range( 0, wield->value[5] );
    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( vch->fighting == victim && victim->fighting != vch )
	{
	    if ( otherFight )
		roll += 7;
	    otherFight = TRUE;
	}
	if ( vch->fighting == victim && victim->fighting == vch )
	{
	    if ( otherFight )
		roll += 3;
	    otherFight = TRUE;
	}
    }
    if ( stamina_v == 3 )
	;
    else if ( stamina_v == 2 )
	roll += 5;
    else if ( stamina_v == 1 )
	roll += 12;
    else
	roll += 25;

    if ( can_channel(ch, 1) )
	roll -= 10;
    roll -= dex_app[get_curr_stat(victim, STAT_DEX)].def_bonus / 2;
    roll -= parry( ch, victim );
    if ( check_skill(victim, gsn_acrobatics, 50, TRUE) )
    {
	int skill;

	act_fight( "$N tumble$^ out of the way, making it harder for $n to hit.",
	    ch, NULL, victim, TO_ALL );
	skill = get_skill( victim, gsn_acrobatics );
	roll -= UMAX( 1, number_range(skill / 4, skill / 2) );
	check_improve(victim, gsn_acrobatics, TRUE, 5);
    }
    roll -= armor_weight( ch );
    if ( stamina == 3 )
	;
    else if ( stamina == 2 )
	roll -= 10;
    else if ( stamina == 1 )
	roll -= 25;
    else
	roll -= 50;
    if ( get_eq_char(ch, WEAR_SECONDARY) )
    {
	roll -= 40;
	if ( secondary )
	    roll -= 20;

	if ( check_skill(ch, gsn_ambidexterity, 50, TRUE) )
	{
	    roll += 40;
	    check_improve( ch, gsn_ambidexterity, TRUE, 3 );
	}
	else
	{
	    roll += 15;
	    check_improve( ch, gsn_ambidexterity, FALSE, 4 );
	}
    }
    roll -= class_table[victim->class].def_bonus;
    roll -= luck_bonus( victim );

    /* Have the modified roll.  If there was a fumble or critical,
       Use those functions instead 
    if ( has_fumble )
    {
	fumble( ch, victim, roll, dam_type, sn );
	return;
    }
    else if ( has_critical )
    {
	critical( ch, victim, roll, dam_type, sn );
	return;
    }
    else */
    {
	if ( roll <= required )
	{
	    if ( secondary )
		damage( ch, victim, 0, dt, dam_type, TRUE );
	    else
		damage( ch, victim, 0, dt, dam_type, FALSE );
	    return;
	}

	/* Otherwise, do standard damage calculation */
	dam = roll_damage( ch, wield, sn, weapon );
    }

    dam += ch->damroll;
    dam += str_app[get_curr_stat(ch, STAT_STR)].dam_bonus;

    if ( check_skill(ch, gsn_enhanced_damage, 100, TRUE) )
    {
	dam += dam * number_percent( ) / 100;
	check_improve( ch, gsn_enhanced_damage, TRUE, 6 );
    }

    if ( dt == gsn_backstab
    &&   wield != NULL )
    {
	int mult;
	if ( wield->value[0] != 2 )
	    mult = 100 + ( get_skill(ch, gsn_backstab) * 3 );
	else
	    mult = 100 + ( get_skill(ch, gsn_backstab) * 5 );
	dam = dam * mult / 100;
    }

    if ( secondary )
	dam = dam * 90 / 100;

    if ( dam <= 0 )
	dam = 1;

    if ( IS_NPC(ch) )
    {
        if ( sn == -1 )
            skill = ch->level * 5 / 6;
        else if (sn == gsn_hand_to_hand)
            skill = 40 + ch->level * 5 / 6;
        else
	{
	    if ( ch->level >= 35 )
		skill = 70 + (ch->level - 35) / 3;
	    else
		skill = 35 + ch->level;
	}
    }
    else
    {
        if ( sn == -1 )
            skill = ch->level * 4 / 5;
        else 
	{
	    if ( ch->pcdata->learned[sn] < 0 )
		skill = 0;
	    else
		skill = SKILL(ch, sn);
	}
    }

    if ( roll - required >= 100 )
    {
	dam *= 2;
	act_fight( "$n strike$% a mighty blow on $N!", ch, NULL, victim,
	    TO_ALL );
    }
    else if ( roll - required >= 50 )
    {
	dam = dam * 150 / 100;
	act_fight( "$n strike$% a powerful blow on $N!", ch, NULL, victim,
	    TO_ALL );
    }

    if ( skill > 85 )
	dam = dam + (dam * (100 + skill - 85)) / 100;	

    if ( sn == gsn_hand_to_hand )
    {
	if ( check_skill(ch, gsn_martial_arts, 50, TRUE) )
	{
	    int skill;

	    skill = get_skill( ch, gsn_martial_arts );
	    check_improve(ch, gsn_martial_arts, TRUE, 4);
	    dam = dam * ( 100 + UMAX(1, skill / 4) ) / 100;
	}
    }
    else if ( sn != -1 )
    {
	if ( check_skill(ch, gsn_weapon_prof, 50, TRUE) )
	{
	    int skill;

	    skill = get_skill( ch, gsn_weapon_prof );
	    check_improve(ch, gsn_weapon_prof, TRUE, 4);
	    dam = dam * ( 100 + UMAX(1, skill / 4) ) / 100;
	}
    }


    if ( secondary )
	damage( ch, victim, dam, dt, dam_type, TRUE );
    else
	damage( ch, victim, dam, dt, dam_type, FALSE );

    tail_chain( );
    return;
}

void riposte_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary)
{
    CHAR_DATA *vch;
    OBJ_DATA *wield;
    bool has_critical, has_fumble, otherFight = FALSE;
    int dam;
    int roll;
    int sn;
    int required;
    int dam_type;
    int stamina, stamina_v;
    int weapon;

    sn = -1;
    has_critical = FALSE;
    has_fumble = FALSE;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if ( secondary )
	wield = get_eq_char( ch, WEAR_SECONDARY );
    else
	wield = get_eq_char( ch, WEAR_WIELD );

    if ( ch->stamina > 0 )
    {
	int loss;
	loss = number_range( 2, 6 );
	if ( check_skill(ch, gsn_endurance, 100, TRUE) )
	    loss /= 2;
	loss = UMIN( loss, ch->stamina );
	lose_stamina( ch, loss, TRUE, FALSE );
    }

    /* Get current stamina percentage */
    stamina	= stamina_status( ch );
    stamina_v	= stamina_status( victim );

    /* Okay .. get the weapon skill */
    if ( secondary )
	sn = get_secondary_sn( ch );
    else
	sn = get_weapon_sn( ch );

    if ( dt == TYPE_UNDEFINED )
    {   
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    
	if ( wield != NULL && IS_SET(wield->extra_flags, ITEM_RUINED) )
	    dt = TYPE_HIT + ch->dam_type;
    }

    if ( dt < TYPE_HIT )
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;    
    if ( dam_type == -1 )
	dam_type = DAM_BASH;

    /* Got the sn, get the weapon */

    if ( wield != NULL
    &&   wield->item_type == ITEM_WEAPON )
	weapon = wield->value[0];
    else
	weapon = 10;

    /* Modify the to hit requirement based on weapon skill
       and do a d100 roll */
    required = get_required( ch, sn, weapon );
    roll = number_percent( );

    /* Okay, now modify it based on:
	Dexterity OB
	Weapon Bonus
	Flash Strike
	Opponent's Armor Weight
	Victim's Position
	Ambidexterity Bonus
       and
	Dexterity DB
	Stamina Percentage
	Secondary Weapon (and Ambidexterity)
    */

    roll += dex_app[get_curr_stat(ch, STAT_DEX)].off_bonus;
    roll += ch->hitroll;
    if ( check_skill(ch, gsn_flash_strike, 50, TRUE) )
    {
	check_improve( ch, gsn_flash_strike, TRUE, 4 );
	roll += 25;
    }
    else
	check_improve( ch, gsn_flash_strike, FALSE, 4 );
    roll += armor_weight( victim );
    if ( !IS_AWAKE(victim) )
	roll += 50;
    else if ( victim->position < POS_FIGHTING )
	roll += 5 * ( POS_FIGHTING - victim->position );
    if ( check_skill(ch, gsn_ambidexterity, 50, TRUE) )
    {
	int skill;

	skill = get_skill( ch, gsn_ambidexterity );
	roll += number_range( skill / 4, skill / 2 );
    }
    roll += class_table[ch->class].off_bonus;
    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( vch->fighting == victim && victim->fighting != vch )
	{
	    if ( otherFight )
		roll += 7;
	    otherFight = TRUE;
	}
	if ( vch->fighting == victim && victim->fighting == vch )
	{
	    if ( otherFight )
		roll += 3;
	    otherFight = TRUE;
	}
    }
    if ( stamina_v == 3 )
	;
    else if ( stamina_v == 2 )
	roll += 5;
    else if ( stamina_v == 1 )
	roll += 12;
    else
	roll += 25;

    roll -= dex_app[get_curr_stat(victim, STAT_DEX)].def_bonus;
    if ( stamina == 3 )
	;
    else if ( stamina == 2 )
	roll -= 10;
    else if ( stamina == 1 )
	roll -= 25;
    else
	roll -= 50;
    if ( get_eq_char(ch, WEAR_SECONDARY) )
    {
	roll -= 40;
	if ( secondary )
	    roll -= 20;

	if ( check_skill(ch, gsn_ambidexterity, 50, TRUE) )
	{
	    roll += 40;
	    check_improve( ch, gsn_ambidexterity, TRUE, 3 );
	}
	else
	{
	    roll += 15;
	    check_improve( ch, gsn_ambidexterity, FALSE, 4 );
	}
    }
    roll -= class_table[ch->class].off_bonus;

    /* Have the modified roll.  If there was a fumble or critical,
       Use those functions instead */
    if ( has_fumble )
    {/*
	fumble( ch, victim, roll, dam_type, sn );
*/	return;
    }
    else if ( has_critical )
    {/*
	critical( ch, victim, roll, dam_type, sn );
*/	return;
    }
    else
    {
	/* Check to see if there is a hit */
	if ( roll < required )
	{
	    if ( secondary )
		damage( ch, victim, 0, dt, dam_type, TRUE );
	    else
		damage( ch, victim, 0, dt, dam_type, FALSE );
	    return;
	}

	/* Otherwise, do standard damage calculation */
	dam = roll_damage(ch, wield, sn, weapon);
    }

    if ( secondary )
	dam = dam * 90 / 100;

    if ( dam <= 0 )
	dam = 1;

    if ( secondary )
	damage( ch, victim, dam, dt, dam_type, TRUE );
    else
	damage( ch, victim, dam, dt, dam_type, FALSE );

    tail_chain( );
    return;
}

/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,
             bool secondary )
{
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    bool immune;
    int stamina;
    int location;

    if ( !secondary )
        obj = get_eq_char( ch, WEAR_WIELD );
    else
	obj = get_eq_char( ch, WEAR_SECONDARY );

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1000 )
    {
	bug( "Damage: %d: more than 1000 points!", dam );
	dam = 1000;
    }

    
    /* damage reduction */
    if ( dam > 30)
	dam = (dam - 30)/2 + 30;
    if ( dam > 75)
	dam = (dam - 75)/2 + 75;
    if ( dam > 200)
	dam = (dam - 200)/2 + 200;
   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;

	if ( ch->fighting == NULL )
	    set_fighting( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   CHARM_SET(victim)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.  ( Same with Shapeshifted -- Joker )
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n snaps into existence.", ch, NULL, NULL, TO_ROOM );
    }

    if ( is_affected( ch, gsn_shape_change ) || 
	 is_affected( ch, gsn_disguise ) )
    {
	act( "$n's form returns to normal.", ch, NULL, NULL, TO_ROOM );
	remove_shape( ch );
    }

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED(victim, AFF_AIR_ARMOR) )
	dam /= 2;

    /*
     * Get a damage location and absorb the damage
     */

    stamina = stamina_status( ch );

    if ( IS_NPC(ch) )
	stamina = 3;

    if ( stamina == 3 )
	;
    else if ( stamina == 2 )
	dam -= dam / 20;
    else if ( stamina == 1 )
	dam -= dam / 10;
    else
	dam -= dam / 4;

    if ( dt == gsn_poison
    ||   dt == gsn_plague )
	location = 0;
    else
    {
	location = hit_loc(ch, victim, dt );
	dam = absorb_damage( victim, dam, dam_type, location );
    }

    immune = FALSE;
    switch( check_immune(victim, dam_type) )
    {
	case ( IS_IMMUNE ):
	    immune = TRUE;
	    dam = 0;
	    break;
	case ( IS_RESISTANT ):	
	    dam -= dam/3;
	    break;
	case ( IS_VULNERABLE ):
	    dam += dam/2;
	    break;
    }

    /* Damage reduction from fatigue */
    /* All the routines that use material types -- Joker */
    /* And routines that involve bonus damage from the weapon */ 

    if ( obj != NULL && dam > 0 && dt >= TYPE_HIT
    &&   !IS_SET(obj->extra_flags, ITEM_RUINED) )
    {
	if ( IS_SET(obj->extra_flags, ITEM_BENT) )
	    dam /= 4;

	/* Check for flaming weapons */
	if ( IS_SET(obj->value[4], WEAPON_FLAMING) &&
	     obj->item_type == ITEM_WEAPON )
	{
	    if ( IS_SET(victim->imm_flags, IMM_FIRE) )
	    {
		act_fight( "$p burns $N, but $E doesn't seem to notice.", ch, obj,
		    victim, TO_NOTVICT );
		act_fight( "$p burns you, but you don't notice.", ch, obj,
		    victim, TO_VICT );
		act_fight( "$p burns $N, but $E doesn't seem to notice.", ch, obj,
		    victim, TO_CHAR );
	    }
	    else
	    {
		act_fight( "$N screams in pain as $p burns $M.", ch, obj,
		    victim, TO_NOTVICT );
		act_fight( "You scream in pain as $p burns you.", ch, obj,
		    victim, TO_VICT );
		act_fight( "$N screams in pain as $p burns $M.", ch, obj,
		    victim, TO_CHAR );
		if ( IS_SET(victim->vuln_flags, VULN_FIRE) )
		    dam += dam / 4;
		else if ( IS_SET(victim->res_flags, RES_FIRE) )
		    dam += dam / 16;
		else
		    dam += dam / 8; 
	    }
	}

	/* Check for freezing weapons */
	if ( IS_SET(obj->value[4], WEAPON_FROST) &&
	     obj->item_type == ITEM_WEAPON )
	{
	    if ( IS_SET(victim->imm_flags, IMM_COLD) )
	    {
		act_fight( "$p chills $N, but $E doesn't seem to notice.", ch, obj,
		    victim, TO_NOTVICT );
		act_fight( "$p chills you, but you don't notice.", ch, obj,
		    victim, TO_VICT );
		act_fight( "$p chills $N, but $E doesn't seem to notice.", ch, obj,
		    victim, TO_CHAR );
	    }
	    else
	    {
		act_fight( "$N screams in pain as $p freezes $M.", ch, obj,
		    victim, TO_NOTVICT );
		act_fight( "You scream in pain as $p freezes you.", ch, obj,
		    victim, TO_VICT );
		act_fight( "$N screams in pain as $p freezes $M.", ch, obj,
		    victim, TO_CHAR );
		if ( IS_SET(victim->vuln_flags, VULN_COLD) )
		    dam += dam / 4;
		else if ( IS_SET(victim->res_flags, RES_COLD) )
		    dam += dam / 16;
		else
		    dam += dam / 8; 
	    }
	}

	/* Check for sharp weapons :) */
	if ( IS_SET(obj->value[4], WEAPON_SHARP) &&
	     obj->item_type == ITEM_WEAPON )
	{
	    int bonus_dice, bonus_sides;

	    bonus_dice = obj->value[1] / 2;
	    bonus_sides = obj->value[2] / 2;

	    dam += dice( bonus_dice, bonus_sides );
	    if ( number_percent() >= 80 )
	    {
		SET_BIT( victim->body, BODY_BLEEDING );
		act_fight( "`3$n's attack slashes you, causing you to bleed badly!`n", ch, NULL, victim, TO_VICT );
	    }
	}

	/* Check bonus damage for steel, mithril, and obsidian */	 
	if ( obj->material == MAT_STEEL
	||   obj->material == MAT_OBSIDIAN )
	    dam++;

	if ( obj->item_type == ITEM_WEAPON
	&&   IS_SET(obj->value[4], WEAPON_POISON) )
	{
	    AFFECT_DATA af;
	    af.type		= gsn_poison;
	    af.strength		= UMAX(1, obj->level/5);
	    af.duration		= UMAX(1, obj->level/10);
	    af.location		= APPLY_STR;
	    af.modifier		= -2;
	    af.bitvector	= AFF_POISON;
	    af.bitvector_2	= 0;
	    af.owner		= NULL;
	    af.flags		= AFFECT_NOTCHANNEL;
	    affect_to_char( victim, &af );
	    send_to_char( "You feel very sick.\n\r", victim );
	    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
	    if ( number_bits(3) == 0 )
		REMOVE_BIT( obj->value[4], WEAPON_POISON );
	    else
		obj->level = UMAX(0, obj->level - 1);
	}
	check_break( obj, dam * 9 / 10 );
    }

    dam_message( ch, victim, dam, dt, immune, location );

    if ( !IS_NPC(ch) && ch->in_room == victim->in_room )
    {
	if ( dam == 0 )
	    ch->pcdata->misses++;
	else
	    ch->pcdata->hits++;
    }

    if ( dam == 0 )
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    lose_health( victim, dam, TRUE );
    if ( dam_type == DAM_BASH )
	lose_stamina( victim, dam / number_range(15, 25), TRUE, TRUE );
    else
	lose_stamina( victim, dam / number_range(20, 35), TRUE, TRUE );

    if ( location == WEAR_LEGS
    &&   (dam_type == DAM_BASH
    ||    dam_type == DAM_SLASH
    ||    dam_type == DAM_PIERCE) )
    {
	int roll, chance;
	roll = number_percent()
	     + number_percent()
	     + number_percent()
	     + number_percent() - 1;

	chance = 0;
	if ( dam > 5 )
	    chance += 1;
	if ( dam > 10 )
	    chance += 2;
	if ( dam > 20 )
	    chance += 5;
	if ( dam > 40 )
	    chance += 8;
	if ( dam > 80 )
	    chance += 10;
	if ( dam > 160 )
	    chance += 12;
	if ( dam > 320 )
	    chance += 27;
	if ( dam_type == DAM_BASH )
	    chance += 25;

	if ( chance >= roll )
	{
	    if ( number_bits(1) == 0 )
	    {
		SET_BIT(victim->body, BODY_LEFT_LEG);
		act( "$N scream$^ as $S left leg is bent out of position, then a *SNAP* is heard.",
		    ch, NULL, victim, TO_ALL );
	    }
	    else
	    {
		SET_BIT(victim->body, BODY_RIGHT_LEG);
		act( "$N scream$^ as $S right leg is bent out of position, then a *SNAP* is heard.",
		    ch, NULL, victim, TO_ALL );
	    }
	}
    }

    if ( dam_type == DAM_SLASH || dam_type == DAM_PIERCE )
    {
	int i, val = 0;

	for ( i = 0; i < 3; i++ )
	    val += number_percent();
	if ( val >= 250 )
	{
	    SET_BIT( victim->body, BODY_BLEEDING );
	    act_fight( "`3$n's attack slashes you, causing you to bleed badly!`n", ch, NULL, victim, TO_VICT );
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( IS_SET(ch->comm, COMM_AUTOHEAL) )
	    group_heal( ch );

	if ( !IS_NPC(victim) )
	{
	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     * NOT!  Lose a level's worth of xp
	     */

	    if ( IS_NPC(ch) )
	    {
		int loss;
		char buf[MAX_STRING_LENGTH];

		switch (victim->level)
		{
		    default:
			loss = -(exp_per_level(victim, victim->pcdata->points));
			break;
		    case 1:
			loss = 0;
			break;
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
		send_to_char( buf, victim );
		gain_exp( victim, loss );
	    }	
	}

        sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);

        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

	raw_kill( victim, ch, dam_type );

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) ) {
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
            }
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_flee( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < 3 )
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( CHARM_SET(victim) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < 3 )
	do_flee( victim, "" );

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim )
{

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	send_to_char("The shopkeeper wouldn't like that.\n\r",ch); 
        return TRUE;
    }
    /* no killing healers, adepts, etc */
    if (IS_NPC(victim) 
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)))
    {
	send_to_char("I don't think the Creator would approve.\n\r",ch);
	return TRUE;
    }

    /* no fighting in safe rooms */
    if (ch->in_room != NULL && IS_SET(ch->in_room->room_flags,ROOM_SAFE))
    {
	send_to_char("Not in this room.\n\r",ch);
	return TRUE;
    }

    if (victim->fighting == ch)
	return FALSE;

    if (IS_NPC(ch))
    {
 	/* charmed mobs and pets cannot attack players */
	if (!IS_NPC(victim) && (CHARM_SET(ch)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;

      	return FALSE;
     }

     else /* Not NPC */
     {	
	if (IS_IMMORTAL(ch))
	    return FALSE;

	return FALSE;
    }
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    /* can't zap self (crash bug) */
    if (ch == victim)
	return TRUE;
    /* immortals not hurt in area attacks */
    if (IS_IMMORTAL(victim) && area)
	return TRUE;

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
        return TRUE;

    /* no killing healers, adepts, etc */
    if (IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_GAIN)
    ||  IS_SET(victim->act,ACT_IS_HEALER)))
	return TRUE;

    /* no fighting in safe rooms */
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
        return TRUE;

    if (victim->fighting == ch)
	return FALSE;
 
    if (IS_NPC(ch))
    {
        /* charmed mobs and pets cannot attack players */
        if (!IS_NPC(victim) && (CHARM_SET(ch)
                            ||  IS_SET(ch->act,ACT_PET)))
            return TRUE;
	
	/* area affects don't hit other mobiles */
        if (IS_NPC(victim) && area)
            return TRUE;
 
        return FALSE;
    }
 
    else /* Not NPC */
    {
	if ( CHARM_SET(ch) && ch->master == victim )
	    return TRUE;

        if (IS_IMMORTAL(ch) && !area)
            return FALSE;

	if ( is_same_group(ch, victim) && area )
	    return TRUE;

	/* cannot use spells if not in same group */
	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	    return TRUE;
  
        return FALSE;
    }
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( CHARM_SET(victim) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim) )
	return;

    /*
     * Charm-o-rama.
     */
    if ( CHARM_SET(ch) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM or AFF_LEASHED",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    affect_strip( ch, gsn_leashing );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    REMOVE_BIT( ch->affected_by_2, AFF_LEASHED );
	    return;
	}
	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL )
	return;

    save_char_obj( ch );
    return;
}



/*
 * Check for parry.
 */
int parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *wield;
    bool secondary = FALSE;

    wield = get_eq_char( victim, WEAR_WIELD );
    if ( wield == NULL )
    {
	wield = get_eq_char( victim, WEAR_SECONDARY );
	secondary = TRUE;
    }

    if ( !IS_AWAKE(victim) )
	return( 0 );
    if ( wield == NULL )
	return( 0 );

    if ( check_skill(ch, gsn_feint, 20, TRUE) )
    {
	act_fight( "$n feint$% quickly, and $O parrying attempt goes awry.",
	    ch, NULL, victim, TO_ALL );
	check_improve( ch, gsn_feint, TRUE, 4 );
	return( 0 );
    }

    if ( check_skill(victim, gsn_parry, 50, TRUE) )
    {
	int parry, skill;

	check_improve( victim, gsn_parry, TRUE, 3 );
	skill = get_skill( victim, gsn_parry );
	parry = number_range( skill / 10, skill / 2 ) * -1;

	if ( parry <= -40 )
	    act_fight( "$N easily deflect$^ $o weapon.", ch, NULL,
		victim, TO_ALL );
	else if ( parry <= -30 )
	    act_fight( "$N move$^ $S weapon a little, deflecting $o attack.",
		ch, NULL, victim, TO_ALL );
	else if ( parry <= -20 )
	    act_fight( "$N deflect$^ $o weapon.", ch, NULL,
		victim, TO_ALL );
	else if ( parry <= -10 )
	    act_fight( "$N deflect$^ $o weapon a little bit.", ch, NULL,
		victim, TO_ALL );
	else
	    act_fight( "$N deflect$^ $o weapon the tiniest bit.", ch, NULL,
		victim, TO_ALL );

	if ( check_skill(ch, gsn_riposte, 25, TRUE) )
	{
	    act_fight( "$N return$^ a slight blow to $n.", ch, NULL,
		victim, TO_ALL );
	    riposte_hit( victim, ch, TYPE_UNDEFINED, secondary );
	    check_improve( victim, gsn_riposte, TRUE, 4 );
	}
	return( parry );
    }
    check_improve( victim, gsn_parry, FALSE, 4 );
    return( 0 );
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	break_con( victim );
	victim->position = POS_DEAD;
	return;
    }

    if ( !IS_NPC(victim)
    &&   victim->guild == guild_lookup("warder")
    &&   victim->pcdata->sedai != NULL )
    {

	if ( victim->hit < -9  )
	{
	    int temp;

	    temp = -1 * victim->hit - 9;

	    if ( number_percent() < temp * 5 )
	    {
		break_con( victim );
		victim->position = POS_DEAD;
		return;
	    }
	}

	     if ( victim->hit <= -6
	&&        !check_skill(victim, gsn_endurance, 10, TRUE) )
	{
	    break_con( victim );
	    victim->position = POS_MORTAL;
	}
	else if ( victim->hit <= -3 
	&&        !check_skill(victim, gsn_endurance, 20, TRUE) )
	{
	    victim->position = POS_INCAP;
	    break_con( victim );
	}
	else if ( !check_skill(victim, gsn_endurance, 30, TRUE) )
	{
	    victim->position = POS_STUNNED;
	    break_con( victim );
	}    

	return;
    }

    if ( victim->hit <= -11  )
    {
	break_con( victim );
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    break_con( victim );
    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int carry, enc;

    break_con( ch );
    break_con( victim );
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    if ( IS_AFFECTED(ch, AFF_WRAP) )
	affect_strip( ch, skill_lookup("wrap") );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    ch->fight_timer = 4;

    if ( check_skill(ch, gsn_flash_strike, 50, TRUE) )
	ch->fight_timer--;
    if (IS_SET(ch->off_flags,OFF_FAST))
	ch->fight_timer--;
    if ( IS_AFFECTED(ch, AFF_SLOW) )
	ch->fight_timer++;

    carry = ch->carry_weight * 100 / can_carry_w( ch );
    if ( carry >= 100 )
	enc = 300; 
    else if ( carry >= 90 )
	enc = 200;
    else if ( carry >= 75 )
	enc = 150;
    else if ( carry >= 60 )
	enc = 125;   
    else
	enc = 100;
    ch->fight_timer = ch->fight_timer * enc
		    * agi_app[get_curr_stat(ch, STAT_AGI)].aff
		    / 10000;

    add_fight_list( ch );

    if ( ch->on != NULL )
	ch->on = NULL;

    if ( !IS_NPC(ch) )
    {
	ch->pcdata->hits = 0;
	ch->pcdata->misses = 0;
    }

    if ( IS_NPC(ch)
    &&   (IS_SET(ch->off_flags, OFF_MEMORY)
    ||    ( IS_SENTIENT(ch) && check_stat(ch, STAT_INT, -10) ))
    &&   ch->memory == NULL )
    {
	ch->memory		= new_mem_data();
	ch->memory->id		= victim->id;
	ch->memory->reaction	|= MEM_HOSTILE;
	ch->memory->when	= current_time;
    }

    else if ( IS_NPC(ch)
    &&        IS_SET(ch->off_flags, OFF_TRACK)
    &&        ch->hunting == NULL )
	ch->hunting = victim;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    NODE_DATA *node, *node_next;
    CHAR_DATA *fch;

    for ( node = fight_list; node != NULL; node = node_next )
    {
	node_next = node->next;

	if ( node->data_type != NODE_FIGHT )
	    continue;

	fch = (CHAR_DATA *) node->data;

	if ( fch == ch || (fBoth && fch->fighting == ch) )
	{
	    rem_fight_list( fch );
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    if ( fch->mount != NULL && fch->in_room == fch->mount->in_room )
		fch->position = POS_MOUNTED;
	    update_pos( fch );
	    if ( !IS_NPC(fch) && buf_string(fch->pcdata->buffer)[0] != '\0' )
		send_to_char( "You have messages in your playback.  Type REPLAY to read them.\n\r", fch );
	}
    }
    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, int dt )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	    corpse->owned = str_dup(ch->name);
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    free_string( corpse->short_descr );
    free_string( corpse->description );
    switch ( dt )
    {
	default:
	    sprintf( buf, "the corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_BASH:
	    sprintf( buf, "the bruised corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The bruised corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_PIERCE:
	    sprintf( buf, "the punctured corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The punctured corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_SLASH:
	    sprintf( buf, "the cut up corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The cut up corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_FIRE:
	    free_string( corpse->name );
	    corpse->name = str_dup( "remains corpse" );
	    sprintf( buf, "the charred remains of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The charred remains of %s lie here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_COLD:
	    sprintf( buf, "the frozen corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The frozen corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_LIGHTNING:
	    sprintf( buf, "the burned corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The burned corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_ACID:
	    free_string( corpse->name );
	    corpse->name = str_dup( "remains corpse" );
	    sprintf( buf, "the melted remains of something" );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The melted remains of something lie here." );
	    corpse->description = str_dup( buf );
	    break;
    }

    if ( IS_NPC(ch) )
    {
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_char( obj );
	    if (obj->item_type == ITEM_POTION)
		obj->timer = number_range(500,1000);
	    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
		obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
		extract_obj( obj );
	    else
		obj_to_obj( obj, corpse );
	}
    }

    obj_to_room( corpse, ch->in_room );
    return;
}

void make_pk_corpse( CHAR_DATA *ch, int dt )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	    corpse->owned = str_dup(ch->name);
	corpse->cost = 0;
    }

    corpse->level = ch->level;


    free_string( corpse->short_descr );
    free_string( corpse->description );
    switch ( dt )
    {
	default:
	    sprintf( buf, "the corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_BASH:
	    sprintf( buf, "the bruised corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The bruised corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_PIERCE:
	    sprintf( buf, "the punctured corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The punctured corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_SLASH:
	    sprintf( buf, "the cut up corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The cut up corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_FIRE:
	    free_string( corpse->name );
	    corpse->name = str_dup( "remains corpse" );
	    sprintf( buf, "the charred remains of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The charred remains of %s lie here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_COLD:
	    sprintf( buf, "the frozen corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The frozen corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_LIGHTNING:
	    sprintf( buf, "the burned corpse of %s", name );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The burned corpse of %s lies here.", name );
	    corpse->description = str_dup( buf );
	    break;
	case DAM_ACID:
	    free_string( corpse->name );
	    corpse->name = str_dup( "remains corpse" );
	    sprintf( buf, "the melted remains of something" );
	    corpse->short_descr = str_dup( buf );
	    sprintf( buf, "The melted remains of something lie here." );
	    corpse->description = str_dup( buf );
	    break;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	    obj->timer = number_range(5,10);
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(6))
    {
    default:
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 && number_bits(2) == 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	obj->item_type = ITEM_TRASH;
/*
	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}
*/
	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( was_in_room != NULL
	&&   ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim, CHAR_DATA *ch, int dt )
{
    CHAR_DATA *fighting;

    fighting = victim->fighting;

    stop_fighting( victim, TRUE );
    death_cry( victim );
    if ( IS_NPC(ch) && !IS_NPC(victim) ) 
        make_corpse( victim, dt );
    else
	make_pk_corpse( victim, dt );

    if ( IS_NPC(victim) )
    {
	if ( victim->level > 10 )
	{
	    if ( number_percent() < victim->level * 3 )
		victim->pIndexData->killed++;
	}
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );

    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= 0;

    if ( IS_GRASPING(victim) )
	REMOVE_BIT(victim->affected_by_2, AFF_GRASP);

    if ( fighting != NULL
    &&   !IS_NPC(fighting)
    &&   victim->level <= 10
    &&   dt != DAM_SLAY )
    {
	AFFECT_DATA af;

	af.type		= skill_lookup( "sleep" );
	af.strength	= 1;
	af.duration	= URANGE(1, abs(victim->hit)/2, 4 );
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_SLEEP;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;

	affect_join( victim, &af );
	victim->position= POS_SLEEPING;
	victim->hit	= 0;
	victim->stamina	= 0;
	victim->body	= 0;
	REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
	save_char_obj( victim ); 
	return;
    }

    if ( fighting == NULL
    ||   IS_NPC(fighting)
    ||   IS_SET(fighting->act, PLR_SUBDUE)
    ||   dt == DAM_SLAY )
    {
	victim->position= POS_RESTING;
	victim->hit	= UMAX( 1, victim->hit  );
	victim->stamina	= UMAX( 1, victim->stamina );
    }
    else
    {
	AFFECT_DATA af;

	af.type		= skill_lookup( "sleep" );
	af.strength	= 1;
	af.duration	= URANGE(1, abs(victim->hit)/2, 4 );
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_SLEEP;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;

	affect_join( victim, &af );
	victim->position= POS_SLEEPING;
	victim->hit	= 0;
	victim->stamina	= 0;
    }

    victim->body	= 0;
    /* RT added to prevent infinite deaths */
    REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
    save_char_obj( victim ); 
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC(victim) || victim == ch )
	return;

    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group(gch, ch) )
        {
	    members++;
	    if ( gch->level > group_levels )
	        group_levels = gch->level;
	}
    }

    if ( members > 1 )
	group_levels += (members - 1);

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	if ( gch->in_room != victim->in_room )
	    continue;

	xp = xp_compute( gch, victim, group_levels );  

	if ( IS_SET(victim->act, ACT_NOEXP) )
	    xp = 0;
	
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );
    }
    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp, min_bonus, max_bonus;
    int level_range;

    level_range = victim->level - total_levels;
 
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -5	:	base_exp =   5;		break;
	case -4 :	base_exp =  10;		break;
	case -3 :	base_exp =  25;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  75;		break;
	case  0 :	base_exp =  95;		break;
	case  1 :	base_exp = 100;		break;
	case  2 :	base_exp = 105;		break;
	case  3 :	base_exp = 110;		break;
	case  4 :	base_exp = 120;		break;
	case  5 :	base_exp = 130;		break;
	case  6 :	base_exp = 140;		break;
    }
    
    if (level_range > 6)
	base_exp = 140 + 8 * (level_range - 6);

    min_bonus = URANGE(0, 10 + level_range, 20);
    max_bonus = URANGE(0, 20 + level_range / 2, 40);

    base_exp += number_range(min_bonus, min_bonus);

    /* calculate exp multiplier */
    xp = base_exp;

    /* more exp at the low levels */
    if (total_levels < 6)
    	xp = xp * 6 / (total_levels);

    /* less at high */
    if (total_levels > 75 )
	xp = xp * 35 / (total_levels - 40);

    if ( IS_NPC(victim) && victim->pIndexData->killed > 15 )
    {
	if ( victim->pIndexData->killed < 115 )
	    xp = xp * (50 - (victim->pIndexData->killed-15) / 2) / 50;
	else
	    xp = xp * 1 / 51;
    }
 
    /* randomize the rewards */
    xp = number_range( xp * 9/10, xp * 11/10 );

    /* adjust for grouping  */
    if ( total_levels != gch->level ) /* is in a group */
    {
	xp = xp * gch->level / UMAX(1, (total_levels - 1));
	xp = xp * 5 / 4;
    }
    else
	xp = xp * gch->level / total_levels; 

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune, int location )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *hp;
    const char *attack;
    char punct;
    int value;

    if ( victim->hit > 0 )
	value = dam * 100 / victim->hit;
    else
	value = 110;

         if ( value == 0
         &&   dam   == 0   ) { hp = "causing no wounds to";     }
    else if ( value == 0
         &&   dam   != 0   ) { hp = "hardly touching";          }
    else if ( value <= 2   ) { hp = "merely tapping";           }
    else if ( value <= 5   ) { hp = "only nicking";             }
    else if ( value <= 10  ) { hp = "barely scratching";        }
    else if ( value <= 15  ) { hp = "grazing";                  }
    else if ( value <= 30  ) { hp = "slightly wounding";        }
    else if ( value <= 45  ) { hp = "injuring";                 }
    else if ( value <= 60  ) { hp = "moderately wounding";      }
    else if ( value <= 75  ) { hp = "wounding";                 }
    else if ( value <= 90  ) { hp = "gravely wounding";         }
    else if ( value <= 95  ) { hp = "almost killing";           }
    else if ( value <= 100 ) { hp = "fatally wounding";         }
    else                     { hp = "killing";                  }

         if ( dam ==   0 ) { vs = "miss";       vp = "misses";          }
    else if ( dam <=   2 ) { vs = "scratch";    vp = "scratches";       }
    else if ( dam <=   4 ) { vs = "graze";      vp = "grazes";          }
    else if ( dam <=  10 ) { vs = "hit";        vp = "hits";            }
    else if ( dam <=  16 ) { vs = "injure";     vp = "injures";         }
    else if ( dam <=  20 ) { vs = "wound";      vp = "wounds";          }
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";           }
    else if ( dam <=  32 ) { vs = "decimate";   vp = "decimates";       }
    else if ( dam <=  44 ) { vs = "devastate";  vp = "devastates";      }
    else if ( dam <=  60 ) { vs = "maim";       vp = "maims";           }
    else if ( dam <=  75 ) { vs = "MUTILATE";   vp = "MUTILATES";       }
    else if ( dam <=  90 ) { vs = "DISEMBOWEL"; vp = "DISEMBOWELS";     }
    else if ( dam <= 105 ) { vs = "DISMEMBER";  vp = "DISMEMBERS";      }
    else if ( dam <= 120 ) { vs = "MASSACRE";   vp = "MASSACRES";       }
    else if ( dam <= 150 ) { vs = "MANGLE";     vp = "MANGLES";         }
    else if ( dam <= 180 ) { vs = "*** DEMOLISH ***";
                             vp = "*** DEMOLISHES ***";                 }
    else if ( dam <= 210 ) { vs = "*** DEVASTATE ***";
                             vp = "*** DEVASTATES ***";                 }
    else if ( dam <= 230 ) { vs = "=== OBLITERATE ===";
                             vp = "=== OBLITERATES ===";                }
    else if ( dam <= 260 ) { vs = ">>> ANNIHILATE <<<";
                             vp = ">>> ANNIHILATES <<<";                }
    else if ( dam <= 300 ) { vs = "<<< ERADICATE >>>";
                             vp = "<<< ERADICATES >>>";                 }
    else                   { vs = "do UNSPEAKABLE things to";
                             vp = "does UNSPEAKABLE things to";         }

    punct   = (dam <= 75) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $melf%s, %s $m%c",
		vp,dam == 0 ? "" :wear_name[location],hp,punct);
	    sprintf( buf2, "`3You %s yourself%s, %s yourself%c`n",
		vs,dam == 0 ? "" :wear_name[location],hp,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%s, %s $M%c",
		vp,dam == 0 ? "" :wear_name[location],hp, punct );
	    sprintf( buf2, "You %s $N%s, %s $M%c",
		vs,dam == 0 ? "" :wear_name[location],hp, punct );
	    sprintf( buf3, "`3$n %s you%s, %s you%c`n",
		vp,dam == 0 ? "" :wear_name[location],hp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].name;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"`3Luckily, you are immune to that.`n");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	    	sprintf(buf2,"$N is unaffected by your %s!",attack);
	    	sprintf(buf3,"`3$n's %s is powerless against you.`n",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%s, %s $m%c",
		    attack,vp,dam == 0 ? "":wear_name[location],hp,punct);
		sprintf( buf2, "`3Your %s %s you%s, %s you%c`n",
		    attack,vp,dam == 0 ? "":wear_name[location],hp,punct);
	    }
	    else
	    {
	    	sprintf( buf1, "$n's %s %s $N%s, %s $M%c",
		    attack,vp,dam == 0 ? "" :wear_name[location],hp,punct);
	    	sprintf( buf2, "Your %s %s $N%s, %s $M%c",
		    attack,vp,dam == 0 ? "" :wear_name[location],hp,punct);
	    	sprintf( buf3, "`3$n's %s %s you%s, %s you%c`n",
		    attack,vp,dam == 0 ? "" :wear_name[location],hp,punct);
	    }
	}
    }

    if (ch == victim)
    {
	act_fight(buf1,ch,NULL,victim,TO_ROOM);
	act_fight(buf2,ch,NULL,victim,TO_CHAR);
    }
    else
    {
    	act_fight( buf1, ch, NULL, victim, TO_NOTVICT );
    	act_fight( buf2, ch, NULL, victim, TO_CHAR );
    	act_fight( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "`3$n disarms you and sends your weapon flying!`n", 
	 ch, NULL, victim, TO_VICT    );
    act( "`3You disarm $N!`n",  ch, NULL, victim, TO_CHAR    );
    act( "`3$n disarms $N!`n",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP)
    ||   IS_OBJ_STAT(obj,ITEM_INVENTORY)
    ||   is_guild_eq(obj->pIndexData->vnum) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
        send_to_char( "You must MURDER a player.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( CHARM_SET(ch) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	if ( !check_skill(ch, gsn_switch_opponent, 50, TRUE) )
	{
	    send_to_char( "You try to switch opponents, but only become confused.\n\r", ch );
	    WAIT_STATE( ch, 6 );
	    check_improve( ch, gsn_switch_opponent, FALSE, 6 );
	}
	else
	{
	    act( "You switch opponents, attacking $N instead.", ch, NULL,
		victim, TO_CHAR );
	    act( "$n switches opponents, and begins attacking you.", ch, NULL,
		victim, TO_VICT );
	    act( "$n switches opponents, attacking $N instead.", ch, NULL,
		victim, TO_NOTVICT );
	    check_improve( ch, gsn_switch_opponent, TRUE, 4 );
	    stop_fighting( ch, FALSE );
	    set_fighting( ch, victim );
	    WAIT_STATE( ch, 4 );
	}
	return;
    }

    WAIT_STATE( ch, 3 );
    act( "$n suddenly attacks $N!", ch, NULL, victim, TO_NOTVICT );
    act( "$n suddenly attacks you!", ch, NULL, victim, TO_VICT );
    act( "You attack $N!", ch, NULL, victim, TO_CHAR );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if ( CHARM_SET(ch) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "You must KILL a non-player.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( CHARM_SET(ch) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 3 );

    sprintf(buf,"$N is attempting to murder %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);

    act( "$n suddenly attacks $N!", ch, NULL, victim, TO_NOTVICT );
    act( "$n suddenly attacks you!", ch, NULL, victim, TO_VICT );
    act( "You attack $N!", ch, NULL, victim, TO_CHAR );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( (victim = ch->fighting) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
        ||  IS_AFFECTED(ch,AFF_BERSERK)
	||  is_affected(ch,gsn_berserk)
	||  (ch->stamina <= 0 && number_bits( 4 )) )
	    continue;

	move_char( ch, door, FALSE );

	if ( (now_in = ch->in_room) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!  You lose 10 exps.\n\r", ch );
	    gain_exp( ch, -10 );
	}

	if ( ch->pet != NULL )
	    do_flee( ch->pet, "" );

	stop_fighting( ch, TRUE );
	return;
    }

    if ( IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk) )
	send_to_char( "You don't feel like running.  You want to FIGHT!\n\r", ch );
    else
        send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( !str_cmp(arg, "all") )
    {
	for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
	{
	    if ( victim == ch )
		continue;

	    if ( !is_same_group(ch,victim))
		continue;

	    if ( !IS_NPC(ch) && IS_NPC(victim) )
		continue;

	    if ( ch->fighting == victim )
		continue;

	    if ( ( fch = victim->fighting ) == NULL )
		continue;

	    if ( !check_skill(ch, gsn_heroic_rescue, 100, TRUE) )
	    {
		send_to_char( "You fail the rescue.\n\r", ch );
		check_improve(ch,gsn_heroic_rescue, FALSE, 4);
		continue;
	    }

	    act( "You heroically rescue $N!",  ch, NULL, victim, TO_CHAR    );
	    act( "$n heroically rescues you!", ch, NULL, victim, TO_VICT    );
	    act( "$n heroically rescues $N!",  ch, NULL, victim, TO_NOTVICT );
	    check_improve(ch,gsn_heroic_rescue,TRUE,4);

	    stop_fighting( fch, FALSE );
	    stop_fighting( victim, FALSE );
	    set_fighting( ch, fch );
	    set_fighting( fch, ch );
	}
	WAIT_STATE( ch, skill_table[gsn_rescue].beats );
	return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !check_skill(ch, gsn_rescue, 100, TRUE) )
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( ch, FALSE );
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim, ch, DAM_SLAY );
    return;
}

void do_assist( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Assist whom?\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->fighting == NULL )
    {
	send_to_char( "They aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
    {
	send_to_char( "You're already fighting someone!\n\r", ch );
	return;
    }

    act( "You rush to help $N!\n\r", ch, NULL, victim, TO_CHAR );
    act( "$n rushes to $N's aid.", ch, NULL, victim, TO_NOTVICT );
    act( "$n rushes to your aid.", ch, NULL, victim, TO_VICT );

    multi_hit(ch, victim->fighting, TYPE_UNDEFINED);
}

void group_heal( CHAR_DATA *ch )
{
    int healing;
    OBJ_DATA *ointment;
    CHAR_DATA *vch;
    bool found_oint = FALSE;

    if ( IS_NPC(ch) )
	return;

    if ( ch->pcdata->learned[gsn_medicine] < 1 )
	return;

    for ( ointment = ch->carrying; ointment != NULL;
	  ointment = ointment->next_content )
    {
	if ( ointment->pIndexData->vnum == OBJ_VNUM_OINTMENT )
	{
	    found_oint = TRUE;
	    break;
	}
    }

    if ( !found_oint )
	return;

    if ( !check_skill(ch, gsn_medicine, 95, TRUE) )
    {
	check_improve( ch, gsn_medicine, FALSE, 5 );
	return;
    }

    if ( found_oint )
	extract_obj( ointment );

    for ( vch = ch->in_room->people; vch != NULL;
	  vch = vch->next_in_room )
    {
	int one_percent;

	if ( !is_same_group(ch, vch) )
	    continue;

	one_percent = UMAX( 1, vch->max_hit / 100 );

	healing = dice( 2, 10 * one_percent );

	if ( vch->position == POS_MORTAL
	||   vch->position == POS_INCAP )
	    healing = number_bits( 2 );

	gain_health( vch, healing, FALSE );
	act( "$n checks you over and treats some of your wounds.", ch,
	    NULL, vch, TO_VICT );
    }
    send_to_char( "You look everyone over and treat their wounds.\n\r",ch );
    return;
}

bool check_break( OBJ_DATA *obj, int damage )
{
    int chance;
    CHAR_DATA *ch, *rch;

    chance = number_percent( );
    ch = obj->carried_by;
    if ( obj->in_room != NULL )
	rch = obj->in_room->people;
    else
	rch = NULL;

    if ( obj->material == MAT_HEARTSTONE
    ||	 obj->material == MAT_UNKNOWN
    ||   IS_SET(obj->extra_flags, ITEM_NOBREAK)
    ||   IS_SET(obj->extra_flags, ITEM_QUEST)
    ||   is_guild_eq(obj->pIndexData->vnum) )
	return FALSE;

    if ( obj->condition <= -100 )
	return TRUE;

    if ( obj->item_type == ITEM_ARMOR
    &&   obj->wear_loc  != WEAR_SHIELD )
	chance = chance * 2;

    if ( damage <= chance * break_table[obj->material].chance / 100 )
	return FALSE;

    if ( ch != NULL )
    {
	act( "`6You frown, $p seems a bit damaged.`n", ch, obj,
	    NULL, TO_CHAR );
	act_fight( "$n frowns, $p seems a bit damaged.", ch, obj,
	    NULL, TO_ROOM );
    }
    if ( rch != NULL )
	act_fight( "$p seems a bit damaged.", rch, obj, NULL, TO_ALL );

    obj->condition -= UMAX( 1, (damage / 10) );
    obj->condition  = UMAX( -100, obj->condition );

    if ( IS_SET(obj->extra_flags, ITEM_BENT)
    &&   obj->condition < 0
    &&	 number_percent() < (obj->condition * -1) )
    {
	if ( ch != NULL )
	{
	    act( "`3You gasp in shock as $p breaks!`n", ch, obj,
		NULL, TO_CHAR );
	    act_fight( "$n gasps in shock as $p breaks.", ch, obj,
		NULL, TO_ROOM );
	}
	if ( rch != NULL )
	    act_fight( "$p suddenly breaks.", rch, obj, NULL, TO_ALL );
	REMOVE_BIT( obj->extra_flags, ITEM_BENT );
	SET_BIT( obj->extra_flags, ITEM_RUINED );
	return TRUE;
    }

    if ( obj->condition <= 0 )
    {
	switch( break_table[obj->material].affect )
	{
	    default:
	    case AFF_NOTHING:
		break;
	    case AFF_SHATTER:
		if ( ch != NULL )
		{
		    act( "`3You gasp in shock as $p breaks!`n", ch, obj,
			NULL, TO_CHAR );
		    act_fight( "$n gasps in shock as $p breaks.", ch, obj,
			NULL, TO_ROOM );
		}
		if ( rch != NULL )
		    act_fight( "$p suddenly breaks.", rch, obj,
			NULL, TO_ALL );
		SET_BIT( obj->extra_flags, ITEM_RUINED );
		return FALSE;
	    case AFF_BEND:
		if ( ch != NULL )
		{
		    act( "`3You frown in dismay as $p bends out of shape.`n", ch, obj,
			NULL, TO_CHAR );
		    act_fight( "$n frowns in dismay as $p bends out of shape.", ch, obj,
			NULL, TO_ROOM );
		}
		if ( rch != NULL )
		    act_fight( "$p bends out of shape.", rch, obj,
			NULL, TO_ALL );
		SET_BIT( obj->extra_flags, ITEM_BENT );
		return TRUE;
	    case AFF_TEAR:
		if ( ch != NULL )
		{
		    act( "`3You gasp in shock as $p tears into pieces!`n",
			ch, obj, NULL, TO_CHAR );
		    act_fight( "$n gasps in shock as $p tears into pieces.",
			ch, obj, NULL, TO_ROOM );
		}
		if ( rch != NULL )
		    act_fight( "$p tears into pieces.", rch, obj,
			NULL, TO_ALL );
		SET_BIT( obj->extra_flags, ITEM_RUINED );
		return FALSE;
	}
    }
    return FALSE;
}

int hit_loc( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int roll;

    roll =  number_percent( );
    if ( ch->size == SIZE_TINY )
	roll -= 60;
    else if ( ch->size == SIZE_SMALL )
	roll -= 30;
    else if ( ch->size == SIZE_MEDIUM )
	;
    else if ( ch->size == SIZE_LARGE )
	roll += 20;
    else if ( ch->size == SIZE_HUGE )
	roll += 40;
    else
	roll += 60;

    if ( dt == gsn_backstab )
	return WEAR_BODY;

         if ( roll >= 90 && IS_SET(victim->parts, PART_HEAD) )
	return WEAR_HEAD;
    else if ( roll >= 85 )
    {
	if ( number_bits(1) == 0 && IS_SET(victim->parts, PART_HEAD) )
	    return WEAR_NECK_1;
	else
	    return WEAR_NECK_2;
    }
    else if ( roll >= 55 )
	return WEAR_BODY;
    else if ( roll >= 40 && IS_SET(victim->parts, PART_ARMS) )
	return WEAR_ARMS;
    else if ( roll >= 30 && IS_SET(victim->parts, PART_HANDS) )
    {
	if ( number_bits(1) == 0 )
	    return WEAR_WRIST_L;
	else
	    return WEAR_WRIST_R;
    }
    else if ( roll >= 30 && IS_SET(victim->parts, PART_HANDS) )
	return WEAR_HANDS;
    else if ( roll >= 20 )
	return WEAR_WAIST;
    else if ( roll >= 5 && IS_SET(victim->parts, PART_LEGS) )
	return WEAR_LEGS;
    else if ( IS_SET(victim->parts, PART_FEET) )
	return WEAR_FEET;

    return WEAR_BODY;
}	

int get_required( CHAR_DATA *ch, int sn, int weapon )
{
    int skill;
    int gsn_wc;

    gsn_wc = guild_lookup( "whitecloak" );

     /* -1 is exotic */
    if ( IS_NPC(ch) )
    {
        if ( sn == -1 )
            skill = ch->level * 5 / 6;
        else if (sn == gsn_hand_to_hand)
            skill = 40 + ch->level * 5 / 6;
        else
	{
	    skill = get_skill(ch, sn);

	    if ( ch->level >= 100 )
		skill += 10;
	    else if ( ch->level >= 90 )
		skill += 11;
	    else if ( ch->level >= 80 )
		skill += 12;
	    else if ( ch->level >= 70 )
		skill += 13;
	    else if ( ch->level >= 60 )
		skill += 14;
	    else if ( ch->level >= 50 )
		skill += 15;
	    else if ( ch->level >= 40 )
		skill += 16;
	    else if ( ch->level >= 30 )
		skill += 17;
	    else if ( ch->level >= 20 )
		skill += 18;
	    else if ( ch->level >= 10 )
		skill += 19;
	}
    }
    else
    {
        if ( sn == -1 )
            skill = ch->level * 4 / 5;
        else 
	{
	    if ( ch->pcdata->learned[sn] < 1 )
		skill = 0;
	    else
		skill = SKILL(ch,sn);
	}
    }

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	if ( check_skill(ch, gsn_blindfighting, 50, FALSE) )
	    skill = skill * 3 / 5;
	else
	    skill = skill * 3 / 8;
    }

    if ( ch->guild == gsn_wc
    &&   check_skill(ch, gsn_group_fighting, 75, TRUE) )
    {
	int rank, number;
	CHAR_DATA *och;
	rank = 0;
	number = 0;

	for ( och = ch->in_room->people; och; och = och->next_in_room )
	{
	    if ( och->guild == gsn_wc
	    &&   !IS_NPC(och)
	    &&   is_same_group(ch, och) )
	    {
		rank += GET_RANK(och,1);
		number++;
	    }
	}
	if ( number > 1 )
	{
	    skill += UMAX(1, rank * 2 / number );
	    act_fight( "The combat tactics of the Children of the Light make victory seem inevitable!", ch, NULL, NULL, TO_ALL );
	}
    }

    return weapon_table[weapon].diff - ( (skill - 60) / 2 );
}

/*
 * Evasion modification for armor weight
 */
int armor_weight( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int i;
    int total;

    total = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	int temp = 0;
	if ( obj->wear_loc < 0 )
	    continue;

	if ( obj->item_type != ITEM_ARMOR )
	    continue;

	if ( !str_cmp("none", flag_string( armor_flags, obj->value[4] )) )
	    continue;

	for ( i = 0; i < 4; i++ )
	    temp += obj->value[i];
	total += (UMAX( 1, temp / 4 ) * break_table[obj->material].weight_mult / 100);
    }

    return( -10 + (total / 18) );
}

int absorb_damage( CHAR_DATA *ch, int dam, int dam_type, int location )
{
    int damage;
    int absorb = 0;
    OBJ_DATA *obj;

    if ( dam < 1 )
	return 0;

    if ( (obj = get_eq_char(ch, WEAR_SHIELD))
    &&   (dam_type == DAM_PIERCE
    ||    dam_type == DAM_SLASH
    ||    dam_type == DAM_BASH)
    &&  number_percent() <= CONDITION(obj) * 2 )
    {
	if ( check_skill(ch, gsn_shield_block, 80, TRUE) )
	{
	    check_improve( ch, gsn_shield_block, TRUE, 4 );
	    check_break( obj, dam * 9 / 10 );
	    if ( number_percent() >= 8 )
	    {
		act_fight( "$n deflect$% the blow with $p.", ch, obj, NULL,
		    TO_ALL );
		return 0;
	    }
	    else
	    {
		act_fight( "$n deflect$% the blow, but still suffer$% some of the affect.", 
		    ch, NULL, NULL, TO_ALL );
		return number_range(1, UMAX(1, dam/8));
	    }
	}
	else 
	    check_improve( ch, gsn_shield_block, FALSE, 5 );
    }

    if ( (obj = get_eq_char( ch, location )) )
    {
	if ( obj->item_type != ITEM_ARMOR )
	    absorb = 1;
	else
	{
	    switch( dam_type )
	    {
		case( DAM_PIERCE ):
		    absorb = obj->value[0];
		    break;
		case( DAM_BASH ):
		    absorb = obj->value[1];
		    break;
		case( DAM_SLASH ):
		    absorb = obj->value[2];
		    break;
		case( DAM_FIRE ):
		    absorb = obj->value[3];
		    break;
		case( DAM_COLD ):
		    absorb = obj->value[4];
		    break;
		case( DAM_LIGHTNING ):
		    absorb = obj->value[5];
		    break;
		default:
		    absorb = obj->value[6];
		    break;
	    }
	}
	absorb = absorb * (CONDITION(obj) / 4) / 25;
	check_break( obj, dam * 9 / 10 );
    }

    switch ( location )
    {
	default:
	    absorb = 0;
	    break;
	case WEAR_NECK_1:
	case WEAR_NECK_2:
	    break;
	case WEAR_BODY:
	    absorb *= 5;
	    if ( (obj = get_eq_char( ch, WEAR_ABOUT )) )
	    {
		int about;
		if ( obj->item_type == ITEM_ARMOR )
		{
		    switch( dam_type )
		    {
			case( DAM_PIERCE ):
			    about = obj->value[0];
			    break;
			case( DAM_BASH ):
			    about = obj->value[1];
			    break;
			case( DAM_SLASH ):
			    about = obj->value[2];
			    break;
			case( DAM_FIRE ):
			    about = obj->value[3];
			    break;
			case( DAM_COLD ):
			    about = obj->value[4];
			    break;
			case( DAM_LIGHTNING ):
			    about = obj->value[5];
			    break;
			default:
			    about = obj->value[6];
			    break;
		    }
		}
		else
		    about = 1;
		about = about * (CONDITION(obj) / 4) / 25;
		check_break( obj, dam * 9 / 10 );
		absorb += about;
	    }
	    break;
	case WEAR_HEAD:
	    absorb *= 2;
	    break;
	case WEAR_LEGS:
	    absorb *= 3;
	    break;
	case WEAR_FEET:
	    break;
	case WEAR_HANDS:
	    break;
	case WEAR_ARMS:
	    absorb *= 3;
	    break;
	case WEAR_WAIST:
	    absorb *= 2;
	    break;
	case WEAR_WRIST_L:
	case WEAR_WRIST_R:
	    break;
    }

    switch( dam_type )
    {
	case( DAM_PIERCE ):
	    absorb += ch->armor[0] / 5;
	    break;
	case( DAM_BASH ):
	    absorb += ch->armor[1] / 5;
	    break;
	case( DAM_SLASH ):
	    absorb += ch->armor[2] / 5;
	    break;
	case( DAM_FIRE ):
	    absorb += ch->armor[3] / 5;
	    break;
	case( DAM_COLD ):
	    absorb += ch->armor[4] / 5;
	    break;
	case( DAM_LIGHTNING ):
	    absorb += ch->armor[5] / 5;
	    break;
	default:
	    absorb += ch->armor[6] / 5;
	    break;
    }

    if ( absorb == 0 )
	return dam;

    absorb = UMIN( absorb, 95 );
    damage = dam - UMAX( 1, (dam * absorb / 100) );

    return UMAX( damage, 1 );
}

int roll_damage( CHAR_DATA *ch, OBJ_DATA *wield, sh_int sn, sh_int weapon )
{
    int dam;

    if ( IS_NPC(ch)
    &&   (!ch->pIndexData->new_format
    ||    wield == NULL
    ||    ( wield && IS_SET(wield->extra_flags, ITEM_RUINED) )) )
    {
	if ( !ch->pIndexData->new_format )
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( ch->race <= race_lookup("human") )
		dam = dice( 1 + dice(1, 4), 2 );
	}
	else
	{
	    if ( ch->race <= race_lookup("human") )
		dam = dice( 1 + dice(1, 4), 2 );
	    else
		dam = dice( ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE] );
	}

	if ( wield )
	    dam += dam / 2;
    }
    else
    {
	if ( sn != -1 )
	    check_improve( ch, sn, TRUE,
		UMAX(1, weapon_table[weapon].diff/20) );
	if ( wield
	&&   !IS_SET(wield->extra_flags, ITEM_RUINED)
	&&   wield->item_type == ITEM_WEAPON )
	{
	    if ( wield->pIndexData->new_format )
		dam = dice( wield->value[1], wield->value[2] );
	    else
		dam = number_range( wield->value[1], wield->value[2] );

	    if ( get_eq_char(ch, WEAR_SHIELD) == NULL )
		dam = dam * 21 / 20;

	    dam = dam + number_range( 0, wield->value[6] );
	    dam = dam * ( 50 + (wield->condition / 2) ) / 100; 
	    dam = UMAX( 1, dam );
	}
	else
	{
	    if ( ch->race <= race_lookup("human") )
		dam = dice( 1, 3 );
	    else
		dam = number_range( 1, 2 * ch->level / 3 );
	}
    }
    return dam;
}


OBJ_DATA *find_item_room( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj;

    if ( !ch || !ch->in_room )
	return NULL;

    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
	if ( obj->item_type == type )
	    return obj;
    }
    return NULL;
}

