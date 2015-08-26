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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_open		);
DECLARE_DO_FUN(do_close		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_grasp		);
DECLARE_DO_FUN(do_release	);


void show_flows( CHAR_DATA *ch, int sn, void *vo, int target_type );


/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_junker		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(	spec_puff		);
DECLARE_SPEC_FUN(	spec_damane		);
DECLARE_SPEC_FUN(	spec_wise_one		);
DECLARE_SPEC_FUN(	spec_omrah		);

/* Object special functions */
DECLARE_SPEC_OBJ(	spec_chaos_weapon	);
DECLARE_SPEC_OBJ(	spec_hot_tub		);

/* Use special functions */
DECLARE_USE_FUN(	use_glowing_orb		);
DECLARE_USE_FUN(	use_envenom		);
DECLARE_USE_FUN(	use_baby_oil		);
DECLARE_USE_FUN(	use_tree		);

/*
 * Special Functions Table.     OLC
 */
const   struct  spec_type       spec_table      [ ] =
{
    /*
     * Special function commands.
     */
    { "spec_breath_any",        spec_breath_any         },
    { "spec_breath_acid",       spec_breath_acid        },
    { "spec_breath_fire",       spec_breath_fire        },
    { "spec_breath_frost",      spec_breath_frost       },
    { "spec_breath_gas",        spec_breath_gas         },
    { "spec_breath_lightning",  spec_breath_lightning   },
    { "spec_cast_adept",        spec_cast_adept         },
    { "spec_cast_cleric",       spec_cast_cleric        },
    { "spec_cast_judge",        spec_cast_judge         },
    { "spec_cast_mage",         spec_cast_mage          },
    { "spec_cast_undead",       spec_cast_undead        },
    { "spec_executioner",       spec_executioner        },
    { "spec_fido",              spec_fido               },
    { "spec_guard",             spec_guard              },
    { "spec_janitor",           spec_janitor            },
    { "spec_junker",            spec_junker             },
    { "spec_mayor",             spec_mayor              },
    { "spec_poison",            spec_poison             },
    { "spec_thief",             spec_thief              },
    { "spec_puff",              spec_puff               },      /* ROM OLC */
    { "spec_damane",		spec_damane		},
    { "spec_wise_one",		spec_wise_one		},
    { "spec_omrah",		spec_omrah		},

    /*
     * End of list.
     */
    { "",                       0       }
};



