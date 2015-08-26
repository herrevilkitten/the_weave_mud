/* Healer code written for Merc 2.0 muds by Alander 
   direct questions or comments to rtaylor@cie-2.uoregon.edu
   any use of this code must include this header */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob, *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int cost;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg1);
    victim = ch;

    if ( arg1[0] != '\0' && (victim = get_char_room(mob, arg1)) == NULL )
    {
	act( "$N says 'I don't see that person here.'", ch, NULL,
	    mob, TO_CHAR );
	return;
    }

    if (arg[0] == '\0')
    {
        /* display price list */
	act("$N says 'I offer the following aid:'",ch,NULL,mob,TO_CHAR);
	send_to_char("  light: cure light wounds      1300 gold\n\r",ch);
	send_to_char("  serious: cure serious wounds  1900 gold\n\r",ch);
	send_to_char("  critic: cure critical wounds  2800 gold\n\r",ch);
	send_to_char("  blind: cure blindness         2300 gold\n\r",ch);
	send_to_char("  disease: cure disease         1800 gold\n\r",ch);
	send_to_char("  poison: cure poison           2800 gold\n\r",ch); 
	send_to_char("  refresh: restore movement      800 gold\n\r",ch);
	send_to_char("  bleeding: stop bleeding       2500 gold\n\r",ch);
	send_to_char("  bones: set limbs              1700 gold\n\r",ch);
	send_to_char(" Type heal <type> to be healed.\n\r",ch);
	return;
    }

    switch (arg[0])
    {
	case 'l' :
	    cost  = 1300;
	    if ( ch->level <= 10 )
		cost = 0;
	    break;

	case 's' :
	    cost  = 1900;
	    break;

	case 'c' :
	    cost  = 2800;
	    break;

	case 'b' :
	    if ( !str_prefix(arg, "blind") )
	    {
		cost  = 2300;
		if ( ch->level <= 10 )
		    cost = 0;
		break;
	    }
	    if ( !str_prefix(arg, "bleeding") )
	    {
		cost  = 2500;
		if ( ch->level <= 10 )
		    cost = 0;
		break;
	    }
	    if ( !str_prefix(arg, "bones") )
	    {
		cost  = 1700;
		if ( ch->level <= 10 )
		    cost = 0;
		break;
	    }
	    act("$N says 'Type 'heal' for a list of healing.'",
	        ch,NULL,mob,TO_CHAR);
	    return;

    	case 'd' :
	    cost  = 1800;
	    if ( ch->level <= 10 )
		cost = 0;
	    break;

	case 'p' :
	    cost  = 2800;
	    if ( ch->level <= 10 )
		cost = 0;
	    break;
	
	case 'r' :
	    cost  = 800;
	    if ( ch->level <= 10 )
		cost = 0;
	    break;

	default :
	    act("$N says 'Type 'heal' for a list of healing.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (cost > ch->gold)
    {
	act("$N says 'You do not have enough gold for my services.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch, 2);

    ch->gold  -= cost;
    mob->gold += cost;

    switch (arg[0])
    {
	case 'l' :
	    act( "$N looks $n over and quickly administers a foul-smelling, green liquid to $m.",
		victim, NULL, mob, TO_ALL );
	    gain_health( victim, dice(1, 10), FALSE );
	    break;

	case 's' :
	    act( "$N looks $n over and quickly administers a foul-smelling, red and orange liquid to $m.",
		victim, NULL, mob, TO_ALL );
	    gain_health( victim, dice(3, 10), FALSE );
	    break;

	case 'c' :
	    act( "$N looks $n over and quickly administers a foul-smelling, black liquid to $m.",
		victim, NULL, mob, TO_ALL );
	    gain_health( victim, dice(6, 10), FALSE );
	    break;

	case 'b' :
	    if ( !str_prefix(arg, "blind") )
	    {
		act( "$N squints into $o eyes, nodding, then pours some water into them.",
		    victim, NULL, mob, TO_ALL );
		cure_condition( victim, BODY_BLIND, 100 );
	    }
	    if ( !str_prefix(arg, "bleeding") )
	    {
		act( "$N carefully bandage$% $o cuts.",
		    victim, NULL, mob, TO_ALL );
		cure_condition( victim, BODY_BLEEDING, 100 );
	    }
	    if ( !str_prefix(arg, "bones") )
	    {
		act( "$N carefully set$^ $o broken limbs.",
		    victim, NULL, mob, TO_ALL );
		cure_condition( victim, BODY_RIGHT_LEG, 100 );
		cure_condition( victim, BODY_LEFT_LEG, 100 );
		cure_condition( victim, BODY_RIGHT_ARM, 100 );
		cure_condition( victim, BODY_LEFT_ARM, 100 );
	    }
	    break;

    	case 'd' :
	    act( "$N pulls out $o tongue, then forces some strange pills down $S throat.",
		victim, NULL, mob, TO_ALL );
	    cure_condition( victim, BODY_DISEASE, 100 );
	    break;

	case 'p' :
	    act( "$N hands some strange pills to $n, then forces $m to swallow them.",
		victim, NULL, mob, TO_ALL );
	    cure_condition( victim, BODY_POISON, 100 );
	    break;
	
	case 'r' :
	    act( "$N knocks $n on the head, then forces $m to rest.",
		victim, NULL, mob, TO_ALL );
	    gain_stamina( victim, dice(2, 10), FALSE );
	    break;

	default :
	    act("$N says 'Type 'heal' for a list of healing.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }    
}
