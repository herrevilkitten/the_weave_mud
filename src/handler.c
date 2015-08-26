/**************************************************************************r
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

/* command procedures needed */
DECLARE_DO_FUN(do_return	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_release	);

DECLARE_SPELL_FUN(      spell_null              );


/*
 * Local functions.
 */
#define	CD	CHAR_DATA
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	apply_spell	args( ( CHAR_DATA *ch, int mod ) );

char *flag_string               args ( ( const struct flag_type *flag_table,
                                         int bits ) );
int flag_value                  args ( ( const struct flag_type *flag_table,
                                         char *argument) );

void	figure_carrying	args( ( CHAR_DATA *ch ) );
CD	*carrier	args( ( OBJ_DATA *obj ) );
void	figure_ac	args( ( CHAR_DATA *ch ) );
#undef	CD

CHAR_DATA *random_room_char( ROOM_INDEX_DATA *in_room )
{
    CHAR_DATA *vch;
    int i, count = 0, vic;

    for ( vch = in_room->people; vch != NULL; vch = vch->next_in_room )
	count++;

    vic = number_range( 0, count );
    for ( i = 0, vch = in_room->people; i < vic; i++, vch = vch->next_in_room )
	if ( i == vic - 1 )
	    return vch;
    return NULL;
}

int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
        return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)  
        if (fch->on == obj)
            count++;

    return count;
}


/*
 *   MATERIAL FUNCTIONS
 */
int material_lookup (char *name)
{
    int material;

    if ( (material = flag_value(material_type, name)) != NO_FLAG )
	return material;

    return 0;
}

char *material_name( sh_int num )
{
    return( material_type[num].name );
}


/*
 *   GUILD FUNCTIONS
 */
bool is_guild_imm( CHAR_DATA *ch )
{
    GUILD_DATA *pGuild;

    if ( IS_NPC(ch) )
	return FALSE;

    for ( pGuild = guild_first; pGuild; pGuild = pGuild->next )
    {
	if ( is_full_name(ch->name, pGuild->imms) )
	    return TRUE;
    }

    return FALSE;
}


bool is_guild_name( CHAR_DATA *ch )
{
    GUILD_DATA *pGuild;

    if ( IS_NPC(ch) )
	return FALSE;

    for ( pGuild = guild_first; pGuild; pGuild = pGuild->next )
    {
	if ( is_name(ch->name, pGuild->imms) )
	    return TRUE;
    }

    return FALSE;
}


GUILD_DATA *guild_struct( int number )
{
    GUILD_DATA *guild;
    int count = 0;

    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
	if ( count == number )
	    return guild;
	count++;
    }

    return NULL;
}

int guild_lookup( const char *name )
{
    GUILD_DATA *guild;
    int count;

    count = 0;
    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
        if ( LOWER(name[0]) == LOWER(guild->name[0]) &&
             !str_prefix(name, guild->name) )
            return count;
        count++;
    }
    
    count = 0;
    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
        if ( LOWER(name[0]) == LOWER(guild->savename[0]) &&
             !str_prefix(name, guild->savename) )
            return count;
        count++;
    }
    
    return -1;
}
        
char *guild_name( int number )
{
    GUILD_DATA *guild;
    int count;
             
    count = 0;
    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
        if ( count == number )
            return( guild->name );
	count++;
    }
        
    return "none";
}

char *guild_savename( int number )
{            
    GUILD_DATA *guild;
    int count;
     
    count = 0;
    for ( guild = guild_first; guild != NULL; guild = guild->next )
    {
        if ( count == number )
            return( guild->savename );
	count++;
    }

    return "none";
}
    
char *guild_rank( int guild, sh_int rank, int type, bool fDup )
{
    GUILD_DATA *pGuild;
    int count;
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    count = 0;

    for ( pGuild = guild_first; pGuild != NULL; pGuild = pGuild->next )
    {
        if ( count == guild )
	{
	    switch( type )
	    {
		default:
		case 1:
		    strcpy( buf, pGuild->rank[0][rank] );
		    break;
		case 2:
		    strcpy( buf, pGuild->rank[1][rank] );
		    break;
		case 3:
		    strcpy( buf, pGuild->rank[2][rank] );
		    break;
		case 4:
		    strcpy( buf, pGuild->rank[3][rank] );
		    break;
	    }
	    if ( !fDup )
		return( buf );
	    else
		return( str_dup(buf) );
	}
        count++;
    }
    if ( !fDup )
	return( "none" );
    else
	return( str_dup("none") );
}
   

/*
 *   RACE FUNCTIONS
 */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
} 


/*
 *   CLASS FUNCTIONS
 */
int class_lookup (const char *name)
{
   int class;
 
   for ( class = 0; class < MAX_CLASS; class++)
   {
        if (LOWER(name[0]) == LOWER(class_table[class].name[0])
        &&  !str_prefix( name,class_table[class].name))
            return class;
   }
 
   return -1;
}


/*
 *   IMMUNITY FUNCTIONS
 */
int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune;
    int bit;

    immune = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    immune = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    immune = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
	default:		return immune;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit))
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
	immune = IS_VULNERABLE;

    return immune;
}


/*
 *   MOB FUNCTIONS
 */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}




/*
 *   CHARACTER FUNCTIONS
 */
void reset_char(CHAR_DATA *ch)
{
     int loc, mod, stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if ( !ch )
	return;

    if (ch->pcdata->perm_hit == 0 
    ||  ch->pcdata->perm_stamina == 0
    ||	ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_STAMINA:	ch->max_stamina	-= mod;break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_STAMINA:	ch->max_stamina	-= mod;		break;
                }
            }
	}
	/* now reset the permanent stats */
	ch->pcdata->perm_hit 	= ch->max_hit;
	ch->pcdata->perm_stamina= ch->max_stamina;
	ch->pcdata->last_level	= ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2) {
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
        }

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_stamina	= ch->pcdata->perm_stamina;
   
    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
		case APPLY_CHR:		ch->mod_stat[STAT_CHR]	+= mod; break;
		case APPLY_LUK:		ch->mod_stat[STAT_LUK]	+= mod; break;
		case APPLY_AGI:		ch->mod_stat[STAT_AGI]	+= mod;break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_STAMINA:	ch->max_stamina		+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
	
		case APPLY_SPELL:
		    apply_spell( ch, mod );
		    break;

    		case APPLY_SKILL:
	    	    if ( ch->pcdata->skill_mod[slot_lookup(mod)] > 0 )
			ch->pcdata->skill_mod[slot_lookup(mod)] += 5;
		    break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
                case APPLY_CHR:         ch->mod_stat[STAT_CHR]  += mod; break;
                case APPLY_LUK:         ch->mod_stat[STAT_LUK]  += mod; break;
                case APPLY_AGI:         ch->mod_stat[STAT_AGI]  += mod;break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_STAMINA:	ch->max_stamina		+= mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SPELL:
		    apply_spell( ch, mod );
		    break;
    		case APPLY_SKILL:
	    	    if ( ch->pcdata->skill_mod[slot_lookup(mod)] > 0 )
			ch->pcdata->skill_mod[slot_lookup(mod)] += 5;
		    break;

            }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
                case APPLY_CHR:         ch->mod_stat[STAT_CHR]  += mod; break;
                case APPLY_LUK:         ch->mod_stat[STAT_LUK]  += mod; break;
                case APPLY_AGI:         ch->mod_stat[STAT_AGI]  += mod;break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_STAMINA:	ch->max_stamina		+= mod;break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
        } 
    }

    figure_ac( ch );

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( !ch )
	return 0;
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}

/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return (ch->start_age) +
	( ch->played + (int) (current_time - ch->logon) ) / 1209600;
}


/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 1;

    return MAX_WEAR + 2 * get_curr_stat(ch,STAT_DEX) + ch->level * 4 / 5;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 1;

    return str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level * 5 / 2;
}