/*****************************************************************************
 Name:          spec_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char *spec_string( SPEC_FUN *fun )      /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_table[cmd].spec_fun != NULL; cmd++ )
        if ( fun == spec_table[cmd].spec_fun )
            return spec_table[cmd].spec_name;

    return 0;
}



/*****************************************************************************
 Name:          spec_lookup
 Purpose:       Given a name, return the appropriate spec fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN *spec_lookup( const char *name )       /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_table[cmd].spec_name != NULL; cmd++ )
        if ( !str_cmp( name, spec_table[cmd].spec_name ) )
            return spec_table[cmd].spec_fun;

    return 0;
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TAR_IGNORE );
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL, TAR_IGNORE );
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    if ( is_affected(ch, skill_lookup( "shield from true source" )) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 
	     && !IS_NPC(victim) && victim->level < 11)
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits( 4 ) )
    {
    case 0:
	act( "$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    case 1:
	act( "$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    case 2:
	act( "$n utters the word 'judicandus noselacri'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    case 3:
	act( "$n utters the word 'judicandus dies'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    case 4:
	act( "$n utters the words 'judicandus sausabru'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    case 5:
	act( "$n utters the words 'candusima'.", ch, NULL, NULL, TO_ROOM );
	return TRUE;

    }

    return FALSE;
}


bool spec_damane( CHAR_DATA *ch )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
	return FALSE;

    if ( ch->fighting == NULL )
    {				/* Not fighting, so scan room */
	CHAR_DATA *vch, *vch_next;
	int asn;

	if ( IS_GRASPING(ch) )
	    do_release( ch, "" );

	asn = skill_lookup( "air shield" );

	if ( !is_affected(ch, asn) )
	    mob_cast( ch, (void *) ch, asn, 125, TAR_CHAR_DEFENSIVE );

	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next_in_room;

	    if ( ch == vch || IS_NPC(vch) )
		continue;

	    if ( can_channel(vch, 1)
	    &&   TRUE_SEX(ch) == TRUE_SEX(vch)
	    &&   !IS_IMMORTAL(vch) )
	    {
		act( "The woman in the gray dress points at $N and screams 'Marath'damane!'",
		    ch, NULL, vch, TO_ROOM );
		mob_cast( ch, (void *) vch, gsn_shield, 150, TAR_CHAR_DEFENSIVE );
		multi_hit( ch, vch, TYPE_UNDEFINED );
		return TRUE;
	    }
	}
    }
    else
    {
	/* Combat spells :) */
	CHAR_DATA *vch;
	int sn;

	vch = ch->fighting;

	switch( number_bits(3) )
	{
	    default:
	    case 0:
		sn = skill_lookup( "fireball" );
		break;
	    case 1:
		if ( can_channel(vch, 1) )
		    sn = gsn_shield;
		else
		    sn = skill_lookup( "slow" );
		break;
	    case 2:
		sn = skill_lookup( "stone strike" );
		break;
	    case 3:
		sn = skill_lookup( "lash" );
		break;
	    case 4:
		sn = skill_lookup( "lightning bolt" );
	    case 5:
	    case 6:
		sn = skill_lookup( "wind mace" );
		break;
	    case 7:
		sn = skill_lookup( "wind spear" );
		break;
	}
	if ( sn != -1 || vch == NULL )
	    mob_cast( ch, (void *) vch, sn, 100, skill_table[sn].target );
	return TRUE;
    }
    return TRUE;
}


bool spec_wise_one( CHAR_DATA *ch )
{
    if ( ch->fighting == NULL )
    {				/* Not fighting, so scan room */
	int asn;

	if ( IS_GRASPING(ch) )
	    do_release( ch, "" );

	asn = skill_lookup( "air shield" );

	if ( !is_affected(ch, asn) )
	    mob_cast( ch, (void *) ch, asn, 125, TAR_CHAR_DEFENSIVE );
    }
    else
    {
	/* Combat spells :) */
	CHAR_DATA *vch;
	int sn;

	vch = ch->fighting;

	switch( number_bits(3) )
	{
	    default:
	    case 0:
		sn = skill_lookup( "fireball" );
		break;
	    case 1:
		if ( can_channel(vch, 1) )
		    sn = gsn_shield;
		else
		    sn = skill_lookup( "slow" );
		break;
	    case 2:
		sn = skill_lookup( "stone strike" );
		break;
	    case 3:
		sn = skill_lookup( "lash" );
		break;
	    case 4:
		sn = skill_lookup( "lightning bolt" );
	    case 5:
	    case 6:
		sn = skill_lookup( "wind mace" );
		break;
	    case 7:
		sn = skill_lookup( "wind spear" );
		break;
	}
	if ( sn != -1 || vch == NULL )
	    mob_cast( ch, (void *) vch, sn, 100, skill_table[sn].target );
	return TRUE;
    }
    return TRUE;
}

