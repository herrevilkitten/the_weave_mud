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
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "mem.h"

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;
NOTE_DATA *apply_list;

int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	if (!hide_note(ch,pnote))
	    count++;

    return count;
}

void do_unread(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    int count;
    bool found = FALSE;

    if (IS_NPC(ch))
	return; 

    if ((count = count_spool(ch,news_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"There %s %d new news article%s waiting.\n\r",
	    count > 1 ? "are" : "is",count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,changes_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"There %s %d change%s waiting to be read.\n\r",
	    count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
        send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,note_list)) > 0)
    {
	found = TRUE;
	sprintf(buf,"You have %d new note%s waiting.\n\r",
	    count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if ((count = count_spool(ch,idea_list)) > 0 && IS_IMMORTAL(ch))
    {
	found = TRUE;
	sprintf(buf,"You have %d unread idea%s to peruse.\n\r",
	    count, count > 1 ? "s" : "");
	send_to_char(buf,ch);
    }
    if (IS_IMMORTAL(ch) && (count = count_spool(ch,penalty_list)) >0)
    {
	found = TRUE;
	sprintf(buf,"%d %s been added.\n\r",
	    count, count > 1 ? "penalties have" : "penalty has");
	send_to_char(buf,ch);
    }
    if (IS_IMMORTAL(ch) && (count = count_spool(ch,apply_list)) >0)
    {
	found = TRUE;
	send_to_char_new( ch, "%d %s been sent in.\r\n",
	    count, count > 1 ? "applications have" : "application has" );
    }
    if (!found)
	send_to_char("You have no unread notes.\n\r",ch);
}

void do_note(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_IDEA);
}

void do_penalty(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_PENALTY);
}

void do_news(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_CHANGES);
}

void do_application(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_APPLY);
}

void save_notes(int type)
{
    FILE *fp;
    char *name;
    NOTE_DATA *pnote;

    switch (type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    pnote = note_list;
	    break;
	case NOTE_IDEA:
	    name = NOTE_IDEA_FILE;
	    pnote = idea_list;
	    break;
	case NOTE_PENALTY:
	    name = NOTE_PENALTY_FILE;
	    pnote = penalty_list;
	    break;
	case NOTE_NEWS:
	    name = NOTE_NEWS_FILE;
	    pnote = news_list;
	    break;
	case NOTE_CHANGES:
	    name = NOTE_CHANGES_FILE;
	    pnote = changes_list;
	    break;
	case NOTE_APPLY:
	    name = APPLY_FILE;
	    pnote = apply_list;
	    break;
    }

    fclose( fpReserve );
    if ( ( fp = fopen( name, "w" ) ) == NULL )
    {
	perror( name );
    }
    else
    {
	for ( ; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Real	  %s~\n", pnote->real_name);
	    fprintf( fp, "Guild   '%s'\n", guild_name(pnote->guild) );
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
   	return;
    }
}
void load_notes(void)
{
    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60);
    load_thread(NOTE_IDEA_FILE,&idea_list, NOTE_IDEA, 14*24*60*60);
    load_thread(NOTE_PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0);
    load_thread(NOTE_NEWS_FILE,&news_list, NOTE_NEWS, 0);
    load_thread(NOTE_CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
    load_thread(APPLY_FILE,&apply_list,NOTE_APPLY, 0);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
 
    if ( ( fp = fopen( name, "r" ) ) == NULL )
	return;
	 
    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char letter;
	 
	do
	{
	    letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );
 
        pnote		= new_note( );
 
        if ( str_cmp( fread_word( fp ), "sender" ) )
	    break;
	pnote->sender	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "real" ) )
            break;
        pnote->real_name= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "guild" ) )
	    break;
	pnote->guild    = guild_lookup(fread_word( fp ));
 
        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date     = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);
 
        if ( str_cmp( fread_word( fp ), "to" ) )
            break;
        pnote->to_list  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text     = fread_string( fp );
 
        if (free_time && pnote->date_stamp < current_time - free_time)
        {
	    free_note(pnote);
            continue;
        }

	pnote->type = type;
 
        if (*list == NULL)
            *list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit( 1 );
    return;
}

