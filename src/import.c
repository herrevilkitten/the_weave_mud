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
/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


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
#include <ctype.h>

#include "merc.h"
#include "interp.h"
#include "mem.h"

#define DB_BOOT_WLD     0
#define DB_BOOT_MOB     1
#define DB_BOOT_OBJ     2
#define DB_BOOT_ZON     3
#define DB_BOOT_SHP     4
#define DB_BOOT_HLP     5

#define INDEX_FILE      "index"         /* index of world files         */ 
#define MINDEX_FILE     "index.mini"    /* ... and for mini-mud-mode    */ 
#define WLD_PREFIX      "world/wld"     /* room definitions             */
#define MOB_PREFIX      "world/mob"     /* monster prototypes           */
#define OBJ_PREFIX      "world/obj"     /* object prototypes            */
#define ZON_PREFIX      "world/zon"     /* zon defs & command tables    */
#define SHP_PREFIX      "world/shp"     /* shop definitions             */
#define HLP_PREFIX      "text/help"     /* for HELP <keyword>           */

char *parse_object(FILE * obj_f, int nr);
void assign_area_vnum( int vnum );
void load_zones(FILE * fl, char *zonename);
void parse_mobile(FILE * mob_f, int nr);
void parse_simple_mob(MOB_INDEX_DATA *pMobIndex, FILE *mob_f, int nr);
void parse_enhanced_mob(MOB_INDEX_DATA *pMobIndex, FILE *mob_f, int nr);

extern		int	newmobs;
int			min_room;
int			max_room;

void     fix_exits_area( AREA_DATA *pArea )
{
    extern const sh_int rev_dir [];
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int iHash;
    int door;
                    
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
              pRoomIndex != NULL;
              pRoomIndex  = pRoomIndex->next )
        {
            bool fexit;
                
            fexit = FALSE;

	    if ( pRoomIndex->area != pArea )
		continue;

            for ( door = 0; door <= 5; door++ )
            {
                if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
                {
                    if ( pexit->u1.vnum <= 0
                    || get_room_index(pexit->u1.vnum) == NULL)
                        pexit->u1.to_room = NULL;
                    else
                    {
                        fexit = TRUE;
                        pexit->u1.to_room = get_room_index( pexit->u1.vnum );
                    }
                }
            }
            if (!fexit)
                SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
        }
    }
             
    return;
}


int convert_room_flag( int flag )
{
    int new_flag;

    new_flag = flag;
    if ( flag && B )
        REMOVE_BIT( new_flag, B );  
    if ( flag && E )
    {
        REMOVE_BIT( new_flag, E );
        SET_BIT( new_flag, K );
        SET_BIT( new_flag, S );
    }
    if ( flag && F )
    {
        REMOVE_BIT( new_flag, F );
        SET_BIT( new_flag, ee );
    }
    if ( flag && G )
        REMOVE_BIT( new_flag, G );
    if ( flag && H )
        REMOVE_BIT( new_flag, H );
    if ( flag && I )
    {
        REMOVE_BIT( new_flag, I );
        SET_BIT( new_flag, J );
    }
    if ( flag && J )
    {
        REMOVE_BIT( new_flag, I );
        SET_BIT( new_flag, N ); 
    }
    if ( flag && K )
    {
        REMOVE_BIT( new_flag, K );
        SET_BIT( new_flag, P );
    }
    if ( flag && L )
        REMOVE_BIT( new_flag, L );
    if ( flag && M )
        REMOVE_BIT( new_flag, M );
    if ( flag && N )
        REMOVE_BIT( new_flag, N );
    if ( flag && O )
        REMOVE_BIT( new_flag, O );
    if ( flag && P )
        REMOVE_BIT( new_flag, P );
    return new_flag;
}

int convert_mob_act( int flag )
{
    int new_flag;

    new_flag = flag;
    if ( flag && A )
	REMOVE_BIT(new_flag, A);
    if ( flag && C )
	REMOVE_BIT(new_flag, A);
    if ( flag && D )
	REMOVE_BIT(new_flag, A);
    if ( flag && E )
	REMOVE_BIT(new_flag, A);
    if ( flag && I )
	REMOVE_BIT(new_flag, I);
    if ( flag && J )
	REMOVE_BIT(new_flag, J);
    if ( flag && K )
	REMOVE_BIT(new_flag, K);
    if ( flag && L )
    {
	REMOVE_BIT(new_flag, L);
	SET_BIT(new_flag, N);
    }
    if ( flag && M )
	REMOVE_BIT(new_flag, M);
    if ( flag && N )
	REMOVE_BIT(new_flag, N);
    if ( flag && O )
	REMOVE_BIT(new_flag, O);
    if ( flag && P )
	REMOVE_BIT(new_flag, P);
    if ( flag && Q )
	REMOVE_BIT(new_flag, Q);
    if ( flag && R )
	REMOVE_BIT(new_flag, R);

    SET_BIT(new_flag, A);
    return new_flag;
}    