/*
 *   OBJECT AND CHARACTER FINDING FUNCTIONS
 */
bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list, name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string, name))
		return TRUE; /* full pattern match */

	    if (!str_cmp(part, name))
		break;
	}
    }
}

bool is_full_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
        str = one_argument(str,part);

        if (part[0] == '\0' )
            return TRUE;

        /* check to see if this is part of namelist */
        list = namelist;
        for ( ; ; )  /* start parsing namelist */
        {
            list = one_argument(list, name);
            if (name[0] == '\0')  /* this name was not found */
                return FALSE;

            if (!str_cmp(string, name))
                return TRUE; /* full pattern match */

            if (!str_cmp(part, name))
                break;
        }
    }
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) || !str_cmp( arg, "me" ) )
	return ch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( IS_DISGUISED(rch) && !IS_NULLSTR(rch->pcdata->new_name) )
	{
	    if ( !can_see(ch, rch)
            ||   !is_name(arg, rch->pcdata->new_name))
	    	continue;
	}
	else
	{
	    if ( !can_see(ch, rch) || !is_name(arg, rch->name) )
	    	continue;
	}
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if ( IS_DISGUISED(wch) && !IS_NULLSTR(wch->pcdata->new_name) )
        {
	    if ( wch->in_room == NULL || !can_see( ch, wch ) 
	    ||   !is_name( arg, wch->pcdata->new_name ) )
	    	continue;
        }
        else
        {
	    if ( wch->in_room == NULL || !can_see( ch, wch ) 
	    ||   !is_name( arg, wch->name ) )
	    	continue;
        }

	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

ROOM_INDEX_DATA *get_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *wr;
    int number;
    int count;
    int iHash;

    number = number_argument( argument, arg );
    count = 0;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( wr = room_index_hash[iHash]; wr; wr = wr->next )
        {
	    if ( !is_name( arg, wr->name ) )
		continue;

	    if ( ++count == number )
		return wr;
	}
    }

    return NULL;
}

CHAR_DATA *get_char_sedai( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL
	||   !is_full_name( arg, wch->name ) )
	    continue;

	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

/*
 * Find a char in the area.
 */
CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( !ch->in_room )
	return NULL;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( !wch->in_room )
	    continue;

	if ( ch->in_room->area != wch->in_room->area )
	    continue;

	if ( IS_DISGUISED(wch) && !IS_NULLSTR(wch->pcdata->new_name) )
        {
	    if ( wch->in_room == NULL || !can_see( ch, wch ) 
	    ||   !is_name( arg, wch->pcdata->new_name ) )
	    	continue;
        }
        else
        {
	    if ( wch->in_room == NULL || !can_see( ch, wch ) 
	    ||   !is_name( arg, wch->name ) )
	    	continue;
        }

	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 *   OBJECT AND CHARACTER MOVEMENT FUNCTIONS
 */
/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    CHAR_DATA *vch;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( vch->hide_type == HIDE_CHAR
	&&   (CHAR_DATA *) vch->hide == ch )
	{
	    send_to_char( "Your cover moves!\n\r", vch );
	    REMOVE_BIT( vch->affected_by, AFF_HIDE );
	    vch->hide_type = HIDE_NONE;
	    vch->hide = NULL;
	}
    }

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on	     = NULL;
    return;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        int save;
        
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }
        
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->strength == 1)
            return;
        
        plague.type 		= gsn_plague;
        plague.strength		= af->strength - 1; 
        plague.duration 	= number_range(1,2 * plague.strength);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
        plague.bitvector_2	= 0;
	plague.owner		= NULL;

        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            switch(check_immune(vch,DAM_DISEASE))
            {
            	case(IS_NORMAL) 	: save = af->strength - 4;break;
            	case(IS_IMMUNE) 	: save = 0;		break;
            	case(IS_RESISTANT) 	: save = af->strength - 8;break;
            	case(IS_VULNERABLE)	: save = af->strength; 	break;
            	default			: save = af->strength - 4;break;
            }
            
            if (save != 0 && !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }
    if ( IS_GRASPING(ch)
    &&   IS_SET(pRoomIndex->room_flags, ROOM_STEDDING) )
    {
	send_to_char( "You cannot touch the True Source here.\n\r", ch );
	do_release(ch, "");
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->leader  
    &&   ch->leader != ch
    &&   ch->leader->in_room != ch->in_room )
    {
	CHAR_DATA *leader;

	leader = ch->leader ? ch->leader : ch;
	send_to_char( "You move out of range of the link.\r\n", ch );
	link_stamina( leader, ch, FALSE );
	do_release( ch, "" );
    }

    return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    figure_carrying( ch );

    if ( obj->item_type == ITEM_ARMOR
    &&   obj->wear_loc  != WEAR_NONE )
	figure_ac( ch );
}

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    figure_carrying( ch );
    return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )   
{
    CHAR_DATA *ch;

    obj->next_content           = obj_to->contains;
    obj_to->contains            = obj;
    obj->in_obj                 = obj_to;
    obj->in_room                = NULL;
    obj->carried_by             = NULL;
    if (obj_to->pIndexData->vnum == 509
    ||  obj_to->pIndexData->vnum == 4701 )
    {
        obj->cost = 0;
	SET_BIT( obj->extra_flags, ITEM_DONATE_ROT );
	obj->timer = 50 - ( obj->level / 2 );
    }

    if ( (ch = carrier( obj )) != NULL )
	figure_carrying( ch );
    return;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;
    CHAR_DATA *ch;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
        bug( "Obj_from_obj: null obj_from.", 0 ); 
        return;
    }
     
    if ( obj == obj_from->contains )
    {
        obj_from->contains = obj->next_content;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = obj_from->contains; prev; prev = prev->next_content )
        {
            if ( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }  
        }
     
        if ( prev == NULL )
        {
            bug( "Obj_from_obj: obj not found.", 0 );
            return;
        }
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    if ( (ch = carrier( obj )) != NULL )
	figure_carrying( ch );

    return;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
        if (ch->on == obj)   
            ch->on = NULL;


    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex == NULL )
    {
	extract_obj( obj );
	return;
    }
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}


/*
 *   EQUIPMENT FUNCTIONS
 */
/*
 * Find the ac value of an obj, including position effect.
 */
