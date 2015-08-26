/***************************************************************************
 *  File: olc_guild.c                                                      *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This work is a derivative of Talen's post to the Merc Mailing List.    *
 *  It has been modified by Jason Dinkel to work with the new OLC.         *
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
#include "olc.h"


#define GEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define EDIT_GUILD(Ch, Guild)	( Guild = (GUILD_DATA *)Ch->desc->pEdit )

/*
 * Guild Editor Prototypes
 */
DECLARE_OLC_FUN( gedit_create 		);
DECLARE_OLC_FUN( gedit_delete 		);
DECLARE_OLC_FUN( gedit_desc 		);
DECLARE_OLC_FUN( gedit_show 		);
DECLARE_OLC_FUN( gedit_name 		);
DECLARE_OLC_FUN( gedit_savename		);
DECLARE_OLC_FUN( gedit_imms 		);
DECLARE_OLC_FUN( gedit_rank 		);
DECLARE_OLC_FUN( gedit_equip 		);
DECLARE_OLC_FUN( gedit_skill 		);



const struct olc_cmd_type gedit_table[] =
{
/*  {   command		function	}, */

    {   "commands",	show_commands	},
    {   "create",	gedit_create	},
    {   "delete",	gedit_delete	},
    {   "desc",		gedit_desc	},
    {	"name",		gedit_name	},
    {	"savename",	gedit_savename	},
    {	"imms",		gedit_imms	},
    {	"rank",		gedit_rank	},
    {	"equip",	gedit_equip	},
    {	"skill",	gedit_skill	},
    {	"show",		gedit_show	},

    {   "?",		show_help	},

    {	"",		0,		}
};



