/**************************************************************************r
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "mem.h"

/* command procedures needed */

/*
 * Local functions.
 */
#define	CD	CHAR_DATA
#undef	CD


/*
 *   SKILL FUNCTIONS
 */


/* Converts 0 - 27 to 3 to 18 */
int normalize( int stat )
{
    int value;
    stat = stat * 10;

    /* Find distance from center */
    value = stat - 135;

    value = value * 15 / 27;

    return (105 + value)/10;
}


int luck_bonus( CHAR_DATA *ch )
{
    int luck;
    CHAR_DATA *vch;

    luck = luk_app[get_curr_stat(ch, STAT_LUK)].percent_mod;
    if ( IS_NPC(ch) )
	return luck;

    if ( ch->pcdata->talent[tn_weak_taveren] )
	luck = luck + number_fuzzy(0) * number_range(1, 10);
    if ( ch->pcdata->talent[tn_taveren] )
	luck = luck + number_fuzzy(0) * number_range(1, 25);
    if ( ch->pcdata->talent[tn_strong_taveren] )
	luck = luck + number_fuzzy(0) * number_range(1, 75);

    for ( vch = ch->in_room->people; vch; vch = vch->next )
    {
	if ( vch == ch || IS_NPC(vch) )
	    continue;

	if ( vch->pcdata->talent[tn_weak_taveren] )
	    luck = luck + number_fuzzy(0) * number_range(1, 5);
	if ( vch->pcdata->talent[tn_taveren] )
	    luck = luck + number_fuzzy(0) * number_range(1, 12);
	if ( vch->pcdata->talent[tn_strong_taveren] )
	    luck = luck + number_fuzzy(0) * number_range(1, 37);	
    }
    return luck;
}


int stat_bonus( CHAR_DATA *ch, sh_int stats )
{
    int count;
    int stat;
    int bonus;
    int stat_count;

    stat_count = 0;
    bonus = 0;
    for (count = 0; count < 8; count++)
    {
	if ( !IS_SET(stats, 1 << count) )
	    continue;

	stat = normalize( ch->perm_stat[count] );
	stat_count++;

	switch( stat )
	{
	    default:
		break;
	    case 18:
		bonus += 25;
		break;
	    case 17:
		bonus += 20;
		break;
	    case 16:
		bonus += 15;
		break;
	    case 15:
		bonus += 10;
		break;
	    case 14:
		bonus += 5;
		break;
	    case 13:
		bonus += 3;
		break;

	    case 8:
		bonus -= 3;
		break;
	    case 7:
		bonus -= 5;
		break;
	    case 6:
		bonus -= 10;
		break;
	    case 5:
		bonus -= 15;
		break;
	    case 4:
		bonus -= 20;
		break;
	    case 3:
		bonus -= 25;
		break;
	}
    }

    if ( stat_count == 0 )
	return 0;

    return ( bonus / stat_count );
}


bool check_skill( CHAR_DATA *ch, int sn, int skill_mod, bool pLuck )
{
    int chance, roll;

    if ( !IS_NPC(ch)
    &&   (ch->level < skill_table[sn].skill_level[ch->class]
    ||    ch->pcdata->learned[sn] < 1) )
	return FALSE;

    chance = get_skill( ch, sn );
    if ( pLuck )
	chance = chance + luck_bonus( ch );

    if ( skill_table[sn].stat != SKILL_NONE )
	chance = chance + stat_bonus( ch, skill_table[sn].stat );

    chance = chance * skill_mod / 100;

    roll = number_percent();
    if ( roll >= chance
    ||	 roll >= 100 )
    {
	check_improve( ch, sn, FALSE, abs(skill_table[sn].rating[ch->class]) );
	return FALSE;
    }

    if ( !IS_IMMORTAL(ch)
    &&   number_percent() >= 96
    &&   check_stat(ch, STAT_WIS, -10)
    &&   number_percent() > get_skill(ch, sn) * 4 / 5 )
    {
	int gain;

	gain = number_range( get_curr_stat(ch, STAT_INT),
			     get_curr_stat(ch, STAT_INT) * 2 );
	gain = UMAX( 1, gain - (get_skill(ch, sn) / 5) ) * 3;
	send_to_char_new( ch, "For successfully using your skills, you gain %d experience.\n\r", gain );
	gain_exp( ch, gain );
    }
    check_improve( ch, sn, TRUE, abs(skill_table[sn].rating[ch->class]) );
    return TRUE;
}