bool spec_omrah( CHAR_DATA *ch )
{
    CHAR_DATA *vch;

    if ( !IS_SET(ch->act, ACT_IS_HEALER) )
	return FALSE;

    if ( number_bits(4) > 4 )
	return FALSE; 

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( !IS_NPC(vch) && vch->level <= 10 )
	{
	    if ( vch->hit < vch->max_hit / 2 )
		break;

	    if ( IS_AFFECTED(vch, AFF_BLIND)
	    ||   is_affected(vch, gsn_blindness) )
		break;

	    if ( vch->body )
		break;

	    if ( IS_AFFECTED(vch, AFF_PLAGUE)
	    ||   is_affected(vch, gsn_plague) )
		break;

	    if ( IS_AFFECTED(vch, AFF_POISON)
	    ||   is_affected(vch, gsn_poison) )
		break;

	    if ( ch->stamina < ch->max_stamina / 2 )
		break;
	}
    }

    if ( !vch )
	return FALSE;

    if ( vch->hit < vch->max_hit / 2 )
    {
	act( "$N looks $n over and quickly administers a foul-smelling, green liquid to $m.",
	    vch, NULL, ch, TO_ALL );
	gain_health( vch, dice(1, 10), FALSE );
	return TRUE;
    }

    if ( IS_AFFECTED(vch, AFF_BLIND)
    ||   is_affected(vch, gsn_blindness) )
    {
	act( "$N squints into $o eyes, nodding, then pours some water into them.",
	    vch, NULL, ch, TO_ALL );
	cure_condition( vch, BODY_BLIND, 100 );
	return TRUE;
    }

    if ( vch->body )
    {
	if ( IS_SET(vch->body, BODY_BLEEDING) )
	{
	    act( "$N carefully bandage$% $o cuts.", vch, NULL, ch, TO_ALL );
	    cure_condition( vch, BODY_BLEEDING, 100 );
	    return TRUE;
	}
	act( "$N carefully set$^ $o broken limbs.", vch, NULL, ch, TO_ALL );
	cure_condition( vch, BODY_RIGHT_LEG, 100 );
	cure_condition( vch, BODY_LEFT_LEG, 100 );
	cure_condition( vch, BODY_RIGHT_ARM, 100 );
	cure_condition( vch, BODY_LEFT_ARM, 100 );
	return TRUE;
    }

    if ( IS_AFFECTED(vch, AFF_PLAGUE)
    ||   is_affected(vch, gsn_plague) )
    {
	act( "$N pulls out $o tongue, then forces some strange pills down $s throat.",
	    vch, NULL, ch, TO_ALL );
	cure_condition( vch, BODY_DISEASE, 100 );
	return TRUE;
    }

    if ( IS_AFFECTED(vch, AFF_POISON)
    ||   is_affected(vch, gsn_poison) )
    {
	act( "$N hands some strange pills to $n, then forces $m to swallow them.",
	   vch, NULL, ch, TO_ALL );
	cure_condition( vch, BODY_POISON, 100 );
	return TRUE;
    }

    if ( ch->stamina < ch->max_stamina / 2 )
    {
	act( "$N knocks $n on the head, then forces $m to rest.", vch,
	    NULL, ch, TO_ALL );
	gain_stamina( vch, dice(2, 10), FALSE );
	return TRUE;
    }
    return TRUE;
}


bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected(ch, skill_lookup( "shield from true source" )) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  6: min_level = 12; spell = "change sex";     break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8:
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	case 11: min_level = 15; spell = "plague";	   break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( ch->position != POS_FIGHTING )
        return FALSE;
 
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }
 
    if ( victim == NULL )
        return FALSE;
 
    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TAR_IGNORE );
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected(ch, skill_lookup( "shield from true source" )) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "teleport";       break;
	case  4: min_level = 11; spell = "colour spray";   break;
	case  5: min_level = 12; spell = "change sex";     break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7:
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	case 10: min_level = 20; spell = "plague";	   break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TAR_IGNORE );
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 21; spell = "teleport";       break;
	case  8: min_level = 20; spell = "plague";	   break;
	default: min_level = 18; spell = "harm";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TAR_IGNORE );
    return TRUE;
}


bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
    }

    if ( victim == NULL )
	return FALSE;

    do_yell( ch, buf );
    act( "$n suddenly attacks $N!", ch, NULL, victim, TO_NOTVICT );
    act( "$n suddenly attacks you!", ch, NULL, victim, TO_VICT );
    act( "You attack $N!", ch, NULL, victim, TO_CHAR );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
	ch->in_room );
    char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
	ch->in_room );
    return TRUE;
}