void gedit( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( (ch->pcdata->security == 0 || !is_guild_imm( ch ))
    &&   ch->level < L2 )
	send_to_char( "GEdit: Insufficient security to modify guild.\n\r", ch );

    if( command[0] == '\0' )
    {
	gedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( (ch->pcdata->security == 0 || !is_guild_imm( ch ))
    &&   ch->level < L2 )
    {
	substitute_alias( ch->desc, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; gedit_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( !str_cmp( command, gedit_table[cmd].name ) )
	{
	    (*gedit_table[cmd].olc_fun) ( ch, argument );
	    return;
	}
    }

    /* Default to Standard Interpreter. */
    substitute_alias( ch->desc, arg );
    return;
}



/* Entry point for editing help_data. */
void do_gedit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    GUILD_DATA	*iGuild;

    if( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  edit guild <keywords>\n\r", ch );
	return;
    }
    else
    {
	for( iGuild = guild_first; iGuild; iGuild= iGuild->next )
	{
	    /*
	     * This guild better not exist already!
	     */
	    if( is_full_name( arg, iGuild->name ) )
	    {
		ch->desc->pEdit = (void *)iGuild;
		ch->desc->editor = ED_GUILD;
		break;
	    }
	}

	if( !iGuild )
	{
	    iGuild			= new_guild();
	    iGuild->name		= str_dup( arg );

	    if( !guild_first )
		guild_first		= iGuild;
	    if( guild_last )
		guild_last->next	= iGuild;

	    guild_last			= iGuild;
	    iGuild->next		= NULL;
	    ch->desc->pEdit		= (void *)iGuild;
	    ch->desc->editor		= ED_GUILD;
	}
    }
    return;
}



GEDIT( gedit_show )
{
    int i, j;
    GUILD_DATA *pGuild;
    char buf[MAX_STRING_LENGTH];
    
    if ( !EDIT_GUILD( ch, pGuild ) )
    {
	send_to_char( "Null guild file.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Name        [%s]\n\r", pGuild->name     );
    send_to_char( buf, ch );
    sprintf( buf, "Savename    [%s]\n\r", pGuild->savename );
    send_to_char( buf, ch );
    sprintf( buf, "Imms        [%s]\n\r", pGuild->imms     );
    send_to_char( buf, ch );
    send_to_char( "Description:\n\r", ch );
    page_to_char( pGuild->desc, ch );
    send_to_char( "\n\r", ch );
    for ( i = 0; i < 4; i++ )
    {
	send_to_char_new( ch, "Skill%2d: [%d - %30.30s]\n\r",
	    i+1, pGuild->skill[i].rank, pGuild->skill[i].name );
    }
    for ( i = 0; i < MAX_RANK; i++ )
    {
	sprintf( buf, "Equip%2d [%5d - %25.25s`n]\n\r",
	    i+1, pGuild->equip[i],
	    get_obj_index(pGuild->equip[i]) ? 
	    get_obj_index(pGuild->equip[i])->short_descr : "not made" );
	send_to_char( buf, ch );
    }

    j = 0;
    send_to_char_new( ch, "Ranks:\n\r"
                          "%-18.18s %-18.18s %-18.18s %-18.18s\n\r",
        "Group 1", "Group 2", "Group 3", "Group 4");
    for ( i = 0; i < MAX_SUB * MAX_RANK; i++ )
    {
        sprintf( buf, "%2d:%-15.15s ", j+1,
            pGuild->rank[i % MAX_SUB][j] );
        send_to_char( buf, ch );
        if ( (i+1) % MAX_SUB == 0 )
        {
            j++;
            send_to_char( "\n\r", ch );
        }
    }

    send_to_char( "Members:\n\r", ch );
    send_to_char( pGuild->members, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}

GEDIT( gedit_name )
{
    GUILD_DATA *pGuild;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
        send_to_char( "Null guild file.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name [string]\n\r", ch );
        return FALSE;
    }
    
    free_string( pGuild->name );
    pGuild->name = str_dup( argument );
    
    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}

GEDIT( gedit_savename )
{
    GUILD_DATA *pGuild;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
        send_to_char( "Null guild file.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name [string]\n\r", ch );
        return FALSE;
    }
    
    free_string( pGuild->savename );
    pGuild->savename = str_dup( argument );
    
    send_to_char( "Save name set.\n\r", ch);
    return TRUE;
}

GEDIT( gedit_imms )
{
    GUILD_DATA *pGuild;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, name );

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
	send_to_char( "Null guild file.\n\r", ch );
	return FALSE;
    }
 
    if( argument[0] == '\0' )
    {
	send_to_char( "Syntax: imms <name>\n\r", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );
    
    if ( strstr( pGuild->imms, name ) != '\0' )
    {
        pGuild->imms = string_replace( pGuild->imms, name, "\0" );
        pGuild->imms = string_unpad( pGuild->imms );
        
        if ( pGuild->imms[0] == '\0' )
        {
            free_string( pGuild->imms );
            pGuild->imms = str_dup( "None" );
        }
        send_to_char( "Immortal removed.\n\r", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
        if ( strstr( pGuild->imms, "None" ) != '\0' )
        {
            pGuild->imms = string_replace( pGuild->imms, "None", "\0" );
            pGuild->imms = string_unpad( pGuild->imms );
        }
    
        if (pGuild->imms[0] != '\0' ) 
        {
            strcat( buf, pGuild->imms );
            strcat( buf, " " );
        }
        strcat( buf, name );
        free_string( pGuild->imms );
        pGuild->imms = string_proper( str_dup( buf ) );
 
        send_to_char( "Immortal added.\n\r", ch );
        return TRUE;  
    }

    return FALSE;
}

GEDIT( gedit_skill )
{
    GUILD_DATA *pGuild;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int rank, skill;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
        send_to_char( "Null guild file.\n\r", ch );
        return FALSE;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1)
    ||   arg2[0] == '\0' || !is_number(arg2) )
    {
        send_to_char( "Syntax:  skill [number] [rank] [skill]\n\r", ch );
        return FALSE;
    }

    skill	= atoi( arg1 );
    rank	= atoi( arg2 );

    if ( skill < 1 || skill > 4 )
    {
	send_to_char( "Skill range is 1 to 4.\n\r", ch );
	return FALSE;
    }

    if ( rank < 1 || rank > MAX_RANK )
    {
	send_to_char_new( ch, "Rank range is 1 to %d.\n\r", MAX_RANK );
	return FALSE;
    }

    free_string( pGuild->skill[skill-1].name );
    pGuild->skill[skill-1].rank = rank;
    pGuild->skill[skill-1].name = str_dup( argument );
    send_to_char( "Skill set.\n\r", ch );
    return TRUE;
}