void figure_ac( CHAR_DATA *ch )
{
    int i, mod;
    OBJ_DATA *obj;
    AFFECT_DATA *paf;

    for ( i = 0;i < 4; i++ )
	ch->armor[i] = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE )
	    continue;

	if (!obj->enchanted)
	{
	    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    {
		mod = paf->modifier;
		switch ( paf->location )
		{
		    case APPLY_AC:
			for ( i = 0; i < 4; i++ )
			    ch->armor[i] += mod;
			break;
		    default:
			break;
		}
	    }
	}
	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
	    mod = paf->modifier;
	    switch ( paf->location )
	    {
		case APPLY_AC:
		    for ( i = 0; i < 4; i++ )
			ch->armor[i] += mod;
		    break;
		default:
		    break;
	    }
	}
    }

    for ( paf = ch->affected; paf; paf = paf->next )
    {
	mod = paf->modifier;
	switch ( paf->location )
	{
	    case APPLY_AC:
		for ( i = 0; i < 4; i++ )
		    ch->armor[i] += mod;
		break;
	    default:
		break;
	}
    }	
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    obj->wear_loc	 = iWear;
    if ( (!IS_SET(obj->extra_flags, ITEM_MALE_ONLY) &&
	  !IS_SET(obj->extra_flags, ITEM_FEMALE_ONLY)) ||
	 (IS_SET(obj->extra_flags, ITEM_MALE_ONLY)
	  && TRUE_SEX( ch ) == SEX_MALE) ||
	 (IS_SET(obj->extra_flags, ITEM_FEMALE_ONLY)
	  && TRUE_SEX( ch ) == SEX_FEMALE) )
    {
        if (!obj->enchanted)
	    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	        affect_modify( ch, paf, TRUE );
        for ( paf = obj->affected; paf != NULL; paf = paf->next )
	    affect_modify( ch, paf, TRUE );
    }
    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    figure_carrying( ch );

    if ( obj->item_type == ITEM_ARMOR
    &&   obj->wear_loc  != WEAR_NONE )
	figure_ac( ch );

    return;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    obj->wear_loc	 = -1;
    if ( (!IS_SET(obj->extra_flags, ITEM_MALE_ONLY) &&
	  !IS_SET(obj->extra_flags, ITEM_FEMALE_ONLY)) ||
	 (IS_SET(obj->extra_flags, ITEM_MALE_ONLY)
	  && TRUE_SEX( ch ) == SEX_MALE) ||
	 (IS_SET(obj->extra_flags, ITEM_FEMALE_ONLY)
	  && TRUE_SEX( ch ) == SEX_FEMALE) )
    {
	if (!obj->enchanted)
	    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		affect_modify( ch, paf, FALSE );
	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	    affect_modify( ch, paf, FALSE );
    }
    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    figure_carrying( ch );

    if ( obj->item_type == ITEM_ARMOR
    &&   obj->wear_loc  != WEAR_NONE )
	figure_ac( ch );
    return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}


/*
 *   EXTRACTION FUNCTIONS
 */
/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    int sn;

    if ( obj == NULL )
    {
	bug( "Extract_obj: NULL obj", 0 );
	return;
    }

    sn = affect_lookup( obj->pIndexData->vnum );
    if ( IS_SET(obj->extra_flags, ITEM_CHANNELED) && sn != -1 )
    {
	void *vo;
	if ( obj->in_room != NULL )
	{
	    if ( obj->in_room->people != NULL )
	    {
		act( skill_table[sn].msg_off, obj->in_room->people,
		    obj, NULL, TO_ALL );
	    }
	}
	else if ( obj->carried_by != NULL )
	{
	    act( skill_table[sn].msg_off, obj->carried_by, obj, NULL,
		TO_ALL );
	}
	vo = (void *) obj;
	rem_weave_list( vo, NODE_WEAVE_CREATE );
    }

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    rem_weave_list( obj, NODE_WEAVE_OBJ );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    if ( obj->match != NULL )
    {
	OBJ_DATA *match;

	match = obj->match;
	match->match = NULL;
	obj->match = NULL;

	extract_obj( match );
    }
    --obj->pIndexData->count;

    free_obj( obj );
    return;
}

/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    NODE_DATA *list;

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )
	die_follower( ch );
    
    stop_fighting( ch, TRUE );
    rem_weave_list( ch, NODE_WEAVE_CHAR );

    if ( fPull )
    {
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
 	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}
    }

    if ( ch->pnote )
    {
	free_note( ch->pnote );
	ch->pnote           = NULL;
    }

    if ( ch->gen_data )
	free_gen_data( ch->gen_data );

    if ( ch->in_room != NULL )
	char_from_room( ch );

    if ( !IS_NPC(ch) )
	die_weave( ch );

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
	if ( wch->action_target == ch )
	{
	    send_to_char( "The person you are performing this action on has left.\r\n", ch );
	    free_action( wch );
	}
	if ( wch->memory && wch->memory->id == ch->id )
	{
	    free_mem_data( wch->memory );
	    wch->memory = NULL;
	}
	if ( wch->hunting == ch )
	    wch->hunting = NULL;
    }

    if ( !fPull )
    {
	char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	for ( list = pc_list; list; list = list->next )
	{
	    CHAR_DATA *wch;

	    if ( list->data_type != NODE_PC )
		continue;

	    wch = (CHAR_DATA *) list->data;

	    if ( IS_IN_GUILD(ch, guild_lookup("seanchan"))
	    &&   ch->pcdata->guild->damane  == wch )
	    {
	    	send_to_char( "The sensation at the other end of the a'dam ends.\n\r", ch );
	    	REMOVE_BIT(wch->affected_by, AFF_CHARM);
	    	REMOVE_BIT(wch->affected_by_2, AFF_LEASHED);
	    	affect_strip( wch, gsn_charm_person );
	    	affect_strip( wch, gsn_leashing );
	    }

	    if ( IS_IN_GUILD(wch, guild_lookup("seanchan"))
	    &&   wch->pcdata->guild->damane == ch )
	    {
	    	OBJ_DATA *obj;
	    	wch->pcdata->guild->damane = NULL;
	    	send_to_char( "The sensation at the other end of the a'dam ends.\n\r", ch );
	   	obj = get_bracelet( wch );
	    	act( "You remove $p.", ch, obj, NULL, TO_CHAR );
	    	extract_obj( obj );
	    }
	}
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	do_return( ch, "" );

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    if ( ch->mount != NULL )
    {
	ch->mount->rider = NULL;
	ch->mount = NULL;
    }

    if ( ch->rider != NULL )
    {
	ch->rider->mount = NULL;
	ch->rider = NULL;
    }

    if ( ch->desc )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   (can_see_obj( ch, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
	sprintf( buf, obj->short_descr, amount );
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
	obj->value[0]		= amount;
	obj->cost		= amount;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
 /*   int number;
 
    if ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number; */
    return 1;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    weight = obj->weight * break_table[obj->material].weight_mult / 100;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	weight += get_obj_weight( obj );

    return UMAX(weight, 1);
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_HERO(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 5 && !IS_IMMORTAL(ch))
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;

    if ( ch == NULL || victim == NULL )
	return FALSE;
    
    if ( !IS_NPC(victim)
    &&   IS_SET(victim->act, PLR_WIZINVIS)
    &&   get_trust( ch ) < victim->invis_level )
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_AWARENESS)
    &&   victim->fighting == NULL
    &&	 (IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim) ))
    {
	int chance;
	chance = get_skill(victim,gsn_sneak);
	chance += get_curr_stat(ch,STAT_DEX) * 3/2;
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance += ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_AWARENESS)
    &&   victim->fighting == NULL )
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( obj->item_type == ITEM_ROOM_AFFECT )
	return FALSE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ))
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    return TRUE;
}



