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
/*

===========================================================================
This snippet was written by Erwin S. Andreasen, 4u2@aabc.dk. You may use
this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://login.dknet.dk/~ea/

If you do not have WWW access, try ftp'ing to login.dknet.dk and examine
the /pub/ea directory.
===========================================================================
Note that function prototypes are not included in this code - remember to add
them to merc.h yourself - also add the 'second' command to interp.c.

Secondary weapon code prototype. You should probably do a clean compile on
this one, due to the change to the MAX_WEAR define.

Last update: Oct 10, 1995

Should work on : MERC2.2

Fixed since last update:

Know bugs and limitations yet to be fixed:
Comments:

*/

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
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_combat	);

DECLARE_SPELL_FUN(	spell_null	);

void warder_scan( CHAR_DATA *ch );
void recurse_scan( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, sh_int door, sh_int count );
int find_scan_dir( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch,
		   int depth, int in_zone );
bool can_cast( CHAR_DATA *ch, int sn );

void	show_char_affect	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void	show_obj_affect		args( ( CHAR_DATA *ch, OBJ_DATA *obj) );
void	show_room_affect	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
char	*bit_name		args( ( AFFECT_DATA *paf) );
char	*grasp_message		args( ( CHAR_DATA *ch ) );
int	get_appraise		args( ( OBJ_DATA *obj ) );
char	*position_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

extern	char	*	const	power_name[5];

char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<secondary weapon>  "  /* ADD THIS */
};

char * const color_name [] =
{
  "AUCTION",
  "OOC",
  "IMMTALK",          
  "BARD",
  "TELL",
  "SAY",
  "GUILD",
  "SPECIAL",
  "ROOM_NAME",
  "ROOM_DESC",
  "CHARACTER",
  "OBJECT",
  "EXIT"
};


/* for do_count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void	do_count		args( ( CHAR_DATA *ch, char *argument ) );
char *flag_string               args ( ( const struct flag_type *flag_table,
                                         int bits ) );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( obj->condition < 90 )
    {
	if ( IS_SET(obj->extra_flags, ITEM_BENT))   strcat( buf, "`3(Bent)`n "   );
	if ( IS_SET(obj->extra_flags, ITEM_RUINED))
	    strcat( buf, "`3(Broken)`n "   );
	else if ( obj->condition > 65 )
	    strcat( buf, "`6`B(slightly damaged)`n " );
	else if ( obj->condition > 40 )
	    strcat( buf, "`6(moderately damaged)`n " );
	else if ( obj->condition > 15 )
	    strcat( buf, "`3`B(heavily damaged)`n " );
	else
	    strcat( buf, "`3(immensely damaged)`n " );
    }
    if ( obj->item_type == ITEM_WEAPON
    &&   IS_SET(obj->value[4], WEAPON_POISON) )	strcat( buf, "`2(green stain)`n " );

    if (IS_SET(ch->act, PLR_ANSI))
        strcat( buf, color_table[ch->colors[COLOR_OBJ]].code );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }
    strcat( buf, "`n" );

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if (prgpstrShow[iShow][0] == '\0')
        {
            free_string(prgpstrShow[iShow]);
            continue;
        }

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output, buf);
	    }
	    else
	    {
		add_buf(output, "     ");
	    }
	}
	add_buf(output, prgpstrShow[iShow]);
	add_buf(output, "\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);
    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)	) strcat( buf, "`B`0(Invis)`b "      );
    if ( !IS_NPC(victim) &&
	  IS_SET(victim->act, PLR_WIZINVIS)	) strcat( buf, "`7(Wizi) " );
    if ( IS_SET(victim->comm, COMM_AFK)	      ) strcat( buf, "`B`3(AFK)`b ");
    if ( IS_WRITING(victim)		      )	strcat( buf, "`B`7(Writing)`b " );
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&   ch->pcdata->talent[tn_sense_taveren] )
    {
	if ( victim->pcdata->talent[tn_weak_taveren] )
	    strcat( buf, "`B`7(Dim sheen)`b " );
	else if ( victim->pcdata->talent[tn_taveren] )
	    strcat( buf, "`B`7(Sheen)`b " );
	else if ( victim->pcdata->talent[tn_strong_taveren] )
	    strcat( buf, "`B`7(Bright sheen)`b " );
    }
    if ( !IS_NPC(victim) && victim->desc == NULL )
	strcat( buf, "`7(Linkless) " );

    if (IS_SET(ch->act, PLR_ANSI))
	strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );

    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0'
	 && victim->on == NULL
	 && (victim->rider == NULL && victim->rider != ch )
	 && (victim->mount == NULL && victim->rider != ch ) )
    {
	strcat( buf, victim->long_descr );
        if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );

	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC(victim) )
	strcat( buf, LASTNAME(victim) );

    if (IS_SET(ch->act, PLR_ANSI))
	strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );

    if ( IS_IN_GUILD(victim, guild_lookup("seanchan"))
    &&   victim->pcdata->guild->damane != NULL )
    {
	if ( victim->pcdata->guild->damane->in_room == victim->in_room
	&&   victim->pcdata->guild->damane != ch )
	{
	    strcat( buf, ", attached by a silvery leash and bracelet to " );
	    strcat( buf, PERS(victim->pcdata->guild->damane, ch) );
	}
	else if ( victim->pcdata->guild->damane->in_room == victim->in_room
	&&        victim->pcdata->guild->damane == ch )
	    strcat( buf, ", attached by a silvery leash and bracelet to you" );
	else
	    strcat( buf, ", attached by a silvery leash and bracelet to someone(!?)" );

	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
        return;
    }


    if ( IS_AFFECTED_2(victim, AFF_LEASHED) )
    {
	CHAR_DATA *suldam;

	suldam = get_suldam( victim );
	if ( suldam && suldam != ch )
	{
	    strcat( buf, ", attached by a silvery leash and collar to " );
	    strcat( buf, PERS(suldam, ch) );
	}
	else if ( suldam && suldam == ch )
	    strcat( buf, ", attached by a silvery leash and collar to you" );
	else
	    strcat( buf, ", attached by a silvery leash to someone(!?)" );

	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
        return;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE) )
    {
	if ( victim->hide_type == HIDE_OBJECT )
	{
	    OBJ_DATA *obj;
	    obj = (OBJ_DATA *) victim->hide;

	    strcat( buf, ", hiding behind " );
	    strcat( buf, obj->short_descr );
	}
	else if ( victim->hide_type == HIDE_CHAR )
	{
	    CHAR_DATA *vch;
	    vch = (CHAR_DATA *) victim->hide;

	    strcat( buf, ", hiding behind " );
	    if ( vch == ch )
		strcat( buf, "you" );
	    else
		strcat( buf, PERS(vch, ch) );
	}
	else	
	    strcat( buf, ", hiding here" );

	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
        return;
    }

    if ( victim->mount )
    {
	strcat( buf, ", mounted on " );
        strcat( buf, PERS(victim->mount, ch) );
	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
        return;
    }

    if ( victim->rider != NULL && victim->rider != ch)
    {
        strcat( buf, ", here with " );
        strcat( buf, PERS(victim->rider, ch) );
        strcat( buf, " sitting on " );
	strcat( buf, (victim->sex == SEX_FEMALE) ? "her" :
	    (victim->sex == SEX_MALE) ? "him" : "it" );

	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
        return;
    }

    if ( victim->rider != NULL && victim->rider == ch )
    {
        strcat( buf, ", here with you sitting on it" );

	if ( IS_GRASPING(victim)
	&&   can_channel(ch, 1)
	&&   TRUE_SEX(ch) == TRUE_SEX(victim)
	&&   channel_strength(ch, POWER_ALL) >= 25
	&&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	{
	    if ( TRUE_SEX(victim) == SEX_MALE )
		strcat( buf, " and surrounded by an " );
	    else
		strcat( buf, " and surrounded by a " );
	    strcat( buf, grasp_message(victim) );
	    strcat( buf, " of `B" );
	    strcat( buf, SOURCE(victim) );
	    strcat( buf, "`b" );
	}

	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );

	strcat( buf, position_message(ch, victim) );
	buf[0] = UPPER(buf[0]);
	send_to_char( buf, ch );
	return;
    }

    if ( IS_GRASPING(victim)
    &&   can_channel(ch, 1)
    &&   TRUE_SEX(ch) == TRUE_SEX(victim)
    &&   channel_strength(ch, POWER_ALL) >= 25
    &&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
    {
	if ( TRUE_SEX(victim) == SEX_MALE )
	    strcat( buf, ", surrounded by an " );
	else
	    strcat( buf, ", surrounded by a " );
	strcat( buf, grasp_message(victim) );
	strcat( buf, " of `B" );
	strcat( buf, SOURCE(victim) );
	strcat( buf, "`b" );
	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "," );
    }

    strcat( buf, position_message(ch, victim) );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}


char *position_message( CHAR_DATA *ch, CHAR_DATA *victim )
{
    static char buf[MAX_STRING_LENGTH];
    bool period = TRUE;

    buf[0] = '\0';

    if ( !IS_NPC(victim)
    &&   !victim->fighting 
    &&   !IS_NULLSTR(victim->pcdata->doing) )
    {
	strcat( buf, " is " );
	strcat( buf, victim->pcdata->doing );
	strcat( buf, "." );
	if (IS_SET(ch->act, PLR_ANSI))
	    strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );
	strcat( buf, "\n\r" );
	return buf;
    }

    switch ( victim->position )
    {
	case POS_DEAD:
	    strcat( buf, " is `3DEAD!!" );
	    period = FALSE;
	    break;
	case POS_MORTAL:
	    strcat( buf, " is `3mortally wounded" );
	    break;
	case POS_INCAP:
	    strcat( buf, " is `3incapacitated" );
	    break;
	case POS_STUNNED:
	    strcat( buf, " is lying here stunned" );
	    break;
	case POS_SLEEPING:
            if (victim->on != NULL)
            {
		char message[MAX_STRING_LENGTH];
		if (IS_SET(victim->on->value[2],SLEEP_AT))
		{
		    sprintf(message," is sleeping at %s",
			victim->on->short_descr);
                    strcat(buf,message);
                }
                else if (IS_SET(victim->on->value[2],SLEEP_ON))
                {
                    sprintf(message," is sleeping on %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else
                {
                    sprintf(message, " is sleeping in %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
            }
            else
                strcat(buf," is sleeping here");
            break;
       case POS_RESTING:
            if (victim->on != NULL)
            {
	        char message[MAX_STRING_LENGTH];
                if (IS_SET(victim->on->value[2],REST_AT)) 
                {
                    sprintf(message," is resting at %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else if (IS_SET(victim->on->value[2],REST_ON))
                {
                    sprintf(message," is resting on %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else
                { 
                    sprintf(message, " is resting in %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
            }
            else
                strcat( buf, " is resting here" );
            break;
        case POS_SITTING:
            if (victim->on != NULL)
            {
	        char message[MAX_STRING_LENGTH];
                if (IS_SET(victim->on->value[2],SIT_AT)) 
                {
                    sprintf(message," is sitting at %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else if (IS_SET(victim->on->value[2],SIT_ON))
                {
                    sprintf(message," is sitting on %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else
                { 
                    sprintf(message, " is sitting in %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
            }
            else
                strcat(buf, " is sitting here");
            break;
        case POS_STANDING:
            if (victim->on != NULL)
            {
	        char message[MAX_STRING_LENGTH];
                if (IS_SET(victim->on->value[2],STAND_AT))
                {
                    sprintf(message," is standing at %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                } 
                else if (IS_SET(victim->on->value[2],STAND_ON))
                {
                    sprintf(message," is standing on %s",
                       victim->on->short_descr);
                    strcat(buf,message);
                }
                else
                { 
                    sprintf(message," is standing in %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
            }
            else
                strcat( buf, " is here" );
            break;
        case POS_RECLINING:
            if (victim->on != NULL)
            {
	        char message[MAX_STRING_LENGTH];
                if (IS_SET(victim->on->value[2],SLEEP_AT))
                {
                    sprintf(message," is reclining at %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else if (IS_SET(victim->on->value[2],SLEEP_ON))
                {
                    sprintf(message," is reclining on %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
                else
                {
                    sprintf(message, " is reclining in %s",
                        victim->on->short_descr);
                    strcat(buf,message);
                }
            }
            else
                strcat(buf," is reclining here");
            break;
        case POS_FIGHTING:
	    period = FALSE;
	    strcat( buf, " is here, fighting " );
	    if ( victim->fighting == NULL )
	        strcat( buf, "thin air??" );
	    else if ( victim->fighting == ch )
	        strcat( buf, "YOU!" );
	    else if ( victim->in_room == victim->fighting->in_room )
 	    {
	        strcat( buf, PERS(victim->fighting, ch) );
	        period = TRUE;
	    }
	    else
	        strcat( buf, "somone who left??" );
	    break;
    }

    if (IS_SET(ch->act, PLR_ANSI))
	strcat( buf, color_table[ch->colors[COLOR_CHAR]].code );

    if ( period )
	strcat( buf, ".\n\r" );
    else
	strcat( buf, "\n\r" );

    return buf;
}


void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    ;
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( IS_NPC(victim) )
    {
	if ( victim->description[0] != '\0' )
	    send_to_char( victim->description, ch );
	else
	    act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
	if ( IS_DISGUISED(victim) )
	{
	    if ( !IS_NULLSTR(victim->pcdata->new_desc) )
		send_to_char( victim->pcdata->new_desc, ch );
	    else
		act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	}
	else
	{
	    if ( !IS_NULLSTR(victim->description) )
		send_to_char( victim->description, ch );
	    else
		act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	}
    }

    if ( !IS_NPC(victim) && !IS_NULLSTR(victim->pcdata->wearing) )
	act( "$Y $I wearing $t.", ch, victim->pcdata->wearing, victim, TO_CHAR );

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in `2excellent condition.`n\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a `2few scratches.`n\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks `3pretty hurt.`n\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in `3awful condition.`n\n\r");
    else
	strcat(buf, " is `3bleeding to death.`n\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    if ( IS_SET(victim->act, PLR_WOLFKIN) )
	act( "$N's eyes seem to be golden.", ch, NULL, victim, TO_CHAR );
    send_to_char( "\n\r", ch );

    if ( !IS_NPC(victim)
    &&   can_channel(ch, 1)
    &&   can_channel(victim, 1)
    &&   !IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL)
    &&   (TRUE_SEX( ch ) == TRUE_SEX( victim )
    ||    IS_IMMORTAL( ch )) )
    {
	const char *strength[] = { "tiny strength", "barely any strength", 
	    "little strength", "some strength", "minor strength",
	    "moderate strength", "decent strength", "major strength", 
	    "a lot of strength","great strength","awesome strength" };

	
	sprintf( buf, "$N has %s in the One Power.",
	    strength[channel_strength(victim, POWER_ALL) / 10] );
	act( buf, ch, NULL, victim, TO_CHAR );
    }

    if ( victim->affected )
	show_char_affect( ch, victim );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   check_skill(ch, gsn_peek, 100, TRUE)
    &&   IS_CONFIG(ch, CONFIG_PEEK) )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < rch->invis_level )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;

  if( *argument && IS_IMMORTAL( ch))
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  /* lists most player flags */
  if (IS_NPC(ch) || IS_NPC( victim))
    return;

  send_to_char("   action     status\n\r",ch);
  send_to_char("---------------------\n\r",ch);
 
  send_to_char("color          ",ch);
  if( IS_SET( victim->act,PLR_ANSI))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("autoassist     ",ch);
  if( IS_SET( victim->act,PLR_AUTOASSIST))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch); 

  send_to_char("autoexit       ",ch);
  if( IS_SET( victim->act,PLR_AUTOEXIT))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("autogold       ",ch);
  if( IS_SET( victim->act,PLR_AUTOGOLD))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("autoloot       ",ch);
  if( IS_SET( victim->act,PLR_AUTOLOOT))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("autosac        ",ch);
  if( IS_SET( victim->act,PLR_AUTOSAC))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("autosplit      ",ch);
  if( IS_SET( victim->act,PLR_AUTOSPLIT))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("prompt         ",ch);
  if( IS_SET( victim->comm,COMM_PROMPT))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  send_to_char("combine items  ",ch);
  if( IS_SET( victim->comm,COMM_COMBINE))
    send_to_char("`2ON`n\n\r",ch);
  else
    send_to_char("`3OFF`n\n\r",ch);

  if( !IS_SET( victim->act,PLR_CANLOOT))
    send_to_char("`2Your corpse is safe from thieves.`n\n\r",ch);
  else 
    send_to_char("`3Your corpse may be looted.`n\n\r",ch);

  if( IS_SET( victim->act,PLR_NOFOLLOW))
    send_to_char("`2You do not welcome followers.`n\n\r",ch);
  else
    send_to_char("`3You accept followers.`n\n\r",ch);
}


