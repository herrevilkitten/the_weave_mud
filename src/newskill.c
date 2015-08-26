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
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "mem.h"

/* command procedures needed */

/* other declarations */
DECLARE_SPELL_FUN(	spell_null	);

int	get_appraise		args( (OBJ_DATA *obj ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
bool    check_break     args( ( OBJ_DATA *obj, int damage ) );
bool	is_in_group	args( ( const char *name, const char *group ) );


void do_disguise( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    int chance;
    char buf[1000];
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );
    if ( (victim = get_char_room( ch, arg )) == NULL || arg[0] == '\0' )
    {
	send_to_char( "Who do you want to disguise yourself as?\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	remove_shape( ch );
        send_to_char( "Disguising yourself as yourself is quite useless.\n\r", ch );
        return;
    }

    if ( victim->race > race_lookup("human")
    ||   !IS_NPC(victim) )
    {
	send_to_char( "You really can't disguise yourself as that.\n\r", ch );
	return;
    }

    chance = 66;
    if ( TRUE_SEX(ch) != TRUE_SEX(victim) )
	chance /= 2;

    if ( !check_skill(ch, gsn_disguise, chance, TRUE)
    ||   IS_IMMORTAL(victim) )
    {
	send_to_char( "You just can't seem to make yourself look right.\n\r", ch);
	return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch == ch )
	    continue;

	if ( (dice(1, 15) + 9) < (number_fuzzy(get_curr_stat( ch, STAT_WIS )) +
	    number_fuzzy( get_curr_stat( ch, STAT_INT )) / 2) )
	{
	    if ( vch == victim )
		act( "You notice that $n is looking a lot like you.", ch, NULL, victim, TO_VICT );
	    else
	    {
		sprintf( buf, "You notice that $n is looking a lot like %s.",
		    PERS(victim, vch) );
		act( buf, ch, NULL, vch, TO_VICT );
	    }
	}
    }

    act( "You think you look just like $N.", ch, NULL, victim, TO_CHAR );

    remove_shape( ch );
    ch->pcdata->new_name = str_dup( IS_NPC(victim) ? victim->short_descr : victim->name );

    af.type		= gsn_disguise;
    af.strength		= ch->level;
    af.duration		= ch->level / 8 + 1;
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector	= AFF_SHAPE_CHANGE;
    af.bitvector_2	= 0;
    af.flags		= AFFECT_NOTCHANNEL;
    af.owner		= NULL;
    affect_to_char( ch, &af );
    af.type		= gsn_disguise;   
    af.strength		= ch->level;
    af.duration		= ch->level / 8 + 1;
    af.location		= APPLY_SEX;  
    af.modifier		= victim->sex;
    af.bitvector	= 0;  
    af.bitvector_2	= 0;
    af.flags		= AFFECT_NOTCHANNEL; 
    af.owner		= NULL;
    affect_to_char( ch, &af );

    return;
}



void do_lore( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Try to guess a little about what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) )
	return;


    if ( !check_skill(ch, gsn_lore, 50, TRUE) )
    {
	act( "$n stares at $p quizically.", ch, obj, NULL, TO_ROOM );
	act( "You stare at $p quizically, and then give up.", ch, obj,
	    NULL, TO_CHAR );
	check_improve( ch, gsn_lore, FALSE, 3 );
	return;
    }

    check_improve( ch, gsn_lore, TRUE, 2 );
    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->name,
	item_type_name( obj ),
	extra_bit_name( obj->extra_flags ),
	obj->weight,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("spear.\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("mace/club.\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("axe.\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("flail.\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);	break;
	    case(WEAPON_STAFF)  : send_to_char("staff.\n\r",ch);	break;
	    default		: send_to_char("unknown.\n\r",ch);	break;
 	}
	if (obj->pIndexData->new_format)
	    sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, 
	"Armor class is %d pierce, %d bash, %d slash, and %d vs. other.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
	break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_teach( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int chance, sn, gain, max_learn, gsn;

    if ( --ch->action_timer < 0 )
    {
	char *new_args;
	int time;

	if ( IS_NPC(ch) )
	{
	    free_action( ch );
	    return;
	}

	if ( IS_IMMORTAL(ch) )
	{
	    free_action( ch );
	    send_to_char( "Immortals may not use teach.\n\r", ch );
	    return;
	}

	new_args = one_argument( argument, arg1 );
	one_argument( new_args, arg2 );

	if ( arg1[0] == '\0' )
	{
	    free_action( ch );
            send_to_char( "Teach what to whom?\n\r", ch );
            return;
	}
    
	if ( (victim = get_char_room( ch, arg2)) == NULL )
	{
	    free_action( ch );
            send_to_char( "They aren't here.\n\r", ch );
            return;
	}

	if ( ch == victim )
	{
	    free_action( ch );
	    send_to_char( "You cannot teach yourself anything.\n\r", ch );
	    return;
	} 

	if ( IS_NPC(victim) )
	{
	    free_action( ch );
	    send_to_char( "You cannot teach non-players things.\n\r", ch );
	    return;
	}

	if ( victim->action_timer > 0 )
	{
	    free_action( ch );
	    act( "$N is busy doing something else.", ch, NULL, victim, TO_CHAR );
	    return;
	}
    
	if ( (sn = skill_lookup( arg1 )) < 0
	||   get_skill(ch, sn) < 1 )
	{
	    free_action( ch );
	    send_to_char( "You do not know of that skill.\n\r", ch );
	    return;
	}

	time = (skill_table[sn].rating[ch->class]
	     + skill_table[sn].rating[victim->class] * 2) * 25 / 3;
	new_action( ch, do_teach, time, argument, victim );
	return;
    }
    else if ( ch->action_timer > 0 )
	return;

    victim   = ch->action_target;
    argument = ch->action_args;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    free_action( ch );

    sn = skill_lookup( arg1 );
    gsn = gsn_teaching;

    chance = 50;

    if ( victim->in_room != ch->in_room )
    {
	send_to_char( "Your pupil has left the area.\r\n", ch );
	return;
    }

    if ( is_channel_skill(sn) )
    {
	if ( !can_channel(ch, 1) )
	{
	    send_to_char( "You cannot channel.\n\r", ch );
	    return;
	}

	if ( TRUE_SEX(ch) == TRUE_SEX(victim) )
	{
	    send_to_char( "You can only teach channeling to those of the same sex.\r\n", ch );
	    return;
	}
	gsn = gsn_teach_channeling;
	if ( ch->pcdata->learned[gsn] < 0 )
	{
	    send_to_char( "You may not teach channeling.\n\r", ch );
	    return;
	}
	if ( !can_channel(victim,0) )
	{
	    act( "$N cannot channel.", ch, NULL, victim, TO_CHAR );
	    return;
	}
    }

    if ( is_in_group(arg2, "unteachable") )
    {
	send_to_char( "That skill cannot be taught.\n\r", ch );
	return;
    }

    if ( check_skill(ch, gsn, 75, TRUE) )
	chance += get_skill(ch, gsn) / 5;

    if ( ch->pcdata->talent[tn_teaching] )
	chance = chance * 125 / 100;

    if ( victim->pcdata->learned[sn] < 0 )
	chance = chance / 6;
    else if ( victim->pcdata->learned[sn] == 0 )
	chance = chance / 3;
    else
	chance = chance * 3 / 5;

    if ( skill_table[sn].rating[victim->class] < 0 )
	if ( skill_table[sn].rating[victim->class] == -1 )
	    chance = chance - 40;
	else
	    chance = chance - 10 * (skill_table[sn].rating[victim->class] * -1);
    else
	chance = chance - 7 * skill_table[sn].rating[victim->class];

    if ( victim->level <= skill_table[sn].skill_level[victim->class] )
    {
	act( "$N cannot seem to understand $t.", ch, skill_table[sn].name,
	    victim, TO_CHARVICT );
	return;
    }

    WAIT_STATE(ch, 6);
    WAIT_STATE(victim, 6);
    act( "$n begin$% to teach $N the basics of $t.", ch,
	skill_table[sn].name, victim, TO_ALL );

    if ( !check_skill(ch, sn, chance, TRUE) )
    {
	act( "$N cannot seem to understand $t.", ch, skill_table[sn].name,
	    victim, TO_CHARVICT );
	return;
    }

    max_learn = UMIN( 10, get_skill(ch, sn) / 10 + ch->pcdata->teach_skill
	      + victim->pcdata->learn_skill + chance / 10 );
    gain = number_range(1, ch->pcdata->teach_skill + victim->pcdata->learn_skill);
    if ( victim->pcdata->learned[sn] >= max_learn )
    {
	act( "$N can learn no more about $t from $n.", ch, skill_table[sn].name,
	    victim, TO_CHARVICT );
	return;
    }
    if ( victim->pcdata->learned[sn] < 0 )
    {
	victim->pcdata->learned[sn] = 0;
	act( "$n teach$& $N the basics of $t.", ch, skill_table[sn].name,
	    victim, TO_ALL );
    }
    else if ( victim->pcdata->learned[sn] == 0 )
    {
	victim->pcdata->learned[sn] += UMAX(1, gain / 2);
	act( "$n teach$& $N more of the art of $t.", ch, skill_table[sn].name,
	    victim, TO_ALL );
    }
    else
    {
	victim->pcdata->learned[sn] = UMIN( max_learn,
	   victim->pcdata->learned[sn] += gain );
	act( "$N make$^ great gains in learning $t from $n.", ch,
	    skill_table[sn].name, victim, TO_ALL );
    }
    return;
}



