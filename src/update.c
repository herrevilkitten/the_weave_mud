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

/* command procedures needed */
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_release	);
void    raw_kill        args( ( CHAR_DATA *victim, CHAR_DATA *ch, int dt ) );
void	free_node	args( (NODE_DATA* node) );
void	insane		args( (CHAR_DATA *ch, int total) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	stamina_gain	args( ( CHAR_DATA *ch ) );
void	min_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	hour_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	second_update	args( ( void ) );
void	affect_update	args( ( void ) );
/* used for saving */

int	save_number = 0;



/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool display )
{
    char buf[MAX_STRING_LENGTH];
    int i;
    int add_hp;
    int add_stamina;
    int add_prac;
    int add_train;
    BUFFER *buffer;

    buffer = new_buf();
    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp
		+ number_range( class_table[ch->class].hp - 2,
				class_table[ch->class].hp + 2 );

    if ( !can_channel(ch, 1) )
	add_hp += number_range( 4, 10 );

    add_stamina	= ( (2 * get_curr_stat( ch, STAT_CON )
		+ get_curr_stat( ch, STAT_STR )) / 3 ) * 3 / 8;

    if ( is_forsaken(ch) )
    {
	add_hp		= con_app[25].hitp + class_table[ch->class].hp * 2;
	add_stamina	= 12;
    }
    add_stamina = UMAX( 1, add_stamina );

    if ( ch->level > ch->pcdata->max_level )
    {
	ch->pcdata->max_level = ch->level;
	add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;
	if ( number_percent() >
	    (95 - ( luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod / 4 )) )
	    add_prac	+= 1;

	add_train		= 1;
	if ( number_percent() >
	    (97 - ( luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod / 4 )) )
	    add_train	+= 1;
    }
    else
    {
	add_train = 0;
	add_prac = 0;
    }

    if ( is_forsaken(ch) )
    {
	if ( ch->class == 0 )
	{
	    add_hp += 3;
	    add_stamina -= 1;
	}
	if ( ch->class == 2 )
	{
	    add_stamina += 3;
	    add_hp -= 1;
	}
	add_stamina += class_table[ch->class].stam;
    }
    else
    {
	if ( ch->class == 0 )
	{
	    add_hp += number_range( 2, 4 );
	    add_stamina -= number_range( 1, 3 );
	}
	if ( ch->class == 2 )
	{
	    add_stamina += number_range( 2, 4 );
	    add_hp -= number_range( 1, 3 );
	}
	add_stamina += class_table[ch->class].stam - 1;
    }

    add_hp	= UMAX(  1, add_hp   );
    add_stamina	= UMAX(  1, add_stamina );

    ch->max_hit 	+= add_hp;
    ch->max_stamina	+= add_stamina;
    ch->practice	+= add_prac;
    ch->train		+= add_train;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_stamina	+= add_stamina;

    sprintf( buf,
	"Your gain is: %d/%d hp, %d/%d stamina, %d/%d train, %d/%d prac.\n\r",
	add_hp,		ch->max_hit,
	add_stamina,	ch->max_stamina,
	add_train,	ch->train,
	add_prac,	ch->practice
	);
    add_buf( buffer, buf );
    for ( i = 0; i < MAX_STATS; i++ )
    {
	if ( number_range(1, 500) < ch->pcdata->stat_use[i]
	&&   ch->perm_stat[i] < 20 + pc_race_table[ch->race].stats[i] )
	{
	    if ( i == 0 )
		add_buf( buffer, "You feel stronger!\n\r" );
	    else if ( i == 1 )
		add_buf( buffer, "You feel smarter!\n\r" );
	    else if ( i == 2 )
		add_buf( buffer, "You feel wiser!\n\r" );
	    else if ( i == 3 )
		add_buf( buffer, "You feel more dextrous!\n\r" );
	    else if ( i == 4 )
		add_buf( buffer, "You feel healthier!\n\r" );
	    else if ( i == 5 )
		add_buf( buffer, "You feel more charismatic!\n\r" );
	    else if ( i == 6 )
		add_buf( buffer, "You feel luckier!\n\r" );
	    else if ( i == 7 )
		add_buf( buffer, "You feel more agile!\n\r" );
	    ch->perm_stat[i]++;
	    ch->pcdata->stat_use[i] = 0;
	}
    }
    if ( display == TRUE )
        page_to_char( buf_string(buffer), ch );
    free_buf( buffer );
    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    char buf[MAX_STRING_LENGTH];
    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO - 1 )
	return;

    ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );
    while ( ch->level < LEVEL_HERO-1 && ch->exp >= 
	exp_per_level(ch,ch->pcdata->points) * (ch->level+1) )
    {
	send_to_char( "You raise a level!!  ", ch );
	ch->level += 1;
        sprintf(buf,"%s gained level %d",ch->name,ch->level);
        log_string(buf);
        sprintf(buf,"$N has attained level %d!",ch->level);
        wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
	advance_level( ch, TRUE );
	save_char_obj(ch);
    }

    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	int number;
	number = get_curr_stat( ch, STAT_CON );
	gain =	number_range( class_table[ch->class].hp - 2,
			      class_table[ch->class].hp + 2 );
	gain += number_range( number / 2, number );

 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch ( ch->position )
	{
	    default:	   	;				break;
	    case POS_SLEEPING: 	gain = gain * 3;		break;
	    case POS_RECLINING: gain = gain * 5 / 2;		break;
	    case POS_RESTING:  	gain = gain * 2;		break;
	    case POS_SITTING:	gain = gain * 3 / 2;		break;
	    case POS_STANDING:					break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	gain = gain * race_table[ch->race].hit_mult / 100;
    }
    else
    {
	int number;
	number = get_curr_stat( ch, STAT_CON );
	gain =	number_range( class_table[ch->class].hp - 2,
			      class_table[ch->class].hp + 2 );
	gain += number_range( number / 2, number );

	if ( check_skill(ch, gsn_fast_healing, 100, TRUE) )
	{
	    gain += number_percent() * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve( ch, gsn_fast_healing, TRUE, 8 );
	}

	switch ( ch->position )
	{
	    default:	   	;				break;
	    case POS_SLEEPING: 	gain = gain * 3;		break;
	    case POS_RECLINING: gain = gain * 5 / 2;		break;
	    case POS_RESTING:  	gain = gain * 2;		break;
	    case POS_SITTING:	gain = gain * 3 / 2;		break;
	    case POS_STANDING:					break;
	    case POS_FIGHTING: 	gain /= 2;			break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( check_skill(ch, gsn_endurance, 66, TRUE) )
	gain += gain / 2;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 2;

    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
	gain /= 4;

    if ( IS_AFFECTED(ch,AFF_HASTE) )
	gain /= 2;

    gain = UMAX( 1, gain );

    return UMIN(gain, ch->max_hit - ch->hit);
}



int stamina_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain		= ( (2 * get_curr_stat( ch, STAT_CON )
			+ get_curr_stat( ch, STAT_STR )) / 3 ) * 3 / 8;
	gain +=	number_range( class_table[ch->class].stam - 2,
			      class_table[ch->class].stam + 2 );

	switch ( ch->position )
	{
	    default:	   	;				break;
	    case POS_SLEEPING: 	gain = gain * 3;		break;
	    case POS_RECLINING: gain = gain * 5 / 2;		break;
	    case POS_RESTING:  	gain = gain * 2;		break;
	    case POS_SITTING:	gain = gain * 3 / 2;		break;
	    case POS_STANDING:					break;
	    case POS_FIGHTING: 	gain /= 2;			break;
	}
    }
    else
    {
	gain		= ( (2 * get_curr_stat( ch, STAT_CON )
			+ get_curr_stat( ch, STAT_STR )) / 3 ) * 3 / 8;
	gain +=	number_range( class_table[ch->class].stam - 2,
			      class_table[ch->class].stam + 2 );

	switch ( ch->position )
	{
	    default:	   	;				break;
	    case POS_SLEEPING: 	gain = gain * 3;		break;
	    case POS_RECLINING: gain = gain * 5 / 2;		break;
	    case POS_RESTING:  	gain = gain * 2;		break;
	    case POS_SITTING:	gain = gain * 3 / 2;		break;
	    case POS_STANDING:					break;
	    case POS_FIGHTING: 	gain /= 2;			break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if ( check_skill(ch, gsn_endurance, 100, FALSE) )
	gain += gain / 2;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 2;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 4;

    if ( IS_SET(ch->body, BODY_BLEEDING) )
	gain /= 2;

    if (IS_AFFECTED(ch,AFF_HASTE))
        gain /= 2;

    gain = UMAX( 1, gain );

    if ( IS_GRASPING(ch) )
	gain = 0;

    return UMIN(gain, ch->max_stamina - ch->stamina);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_HERO)
	return;

    condition				= ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( IS_WRITING(ch) )
	return;

    if ( ch->pcdata->condition[iCond] >= 45
    &&   value > 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are getting full.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "Your thirst is near quenched.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are EXTREMELY drunk.\n\r", ch );
	    break;
	}
    }
    else if ( ch->pcdata->condition[iCond] <= 5 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are getting hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are getting thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You feel less drunk.\n\r", ch );
	    break;
	}
    }
    else if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void min_update( void )
{
    static int lose;
    AFFECT_DATA *paf, *paf_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    if ( --lose <= 0 )
	lose = 10;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	for ( paf = ch->affected; paf; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == gsn_charm_person 
	    &&   check_stat(ch, STAT_WIS, -( paf->strength/10 )) )
	    {
		if ( paf->owner != NULL )
		    send_to_char_new( paf->owner, "%s's will unconsciously breaks through your weave.\n\r", PERS(ch, paf->owner) );
		affect_remove( ch, paf );
	    }
	}

	if ( ch->action_timer > 0 )
	{
	    if ( ch->action )
		(*ch->action) ( ch, ch->action_args );
	}

	if ( ch->position >= POS_STUNNED )
	{
	    if ( !IS_SET(ch->body, BODY_BLEEDING) )
	    {
		if ( ch->hit  < ch->max_hit )
		    ch->hit  += hit_gain(ch);
		else
		    ch->hit = ch->max_hit;
	    }
	    else
	    {
		int loss, chance = 1;

		loss = UMAX( 1, dice(2, 5) - get_curr_stat(ch,STAT_CON)/4);
		send_to_char( "`3* * You are bleeding badly! * *`n\n\r", ch );
		act( "$n bleeds heavily.", ch, NULL, NULL, TO_ROOM );
		lose_health( ch, loss, TRUE );
		if ( ch->position == POS_DEAD )
		{
		    if ( ch->fighting != NULL )
			group_gain( ch->fighting, ch );

		    sprintf( log_buf, "%s bled to death in %s [room %d]",
		        (IS_NPC(ch) ? ch->short_descr : ch->name),
		        ch->in_room->name, ch->in_room->vnum);

		    if (IS_NPC(ch))
		        wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
		    else
		        wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

		    if ( !IS_NPC(ch) )
		    {
		        int loss;
		        char buf[MAX_STRING_LENGTH];

		        switch (ch->level)
		        {
			default:
			    loss = -(exp_per_level(ch, ch->pcdata->points));
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
                	send_to_char( buf, ch );
                	gain_exp( ch, loss );
		    }
                    raw_kill( ch, ch, DAM_BLEEDING );
		    continue;
		}

		if ( check_skill(ch, gsn_fast_healing, 50, TRUE) )
		    chance++;
		if ( check_skill(ch, gsn_endurance, 50, TRUE) )
		    chance++;
		if ( check_skill(ch, gsn_hardiness, 50, TRUE) )
		    chance++;

		if ( number_bits(7) < chance )
		{
		    send_to_char( "`2* * Fortunately, your bleeding stops on its own! * *`n\n\r", ch );
		    REMOVE_BIT(ch->body, BODY_BLEEDING);
		}
	    }

	    if ( IS_LINKING(ch) )
		lose_stamina( ch, dice(3,4), FALSE, TRUE );
	    if ( ch->stamina < ch->max_stamina )
		ch->stamina += stamina_gain(ch);
	    else
		ch->stamina = ch->max_stamina;
	}

	if ( ch->mount != NULL && ch->in_room != ch->mount->in_room )
	{
	    ch->position = POS_STANDING;
	    ch->mount->rider = NULL;
	    ch->mount = NULL;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( ch->stamina <= 0 )
	{
	    int chance;

	    chance = ch->stamina * -2 / 3;

	    if ( number_percent() < chance )
	    {
		AFFECT_DATA af;
                af.type      = skill_lookup( "sleep" );
                af.strength     = 1;
                af.duration  = 4;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_SLEEP;
                af.bitvector_2 = 0;
                af.owner     = NULL;
                af.flags     = AFFECT_NOTCHANNEL;

                affect_to_char( ch, &af );

	        if ( IS_GRASPING(ch) )
                    do_release( ch, "" );

		ch->position = POS_SLEEPING;
		act( "$n falls to the ground, unconscious.", ch, NULL,
		    NULL, TO_ROOM );
		act( "You pass out from exhaustion!", ch, NULL, NULL,
		    TO_CHAR );
	    }
	}

	if ( !IS_NPC(ch) && !IS_AFFECTED_2(ch, AFF_STILL) )
	{
	    int i;
	    for ( i = 0; i < MAX_SKILL; i++ )
	    {
		if ( number_bits(2) == 0 )
		    ch->pcdata->usage[i]--;
		if ( ch->pcdata->usage[i] < 0 )
		    ch->pcdata->usage[i] = 0;
	    }

	    /* INSANE! */
	    if ( !is_protected(ch)
	    &&   lose == 8 )
	    {
		if ( check_stat(ch, STAT_WIS, ( ch->pcdata->insane / 20 ) * -1)
		&&   number_percent() <= ch->pcdata->insane
		&&   ch->pcdata->insane >= 1 )
		    insane( ch, ch->pcdata->insane );
	    }
	}

	/* Do cool storm stuff */
	if ( ch != NULL && ch->in_room != NULL
	&&   (is_room_affected( ch->in_room, gsn_hail_storm )
	||    is_room_affected( ch->in_room, gsn_lightning_storm )) )	
	{
	    CHAR_DATA *room;
	    int count, roll, dam, dt;

	    dt = DAM_NONE;
	    count = 0;
	    dam = 0;
	    roll = dice( 1, 100 );

	    for ( room = ch->in_room->people; room != NULL;
		  room = room->next_in_room)
		count++;

	    if ( is_room_affected(ch->in_room, gsn_hail_storm) )
	    {
		if  (roll <= ( 75 / count ))
		{
		    dt = DAM_BASH;
		    dam = dice( 8, 5 );
		    act( "$n screams as hail pelts $m!", ch, NULL, NULL, TO_ROOM );
		    act( "You scream in pain as hail pelts you!", ch, NULL, NULL, TO_CHAR );
		    lose_health( ch, dam, TRUE );
		}
	    }    

	    if ( is_room_affected(ch->in_room, gsn_lightning_storm) )
	    {
		if  (roll <= ( 30 / count ))
		{
		    dt = DAM_LIGHTNING;
		    dam = dice( 10, 10 );
		    act( "$n screams as a bolt of lightning strikes $m!", ch, NULL, NULL, TO_ROOM );
		    act( "You scream in pain as a bolt of lightning strikes you!", ch, NULL, NULL, TO_CHAR );
		    lose_health( ch, dam, TRUE );
		}
	    }

    	    if ( ch->position == POS_DEAD )
    	    {
		sprintf( log_buf, "%s got toasted by storms at %s [room %d]",
		    (IS_NPC(ch) ? ch->short_descr : ch->name),
		    ch->in_room->name, ch->in_room->vnum);

		if (IS_NPC(ch))
		    wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
		else
		    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

		if ( !IS_NPC(ch) )
		{
		    int loss;
		    char buf[MAX_STRING_LENGTH];

		    switch (ch->level)
		    {
			default:
			    loss = -(exp_per_level(ch, ch->pcdata->points));
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
                    send_to_char( buf, ch );
                    gain_exp( ch, loss );
		}
                raw_kill( ch, ch, dt );
		continue;
	    }
	}

	if ( IS_NPC(ch)
	&&   (ch->memory || ch->hunting) )
	{
	    CHAR_DATA *victim;
	    if ( ch->memory
	    &&   (victim = get_pc_from_id(ch->memory->id)) )
	    {
		int diff;

		diff = current_time - ch->memory->when;
		diff = (int) diff / 3600;

		if ( !check_stat(ch, STAT_INT, 1-diff) )
		{
		    free_mem_data( ch->memory );
		    ch->memory = NULL;
		}

		if ( IS_AFFECTED_2(victim, AFF_NOAGGRO) )
		{
		    free_mem_data( ch->memory );
		    ch->memory = NULL;
		}
	    }

	    if ( ch->memory && victim ==  NULL )
	    {
		free_mem_data( ch->memory );
		ch->memory = NULL;
	    }

	    if ( ch->hunting
	    &&   IS_AFFECTED_2(ch->hunting, AFF_NOAGGRO) )
	    {
		if ( !check_stat(ch, STAT_INT, -1) )
		    ch->hunting = NULL;
	    }
	}

	if ( !IS_NPC(ch) || ch->in_room == NULL || CHARM_SET(ch) )
	    continue;

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	if ( is_affected(ch, skill_lookup( "wrap" )) )
	    continue;

	/* Examine call for special procedure */
	if ( ch->pIndexData->spec_fun != 0 )
	{
	    if ( (*ch->pIndexData->spec_fun) (ch) )
		continue;
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&&   number_bits(2) == 0
	&&   (door = number_bits( 5 )) <= 5
	&&   (pexit = ch->in_room->exit[door]) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&&   !((IS_SET(ch->act, ACT_STAY_INSIDE)
	&&      !IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS))
	||     (IS_SET(ch->act, ACT_STAY_OUTSIDE)
	&&      IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS)))
	&&   (!IS_SET(ch->act, ACT_STAY_AREA)
	||    pexit->u1.to_room->area == ch->in_room->area) )
	{
	    move_char( ch, door, FALSE );
	}

/*	 Flee
	if ( ch->hit < ch->max_hit / 2
	&& ( door = number_bits( 3 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = pexit->u1.to_room->people;
		  rch != NULL;
		  rch  = rch->next_in_room )
	    {
		if ( !IS_NPC(rch) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
		move_char( ch, door, FALSE );
	}
*/
    }
    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&	 !IS_WRITING(d->character)
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}