int convert_mob_aff( int flag )
{
    int new_flag;

    new_flag = flag;
    if ( flag && C )
	REMOVE_BIT(new_flag, C);
    if ( flag && D )
	REMOVE_BIT(new_flag, D);
    if ( flag && E )
	REMOVE_BIT(new_flag, E);
    if ( flag && G )
	REMOVE_BIT(new_flag, G);
    if ( flag && I )
	REMOVE_BIT(new_flag, I);
    if ( flag && J )
	REMOVE_BIT(new_flag, J);
    if ( flag && K )
	REMOVE_BIT(new_flag, K);
    if ( flag && L )
    {
	REMOVE_BIT(new_flag, I);
	SET_BIT(new_flag, M);
    }
    if ( flag && M )
	REMOVE_BIT(new_flag, M);
    if ( flag && N )
	REMOVE_BIT(new_flag, N);
    if ( flag && O )
    {
	REMOVE_BIT(new_flag, O);
	SET_BIT(new_flag, R);
    }
    if ( flag && P )
	REMOVE_BIT(new_flag, P);
    if ( flag && Q )
	REMOVE_BIT(new_flag, Q);
    if ( flag && R )
	REMOVE_BIT(new_flag, R);
    if ( flag && S )
    {
	REMOVE_BIT(new_flag, S);
	SET_BIT(new_flag, P);
    }
    if ( flag && T )
    {
	REMOVE_BIT(new_flag, T);
	SET_BIT(new_flag, Q);
    }
    if ( flag && U )
	REMOVE_BIT(new_flag, U);
    if ( flag && V )
	REMOVE_BIT(new_flag, V);

    return new_flag;
}

int convert_item_type( int flag )
{
    if ( flag == 3 )
	return ITEM_TRASH;
    if ( flag == 4 )
	return ITEM_TRASH;
    if ( flag == 6 )
	return ITEM_TRASH;
    if ( flag == 7 )
	return ITEM_TRASH;
    if ( flag == 12 )
	return ITEM_TRASH;
    if ( flag == 14 )
	return ITEM_TRASH;
    if ( flag == 16 )
	return ITEM_TRASH;
    if ( flag == 21 )
	return ITEM_TRASH;
    if ( flag == 23 )
	return ITEM_FOUNTAIN;

    return flag;
}

long asciiflag_conv(char *flag)
{
  long flags = 0;
  int is_number = 1;
  register char *p;
          
  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
      flags |= 1 << (26 + (*p - 'A'));
      
    if (!isdigit(*p))
      is_number = 0;
  }
          
  if (is_number)
    flags = atol(flag);
 
  return flags;  
}


long flag_circle(char letter )
{    
    long bitsum = 0;  
    char i;
        
    if ('a' <= letter && letter <= 'z')
    {
        bitsum = 1;
        for (i = letter; i > 'a'; i--)
            bitsum *= 2;
    }    
    else if ('A' <= letter && letter <= 'Z')
    {
        bitsum = 67108864; /* 2^26 */
        for (i = letter; i > 'A'; i --)
            bitsum *= 2;
    }   

    return bitsum;
}
    
