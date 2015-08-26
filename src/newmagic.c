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

#define MAX_DAMAGE_MESSAGE 32
#define RID		ROOM_INDEX_DATA

#define DO_TIE		0
#define DO_UNTIE	1
#define DO_CANCEL	2
#define DO_INVERT	3

/* command procedures needed */
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_grasp	);
DECLARE_DO_FUN(	do_release	);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_emote	);

DECLARE_SPELL_FUN( spell_null	);

void    group_heal      args( ( CHAR_DATA *ch ) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *victim, CHAR_DATA *ch, int dt ) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
RID	*get_random_room	args( ( CHAR_DATA *ch ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
int	get_random_door	args( ( ROOM_INDEX_DATA *pRoom ) );
bool	is_vowel	args( ( char letter ) );
int     weave_result    args( ( CHAR_DATA *ch, int sn, int strength ) );
void	weave_number	args( ( CHAR_DATA *ch, int an, sh_int type ) );
void	tie_number	args( ( CHAR_DATA *ch, int an ) );
void	untie_number	args( ( CHAR_DATA *ch, int an ) );
void	invert_number	args( ( CHAR_DATA *ch, int an ) );
void	cancel_number	args( ( CHAR_DATA *ch, int an ) );
char	*grasp_message	args( ( CHAR_DATA *ch ) );
int     hit_loc		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	absorb_damage	args( ( CHAR_DATA *ch, int dam, int dam_type, int location ) );
int	plain_strength	args( ( CHAR_DATA *ch, int power ) );


/*
 * Local functions.
 */
bool	check_unconc	args( ( CHAR_DATA *ch ) );
void	show_flows	args( ( CHAR_DATA *ch, int sn, void *vo, int target_type ) );
void	spell_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim,int dam,
			        int dt,bool immune, int location ) );
bool	owns_affect	args( ( CHAR_DATA *ch, bool fTied ) );
void	area_spell	args( ( CHAR_DATA *ch, int sn, int power ) );

#define POWER_REQ(sn, pow)      ( skill_table[sn].power[pow] > 0 )


/* Constants */
extern	char 	*	const   		dir_name	[];
extern			const	sh_int		rev_dir		[];


int power_percent( CHAR_DATA *ch, int sn )
{
    int powers[5], count, total, i;

    total = 0;
    count = 0;
    for ( i = 0; i < 5; i++ )
    {
	if ( skill_table[sn].power[i] > 0 )
	{
	    powers[i] = channel_strength(ch, 1 << i) * 100 / skill_table[sn].power[i];
	    if ( powers[i] > 1500 )
		powers[i] = 664 + (powers[i] - 1500) / 7;
	    else if ( powers[i] > 1400 )
		powers[i] = 548 + (powers[i] - 1400) / 6;
	    else if ( powers[i] > 1200 )
		powers[i] = 508 + (powers[i] - 1200) / 5;
	    else if ( powers[i] > 900 )
		powers[i] = 433 + (powers[i] - 900) / 4;
	    else if ( powers[i] > 500 )
		powers[i] = 300 + (powers[i] - 500) / 3;
	    else if ( powers[i] > 100 )
		powers[i] = 100 + (powers[i] - 100) / 2;
	    count++;
	}
	else
	    powers[i] = 0;
	total = total + powers[i];
    }

    if ( count == 0 )
        return 0;
        
    return total / count;
};  


/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL 
	||   skill_table[sn].name[0] == '\0' )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}

int talent_lookup( const char *name )
{
    int tn;

    for ( tn = 0; tn < MAX_TALENT; tn++ )
    {
	if ( talent_table[tn].name == NULL 
	||   talent_table[tn].name[0] == '\0' )
	    break;
	if ( LOWER(name[0]) == LOWER(talent_table[tn].name[0])
	&&   !str_prefix( name, talent_table[tn].name ) )
	    return tn;
    }

    return -1;
}

bool is_in_group( const char *name, const char *group )
{
    int skill, gn;

    gn = group_lookup( group );

    for ( skill = 0; skill < MAX_IN_GROUP; skill++ )
    {
	if ( group_table[gn].spells[skill] == NULL )
	    break;
	if ( !str_cmp(name, group_table[gn].spells[skill]) )
	    return TRUE;
    }
    return FALSE;
}

int weave_result( CHAR_DATA *ch, int sn, int strength )
{
    int power_req = 0;
    int power_have = 0;
    int i;
    int diff;
    int usage;
    
    if ( !IS_NPC(ch) )
	usage = ch->pcdata->usage[sn];
    else
	usage = 0;
    usage = UMAX( usage, 0 );

    for ( i = 0; i < 5; i++ )
    {
	int current_power;
	int current_req;
	int psn;

	current_req = skill_table[sn].power[i];
	if ( current_req < 1 )
	    continue;

	diff = 2 + (usage > 5)  + (usage > 10) + (usage > 20)
	         + (usage > 40) + (usage > 80) + (usage >= 100); 

	if ( i == 0 )
	{
	    current_power = channel_strength( ch, POWER_EARTH );
	    psn = gsn_earth;
	}
	else if ( i == 1 )
	{
	    current_power = channel_strength( ch, POWER_AIR );
	    psn = gsn_air;
	}
	else if ( i == 2 )
	{
	    current_power = channel_strength( ch, POWER_FIRE );
	    psn = gsn_fire;
	}
	else if ( i == 3 )
	{
	    current_power = channel_strength( ch, POWER_WATER );
	    psn = gsn_water;
	}
	else
	{
	    current_power = channel_strength( ch, POWER_SPIRIT );
	    psn = gsn_spirit;
	}

	if ( current_req - 5 > current_power ) /* Not enough strength to use it */
	{
	    check_improve( ch, psn, FALSE, diff + 1 );
	    return -100;
	}
	else if ( current_req > current_power ) /* range -5 to -1 */
	{
	    current_power = current_power - 8 * (current_req - current_power);
	    check_improve( ch, psn, FALSE, diff );
	}
	else
	    check_improve( ch, psn, TRUE, diff );

	power_have += current_power;
	power_req  += current_req;
    }
    return (power_have - power_req);
}

bool can_cast( CHAR_DATA *ch, int sn )
{
    int power_req, power_has, i;

    power_req = 0;
    power_has = 0;
    for ( i = 0; i < 5; i++ )
	power_req += skill_table[sn].power[i];

    power_has += get_skill( ch, gsn_earth );  
    power_has += get_skill( ch, gsn_air );  
    power_has += get_skill( ch, gsn_fire );  
    power_has += get_skill( ch, gsn_water );  
    power_has += get_skill( ch, gsn_spirit );  
    if ( power_has >= power_req
    &&   ch->level >= skill_table[sn].skill_level[ch->class] )
	return TRUE;
    return FALSE;
}

bool has_flows( CHAR_DATA *ch, int sn )
{
    int i;

    for ( i = 0; i < 5; i++ )
	if ( ch->channel_max[i] < skill_table[sn].power[i] )
	    return FALSE;

    return TRUE;
}

int table_normal( int table, int strength )
{
    int value;
    int min, max;
    int upper_base, lower_base;
    int i;

    if ( strength < 1 )
	return 0;

    lower_base = 3 * number_fuzzy( 9 ) + dice( 6, 3 );
    upper_base = lower_base;
    for ( i = 0; i < table * 5 / 2; i++ )
    {
	upper_base += number_range( 7, 11 );
	if ( table >= 4 )
	   upper_base += 1;

	if ( table >= 12 )
	   upper_base += 1;
 
	if ( table >= 24 )
	   upper_base += 1;

	if ( table == 40 )
	   upper_base += 1;

	if ( table == 60 )
	   upper_base += 1;

	if ( table == 84 )
	   upper_base += 1;

	if ( table == 112 )
	   upper_base += 1;

    }

    if ( table == 1 )
    {
	min = 0;
	max = 30;
    }
    else if ( table == 2 )
    {
	min = 0;
	max = upper_base;
    }
    else if ( table == 3 )
    {
	min = 0;
	max = upper_base;
    }
    else if ( table == 4 )
    {
	min = 0;
	max = upper_base;
    }
    else
    {
	min = ((table - 4) * 5) + lower_base;
	max = upper_base;
    }
    value = min + number_fuzzy( strength ) * ( max - min ) / 100;

    return( value );
}

int table_multiplier( int strength, int multiplier )
{
    int value;
    int min, max;
    int upper_base, lower_base;
    int i;

    if ( strength < 1 )
	return 0;

    lower_base = 0;
    upper_base = lower_base;
    for ( i = 0; i < strength / 2; i++ )
    {
	upper_base++;
	if ( strength >= 8 )
	   upper_base += 1;

	if ( strength >= 24 )
	   upper_base += 1;
 
	if ( strength >= 48 )
	   upper_base += 1;

	if ( strength >= 80 )
	   upper_base += 1;

	if ( strength >= 120 )
	   upper_base += 1;
    }

    if ( strength == 1 )
    {
	min = 0;
	max = 30;
    }
    else if ( strength == 2 )
    {
	min = 0;
	max = upper_base;
    }
    else if ( strength == 3 )
    {
	min = 0;
	max = upper_base;
    }
    else if ( strength == 4 )
    {
	min = 0;
	max = upper_base;
    }
    else
    {
	min = (strength - 4) + lower_base;
	max = upper_base;
    }
    value = min + number_fuzzy( strength ) * ( max - min ) / 100;
    value = UMAX( 1, value * multiplier / 100 );

    return( value );
}

bool tie_weave( CHAR_DATA *ch, AFFECT_DATA *paf, int diff )
{
    int chance;
    int strength;

    strength = channel_strength( ch, POWER_ALL );
    chance = UMIN(100, 100 - paf->strength - diff + strength);

    if ( paf->owner != ch )
    {
	send_to_char( "You are not maintaining that weave.\n\r", ch );
	return FALSE;
    }

    if ( !check_skill(ch, gsn_tie_weave, chance, TRUE) )
    {
        send_to_char( "You are unable to tie the weave.\n\r", ch );
        check_improve( ch, gsn_tie_weave, FALSE, 3 );
	return FALSE;
    }
    check_improve( ch, gsn_tie_weave, TRUE, 3 );
    return TRUE;
}

bool untie_weave( CHAR_DATA *ch, AFFECT_DATA *paf, int diff )
{
    int chance;
    int strength;

    strength = channel_strength( ch, POWER_ALL );
    chance = UMIN(100, 100 - paf->strength - diff + strength);

    if ( !IS_TIED(paf) )
    {
	send_to_char( "That weave is not tied off.\n\r", ch );
	return FALSE;
    }

    if ( !check_skill(ch, gsn_tie_weave, chance, TRUE) )
    {
	send_to_char( "You are unable to untie the weave.\n\r", ch );
	check_improve( ch, gsn_tie_weave, FALSE, 3 );
	return FALSE;
    }
    check_improve( ch, gsn_tie_weave, TRUE, 3 );
    return TRUE;
}

