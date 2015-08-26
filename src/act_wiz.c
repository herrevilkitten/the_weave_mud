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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include "merc.h"
#include "mem.h"
#include "interp.h"

DECLARE_SPELL_FUN(      spell_null              );


int close(int fd);
pid_t waitpid(pid_t pid, int *status, int options);
int execv( const char *path, char *const argv[]);

char    *bit_name               args( ( AFFECT_DATA *paf) );
char	*print_flags		args( (int flag) );
extern  const   struct  cmd_type        cmd_table[];



/* To have VLIST show more than vnum 0 - 9900, change the number below: */
#define MAX_SHOW_VNUM   199 /* show only 1 - 100*100 */
#define NUL '\0'

#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2



  
extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */

/* opposite directions */
const sh_int opposite_dir [6] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };

/* command procedures needed */
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_tset		);
DECLARE_DO_FUN(do_gset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_help		);

void	save_area_list	( );
void	save_area( AREA_DATA *pArea );
void	save_guilds( );

int flag_value                  args ( ( const struct flag_type *flag_table,
                                         char *argument) );
char *flag_string               args ( ( const struct flag_type *flag_table,
                                         int bits ) );

bool check_parse_name (char* name);  /* comm.c */

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	int i;
	for ( i = 0; i < 3; i++ )
	{
	    obj = create_object( get_obj_index(4221), 0 );
	    obj->cost = 0;
	    obj_to_char( obj, ch );
	}
    }

    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(4218), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    if ( ( obj = get_eq_char( ch, WEAR_HEAD ) ) == NULL )
    {
	obj = create_object( get_obj_index(4217), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_HEAD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_LEGS ) ) == NULL )
    {
	obj = create_object( get_obj_index(4219), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LEGS );
    }

    if ( ( obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(4220), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    if ( get_obj_index(ch->pcdata->weapon)
    &&   (obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    { 
        obj = create_object( get_obj_index(ch->pcdata->weapon),0);
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_WIELD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_ABOUT ) ) == NULL )
    { 
	OBJ_DATA *food;
	int i;
        obj = create_object( get_obj_index(4208),0);
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_ABOUT );
	for ( i = 0; i < 5; i++ )
	{
	    food = create_object( get_obj_index(4222),0);
	    obj_to_obj( food, obj );
	}
    }

    obj = create_object( get_obj_index(4291), 0 );
    obj->cost = 0;
    obj_to_char( obj, ch );

    obj = create_object( get_obj_index(527), 0 );
    obj->cost = 0;
    obj_to_char( obj, ch );
    return;
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel priviliges.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
        sprintf(buf,"$N restores channels to %s",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0); 

    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
        sprintf(buf,"$N revokes %s's channels.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	strcat( argument, "`n" );
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }

	strcat( argument, "`n" );
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( (victim = get_char_world( ch, arg )) == NULL )
    {
	/* check char_list */

	for ( victim = char_list; victim; victim = victim->next )
	{
	    if ( !IS_NPC(victim) && !str_cmp(victim->name, arg) )
	    {
		do_quit( victim, "" );
		send_to_char( "Forced to quit.\n\r", ch );
		return;
	    }
	}
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}





void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if ( IS_WRITING(d->character) )
		continue;

	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
	    if ( IS_WRITING(d->character) )
		continue;

            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character == NULL )
	    continue;

	victim = d->character;

	if ( !str_cmp(d->character->name, arg) )
	{
	    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
		write_to_buffer( d, "personal> ", 0 );

	    write_to_buffer( d, argument, 0 );
	    write_to_buffer( d, "\n\r", 0 );
	    send_to_char( "personal> ",ch);
	    send_to_char(argument,ch);
	    send_to_char("\n\r",ch);
	    return;
	}
    }

    send_to_char("Target not found.\n\r",ch);
    return;
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->level > ch->level )
    {
	act( "$n just tried to transfer you!", ch, NULL, victim, TO_VICT );
	act( "You cannot transfer $N!", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->position < POS_FIGHTING )
	do_stand( victim, "" );

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n $t", victim, ch->pcdata->transout, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n $t", victim, ch->pcdata->transin, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "moved" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    OBJ_DATA *on;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) && get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout))
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    if ( ch->position < POS_FIGHTING )
	do_stand( ch, "" );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin))
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "moved" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) && 
	 get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d.  Sector: %d.  Light: %d.\n\r",
	location->vnum,
	location->sector_type,
	location->light );
    send_to_char( buf, ch );

    send_to_char_new( ch, "Resources: %s\n\r",
	resource_string(location->resources) );

    sprintf( buf,
	"Room flags: %d - %s.\n\rDescription:\n\r%s",
	location->room_flags,
	room_flag_name( location->room_flags ),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}

void do_marry( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim1;
    CHAR_DATA *victim2;
    DESCRIPTOR_DATA *d;


    if (argument[0] == '\0')
    {
	send_to_char( "Usage: Marry <Player1> <Player2>\n\r", ch);
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (( ( victim1 = get_char_world( ch, arg1 ) ) == NULL ) ||
	( ( victim2 = get_char_world( ch, arg2 ) ) == NULL ))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (ch == victim1 || ch == victim2)
    {
	send_to_char("You must have someone else marry you.\n\r", ch);
	return;
    }

    if (IS_NPC(victim1) || IS_NPC(victim2))
    {
	send_to_char("Mobs cannot marry.", ch);
	return;
    }

    if ( ( victim1->in_room != ch->in_room ) ||
	 ( victim2->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ((!IS_SET(victim1->act, PLR_MARRIED)) && 
        (!IS_SET(victim2->act, PLR_MARRIED)) )
    {
	sprintf( buf, "Congratulations, %s, you are now married to %s!\n\r",
		victim1->name, victim2->name );
	send_to_char( buf, victim1);
	buf[0] = '\0';
	sprintf( buf, "Congratulations, %s, you are now married to %s!\n\r",
		victim2->name, victim1->name );
	send_to_char( buf, victim2 );
	SET_BIT(victim1->act, PLR_MARRIED);
	SET_BIT(victim2->act, PLR_MARRIED);
   
	free_string( victim1->pcdata->spouse );
	free_string( victim2->pcdata->spouse );
	victim1->pcdata->spouse = str_dup( victim2->name );
	victim2->pcdata->spouse = str_dup( victim1->name );
	 
	buf[0]='\0';
	sprintf(buf, "%s and %s are now married!\n\r", victim1->name,
		victim2->name);

	for ( d = descriptor_list; d != NULL; d = d->next )
        {
	    CHAR_DATA *msg_char;
	    msg_char = d->character;
	    send_to_char( buf, msg_char);
	}

    }
    return;
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->wiznet,WIZ_ON))
        {
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
        }
        else
        {
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
        }
        return;
    }
    
    if (!str_prefix(argument,"on"))
    {
        send_to_char("Welcome to Wiznet!\n\r",ch);
        SET_BIT(ch->wiznet,WIZ_ON);
        return;
    }
            
    if (!str_prefix(argument,"off"))
    {
        send_to_char("Signing off of Wiznet.\n\r",ch);
        REMOVE_BIT(ch->wiznet,WIZ_ON);
        return;
    }
         
    /* show wiznet status */
    if (!str_prefix(argument,"status"))
    {
        buf[0] = '\0';
     
        if (!IS_SET(ch->wiznet,WIZ_ON))
            strcat(buf,"off ");

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
            if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
            {
                strcat(buf,wiznet_table[flag].name);
                strcat(buf," ");
            }
        
        strcat(buf,"\n\r");
         
        send_to_char("Wiznet status:\n\r",ch);
        send_to_char(buf,ch);
        return;
    }
     
    if (!str_prefix(argument,"show"))  
    /* list of all wiznet options */
    {
        buf[0] = '\0';

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                strcat(buf,wiznet_table[flag].name);
                strcat(buf," ");
            }
        }
        
        strcat(buf,"\n\r");
     
        send_to_char("Wiznet options available to you are:\n\r",ch);
        send_to_char(buf,ch);
        return;
    }
        
    flag = wiznet_lookup(argument);
        
    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
        send_to_char("No such option.\n\r",ch);
        return;
    }
             
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {   
        sprintf(buf,"You will no longer see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
    }
    else
    {
        sprintf(buf,"You will now see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        SET_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
    }
             
}
        
void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
            long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;
        
    for ( d = descriptor_list; d != NULL; d = d->next )
    {   
        if (d->connected == CON_PLAYING
        &&  IS_IMMORTAL(d->character)
        &&  IS_SET(d->character->wiznet,WIZ_ON)
        &&  (!flag || IS_SET(d->character->wiznet,flag))
        &&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  !IS_WRITING(d->character)
        &&  d->character != ch)
        {    
            if (IS_SET(d->character->wiznet,WIZ_PREFIX))
                send_to_char("--> ",d->character);
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
        }
    }
    
    return;
}