void do_autoassist(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here. \r\n", ch);
	return;
      }

  if (IS_NPC(ch) || IS_NPC( victim))
    return;
    
  if( IS_SET( victim->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOASSIST);
    }
  else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  if (IS_NPC(ch) || IS_NPC( victim))
    return;
 
  if( IS_SET( victim->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOEXIT);
    }
  else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  if (IS_NPC(ch) || IS_NPC( victim))
    return;
 
  if( IS_SET( victim->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOGOLD);
    }
  else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }
  
  if( IS_NPC(ch) || IS_NPC( victim))
    return;
 
  if( IS_SET( victim->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOLOOT);
    }
  else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here. \r\n", ch);
	return;
      }

  if( IS_NPC(ch) || IS_NPC( victim))
    return;
 
  if( IS_SET( victim->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOSAC);
    }
  else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( *argument && IS_IMMORTAL( ch))
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  if( IS_NPC(ch) || IS_NPC( victim))
    return;
 
  if( IS_SET( victim->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT( victim->act,PLR_AUTOSPLIT);
    }
  else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT( victim->act,PLR_AUTOSPLIT);
    }
}

void do_autoall(CHAR_DATA *ch, char *argument)
{
    do_autoassist( ch, argument );
    do_autoexit( ch, argument );
    do_autogold( ch, argument );
    do_autoloot( ch, argument );
    do_autosac( ch, argument );
    do_autosplit( ch, argument );
    send_to_char( "All autos toggled.\n\r", ch );
    return;
}

void do_config( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Configuration options:\n\r", ch );
	send_to_char( "COMBHELP     ", ch );
	if ( IS_CONFIG(ch, CONFIG_COMBHELP) )
	    send_to_char( "`2You see help files in combined format.`n\n\r", ch );
	else
	    send_to_char( "`3You do not see help files in combined format.`n\n\r", ch );

	send_to_char( "PEEK         ", ch );
	if ( IS_CONFIG(ch, CONFIG_PEEK) )
	    send_to_char( "`2You will peek into others' inventory.`n\n\r", ch );
	else
	    send_to_char( "`3You will NOT peek into others' inventory.`n\n\r", ch );
	return;
    }

    if ( !str_prefix( argument, "combhelp" ) )
    {
	if ( IS_CONFIG(ch, CONFIG_COMBHELP) )
	{
	    REMOVE_BIT(ch->pcdata->config, CONFIG_COMBHELP);
	    send_to_char( "You no longer see helps in combined format.\n\r", ch );
	}
	else
	{
	    SET_BIT(ch->pcdata->config, CONFIG_COMBHELP);
	    send_to_char( "You now see helps in combined format.\n\r", ch );
	}
	return;
    }

    if ( !str_prefix( argument, "peek" ) )
    {
	if ( IS_CONFIG(ch, CONFIG_PEEK) )
	{
	    REMOVE_BIT(ch->pcdata->config, CONFIG_PEEK);
	    send_to_char( "You no longer peek into others' inventory.\n\r", ch );
	}
	else
	{
	    SET_BIT(ch->pcdata->config, CONFIG_PEEK);
	    send_to_char( "You now peek into others' inventory.\n\r", ch );
	}
	return;
    }
    do_config( ch, "" );
    return;
}

void do_color(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  victim = ch;
  if( *argument && IS_IMMORTAL( ch))
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  if( IS_NPC(ch) || IS_NPC( victim))
    return;

  if( !IS_SET( victim->act, PLR_ANSI) )
    {
      send_to_char("ANSI color now activated.\n\r", ch);
      SET_BIT( victim->act, PLR_ANSI);
    }
  else
    {
      send_to_char("ANSI color now deactivated.\n\r", ch);
      REMOVE_BIT( victim->act, PLR_ANSI);
    }
    return;
}

void do_setcolor( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int c;
    int c2; 
  
    if ( IS_NPC(ch) )
        return;


    argument = one_argument( argument, arg );
    one_argument( argument, arg1 );
  
    if ( arg[0] == '\0' )
    {
	if ( IS_SET( ch->act, PLR_ANSI ) )
	for ( c = 0; c < MAX_COLORS; c++ )
	{
	    sprintf( buf, "%10ss will appear in %s%s`n\n\r",
		color_name[c],
		color_table[ch->colors[c]].code,
		color_table[ch->colors[c]].name );
	    send_to_char( buf, ch );
	};
	send_to_char( "To toggle ANSI color, type 'color'.  Type HELP COLOR for more help.\n\r", ch );
	return;
    };

    if ( !str_cmp(arg, "default") )
    {
	send_to_char( "Restoring default colors.\n\r", ch );
	ch->colors[COLOR_AUCTION]   = 7;
	ch->colors[COLOR_OOC]       = 1;
	ch->colors[COLOR_IMMTALK]   = 4;
	ch->colors[COLOR_BARD]      = 15;
	ch->colors[COLOR_TELL]      = 11;
	ch->colors[COLOR_SAY]       = 7;
	ch->colors[COLOR_GUILD]     = 14;
	ch->colors[COLOR_SPECIAL]   = 12;
    
	ch->colors[COLOR_NAME]      = 15;
	ch->colors[COLOR_DESC]      = 9;
	ch->colors[COLOR_CHAR]      = 3;
	ch->colors[COLOR_OBJ]       = 10;
	ch->colors[COLOR_EXIT]      = 5; 
	return;
    }

    for ( c = 0; c < MAX_COLORS; c++ )
    {
       if ( !str_prefix( arg, color_name[c] ) )
       break;
    }; 
        
    if ( !is_number( arg1 ) )
    {
	bool cFound = FALSE;
	for ( c2 = 0; c2 < 20; c2++ )
	{
	    if ( !str_prefix( arg1, color_table[c2].name ) )
	    {
		cFound = TRUE;
		break;
	    }
	}
	if ( !cFound )
	    c2 = -1;
    }
    else c2 = atoi( arg1 );
          
    if ((c < MAX_COLORS) && (c >= 0) && (c2 < 20) && (c2 >= 0))
    {
        ch->colors[c] = c2;
        sprintf( buf, "Set %s color to %s%s.\n\r", color_name[c],
                                                   color_table[c2].code,
                                                   color_table[c2].name );
        send_to_char( buf, ch );
	return;
    };

    send_to_char("Invalid color index.  ", ch);
    send_to_char("Valid colors are:\n\r", ch);
    for (c = 0; c < 20; c++)
    {
	sprintf( buf, "%2d -- %sTEST`n %s\n\r", c, color_table[c].code, 
	    color_table[c].name );
	send_to_char( buf, ch );
    }
    return;
}