void do_appraise( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;

    sn = skill_lookup( "appraising" );
    one_argument( argument, arg );
    if ( (obj = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    act( "$n carefully look$% over $p.", ch, obj, NULL, TO_ALL );
    if ( !check_skill(ch, sn, 100, TRUE) )
    {
	send_to_char( "But you don't find out anything.\n\r", ch );
	check_improve( ch, sn, FALSE, 3 );
	return;
    }

    if ( obj->material == MAT_UNKNOWN )
	send_to_char( "It seems to be made of an unknown material.\n\r", ch );
    else
	send_to_char_new( ch, "You would guess the item to be made of %s.\n\r",
		      material_name(obj->material) );
    if ( get_appraise(obj) > 0 )
	send_to_char_new( ch, "It is definitely worth at least %d coins.\n\r",
			  get_appraise(obj) );
    else
	send_to_char( "It's not worth anything.\n\r", ch );
    if ( obj->condition > 0 )
	send_to_char_new( ch, "It's about at %d%% of it's normal condition.\n\r",
			  obj->condition );
    else
	send_to_char( "It's broken badly.\n\r", ch );

    check_improve( ch, sn, TRUE, 2 );
    return;
}

int get_appraise( OBJ_DATA *obj )
{
    int cost;

    if ( IS_SET(obj->extra_flags, ITEM_QUEST)
    ||   is_guild_eq(obj->pIndexData->vnum) )
        return 0;

    cost = obj->cost;
                
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



void do_stalk( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int stalk = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stalk whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( CHARM_SET(ch) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_stalk(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_HERO(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    stalk = TRUE;

    if ( !check_skill(ch, gsn_stalk, 50, TRUE) )
    {
	act( "You aren't able to shadow $N.", ch, NULL, victim, TO_CHAR );
	check_improve( ch, gsn_stalk, FALSE, 4 );
	stalk = FALSE;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);

    if ( ch->master != NULL )
	stop_stalk( ch );

    if ( stalk == TRUE )
    {
	check_improve( ch, gsn_stalk, TRUE, 2 );
	add_stalk( ch, victim );
    }
    else
    {
	if ( dice(1,15) + 9 < ((get_curr_stat(victim, STAT_WIS) +
	     get_curr_stat(victim, STAT_INT)) / 2) )
	{
	    act( "You notice $n trying to follow you around.", ch, NULL,
		victim, TO_VICT );
	}

        add_follower(ch, victim );
    }
    return;
}

void add_stalk( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_stalk: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    act( "You now stalk $N.",  ch, NULL, master, TO_CHAR );
    SET_BIT( ch->affected_by, AFF_STALK );  

    return;
}


void stop_stalk( CHAR_DATA *ch )
{
    REMOVE_BIT( ch->affected_by, AFF_STALK );  

    if ( ch->master == NULL )
    {
	act( "You stop stalking your target.", ch, NULL, NULL, TO_CHAR );
	bug( "Stop_stalk: null master.", 0 );
	return;
    }
    act( "You stop stalking $N.",      ch, NULL, ch->master, TO_CHAR    );
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void do_second (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int sn, skill;

    if (argument[0] == '\0') /* empty */
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument);
    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }


    /* check if the char is using a shield or a held weapon */

    if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
         (get_eq_char (ch,WEAR_HOLD)   != NULL) )
    {
        send_to_char ("You cannot use a secondary weapon while using a shield or holding an item.\n\r", ch );
        return;
    }

/* check that the character is using a first weapon at all */
    if (get_eq_char (ch, WEAR_WIELD) == NULL) /* oops - != here was a bit wrong */
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one.\n\r", ch );
        return;
    }

    if ( get_eq_char(ch, WEAR_WIELD) != NULL )
    {
	OBJ_DATA *wield;
	wield = get_eq_char( ch, WEAR_WIELD );
	if ( IS_SET(wield->value[4], WEAPON_TWO_HANDS) )
	{
	    send_to_char( "You cannot wield a secondary weapon if your primary weapon is two-handed.\n\r", ch );
	    return;
	}
    }

    if ( IS_SET(obj->value[4], WEAPON_TWO_HANDS) )
    {
	send_to_char( "You cannot wield a two-handed weapon in your off hand.\n\r", ch );
	return;
    }

    if ( !check_skill(ch, gsn_dual_wield, 50, TRUE) )
    {
	act( "You just can't seem to arrange the $P right.", ch, NULL,
	    obj, TO_CHAR );
	return;
    } 

/* check for str - secondary weapons have to be lighter */
    if ( get_obj_weight( obj ) > ( str_app[get_curr_stat(ch, STAT_STR)].wield / 2) )
    {
        send_to_char( "This weapon is too heavy to be used as a secondary one.\n\r", ch );
        return;
    }

/* check if the secondary weapon is at least half as light as the primary weapon */
    if ( (get_obj_weight (obj)*2) > get_obj_weight(get_eq_char(ch,WEAR_WIELD)) )
    {
        send_to_char("Your secondary weapon has to be considerably lighter than your primary one.\n\r", ch );
        return;
    }

/* at last - the char uses the weapon */

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */

/* char CAN use the item! that didn't take long at aaall */

    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);

    sn = get_secondary_sn(ch);

    if (sn == gsn_hand_to_hand)
        return;

    skill = get_secondary_skill(ch,sn);

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



void do_treat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    int skill;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Treat whom for what?\n\r", ch );
	return;
    }

    if ( (vch = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    skill = get_skill( ch, gsn_medicine );

    if ( skill < 1 )
    {
	send_to_char( "You don't know how to treat anything.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Treat what?\n\r", ch );
	return;
    }

    if ( !str_prefix(arg2, "health") )
    {
	obj = get_obj_carry( ch, "ointment" );
	if ( obj == NULL || obj->pIndexData->vnum != 4226 )
	{
	    send_to_char( "You do not have any ointment.\n\r", ch );
	    return;
	}

	extract_obj( obj );
	gain_health( vch, dice(10, 10) * skill / 50, FALSE );
	act( "$n rub$% some ointment on $O wounds.", ch, NULL, vch,
	    TO_ALL );
	return;
    }

    if ( !str_prefix(arg2, "bleeding") )
    {
	obj = get_obj_carry( ch, "bandages" );
	if ( obj == NULL || obj->pIndexData->vnum != 4225 )
	{
	    send_to_char( "You do not have any bandages.\n\r", ch );
	    return;
	}

	if ( !IS_SET(vch->body, BODY_BLEEDING) )
	{
	    act( "$N is not bleeding.", ch, NULL, vch, TO_CHAR );
	    return;
	}

	extract_obj( obj );
	gain_health( vch, dice(1, 5) * skill / 50, FALSE );
	act( "$n bandage$% $O wounds.", ch, NULL, vch,
	    TO_ALL );
	REMOVE_BIT(vch->body, BODY_BLEEDING);
	return;
    }

    if ( !str_prefix(arg2, "poison") )
    {
	obj = get_obj_carry( ch, "antivenom" );
	if ( obj == NULL || obj->pIndexData->vnum != 4229 )
	{
	    send_to_char( "You do not have any antivenom.\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(vch, AFF_POISON) )
	{
	    act( "$N is not poisoned.", ch, NULL, vch, TO_CHAR );
	    return;
	}

	gain_health( vch, dice(1, 5) * skill / 50, FALSE );
	act( "$n open$% $p and pour$% it down $O throat.", ch, obj, vch,
	    TO_ALL );
	extract_obj( obj );
	affect_strip( vch, gsn_poison );
	REMOVE_BIT(vch->affected_by, AFF_POISON);
	return;
    }

    if ( !str_prefix(arg2, "bones") )
    {
	AFFECT_DATA af;

	if ( !IS_SET(vch->body, BODY_RIGHT_LEG)
	&&   !IS_SET(vch->body, BODY_LEFT_LEG)
	&&   !IS_SET(vch->body, BODY_RIGHT_ARM)
	&&   !IS_SET(vch->body, BODY_LEFT_ARM) )
	{
	    act( "$N has no broken limbs.", ch, NULL, vch, TO_CHAR );
	    return;
	}

	af.type			= skill_lookup( "set limb" );
	af.strength		= skill * 5 / 2;
	af.duration		= 5 - number_range(1, skill/23);
	af.modifier		= 0;
	af.location		= 0;
	af.bitvector		= 0;
	af.bitvector_2		= 0;
	af.owner		= NULL;
	af.flags		= 0;
	affect_to_char( vch, &af );

	gain_health( vch, dice(1, 2) * skill / 50, FALSE );
	act( "$n take$% $O broken limb and set$% it.", ch, NULL, vch,
	    TO_ALL );
	WAIT_STATE(ch, 4);
	return;
    }

    send_to_char( "Treat what?\n\r", ch );
    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *vch;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you want to hide behind?\n\r", ch );
	return;
    }

    if ( (obj = get_obj_here( ch, arg )) )
    {
	switch( obj->item_type )
	{
	    default:
		send_to_char( "You cannot hide behind that.\n\r", ch );
		return;
	    case ITEM_FOUNTAIN:
	    case ITEM_FURNITURE:
		break;
	}

	send_to_char_new( ch, "You attempt to hide behind %s.",
	    obj->short_descr );

	if ( IS_AFFECTED(ch, AFF_HIDE) )
	{
	    REMOVE_BIT(ch->affected_by, AFF_HIDE);
	    ch->hide = NULL;
	    ch->hide_type = HIDE_NONE;
	}

	if ( check_skill(ch, gsn_hide, 100, TRUE) )
	{
	    SET_BIT(ch->affected_by, AFF_HIDE);
	    check_improve(ch,gsn_hide,TRUE,3);
	    ch->hide_type	= HIDE_OBJECT;
	    ch->hide		= (void *) obj;
	}
	else
	    check_improve(ch,gsn_hide,FALSE,3);
	return;
    }

    if ( (vch = get_char_room( ch, arg )) )
    {
	if ( vch == ch )
	{
	    REMOVE_BIT(ch->affected_by, AFF_HIDE);
	    ch->hide = NULL;
	    ch->hide_type = HIDE_NONE;
	    send_to_char( "Try someone else.\n\r", ch );
	    return;
	}

	act( "You attempt to hide behind $N.", ch, NULL, vch, TO_CHAR );
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	{
	    REMOVE_BIT(ch->affected_by, AFF_HIDE);
	    ch->hide = NULL;
	    ch->hide_type = HIDE_NONE;
	}

	if ( ch->size > vch->size )
	{
	    act( "$N is kinda small for you.", ch, NULL, vch, TO_CHAR );
	    return;
	}

	if ( check_skill(ch, gsn_hide, 100, TRUE) )
	{
	    SET_BIT( ch->affected_by, AFF_HIDE );
	    check_improve( ch, gsn_hide, TRUE, 3 );
	    ch->hide_type	= HIDE_CHAR;
	    ch->hide		= (void *) vch;
	}
	else
	    check_improve( ch, gsn_hide, FALSE, 4 );

	return;
    }

    send_to_char( "Hide behind what?\n\r", ch );
    return;
}



void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk) )
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling too mellow to berserk.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance + luck)
    {
	AFFECT_DATA af;

	WAIT_STATE( ch,  3 );
	ch->stamina = ch->stamina * 3 / 4;

	/* heal a little damage */
	gain_health( ch, chance * 2 / 3, FALSE );

	send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve( ch, gsn_berserk, TRUE, 2 );

	af.type		= gsn_berserk;
	af.strength	= ch->level;
	af.duration	= chance / 5;
	af.modifier	= chance / 8;
	af.bitvector 	= AFF_BERSERK;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE( ch, 6 );
	ch->stamina = ch->stamina * 5 / 6;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}



void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( CHARM_SET(ch) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( ch->stamina < 1 )
    {
        send_to_char( "You are too tired to bash someone.\n\r", ch );
        return;
    }   
    lose_stamina( ch, 1, FALSE, TRUE );

    /* modifiers */
    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 5; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR) * 4/3;
    chance += get_curr_stat(ch, STAT_AGI) * 2/3;
    chance -= get_curr_stat(victim,STAT_DEX) * 3/4;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

    /* now the attack */
    if (number_percent() < chance + luck)
    {
	act("$n sends you sprawling with a powerful bash!",
		ch,NULL,victim,TO_VICT);
	act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
	act("$n sends $N sprawling with a powerful bash.",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(victim, chance/30 );
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_SITTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH, FALSE);	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH, FALSE);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}