/*
 * True if char can NOT drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    if ( is_guild_eq(obj->pIndexData->vnum) )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
    case ITEM_INGREDIENT:	return "ingredient";
    case ITEM_LIGHT:		return "light";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:		return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink container";
    case ITEM_KEY:		return "key";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc corpse";
    case ITEM_CORPSE_PC:	return "pc corpse";
    case ITEM_FOUNTAIN:		return "fountain";
    case ITEM_MAP:		return "map";
    case ITEM_CLOTHING:		return "clothing";
    case ITEM_FAQ:		return "faq";
    case ITEM_ORE:		return "ore";
    case ITEM_GATE:		return "gate";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_LUK:		return "luck";
    case APPLY_CHR:		return "charisma";
    case APPLY_AGI:		return "agility";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_HIT:		return "hp";
    case APPLY_STAMINA:		return "stamina";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SPELL:		return "adds spell affect";
    case APPLY_SKILL:		return "increases skill";
    case APPLY_SUBSKILL:	return "decreases skill";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_WRAP          ) strcat( buf, " wrapped"       );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_WARDED	    ) strcat( buf, " warded"	    );
    if ( vector & AFF_AWARENESS     ) strcat( buf, " aware" 	    );
    if ( vector & AFF_GAG           ) strcat( buf, " gagged"        );
    if ( vector & AFF_AIR_ARMOR     ) strcat( buf, " air_armor"     );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_STALK         ) strcat( buf, " stalk"         );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_BERSERK	    ) strcat( buf, " berserk"	    );
    if ( vector & AFF_CALM	    ) strcat( buf, " calm"	    );
    if ( vector & AFF_HASTE	    ) strcat( buf, " haste"	    );
    if ( vector & AFF_PLAGUE	    ) strcat( buf, " plague" 	    );
    if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
    if ( vector & AFF_SLOW          ) strcat( buf, " slowed"	    );
    if ( vector & AFF_SHAPE_CHANGE  ) strcat( buf, " disguised"	    );
    if ( vector & AFF_WARDED        ) strcat( buf, " warded"	    );
    return ( buf[0] != '\0' ) ? buf+1 : "";
}

char *affect_bit_name_2( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_STILL         ) strcat( buf, " still"		);
    if ( vector & AFF_LINK          ) strcat( buf, " link"		);
    if ( vector & AFF_NOAGGRO       ) strcat( buf, " noaggro"		);
    if ( vector & AFF_GRASP         ) strcat( buf, " grasping"		);
    if ( vector & AFF_CAN_LINK	    ) strcat( buf, " can_link"		);
    if ( vector & AFF_LEASHED	    ) strcat( buf, " leashed"		);
    if ( vector & AFF_HIDE_CHANNEL  ) strcat( buf, " hide-channel"	);
    if ( vector & AFF_CAPTURED	    ) strcat( buf, " captured"		);
    if ( vector & AFF_STOP_CHANNEL  ) strcat( buf, " stop_channel"	);
    return ( buf[0] != '\0' ) ? buf : "";
}

/*
 * Return ascii name of an affect bit vector.
 */
char *bit_name( AFFECT_DATA *paf )
{
    static char buf[512];
    int vector;

    buf[0] = '\0';
    vector = paf->bitvector;
    if ( vector & AFF_BLIND		) strcat( buf, " BLIND"		);
    if ( vector & AFF_SHIELDED		) strcat( buf, " SHIELDED"	);
    if ( vector & AFF_INVISIBLE		) strcat( buf, " INVISIBLE"	);
    if ( vector & AFF_WRAP		) strcat( buf, " WRAPPED"	);
    if ( vector & AFF_DETECT_INVIS	) strcat( buf, " DETECT_INVIS"	);
    if ( vector & AFF_WARDED		) strcat( buf, " WARDED"	);
    if ( vector & AFF_AWARENESS		) strcat( buf, " AWARE"		);
    if ( vector & AFF_GAG		) strcat( buf, " GAGGED"	);
    if ( vector & AFF_AIR_ARMOR		) strcat( buf, " AIR_ARMOR"	);
    if ( vector & AFF_POISON		) strcat( buf, " POISONED"	);
    if ( vector & AFF_STALK		) strcat( buf, " STALKING"	);
    if ( vector & AFF_SLEEP		) strcat( buf, " SLEEP"		);
    if ( vector & AFF_SNEAK		) strcat( buf, " SNEAKING"	);
    if ( vector & AFF_HIDE		) strcat( buf, " HIDING"	);
    if ( vector & AFF_CHARM		) strcat( buf, " CHARMED"	);
    if ( vector & AFF_FLYING		) strcat( buf, " FLYING"	);
    if ( vector & AFF_BERSERK		) strcat( buf, " BERSERKED"	);
    if ( vector & AFF_CALM		) strcat( buf, " CALMED"	);
    if ( vector & AFF_HASTE		) strcat( buf, " HASTED"	);
    if ( vector & AFF_PLAGUE		) strcat( buf, " PLAGUED"	);
    if ( vector & AFF_DARK_VISION	) strcat( buf, " DARK_VISION"	);
    if ( vector & AFF_SLOW		) strcat( buf, " SLOWED"	);
    if ( vector & AFF_SHAPE_CHANGE	) strcat( buf, " DISGUISED"	);
    if ( vector & AFF_HASTE		) strcat( buf, " HASTED"	);
    vector = paf->bitvector_2;
    if ( vector & AFF_STILL		) strcat( buf, " STILLED"	);
    if ( vector & AFF_LINK		) strcat( buf, " LINKED"	);
    if ( vector & AFF_NOAGGRO		) strcat( buf, " NO_AGGRO"	);
    if ( vector & AFF_GRASP		) strcat( buf, " GRASP"		);
    if ( vector & AFF_CAN_LINK		) strcat( buf, " CAN_LINK"	);
    if ( vector & AFF_LEASHED		) strcat( buf, " LEASHED"	);
    if ( vector & AFF_HIDE_CHANNEL	) strcat( buf, " HIDE_CHANNEL"	);
    if ( vector & AFF_CAPTURED		) strcat( buf, " CAPTURED"	);
    if ( vector & AFF_STOP_CHANNEL	) strcat( buf, " STOP_CHANNEL"	);
    return ( buf[0] != '\0' ) ? buf : "";
}