void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
            
   if ( argument[0] == '\0' )
   {
        if (IS_SET(ch->comm,COMM_PROMPT))
        {
            send_to_char("You will no longer see prompts.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        }
        else
        {
            send_to_char("You will now see prompts.\n\r",ch);
            SET_BIT(ch->comm,COMM_PROMPT);
        }
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "< %hhp %sst > ");
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
        strcat(buf," ");
         
   }
            
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if ( CHARM_SET(ch) )
    {
	send_to_char( "You can't use this command while charmed or leashed.\n\r", ch );
	return;
    }

    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}


void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char colorbuf [1024];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->in_room == NULL )
	return;

    if ( IS_WRITING(ch) )
	return;
 
    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "moved" ) )
    {
	AFFECT_DATA *paf;

	/* 'look' or 'look auto' */
        if (IS_SET(ch->act, PLR_ANSI))
        {
            sprintf( colorbuf, "%s",
		color_table[ch->colors[COLOR_NAME]].code );
            send_to_char( colorbuf, ch );
        }

        send_to_char( ch->in_room->name, ch );
        ansi_color( NTEXT, ch );
	if ( IS_IMMORTAL(ch) )
	{
	    send_to_char( " [`B", ch );
	    send_to_char( flag_string(sector_flags, ch->in_room->sector_type), ch );
	    send_to_char( "`n]", ch );
	}
        send_to_char( "\n\r", ch );

        if (IS_SET(ch->act, PLR_ANSI))
        {
            sprintf( colorbuf, "%s",
		color_table[ch->colors[COLOR_DESC]].code );
            send_to_char( colorbuf, ch );
        }

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "",ch);
	    send_to_char( ch->in_room->description, ch );
	}

	for ( paf = ch->in_room->affected; paf; paf = paf->next )
	{
	    if ( paf->type == gsn_lightning_storm
	    ||   paf->type == gsn_hail_storm )
	    {
		send_to_char( "  Dark clouds are overhead.\n\r", ch );
		continue;
	    }

	    if ( paf->type == gsn_wind_barrier )
	    {
		send_to_char( "  Winds swirl about the room.\n\r", ch );
		continue;
	    }

	    if ( paf->type == gsn_earth_barrier )
	    {
		send_to_char( "  Piles of dirt surround the room.\n\r", ch );
		continue;
	    }

	    if ( paf->type == gsn_fire_wall )
	    {
		send_to_char_new( ch, "  A wall of flame blocks the %swards exit.\n\r", dir_name[paf->modifier] );
		continue;
	    }

	    if ( paf->type == gsn_ice_wall )
	    {
		send_to_char_new( ch, "  A wall of ice blocks the %swards exit.\n\r", dir_name[paf->modifier] );
		continue;
	    }
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_STEDDING)
	&&   can_channel(ch, 1) )
	    send_to_char( "You cannot sense the True Source here.\n\r", ch );

	if ( check_skill(ch, gsn_detect_shadowspawn, 100, FALSE) )
	{
	    warder_scan( ch );
	    check_improve( ch, gsn_detect_shadowspawn, TRUE, 4 );
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_FOG) )
	    send_to_char( "A thick fog is in the area.\r\n", ch );

        ansi_color( NTEXT, ch );

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
        ansi_color( NTEXT, ch );

	show_char_to_char( ch->in_room->people,   ch );
        ansi_color( NTEXT, ch );

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
            do_exits( ch, "auto" );

        ansi_color( GREY, ch); 
        ansi_color( NTEXT, ch );

	if ( ch->in_room->affected )
	    show_room_affect( ch, ch->in_room );

	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_GATE:
	    {
		ROOM_INDEX_DATA *to_room;
		ROOM_INDEX_DATA *from_room;

		from_room = ch->in_room;
		if ( IS_SET(obj->value[2], GATE_RANDOM)
		||   obj->value[3] == -1 )
		    to_room = get_random_room( ch );
		else
		    to_room = get_room_index( obj->value[3] );
		char_from_room( ch );
		char_to_room( ch, to_room );
		do_look( ch, "moved" );
		char_from_room( ch );
		char_to_room( ch, from_room );
	    }
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %s half full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{

	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL ) {
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;
            }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL ) {
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else continue;
            }

	    if ( is_name( arg3, obj->name ) )
	    {
	    	if (++count == number)
	    	{
	            if (obj->item_type == ITEM_FAQ)
	    	    {
			buf[0]='\0';
			sprintf( buf, "faq %i", obj->pIndexData->vnum );
			do_help(ch, buf );
	    	    }
		    else
		    {
			if ( obj->material == MAT_SPECIAL )
			    sprintf( buf, "It's made of something special and " );
		        else if ( obj->material == MAT_UNKNOWN )
			    sprintf( buf, "It seems to be made of an unknown material and " );
		        else
			    sprintf( buf, "It seems to be made of %s and ",
			        material_name( obj->material ) );
		        if ( obj->condition >  90 ) strcat( buf, "is in excellent condition.");
		        else if ( obj->condition >  75 ) strcat( buf, "is in good condition.");
		        else if ( obj->condition >  50 ) strcat( buf, "is in decent shape." );
		        else if ( obj->condition >  25 ) strcat( buf, "is looking worn and damaged." );
		        else if ( obj->condition >  10 ) strcat( buf, "looks like it could use some fixing." );
		        else if ( obj->condition >   0 ) strcat( buf, "is in bad shape, and almost broken." );
		        else strcat( buf, "is broken and in need of repair." ); 

		        strcat( buf, "\n\r" );
			send_to_char( buf, ch );
		    }
		    if ( obj->affected )
			show_obj_affect( ch, obj );
	    	    return;
	    	}
	    }
	}
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{

	    if ( obj->item_type == ITEM_GATE )
            {
                ROOM_INDEX_DATA *to_room;
                ROOM_INDEX_DATA *from_room;

                from_room = ch->in_room;
		if ( IS_SET(obj->value[2], GATE_RANDOM)
		||   obj->value[3] == -1 )
		    to_room = get_random_room( ch );
		else
		    to_room = get_room_index( obj->value[3] );
		if ( to_room != NULL )
		{
                    char_from_room( ch );
                    char_to_room( ch, to_room );
                    do_look( ch, "moved" );
                    char_from_room( ch );
                    char_to_room( ch, from_room );
		}
		return;
            }

	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	}

	if ( is_name( arg3, obj->name ) )
	{
	    if (++count == number)
	    {
	        if (obj->item_type == ITEM_FAQ)
	        {
		    buf[0]='\0';
		    sprintf( buf, "faq %i", obj->pIndexData->vnum );
		    do_help(ch, buf );
		}
		else
		{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char("\n\r",ch);

		    if ( obj->material == MAT_SPECIAL )
			sprintf( buf, "It's made of something special and " );
		    else if ( obj->material == MAT_UNKNOWN )
			sprintf( buf, "It seems to be made of an unknown material and " );
		    else
			sprintf( buf, "It seems to be made of %s and ",
			    material_name( obj->material ) );
		    if ( obj->condition >  90 ) strcat( buf, "is in excellent condition.");
		    else if ( obj->condition >  75 ) strcat( buf, "is in good condition.");
		    else if ( obj->condition >  50 ) strcat( buf, "is in decent shape." );
		    else if ( obj->condition >  25 ) strcat( buf, "is looking worn and damaged." );
		    else if ( obj->condition >  10 ) strcat( buf, "looks like it could use some fixing." );
		    else if ( obj->condition >   0 ) strcat( buf, "is in bad shape, and almost broken." );
		    else strcat( buf, "is broken and in need of repair." );

		    strcat( buf, "\n\r" );
		    send_to_char( buf, ch );
		}
		if ( obj->affected )
		    show_obj_affect( ch, obj );
	    	return;
	    }
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d %s's here.\n\r",count,arg3);
    	
    	send_to_char(buf,ch);
    	return;
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( IS_SET(pexit->rs_flags, EX_OPENEXIT) )
    {
	ROOM_INDEX_DATA *new_room, *in_room;
	new_room = pexit->u1.to_room;
	in_room = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, new_room);
	do_look( ch, "" );
	char_from_room(ch);
	char_to_room(ch, in_room);
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    char colorbuf[1024];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (IS_SET(ch->act, PLR_ANSI))
    {
        sprintf( colorbuf, "%s",
	    color_table[ch->colors[COLOR_EXIT]].code );
        send_to_char( colorbuf, ch );
    }

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room)
	&&   ( !IS_SET(pexit->exit_info, EX_HIDDEN) ||
		IS_IMMORTAL(ch) )
	&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		if ( IS_SET(pexit->exit_info, EX_HIDDEN) )
		    strcat( buf, "*" );
		strcat( buf, dir_name[door] );
		if ( IS_SET(pexit->exit_info, EX_HIDDEN) )
		    strcat( buf, "*" );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		    capitalize( dir_name[door] ),		    
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
	 		: pexit->u1.to_room->name
		    );
	    }
	}
	else if ( ( pexit = ch->in_room->exit[door] ) != NULL
	     &&   pexit->u1.to_room != NULL
	     &&   can_see_room(ch,pexit->u1.to_room)
	     &&   ( !IS_SET(pexit->exit_info, EX_HIDDEN) ||
		     IS_IMMORTAL(ch) )
	     &&   IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " -" );
		if ( IS_SET(pexit->exit_info, EX_HIDDEN) )
		    strcat( buf, "*" );
		strcat( buf, dir_name[door] );
		if ( IS_SET(pexit->exit_info, EX_HIDDEN) )
		    strcat( buf, "*" );
		strcat( buf, "-" );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - A closed door\n\r",
		    capitalize( dir_name[door] ) );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    ansi_color( GREY, ch);
    ansi_color( NTEXT, ch );

    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *tch;

  tch = ch;

  if (IS_IMMORTAL(ch) && *argument) {
    if ( ( tch = get_char_world( ch, argument ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  }

  if (tch == ch)
    sprintf( buf, "You have ");
  else
    sprintf( buf, "%s has ", tch->name);

  if (IS_NPC(tch)) {
    sprintf( buf, "%s%ld gold on hand and %ld gold in the bank.\n\r",
	    buf, tch->gold, tch->bank );
    send_to_char( buf, ch);
    return;
  }

  sprintf(buf, "%s`^%ld`n gold on hand, "
	  "`^%ld`n gold in the bank,"
	  "and `4%d`n experience (`4%d`n exp to level).\n\r",
	  buf, tch->gold, tch->bank, tch->exp,
	  (tch->level + 1) * exp_per_level(tch, tch->pcdata->points) - tch->exp);

  send_to_char( buf, ch);
  return;
}

char *form_ref_string( int number, int outof, char *col_str)
{
  static char buf[80];
  buf[0] = '\0';

  sprintf( buf, "%s%4d`n(`4%4d`n)", col_str, number, outof);
  return( buf );
}

char *form_weight_string( int number, int outof, char *col_str)
{
  static char buf[80];
  buf[0] = '\0';

  sprintf( buf, "%s%4d`n(`4%4d`n)", col_str, number, outof);
  return( buf );
}

char *form_stat_string( int number, int outof, char *col_str)
{
  static char buf[80];
  buf[0] = '\0';

  sprintf( buf, "%s%2d`n(`4%2d`n)", col_str, number, outof);
  return(buf );
} 
  
char *stat_string( CHAR_DATA *ch, int stat)
{
  int curr, perm;

  curr = get_curr_stat( ch, stat);
  perm = ch->perm_stat[ stat];

  if ( perm == curr)
    return form_stat_string( curr, perm, "`7`B");
  else 
    if (perm > curr)
      return form_stat_string( curr, perm, "`n`3");

  return form_stat_string( curr, perm, "`n`2");
}
	

char *weight_string( int curr, int perm)
{
  int ratio;

  ratio = curr * 100 / perm;

  if ( ratio >= 75)
    return form_weight_string( curr, perm, "`n`3");
  else
    if ( ratio >= 50)
      return form_weight_string( curr, perm, "`6`B");

  return form_weight_string( curr, perm, "`n`2");
}

char *stamina_string( CHAR_DATA *ch )
{
  int ratio;

  ratio = stamina_status( ch );

  if ( ratio >= 2)
    return form_ref_string( ch->stamina, ch->max_stamina, "`n`2");
  else
    if ( ratio == 1)
      return form_ref_string( ch->stamina, ch->max_stamina, "`6`B");

  return form_ref_string( ch->stamina, ch->max_stamina, "`n`3");
}

char *health_string( CHAR_DATA *ch )
{
  int ratio;

  ratio = health_status( ch );

  if ( ratio >= 2)
    return form_ref_string( ch->hit, ch->max_hit, "`n`2");
  else
    if ( ratio == 1)
      return form_ref_string( ch->hit, ch->max_hit, "`6`B");

  return form_ref_string( ch->hit, ch->max_hit, "`n`3");
}


void do_score( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  
  CHAR_DATA *tch;

  tch = ch;

  if (IS_IMMORTAL(ch) && *argument) {
    if ( ( tch = get_char_world( ch, argument ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  }

/*  Temporary fix */
  if ( IS_NPC(tch) )
    return;

  sprintf( buf, "`nName: %s%s%s`n\r\n",
	   FIRSTNAME(tch),
	   LASTNAME(tch),
	   TITLE(tch) );
  send_to_char( buf, ch );

  if ( !IS_NPC(ch) && !IS_NULLSTR(ch->pcdata->doing) )
    send_to_char_new( ch, "You are %s\r\n", ch->pcdata->doing );
  send_to_char( "\n\r", ch );

  sprintf( buf, "Level:   `4%8d`n :   "
	        "Race:  `4%12s`n :   "
	        "Age:     `4%8d`n\r\n",
	   tch->level,
	   race_table[tch->race].name, 
	   get_age(tch) );
  send_to_char( buf, ch );
 

  sprintf( buf, "Sex:     `4%8s`n :   "
  		"Hours:     `4%8d`n :   "
		"\n\r",
	   tch->sex == 0 ?
	     " sexless"
	     : tch->sex == 1 ?
	       "    male"
	       : "  female",
	   ( tch->played + (int) (current_time - tch->logon) ) / 3600 );
  send_to_char( buf, ch );

  if ( IS_GUILDED(tch) )
  {
    char *rank2;
    char *rank3;
    char *rank4;

    send_to_char( "\n\r", ch );
    sprintf( buf, "Guild:   `4%20s`n\n\r"
		  "Rank:    `4%20s`n\n\r",
	guild_name(tch->guild),
	guild_rank(tch->guild, GET_RANK(tch,1), 1, FALSE) );
    send_to_char( buf, ch );
    if ( tch->guild == guild_lookup("aes sedai") )
    {
	sprintf( buf, "Ajah:    `4%20s`n\n\r",
	    guild_rank(ch->guild, GET_RANK(tch,2), 2, FALSE) );
	send_to_char( buf, ch );
    }
    else if ( tch->guild == guild_lookup("aiel") )
    {
	rank2 = guild_rank(tch->guild, GET_RANK(tch,2), 2, TRUE);
	rank3 = guild_rank(tch->guild, GET_RANK(tch,3), 3, TRUE);

	sprintf( buf, "Clan:    `4%20s\n\r"
		      "Society: `4%20s\n\r",
	    rank2, rank3 );
	send_to_char( buf, ch );
	free_string( rank2 );
	free_string( rank3 );
    }
    else if ( tch->guild == guild_lookup("warder") )
    {
	rank2 = guild_rank(tch->guild, GET_RANK(tch,2), 2, TRUE);
	rank3 = guild_rank(tch->guild, GET_RANK(tch,3), 3, TRUE);

	sprintf( buf, "Order:   `4%s%s%s`n\n\r",
	    IS_NULLSTR(rank2) ? "" : rank2,
	    IS_NULLSTR(rank2) ? "" : " ",
	    rank3 );
	send_to_char( buf, ch );
	free_string( rank2 );
	free_string( rank3 );
    }
  }
  send_to_char( "\n\r", ch );

  sprintf( buf, "Str:   %20s :   "
	        "Hit:   %20s :   "
	        "Items:     %9s"
	        "\r\n",
	   stat_string(tch, STAT_STR),
	   health_string(tch),
	   weight_string(tch->carry_number, can_carry_n( tch )) );
  send_to_char( buf, ch);

  
  sprintf( buf, "Int:   %20s :   "
                "Stam:  %20s :   "
	        "Weight: %20s"
	        "\r\n",
	   stat_string( tch, STAT_INT),
	   stamina_string(tch),
	   weight_string( tch->carry_weight, can_carry_w( tch)));
  send_to_char( buf, ch);

  
  sprintf( buf, "Wis:    %19s :   "
	        "Xp:     `4%11d`n :   "
	        "Gold: `^%15ld`n"
	        "\r\n",
	   stat_string( tch, STAT_WIS),
	   tch->exp,
	   tch->gold);
  send_to_char( buf, ch);

  
  sprintf( buf, "Dex:    %19s :   "
	        "To Lvl: `4%11d`n :   "
	        "Bank:   `^%13ld`n"
	        "\r\n",
	   stat_string( tch, STAT_DEX),
	   ((tch->level + 1) * exp_per_level( tch, tch->pcdata->points)
	    - tch->exp),	   
	   tch->bank);
  send_to_char( buf, ch);

  
  sprintf( buf, "Con:    %19s :   "
	        "Qp:     `4%11d`n :   "
	        "\r\n",
	   stat_string( tch, STAT_CON),
	   IS_NPC( tch) ? 0 : tch->pcdata->qp );
  send_to_char( buf, ch);

  
  sprintf( buf, "Cha:    %19s :   "
	        "Prac:   `4%11d`n :   "
	        "\r\n",
	   stat_string( tch, STAT_CHR),
	   tch->practice );
  send_to_char( buf, ch);

  
  sprintf( buf, "Luc:    %19s :   "
	        "Train:  `4%11d`n :   "
	        "%10s`4%11d`n"
	        "\r\n",
	   stat_string( tch, STAT_LUK),
	   tch->train,
	   IS_IMMORTAL(tch) ? "Invis lvl:" : "",
	   (IS_SET( tch->act, PLR_WIZINVIS)) ? tch->invis_level : 0);
  send_to_char( buf, ch);

  
  sprintf( buf, "Agi:    %19s :   "
	        "Wimp:   `4%11d`n :   ",
	   stat_string( tch, STAT_AGI),
	   tch->wimpy );
  send_to_char( buf, ch);

  if ( IS_IMMORTAL(tch) )
    send_to_char_new( ch, "Home:        `4%8d`n", tch->home );

  send_to_char( "\r\n\r\n", ch );

    if ( !IS_NPC(tch) && tch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "`3You are drunk.`n\n\r",   ch );
    if ( !IS_NPC(tch) && tch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "`3You are thirsty.`n\n\r", ch );
    if ( !IS_NPC(tch) && tch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "`3You are hungry.`n\n\r",  ch );

    switch ( tch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are `3DEAD!!`n\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are `3mortally wounded`n.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are `3`Bincapacitated`n.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are `6`Bstunned`n.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_SITTING:
	send_to_char( "You are sitting.\n\r", 	ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r", 	ch );
	break;
    case POS_RECLINING:
	send_to_char( "You are reclining.\n\r", 	ch );
	break;
    case POS_MOUNTED:
	if ( tch->mount == NULL )
	    send_to_char( "You are mounted on something?.\n\r", 	ch );
	else
	    act( "You are mounted on $N.", ch, NULL, tch->mount, TO_CHAR );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are `3fighting`n.\n\r",		ch );
	break;
    }

    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(tch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(tch->act,PLR_HOLYLIGHT))
        send_to_char("`2on`n\r\n",ch);
      else
        send_to_char("`3off`n\r\n",ch);
    }

    if ( !IS_NPC(tch) && tch->pcdata->afk_message != NULL )
    {
	sprintf( buf, "Your AFK message is `4%s`n\n\r",
	    tch->pcdata->afk_message );
	send_to_char( buf, ch );
    }

    if ( IS_IN_GUILD(tch, guild_lookup("aes sedai")) )
    {
	char *temp;
	char name[MAX_INPUT_LENGTH];

	temp = one_argument(tch->pcdata->guild->warder, name);
	while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) )
	{
	    act( "$t is one of your warders.", ch, name, NULL,
		TO_CHAR );
	    temp = one_argument(temp, name);
	}
    }

    if ( tch->action_timer > 0 )
	send_to_char( "You are busy doing something at the moment.\n\r", ch );
    send_to_char( "\n\r", ch );
    if ( can_channel(tch, 1) && !IS_AFFECTED_2(tch, AFF_STILL) )
	send_to_char( "For CHANNELING information, use the \"channeling\" command.\n\r", ch );
    else if ( can_channel(tch, 1) && IS_AFFECTED_2(tch, AFF_STILL) )
	send_to_char( "You have been severed from the True Source!\n\r", ch );
    send_to_char( "For ARMOR information, use the \"armor\" command.\n\r", ch );
    send_to_char( "For AFFECT information, use the \"affects\" command.\n\r", ch );

    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day, hour, minute, second;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is `4%d`n o'clock `4%s`n, Day of `4%s`n, `4%d%s`n the Month of `4%s`n.\n\rThe `1W`2e`3a`4v`5e`n started up at `4%s`n\rThe system time is `4%s`n\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time )
	);
    send_to_char( buf, ch );

    hour	= (int) (current_time - boot_time) / 3600;
    minute	= ((int) (current_time - boot_time) / 60) % 60;
    second	= (int) (current_time - boot_time) % 60;

    sprintf( buf,
	"The `1W`2e`3a`4v`5e`n has been up for `4%d`n hour%s, `4%d`n minute%s, and `4%d`n second%s.\n\r",
	hour, hour != 1 ? "s" : "",
	minute, minute != 1 ? "s" : "",
	second, second != 1 ? "s" : "" );
    send_to_char( buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    const char * sky_look[6] =
    {
	"Not a cloud can be seen in the sky",
	"A layer of clouds hides the sky from you",
	"Rain is falling around you",
	"White snow is drifting down from the sky",
	"Hailing is pelting down from above",
	"The sky is lit by flashes of lightning"
    };

    const char * moon_look[8] =
    {
	"The night is pitch and dark, it must be a new moon.",
	"A thin silver crescent is on the eastern side of the moon .. it must be waxing.",
	"The eastern half of the moon is bright and silver.",
	"Almost all of the moon is full, except for a small crescent on the western side .. it must be waxing.",
	"The moon is bright and full.",
	"Almost all of the moon is full, except for a small crescent on the eastern side .. it must be waxing.",
	"The western half of the moon is bright and silver.",
	"A thin silver crescent is on the western side of the moon .. it must be waxing."
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "%s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    if ( weather_info.sunlight == SUN_DARK
    &&   weather_info.sky == SKY_CLOUDLESS )
    {
	sprintf( buf, "%s\r\n", moon_look[weather_info.moonlight % 8] );
	send_to_char( buf, ch );
    }
    return;
}

void do_scan( CHAR_DATA *ch, char *argument )
{
    int i;

    for (i = 0; i < 6; i++)
    {
	if (ch->in_room->exit[i] == NULL)
	    continue;

	if ( IS_SET(ch->in_room->exit[i]->exit_info, EX_HIDDEN) )
	    continue;

	switch (i)
	{
	    case 0: send_to_char( "`nTo the north you see...\n\r", ch);
		    break;
	    case 1: send_to_char( "`nTo the east you see...\n\r", ch);
		    break;
	    case 2: send_to_char( "`nTo the south you see...\n\r", ch);
		    break;
	    case 3: send_to_char( "`nTo the west you see...\n\r", ch);
		    break;
	    case 4: send_to_char( "`nUpwards you see...\n\r", ch);
		    break;
	    case 5: send_to_char( "`nDownwards you see...\n\r", ch);
		    break;
	}

	if ( IS_SET(ch->in_room->exit[i]->exit_info, EX_CLOSED) )
	{
	    char buf[MAX_INPUT_LENGTH];
	    sprintf( buf, "  `4A closed $d is here.`n" );
	    act( buf, ch, NULL, 
		ch->in_room->exit[i]->keyword, TO_CHAR );
	    continue;
	}

	recurse_scan( ch, ch->in_room->exit[i]->u1.to_room, i, 0 );
    }

    if ( IS_IN_GUILD(ch, guild_lookup("aes sedai")) )
    {
	char *temp;
	char name[MAX_INPUT_LENGTH];
        CHAR_DATA *warder;
        char buf[MAX_STRING_LENGTH];
        int result;

	temp = one_argument(ch->pcdata->guild->warder, name);
	while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) )
	{
            if ( (warder = get_char_sedai( ch, name )) == NULL )
            {
                act ( "You could not sense $t anywhere.", ch,
                    name, NULL, TO_CHAR );
		temp = one_argument(temp, name);
                continue;
            }

            sprintf( buf, "You sense %s ", name );
            send_to_char( buf, ch );

            if ( ch->in_room->vnum == warder->in_room->vnum )
            {
                send_to_char( "in this room.\n\r", ch );
		temp = one_argument(temp, name);
                continue;
            }

            result = find_scan_dir( ch->in_room->vnum,
                warder->in_room->vnum, ch, -40000, FALSE );

            if ( result == -1 )
            {
                send_to_char( "in some place far away.\n\r", ch );
		temp = one_argument(temp, name);
                continue;
            }
	    temp = one_argument(temp, name);
	}
    }

    if ( !IS_NPC(ch)
    &&   !IS_NULLSTR(ch->pcdata->sedai) )
    {
        CHAR_DATA *sedai;
        char buf[MAX_STRING_LENGTH];
        int result;

        if ( (sedai = get_char_sedai( ch, ch->pcdata->sedai )) == NULL )
        {
            act ( "You could not sense $t anywhere.", ch,
                ch->pcdata->sedai, NULL, TO_CHAR );
            return;
        }

        sprintf( buf, "You sense %s ", ch->pcdata->sedai );
        send_to_char( buf, ch );

        if ( ch->in_room == sedai->in_room )
        {
            send_to_char( "in this room.\n\r", ch );
            return;
        }

        result = find_scan_dir( ch->in_room->vnum,
            sedai->in_room->vnum, ch, -40000, FALSE );

        if ( result == -1 )
        {
            send_to_char( "in some place far away.\n\r", ch );
            return;
        }
    }
    return;
}


void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    bool found = FALSE;
    BUFFER *output;
    int level;

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    output = new_buf();

    if ( !str_cmp(argall, "index") )
    {
	char buf[MAX_STRING_LENGTH];
	int col = 0;
	for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
	{
	    level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;
	    if ( level > get_trust(ch) )
		continue;
	    if ( strlen(pHelp->keyword) > 80 - 24 * (col % 3) )
	    {
		add_buf( output, "\r\n" );
		col = 0;
	    }
	    sprintf( buf, "%24s  ", pHelp->keyword );
	    add_buf( output, buf );

	    if ( ++col % 3 == 0 )
		add_buf( output, "\r\n" );
	}
	if ( col % 3 != 0 )
	    add_buf( output, "\r\n" );
	page_to_char( buf_string(output), ch );
	return;
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;
	if ( level > get_trust( ch ) )
	    continue;
        if ( is_name(argall, pHelp->keyword) )
        {
            /* add seperator if found */
            if (found)
                add_buf(output,
"\n\r============================================================\n\r\n\r");
            if ( pHelp->level >= 0
	    &&   LOWER(argall[0]) == 'i'
	    &&   str_cmp( argall, "imotd" ) )
            {
                add_buf(output,pHelp->keyword);
                add_buf(output,"\n\r");
            }
         
            /* 
             * Strip leading '.' to allow initial blanks.
             */
            if ( pHelp->text[0] == '.' )
                add_buf(output,pHelp->text+1);
            else
                add_buf(output,pHelp->text);   
            found = TRUE;
            /* small hack :) */
            if (ch->desc != NULL && ch->desc->connected != CON_PLAYING
            &&                      ch->desc->connected != CON_GEN_GROUPS)
                break;
        }
	if ( found && !IS_CONFIG(ch,CONFIG_COMBHELP) )
	    break;
    }
            
    if (!found)
        send_to_char( "No help on that word.\n\r", ch );
    else
        page_to_char(buf_string(output),ch);
    free_buf(output);
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    BUFFER *output;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if ( !str_prefix(arg, FIRSTNAME(wch)) )
	{
	    found = TRUE;
	    class = "";

	    if ( IS_IMMORTAL(wch) )
	    {
	        switch ( wch->level )
	        {
		    default:
			class = "`1`B[ IMM ]";
			break;
		    case MAX_LEVEL - 0:
			class = "`7`B[ IMP ]";
			break;
	        }
	    }

   	    if ( is_guild_imm(wch) )
	        class = "`4`B[GUILD]";

	    if ( !IS_NPC(wch) && !is_guild_imm(wch)
	    &&   wch->pcdata->security > 0
	    &&   wch->level < L2 )
	        class = "`2`B[BUILD]";
	    
	    /*
	     * Format it up.
	     */

	    if ( IS_SET(wch->comm, COMM_AFK) )
	    {
                sprintf( buf, "`3`B[ AFK ]`n %s%s%s`n",
                   PERS(wch, ch),
                   LASTNAME(wch),
		   TITLE(wch) );
	    }
	    else
	    {
	        sprintf( buf, "%7s`n %s%s%s`n",
	            IS_DISGUISED(wch) ? "" : class,
                    PERS(wch, ch),
                    LASTNAME(wch),
		    TITLE(wch) );
	    }

	    if (IS_SET(wch->act, PLR_MARRIED))
	        strcat( buf, " `3<Married!>`n" );

	    if ( IS_IDLE(wch) )
	        strcat( buf, " `7`B<Idle>`n" );

            if ( IS_WRITING(wch) )
	        strcat( buf, " `B`7<Writing>`n" );

	    if ( IS_SET(wch->act, PLR_NEWBIEHELPER) )
		strcat( buf, " `%<Newbie Helper!>`n" );

	    if ( IS_SET(wch->act, PLR_QUESTING) )
		strcat( buf, " `@<Questing>`n" );

	    add_buf(output, buf);
	    add_buf(output, "\n\r");

	    if ( !IS_NPC(wch) && IS_SET(wch->act, PLR_MARRIED))
	    {
		char buf2[100];
		buf2[0] = '\0';
		if ( wch->pcdata->spouse == NULL)
		    sprintf( buf2, "        Married.\n\r");
		else
		    sprintf( buf2, "        Married to `4%s`n.\n\r",
			wch->pcdata->spouse);
		add_buf(output, buf2);
	    }
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);
    ansi_color( GREY,  ch );
    ansi_color( NTEXT, ch );
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int count;
    int rGuild;
    bool fGuildRestrict;
    bool fImmortalOnly;
    bool fHelperOnly;
    bool fQuestOnly;
    int wg, ag;
    char *invis;

    ag = guild_lookup("aes sedai");
    wg = guild_lookup("warder");

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fGuildRestrict = FALSE;
    fImmortalOnly  = FALSE;
    fHelperOnly    = FALSE;
    fQuestOnly     = FALSE;
    rGuild	   = 0;
    

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) && IS_IMMORTAL(ch) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{

	    /*
	     * Look for classes to turn on.
	     */
	    if ( arg[0] == 'i' )
	    {
		fImmortalOnly = TRUE;
	    }
	    else if ( !str_prefix(arg, "guild") )
	    {
		fGuildRestrict = TRUE;
		rGuild = ch->guild;
	    }
	    else if ( !str_prefix(arg, "quest") )
	    {
		fQuestOnly = TRUE;
	    }
	    else if ( !str_prefix(arg, "helper") )
	    {
		fHelperOnly = TRUE;
	    }
	    else if ( !str_prefix(arg, "aes sedai")
	    &&	      (ch->guild == wg
	    ||         ch->guild == ag) )
	    {
		fGuildRestrict = TRUE;
		rGuild = guild_lookup( "aes-sedai" );
	    }
	    else if ( !str_prefix(arg, "warder")
	    &&	      (ch->guild == wg
	    ||         ch->guild == ag) )
	    {
		fGuildRestrict = TRUE;
		rGuild = guild_lookup( "warder" );
	    }
	    else
	    {
		send_to_char( "The following arguments are allowed for who:\n\r", ch );
		send_to_char( "IMMORTAL - show all visible immortals.\n\r", ch );
		send_to_char( "GUILD    - show all members of your guild.\n\r", ch );
		send_to_char( "QUEST    - show all people involved with a quest.\n\r", ch );
		send_to_char( "HELPER   - show all newbie helpers\n\r", ch );
		if ( ch->guild == wg
		||   ch->guild == ag )
		{
		    send_to_char( "WARDER   - show all members of the Warder guild.\n\r", ch );
		    send_to_char( "AES SEDAI- show all members of the Aes Sedai guild.\n\r", ch );
		}
		return;
	    }
	}
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
    add_buf(output, " -- == The `1W`2e`3a`4v`5e`n Player list == --\n\r");
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *class;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	    continue;

	wch   = ( d->original != NULL ) ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_HERO )
	|| ( fGuildRestrict && rGuild != wch->guild )
	|| ( fHelperOnly && !IS_SET(wch->act, PLR_NEWBIEHELPER))
	|| ( fQuestOnly && !IS_SET(wch->act, PLR_QUESTING)) )
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class.
	 */
	class = "";

	if ( IS_IMMORTAL(wch) )
	{
	    switch ( wch->level )
	    {
		default:
			class = "`1`B[ IMM ]";
			break;
		case MAX_LEVEL - 0:
			class = "`7`B[ IMP ]";
			break;
	    }
	}

	if ( is_guild_imm(wch) )
	    class = "`4`B[GUILD]";

	if ( !IS_NPC(wch) && !is_guild_imm(wch)
	&&   wch->pcdata->security > 0
	&&   wch->level < L2 )
	    class = "`2`B[BUILD]";

	/*
	 * Format it up.
	 */

	if (IS_SET( wch->act, PLR_WIZINVIS)&&IS_IMMORTAL(ch))invis="`&+`n";
	else invis=" ";

	if ( IS_SET(wch->comm, COMM_AFK) )
	{
            sprintf( buf, "`3`B[ AFK ]`n%s%s%s%s`n",
		invis,
                PERS(wch,ch),
	        LASTNAME(wch),
		TITLE(wch) );
	}    
	else
	{
	    sprintf( buf, "%7s`n%s%s%s%s`n",
	        IS_DISGUISED(wch) ? "" : class,
		invis,
	        PERS(wch,ch),
	        LASTNAME(wch),
		TITLE(wch) );
	}
	if ( IS_IDLE(wch) )
	    strcat( buf, " `7`B<Idle>`n" );

	if ( IS_WRITING(wch) )
	    strcat( buf, " `7`B<Writing>`n" );

	if ( IS_SET(wch->act, PLR_NEWBIEHELPER) )
	    strcat( buf, " `%<Newbie Helper!>`n" );

	if ( IS_SET(wch->act, PLR_QUESTING) )
	    strcat( buf, " `@<Questing>`n" );

	add_buf(output, buf);
	add_buf(output, "\n\r");
    }
    sprintf( buf2, "`n\n\rPlayers found: `4%d`n\n\r", nMatch );

    add_buf(output,buf2);
    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf2,"There %s `4%d`n character%s on, the most so far today.\n\r",
	    ( count == 1 ) ? "is" : "are",
	    count,
	    ( count == 1 ) ? "" : "s" );
    else
	sprintf(buf2,"There %s `4%d`n character%s on, the most on today was `4%d`n.\n\r",
	    ( count == 1 ) ? "is" : "are",
	    count,
	    ( count == 1 ) ? "" : "s",
	    max_on);

    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );

    ansi_color( GREY,  ch );
    ansi_color( NTEXT, ch );
    free_buf(output);
    return;
}

