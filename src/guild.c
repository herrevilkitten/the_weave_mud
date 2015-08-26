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

/*
    Guild Code ver. 1.0
    For use with the Weave MUD
    Copyright 1996 Eric Kidder
    Use of this code without permission is FORBIDDEN
    But if you ask nicely I'll think about it :)

    Note that the first entry of the guild file should be the entry
    for guild "none".  Otherwise, bad things can happen :)
*/

DECLARE_DO_FUN( do_oload	);
DECLARE_DO_FUN( do_wake		);

bool	is_collared( CHAR_DATA *ch );
int	isalpha( int c );


extern  FILE    *       fpArea;
extern  char            strArea[MAX_INPUT_LENGTH];

void load_guilds( )
{
    FILE *fp;
    GUILD_DATA *pGuild;
    int i;

    if ( (fp = fopen( GUILD_FILE, "r" )) == NULL )
	return;

    for ( ; ; )
    {
	pGuild = new_guild();

	pGuild->name		= fread_string( fp );
	if ( pGuild->name == NULL || pGuild->name[0] == '$' )
	    break;

	fprintf( stderr, "Load_guilds: Loading %s\n", pGuild->name );

	pGuild->savename	= fread_string( fp );
	pGuild->imms		= fread_string( fp );
	for ( i = 0; i < 4; i++ )
	{
	    pGuild->skill[i].rank	= fread_number( fp );
	    pGuild->skill[i].name	= fread_string( fp );
	}
	for ( i = 0; i < MAX_RANK; i++ )
	    pGuild->equip[i]	= fread_number( fp );

	for ( i = 0; i < MAX_SUB; i++ )
	{
	    int j;
	    for ( j = 0; j < MAX_RANK; j++ )
		pGuild->rank[i][j]	= fread_string( fp );
	}

	pGuild->members		= fread_string( fp );
	pGuild->desc		= fread_string( fp );

	if ( guild_first == NULL )
	    guild_first		= pGuild;
	else
	    guild_last->next	= pGuild;

	guild_last	= pGuild;
	pGuild->next	= NULL;
    }

    fclose( fp );
    return;
}

void save_guilds( )
{
    FILE *fp;
    GUILD_DATA *guild;
    int i;

    fclose( fpReserve );
    if ( (fp = fopen( GUILD_FILE, "w" )) == NULL )
    {
	perror( GUILD_FILE );
	return;
    }

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	fprintf( fp, "%s~\n",	guild->name	);
	fprintf( fp, "%s~\n",	guild->savename	);
	fprintf( fp, "%s~\n",	guild->imms	);
	for ( i = 0; i < 4; i++ )
	{
	    fprintf( fp, "%d %s~\n", guild->skill[i].rank,
		guild->skill[i].name );
	}
	for ( i = 0; i < MAX_RANK; i++ )
	    fprintf( fp, "%d\n", guild->equip[i] );

	for ( i = 0; i < MAX_SUB; i++ )
	{
	    int j;
	    for ( j = 0; j < MAX_RANK; j++ )
		fprintf( fp, "%s~\n", guild->rank[i][j] );
	}

	fprintf( fp, "%s~\n",	guild->members	);
	fprintf( fp, "%s~\n\n\n",	guild->desc	);
    }
    fprintf( fp, "$~" );
    fclose( fp );

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
    GUILDADD - adds a member to the guild you are in.  Must be a guild
    imm for this to work