bool cancel_weave( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( paf->owner != ch )
    {
	send_to_char( "You have not woven that, you must untie it instead.\n\r", ch );
	return FALSE;
    }
    send_to_char( "You cancel the weave, causing it to fade.\n\r", ch );
    return TRUE;
}

bool invert_weave( CHAR_DATA *ch, AFFECT_DATA *paf, int diff )
{
    int chance;
    int strength;

    strength = channel_strength( ch, POWER_ALL );
    chance = UMIN(100, 100 - paf->strength - diff + strength);

    if ( paf->owner != ch )
    {
	send_to_char( "You have not woven that.\n\r", ch );
	return FALSE;
    }

    if ( !check_skill(ch, gsn_tie_weave, chance, TRUE) )
    {
	send_to_char( "You were unable to invert the flows.\n\r", ch );
	check_improve( ch, gsn_invert_weave, FALSE, 3 );
	return FALSE;
    }
    check_improve( ch, gsn_invert_weave, TRUE, 2 );
    return TRUE;
}


void do_tie( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    OBJ_DATA *vobj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA* vRoom;
    int sn;
    bool found = FALSE;

    if ( IS_NPC(ch) )
        return;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( !IS_GRASPING(ch) )
    {
	send_to_char( "You must be grasping the One Power to tie weaves.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	int an;

	an = atoi(arg1);
	tie_number( ch, an );
	return;
    }
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Which weave on what do you wish to tie off?\n\r", ch );
        return;
    }
    
    if ( (sn = skill_lookup( arg1 )) < 0 )
    {       
        send_to_char( "That is not a weave.\n\r", ch );
        return;
    }
    
    if ( !str_cmp("here", arg2) || !str_cmp("room", arg2) )
    {
        vRoom = ch->in_room;
        for ( paf = vRoom->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
            {
		if ( !tie_weave(ch, paf, 0) )
		    return;
		send_to_char( "You tie the weave off.\n\r", ch );
		SET_BIT(paf->flags, AFFECT_TIED);
                found = TRUE;
            }
        }
        if ( !found )
            send_to_char( "You cannot see that weave here.\n\r", ch );
        return;
    }
    
    if ( (victim = get_char_room( ch, arg2)) != NULL )
    {
        for ( paf = victim->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
            {
		if ( !tie_weave(ch, paf, 0) )
		    return;
                found = TRUE;
                send_to_char( "You tie the weave off.\n\r", ch );
		SET_BIT(paf->flags, AFFECT_TIED);
            }
        }
        if ( !found )
            act( "You cannot see that weave on $t.", ch, ch->short_descr,
                NULL, TO_CHAR );
        return;  
    }
                
    if ( (vobj = get_obj_here( ch, arg2)) != NULL )
    {
        for ( paf = vobj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
            {
		if ( !tie_weave(ch, paf, 0) )
		    return;
                found = TRUE;
                send_to_char( "You tie the weave off.\n\r", ch );
		SET_BIT(paf->flags, AFFECT_TIED);
            }
        }
        if ( !found )
            act( "You cannot see that weave on $t.", ch, vobj->short_descr,
                NULL, TO_CHAR );
        return;  
    }
                
    send_to_char( "You cannot find that weave.\n\r", ch );
    return;
}
                    
void do_untie( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    OBJ_DATA *vobj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA* vRoom;
    int sn;
    bool found = FALSE;
     
    if ( IS_NPC(ch) )
        return;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( !IS_GRASPING(ch) )
    {
	send_to_char( "You must be grasping the One Power to untie weaves.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	int an;

	an = atoi(arg1);
	untie_number( ch, an );
	return;
    }
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Which weave on what do you wish to untie?\n\r", ch );
        return;
    }
    
    if ( (sn = skill_lookup( arg1 )) < 0 )
    {
        send_to_char( "That is not a weave.\n\r", ch );
        return;
    }

    if ( !str_cmp("here", arg2) || !str_cmp("room", arg2) )
    {
	vRoom = ch->in_room;
	for ( paf = vRoom->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		if ( !untie_weave(ch, paf, 10) )
		    return;
		if ( paf->owner == ch && IS_GRASPING(ch) )
		{
		    REMOVE_BIT(paf->flags, AFFECT_TIED);
		    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
		}
		else
		{
		    if ( paf->owner != ch )
			act( "You feel your $t weave in $T being taken apart.",
			    paf->owner, skill_table[sn].name,
			    vRoom->name, TO_CHAR );
		    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
		    affect_room_remove( vRoom, paf ); /* untied */
		}
	    }
	}
	if ( !found )
	    send_to_char( "You cannot see that weave here.\n\r", ch );
	return;
    }
                 
    if ( (victim = get_char_room( ch, arg2)) != NULL )
    {
	for ( paf = victim->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		found = TRUE;
		if ( !untie_weave(ch, paf, 15) )
		    return;
		if ( paf->owner == ch && IS_GRASPING(ch) )
		{
		    REMOVE_BIT(paf->flags, AFFECT_TIED);
		    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
		}
		else
		{
		    if ( paf->owner != NULL && paf->owner != ch )
			act( "You feel your $t weave on $N being taken apart.",
			    paf->owner, skill_table[sn].name,
			   victim, TO_CHAR );
		    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
		    if ( paf->type > 0 && skill_table[sn].msg_off )
		    {
			send_to_char( skill_table[sn].msg_off, victim );
			send_to_char( "\n\r", victim );
		    }
		    affect_remove( victim, paf ); /* untied */
		}
	    }
	}
	if ( !found )
	    act( "You cannot see that weave on $N.", ch, NULL,
		victim, TO_CHAR );
        return;
    }
                        
    if ( (vobj = get_obj_here( ch, arg2)) != NULL )
    {
	for ( paf = vobj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		found = TRUE;
		if ( !untie_weave(ch, paf, 15) )
		    return;

		if ( paf->owner == ch && IS_GRASPING(ch) )
		{
		    REMOVE_BIT(paf->flags, AFFECT_TIED );
		    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
		}
		else
		{
		    if ( paf->owner != NULL )
			act( "You feel your $t weave on $P being taken apart.",
			    paf->owner, skill_table[sn].name,
			    vobj, TO_CHAR );
		    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
		    affect_obj_remove( vobj, paf ); /* untied */
		}
            }
        }
        if ( !found )
            act( "You cannot see that weave on $P.", ch, NULL,
                vobj, TO_CHAR );
        return;
    }
                    
    send_to_char( "You have not woven that.\n\r", ch );
    return;
}
                        
void do_cancel( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    OBJ_DATA *vobj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA* vRoom;
    int sn;
    bool found = FALSE;
     
    if ( IS_NPC(ch) )
        return;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	int an;

	an = atoi(arg1);
	cancel_number( ch, an );
	return;
    }
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Which weave on what do you wish to cancel?\n\r", ch );
        return;
    }

    if ( (sn = skill_lookup( arg1 )) < 0 )
    {
        send_to_char( "That is not a weave.\n\r", ch );
        return;
    }

    if ( !str_cmp("here", arg2) || !str_cmp("room", arg2) )
    {
	vRoom = ch->in_room;
	for ( paf = vRoom->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( !cancel_weave(ch, paf) )
		return;
	    affect_room_remove( vRoom, paf );
	    found = TRUE;
	}
	if ( !found )
	    send_to_char( "You cannot see that weave here.\n\r", ch );
	return;
    }
            
    if ( (victim = get_char_room( ch, arg2)) != NULL )
    {
	for ( paf = victim->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		found = TRUE;
		if ( !cancel_weave(ch, paf) )
		    return;
		if ( paf->type > 0 && skill_table[sn].msg_off )
		{
		    send_to_char( skill_table[sn].msg_off, victim );
		    send_to_char( "\n\r", victim );   
		}
		affect_remove( victim, paf );
	    }
	}
	if ( !found )
	    act( "You cannot see that weave on $N.", ch, NULL,
		victim, TO_CHAR );
	return;  
    }
                    
    if ( (vobj = get_obj_here( ch, arg2)) != NULL )
    {
	for ( paf = vobj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn )
	    {
		found = TRUE;
		if ( !cancel_weave(ch, paf) )
		    return;
		affect_obj_remove( vobj, paf ); /* untied */
	    }
	}           
	if ( !found )
	    act( "You cannot see that weave on $P.", ch, NULL,
		vobj, TO_CHAR );
	return;
    }
            
    send_to_char( "You have not woven that.\n\r", ch );
    return;
}

void do_invert( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    OBJ_DATA *vobj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA* vRoom;
    int sn;
    bool found = FALSE;

    if ( IS_NPC(ch) )
        return;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( !IS_GRASPING(ch) )
    {
	send_to_char( "You must be grasping the One Power to invert weaves.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	int an;

	an = atoi(arg1);
	invert_number( ch, an );
	return;
    }
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Which weave on what do you wish to invert?\n\r", ch );
        return;
    }

    if ( (sn = skill_lookup( arg1 )) < 0 )
    {
        send_to_char( "That is not a weave.\n\r", ch );
        return;
    }

    if ( !str_cmp("here", arg2) || !str_cmp("room", arg2) )
    {
	vRoom = ch->in_room;
	for ( paf = vRoom->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		if ( !invert_weave(ch, paf, 10) )
		    return;
		found = TRUE;
		send_to_char( "You invert the flows of the weave.\n\r", ch );
		TOGGLE_BIT(paf->flags, AFFECT_INVERT);
	    }
	}
	if ( !found )
	    send_to_char( "You cannot see that weave here.\n\r", ch );
	return;
    }
            
    if ( (victim = get_char_room( ch, arg2)) != NULL )
    {
	for ( paf = victim->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		if ( !invert_weave(ch, paf, 15) )
		    return;
		found = TRUE;
		send_to_char( "You invert the flows of the weave.\n\r", ch );
		TOGGLE_BIT(paf->flags, AFFECT_INVERT);
	    }
	}
	if ( !found )
	    act( "You cannot see that weave on $N.", ch, NULL,
		victim, TO_CHAR );
	return;  
    }
                    
    if ( (vobj = get_obj_here( ch, arg2)) != NULL )
    {
	for ( paf = vobj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;
	    if ( paf->type == sn && !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		if ( !invert_weave(ch, paf, 15) )
		    return;
		found = TRUE;
		send_to_char( "You invert the flows of the weave.\n\r", ch );
		TOGGLE_BIT(paf->flags, AFFECT_INVERT);
	    }
	}           
	if ( !found )
	    act( "You cannot see that weave on $P.", ch, NULL,
		vobj, TO_CHAR );
	return;
    }
            
    send_to_char( "You have not woven that.\n\r", ch );
    return;
}