void do_count( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There %s `4%d`n character%s on, the most so far today.\n\r",
	    ( count == 1 ) ? "is" : "are",
	    count,
	    ( count == 1 ) ? "" : "s" );
    else
	sprintf(buf,"There %s `4%d`n character%s on, the most on today was `4%d`n.\n\r",
	    ( count == 1 ) ? "is" : "are",
	    count,
	    ( count == 1 ) ? "" : "s",
	    max_on);

    send_to_char(buf,ch);
}


void do_inventory( CHAR_DATA *ch, char *argument )
{ 
/*
  extern char *buf;
*/
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *tch;

  tch = ch;

  if (IS_IMMORTAL(ch) && *argument) {
    if (( tch = get_char_world( ch, argument)) == NULL) {
      send_to_char("They aren't here.\r\n", ch);
      return;
    }
  }

  if( ch == tch)
    strcpy( buf, "You are");
  else
    sprintf( buf, "%s is", tch->name);

  sprintf( buf, "%s carrying:\n\r", buf);
  send_to_char( buf, ch);
  show_list_to_char( tch->carrying, ch, TRUE, TRUE );
  return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
/*
  extern char *buf;
*/
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int iWear;
  bool found;
  CHAR_DATA *tch;

  tch = ch;

    if (IS_IMMORTAL( ch) && *argument) {
      if (( tch = get_char_world( ch, argument)) == NULL) {
	send_to_char( "They aren't here.\r\n", ch);
	return;
      }
    }

    if( ch == tch)
      strcpy( buf, "You are");
    else
      sprintf( buf, "%s is", tch->name);

    sprintf( buf, "%s using:\n\r", buf);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( tch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    OBJ_DATA *obj3;
    int value1;
    int value2;
    int value3;
    int value4;  
    int value5;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    obj3	= NULL;
    msg		= NULL;
    value1	= 0;
    value2	= 0;
    value3	= 0;
    value4	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    obj3 = get_eq_char( ch, WEAR_SECONDARY );
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];

	    if ( obj3 )
	    {
		if (obj1->pIndexData->new_format)
		    value3 = (1 + obj1->value[2]) * obj1->value[1];
		else
	    	    value3 = obj1->value[1] + obj1->value[2];

		if (obj3->pIndexData->new_format)
		    value4 = (1 + obj3->value[2]) * obj3->value[1];
		else
	    	    value4 = obj3->value[1] + obj3->value[2];
	    }		
	    break;
	}
    }

    if ( msg == NULL )
    {
        value5 = value1 - value2;

        if ( value5 >= 10 )
	    msg = "$p looks many times better than $P.";
        else if ( value5 >= 5 && value5 < 10 )
	    msg = "$p looks better than $P."; 
        else if ( value5 >= 1 && value5 < 5 )
	    msg = "$p looks slightly better than $P."; 
        else if ( value5 == 0 )
	    msg = "$p and $P look about the same."; 
        else if ( value5 >= -4 && value5 < 0 )
	    msg = "$p looks slightly worse than $P."; 
        else if ( value5 >= -9 && value5 < -4 )
	    msg = "$p looks worse than $P."; 
        else
	    msg = "$p looks many times worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    if ( obj3 )
    {
	value5 = value3 - value4;

        if ( value5 >= 10 )
	    msg = "$p looks many times better than $P.";
        else if ( value5 >= 5 && value5 < 10 )
	    msg = "$p looks better than $P."; 
        else if ( value5 >= 1 && value5 < 5 )
	    msg = "$p looks slightly better than $P."; 
        else if ( value5 == 0 )
	    msg = "$p and $P look about the same."; 
        else if ( value5 >= -4 && value5 < 0 )
	    msg = "$p looks slightly worse than $P."; 
        else if ( value5 >= -9 && value5 < -4 )
	    msg = "$p looks worse than $P."; 
        else
	    msg = "$p looks many times worse than $P.";
	act( msg, ch, obj1, obj3, TO_CHAR );
    }
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "You may not consider players.\n\r", ch );
	return;
    }
    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    act( "$n look$% over $N carefully.", ch, NULL, victim, TO_ALL );

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";
    act( msg, ch, NULL, victim, TO_CHAR );

    if ( !can_channel(ch, 1)
    ||   !can_channel(victim,1)
    ||   IS_AFFECTED_2(victim, AFF_HIDE_CHANNEL) )
	return;

    if ( TRUE_SEX(ch) != TRUE_SEX(victim) )
	return;

    diff = channel_strength(victim, POWER_ALL)
         - channel_strength(ch, POWER_ALL);

         if ( diff <= -20 ) msg = "You are much stronger than $N.";
    else if ( diff <= -10 ) msg = "You are stronger than $N.";
    else if ( diff <=  -5 ) msg = "You are a bit stronger than $N.";
    else if ( diff <=  -2 ) msg = "You are a fraction stronger than $N.";
    else if ( diff <=   1 ) msg = "You are about the same strength as $N.";
    else if ( diff <=   4 ) msg = "You are just a little weaker than $N.";
    else if ( diff <=   9 ) msg = "$N is a bit stronger than you.";
    else if ( diff <=  19 ) msg = "You are much weaker than $N.";
    else                    msg = "$N is much more powerful than you.";
    act( msg, ch, NULL, victim, TO_CHAR );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' && title[0] != '-' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    if ( IS_DISGUISED(ch) )
    {
	free_string( ch->pcdata->new_title );
	ch->pcdata->new_title = str_dup( buf );
    }
    else
    {
	free_string( ch->pcdata->title );
	ch->pcdata->title = str_dup( buf );
    }
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    char buf[1024];
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( IS_DISGUISED(ch) )
	{
	    free_string( ch->pcdata->new_title );
	    ch->pcdata->new_title = str_dup( "" );
	    sprintf( buf, "%s%s%s.\n\r", ch->pcdata->new_name, 
		ch->pcdata->new_last,
		ch->pcdata->new_title );
	    send_to_char( "Ok.  You are known as ", ch );
	    send_to_char( buf, ch );
	    return;
	}
	free_string( ch->pcdata->title );
	ch->pcdata->title = str_dup( "" );
	sprintf( buf, "%s%s%s.\n\r", ch->name, 
	    ch->pcdata->last_name,
	    ch->pcdata->title );
	send_to_char( "Ok.  You are known as ", ch );
	send_to_char( buf, ch );
	return;
    }

    if ( strlen(argument) > 45 )
    {
	argument[43] = '`';
	argument[44] = 'n';
	argument[45] = '\0';
    }
    else
	strcat( argument, "`n" );

    smash_tilde( argument );
    set_title( ch, argument );

    if ( IS_DISGUISED(ch) )
    {
	sprintf( buf, "%s%s%s.\n\r", ch->pcdata->new_name, 
	    ch->pcdata->new_last,
	    ch->pcdata->new_title );
    }
    else
    {
	sprintf( buf, "%s%s%s.\n\r", ch->name, 
	    ch->pcdata->last_name,
	    ch->pcdata->title );
    }
    send_to_char( "Ok.  You are known as ", ch );
    send_to_char( buf, ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You cannot write your description while fighting.\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	act( "$n begins to write something.", ch, NULL, NULL, TO_ROOM );
	if ( IS_DISGUISED(ch) )
	    string_append( ch, &ch->pcdata->new_desc );
	else
	    string_append( ch, &ch->description );
	return;
    }
    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );

    send_to_char( "Syntax:  description\n\r", ch );
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    int xp_to_level;

    xp_to_level = (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp;

    sprintf( buf,
	"You say 'I have %d/%d hp, %d/%d stamina, %d xp and %d xp to next level.'\n\r",
	ch->hit,  ch->max_hit,
	ch->stamina, ch->max_stamina,
	ch->exp, xp_to_level   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp, %d/%d stamina, %d xp and %d xp to next level.'",
	ch->hit,  ch->max_hit,
	ch->stamina, ch->max_stamina,
	ch->exp, xp_to_level   );

    act( buf, ch, NULL, NULL, TO_GROUP );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH*4];
    int sn;

    if ( IS_NPC(ch) )
	return;

    buf1[0] = '\0';
    buf[0] = '\0';
    if ( argument[0] == '\0' )
    {
	CHAR_DATA *mob;
	int col, trainer;

	trainer = 0;
	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		trainer = 1;
	}

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    char letter;

	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class] 
	      || (skill_table[sn].rating[ch->class] < 1 && !IS_IMMORTAL(ch) ) 
	      || ch->pcdata->learned[sn] < 0 )  /* Skill not gained */
		continue;

	    if ( ch->pcdata->learned[sn] < 1 && trainer == 0 ) /* Skill gained, */
		continue;				      /* but no trainer */
							      /* around */
	    switch (abs(skill_table[sn].rating[ch->class]))
	    {
		case 1:
		case 2:
			letter = 'E';
			break;
		case 3:
		case 4:
			letter = 'M';
			break;
		case 5:
		case 6:
			letter = 'D';
			break;
		default:
			letter = 'I';
			break;
	    }
	    sprintf( buf, "%-16.16s %3d%%[%c]  ",
		skill_table[sn].name, SKILL(ch,sn), letter );
	    strcat( buf1, buf );
	    if ( ++col % 3 == 0 )
		strcat( buf1, "\n\r" );
	}

	if ( col % 3 != 0 )
	    strcat( buf1, "\n\r" );

	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->practice );
	strcat( buf1, buf );
	page_to_char( buf1, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;
	int count;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	count = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( ch->pcdata->learned[sn] > -1
	    &&   skill_table[sn].spell_fun == spell_null )
		count++;
	}


	if ( ( sn = skill_lookup( argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->class] 
 	||    ch->pcdata->learned[sn] < 0 /* skill is not known */
	||    skill_table[sn].rating[ch->class] < 1 )))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	if ( skill_table[sn].spell_fun != spell_null )
	{
	    if ( !IS_SET(mob->act, ACT_CHANNELER) )
	    {
		send_to_char( "You can't practice that.\n\r", ch );
		return;
	    }

	    if ( !can_cast(mob, sn) )
	    {
		act( "$N tells you 'I don't know those flows.'",
		    ch, NULL, mob, TO_CHAR );
		ch->reply = mob;
		return;
	    }
	}

	if ( skill_table[sn].prerequisite != NULL )
	{
	    int pre;

	    pre = skill_lookup( skill_table[sn].prerequisite );
	    if ( pre != -1 && ch->pcdata->learned[pre] < 70 )
	    {
		act( "$N tells you 'You must learn more of $t first.'",
		    ch, skill_table[pre].name, mob, TO_CHAR );
		ch->reply = mob;
		return;
	    }
	}
	if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	{
	    act( "$N tells you 'You have no need of that.'",
		ch, NULL, mob, TO_CHAR );
	    ch->reply = mob;
	    return;
	}

	adept = IS_NPC(ch) ? 100 :
	    ( 72 - abs((skill_table[sn].rating[ch->class]) * 2) );

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		(int_app[get_curr_stat(ch,STAT_INT)].learn * 2 / 
	        abs(skill_table[sn].rating[ch->class]) ) - count / 10;
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}