/* A procedure for Puff the Fractal Dragon--> it gives her an attitude.
	Note that though this procedure may make Puff look busy, she in
	fact does nothing quite more often than she did in Merc 1.0;
	due to null victim traps, my own do-nothing options, and various ways
	to return without doing much, Puff is... well, not as BAD of a gadfly
	as she may look, I assure you.  But I think she's fun this way ;)

	(btw--- should you ever want to test out your socials, just tweak
	the percentage table ('silliness') to make her do lots of socials,
	and then go to a quiet room and load up about thirty Puffs... ;) 
			
		written by Seth of Rivers of Mud         */
			
bool spec_puff( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	int rnd_social, sn, silliness;
	bool pc_found = TRUE;
    CHAR_DATA *v_next;
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *nch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;
	extern social_count;
 	
	if ( !IS_AWAKE(ch) )
		return FALSE;

	victim = NULL;
  
/* Here's Furey's aggress routine, with some surgery done to it.  
  	All it does is pick a potential victim for a social.  
  	(Thank you, Furey-- I screwed this up many times until I
  	learned of your way of doing it)                      */
  		
    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->in_room == NULL )
	    continue;

	for ( nch = wch->in_room->people; nch != NULL; nch = ch_next )
	{
	    int count;

	    ch_next	= nch->next_in_room;

	    if ( !IS_NPC(nch) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'nch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if (victim == NULL)
			return FALSE;
	}

	}
		rnd_social = (number_range (0, ( social_count - 1)) );
			
	/* Choose some manner of silliness to perpetrate.  */
	
	silliness = number_range (1, 100);
		
	if ( silliness <= 20)
		return TRUE;
	else if ( silliness <= 30)
		{
		sprintf( buf, "Tongue-tied and twisted, just an earthbound misfit, ..."); 
		do_say ( ch, buf);
		}
	else if ( silliness <= 40)
		{
		 sprintf( buf, "The colors, the colors!");
		do_say ( ch, buf);
		}
	else if ( silliness <= 55)
		{
		sprintf( buf, "Did you know that I'm written in C?");
		do_say ( ch, buf);
		}
	else if ( silliness <= 75)
		{
		act( social_table[rnd_social].others_no_arg, 
			ch, NULL, NULL, TO_ROOM    );
		act( social_table[rnd_social].char_no_arg,   
			ch, NULL, NULL, TO_CHAR    );
		}
	else if ( silliness <= 85)
		{		
		if ( (!pc_found)
		|| 	 (victim != ch->in_room->people) ) 
			return FALSE;
		act( social_table[rnd_social].others_found, 
			 ch, NULL, victim, TO_NOTVICT );
		act( social_table[rnd_social].char_found,  
			 ch, NULL, victim, TO_CHAR    );
		act( social_table[rnd_social].vict_found, 
			 ch, NULL, victim, TO_VICT    );
		}
		
	else if ( silliness <= 97)	
		{	
		act( "For a moment, $n flickers and phases.", 
			ch, NULL, NULL, TO_ROOM );
		act( "For a moment, you flicker and phase.", 
			ch, NULL, NULL, TO_CHAR );
		}
	
/* The Fractal Dragon sometimes teleports herself around, to check out
	new and stranger things.  HOWEVER, to stave off some possible Puff
	repop problems, and to make it possible to play her as a mob without
	teleporting helplessly, Puff does NOT teleport if she's in Limbo,
	OR if she's not fighting or standing.  If you're playing Puff and 
	you want to talk with someone, just rest or sit!
*/
	
	else{
		if (ch->position < POS_FIGHTING)
			{
			act( "For a moment, $n seems lucid...", 
				ch, NULL, NULL, TO_ROOM );
			act( "   ...but then $e returns to $s contemplations once again.", 
				ch, NULL, NULL, TO_ROOM );
			act( "For a moment, the world's mathematical beauty is lost to you!",
				ch, NULL, NULL, TO_CHAR );
			act( "   ...but joy! yet another novel phenomenon seizes your attention.", 
				ch, NULL, NULL, TO_CHAR);
			return TRUE;
			}
		if ( ( sn = skill_lookup( "teleport" ) ) < 0 )
			return FALSE;
    	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, ch, TAR_IGNORE );
 		}


