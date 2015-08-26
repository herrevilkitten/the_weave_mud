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
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "mem.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

/*
#define	DEBUG_SAVE
*/

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch, FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj,
				FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch, FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch, FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch, FILE *fp ) );
int	flag_value	args( ( const struct flag_type *flag_table,
				char *argument) );
void	handedness	args( ( CHAR_DATA *ch ) );
void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	add_guild	args( ( CHAR_DATA *ch ) );
void	*value_from_file args( ( FILE *fp, char *match, int type ) );

DECLARE_DO_FUN( do_advance	);



char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];

         
    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }    

    if (pos == 0)
    {
        buf[pos] = '0'; 
        pos++;
    }    

    buf[pos] = '\0';
        
    return buf;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_HERO(ch) || ch->level >= LEVEL_HERO)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, correct_name(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

    fclose( fpReserve );
    sprintf( strsave, "%s%c/%s", PLAYER_DIR,
	LOWER(ch->name[0]), correct_name( ch->name ) );
    if ( ( fp = fopen( PLAYER_TEMP, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    /* move the file */
    sprintf(buf,"mv %s %s",PLAYER_TEMP,strsave);
    system(buf);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int tn, sn, gn, i;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %ld\n",	ch->id			);
    if ( !IS_NULLSTR(ch->pcdata->last_name) )
	fprintf( fp, "Last %s~\n", ch->pcdata->last_name );
    fprintf( fp, "Vers %d\n",   3			);
    if ( ch->desc && !IS_NULLSTR(ch->desc->ident) )
	fprintf( fp, "LastIdent %s~\n", ch->desc->ident );
    if ( ch->desc && !IS_NULLSTR(ch->desc->host) )
	fprintf( fp, "LastHost %s~\n", ch->desc->host );
    if ( ch->char_made )
	fprintf( fp, "Made 1\n" );
    if ( !IS_NULLSTR(ch->short_descr) )
      	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if ( !IS_NULLSTR(ch->long_descr) )
	fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if ( !IS_NULLSTR(ch->description) )
    	fprintf( fp, "Desc %s~\n",	ch->description	);
    if ( !IS_NULLSTR(ch->prompt) || !str_cmp(ch->prompt, "< %hhp %sst > ") )
	fprintf( fp, "Prom %s~\n", 	ch->prompt );
    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf( fp, "Age  %d\n",	ch->start_age		);
    fprintf( fp, "Cla  %d\n",	ch->class		);
    fprintf( fp, "Levl %d\n",	ch->level		);
    fprintf( fp, "MaxL %d\n",	ch->pcdata->max_level	);
    fprintf( fp, "Races '%s'\n",race_table[ch->race].name );
    if ( ch->trust != 0 )
	fprintf( fp, "Tru  %d\n",	ch->trust	);
    if ( ch->pcdata->security != 0 )
	fprintf( fp, "Sec  %d\n",    ch->pcdata->security   );      /* OLC */
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    fprintf( fp, "Note %ld\n",	ch->pcdata->last_note	);
    fprintf( fp, "Idea %ld\n",	ch->pcdata->last_idea	);
    fprintf( fp, "Chng %ld\n",	ch->pcdata->last_changes);
    fprintf( fp, "Pena %ld\n",	ch->pcdata->last_penalty);
    fprintf( fp, "News %ld\n",	ch->pcdata->last_news	);
    fprintf( fp, "Appl %ld\n",	ch->pcdata->last_apply	);
    fprintf( fp, "Scro %d\n", 	ch->lines		);
    fprintf( fp, "Room %d\n",
	(ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&& ch->was_in_room != NULL)
	    ? ch->was_in_room->vnum
	    : ch->in_room == NULL ? 504 : ch->in_room->vnum );

    fprintf( fp, "HS  %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->stamina, ch->max_stamina );
    if ( ch->gold > 0 )
	fprintf( fp, "Gold %ld\n",ch->gold		);
    else
	fprintf( fp, "Gold %d\n", 0			);
    if ( ch->bank > 0 )
	fprintf( fp, "Bank %ld\n",ch->bank		);
    else
	fprintf( fp, "Bank %d\n", 0			);
    if ( IS_GUILDED(ch) )
    {
	fprintf( fp, "Guild  '%s'\n", guild_name(ch->guild)	);
	fprintf( fp, "GuildR  %d\n",  ch->pcdata->guild->rank	);
	if ( !IS_NULLSTR(ch->pcdata->guild->warder) )
	    fprintf( fp, "Warder %s~\n", ch->pcdata->guild->warder );
	if ( !IS_NULLSTR(ch->pcdata->guild->damane_name) )
	    fprintf( fp, "Damname %s~\n", ch->pcdata->guild->damane_name );
    }
    fprintf( fp, "ShadR  %d\n", ch->pcdata->shadow_rank	);
    if ( !IS_NULLSTR(ch->pcdata->wearing) )
	fprintf( fp, "Wearing %s~\n", ch->pcdata->wearing );
    if ( !IS_NULLSTR(ch->pcdata->shadow_name) )
	fprintf( fp, "ShadN  %s~\n", ch->pcdata->shadow_name );
    if ( !IS_NULLSTR(ch->pcdata->sedai) )
	fprintf( fp, "Sedai %s~\n", ch->pcdata->sedai );
    fprintf( fp, "QP   %d\n",   ch->pcdata->qp		);
    fprintf( fp, "Exp  %d\n",	ch->exp			);
    if ( ch->act != 0 )
	fprintf( fp, "ActF  %s\n",	print_flags(ch->act)	);
    if ( ch->pcdata->config != 0 )
	fprintf( fp, "ConfigF %s\n",	print_flags(ch->pcdata->config) );

    if ( ch->affected_by != 0 )
    {
	int old_affect;
	old_affect = ch->affected_by;

	REMOVE_BIT( ch->affected_by, AFF_STALK );
	REMOVE_BIT( ch->affected_by, AFF_SHAPE_CHANGE );

	fprintf( fp, "AfByF %s\n",	print_flags(ch->affected_by)	);
	ch->affected_by = old_affect;
    }

    if (ch->affected_by_2 != 0)
    {
	int old_affect;
	old_affect = ch->affected_by_2;

	REMOVE_BIT( ch->affected_by_2, AFF_LINK );
	REMOVE_BIT( ch->affected_by_2, AFF_LEASHED );

	fprintf( fp, "AfB2F %s\n",	print_flags(ch->affected_by_2));
	ch->affected_by_2 = old_affect;
    }

    if ( ch->book != 0 )
	fprintf( fp, "BookF %s\n",   print_flags(ch->book));

    if ( IS_SET(ch->comm, COMM_AFK) )
    {
	REMOVE_BIT(ch->comm, COMM_AFK);
	fprintf( fp, "CommF %s\n",   print_flags(ch->comm)	);
	SET_BIT(ch->comm, COMM_AFK);
    }
    else
	fprintf( fp, "CommF %s\n",   print_flags(ch->comm)	);

    if (ch->wiznet != 0)
        fprintf( fp, "WiznF %s\n",   print_flags(ch->wiznet));

    fprintf( fp, "Auct %d\n",	ch->colors[COLOR_AUCTION]	);
    fprintf( fp, "Goss %d\n",	ch->colors[COLOR_OOC]		);
    fprintf( fp, "Immo %d\n",	ch->colors[COLOR_IMMTALK]	);
    fprintf( fp, "Musi %d\n",	ch->colors[COLOR_BARD]		);
    fprintf( fp, "Tell %d\n",	ch->colors[COLOR_TELL]		);
    fprintf( fp, "Say  %d\n",	ch->colors[COLOR_SAY]		);
    fprintf( fp, "GuiC %d\n",   ch->colors[COLOR_GUILD]		);
    fprintf( fp, "SpeC %d\n",   ch->colors[COLOR_SPECIAL]	);
    fprintf( fp, "R_Ti %d\n",   ch->colors[COLOR_NAME]		);
    fprintf( fp, "R_De %d\n",   ch->colors[COLOR_DESC]		);
    fprintf( fp, "R_Ch %d\n",   ch->colors[COLOR_CHAR]		);
    fprintf( fp, "R_Ob %d\n",   ch->colors[COLOR_OBJ]		);
    fprintf( fp, "R_Ex %d\n",   ch->colors[COLOR_EXIT]		);

    if (ch->invis_level != 0)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);

    if (ch->practice != 0)
	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Att %d %d %d %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON],
	ch->perm_stat[STAT_CHR],
	ch->perm_stat[STAT_LUK],
	ch->perm_stat[STAT_AGI] );

    fprintf (fp, "AMods %d %d %d %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON],
	ch->mod_stat[STAT_CHR],
	ch->mod_stat[STAT_LUK],
	ch->mod_stat[STAT_AGI] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "SUse %d %d %d %d %d %d %d %d\n",
	    ch->pcdata->stat_use[0], ch->pcdata->stat_use[1],
	    ch->pcdata->stat_use[2], ch->pcdata->stat_use[3],
	    ch->pcdata->stat_use[4], ch->pcdata->stat_use[5],
	    ch->pcdata->stat_use[6], ch->pcdata->stat_use[7] );
	fprintf( fp, "PMax %d %d %d %d %d\n",
	    ch->channel_max[0], ch->channel_max[1],
	    ch->channel_max[2], ch->channel_max[3],
	    ch->channel_max[4] );
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if ( !IS_NULLSTR(ch->pcdata->email) )
	    fprintf( fp, "Emai %s~\n", ch->pcdata->email );
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
	if ( IS_IMMORTAL(ch) )
	{
	    if ( !IS_NULLSTR(ch->pcdata->bamfin) )
		fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	    if ( !IS_NULLSTR(ch->pcdata->bamfout) )
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	    fprintf( fp, "Home %d\n",	ch->home		);
	    fprintf( fp, "Tin   %s~\n", ch->pcdata->transin		);
	    fprintf( fp, "Tout  %s~\n", ch->pcdata->transout	);
	}

	for ( i = 0; i < MAX_ALIAS; i++ )
	{
	    if ( !IS_NULLSTR(ch->pcdata->alias[i]) )
		fprintf( fp, "Alias    %s %s~\n", ch->pcdata->alias[i],
		    ch->pcdata->alias_sub[i] );
	}
	if (!IS_NULLSTR(ch->pcdata->spouse))
	    fprintf( fp, "Spou %s~\n",	ch->pcdata->spouse	);

	if (ch->pcdata->new_desc && ch->pcdata->new_desc[0] != '\0' )
	    fprintf( fp, "NDesc %s~\n", ch->pcdata->new_desc	);
	if (ch->pcdata->new_last && ch->pcdata->new_last[0] != '\0' )
	    fprintf( fp, "NLast %s~\n", ch->pcdata->new_last	);
	if (ch->pcdata->new_title && ch->pcdata->new_title != '\0' )
	    fprintf( fp, "NTitle %s~\n", ch->pcdata->new_title	);

    	fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "LTax %ld\n",	ch->pcdata->last_tax	);
	fprintf( fp, "HSP %d %d\n", ch->pcdata->perm_hit,
	    ch->pcdata->perm_stamina);
	fprintf( fp, "Teac %d\n",	ch->pcdata->teach_skill );
	fprintf( fp, "Lear %d\n",	ch->pcdata->learn_skill );
	fprintf( fp, "Cond %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2] );

	fprintf( fp, "Hand %d\n", ch->pcdata->hand );
	fprintf( fp, "Weap %d\n", ch->pcdata->weapon );


	for ( tn = 0; tn < MAX_TALENT; tn++ )
	{
	    if ( talent_table[tn].name != NULL && ch->pcdata->talent[tn] )
		fprintf( fp, "Tal '%s'\n", talent_table[tn].name );
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] != -1 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->usage[sn] >= 0 )
	    {
		if ( ch->pcdata->usage[sn] != 0 )
		    fprintf( fp, "Use %d '%s'\n", ch->pcdata->usage[sn], 
			skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn] )
            {
                fprintf( fp, "Gr '%s'\n",group_table[gn].name);
            }
        }
    }

    if ( ch->pcdata->insane != 0 )
	fprintf( fp, "Insa %d\n", ch->pcdata->insane );

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;

	if ( paf->type == gsn_disguise
	||   paf->type == gsn_shape_change )
	    continue;


	fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d %10d %d %s\n",
	    skill_table[paf->type].name,
	    paf->strength,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    paf->bitvector_2,
	    paf->flags,
	    (paf->owner) ? paf->owner->name : "*no-one*"
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);

    fprintf(fp,"Name %s~\n", pet->name);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Races '%s'\n", race_table[pet->race].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HS  %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->stamina, pet->max_stamina);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %ld\n", pet->act);
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %d\n", pet->affected_by);
    if (pet->comm != 0)
    	fprintf(fp, "Comm %ld\n", pet->comm);
    fprintf(fp, "Att %d %d %d %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHR],
	pet->perm_stat[STAT_LUK], pet->perm_stat[STAT_AGI] );
    fprintf(fp, "AMod %d %d %d %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHR],
	pet->mod_stat[STAT_LUK], pet->mod_stat[STAT_AGI]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
	fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d %10d %1d %s\n",
	    skill_table[paf->type].name,
	    paf->strength,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    paf->bitvector_2,
	    paf->flags,
	    (paf->owner) ? paf->owner->name : "*no-one*"
	    );
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( obj->pIndexData->vnum == 4293 )
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->owner != NULL )
	fprintf( fp, "Own  %s\n",	obj->owner->name	     );

    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != 0)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cond %d\n",	obj->condition		     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[3] != obj->pIndexData->value[4]
    ||  obj->value[3] != obj->pIndexData->value[5]
    ||  obj->value[3] != obj->pIndexData->value[6]
    ||  obj->value[4] != obj->pIndexData->value[7])
    	fprintf( fp, "Value  %d %d %d %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4], obj->value[5], obj->value[6], obj->value[7] );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;

	fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d %10d %1d %s\n",
	    skill_table[paf->type].name,
	    paf->strength,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    paf->bitvector_2,
	    paf->flags,
	    (paf->owner) ? paf->owner->name : "*no-one*"
	    );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat, i;

    ch					= new_char();
    clear_char( ch );
    ch->pcdata				= new_pcdata();

    d->character			= ch;
    ch->desc				= d;

    ch->name				= str_dup( name );
    ch->id				= get_pc_id();

    ch->pcdata->max_level		= 1;
    ch->pcdata->last_name		= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->transin			= str_dup( "arrives from a cloud of smoke." );
    ch->pcdata->transout		= str_dup( "disappears in a cloud of smoke." );

    ch->pcdata->guild			= NULL;
    ch->pcdata->last_tax		= current_time;
    ch->gen_data			= NULL;

    for ( i = 0; i < MAX_TALENT; i++ )
	ch->pcdata->talent[i]		= FALSE;

    ch->hide				= NULL;
    ch->hide_type			= HIDE_NONE;
    ch->char_made			= FALSE;
    ch->version				= 0;
    ch->race				= race_lookup("Andoran");
    ch->affected_by			= 0;
    ch->affected_by_2			= 0;
    ch->class				= WARRIOR_TYPE;
    ch->act				= PLR_AUTOSAC
					| PLR_AUTOLOOT
					| PLR_AUTOEXIT
					| PLR_SUBDUE;
    ch->comm				= COMM_COMBINE 
					| COMM_PROMPT
					| COMM_NOTES
					| COMM_NOTICK
					| COMM_NOSPAM;
    ch->book				= 0;
    ch->invis_level			= 0;
    ch->practice			= 0;
    ch->start_age			= 0;
    ch->still				= 0;
    ch->train				= 0;
    ch->hitroll				= 0;
    ch->damroll				= 0;
    ch->trust				= 0;
    ch->wimpy			 	= 0;

    ch->home				= ROOM_VNUM_RECALL;
    ch->on				= NULL;
    ch->mount				= NULL;
    ch->rider				= NULL;
    ch->memory				= NULL;
    ch->hunting				= NULL;

    /* Channel colors */
    ch->colors[COLOR_AUCTION]	= 7;
    ch->colors[COLOR_OOC]	= 1;
    ch->colors[COLOR_IMMTALK]	= 4;
    ch->colors[COLOR_BARD]	= 15;
    ch->colors[COLOR_TELL]	= 11;
    ch->colors[COLOR_SAY]	= 7;
    ch->colors[COLOR_GUILD]	= 14;
    ch->colors[COLOR_SPECIAL]	= 12;

    /* Room stuff */
    ch->colors[COLOR_NAME]	= 15;
    ch->colors[COLOR_DESC]	= 9;
    ch->colors[COLOR_CHAR]	= 3;
    ch->colors[COLOR_OBJ]	= 10;
    ch->colors[COLOR_EXIT]	= 5;
    ch->guild			= guild_lookup( "none" );

    ch->action_timer		= 0;
    ch->action_args		= &str_empty[0];
    ch->action_target		= NULL;

    ch->prompt				= str_dup( "< %hhp %sst > " );
    ch->pcdata->config			= CONFIG_COMBHELP;
    ch->pcdata->hits			= 0;
    ch->pcdata->misses			= 0;
    ch->pcdata->insane			= 0;
    ch->pcdata->points			= 30;
    ch->pcdata->confirm_delete		= FALSE;
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    for ( stat = 0; stat < 5; stat++ )
	ch->channel_max[stat] = 0;
    ch->pcdata->perm_hit		= 0;
    ch->pcdata->perm_stamina		= 0;
    ch->pcdata->true_sex		= 0;
    ch->pcdata->last_level		= 0;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->security                = 0;    /* OLC */
    ch->pcdata->weapon			= 0;
    ch->pcdata->learn_skill		= dice( 1, 10 );
    ch->pcdata->teach_skill		= dice( 1, 10 );
    ch->pcdata->stat_count		= 10;
    ch->pcdata->stat_point		= 10;
    ch->pcdata->guild			= NULL;
    ch->pcdata->shadow_rank		= -1;
    ch->pcdata->hits			= 0;
    ch->pcdata->misses			= 0;
    handedness( ch );

    for ( i = 0; i < MAX_SKILL; i++ )
    {
	ch->pcdata->learned[i] = -1;	/* For skill thing */
	ch->pcdata->skill_mod[i] = 0;	/* For skill thing */
	ch->pcdata->usage[i] = 0;
    }

    sprintf( buf, "Attempting to load: %s", name );
    log_string( buf );

    found = FALSE;
    fclose( fpReserve );
    add_pc_list( ch );

    sprintf( filename, "%s", correct_name(ch->name) );

    sprintf( strsave, "%s%s", PLAYER_DIR, filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"mv %s%s %s%c/%s", PLAYER_DIR,
	    filename, PLAYER_DIR, LOWER(filename[0]), filename );
	system(buf);
    }

    /* move from archive if in archive */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, "archive/",
	filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"mv %s%s%s %s%c/%s", PLAYER_DIR, "archive/",
	    filename, PLAYER_DIR, LOWER(filename[0]), filename );
	system(buf);
    }

    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%c/%s%s", PLAYER_DIR,
	LOWER(filename[0]), filename,".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%c/%s", PLAYER_DIR, 
	LOWER(filename[0]), filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    remove_skill( ch );

    /* initialize race */
    if (found)
    {
	if (ch->race == 0)
	    ch->race = race_lookup("Andorian");

	ch->dam_type = 17; /*punch */

	ch->affected_by = ch->affected_by|race_table[1].aff;
	ch->imm_flags	= ch->imm_flags | race_table[1].imm;
	ch->res_flags	= ch->res_flags | race_table[1].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[1].vuln;
	ch->form	= race_table[1].form;
	ch->parts	= race_table[1].parts;
    }

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}
/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif          

