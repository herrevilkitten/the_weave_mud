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

typedef	struct	parryforms	PARRY_FORM;
typedef	struct	attackforms	ATTACK_FORM;
typedef	struct	specialforms	SPECIAL_FORM;

struct	parryforms
{
    char *	message;
    int		bonus;
};

struct	attackforms
{
    char *	message;
    int		bonus;
    int		location;
};

struct	specialforms
{
    char *	message;
    int		bonus;
    int		location;
};

int form_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    const	PARRY_FORM	parry	[]	=
    {
	{
	    "$N circle$% $S blade around $o, parrying the attack.",
	    15
	},
	{
	    "$N trail$^ $S blade downward$^ from head level, idly deflecting $o attack.",
	    20
	},
	{
	    "$N trail$^ $S blade downward$^ about chest level, idly deflecting $o attack.",
	    20
	}
    };
}
