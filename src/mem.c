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
/***************************************************************************
 *  File: mem.c                                                            *
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


/*
 * Globals
 */
extern		int			top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern		int			top_guard;
extern		int			top_guild;
extern		int			top_help;
extern		int			top_mob_index;
extern		int			top_node;
extern		int			top_note;
extern		int			top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern		int			top_shop;

extern		int			mobile_count;
extern		int			object_count;

extern		bool			fBootDb;
extern		bool			fPlaceTree;

HELP_DATA		*	help_free;
HELP_DATA		*	help_first;
HELP_DATA		*	help_last;

SHOP_DATA		*	shop_free;
SHOP_DATA		*	shop_first;
SHOP_DATA		*	shop_last;

GUILD_DATA		*	guild_free;
GUILD_DATA		*	guild_first;
GUILD_DATA		*	guild_last;

/*
 *  Not used in this file
 */
CHAR_DATA		*	char_free;
OBJ_DATA		*	obj_free;
PC_DATA			*	pcdata_free;

NOTE_DATA		*	note_free;
AREA_DATA		*	area_free;
EXTRA_DESCR_DATA	*	extra_descr_free;
EXIT_DATA		*	exit_free;
ROOM_INDEX_DATA		*	room_index_free;
OBJ_INDEX_DATA		*	obj_index_free;
MOB_INDEX_DATA		*	mob_index_free;
RESET_DATA		*	reset_free;
NODE_DATA		*	node_free;
GEN_DATA		*	gen_free;
TEXT_DATA		*	text_free;
GUARD_DATA		*	guard_free;
GUILD			*	char_guild_free;


RESET_DATA *new_reset_data( void )
{
    static RESET_DATA pReset_zero;
    RESET_DATA *pReset;

    if ( !reset_free )
    {
	wiznet( "Allocating new reset.\r\n", NULL, NULL, WIZ_MEMORY, 0, 0 );
        pReset          =   alloc_perm( sizeof(*pReset) );
        top_reset++;
    }
    else
    {
        pReset          =   reset_free;
        reset_free      =   reset_free->next;
    }

    *pReset		=   pReset_zero;
    pReset->command     =   'X';

    return pReset;
}



void free_reset_data( RESET_DATA *pReset )
{
    pReset->next            = reset_free;
    reset_free              = pReset;
    return;
}