void append_note(NOTE_DATA *pnote)
{
    FILE *fp;
    char *name;
    char *type;
    NOTE_DATA **list;
    NOTE_DATA *last;
    DESCRIPTOR_DATA *d;

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    list = &note_list;
	    type = "note";
	    break;
	case NOTE_IDEA:
	    name = NOTE_IDEA_FILE;
	    list = &idea_list;
	    type = "idea";
	    break;
	case NOTE_PENALTY:
	    name = NOTE_PENALTY_FILE;
	    list = &penalty_list;
	    type = "penalty";
	    break;
	case NOTE_NEWS:
	    name = NOTE_NEWS_FILE;
	    list = &news_list;
	    type = "news item";
	    break;
	case NOTE_CHANGES:
	    name = NOTE_CHANGES_FILE;
	    list = &changes_list;
	    type = "change";
	    break;
	case NOTE_APPLY:
	    name = APPLY_FILE;
	    list = &apply_list;
	    type = "application";
	    break;
    }

    if (*list == NULL)
	*list = pnote;
    else
    {
	for ( last = *list; last->next != NULL; last = last->next);
	last->next = pnote;
    }

    fclose(fpReserve);
    if ( ( fp = fopen(name, "a" ) ) == NULL )
    {
        perror(name);
    }
    else
    {
        fprintf( fp, "Sender  %s~\n", pnote->sender);
        fprintf( fp, "Real    %s~\n", pnote->real_name);
        fprintf( fp, "Guild   '%s'\n", guild_name(pnote->guild) );
	fprintf( fp, "Date    %s~\n", pnote->date);
        fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
        fprintf( fp, "To      %s~\n", pnote->to_list);
        fprintf( fp, "Subject %s~\n", pnote->subject);
        fprintf( fp, "Text\n%s~\n", pnote->text);
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL
	&&   IS_SET(d->character->comm, COMM_NOTES)   
	&&   is_note_to(d->character, pnote)
	&&   str_cmp(d->character->name, pnote->real_name)
	&&   !IS_WRITING(d->character) )
	{
	    send_to_char_new( d->character, 
		"A new %s has been posted.\n\r", type );
        }
    }
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp(ch->name, pnote->real_name) )
	return TRUE;

    if ( pnote->type == NOTE_IDEA
    &&   !IS_IMMORTAL(ch) )
	return FALSE;

    if ( is_full_name( "all", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
	return TRUE;

    if ( is_full_name(ch->name, pnote->to_list) )
	return TRUE;

    if ( pnote->guild == ch->guild
    &&   is_full_name("guild", pnote->to_list) )
	return TRUE;

    if ( is_full_name("warder", pnote->to_list)
    &&   (ch->guild == guild_lookup( "aes-sedai" )
    ||    ch->guild == guild_lookup( "warder" )) )
        return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->real_name	= str_dup( FIRSTNAME(ch) );
    pnote->guild	= 0;
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE_DATA *prev;
    NOTE_DATA **list;
    char *to_list;

    if (!delete)
    {
	/* make a new list */
        to_new[0]	= '\0';
        to_list	= pnote->to_list;
        while ( *to_list != '\0' )
        {
    	    to_list	= one_argument( to_list, to_one );
    	    if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	    {
	        strcat( to_new, " " );
	        strcat( to_new, to_one );
	    }
        }
        /* Just a simple recipient removal? */
       if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
       {
	   free_string( pnote->to_list );
	   pnote->to_list = str_dup( to_new + 1 );
	   return;
       }
    }
    /* nuke the whole note */

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	    list = &news_list;
	    break;
	case NOTE_CHANGES:
	    list = &changes_list;
	    break;
	case NOTE_APPLY:
	    list = &changes_list;
	    break;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == *list )
    {
	*list = pnote->next;
    }
    else
    {
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t last_read;

    if (IS_NPC(ch))
	return TRUE;

    switch (pnote->type)
    {
	default:
	    return TRUE;
	case NOTE_NOTE:
	    last_read = ch->pcdata->last_note;
	    break;
	case NOTE_IDEA:
	    last_read = ch->pcdata->last_idea;
	    break;
	case NOTE_PENALTY:
	    last_read = ch->pcdata->last_penalty;
	    break;
	case NOTE_NEWS:
	    last_read = ch->pcdata->last_news;
	    break;
	case NOTE_CHANGES:
	    last_read = ch->pcdata->last_changes;
	    break;
	case NOTE_APPLY:
	    last_read = ch->pcdata->last_apply;
	    break;
    }
    
    if (pnote->date_stamp <= last_read)
	return TRUE;

    if (!str_cmp(ch->name,pnote->real_name))
	return TRUE;

    if (!is_note_to(ch,pnote))
	return TRUE;

    return FALSE;
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t stamp;

    if (IS_NPC(ch))
	return;

    stamp = pnote->date_stamp;

    switch (pnote->type)
    {
        default:
            return;
        case NOTE_NOTE:
	    ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
            break;
        case NOTE_IDEA:
	    ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
            break;
        case NOTE_PENALTY:
	    ch->pcdata->last_penalty = UMAX(ch->pcdata->last_penalty,stamp);
            break;
        case NOTE_NEWS:
	    ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
            break;
        case NOTE_CHANGES:
	    ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
            break;
    }
}


void parse_note( CHAR_DATA *ch, char *argument, int type )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA *pnote;
    NOTE_DATA **list;
    char *list_name;
    int vnum;
    int anum;
    time_t note_time;

    if ( IS_NPC(ch) )
	return;

    switch(type)
    {
	default:
	    return;
        case NOTE_NOTE:
            list = &note_list;
	    list_name = "notes";
	    note_time = ch->pcdata->last_note;
            break;
        case NOTE_IDEA:
            list = &idea_list;
	    list_name = "ideas";
	    note_time = ch->pcdata->last_idea;
            break;
        case NOTE_PENALTY:
            list = &penalty_list;
	    list_name = "penalties";
	    note_time = ch->pcdata->last_penalty;
            break;
        case NOTE_NEWS:
            list = &news_list;
	    list_name = "news";
	    note_time = ch->pcdata->last_news;
            break;
        case NOTE_CHANGES:
            list = &changes_list;
	    list_name = "changes";
	    note_time = ch->pcdata->last_changes;
            break;
        case NOTE_APPLY:
            list = &apply_list;
	    list_name = "applications";
	    note_time = ch->pcdata->last_apply;
            break;
    }

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( (( type != NOTE_IDEA && type != NOTE_APPLY ) || IS_IMMORTAL( ch ))
    &&   (arg[0] == '\0' || !str_prefix( arg, "read") || is_number( arg )) )
    {
        bool fAll;
	BUFFER *buffer;

        if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
 
        else if ( (argument[0] == '\0' || !str_prefix(argument, "next"))
	&&        !is_number(arg) )
        /* read next unread note */
        {
            vnum = 0;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
		    buffer = new_buf();
                    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                        vnum,
                        IS_IMMORTAL(ch) ? pnote->real_name : pnote->sender,
                        pnote->subject,
                        pnote->date,
                        pnote->to_list);
                    add_buf( buffer, buf );
                    add_buf( buffer, pnote->text );
		    page_to_char( buf_string(buffer), ch );
		    free_buf( buffer );
                    update_read(ch,pnote);
                    return;
                }
                else if (is_note_to(ch,pnote))
                    vnum++;
            }
	    sprintf(buf,"You have no unread %s.\n\r",list_name);
	    send_to_char(buf,ch);
            return;
        }
 
        else if ( is_number(argument) || is_number(arg) )
        {
            fAll = FALSE;
	    if ( is_number(arg) )
		anum = atoi( arg );
	    else
		anum = atoi( argument );
        }
        else
        {
            send_to_char( "Read which number?\n\r", ch );
            return;
        }
 
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
		buffer = new_buf();
                sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                    vnum - 1,
                    IS_IMMORTAL(ch) ? pnote->real_name : pnote->sender,
                    pnote->subject,
                    pnote->date,
                    pnote->to_list
                    );
		add_buf( buffer, buf );
		add_buf( buffer, pnote->text );
		page_to_char( buf_string(buffer), ch );
		free_buf( buffer );
		update_read(ch,pnote);
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix(arg, "list")
    &&   (( type != NOTE_IDEA && type != NOTE_APPLY )
    ||    IS_IMMORTAL( ch )) )
    {
	char arg2[MAX_INPUT_LENGTH];
	char buf2[MAX_STRING_LENGTH * 2];
	int restricted;
	BUFFER *buffer;

	restricted = 0;
	vnum = 0;
	buf2[0] = '\0';
	one_argument( argument, arg2 );

        if ( !str_cmp( arg2, "guild" ) )
            restricted = 1;
        if ( !str_cmp( arg2, "immortal" ) )
            restricted = 2;
        if ( !str_cmp( arg2, "public" ) )
            restricted = 3;
        if ( !str_cmp( arg2, "private" ) )
            restricted = 4;
        if ( !str_cmp( arg2, "new" ) )
            restricted = 5;
        if ( !str_cmp( arg2, "warder" ) )
            restricted = 6;

	if ( type != NOTE_NOTE )
	    restricted = 0;

	buffer = new_buf();
	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to(ch, pnote) )
	    {
		sprintf( buf, "[%3d%s] %s: %s\n\r",
		    vnum, hide_note(ch,pnote) ? " " : "N", 
		    IS_IMMORTAL(ch) ? pnote->real_name : pnote->sender,
		    pnote->subject );
		vnum++;
                if ( restricted == 1
		&&   !is_full_name( "guild", pnote->to_list) )
                    continue;
                if ( restricted == 2
		&&   !is_full_name( "immortal", pnote->to_list) )
                    continue;
                if ( restricted == 3
		&&   !is_full_name( "all", pnote->to_list) )
                    continue;
                if ( restricted == 4
		&&   (is_full_name( "all", pnote->to_list ) 
		||    is_full_name( "guild", pnote->to_list )
		||    is_full_name( "immortal", pnote->to_list )) )
                    continue;
                if ( restricted == 5
		&&   (pnote->date_stamp <= note_time
		||    !str_cmp( pnote->sender, ch->name )) )
                    continue;
                if ( restricted == 6
		&&   !is_full_name("warder", pnote->to_list) )
                    continue;
		add_buf( buffer, buf );
	    }
	}
	if (!vnum)
	{
	    switch(type)
	    {
		case NOTE_NOTE:	
		    send_to_char("There are no notes for you.\n\r",ch);
		    break;
		case NOTE_IDEA:
		    send_to_char("There are no ideas for you.\n\r",ch);
		    break;
		case NOTE_PENALTY:
		    send_to_char("There are no penalties for you.\n\r",ch);
		    break;
		case NOTE_NEWS:
		    send_to_char("There is no news for you.\n\r",ch);
		    break;
		case NOTE_CHANGES:
		    send_to_char("There are no changes for you.\n\r",ch);
		    break;
	    }
	}
	else
	    page_to_char( buf_string(buffer), ch );
	free_buf( buffer );
	return;
    }

    if ( !str_prefix(arg, "remove") && type != NOTE_IDEA )
    {
        if ( !is_number(argument) )
        {
            send_to_char( "Note remove which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to(ch, pnote) && vnum++ == anum )
            {
                note_remove( ch, pnote, FALSE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.",list_name);
	send_to_char(buf,ch);
        return;
    }
 
    if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_remove( ch, pnote,TRUE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }

 	sprintf(buf,"There aren't that many %s.",list_name);
	send_to_char(buf,ch);
        return;
    }

    if (!str_prefix(arg,"catchup"))
    {
	switch(type)
	{
	    case NOTE_NOTE:	
		ch->pcdata->last_note = current_time;
		break;
	    case NOTE_IDEA:
		ch->pcdata->last_idea = current_time;
		break;
	    case NOTE_PENALTY:
		ch->pcdata->last_penalty = current_time;
		break;
	    case NOTE_NEWS:
		ch->pcdata->last_news = current_time;
		break;
	    case NOTE_CHANGES:
		ch->pcdata->last_changes = current_time;
		break;
	}
	return;
    }

    /* below this point only certain people can edit notes */
    if ( (type == NOTE_NEWS && !IS_IMMORTAL( ch ))
    ||   (type == NOTE_PENALTY && !IS_IMMORTAL( ch ))
    ||   (type == NOTE_CHANGES && !IS_IMMORTAL( ch )) )
    {
	sprintf(buf,"You aren't high enough level to write %s.",list_name);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg, "edit" ) )
    {
	if ( ch->fighting != NULL )
	{
	    send_to_char( "You cannot edit notes while fighting.\n\r", ch );
	    return;
	}

	note_attach( ch,type );
	if (ch->pnote->type != type)
	{
	    send_to_char(
		"You already have a different note in progress.\n\r",ch);
	    return;
	}

        act( "$n begins to write something.", ch, NULL, NULL, TO_ROOM );
        string_append( ch, &ch->pnote->text );

	return;
    }

    if ( !str_prefix( arg, "subject" ) )
    {
	note_attach( ch,type );
        if (ch->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	free_string( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
	note_attach( ch,type );
        if (ch->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }
	if ( is_full_name("guild", argument) )
	{
	    if ( ch->guild == 0 )
	    {
		send_to_char( "You must belong to a guild in order to post to a guild.\n\r", ch );
		return;
	    }
	    else
		ch->pnote->guild = ch->guild;
	}

	if ( is_full_name("warder", argument)
	&&   ch->guild != guild_lookup("aes sedai")
	&&   ch->guild != guild_lookup("warder") )
	{
	    send_to_char( "You must be a Warder or Aes Sedai to post here.\n\r", ch );
	    return;
	}
	free_string( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char( "Remember that you must use the FULL name of the person you are sending notes to.\r\n", ch );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if ( ch->pnote != NULL )
	{
	    free_note(ch->pnote);
	    ch->pnote = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix(arg, "forward") )
    {
	char arg2[MAX_STRING_LENGTH];


	if ( ch->pnote )
	{
	    send_to_char( "Due to coding restrictions, you cannot forward a note while editing one.\r\n", ch );
	    return;
	}
	argument = one_argument( argument, arg2 );

        if ( !is_number( arg2 ) )
        {
            send_to_char( "Forward which number?\n\r", ch );
            return;
        }
 
        anum = atoi( arg2 );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
		note_attach( ch, pnote->type );

		free_string( ch->pnote->sender );
		free_string( ch->pnote->real_name );
		free_string( ch->pnote->date );
		free_string( ch->pnote->subject );
		free_string( ch->pnote->to_list );
		free_string( ch->pnote->text );

		ch->pnote->sender = str_dup( pnote->sender );
		ch->pnote->real_name = str_dup( pnote->sender );
		ch->pnote->date = str_dup( pnote->date );
		sprintf( buf, "(Forwarded by: %s) %s", ch->name,
		    pnote->subject );
		ch->pnote->subject = str_dup( buf );
		ch->pnote->to_list = str_dup( argument );
		ch->pnote->text = str_dup( pnote->text );
		ch->pnote->guild = 0;
		ch->pnote->date_stamp = current_time;
		ch->pnote->type = pnote->type;

		append_note(ch->pnote);
		ch->pnote = NULL;
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }

 	sprintf(buf,"There aren't that many %s.",list_name);
	send_to_char(buf,ch);
        return;	
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (ch->pnote->type != type)
	{
	    send_to_char("You aren't working on that kind of note.\n\r",ch);
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    IS_IMMORTAL(ch) ? ch->pnote->real_name : ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	    );
	send_to_char( buf, ch );
	page_to_char( ch->pnote->text, ch );
	return;
    }

    if ( !str_prefix(arg, "post") || !str_prefix(arg, "send"))
    {
	char *strtime;

	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

        if (ch->pnote->type != type)
        {
            send_to_char("You aren't working on that kind of note.\n\r",ch);
            return;
        }

	if (!str_cmp(ch->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, guild, warder, immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	free_string( ch->pnote->date );
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

        if ( ch->pnote->guild )
        {
            char buf2[MAX_INPUT_LENGTH];
            sprintf( buf2, "[GUILD] %s", ch->pnote->subject );
            free_string( ch->pnote->subject ); 
            ch->pnote->subject = str_dup( buf2 );
        }
        
        if ( is_full_name("warder", ch->pnote->to_list) )
        {
            char buf2[MAX_INPUT_LENGTH];
            sprintf( buf2, "[WARDER] %s", ch->pnote->subject );
            free_string( ch->pnote->subject );
            ch->pnote->subject = str_dup( buf2 );
        }

	append_note(ch->pnote);
	ch->pnote = NULL;
	return;
    }

    send_to_char( "You can't do that.\n\r", ch );
    return;
}