void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 10 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT configure command SMASHED */

void do_affects( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    send_to_char( "You are affected by:\n\r", ch );
    if ( ch->affected != NULL )
    {
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    sprintf( buf, "  `4%-s`n", skill_table[paf->type].name );
	    send_to_char( buf, ch );

	    if ( ch->level >= 30 )
	    {
		if ( paf->modifier != 0 )
		{
		    sprintf( buf,
			" modifies `4%s`n by `4%d`n for `4%d`n minute%s",
			affect_loc_name( paf->location ),
		 	paf->modifier,
			paf->duration + 1,
			(paf->duration+1 != 1) ? "s" : ""  );
		}
		else
		{
		    sprintf( buf, " will last for `4%d`n minute%s",
			paf->duration + 1,
			(paf->duration+1 != 1) ? "s" : "" );
		}
		send_to_char( buf, ch );

		if ( paf->bitvector != 0
		||   paf->bitvector_2 != 0 )
		{
		    send_to_char( " and sets`4", ch );
		    send_to_char( bit_name(paf), ch );
		}
	    }
	    send_to_char( "`n.", ch );
	    if ( !IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    {
		if ( IS_TIED(paf) )
		    send_to_char( "  It is tied off.", ch );
		else
		    send_to_char( "  It is not tied off.", ch );
	    }
	    send_to_char( "\n\r", ch );
	}
    }
    else
	send_to_char( "  You have no affects on you.\n\r", ch );
    if ( IS_SET(ch->body, BODY_RIGHT_LEG)
    ||   IS_SET(ch->body, BODY_LEFT_LEG) )
	send_to_char_new(ch, "Your %s%s%s %s broken.\n\r",
	    IS_SET(ch->body, BODY_LEFT_LEG) ? "left leg" : "",
	    (IS_SET(ch->body, BODY_RIGHT_LEG) && IS_SET(ch->body, BODY_LEFT_LEG)) ?
	    " and " : "",
	    IS_SET(ch->body, BODY_RIGHT_LEG) ? "right leg" : "",
	    (IS_SET(ch->body, BODY_RIGHT_LEG) && IS_SET(ch->body, BODY_LEFT_LEG)) ?
	    "are" : "is" );
    if ( IS_SET(ch->body, BODY_BLEEDING) )
	send_to_char_new( ch, "You are bleeding badly.\n\r" );
}