void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): `4%s`n\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: `4%d`n  Format: `4%s`n  Type: `4%s`n  Resets: `4%d`n\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_type_name(obj), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: `4%s`n\n\rLong description: `4%s`n\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: `4%s`n\n\rExtra bits: `4%s`n\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: `4%d`n/`4%d`n  Weight: `4%d`n/`4%d`n\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: `4%d`n  Cost: `4%d`n  Condition: `4%d`n  Timer: `4%d`n\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: `4%d`n  In object: `4%s`n  Carried by: `4%s`n  Wear_loc: `4%d`n\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );

    /* Material type -- new :)  Joker */

    sprintf( buf, "Material: `4%s`n\n\r", material_name( obj->material ) );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: `4%d`n `4%d`n `4%d`n `4%d`n `4%d`n\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );

    if ( obj->pIndexData->spec_fun != 0 )
    {
	send_to_char( "Object has special procedure: ", ch );
	send_to_char( spec_obj_string(obj->pIndexData->spec_fun), ch );
	send_to_char( "\n\r", ch );
    }
    if ( obj->pIndexData->use_fun != 0 )
    {
	send_to_char( "Object has use function: ", ch );
	send_to_char( use_fun_string(obj->pIndexData->use_fun), ch );
	send_to_char( "\n\r", ch );
    }
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
	case ITEM_INGREDIENT:
	    send_to_char_new( ch, "Ingredient type: %s\n\r",
		flag_string(ingredient_flags, obj->value[0]) );
	    break;
    	case ITEM_POTION:
	    send_to_char_new( ch, "Has a (%d/%d) of %s.\n\r",
		obj->value[2], obj->value[1],
		potion_name(obj->value[0]) );
	    break;
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC) 	: send_to_char("`4exotic`n\n\r",ch);	break;
	    	case(WEAPON_SWORD)  	: send_to_char("`4sword`n\n\r",ch);	break;	
	    	case(WEAPON_DAGGER) 	: send_to_char("`4dagger`n\n\r",ch);	break;
	    	case(WEAPON_SPEAR)	: send_to_char("`4spear`n\n\r",ch);	break;
	    	case(WEAPON_MACE) 	: send_to_char("`4mace/club`n\n\r",ch);	break;
	   	case(WEAPON_AXE)	: send_to_char("`4axe`n\n\r",ch);		break;
	    	case(WEAPON_FLAIL)	: send_to_char("`4flail`n\n\r",ch);	break;
	    	case(WEAPON_WHIP)	: send_to_char("`4whip`n\n\r",ch);		break;
	    	case(WEAPON_POLEARM)	: send_to_char("`4polearm`n\n\r",ch);	break;
	    	case(WEAPON_STAFF)	: send_to_char("`4staff`n\n\r",ch); break;
	    	default			: send_to_char("`4unknown`n\n\r",ch);	break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Damage is `4%d`nd`4%d`n (average `4%d`n)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Damage is `4%d`n to `4%d`n (average `4%d`n)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: `4%s`n\n\r",weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	    sprintf( buf, "To hit bonus: `4%d`n\n\r"
		     "Damage bonus: `4%d`n\n\r",
		obj->value[5], obj->value[6] );
	    send_to_char( buf, ch );
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    	"Armor class is `4%d`n pierce, `4%d`n bash, `4%d`n slash, `4%d`n fire,\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	    sprintf( buf, 
	    	"               `4%d`n cold, `4%d`n electricity, `4%d`n vs. other\n\r", 
	        obj->value[4], obj->value[5], obj->value[6] );
	    send_to_char( buf, ch );
	    sprintf( buf, "The armor covers: `4%s`n\n\r",
		flag_string(armor_flags, obj->value[7]) );
	    send_to_char( buf, ch );
	break;
    }

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects `4%s`n by `4%d`n, level `4%d`n.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->strength);
	send_to_char( buf, ch );
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects `4%s`n by `4%d`n, level `4%d`n.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->strength );
	send_to_char( buf, ch );
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if ( ch->level < 106 )
	{
	    send_to_char( "You may not stat players.\n\r", ch );
	    return;
	}
        sprintf(buf,"$N is statting player %s", victim->name);
        wiznet(buf,ch,NULL,WIZ_STAT,0,0); 
    }

    sprintf( buf, "Name: `4%s`n.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: `4%d`n  Format: `4%s`n  Race: `4%s`n  Sex: `4%s`n  Room: `4%d`n\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    sprintf( buf, "Guild: `4%s`n", guild_name(victim->guild) );
    if ( IS_GUILDED(victim) )
    {
	char buf2[1000];
	sprintf( buf2, "  Guild Rank: `4%d %d %d %d`n",
	    GET_RANK(victim,1),
	    GET_RANK(victim,2),
	    GET_RANK(victim,3),
	    GET_RANK(victim,4) );
	strcat( buf, buf2 );
    }
    strcat( buf, "\n\r" );
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: `4%d`n  Killed: `4%d`n\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: `4%d`n(`4%d`n)  Int: `4%d`n(`4%d`n)  Wis: `4%d`n(`4%d`n)  Dex: `4%d`n(`4%d`n)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX) );
    send_to_char( buf, ch );
    sprintf( buf,
        "Con: `4%d`n(`4%d`n)  Cha: `4%d`n(`4%d`n)  Luc: `4%d`n(`4%d`n)  Agi: `4%d`n(`4%d`n)\n\r",
        victim->perm_stat[STAT_CON],   
        get_curr_stat(victim,STAT_CON),
        victim->perm_stat[STAT_CHR],   
        get_curr_stat(victim,STAT_CHR),
        victim->perm_stat[STAT_LUK],   
        get_curr_stat(victim,STAT_LUK),
        victim->perm_stat[STAT_AGI],   
        get_curr_stat(victim,STAT_AGI) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: `4%d`n/`4%d`n  Stamina: `4%d`n/`4%d`n  Practices: `4%d`n\n\r",
	victim->hit,		victim->max_hit,
	victim->stamina,	victim->max_stamina,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char( buf, ch );

    if ( (IS_NPC( victim ) && IS_SET( victim->act, ACT_CHANNELER ))
    ||   (!IS_NPC( victim ) && can_channel( victim, 0 ) && ch->level >= 106) )
    {
	send_to_char_new( ch, "Earth: `4%d`n/`4`B%d`n Air: `4%d`n/`4`B%d`n Fire: `4%d`n/`4`B%d`n Water: `4%d`n/`4`B%d`n Spirit: `4%d`n/`4`B%d`n\n\r",
	    get_skill(victim, gsn_earth),	victim->channel_max[0],
	    get_skill(victim, gsn_air),		victim->channel_max[1],
	    get_skill(victim, gsn_fire),	victim->channel_max[2],
	    get_skill(victim, gsn_water),	victim->channel_max[3],
	    get_skill(victim, gsn_spirit),	victim->channel_max[4] );
    }

    if ( !IS_NPC(victim) )
	send_to_char_new( ch, "Insanity: `4%d`n\n\r", victim->pcdata->insane );
    sprintf( buf,
	"Lv: `4%d`n  Class: `4%s`n  Gold: `4%ld`n  Exp: `4%d`n\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
	victim->gold,         victim->exp );
    send_to_char( buf, ch );

    sprintf( buf, "Position: `4%d`n  Wimpy: `4%d`n\n\r",
	victim->position,    victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: `4%d`nd`4%d`n  Message:  `4%s`n\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].name);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: `4%s`n\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: `4%d`n  Full: `4%d`n  Drunk: `4%d`n\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK] );
	send_to_char( buf, ch );
    }

    sprintf( buf, "Carry number: `4%d`n  Carry weight: `4%d`n\n\r",
	victim->carry_number, victim->carry_weight );
    send_to_char( buf, ch );

    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: `4%d`n  Played: `4%d`n  Last Level: `4%d`n  Timer: `4%d`n\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
    	send_to_char( buf, ch );
    }

    sprintf(buf, "Act: `4%s`n\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    if (victim->comm)
    {
    	sprintf(buf,"Comm: `4%s`n\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
	sprintf(buf, "Offense: `4%s`n\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: `4%s`n\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: `4%s`n\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: `4%s`n\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: `4%s`n\n\rParts: `4%s`n\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by `4%s`n\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    sprintf( buf, "Master: `4%s`n  Leader: `4%s`n  Pet: `4%s`n\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf, "Security: `4%d`n.\n\r", victim->pcdata->security );  /* OLC */
	send_to_char( buf, ch );                                    /* OLC */
    }

    sprintf( buf, "Short description: `4%s`n\n\rLong  description: `4%s`n",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->pIndexData->spec_fun != 0 )
    {
	send_to_char( "Mobile has special procedure: ", ch );
	send_to_char( spec_string(victim->pIndexData->spec_fun), ch );
	send_to_char( "\n\r", ch );
    }
    if ( IS_NPC(victim) && victim->hunting != NULL )
    {
        sprintf(buf, "Tracking victim: %s (%s)\n\r",
                IS_NPC(victim->hunting) ? victim->hunting->short_descr
                                        : victim->hunting->name,
                IS_NPC(victim->hunting) ? "MOB" : "PLAYER" );
        send_to_char( buf, ch );
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '`4%s`n' modifies `4%s`n by `4%d`n for `4%d`n minutes with bits`4%s`n, level `4%d`n.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    bit_name(paf),
	    paf->strength
	    );
	send_to_char( buf, ch );
    }

    if ( !IS_NPC(victim) )
    {
	int tn, gn;

	for ( tn = 0; tn < MAX_TALENT; tn++ )
	{
	    if ( talent_table[tn].name == NULL )
		break;

	    if ( !victim->pcdata->talent[tn] )
		continue;

	    send_to_char_new( ch, "  Talent: %s\n\r", talent_table[tn].name );
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
	{
	    if ( group_table[gn].name == NULL )
		break;

	    if ( !victim->pcdata->group_known[gn] )
		continue;

	    send_to_char_new( ch, "  Group: %s\n\r", group_table[gn].name );
	}
    }
    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;

	    if (!str_cmp( argument, "area" ))
	    {
		if ((pMobIndex->vnum >= ch->in_room->area->lvnum) &&
		    (pMobIndex->vnum <= ch->in_room->area->uvnum))
		{
		    found = TRUE;
		    sprintf( buf, "[%5d] %s\n\r",
		        pMobIndex->vnum, pMobIndex->short_descr );
		    send_to_char( buf, ch );
		}
	    }
	    else
	    {
	        if ( fAll || is_name( argument, pMobIndex->player_name ) )
	        {
		    found = TRUE;
		    sprintf( buf, "[%5d] %s\n\r",
		        pMobIndex->vnum, pMobIndex->short_descr );
		    send_to_char( buf, ch );
		}
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;

	    if (!str_cmp( argument, "area" ))
	    {
		if ((pObjIndex->vnum >= ch->in_room->area->lvnum) &&
		    (pObjIndex->vnum <= ch->in_room->area->uvnum))
		{
		    found = TRUE;
		    sprintf( buf, "[%5d] %s\n\r",
		        pObjIndex->vnum, pObjIndex->short_descr );
		    send_to_char( buf, ch );
		}
	    }
	    else
	    {
	        if ( fAll || is_name( argument, pObjIndex->name ) )
	        {
		    found = TRUE;
		    sprintf( buf, "[%5d] %s\n\r",
		        pObjIndex->vnum, pObjIndex->short_descr );
		    send_to_char( buf, ch );
		}
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}



void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	    strcat(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
	if (ch->lines)
	    page_to_char(buffer,ch);
	else
	    send_to_char(buffer,ch);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_fullreboot( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (!IS_SET(ch->act,PLR_WIZINVIS))
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_echo( ch, buf );
    }
    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }
    send_to_char( "You saved the world.\n\r", ch );

    save_helps();
    save_guilds( );
    do_force ( ch, "all save");
    do_save (ch, "");
    merc_down = TRUE;
    system( "rm -f command.list.back.gz" );
    system( "gzip command.list.back" );
    system( "mv command.list command.list.back" );
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
    	close_socket(d);
    }
    
    return;
}

void reboot_helps( CHAR_DATA *ch )
{    
    extern	char	*help_greeting;
    HELP_DATA *pHelp, *pHelp_next;
    FILE *fp;

    if ( (fp = fopen(HELP_FILE, "r")) == NULL )
    {
	bug( "Help file not valid.", 0 );
	return;
    }
    send_to_char( "Rebooting help texts.\r\n", ch );

    /* Unlink the current help texts */
    for ( pHelp = help_first; pHelp; pHelp = pHelp_next )
    {
	pHelp_next = pHelp->next;
	free_help( pHelp );
    }

    help_first = NULL;
    help_last  = NULL;

    for ( ; ; )
    {
	char *word;

	if ( fread_letter( fp ) != '#' )
	{
	    bug( "Reboot_helps:  # not found.", 0 );
	    exit( 1 );
	}

	word = fread_word( fp );

	if ( word[0] == '$' )
	    break;
	else if ( !str_cmp(word, "HELPS") )
	{
	    for ( ; ; )
	    {           
		pHelp           = new_help();
		pHelp->level    = fread_number( fp );
		pHelp->keyword  = fread_string( fp );
		if ( pHelp->keyword[0] == '$' )
		    break;
		pHelp->text     = fread_string( fp );
		if ( !str_cmp( pHelp->keyword, "greeting" ) )
		    help_greeting = pHelp->text;
		if ( help_first == NULL )
		    help_first = pHelp;
		if ( help_last  != NULL )
		    help_last->next = pHelp;
		help_last       = pHelp;
		pHelp->next     = NULL;
	    }
	}
	else
	    break;
    }

    fclose( fp );
    return;
}

bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
extern bool close_control;

void do_reboot( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    extern char program_name[];
    extern int port_number, control_socket;
    FILE *file;
    char *argv[]  = { program_name, "", NULL };
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    int temp;

    sprintf( buf, "\nReboot by %s.", ch->name );
    system( "/home/weave/scripts/kill_resolv.pl" );

    one_argument( argument, arg );

    if ( arg[0] != '\0' && !str_prefix(arg, "helps") )
    {
	reboot_helps( ch );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	write_to_descriptor(d->descriptor,buf,0);
	write_to_descriptor( d->descriptor,
	    "\nRebooting, please wait.\n", 0 );

	if (d->ipid > 0)          /* Kill any old resolve */
	{
	    kill(d->ipid, SIGKILL);
	    waitpid(d->ipid,&temp,0);
	    close( d->ifd );
	}
    }

    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }
    send_to_char( "You saved the world.\n\r", ch );

    save_helps();
    save_guilds( );
    do_force( ch, "all save" );
    do_save( ch, "" );
    merc_down = TRUE;

    if ( (file = fopen( "reboot.tmp", "w" )) == NULL
    ||   (*argument != '\0' && !str_cmp( argument,"kill" )) )
    {
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;
            close_socket( d );
        }
        return;
    }

    close_control = FALSE;

    fprintf(file,"%d\n",control_socket);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
       if (d->connected == CON_PLAYING && CH(d) && CH(d)->in_room != NULL)
          fprintf(file,"CHAR %d %s~\n",
                       d->descriptor,CH(d)->name);

       else fprintf(file,"DESC %d\n",d->descriptor);
    }
    fprintf(file,"DESC -1\n");
    fclose( file );

    log_string( "Rebooting.");

    sprintf( buf, "%d", port_number );
    argv[1] = buf;
    execv( program_name, argv );
    exit( 1 );
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );
    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }
    send_to_char( "You saved the world.\n\r", ch );

    save_helps();
    save_guilds( );
    do_force ( ch, "all save");
    do_save (ch, "");
    merc_down = TRUE;
    system( "rm -f command.list.back.gz" );
    system( "gzip command.list.back" );
    system( "mv command.list command.list.back" );
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	close_socket(d);
    }
    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
        wiznet("$N stops being such a snoop.",
                ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
        (IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    if ( ch->prompt != NULL )
    {
	free_string( victim->prompt );
	victim->prompt = str_dup( ch->prompt );
    }
    /* change communications to match */
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    if ( !IS_NULLSTR(ch->prompt) )
    {
	free_string( ch->prompt );
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    send_to_char( "You return to your original body.\n\r", ch );
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
        wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
        sprintf(buf,"$N clones %s.",clone->short_descr);   
        wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));

        return;
    }
}


