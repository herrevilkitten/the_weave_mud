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
#include "interp.h"
#include "mem.h"

extern	const	struct	cmd_type	cmd_table[];

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);


void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
int isalpha(int c);

/*
 * Local functions.
 */
void	note_attach	args( ( CHAR_DATA *ch ) );
void	note_remove	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	note_delete	args( ( NOTE_DATA *pnote ) );


void do_nonote( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->comm, COMM_NOTES) )
    {
	send_to_char( "Okay, you are no longer informed of new notes.\n\r", ch );
	REMOVE_BIT( ch->comm, COMM_NOTES );
	return;
    }

    send_to_char( "Okay, you are now informed of new notes.\n\r", ch );
    SET_BIT( ch->comm, COMM_NOTES );
    return;
}

/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
            wiznet("$N doesn't want to disappear.",ch,NULL,0,0,0);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%c/%s", PLAYER_DIR,
		LOWER(ch->name[0]), correct_name(ch->name) );
            wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    do_quit(ch,"");
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));

}

/* Hacked in color code */	    

int chan( int channel )
{
    int c = 0;
    switch( channel )
    {
	case COMM_NOWIZ		: c = COLOR_IMMTALK;	break;
	case COMM_NOOOC		: c = COLOR_OOC;	break;
/*	case COMM_NOQUESTION	: c = COLOR_QUESTION;	break;	*/
	case COMM_NOBARD	: c = COLOR_BARD;	break;
	case COMM_NOAUCTION	: c = COLOR_AUCTION;	break;
	default			: bug ( "invalid chan code %d", channel );
				  c = 0; break;
    }
    return c;
}

