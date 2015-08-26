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
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.					           *
 ***************************************************************************/
  
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

NODE_DATA	*new_node( );
void		free_node( NODE_DATA *node );

/*
 *  Go through the node lists for each and add/remove data
 *  Data types:
 *   -1 - Node is empty
 *    0 - PC list
 *    1 - Fight list
 *    2 - Affect list: Character
 *    3 - Affect list: Object
 *    4 - Affect list: Room
 *    5 - Affect list: Created objects
 *    7 - Spec_fun list: Character
 *    8 - Spec_fun list: Object
 *    9 - Spec_fun list: Room
 *   10 - Wait list
 *   11 - Room list
 */

void destroy_list( NODE_DATA *list )
{
    NODE_DATA *node, *node_next;
    for ( node = list; node != NULL; node = node_next )
    {
	node_next = node->next;
	free_node( node );
    }

    list = NULL;
}

bool list_contains( NODE_DATA *list, void *data, sh_int type )
{

    return FALSE;
}

/*
 * PC list functions
 */
void add_pc_list( CHAR_DATA *ch )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = pc_list; list != NULL; list = list->next )
    {
	CHAR_DATA *person;
	if ( list->data_type != NODE_PC )
	    continue;

	person = (CHAR_DATA *) list->data;
	if ( person == ch )
	    return;
    }

    node		= new_node( );
    node->next		= pc_list;
    node->data		= (void *) ch;
    node->data_type	= NODE_PC;
    pc_list		= node;

    return;
}       
    
