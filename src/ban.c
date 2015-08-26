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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "mem.h"

BAN_DATA *ban_list;

void save_bans(void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose( fpReserve ); 
    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
        perror( BAN_FILE );
    }

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
	if (IS_SET(pban->ban_flags,BAN_PERMANENT))
	{
	    found = TRUE;
	    fprintf(fp,"'%s' '%s' %-2d %s\n",pban->user,
		pban->host, pban->level,
		print_flags(pban->ban_flags));
	}
     }

     fclose(fp);
     fpReserve = fopen( NULL_FILE, "r" );
     if (!found)
	unlink(BAN_FILE);
}

void load_bans(void)
{
    FILE *fp;
    BAN_DATA *ban_last;
 
    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
        return;
 
    ban_last = NULL;
    for ( ; ; )
    {
        BAN_DATA *pban;

        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }
 
        pban = new_ban();

	pban->user = str_dup( fread_word(fp) );
        pban->host = str_dup( fread_word(fp) );
	pban->level = fread_number( fp );
	pban->ban_flags = fread_flag( fp );
	fread_to_eol( fp );

        if (ban_list == NULL)
	    ban_list = pban;
	else
	    ban_last->next = pban;
	ban_last = pban;
    }
}

bool check_ban( DESCRIPTOR_DATA *d, int type )
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];

    strcpy(host,capitalize(d->host));
    host[0] = LOWER(host[0]);

    for ( pban = ban_list; pban != NULL; pban = pban->next ) 
    {
	if( !IS_SET(pban->ban_flags, type) )
	    continue;

	if ( IS_SET(pban->ban_flags, BAN_USER)
	&&   str_cmp(pban->user, d->ident) )
	    continue;

	if (IS_SET(pban->ban_flags,BAN_PREFIX) 
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)  
	&&  strstr(pban->host,host) != NULL)
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->host,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->host,host))
	    return TRUE;
    }

    return FALSE;
}


void ban_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name, *user, *host;
    BAN_DATA *pban, *prev;
    bool prefix = FALSE,suffix = FALSE, fUser = FALSE;
    int type;
    BUFFER *buffer;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
	if (ban_list == NULL)
	{
	    send_to_char("No sites banned at this time.\n\r",ch);
	    return;
  	}
	buffer = new_buf();

        add_buf( buffer,"Banned sites        level  type     status\n\r");
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
	    sprintf(buf2,"%s%s%s%s%s",
		pban->user,
		IS_SET(pban->ban_flags,BAN_USER) ? "@" : "",
		IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "",
		pban->host,
		IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
	    sprintf(buf,"%-18s    %-3d  %-7s  %s\n\r",
		buf2, pban->level,
		IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
		IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
		IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
	    	IS_SET(pban->ban_flags,BAN_PERMANENT) ? "perm" : "temp");
	    add_buf(buffer,buf);
        }

        page_to_char( buf_string(buffer), ch );
	free_buf(buffer);
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	send_to_char("Acceptable ban types are all, newbies, and permit.\n\r",
	    ch); 
	return;
    }

    name = arg1;

    user = &str_empty[0];
    for ( host = name; *name != '\0'; )
    {
	if ( *name == '@' )
	{
	    fUser	= TRUE;
	    user	= host;
	    *name	= '\0';
	    host	= name+1;
	    break;
	}
	name++;
    }

    if (host[0] == '*')
    {
	prefix = TRUE;
	host++;
    }

    if (host[strlen(host) - 1] == '*')
    {
	suffix = TRUE;
	host[strlen(host) - 1] = '\0';
    }

    if (strlen(host) == 0)
    {
	send_to_char("You have to ban SOMETHING.\n\r",ch);
	return;
    }

    prev = NULL;
    for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next )
    {
	if ( IS_SET(pban->ban_flags, BAN_USER)
	&&   str_cmp(pban->user, user) )
	    continue;

        if (!str_cmp(host,pban->host))
        {
	    if (pban->level > get_trust(ch))
	    {
            	send_to_char( "That ban was set by a higher power.\n\r", ch );
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    ban_list = pban->next;
		else
		    prev->next = pban->next;
		free_ban(pban);
	    }
        }
    }

    pban = new_ban();
    pban->user = str_dup(user);
    pban->host = str_dup(host);
    pban->level = get_trust(ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);
    if ( fUser )
	SET_BIT(pban->ban_flags,BAN_USER);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    sprintf(buf,"%s%s%s has been banned.\n\r",
	pban->user,
	IS_SET(pban->ban_flags, BAN_USER) ? "@" : "",
	pban->host );
    send_to_char( buf, ch );
    return;
}

void do_ban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow( CHAR_DATA *ch, char *argument )                        
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;
    char *name, *user, *host;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    name = arg;

    user = &str_empty[0];
    for ( host = name; *name != '\0'; )
    {
	if ( *name == '@' )
	{
	    user	= host;
	    *name	= '\0';
	    host	= name+1;
	    break;
	}
	name++;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( IS_SET(curr->ban_flags, BAN_USER)
	&&   str_cmp(curr->user, user) )
	    continue;

        if ( !str_cmp( host, curr->host ) )
        {
	    if (curr->level > get_trust(ch))
	    {
		send_to_char(
		   "You are not powerful enough to lift that ban.\n\r",ch);
		return;
	    }
            if ( prev == NULL )
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

	    if ( IS_SET(curr->ban_flags, BAN_USER) )
		sprintf( buf, "Ban on %s@%s lifted.\r\n",
		    curr->user, curr->host );
	    else
		sprintf( buf, "Ban on %s lifted.\r\n",
		    curr->host );
            free_ban(curr);
            send_to_char( buf, ch );
	    save_bans();
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}