void show_flows( CHAR_DATA *ch, int sn, void *vo, int target_type )
{
    CHAR_DATA *rch, *victim = NULL;
    OBJ_DATA *obj = NULL;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int count;
    int i;
    const char	*	power_name[] =
    {
	"$C0", "$C1", "$C2", "$C3","$C4"
    };

    count = 0;
    for ( i = 0; i < 5; i++ )
	if ( skill_table[sn].power[i] > 0 )
	    count++;

    sprintf( buf1, "You weave %sflow%s of", (count > 1) ? "" : "a ",
	(count > 1) ? "s" : "" );
    sprintf( buf2, "$n weaves %sflow%s of", (count > 1) ? "" : "a ",
	(count > 1) ? "s" : "" ); 

    count = 0;
    for ( i = 0; i < 5; i++ )
    {
	bool and_found = TRUE;
	int j;
	if ( skill_table[sn].power[i] > 0 )
	   count++;
	for ( j = i + 1; j < 5; j++ )
	{
	    if ( skill_table[sn].power[j] > 0 )
		and_found = FALSE;
	}

	if ( count > 1 && !and_found && skill_table[sn].power[i] > 0 )
	{
	    strcat( buf1, "," );
	    strcat( buf2, "," );
	}

	if ( count > 1 && and_found && skill_table[sn].power[i] > 0 )
	{
	    strcat( buf1, " and" );
	    strcat( buf2, " and" );
	}
	if ( skill_table[sn].power[i] > 0 )
	{
	    strcat( buf1, " " );
	    strcat( buf2, " " );
	    strcat( buf1, power_name[i] );
	    strcat( buf2, power_name[i] );
	}
    }

    switch ( target_type )
    {
	default:
	case TAR_CHAR_WORLD:
	case TAR_IGNORE:
	    strcat( buf1, "." );
	    strcat( buf2, "." );
	    act( buf1, ch, NULL, NULL, TO_CHAR );
	    act_channel( buf2, ch, NULL, NULL, TO_ROOM );
	    break;
	case TAR_CHAR_SELF:
	    strcat( buf1, " around yourself." );
	    strcat( buf2, " around $mself." );
	    act( buf1, ch, NULL, NULL, TO_CHAR );
	    act_channel( buf2, ch, NULL, NULL, TO_ROOM );
	    break;
	case TAR_CHAR_OTHER:
	    victim = (CHAR_DATA *) vo;
	    strcat( buf1, " around $N." );
	    strcat( buf2, " around $N." );
	    act( buf1, ch, NULL, victim, TO_CHAR );
	    act_channel( buf2, ch, NULL, victim, TO_ROOM );
	    break;
	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_OFFENSIVE:
	    victim = (CHAR_DATA *) vo;
	    if ( ch == victim )
	    {
		strcat( buf1, " around yourself." );
		strcat( buf2, " around $mself." );
		act( buf1, ch, NULL, NULL, TO_CHAR );
		act_channel( buf2, ch, NULL, NULL, TO_ROOM );
	    }
	    else
	    {
		strcat( buf1, " around $N." );
		strcat( buf2, " around $N." );
		act( buf1, ch, NULL, victim, TO_CHAR );
		act_channel( buf2, ch, NULL, victim, TO_ROOM );
	    }
	    break;
	case TAR_IN_ROOM:
	    strcat( buf1, " around the area." );
	    strcat( buf2, " around the area." );
	    act( buf1, ch, obj, NULL, TO_CHAR );
	    act_channel( buf2, ch, obj, NULL, TO_ROOM );
	    break;
	case TAR_OBJ_HERE:
	case TAR_OBJ_INV:
	    obj = (OBJ_DATA *) vo;
	    strcat( buf1, " around $p." );
	    strcat( buf2, " around $p." );
	    act( buf1, ch, obj, NULL, TO_CHAR );
	    act_channel( buf2, ch, obj, NULL, TO_ROOM );
	    break;
    }

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	{
	    if ( !can_channel(rch, 1) )
		continue;

	    if ( !IS_AWAKE(rch) )
		continue;

	    if ( channel_strength(rch, POWER_ALL) <= number_fuzzy(30) )
		continue;

	    if ( TRUE_SEX(ch) == TRUE_SEX(rch) )
	    {	
		if ( !IS_NPC(rch) && rch->pcdata->learned[sn] >= 1 )
		    act( "$n weaves a flow of $t.", ch, skill_table[sn].name, rch, TO_VICT );
		else
		    act( "$n weaves a flow that you don't recognize.", ch, NULL, rch, TO_VICT );
		if ( !IS_NPC(rch) && rch->pcdata->learned[sn] == 0 )
		{
		    if ( number_percent() == 1 )
		    {
			act( "You blink, suddenly realizing how those flows are woven!",
			    rch, NULL, NULL, TO_CHAR );
			rch->pcdata->learned[sn] = 1;
		    }
		}
		check_improve( rch, sn, TRUE, 4 );
	    }
	    else if ( TRUE_SEX(rch) == SEX_MALE )
		act( "Your skin feels tingly.", rch, NULL, NULL, TO_CHAR );
	}
    }
    return;
}

void emote_flows( CHAR_DATA *ch, int powers )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int count;
    int i;
    const char	*	power_name[] =
    {
	"$C0", "$C1", "$C2", "$C3","$C4"
    };

    count = 0;
    for ( i = 0; i < 5; i++ )
	if ( powers & (1 << i) )
	    count++;

    sprintf( buf1, "You weave %sflow%s of", (count > 1) ? "" : "a ",
	(count > 1) ? "s" : "" );
    sprintf( buf2, "$n weaves %sflow%s of", (count > 1) ? "" : "a ",
	(count > 1) ? "s" : "" ); 

    count = 0;
    for ( i = 0; i < 5; i++ )
    {
	bool and_found = TRUE;
	bool power_found = powers & (1 << i);
	int j;
	if ( power_found )
	   count++;
	for ( j = i + 1; j < 5; j++ )
	{
	    if ( powers & (1 << j) )
		and_found = FALSE;
	}

	if ( count > 1 && !and_found && power_found )
	{
	    strcat( buf1, "," );
	    strcat( buf2, "," );
	}

	if ( count > 1 && and_found && power_found )
	{
	    strcat( buf1, " and" );
	    strcat( buf2, " and" );
	}
	if ( power_found )
	{
	    strcat( buf1, " " );
	    strcat( buf2, " " );
	    strcat( buf1, power_name[i] );
	    strcat( buf2, power_name[i] );
	}
    }

    strcat( buf1, "." );
    strcat( buf2, "." );
    act( buf1, ch, NULL, NULL, TO_CHAR );
    act_channel( buf2, ch, NULL, NULL, TO_ROOM );

    return;
};

/* routine for Cutting Weaves */
bool break_weave( int char_strength, int weave_strength )
{
    char_strength += number_fuzzy( 0 ) * dice( 1, 3 );
    weave_strength += number_fuzzy( 0 ) * dice( 1, 3 );

    if ( char_strength >= weave_strength )
	return TRUE;

    return FALSE;
}

bool break_shield( CHAR_DATA *ch )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    bool shield_found = FALSE;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == skill_lookup("shield from true source") )
	{
	    int strength;
	    int shield_strength;
	    strength = channel_strength( ch, POWER_ALL );

	    if ( IS_TIED(paf) || paf->owner == NULL )
		shield_strength = number_range( paf->strength * 3 / 4, paf->strength );
	    else
		shield_strength = channel_strength( paf->owner, POWER_ALL );

	    if ( break_weave(strength, shield_strength) )
	    {
		shield_found = TRUE;
		act( "You tear through the Shield, and `B$t`n pours into you!",
		    ch, SOURCE(ch), NULL, TO_CHAR );
		if ( paf->owner != NULL )
		{
		    act( "You recoil in agony as your Shield on $n is broken through, $t surging through you.",
			ch, SOURCE(paf->owner), paf->owner, TO_VICT );
		    WAIT_STATE(paf->owner, 4 );
		    lose_stamina( ch, UMAX(1, strength/10), FALSE, TRUE );
		}
		affect_strip( ch, paf->type );
	    }
	    else
	    {
		if ( paf->owner != NULL )
		    act( "$n tries to break through your Shield on $m!",
			ch, NULL, paf->owner, TO_VICT );
		send_to_char( "You fail to break the shield.\n\r", ch );
		WAIT_STATE( ch, 3 );
	    }		
	    break;
	}
    }
    if ( shield_found == FALSE )
	return FALSE;
    return TRUE;
}

int plain_strength( CHAR_DATA *ch, int power )
{
    int value = 0;
    int count = 0;

    if ( IS_NPC(ch) )
    {
        if ( !IS_SET(ch->act, ACT_CHANNELER) )
            return 0; 
        if ( power & POWER_EARTH )
        {
            count++;
            value += ch->channel_skill[0];
        }
        if ( power & POWER_AIR )
        {
            count++;
            value += ch->channel_skill[1];
        }
        if ( power & POWER_FIRE )
        {
            count++;
            value += ch->channel_skill[2];
        }
        if ( power & POWER_WATER )
        {
            count++;
            value += ch->channel_skill[3];
        }
        if ( power & POWER_SPIRIT )
        {
            count++;
            value += ch->channel_skill[4];
        }
            
        if ( value == 0 || count == 0 )
            return 0;
        return( value / count );
    }
            
    if ( power & POWER_EARTH )
    {
        count++;
	value += SKILL(ch,gsn_earth);
    }
    if ( power & POWER_AIR )
    {
        count++;
	value += SKILL(ch,gsn_air);
    }
    if ( power & POWER_FIRE )   
    {
        count++;
	value += SKILL(ch,gsn_fire);
    }
    if ( power & POWER_WATER )
    {
        count++;
	value += SKILL(ch,gsn_water);
    }
    if ( power & POWER_SPIRIT )
    {
        count++;
	value += SKILL(ch,gsn_spirit);
    }
        
    if ( value == 0 || count == 0 )
        return 0;
    
    return( value / count );
}


/* for finding stamina costs -- temporary version */
int stamina_cost( CHAR_DATA *ch, int sn )
{
    int stamina, min_stamina, i, strength;
    int reduce[11] =
    {	0, 0, 0, 0, 0, 0, 4, 12, 24, 40, 60	};

    strength = plain_strength( ch, POWER_ALL );

    min_stamina = 0;
    for ( i = 0; i < 5; i++ )
	min_stamina += skill_table[sn].power[i];

/*
    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	stamina = 50;
    else
    	stamina = UMAX(
	    min_stamina * 5 / 6,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );
*/

    if ( ch->level < skill_table[sn].skill_level[ch->class] )
	stamina = min_stamina + ( 2 * min_stamina * ch->level / skill_table[sn].skill_level[ch->class] );
    else
	stamina = min_stamina * 6 / 5;

    if ( skill_table[sn].target == TAR_IN_ROOM )
	stamina = stamina * 5 / 3;
    stamina = stamina - (stamina * reduce[strength / 10] / 100);
    return( stamina );
}