/* Puff has only one spell, and it's the most annoying one, of course.
  	(excepting energy drain, natch)  But to a bemused mathematician,
  	what could possibly be a better resolution to conflict? ;) 
  	Oh-- and notice that Puff casts her one spell VERY well.     */
  			
	if ( ch->position != POS_FIGHTING )
		return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	   	break;
    }

    if ( victim == NULL )
		return FALSE;

    if ( ( sn = skill_lookup( "teleport" ) ) < 0 )
		return FALSE;
    (*skill_table[sn].spell_fun) ( sn, 50, ch, victim, TAR_IGNORE );
    	return TRUE;

}

bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;

    if ( !IS_AWAKE(ch) || ch->fighting != NULL )
	return FALSE;

    ech      = NULL;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim->fighting != NULL
	&&   victim->fighting != ch )
	{
	    if ( IS_NPC(victim->fighting) && !IS_NPC(victim) )
		ech      = victim;
	}
    }

    if ( ech != NULL )
    {
	act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	    ch, NULL, NULL, TO_ROOM );
	act( "$n suddenly attacks $N!", ch, NULL, ech, TO_NOTVICT );
	act( "$n suddenly attacks you!", ch, NULL, ech, TO_VICT );
	act( "You attack $N!", ch, NULL, ech, TO_CHAR );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_cleric( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, path[pos] - '0', FALSE );
	break;

    case 'W':
	ch->position = POS_STANDING;
	act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
	act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
	act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
	act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
	act( "$n says 'I hereby declare the city of Midgaard open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
	act( "$n says 'I hereby declare the city of Midgaard closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
/*	do_unlock( ch, "gate" ); */
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
/*	do_lock( ch, "gate" ); */
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level )
	return FALSE;

    act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", ch, NULL, victim, TO_VICT    );
/*    spell_poison( gsn_poison, ch->level, ch, victim, TAR_IGNORE ); */
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 5 ) != 0 
	||   !can_see(ch,victim))
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		ch, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    gold = victim->gold * UMIN(number_range( 1, 20 ),ch->level) / 100;
	    gold = UMIN(gold, ch->level * ch->level * 20 );
	    ch->gold     += gold;
	    victim->gold -= gold;
	    return TRUE;
	}
    }

    return FALSE;
}


bool spec_junker( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    if ( number_bits(3) != 0 )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(ch,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    char buf[MAX_INPUT_LENGTH];

	    one_argument( trash->name, buf );
	    do_get( ch, buf );
	    return TRUE;
	}
	if ( ch->carry_number > 10 )
	    extract_obj( ch->carrying );
    }

    return FALSE;
}


/*
 * Special Functions Table.     OLC
 */
const   struct  spec_obj_type       spec_obj_table      [ ] =
{
    /*
     * Special function commands.
     */
    { "spec_chaos_weapon",	spec_chaos_weapon	},
    { "spec_hot_tub",		spec_hot_tub		},
    /*
     * End of list.
     */
    { "",                       0       }
};



/*****************************************************************************
 Name:          spec_obj_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char *spec_obj_string( SPEC_OBJ *fun )      /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_obj_table[cmd].spec_fun != NULL; cmd++ )
        if ( fun == spec_obj_table[cmd].spec_fun )
            return spec_obj_table[cmd].spec_name;

    return 0;
}



/*****************************************************************************
 Name:          spec_lobj_ookup
 Purpose:       Given a name, return the appropriate spec obj fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_OBJ *spec_obj_lookup( const char *name )       /* OLC */
{
    int cmd;

    for ( cmd = 0; spec_obj_table[cmd].spec_name != NULL; cmd++ )
        if ( !str_cmp( name, spec_obj_table[cmd].spec_name ) )
            return spec_obj_table[cmd].spec_fun;

    return 0;
}