/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char color_code[10];
    int position;
    int old_con;

    sprintf( color_code, "$1" );
 
    if ( argument[0] == '\0' )
    {
        sprintf( buf, "%s what?\n\r", verb );
        buf[0] = UPPER(buf[0]);
        return;
    }
    
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
        sprintf( buf, "You can't %s.\n\r", verb );
        send_to_char( buf, ch );
        return;
    }
        
    REMOVE_BIT(ch->comm, channel);
     
    switch ( channel )
    {
    default:
	    if ( channel == COMM_NOAUCTION )
		sprintf( color_code, "$1" );
	    if ( channel == COMM_NOOOC )
		sprintf( color_code, "$2" );
	    if ( channel == COMM_NOBARD )
		sprintf( color_code, "$4" );

            sprintf( buf, "`n%sYou %s '$t'`n",
		color_code,
                verb );
    	    position = ch->position;
    	    ch->position = POS_STANDING;
    	    act( buf, ch, argument, NULL, TO_CHAR );
    	    ch->position = position;
    
    	    sprintf( buf, "`n%s$n %ss '$t'`n",
		color_code,
	        verb );
            break;
 
    case COMM_NOWIZ:
        sprintf( buf, "`n$3$n: $t`n" );
        position        = ch->position;
        ch->position    = POS_STANDING;
        act( buf, ch, argument, NULL, TO_CHAR );
        ch->position    = position;
        sprintf( buf, "`n$3$n: $t`n" );
        break;
    }
    
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;
        
        och = d->original ? d->original : d->character;
        vch = d->character;
        
        if ( vch != NULL && vch != ch 
        &&  !IS_SET(och->comm, channel) )
        {
            char buf2 [MAX_STRING_LENGTH];
        
	    if ( channel == COMM_NOSHOUT && IS_SET(och->comm, COMM_DEAF) )
		continue;

            if ( channel == COMM_NOWIZ && !IS_HERO(och) )
                continue;
	/*
            if ( channel == COMM_NOYELL
                && vch->in_room->area != ch->in_room->area )
                continue;
	*/
	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;

	    if ( vch->in_room != NULL
	    &&   IS_SET(vch->in_room->room_flags, ROOM_NONOISE)
	    &&   ch->level < MAX_LEVEL - 2  )
		continue;

	    if ( IS_WRITING(vch) )
		continue;

             position            = vch->position;
             old_con             = d->connected;
             d->connected        = CON_PLAYING;
             if ( channel != COMM_NOSHOUT  )
                 vch->position   = POS_STANDING;
        
             sprintf( buf2, "%s`n",
                 buf );  
             act( buf2, ch, argument, vch, TO_VICT );
             vch->position       = position;
             d->connected        = old_con;
        }
    }
                
    return;
}
                 


 /* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    victim = ch;

    if( IS_IMMORTAL( ch) && *argument)
	if( ( victim = get_char_world( ch, argument)) == NULL)
	{
	    send_to_char( "They are not here.\r\n", ch);
	    return;
	}

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("OOC            ",ch);
    if( !IS_SET( victim->comm, COMM_NOOOC))
	send_to_char("`2ON`n\n\r",ch);
    else
	send_to_char("`3OFF`n\n\r",ch);

    send_to_char("auction        ",ch);
    if( !IS_SET( victim->comm,COMM_NOAUCTION))
	send_to_char("`2ON`n\n\r",ch);
    else
	send_to_char("`3OFF`n\n\r",ch);

    send_to_char("bard           ",ch);
    if( !IS_SET( victim->comm,COMM_NOBARD))
	send_to_char("`2ON`n\n\r",ch);
    else
	send_to_char("`3OFF`n\n\r",ch);

    if (IS_HERO( victim))
    {
	send_to_char("god channel    ",ch);
	if( !IS_SET( victim->comm,COMM_NOWIZ))
	    send_to_char("`2ON`n\n\r",ch);
	else
	    send_to_char("`3OFF`n\n\r",ch);
    }

    send_to_char("shouts         ",ch);
    if( !IS_SET( victim->comm,COMM_DEAF))
	send_to_char("`2ON`n\n\r",ch);
    else
	send_to_char("`3OFF`n\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if( IS_SET( victim->comm,COMM_QUIET))
	send_to_char("`2ON`n\n\r",ch);
    else
	send_to_char("`3OFF`n\n\r",ch);
   
    if( victim->lines != PAGELEN)
    {
	if( victim->lines)
	{
	    sprintf( buf, "You display `4%d`n lines of scroll.\n\r",
		victim->lines + 2);
	    send_to_char( buf, ch);
	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if ( !IS_NULLSTR(ch->prompt) )
    {
	sprintf( buf, "Your current prompt is: %s\r\n", ch->prompt );
	send_to_char( buf, ch );
    }

    if( IS_SET( victim->comm, COMM_NOTES) )
	send_to_char( "`2You are informed of new notes.`n\n\r", ch );
    else
	send_to_char( "`3You are not informed of new notes.`n\n\r", ch );

    if( IS_SET( victim->comm,COMM_NOSHOUT))
	send_to_char("`3You cannot shout.`n\n\r",ch);
  
    if( IS_SET( victim->comm,COMM_NOTELL))
	send_to_char("`3You cannot use tell.`n\n\r",ch);
 
    if( IS_SET( victim->comm,COMM_NOCHANNELS))
	send_to_char("`3You cannot use channels.`n\n\r",ch);

    if( IS_SET( victim->comm,COMM_NOEMOTE))
	send_to_char("`3You cannot show emotions.`n\n\r",ch);
}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_NOSHOUT))
    {
      send_to_char("The gods have taken away your ability to shout.\n\r",ch);
      return;
    }
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear shouts again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear shouts.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* RT auction rewritten in ROM style */
void do_auction( CHAR_DATA *ch, char *argument )
{

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOAUCTION))
      {
	send_to_char("Auction channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOAUCTION);
      }
      else
      {
	send_to_char("Auction channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOAUCTION);
      }
    }
    else  /* auction message sent, turn auction on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;

	  REMOVE_BIT(ch->comm,COMM_NOAUCTION);
        }

	talk_channel( ch, argument, COMM_NOAUCTION, "auction" );
    }
}

/* RT chat replaced with ROM gossip */
void do_ooc( CHAR_DATA *ch, char *argument )
{
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOOOC))
      {
        send_to_char("OOC channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOOOC);
      }
      else
      {
        send_to_char("OOC channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOOOC);
      }
    }
    else  /* OOC message sent, turn OOC on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}

 
	talk_channel( ch, argument, COMM_NOOOC, "OOC" );
    }
}

/* The ever-popular Marriage channel =P */
void do_mtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (!IS_SET(ch->act, PLR_MARRIED) )
    {
	send_to_char( "You must be married to use marriage talk.\n\r", ch );
	return;
    }

    if (ch->pcdata->spouse == NULL )
    {
	sprintf( buf, "Spouse is NULL for %s", ch->name );
	bug( buf, 0);
	return;
    }


    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;
 
        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_QUIET)
	     && (!strcmp(ch->pcdata->spouse, victim->name)) )
        {
	    sprintf( buf, "You tell %s '%s'\n\r", ch->pcdata->spouse, argument );
	    send_to_char( buf, ch );
            act_new( "$n tells you '$t'", 
		     ch,argument, d->character, TO_VICT,POS_SLEEPING );
	    return;
        }
      
    }
    sprintf( buf, "%s isn't connected right now.\n\r", ch->pcdata->spouse );
    send_to_char( buf, ch );
    return;
}