/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_ml_olc(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    sprintf( buf, "mob %s", argument);
    do_load( ch, buf);
    return;
}

void do_ol_olc(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    sprintf( buf, "obj %s", argument);
    do_load( ch, buf);
    return;
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( victim != ch && IS_NPC(victim) )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !is_number(arg2) )
    {
        send_to_char( "Second argument must be a number.\n\r", ch );
        return;
    }


    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 1 to 110.\n\r", ch );
	return;
    }

    if ( level > ch->level )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;
        int temp_train;
 
	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	temp_train = victim->train;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit		= 3 * number_fuzzy( class_table[ch->class].hp ) + dice( 6, 3 ) + 60;
	victim->max_stamina	= 5 * number_fuzzy( get_curr_stat(ch, STAT_CON) ) +number_range( 1, get_curr_stat(ch, STAT_STR) )+ dice( 2, 10 ) + 60;
	victim->pcdata->perm_hit	= victim->max_hit;
	victim->pcdata->perm_stamina	= victim->max_stamina;
	victim->practice = 0;
	victim->train = 0;
	victim->hit      = victim->max_hit;
	victim->stamina  = victim->max_stamina;
	advance_level( victim, FALSE );
	victim->train	 = temp_train;
	victim->practice = temp_prac;
   }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim, FALSE );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 110 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 110.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
	    vch->body	= 0;
	    if ( !IS_NPC(vch) )
	    {
		vch->pcdata->condition[COND_DRUNK]	= 0;
		vch->pcdata->condition[COND_FULL]	= 48;
		vch->pcdata->condition[COND_THIRST]	= 48;
	    }
            
            vch->hit 	= vch->max_hit;
            vch->stamina= vch->max_stamina;
	    vch->body	= 0;
            update_pos( vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }
        
        send_to_char("Room restored.\n\r",ch);
        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));   
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL-5 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
	    victim->body	= 0;
	    if ( !IS_NPC(victim) )
	    {
		victim->pcdata->condition[COND_DRUNK]	= 0;
		victim->pcdata->condition[COND_FULL]	= 48;
		victim->pcdata->condition[COND_THIRST]	= 48;
	    }
            
            victim->hit 	= victim->max_hit;
            victim->stamina	= victim->max_stamina;
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit		= victim->max_hit;
    victim->stamina	= victim->max_stamina;
    victim->body	= 0;
    if ( !IS_NPC(victim) )
    {
	victim->pcdata->condition[COND_DRUNK]	= 0;
	victim->pcdata->condition[COND_FULL]	= 48;
	victim->pcdata->condition[COND_THIRST]	= 48;
    }
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
        IS_NPC(victim) ? victim->short_descr : victim->name); 
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
        sprintf(buf,"$N thaws %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
        sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
        sprintf(buf,"$N restores emotes to %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
        sprintf(buf,"$N revokes %s's emotes.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
        sprintf(buf,"$N restores shouts to %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
        sprintf(buf,"$N revokes %s's shouts.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);

    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
        sprintf(buf,"$N restores tells to %s.",victim->name); 
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);   
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
        sprintf(buf,"$N revokes %s's tells.",victim->name); 
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		ban_free;

/*
void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    strcat( buf, "\n\r" );
	}
	send_to_char( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    pban		= new_ban( );

    pban->name	= str_dup( arg );
    pban->next	= ban_list;
    ban_list	= pban;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    free_string( curr->name );
	    curr->next	= ban_free;
	    ban_free	= curr;
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}
*/


void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
        wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
        wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }
    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
        wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
        wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    int sn;
    int found = FALSE;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    buffer = new_buf();

    if ( !str_cmp( argument, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
            if (found)
                add_buf(buffer,
"============================================================\n\r\n\r");

	    sprintf( buf, "Sn: %3d  Slot: %3d  %s: '%s'\n\r",
		sn,
		skill_table[sn].slot,
		skill_table[sn].spell_fun == spell_null ? "skill" : "spell",
		skill_table[sn].name );
	    add_buf( buffer, buf );
	    sprintf( buf, "Required level:\n\r"
			  "  Warrior: %3d  Rogue: %3d  Scholar: %3d\n\r", 
		skill_table[sn].skill_level[0],
		skill_table[sn].skill_level[1],
		skill_table[sn].skill_level[2] );
	    add_buf( buffer, buf );
	    sprintf( buf, "Difficulty:\n\r"
			  "  Warrior: %3d  Rogue: %3d  Scholar: %3d\n\r", 
		skill_table[sn].rating[0],
		skill_table[sn].rating[1],
		skill_table[sn].rating[2] );
	    add_buf( buffer, buf );
	    if ( skill_table[sn].spell_fun != spell_null )
	    {
		sprintf( buf, "Required flows:\n\r"
			      "  Earth: %3d  Air: %3d  Fire: %3d  Water: %3d  Spirit: %3d\n\r",
		    skill_table[sn].power[0], skill_table[sn].power[1],
		    skill_table[sn].power[2], skill_table[sn].power[3],
		    skill_table[sn].power[4] );
		add_buf( buffer, buf );
	    }
	    found = TRUE;
	}
    }
    else
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( !is_name(argument, skill_table[sn].name) )
		continue;

            if (found)
                add_buf(buffer,
"============================================================\n\r\n\r");

	    sprintf( buf, "Sn: %3d  Slot: %3d  %s: '%s'\n\r",
		sn,
		skill_table[sn].slot,
		skill_table[sn].spell_fun == spell_null ? "skill" : "spell",
		skill_table[sn].name );
	    add_buf( buffer, buf );
	    sprintf( buf, "Required level:\n\r"
			  "  Warrior: %3d  Rogue: %3d  Scholar: %3d\n\r", 
		skill_table[sn].skill_level[0],
		skill_table[sn].skill_level[1],
		skill_table[sn].skill_level[2] );
	    add_buf( buffer, buf );
	    sprintf( buf, "Difficulty:\n\r"
			  "  Warrior: %3d  Rogue: %3d  Scholar: %3d\n\r", 
		skill_table[sn].rating[0],
		skill_table[sn].rating[1],
		skill_table[sn].rating[2] );
	    add_buf( buffer, buf );
	    if ( skill_table[sn].spell_fun != spell_null )
	    {
		sprintf( buf, "Required flows:\n\r"
			      "  Earth: %3d  Air: %3d  Fire: %3d  Water: %3d  Spirit: %3d\n\r",
		    skill_table[sn].power[0], skill_table[sn].power[1],
		    skill_table[sn].power[2], skill_table[sn].power[3],
		    skill_table[sn].power[4] );
		add_buf( buffer, buf );
	    }
	    found = TRUE;
	}
    }
    if ( !found )
    {
	send_to_char( "No skill or spell found named that.\n\r", ch );
	return;
    }
    page_to_char( buf_string(buffer), ch);
    free_buf( buffer );
    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
        send_to_char("  set group <name> <group>\n\r",ch);
        send_to_char("  set talent <name> <talent>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"group") && ch->level >= 109)
    {
	do_gset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"talent") && ch->level >= 109)
    {
	do_tset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_gset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int gn;
    bool fAll, value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set group <name> <group> on/off\n\r", ch);
	send_to_char( "  set group <name> all on/off\n\r",ch);  
	send_to_char("   (use the name of the group, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp(arg3, "on")
    ||   !str_cmp(arg3, "true") )
	value = TRUE;
    else if ( !str_cmp(arg3, "off")
    ||   !str_cmp(arg3, "false") )
	value = FALSE;
    else
    {
	send_to_char( "Set the group to on or off?\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    gn   = 0;
    if ( !fAll && ( gn = group_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such group.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( gn = 0; gn < MAX_GROUP; gn++ )
	{
	    if ( value && group_table[gn].name != NULL )
		group_add( victim, group_table[gn].name, FALSE );
	    if ( !value && group_table[gn].name != NULL )
		group_remove( victim, group_table[gn].name );
	}
    }
    else
    {
	if ( value )
	    group_add( victim, group_table[gn].name, FALSE );
	else
	    group_remove( victim, group_table[gn].name );
    }

    return;
}


void do_tset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int tn;
    bool fAll, value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set talent <name> <talent> on/off\n\r", ch);
	send_to_char( "  set talent <name> all on/off\n\r",ch);  
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp(arg3, "on")
    ||   !str_cmp(arg3, "true") )
	value = TRUE;
    else if ( !str_cmp(arg3, "off")
    ||   !str_cmp(arg3, "false") )
	value = FALSE;
    else
    {
	send_to_char( "Set the talent to on or off?\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    tn   = 0;
    if ( !fAll && ( tn = talent_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( tn = 0; tn < MAX_TALENT; tn++ )
	{
	    if ( skill_table[tn].name != NULL )
		victim->pcdata->talent[tn]	= value;
	}
    }
    else
    {
	victim->pcdata->talent[tn] = value;
    }

    return;
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < -1 || value > 100 )
    {
	send_to_char( "Value range is -1 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con cha luc agi sex class level\n\r",	ch );
	send_to_char( "    race guild shadow rank gold hp stamina practice\n\r",	ch );
	send_to_char( "    train thirst drunk full security track experience\n\r", ch );
	send_to_char( "    earth air fire water spirit\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d.\n\r",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity ranges is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CHR) )
	{
	    sprintf(buf,
		"Charisma range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CHR));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CHR] = value;
	return;
    }

    if ( !str_cmp( arg2, "luc" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_LUK) )
	{
	    sprintf(buf,
		"Luck range is 3 to %d.\n\r",
		get_max_train(victim,STAT_LUK));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_LUK] = value;
	return;
    }

    if ( !str_cmp( arg2, "agi" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_AGI) )
	{
	    sprintf(buf,
		"Agility range is 3 to %d.\n\r",
		get_max_train(victim,STAT_AGI));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_AGI] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 60 )
	{
	    send_to_char( "Level range is 0 to 60.\n\r", ch );
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "stamina" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Stamina range is 0 to 30,000 stamina points.\n\r", ch );
	    return;
	}
	victim->max_stamina = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_stamina = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim))
	{
	    send_to_char("You cannot change the race of players.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }

    if ( !str_prefix( arg2, "guild" ) )
    {
	int guild;
        guild = guild_lookup( argument );

	if ( ch->level < 106 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

        if ( guild == -1 )
        {
            GUILD_DATA *pGuild;
            char buf[MAX_STRING_LENGTH];
            int count = 0;
        
            send_to_char( "Available guilds are:", ch );
        
            for ( pGuild = guild_first; pGuild != NULL; pGuild = pGuild->next )
            {
                if ( (count % 3) == 0 )
                    send_to_char( "\n\r", ch );
                sprintf( buf, " %-20.20s", pGuild->name );
                send_to_char( buf, ch );
                count++;  
            }
            send_to_char( "\n\r", ch );
            return;
        }

	victim->guild = guild;
	return;
    }

    if ( !str_prefix(arg2, "experience") )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char( "This option may only be used on players.\n\r", ch );
	    return;
	}

	if ( ch->level < 109 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < 0 )
	{
	    send_to_char( "You cannot assign negative experience.\n\r", ch );
	    return;
	}
	victim->exp = value;
	return;
    }

    if ( !str_prefix(arg2, "earth") )
    {
	if ( ch->level < 108 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->channel_max[0] = value;
	return;
    }

    if ( !str_prefix(arg2, "air") )
    {
	if ( ch->level < 108 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->channel_max[1] = value;
	return;
    }

    if ( !str_prefix(arg2, "fire") )
    {
	if ( ch->level < 108 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->channel_max[2] = value;
	return;
    }

    if ( !str_prefix(arg2, "water") )
    {
	if ( ch->level < 108 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->channel_max[3] = value;
	return;
    }

    if ( !str_prefix(arg2, "spirit") )
    {
	if ( ch->level < 108 )
	{
	    send_to_char( "You are not high enough level for this.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->channel_max[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "security" ) ) /* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( (value > ch->pcdata->security) || (value < 0) )
        {
            if ( ch->pcdata->security != 0 )
            {
                sprintf( buf, "Valid security is 0-%d.\n\r",
                    ch->pcdata->security );
                send_to_char( buf, ch );
            }
            else
            {
                send_to_char( "Valid security is 0 only.\n\r", ch );
            }
            return;
        }
        victim->pcdata->security = value;
        return;
    }

    else if (!str_prefix(arg2, "track"))
    {
        CHAR_DATA *hunted = NULL;

        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( str_cmp( arg3, "." ) )
          if ( (hunted = get_char_world(victim, arg3)) == NULL )
            {
              send_to_char("Mob couldn't locate the victim to hunt.\n\r", ch);
              return;
            }

        victim->hunting = hunted;
        return;
    }

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}
	return;
    }

    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr( );

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
        if ( ( value = flag_value( extra_flags, arg3 ) ) != NO_FLAG )
        {
            TOGGLE_BIT(obj->extra_flags, value);
            return;
        }
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
        if ( ( value = flag_value( wear_flags, arg3 ) ) != NO_FLAG )
        {
            TOGGLE_BIT(obj->wear_flags, value);
	    return;
	}
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	if ( ( value = flag_value( room_flags, arg3 ) ) != NO_FLAG )
	{
	    TOGGLE_BIT(location->room_flags, value);
	    return;
	}
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	if ( ( value = flag_value( sector_flags, arg3 ) ) != NO_FLAG )
	{
	    location->sector_type	= value;
	    return;
	}
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
    BUFFER *buf;
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *tmp;
    DESCRIPTOR_DATA *d;
    int count;
    bool	fNotconnect = FALSE;

    count	= 0;

    buf = new_buf();
    one_argument(argument,arg);
    if ( arg[0] == '-' )
    {
	fNotconnect = TRUE;
	tmp = arg+1;
    }
    else
	tmp = arg;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( (d->character != NULL && !fNotconnect)
	&& can_see( ch, d->character ) 
	&& (tmp[0] == '\0' || is_name(tmp,d->character->name)
			   || (d->original && is_name(tmp,d->original->name))))
	{
            char addr[MAX_STRING_LENGTH];
            count++;
            if ( !str_cmp( d->ident, "???" ) )
                strcpy( addr, d->host );
            else
            	sprintf( addr, "%s`n@`4%s", d->ident, d->host );
            sprintf( buf2, 
		"[`4%3d %2d`n] "
		"(%2d:%2d:%2d) `4%s`n (`4%s`n)"
		" Level: %d %s\r\n",
                d->descriptor,
                d->connected,
		(d->idle / 3600) % 60, (d->idle / 60) % 60, d->idle % 60,
                d->original  ? d->original->name  :
                d->character ? d->character->name : "(none)",
                addr,
		d->original  ? d->original->level  :
		d->character ? d->character->level : 0,
		d->pString ? "`B(Writing)`n" : ""
		);
            add_buf( buf, buf2 );
        }
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\r\n", count, count == 1 ? "" : "s" );
    add_buf( buf, buf2 );
    page_to_char( buf_string(buf), ch );
    free_buf( buf );
    return;
}


void do_snoops( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int p_count, s_count;

    p_count	= 0;
    s_count	= 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	p_count++;
	if ( d->character
	&&   can_see(ch, d->character)
	&&   d->snoop_by
	&&   d->snoop_by->character )
	{
	    s_count++;
	    sprintf( buf, "%s is being snooped by %s.\n\r", d->character->name,
		d->snoop_by->character->name );
	    send_to_char( buf, ch );
	}
    }

    send_to_char_new( ch, "Players: %d  Snoops: %d\n\r", p_count, s_count );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( IS_SET(ch->act, PLR_WIZINVIS) )
      {
	  REMOVE_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  SET_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = UMIN( MAX_LEVEL-9, get_trust(ch) );
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          SET_BIT(ch->act, PLR_WIZINVIS);
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_home( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *home_room;
    CHAR_DATA *rch;
    char num[10];

    sprintf(num, "%d", ch->home );

    home_room = find_location( ch, num );
     if ( home_room == NULL )
    {
	send_to_char( "That room no longer exists.", ch );
	ch->home = 0;
	return;
    }

    if ( ch->on != NULL )
	do_stand( ch, "" );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout))
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    char_from_room( ch );
    send_to_char( "You decide to go home.\n\r", ch );
    char_to_room( ch, home_room );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin))
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_look( ch, "moved" );

    return;
}

void do_sethome( CHAR_DATA *ch, char *argument )
{
    int new_home;
    new_home = ch->in_room->vnum;

    ch->home = new_home;
    send_to_char( "Okay, this room is your new home.\n\r", ch );
    return;
}

void do_sfind( CHAR_DATA *ch, char *argument )
{
    do_slookup( ch, argument );

    return;
}


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

  Various administrative utility commands.
  Version: 3 - Last update: January 1996.

  To use these 2 commands you will have to add a filename field to AREA_DATA.
  This value can be found easily in load_area while booting - the filename
  of the current area boot_db is reading from is in the strArea global.

  Since version 2 following was added:

  A rename command which renames a player. Search for do_rename to see
  more info on it.

  A FOR command which executes a command at/on every player/mob/location.

  Fixes since last release: None.


*/

/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players.
 * .gz files are checked for too, just in case.
 */

void do_newname (CHAR_DATA* ch, char* argument)
{
        
    char old_name[MAX_INPUT_LENGTH],
	new_name[MAX_INPUT_LENGTH],
	strsave [MAX_INPUT_LENGTH],
	filename[MAX_INPUT_LENGTH];

    CHAR_DATA* victim;
    FILE* file;

    argument = one_argument(argument, old_name);
    one_argument (argument, new_name);

    if (!old_name[0])
    {
	send_to_char ("Rename who?\n\r",ch);
	return;
    }

        victim = get_char_world (ch, old_name);

        if (!victim)
        {
            send_to_char ("There is no such a person online.\n\r",ch);
            return;
        }

        if (IS_NPC(victim))
        {
            send_to_char ("You cannot use newname on NPCs.\n\r",ch);
            return;
        }

        /* allow rename self new_name,but otherwise only lower level */
        if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
        {
            send_to_char ("You failed.\n\r",ch);
            return;
        }

        if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
        {
                send_to_char ("This player has lost his link or is inside a pager or the like.\n\r", ch );
                return;
        }

        if (!new_name[0])
        {
            send_to_char ("Rename to what new name?\n\r",ch);
            return;
        }

        if (!check_parse_name(new_name))
        {
            send_to_char ("The new name is illegal.\n\r",ch);
            return;
        }

    sprintf( filename, "%s", correct_name(new_name) );

        /* First, check if there is a player named that off-line */
    fclose (fpReserve); /* close the reserve file */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, "archive/",
        filename );
    if ( ( file = fopen( strsave, "r" ) ) != NULL )
    {
	send_to_char ("A player with that name already exists!\n\r",ch);
        fclose(file);
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
    fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

    fclose (fpReserve); /* close the reserve file */
    sprintf( strsave, "%s%c/%s%s", PLAYER_DIR,   
        LOWER(filename[0]), filename,".gz");
    if ( ( file = fopen( strsave, "r" ) ) != NULL )
    {
	send_to_char ("A player with that name already exists!\n\r",ch);
        fclose(file);
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
    fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

    sprintf( strsave, "%s%c/%s", PLAYER_DIR, LOWER(new_name[0]), filename );

        
    fclose (fpReserve); /* close the reserve file */
    file = fopen (strsave, "r"); /* attempt to to open pfile */
    if (file) 
    {
	send_to_char ("A player with that name already exists!\n\r",ch);
	fclose (file);
	fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these */
	return;
    }
    fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        if (get_char_sedai(ch,new_name))
        {
                send_to_char ("A player with the name you specified already exists!\n\r",ch);
                return;
        }

        /* Save the filename of the old name */

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, LOWER(victim->name[0]), 
	correct_name(victim->name) );
#else
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, LOWER(victim->name[0]), 
	correct_name(victim->name) );
#endif


        /* Rename the character and save him to a new file */
        /* NOTE: Players who are level 1 do NOT get saved under a new name */

        free_string (victim->name);
        victim->name = str_dup (capitalize(new_name));

        save_char_obj (victim);

        /* unlink the old file */
        unlink (strsave); /* unlink does return a value.. but we do not care */

        /* That's it! */

        send_to_char ("Character renamed.\n\r",ch);

        victim->position = POS_STANDING; /* I am laaazy */
        act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);

} /* do_rename */

void do_permdelet (CHAR_DATA *ch, char *argument )
{
    send_to_char( "You must fully type the command to delete someone.", ch );
}

void do_permdelete (CHAR_DATA* ch, char* argument)
{
        char strsave [MAX_INPUT_LENGTH];

        CHAR_DATA* victim;
        FILE* file;


        /* Trivial checks */
        if (!argument[0])
        {
                send_to_char ("Delete who?\n\r",ch);
                return;
        }

        victim = get_char_world (ch, argument);

        if (!victim)
        {
                send_to_char ("There is no such a person online.\n\r",ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char ("You cannot use delete on NPCs.\n\r",ch);
                return;
        }

        /* allow rename self new_name,but otherwise only lower level */
        if ( ((victim != ch) && (get_trust (victim) >= get_trust (ch))) ||
	     (victim == ch))
        {
                send_to_char ("You failed.\n\r",ch);
                return;
        }

        
    if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
    {
	send_to_char ("This player has lost his link or is inside an editor or the like.\n\r", ch );
	return;
    }

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, LOWER(argument[0]),
	correct_name(argument) );
#else
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, LOWER(argument[0]), 
	correct_name(argument) );
#endif

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                fclose (file);
		do_quit( victim, "" );
                unlink (strsave); /* unlink does return a value.. but we do not care */
        fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these */
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        /* Check .gz file ! */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%c/%s.gz", PLAYER_DIR, LOWER(argument[0]), 
	correct_name(argument) );