bool check_unconc( CHAR_DATA *ch )
{
    if ( ch->stamina >= channel_strength(ch, POWER_ALL) * -1 )
    {
	if ( number_percent() < ( ch->stamina * -1 ))
	{
	    AFFECT_DATA af;
	    int mod;
	    mod = UMAX( ch->stamina / 4 * -1, 1);
	    send_to_char( "`3Managing the flows is too much for you, you pass out!`n\n\r", ch );
	    act( "$n screams suddenly and passes out!", ch, NULL, NULL, TO_ROOM );
	    if ( IS_GRASPING(ch) )
		do_release( ch, "" );
	    af.type      = skill_lookup( "sleep" );
	    af.strength		= 1;
	    af.duration  = 4;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_SLEEP;
	    af.bitvector_2 = 0;
	    af.owner     = NULL;
	    af.flags	 = AFFECT_NOTCHANNEL;
	    affect_join( ch, &af );
	    ch->position = POS_SLEEPING;
	    return TRUE;
	}
	send_to_char( "You feel the strain of channeling upon you.\n\r", ch );
	return FALSE;
    }
    else if ( ch->stamina >= channel_strength(ch, POWER_ALL) * -2 )
    {
	if ( number_percent() < ( (ch->stamina + channel_strength(ch, POWER_ALL)) * -1 ) )
	{
	    send_to_char( "You draw too much of the One Power, and burn yourself out!\n\r", ch );
	    act( "$n screams suddenly and pales.", ch, NULL, NULL, TO_ROOM );
	    if ( owns_affect(ch, TRUE) )
		send_to_char( "All of your maintained weaves fade.\n\r", ch );
	    die_weave( ch );
	    if ( IS_GRASPING(ch) )
		do_release( ch, "" );
	    SET_BIT( ch->affected_by_2, AFF_STILL );
	    return TRUE;
	}
	send_to_char( "You feel a great strain upon you.\n\r", ch );
	return FALSE;
    }
    else
    {
	send_to_char( "You attempt to channel too much of the One Power, and it races through your body, killing you.\n\r", ch );
	act( "$n screams for a second, then collapses!", ch, NULL, NULL, TO_ROOM );
	ch->hit = 0;
	ch->stamina = 0;
	update_pos( ch );
	raw_kill( ch, ch, DAM_ENERGY );
	return TRUE;
    }
    return FALSE;
}


bool spell_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{

    OBJ_DATA *corpse;
    bool immune;
    int location;

    if ( victim->position == POS_DEAD )
	return FALSE; 

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;

        if ( ch->fighting == NULL
	&&   ch->in_room == victim->in_room )
            set_fighting( ch, victim );
 
	if ( victim->position > POS_STUNNED
	&&   ch->in_room == victim->in_room )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
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
	affect_strip( ch, gsn_shape_change );
	affect_strip( ch, gsn_disguise );
	REMOVE_BIT( ch->affected_by, AFF_SHAPE_CHANGE );
	free_string( ch->pcdata->new_name );
	ch->pcdata->new_name = NULL;
    }
        
    /*
     * Damage modifiers.
     */         
    if ( IS_AFFECTED(victim, AFF_AIR_ARMOR) )
	dam /= 2;

    location = hit_loc( ch, victim, dt );
    switch( dam_type )
    {
	default:
	    break;
	case DAM_BASH:
	case DAM_PIERCE:
	case DAM_SLASH:
	case DAM_FIRE:
	case DAM_COLD:
	case DAM_LIGHTNING:
	case DAM_ACID:
	    dam = absorb_damage( victim, dam, dam_type, location );         
	    break;
    }
    immune = FALSE;
        
    switch( check_immune(victim, dam_type) ) 
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    spell_message( ch, victim, dam, dt, immune, location );
    
    if (dam == 0)
	return FALSE;
     
    /*
     * Hurt the victim.   
     * Inform the victim of his new state.
     */
    lose_health( victim, dam, TRUE );
    if ( dt == DAM_BASH )
	lose_stamina( victim, dam / number_range(10, 25), TRUE, TRUE );
    else
	lose_stamina( victim, dam / number_range(25, 50), TRUE, TRUE );
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
        
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );
            
	if ( IS_SET(ch->comm, COMM_AUTOHEAL) )
	    group_heal( ch );
	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d.",
		victim->name,
	        (IS_NPC(ch) ? ch->short_descr : ch->name),
	        victim->in_room->vnum );
	    log_string( log_buf );
        
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
		sprintf( buf, "You have lost %d experience points.\n\r", loss );
		send_to_char( buf, victim );
		gain_exp( victim, loss );
	    }
	}
                        
	raw_kill( victim, ch, dam_type );

	/* RT new auto commands */  
                        
	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );
                    
	    if ( IS_SET(ch->act, PLR_AUTOLOOT)
	    &&   corpse && corpse->contains ) /* exists and not empty */
		do_get( ch, "all corpse" );
                
            if ( IS_SET(ch->act,PLR_AUTOGOLD) && corpse
	    &&   corpse->contains
	    &&   !IS_SET(ch->act,PLR_AUTOLOOT) )
		do_get(ch, "gold corpse");
                        
	    if ( IS_SET(ch->act, PLR_AUTOSAC) ) {
		if ( IS_SET(ch->act,PLR_AUTOLOOT)
		&&   corpse && corpse->contains )
		    return TRUE;  /* leave if corpse has treasure */
		else      
		    do_sacrifice( ch, "corpse" );
            }
	}
        
	return TRUE;
    }

    if ( victim == ch )
	return FALSE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_flee( victim, "" );
	    return FALSE;
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
    return FALSE;
}

void spell_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune, int location )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *hp;
    const char *attack;
    const char *an;
    char punct;
    int value;
    
    if ( victim->hit > 0 )
	value = dam * 100 / victim->hit;
    else
	value = 110;
         
         if ( value == 0
         &&   dam   == 0   ) { hp = "causing no wounds to";	}
    else if ( value == 0
         &&   dam   != 0   ) { hp = "hardly touching";		}
    else if ( value <= 2   ) { hp = "merely tapping";		}
    else if ( value <= 5   ) { hp = "only nicking";		}
    else if ( value <= 10  ) { hp = "barely scratching";	}
    else if ( value <= 15  ) { hp = "grazing";			}
    else if ( value <= 30  ) { hp = "slightly wounding";	}
    else if ( value <= 45  ) { hp = "injuring";			}
    else if ( value <= 60  ) { hp = "moderately wounding";	}
    else if ( value <= 75  ) { hp = "wounding";			}
    else if ( value <= 90  ) { hp = "gravely wounding";		}
    else if ( value <= 95  ) { hp = "almost killing";		}
    else if ( value <= 100 ) { hp = "fatally wounding";		}
    else                     { hp = "killing";			}

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
		vp,dam == 0 ? "" : wear_name[location],hp,punct);
	    sprintf( buf2, "`3You %s yourself%s, %s yourself%c`n",
		vs,wear_name[location],hp,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%s, %s $M%c",
		vp,dam == 0 ? "" : wear_name[location],hp, punct );
	    sprintf( buf2, "You %s $N%s, %s $M%c",
		vs,dam == 0 ? "" : wear_name[location],hp, punct );
	    sprintf( buf3, "`3$n %s you%s, %s you%c`n",
		vp,dam == 0 ? "" : wear_name[location],hp, punct );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack = skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&&        dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
	    attack      = attack_table[dt - TYPE_HIT].name;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}
	if ( is_vowel(attack[0]) )
	    an = "An";
	else
	    an = "A";

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.",attack);
		sprintf(buf2,"`3Luckily, you are immune to that.`n");
	    }
	    else
	    {
		sprintf(buf1,"$N is unaffected by a %s!",attack);
		sprintf(buf2,"$N is unaffected by your %s!",attack);
		sprintf(buf3,"`3$n's %s is powerless against you.`n",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "%s %s %s $m%s, %s $m%c",
		    an,attack, vp, dam == 0 ? "" :wear_name[location],hp,punct);
		sprintf( buf2, "`3Your %s %s you%s, %s you%c`n",
		    attack,vp,dam == 0 ? "" : wear_name[location],hp,punct);
	    }
	    else
	    {
		sprintf( buf1, "%s %s %s $N%s, %s $M%c",
		    an,attack,vp,dam == 0 ?"" : wear_name[location],hp,punct);
		sprintf( buf2, "Your %s %s $N%s, %s $M%c",
		    attack,vp,dam == 0 ? "" :wear_name[location],hp,punct);
		sprintf( buf3, "`3%s %s %s you%s, %s you%c`n",
		    an,attack,vp,dam == 0 ? "" : wear_name[location],hp,punct );
	    }
	}
    }
                
    if (ch == victim)
    {
	act_fight(buf1,ch,NULL,victim,TO_ROOM);
    }
    else 
    {
	if ( ch->in_room == victim->in_room )
	    act_fight( buf2, ch, NULL, victim, TO_CHAR );
	act_fight( buf1, ch, NULL, victim, TO_NOTVICT );
	act_fight( buf3, ch, NULL, victim, TO_VICT );
    }
            
    return;  
}