void do_last( CHAR_DATA *ch, char *argument )
{
    char buf[1024];
    char buf2[1024];
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( IS_DISGUISED(ch) )
	{
	    free_string( ch->pcdata->new_last );
	    ch->pcdata->new_last = str_dup( "" );
	    sprintf( buf, "%s%s%s.\n\r", ch->pcdata->new_name, 
		ch->pcdata->new_last,
		ch->pcdata->new_title );
	}
	else
	{
	    free_string( ch->pcdata->last_name );
	    ch->pcdata->last_name = str_dup( "" );
	    sprintf( buf, "%s%s%s.\n\r", ch->name, 
		ch->pcdata->last_name,
		ch->pcdata->title );
	}
	send_to_char( "Ok.  You are known as ", ch );
	send_to_char( buf, ch );
	return;
    }

    if ( strlen(argument) > 15 )
	argument[15] = '\0';

    smash_tilde( argument );

    buf2[0] = ' ';
    strcpy( buf2+1, argument );

    if ( IS_DISGUISED(ch) )
    {
	free_string( ch->pcdata->new_last );
	ch->pcdata->new_last = str_dup( buf2 );

	sprintf( buf, "%s%s%s.\n\r", ch->pcdata->new_name, 
	    ch->pcdata->new_last,
	    ch->pcdata->new_title );
    }
    else
    {
	free_string( ch->pcdata->last_name );
	ch->pcdata->last_name = str_dup( buf2 );

	sprintf( buf, "%s%s%s.\n\r", ch->name, 
	    ch->pcdata->last_name,
	    ch->pcdata->title );
    }
    send_to_char( "Ok.  You are known as ", ch );
    send_to_char( buf, ch );
}