long fread_circleflag( FILE *fp )
{
    long number;
    char c;

    do
    {
	c = getc( fp );
    }
    while( isspace(c) );

    number = 0;

    if ( !isdigit(c) && c != '-' )
    {
	while ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
	{
	    number += flag_circle(c);
	    c = getc( fp );
	}
    }

    if ( c == '-' )
    {
	number = fread_number( fp );
	return -number;
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c = getc( fp );
    }

    if ( c == '|' )
	number += fread_circleflag( fp );

    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/* load the rooms */
void parse_room(FILE * fl, int vnum)
{
    ROOM_INDEX_DATA *pRoomIndex;
    char letter;
    int door;
    int iHash;

    if ( area_last == NULL )
    {
	bug( "Parse_room: No area seen yet.", 0 );
	exit( 0 );
    }

    pRoomIndex 			= new_room_index( );
    pRoomIndex->people		= NULL;
    pRoomIndex->contents	= NULL;
    pRoomIndex->extra_descr	= NULL;
    pRoomIndex->area		= area_last;
    pRoomIndex->vnum		= vnum;

    pRoomIndex->name		= fread_string( fl );
    pRoomIndex->description	= fread_string( fl );
    				  fread_number( fl );
    pRoomIndex->room_flags	= convert_room_flag( fread_circleflag(fl) );
    pRoomIndex->sector_type	= fread_number( fl );

    for ( door = 0; door < 6; door++ )
	pRoomIndex->exit[door]	= NULL;
    pRoomIndex->resources		= 0;

    for ( ; ; )
    {
	letter = fread_letter( fl );
	
	if ( letter == 'S' )
	    break;

	if ( letter == 'D' )
	{
	    EXIT_DATA *pExit;
	    int locks;

	    door = fread_number( fl );
	    if ( door < 0 || door > 5 )
	    {
		bug( "Parse_room: vnum %d has bad door number.", vnum );
		exit( 1 );
	    }

	    pExit 			= new_exit( );
	    pExit->description		= fread_string( fl );
	    pExit->keyword		= fread_string( fl );
	    pExit->exit_info		= 0;
	    pExit->rs_flags		= 0;

	    locks			= fread_number( fl );
	    pExit->key			= fread_number( fl );
	    pExit->u1.vnum		= fread_number( fl );
	    pExit->orig_door		= door;

	    if ( locks == 1 )
		SET_BIT(pExit->rs_flags, EX_ISDOOR);
	    else if ( locks == 2 )
	    {
		SET_BIT(pExit->rs_flags, EX_ISDOOR);
		SET_BIT(pExit->rs_flags, EX_PICKPROOF);
	    }

	    pRoomIndex->exit[door]	= pExit;
	}
	else if ( letter == 'E' )
	{
	    EXTRA_DESCR_DATA *ed;

	    ed				= new_extra_descr( );
	    ed->keyword			= fread_string( fl );
	    ed->description		= fread_string( fl );
	    ed->next			= pRoomIndex->extra_descr;
	    pRoomIndex->extra_descr	= ed;
	    top_ed++;
	}
	else
	{
	    bug( "Parse_room: vnum %d has flag not 'DES'.", vnum );
	    exit( 1 );
	}
    }
    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next		= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_room++;
    top_vnum_room = UMAX( top_vnum_room, vnum );
}


void discrete_load(FILE * fl, int mode)
{
    int nr = -1, last = 0;
    char line[256];

    for (;;)
    {
	char letter;

	do
	{
	    letter = getc( fl );
	}
	while( isspace(letter) );
    	ungetc( letter, fl );

	if ( (letter = getc( fl )) == EOF )
	{
	    fprintf( stderr, "EOF after %d.\n", nr );
	    exit(1);
	}
	if (letter == '$')
	    return;
      
	if (letter == '#')
	{
	    last = nr;
	    nr = fread_number( fl );

	    if (nr >= 99999)
        	return;
	    else
		switch (mode)
		{
		    case DB_BOOT_WLD:
			if ( nr < min_room )
			    min_room = nr;
			if ( nr > max_room )
			    max_room = nr;
			parse_room(fl, nr);
			break;
		    case DB_BOOT_MOB:
			parse_mobile(fl, nr);
			break;
		    case DB_BOOT_OBJ:
			strcpy(line, parse_object(fl, nr));
			break;
		}
	}
	else
	{
	    fprintf(stderr, "Format error infile near #%d\n",nr);
	    fprintf(stderr, "Offending line: '%s'\n", line);
	    exit(1); 
	}
    }
}
          
          

void index_boot( int mode, char *filename )
{
    char tmpname[MAX_STRING_LENGTH];
    FILE *fp;

    switch( mode )
    {
	case DB_BOOT_WLD:
	    sprintf( tmpname, "%s.wld", filename );
	    break;
	case DB_BOOT_OBJ:
	    sprintf( tmpname, "%s.obj", filename );
	    break;
	case DB_BOOT_MOB:
	    sprintf( tmpname, "%s.mob", filename );
	    break;
	case DB_BOOT_ZON:
	    sprintf( tmpname, "%s.zon", filename );
	    break;
    }
   
    if ( (fp = fopen(tmpname, "r")) == NULL )
    {
	bug( "Index_boot: Invalid filename.", 0 );
	exit(1);
    }

    switch( mode )
    {
	case DB_BOOT_WLD:
	case DB_BOOT_OBJ:
	case DB_BOOT_MOB:
	    discrete_load( fp, mode );
	    break;
	case DB_BOOT_ZON:
	    load_zones( fp, filename );
	    break;
    }
    fclose( fp );
}


void import_world( char *filename )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "Attempting to import area.  Reading %s.zon", filename );
    log_string( buf );
    index_boot( DB_BOOT_ZON, filename );

    sprintf( buf, "Attempting to import rooms.  Reading %s.wld", filename );
    log_string( buf );
    index_boot( DB_BOOT_WLD, filename );

    sprintf( buf, "Attempting to import mobs.  Reading %s.mob", filename );
    log_string( buf );
    index_boot( DB_BOOT_MOB, filename );
   
    sprintf( buf, "Attempting to import objects.  Reading %s.obj", filename );
    log_string( buf );
    index_boot( DB_BOOT_OBJ, filename );

    return;
}