#define KEYS( literal, field, value )                                   \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    free_string(field);                 \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }


void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int  alias_ind = 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;
#if defined(DEBUG_SAVE)
	sprintf( buf, "[%s] loading: %s", ch->name, word );
	log_string( buf );
#endif
	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "ActF",	ch->act,		fread_flag(fp));
	    KEY( "Age",		ch->start_age,		fread_number(fp));
	    KEY( "AfByF",	ch->affected_by,	fread_flag(fp));
	    KEY( "AfB2F",	ch->affected_by_2,	fread_flag(fp));
	    KEY( "Appl",	ch->pcdata->last_apply,	fread_number(fp));
	    KEY( "Auct", 	ch->colors[COLOR_AUCTION],fread_number( fp ) );

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp(word, "Alias") )
	    {
		char *arg;
		char arg1[MAX_INPUT_LENGTH];
		arg = fread_string( fp );
		arg = one_argument( arg, arg1 );
		if ( alias_ind < MAX_ALIAS && arg1[0] != '\0' )
		{
		    free_string( ch->pcdata->alias_sub[alias_ind] );
		    free_string( ch->pcdata->alias[alias_ind] );
		    ch->pcdata->alias[alias_ind] = str_dup( arg1 );
		    ch->pcdata->alias_sub[alias_ind] = str_dup( arg );
		    alias_ind++;
		}
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "Aff" ) 
	    ||   !str_cmp( word, "AffD"))
	    {
		AFFECT_DATA *paf;
		void *vo;
		char *name;

		paf = new_affect();

	  	if (!str_cmp(word,"AffD"))
		{
		    int sn;
		    sn = skill_lookup(fread_word(fp));
		    if (sn < 0)
			bug("Fread_char: unknown skill.",0);
		    else
			paf->type = sn;
	 	}
		else  /* old form */
		    paf->type	= fread_number( fp );
		if (ch->version == 0)
		    paf->strength = ch->level;
		else
		    paf->strength= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->bitvector_2= fread_number( fp );
		paf->flags	= fread_number( fp );
		name		= fread_word( fp );
		paf->owner	= get_char_sedai( ch, name );
		if ( !str_cmp(name, ch->name) )
		    paf->owner	= ch;
		paf->next	= ch->affected;
		ch->affected	= paf;
		vo = (void *) ch;
		add_weave_list( vo, NODE_WEAVE_CHAR );
		if ( !IS_SET(paf->flags, AFFECT_NOTCHANNEL)
		&&   !IS_TIED(paf)
		&&   (paf->owner == NULL
		||    ( paf->owner != NULL
		&&      !IS_GRASPING(paf->owner) )) )
		    affect_remove( ch, paf );
		if ( paf->type == gsn_linking )
		    affect_remove( ch, paf );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < 7; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < 7; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp(word,"Att"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp(word,"AMods"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Bank",	ch->bank,		fread_number( fp ) );
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "BookF",	ch->book,		fread_flag(fp));
	    break;

	case 'C':
	    KEY( "ConfigF",	ch->pcdata->config,	fread_flag(fp));
	    KEY( "Chng",	ch->pcdata->last_changes,fread_number(fp));
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",		ch->class,		fread_number( fp ) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY("CommF",		ch->comm,	fread_flag(fp)); 
	    break;

	case 'D':
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    KEY( "Desc",	ch->description,	fread_string( fp ) );
	    if ( !str_cmp(word, "damname") )
	    {
		add_guild( ch );
		ch->pcdata->guild->damane_name = fread_string( fp );
		fMatch = TRUE;
	    }
	    break;
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return;
	    KEY( "Emai",	ch->pcdata->email,	fread_string( fp ));
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
	    KEY( "Goss",	ch->colors[COLOR_OOC],fread_number( fp ) );
	    KEY( "GuiC",	ch->colors[COLOR_GUILD],fread_number( fp ) );
	    if ( !str_cmp(word, "Guild") )
	    {
		add_guild( ch );
		ch->guild	= guild_lookup( fread_word(fp) );
                fMatch		= TRUE;
	    }
	    if ( !str_cmp(word, "GuildR") )
	    {
		add_guild( ch );
		ch->pcdata->guild->rank = fread_number( fp );
                fMatch = TRUE;
	    }
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;
 
                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }
	    break;

	case 'H':
	    KEY("Hand", 	ch->pcdata->hand,	fread_number(fp));

	    if ( !str_cmp( word, "HpManaMove" )
	    ||   !str_cmp(word, "HMV")
	    ||   !str_cmp(word, "HS") )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->stamina	= fread_number( fp );
		ch->max_stamina	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" )
	    || !str_cmp(word,"HMVP")
	    || !str_cmp(word,"HSP") )
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_stamina= fread_number( fp );
                fMatch = TRUE;
                break;
            }

	    KEY( "Home",	ch->home,		fread_number( fp ) );
      
	    break;

	case 'I':
	    KEY( "Idea",	ch->pcdata->last_idea,	fread_number(fp));
	    KEY( "Id",		ch->id,			fread_number(fp));
	    KEY( "Immo",	ch->colors[COLOR_IMMTALK],fread_number(fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Insa",	ch->pcdata->insane,	fread_number( fp ) );
	    break;

	case 'L':
	    if ( !str_cmp(word, "LastIdent")
	    ||   !str_cmp(word, "LastHost") )
	    {
		fread_to_eol( fp );
                fMatch = TRUE;
                break;
	    }
	    KEY( "LTax",	ch->pcdata->last_tax,	fread_number( fp));
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ));
	    KEY( "Lear",	ch->pcdata->learn_skill,fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );

	    if ( !str_cmp( word, "Last" )  || !str_cmp( word, "LastName"))
	    {
		free_string( ch->pcdata->last_name );
		ch->pcdata->last_name = fread_string( fp );
		sprintf( buf, " %s", ch->pcdata->last_name );
		free_string( ch->pcdata->last_name );
	 	ch->pcdata->last_name = str_dup( buf );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'M':
	    KEY( "MaxL",	ch->pcdata->max_level,	fread_number(fp) );
	    KEY( "Made",	ch->char_made,		fread_number(fp) );
	    KEY( "Musi",	ch->colors[COLOR_BARD],fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "NDesc",	ch->pcdata->new_desc,	fread_string(fp));
	    KEYS( "Name",	ch->name,		fread_string(fp));
	    KEY("News",	ch->pcdata->last_news,	fread_number(fp));
	    KEY( "Note",	ch->pcdata->last_note,	fread_number(fp));

	    if ( !str_cmp(word, "NLast") )
	    {
		free_string( ch->pcdata->new_last );
		ch->pcdata->new_last = fread_string( fp );
		sprintf( buf, " %s", ch->pcdata->new_last );
		free_string( ch->pcdata->new_last );
	 	ch->pcdata->new_last = str_dup( buf );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp(word, "NTitle") )
	    {
		free_string( ch->pcdata->new_title );
		ch->pcdata->new_title = fread_string( fp );
		sprintf( buf, " %s", ch->pcdata->new_title );
		free_string( ch->pcdata->new_title );
	 	ch->pcdata->new_title = str_dup( buf );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
	    KEY( "Pena",	ch->pcdata->last_penalty,fread_number(fp));
	    KEYS("Prompt",	ch->prompt,		fread_string(fp));
	    KEY( "Prom",	ch->prompt,		fread_string(fp));
	    if ( !str_cmp(word, "PMax") )
	    {
		ch->channel_max[0]	= fread_number( fp );
		ch->channel_max[1]	= fread_number( fp );
		ch->channel_max[2]	= fread_number( fp );
		ch->channel_max[3]	= fread_number( fp );
		ch->channel_max[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'Q':
	    KEY( "QP",		ch->pcdata->qp,		fread_number( fp ) );
	    break;

	case 'R':
            KEY( "R_Ti",        ch->colors[COLOR_NAME],fread_number( fp ) );
            KEY( "R_De",        ch->colors[COLOR_DESC],fread_number( fp ) );
            KEY( "R_Ch",        ch->colors[COLOR_CHAR],fread_number( fp ) );
            KEY( "R_Ob",        ch->colors[COLOR_OBJ],fread_number( fp ) );
            KEY( "R_Ex",        ch->colors[COLOR_EXIT],fread_number( fp ) );

	    KEY( "Race",        ch->race,	
				race_lookup(fread_string( fp )) );
	    KEY( "Races",        ch->race,	
				race_lookup(fread_word( fp )) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "Say",		ch->colors[COLOR_SAY],	fread_number(fp));
	    KEY( "Scro",	ch->lines,		fread_number(fp));
	    KEY( "Sex",		ch->sex,		fread_number(fp));
	    KEY( "ShortDescr",	ch->short_descr,	fread_string(fp));
	    KEY( "ShD",		ch->short_descr,	fread_string(fp));
	    KEY( "ShadR",	ch->pcdata->shadow_rank,fread_number(fp));
	    KEY( "ShadN",	ch->pcdata->shadow_name,fread_string(fp));
	    KEY( "Sec",         ch->pcdata->security,   fread_number(fp));
	    KEY( "Spou",	ch->pcdata->spouse,	fread_string(fp));
	    KEY( "Sedai",	ch->pcdata->sedai,	fread_string(fp));
	    KEY( "SpeC",	ch->colors[COLOR_SPECIAL],fread_number( fp ) );
	    if ( !str_cmp(word, "SUse") )
	    {
		ch->pcdata->stat_use[0]	= fread_number( fp );
		ch->pcdata->stat_use[1]	= fread_number( fp );
		ch->pcdata->stat_use[2]	= fread_number( fp );
		ch->pcdata->stat_use[3]	= fread_number( fp );
		ch->pcdata->stat_use[4]	= fread_number( fp );
		ch->pcdata->stat_use[5]	= fread_number( fp );
		ch->pcdata->stat_use[6]	= fread_number( fp );
		ch->pcdata->stat_use[7]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;

		value = fread_number( fp );
		sn    = skill_lookup( fread_word( fp ) );
		if ( sn < 0 )
		{
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }
	    break;

	case 'T':
	    KEY( "Teac",	ch->pcdata->teach_skill,fread_number( fp ) );
	    KEY( "Tell",	ch->colors[COLOR_TELL],fread_number( fp ) );
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );
	    KEY( "Tin",		ch->pcdata->transin,	fread_string( fp ));
	    KEY( "Tout",	ch->pcdata->transout,	fread_string( fp ));

	    if ( !str_cmp( word, "Talent" ) || !str_cmp(word,"Ta"))
	    {
		int tn;
		tn    = talent_lookup( fread_string( fp ) );
		if ( tn < 0 )
		{
		    bug( "Fread_char: unknown talent. ", 0 );
		}
		else
		    ch->pcdata->talent[tn] = TRUE;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "Talent" ) || !str_cmp(word,"Tal"))
	    {
		int tn;
		tn    = talent_lookup( fread_word( fp ) );
		if ( tn < 0 )
		{
		    bug( "Fread_char: unknown talent. ", 0 );
		}
		else
		    ch->pcdata->talent[tn] = TRUE;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		free_string( ch->pcdata->title );
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'U':
	    if ( !str_cmp( word, "Use" ) || !str_cmp(word,"Us"))
	    {
		int sn;
		int value;

		value = fread_number( fp );
		sn    = skill_lookup( fread_word( fp ) );
		if ( sn < 0 )
		{
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->usage[sn] = value;
		fMatch = TRUE;
	    }

	case 'V':
	    if ( !str_cmp(word, "Vers") )
		ch->char_made = TRUE;
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Weap",	ch->pcdata->weapon,	fread_number( fp ) );
	    KEY( "Wearing",	ch->pcdata->wearing,	fread_string(fp));
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
            KEY( "WiznF",       ch->wiznet,		fread_flag(fp));
	    if ( !str_cmp(word, "warder") )
	    {
		add_guild( ch );
		ch->pcdata->guild->warder = fread_string( fp );
		fMatch = TRUE;
	    }
	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}


/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_number(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_number(fp));
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
		void *vo;
		char *name;
    	    	
		paf = new_affect();

    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->strength	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->bitvector_2= fread_number(fp);
    	     	paf->flags	= fread_number(fp);
		name		= fread_word( fp );
		paf->owner	= get_char_sedai( ch, name );
		if ( !str_cmp(name, ch->name) )
		    paf->owner	= ch;
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
		vo = (void *) pet;
		add_weave_list( vo, NODE_WEAVE_CHAR );
		if ( !IS_SET(paf->flags, AFFECT_NOTCHANNEL)
		&&   !IS_TIED(paf)
		&&   (paf->owner == NULL
		||    ( paf->owner != NULL
		&&      !IS_GRASPING(paf->owner) )) )
		    affect_remove( ch, paf );		    
		if ( paf->type == gsn_linking )
		    affect_remove( ch, paf );
    	     	fMatch		= TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < 7; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < 7; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    if (!str_cmp(word,"AMods"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Att"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
    	     KEY( "Comm",	pet->comm,		fread_number(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV") || !str_cmp(word,"HS") )
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->stamina	= fread_number(fp);
    	     	pet->max_stamina= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    KEY( "Races",	pet->race, race_lookup(fread_word(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
    	    break;
    	}

    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    }
    
}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
#if defined(DEBUG_SAVE)
    char buf[MAX_STRING_LENGTH];
#endif
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	}
        else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
	obj		= new_obj( );

    	*obj		= obj_zero;
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;
#if defined(DEBUG_SAVE)
	sprintf( buf, "[%s] loading: %s", ch->name, word );
	log_string( buf );
#endif
	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) || !str_cmp(word,"Aff")
	    ||   !str_cmp( word, "AffD"))
	    {
		AFFECT_DATA *paf;
		void *vo;
		char *name;

		paf = new_affect();

		if (!str_cmp(word, "AffD"))
		{
		    int sn;
		    sn = skill_lookup(fread_word(fp));
		    if (sn < 0)
			bug("Fread_obj: unknown skill.",0);
		    else
		   	paf->type = sn;
		}
		else /* old form */
		    paf->type	= fread_number( fp );
		if (ch->version == 0)
		  paf->strength = 20;
		else
		  paf->strength	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->bitvector_2= fread_number( fp );
		paf->flags	= fread_number( fp );
		name		= fread_word( fp );
		paf->owner	= get_char_sedai( ch, name );
		if ( !str_cmp(name, ch->name) )
		    paf->owner	= ch;
		if ( (paf->location == APPLY_HITROLL
		||    paf->location == APPLY_DAMROLL)
		&&    obj->item_type != ITEM_WEAPON )
		    free_affect( paf );
		else
		{
		    paf->next	= obj->affected;
		    obj->affected	= paf;
		    vo = (void *) obj;
		    add_weave_list( vo, NODE_WEAVE_OBJ );
		    if ( !IS_SET(paf->flags, AFFECT_NOTCHANNEL)
		    &&   !IS_TIED(paf)
		    &&   (paf->owner == NULL
		    ||    ( paf->owner != NULL
		    &&      !IS_GRASPING(paf->owner) )) )
		    	affect_obj_remove( obj, paf );
		}
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_flag(fp));

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
                {
                    bug( "Fread_obj: incomplete object.", 0 );
                    free_string( obj->name        );
                    free_string( obj->description );
                    free_string( obj->short_descr );
                    obj->next = obj_free;
                    obj_free  = obj;
                    return;
                }
                else
                {
                    if ( !fVnum )
                    {
                        free_string( obj->name        );
                        free_string( obj->description );
                        free_string( obj->short_descr );
                        obj->next = obj_free;
                        obj_free  = obj;

                        obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0);
                    }

                    if (!new_format)
                    {
                        obj->next       = object_list;
                        object_list     = obj;
                        obj->pIndexData->count++;
                    }

                    if (!obj->pIndexData->new_format
                    && obj->item_type == ITEM_ARMOR
                    &&  obj->value[1] == 0)
                    {
                        obj->value[1] = obj->value[0];
                        obj->value[2] = obj->value[0];
                    }
                    if (make_new)
                    {
                        int wear;

                        wear = obj->wear_loc;
                        extract_obj(obj);

                        obj = create_object(obj->pIndexData,0);
                        obj->wear_loc = wear;
                    }
                    if ( iNest == 0 || rgObjNest[iNest] == NULL )
		    {
                        obj_to_char( obj, ch );
		    } 
                    else
                        obj_to_obj( obj, rgObjNest[iNest-1] );

		    if ( IS_SET(obj->extra_flags, ITEM_CHANNELED) )
		    {
			void *vo;
			vo = (void *) obj;
			add_weave_list( vo, NODE_WEAVE_CREATE );
		    }
		    if ( IS_SET(obj->extra_flags, ITEM_CHANNELED)
		    &&   !IS_SET(obj->extra_flags, ITEM_TIED)
		    &&   (obj->owner == NULL
		    ||    ( obj->owner != NULL
		    &&      !IS_GRASPING(obj->owner) )) )
			extract_obj( obj );
                    return;
                }
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp(word, "Own") )
	    {
		char *name;
		name		= fread_word( fp );
		obj->owner	= get_char_sedai( ch, name );
		if ( !str_cmp(name, ch->name) )
		    obj->owner = ch;
		fMatch = TRUE;
	    }
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp(word, "Value") )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		obj->value[5]	= fread_number( fp );
		obj->value[6]	= fread_number( fp );
		obj->value[7]	= fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void *load_value( char *argument, int type )
{
    FILE *fp;
    bool found;
    char strsave[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    found = FALSE;
    fclose( fpReserve );

    sprintf( filename, "%s", correct_name(arg1) );

    sprintf( strsave, "%s%s", PLAYER_DIR, filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"mv %s%s %s%c/%s", PLAYER_DIR,
	    filename, PLAYER_DIR, LOWER(filename[0]), filename );
	system(buf);
    }

    /* move from archive if in archive */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, "archive/",
	filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"mv %s%s%s %s%c/%s", PLAYER_DIR, "archive/",
	    filename, PLAYER_DIR, LOWER(filename[0]), filename );
	system(buf);
    }

    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%c/%s%s", PLAYER_DIR,
	LOWER(filename[0]), filename,".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%c/%s", PLAYER_DIR, 
	LOWER(filename[0]), filename );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) )
		return value_from_file(fp, arg2, type);
	    else
	    {
		bug( "Load_value: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return NULL;
}


void *value_from_file( FILE *fp, char *match, int type )
{
    static char buf[MAX_STRING_LENGTH];
    static int  number;
    char *word;
    char *str = NULL;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	if ( str )
	    free_string( str );

	if ( word[0] == '*' )
	    fread_to_eol( fp );

	if ( !str_cmp(word, "End") )
	    return NULL;

	if ( !str_cmp(word, match) )
	{
	    if ( type == 0 ) /* string */
	    {
		str = fread_string(fp);
		sprintf( buf, "%s", str );
		free_string( str );
		return (void *)buf;
	    }
	    if ( type == 1 ) /* number */
	    {
		number = fread_number(fp);
		return (void *) &number;
	    }
	    if ( type == 2 ) /* flag */
	    {
		number = fread_flag(fp);
		return (void *) &number;
	    }
	}
    }
    return NULL;
}
