/* db_new.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "merc.h"
#include "db.h"
#include "mem.h"


/* values for db2.c */
struct 			social_type	social_table		[MAX_SOCIALS];
int					social_count		= 0;

/* snarf a socials file */
void load_socials( FILE *fp)
{
    for ( ; ; ) 
    {
    	struct social_type social;
    	char *temp;
        /* clear social */
	social.char_no_arg = NULL;
	social.others_no_arg = NULL;
	social.char_found = NULL;
	social.others_found = NULL;
	social.vict_found = NULL; 
	social.char_not_found = NULL;
	social.char_auto = NULL;
	social.others_auto = NULL;

    	temp = fread_word(fp);
    	if (!strcmp(temp,"#0"))
	    return;  /* done */

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.char_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
	    social.char_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_found = temp; 

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.vict_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_not_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_auto = temp; 
	
	social_table[social_count] = social;
    	social_count++;
   }
   return;
}
    





/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    
    if ( !area_last )   /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter,temp;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pMobIndex                       = new_mob_index();
        pMobIndex->vnum                 = vnum;
	pMobIndex->area			= area_last; /* OLC */
	pMobIndex->new_format		= TRUE;
	newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
	pMobIndex->race		 	= race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;
        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
        pMobIndex->pShop                = NULL;
        letter                          = fread_letter( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
        /* 'd'          */                fread_letter( fp ); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
        /* '+'          */                fread_letter( fp );   
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

 	/* read mana dice */
	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= fread_number( fp );

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flag( fp ) 
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].imm;
	pMobIndex->res_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].res;
	pMobIndex->vuln_flags		= fread_flag( fp )
					| race_table[pMobIndex->race].vuln;

	/* vital statistics */
	pMobIndex->start_pos		= fread_number( fp );
	if ( pMobIndex->start_pos == POS_MOUNTED )
	    pMobIndex->start_pos = POS_STANDING;
	pMobIndex->default_pos		= fread_number( fp );
	if ( pMobIndex->default_pos == POS_MOUNTED )
	    pMobIndex->default_pos = POS_STANDING;
	pMobIndex->sex			= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );

	pMobIndex->form			= fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts		= fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	temp				= fread_letter( fp );
	switch (temp)
	{
	    case ('T') :		pMobIndex->size = SIZE_TINY;	break;
	    case ('S') :		pMobIndex->size = SIZE_SMALL;	break;
	    case ('M') :		pMobIndex->size = SIZE_MEDIUM;	break;
	    case ('L') :		pMobIndex->size = SIZE_LARGE; 	break;
	    case ('H') :		pMobIndex->size = SIZE_HUGE;	break;
	    case ('G') :		pMobIndex->size = SIZE_GIANT;	break;
	    default:			pMobIndex->size = SIZE_MEDIUM; break;
	}
	pMobIndex->guild		= guild_lookup(fread_word( fp ));
	pMobIndex->material		= material_lookup(fread_word( fp ));
        pMobIndex->max                  = fread_number( fp );

        if ( letter != 'S' )
        {
            bug( "Load_mobiles: vnum %d non-S.", vnum );
            exit( 1 );
        }

        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );
 
            if ( letter == 'G' )
	    {
		GUARD_DATA *guard;
		guard = new_guard( );
		guard->exit		= fread_flag( fp );
		guard->allow		= fread_flag( fp );
		guard->flag		= fread_flag( fp );
		guard->message		= fread_number( fp );
		guard->next		= pMobIndex->guard;
		pMobIndex->guard	= guard;
	    }
 
            else if ( letter == 'T' )
		pMobIndex->text	= fread_string( fp );
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
	top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
        assign_area_vnum( vnum );                                  /* OLC */
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    if ( !area_last )   /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        pObjIndex                       = new_obj_index();
        pObjIndex->vnum                 = vnum;
	pObjIndex->area                 = area_last;            /* OLC */
        pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num		= 0;
	newobjs++;
	pObjIndex->version		= fread_number( fp );
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material		= material_lookup(fread_string( fp ));
 
        pObjIndex->item_type            = fread_number( fp );
        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
        pObjIndex->value[0]             = fread_flag( fp );
        pObjIndex->value[1]             = fread_flag( fp );
        pObjIndex->value[2]             = fread_flag( fp );
        pObjIndex->value[3]             = fread_flag( fp );
	pObjIndex->value[4]		= fread_flag( fp );
        pObjIndex->value[5]		= fread_flag( fp );
        pObjIndex->value[6]		= fread_flag( fp );
	pObjIndex->value[7]		= fread_flag( fp );
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp ); 
	pObjIndex->qp			= fread_number( fp );

        /* condition */
        letter 				= fread_letter( fp );
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}

        pObjIndex->max                  = fread_number( fp );
 
        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );
 
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = new_affect();
                paf->type               = -1;
                paf->strength           = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->bitvector_2        = 0;
		paf->flags		= AFFECT_NOTCHANNEL;
                paf->owner              = NULL;
		if ( (paf->location == APPLY_HITROLL
		||    paf->location == APPLY_DAMROLL)
		&&    pObjIndex->item_type != ITEM_WEAPON )
		    free_affect( paf );
		else
		{
                    paf->next               = pObjIndex->affected;
                    pObjIndex->affected     = paf;
		}
            }
 
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = new_extra_descr();
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }

	if ( pObjIndex->item_type == ITEM_ARMOR )
	{
	    int i;
	    int min, max;

	    if ( pObjIndex->material == 0 )
		pObjIndex->material = MAT_IRON;

	    min = break_table[pObjIndex->material].armor_min;
	    max = break_table[pObjIndex->material].armor_max;

	    for ( i = 0; i < 3; i++ )
	    {
		if ( pObjIndex->value[i] < min
		||   pObjIndex->value[i] > max )
		    pObjIndex->value[i] = number_range( min, max );
	    }
	    if ( pObjIndex->value[3] >= max/2 )
		pObjIndex->value[3] = number_range(0, max/2);
	}

	if ( pObjIndex->item_type == ITEM_WEAPON )
	{
	    int v1, v2;

	    v1 = pObjIndex->value[1];
	    v2 = pObjIndex->value[2];

	    switch( pObjIndex->value[0] )
	    {
		case WEAPON_SWORD:
			if ( v1 * v2 > 30 || v2 != 10 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 10;
			}
			break;
		case WEAPON_MACE:
			if ( v1 * v2 > 18 || v2 != 6 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 6;
			}
			break;
		case WEAPON_DAGGER:
			if ( v1 * v2 > 9 || v2 != 3 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 3;
			}
			break;
		case WEAPON_AXE:
			if ( v1 * v2 > 36 || v2 != 12 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 12;
			}
			break;
		case WEAPON_SPEAR:
			if ( v1 * v2 > 18 || v2 != 6 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 6;
			}
			break;
		case WEAPON_FLAIL:
			if ( v1 * v2 > 24 || v2 != 8 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 8;
			}
			break;
		case WEAPON_WHIP:
			if ( v1 * v2 > 12 || v2 != 4 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 4;
			}
			break;
		case WEAPON_POLEARM:
			if ( v1 * v2 > 48 || v2 != 16 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 16;
			}
			break;
		case WEAPON_STAFF:
			if ( v1 * v2 > 21 || v2 != 7 )
			{
			    v1 = number_fuzzy( 2 );
			    v2 = 7;
			}
			break;
	    }
	    if ( !IS_SET(pObjIndex->extra_flags, ITEM_QUEST) )
	    {
		if ( is_guild_eq(pObjIndex->vnum) )
		{
		    v1 = 5;
		    v2 = 10;
		}
		else
		{
		    pObjIndex->value[1] = v1;
		    pObjIndex->value[2] = v2;
		}
	    }
	}
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
	top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum ); 
    }
 
    return;
}