void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    one_argument(argument,arg);

    if ( ch->position == POS_MOUNTED || ch->mount != NULL )
    {
	send_to_char( "You can't kick dirt from the back of a mount.\n\r", ch );
	return;
    }

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( CHARM_SET(ch) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( ch->stamina < 1 )
    {
        send_to_char( "You are too tired to kick dirt.\n\r", ch );
        return;
    }
     
    lose_stamina( ch, 1, FALSE, TRUE );


    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,OFF_FAST))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance + luck)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE, FALSE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.type 	= gsn_dirt;
	af.strength 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;
	af.bitvector_2	= 0;
	af.owner	= NULL;
	af.flags	= AFFECT_NOTCHANNEL;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE, FALSE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}



void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if ( CHARM_SET(ch) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( ch->stamina < 2 )
    {
        send_to_char( "You are too tired to trip someone.\n\r", ch );
        return;
    }
     
    lose_stamina( ch, 2, FALSE, TRUE );


    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance + luck)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	WAIT_STATE(victim, 4 );
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH, FALSE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH, FALSE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( --ch->action_timer < 0 )
    {
	CHAR_DATA *vch;

	if ( !ch->in_room )
	{
	    free_action( ch );
	    ch->action_timer = 0;
	    return;
	}

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Backstab whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "How can you sneak up on yourself?\n\r", ch );
	    return;
	}

	if ( is_safe( ch, victim ) )
	    return;

	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	{
            send_to_char("Kill stealing is not permitted.\n\r",ch);
            return;
	}

	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
	{
	    send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	    return;
	}

	if ( victim->fighting != NULL )
	{
	    send_to_char( "You can't backstab a fighting person.\n\r", ch);
	    return;
	}

	if ( victim->hit < victim->max_hit )
	{
	    act( "$N is hurt and suspicious ... you can't sneak up.",
		ch, NULL, victim, TO_CHAR );
	    return;
	}

	act( "You begin to sneak towards $N", ch, NULL, victim, TO_CHAR );
	for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
	{
	    char buf[MAX_STRING_LENGTH];
	    if ( ch == vch )
		continue;

	    sprintf( buf, "You notice %s is sneaking toward $Y.",
		PERS(ch, vch) );
	    if ( check_stat(vch, STAT_WIS, -10) )
		act( buf, vch, NULL, victim, TO_CHAR );
	}
	new_action( ch, do_backstab, 2, argument, victim );
    }
    else if ( ch->action_timer > 0 )
	return;

    victim = ch->action_target;
    free_action( ch );

    if ( victim->in_room != ch->in_room )
    {
	send_to_char( "Your victim has left the area.\r\n", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "You can't backstab a fighting person.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   check_skill(ch, gsn_backstab, 100, TRUE) )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE, FALSE );
    }

    return;
}