char *room_flag_name( int room_flag )
{
    static char buf[2048];
    buf[0] = '\0';

    if ( room_flag & ROOM_DARK		) strcat( buf, " dark"		);
    if ( room_flag & ROOM_NO_MOB	) strcat( buf, " no_mob"	);
    if ( room_flag & ROOM_INDOORS	) strcat( buf, " indoors"	);
    if ( room_flag & ROOM_PRIVATE	) strcat( buf, " private"	);
    if ( room_flag & ROOM_SAFE		) strcat( buf, " safe"		);
    if ( room_flag & ROOM_SOLITARY	) strcat( buf, " solitary"	);
    if ( room_flag & ROOM_PET_SHOP	) strcat( buf, " pet_shop"	);
    if ( room_flag & ROOM_NO_RECALL	) strcat( buf, " no_recall"	);
    if ( room_flag & ROOM_IMP_ONLY	) strcat( buf, " imp_only"	);
    if ( room_flag & ROOM_GODS_ONLY	) strcat( buf, " gods_only"	);
    if ( room_flag & ROOM_HEROES_ONLY	) strcat( buf, " heroes_only"	);
    if ( room_flag & ROOM_NEWBIES_ONLY	) strcat( buf, " newbies_only"	);
    if ( room_flag & ROOM_LAW		) strcat( buf, " law"		);
    if ( room_flag & ROOM_STEDDING	) strcat( buf, " stedding"	);
    if ( room_flag & ROOM_QUEST_SHOP	) strcat( buf, " quest-shop"	);
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *resource_string( int resource )
{
    static char buf[2048];
    buf[0] = '\0';

    if ( resource & RES_HERB		) strcat( buf, " herbs"		);
    if ( resource & RES_ORE		) strcat( buf, " ores"		);
    if ( resource & RES_LUMBER		) strcat( buf, " lumber"	);
    if ( resource & RES_GEMS		) strcat( buf, " gems"		);
    if ( resource & RES_FISH		) strcat( buf, " fish"		);
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


char *ingredient_string( int ingredient )
{
    static char buf[2048];
    buf[0] = '\0';

    if ( ingredient & HERB_ALDAKA	) strcat( buf, " aldaka"	);
    if ( ingredient & HERB_CUREALL	) strcat( buf, " cureall"	);
    if ( ingredient & HERB_BELRAMBA	) strcat( buf, " belramba"	);
    if ( ingredient & HERB_CATS_TAIL	) strcat( buf, " cat's_tail"	);
    if ( ingredient & HERB_DAGMATHER	) strcat( buf, " dagmather"	);
    if ( ingredient & HERB_FETHERFEW	) strcat( buf, " fetherfew"	);
    if ( ingredient & HERB_GOATS_RUE	) strcat( buf, " goat's_rue"	);
    if ( ingredient & HERB_HARES_EARS	) strcat( buf, " hare's_ears"	);
    if ( ingredient & HERB_HOREHOUND	) strcat( buf, " horehound"	);
    if ( ingredient & HERB_MASTERWORT	) strcat( buf, " masterwort"	);
    if ( ingredient & HERB_PARGEN	) strcat( buf, " pargen"	);
    if ( ingredient & HERB_SWEET_TREFOILE) strcat( buf, " sweet_trefoile");
    if ( ingredient & HERB_ORACH	) strcat( buf, " orach"		);
    if ( ingredient & HERB_MUGWORT	) strcat( buf, " mugwort"	);
    if ( ingredient & HERB_WILLOW_HERB	) strcat( buf, " willow_herb"	);
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " vis_death"	);
    if ( extra_flags & ITEM_BENT	 ) strcat( buf, " bent"	);
    if ( extra_flags & ITEM_RUINED	 ) strcat( buf, " broken"	);
    if ( extra_flags & ITEM_MALE_ONLY	 ) strcat( buf, " male-only"	);
    if ( extra_flags & ITEM_FEMALE_ONLY	 ) strcat( buf, " female-only"	);
    if ( extra_flags & ITEM_NOBREAK	 ) strcat( buf, " nobreak"	);
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    { 
 	strcat(buf," npc");
    	if (act_flags & ACT_SENTINEL 	) strcat(buf, " sentinel");
	if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " aggressive");
	if (act_flags & ACT_STAY_AREA	) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY	) strcat(buf, " wimpy");
	if (act_flags & ACT_PET		) strcat(buf, " pet");
	if (act_flags & ACT_TRAIN	) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE	) strcat(buf, " practice");
	if (act_flags & ACT_SCHOLAR	) strcat(buf, " scholar");
	if (act_flags & ACT_ROGUE	) strcat(buf, " rogue");
	if (act_flags & ACT_WARRIOR	) strcat(buf, " warrior");
	if (act_flags & ACT_NOALIGN	) strcat(buf, " no_align");
	if (act_flags & ACT_NOPURGE	) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER	) strcat(buf, " healer");
	if (act_flags & ACT_GAIN	) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
        if (act_flags & ACT_FORGE_WEAPON) strcat(buf," forge_weapon");
        if (act_flags & ACT_FORGE_ARMOR) strcat(buf," forge_armor");
        if (act_flags & ACT_MOUNT) strcat(buf," is_mount");

    }
    else
    {
	strcat(buf," player");
	if (act_flags & PLR_BOUGHT_PET	) strcat(buf, " owner");
	if (act_flags & PLR_AUTOASSIST	) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT	) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT	) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC	) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD	) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT	) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
	if (act_flags & PLR_WIZINVIS	) strcat(buf, " wizinvis");
	if (act_flags & PLR_CANLOOT	) strcat(buf, " loot_corpse");
	if (act_flags & PLR_NOFOLLOW	) strcat(buf, " no_follow");
	if (act_flags & PLR_LOG		) strcat(buf, " logged");
	if (act_flags & PLR_DENY	) strcat(buf, " denied");
	if (act_flags & PLR_FREEZE	) strcat(buf, " frozen");
	if (act_flags & PLR_ANSI	) strcat(buf, " ansi-on");
	if (act_flags & PLR_SUBDUE	) strcat(buf, " subdues");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET		) strcat(buf, " quiet");
    if (comm_flags & COMM_DEAF		) strcat(buf, " deaf");
    if (comm_flags & COMM_NOWIZ		) strcat(buf, " no_wiz");
    if (comm_flags & COMM_NOAUCTION	) strcat(buf, " no_auction");
    if (comm_flags & COMM_NOOOC		) strcat(buf, " no_ooc");
    if (comm_flags & COMM_NOBARD	) strcat(buf, " no_bard");
    if (comm_flags & COMM_NOGDT		) strcat(buf, " no_guild");
    if (comm_flags & COMM_NOWDT		) strcat(buf, " no_warder");
    if (comm_flags & COMM_NODFT		) strcat(buf, " no_darkfriend");
    if (comm_flags & COMM_COMPACT	) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF		) strcat(buf, " brief");
    if (comm_flags & COMM_PROMPT	) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE	) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE	) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT	) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL	) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " no_channels");
    if (comm_flags & COMM_NOTES		) strcat(buf, " no_notes");
    if (comm_flags & COMM_AFK		) strcat(buf, " afk");
    if (comm_flags & COMM_NOSPAM	) strcat(buf, " no_spam" );
    if (comm_flags & COMM_TANKCOND	) strcat(buf, " tankcond" );
    if (comm_flags & COMM_AUTOHEAL	) strcat(buf, " autoheal" );
    if (comm_flags & COMM_NOTICK	) strcat(buf, " no_tick" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM		) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH		) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH		) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE		) strcat(buf, " fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID		) strcat(buf, " acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY		) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE	) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT		) strcat(buf, " light");
    if (imm_flags & VULN_IRON		) strcat(buf, " iron");
    if (imm_flags & VULN_WOOD		) strcat(buf, " wood");
    if (imm_flags & VULN_SILVER	) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON	) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT	) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER	) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE	) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
    if (weapon_flags & WEAPON_SHEATHED	) strcat(buf, " sheathed");
    if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_POISON    ) strcat(buf, " poisoned");
    if (weapon_flags & WEAPON_NOFUMBLE  ) strcat(buf, " no-fumbled");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB	) strcat(buf, " backstab");
    if (off_flags & OFF_BASH		) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK		) strcat(buf, " berserk");
    if (off_flags & OFF_DISARM		) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE		) strcat(buf, " dodge");
    if (off_flags & OFF_FADE		) strcat(buf, " fade");
    if (off_flags & OFF_FAST		) strcat(buf, " fast");
    if (off_flags & OFF_KICK		) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT	) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY		) strcat(buf, " parry");
    if (off_flags & OFF_RESCUE		) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL		) strcat(buf, " tail");
    if (off_flags & OFF_TRIP		) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH		) strcat(buf, " crush");
    if (off_flags & ASSIST_ALL		) strcat(buf, " assist_all");
    if (off_flags & ASSIST_RACE		) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS	) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD	) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM		) strcat(buf, " assist_vnum");
    if (off_flags & ASSIST_GUILD	) strcat(buf, " assist_guild");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *color_value_string( int color, bool bold, bool flash )
{
    static char buf[64];
    static sh_int COLOR_INDEX[8] = { 30, 34, 32, 31, 36, 35, 33, 37 };

    if ( flash && bold )
        sprintf( buf, "\x01B[1;%dm%s", COLOR_INDEX[color % 8], FLASH );
    else if ( flash )
        sprintf( buf, "\x01B[0;%dm%s", COLOR_INDEX[color % 8], FLASH );
    else if ( bold )
        sprintf( buf, "%s\x01B[1;%dm", NTEXT, COLOR_INDEX[color % 8] );
    else
        sprintf( buf, "%s\x01B[0;%dm", NTEXT, COLOR_INDEX[color % 8] );

    return buf;
}

int strlen_color( char *argument )
{
    char        *str;
    sh_int      length;

    if ( argument==NULL || argument[0]=='\0' )
        return 0;

    length=0;
    str=argument;
    while ( *str != '\0' )
    {
        if ( *str != '`' )
        {
            str++;
            length++;
            continue;
        }

        switch ( *(++str) )
        {
            default:    length+=2;      break;
            case '-':
            case 'x':   length++;       break;
            case '<':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case 'B':
            case 'b':
            case 'F':
            case 'f':
            case 'n':                   break;
        }

        str++;
    }

    return length;
}

bool is_metal( OBJ_DATA *obj )
{
    int mat;
    mat = obj->material;

    switch (mat)
    {
	case MAT_GOLD:
	case MAT_IRON:
	case MAT_SILVER:
	case MAT_STEEL:
	case MAT_DARKSILVER:
	case MAT_COPPER:
	case MAT_HEARTSTONE:
	    return(TRUE);
	default:
	    return(FALSE);
    }
    return(FALSE);
}