/*****************************************************************************
 Name:          convert_objects
 Purpose:       Converts all old format objects to new format
 Called by:     boot_db (db.c).
 Note:          Loops over all resets to find the level of the mob
                loaded before the object to determine the level of
                the object.
                It might be better to update the levels in load_resets().
                This function is not pretty.. Sorry about that :)
 Author:        Hugin
 ****************************************************************************/
void convert_objects( void )
{
    int vnum;
    AREA_DATA  *pArea;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;

    if ( newobjs == top_obj_index ) return; /* all objects in new format */

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
        {
            if ( !( pRoom = get_room_index( vnum ) ) ) continue;

            for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
            {
                switch ( pReset->command )
                {
                case 'M':
                    if ( !( pMob = get_mob_index( pReset->arg1 ) ) )
                        bug( "Convert_objects: 'M': bad vnum %d.", pReset->arg1);
                    break;

                case 'O':
                    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                    {
                        bug( "Convert_objects: 'O': bad vnum %d.", pReset->arg1);
                        break;
                    }

                    if ( pObj->new_format )
                        continue;

                    if ( !pMob )
                    {
                        bug( "Convert_objects: 'O': No mob reset yet.", 0 );
                        break;
                    }

                    pObj->level = pObj->level < 1 ? pMob->level - 2
                        : UMIN(pObj->level, pMob->level - 2);
                    break;

                case 'P':
                    {
                        OBJ_INDEX_DATA *pObj, *pObjTo;

                        if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                        {
                            bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg1 );
                            break;
                        }

                        if ( pObj->new_format )
                            continue;

                        if ( !( pObjTo = get_obj_index( pReset->arg3 ) ) )
                        {
                            bug( "Convert_objects: 'P': bad vnum %d.", pReset->arg3 );
                            break;
                        }

                        pObj->level = pObj->level < 1 ? pObjTo->level
                            : UMIN(pObj->level, pObjTo->level);
                    }
                    break;

                case 'G':
                case 'E':
                    if ( !( pObj = get_obj_index( pReset->arg1 ) ) )
                    {
                        bug( "Convert_objects: 'E' or 'G': bad vnum %d.", pReset->arg1 );

                        break;
                    }

                    if ( !pMob )
                    {
                        bug( "Convert_objects: 'E' or 'G': null mob for vnum %d.",
                             pReset->arg1 );
                        break;
                    }

                    if ( pObj->new_format )
                        continue;

                    if ( pMob->pShop )
                    {
                        switch ( pObj->item_type )
                        {
                        default:
                            pObj->level = UMAX(0, pObj->level);
                            break;
                        case ITEM_POTION:
                            pObj->level = UMAX(5, pObj->level);
                            break;
                        case ITEM_ARMOR:
                        case ITEM_WEAPON:
                            pObj->level = UMAX(10, pObj->level);
                            break;
                        case ITEM_TREASURE:
                            pObj->level = UMAX(15, pObj->level);
                            break;
                        }
                    }
                    else
                        pObj->level = pObj->level < 1 ? pMob->level
                            : UMIN( pObj->level, pMob->level );
                    break;
                } /* switch ( pReset->command ) */
            }
        }

    /* do the conversion: */

    for ( pArea = area_first; pArea ; pArea = pArea->next )
        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
            if ( (pObj = get_obj_index( vnum )) )
                if ( !pObj->new_format )
                    convert_object( pObj );

    return;
}
}
void convert_object( OBJ_INDEX_DATA *pObjIndex )
{

    int level;
    int number, type;  /* for dice-conversion */

    if ( !pObjIndex || pObjIndex->new_format ) return;

    level = pObjIndex->level;

    pObjIndex->level    = UMAX( 0, pObjIndex->level ); /* just to be sure */
    pObjIndex->cost     = 10*level;

    switch ( pObjIndex->item_type )
    {
        default:
            bug( "Obj_convert: vnum %d bad type.", pObjIndex->item_type );
            break;

        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_WEAPON:

            /*
             * The conversion below is based on the values generated
             * in one_hit() (fight.c).  Since I don't want a lvl 50
             * weapon to do 15d3 damage, the min value will be below
             * the one in one_hit, and to make up for it, I've made
             * the max value higher.
             * (I don't want 15d2 because this will hardly ever roll
             * 15 or 30, it will only roll damage close to 23.
             * I can't do 4d8+11, because one_hit there is no dice-
             * bounus value to set...)
             *
             * The conversion below gives:

             level:   dice      min      max      mean
               1:     1d8      1( 2)    8( 7)     5( 5)
               2:     2d5      2( 3)   10( 8)     6( 6)
               3:     2d5      2( 3)   10( 8)     6( 6)
               5:     2d6      2( 3)   12(10)     7( 7)
              10:     4d5      4( 5)   20(14)    12(10)
              20:     5d5      5( 7)   25(21)    15(14)
              30:     5d7      5(10)   35(29)    20(20)
              50:     5d11     5(15)   55(44)    30(30)

             */

            number = UMIN(level/4 + 1, 5);
            type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
            break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
            break;

        case ITEM_POTION:
            break;

        case ITEM_MONEY:
            pObjIndex->value[0] = pObjIndex->cost;
            break;
    }

    pObjIndex->new_format = TRUE;
    ++newobjs;

    return;
}