#else
    sprintf( strsave, "%s%c/%s.gz", PLAYER_DIR, LOWER(argument[0]),
	correct_name(argument) );
#endif

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                fclose (file);
		do_quit( victim, "" );
                unlink (strsave); /* unlink does return a value.. but we do not care */
        fpReserve = fopen( NULL_FILE, "r" );
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

        send_to_char ("That filename does not exist.\n\r",ch);
} /* do_rename */

void do_pfind( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%-15.15s %-25.25s %-5s %-25.25s\n\r", "Name", "Room", "Vnum", "Area" );
    send_to_char( buf, ch );
    send_to_char(
	"--------------- ------------------------- ----- -------------------------\n\r",
	ch );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character == NULL || d->connected != 0
	||   !can_see(ch, d->character) )
	    continue;

	vch = d->character;

	sprintf( buf, "%-15.15s %-25.25s %-5d %-25.25s\n\r", vch->name, vch->in_room->name,
	    vch->in_room->vnum, vch->in_room->area->name );
	send_to_char( buf, ch );
    }
    return;
}

void do_mcount( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Mcount whom?\n\r", ch );
	return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    strcpy( buffer, victim->name );
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    {
	sprintf( buf, "You have found %d of %s (%s).\n\r", count,
	    argument, buffer );
	send_to_char( buf, ch );
    }

    return;
}