void do_ecast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    char *ptr;
    int powers = 0;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You do not know how to channel.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   (ch->master != NULL
    ||    ( ch->leader != NULL && ch->leader != ch )) )
    {
	send_to_char( "You cannot channel when not the controller of a link.\n\r", ch );
	return;
    }

    if ( !IS_GRASPING(ch) )
	do_grasp( ch, "" );
    if ( !IS_GRASPING(ch) )
	return;

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) )
    {
	send_to_char( "You must enter both the flows you use and the effect to echo.\r\n", ch );
	return;
    }
    for ( ptr = arg; *ptr != '\0'; ptr++ )
    {
	if ( UPPER(*ptr) == 'E' )
	    powers |= 1;
	if ( UPPER(*ptr) == 'A' )
	    powers |= 2;
	if ( UPPER(*ptr) == 'F' )
	    powers |= 4;
	if ( UPPER(*ptr) == 'W' )
	    powers |= 8;
	if ( UPPER(*ptr) == 'S' )
	    powers |= 16;
    }

    emote_flows( ch, powers );
    sprintf( arg, "[EMOTE-CAST] %s", argument );
    act( arg, ch, NULL, NULL, TO_ALL );
    return;
};


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
bool successful_cast;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *place;
    void *vo;
    int stamina_req;
    int stamina_per;
    int stamina_add;
    int concentrate_mod;
    int sn;
    int luck = 0;
    int target_type = TAR_IGNORE;
    int multiplier = 100;
    bool failure = FALSE;
    bool strength = 0;

    luck = luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod;
    stamina_per = stamina_status( ch );
    stamina_add = 0;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) )
	return;

    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You do not know how to channel.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   (ch->master != NULL
    ||    ( ch->leader != NULL && ch->leader != ch )) )
    {
	send_to_char( "You cannot channel when not the controller of a link.\n\r", ch );
	return;
    }

    target_name = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Channel which what where?\n\r", ch );
	return;
    }

    if ( is_number(arg1) )
    {
	stamina_add = atoi( arg1 );
	target_name = one_argument( target_name, arg1 );
    }

    one_argument( target_name, arg2 );

    if ( (sn = skill_lookup( arg1 )) < 0
    ||   (!IS_NPC(ch) && ch->level < skill_table[sn].skill_level[ch->class])
    ||   get_skill(ch, sn) < 1 )
    {
	send_to_char( "You do not know that weave.\n\r", ch );
	return;
    }

    if ( skill_table[sn].spell_fun == spell_null )
    {
	send_to_char( "That is not a weave.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL
    &&   skill_table[sn].minimum_position != POS_FIGHTING )
    {
        send_to_char( "You cannot use this weave during combat.\n\r", ch ); 
        return;
    }
  
    if ( !IS_GRASPING(ch) )
	do_grasp( ch, "" );
    if ( !IS_GRASPING(ch) )
	return;

    stamina_req = stamina_cost( ch, sn );

    if ( stamina_add > 0 )
    {
	int allowed;
	allowed = stamina_req * channel_strength( ch, POWER_ALL ) * 3 / 200;
	if ( stamina_add > allowed )
	{
	    stamina_add = allowed;
	    send_to_char_new( ch, "You may only put an additional %d stamina into this weave at the moment.\n\r", allowed );
	}
	multiplier = ( stamina_req + stamina_add ) * 100 / stamina_req;
	stamina_req += stamina_add;
    }

    if ( ch->wait > 0 )
    {
        int penalty;

        penalty = 100 - ( ch->wait * 5 );
        multiplier = multiplier * penalty / 100;
    }

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IN_ROOM:
	break;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Use the weave on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	if ( !IS_NPC(ch) )
	{

            if (is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}
	if ( ch == victim )
	{
	    send_to_char( "You cannot use this on yourself.\n\r", ch );
	    return;
	}
	target_type = TAR_CHAR_OFFENSIVE;
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
	target_type = TAR_CHAR_DEFENSIVE;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
	{
	    send_to_char( "You cannot use this weave on another.\n\r", ch );
	    return;
	}
	target_type = TAR_CHAR_SELF;
	vo = (void *) ch;
	break;

    case TAR_CHAR_OTHER:
	if ( arg2[0] == '\0'  )
	{
	    send_to_char( "You cannot use this weave on yourself.\n\r", ch );
	    return;
	}
	if ( (victim = get_char_room( ch, arg2 )) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "You cannot use this weave on yourself.\n\r", ch );
	    return;
	}
	target_type = TAR_CHAR_OTHER;
	vo = (void *) victim;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the weave be placed upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}
	target_type = TAR_OBJ_INV;
	vo = (void *) obj;
	break;

    case TAR_OBJ_HERE:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the weave be placed upon?\n\r", ch );
	    return;
	}
	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}
	target_type = TAR_OBJ_HERE;
	break;

    case TAR_CHAR_OBJ:
	if ( arg2[0] == '\0' )
	{
	    target_type = TAR_CHAR_DEFENSIVE;
	    victim = ch;
	    vo = (void *) victim;
	    break;
	}
	if ( ( obj = get_obj_carry( ch, arg2 ) ) != NULL )
	{
	    target_type = TAR_OBJ_INV;
	    vo = (void *) obj;
	    break;
	}
	if ( ( victim = get_char_room( ch, arg2 ) ) != NULL )
	{
	    target_type = TAR_CHAR_DEFENSIVE;
	    vo = (void *) victim;
	    break;
	}
	send_to_char( "You do not see anything like that here.\n\r", ch );
	return;
	break;
    case TAR_CHAR_WORLD:
        if ( ( victim = get_char_world( ch, arg2 ) ) != NULL )
        {
            target_type = TAR_CHAR_WORLD;
            vo = (void *) victim;
            break;
        }
	send_to_char( "You could not find them anywhere.\n\r", ch );
	return;
	break;
    case TAR_ROOM:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "You must give the name of a place.\n\r", ch );
	    return;
	}
	place = get_room(ch, target_name);

	vo = (void *) place;
	target_type = TAR_ROOM;
	break;
    }

    if ( !IS_NPC(ch) )
	ch->pcdata->usage[sn]++;

    WAIT_STATE( ch, skill_table[sn].beats );

    if ( stamina_per == 3 )
	concentrate_mod = 0;
    else if ( stamina_per == 2 )
	concentrate_mod = -5;
    else if ( stamina_per == 1 )
	concentrate_mod = -10;    
    else
	concentrate_mod = -25;

    if ( number_percent() >= ch->pcdata->learned[sn] + concentrate_mod + luck / 2 )
    {
	if ( !check_skill(ch, gsn_concentration, 90 / (stamina_per + 1), TRUE) )
	{
	    send_to_char( "You lost concentration.\n\r", ch );
	    lose_stamina( ch, stamina_req / 3, FALSE, TRUE );
	    if ( ch->stamina < 0 )
		check_unconc( ch );
	    return;
	}
    }
    successful_cast = TRUE;

    if ( stamina_req >= 50 || is_in_group(skill_table[sn].name, "weather") )
	area_spell( ch, sn, stamina_req );

    strength  = power_percent( ch, sn );
    weave_result( ch, sn, strength );
            
    if ( strength < 1 )
        failure = TRUE;
    else if ( strength < 10 && number_percent() > strength * 10 )
        failure = TRUE;
                
    if ( number_range(1, 600) < UMAX(1, stamina_req / 8)
    &&   !is_protected(ch)  )
    {
        ch->pcdata->insane++;
        sprintf( log_buf, "$N now has insanity level of %d.",
            ch->pcdata->insane );
        wiznet( log_buf, ch, NULL, WIZ_INSANE, 0, 0 );
    }       
    
    if ( !IS_NPC(ch) && failure )
    {       
        send_to_char( "You fail to manage the flows, perhaps you must be a bit stronger first.\r\n", ch );
        lose_stamina( ch, stamina_req / 2, FALSE, TRUE );
	if ( ch->stamina < 0 )
	    check_unconc( ch );
	return;
    }
    else
    {
	int tn;
	lose_stamina( ch, stamina_req, FALSE, TRUE );
	show_flows( ch, sn, vo, target_type );

	if ( ch->stamina < 0 )
	    if ( check_unconc( ch ) )
		return;

	for ( tn = 0; tn < MAX_TALENT; tn++ )
	{
	    if ( talent_table[tn].name == NULL )
		continue;

	    if ( group_lookup(talent_table[tn].name) == -1 )
		continue;

	    if ( ch->pcdata->talent[tn] == FALSE )
		continue;

	    if ( is_in_group(skill_table[sn].name, talent_table[tn].name) )
	    {
		multiplier = multiplier * 125 / 100;
		if ( number_percent() == 1 )
		    act( "You seem to be especially skilled in $t.", ch,
			talent_table[tn].name, NULL, TO_CHAR );
	    }
	}

	strength = strength * multiplier / 100;
	if ( IS_IMMORTAL(ch) )
	{
	    sprintf( buf, "Power multiplier: %d%%\r\n", strength );
	    send_to_char( buf, ch );
	}
	(*skill_table[sn].spell_fun) ( sn, strength, ch, vo, target_type );
	check_improve(ch,sn,TRUE,3);
    }

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch
    &&   successful_cast )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "$N has used an offensive weave on %s",
		victim->name );
	    wiznet( log_buf, ch, NULL, WIZ_FLAGS, 0, 0 );
	}

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
    return;
}



void mob_cast( CHAR_DATA *ch, void *vo, int sn, int multiplier, int target_type )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *place;
    int luck;
    int strength;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod / 3;

    if ( !can_channel(ch, 1) )
	return;

    if ( skill_table[sn].spell_fun == spell_null )
	return;

    if ( ch->wait > 0 )
	return;

    if ( !IS_GRASPING(ch) )
	do_grasp( ch, "" );
    if ( !IS_GRASPING(ch) )
	return;

    victim	= NULL;
    place	= NULL;
    obj		= NULL;
      
    switch ( target_type )
    {
    default:
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_WORLD:
    case TAR_CHAR_OTHER:
	victim = (CHAR_DATA *) vo;
	break;

    case TAR_CHAR_SELF:
	victim = ch;
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
    case TAR_OBJ_HERE:
	obj = (OBJ_DATA *) vo;
	break;

    case TAR_ROOM:
	place = (ROOM_INDEX_DATA *) vo;
	break;
    }

    WAIT_STATE( ch, skill_table[sn].beats / 2 );
    show_flows( ch, sn, vo, target_type );

    strength  = power_percent( ch, sn );
            
    (*skill_table[sn].spell_fun) ( sn, strength, ch, vo, target_type );

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch
    &&   successful_cast )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "$N has used an offensive weave on %s",
		victim->name );
	    wiznet( log_buf, ch, NULL, WIZ_FLAGS, 0, 0 );
	}

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target_type = TAR_IGNORE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe_spell(ch,victim,FALSE) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	target_type = TAR_CHAR_OFFENSIVE;
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_OTHER:
	if ( victim == NULL )
	    victim = ch;
	target_type = TAR_CHAR_DEFENSIVE;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	target_type = TAR_CHAR_SELF;
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	target_type = TAR_OBJ_INV;
	vo = (void *) obj;
	break;

    case TAR_OBJ_HERE:
	vo = (void *) obj;
	target_type = TAR_OBJ_HERE;
	break;

    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, 100, ch, vo, target_type );
    
    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
    return;
}

bool is_vowel( char letter )
{
   switch( letter )
   {
	default:
	    return FALSE;
	case 'a':
	case 'A':
	case 'e':
	case 'E':
	case 'i':
	case 'I':
	case 'o':
	case 'O':
	case 'u':
	case 'U':
	    return TRUE;
    }
    return FALSE;
}

