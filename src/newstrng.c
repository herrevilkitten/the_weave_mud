/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "mem.h"


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( "  Terminate with a @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString )
{
    if ( ch->desc->connected == CON_PLAYING )
    {
	send_to_char( "-=======- Entering APPEND Mode -========-\n\r", ch );
	send_to_char( "    Type .h on a new line for help\n\r", ch );
	send_to_char( "  Terminate with a @ on a blank line.\n\r", ch );
	send_to_char( "-=======================================-\n\r", ch );
    }

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    if ( ch->desc->connected == CON_PLAYING )
	send_to_char( *pString, ch );

    if ( *(*pString + strlen( *pString ) - 1) != '\r' )
	send_to_char( "\n\r", ch );

    ch->desc->pString = pString;
    return;
}




/*****************************************************************************
 Name:		count_lines
 Purpose:	Counts the number of lines.
 Called by:	
 ****************************************************************************/
int count_lines( char *text )
{
    int i, lines = 0;

    if ( text == NULL || *text == '\0' )
	return 0;

    for ( i = 0;i < strlen(text) ; i++ )
	if ( text[i] == '\r' )
	    lines++;

    return UMAX(1, lines);
}


/*****************************************************************************
 Name:		delete_last
 Purpose:	Deletes the last line.
 Called by:	string_add(string.c)
 ****************************************************************************/
char * delete_line( char * orig )
{
    char buf[MAX_STRING_LENGTH];
    int len;
    bool found = FALSE;

    strcpy( buf, orig );
    for ( len = strlen(buf); len > 0; len-- )
    {
	if ( buf[len] == '\r' )
	{
	    if ( !found )
	    {
		if ( len > 0 )
		    len--;
		found = TRUE;
	    }
	    else
	    {
		buf[len+1] = '\0';
		free_string( orig );
		return str_dup( buf );
	    }
	}
    }
    buf[0] = '\0';
    free_string( orig );
    return str_dup( buf );
}

char * delete_line_number( char * orig, int line1, int line2 )
{
    char buf[MAX_STRING_LENGTH];
    int len, lines = 0;
    int j = 0;
    bool pWrite = TRUE;

    buf[0] = '\0';
    if ( line1 == 0 )
	pWrite = FALSE;
    for ( len = 0; len < strlen(orig); len++ )
    {
	if ( pWrite )
	{
	    buf[j] = orig[len];
	    j++;
	}

	if ( orig[len] == '\r' )
	    lines++;

	if ( lines == line1 )
	    pWrite = FALSE;
	if ( lines == line2 )
	    pWrite = TRUE;
    }
    buf[j+1] = '\0';
    free_string( orig );
    return str_dup( buf );
}


void show_text_with_numbers( CHAR_DATA *ch, char * text )
{
    char buf[MAX_STRING_LENGTH];
    int lines = 1;
    int i, j;

    if ( text == NULL || *text == '\0' )
	return;

    buf[0] = '\0';
    j = 0;
    for ( i = 0; i < strlen(text); i++ )
    {
	buf[j] = text[i];
	if ( text[i] == '\r' )
	{
	    buf[j+1] = '\0';
	    send_to_char_new( ch, "%2d> %s", lines, buf );
	    lines++;
	    j = 0;
	    continue;
	}
	j++;
    }
    if ( lines == 1 )
	send_to_char( "\n\r", ch );
	
    send_to_char( "\n\r", ch );
    return;
}


/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace( char * orig, char * old, char * new )
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, old ) != NULL )
    {
        i = strlen( orig ) - strlen( strstr( orig, old ) );
        xbuf[i] = '\0';
        strcat( xbuf, new );
        strcat( xbuf, &orig[i+strlen( old )] );
    }

    free_string( orig );
    return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Thanks to James Seng
     */
    smash_tilde( argument );

    if ( *argument == '.' || *argument == '/' )
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg1 );

	if ( !str_prefix(arg1, ". ")
	||   !str_prefix(arg1, ".") )
	{
	    send_to_char( "Invalid dot command\n\r", ch );
	    return;
	}

        if ( !str_prefix(arg1, ".clear")
	||   !str_prefix(arg1, "/clear") )
        {
            send_to_char( "String cleared.\n\r", ch );
	    free_string (*ch->desc->pString);
	    *ch->desc->pString = str_dup("");
            return;
        }

        if ( !str_prefix(arg1, ".show")
	||   !str_prefix(arg1, "/show") )
        {
	    one_argument( argument, arg2 );
            send_to_char( "String so far:\n\r", ch );
	    if ( arg2[0] == '\0' )
		send_to_char( *ch->desc->pString, ch );
	    else if ( !str_prefix(arg2, "numbers") )
		show_text_with_numbers( ch, *ch->desc->pString );
	    else
		send_to_char( *ch->desc->pString, ch );
            return;
        }

        if ( !str_prefix(arg1, ".replace")
	||   !str_prefix(arg1, "/replace") )
        {
            argument = first_arg( argument, arg2, FALSE );
            argument = first_arg( argument, arg3, FALSE );
            if ( arg2[0] == '\0' )
            {
                send_to_char(
                    "usage:  .replace \"old string\" \"new string\"\n\r", ch );
                return;
            }

	    smash_tilde( arg3 );   /* Just to be sure -- Hugin */
            *ch->desc->pString =
                string_replace( *ch->desc->pString, arg2, arg3 );
            sprintf( buf, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
            send_to_char( buf, ch );
            return;
        }

	if ( !str_prefix(arg1, ".delete")
	||   !str_prefix(arg1, "/delete") )
	{
	    int line1, line2;
	    argument = one_argument( argument, arg2 );
	    one_argument( argument, arg3 );

	    line1 = atoi( arg2 );
	    line2 = atoi( arg3 );

	    if ( line1 < 0 || line2 < 0 )
	    {
		send_to_char( "Negative line numbers are not allowed.\n\r", ch );
		return;
	    }

	    if ( arg2[0] == '\0' )
	    {
		*ch->desc->pString = delete_line( *ch->desc->pString );
		send_to_char( "Last line deleted.\n\r", ch );
	    }
	    else
	    {
		if ( arg3[0] == '\0' || line2 <= line1 )
		{
		    *ch->desc->pString = delete_line_number( *ch->desc->pString, line1 - 1, line1 );
		    send_to_char_new( ch, "Line %d deleted.\n\r", line1 );
		}
		else
		{
		    *ch->desc->pString = delete_line_number(*ch->desc->pString, line1 - 1, line2 - 1 );
		    send_to_char_new( ch, "Lines %d to %d deleted.\n\r", line1, line2 );
		}
	    }
	    return;
	}

        if ( !str_prefix(arg1, ".format")
	||   !str_prefix(arg1, "/format") )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            send_to_char( "String formatted.\n\r", ch );
            return;
        }
        
        if ( !str_prefix(arg1, ".help")
	||   !str_prefix(arg1, "/help") )
        {
            send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
            send_to_char( ".replace 'old' 'new'   - replace a substring\n\r", ch );
            send_to_char( "/replace 'old' 'new'     (requires '', \"\")\n\r", ch );
            send_to_char( ".help                  - get help (this info)\n\r", ch );
            send_to_char( "/help\n\r", ch );
            send_to_char( ".show [\"numbers\"]      - show string so far\n\r", ch );
            send_to_char( "/show [\"numbers\"]       \"numbers\" show line numbers\n\r", ch );
            send_to_char( ".format                - (word wrap) string\n\r", ch );
            send_to_char( "/format\n\r", ch );
            send_to_char( ".clear                 - clear string so far\n\r", ch );
            send_to_char( "/clear\n\r", ch );
	    send_to_char( ".delete		  - delete last line\n\r", ch );
	    send_to_char( "/delete\n\r", ch );
            send_to_char( ".quit                  - end string and save\n\r", ch );
	    send_to_char( "/quit\n\r", ch );
            return;
        }

	if ( !str_prefix(arg1, ".quit")
	||   !str_prefix(arg1, "/quit") )
	{
        ch->desc->pString = NULL;
	if ( ch->desc->connected == CON_MAKE_DESCRIPTION )
	{
	    send_to_char( "\n\rThe following question will help determine what skills and stats\n\r", ch );
	    send_to_char( "your character initially knows and can learn.  The cost of skill\n\r", ch );
	    send_to_char( "is also affected by this choice.\n\r", ch );
	    send_to_char( "When growing up, did you:\n\r", ch );
	    send_to_char( "A)  Play with many weapons, pretending you were a soldier.\n\r", ch );
	    send_to_char( "B)  Play hide and seek a lot, hiding and sneaking after people.\n\r", ch );
	    send_to_char( "C)  Do much reading, gaining insightful knowledge about the world.\n\r", ch );
	    send_to_char( "D)  Learn many of the secrets of the herbs and healing arts.\n\r", ch );
	    send_to_char( "E)  Hunt a lot, learning the ways of the forestmaster.\n\r", ch );
	    send_to_char( "F)  Work hard in a trade, earning both money and skills.\n\r\n\r", ch );
	    send_to_char( "How did you spend your early years? ", ch );
	    ch->desc->connected = CON_GET_QUESTION;
	}
	if ( !IS_NPC(ch) && buf_string(ch->pcdata->buffer)[0] != '\0' )
	    send_to_char( "You have messages in your playback.  Type REPLAY to read them.\n\r", ch );
        return;
	}
            

        send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
        return;
    }

    if ( *argument == '~'
    ||   *argument == '@' )
    {
        ch->desc->pString = NULL;
	if ( ch->desc->connected == CON_MAKE_DESCRIPTION )
	{
	    send_to_char( "\n\rThe following question will help determine what skills and stats\n\r", ch );
	    send_to_char( "your character initially knows and can learn.  The cost of skill\n\r", ch );
	    send_to_char( "is also affected by this choice.\n\r", ch );
	    send_to_char( "When growing up, did you:\n\r", ch );
	    send_to_char( "A)  Play with many weapons, pretending you were a soldier.\n\r", ch );
	    send_to_char( "B)  Play hide and seek a lot, hiding and sneaking after people.\n\r", ch );
	    send_to_char( "C)  Do much reading, gaining insightful knowledge about the world.\n\r", ch );
	    send_to_char( "D)  Learn many of the secrets of the herbs and healing arts.\n\r", ch );
	    send_to_char( "E)  Hunt a lot, learning the ways of the forestmaster.\n\r", ch );
	    send_to_char( "F)  Work hard in a trade, earning both money and skills.\n\r\n\r", ch );
	    send_to_char( "How did you spend your early years? ", ch );
	    ch->desc->connected = CON_GET_QUESTION;
	}
	if ( !IS_NPC(ch) && buf_string(ch->pcdata->buffer)[0] != '\0' )
	    send_to_char( "You have messages in your playback.  Type REPLAY to read them.\n\r", ch );

        return;
    }

    strcpy( buf, *ch->desc->pString );

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if ( strlen(buf) + strlen(argument) >= (MAX_STRING_LENGTH - 4) )
    {
        send_to_char( "String too long, last line skipped.\n\r", ch );

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde( argument );

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument )
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            s++;
        s--;

        while( *s == ' ' )
            s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}




/*****************************************************************************
 Name:		format_area
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_area( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}


