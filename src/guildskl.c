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
#include "olc.h"

DECLARE_DO_FUN(	do_wake		);
bool is_collared( CHAR_DATA *ch );

/* Guild skills

    Ajah

Blue = locate area... 1 tick/level duration - 100 stamina cost Reveals the
path to an area of the MUD... 'whereis Caemlyn' would return 'to the
north'
 
Brown = identify creature... instantaneous duration - 75 stamina Would
reveal the statistics (weaknesses, hp, stamina, etc) of one creature the
Sedai is in the room with. 
 
Green = Show of Force... instantaneous duration - 100 stamina reduces the
target's hp by 1/4 of its current total 1000 current hp would lose 250 hp
 
Grey = complaisance... 1 tick/level duration - 100 stamina Sets a NO_AGGRO
flag on the Sedai and her group IF she is the leader.  No Aggro mobs can
attack them.
 
Red = gentle (of course)... instantaneous duration - 100 stamina Must be
cast by two Red Sisters on a male channeler. Removes his ability to
channel. This may need more limitations placed on it. Let me know. 
 
White = logical thinking... 4 ticks (???) - 125 stamina Gives the White
Sister an increased chance of success on all of her rolls. I was thinking
an all around 20% bonus. If this seems unreasonable, let me know. 
 
Yellow = restoration... instantaneous duration - 100 stamina from caster
AND from the recipient Heals 1/2 of the current damage the target has
taken. For example, if the target has taken a TOTAL of 800 points of
damage, restoration will heal 400 hp, leaving him with 400 hp in damage.
The next casting on this same person will only heal 200 pts, etc.
*/

void do_complaisance( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int luck;

    if ( IS_NPC(ch) )
	return;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod / 2;

    if ( ch->stamina < 100 )
    {
	send_to_char( "You just don't have the energy to make the world more peaceful.\n\r", ch );
	return;
    }

    if ( !check_skill(ch, gsn_complaisance, TRUE, 75) )
    {
	send_to_char( "You can't seem to calm down enough.\n\r", ch );
	ch->stamina -= 50;
	return;
    }

    ch->stamina -= 100;

    af.type		= skill_lookup( "complaisance" );
    af.strength		= ch->level;
    af.duration		= UMAX( ch->level / 2, 1 );
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_NOAGGRO;
    af.owner		= NULL;
    af.flags		= 0;
    affect_join( ch, &af );

    act( "You smile, and the world seems more peaceful.", ch, NULL, NULL,
	TO_CHAR );
    return;
}