void do_grasp( CHAR_DATA *ch, char *argument )
{
    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( IS_GRASPING(ch) )
    {
	send_to_char_new( ch, "You are already filled with `B%s`n.\n\r",
	    SOURCE(ch) );
	return;
    }

    if ( IS_SET(ch->affected_by_2, AFF_STILL) )
    {
	send_to_char_new( ch, "You cannot touch the True Source, you are %s!\n\r", SEVER(ch) );
	return;
    }

    if ( IS_SET(ch->affected_by_2, AFF_STOP_CHANNEL)
    &&   number_percent() > 25 )
    {
	send_to_char( "You seem to have trouble touching the True Source.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->affected_by, AFF_SHIELDED) && !break_shield(ch) )
    {
	act( "You have been shielded from the True Source!", ch,
	    NULL, NULL, TO_CHAR );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_STEDDING) )
    {
	send_to_char( "You cannot touch the True Source here.\n\r", ch );
	return;
    }

    if ( TRUE_SEX(ch) == SEX_FEMALE
    &&   !IS_AFFECTED_2(ch, AFF_HIDE_CHANNEL) )
	act_channel( "$n is surrounded by a $t of $T.", ch,
	    grasp_message(ch), SOURCE(ch), TO_ROOM );
    else if ( !IS_AFFECTED_2(ch, AFF_HIDE_CHANNEL) )
	act_channel( "$n fills $mself with `B$t`n.", ch, SOURCE(ch),
	    NULL, TO_ROOM );

    if ( TRUE_SEX(ch) == SEX_MALE )
	send_to_char( "`BSaidin`n pours through you - raging fire and chilling ice.\n\r", ch ); 
    else
	send_to_char( "The sweetness of `Bsaidar`n flows through you.\n\r", ch );

    SET_BIT( ch->affected_by_2, AFF_GRASP );
    return;
}

void do_release( CHAR_DATA *ch, char *argument )
{
    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( !IS_GRASPING(ch) )
    {
	send_to_char_new( ch, "You are not holding on to `B%s`n.\n\r",
	    SOURCE(ch) );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   (ch->master != NULL
    ||    ( ch->leader != NULL && ch->leader != ch )) )
    {
	send_to_char( "You cannot release the one power when not the controller of a link.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
    {
	CHAR_DATA *gch;

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	{
	    if ( !is_same_group(ch, gch)
	    ||   !IS_GRASPING(gch)
	    ||   !IS_AFFECTED_2(gch, AFF_LINK)
	    ||   gch->leader != ch
	    ||   gch == ch )
		continue;

	    send_to_char_new( gch, "%s dissolves the link.\r\n", PERS(ch, gch) );
	    link_stamina( ch, gch, FALSE );
	    REMOVE_BIT(gch->affected_by_2, AFF_LINK);
	    do_release( gch, "" );
	}
    }

    REMOVE_BIT(ch->affected_by_2, AFF_LINK);
    if ( !IS_AFFECTED_2(ch, AFF_HIDE_CHANNEL) )
	act_channel( "The aura of `B$t`n leaves $n.", ch, SOURCE(ch),
	    NULL, TO_ROOM );
    if ( TRUE_SEX(ch) == SEX_MALE )
	send_to_char( "You release `Bsaidin`n and the raging battle with it ends.\n\r", ch ); 
    else
	send_to_char( "You reluctantly let go of `Bsaidar`n, leaving you feeling empty.\n\r", ch );
    
    if ( owns_affect(ch, TRUE) )
	send_to_char( "All of your maintained weaves fade.\n\r", ch );
    die_weave( ch );
    REMOVE_BIT( ch->affected_by_2, AFF_GRASP );
    return;
}

void do_weaves( CHAR_DATA *ch, char *argument )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		if ( paf->owner == ch )
		{
		    if ( IS_TIED(paf) )
			send_to_char( "`2", ch );
		    else
			send_to_char( "`3", ch );
		    if ( IS_INVERTED(paf) )
			send_to_char( "`B", ch );
		    else
			send_to_char( "`b", ch );

		    send_to_char_new( ch, "(%2d) ", count );

		    send_to_char_new( ch, "You have placed a weave of %s on %s",
			skill_table[paf->type].name, PERS(vch, ch) );
		    if ( IS_TIED(paf) )
			send_to_char( " and tied it off.", ch );
		    else
			send_to_char( " without tying it off.", ch );

		    if ( IS_INVERTED(paf) )
			send_to_char( "  It is inverted.", ch );
		    send_to_char( "`n\n\r", ch );
		    count++;
		    found = TRUE;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		if ( paf->owner == ch )
		{
		    if ( IS_TIED(paf) )
			send_to_char( "`2", ch );
		    else
			send_to_char( "`3", ch );
		    if ( IS_INVERTED(paf) )
			send_to_char( "`B", ch );
		    else
			send_to_char( "`b", ch );

		    send_to_char_new( ch, "(%2d) ", count );

		    send_to_char_new( ch, "You have placed a weave of %s on %s",
			skill_table[paf->type].name, vobj->short_descr );
		    if ( IS_TIED(paf) )
			send_to_char( " and tied it off.", ch );
		    else
			send_to_char( " without tying it off.", ch );

		    if ( IS_INVERTED(paf) )
			send_to_char( "  It is inverted.", ch );
		    send_to_char( "`n\n\r", ch );
		    count++;
		    found = TRUE;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		if ( paf->owner == ch )
		{
		    if ( IS_TIED(paf) )
			send_to_char( "`2", ch );
		    else
			send_to_char( "`3", ch );
		    if ( IS_INVERTED(paf) )
			send_to_char( "`B", ch );
		    else
			send_to_char( "`b", ch );

		    send_to_char_new( ch, "(%2d) ", count );

		    send_to_char_new( ch, "You have placed a weave of %s in %s",
			skill_table[paf->type].name, vroom->name );
		    if ( IS_TIED(paf) )
			send_to_char( " and tied it off.", ch );
		    else
			send_to_char( " without tying it off.", ch );

		    if ( IS_INVERTED(paf) )
			send_to_char( "  It is inverted.", ch );
		    send_to_char( "`n\n\r", ch );

		    count++;
		    found = TRUE;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;
		if ( IS_SET(vobj->extra_flags, ITEM_TIED) )
		    send_to_char( "`2", ch );
		else
		    send_to_char( "`3", ch );

		sn = affect_lookup( vobj->pIndexData->vnum );

		send_to_char_new( ch, "(%2d) ", count );

		if ( vobj->in_room != NULL )
		{
		    send_to_char_new( ch, "You have %s in %s",
			vobj->short_descr, vobj->in_room->name );
		}
		else if ( vobj->carried_by != NULL )
		{
		    send_to_char_new( ch, "You have %s on %s",
			vobj->short_descr, PERS(vobj->carried_by, ch) );
		}
		else
		{
		    send_to_char_new( ch, "You have %s",
			vobj->short_descr );
		}

		if ( IS_SET(vobj->extra_flags, ITEM_TIED) )
		    send_to_char( " and tied it off.", ch );
		else
		    send_to_char( " without tying it off.", ch );

		send_to_char( "`n\n\r", ch );
		count++;
		found = TRUE;
	    }
	}


    }
    if ( !found )
	send_to_char( "You do not have any permanent weaves at the moment.\n\r", ch );
    return;
}

bool owns_affect( CHAR_DATA *ch, bool fTied )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;

    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		if ( fTied && IS_TIED(paf) )
		    continue;
		if ( paf->owner == ch )
		    return TRUE;
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		if ( fTied && IS_TIED(paf) )
		    continue;
		if ( paf->owner == ch )
		    return TRUE;
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		if ( fTied && IS_TIED(paf) )
		    continue;
		if ( paf->owner == ch )
		    return TRUE;
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( fTied && IS_SET(vobj->extra_flags, ITEM_TIED) )
		continue;
	    if ( vobj->owner == ch )
		return TRUE;
	}
    }
    return FALSE;
}
/*
void weave_number( CHAR_DATA *ch, int an, sh_int type )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;
    int diff;
    int sn;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    diff	= 15;
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( type == DO_TIE
			&&   vch->in_room == ch->in_room )
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			if ( type == DO_UNTIE
			&&        vch->in_room == ch->in_room )
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED);
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != NULL && paf->owner != ch )
					act( "You feel your $t weave on $N being taken apart.",
					    paf->owner, skill_table[sn].name,
					   vch, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    if ( paf->type > 0 && skill_table[sn].msg_off )
				    {
					send_to_char( skill_table[sn].msg_off, vch );
					send_to_char( "\n\r", vch );
				    }
				    affect_remove( vch, paf );
				}
			    }
			if ( type == DO_CANCEL )
			{
			    if ( !IS_TIED(paf)
			    ||   ch->in_room == vch->in_room )
				if ( cancel_weave(ch, paf) )
				{
				    if ( paf->type > 0 && skill_table[sn].msg_off )
				    {
				        send_to_char( skill_table[sn].msg_off, vch );
				        send_to_char( "\n\r", vch );   
				    }
				    affect_remove( vch, paf );
				}
			}
			if ( type == DO_INVERT
			&&        vch->in_room == ch->in_room )
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    diff	= 15;
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( type == DO_TIE
			&&   (ch->in_room == vobj->in_room
			||    vobj->carried_by == ch) )
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			if ( type == DO_UNTIE
			&&        (ch->in_room == vobj->in_room
			||         vobj->carried_by == ch) )
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED );
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != NULL )
					act( "You feel your $t weave on $P being taken apart.",
					    paf->owner, skill_table[sn].name,
					    vobj, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    affect_obj_remove( vobj, paf );
				}
			    }
			if ( type == DO_CANCEL )
			{
			    if ( !IS_TIED(paf)
			    ||   ch->in_room == vobj->in_room
			    ||   vobj->carried_by == ch )
				if ( cancel_weave(ch, paf, diff) )
				{
				    affect_obj_remove( vobj, paf );
				}
			}
			if ( type == DO_INVERT
			&&        (ch->in_room == vobj->in_room
			||         vobj->carried_by == ch) )
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    diff	= 10;
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( type == DO_TIE
			&&   vroom == ch->in_room )
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			if ( type == DO_UNTIE
			&&        vroom == ch->in_room )
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED);
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != ch )
					act( "You feel your $t weave in $T being taken apart.",
					    paf->owner, skill_table[sn].name,
					    vroom->name, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    affect_room_remove( vroom, paf );
				}
			    }
			if ( type == DO_CANCEL )
			{
			    if ( !IS_TIED(paf)
			    ||   ch->in_room == vroom )
				if ( cancel_weave(ch, paf, diff) )
				{
				    affect_room_remove( vroom, paf );
				}
			}
			if ( type == DO_INVERT
			&&        vroom == ch->in_room )
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;

		sn = affect_lookup( vobj->pIndexData->vnum );

		if ( count == an )
		{
		    if ( type == DO_TIE
		    &&   (vobj->in_room == ch->in_room
		    ||    vobj->carried_by == ch
		    ||    ( vobj->carried_by != NULL
		    &&      vobj->carried_by->in_room == ch->in_room )) )
		    {
			send_to_char( "You tie the weave off.\n\r", ch );
			SET_BIT(vobj->extra_flags, ITEM_TIED);
		    }
		    if ( type == DO_UNTIE
		    &&   (vobj->in_room == ch->in_room
		    ||    vobj->carried_by == ch
		    ||    ( vobj->carried_by != NULL
		    &&      vobj->carried_by->in_room == ch->in_room )) )
		    {
			if ( IS_GRASPING(ch) )
			{
			    REMOVE_BIT(vobj->extra_flags, ITEM_TIED);
			    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
			}
			else
			{
			    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
			    extract_obj( vobj );
			}
		    }
		    if ( type == DO_CANCEL )
		    {
			if ( !IS_SET(vobj->extra_flags, ITEM_TIED)
			||   vobj->in_room == ch->in_room
			||   vobj->carried_by == ch
			||   (vobj->carried_by != NULL
			&&    vobj->carried_by->in_room == ch->in_room) )
			{
			    extract_obj( vobj );
			}
		    }
		    return;
		}
		count++;
	    }
	}
    }
    if ( !found )
	send_to_char( "You are not maintaining that many weaves.\n\r", ch );
    return;
}
*/

