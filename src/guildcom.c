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
#include <time.h>
#include "merc.h"
#include "mem.h"
#include "olc.h"


void gdt_aes_sedai( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *rank1;
    char *rank2;
    char *rank3;
    char *rank4;
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    rank1 = guild_rank(ch->guild, GET_RANK(ch,1), 1, TRUE);
    rank2 = guild_rank(ch->guild, GET_RANK(ch,2), 2, TRUE);
    rank3 = guild_rank(ch->guild, GET_RANK(ch,3), 3, TRUE);
    rank4 = guild_rank(ch->guild, GET_RANK(ch,4), 4, TRUE);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( vch->guild != ch->guild )
		continue;
	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;
	    if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOGDT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    switch( GET_RANK(ch,1) )
	    {
		default:
		    sprintf( buf, "%s %s: $7$T`n",
			rank1, FIRSTNAME(ch) );
		    break;
		case 3:
		    sprintf( buf, "%s %s (%s): $7$T`n",
			FIRSTNAME(ch), rank1, rank2 );
		    break;
		case 4:
		case 5:
		case 10:
		    sprintf( buf, "%s %s (%s): $7$T`n",
			rank1, FIRSTNAME(ch), rank2 );
		    break;
		case 11:
		    sprintf( buf, "%s %s: $7$T`n",
			rank1, FIRSTNAME(ch) );
		    break;
	    }
	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    free_string( rank1 );
    free_string( rank2 );
    free_string( rank3 );
    free_string( rank4 );
    return;
}

void gdt_warder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *rank1;
    char *rank2;
    char *rank3;
    char *rank4;
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    rank1 = guild_rank(ch->guild, GET_RANK(ch,1), 1, TRUE);
    rank2 = guild_rank(ch->guild, GET_RANK(ch,2), 2, TRUE);
    rank3 = guild_rank(ch->guild, GET_RANK(ch,3), 3, TRUE);
    rank4 = guild_rank(ch->guild, GET_RANK(ch,4), 4, TRUE);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( vch->guild != ch->guild )
		continue;
	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;
	    if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOGDT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    switch( GET_RANK(ch,1) )
	    {
		default:
		    sprintf( buf, "%s %s: $7$T`n",
			rank1, FIRSTNAME(ch) );
		    break;

		case 0:
		    sprintf( buf, "%s, %s %s%s%s: $7$T`n",
			FIRSTNAME(ch), rank1,
			rank2,
			IS_NULLSTR(rank2) ? "" : " ",
			rank3 );
		    break;

	    }
	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    free_string( rank1 );
    free_string( rank2 );
    free_string( rank3 );
    free_string( rank4 );
    return;
}

void gdt_seanchan( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *rank1;
    char *rank2;
    char *rank3;
    char *rank4;
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    rank1 = guild_rank(ch->guild, GET_RANK(ch,1), 1, TRUE);
    rank2 = guild_rank(ch->guild, GET_RANK(ch,2), 2, TRUE);
    rank3 = guild_rank(ch->guild, GET_RANK(ch,3), 3, TRUE);
    rank4 = guild_rank(ch->guild, GET_RANK(ch,4), 4, TRUE);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( vch->guild != ch->guild )
		continue;
	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;
	    if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOGDT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    switch( GET_RANK(ch,1) )
	    {
		default:
		    sprintf( buf, "%s %s %s (%s) %s: $7$T`n",
			rank2, rank1, FIRSTNAME(ch), rank3, rank4 );
		    break;
		case 0:
		    if ( !IS_NULLSTR(ch->pcdata->guild->damane_name) )
			sprintf( buf, "%s %s %s [%s] (%s) %s: $7$T`n",
			    rank2, rank1, FIRSTNAME(ch),
			    ch->pcdata->guild->damane_name,
			    rank3, rank4 );
		    else
			sprintf( buf, "%s %s: $7$T`n",
			    rank1, FIRSTNAME(ch) );
		    break;
	    }
	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    free_string( rank1 );
    free_string( rank2 );
    free_string( rank3 );
    free_string( rank4 );
    return;
}

void gdt_aiel( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *rank1;
    char *rank2;
    char *rank3;
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    rank1 = guild_rank(ch->guild, GET_RANK(ch,1), 1, TRUE);
    rank2 = guild_rank(ch->guild, GET_RANK(ch,2), 2, TRUE);
    rank3 = guild_rank(ch->guild, GET_RANK(ch,3), 3, TRUE);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( vch->guild != ch->guild )
		continue;
	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;
	    if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOGDT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    switch( GET_RANK(ch,1) )
	    {
		default:
		    sprintf( buf, "%s %s (of the %s Aiel): $7$T`n",
			rank1, FIRSTNAME(ch), rank2 );
		case 4:
		case 5:
		case 6:
		case 14:
		    sprintf( buf, "%s %s (%s of the %s Aiel): $7$T`n",
			rank1, FIRSTNAME(ch), rank3, rank2 );
		    break;
	    }
	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    free_string( rank1 );
    free_string( rank2 );
    free_string( rank3 );
    return;
}