/* RT question channel */
/*
void do_ic( CHAR_DATA *ch, char *argument )
{
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
        send_to_char("IC channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
        send_to_char("IC channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else 
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}

	if ( is_affected(ch, skill_lookup( "gag" )) )
	{
	    send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	    return;
	}

        REMOVE_BIT(ch->comm,COMM_NOQUESTION);
	talk_channel( ch, argument, COMM_NOQUESTION, "IC" ); 
    }
}
*/

/* RT music channel */
void do_bard( CHAR_DATA *ch, char *argument )
{
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOBARD))
      {
        send_to_char("Bard channel is now ON.\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOBARD);
      }
      else
      {
        send_to_char("Bard channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOBARD);
      }
    }
    else  /* bard sent, turn bard on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
	}
 
        REMOVE_BIT(ch->comm,COMM_NOBARD);
 	talk_channel( ch, argument, COMM_NOBARD, "bard" );
    }
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    talk_channel( ch, argument, COMM_NOWIZ, "immtalk" );
    return;
}



void do_say( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS)
    &&   ch->pIndexData->pShop == NULL )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	return;
    }

    sprintf( buf, "You say $6'$T'`n" );
    act( buf, ch, NULL, argument, TO_CHAR );

    sprintf( buf, "$n says $6'$T'`n" );
    act( buf, ch, NULL, argument, TO_ROOM );

    return;
}

void do_osay( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what out of character?\n\r", ch );
	return;
    }


    sprintf( buf, "You say '(ooc) $6$T'`n" );
    act( buf, ch, NULL, argument, TO_CHAR );

    sprintf( buf, "$n says '(ooc) $6$T'`n" );
    act( buf, ch, NULL, argument, TO_ROOM );
    return;
}

void do_think( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Think what out of character?\n\r", ch );
	return;
    }

    act( "$n thinks ooc'ly . o O ( $T `n)", ch, NULL, argument, TO_ROOM );
    act( "You think ooc'ly . o O ( $T `n)", ch, NULL, argument, TO_CHAR );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't shout.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_DEAF))
    {
	send_to_char( "Deaf people can't shout.\n\r",ch);
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Shout what?\n\r", ch );
	return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 3 );
    act( "You shout '$T'`n", ch, NULL, argument, TO_CHAR );
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( victim == NULL )
	    continue;

	if ( IS_WRITING(victim) )
	    continue;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_DEAF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    act("$n shouts '$t'`n",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !IS_NPC(victim) && IS_IDLE(victim) )
    {
	act("$N is rather idle...try again later.",
	    ch,NULL,victim,TO_CHAR);
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if ( !IS_NPC(victim) && IS_WRITING(victim) )
    {
	sprintf( buf, "%s is writing right now.\n\r", victim->name );
	send_to_char( buf, ch );
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    sprintf( buf, "You tell $N $5'$t'`n" );
    act( buf, ch, argument, victim, TO_CHAR );

    sprintf( buf, "$N tells you $5'$t'`n" );
    act_new( buf, victim, argument, ch, TO_CHAR, POS_DEAD );

    victim->reply       = ch;

    if ( victim->fighting )
    {
	send_to_char_new(ch, "%s is fighting right now.\n\r", PERS(victim, ch) );
	if ( !IS_NPC(victim) )
	{
	    send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	    sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
            buf[0] = UPPER(buf[0]);
            add_buf(victim->pcdata->buffer,buf);
	}
	return;
    }

    if ( IS_SET(victim->comm, COMM_AFK) )
    {
        if ( victim->pcdata->afk_message == NULL )
        {
	    sprintf( buf, "%s has AFK on, but has left no message.\n\r",
		PERS(victim, ch) );
	    send_to_char( buf, ch );
	    send_to_char_new( ch, "Your message is stored in %s's playback.\n\r", PERS(victim, ch) );
        }
	else
	{
	    sprintf( buf, "%s has AFK on!\n\rMESSAGE: %s`n\n\r",
		 victim->name,  victim->pcdata->afk_message );
            send_to_char( buf, ch );
	    send_to_char_new( ch, "Your message is stored in %s's playback.\n\r", PERS(victim, ch) );
	}
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
    }
    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( IS_IDLE(victim) )
    {
	act("$N is rather idle...try again later.",
	    ch,NULL,victim,TO_CHAR);
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
        act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
        return;
    }

    if ( IS_WRITING(victim) )
    {
	sprintf( buf, "%s is writing right now.\n\r", victim->name );
	send_to_char( buf, ch );
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    sprintf( buf, "You tell $N $5'$t'`n" );
    act( buf, ch, argument, victim, TO_CHAR );

    sprintf( buf, "$N tells you $5'$t'`n" );
    act_new( buf, victim, argument, ch, TO_CHAR, POS_DEAD );

    victim->reply	= ch;

    if ( victim->fighting )
    {
	send_to_char_new(ch, "%s is fighting right now.\n\r", PERS(victim, ch) );
	send_to_char_new(ch, "Your message has been placed in %s's playback.\n\r", PERS(victim, ch) );
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( IS_SET(victim->comm, COMM_AFK) )
    {
        if ( victim->pcdata->afk_message == NULL )
        {
	    sprintf( buf, "%s has AFK on, but has left no message.\n\r",
		PERS(victim, ch) );
	    send_to_char( buf, ch );
	    send_to_char_new( ch, "Your message is stored in %s's playback.\n\r", PERS(victim, ch) );
        }
	else
	{
	    sprintf( buf, "%s has AFK on!\n\rMESSAGE: %s`n\n\r",
		 victim->name,  victim->pcdata->afk_message );
            send_to_char( buf, ch );
	    send_to_char_new( ch, "Your message is stored in %s's playback.\n\r", PERS(victim, ch) );
	}
	sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
    }

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }
    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	return;
    }

    act("You yell '$t'",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
        &&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act("$n yells '$t'`n",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }
    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS) )
    {
	send_to_char( "You have been nochanneled.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	if ( strstr(right_case( argument, CASE_LOWER ), "say") )
	{
	    send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	    return;
	}
    }

    if ( !isalpha(argument[0]) && argument[0] != '`' )
    {
	act( "$6$n$T`n", ch, NULL, argument, TO_ROOM );
	act( "$6$n$T`n", ch, NULL, argument, TO_CHAR );
    }
    else
    {
	act( "$6$n $T`n", ch, NULL, argument, TO_ROOM );
	act( "$6$n $T`n", ch, NULL, argument, TO_CHAR );
    }
    return;
}





void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}





void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act, PLR_NOQUIT) )
    {
	send_to_char( "You can't quit right now.", ch );
	return;
    }

    send_to_char( 
	"The wheel weaves as the wheel wills.\n\rReturn to the pattern soon, friend.\n\r",ch);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet("$N is spun out of the pattern.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    if (IS_SET(ch->comm, COMM_AFK))
	REMOVE_BIT(ch->comm, COMM_AFK);

    REMOVE_BIT(ch->affected_by, AFF_CHARM);
    REMOVE_BIT(ch->affected_by_2, AFF_LEASHED);
    affect_strip(ch, gsn_charm_person);
    affect_strip(ch, gsn_leashing);

    /*
     * After extract_char the ch is no longer valid!
     */

    /* Turn off that cool stalk code ;-) */
    if ( ch->master != NULL && IS_SET(ch->affected_by, AFF_STALK) )
	stop_stalk( ch );

    save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
        CHAR_DATA *tch;
    
        d_next = d->next;
        tch = d->original ? d->original : d->character;
        if (tch && tch->id == id)
        {
            extract_char(tch,TRUE);
            close_socket(d);
        }
    }

    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    act( "Saving $n.", ch, NULL, NULL, TO_CHAR );
    WAIT_STATE(ch, 3 );
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
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
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_HERO(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    REMOVE_BIT( ch->affected_by, AFF_STALK );
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( CHARM_SET(ch) && ch->master != NULL )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	REMOVE_BIT( ch->affected_by_2, AFF_LEASHED );
	affect_strip( ch, gsn_charm_person );
	affect_strip( ch, gsn_leashing );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    drop_link( ch );
    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	if ( IS_SET(ch->affected_by, AFF_STALK) )
	    stop_stalk( ch );
	else
	    stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found = FALSE;
    bool fAll;
    int cmd;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( CHARM_SET(ch) && ch->master != NULL )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( LOWER(arg2[0]) == LOWER(cmd_table[cmd].name[0])
        &&   !str_prefix(arg2, cmd_table[cmd].name) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
	send_to_char( "Hmm .. your followers can't seem to do that.\n\r", ch );
	return;
    }

    if ( !str_cmp(argument, "quit") )
    {
	send_to_char( "Do it yourself.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !CHARM_SET(ch)
	|| victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( CHARM_SET(ch)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    if ( IS_NPC(och) && !IS_SENTIENT(och)
	    &&   !IS_SET(och->act, ACT_PET) )
		act( "$N looks at you without comprehension.", ch, NULL,
		    och, TO_CHAR );
	    else
		interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
            	char *color_str_1;
            	char *color_str_2;

		int val1 = 100;
		int val2 = 100;

		if ( gch->max_hit > 0 )
		    val1 = 100 * gch->hit / gch->max_hit;
		if ( gch->max_stamina > 0 )
		    val2 = 100 * gch->stamina / gch->max_stamina;

		if ( val1 >= 50 )
		    color_str_1 = "`2";
		else if ( val1 >= 25 )
		    color_str_1 = "`B`6";
		else
		    color_str_1 = "`3";

		if ( val2 >= 50 )
		    color_str_2 = "`2";
		else if ( val2 >= 25 )
		    color_str_2 = "`B`6";
		else
		    color_str_2 = "`3";

		sprintf( buf,
		"[%3d] %-36.36s %s%3d`n%% health  %s%3d`n%% stamina\n\r",
		    gch->level,
		    capitalize( PERS(gch, ch) ),
		    color_str_1, val1,
		    color_str_2, val2 );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( str_cmp(arg, "all") )
    {
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
        }

        if ( ch->master != NULL
	||   (ch->leader != NULL && ch->leader != ch) )
        {
	    send_to_char( "But you are following someone else!\n\r", ch );
	    return;
        }

        if ( (victim->master != ch && ch != victim)
	||   IS_SET(victim->affected_by, AFF_STALK) )
        {
	    act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	    return;
        }
    
	if ( CHARM_SET(ch) && ch->master != NULL )
        {
            send_to_char("You can't remove charmed people from your group.\n\r",ch);
            return;
        }
    
	if ( CHARM_SET(ch) && ch->master != NULL )
        {
    	    act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
    	    return;
        }

        if ( is_same_group( victim, ch ) && ch != victim )
        {
	    victim->leader = NULL;
	    act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	    act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	    act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	    return;
        }
/*
    if ( ch->level - victim->level < -8
    ||   ch->level - victim->level >  8 )
    {
	act( "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }
*/

        victim->leader = ch;
        act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
        act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
        act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	CHAR_DATA *vch;
        if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
        {
	    send_to_char( "But you are following someone else!\n\r", ch );
	    return;
        }

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
	    if ( vch->master == ch && !is_same_group(vch, ch)
		&& !IS_SET(vch->affected_by, AFF_STALK) )
	    {    
		vch->leader = ch;
		act( "$N joins $n's group.", ch, NULL, vch, TO_NOTVICT );
		act( "You join $n's group.", ch, NULL, vch, TO_VICT    );
		act( "$N joins your group.", ch, NULL, vch, TO_CHAR    );
	    }
	}
    }

    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group(gch, ch) && !CHARM_SET(gch) )
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch, ch) && !CHARM_SET(gch))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '%s'`n\n\r",
	FIRSTNAME(ch), argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    while (ach->leader && ach->leader != ach) ach=ach->leader;
    while (bch->leader && bch->leader != bch) bch=bch->leader;
    return ach == bch;
}

void do_ungroup( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch->master == ch && is_same_group(vch, ch)
	    && !IS_SET(vch->affected_by, AFF_STALK) )
	{    
	    vch->leader = ch;
	    act( "$n removes $N from $n's group.", ch, NULL, vch, TO_NOTVICT );
	    act( "You are removed from $n's group.", ch, NULL, vch, TO_VICT    );
	    act( "You remove $N from your group.", ch, NULL, vch, TO_CHAR    );
	    stop_follower( vch );
	}
    }

    return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Whisper whom what?\n\r", ch );
	return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL
	 || victim->in_room != ch->in_room )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_WRITING(victim) )
    {
	act( "$E is writing something right now.", ch, NULL, victim,
	    TO_CHAR );
	return;
    }

  
    sprintf( buf, "You whisper to $N $6'$t'`n" );
    act( buf, ch, argument, victim, TO_CHAR );

    act( "$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT );

    sprintf( buf, "$N whispers to you $6'$t'`n" );
    act_new( buf, victim, argument, ch, TO_CHAR, POS_DEAD );
    return;
}