void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *boots;
    int dam;   
    
    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
        send_to_char(
            "You better leave the martial arts to fighters.\n\r", ch );
        return;
    }   
    
    if ( IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK) )
        return;
    
    if ( (victim = ch->fighting) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
        
    if ( ch->stamina < 3 )
    {
	send_to_char( "You are too tired to kick people around.\n\r", ch );
	return;
    }
    
    lose_stamina( ch, 3, FALSE, TRUE );
    dam = number_range( 1, get_skill(ch, gsn_kick) / 7 ) * 2;
        
    boots = get_eq_char( ch, WEAR_FEET );
    if ( boots != NULL )
    {
	if ( is_metal(boots) )
	    dam = dam * 150 / 100;
    }
    else
	dam = dam * 75 / 100;

    if ( check_skill(ch, gsn_martial_arts, 80, TRUE) )
    {
	dam = dam * 125 / 100;
	check_improve(ch, gsn_martial_arts, TRUE, 3);
    }
     
    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || check_skill(ch,gsn_kick,100, TRUE) )
    {
	damage( ch, victim, dam, gsn_kick,DAM_BASH, FALSE );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {   
	damage( ch, victim, 0, gsn_kick,DAM_BASH, FALSE );
	check_improve(ch,gsn_kick,FALSE,1);
    }
        
    return;
}