void do_combat( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Combat options:\n\r", ch );
	send_to_char( "TANKCOND     ", ch );
	if ( IS_SET(ch->comm, COMM_TANKCOND) )
	    send_to_char( "`2You see tank status after every combat round.`n\n\r", ch );
	else
	    send_to_char( "`3You do not see tank status after every combat round.`n\n\r", ch );
	send_to_char( "AUTOHEAL     ", ch );
	if ( IS_SET(ch->comm, COMM_AUTOHEAL) )
	    send_to_char( "`2You autoheal group members.`n\n\r", ch );
	else
	    send_to_char( "`3You do not autoheal group members.`n\n\r", ch );
	send_to_char( "NOSPAM       ", ch );
	if ( IS_SET(ch->comm, COMM_NOSPAM) )
	    send_to_char( "`2You do not see combat messages.`n\n\r", ch );
	else
	    send_to_char( "`3You see combat messages.`n\n\r", ch );
	send_to_char( "NOTICK       ", ch );
	if ( IS_SET(ch->comm, COMM_NOTICK) )
	    send_to_char( "`2You do not see the combat timer.`n\n\r", ch );
	else
	    send_to_char( "`3You see the combat timer.`n\n\r", ch );
	send_to_char( "SUBDUE       ", ch );
	if ( IS_SET(ch->act, PLR_SUBDUE) )
	    send_to_char( "`2You are subdued in player kills.`n\n\r", ch );
	else
	    send_to_char( "`3You are killed in player kills.`n\n\r", ch );
	send_to_char( "FOECOND      ", ch );
	if ( IS_SET(ch->act, PLR_FOECOND) )
	    send_to_char( "`2You see your foe's condition each update.`n\n\r", ch );
	else
	    send_to_char( "`3You do not see your foe's condition each update.`n\n\r", ch );
	return;
    }

    if ( !str_prefix( argument, "tankcond" ) )
    {
	if ( IS_SET(ch->comm, COMM_TANKCOND) )
	{
	    REMOVE_BIT(ch->comm, COMM_TANKCOND);
	    send_to_char( "You no longer see the tank's condition.\n\r", ch );
	}
	else
	{
	    SET_BIT(ch->comm, COMM_TANKCOND);
	    send_to_char( "You now see the tank's condition.\n\r", ch );
	}
	return;
    }

    if ( !str_prefix( argument, "autoheal" ) )
    {
        if ( IS_SET(ch->comm, COMM_AUTOHEAL) )
        {
            REMOVE_BIT(ch->comm, COMM_AUTOHEAL);
            send_to_char( "You no longer autoheal group members.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->comm, COMM_AUTOHEAL);
            send_to_char( "You now autoheal group members.\n\r", ch );
        }
        return;
    }

    if ( !str_prefix( argument, "autoheal" ) )
    {
        if ( IS_SET(ch->comm, COMM_AUTOHEAL) )
        {
            REMOVE_BIT(ch->comm, COMM_AUTOHEAL);
            send_to_char( "You no longer autoheal group members.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->comm, COMM_AUTOHEAL);
            send_to_char( "You now autoheal group members.\n\r", ch );
        }
        return;
    }

    if ( !str_prefix( argument, "nospam" ) )
    {
        if ( IS_SET(ch->comm, COMM_NOSPAM) )
        {
            REMOVE_BIT(ch->comm, COMM_NOSPAM);
            send_to_char( "You now see combat messages.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->comm, COMM_NOSPAM);
            send_to_char( "You no longer see combat messages.\n\r", ch );
        }
        return;
    }

    if ( !str_prefix( argument, "notick" ) )
    {
        if ( IS_SET(ch->comm, COMM_NOTICK) )
        {
            REMOVE_BIT(ch->comm, COMM_NOTICK);
            send_to_char( "You now see the combat timer.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->comm, COMM_NOTICK);
            send_to_char( "You no longer see the combat timer.\n\r", ch );
        }
        return;
    }

    if ( !str_prefix( argument, "subdue" ) )
    {
        if ( IS_SET(ch->act, PLR_SUBDUE) )
        {
            REMOVE_BIT(ch->act, PLR_SUBDUE);
            send_to_char( "You will now be killed in player kills.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->act, PLR_SUBDUE);
            send_to_char( "You will now be subdued in player kills.\n\r", ch );
        }
        return;
    }

    if ( !str_prefix( argument, "foecond" ) )
    {
        if ( IS_SET(ch->act, PLR_FOECOND) )
        {
            REMOVE_BIT(ch->act, PLR_FOECOND);
            send_to_char( "You will no longer see your foe's condition each update.\n\r", ch );
        }
        else
        {
            SET_BIT(ch->act, PLR_FOECOND);
            send_to_char( "You will now see your foe's condition each update.\n\r", ch );
        }
        return;
    }

    do_combat( ch, "" );
    return;
}


void warder_scan( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *found_chars;
    int exit_num;
    bool found = FALSE;

    for (found_chars = ch->in_room->people;
	 found_chars != NULL;
	 found_chars = found_chars->next_in_room)
    {
	if ( found_chars->race == race_lookup("myrddraal")
	||   found_chars->race == race_lookup("trolloc")
	||   found_chars->race == race_lookup("dragkhar") )
	    found = TRUE;

	if ( !str_cmp(found_chars->name, "Shaidar")
	&&   can_see(ch, found_chars) )
	    found = TRUE;
    }
    sprintf( buf, "You sense shadowspawn here.\n\r" );
    if ( found )
	send_to_char( buf, ch );

    for (exit_num = 0; exit_num < 6; exit_num++)
    {
	if (ch->in_room->exit[exit_num] != NULL)
	{
	    if ( ch->in_room->exit[exit_num]->u1.to_room == NULL )
		continue;

	    found = FALSE;
	    for (found_chars = ch->in_room->exit[exit_num]->u1.to_room->people;
		 found_chars != NULL;
		 found_chars = found_chars->next_in_room)
	    {

		if ( found_chars->race == race_lookup("myrddraal")
		||   found_chars->race == race_lookup("trolloc")
		||   found_chars->race == race_lookup("dragkhar") )
		    found = TRUE;

		if ( !str_cmp(found_chars->name, "Shaidar")
		&&   can_see(ch, found_chars) )
		    found = TRUE;
	    }
	    sprintf( buf, "You sense shadowspawn %swards.\n\r",
		dir_name[exit_num] );
	    if ( found )
		send_to_char( buf, ch );
	}
    }
}

void recurse_scan( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, sh_int door, sh_int count )
{
    CHAR_DATA *vch;

    count++;

    if ( room_is_dark(pRoom) )
    {
	char buf[MAX_INPUT_LENGTH];
	sprintf( buf, "  `4It's too dark %s.`n",
	    count == 1 ? "nearby" :
	    count == 2 ? "a short distance away" :
	    "a bit of a distance away" );
	act( buf, ch, NULL, NULL, TO_CHAR );
	return;
    }

    for (vch = pRoom->people; vch != NULL; vch = vch->next_in_room)
    {
	char buf[MAX_INPUT_LENGTH];

	if ( !can_see( ch, vch ) )
	    continue;

	sprintf( buf, "  `4$N is %s.`n",
	    count == 1 ? "nearby" :
	    count == 2 ? "a short distance away" :
	    "a bit of a distance away" );
	act( buf, ch, NULL, 
	    vch, TO_CHAR );
    }

    if ( pRoom->exit[door] != NULL
    &&	 IS_SET(pRoom->exit[door]->exit_info, EX_CLOSED) )
    {
	char buf[MAX_INPUT_LENGTH];
	sprintf( buf, "  `4A closed $d is %s.`n",
	    count == 1 ? "nearby" :
	    count == 2 ? "a short distance away" :
	    "a bit of a distance away" );
	act( buf, ch, NULL, 
	    pRoom->exit[door]->keyword, TO_CHAR );
	return;
    }

    if ( IS_SET(pRoom->room_flags, ROOM_FOG) )
    {
	char buf[MAX_INPUT_LENGTH];
	sprintf( buf, "  `4There is too much fog %s to see through.`n",
	    count == 1 ? "nearby" :
	    count == 2 ? "a short distance away" :
	    "a bit of a distance away" );
	act( buf, ch, NULL, NULL , TO_CHAR );
	return;
    }

    if ( pRoom->exit[door] != NULL
    &&   !IS_SET(pRoom->room_flags, ROOM_FOG)
    &&   count < 3 )
	recurse_scan( ch, pRoom->exit[door]->u1.to_room, door, count );

    return;
}

void show_char_affect( CHAR_DATA *ch, CHAR_DATA *victim )
{
    AFFECT_DATA *paf;

    if ( !can_channel(ch, 1) )
	return;

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	char buf[MAX_STRING_LENGTH];
	if ( !SAME_SEX(paf, ch) )
	    continue;

	if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    continue;

	if ( IS_INVERTED(paf) && ch != paf->owner )
	    continue;

	if ( paf->next && paf->type == paf->next->type )
	    continue;

	if ( paf->owner == ch )
	    sprintf( buf, "You have placed a weave of $t on $N " );
	else if ( paf->owner == NULL )
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t on $N " );
	    else
		sprintf( buf, "Someone has placed a strange weave on $N " );
	}
	else
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t on $N " );
	    else
		sprintf( buf, "Someone has placed a strange weave on $N " );
	}

	if ( IS_TIED(paf) )
	    strcat( buf, "and tied it off." );
	else
	    strcat( buf, "without tying it off." );
	act( buf, ch, skill_table[paf->type].name, victim, TO_CHAR );
    }

    return;
}

void show_obj_affect( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;

    if ( !can_channel(ch, 1) )
	return;

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	char buf[MAX_STRING_LENGTH];
	if ( !SAME_SEX(paf, ch) )
	    continue;

	if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    continue;

	if ( IS_INVERTED(paf) && ch != paf->owner )
	    continue;

	if ( paf->owner == ch )
	    sprintf( buf, "You have placed a weave of $t on $P " );
	else if ( paf->owner == NULL )
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t on $P " );
	    else
		sprintf( buf, "Someone has placed a strange weave on $P " );
	}
	else
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t on $P " );
	    else
		sprintf( buf, "Someone has placed a strange weave on $P " );
	}

	if ( IS_TIED(paf) )
	    strcat( buf, "and tied it off." );
	else
	    strcat( buf, "without tying it off." );
	act( buf, ch, skill_table[paf->type].name, obj, TO_CHAR );
    }

    return;
}