void rem_pc_list( CHAR_DATA *ch )
{    
    NODE_DATA *list, *prev_list, *next_list;

    prev_list = NULL;
    for ( list = pc_list; list != NULL; list = next_list )
    {
	CHAR_DATA *person;
	next_list = list->next;

	person = (CHAR_DATA *) list->data;
	if ( list->data_type != NODE_PC )
	    continue;

	if ( person == ch )
	{
	    if ( prev_list == NULL )
	    {
		pc_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}


/*
 *  Fight list functions
 */
void add_fight_list( CHAR_DATA *ch )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = fight_list; list != NULL; list = list->next )
    {
	CHAR_DATA *person;

	if ( list->data_type != NODE_FIGHT )
	    continue;

	person = (CHAR_DATA *) list->data;
	if ( person == ch )
	    return;
    }

    node		= new_node( );
    node->next		= fight_list;
    node->data		= (void *) ch;
    node->data_type	= NODE_FIGHT;
    fight_list		= node;

    return;
}       
    
void rem_fight_list( CHAR_DATA *ch )
{    
    NODE_DATA *list, *prev_list, *next_list;

    prev_list = NULL;
    for ( list = fight_list; list != NULL; list = next_list )
    {
	CHAR_DATA *person;

	next_list = list->next;
	if ( list->data_type != NODE_FIGHT )
	    continue;

	person = (CHAR_DATA *) list->data;
	if ( person == ch )
	{
	    if ( prev_list == NULL )
	    {
		fight_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}


/*
 *  Weave list
 */
void add_weave_list( void *data, sh_int data_type )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = weave_list; list != NULL; list = list->next )
    {
	if ( data_type == NODE_WEAVE_CHAR
	&&   list->data_type == NODE_WEAVE_CHAR )
	{
	    CHAR_DATA *person, *list_person;

	    list_person = (CHAR_DATA *) list->data;
	    person	= (CHAR_DATA *) data;
	    if ( person == list_person )
		return;
	}

	if ( data_type == NODE_WEAVE_OBJ
	&&   list->data_type == NODE_WEAVE_OBJ )
	{
	    OBJ_DATA *object, *list_object;

	    list_object	= (OBJ_DATA *) list->data;
	    object	= (OBJ_DATA *) data;
	    if ( object == list_object )
		return;
	}

	if ( data_type == NODE_WEAVE_ROOM
	&&   list->data_type == NODE_WEAVE_ROOM )
	{
	    ROOM_INDEX_DATA *room, *list_room;

	    list_room	= (ROOM_INDEX_DATA *) list->data;
	    room	= (ROOM_INDEX_DATA *) data;
	    if ( room == list_room )
		return;
	}

	if ( data_type == NODE_WEAVE_CREATE
	&&   list->data_type == NODE_WEAVE_CREATE )
	{
	    OBJ_DATA *object, *list_object;

	    list_object	= (OBJ_DATA *) list->data;
	    object	= (OBJ_DATA *) data;
	    if ( object == list_object )
		return;
	}
    }

    node		= new_node( );
    node->next		= weave_list;
    node->data		= data;
    node->data_type	= data_type;
    weave_list		= node;

    return;
}       
    
void rem_weave_list( void *data, sh_int data_type )
{    
    NODE_DATA *list, *prev_list, *next_list;
    bool found;

    prev_list = NULL;
    for ( list = weave_list; list != NULL; list = next_list )
    {
	found = FALSE;
	next_list = list->next;

	if ( data_type == NODE_WEAVE_CHAR )
	{
	    CHAR_DATA *person;
	    CHAR_DATA *data_person;

	    person 	= (CHAR_DATA *) data;
	    data_person = (CHAR_DATA *) list->data;

	    if ( person == data_person )
		found = TRUE;
	}

	if ( data_type == NODE_WEAVE_OBJ )
	{
	    OBJ_DATA *object;
	    OBJ_DATA *data_object;

	    object 	= (OBJ_DATA *) data;
	    data_object = (OBJ_DATA *) list->data;

	    if ( object == data_object )
		found = TRUE;
	}

	if ( data_type == NODE_WEAVE_ROOM )
	{
	    ROOM_INDEX_DATA *room;
	    ROOM_INDEX_DATA *data_room;

	    room 	= (ROOM_INDEX_DATA *) data;
	    data_room	= (ROOM_INDEX_DATA *) list->data;

	    if ( room == data_room )
		found = TRUE;
	}

	if ( data_type == NODE_WEAVE_CREATE )
	{
	    OBJ_DATA *object;
	    OBJ_DATA *data_object;

	    object 	= (OBJ_DATA *) data;
	    data_object = (OBJ_DATA *) list->data;

	    if ( object == data_object )
		found = TRUE;
	}

	if ( found )
	{
	    if ( prev_list == NULL )
	    {
		weave_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}

/*
 * Wait list functions
 */
void add_wait_list( CHAR_DATA *ch )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = wait_list; list != NULL; list = list->next )
    {
	CHAR_DATA *person;
	if ( list->data_type != NODE_WAIT )
	    continue;

	person = (CHAR_DATA *) list->data;
	if ( person == ch )
	    return;
    }

    node		= new_node( );
    node->next		= wait_list;
    node->data		= (void *) ch;
    node->data_type	= NODE_WAIT;
    wait_list		= node;

    return;
}       
    
void rem_wait_list( CHAR_DATA *ch )
{    
    NODE_DATA *list, *prev_list, *next_list;

    prev_list = NULL;
    for ( list = wait_list; list != NULL; list = next_list )
    {
	CHAR_DATA *person;
	next_list = list->next;

	person = (CHAR_DATA *) list->data;
	if ( list->data_type != NODE_WAIT )
	    continue;

	if ( person == ch )
	{
	    if ( prev_list == NULL )
	    {
		wait_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}

void add_room_list( ROOM_INDEX_DATA *pRoom )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = room_list; list != NULL; list = list->next )
    {
	ROOM_INDEX_DATA *room;
	if ( list->data_type != NODE_ROOM )
	    continue;

	room = (ROOM_INDEX_DATA *) list->data;
	if ( pRoom == room )
	    return;
    }

    node		= new_node( );
    node->next		= room_list;
    node->data		= (void *) pRoom;
    node->data_type	= NODE_ROOM;
    room_list		= node;

    return;
}       
    
void rem_room_list( ROOM_INDEX_DATA *pRoom )
{    
    NODE_DATA *list, *prev_list, *next_list;

    prev_list = NULL;
    for ( list = room_list; list != NULL; list = next_list )
    {
	ROOM_INDEX_DATA *room;
	next_list = list->next;
	room = (ROOM_INDEX_DATA *) list->data;
	if ( list->data_type != NODE_ROOM )
	    continue;

	if ( room == pRoom )
	{
	    if ( prev_list == NULL )
	    {
		room_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}

/*
 *  Special functions list
 */
void add_spec_list( void *data, sh_int data_type )
{
    NODE_DATA *list;
    NODE_DATA *node;

    for ( list = spec_list; list != NULL; list = list->next )
    {
	if ( data_type == NODE_SPEC_CHAR
	&&   list->data_type == NODE_SPEC_CHAR )
	{
	    CHAR_DATA *person, *list_person;

	    list_person = (CHAR_DATA *) list->data;
	    person	= (CHAR_DATA *) data;
	    if ( person == list_person )
		return;
	}

	if ( data_type == NODE_SPEC_OBJ
	&&   list->data_type == NODE_SPEC_OBJ )
	{
	    OBJ_DATA *object, *list_object;

	    list_object	= (OBJ_DATA *) list->data;
	    object	= (OBJ_DATA *) data;
	    if ( object == list_object )
		return;
	}

	if ( data_type == NODE_SPEC_ROOM
	&&   list->data_type == NODE_SPEC_ROOM )
	{
	    ROOM_INDEX_DATA *room, *list_room;

	    list_room	= (ROOM_INDEX_DATA *) list->data;
	    room	= (ROOM_INDEX_DATA *) data;
	    if ( room == list_room )
		return;
	}
    }

    node		= new_node( );
    node->next		= spec_list;
    node->data		= data;
    node->data_type	= data_type;
    weave_list		= node;

    return;
}       
    
void rem_spec_list( void *data, sh_int data_type )
{    
    NODE_DATA *list, *prev_list, *next_list;
    bool found;

    prev_list = NULL;
    for ( list = spec_list; list != NULL; list = next_list )
    {
	found = FALSE;
	next_list = list->next;

	if ( data_type == NODE_SPEC_CHAR )
	{
	    CHAR_DATA *person;
	    CHAR_DATA *data_person;

	    person 	= (CHAR_DATA *) data;
	    data_person = (CHAR_DATA *) list->data;

	    if ( person == data_person )
		found = TRUE;
	}

	if ( data_type == NODE_SPEC_OBJ )
	{
	    OBJ_DATA *object;
	    OBJ_DATA *data_object;

	    object 	= (OBJ_DATA *) data;
	    data_object = (OBJ_DATA *) list->data;

	    if ( object == data_object )
		found = TRUE;
	}

	if ( data_type == NODE_SPEC_ROOM )
	{
	    ROOM_INDEX_DATA *room;
	    ROOM_INDEX_DATA *data_room;

	    room 	= (ROOM_INDEX_DATA *) data;
	    data_room	= (ROOM_INDEX_DATA *) list->data;

	    if ( room == data_room )
		found = TRUE;
	}

	if ( found )
	{
	    if ( prev_list == NULL )
	    {
		spec_list = list->next;
		free_node( list );
		return;
	    }
	    else
	    {
		prev_list->next = list->next;
		free_node( list );
		return;
	    }
	}
	prev_list = list;
    }
    return;
}