void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
    int luck;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( (get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM)))) )
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance + luck )
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    return;
}


void do_wolfsummon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA		*wolf;
    ROOM_INDEX_DATA	*room;
    int			count = 0;

    if ( IS_NPC(ch) )
	return;

    if ( ch->in_room == NULL )
	return;

    if ( !IS_SET(ch->act, PLR_WOLFKIN) )
    {
	send_to_char( "Only the Wolfkin may summon wolves to their aid.\n\r", ch );
	return;
    }

    for ( wolf = char_list; wolf; wolf = wolf->next )
    {
	if ( !IS_NPC(wolf) )
	    continue;

	if ( (wolf->pIndexData->vnum == 4200
	||    wolf->pIndexData->vnum == 4201)
	&&   (wolf->leader == ch
	||    wolf->master == ch) )
	    count++;
    }


    room = ch->in_room;
    switch( room->sector_type )
    {
	default:
	    break;
	case SECT_CITY:
	case SECT_INSIDE:
	case SECT_UNDERGROUND:
	case SECT_WATER_NOSWIM:
	case SECT_UNUSED:
	case SECT_AIR:
	    send_to_char( "Wolves may not be summoned here.\n\r", ch );
	    return;
    }

    WAIT_STATE(ch, 5);

    if ( count >= get_skill(ch, gsn_summon_wolf)/15 )
    {
	send_to_char( "You have as much protection as you need.\n\r", ch );
	return;
    }

    if ( check_skill(ch, gsn_summon_wolf, 75, TRUE) )
    {
	wolf = create_mobile( get_mob_index(4201) );
	char_to_room( wolf, room );
	SET_BIT(wolf->affected_by, AFF_CHARM);
	SET_BIT(wolf->act, PLR_AUTOASSIST);

	add_follower( wolf, ch );
	wolf->leader = ch;

	act( "$N walk$^ up, and lick$^ $o hand.", ch, NULL, wolf, TO_ALL );
	return;
    }
    if ( check_skill(ch, gsn_summon_wolf, 17, TRUE) )
    {
	wolf = create_mobile( get_mob_index(4200) );
	char_to_room( wolf, room );
	SET_BIT(wolf->affected_by, AFF_CHARM);
	SET_BIT(wolf->act, PLR_AUTOASSIST);

	add_follower( wolf, ch );
	wolf->leader = ch;

	act( "$N walk$^ up, and lick$^ $o hand.", ch, NULL, wolf, TO_ALL );
	return;
    }
    send_to_char( "No wolves answer your call.\n\r", ch );
    return;
}