GEDIT( gedit_rank )
{
    GUILD_DATA *pGuild;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int sub, rank;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
        send_to_char( "Null guild file.\n\r", ch );
        return FALSE;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1) || !is_number(arg2) )
    {
        send_to_char( "Syntax:  rank [sub_group] [rank_number] [string]\n\r", ch );
        return FALSE;
    }

    sub  = atoi( arg1 );
    rank = atoi( arg2 );

    if ( sub < 1 || sub > MAX_SUB )
    {
	send_to_char_new( ch, "Sub group range is 1 to %d.\n\r", MAX_RANK );
	return FALSE;
    }

    if ( rank < 1 || rank > MAX_RANK )
    {
	send_to_char_new( ch, "Rank number range is 1 to %d.\n\r",
	    MAX_RANK );
	return FALSE;
    }

    free_string( pGuild->rank[sub-1][rank-1] );
    pGuild->rank[sub-1][rank-1] = str_dup( argument );
    send_to_char( "Rank set.\n\r", ch );
    return TRUE;
}

GEDIT( gedit_equip )
{
    GUILD_DATA *pGuild;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int rank;
    int vnum;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
        send_to_char( "Null guild file.\n\r", ch );
        return FALSE;
    }

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1)
    ||   arg2[0] == '\0' || !is_number(arg2) )
    {
        send_to_char( "Syntax:  equip [number] [vnum]\n\r", ch );
        return FALSE;
    }

    rank = atoi( arg1 );
    vnum = atoi( arg2 );

    if ( rank < 1 || rank > MAX_RANK )
    {
	send_to_char_new( ch, "Range is 1 to %d.\n\r", MAX_RANK );
	return FALSE;
    }

    pGuild->equip[rank-1] = vnum;
    send_to_char( "Equipment set.\n\r", ch );
    return TRUE;
}
    

GEDIT( gedit_create )
{
    GUILD_DATA *iGuild;
    GUILD_DATA *NewGuild;
    char buf[MAX_STRING_LENGTH];

    if ( !EDIT_GUILD( ch, iGuild ) )
    {
	send_to_char( "Null guild file.\n\r", ch );
	return FALSE;
    }

    if( argument[0] == '\0' )
    {
	send_to_char( "Syntax: create <name>\n\r", ch );
	return FALSE;
    }

    /*
     * This guild better not exist already!
     */
    for( iGuild = guild_first; iGuild; iGuild = iGuild->next )
    {
	if( is_name( argument, iGuild->name ) )
	{
	    send_to_char( "That guild already exists.\n\r", ch );
	    return FALSE;
	}
    }

    NewGuild		= new_guild();
    NewGuild->name	= str_dup( argument );

    if( !guild_first )	/* If it is we have a leak */
	guild_first		= NewGuild;
    if( guild_last )
	guild_last->next	= NewGuild;

    guild_last	= NewGuild;
    NewGuild->next	= NULL;
    ch->desc->pEdit	= (void *)NewGuild;
    ch->desc->editor = ED_GUILD;

    sprintf( buf, "Created guild with name: %s\n\r",
	NewGuild->name );
    send_to_char( buf, ch );

    return TRUE;
}



GEDIT( gedit_delete )
{
    GUILD_DATA *pGuild;
    GUILD_DATA *PrevGuild = NULL;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
	send_to_char( "Null guild file.\n\r", ch );
	return FALSE;
    }

    if( argument[0] == '\0' )
    {
	send_to_char( "Syntax: delete <name>\n\r", ch );
	return FALSE;
    }

    /*
     * This guild better exist
     */
    for( pGuild = guild_first; pGuild; PrevGuild = pGuild, pGuild = pGuild->next )
    {
	if( is_name( argument, pGuild->name ) )
	    break;
    }

    if( !pGuild )
    {
	send_to_char( "That guild does not exist.\n\r", ch );
	return FALSE;
    }

    if( pGuild == (GUILD_DATA *)ch->desc->pEdit )
    {
	edit_done( ch );
    }

    if( !PrevGuild )          /* At first help file   */
    {
	guild_first  = pGuild->next;
	free_guild( pGuild );
    }
    else if( !pGuild->next )  /* At the last help file*/
    {
	guild_last           = PrevGuild;
	PrevGuild->next      = NULL;
	free_guild( pGuild );
    }
    else                            /* Somewhere else...    */
    {
	PrevGuild->next      = pGuild->next;
	free_guild( pGuild );
    }

    send_to_char( "Guild deleted.\n\r", ch );
    return TRUE;
}



GEDIT( gedit_desc )
{
    GUILD_DATA *pGuild;

    if ( !EDIT_GUILD( ch, pGuild ) )
    {
	send_to_char( "Null guild file.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] != '\0' )
    {
	send_to_char( "Syntax:  desc\n\r", ch );
	return FALSE;
    }
    
    string_append( ch, &pGuild->desc );
    return TRUE;
}