void do_dirsay( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Say what to whom?\n\r", ch );
        return;
    }

    if ( is_affected(ch, skill_lookup( "gag" )) )
    {
	send_to_char( "You cannot say anything, you are gagged.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL
         || victim->in_room != ch->in_room )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
        act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
        return;
    }

    if ( IS_WRITING(victim) )
    {
	act( "$E is writing something right now.", ch, NULL, victim,
	    TO_CHAR );
	return;
    }

    if ( ch != victim )
    {
	sprintf( buf, "You say to $N $6'$t'`n" );
	act( buf, ch, argument, victim, TO_CHAR );

	act( "$n says to $N $6'$t'`n", ch, argument, victim, TO_NOTVICT );

	sprintf( buf, "$N says to you $6'$t'`n" );
	act_new( buf, victim, argument, ch, TO_CHAR, POS_DEAD );
    }
    else
    {
	act( "You say to yourself $6'$t'`n", ch, argument, NULL, TO_CHAR );
	act( "$n says to $mself $6'$t'`n", ch, argument, NULL, TO_ROOM );
    }

    return;
}


/* RT configure command SMASHED */

void do_afk( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( argument[0] != '\0' )
    {
        /*( Fusion was here )*/

	free_string( ch->pcdata->afk_message );
        ch->pcdata->afk_message = str_dup( argument );
    }

    if ( argument[0] == '\0' && !IS_SET(ch->comm, COMM_AFK) )
    {
	free_string( ch->pcdata->afk_message );
        ch->pcdata->afk_message = NULL;
    }

    if ( IS_SET(ch->comm, COMM_AFK) )
    {
        REMOVE_BIT(ch->comm, COMM_AFK);
        send_to_char( "AFK mode off.\n\r", ch );
        act( "$n has come back from AFK.", ch, NULL, NULL, TO_ROOM );
        if ( ch->pcdata->afk_message )
            free_string( ch->pcdata->afk_message );
        ch->pcdata->afk_message = NULL;
	if ( buf_string(ch->pcdata->buffer)[0] != '\0' )
	    send_to_char( "You have tells waiting.  Type REPLAY to read them.\n\r", ch );
    }
    else
    {
        SET_BIT(ch->comm, COMM_AFK);
        send_to_char( "AFK mode on.\n\r", ch );
        act( "$n has gone AFK.", ch, NULL, NULL, TO_ROOM );
	if ( ch->pcdata->afk_message )
	    send_to_char_new( ch, "Your AFK message is: %s\n\r",
		ch->pcdata->afk_message );
    }
    return;
}