void do_identify( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int luck, percent;

    if ( IS_NPC(ch) )
	return;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod / 2;

    if ( ch->stamina < 75 )
    {
	send_to_char( "You don't have the energy for such an examination.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "What would you like to identify?\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    if ( !check_skill(ch, gsn_identify_fauna, 75, TRUE) )
    {
	send_to_char( "You don't learn anything about it.\n\r", ch );
	ch->stamina -= 37;
	return;
    }
    ch->stamina -= 75;
    percent = 100;

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

    if (victim->imm_flags)
    {
        sprintf(buf, "It seems %s is immune to `4%s`n\n\r",
            victim->sex == 0 ? "it" : victim->sex == 1 ? "he" : "she",
            imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
        
    if (victim->res_flags)
    {
        sprintf(buf, "You would guess %s is resistant to `4%s`n\n\r",
            victim->sex == 0 ? "it" : victim->sex == 1 ? "he" : "she",
	    imm_bit_name(victim->res_flags));
        send_to_char(buf,ch);
    }
     
    if (victim->vuln_flags)
    {
        sprintf(buf, "%s appears vulnerable to `4%s`n\n\r",
            victim->sex == 0 ? "It" : victim->sex == 1 ? "He" : "She",
	    imm_bit_name(victim->vuln_flags));
        send_to_char(buf,ch);
    }
    return;
}

void do_sweep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    int luck;

    if ( IS_NPC(ch) )
	return;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod / 3;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( !check_skill(ch, gsn_sweep, 100, TRUE) )
	    continue;

	if ( is_safe(ch, vch) || is_same_group(ch, vch) )
	    continue;

	if ( ch == vch )
	    continue;

	WAIT_STATE( ch, 1 );
	act( "$n suddenly attack$% $N!", ch, NULL, vch, TO_ALL );
	multi_hit( ch, vch, TYPE_UNDEFINED );
    }
    return;
}

void do_glist( CHAR_DATA *ch, char *argument )
{
    GUILD_DATA *guild;
    char buf[MAX_STRING_LENGTH];

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	sprintf( buf, "Guild: %s\n\r[%s]\n\r", guild->name,
	    guild->members );
	send_to_char( buf, ch );
    }
    return;
}


void do_bond( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int count = 0;
    char *temp;

    if ( !IS_IN_GUILD(ch, guild_lookup("aes sedai")) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Bond who?\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
	send_to_char( "You may not bond yourself.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "You may not bond NPCs.\n\r", ch );
        return;
    }

    /* Found 'em - bond 'em */

    temp = one_argument(ch->pcdata->guild->warder, name);
    while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) )
    {
	if ( !str_cmp(name, victim->name) )
	{
	    act( "$N is already one of your warders.", ch, NULL, victim,
		TO_CHAR );
	    return;
	}
	count++;
	temp = one_argument(temp, name);
    }

    if ( !IS_NULLSTR(victim->pcdata->sedai)
    &&   !str_cmp(ch->name, victim->pcdata->sedai) )
    {
	act( "$N is already one of your warders.", ch, NULL, victim,
	    TO_CHAR );
	return;
    }

    if ( !IS_NULLSTR(victim->pcdata->sedai) )
    {
        act( "$N is already bonded to someone.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( count >= 4 )
    {
        send_to_char( "You cannot bond anymore warders.\n\r", ch );
        return;
    }

    /* Okay, got room, got a warder, let's make 'em cool */
    free_string( victim->pcdata->sedai );
    victim->pcdata->sedai = str_dup( ch->name );
    
    strcpy( name, ch->pcdata->guild->warder );
    strcat( name, " " );
    strcat( name, victim->name );
    free_string( ch->pcdata->guild->warder );
    ch->pcdata->guild->warder = str_dup( name );

    act( "$n steps up to $N and places $s hands on $M for a second.",
	ch, NULL, victim, TO_NOTVICT );
    act( "$N shivers for a second, and $n releases $s hands and backs up.",
	ch, NULL, victim, TO_NOTVICT );
    act( "$n has bonded you as $s warder.", ch, NULL, victim, TO_VICT );
    act( "You bond $N as your warder.", ch, NULL, victim, TO_CHAR );
    return;
}

void do_unbond( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( !IS_IN_GUILD(ch, guild_lookup("aes sedai")) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unbond who?\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
        send_to_char( "They are not here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
	send_to_char( "You may not unbond yourself.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "You may not unbond NPCs.\n\r", ch );
        return;
    }

    if ( !is_full_name(victim->name, ch->pcdata->guild->warder) )
    {
        act( "$N is not one of your warders.", ch, NULL, victim, TO_CHAR );
        return;
    }

    buf[0] = '\0';
    ch->pcdata->guild->warder = string_replace(
	ch->pcdata->guild->warder, victim->name, "\0" );
    ch->pcdata->guild->warder = string_unpad(
	ch->pcdata->guild->warder );

    free_string( victim->pcdata->sedai );
    victim->pcdata->sedai = NULL;

    act( "$n steps up to $N and places $s hands on $M for a second.",
	ch, NULL, victim, TO_NOTVICT );
    act( "$N shivers for a second, and $n releases $s hands and backs up.",
	ch, NULL, victim, TO_NOTVICT );
    act( "You are no longer a warder to $n.", ch, NULL, victim, TO_VICT );
    act( "$N is no longer your warder.", ch, NULL, victim, TO_CHAR );
    return;
}

void do_emotion( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; i < strlen(argument); i++ )
    {
	if ( !isalpha(argument[i]) )
	{
	    send_to_char( "Your feelings may only contain letters.\n\r", ch );
	    return;
	}
    }

    one_argument( argument, arg );

    if ( ch->guild == guild_lookup("aes sedai") )
    {
	char *temp;
	char name[MAX_INPUT_LENGTH];

	if ( IS_NULLSTR(ch->pcdata->guild->warder) )
	{
	    send_to_char( "You don't have a warder.\n\r", ch );
	    return;
	}

	temp = one_argument(ch->pcdata->guild->warder, name);
/*	while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) ) 
	{ */
	    if ( (vch = get_char_sedai(ch, name)) )
		send_to_char_new( vch, "%s feels %s.\n\r", PERS(ch, vch), arg );
	    temp = one_argument(temp, name);
/*	} */
	send_to_char_new( ch, "You feel %s.\n\r", arg );
	return;
    }
    else if ( ch->guild == guild_lookup("warder") )
    {
	if ( !ch->pcdata->sedai )
	{
	    send_to_char( "You are not bonded to a Sedai.\n\r", ch );
	    return;
	}
	if ( (vch = get_char_sedai( ch, ch->pcdata->sedai )) )
	    send_to_char_new( vch, "%s feels %s.\n\r", PERS(ch, vch), arg );
	send_to_char_new( ch, "You feel %s.\n\r", arg );
    }
    return;
}



void do_leash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    int chance;

    if ( !IS_IN_GUILD(ch, guild_lookup("seanchan")) )
	return;

    one_argument( argument, arg );

    if ( (vch = get_char_room(ch, arg)) == NULL )
    {
	send_to_char( "You do not see them here.\n\r", ch );
	return;
    }

    if ( vch == ch )
    {
	send_to_char( "You may not leash yourself.\n\r", ch );
	return;
    }

    if ( IS_IMMORTAL(vch) )
    {
	send_to_char( "Not likely.\n\r", ch );
	return;
    }

    if ( ch->pcdata->guild->damane != NULL
    &&   vch != ch->pcdata->guild->damane )
    {
	send_to_char( "You are already linked to a damane.\n\r", ch );
	return;
    }
    else if ( ch->pcdata->guild->damane != NULL
    &&	      vch == ch->pcdata->guild->damane )
    {
	ch->pcdata->guild->damane = NULL;
	REMOVE_BIT(vch->affected_by, AFF_CHARM);
	REMOVE_BIT(vch->affected_by_2, AFF_LEASHED);
	affect_strip(vch, gsn_charm_person);
	affect_strip(vch, gsn_leashing);
	stop_follower( vch );

	if ( (obj = get_bracelet(ch)) )
	{
	    act( "$n remove$% $p, making sure that $O collar is secure.",
		ch, obj, vch, TO_ALL );
	    extract_obj( obj );
	}
	else
	    act( "$n remove$% $s bracelet, making sure that $O collar is secure.",
		ch, NULL, vch, TO_ALL );	
	return;
    }

    if ( IS_NPC(vch) )
    {
	send_to_char( "You may not leash NPCs.\n\r", ch );
	return;
    }

    if ( get_eq_char(ch, WEAR_WRIST_L)
    &&   get_eq_char(ch, WEAR_WRIST_R) )
    {
	act( "You must have your wrists free before you can wear the a'dam bracelet.",
	    ch, NULL, NULL, TO_CHAR );
	return;
    }

    if ( is_collared(vch) )
    {
	AFFECT_DATA af;

	obj = create_object( get_obj_index(OBJ_VNUM_ADAM_BRACELET), 1 );
	obj_to_char( obj, ch );

	if ( get_eq_char(ch, WEAR_WRIST_L) )
	    equip_char( ch, obj, WEAR_WRIST_R );
	else
	    equip_char( ch, obj, WEAR_WRIST_L );

	act( "$n take$% $O leash and wear$% the bracelet.", ch, NULL,
	    vch, TO_ALL );

	REMOVE_BIT(vch->affected_by, AFF_CHARM);
	affect_strip( ch, gsn_charm_person );

	if (ch->leader != NULL)
	{
	    CHAR_DATA *ldr = ch->leader;
	    while (ldr != NULL)
	    {
		if (ldr == vch)
		{
		    stop_follower(ch);
		    break;
		}
		ldr = ldr->leader;
	    }
	}
	if ( vch->master )
	    stop_follower( vch );
	add_follower( vch, ch );

	af.type			= gsn_leashing;
	af.strength		= -1;
	af.duration		= -1;
	af.location		= 0;
	af.modifier		= 0;
	af.bitvector		= 0;
	af.bitvector_2		= AFF_LEASHED;
	af.owner		= NULL;
	af.flags		= AFFECT_NOTCHANNEL;
	affect_to_char( vch, &af );

	ch->pcdata->guild->damane	= vch;
	return;
    }
 
    if ( (obj = get_eq_char(vch, WEAR_NECK_1))
    &&   obj->pIndexData->vnum == OBJ_VNUM_ADAM_COLLAR )
    {
	act( "$N is already wearing $p.", ch, obj, vch, TO_CHAR );
	return;
    }

    if ( (obj = get_eq_char(vch, WEAR_NECK_2))
    &&   obj->pIndexData->vnum == OBJ_VNUM_ADAM_COLLAR )
    {
	act( "$N is already wearing $p.", ch, obj, vch, TO_CHAR );
	return;
    }

    obj = get_collar(ch);
    if ( obj == NULL
    ||   obj->wear_loc == WEAR_NECK_1
    ||   obj->wear_loc == WEAR_NECK_2 )
    {
	send_to_char( "You must have an empty a'dam to leash marath'damane.\n\r", ch );
	return;
    }

    if ( get_eq_char(vch, WEAR_NECK_1) 
    &&   get_eq_char(vch, WEAR_NECK_2) )
    {
	act( "$N is wearing many things on $S neck.  This will be harder.",
	    ch, NULL, vch, TO_CHAR );
	chance = 20;
    }
    else
	chance = 35;

    act( "$n look$% at $N carefully, walking toward $M.", ch, NULL,
	vch, TO_ALL );
    if ( !IS_AWAKE(vch) )
    {
	if ( number_percent() < 3
	&&   !IS_AFFECTED(vch, AFF_SLEEP) )
	{
	    do_wake( vch, "" );
	    chance += 60;
	}
	else
	    chance += 70;
    }

    if ( !check_skill(ch, gsn_leashing, chance, TRUE) )
    {
	act( "$n fumble$% with $p, unable to collar $N.", ch, obj,
	    vch, TO_ALL );
	return;
    }

    if ( get_eq_char(vch, WEAR_NECK_1)
    &&   get_eq_char(vch, WEAR_NECK_2) )
    {
	OBJ_DATA *obj;

	obj = get_eq_char(vch, WEAR_NECK_1);
	if ( !check_stat(ch, STAT_AGI, -2)
	||   IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	{
	    obj = get_eq_char(vch, WEAR_NECK_2);

	    if ( !check_stat(ch, STAT_AGI, -2)
	    ||   IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	    {
		act("$n can't remove anything from $O neck.", ch,
		    get_eq_char(vch,WEAR_NECK_1), vch, TO_ALL );
		return;
	    }
	    act( "$n deftly remove$% $p from $O neck.", ch,
		obj, vch, TO_ALL );
	    unequip_char( vch, obj );
	}
	else
	{
	    act( "$n deftly remove$% $p from $O neck.", ch,
		obj, vch, TO_ALL );
	    unequip_char( vch, obj );	    
	}
    }

    /* sucker :) */
    extract_obj( obj );
    obj = create_object( get_obj_index(OBJ_VNUM_ADAM_COLLAR), 1 );
    act( "$n snap$% $p around $O neck and snap$% the bracelet onto $s wrist!",
	ch, obj, vch, TO_ALL );
    obj_to_char( obj, vch );
    if ( get_eq_char(vch, WEAR_NECK_1) )
	equip_char( vch, obj, WEAR_NECK_2 );
    else
	equip_char( vch, obj, WEAR_NECK_1 );

    if ( !can_channel(vch, 1) || TRUE_SEX(vch) != SEX_FEMALE )
    {
	REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
	return;
    }

    if ( TRUE_SEX(vch) == SEX_MALE
    &&   can_channel(vch, 1) )
    {
	/* heehee */
	int dam;
	int dt;

	dt = skill_lookup( "electrical shock" );
	dam = (channel_strength(vch, POWER_ALL) + channel_strength(ch, POWER_ALL)) * 4;

	REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
	act( "$n and $N scream loudly, their faces filled with pain!",
	    ch, NULL, vch, TO_NOTVICT );
	act( "You and $N scream loudly, your faces filled with pain!",
	    ch, NULL, vch, TO_CHAR );
	act( "$n and you scream loudly, your faces filled with pain!",
	    ch, NULL, vch, TO_VICT );
	spell_damage( ch, ch, dam, dt, DAM_LIGHTNING );
	spell_damage( vch, vch, dam, dt, DAM_LIGHTNING );
    }

    WAIT_STATE(ch, 4);
    do_leash( ch, PERS(vch, ch) );
    return;
}

void do_forcecast( CHAR_DATA *ch, char *argument )
{
    return;
}


void do_capture( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    OBJ_DATA *obj;
    bool pFound = FALSE;

    one_argument( argument, arg );

    if ( (vch = get_char_room(ch, arg)) == NULL )
    {
	send_to_char( "You do not see them here.\n\r", ch );
	return;
    }

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == 4292 )
	{
	    pFound = TRUE;
	    break;
	}
    }

    if ( !pFound )
    {
	send_to_char( "You must have a length of rope to capture someone.\n\r", ch );
	return;
    }

    if ( IS_IMMORTAL(vch) )
    {
	send_to_char( "Not likely to happen.\n\r", ch );
	return;
    }

    if ( IS_SET(vch->affected_by_2, AFF_CAPTURED) )
    {
	send_to_char_new( ch, "Someone has already tied up %s.\n\r",
	    PERS(vch, ch) );
	return;
    }

    if ( !IS_AWAKE(vch) )
    {
	if ( number_percent() >= 95 )
	    do_wake(vch, "");
	else
	{
	    if ( check_skill(ch, gsn_capture, 60, TRUE) )
	    {
		if ( check_skill(ch, gsn_capture, 20, FALSE) )
		    af.duration	= -1;
		else
		    af.duration = get_skill(ch, gsn_capture) / 6;
	    }
	    else
		af.duration = number_range(1, 3);
	    af.type		= gsn_capture;
	    af.strength		= get_skill(ch, gsn_capture);
	    af.modifier		= 0;
	    af.location		= 0;
	    af.bitvector	= 0;
	    af.bitvector_2	= AFF_CAPTURED;
	    af.owner		= NULL;
	    af.flags		= AFFECT_NOTCHANNEL;
	    affect_to_char(vch, &af);

	    act( "$n walk$% up to the sleeping form of $N and quickly bind$% $M with $p.",
		ch, obj, vch, TO_ALL );
	    extract_obj( obj );
	    return;
	}
    }

    if ( is_affected(vch, skill_lookup("wrap")) )
    {
	if ( check_skill(ch, gsn_capture, 60, TRUE) )
	{
	    if ( check_skill(ch, gsn_capture, 20, FALSE) )
		af.duration	= -1;
	    else
		af.duration	= get_skill(ch, gsn_capture) / 6;
	}
	else
	    af.duration = number_range(1, 3);
	af.type			= gsn_capture;
	af.strength		= get_skill(ch, gsn_capture);
	af.modifier		= 0;
	af.location		= 0;
	af.bitvector		= 0;
	af.bitvector_2		= AFF_CAPTURED;
	af.owner		= NULL;
	af.flags		= AFFECT_NOTCHANNEL;
	affect_to_char(vch, &af);

	act( "$n walk$% up to the unmoving form of $N and quickly bind$% $M with $p.",
	    ch, obj, vch, TO_ALL );
	extract_obj( obj );
	return;
    }


    if ( check_skill(ch, gsn_capture, 33, FALSE) )
    {
	if ( check_skill(ch, gsn_capture, 17, TRUE) )
	    af.duration	= -1;
	else
	    af.duration = get_skill(ch, gsn_capture) / 3;
    }
    else
	af.duration = number_range(4, 8);
    af.type		= gsn_capture;
    af.strength		= get_skill(ch, gsn_capture);
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_CAPTURED;
    af.owner		= NULL;
    af.flags		= AFFECT_NOTCHANNEL;
    affect_to_char(vch, &af);

    act( "$n grab$% $N, and quickly tie$% $p around $M.", ch, obj, vch,
	TO_ALL );
    extract_obj( obj );
    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int skill;

    skill = get_skill(ch, gsn_incognito);

    if ( !check_skill(ch, gsn_incognito, 100, TRUE)
    ||   IS_AFFECTED_2(ch, AFF_INCOGNITO) )
    {
	send_to_char( "You look just as normal as everyone else.\r\n", ch );
	REMOVE_BIT(ch->affected_by_2, AFF_INCOGNITO);
	affect_strip( ch, gsn_incognito );
	return;
    }

    affect_strip( ch, gsn_incognito );
    send_to_char( "You seem plainer than normal.\r\n", ch );
    af.type		= gsn_incognito;
    af.strength		= skill;
    af.duration		= number_range(1, skill);
    af.modifier		= 0;
    af.location		= 0;
    af.bitvector	= 0;
    af.bitvector_2	= AFF_INCOGNITO;
    af.owner		= NULL;
    af.flags		= AFFECT_NOTCHANNEL;

    affect_to_char(ch, &af);
    return;
}

