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

void check_taveren( CHAR_DATA *ch )
{
    int roll, flag = 0;

    if ( IS_NPC(ch) )
	return;

    if ( ch->pcdata->talent[tn_weak_taveren] )
	flag += 1;
    if ( ch->pcdata->talent[tn_taveren] )
	flag += 2;
    if ( ch->pcdata->talent[tn_strong_taveren] )
	flag += 4;

    if ( flag == 1 || flag == 2 || flag == 4 )
	return;

    /* More than one ta'veren bit */

    roll = number_percent( );
    switch( flag )
    {
	default:
	    break;
	case 3:
	    if ( roll <= 66 )
		ch->pcdata->talent[tn_taveren] = FALSE;
	    else
		ch->pcdata->talent[tn_weak_taveren] = FALSE;
	    break;
	case 5:
	    if ( roll <= 83 )
		ch->pcdata->talent[tn_strong_taveren] = FALSE;
	    else
		ch->pcdata->talent[tn_weak_taveren] = FALSE;
	    break;
	case 6:
	    if ( roll <= 66 )
		ch->pcdata->talent[tn_strong_taveren] = FALSE;
	    else
		ch->pcdata->talent[tn_taveren] = FALSE;
	    break;
	case 7:
	    if ( roll <= 66 )
	    {
		ch->pcdata->talent[tn_strong_taveren] = FALSE;
		ch->pcdata->talent[tn_taveren] = FALSE;
	    }
	    else if ( roll <= 83 )
	    {
		ch->pcdata->talent[tn_weak_taveren] = FALSE;
		ch->pcdata->talent[tn_strong_taveren] = FALSE;
	    }
	    else
	    {
		ch->pcdata->talent[tn_taveren] = FALSE;
		ch->pcdata->talent[tn_weak_taveren] = FALSE;
	    }
	    break;
    }
    return;
}