bool spec_chaos_weapon( OBJ_DATA *obj )
{
    int dice, size, bit = 0;

    if ( obj->item_type != ITEM_WEAPON )
	return FALSE;

    size = number_range( 1, 12 );
    dice = UMAX(1, number_range( 1, 120 ) / size);

    bit = number_range( 0, 63 );

    obj->value[1] = dice;
    obj->value[2] = size;
    obj->value[4] = bit;

    REMOVE_BIT( obj->value[4], WEAPON_TWO_HANDS );

    if ( obj->carried_by == NULL && obj->in_room == NULL )
	return TRUE;


    if ( obj->carried_by != NULL )
	act( "$p flashes suddenly.", obj->carried_by, obj, NULL, TO_ALL );
    else if ( obj->in_room->people != NULL )
	act( "$p flashes suddenly.", obj->in_room->people, obj, NULL, TO_ALL );
    return TRUE;
}

bool spec_hot_tub( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( (ch = obj->in_room->people) == NULL )
	return FALSE;

    if ( number_bits(3) == 0 )
	return FALSE;

    switch( number_bits(2) )
    {
	default:
	case 0:
	    act( "The hot tub bubbles.", ch, NULL, NULL, TO_ALL );
	    break;
	case 1:
	    {
		const char * colors[11] = 
		{
		    "red", "orange", "yellow", "green", "blue", "purple",
		    "Bright white", "Aqua colored", "Pink",
		    "Cranberry", "Lime"
		};
		act( "$t lights come on under the water.", ch,
		     colors[number_range(0, 10)], NULL, TO_ALL );
	    }
	    break;
	case 2:
	    {
		CHAR_DATA *victim;
		victim = random_room_char( obj->in_room );

		if ( victim == NULL )
		    break;

		if ( victim->on != obj )
		    break;

		act( "Bubbles rise up around $n, tickling $m.", victim,
		   NULL, NULL, TO_ALL );
	    }
	    break;
    }
    return TRUE;
}	    

/*
 * Special Functions Table.     OLC
 */
const   struct  use_fun_type	use_fun_table      [] =
{
    /*
     * Special function commands.
     */
    { "use_envenom",		use_envenom	},
    { "use_baby_oil",		use_baby_oil	},
    { "use_tree",		use_tree	},

    /*
     * End of list.
     */
    { "",                       0       }
};



/*****************************************************************************
 Name:          use_fun_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char *use_fun_string( USE_FUN *fun )      /* OLC */
{
    int cmd;

    for ( cmd = 0; use_fun_table[cmd].use_fun != NULL; cmd++ )
        if ( fun == use_fun_table[cmd].use_fun )
            return use_fun_table[cmd].use_name;

    return 0;
}



/*****************************************************************************
 Name:          use_fun_lookup
 Purpose:       Given a name, return the appropriate use fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
USE_FUN *use_fun_lookup( const char *name )       /* OLC */
{
    int cmd;

    for ( cmd = 0; use_fun_table[cmd].use_name != NULL; cmd++ )
        if ( !str_cmp( name, use_fun_table[cmd].use_name ) )
            return use_fun_table[cmd].use_fun;

    return 0;
}