void tie_number( CHAR_DATA *ch, int an )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;
    int diff;
    int sn;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    diff	= 15;
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vch->in_room == ch->in_room )
			{
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			}
			else
			    send_to_char( "You must be in the same room to tie off a weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    diff	= 15;
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( ch->in_room == vobj->in_room
			||   vobj->carried_by == ch )
			{
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			}
			else
			    send_to_char( "You must be in the same room or holding the object to tie off a weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    diff	= 10;
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vroom == ch->in_room )
			{
			    if ( tie_weave(ch, paf, diff) )
			    {
				send_to_char( "You tie the weave off.\n\r", ch );
				SET_BIT(paf->flags, AFFECT_TIED);
			    }
			}
			else
			    send_to_char( "You must be in the same room to tie off a weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;

		sn = affect_lookup( vobj->pIndexData->vnum );

		if ( count == an )
		{
		    if ( vobj->in_room == ch->in_room
		    ||   vobj->carried_by == ch
		    ||   (vobj->carried_by != NULL
		    &&    vobj->carried_by->in_room == ch->in_room) )
		    {
			send_to_char( "You tie the weave off.\n\r", ch );
			SET_BIT(vobj->extra_flags, ITEM_TIED);
		    }
		    else
			send_to_char( "You must be in the same room or holding the object to tie this weave.\n\r", ch );
		    return;
		}
		count++;
	    }
	}
    }
    if ( !found )
	send_to_char( "You are not maintaining that many weaves.\n\r", ch );

}


void untie_number( CHAR_DATA *ch, int an )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;
    int diff;
    int sn;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    diff	= 15;
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vch->in_room == ch->in_room )
			{
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED);
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != NULL && paf->owner != ch )
					act( "You feel your $t weave on $N being taken apart.",
					    paf->owner, skill_table[sn].name,
					   vch, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    if ( paf->type > 0 && skill_table[sn].msg_off )
				    {
					send_to_char( skill_table[sn].msg_off, vch );
					send_to_char( "\n\r", vch );
				    }
				    affect_remove( vch, paf );
				}
			    }
			}
			else
			    send_to_char( "You must be in the same room to untie this weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    diff	= 15;
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( ch->in_room == vobj->in_room
			||   vobj->carried_by == ch )
			{
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED );
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != NULL )
					act( "You feel your $t weave on $P being taken apart.",
					    paf->owner, skill_table[sn].name,
					    vobj, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    affect_obj_remove( vobj, paf );
				}
			    }
			}
			else
			    send_to_char( "You must be in the same room, or holding the object, to untie this weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    diff	= 10;
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vroom == ch->in_room )
			{
			    if ( untie_weave(ch, paf, diff) )
			    {
				if ( paf->owner == ch && IS_GRASPING(ch) )
				{
				    REMOVE_BIT(paf->flags, AFFECT_TIED);
				    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
				}
				else
				{
				    if ( paf->owner != ch )
					act( "You feel your $t weave in $T being taken apart.",
					    paf->owner, skill_table[sn].name,
					    vroom->name, TO_CHAR );
				    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
				    affect_room_remove( vroom, paf );
				}
			    }
			}
			else
			    send_to_char( "You must be in the same room to untie this weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;

		sn = affect_lookup( vobj->pIndexData->vnum );

		if ( count == an )
		{
		    if ( vobj->in_room == ch->in_room
		    ||   vobj->carried_by == ch
		    ||   (vobj->carried_by != NULL
		    &&    vobj->carried_by->in_room == ch->in_room) )
		    {
			if ( IS_GRASPING(ch) )
			{
			    REMOVE_BIT(vobj->extra_flags, ITEM_TIED);
			    send_to_char( "You untie the weave, and maintain it again.\n\r", ch );
			}
			else
			{
			    send_to_char( "You untie the weave, causing it to unravel and fade.\n\r", ch );
			    extract_obj( vobj );
			}
		    }
		    else
			send_to_char( "You must be in the same room as the object to untie it.\n\r", ch );
		    return;
		}
		count++;
	    }
	}
    }
    if ( !found )
	send_to_char( "You are not maintaining that many weaves.\n\r", ch );
    return;
}


void invert_number( CHAR_DATA *ch, int an )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;
    int diff;
    int sn;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    diff	= 15;
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vch->in_room == ch->in_room )
			{
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			}
			else
			    send_to_char( "You mustbe in the same room to invert weaves.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    diff	= 15;
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( ch->in_room == vobj->in_room
			||   vobj->carried_by == ch )
			{
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			}
			else
			    send_to_char( "You must be in the same room, or holding the object, to invert this weave.\n\r",ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    diff	= 10;
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( vroom == ch->in_room )
			{
			    if ( invert_weave(ch, paf, diff) )
			    {
				send_to_char( "You invert the flows of the weave.\n\r", ch );
				TOGGLE_BIT(paf->flags, AFFECT_INVERT);
			    }
			}
			else
			    send_to_char( "You must be in the same room to invert this weave.\n\r", ch );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;

		sn = affect_lookup( vobj->pIndexData->vnum );

		if ( count == an )
		{
		    send_to_char( "You cannot invert this kind of weave.\n\r", ch );
		    return;
		}
		count++;
	    }
	}
    }
    if ( !found )
	send_to_char( "You are not maintaining that many weaves.\n\r", ch );
    return;
}


void cancel_number( CHAR_DATA *ch, int an )
{
    NODE_DATA		*node;
    CHAR_DATA		*vch;
    OBJ_DATA		*vobj;
    ROOM_INDEX_DATA	*vroom;
    AFFECT_DATA		*paf;
    bool		found;
    int count	=	0;
    int diff;
    int sn;

    found = FALSE;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    diff	= 15;
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( IS_TIED(paf) )
			    untie_number( ch, an );
			if ( IS_TIED(paf) )
			{
			    send_to_char( "You could not cancel this weave.\n\r", ch );
			    return;
			}
			if ( cancel_weave(ch, paf) )
			{
			    if ( paf->type > 0 && skill_table[sn].msg_off )
			    {
				send_to_char( skill_table[sn].msg_off, vch );
				send_to_char( "\n\r", vch );   
			    }
			    affect_remove( vch, paf );
			}
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_OBJ )
	{
	    diff	= 15;
	    vobj	= (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( IS_TIED(paf) )
			    untie_number( ch, an );
			if ( IS_TIED(paf) )
			{
			    send_to_char( "You could not cancel this weave.\n\r", ch );
			    return;
			}
			if ( cancel_weave(ch, paf) )
			    affect_obj_remove( vobj, paf );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    diff	= 10;
	    vroom	= (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf->next )
	    {
		sn = paf->type;
		if ( paf->owner == ch )
		{
		    if ( count == an )
		    {
			if ( IS_TIED(paf) )
			    untie_number( ch, an );
			if ( IS_TIED(paf) )
			{
			    send_to_char( "You could not cancel this weave.\n\r", ch );
			    return;
			}
			if ( cancel_weave(ch, paf) )
			    affect_room_remove( vroom, paf );
			return;
		    }
		    count++;
		}
	    }
	}

	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj	= (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch )
	    {
		int sn;

		sn = affect_lookup( vobj->pIndexData->vnum );

		if ( count == an )
		{
		    if ( IS_SET(vobj->extra_flags, ITEM_TIED) )
			untie_number( ch, an );
		    if ( IS_SET(vobj->extra_flags, ITEM_TIED) )
		    {
			send_to_char( "You could not cancel this weave.\n\r", ch );
			return;
		    }
		    extract_obj( vobj );
		    return;
		}
		count++;
	    }
	}
    }
    if ( !found )
	send_to_char( "You are not maintaining that many weaves.\n\r", ch );
    return;
}