bool is_warder( CHAR_DATA *ch )
{
    if ( ch->guild == guild_lookup("warder") )
	return TRUE;

    if ( ch->guild == guild_lookup("aes-sedai") )
	return TRUE;

    if ( IS_NPC(ch) )
	return FALSE;

    if ( !IS_NULLSTR(ch->pcdata->sedai) )
	return TRUE;

    return FALSE;
} 

bool is_darkfriend( CHAR_DATA *ch )
{
    if ( ch->guild == guild_lookup("darkfriend") )
	return TRUE;

    if ( IS_NPC(ch) )
	return FALSE;

    if ( ch->pcdata->shadow_rank >= 0 )
	return TRUE;

    return FALSE;
}

bool is_protected( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) )
	return TRUE;

    if ( IS_IMMORTAL(ch) )
	return TRUE;

    if ( TRUE_SEX(ch) != SEX_MALE )
	return TRUE;

    if ( IS_GUILDED(ch) )
    {
	if ( ch->guild == guild_lookup("darkfriend")
	&&   (GET_RANK(ch, 1) == 9
	||    GET_RANK(ch, 1) == 10) )
	    return TRUE;
    }
    if ( ch->pcdata->shadow_rank == 9 )
	return TRUE;

    if ( LOWER(ch->name[0]) == 'r'
    &&   !str_cmp(ch->name, "Rand") )
	return TRUE;

    return FALSE;
}

void die_weave( CHAR_DATA *ch )
{
    NODE_DATA *node, *node_next;
    AFFECT_DATA *paf, *paf_next;
    CHAR_DATA *vch;
    OBJ_DATA *vobj;
    ROOM_INDEX_DATA *vroom;
 
    for ( node = weave_list; node != NULL; node = node_next )
    {
	node_next = node->next;
	if ( node->data_type == NODE_WEAVE_CHAR )
	{
	    vch		= (CHAR_DATA *) node->data;
	    for ( paf = vch->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->owner == ch && !IS_TIED(paf) )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, vch );
			send_to_char( "\n\r", vch );   
		    }
		    affect_remove( vch, paf );
		}
	    }
	}
        if ( node->data_type == NODE_WEAVE_OBJ )
        {
	    vobj        = (OBJ_DATA *) node->data;
	    for ( paf = vobj->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->owner == ch && !IS_TIED(paf) )
		    affect_obj_remove( vobj, paf );
	    }
	}
	if ( node->data_type == NODE_WEAVE_ROOM )
	{
	    vroom       = (ROOM_INDEX_DATA *) node->data;
	    for ( paf = vroom->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->owner == ch && !IS_TIED(paf) )
		    affect_room_remove( vroom, paf );
	    }
	}
	if ( node->data_type == NODE_WEAVE_CREATE )
	{
	    vobj = (OBJ_DATA *) node->data;
	    if ( vobj->owner == ch && !IS_SET(vobj->extra_flags, ITEM_TIED) )
		extract_obj( vobj );
	}
    }
    return;
}

int channel_strength( CHAR_DATA *ch, int power )
{
    int value = 0;
    int count = 0;

    if ( IS_NPC(ch) )
    {
	if ( !IS_SET(ch->act, ACT_CHANNELER) )
	    return 0;
	if ( power & POWER_EARTH )
	{
	    count++;
	    value += ch->channel_skill[0];
	}
	if ( power & POWER_AIR )
	{
	    count++;
	    value += ch->channel_skill[1];
	}
	if ( power & POWER_FIRE )
	{
	    count++;
	    value += ch->channel_skill[2];
	}
	if ( power & POWER_WATER )
	{
	    count++;
	    value += ch->channel_skill[3];
	}
	if ( power & POWER_SPIRIT )
	{
	    count++;
	    value += ch->channel_skill[4];
	}

	if ( value == 0 || count == 0 )
	    return 0;
	return( value / count );
    }

    if ( power & POWER_EARTH )
    {
	count++;
	value += SKILL(ch,gsn_earth);
	if ( ch->pcdata->talent[tn_earth_talent] )
	    value += 5;
    }
    if ( power & POWER_AIR )
    {
	count++;
	value += SKILL(ch,gsn_air);
	if ( ch->pcdata->talent[tn_air_talent] )
	    value += 5;
    }
    if ( power & POWER_FIRE )
    {
	count++;
	value += SKILL(ch,gsn_fire);
	if ( ch->pcdata->talent[tn_fire_talent] )
	    value += 5;
    }
    if ( power & POWER_WATER )
    {
	count++;
	value += SKILL(ch,gsn_water);
	if ( ch->pcdata->talent[tn_water_talent] )
	    value += 5;
    }
    if ( power & POWER_SPIRIT )
    {
	count++;
	value += SKILL(ch,gsn_spirit);
	if ( ch->pcdata->talent[tn_spirit_talent] )
	    value += 5;
    }

    if ( IS_AFFECTED_2(ch, AFF_LINK)
    &&   ch->master == NULL
    &&   (ch->leader == NULL || ch->leader == ch) )
    {
	CHAR_DATA *gch;

	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	{
	    if ( IS_AFFECTED_2(gch, AFF_LINK)
	    &&   gch != ch
	    &&   is_same_group(ch, gch)
	    &&   gch->leader == ch )
		value += channel_strength( gch, power ) / 5;
	}
    }

    if ( value == 0 || count == 0 )
	return 0;
    return( value / count );
}

bool can_channel( CHAR_DATA *ch, int skill )
{
    if ( IS_NPC(ch) )
    {
	if ( !IS_SET(ch->act, ACT_CHANNELER) )
	    return FALSE;

	if ( ch->channel_skill[0] >= skill )
	    return TRUE;

	if ( ch->channel_skill[1] >= skill )
	    return TRUE;

	if ( ch->channel_skill[2] >= skill )
	    return TRUE;

	if ( ch->channel_skill[3] >= skill )
	    return TRUE;

	if ( ch->channel_skill[4] >= skill )
	    return TRUE;

	return FALSE;
    }

    if ( ch->pcdata->learned[gsn_earth] >= skill )
	return TRUE;
    if ( ch->pcdata->learned[gsn_air] >= skill )
	return TRUE;
    if ( ch->pcdata->learned[gsn_fire] >= skill )
	return TRUE;
    if ( ch->pcdata->learned[gsn_water] >= skill )
	return TRUE;
    if ( ch->pcdata->learned[gsn_spirit] >= skill )
	return TRUE;

    return FALSE;
}


bool lose_stamina( CHAR_DATA *ch, int loss, bool use_weight, bool force_loss )
{
    int carry, enc;
    int before, after;

    carry = ch->carry_weight * 100 / can_carry_w( ch );
    if ( carry >= 100 )
	enc = 8;
    else if ( carry >= 90 )
	enc = 6;
    else if ( carry >= 75 )
	enc = 3;
    else if ( carry >= 60 )
	enc = 2;
    else
	enc = 1;

    if ( IS_AFFECTED(ch, AFF_BERSERK) )
	loss *= 2;

    if ( use_weight )
	loss *= enc;

    if ( (ch->stamina - loss) < 0 && !force_loss )
	return FALSE;

    before = stamina_status( ch );
    ch->stamina -= loss;
    after = stamina_status( ch );
    if ( before >= 1 && after == 0 )
	send_to_char( "`3You are exhausted!`n\n\r", ch );
    return TRUE;
}

bool gain_stamina( CHAR_DATA *ch, int gain, bool force_gain )
{
    if ( ch->stamina >= ch->max_stamina && !force_gain )
	return FALSE;

    if ( (ch->stamina + gain) > ch->max_stamina && !force_gain )
    {
	ch->stamina = ch->max_stamina;
	return TRUE;
    }

    ch->stamina += gain;
    return TRUE;
}