char * area_name (AREA_DATA *pArea)
{
        static char buffer[64]; /* short filename */
        char  *period;

        assert (pArea != NULL);
  
        strncpy (buffer, pArea->filename, 64); /* copy the filename */
        period = strchr (buffer, '.'); /* find the period (midgaard.are) */
        if (period) /* if there was one */
                *period = '\0'; /* terminate the string there (midgaard) */
 
        return buffer;
}

typedef enum {exit_from, exit_to, exit_both} exit_status;
  
/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex,  char *buffer)

{
        char *sExit;
 
    switch (ex)   
    {
	default:
	    sExit = "??"; break; /* invalid usage */
	case exit_from:
	    sExit = "< "; break;
	case exit_to:
	    sExit = " >"; break;
	case exit_both:
            sExit = "<>"; break;
    }

    sprintf( buffer, "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r",
	left->vnum, left->name,
	sExit,
	right->vnum, right->name,
	area_name(right->area) );
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
    char buf[MAX_STRING_LENGTH];
    int i;
    EXIT_DATA *exit;
    ROOM_INDEX_DATA *to_room;

    strcpy (buffer, "");
    for (i = 0; i < 6; i++)
    {
	exit = room->exit[i];
	if (!exit)
	    continue;
	else
	    to_room = exit->u1.to_room;

    if (to_room)  /* there is something on the other side */
    {
        if ( (room->area == pArea) && (to_room->area != pArea) )
        {	/* an exit from our area to another area */
          	/* check first if it is a two-way exit */
        
            if ( to_room->exit[opposite_dir[i]] &&
                 to_room->exit[opposite_dir[i]]->u1.to_room == room )
                room_pair(room,to_room,exit_both,buf);
            else
                room_pair(room,to_room,exit_to,buf);

            strcat (buffer, buf);
        }
        else if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) )
        { /* an exit from another area to our area */ 
            if  ( !(to_room->exit[opposite_dir[i]] &&
                    to_room->exit[opposite_dir[i]]->u1.to_room == room) )
            /* two-way exits are handled in the other if */
            {
		room_pair( to_room, room, exit_from, buf );
		strcat(buffer, buf);
	    }
	} /* if room->area */
    } /* for */
    }
}
                                 