void insane( CHAR_DATA *ch, int total )
{
    const char *	insane_thought[] =
    {
    "The world grows fuzzy as the voices in your head clammer for attention.",
    "KILL ... KILL ... KILL ... KILL ...",
    "Help?  Mother .. please .. help me ..",
    "A defeaning silences fills you.",
    "Stars beginning spinning around you, dancing.",
    "You feel a little dizzy.",
    "You have a slight headache.",
    "You feel like someone is watching you.",
    "You swear you just heard something.",
    "You hear a strange voice in your head.",
    "You hear a strange voice, only for a moment, in your head.",
    "A strange voice speaks in your head, 'They are after you.'",
    "You must kill the Aes Sedai .. you must .. to save yourself ..",
    "Look around .. they all want to kill you, you know .. they want to SEE YOU DIE!",
    NULL
    };
    char *		insane_say[] =
    {
    "Get it off me!  Get it OFF!",
    "I am well.  Thank you for asking.",
    "HHHHEEEEELLLPPP!!?!!",
    "Who am I?  Who are you?  Who is anyone?!?",
    "Mother?  Leave me alone, mother - please .. .. please .. ?",
    NULL
    };
    char *		insane_emote[] =
    {
    NULL
    };

    int sn;
    CHAR_DATA *victim;
    void *vo;
    int stat;
    int roll;
    int i;
    int thought = 0, total_thought = 0;
    int say = 0, total_say = 0;
    int emote = 0, total_emote = 0;

    if ( IS_NPC(ch) )
	return;

    stat = number_range( 0, 7 );
    while ( ch->perm_stat[stat] < 3 )
	stat = number_range( 0, 7 );

    sprintf( log_buf, "$N just went bonkers." );
    wiznet( log_buf, ch, NULL, WIZ_INSANE, 0, 0 );

    for ( i = 0; insane_thought[i]; i++ )
	total_thought++;

    for ( i = 0; insane_say[i]; i++ )
	total_say++;

    for ( i = 0; insane_emote[i]; i++ )
	total_emote++;

    if ( total_thought )
	thought = number_range( 0, total_thought - 1 );
    if ( total_say )
	say = number_range( 0, total_say - 1 );
    if ( total_emote )
	emote = number_range( 0, total_emote - 1 );

    sn = number_range( 1, MAX_SKILL-1 );
    while ( skill_table[sn].name != NULL && skill_table[sn].spell_fun != spell_null )
	sn = number_range( 1, MAX_SKILL-1 );

    victim = random_room_char( ch->in_room );

    if ( victim == ch )
	victim = NULL;

    if ( total_say && number_bits(2) == 0 )
	do_say( ch, insane_say[say] );

    if ( total_emote && number_bits(2) == 0 )
	do_emote( ch, insane_emote[emote] );

    if ( total_thought && number_bits(2) == 0 )
    {
	send_to_char( insane_thought[thought], ch );
	send_to_char( "\n\r", ch );
    }

    roll = number_percent() + total;
    if ( roll >= 200 ) /* balefire self */
    {
	/* 1/100 chance ONLY if insanity = 100. REALLY REALLY bad
	   Doesn't work if they're gentled */

	if ( !can_channel(ch, 1) )
	    return;
	if ( !IS_GRASPING(ch) )
	    do_grasp( ch, "" );
	if ( !IS_GRASPING(ch) )
	    return;

	do_emote( ch, "cackles and raises his arms to the sky!" );
	vo = (void *) ch;
	mob_cast( ch, vo, gsn_balefire, 200,
	    skill_table[gsn_balefire].target );
    }
    else if ( roll >= 195 ) /* lose permanent 1 hp, 1 stamina */
    {
	send_to_char( "Death and decay creep into you.\n\r", ch );
	ch->max_hit--;
	ch->pcdata->perm_hit--;
	ch->max_stamina--;
	ch->pcdata->perm_stamina--;
    }
    else if ( roll >= 190 ) /* lose 1 stat */
    {
	send_to_char( "Your body rots away.\n\r", ch );
	ch->perm_stat[stat] -= 1;
	if ( ch->perm_stat[stat] < 3 )
	    ch->perm_stat[stat] = 3;
    }
    else if ( roll >= 180 ) /* random spells */
    {
	act( "$n cackle$% and raise$% $o hands toward the sky!", ch,
	    NULL, NULL, TO_ALL );
	send_to_char( "The One Power surges within you!\n\r", ch );
	switch( skill_table[sn].target )
	{
	    default:
		bug( "insane: bad target for sn %d.", sn );
		return;
	    case TAR_IGNORE:
	    case TAR_CHAR_OFFENSIVE:
	    case TAR_CHAR_DEFENSIVE:
		break;

	    case TAR_CHAR_SELF:
		victim = ch;
		break;

	    case TAR_CHAR_OTHER:
	    case TAR_OBJ_INV:
	    case TAR_OBJ_HERE:
	    case TAR_CHAR_OBJ:
	    case TAR_ROOM:
	    case TAR_CHAR_WORLD:
		return;
	}
	vo = (void *) victim;
	mob_cast( ch, vo, sn, 100, skill_table[sn].target );
    }
    else if ( roll >= 165 ) /* attack someone */
    {
	if ( victim == NULL )
	    victim = ch;

	send_to_char( "The voices in your head tell you to kill!\n\r", ch );
	if ( victim != ch )
	    act( "$n suddenly attack$% $N!", ch, NULL, victim, TO_ALL );
	else
	{
	    act( "$n suddenly hits $mself!", ch, NULL, victim, TO_ROOM );
	    act( "You hit yourself!", ch, NULL, victim, TO_CHAR );
	}
	multi_hit( ch, victim, TYPE_UNDEFINED );
    }
    else if ( roll >= 90 ) /* flee */
    {
	int attempt;
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;

	was_in = ch->in_room;
	for ( attempt = 0; attempt < 6; attempt++ )
	{
	    EXIT_DATA *pexit;
	    int door;

	    door = number_door( );

	    if ( (pexit = was_in->exit[door]) == 0
	    ||   pexit->u1.to_room == NULL
	    ||   IS_SET(pexit->exit_info, EX_CLOSED)
	    ||  (ch->stamina <= 0 && number_bits( 4 )) )
		continue;

	    move_char( ch, door, FALSE );
	    if ( (now_in = ch->in_room) == was_in )
		continue;

	    ch->in_room = was_in; 
	    act( "$n run$% from the room screaming!", ch, NULL, NULL,
		TO_ALL );
	    ch->in_room = now_in;
        
	    if ( ch->pet != NULL )
		do_flee( ch->pet, "" );
        
	    if ( ch->fighting )
		stop_fighting( ch, TRUE );
	}
    }
    else if ( roll >= 75 ) /* pass out */
    {
    }
    else if ( roll >= 70 ) /* grasp or release One Power */
    {
	if ( IS_GRASPING(ch) )
	    do_release( ch, "" );
	else
	    do_grasp( ch, "" );
    }
    else if ( roll >= 50 ) /* confusion */
    {
    }

    WAIT_STATE(ch, 1);
    return;
}

void recurse_room( ROOM_INDEX_DATA* pRoom, CHAR_DATA *ch, int power, int depth )
{
    CHAR_DATA *vch;
    int door;
    bool newRoom = TRUE;
    NODE_DATA *list;

    --depth;
    for ( list = room_list; list; list = list->next )
    {
	ROOM_INDEX_DATA *room;

	if ( list->data_type != NODE_ROOM )
	    continue;

	room = (ROOM_INDEX_DATA *)list->data;
	if ( room == pRoom )
	{
	    newRoom = FALSE;
	    break;
	}
    }

    if ( newRoom )
    {
	for ( vch = pRoom->people; vch != NULL; vch = vch->next_in_room )
	{
	    if ( TRUE_SEX(vch) != TRUE_SEX(ch) )
		continue;

	    if ( !can_channel(vch, 1) )
		continue;

	    if ( channel_strength(vch, POWER_ALL) <= 25 )
		continue;

	    if ( IS_WRITING(vch) )
		continue;

	    if ( ch == vch )
		continue;

	    if ( power >= 160 )
		send_to_char( "You sense someone channeling immense amounts of the One Power.\n\r", vch );
	    else if ( power >= 120 )
		send_to_char( "You sense someone channeling great amounts of the One Power.\n\r", vch );
	    else if ( power >= 90 )
		send_to_char( "You sense someone channeling the One Power.\n\r", vch );
	    else
		send_to_char( "You sense someone channeling faint amounts of the One Power.\n\r", vch );
	}
	add_room_list( pRoom );
    }

    if ( depth == 0 )
	return;

    for ( door = 0; door < 6; door++ )
    {
	if ( pRoom->exit[door] == NULL )
	    continue;

	if ( pRoom->exit[door]->u1.to_room == NULL )
	    continue;

	recurse_room( pRoom->exit[door]->u1.to_room, ch, power, depth );
    }
    return;
}
	
void area_spell( CHAR_DATA *ch, int sn, int power )
{
    int door, depth;

    return;
    depth = UMAX( 1, (power - 50) / 15 );

    for ( door = 0; door < 6; door++ )
    {
	if ( ch->in_room->exit[door] == NULL )
	    continue;

	if ( ch->in_room->exit[door]->u1.to_room == NULL )
	    continue;

	recurse_room( ch->in_room->exit[door]->u1.to_room, ch, power, depth );
    }
    destroy_list( room_list );
    return;
}

int weave_use( int sn )
{
    int flows = 0;

    if ( skill_table[sn].power[0] > 0 )
	flows += 1;

    if ( skill_table[sn].power[1] > 0 )
	flows += 2;

    if ( skill_table[sn].power[2] > 0 )
	flows += 4;

    if ( skill_table[sn].power[3] > 0 )
	flows += 8;

    if ( skill_table[sn].power[4] > 0 )
	flows += 16;

    return flows;
}

char *grasp_message( CHAR_DATA *ch )
{
    int total;

    total = channel_strength( ch, POWER_ALL ) * 5;

    if ( TRUE_SEX(ch) == SEX_FEMALE )
    {
	if ( total >= 451 )
	    return "blinding light";
	else if ( total >= 401 )
	    return "shining nimbus";
	else if ( total >= 351 )
	    return "blazing glow";
	else if ( total >= 301 )
	    return "strong glow";
	else if ( total >= 251 )
	    return "bright glow";
	else if ( total >= 201 )
	    return "glow";
	else if ( total >= 151 )
	    return "slight glow";
	else if ( total >= 101 )
	    return "weak glow";
	else if ( total >= 51 )
	    return "dim glow";
	else 
	    return "faint aura";
    }
    else
	return "aura";
    return "";
}

void remove_shape( CHAR_DATA *ch )
{
    affect_strip( ch, gsn_shape_change );
    affect_strip( ch, gsn_disguise );
    REMOVE_BIT( ch->affected_by, AFF_SHAPE_CHANGE );

    if ( !IS_NPC(ch) )
    {
	free_string( ch->pcdata->new_name );
	free_string( ch->pcdata->new_last );
	free_string( ch->pcdata->new_desc );
	free_string( ch->pcdata->new_title );
	ch->pcdata->new_name = NULL;
	ch->pcdata->new_last = NULL;
	ch->pcdata->new_desc = NULL;
	ch->pcdata->new_title = NULL;
    }
    return;
}

bool check_power( CHAR_DATA *ch, int power )
{
    int value = 0;
    int max = 0;
    int percent = 0;

    if ( IS_NPC(ch) )
    {
	if ( !IS_SET(ch->act, ACT_CHANNELER) )
	    return FALSE;

	if ( power & POWER_EARTH )
	{
	    max = ch->channel_max[0];
	    value = ch->channel_skill[0];
	}

	if ( power & POWER_AIR )
	{
	    max = ch->channel_max[1];
	    value = ch->channel_skill[1];
	}

	if ( power & POWER_FIRE )
	{
	    max = ch->channel_max[2];
	    value = ch->channel_skill[2];
	}

	if ( power & POWER_WATER )
	{
	    max = ch->channel_max[3];
	    value = ch->channel_skill[3];
	}

	if ( power & POWER_SPIRIT )
	{
	    max = ch->channel_max[4];
	    value = ch->channel_skill[4];
	}
    }
    else
    {
	if ( power & POWER_EARTH )
	{
	    max = ch->channel_max[0];
	    value = ch->pcdata->learned[gsn_earth];
	}

	if ( power & POWER_AIR )
	{
	    max = ch->channel_max[1];
	    value = ch->pcdata->learned[gsn_air];
	}

	if ( power & POWER_FIRE )
	{
	    max = ch->channel_max[2];
	    value = ch->pcdata->learned[gsn_fire];
	}

	if ( power & POWER_WATER )
	{
	    max = ch->channel_max[3];
	    value = ch->pcdata->learned[gsn_water];
	}

	if ( power & POWER_SPIRIT )
	{
	    max = ch->channel_max[4];
	    value = ch->pcdata->learned[gsn_spirit];
	}
    }

    if ( value == 0 || max == 0 )
	return FALSE;

    percent = value * 100 / max;

    if ( number_percent() > percent )
	return FALSE;

    return TRUE;
}


int exp_strength( int number )
{
    int i, value = 0;

    for ( i = 0; i < number; i++ )
    {
	value++;

	if ( i > 30 )
	    value++;

	if ( i > 40 )
	    value++;

	if ( i > 60 )
	    value++;

	if ( i > 90 )
	    value++;
    }
    return value;
};