*/
void do_guildadd( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH]; /* Person's name */
    CHAR_DATA *victim;
    GUILD_DATA *guild;

    /* No switched NPCs, sorry ... */
    if ( IS_NPC(ch) )
	return;

    /* Parse the input so we know the separate commands to run */
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "guildadd <name>\n\r", ch );
	send_to_char( "guildadd <guild name> (adds self to guild)\n\r", ch );
	return;
    }

    if ( ch->guild == 0 ) /* No guild, so can only add self to a guild */
    {
	int new_guild;

	new_guild = guild_lookup( arg1 );

	if ( new_guild == 0 )
	{
	    send_to_char( "Okay, you no longer belong to any guild.\n\r", ch );
	    free_char_guild( ch->pcdata->guild );
	    ch->guild = 0;
	    return;
	}

	if ( new_guild == -1 )
	{
	    send_to_char( "That is not a guild.\n\r", ch );
	    return;
	}

	act( "Okay, you are now a member of the $t guild.", ch,
	    guild_name( new_guild ), NULL, TO_CHAR );
	ch->guild = new_guild;
	add_guild( ch );
        guild = guild_struct( new_guild );
        if ( strstr(guild->members, ch->name) == '\0' )
        {       
	    char buf[MAX_STRING_LENGTH];
            buf[0] = '\0';
            if ( strstr( guild->members, "None" ) != '\0' )
            {
                guild->members = string_replace( guild->members, "None","\0" );
                guild->members = string_unpad( guild->members );
            }
        
            if ( guild->members[0] != '\0' )
            {
                strcat( buf, guild->members );
                strcat( buf, " " );
            }
            strcat( buf, ch->name );
            free_string( guild->members );
            guild->members = string_proper( str_dup( buf ) );
        }

	return;
    }

    /* Member of guild, but is CH a guild imm? */
    if ( !is_guild_imm(ch) && ch->level < L2 )
    {
	send_to_char( "You must be a guild imm in order to use this command.\n\r", ch );
	return;
    }

    /*
	Okay, person is a guild imm, search for the victim.  Victim must
	be in the room in order to be guilded.
    */
    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    /* Victim is in the room, but do they belong to another guild? */
    if ( IS_GUILDED(victim) )
    {
	act( "$N belongs to another guild.", ch, NULL, victim, TO_CHAR );
	return;
    }

    /* FINALLY, guild the sucker :) */
    add_guild( victim );
    victim->guild = ch->guild;

    guild = guild_struct( victim->guild );
    if ( guild && strstr(guild->members, victim->name) == '\0' )
    {       
	char buf[MAX_STRING_LENGTH];
        buf[0] = '\0';
        if ( strstr( guild->members, "None" ) != '\0' )
        {
            guild->members = string_replace( guild->members, "None","\0" );
            guild->members = string_unpad( guild->members );
        }
        
        if ( guild->members[0] != '\0' )
        {
            strcat( buf, guild->members );
            strcat( buf, " " );
        }
        strcat( buf, victim->name );
        free_string( guild->members );
        guild->members = string_proper( str_dup( buf ) );

        send_to_char( "Member added.\n\r", ch );
    }

    return;
}

/*
    GUILDREMOVE - Removes a person from a guild.  Must be guild imm or
    level MAX_LEVEL-2 to use this command.
*/
void do_guildremove( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    GUILD_DATA *guild;
    int old_guild;

    /* No switched NPCs, sorry ... */
    if ( IS_NPC(ch) )
	return;

    /* Parse the input so we know the separate commands to run */
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "guildremove <name>\n\r", ch );
	return;
    }

    if ( ch->guild == 0 )
    {
	send_to_char( "You must be a member of a guild in order to use this command.\n\r", ch );
	return;
    }

    /* Member of guild, but is CH a guild imm? */
    if ( !is_guild_imm(ch) && ch->level < L2 )
    {
	send_to_char( "You must be a guild imm in order to use this command.\n\r", ch );
	return;
    }

    /*
	Okay, person is a guild imm, search for the victim.  Victim must
	be in the room in order to be removed.
    */
    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    /* Victim is in the room, but do they belong CH's guild? */
    if ( !IS_GUILDED(victim) || ch->guild != victim->guild )
    {
	act( "$N does not belong to your guild.", ch, NULL, victim, TO_CHAR );
	return;
    }

    old_guild = victim->guild;
    victim->guild = 0;
    free_char_guild( victim->pcdata->guild );

    guild = guild_struct( old_guild );
    if ( guild && strstr(guild->members, victim->name) != '\0' )
    {       
	char buf[MAX_STRING_LENGTH];
        buf[0] = '\0';

	guild->members = string_replace( guild->members, victim->name, "\0" );
        guild->members = string_unpad( guild->members );

	if ( guild->members == '\0' )
	{
	    free_string( guild->members );
	    guild->members = str_dup( "None" );
	}
        send_to_char( "Member removed.\n\r", ch );
    }
    return;
}