/* for now, no arguments, just list the current area */
void do_exlist(CHAR_DATA *ch, char * argument)
{
    AREA_DATA* pArea;
    ROOM_INDEX_DATA* room;
    int i;                   
    char buffer[MAX_STRING_LENGTH];
                                          
    /* run through all the rooms on the MUD */
    pArea = ch->in_room->area; /* this is the area we want info on */
    for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
    {
        for (room = room_index_hash[i]; room != NULL; room = room->next)
 	{
            checkexits (room, pArea, buffer);
	    send_to_char (buffer, ch);
        }
    }
}

/* show a list of all used VNUMS */
        
#define COLUMNS                 5   /* number of columns */
#define MAX_ROW                 ((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */
        
void do_vlist (CHAR_DATA *ch, char *argument)
{
    int i,j,vnum, k;
    AREA_DATA *area;
    char buffer[MAX_ROW*100]; /* should be plenty */
    char buf2 [100];
         
    for (i = 0; i < MAX_ROW; i++)
    {
	strcpy (buffer, ""); /* clear the buffer for this row */
 
        for (j = 0; j < COLUMNS; j++) /* for each column */
        {
	    vnum = ((j*MAX_ROW) + i);
            if (vnum < MAX_SHOW_VNUM)
            {
		bool found = FALSE;
		area = NULL;
		for ( k = vnum * 100; k < vnum * 100 + 100; k++ )
		{
		    for ( area = area_first; area; area = area->next )
		    {
			if ( k >= area->lvnum && k <= area->uvnum )
			{
			    found = TRUE;
			    break;
			}
		    }
		}
		if ( found && area )
		    sprintf( buf2, "%3d %-8.8s  ", vnum, area_name(area) );
		else
		    sprintf( buf2, "%3d %-8.8s  ", vnum, "-" );
                strcat (buffer,buf2);
            }
        } /* for columns */

	send_to_char (buffer,ch);
        send_to_char ("\n\r",ch);
    } /* for rows */
}

void do_ocount( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Ocount what?\n\r", ch );
	return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( is_name( argument, obj->name ) )
	{
	    found = TRUE;
	    count++;
	    strcpy( buffer, obj->name );
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    {
	sprintf( buf, "You have found %d of %s (%s).\n\r", count,
	    argument, buffer );
	send_to_char( buf, ch );
    }

    return;
}


void do_rlist( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoom;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    char		buf1 [ MAX_STRING_LENGTH*4 ];
    bool found;
    int vnum;
    int  col = 0;

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) )
	{
		sprintf( buf, "[%5d] %-17.16s",
		    pRoom->vnum, capitalize( pRoom->name ) );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\n\r" );
	        found = TRUE;
	}
    }

    if ( !found )
    {
	send_to_char( "Room(s) not found in this area.\n\r", ch);
	return;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );

    page_to_char( buf1, ch );
    return;
}

void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *vobj;
    bool found;

    if ( argument[0] == '\0' )
    {
	send_to_char( "owhere what?\n\r", ch );
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( vobj = object_list; vobj != NULL; vobj = vobj->next )
    {
	if ( vobj->in_room != NULL
	&&   is_name( argument, vobj->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28.26s [%5d] %-15.15s\n\r",
		vobj->pIndexData->vnum,
		vobj->short_descr,
		vobj->in_room->vnum,
		vobj->in_room->name );
	    buf[0] = UPPER(buf[0]);
	    add_buf(buffer, buf);
	}
	else if ( vobj->in_room == NULL &&
		  vobj->carried_by != NULL &&
		  is_name( argument, vobj->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28.26s carried by [%5d] %-15.15s\n\r",
		vobj->pIndexData->vnum,
		vobj->short_descr,
		IS_NPC(vobj->carried_by) ? vobj->carried_by->pIndexData->vnum :
		0,
		PERS( vobj->carried_by, ch ) );
	    buf[0] = UPPER(buf[0]);
	    add_buf(buffer, buf);
	}
	else if ( vobj->in_room == NULL &&
		  vobj->in_obj != NULL &&
		  is_name( argument, vobj->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28.26s stored in [%5d] %-15.15s\n\r",
		vobj->pIndexData->vnum,
		vobj->short_descr,
		vobj->in_obj->pIndexData->vnum,
		vobj->in_obj->short_descr );
	    buf[0] = UPPER(buf[0]);
	    add_buf(buffer, buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
	if (ch->lines)
	    page_to_char(buf_string(buffer),ch);
	else
	    send_to_char(buf_string(buffer),ch);

    free_buf(buffer);
    return;
}


void do_mlist( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    char		arg  [ MAX_INPUT_LENGTH   ];
    BUFFER		*buffer;
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
	return;
    }
    buffer = new_buf();

    pArea = ch->in_room->area;
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) )
	{
	    if ( fAll || is_name(arg, pMobIndex->player_name) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pMobIndex->vnum, capitalize(pMobIndex->short_descr) );
		add_buf( buffer, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buffer, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Mob(s) not found in this area.\n\r", ch);
	return;
    }

    if ( col % 3 != 0 )
	add_buf( buffer, "\n\r" );

    page_to_char( buf_string(buffer), ch );
    free_buf(buffer);            
    return;
}



void do_olist( CHAR_DATA *ch, char*argument )
{
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    char		buf1 [ MAX_STRING_LENGTH*4 ];
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  olist <all/name/item_type>\n\r", ch );
	return;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll || is_name( arg, pObjIndex->name )
	    || flag_value( type_flags, arg ) == pObjIndex->item_type )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Object(s) not found in this area.\n\r", ch);
	return;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );

    page_to_char( buf1, ch );
    return;
}

void do_rename( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' && IS_DISGUISED(ch) )
    {
	send_to_char( "Restoring original name.\n\r", ch );
	remove_shape( ch );
	return;
    }
    else if ( argument[0] == '\0' && !IS_DISGUISED(ch) )
    {
	send_to_char( "What do you want your new name to be?\n\r", ch );
	return;
    }

    strcpy( arg, argument );
    smash_tilde( arg );

    remove_shape( ch );
    ch->pcdata->new_name = str_dup( arg );
    SET_BIT( ch->affected_by, AFF_SHAPE_CHANGE );

    sprintf( buf, "Okay, you are now known as `4%s`n.\n\r",ch->pcdata->new_name );
    send_to_char( buf, ch );
    return;
}

void do_resex( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Restoring true sex.\n\r", ch );
	ch->sex = ch->pcdata->true_sex;
	return;
    }

    if ( str_prefix(arg, "male")
    &&   str_prefix(arg, "female")
    &&   str_prefix(arg, "man")
    &&   str_prefix(arg, "woman") )
    {
	send_to_char( "What sex would you like to be?\n\r", ch );
	return;
    }

    if ( !str_prefix(arg, "male")
    ||   !str_prefix(arg, "man") )
    {
	ch->sex = 1;
	send_to_char( "Okay, your sex is set to male.\n\r", ch );
	return;
    }

    if ( !str_prefix(arg, "female")
    ||   !str_prefix(arg, "woman") )
    {
	ch->sex = 2;
	send_to_char( "Okay, your sex is set to female.\n\r", ch );
	return;
    }
    return;
}


void do_toggle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Toggle whose bits?\n\r", ch );
	return;
    }

    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "Could not find them anywhere.\n\r", ch );
	return;
    }

    if ( (value = flag_value( affect_flags, arg2 )) != NO_FLAG )
    {
	char buf[MAX_STRING_LENGTH];
	TOGGLE_BIT(victim->affected_by, value);
	sprintf( buf, "Bit toggled.  Current value is: %s\n\r",
	    IS_SET(victim->affected_by, value) ? "`2on`n" : "`3off`n" );
	send_to_char( buf, ch );
	return;
    }

    if ( (value = flag_value( affect_2_flags, arg2 )) != NO_FLAG )
    {
	char buf[MAX_STRING_LENGTH];
	TOGGLE_BIT(victim->affected_by_2, value);
	sprintf( buf, "Bit toggled.  Current value is: %s\n\r",
	    IS_SET(victim->affected_by_2, value) ? "`2on`n" : "`3off`n" );
	send_to_char( buf, ch );
	return;
    }
    send_to_char( "That is not an affect.\n\r", ch );
    return;
}