bool lose_health( CHAR_DATA *ch, int loss, bool force_loss )
{
    ch->hit -= loss;
    update_pos( ch );

    if ( loss == 0 )
	return TRUE;

    if ( !IS_NPC(ch) && ch->guild == guild_lookup( "aes-sedai" ))
    {
	CHAR_DATA *warder;
	char *temp;
	char name[MAX_INPUT_LENGTH];

        temp = one_argument(ch->pcdata->guild->warder, name);
        while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) )
        {
	    if ( (warder = get_char_sedai( ch, name ))== NULL )
	    {
		temp = one_argument(temp, name);
		continue;
	    }
	    act ("`3You flinch, $n seems to have been hurt!`n", ch, NULL,
		warder, TO_VICT );
            temp = one_argument(temp, name);
        }
    }

    if ( IS_IN_GUILD(ch, guild_lookup("seanchan")) )
    {
	if ( GET_RANK(ch,1) == 7 && !IS_AFFECTED_2(ch,AFF_LEASHED) )
	{
	    if ( ch->pcdata->guild->damane != NULL )
	    {
		int damane_loss;
		damane_loss = URANGE(1, loss/10, 30);
		send_to_char( "Your body is filled with pain!\n\r",
		    ch->pcdata->guild->damane );
		lose_health( ch->pcdata->guild->damane, damane_loss / 2, TRUE );
		lose_stamina( ch->pcdata->guild->damane, damane_loss * 3 / 2, FALSE, TRUE );
	    }
	}
    }
    switch( ch->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    ch );
	break;
    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    ch );
	break;
    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    ch );
	break;
    case POS_DEAD:
	act( "$n is DEAD!!", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You have been `3KILLED`n!!\n\r\n\r", ch );
	break;
    default:
	if ( loss > ch->max_hit / 4 )
	    send_to_char( "`3That really did HURT!`n\n\r", ch );
	if ( ch->hit < ch->max_hit / 4 )
	    send_to_char( "`3You sure are BLEEDING!`n\n\r", ch );
	break;
    }
    if ( ch->position == POS_DEAD )
	return FALSE;

    return TRUE;
}

bool gain_health( CHAR_DATA *ch, int gain, bool force_gain )
{
    if ( ch->hit >= ch->max_hit && !force_gain )
	return FALSE;

    if ( (ch->hit + gain) > ch->max_hit && !force_gain )
    {
	ch->hit = ch->max_hit;
	return TRUE;
    }
    ch->hit += gain;
    update_pos( ch );
    return TRUE;
}

int health_status( CHAR_DATA *ch )
{
    int percent;

    if ( ch->hit <= 0 )
	return 0;

    percent = health_percent( ch );

    if ( percent >= 50 )
	return 3;
    else if ( percent >= 25 )
	return 2;
    else
	return 1;

    return 3;
}

int health_percent( CHAR_DATA *ch )
{
    if ( ch->max_hit <= 0 )
	return 100;

    if ( ch->hit < 0 )
	return 0;

    return (ch->hit * 100 / ch->max_hit);
}

int stamina_status( CHAR_DATA *ch )
{
    int lower, middle, upper, lower_percent;

    if ( ch->stamina <= 0 )
	return 0;

    if ( ch->max_stamina <= 0 )
	return 0;

    lower_percent = 25 - get_curr_stat(ch, STAT_CON) / 3;
    lower = ch->max_stamina * lower_percent / 100 - ch->max_stamina / 10;
    lower  = UMAX(1, lower);
    middle = UMAX(1, lower * 2);
    upper  = UMAX(1, lower * 3);

    if ( ch->stamina >= upper )
	return 3;
    else if ( ch->stamina >= middle )
	return 2;
    else if ( ch->stamina >= lower )
	return 1;
    else
	return 0;

    return 3;
}

int stamina_percent( CHAR_DATA *ch )
{
    if ( ch->max_stamina == 0 )
	return 100;

    if ( ch->stamina < 0 )
	return 0;

    return (ch->stamina * 100 / ch->max_stamina);
}

bool has_color( const char *str )
{
    int i;
    for ( i = 0; str[i] != '\0'; i++ )
	if ( str[i] == '`' )
	    return TRUE;

    return FALSE;
}

bool cure_condition( CHAR_DATA *ch, int condition, int chance )
{
    AFFECT_DATA *paf, *paf_next;

    if ( number_percent() >= chance )
	return FALSE;

    switch ( condition )
    {
	default:
	    return FALSE;
	case BODY_RIGHT_LEG:
	    if ( !IS_SET(ch->body, BODY_RIGHT_LEG) )
		return FALSE;
	    send_to_char( "The bones in your right leg are Healed.\n\r",ch );
	    REMOVE_BIT( ch->body, BODY_RIGHT_LEG );
	    break;
	case BODY_LEFT_LEG:
	    if ( !IS_SET(ch->body, BODY_LEFT_LEG) )
		return FALSE;
	    send_to_char( "The bones in your left leg are Healed.\n\r",ch );
	    REMOVE_BIT( ch->body, BODY_LEFT_LEG );
	    break;
	case BODY_RIGHT_ARM:
	    if ( !IS_SET(ch->body, BODY_RIGHT_ARM) )
		return FALSE;
	    send_to_char( "The bones in your right arm are Healed.\n\r",ch );
	    REMOVE_BIT( ch->body, BODY_RIGHT_ARM );
	    break;
	case BODY_LEFT_ARM:
	    if ( !IS_SET(ch->body, BODY_LEFT_ARM) )
		return FALSE;
	    send_to_char( "The bones in your left arm are Healed.\n\r",ch );
	    REMOVE_BIT( ch->body, BODY_LEFT_ARM );
	    break;
	case BODY_BLEEDING:
	    if ( !IS_SET(ch->body, BODY_BLEEDING) )
		return FALSE;
	    send_to_char( "Your bleeding has stopped.\n\r", ch );
	    REMOVE_BIT( ch->body, BODY_BLEEDING );
	    break;
	case BODY_BLIND:
	    for ( paf = ch->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->type == gsn_blindness )
		{
		    if ( (paf->owner == NULL
		    ||    IS_SET( paf->flags, AFFECT_NOTCHANNEL ))
		    &&   number_percent() <= chance )
		    {
			affect_strip( ch, gsn_blindness );
			if ( skill_table[gsn_blindness].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_blindness].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			return TRUE;
		    }
		    else if ( paf->owner != NULL
		    &&	      number_percent() <= (chance - paf->strength) )
 		    {
			affect_strip( ch, gsn_blindness );
			if ( skill_table[gsn_blindness].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_blindness].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			act( "You feel your $t weave on $N fade.",
			    paf->owner, skill_table[gsn_blindness].name,
			    ch, TO_CHAR );
			return TRUE;
		    }
		}
	    }
	    return FALSE;
	    break;
	case BODY_DISEASE:
	    for ( paf = ch->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->type == gsn_plague )
		{
		    if ( (paf->owner == NULL
		    ||    IS_SET( paf->flags, AFFECT_NOTCHANNEL ))
		    &&   number_percent() <= chance )
		    {
			affect_strip( ch, gsn_plague );
			if ( skill_table[gsn_plague].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_plague].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			return TRUE;
		    }
		    else if ( paf->owner != NULL
		    &&	      number_percent() <= (chance - paf->strength) )
 		    {
			affect_strip( ch, gsn_plague );
			if ( skill_table[gsn_plague].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_plague].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			act( "You feel your $t weave on $N fade.",
			    paf->owner, skill_table[gsn_plague].name,
			    ch, TO_CHAR );
			return TRUE;
		    }
		}
	    }
	    return FALSE;
	    break;
	case BODY_POISON:
	    for ( paf = ch->affected; paf != NULL; paf = paf_next )
	    {
		paf_next = paf->next;
		if ( paf->type == gsn_poison )
		{
		    if ( (paf->owner == NULL
		    ||    IS_SET( paf->flags, AFFECT_NOTCHANNEL ))
		    &&   number_percent() <= chance )
		    {
			affect_strip( ch, gsn_poison );
			if ( skill_table[gsn_poison].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_poison].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			return TRUE;
		    }
		    else if ( paf->owner != NULL
		    &&	      number_percent() <= (chance - paf->strength) )
 		    {
			affect_strip( ch, gsn_poison );
			if ( skill_table[gsn_poison].msg_off[0] != '!' )
			{
			    send_to_char( skill_table[gsn_poison].msg_off, ch );
			    send_to_char( "\n\r", ch );
			}
			act( "You feel your $t weave on $N fade.",
			    paf->owner, skill_table[gsn_poison].name,
			    ch, TO_CHAR );
			return TRUE;
		    }
		}
	    }
	    return FALSE;
	    break;
    }
    return TRUE;
}