void load_zones(FILE * fl, char *zonename)
{
    char temp[MAX_STRING_LENGTH];
    AREA_DATA *pArea;

    sprintf( temp, "%s.are", zonename );
    pArea			= new_area( );
    pArea->age			= 15;
    pArea->nplayer		= 0;
    pArea->empty		= FALSE;
    pArea->filename		= str_dup( temp );
    pArea->vnum			= top_area;
    pArea->builders		= str_dup( "" );
    pArea->security		= 9;
    pArea->lvnum		= 0;
    pArea->uvnum		= 0;
    pArea->area_flags		= 0;

    getc( fl );
    fread_number( fl );
    pArea->name			= fread_string( fl );

    fread_number( fl );
    fread_number( fl );
    fread_number( fl );

    if ( area_first == NULL )
	area_first 		= pArea;
    if ( area_last != NULL )
	area_last->next 	= pArea;
    area_last 			= pArea;
    pArea->next 		= NULL;
    return;
}

/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, int nr)
{
    char buf[MAX_STRING_LENGTH];
  char *tmpptr;  
  OBJ_INDEX_DATA *nObj;

  nObj = new_obj_index( );
  nObj->vnum	= nr;


  /* *** string data *** */
  if ( (nObj->name = fread_string(obj_f)) == NULL )
  {
    fprintf(stderr, "Null obj name or format\n");
    exit(1);     
  }

  nObj->short_descr = fread_string(obj_f);
  nObj->description = fread_string(obj_f);
  
  tmpptr = fread_string(obj_f);
  free_string( tmpptr );
  
  nObj->item_type	= convert_item_type( fread_number(obj_f) );
			  fread_circleflag( obj_f );
  nObj->extra_flags	= 0;
  nObj->wear_flags	= fread_circleflag( obj_f );

  nObj->value[0]= fread_number( obj_f );    
  nObj->value[1]= fread_number( obj_f );    
  nObj->value[2]= fread_number( obj_f );    
  nObj->value[3]= fread_number( obj_f );    

  nObj->weight	= fread_number( obj_f );
  nObj->cost	= fread_number( obj_f );
		  fread_number( obj_f );

  if ( nObj->item_type == ITEM_DRINK_CON
  ||   nObj->item_type == ITEM_FOUNTAIN )
    if ( nObj->weight < nObj->value[1] )
	nObj->weight = nObj->value[1] + 5;
        
  for ( ; ; )
  {
    char letter;
    letter = fread_letter( obj_f );

    if ( letter == 'A' )
    {
	AFFECT_DATA *paf;

	paf		= new_affect();
	paf->type	= -1;
	paf->strength	= nObj->level;
	paf->duration	= -1;
	paf->location	= fread_number( obj_f );
	paf->modifier	= fread_number( obj_f );
	paf->bitvector	= 0;
	paf->bitvector_2= 0;
	paf->flags	= AFFECT_NOTCHANNEL;
	paf->owner	= NULL;
	if ( (paf->location == APPLY_HITROLL
	||    paf->location == APPLY_DAMROLL)
	&&    nObj->item_type != ITEM_WEAPON )
	    free_affect( paf );
	else
	{
	    paf->next		= nObj->affected;
	    nObj->affected	= paf;
	}
    }

    else if ( letter == 'E' )      
    {
	EXTRA_DESCR_DATA *ed;
	ed		= new_extra_descr();
	ed->keyword	= fread_string( obj_f );
	ed->description	= fread_string( obj_f );
	ed->next	= nObj->extra_descr;
	nObj->extra_descr= ed;
	top_ed++;
    }
    else if ( letter == '$' )
    {
	sprintf( buf, "Done with object %d", nr );
	log_string( buf );
	return "";
    }
    else
    {
	sprintf( buf, "Done with object %d", nr );
	log_string( buf );
      ungetc( letter, obj_f );
      return "";
    }
  }
}