void show_room_affect( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
    AFFECT_DATA *paf;

    if ( !can_channel(ch, 1) )
	return;

    for ( paf = room->affected; paf != NULL; paf = paf->next )
    {
	char buf[MAX_STRING_LENGTH];
	if ( !SAME_SEX(paf, ch) )
	    continue;

	if ( IS_SET(paf->flags, AFFECT_NOTCHANNEL) )
	    continue;

	if ( IS_INVERTED(paf) && ch != paf->owner )
	    continue;

	if ( paf->owner == ch )
	    sprintf( buf, "You have placed a weave of $t here " );
	else if ( paf->owner == NULL )
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t here " );
	    else
		sprintf( buf, "Someone has placed a strange weave here " );
	}
	else
	{
	    if ( get_skill(ch, paf->type) >= number_percent() )
		sprintf( buf, "Someone has placed a weave of $t here " );
	    else
		sprintf( buf, "Someone has placed a strange weave here " );
	}

	if ( IS_TIED(paf) )
	    strcat( buf, "and tied it off." );
	else
	    strcat( buf, "without tying it off." );
	act( buf, ch, skill_table[paf->type].name, NULL, TO_CHAR );
    }

    return;
}


void do_armor( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *tch;
    int i;
    int total[7];

    tch = ch;

    if (IS_IMMORTAL(ch) && *argument)
    {
	if ( ( tch = get_char_world( ch, argument ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
    }

    if ( (obj = get_eq_char( tch, WEAR_NECK_1 )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Neck (spot 1)    "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }
    if ( (obj = get_eq_char( tch, WEAR_NECK_2 )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Neck (spot 2)    "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_BODY )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Body             "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0] * 5, total[1] * 5, total[2] * 5, total[3] * 5 );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_HEAD )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Head             "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0] * 2, total[1] * 2, total[2] * 2, total[3] * 2 );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_LEGS )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Legs             "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0] * 3, total[1] * 3, total[2] * 3, total[3] * 3 );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_FEET )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Feet             "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_HANDS )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Hands            "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_ARMS )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Arms             "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0] * 3, total[1] * 3, total[2] * 3, total[3] * 3 );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_WAIST )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Waist            "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0] * 2, total[1] * 2, total[2] * 2, total[3] * 2 );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_WRIST_L )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Wrist (left)     "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    if ( (obj = get_eq_char( tch, WEAR_WRIST_R )) != NULL )
    {
	for ( i= 0; i < 7; i++ )
	{
	    if ( obj->item_type == ITEM_ARMOR )
		total[i] = obj->value[i];
	    else
		total[i] = 0;
	}
	send_to_char_new( ch, "Wrist (right)    "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	    total[0], total[1], total[2], total[3] );
	send_to_char_new( ch, "                 Cold: %2d"
			      "  Electricity: %2d"
			      "  Other: %2d\n\r",
	    total[4], total[5], total[6] );
    }

    send_to_char_new( ch, "Affect Bonus     "
			      "Piercing: %2d"
			      "  Bashing: %2d"
			      "  Slashing: %2d"
		              "  Fire: %d\n\r",
	ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3] );
    send_to_char_new( ch, "                 Cold: %2d"
			  "  Electricity: %2d"
			  "  Other: %2d\n\r",
	ch->armor[4], ch->armor[5], ch->armor[6] );
    return;
}


void do_channeling( CHAR_DATA *ch, char *argument )
{
    if ( !can_channel(ch, 1) )
    {
	send_to_char( "You cannot channel.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED_2(ch, AFF_STILL) )
    {
	send_to_char( "You have been severed from the True Source!\r\n", ch );
	return;
    }
    send_to_char_new( ch, "Earth:  `4%3d`n", SKILL(ch,gsn_earth) );
    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
	send_to_char_new( ch, "(`4%3d`n) ",
	    channel_strength(ch, POWER_EARTH) );
    else
	send_to_char( "      ", ch );
    if ( get_skill(ch, gsn_earth) >= ch->channel_max[0] )
	send_to_char( "You have reached your potential in earth.\n\r", ch );
    else
	send_to_char( "You have not reached your potential in earth.\n\r", ch );

    send_to_char_new( ch, "Air:    `4%3d`n", SKILL(ch,gsn_air) );
    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
	send_to_char_new( ch, "(`4%3d`n) ", 
	    channel_strength(ch, POWER_AIR) );
    else
	send_to_char( "      ", ch );
    if ( get_skill(ch, gsn_air) >= ch->channel_max[1] )
	send_to_char( "You have reached your potential in air.\n\r", ch );
    else
	send_to_char( "You have not reached your potential in air\n\r", ch );

    send_to_char_new( ch, "Fire:   `4%3d`n", SKILL(ch,gsn_fire) );
    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
	send_to_char_new( ch, "(`4%3d`n) ", 
	    channel_strength(ch, POWER_FIRE) );
    else
	send_to_char( "      ", ch );
    if ( get_skill(ch, gsn_fire) >= ch->channel_max[2] )
	send_to_char( "You have reached your potential in fire.\n\r", ch );
    else
	send_to_char( "You have not reached your potential in fire.\n\r", ch );

    send_to_char_new( ch, "Water:  `4%3d`n", SKILL(ch,gsn_water) );
    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
	send_to_char_new( ch, "(`4%3d`n) ", 
	    channel_strength(ch, POWER_WATER) );
    else
	send_to_char( "      ", ch );
    if ( get_skill(ch, gsn_water) >= ch->channel_max[3] )
	send_to_char( "You have reached your potential in water.\n\r", ch );
    else
	send_to_char( "You have not reached your potential in water.\n\r", ch );

    send_to_char_new( ch, "Spirit: `4%3d`n", SKILL(ch,gsn_spirit) );
    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
	send_to_char_new( ch, "(`4%3d`n) ",
	    channel_strength(ch, POWER_SPIRIT) );
    else
	send_to_char( "      ", ch );
    if ( get_skill(ch, gsn_spirit) >= ch->channel_max[4] )
	send_to_char( "You have reached your potential in spirit.\n\r", ch );
    else
	send_to_char( "You have not reached your potential in spirit.\n\r", ch );

    send_to_char_new( ch, "You may put an extra `4%d%%`n stamina into your weavings.\n\r", 
	channel_strength(ch, POWER_ALL) * 3 / 2 );
    if ( IS_GRASPING(ch) )
	send_to_char( "You are grasping the One Power.\n\r", ch );
    return;
}

void do_doing( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	free_string(ch->pcdata->doing);
	ch->pcdata->doing = NULL;
	send_to_char( "Ok.  You're not doing anything special.\n\r", ch );
	return;
    }

    smash_tilde( argument );

    free_string( ch->pcdata->doing );
    ch->pcdata->doing = str_dup( argument );
    send_to_char( "Ok.  Now you are ", ch );
    send_to_char( ch->pcdata->doing, ch );
    send_to_char( ".\n\r", ch );
}


void do_wearing( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	free_string(ch->pcdata->wearing);
	ch->pcdata->wearing = NULL;
	send_to_char( "Ok.  You're not wearing anything special.\n\r", ch );
	return;
    }

    smash_tilde( argument );

    free_string( ch->pcdata->wearing );
    ch->pcdata->wearing = str_dup( argument );
    send_to_char( "Ok.  Now you are wearing ", ch );
    send_to_char( ch->pcdata->wearing, ch );
    send_to_char( ".\n\r", ch );
}