void do_mforce( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) || ch->in_room != vch->in_room )
		continue;
	    interpret( vch, argument );
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) )
	{
	    send_to_char( "You may only use this command on NPCs\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_transout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );

        if (argument[0] == '\0')
        {
	    send_to_char( "Reseting transout message.\n\r", ch );
	    free_string( ch->pcdata->transout );
	    ch->pcdata->transout = str_dup( "disappears in a cloud of smoke." );
            return;
        }

	strcat( argument, "`n" );
        free_string( ch->pcdata->transout );
        ch->pcdata->transout = str_dup( argument );

        sprintf(buf,"Your transout message is now `4%s`n\n\r",
	    ch->pcdata->transout);
        send_to_char(buf,ch);
    }
    return;
}


void do_transin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );

        if (argument[0] == '\0')
        {
            send_to_char( "Reseting transin message.\n\r", ch );
            free_string( ch->pcdata->transin );
            ch->pcdata->transin = str_dup( "appears from a cloud of smoke." );
            return;
        }

	strcat( argument, "`n" );
        free_string( ch->pcdata->transin );
        ch->pcdata->transin = str_dup( argument );

        sprintf(buf,"Your transin message is now `4%s`n\n\r",
	    ch->pcdata->transin);
        send_to_char(buf,ch);
    }
    return;
}



void do_vstat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int num;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2) || (str_prefix(arg1,"mob") && str_prefix(arg1,"obj")) )
    {
        send_to_char("Syntax:\n\r",ch);
        send_to_char("   vstat obj <vnum>\n\r",ch);
        send_to_char("   vstat mob <vnum>\n\r",ch);
        return;
    }

    if ( (num = atoi(arg2)) < 0)
    {
       send_to_char("A negative vnum? I think not.\r\n",ch);
       return;
    }

    if ( !str_prefix(arg1,"mob"))
    {
       if ( (pMobIndex = get_mob_index( num )) == NULL )
       {
          send_to_char("There is no mob with that number.\n\r",ch);
          return;
       }

       victim = create_mobile( pMobIndex );
       char_to_room( victim, ch->in_room );
       do_mstat(ch, victim->name);
       extract_char(victim, TRUE);
       return;
    }

    if ( !str_prefix(arg1,"obj") )
    {
       if ( (pObjIndex = get_obj_index ( num )) == NULL )
       {
          send_to_char("There is no obj with that number.\n\r",ch);
          return;
       }

       obj = create_object ( pObjIndex, 1 );
       obj_to_room( obj, ch->in_room );
       do_ostat(ch, obj->name);
       extract_obj(obj);
       return;
    }
}

void do_lists( CHAR_DATA *ch, char *argument )
{
    NODE_DATA *node;
    char buf[MAX_INPUT_LENGTH];
    int count;

    send_to_char( "PCs:\n\r", ch );
    for ( node = pc_list; node != NULL; node = node->next )
    {
	CHAR_DATA *person;

	if ( node->data_type != NODE_PC )
	    continue;

	person = (CHAR_DATA *) node->data;
	if ( person->desc != NULL )
	    sprintf( buf, "  %s is a player and has link.\n\r", PERS(person, ch) );
	else
	    sprintf( buf, "  %s is a player but is linkless.\n\r", PERS(person, ch) );
	
	send_to_char( buf, ch );
    }

    send_to_char( "Fights:\n\r", ch );
    for ( node = fight_list; node != NULL; node = node->next )
    {
	CHAR_DATA *person;

	if ( node->data_type != NODE_FIGHT )
	    continue;

	person = (CHAR_DATA *) node->data;
	if ( person->fighting != NULL )
	    sprintf( buf, "  %s is fighting %s.\n\r", PERS(person, ch),
		PERS(person->fighting, ch) );
	else
	    sprintf( buf, "  %s is fighting (NULL).\n\r", PERS(person, ch) );
	send_to_char( buf, ch );
    }

    send_to_char( "Affects in the game:\n\r", ch );
    count = 0;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type != NODE_WEAVE_CHAR )
	    continue;

	count++;
    }
    send_to_char_new( ch, "  Affects on characters: %d\n\r", count );
    count = 0;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type != NODE_WEAVE_OBJ )
	    continue;

	count++;
    }
    send_to_char_new( ch, "  Affects on objects: %d\n\r", count );
    count = 0;
    for ( node = weave_list; node != NULL; node = node->next )
    {
	if ( node->data_type != NODE_WEAVE_ROOM )
	    continue;

	count++;
    }
    send_to_char_new( ch, "  Affects on rooms: %d\n\r", count );
    send_to_char( "Items created by spells:\n\r", ch );
    for ( node = weave_list; node != NULL; node = node->next )
    {
	OBJ_DATA *obj;

	if ( node->data_type != NODE_WEAVE_CREATE )
	    continue;

	obj = (OBJ_DATA *) node->data;
	sprintf( buf, "%s was created by %s.\n\r", obj->short_descr,
	    obj->owner->name );
	send_to_char( "  ", ch );
	send_to_char( buf, ch );
    }
    send_to_char( "Characters with wait states:\n\r", ch );
    for ( node = wait_list; node != NULL; node = node->next )
    {
	CHAR_DATA *person;

	if ( node->data_type != NODE_PC )
	    continue;

	person = (CHAR_DATA *) node->data;
	sprintf( buf, "  %s has a wait state of %d.\n\r", PERS(person, ch),
	    person->wait );
	
	send_to_char( buf, ch );
    }
    return;
}

void do_request( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char strsave[MAX_STRING_LENGTH];
    

    one_argument( argument, arg );

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s", PLAYER_DIR, correct_name(arg) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, correct_name(arg) );
#endif          

    return;
}
    

void do_potion( CHAR_DATA *ch, char *argument )
{
    int i;
    for ( i = 0; potion_table[i].name != NULL; i++ )
    {
	send_to_char_new( ch, "Potion: `4%s`n\n\r",
	    potion_table[i].name );
	send_to_char_new( ch, "Ingredients: `4%s`n\n\r",
	    ingredient_string( potion_table[i].ingredients) );
	send_to_char_new( ch, "Difficulty: `4%d`n\n\r",
	    potion_table[i].diff );
	send_to_char( "Potion is: ", ch );
	switch( potion_table[i].type )
	{
	    default:
		send_to_char( "drunk\n\r", ch );
		break;
	    case POTION_DRINK:
		send_to_char( "a liquid\n\r", ch );
		break;
	    case POTION_BALM:
		send_to_char( "a balm/salve\n\r", ch );
		break;
	    case POTION_EAT:
		send_to_char( "pills\n\r", ch );
		break;
	}

	send_to_char( "Affects: ", ch );
	if ( potion_table[i].affect & HERB_HEAL )
	    send_to_char( " heals", ch );
	if ( potion_table[i].affect & HERB_CURE_POISON )
	    send_to_char( " cures_poison", ch );
	if ( potion_table[i].affect & HERB_CURE_DISEASE )
	    send_to_char( " cures_disease", ch );
	if ( potion_table[i].affect & HERB_REFRESH )
	    send_to_char( " refreshes", ch );
	if ( potion_table[i].affect & HERB_SLEEP )
	    send_to_char( " causes_sleep", ch );
	if ( potion_table[i].affect & HERB_SPECIAL )
	    send_to_char( " special", ch );

	if ( potion_table[i].affect & HERB_HURT )
	    send_to_char( " hurts", ch );
	if ( potion_table[i].affect & HERB_POISON )
	    send_to_char( " causes_poison", ch );
	if ( potion_table[i].affect & HERB_DISEASE )
	    send_to_char( " causes_disease", ch );
	if ( potion_table[i].affect & HERB_DRAIN )
	    send_to_char( " drains_stamina", ch );
	if ( potion_table[i].affect & HERB_STOP_CHANNEL )
	    send_to_char( " prevents_channeling", ch );
	send_to_char( "\n\r\n\r", ch );
    }
    return;
}


void do_tool( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  tool <name> <field> <string>\n\r",ch);
	send_to_char("    fields: keyword/name short long extended\n\r",ch);
	return;
    }
    
    /* string an obj */    	
    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }
    	
    if ( !str_prefix(arg2, "name") || !str_prefix(arg2, "keyword") )
    {
	free_string( obj->name );
	obj->name = str_dup( arg3 );
	return;
    }

    if ( !str_prefix( arg2, "short" ) )
    {
	free_string( obj->short_descr );
	obj->short_descr = str_dup( arg3 );
	return;
    }

    if ( !str_prefix( arg2, "long" ) )
    {
	free_string( obj->description );
	obj->description = str_dup( arg3 );
	return;
    	}

    if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    {
	EXTRA_DESCR_DATA *ed;

	argument = one_argument( argument, arg3 );
	if ( argument == NULL )
	{
	    send_to_char( "Syntax: tool <object> ed <keyword> <string>\n\r",
		ch );
	    return;
	}
 	strcat(argument,"\n\r");

	ed = new_extra_descr( );

	ed->keyword		= str_dup( arg3 );
	ed->description		= str_dup( argument );
	ed->next		= obj->extra_descr;
	obj->extra_descr	= ed;
	return;
    }
        	
    /* echo bad use message */
    do_string(ch,"");
}


void do_isay( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( IS_NPC(ch) && IS_SET(ch->comm, COMM_NOCHANNELS)
    &&   ch->pIndexData->pShop != NULL )
    {
        send_to_char( "You have been nochanneled.\n\r", ch );
        return;
    }
    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what?\n\r", ch );
        return;
    }
 
    sprintf( buf, "You say to IMMORTALS: '$T'`n" ); 
    act_immortal( buf, ch, NULL, argument, TO_CHAR );
        
    sprintf( buf, "$n says to IMMORTALS: '$T'`n" );
    act_immortal( buf, ch, NULL, argument, TO_ROOM );

   return;
}