void hour_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	weather_info.moonlight++;
	if ( weather_info.moonlight == 4 * MOON_PERIOD )
	    weather_info.moonlight = (weather_info.moonlight % 8);
	break;
    }

    if ( time_info.day   >= DAYS_PER_MONTH )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= MONTHS_PER_YEAR )
    {
	time_info.month = 0;
	time_info.year++;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&	 !IS_WRITING(d->character)
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }
}

/*
 * Do HP, mana and move increases for mobs and chars
 */

/*
 * Update all chars, including mobs.
 */
void timer_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    bool fDead = FALSE;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;
	fDead = FALSE;

        if ( !IS_NPC(ch) && ch->desc == NULL )
	{
	    if ( ch->timer >= 3 )
		ch_quit = ch;
	}

	++ch->timer;

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if ( !IS_IMMORTAL(ch) && IS_IDLE(ch) )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    act( "$n disappears into the void.",
			ch, NULL, NULL, TO_ROOM );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    if (ch->level > 1)
		        save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }

	    if ( ch->timer >= 30 )
		ch->timer = 15;

	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL,   -1 );
	    gain_condition( ch, COND_THIRST, -1 );
	}


	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        if (is_affected(ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int save, dam;

	    if (ch->in_room == NULL)
		return;
            
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == gsn_plague)
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            	return;
            }
        
            if (af->strength == 1)
            	return;
        
            plague.type 		= gsn_plague;
            plague.strength 		= af->strength - 1; 
            plague.duration 	= number_range(1,2 * plague.strength);
            plague.location		= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
	    plague.bitvector_2	= 0;
	    plague.flags	= AFFECT_NOTCHANNEL;
	    plague.owner	= NULL;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
            	switch(check_immune(vch,DAM_DISEASE))
            	{
            	    case(IS_NORMAL) 	: save = af->strength - 4;break;
            	    case(IS_IMMUNE) 	: save = 0;		break;
            	    case(IS_RESISTANT) 	: save = af->strength - 8;break;
            	    case(IS_VULNERABLE)	: save = af->strength; 	break;
            	    default		: save = af->strength-4;break;
            	}
            
                if (save != 0 && !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->level,5);
	    ch->stamina -= dam * 2;
	    if ( damage(ch, ch, dam, gsn_plague,DAM_DISEASE, FALSE) )
		fDead = TRUE;
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL)
	{
	    act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You shiver and suffer.\n\r", ch );
	    if ( damage(ch, ch, 2, gsn_poison, DAM_POISON, FALSE) )
		fDead = TRUE;
	}
	else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	    if ( damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE) )
		fDead = TRUE;
	}
	else if ( ch->position == POS_MORTAL )
	{
	    if ( damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE) )
		fDead = TRUE;
	}

	if ( fDead )
	    continue;

	if (ch->desc != NULL && ch->desc->descriptor % 10 == save_number)
	    save_char_obj(ch);

        if ( ch == ch_quit )
            do_quit( ch, "" );
    }

    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	if ( IS_SET(obj->extra_flags, ITEM_CHANNELED) )
	    continue;

	/* Examine call for special procedure */
	if ( obj->pIndexData->spec_fun != 0 )
	{
	    
	    if ( obj->timer <= 0 || --obj->timer > 0 )
	    {
		if ( (*obj->pIndexData->spec_fun) ( obj ) )
		    continue;
		else
		    continue;
	    }
	}
	else
	    if ( obj->timer <= 0 || --obj->timer > 0 )
		continue;

	message = NULL;
	switch ( obj->item_type )
	{
	default:              
		message = "$p crumbles into dust."; 
		break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse."; break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->gold += obj->cost/5;
	    else
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
	           && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
		if ( message != NULL )
		{
		    act( message, rch, obj, NULL, TO_ROOM );
		    act( message, rch, obj, NULL, TO_CHAR );
		}
	    }
	}

        if (obj->item_type == ITEM_CORPSE_PC && obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		if (obj->carried_by)  /* carried */
		    obj_to_char(t_obj,obj->carried_by);

		if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    NODE_DATA *node, *node_next;
    CHAR_DATA *wch;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( node = pc_list; node != NULL; node = node_next )
    {
	node_next = node->next;

	if ( node->data_type != NODE_PC )
	    continue;

	wch = (CHAR_DATA *) node->data;

	if ( IS_NPC(wch)
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( ch->memory )
	    {
		CHAR_DATA *victim = NULL;
		if ( (victim = get_pc_from_id( ch->memory->id )) )
		{
		    if ( ch->in_room == victim->in_room
		    &&   ch->fighting == NULL
		    &&   IS_SET(ch->memory->reaction, MEM_HOSTILE)
		    &&   can_see(ch, victim) )
		    {
			if ( IS_SENTIENT(ch) )
			    act( "$n scream$%, 'You're the one who attacked me!'",
				ch, NULL, victim, TO_ALL );
			else 
			    act( "$n look$% at $N.", ch, NULL, victim, TO_ALL );
			act( "$n suddenly attacks $N!", ch, NULL, victim, TO_ALL );
			multi_hit( ch, victim, TYPE_UNDEFINED );
			free_mem_data( ch->memory );
			ch->memory = NULL;
			continue;
		    }
		}
	    }

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   CHARM_SET(ch)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0
	    ||   IS_AFFECTED_2(wch, AFF_NOAGGRO) )
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see(ch, vch)
		&&   !(IS_AFFECTED(vch, AFF_SNEAK) && number_bits(1) == 0) )
		{
		    if ( number_range(0, count) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    if ( IS_ROGUE(ch)
	    &&   check_skill(ch, gsn_backstab, 90, FALSE) )
	    {
		act( "$n sneak$% up on $N and stab$% $M in $O back!",
		    ch, NULL, victim, TO_ALL );
		multi_hit( ch, victim, gsn_backstab );
	    }
	    else
	    {
		act( "$n suddenly attack$% $N!", ch, NULL, victim, TO_ALL );
		multi_hit( ch, victim, TYPE_UNDEFINED );
	    }
	}
    }

    return;
}