void parse_mobile(FILE * mob_f, int nr)
{
    MOB_INDEX_DATA *pMobIndex;
    int iHash;
    char letter;

    if ( !area_last )
    {
	bug( "Parse_mobile: no area seen yet.", 0 );
	exit( 1 );
    }

    if ( get_mob_index(nr) )
    {
	bug( "Parse_mobile: vnum %d duplicated.", nr );
	exit(1);
    }

    pMobIndex			= new_mob_index( );
    pMobIndex->vnum		= nr;
    pMobIndex->area		= area_last;
    pMobIndex->new_format	= TRUE;
    newmobs++;

    /***** String data *** */
    pMobIndex->player_name	= fread_string( mob_f );
    pMobIndex->short_descr	= fread_string( mob_f );
    pMobIndex->long_descr	= fread_string( mob_f );
    pMobIndex->description	= fread_string( mob_f );

    pMobIndex->race		= race_lookup( "human" );

    pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
    pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

    pMobIndex->act		= convert_mob_act( fread_circleflag(mob_f) );
    pMobIndex->affected_by	= convert_mob_aff( fread_circleflag(mob_f) );
    				  fread_number( mob_f );
    
    letter			= fread_letter( mob_f );  

    switch (letter)
    {
	case 'S':
	case 's':
	    parse_simple_mob( pMobIndex, mob_f, nr );
	    break;
	case 'E':
	case 'e':
	    parse_enhanced_mob( pMobIndex, mob_f, nr );
	    break;
	default:
	    fprintf(stderr, "Unsupported mob type '%c' in mob #%d\n", letter, nr);
	    exit(1);
	    break;
    }

    pMobIndex->pShop		= NULL;
    iHash			= nr % MAX_KEY_HASH;
    pMobIndex->next		= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMobIndex;
    top_vnum_mob		= UMAX( top_vnum_mob, nr );
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    return;
}

void parse_enhanced_mob(MOB_INDEX_DATA *pMobIndex, FILE *mob_f, int nr)
{
    char *line;
    parse_simple_mob(pMobIndex, mob_f, nr);

    while ( (line = fread_string( mob_f )) )
    {
	if (!strcmp(line, "E"))     /* end of the ehanced section */
	{
	    free_string( line );
	    return;
	}
	else if (*line == '#')
	{    /* we've hit the next mob, maybe? */
	    fprintf(stderr, "Unterminated E section in mob #%d\n", nr);
	    exit(1);
	}
	else
	    free_string( line );
    }
    fprintf(stderr, "Unexpected end of file reached after mob #%d\n", nr);
    exit(1);
}   

void parse_simple_mob(MOB_INDEX_DATA *pMobIndex, FILE *mob_f, int nr)
{
    pMobIndex->level			= fread_number( mob_f );
    pMobIndex->hitroll			= fread_number( mob_f );
					  fread_number( mob_f );
					  fread_word( mob_f );
					  fread_number( mob_f );
					  fread_word( mob_f );

    pMobIndex->gold			= fread_number( mob_f );
					  fread_number( mob_f );

    pMobIndex->start_pos		= fread_number( mob_f );
    pMobIndex->default_pos		= fread_number( mob_f );
    pMobIndex->sex			= fread_number( mob_f );

    pMobIndex->off_flags		= race_table[pMobIndex->race].off;
    pMobIndex->res_flags		= race_table[pMobIndex->race].res;
    pMobIndex->vuln_flags		= race_table[pMobIndex->race].vuln;

    pMobIndex->form			= race_table[pMobIndex->race].form;
    pMobIndex->parts			= race_table[pMobIndex->race].parts;
    return;
}
    
    

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#undef CASE
    

void do_import( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    send_to_char_new( ch, "Attempting to import: %s\r\n", arg );

    min_room = 99999;
    max_room = 0;
    import_world( arg );
    area_last->uvnum = max_room;
    area_last->lvnum = min_room;
    fix_exits_area( area_last );
    return;
}