void do_mudstat( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    AREA_DATA *a;
    int count_p = 0;
    int count_a = 0;

    for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING )
	    count_p++;
    for ( a = area_first; a; a = a->next )
	if ( IS_SET(a->area_flags, AREA_COMPLETE) )
	    count_a++;

    send_to_char( "Statistics for Weave:\n\r\n\r", ch );
    send_to_char_new( ch, "Fish resource rooms: %d (out of %d)\n\r",
	num_fish, MAX_FISH );
    send_to_char_new( ch, "Herb resource rooms: %d (out of %d)\n\r",
	num_herb, MAX_HERB );
    send_to_char_new( ch, "Gem resource rooms: %d (out of %d)\n\r",
	num_gem, MAX_GEM );
    send_to_char_new( ch, "Lumber resource rooms: %d (out of %d)\n\r",
	num_lumber, MAX_LUMBER );
    send_to_char_new( ch, "Ore resource rooms: %d (out of %d)\n\r",
	num_ore, MAX_ORE );
    send_to_char_new( ch, "Explore experience rooms: %d (out of %d)\n\r",
	num_explore, count_a * count_p / 15 );
    send_to_char_new( ch, "Connected players: %d (out of %d max today)\n\r",
	count_p, max_on );
    return;
}


/* Super-AT command: 

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>

Executes action several times, either on ALL players (not including
yourself), MORTALS (including trusted characters), GODS (characters with
level higher than L_HERO), MOBS (Not recommended) or every room (not
recommended either!) 
    
If you insert a # in the action, it will be replaced by the name of the
target. 

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room
containg at least one target, but only once per room. # cannot be used
with FOR EVERY- WHERE. # can be anywhere in the action. 

Example:
                
FOR ALL SMILE -> you will only smile once in a room with 2 players.  
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B. 

Destroying the characters this command acts upon MAY cause it to fail. Try
to avoid something like FOR MOBS PURGE (although it actually works at my
MUD). 

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT
work though :) 
                
The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated. 
*/  

/*
   Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/ 
const char * name_expand (CHAR_DATA *ch)
{
    int count = 1;
    CHAR_DATA *rch;
    char name[MAX_INPUT_LENGTH];
    static char outbuf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch)) 
	return ch->name;

    one_argument (ch->name, name); /* copy the first word into name */
                
    if (!name[0]) /* weird mob .. no keywords */
    { 
	strcpy (outbuf, "");
	return outbuf;
    } 
   
    for ( rch = ch->in_room->people; rch && (rch != ch); rch = rch->next_in_room )
	if (is_name (name, rch->name))
	    count++;

    sprintf (outbuf, "%d.%s", count, name);
    return outbuf;
}
        
void do_for (CHAR_DATA *ch, char *argument)
{
    char range[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool fGods = FALSE;
    bool fMortals = FALSE;
    bool fMobs = FALSE;
    bool fEverywhere = FALSE;
    bool fGuild = FALSE;
    bool fQuest = FALSE;
    bool fNewbie = FALSE;
    bool found; 
    ROOM_INDEX_DATA *room, *old_room;
    CHAR_DATA *p, *p_next;
    int i;
          
    argument = one_argument (argument, range);
   
    if (!range[0] || !argument[0])
    {       
	do_help (ch, "for");
	return;
    }
        
    if (!str_prefix("quit", argument))
    {
	send_to_char ("Are you trying to crash the MUD or something?\n\r",ch); 
	return;
    }

    if (!str_cmp (range, "all")) 
    { 
	fMortals = TRUE;
	fGods = TRUE; 
    }
    else if (!str_cmp (range, "gods"))
	fGods = TRUE;
    else if (!str_cmp (range, "mortals"))
	fMortals = TRUE;
    else if (!str_cmp (range, "mobs"))
	fMobs = TRUE;
    else if (!str_cmp (range, "everywhere"))
	fEverywhere = TRUE;
    else if (!str_cmp (range, "guild"))
	fGuild = TRUE;
    else if (!str_cmp (range, "quest"))
	fQuest = TRUE;
    else if (!str_cmp (range, "newbies"))
	fNewbie = TRUE;
    else
	do_help (ch, "for"); /* show syntax */
         
    if (fEverywhere && strchr (argument, '#'))
    {       
	send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
	return;
    }

    if ( strchr(argument, '#') ) /* replace # ? */
    {       
	for (p = char_list; p ; p = p_next)
	{
	    p_next = p->next;
	    found = FALSE;

	    if (!(p->in_room) || room_is_private(p->in_room)|| (p == ch))
		continue;

	    if (IS_NPC(p) && fMobs)
		found = TRUE;
	    else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
		found = TRUE;
	    else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
		found = TRUE;
	    else if (IS_GUILDED(p) && p->guild == ch->guild && fGuild )
		found = TRUE;
	    else if (p->level < 10 && fNewbie )
		found = TRUE;

	    if (found) /* p is 'appropriate' */
	    {
		char *pSource = argument;            
		char *pDest = buf;
		while (*pSource)
		{
		    if (*pSource == '#')
		    {
			const char *namebuf=name_expand(p);
			if (namebuf)
			    while (*namebuf)
				*(pDest++) = *(namebuf++);
			pSource++;
		    }
		    else
			*(pDest++) = *(pSource++);
		} /* while */
                                
		*pDest = '\0'; /* Terminate */
		old_room = ch->in_room;
		char_from_room (ch); 
		char_to_room (ch,p->in_room);
		interpret (ch, buf);
		char_from_room (ch);
		char_to_room (ch,old_room);
	    } /* if found */        
	} /* for every char */
    }                                               
    else /* just for every room with the appropriate people in it */
    {                                                       
	for (i = 0; i < MAX_KEY_HASH; i++)
	    for (room = room_index_hash[i] ; room ; room =room->next)
	    {               
		found = FALSE;  
                                
		if (fEverywhere)      
		    found = TRUE;
		else if (!room->people)
		    continue;

		for (p = room->people; p && !found; p = p->next_in_room)
		{
		    if (p == ch)
			continue;

		    if (IS_NPC(p) && fMobs)
			found = TRUE; 
		    else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
			found = TRUE;
		    else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)       
			found = TRUE;
		    else if (IS_GUILDED(p) && p->guild == ch->guild && fGuild )
			found = TRUE;
		}
		if (found && !room_is_private(room))
		{
		    old_room = ch->in_room;
		    char_from_room (ch);
		    char_to_room (ch, room);
		    interpret (ch, argument);
		    char_from_room (ch);
		    char_to_room (ch, old_room);
		}
	    }
    }
}

void do_newbiehelper( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->act, PLR_NEWBIEHELPER) )
    {
	send_to_char( "Okay, you are no longer in the mood to help newbies.\n\r", ch );
	REMOVE_BIT(ch->act, PLR_NEWBIEHELPER);
	return;
    }

    send_to_char( "Okay, you are now in the mood to help newbies.\n\r", ch );
    SET_BIT(ch->act, PLR_NEWBIEHELPER);
    return;
}

void do_loadplayer( CHAR_DATA *ch, char *argument )
{
    char filename[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Load which character linkless?\r\n", ch );
	return;
    }

    if ( get_char_sedai(ch, arg) )
    {
	send_to_char( "That person is playing right now.\r\n", ch );
	return;
    }

    sprintf( filename, "%s", correct_name(arg) );
    if ( !load_char_obj(&d, arg) )
    {
	send_to_char( "Player not found.\r\n", ch );
	return;
    }

    sprintf( buf, "$N loads %s linkless.", arg );
    wiznet( buf, ch, NULL, WIZ_LOGINS, 0, 0 );

    d.character->desc = NULL;
    char_to_room( d.character, ch->in_room );
    if ( d.character->pet )
	char_to_room( d.character->pet, ch->in_room );

    SET_BIT(d.character->act, PLR_SAFE);
    return;
};

void do_endplayer( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Logoff which linkless character?\r\n", ch );
	return;
    }

    if ( (vch = get_char_room(ch, arg)) == NULL )
    {
	send_to_char( "That person is not in this room.\r\n", ch );
	return;
    }

    sprintf( buf, "$N forces %s (linkless) to logoff.", arg );
    wiznet( buf, ch, NULL, WIZ_LOGINS, 0, 0 );

    REMOVE_BIT(vch->act, PLR_SAFE);
    do_quit( vch, "" );
    return;    
}

void do_command( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Do what with the command?\r\n", ch );
	return;
    }

    if ( !str_prefix(arg, "save") )
    {
	FILE *fp;
	int i;

	fclose( fpReserve );
	if ( (fp = fopen( COMMAND_LIST, "w" )) == NULL )
	{
	    bug( "Do_command: Command file not writable.", 0 );
	    exit(1);
	}
	send_to_char( "Saving commands.\r\n", ch );

	fprintf( fp, "# %-18s Pos Lev Flags\n", "Command" );
	for ( i = 0; cmd_table[i].do_fun != NULL; i++ )
	{
	    int flag = 0;

	    if ( cmd_table[i].log == LOG_ALWAYS )
		flag = flag | G;
	    if ( cmd_table[i].log == LOG_NORMAL )
		flag = flag | H;
	    if ( cmd_table[i].log == LOG_NEVER )
		flag = flag | I;
	    if ( cmd_table[i].show )
		flag = flag | SHOW; 

	    fprintf( fp, "%-20s %3d %3d %s\n",
		cmd_table[i].name,
		cmd_table[i].position,
		cmd_table[i].level,
		print_flags(flag) );
	}

	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
}
