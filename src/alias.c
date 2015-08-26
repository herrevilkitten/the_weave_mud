/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
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
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],
         name[MAX_INPUT_LENGTH];

    int alias;
    int i;
    char *point;

    ch = d->character;     
/*
    if (ch->function)
    {
       interpret(ch, argument);
       return;
    }
*/

    if ( !ch )
	return;

    if (!str_prefix("alias",argument) || !str_prefix("unal",argument))
    {
/*
       if ( !run_olc_editor( d, argument ) )
*/
  	  interpret(d->character,argument);
       return;
    }

    strcpy(buf,argument);

    if (!(IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||  !str_prefix("alias",argument) || !str_prefix("una",argument)))
    {
       for (alias = 0; alias < MAX_ALIAS; alias++) /* go through the aliases */
       {
  	   if (ch->pcdata->alias[alias] == NULL)
	      break;

	   if (!str_prefix(ch->pcdata->alias[alias],argument))
	   {
	       point = one_argument(argument,name);
	       if (!strcmp(ch->pcdata->alias[alias],name))
	       {
                  char variable[10][MAX_INPUT_LENGTH];
                  int  var_used;
                  char *text[11];
                  char sub_alias[MAX_STRING_LENGTH];
                  char *p;
                  char *pp;
                  bool str_in;

                  for (i=0;i<10;i++)
                     *variable[i] = 0;

                  var_used = 0;

	   	  *buf = 0;
		  strcpy(sub_alias,ch->pcdata->alias_sub[alias]);
                  p = sub_alias;
                  pp = buf;
                  text[0] = point;
                  str_in = FALSE;

                  for (i=0;i<10 && *point != '\0';i++)
                  {
                     point = first_arg(point,variable[i], FALSE);
                     text[i + 1] = point;
                  }

                  for(p = sub_alias; *p != '\0';p++)
                  {
                     if (*p != '$')
                     {
                        *pp++ = *p;  
                        continue;
                     }

                     p++;
 
                     if (*p >= '0' && *p <= '9')
                     {
                        i = *p - '0';
                        if (i < 0 || i > 9)
                        {
                           send_to_char("ERROR in alias, report to a coder.\n\r",ch);
                           return;
                        }

                        if ((i + 1) > var_used) 
                           var_used = i + 1;

                        strcpy(pp, variable[i]);
                        while (*pp != '\0') pp++;
                        continue;
                     }
                     else if (*p == '*')
                     {
                        *pp++ = '$';
                        *pp++ = '*';
                        str_in = TRUE;
                        continue;
                     }
                     *pp++ = *p;
                  }
                  *pp = '\0';

                  if (str_in)
                  {
                     strcpy(sub_alias,buf);
                     *buf = 0;
                     pp = buf;
                     for(p = sub_alias; *p != '\0';p++)
                     {
                        if (*p != '$')
                        {
                           *pp++ = *p;
                           continue;
                        }
                        p++;
                        if (*p != '*')
                        {
                           *pp++ = *p;
                           continue;
                        }
                        strcpy(pp,text[var_used]);
                        while (*pp != '\0') pp++;
                     } 
                     *pp = '\0';
                  }   
		  break;
	       }
	   }
       }

       if (strlen(buf) > MAX_INPUT_LENGTH)
       {
          send_to_char("Alias substitution too long. Truncated.\r\n",ch);
          buf[MAX_INPUT_LENGTH -1] = '\0';
       }

    }

    point = buf;

    *name = 0;

    point = one_command(point,name);
/*
    if ( !run_olc_editor( d, name ) )
*/
          interpret(d->character,name);

    if (*(d->delayed) != '\0') 
       sprintf(d->delayed + strlen(d->delayed),";;%s",point);
    else 
       strcpy(d->delayed,point);

    return;
}

void do_alia(CHAR_DATA *ch, char *argument)
{
    send_to_char("I'm sorry, alias must be entered in full.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;
    char *p;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    smash_tilde(argument);
    argument = one_argument(argument,arg);    

    if (arg[0] == '\0')
    {

	if (rch->pcdata->alias[0] == NULL)
	{
	    send_to_char("You have no aliases defined.\n\r",ch);
	    return;
	}
	send_to_char("Your current aliases are:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    sprintf(buf,"%15s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_prefix("unalias",arg) || !str_cmp("alias",arg))
    {
	send_to_char("Sorry, that word is reserved.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
		sprintf(buf,"%s aliases to '%s'.\n\r",rch->pcdata->alias[pos],
			rch->pcdata->alias_sub[pos]);
		send_to_char(buf,ch);
		return;
	    }
	}

	send_to_char("That alias is not defined.\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	send_to_char("That shall not be done!\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    sprintf(buf,"%s is now realiased to '%s'.\n\r",arg,argument);
	    send_to_char(buf,ch);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	send_to_char("Sorry, you have reached the alias limit.\n\r",ch);
	return;
     }
  
     /* make a new alias */
     free_string( rch->pcdata->alias[pos] );
     free_string( rch->pcdata->alias_sub[pos] );
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     sprintf(buf,"%s is now aliased to '%s'.\n\r",arg,argument);
     send_to_char(buf,ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument,arg);

    if (arg == '\0')
    {
	send_to_char("Unalias what?\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    send_to_char("Alias removed.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No alias of that name to remove.\n\r",ch);
}
     
char *one_command( char *argument, char *arg_first )
{
    while ( isspace(*argument) )
        argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == ';' && *(argument + 1) == ';' )
        {
            argument += 2;
            break;
        }
   /*   *arg_first = LOWER(*argument);   */
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
        argument++;

    return argument;
}