void do_guildrank( CHAR_DATA *ch, char *argument )
{

    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    GUILD_DATA *guild;
    sh_int rank, num, i;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    one_argument( argument, arg3 );
    

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2) )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "guildrank <name> <subrank number> <rank>\n\r", ch);
	return;
    }

    num = atoi(arg2);
    rank = atoi(arg3);

    if ( num < 1 || num > MAX_SUB )
    {
	sprintf( buf, "Valid sub rank ranges are 1 to %d.\n\r", MAX_SUB );
	send_to_char( buf, ch );
	return;
    }

    if ( rank < 1 || rank > MAX_RANK )
    {
	sprintf( buf, "Valid rank ranges are 1 to %d.\n\r", MAX_RANK );
	send_to_char( buf, ch );
	return;
    }

    if ( ch->guild == 0 )
    {
	send_to_char( "You are not a member of a guild.\n\r", ch );
	return;
    }

    if ( !is_guild_imm(ch) && ch->level < L2 )
    {
	send_to_char( "You must be a guild imm in order to use this command.\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( !IS_GUILDED(victim) )
    {
	act( "$N does not belong to a guild.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if ( victim->guild != ch->guild )
    {
	act( "$N belongs to another guild.", ch, NULL, victim, TO_CHAR );
	return;
    }

    guild = guild_struct( ch->guild );

    if ( GET_RANK(victim, num) == rank - 1 )
    {
	act( "$N is already that rank.", ch, NULL, victim, TO_CHAR );
	return;
    }

    sprintf( buf, "$n has set your sub rank %d to %s.", num,
	guild_rank(victim->guild, rank-1, num, FALSE) );
    act( buf, ch, NULL, victim, TO_VICT );
    sprintf( buf, "You set $O sub rank %d to %s.", num,
	guild_rank(victim->guild, rank-1, num, FALSE) );
    act( buf, ch, NULL, victim, TO_CHAR );

    if ( num == 1 )
    {
	for (i = 0; i < 4; i++ )
	{
	    int sn;
	    if ( (sn = skill_lookup( guild->skill[i].name )) == -1 )
		continue;

	    if ( guild->skill[i].rank > rank
	    &&   victim->pcdata->learned[sn] > -1 )
	    {
		victim->pcdata->learned[sn] = -1;
		act( "You have lost the guild skill of $t.", victim,
		    guild->skill[i].name, NULL, TO_CHAR );
	    }
	    else if ( guild->skill[i].rank <= rank
	    &&        victim->pcdata->learned[sn] < 70 )
	    {
		victim->pcdata->learned[sn] = 70;
		act( "You have gained the guild skill of $t.", victim,
		    guild->skill[i].name, NULL, TO_CHAR );
	    }
	}
    }
    SET_RANK(victim, num, rank - 1);
    return;
}


void do_shdname( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "shdname <person> <shadow name>\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( !is_darkfriend(victim) )
    {
	act( "$N is not a darkfriend.", ch, NULL, victim, TO_CHAR );
	return;
    }

    free_string( victim->pcdata->shadow_name );
    victim->pcdata->shadow_name = str_dup( capitalize(arg2) );
    act( "$O shadow name is now '$t'.", ch, victim->pcdata->shadow_name,
	victim, TO_CHAR );
    act( "Your shadow name is now '$t'.", ch, victim->pcdata->shadow_name,
	victim, TO_VICT );
    return;
}


void do_damname( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "damname <person> <damane name>\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( victim->guild != guild_lookup("seanchan") )
    {
	act( "$N is not part of the Seanchan guild.", ch, NULL, victim,
	    TO_CHAR );
	return;
    }

    if ( GET_RANK(victim, 1) != 0 )
    {
	act( "$N is not a damane.", ch, NULL, victim, TO_CHAR );
	return;
    }

    free_string( victim->pcdata->guild->damane_name );
    victim->pcdata->guild->damane_name = str_dup( capitalize(arg2) );
    act( "$O damane name is now '$t'.", ch, victim->pcdata->guild->damane_name,
	victim, TO_CHAR );
    act( "Your damane name is now '$t'.", ch, victim->pcdata->guild->damane_name,
	victim, TO_VICT );
    return;
}


void do_shdrank( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    sh_int rank;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2) )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "shdrank <name> <rank>\n\r", ch );
	return;
    }

    rank = atoi(arg2);

    if ( rank < 0 || rank > MAX_RANK )
    {
	sprintf( buf, "Valid rank ranges are 0 to %d.\n\r", MAX_RANK );
	send_to_char( buf, ch );
	return;
    }

    if ( ch->guild != guild_lookup( "darkfriend" ) )
    {
	send_to_char( "Only Shaidar Haran and the Dark One can use this command.\n\r", ch );
	return;
    }

    if ( (victim = get_char_room( ch, arg1 )) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( rank == 0 )
    {
	victim->pcdata->shadow_rank = -1;
	act( "You have been removed from the Darkfriends by $n.",
	    ch, NULL, victim, TO_VICT );
	act( "You have removed $N from the Darkfriends.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }
    rank--;
    if ( victim->pcdata->shadow_rank > rank )
    {
	act( "You have been demoted to $t by $n.", ch,
	    guild_rank(victim->guild, rank, 1, FALSE), victim, TO_VICT);
	act( "You have demoted $N to $t.", ch,
	    guild_rank(guild_lookup("darkfriend"), rank, 1, FALSE),
	    victim, TO_CHAR );
	victim->pcdata->shadow_rank = rank;
    }
    else if ( victim->pcdata->shadow_rank < rank )
    {
	act( "You have been promoted to $t by $n.", ch,
	    guild_rank(victim->guild, rank, 1, FALSE), victim, TO_VICT );
	act( "You have promoted $N to $t.", ch,
	    guild_rank(guild_lookup("darkfriend"), rank, 1, FALSE),
	    victim, TO_CHAR );
	victim->pcdata->shadow_rank = rank;
    }
    return;
}

void do_guildeq( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    GUILD_DATA *guild;
    int eq;

    /* No switched NPCs, sorry ... */
    if ( IS_NPC(ch) )
	return;

    /* Parse the input so we know the separate commands to run */
    one_argument( argument, arg1 );

    if ( arg1[0] == '\0' || !is_number(arg1) )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "guildeq <1-12>\n\r", ch );
	return;
    }

    eq = atoi(arg1);

    if ( eq < 1 || eq > 12 )
    {
	sprintf( buf, "Valid eq ranges are 1 to 12.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    if ( ch->guild == 0 )
    {
	send_to_char( "You are not a member of a guild.\n\r", ch );
	return;
    }

    guild = guild_struct( ch->guild );

    if ( !is_guild_imm(ch) && ch->level < L2 )
    {
	send_to_char( "You must be a guild imm in order to use this command.\n\r", ch );
	return;
    }

    sprintf( buf, "%d", guild->equip[eq - 1] );
    do_oload( ch, buf );
    return;
}

void do_gstat( CHAR_DATA *ch, char *argument )
{
    GUILD_DATA *pGuild;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int num, i, j;

    one_argument( argument, arg );

    if ( (num = guild_lookup( arg )) == -1 )
    {
	send_to_char( "That is not a guild.\n\r", ch );
	return;
    }

    if ( num == 0 )
    {
	send_to_char( "0 is the NONE guild.  It may not be statted.\n\r", ch );
	return;
    }

    pGuild = guild_struct( num );

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
    return;
}

bool is_guild_eq( int vnum )
{
    GUILD_DATA *guild;

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	int i;
	for ( i = 0; i < 12; i++ )
	{
	    if ( vnum == guild->equip[i] )
		return TRUE;
	}
    }
    return FALSE;
}

int get_skill_rank( int sn )
{
    GUILD_DATA *guild;

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	int i;
	for ( i = 0; i < 4; i++ )
	{
	    if ( guild->skill[i].sn == sn )
		return (guild->skill[i].rank - 1);
	}
    }
 
    return -99;
}

int get_guild_skill( int sn )
{
    GUILD_DATA *guild;
    int guild_num = 0;

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	int i;
	guild_num++;
	for ( i = 0; i < 4; i++ )
	{
	    if ( guild->skill[i].sn == sn )
		return guild_num;
	}
    }
 
    return 0;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( !str_prefix(arg, "list") )
    {
	GUILD_DATA *guild;
	char buf[MAX_STRING_LENGTH];

	if ( guild_first == NULL )
	{
	    send_to_char( "The guilds on Weave seem to be broken at the moment.\n\r", ch );
	    return;
	}

	send_to_char("The following guilds are available on the `1W`2e`3a`4v`5e`n:\n\r", ch );

	for ( guild = guild_first->next; guild != NULL; guild = guild->next )
	{
	    sprintf( buf, "  `4" );
	    strcat( buf, guild->name );
	    strcat( buf, "`n is run by `4" );
	    strcat( buf, guild->imms );
	    strcat( buf, "`n\n\r" );
	    send_to_char( buf, ch );
	}
    }
    return;
}