void use_envenom( OBJ_DATA *obj, CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *vobj;

    if ( obj->pIndexData->vnum != OBJ_VNUM_POISON )
    {
	send_to_char( "Only poison can be used to poison items.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( (vobj = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "What do you want to poison?\n\r", ch );
	return;
    }

    switch (vobj->item_type)
    {
	case ITEM_WEAPON:
		SET_BIT(vobj->value[4], WEAPON_POISON);
		act( "$n pour$% $p on $P.", ch, obj, vobj, TO_ALL );
		break;
	case ITEM_POTION:
		vobj->value[3] = TRUE;
		act( "$n pour$% $p into $P, thoroughly mixing it.", ch,
		    obj, vobj, TO_ALL );
		break;
	case ITEM_FOOD:
		vobj->value[3] = TRUE;
		act( "$n pour$% $p into $P, thoroughly mixing it.", ch,
		    obj, vobj, TO_ALL );
		break;
	case ITEM_INGREDIENT:
		vobj->value[3] = TRUE;
		act( "$n pour$% $p into $P, thoroughly mixing it.", ch,
		    obj, vobj, TO_ALL );
		break;
	case ITEM_DRINK_CON:
		vobj->value[3] = TRUE;
		act( "$n pour$% $p into $P, thoroughly mixing it.", ch,
		    obj, vobj, TO_ALL );		
		break;
	default:
		send_to_char( "You cannot poison that.\n\r", ch );
		return;
    }

    vobj->level = obj->level;
    extract_obj( obj );
    return;
}


void use_baby_oil( OBJ_DATA *obj, CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( (victim = get_char_room(ch, arg)) == NULL )
    {
	send_to_char( "You do not see them here.\n\r", ch );
	return;
    }

    if ( obj->value[1] <= 0 )
    {
	send_to_char( "No more oil in that.\n\r", ch );
	return;
    }

    act( "$n rub$% oil all over $O body.", ch, NULL, victim, TO_ALL );
    obj->value[1]--;
    return;
}


void use_tree( OBJ_DATA *obj, CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *fruit;
    int chance;

    if ( obj->item_type != ITEM_FRUIT_TREE )
	return;

    chance = number_range(1, obj->value[1]);

    if ( ch->level < 10 )
	chance *= 2;

    if ( number_percent() > chance )
    {
	send_to_char( "You can't seem to reach the fruit on the limbs.\n\r", ch );
	return;
    }

    fruit = create_object( get_obj_index(OBJ_VNUM_DUMMY), 0 );
    free_string( fruit->name        );
    free_string( fruit->short_descr );
    free_string( fruit->description );
    fruit->item_type = ITEM_FOOD;
    fruit->value[3]  = 0;
    fruit->value[0]  = number_range(1, 3);
    switch( obj->value[0] )
    {
	default:
	    bug( "Invalid fruit type.", 0 );
	    extract_obj( fruit );
	    return;
	case FRUIT_APPLE:
	    fruit->name = str_dup( "fruit apple" );
	    fruit->short_descr = str_dup( "an apple" );
	    fruit->description = str_dup( "A small apple sits here." );
	    break;
	case FRUIT_BANANA:
	    fruit->name = str_dup( "fruit banana" );
	    fruit->short_descr = str_dup( "a banana" );
	    fruit->description = str_dup( "A banana rests here." );
	    break;
	case FRUIT_PEAR:
	    fruit->name = str_dup( "fruit pear" );
	    fruit->short_descr = str_dup( "a pear" );
	    fruit->description = str_dup( "A small pear sits here." );
	    break;
	case FRUIT_CHERRY:
	    fruit->name = str_dup( "fruit cherry cherries" );
	    fruit->short_descr = str_dup( "a bunch of cherries" );
	    fruit->description = str_dup( "A small bunch of cherries rests here." );
	    break;
	case FRUIT_ORANGE:
	    fruit->name = str_dup( "fruit orange" );
	    fruit->short_descr = str_dup( "an orange" );
	    fruit->description = str_dup( "A small orange sits here." );
	    break;
	case FRUIT_PEACH:
	    fruit->name = str_dup( "fruit peach" );
	    fruit->short_descr = str_dup( "a peach" );
	    fruit->description = str_dup( "A small peach sits here." );
	    break;
	case FRUIT_PLUM:
	    fruit->name = str_dup( "fruit plum" );
	    fruit->short_descr = str_dup( "a plum" );
	    fruit->description = str_dup( "A small plum sits here." );
	    break;
    }
    if ( number_percent() < obj->value[1] )
	obj->value[1] = UMAX(1, obj->value[1] - 1);
    obj_to_char( fruit, ch );
    return;
}