/*****************************************************************************
 Name:          convert_mobile
 Purpose:       Converts an old_format mob into new_format
 Called by:     load_old_mob (db.c).
 Note:          Dug out of create_mobile (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_mobile( MOB_INDEX_DATA *pMobIndex )
{
    int i;
    int type, number, bonus;
    int level;

    if ( !pMobIndex || pMobIndex->new_format ) return;

    level = pMobIndex->level;

    pMobIndex->act              |= ACT_WARRIOR;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)

     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

       level:     dice         min         max        diff       mean
         1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
         2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
         3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
         5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
        10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
        15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
        30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
        50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

        The values in parenthesis give the values generated in create_mobile.
        Diff = max - min.  Mean is the arithmetic mean.
        (hmm.. must be some roundoff error in my calculations.. smurfette got
         1d6+23 hp at level 3 ? -- anyway.. the values above should be
         approximately right..)
     */
    type   = level*level*27/40;
    number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*(8 + level)*.9 - number*type);

    pMobIndex->hit[DICE_NUMBER]    = number;
    pMobIndex->hit[DICE_TYPE]      = type;
    pMobIndex->hit[DICE_BONUS]     = bonus;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */
    type   = level*7/4;
    number = UMIN(type/8 + 1, 5);
    type   = UMAX(2, type/number);
    bonus  = UMAX(0, level*9/4 - number*type);

    pMobIndex->damage[DICE_NUMBER] = number;
    pMobIndex->damage[DICE_TYPE]   = type;
    pMobIndex->damage[DICE_BONUS]  = bonus;

    switch ( number_range( 1, 3 ) )
    {
        case (1): pMobIndex->dam_type =  3;       break;  /* slash  */
        case (2): pMobIndex->dam_type =  7;       break;  /* pound  */
        case (3): pMobIndex->dam_type = 11;       break;  /* pierce */
    }

    for (i = 0; i < 3; i++)
        pMobIndex->ac[i]         = interpolate( level, 100, -100);
    pMobIndex->ac[3]             = interpolate( level, 100, 0);    /* exotic */

    pMobIndex->gold             /= 100;
    pMobIndex->size              = SIZE_MEDIUM;
    pMobIndex->material          = 0;

    pMobIndex->new_format        = TRUE;
    ++newmobs;

    return;
}

/*
 * Snarf a text section.
 */
void load_text( FILE *fp )
{
    TEXT_DATA *TextIndex;    

    for ( ; ; )
    {
        char letter;
        int iHash, vnum;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_text: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_text_index( vnum ) != NULL )
        {
            bug( "Load_text: id %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        TextIndex			= new_text();
        TextIndex->id			= vnum;
	TextIndex->keyword		= fread_string( fp );
	TextIndex->text			= fread_string( fp );

        iHash			= vnum % MAX_KEY_HASH;
        TextIndex->next		= text_hash[iHash];
        text_hash[iHash]	= TextIndex;
	top_text		= top_text < vnum ? vnum : top_text;
    } 
    return;
}