bool check_stat( CHAR_DATA *ch, int stat, int mod )
{
    int value;
    int roll;

    roll = number_range( 1, 20 );

    value = get_curr_stat(ch, stat) + mod;
    if ( roll > value || roll == 20 )
	return FALSE;

    return TRUE;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
        if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
        && !str_prefix(name,wiznet_table[flag].name))
            return flag;
    }
            
    return -1;
}
    

bool is_forsaken( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) )
	return FALSE;

    if ( ch->guild != guild_lookup("darkfriend") )
	return FALSE;

    if ( GET_RANK(ch, 1) < 5 )
	return FALSE;

    return TRUE;
}

bool is_collared( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( IS_NPC(ch) )
	return FALSE;

    if ( IS_AFFECTED_2(ch, AFF_LEASHED) )
	return FALSE;

    if ( ch->guild == guild_lookup("seanchan")
    &&   GET_RANK(ch, 1) == 0 )
	return TRUE;

    if ( (obj = get_eq_char(ch, WEAR_NECK_1)) )
    {
	if ( obj->pIndexData->vnum == OBJ_VNUM_ADAM_COLLAR
	&& IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	    return TRUE;
    }

    if ( (obj = get_eq_char(ch, WEAR_NECK_2)) )
    {
	if ( obj->pIndexData->vnum == OBJ_VNUM_ADAM_COLLAR
	&& !IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
	    return TRUE;
    }

    return FALSE;
}

bool eq_is_free( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *vobj;
    int val;
    int j;
    int count[2];

    if ( obj->item_type != ITEM_ARMOR )
	return TRUE;

    count[0] = 0;
    count[1] = 0;

    val =  obj->value[4];
    for ( vobj = ch->carrying; vobj != NULL; vobj = vobj->next_content )
    {
	if ( vobj == obj )
	    continue;

	if ( vobj->wear_loc < 0 )
	    continue;

	if ( vobj->item_type != ITEM_ARMOR )
	    continue;

	if ( !str_cmp("none", flag_string( armor_flags, vobj->value[4] )) )
	    continue;

	j = vobj->value[4];
	if ( val & j )
	{
	    if ( vobj->wear_loc == WEAR_NECK_1
	    ||   vobj->wear_loc == WEAR_NECK_2 )
	    {
		count[0]++;
		if ( count[0] == 2 )
		    return FALSE;
	    }
	    else if ( vobj->wear_loc == WEAR_WRIST_L
	    ||        vobj->wear_loc == WEAR_WRIST_R )
	    {
		count[1]++;
		if ( count[1] == 2 )
		    return FALSE;
 	    }
	    else
		return FALSE;
	}
    }

    return TRUE;
}



void figure_carrying( CHAR_DATA *ch )
{
    OBJ_DATA *obj, *obj_next;
    int number = 0, weight = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if (obj->wear_loc != WEAR_NONE)
	    continue;

	number++;
	weight += get_obj_weight( obj );
    }

    ch->carry_number = number;
    ch->carry_weight = weight + ( ch->gold / 500 );

    return;
}

CHAR_DATA *carrier( OBJ_DATA *obj )
{
  OBJ_DATA *tmp_obj;
  CHAR_DATA *ch;

  if ( obj->in_room != NULL )
    return NULL;

  ch = obj->carried_by;

  return NULL;	/* temp fix */

  for ( tmp_obj = obj->in_obj;
	ch == NULL && tmp_obj->in_obj != NULL;
	tmp_obj = tmp_obj->in_obj) {

    ch = tmp_obj->carried_by;
    if ( tmp_obj->in_room != NULL )
      return NULL;
    }

  return ch;
}

char *personal( CHAR_DATA *ch, CHAR_DATA *looker )
{
    static char buf[1024];
    buf[0] = '\0';

    if ( !can_see(looker, ch) )
	return "someone";

    if ( IS_DISGUISED(ch) && !IS_NULLSTR(ch->pcdata->new_name) )
    {
	if ( IS_IMMORTAL(looker) )
	{
	    strcat( buf, "(Disguised: " );
	    if ( IS_NPC(ch) )
		strcat( buf, ch->short_descr );
	    else
		strcat( buf, ch->name );
	    strcat( buf, ") " );
	}
	strcat( buf, ch->pcdata->new_name );
	return( buf );
    }

    if ( IS_NPC(ch) )
	return( ch->short_descr );
    else
	return( ch->name );
}


OBJ_DATA *get_bracelet( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( (obj = get_eq_char(ch, WEAR_WRIST_L)) )
    {
	if ( obj->pIndexData->vnum == 4293 )
	    return obj;
    }

    if ( (obj = get_eq_char(ch, WEAR_WRIST_R)) )
    {
	if ( obj->pIndexData->vnum == 4293 )
	    return obj;
    }

    return NULL;
}

CHAR_DATA *get_suldam( CHAR_DATA *ch )
{
    NODE_DATA *node;

    for ( node = pc_list; node; node = node->next )
    {
	CHAR_DATA *person;

	if ( node->data_type != NODE_PC )
	    continue;

	person = (CHAR_DATA *) node->data;
	if ( IS_IN_GUILD(person, guild_lookup("seanchan"))
	&&   person->pcdata->guild->damane == ch
	&&   person->in_room == ch->in_room )
	    return(person);
    }

    return NULL;
}


OBJ_DATA *get_collar( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == 4294 )
	    return obj;
    }

    return NULL;
}


bool is_channel_skill( int sn )
{
    if ( sn == gsn_earth
    ||   sn == gsn_air
    ||   sn == gsn_fire
    ||   sn == gsn_water
    ||   sn == gsn_spirit
    ||   sn == gsn_tie_weave
    ||   sn == gsn_invert_weave )
	return TRUE;

    if ( skill_table[sn].spell_fun != spell_null )
	return TRUE;

    return FALSE;
}


int hand_count( CHAR_DATA *ch )
{
    int count = 0;
    OBJ_DATA *obj;

    if ( (obj = get_eq_char(ch, WEAR_SHIELD))
    &&   !IS_SET(obj->extra_flags, ITEM_SMALL_SHIELD) )
	count++;
    if ( get_eq_char(ch, WEAR_WIELD) )
	count++;
    if ( get_eq_char(ch, WEAR_HOLD) )
	count++;
    if ( get_eq_char(ch, WEAR_SECONDARY) )
	count++;

    return count;
}


void break_con( CHAR_DATA *ch )
{
    if ( ch->action_timer == 0 )
	return;

    free_action( ch );
    act( "$y lose$% $o concentration in the task at hand and must start over.",
	ch, NULL, NULL, TO_ALL );
    return;
}