void second_update( )
{
    DESCRIPTOR_DATA *d, *d_next;
    NODE_DATA *node, *node_next;

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	CHAR_DATA *ch;
	d_next = d->next;
	d->idle++;
	
	ch = d->character;
	
	if ( ch != NULL )
	{
	    if ( !IS_IMMORTAL(ch)
	    &&   d->idle >= 600
	    &&   !IS_WRITING(ch) )
		do_quit( ch, "" );

	    if ( !IS_IMMORTAL(ch)
	    &&   IS_GRASPING(ch)
	    &&   IS_IDLE(ch) )
		do_release( ch, "" );

	    if ( d->idle >= 1800 )
		do_quit( ch, "" );
	}
	else
	{	/* no character .. so we just close_socket */
	   if ( d->idle >= 300 )
		close_socket( d );
	}
    }

    for ( node = wait_list; node != NULL; node = node_next )
    {
	CHAR_DATA *ch;

	node_next = node->next;
	if ( node->data_type != NODE_WAIT )
	    continue;

	ch = (CHAR_DATA *) node->data;
	--ch->wait;

	if ( ch->wait <= 0 )
	    rem_wait_list( ch );
    }
    return;
}

void affect_update( )
{
    NODE_DATA *node_prev, *node, *node_next;
    AFFECT_DATA *paf, *paf_next;
    int count;
    static int affect_loss;
    bool loss = FALSE;

    if ( --affect_loss <= 0 )
    {
	loss = TRUE;
	affect_loss = 10;
    }

    for ( node = pc_list; node != NULL; node = node->next )
    {
	CHAR_DATA *person;
	if ( node->data_type != NODE_PC ) 
	    continue;

	person = (CHAR_DATA *) node->data;
	if ( IS_NPC(person) )
	    continue;

	person->pcdata->count = 0;
    }

    node_prev = NULL;
    for ( node = weave_list; node != NULL; node = node_next )
    {
	node_next = node->next;

	if ( node->data_type < NODE_WEAVE_CHAR
	||   node->data_type > NODE_WEAVE_CREATE )
	{
	    if ( node_prev == NULL )
	    {
		weave_list = node->next;
		free_node( node );
		continue;
	    }
	    else
	    {
		node_prev->next = node->next;
		free_node( node );
		continue;
	    }
	}

	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    CHAR_DATA *ch;
	    ch = (CHAR_DATA *) node->data;
	    count = 0;

	    for ( paf = ch->affected; paf != NULL; paf = paf_next )
	    {
		CHAR_DATA *owner;
		paf_next	= paf->next;
		owner		= paf->owner;
		count++;

		/* Special cases */
		if ( paf->type == gsn_capture )
		{
		    if ( IS_AWAKE(ch)
		    &&   check_stat(ch, STAT_STR, -10)
		    &&   loss )
		    {
			act( "$n struggle$% in $s bonds.", ch, NULL,
			    NULL, TO_ALL );
			paf->duration--;
			if ( paf->duration == 0 )
			{
			    if ( paf_next == NULL
			    ||   paf_next->type != paf->type
			    ||   paf_next->duration > 0 )
			    {
				if ( paf->type > 0 && skill_table[paf->type].msg_off &&
				    skill_table[paf->type].msg_off[0] != '!' )
				{
				    send_to_char( skill_table[paf->type].msg_off, ch );
				    send_to_char( "\n\r", ch );
				}
			    }
			    affect_remove( ch, paf );
			}
		    }
		}

		if ( paf->duration > 0 )
		{
		    if ( !IS_TIED(paf) )
		    {
			int stamina;
			int sn;
			sn = paf->type;
			stamina = stamina_cost( owner, sn );
			stamina = UMAX( 1, stamina / 6 );

			if ( owner->stamina < stamina )
			{
			    if ( paf_next == NULL
			    ||   paf->type != paf_next->type
			    ||   paf_next->duration > 0 )
			    {
				if ( sn > 0 && skill_table[sn].msg_off )
				{
				    send_to_char( skill_table[sn].msg_off, ch );
				    send_to_char( "\n\r", ch );
				}
			    }
			    act( "Maintaining the $t on $N proves too much, and you are forced to drop it.",
				owner, skill_table[sn].name, ch, TO_CHAR );
			    owner->stamina = UMIN( 0, owner->stamina );
			    affect_remove( ch, paf );  
			    continue;
			}
			lose_stamina( owner, stamina, FALSE, TRUE );
		    }
		    if ( loss && IS_TIED(paf) )
		    {
			paf->duration--;
			if (number_range(0,2) == 1 && paf->strength > 0)
			    paf->strength--; 
		    }
		}
		else if ( paf->duration < 0 )
		    ;
		else
		{
		    if ( paf_next == NULL
		    ||   paf_next->type != paf->type
		    ||   paf_next->duration > 0 )
		    {
			if ( paf->type > 0 && skill_table[paf->type].msg_off &&
			    skill_table[paf->type].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[paf->type].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
		    }
		    affect_remove( ch, paf );
		}
	    }
	    if ( count == 0 )
	    {
		if ( node_prev == NULL )
		{
		    weave_list = node->next;
		    free_node( node );
		}
		else
		{
		    node_prev->next = node->next;
		    free_node( node );
		}   
	    }
	    continue;
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    OBJ_DATA *obj;
	    obj = (OBJ_DATA *) node->data;
	    count = 0;

	    for ( paf = obj->affected; paf != NULL; paf = paf_next )
	    {
		CHAR_DATA *owner;
		paf_next	= paf->next;
		owner		= paf->owner;
		count++;

		if ( paf->duration > 0 )
		{
		    if ( !IS_TIED(paf) )
		    {
			int stamina;
			int sn;
			sn = paf->type;
			stamina = stamina_cost( owner, sn );
			stamina = UMAX( 1, stamina / 6 );

			if ( owner->stamina < stamina )
			{
			    act( "Maintaining the $t on $P proves too much, and you are forced to drop it.",
				owner, skill_table[sn].name, obj, TO_CHAR );
			    owner->stamina = UMIN( 0, owner->stamina );
			    affect_obj_remove( obj, paf );  
			    continue;
			}
			lose_stamina( owner, stamina, FALSE, TRUE );
		    }
		    if ( loss && IS_TIED(paf) )
		    {
			paf->duration--;
			if (number_range(0,2) == 1 && paf->strength > 0)
			    paf->strength--; 
		    }
		}
		else if ( paf->duration < 0 )
		    ;
		else
		    affect_obj_remove( obj, paf );
	    }

	    if ( count == 0 )
	    {
		if ( node_prev == NULL )
		{
		    weave_list = node->next;
		    free_node( node );
		}
		else
		{
		    node_prev->next = node->next;
		    free_node( node );
		}   
	    }
	    continue;
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    ROOM_INDEX_DATA *room;
	    room = (ROOM_INDEX_DATA *) node->data;
	    count = 0;

	    for ( paf = room->affected; paf != NULL; paf = paf_next )
	    {
		CHAR_DATA *owner;
		paf_next	= paf->next;
		owner		= paf->owner;
		count++;

		if ( paf->duration > 0 )
		{
		    if ( !IS_TIED(paf) )
		    {
			int stamina;
			int sn;
			sn = paf->type;
			stamina = stamina_cost( owner, sn );
			stamina = UMAX( 1, stamina / 6 );

			if ( owner->stamina < stamina )
			{
			    act( "Maintaining the $t in $T proves too much, and you are forced to drop it.",
				owner, skill_table[sn].name, room->name, TO_CHAR );
			    owner->stamina = UMIN( 0, owner->stamina );
			    affect_room_remove( room, paf );  
			    continue;
			}
			lose_stamina( owner, stamina, FALSE, TRUE );
		    }
		    if ( loss && IS_TIED(paf) )
		    {
			paf->duration--;
			if (number_range(0,2) == 1 && paf->strength > 0)
			    paf->strength--; 
		    }
		}
		else if ( paf->duration < 0 )
		    ;
		else
		    affect_room_remove( room, paf );
	    }
	    if ( count == 0 )
	    {
		if ( node_prev == NULL )
		{
		    weave_list = node->next;
		    free_node( node );
		}
		else
		{
		    node_prev->next = node->next;
		    free_node( node );
		}   
	    }
	    continue;
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    OBJ_DATA *obj;
	    CHAR_DATA *owner;

	    obj = (OBJ_DATA *) node->data;
	    owner = obj->owner;

	    if ( !IS_SET(obj->extra_flags, ITEM_CHANNELED) )
	    {
		void *vo;
		vo = (void *) obj;
		rem_weave_list( vo, NODE_WEAVE_CREATE );
		continue;
	    }

	    if ( obj->timer > 0 )
	    {
		if ( owner != NULL && !IS_SET(obj->extra_flags, ITEM_TIED) )
		{
		    int stamina;
		    int sn;
		    sn = affect_lookup( obj->pIndexData->vnum );
		    stamina = stamina_cost( owner, sn );
		    stamina = UMAX( 1, stamina / 6 );

		    if ( owner->stamina < stamina )
		    {
			act( "Maintaining the $t proves too much, and you are forced to drop it.",
			    owner, skill_table[sn].name, NULL, TO_CHAR );
			owner->stamina = UMIN( 0, owner->stamina );
			extract_obj( obj );
			continue;
		    }
		    lose_stamina( owner, stamina, FALSE, TRUE );
		}
		if ( loss && IS_SET(obj->extra_flags, ITEM_TIED) )
		    obj->timer--;
	    }
	    else if ( obj->timer < 0 )
		;
	    else
		extract_obj( obj );
	    continue;
	}
	node_prev = node;
    }
    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_hour;
    static  int	    pulse_min;
    static  int	    pulse_second;
    static  int	    pulse_obj;
    static  int	    pulse_weather;
    static  int	    pulse_timer;

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
    }

    if ( --pulse_hour  <= 0 )
    {
	pulse_hour	= PULSE_HOUR;
	hour_update	( );
    }

    if ( --pulse_timer    <= 0 )
    {
	pulse_timer     = PULSE_TIMER;
	timer_update	( );
    }

    if ( --pulse_weather <= 0 )
    {
	pulse_weather = PULSE_WEATHER;
	weather_update	( );
    }

    if (--pulse_obj	<= 0 )
    {
	pulse_obj	= PULSE_OBJ;
	obj_update	( );
    }

    if (--pulse_min	<= 0 )
    {
	pulse_min	= PULSE_MIN;
	min_update	( );
	affect_update	( );
    }

    if (--pulse_second	<= 0 )
    {
	pulse_second	= PULSE_SECOND;
	second_update		( );
	violence_update		( );
    }
    aggr_update		( );

    tail_chain( );
    return;
}