void gdt_generic( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    if ( !IS_NPC(ch) )
    {
        sprintf( buf, "%s %s: $7$T`n",
            guild_rank(ch->guild, GET_RANK(ch,1), 1, FALSE),
            FIRSTNAME(ch) );
    }
    else
    {
        sprintf( buf, "%s %s: $7$T`n",
            guild_name(ch->guild),
            FIRSTNAME(ch) );
    }
     
    for ( d = descriptor_list; d != NULL; d = d->next )   
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;
    
        och = d->original ? d->original : d->character;
        vch = d->character;
            
        if ( vch != NULL )
        {
            if ( IS_WRITING(vch) )
                continue;
        
            if ( vch->guild != ch->guild )
                continue;
     
            if (IS_SET(vch->comm, COMM_QUIET) )
                continue;
     
            if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOGDT) )
                continue;
    
            position            = vch->position;
            old_con             = d->connected;
            d->connected        = CON_PLAYING;

            vch->position   = POS_STANDING;
            
            act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
        }
    }
    return;
}



void do_gdtalk( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( ch->guild == 0 )
    {
	send_to_char( "You're not a member of any guild.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( IS_SET(ch->comm,COMM_NOGDT) )
	{
            send_to_char("Guild channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOGDT);
        }
        else
        {
            send_to_char("Guild channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOGDT);
        }
	return;
    }

    if ( IS_SET(ch->comm, COMM_NOGDT) )
    {
	send_to_char( "You must be listening to the guild channel first.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what to your guild?\n\r", ch );
        return;
    }

    if ( ch->guild == guild_lookup("aes sedai") )
	gdt_aes_sedai( ch, argument );
    else if ( ch->guild == guild_lookup("warder") )
	gdt_warder( ch, argument );
    else if ( ch->guild == guild_lookup("seanchan") )
	gdt_seanchan( ch, argument );
    else if ( ch->guild == guild_lookup("aiel") )
	gdt_aiel( ch, argument );
    else
	gdt_generic( ch, argument );
}

void do_dftalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;

    if ( argument[0] == '\0' )
    {
	if ( IS_SET(ch->comm,COMM_NODFT) )
	{
            send_to_char("Darkfriend channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NODFT);
        }
        else
        {
            send_to_char("Darkfriend channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm,COMM_NODFT);
        }
	return;
    }

    if ( IS_SET(ch->comm, COMM_NODFT) )
    {
	send_to_char( "You must be listening to the darkfriend channel first.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }

    if ( !is_darkfriend(ch) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what to the darkfriends??\n\r", ch );
        return;
    }

    if ( ch->guild != guild_lookup( "darkfriend" ) )
    {
	sprintf( buf, "*%s %s*: $8$T`n", 
	    IS_NPC(ch) ? "darkfriend" :
	    guild_rank(guild_lookup( "darkfriend" ), ch->pcdata->shadow_rank, 1, FALSE),
	    SHADOWNAME(ch) );
    }
    else
    {
	sprintf( buf, "*%s %s*: $8$T`n", 
	    IS_NPC(ch) ? "darkfriend" :
	    guild_rank(ch->guild, GET_RANK(ch, 1), 1, FALSE),
	    SHADOWNAME(ch) );
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( vch->guild != guild_lookup( "darkfriend" )
	    &&   (!IS_NPC( ch ) && vch->pcdata->shadow_rank == -1) )
		continue;

	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;

	    if (!IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NODFT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    return;
}


void do_wdtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int old_con;
    int position;


    if ( argument[0] == '\0' )
    {
	if ( IS_SET(ch->comm,COMM_NOWDT) )
	{
            send_to_char("Warder channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOWDT);
        }
        else
        {
            send_to_char("Warder channel is now OFF.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOWDT);
        }
	return;
    }

    if ( IS_SET(ch->comm, COMM_NOWDT) )
    {
	send_to_char( "You must be listening to the warder channel first.\n\r", ch );
	return;
    }


    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }

    if ( !is_warder(ch) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what to the bonded Warders and Sedai??\n\r", ch );
        return;
    }

    if ( ch->guild == guild_lookup("warder")
    ||   ch->guild == guild_lookup("aes-sedai") )
    {
	sprintf( buf, "*%s %s*: $8$T`n", 
	    IS_NPC(ch) ? guild_name(ch->guild) :
	    guild_rank(ch->guild, GET_RANK(ch, 1), 1, FALSE),
	    FIRSTNAME(ch) );
    }
    else
    {
	sprintf( buf, "*Warder %s*: $8$T`n", 
	    FIRSTNAME(ch) );
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if ( !is_warder(vch) )
		continue;

	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;

	    if ( !IS_IMMORTAL(ch) && IS_SET(vch->comm, COMM_NOWDT) )
		continue;

	    position            = vch->position;
	    old_con             = d->connected;
	    d->connected        = CON_PLAYING;

	    vch->position   = POS_STANDING;

	    act( buf, vch, NULL, argument, TO_CHAR );
            vch->position       = position;
            d->connected        = old_con;
	}
    }
    return;
}