void do_book( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    char arg[MAX_INPUT_LENGTH];
    int book;
    int position, old_con;
    int bit = BOOK_1;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "BOOK 1    ", ch );
	if ( IS_SET(ch->book, BOOK_1) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 2    ", ch );
	if ( IS_SET(ch->book, BOOK_2) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 3    ", ch );
	if ( IS_SET(ch->book, BOOK_3) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 4    ", ch );
	if ( IS_SET(ch->book, BOOK_4) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 5    ", ch );
	if ( IS_SET(ch->book, BOOK_5) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 6    ", ch );
	if ( IS_SET(ch->book, BOOK_6) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );

	send_to_char( "BOOK 7    ", ch );
	if ( IS_SET(ch->book, BOOK_7) )
	    send_to_char( "`2ON`n\n\r", ch );
	else
	    send_to_char( "`3OFF`n\n\r", ch );
	return;
    }

    book = atoi( arg );
    if ( book < 1 || book > 7 )
    {
	send_to_char( "The valid book channels are currently 1 through 7.\n\r", ch );
	return;
    }

    switch( book )
    {
	case 1:
	    bit = BOOK_1;
	    break;
	case 2:
	    bit = BOOK_2;
	    break;
	case 3:
	    bit = BOOK_3;
	    break;
	case 4:
	    bit = BOOK_4;
	    break;
	case 5:
	    bit = BOOK_5;
	    break;
	case 6:
	    bit = BOOK_6;
	    break;
	case 7:
	    bit = BOOK_7;
	    break;
    }

    if ( argument[0] == '\0' )
    {
	if ( IS_SET(ch->book, bit) )
	{
	    REMOVE_BIT(ch->book, bit);
	    send_to_char_new( ch, "You are no longer listening to the book %d channel.\n\r", book );
	}
	else
	{
	    SET_BIT(ch->book, bit);
	    send_to_char_new( ch, "You are now listening to the book %d channel.\n\r", book );
	}
	return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
	send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	return;
    }

    if ( !IS_SET(ch->book, bit) )
    {
	send_to_char( "You must listen to a channel before you can announce on it.\n\r", ch );
	return;
    }

    sprintf( buf, "[BOOK %d] %s: %s", book, FIRSTNAME(ch), argument );

    for ( d = descriptor_list; d; d = d->next )
    {
        CHAR_DATA *och;
        CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;
	
	if ( vch != NULL )
	{
	    if ( IS_WRITING(vch) )
		continue;

	    if (IS_SET(vch->comm, COMM_QUIET) )
		continue;

	    if ( !IS_SET(vch->book, bit) )
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


void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))   
    {
        send_to_char("You can't replay.\n\r",ch);
        return;
    }
        
    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
        send_to_char("You have no tells to replay.\n\r",ch);
        return;
    }
        
    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

void do_email( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    smash_tilde( argument );
    free_string( ch->pcdata->email );
    ch->pcdata->email = str_dup( argument );

    send_to_char_new( ch, "Your email address is now recorded as: %s\r\n",
	ch->pcdata->email );
    return;
}