int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 4 / 5;
    }

    else if (sn < 1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

    else if (!IS_NPC(ch))
    {
	if (ch->level < skill_table[sn].skill_level[ch->class])
	    skill = 0;
	else
	{
	    if ( ch->pcdata->learned[sn] < 1 )
		skill = 0;
	    else
		skill = SKILL(ch,sn);
	}
    }

    else /* mobiles */
    {

	if (sn == gsn_sneak)
	    skill = ch->level * 2 + 20;

	else if ( sn == gsn_earth
	&&   IS_SET(ch->act, ACT_CHANNELER)
	&&   ch->channel_skill[0] > -1 )
	    skill = ch->channel_skill[0];

	else if ( sn == gsn_air
	&&   IS_SET(ch->act, ACT_CHANNELER)
	&&   ch->channel_skill[1] > -1 )
	    skill = ch->channel_skill[1];

	else if ( sn == gsn_fire
	&&   IS_SET(ch->act, ACT_CHANNELER)
	&&   ch->channel_skill[2] > -1 )
	    skill = ch->channel_skill[2];

	else if ( sn == gsn_water
	&&   IS_SET(ch->act, ACT_CHANNELER)
	&&   ch->channel_skill[3] > -1 )
	    skill = ch->channel_skill[3];

	else if ( sn == gsn_spirit
	&&   IS_SET(ch->act, ACT_CHANNELER)
	&&   ch->channel_skill[4] > -1 )
	    skill = ch->channel_skill[4];

	else if (sn == gsn_parry
        && (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_ROGUE)))
	    skill = UMIN( 30, ch->level );

	else if (sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
	    skill = UMIN( 30, ch->level * 3 / 4 );

	else if (sn == gsn_hand_to_hand)
	    skill = 20 + ch->level * 2 / 3;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 5 + ch->level * 3 / 4;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->off_flags,ACT_WARRIOR)
	     ||	  IS_SET(ch->off_flags,ACT_ROGUE)))
	    skill = 10 + ch->level * 5 / 6;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = ch->level * 9 / 10;

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_staff)
	    skill = 30 + ch->level * 5 / 4;

	else if (sn == gsn_weapon_prof)
	    skill = 20 + ch->level * 5 / 6;

	else if ( sn == gsn_ambidexterity 
	     &&   IS_SET(ch->off_flags,ACT_WARRIOR) )
	    skill = 20 + ch->level * 3 / 4;

	else if ( sn == gsn_acrobatics
	     &&   ch->position >= POS_FIGHTING
	     &&   IS_SET(ch->off_flags, OFF_ACROBATIC))
	    skill = 15 + ch->level * 3 / 5;

	else if ( sn == gsn_dual_wield
	     &&   IS_SET(ch->off_flags,ACT_WARRIOR) )
	    skill = 20 + ch->level / 2;

	else if ( sn == gsn_hunt )
	    skill = 30 + ch->level * 2 / 5;

	else if ( sn == gsn_riding )
	    skill = 50 + ch->level * 3 / 5;

	else if ( sn == gsn_feint 
	     &&   IS_SET(ch->off_flags,ACT_WARRIOR)
	     &&   ch->level > 70 )
	    skill = 10 + (ch->level - 70) * 3 / 4;

	else if ( sn == gsn_riposte 
	     &&   IS_SET(ch->off_flags,ACT_WARRIOR)
	     &&   ch->level > 70 )
	    skill = 15 + ( ch->level - 70 ) / 4;

	else if ( sn == gsn_blindfighting
	     &&   IS_SET(ch->off_flags,ACT_WARRIOR)
	     &&   ch->level > 70 )
	    skill = 30 + ( ch->level - 70 ) * 3 / 5;

	else if ( sn == gsn_flash_strike 
	     &&   ch->guild == guild_lookup("warder") )
	    skill = ch->level / 6;

	else if ( sn == gsn_endurance 
	     &&   ch->guild == guild_lookup("warder") )
	    skill = 10 + ch->level / 4;

	else if ( sn == gsn_detect_shadowspawn
	     &&   ch->guild  == guild_lookup("warder") )
	    skill = 25 + ch->level / 2;

	else 
	   skill = 0;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK))
	skill -= ch->level / 2;

    return URANGE(0,skill,100);
}


int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
        case(WEAPON_STAFF):     sn = gsn_staff;         break;
   }
   return sn;
}

int get_secondary_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_SECONDARY );
    if ( wield == NULL || wield->item_type != ITEM_WEAPON )
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
        case(WEAPON_STAFF):     sn = gsn_staff;         break;
   }
   return sn;
}


int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = SKILL(ch,sn);
    }

    return URANGE(0,skill,100);
} 


int get_secondary_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = SKILL(ch,sn);
    }

    return URANGE(0,skill,100);
} 


/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max = 25;
    int value;

    value = ch->perm_stat[stat] + ch->mod_stat[stat];
    if ( !IS_NPC(ch) && ch->pcdata->talent[stat] )
    {
	value += 2;
	max   += 2;
    }

    return URANGE( 0, value, max );
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max = 25;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return 25;

    max = 20 + pc_race_table[ch->race].stats[stat];

    if ( !IS_NPC(ch) && ch->pcdata->talent[stat] )
	max   += 2;

    return UMIN( max, 27 );
}