AREA_DATA *new_area( void )
{
    AREA_DATA *pArea;
    char buf[MAX_INPUT_LENGTH];

    if ( !area_free )
    {
	wiznet( "Allocating new area.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pArea   =   alloc_perm( sizeof(*pArea) );
        top_area++;
    }
    else
    {
        pArea       =   area_free;
        area_free   =   area_free->next;
    }

    pArea->next             =   NULL;
    pArea->name             =   str_dup( "New area" );
    pArea->area_flags       =   AREA_ADDED;
    pArea->security         =   1;
    pArea->builders         =   str_dup( "None" );
    pArea->moddate	    =   &str_empty[0];
    pArea->lvnum            =   0;
    pArea->uvnum            =   0;
    pArea->age              =   0;
    pArea->nplayer          =   0;
    pArea->empty            =   TRUE;              /* ROM patch */
    pArea->climate	    =   CLIMATE_TEMPERATE;
    pArea->weather	    =   WEATHER_SUNNY;
    pArea->terrain	    =   TERRAIN_GRASSLAND;
    sprintf( buf, "area%d.are", pArea->vnum );
    pArea->filename         =   str_dup( buf );
    pArea->vnum             =   top_area - 1;
    if ( fBootDb )
    {
	pArea->age		= 30;
        pArea->empty		= TRUE;              /* ROM patch */
	pArea->nplayer		= 0;
    }
    return pArea;
}



void free_area( AREA_DATA *pArea )
{
    free_string( pArea->name );
    free_string( pArea->filename );
    free_string( pArea->builders );
    free_string( pArea->moddate );

    pArea->next         =   area_free->next;
    area_free           =   pArea;
    return;
}



EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;

    if ( !exit_free )
    {
	wiznet( "Allocating new exit.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pExit           =   alloc_perm( sizeof(*pExit) );
        top_exit++;
    }
    else
    {
        pExit           =   exit_free;
        exit_free       =   exit_free->next;
    }

    pExit->u1.to_room   =   NULL;                  /* ROM OLC */
    pExit->next         =   NULL;
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   &str_empty[0];
    pExit->description  =   &str_empty[0];
    pExit->rs_flags     =   0;

    return pExit;
}



void free_exit( EXIT_DATA *pExit )
{
    free_string( pExit->keyword );
    free_string( pExit->description );

    pExit->next         =   exit_free;
    exit_free           =   pExit;
    return;
}



EXTRA_DESCR_DATA *new_extra_descr( void )
{
    EXTRA_DESCR_DATA *pExtra;

    if ( !extra_descr_free )
    {
	wiznet( "Allocating new extra description.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pExtra              =   alloc_perm( sizeof(*pExtra) );
        top_ed++;
    }
    else
    {
        pExtra              =   extra_descr_free;
        extra_descr_free    =   extra_descr_free->next;
    }

    pExtra->keyword         =   NULL;
    pExtra->description     =   NULL;
    pExtra->next            =   NULL;

    return pExtra;
}



void free_extra_descr( EXTRA_DESCR_DATA *pExtra )
{
    free_string( pExtra->keyword );
    free_string( pExtra->description );

    pExtra->next        =   extra_descr_free;
    extra_descr_free    =   pExtra;
    return;
}



ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    if ( !room_index_free )
    {
	wiznet( "Allocating new room.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pRoom           =   alloc_perm( sizeof(*pRoom) );
        top_room++;
    }
    else
    {
        pRoom           =   room_index_free;
        room_index_free =   room_index_free->next;
    }

    pRoom->next             =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->extra_descr      =   NULL;
    pRoom->area             =   NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->name             =   &str_empty[0];
    pRoom->description      =   &str_empty[0];
    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;
    pRoom->resources	    =   0;
    pRoom->spec_fun	    =	NULL;

    return pRoom;
}



void free_room_index( ROOM_INDEX_DATA *pRoom )
{
    int door;
    EXTRA_DESCR_DATA *pExtra, *ed_next;
    RESET_DATA *pReset, *reset_next;

    free_string( pRoom->name );
    free_string( pRoom->description );

    for ( door = 0; door < MAX_DIR; door++ )
    {
        if ( pRoom->exit[door] )
            free_exit( pRoom->exit[door] );
    }

    for ( pExtra = pRoom->extra_descr; pExtra; pExtra = ed_next )
    {
	ed_next = pExtra->next;
        free_extra_descr( pExtra );
    }

    for ( pReset = pRoom->reset_first; pReset; pReset = reset_next )
    {
	reset_next = pReset->next;
        free_reset_data( pReset );
    }

    pRoom->next     =   room_index_free;
    room_index_free =   pRoom;
    return;
}



AFFECT_DATA *new_affect( void )
{
    AFFECT_DATA *pAf;

    if ( !affect_free )
    {
	wiznet( "Allocating new affect.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pAf             =   alloc_perm( sizeof(*pAf) );
        top_affect++;
    }
    else
    {
        pAf             =   affect_free;
        affect_free     =   affect_free->next;
    }

    pAf->next		= NULL;
    pAf->location	= 0;
    pAf->modifier	= 0;
    pAf->type		= 0;
    pAf->duration	= 0;
    pAf->bitvector	= 0;
    pAf->bitvector_2	= 0;
    pAf->owner		= NULL;
    pAf->flags		= 0;

    return pAf;
}



void free_affect( AFFECT_DATA* pAf )
{
    pAf->next           = affect_free;
    affect_free         = pAf;
    return;
}



SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop;
    int buy;

    if ( !shop_free )
    {
	wiznet( "Allocating new shop.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pShop           =   alloc_perm( sizeof(*pShop) );
        top_shop++;
    }
    else
    {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   =   100;
    pShop->profit_sell  =   100;
    pShop->open_hour    =   0;
    pShop->close_hour   =   23;

    return pShop;
}



void free_shop( SHOP_DATA *pShop )
{
    pShop->next = shop_free;
    shop_free   = pShop;
    return;
}



OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !obj_index_free )
    {
	wiznet( "Allocating new object index.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pObj           =   alloc_perm( sizeof(*pObj) );
        top_obj_index++;
    }
    else
    {
        pObj            =   obj_index_free;
        obj_index_free  =   obj_index_free->next;
    }

    pObj->next          =   NULL;
    pObj->extra_descr   =   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->version	=   OBJ_VERSION;
    pObj->name          =   str_dup( "no name" );
    pObj->short_descr   =   str_dup( "(no short description)" );
    pObj->description   =   str_dup( "(no description)" );
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    pObj->qp		=   0;
    pObj->material      =   material_lookup( "" );      /* ROM */
    pObj->condition     =   100;                        /* ROM */
    for ( value = 0; value < 8; value++ )               /* 8 - Weave */
        pObj->value[value]  =   0;
    pObj->max		=   0;
    pObj->spec_fun	= NULL;
    pObj->use_fun	= NULL;

    pObj->new_format    = TRUE; /* ROM */

    return pObj;
}



void free_obj_index( OBJ_INDEX_DATA *pObj )
{
    EXTRA_DESCR_DATA *pExtra;
    AFFECT_DATA *pAf;

    free_string( pObj->name );
    free_string( pObj->short_descr );
    free_string( pObj->description );

    for ( pAf = pObj->affected; pAf; pAf = pAf->next )
    {
        free_affect( pAf );
    }

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next )
    {
        free_extra_descr( pExtra );
    }
    
    pObj->next              = obj_index_free;
    obj_index_free          = pObj;
    return;
}



MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;

    if ( !mob_index_free )
    {
	wiznet( "Allocating new mob index.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pMob           =   alloc_perm( sizeof(*pMob) );
        top_mob_index++;
    }
    else
    {
        pMob            =   mob_index_free;
        mob_index_free  =   mob_index_free->next;
    }

    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->player_name   =   str_dup( "no name" );
    pMob->short_descr   =   str_dup( "(no short description)" );
    pMob->long_descr    =   str_dup( "(no long description)\n\r" );
    pMob->description   =   &str_empty[0];
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_IS_NPC;
    pMob->affected_by   =   0;
    pMob->hitroll	=   0;
    pMob->race          =   race_lookup( "human" ); /* - Hugin */
    pMob->form          =   0;           /* ROM patch -- Hugin */
    pMob->parts         =   0;           /* ROM patch -- Hugin */
    pMob->imm_flags     =   0;           /* ROM patch -- Hugin */
    pMob->res_flags     =   0;           /* ROM patch -- Hugin */
    pMob->vuln_flags    =   0;           /* ROM patch -- Hugin */
    pMob->material      =   material_lookup( "" ); /* -- Hugin */
    pMob->off_flags     =   0;           /* ROM patch -- Hugin */
    pMob->size          =   SIZE_MEDIUM; /* ROM patch -- Hugin */
    pMob->ac[AC_PIERCE]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_BASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_SLASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_EXOTIC]	=   0;           /* ROM patch -- Hugin */
    pMob->hit[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->start_pos             =   POS_STANDING; /*  -- Hugin */
    pMob->default_pos           =   POS_STANDING; /*  -- Hugin */
    pMob->gold                  =   0;
    pMob->guild			=   0;
    pMob->max			=   1;
    pMob->text			= &str_empty[0];
    pMob->guard			= NULL;

    pMob->new_format            = TRUE;  /* ROM */

    return pMob;
}



void free_mob_index( MOB_INDEX_DATA *pMob )
{
    GUARD_DATA *gd, *gd_next;

    free_string( pMob->player_name );
    free_string( pMob->short_descr );
    free_string( pMob->long_descr );
    free_string( pMob->description );
    free_string( pMob->text );

    for ( gd = pMob->guard; gd; gd = gd_next )
    {
	gd_next = gd->next;
	free_guard( gd );
    }
    free_shop( pMob->pShop );

    pMob->next              = mob_index_free;
    mob_index_free          = pMob;
    return;
}

HELP_DATA *new_help(void)
{
    HELP_DATA *NewHelp;

    if( help_free == NULL )
    {
	wiznet( "Allocating new help.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        NewHelp = alloc_perm( sizeof(*NewHelp) );
        top_help++;
    }
    else
    {
        NewHelp         = help_free;
        help_free       = help_free->next;
    }

    NewHelp->next       = NULL;
    NewHelp->level      = 0;
    NewHelp->keyword    = &str_empty[0];
    NewHelp->text       = &str_empty[0];

    return NewHelp;
}



void free_help( HELP_DATA *pHelp )
{
    free_string( pHelp->keyword );
    free_string( pHelp->text );

    pHelp->next = help_free;
    help_free   = pHelp;
    return;
}

GUILD_DATA *new_guild(void)
{
    GUILD_DATA *NewGuild;
    int i;

    if( guild_free == NULL )
    {
	wiznet( "Allocating new guild.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        NewGuild = alloc_perm( sizeof(*NewGuild) );
        top_guild++;
    }
    else
    {
        NewGuild        = guild_free;
        guild_free      = guild_free->next;
    }

    NewGuild->name	 = &str_empty[0];
    NewGuild->savename	 = &str_empty[0];
    NewGuild->imms	 = &str_empty[0];
    for ( i = 0; i < 4; i++ )
    {
	NewGuild->skill[i].rank = 0;
	NewGuild->skill[i].name = &str_empty[0];
    }
    for ( i = 0; i < 12; i++ )
	NewGuild->equip[i] = 0;
    for ( i = 0; i < MAX_SUB; i++ )
    {
	int j;
	for ( j = 0; j < MAX_RANK; j++ )
	    NewGuild->rank[i][j] = &str_empty[0];
    }
    NewGuild->desc	 = &str_empty[0];
    NewGuild->members	 = &str_empty[0];
    NewGuild->next       = NULL;
    return NewGuild;
}

     
        
void free_guild( GUILD_DATA *pGuild )
{ 
    int i;
    free_string( pGuild->name		);
    free_string( pGuild->savename	);
    free_string( pGuild->imms		);
    free_string( pGuild->desc		);
    free_string( pGuild->members	);
    for ( i = 0; i < 4; i++ )
	free_string( pGuild->skill[i].name );
    for ( i = 0; i < MAX_SUB; i++ )
    {
	int j;
	for ( j = 0; j < MAX_RANK; j++ )
	    pGuild->rank[i][j] = &str_empty[0];
    }
    pGuild->next = guild_free;
    guild_free   = pGuild;
    return;
}

NODE_DATA *new_node( )
{
    NODE_DATA *node;

    if ( !node_free )
    {
	wiznet( "Allocating new node.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	node		= alloc_perm( sizeof(*node) );
	top_node++;
    }
    else
    {
	node		= node_free;
	node_free	= node_free->next;
    }

    node->next		= NULL;
    node->data		= NULL;
    node->data_type	= NODE_EMPTY;
    return( node );
}

void free_node( NODE_DATA *node )
{
    node->next		= node_free;
    node_free		= node;
    return;
}

GEN_DATA *new_gen_data( void )
{
    GEN_DATA *gen;
    int i;

    if ( !gen_free )
    {
	wiznet( "Allocating new generation data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	gen		= alloc_perm( sizeof(*gen) );
    }
    else
    {
	gen		= gen_free;
	gen_free	= gen_free->next;
    }

    for ( i = 0; i < MAX_SKILL; i++ )
	gen->skill_chosen[i] = FALSE;
    for ( i = 0; i < MAX_GROUP; i++ )
	gen->group_chosen[i] = FALSE;
    gen->points_chosen = 0;

    gen->next		= NULL;
    return( gen );
}

void free_gen_data( GEN_DATA *gen )
{
    gen->next		= gen_free;
    gen_free		= gen;
    return;
}

NOTE_DATA *new_note( void )
{
    NOTE_DATA *note;

    if ( !note_free )
    {
	wiznet( "Allocating new note.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	note		= alloc_perm( sizeof(*note) );
	top_note++;
    }
    else
    {
	note		= note_free;
	note_free	= note_free->next;
    }

    note->sender	= &str_empty[0];
    note->real_name	= &str_empty[0];
    note->guild		= 0;
    note->date		= &str_empty[0];
    note->subject	= &str_empty[0];
    note->to_list	= &str_empty[0];
    note->text		= &str_empty[0];
    note->date_stamp	= 0;
    note->next		= NULL;

    return( note );
}

void free_note( NOTE_DATA *note )
{
    free_string( note->sender		);
    free_string( note->real_name	);
    free_string( note->date		);
    free_string( note->to_list		);
    free_string( note->subject		);
    free_string( note->text		);
    note->next		= note_free;
    note_free		= note;
    return;
}

CHAR_DATA *new_char( void )
{
    static CHAR_DATA ch_zero;
    CHAR_DATA *ch;
    
    if ( char_free == NULL )
    {
	wiznet( "Allocating new character.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	ch		= alloc_perm( sizeof(*ch) );
    }
    else
    {
        ch		= char_free;
        char_free	= char_free->next;
    }

    *ch                         = ch_zero;
    ch->next			= NULL;
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->prompt			= &str_empty[0];

    ch->action_args		= &str_empty[0];
    ch->action_timer		= 0;
    ch->action_target		= NULL;

    ch->memory			= NULL;
    ch->action_timer		= 0;
    return( ch );
}

void free_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    
    if ( IS_NPC(ch) )
	mobile_count--;
   
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	extract_obj( obj );
    }
 
    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	affect_remove( ch, paf );
    }

    if ( ch->gen_data )
	free_gen_data( ch->gen_data );

    if ( ch->memory )
	free_mem_data( ch->memory );

    if ( ch->pcdata )
    {
	free_pcdata( ch->pcdata );
	rem_pc_list( ch );
    }

    free_action( ch );
        
    free_string( ch->name               );
    free_string( ch->short_descr        );
    free_string( ch->long_descr         );
    free_string( ch->description        );
    free_string( ch->prompt		);

    ch->next         = char_free;
    char_free        = ch;
    return;
}

PC_DATA *new_pcdata( void )
{
    PC_DATA *pcdata;
    static PC_DATA pcdata_zero;
    int alias;

    if ( pcdata_free == NULL )  
    {
	wiznet( "Allocating new pc data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	pcdata				= alloc_perm( sizeof(*pcdata) );
    }
    else
    {
        pcdata				= pcdata_free;
        pcdata_free			= pcdata_free->next;
    }

    *pcdata				= pcdata_zero;

    for ( alias = 0; alias < MAX_ALIAS; alias++ )
    {
	pcdata->alias[alias]	= NULL;
	pcdata->alias_sub[alias]= NULL;
    }
    if ( pcdata->guild )
	free_char_guild( pcdata->guild );

    pcdata->buffer	= new_buf();

    return( pcdata );
}

void free_pcdata( PC_DATA *pcdata )
{
    int alias;

    free_string( pcdata->last_name		);
    free_string( pcdata->pwd			);
    free_string( pcdata->transin		);
    free_string( pcdata->transout		);
    free_string( pcdata->bamfin			);
    free_string( pcdata->bamfout		);
    free_string( pcdata->title			);
    free_string( pcdata->spouse			);
    free_string( pcdata->afk_message		);
    free_string( pcdata->new_name		);
    free_string( pcdata->new_last		);
    free_string( pcdata->new_title		);
    free_string( pcdata->new_desc		);
    free_string( pcdata->sedai			);
    free_string( pcdata->shadow_name		);
    free_string( pcdata->doing			);
    free_string( pcdata->wearing		);
    free_string( pcdata->email			);

    free_buf(pcdata->buffer);
    if ( pcdata->guild )
	free_char_guild( pcdata->guild );

    for ( alias = 0; alias < MAX_ALIAS; alias++ )
    {
	free_string( pcdata->alias[alias]	);
	free_string( pcdata->alias_sub[alias]	);
    }

    pcdata->next = pcdata_free;
    pcdata_free      = pcdata;
}


OBJ_DATA *new_obj( void )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    
    if (obj_free == NULL)
    {
	wiznet( "Allocating new object.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        obj = alloc_perm(sizeof(*obj));
    }
    else
    {
        obj = obj_free;
        obj_free = obj_free->next;
    }
    *obj = obj_zero;

    obj->owned			= &str_empty[0];
    obj->name			= str_dup( "" );
    obj->short_descr		= str_dup( "" );
    obj->description		= str_dup( "" );

    return obj;
}

void free_obj(OBJ_DATA *obj)
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;
    
    for (paf = obj->affected; paf != NULL; paf = paf_next)
    {
        paf_next = paf->next;
        free_affect( paf );
    }
    obj->affected = NULL;
    
    for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
    {
        ed_next = ed->next; 
        free_extra_descr( ed );
     }
     obj->extra_descr = NULL;
    
    free_string( obj->name		);
    free_string( obj->description	);
    free_string( obj->short_descr	);
    free_string( obj->owned		);
        
    obj->next   = obj_free;
    obj_free    = obj;
}

DESCRIPTOR_DATA *new_descriptor(void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;
    
    if (descriptor_free == NULL)
    {
	wiznet( "Allocating new descriptor.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        d = alloc_perm(sizeof(*d));
    }
    else
    {
        d = descriptor_free;
        descriptor_free = descriptor_free->next;
    }
 
    *d = d_zero;
    d->host			= &str_empty[0];
    d->outbuf			= &str_empty[0];
    d->showstr_head		= &str_empty[0];
    d->showstr_point		= &str_empty[0];
    d->ident			= &str_empty[0];
    return d;
}    

void free_descriptor(DESCRIPTOR_DATA *d)
{
    free_string( d->host );
    free_string( d->ident );
    free_mem( d->outbuf, d->outsize );
    d->next = descriptor_free;
    descriptor_free = d;
}

BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;
    
    if (ban_free == NULL)
    {
	wiznet( "Allocating new ban data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        ban = alloc_perm(sizeof(*ban));
    }
    else
    {
        ban = ban_free;
        ban_free = ban_free->next;
    }

    *ban = ban_zero;
    ban->user = &str_empty[0];
    ban->host = &str_empty[0];
    return ban;
}
 
void free_ban(BAN_DATA *ban) 
{
    free_string(ban->user);
    free_string(ban->host);
        
    ban->next = ban_free;
    ban_free = ban;
}
    

TEXT_DATA *new_text(void)
{
    static TEXT_DATA text_zero;
    TEXT_DATA *text;

    if (text_free == NULL)
    {
	wiznet( "Allocating text data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	text = alloc_perm(sizeof(*text));
    }
    else
    {
        text = text_free;
        text_free = text_free->next;
    }

    *text = text_zero;
    text->keyword		= &str_empty[0];
    text->text			= &str_empty[0];
    return text;
}
 
void free_text(TEXT_DATA *text) 
{
    free_string(text->keyword);
    free_string(text->text);
        
    text->next = text_free;
    text_free = text;
}


GUARD_DATA *new_guard( void )
{
    static GUARD_DATA guard_zero;
    GUARD_DATA *guard;
    if (guard_free == NULL)
    {
	wiznet( "Allocating guard data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	guard = alloc_perm(sizeof(*guard));
	top_guard++;
    }
    else
    {
        guard = guard_free;
        guard_free = guard_free->next;
    }

    *guard = guard_zero;

    return guard;
}


void free_guard(GUARD_DATA *guard) 
{
    guard->next = guard_free;
    guard_free = guard;
}



/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

CHAR_DATA *get_pc_from_id( long id )
{
    NODE_DATA *node;
    CHAR_DATA *ch;

    for ( node = pc_list; node; node = node->next )
    {
	if ( node->data_type != NODE_PC )
	    continue;

	ch = (CHAR_DATA *) node->data;
	if ( ch->id == id )
	    return ch;
    }
    return NULL;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
    MEM_DATA *memory;
  
    if (mem_data_free == NULL)
	memory = alloc_mem(sizeof(*memory));
    else
    {
	memory = mem_data_free;
	mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;

    return memory;
}

void free_mem_data(MEM_DATA *memory)
{
    memory->next = mem_data_free;
    mem_data_free = memory;
}



/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
	if (buf_size[i] >= val)
	{
	    return buf_size[i];
	}
    
    return -1;
}

BUFFER *new_buf()
{
    BUFFER *buffer;

    if (buf_free == NULL) 
    {
	wiznet( "Allocating new buffer.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
	buffer = alloc_perm(sizeof(*buffer));
    }
    else
    {
	buffer = buf_free;
	buf_free = buf_free->next;
    }

    buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
    buffer->size	= get_size(BASE_BUF);

    buffer->string	= alloc_mem(buffer->size);
    buffer->string[0]	= '\0';

    return buffer;
}

BUFFER *new_buf_size(int size)
{
    BUFFER *buffer;
 
    if (buf_free == NULL)
    {
	wiznet( "Allocating new buffer.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        buffer = alloc_perm(sizeof(*buffer));
    }
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }
 
    buffer->next        = NULL;
    buffer->state       = BUFFER_SAFE;
    buffer->size        = get_size(size);
    if (buffer->size == -1)
    {
        bug("new_buf: buffer size %d too large.",size);
        exit(1);
    }
    buffer->string      = alloc_mem(buffer->size);
    buffer->string[0]   = '\0';
 
    return buffer;
}


void free_buf(BUFFER *buffer)
{
    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;

    buffer->next  = buf_free;
    buf_free      = buffer;
}


bool add_buf(BUFFER *buffer, char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
	return FALSE;

    len = strlen(buffer->string) + strlen(string) + 1;

    while (len >= buffer->size) /* increase the buffer size */
    {
	buffer->size 	= get_size(buffer->size + 1);
	{
	    if (buffer->size == -1) /* overflow */
	    {
		buffer->size = oldsize;
		buffer->state = BUFFER_OVERFLOW;
		bug("buffer overflow past size %d",buffer->size);
		return FALSE;
	    }
  	}
    }

    if (buffer->size != oldsize)
    {
	buffer->string	= alloc_mem(buffer->size);

	strcpy(buffer->string,oldstr);
	free_mem(oldstr,oldsize);
    }

    strcat(buffer->string,string);
    return TRUE;
}


void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}

GUILD *new_char_guild( )
{
    GUILD *pGuild;

    if ( !char_guild_free )
    {
	wiznet( "Allocating new player guild data.\r\n", NULL, NULL,
	    WIZ_MEMORY, 0, 0 );
        pGuild			=	alloc_perm( sizeof(*pGuild) );
    }
    else
    {
        pGuild			=	char_guild_free;
        char_guild_free		=	char_guild_free->next;
    }

    pGuild->next		= NULL;
    pGuild->warder		= &str_empty[0];
    pGuild->damane_name		= &str_empty[0];
    pGuild->damane		= NULL;
    pGuild->rank		= 0;

    return pGuild;
}

void free_char_guild( GUILD *pGuild )
{
    free_string( pGuild->warder	);
    free_string( pGuild->damane_name	);
    pGuild->damane	= NULL;
    pGuild->next	= char_guild_free;
    char_guild_free	= pGuild;
}

void add_guild( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) )
	return;
    if ( ch->pcdata->guild )
	return;
    ch->pcdata->guild = new_char_guild( );
    return;
}

void new_action( CHAR_DATA *ch, DO_FUN *func, int time, char *args, CHAR_DATA *victim )
{
    ch->action		= func;
    ch->action_timer	= time;
    ch->action_args	= str_dup( args );
    ch->action_target	= victim;
    return;
}

void free_action( CHAR_DATA *ch )
{
    ch->action		= NULL;
    ch->action_timer	= 0;
    ch->action_target	= NULL;
    free_string( ch->action_args );
    return;
}

