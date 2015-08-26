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
#include <sys/resource.h>
#endif

#include "merc.h"
#include "mem.h"
#include "db.h"

void save_area( AREA_DATA *pArea );
char *flag_string               args ( ( const struct flag_type *flag_table,
                                         int bits ) );
void add_explore( );
int flag_value                  args ( ( const struct flag_type *flag_table,
                                         char *argument) );

/*
 #if defined(unix)
 extern int getrlimit(int resource, struct rlimit *rlp);
 extern int setrlimit(int resource, struct rlimit *rlp);
 #endif
*/

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif



/*
 * Globals. - now declared in mem.c
 */

CHAR_DATA *		char_list;
NODE_DATA *		fight_list;
NODE_DATA *		weave_list;
NODE_DATA *		wait_list;
NODE_DATA *		pc_list;
NODE_DATA *		room_list;
GEN_DATA *		gen_list;
NOTE_DATA *		note_list;
OBJ_DATA *		object_list;
BAN_DATA *		ban_list;
NODE_DATA *		spec_list;

TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

char *			help_greeting;
char			bug_buf		[2*MAX_INPUT_LENGTH];
char			log_buf		[2*MAX_INPUT_LENGTH];
KILL_DATA		kill_table	[MAX_LEVEL];

int			num_fish;
int			num_herb;
int			num_gem;
int			num_lumber;
int			num_ore;
int			num_explore;

/*
 *  Talent tns
 */

sh_int			tn_str_talent;
sh_int			tn_int_talent;
sh_int			tn_wis_talent;
sh_int			tn_dex_talent;
sh_int			tn_con_talent;
sh_int			tn_cha_talent;
sh_int			tn_luc_talent;
sh_int			tn_agi_talent;

sh_int			tn_herbalism;
sh_int			tn_geology;
sh_int			tn_teaching;

sh_int			tn_blademastery;
sh_int			tn_master_thief;
sh_int			tn_craftsmanship;

sh_int			tn_earth_talent;
sh_int			tn_air_talent;
sh_int			tn_fire_talent;
sh_int			tn_water_talent;
sh_int			tn_spirit_talent;

sh_int			tn_sense_taveren;
sh_int			tn_sense_terangreal;
sh_int			tn_create_terangreal;
sh_int			tn_dreamwalking;
sh_int			tn_foretelling;
sh_int			tn_shielding;
sh_int			tn_charm;
sh_int			tn_combat;
sh_int			tn_creation;
sh_int			tn_detection;
sh_int			tn_enchantment;
sh_int			tn_healing;
sh_int			tn_maladictions;
sh_int			tn_protective;
sh_int			tn_transportation;
sh_int			tn_weather;
sh_int			tn_channeler;
sh_int			tn_powerful_channeler;

sh_int			tn_weak_taveren;
sh_int			tn_taveren;
sh_int			tn_strong_taveren;

/*
 *  Skill gsns
 */

sh_int			gsn_backstab;
sh_int			gsn_dodge;
sh_int			gsn_hide;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_sneak;
sh_int			gsn_steal;

sh_int			gsn_disarm;
sh_int			gsn_enhanced_damage;
sh_int			gsn_kick;
sh_int			gsn_parry;
sh_int			gsn_rescue;
sh_int			gsn_shield;

sh_int			gsn_blindness;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_invis;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_plague;
sh_int			gsn_sleep;

/* new gsns */

sh_int  		gsn_axe;
sh_int  		gsn_dagger;
sh_int  		gsn_flail;
sh_int  		gsn_mace;
sh_int  		gsn_polearm;
sh_int			gsn_shield_block;
sh_int  		gsn_spear;
sh_int  		gsn_staff;
sh_int  		gsn_sword;
sh_int  		gsn_whip;
 
sh_int  		gsn_bash;
sh_int  		gsn_berserk;
sh_int  		gsn_dirt;
sh_int  		gsn_hand_to_hand;
sh_int  		gsn_trip;
sh_int			gsn_weapon_prof;
sh_int			gsn_martial_arts;
sh_int			gsn_shield_bash;
 
sh_int  		gsn_fast_healing;
sh_int  		gsn_haggle;
sh_int  		gsn_lore;
sh_int  		gsn_concentration;

sh_int			gsn_geography;
sh_int			gsn_foraging;
sh_int			gsn_acrobatics;
sh_int			gsn_old_tongue;
sh_int			gsn_hail_storm;
sh_int			gsn_lightning_storm;
sh_int			gsn_create_flame_sword;
sh_int			gsn_create_air_sword;
sh_int			gsn_create_spring;
sh_int			gsn_travel;
sh_int			gsn_skim;
sh_int			gsn_brew;
sh_int			gsn_herbalism;
sh_int			gsn_forestry;
sh_int			gsn_mining;

/* Newer gsns */
sh_int			gsn_shape_change;
sh_int			gsn_earth;
sh_int			gsn_air;
sh_int			gsn_fire;
sh_int			gsn_water;
sh_int			gsn_spirit;
sh_int			gsn_fourth_attack;
sh_int			gsn_stalk;
sh_int			gsn_teaching;
sh_int			gsn_gambling;
sh_int			gsn_disguise;
sh_int			gsn_haste;
sh_int			gsn_air_armor;
sh_int			gsn_ward_person;
sh_int			gsn_ambidexterity;
sh_int			gsn_dual_wield;
sh_int			gsn_set_snare;
sh_int			gsn_hunt;
sh_int			gsn_swimming;
sh_int			gsn_riding;
sh_int			gsn_medicine;
sh_int			gsn_tie_weave;
sh_int			gsn_invert_weave;
sh_int			gsn_summon_wolf;
sh_int			gsn_envenom;
sh_int			gsn_feint;
sh_int			gsn_forms;
sh_int			gsn_riposte;
sh_int			gsn_switch_opponent;
sh_int			gsn_heroic_rescue;
sh_int			gsn_pilfer;
sh_int			gsn_blindfighting;
sh_int			gsn_wind_barrier;
sh_int			gsn_earth_barrier;
sh_int			gsn_fire_wall;
sh_int			gsn_ice_wall;
sh_int			gsn_incognito;

/* Aes Sedai gsns */
sh_int			gsn_detect_shadowspawn;
sh_int			gsn_detect_channeler;
sh_int			gsn_teach_channeling;
sh_int			gsn_flash_strike;
sh_int			gsn_sweep;
sh_int			gsn_identify_fauna;
sh_int			gsn_endurance;
sh_int			gsn_complaisance;
sh_int			gsn_leashing;
sh_int			gsn_hardiness;
sh_int			gsn_spear_dancing;
sh_int			gsn_capture;
sh_int			gsn_group_fighting;
sh_int			gsn_gemworking;
sh_int			gsn_sewing;
sh_int			gsn_smithing;
sh_int			gsn_carpentry;
sh_int			gsn_leatherworking;
sh_int			gsn_repairing;
sh_int			gsn_balefire;
sh_int			gsn_grasping;
sh_int			gsn_linking;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
TEXT_DATA	*	text_hash		[MAX_KEY_HASH];
char *			string_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_guild;
int			top_help;
int			top_mob_index;
int			top_node;
int			top_note;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int                     top_vnum_room;  /* OLC */
int                     top_vnum_mob;   /* OLC */
int                     top_vnum_obj;   /* OLC */
int			top_text;
int			top_guard;

int 			mobile_count = 0;
int 			object_count = 0;
int			newmobs = 0;
int			newobjs = 0;


/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING	2303952
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	11

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;



/*
 * Semi-locals.
 */
bool			fBootDb;
bool			fPlaceTree;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];



/*
 * Local booting procedures.
*/
void    init_mm         args( ( void ) );
void	load_area	args( ( FILE *fp ) );
void    new_load_area   args( ( FILE *fp ) );   /* OLC */
void	load_helps	args( ( FILE *fp ) );
void 	load_mobiles	args( ( FILE *fp ) );
void 	load_objects	args( ( FILE *fp ) );
void	load_resets	args( ( FILE *fp ) );
void	load_rooms	args( ( FILE *fp ) );
void	load_shops	args( ( FILE *fp ) );
void 	load_socials	args( ( FILE *fp ) );
void	load_specials	args( ( FILE *fp ) );
void	load_notes	args( ( void ) );
void	load_guilds	args ( ( void ) );
void    load_bans       args( ( void ) );
void	load_text	args( ( FILE *fp ) );

void	place_tree	args( ( ROOM_INDEX_DATA *pRoom ) );

void	fix_exits	args( ( void ) );

void	reset_area	args( ( AREA_DATA * pArea ) );

#if defined(unix)
/* RT max open files fix */
 
void maxfilelimit()
{
    struct rlimit r;
 
    getrlimit(RLIMIT_NOFILE, &r);
    r.rlim_cur = r.rlim_max;
    setrlimit(RLIMIT_NOFILE, &r);
}
#endif

/*
 * Big mama top level function.
 */
void boot_db( void )
{

#if defined(unix)
    /* open file fix */
    maxfilelimit();
#endif

    fprintf( stderr, "Boot_db: Entering\n" );
    /*
     * Init some data space stuff.
     */
    {
	if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
	{
	    bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
	    exit( 1 );
	}
	top_string	= string_space;
	fBootDb		= TRUE;
    }
    fprintf( stderr, "Boot_db: Setting randomizer\n" );

    /*
     * Init random number generator.
     */
    {
        init_mm( );
    }

    fprintf( stderr, "Boot_db: Setting weather\n" );
    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;

	lhour		= (current_time - 650336715)
			/ (PULSE_HOUR / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.moonlight	= number_range( 0, 29 );

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }

    fprintf( stderr, "Boot_db: Assigning GSNs\n" );

    /*
     * Assign gsn's for skills and talents which have them.
     */
    {
	int sn;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].pgsn != NULL )
		*skill_table[sn].pgsn = sn;
	}
    }
    {
	int tn;

	for ( tn = 0; tn < MAX_TALENT; tn++ )
	{
	    if ( talent_table[tn].pgsn != NULL )
		*talent_table[tn].pgsn = tn;
	}
    }
    fprintf( stderr, "Boot_db: Reading guilds\n" );
    load_guilds( );
    fprintf( stderr, "Boot_db: Reading areas\n" );

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
	{
	    perror( AREA_LIST );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    if ( strArea[0] == '-' )
	    {
		fpArea = stdin;
	    }
	    else
	    {
		if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
		{
		    perror( strArea );
		    exit( 1 );
		}
	    }
	    fprintf( stderr, "Boot_db: Reading area \'%s\'\n", strArea );

	    for ( ; ; )
	    {
		char *word;

		if ( fread_letter( fpArea ) != '#' )
		{
		    bug( "Boot_db: # not found.", 0 );
		    exit( 1 );
		}

		word = fread_word( fpArea );

		     if ( word[0] == '$'               )                 break;
		else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
		else if ( !str_cmp( word, "AREADATA" ) ) new_load_area(fpArea);
		else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
		else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
	  	else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
		else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
		else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
		else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
		else if ( !str_cmp( word, "SOCIALS"  ) ) load_socials (fpArea);
		else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
		else if ( !str_cmp( word, "TEXT"     ) ) load_text    (fpArea);
		else
		{
		    bug( "Boot_db: bad section name.", 0 );
		    exit( 1 );
		}
	    }

	    if ( fpArea != stdin )
		fclose( fpArea );
	    fpArea = NULL;
	}
	fclose( fpList );
    }

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
	fBootDb	= FALSE;
	fPlaceTree = TRUE;
	fix_exits( );
	convert_objects( );
	load_notes( );
	load_bans( );
	area_update( );
	fPlaceTree = FALSE;
    }

    if ( !help_greeting )
    {
	bug( "boot_db: No help_greeting read.", 0 );
	help_greeting = "By what name do you wish to be known ? ";
    }

    return;
}



/*
 * Snarf an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;

    pArea               = new_area( );

    pArea->name         = fread_string( fp );
    pArea->area_flags   = AREA_LOADING;         /* OLC */
    pArea->security     = 9;                    /* OLC */ /* 9 -- Hugin */
    pArea->builders     = str_dup( "None" );    /* OLC */
    pArea->lvnum        = 0;                    /* OLC */
    pArea->uvnum        = 0;                    /* OLC */
    pArea->vnum         = top_area;             /* OLC */
    pArea->filename     = str_dup( strArea );   /* OLC */
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->empty        = FALSE;                /* ROM OLC*/

    if ( !area_first )
        area_first = pArea;
    if ( area_last )
    {
        area_last->next = pArea;
        REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
    }
    area_last   = pArea;
    pArea->next = NULL;

    return;
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void new_load_area( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    pArea               = new_area();
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->empty	= FALSE;
    pArea->filename     = str_dup( strArea );
    pArea->vnum         = top_area;
    pArea->name         = str_dup( "New Area" );
    pArea->builders     = str_dup( "" );
    pArea->security     = 9;                    /* 9 -- Hugin */
    pArea->lvnum        = 0;
    pArea->uvnum        = 0;
    pArea->area_flags   = 0;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
           case 'B':
            SKEY( "Builders", pArea->builders );
            break;
	   case 'C':
	    if ( !str_cmp(word, "Climate") )
	    {
		fMatch = TRUE;
		if ( (pArea->climate = flag_value(climate_flags, fread_word(fp))) == NO_FLAG )
		    pArea->climate = CLIMATE_TEMPERATE;
	    }
	    break;
           case 'E':
             if ( !str_cmp( word, "End" ) )
             {
                 fMatch = TRUE;
                 if ( area_first == NULL )
                    area_first = pArea;
                 if ( area_last  != NULL )
                    area_last->next = pArea;
                 area_last   = pArea;
                 pArea->next = NULL;
                 return;
            }
            break;
	   case 'F':
	    KEY( "Flags", pArea->area_flags, fread_number(fp) );
	    break;
	   case 'M':
	    SKEY( "ModDate", pArea->moddate );
	    break;
           case 'N':
            SKEY( "Name", pArea->name );
            break;
           case 'S':
             KEY( "Security", pArea->security, fread_number( fp ) );
            break;
	   case 'T':
	    if ( !str_cmp(word, "Terrain") )
	    {
		fMatch = TRUE;
		if ( (pArea->terrain = flag_value(terrain_flags, fread_word(fp))) == NO_FLAG )
		    pArea->terrain = TERRAIN_GRASSLAND;
	    }
	    break;
           case 'V':
            if ( !str_cmp( word, "VNUMs" ) )
            {
                pArea->lvnum = fread_number( fp );
                pArea->uvnum = fread_number( fp );
            }
            break;
	   case 'W':
	    if ( !str_cmp(word, "Weather") )
	    {
		fMatch = TRUE;
		if ( (pArea->weather = flag_value(weather_flags, fread_word(fp))) == NO_FLAG )
		    pArea->weather = WEATHER_SUNNY;
	    }
	    break;
        }
    }
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
    if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
        area_last->lvnum = area_last->uvnum = vnum;
    if ( vnum != URANGE( area_last->lvnum, vnum, area_last->uvnum ) )
        if ( vnum < area_last->lvnum )
            area_last->lvnum = vnum;
        else
            area_last->uvnum = vnum;
    return;
}


/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
	pHelp		= new_help();
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
	pHelp->text	= fread_string( fp );

	if ( !str_cmp( pHelp->keyword, "greeting" ) )
	    help_greeting = pHelp->text;

	if ( help_first == NULL )
	    help_first = pHelp;
	if ( help_last  != NULL )
	    help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
    }

    return;
}


/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
    RESET_DATA *pr;

    if ( !pR )
       return;

    pr = pR->reset_last;

    if ( !pr )
    {
        pR->reset_first = pReset;
        pR->reset_last  = pReset;
    }
    else
    {
        pR->reset_last->next = pReset;
        pR->reset_last       = pReset;
        pR->reset_last->next = NULL;
    }

    return;
}

/*
 * Snarf a reset section.       Changed for OLC.
 */
void load_resets( FILE *fp )
{
    RESET_DATA  *pReset;
    int         iLastRoom = 0;
    int         iLastObj  = 0;

    if ( !area_last )
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        EXIT_DATA       *pexit;
        ROOM_INDEX_DATA *pRoomIndex;
        char             letter;

        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        pReset          = new_reset_data();
        pReset->command = letter;
        /* if_flag */     fread_number( fp );
        pReset->arg1    = fread_number( fp );
        pReset->arg2    = fread_number( fp );
        pReset->arg3    = ( letter == 'G' || letter == 'R' )
                            ? 0 : fread_number( fp );
                          fread_to_eol( fp );

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
            exit( 1 );
            break;

        case 'M':
            get_mob_index  ( pReset->arg1 );
            if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastRoom = pReset->arg3;
            }
            break;

        case 'O':
            get_obj_index  ( pReset->arg1 );
            if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastObj = pReset->arg3;
            }
            break;

        case 'P':
            get_obj_index  ( pReset->arg1 );
            if ( ( pRoomIndex = get_room_index ( iLastObj ) ) )
            {
		new_reset( pRoomIndex, pReset );
            }
            break;

        case 'G':
        case 'E':
            get_obj_index  ( pReset->arg1 );
            if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastObj = iLastRoom;
            }
            break;

        case 'D':
            pRoomIndex = get_room_index( pReset->arg1 );

            if (   pReset->arg2 < 0
                || pReset->arg2 > 5
                || !pRoomIndex
                || !( pexit = pRoomIndex->exit[pReset->arg2] )
                || (!IS_SET( pexit->rs_flags, EX_ISDOOR ) &&
		    !IS_SET( pexit->rs_flags, EX_HIDDEN) ))
            {
                bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
		break;
/*                exit( 1 );*/
            }

	    if ( (pReset->arg3 > 31) || (pReset->arg3 < 0) )
		pReset->arg3 = 0;


	    if ( pReset->arg3 & 1 )
		SET_BIT( pexit->rs_flags, EX_CLOSED );

	    if ( pReset->arg3 & 2 )
		SET_BIT( pexit->rs_flags, EX_LOCKED );

	    if ( pReset->arg3 & 4 )
		SET_BIT( pexit->rs_flags, EX_PICKPROOF );

	    if ( pReset->arg3 & 8 )
		SET_BIT( pexit->rs_flags, EX_HIDDEN );

/*
            switch ( pReset->arg3 )
            {
                default:
                    bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
                case 0: break;
                case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
			break;
                case 2: SET_BIT( pexit->rs_flags, EX_CLOSED );
			SET_BIT( pexit->rs_flags, EX_LOCKED );
			break;
		case 3: SET_BIT( pexit->rs_flags, EX_CLOSED );
			SET_BIT( pexit->rs_flags, EX_LOCKED );
			SET_BIT( pexit->rs_flags, EX_PICKPROOF );
			break;
            }
*/
            /*
             * By calling new_reset we are assigning reset data for doors.

             * This data is not used in updating the game any longer.  But
             * displaying resets in this manner may be to your liking.
             * I have left the code here so you may do so.  Uncomment data in
             * display_resets in olc.c if you wish to do this.
             *
             *
            new_reset( pRoomIndex, pReset );
             *
             * End Resets Comment.
             */
            break;

        case 'R':
            if ( pReset->arg2 < 0 || pReset->arg2 > 6 ) /* Last Door. */
            {
                bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                exit( 1 );
            }

            if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) )
                new_reset( pRoomIndex, pReset );

            break;
	case 'A':
            if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastObj = iLastRoom;
            }
            break;

        }
    }

    return;
}


/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int door;
	int iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex			= new_room_index();
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	/* Area number */		  fread_number( fp );
	pRoomIndex->room_flags		= fread_flag( fp );

	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;
	for ( door = 0; door <= 5; door++ )
	    pRoomIndex->exit[door] = NULL;
	pRoomIndex->resources		= 0;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door > 5 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= new_exit();
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		pexit->exit_info	= 0;
		pexit->rs_flags         = 0;                    /* OLC */
		locks			= fread_number( fp );
		pexit->key		= fread_number( fp );
		pexit->u1.vnum		= fread_number( fp );
		pexit->orig_door        = door;                 /* OLC */

		if ( locks & 1 )
		    SET_BIT(pexit->rs_flags, EX_ISDOOR);
		if ( locks & 2 )
		    SET_BIT(pexit->rs_flags, EX_CLOSED);
		if ( locks & 4 )
		    SET_BIT(pexit->rs_flags, EX_LOCKED);
		if ( locks & 8 )
		    SET_BIT(pexit->rs_flags, EX_PICKPROOF);
		if ( locks & 16 )
		    SET_BIT(pexit->rs_flags, EX_HIDDEN);

		pRoomIndex->exit[door]	= pexit;
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= new_extra_descr();
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
	top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
        assign_area_vnum( vnum );                                    /* OLC */

    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	pShop			= new_shop();
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( shop_first == NULL )
	    shop_first = pShop;
	if ( shop_last  != NULL )
	    shop_last->next = pShop;

	shop_last	= pShop;
	pShop->next	= NULL;
	top_shop++;
    }

    return;
}


/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
		exit( 1 );
	    }
	    break;
	case 'O':
	    pObjIndex		= get_obj_index	( fread_number ( fp ) );
	    pObjIndex->spec_fun	= spec_obj_lookup( fread_word   ( fp ) );
	    if ( pObjIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'O': vnum %d.", pObjIndex->vnum );
		exit( 1 );
	    }
	    break;
	case 'U':
	    pObjIndex		= get_obj_index	( fread_number ( fp ) );
	    pObjIndex->use_fun	= use_fun_lookup( fread_word   ( fp ) );
	    if ( pObjIndex->use_fun == 0 )
	    {
		bug( "Load_specials: 'U': vnum %d.", pObjIndex->vnum );
		exit( 1 );
	    }
	    break;

	}

	fread_to_eol( fp );
    }
}



/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    extern const sh_int rev_dir [];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
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

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( door = 0; door <= 5; door++ )
	    {
		if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
		&&   ( to_room   = pexit->u1.to_room            ) != NULL
		&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
		&&   pexit_rev->u1.to_room != pRoomIndex 
		&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
		{
		    sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
			pRoomIndex->vnum, door,
			to_room->vnum,    rev_dir[door],
			(pexit_rev->u1.to_room == NULL)
			    ? 0 : pexit_rev->u1.to_room->vnum );
/*		    bug( buf, 0 ); */
		}
	    }
	}
    }

    return;
}



/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {

	if ( ++pArea->age < 5 )
	    continue;

	/*
	 * Check age and reset.
	 */
	if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
	||    pArea->age >= 20 )
	{
            if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
            {
                save_area( pArea );
                REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
            }

	    reset_area( pArea );
            sprintf(buf,"%s has just been reset.",pArea->name);
            wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);

	    pArea->age = number_range( 0, 3 );
	    if (pArea->nplayer == 0) 
		pArea->empty = TRUE;
	}
    }

    return;
}

/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;

    if ( !pRoom )
        return;

    pMob        = NULL;
    last        = FALSE;

    switch( pRoom->sector_type )
    {
	default:
	    break;
	case SECT_MOUNTAIN:
	    if ( number_percent() + number_percent() >= 160 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 190
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 160
	    &&   num_ore < MAX_ORE )
	    {
		SET_BIT(pRoom->resources, RES_ORE);
		num_ore++;
	    }
	    if ( number_percent() + number_percent() >= 195
	    &&   num_gem < MAX_GEM )
	    {
		SET_BIT(pRoom->resources, RES_GEMS);
		num_gem++;
	    }
	    if ( number_percent() + number_percent() >= 199
	    &&   num_lumber < MAX_LUMBER )
	    {
		SET_BIT(pRoom->resources, RES_LUMBER);
		num_lumber++;
	    }
	    break;
	case SECT_HILLS:
	    if ( number_percent() + number_percent() >= 120 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 185
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 175
	    &&   num_ore < MAX_ORE )
	    {
		SET_BIT(pRoom->resources, RES_ORE);
		num_ore++;
	    }
	    if ( number_percent() + number_percent() >= 199
	    &&   num_gem < MAX_GEM )
	    {
		SET_BIT(pRoom->resources, RES_GEMS);
		num_gem++;
	    }
	    if ( number_percent() + number_percent() >= 190
	    &&   num_lumber < MAX_LUMBER )
	    {
		SET_BIT(pRoom->resources, RES_LUMBER);
		num_lumber++;
	    }
	    break;
	case SECT_DESERT:
	    if ( number_percent() + number_percent() >= 190 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 199
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 199
	    &&   num_ore < MAX_ORE )
	    {
		SET_BIT(pRoom->resources, RES_ORE);
		num_ore++;
	    }
	    if ( number_percent() + number_percent() >= 160
	    &&   num_gem < MAX_GEM )
	    {
		SET_BIT(pRoom->resources, RES_GEMS);
		num_gem++;
	    }
	    break;
	case SECT_UNDERGROUND:
	    if ( number_percent() + number_percent() >= 135 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 140
	    &&   num_ore < MAX_ORE )
	    {
		SET_BIT(pRoom->resources, RES_ORE);
		num_ore++;
	    }
	    if ( number_percent() + number_percent() >= 180
	    &&   num_gem < MAX_GEM )
	    {
		SET_BIT(pRoom->resources, RES_GEMS);
		num_gem++;
	    }
	    break;
	case SECT_FOREST:
	    if ( number_percent() + number_percent() >= 135 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 160
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 175
	    &&   num_lumber < MAX_LUMBER )
	    {
		SET_BIT(pRoom->resources, RES_LUMBER);
		num_lumber++;
	    }
	    break;
	case SECT_FIELD:
	    if ( number_percent() + number_percent() >= 145 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 140
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    break;
	case SECT_WATER_SWIM:
	    if ( number_percent() + number_percent() >= 0 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 199
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 180
	    &&   num_fish < MAX_FISH )
	    {
		SET_BIT(pRoom->resources, RES_FISH);
		num_fish++;
	    }
	    break;
	case SECT_SWAMP:
	    if ( number_percent() + number_percent() >= 80 )
		SET_BIT(pRoom->resources, RES_WATER);
	    if ( number_percent() + number_percent() >= 190
	    &&   num_herb < MAX_HERB )
	    {
		SET_BIT(pRoom->resources, RES_HERB);
		num_herb++;
	    }
	    if ( number_percent() + number_percent() >= 199
	    &&   num_lumber < MAX_LUMBER )
	    {
		SET_BIT(pRoom->resources, RES_LUMBER);
		num_lumber++;
	    }
	    if ( number_percent() + number_percent() >= 190
	    &&   num_fish < MAX_FISH )
	    {
		SET_BIT(pRoom->resources, RES_FISH);
		num_fish++;
	    }
	    break;
    }

    if ( fPlaceTree )
	place_tree( pRoom );

    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] )
          /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */ )
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
              && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
	CHAR_DATA	*vch;
	int count_in_room = 0;

        switch ( pReset->command )
        {
        default:
                bug( "Reset_room: bad command %c.", pReset->command );
                break;

        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

	    for ( vch = pRoom->people; vch != NULL; vch = vch->next_in_room )
		if ( vch->pIndexData == pMobIndex )
		    count_in_room++;

            if ( count_in_room >= pReset->arg2 ||
		(pMobIndex->count >= pMobIndex->max && pMobIndex->max > 0) ||
		(pMobIndex->max == 0 && pMobIndex->count >= 20) )
            {
                last = FALSE;
                break;
            }

            pMob = create_mobile( pMobIndex );

            /*
             * Pet shop mobiles get ACT_PET set.
             */
            {
                ROOM_INDEX_DATA *pRoomIndexPrev;

                pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
                if ( pRoomIndexPrev
                    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    SET_BIT( pMob->act, ACT_PET);
            }

            char_to_room( pMob, pRoom );

            LastMob = pMob;

            level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 ); /* -1 ROM */
            last = TRUE;
            break;

        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'O': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'O': bad vnum %d.", pReset->arg3 );
                continue;
            }

   /*         if ( pRoom->area->nplayer > 0
              || count_obj_list( pObjIndex, pRoom->contents ) > 0 )
                break;   Dumb-ass code */

	      if (count_obj_list( pObjIndex, pRoom->contents ) > 0 ||
		  (pObjIndex->count >= pObjIndex->max && pObjIndex->max > 0) ||
                (pObjIndex->max == 0 && pObjIndex->count >= 20) )
		break;

            pObj = create_object( pObjIndex,              /* UMIN - ROM OLC */
                                  UMIN(number_fuzzy( level ), LEVEL_HERO -1) );
            pObj->cost = 0;
            obj_to_room( pObj, pRoom );
            break;

        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if ( pRoom->area->nplayer > 0
            ||   !(LastObj = get_obj_type( pObjToIndex ))
            ||   count_obj_list(pObjIndex, LastObj->contains) > 0
	    ||   (pObjIndex->count >= pObjIndex->max && pObjIndex->max > 0)
	    ||   (pObjIndex->max == 0 && pObjIndex->count >= 20) )
                break;
                                                /* lastObj->level  -  ROM */
            pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
            obj_to_obj( pObj, LastObj );
            break;

        case 'G':
        case 'E':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !last )
                break;

            if ( !LastMob )
            {
                bug( "Reset_room: 'E' or 'G': null mob for vnum %d.",
                    pReset->arg1 );
                last = FALSE;
                break;
            }

            if ( LastMob->pIndexData->pShop )   /* Shop-keeper? */
            {
                int olevel;

                switch ( pObjIndex->item_type )
                {
                default:                olevel = 0;                      break;
                case ITEM_POTION:       olevel = number_range(  0, 10 ); break;
                case ITEM_ARMOR:        olevel = number_range(  5, 15 ); break;
                /* ROM patch weapon, treasure */
                case ITEM_WEAPON:       olevel = number_range(  5, 15 ); break;
                case ITEM_TREASURE:     olevel = number_range( 10, 20 ); break;

                  break;
                }

                pObj = create_object( pObjIndex, olevel );
                SET_BIT( pObj->extra_flags, ITEM_INVENTORY );  /* ROM OLC */

            }
            else   /* ROM OLC else version */
            {
                int count = 0;
		OBJ_DATA *objIndex;

		for (objIndex = LastMob->carrying; objIndex != NULL;
		     objIndex = objIndex->next_content)
		{
		    if (objIndex->pIndexData->vnum == pObjIndex->vnum)
			count++;
		} 

		if ( pReset->command == 'E'
		&&   get_eq_char(LastMob, pReset->arg3) != NULL )
		    break;

		if ( pReset->command == 'G' && count > 2 )
		    break;
	
                    pObj = create_object( pObjIndex,
                           UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
            }

            obj_to_char( pObj, LastMob );
            if ( pReset->command == 'E' )
                equip_char( LastMob, pObj, pReset->arg3 );
            last = TRUE;
            break;

        case 'D':
            break;

        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
                continue;
            }

            {
                EXIT_DATA *pExit;
                int d0;
                int d1;

                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1                   = number_range( d0, pReset->arg2-1 );
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;

	case 'A':
	    break;

        }
    }

    return;
}



/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoom;
    MOB_INDEX_DATA *pMob;
    int  vnum;

    add_explore( );

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if ( ( pRoom = get_room_index(vnum) ) )
            reset_room(pRoom);

	if ( (pMob = get_mob_index(vnum)) != NULL )
	{
	    pMob->killed -= UMAX( 1, number_fuzzy(1) );
	    pMob->killed = UMAX( 0, pMob->killed );
	}
    }

    return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;
    int i;

    mobile_count++;

    if ( pMobIndex == NULL )
    {
	bug( "Create_mobile: NULL pMobIndex.", 0 );
	exit( 1 );
    }

    mob			= new_char();
    clear_char( mob );
    mob->pIndexData	= pMobIndex;

    mob->name		= str_dup( pMobIndex->player_name );
    mob->id		= get_mob_id();
    mob->short_descr	= str_dup( pMobIndex->short_descr );
    mob->long_descr	= str_dup( pMobIndex->long_descr );
    mob->description	= str_dup( pMobIndex->description ); 
    mob->on		= NULL;
    mob->rider		= NULL;
    mob->mount		= NULL;
    mob->memory		= NULL;
    mob->hunting	= NULL;
    mob->start_age	= 17;

    if ( is_name("guard", mob->name) )
	SET_BIT( mob->act, ACT_WARRIOR );

    if (pMobIndex->new_format)
    /* load in new style */
    {
	/* read from prototype */
	mob->act 		= pMobIndex->act;
	mob->comm		= COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
	mob->affected_by	= pMobIndex->affected_by;
	mob->affected_by_2	= 0;
	mob->level		= pMobIndex->level;
	if ( mob->level < 1 )
	    mob->level = 1;
	for ( i = 0; i < 5; i++ )
	{
	    mob->channel_max[i] = 0;
	    mob->channel_skill[i] = -1;
	}

	mob->race		= pMobIndex->race;

	mob->damage[DICE_NUMBER]= race_table[mob->race].dam_dice
				+ number_range( 0, mob->level / 12 );
	mob->damage[DICE_TYPE]	= race_table[mob->race].dam_side
				+ number_range( 0, mob->level / 8 );
	mob->max_stamina	= 100 + (mob->level * 5 );
        if(mob->race==race_lookup("horse"))
                mob->max_stamina=5000;
	mob->stamina		= mob->max_stamina;
	mob->dam_type		= pMobIndex->dam_type;

	for (i = 0; i < 7; i++)
	    mob->armor[i]	= mob->level * number_range(1, 15) / 4;
	mob->hitroll		= mob->level * number_range(1, 5) / 5;
	mob->damroll		= mob->level * number_range(1, 4) / 3;

	mob->prompt		= NULL;
	mob->off_flags		= pMobIndex->off_flags;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex		= pMobIndex->sex;
        if (mob->sex == 3) /* random sex */
            mob->sex = number_range(1,2);
	if ( pMobIndex->gold == 0
	||  !IS_SENTIENT(pMobIndex) )
	    mob->gold = 0;
	else
	    mob->gold		= number_range(pMobIndex->gold/2,
					       pMobIndex->gold * 3/2 );
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	mob->material		= pMobIndex->material;

	/* computed on the spot */

    	for (i = 0; i < MAX_STATS; i ++)
            mob->perm_stat[i] = UMIN(25, 11 + mob->level / 6) + dice(2,3);

	if ( IS_SET(mob->act, ACT_CHANNELER) )
	{
	    int luck, j;
	    luck = luk_app[get_curr_stat( mob, STAT_LUK )].percent_mod / 4;
    
	    /* Find the person's strongest power :) */
	    if ( TRUE_SEX(mob) == SEX_MALE )
		i = number_fuzzy( number_range(0, 1) );
	    else
		i = number_fuzzy( number_range(2, 3) );
	    switch( i )
	    {
	        default:
	                j = 4;
	                break;
	        case 0:
			if ( number_percent() >= 75 )
			    j = number_fuzzy(0);
			else
			    j = 0;
	                break;
	        case 1:
			if ( number_percent() >= 75 )
			    j = number_fuzzy(2);
			else
			    j = 2;
	                break;
	        case 2:
			if ( number_percent() >= 75 )
			    j = number_fuzzy(1);
			else
			    j = 1;
	                break;
	        case 3:
			if ( number_percent() >= 75 )
			    j = number_fuzzy(3);
			else
			    j = 3;
	                break;
	    }

	    if ( j < 0 || j > 4 )
		j = 4;
	    mob->channel_max[j] = number_range( 60, 80 );
	
	    /* Get the rest */
	    for ( i = 0; i < 5; i++ )
	    {
		if ( mob->channel_max[i] == 0 )
		{
		    mob->channel_max[i] = number_range( 20, 60 )
				        + number_range( 0, luck );
		}
		if ( TRUE_SEX(mob) == SEX_MALE )
		    mob->channel_max[i] += number_range( 1, 4 );
		mob->channel_skill[i] = mob->channel_max[i];
	    }
	}

        if (IS_SET(mob->act,ACT_WARRIOR))
        {
            mob->perm_stat[STAT_STR] += 3;
            mob->perm_stat[STAT_INT] -= 1;
            mob->perm_stat[STAT_CON] += 2;
        }
        
        if (IS_SET(mob->act,ACT_ROGUE))
        {
            mob->perm_stat[STAT_DEX] += 3;
            mob->perm_stat[STAT_INT] += 1;
            mob->perm_stat[STAT_WIS] -= 1;
        }
        
        if (IS_SET(mob->act,ACT_SCHOLAR))
        {
            mob->perm_stat[STAT_INT] += 3;
            mob->perm_stat[STAT_STR] -= 1;
            mob->perm_stat[STAT_DEX] += 1;
        }

	if ( !IS_SENTIENT(mob) )
	{
	    mob->perm_stat[STAT_INT] /= 5;
	    mob->perm_stat[STAT_WIS] /= 5;
	}
        
	if ( mob->race != 0 
	&&   mob->race < race_lookup("ogier") )
	{
	    for ( i = 0; i < MAX_STATS; i++ )
		mob->perm_stat[i] += pc_race_table[mob->race].stats[i];
	}

        mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
        mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

	mob->max_hit = 3 * number_fuzzy( 9 ) + dice( 6, 3 );
	for ( i = 0; i < mob->level; i++ )
	    mob->max_hit += number_range( 7, 11 );
	if ( IS_SET(mob->act, ACT_WARRIOR) )
	{
	    mob->max_hit = 3 * number_fuzzy( 11 ) + dice( 6, 3 );
	    for ( i = 0; i < mob->level; i++ )
		mob->max_hit += number_range( 9, 13 );
	}
	else if ( IS_SET(mob->act, ACT_SCHOLAR) )
	{
	    mob->max_hit = 3 * number_fuzzy( 7 ) + dice( 6, 3 );
	    for ( i = 0; i < mob->level; i++ )
		mob->max_hit += number_range( 5, 9 );
	}

	mob->max_hit += con_app[get_curr_stat(mob, STAT_CON)].hitp * mob->level;
	mob->max_stamina = 5 * number_fuzzy( get_curr_stat(mob, STAT_CON) )
                         + number_range( 1, get_curr_stat(mob, STAT_STR) )
                         + dice( 2, 10 );

	if(mob->race==race_lookup("horse"))
	  mob->max_stamina = 5000;

	mob->max_hit = mob->max_hit * race_table[mob->race].hit_mult / 100;

	if ( mob->max_hit < 1 )
	{
	    mob->max_hit = number_range( 7, 11 );
	    mob->max_hit += number_fuzzy( 9 ) + dice( 6, 3 );

	    if ( IS_SET(mob->act, ACT_WARRIOR) )
	    {
		mob->max_hit = number_range( 9, 13 );
		mob->max_hit += number_fuzzy( 11 ) + dice( 6, 3 );
	    }
	    else if ( IS_SET(mob->act, ACT_SCHOLAR) )
	    {
		mob->max_hit = number_range( 5, 9 );
		mob->max_hit += number_fuzzy( 7 ) + dice( 6, 3 );
	    }

	    mob->max_hit += con_app[get_curr_stat(mob, STAT_CON)].hitp;
	}
	mob->hit		= mob->max_hit;

    }
    else /* read in old format and convert */
    {
	mob->act		= pMobIndex->act|ACT_WARRIOR;
	mob->affected_by	= pMobIndex->affected_by;
	mob->affected_by_2	= 0;
	mob->level		= pMobIndex->level;
	mob->hitroll		= pMobIndex->hitroll;
	mob->damroll		= 0;
	mob->max_hit		= mob->level * 8 + number_range(
					mob->level * mob->level/4,
					mob->level * mob->level);
	mob->max_hit *= .9;
	mob->hit		= mob->max_hit;
	switch(number_range(1,3))
	{
	    case (1): mob->dam_type = 3; 	break;  /* slash */
	    case (2): mob->dam_type = 7;	break;  /* pound */
	    case (3): mob->dam_type = 11;	break;  /* pierce */
	}
	for (i = 0; i < 3; i++)
	    mob->armor[i]	= interpolate(mob->level,100,-100);
	mob->armor[3]		= interpolate(mob->level,100,0);
	mob->race		= pMobIndex->race;
	mob->off_flags		= pMobIndex->off_flags;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex		= pMobIndex->sex;
	mob->gold		= pMobIndex->gold/100;
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= SIZE_MEDIUM;
	mob->material		= 0;

        for (i = 0; i < MAX_STATS; i ++)
            mob->perm_stat[i] = 9 + mob->level/6;
    }

    if ( IS_AFFECTED(mob, AFF_HASTE)
    ||   IS_SET(mob->off_flags, OFF_FAST) )
    {
	AFFECT_DATA af;
	int strength;

	if ( can_channel(mob, 1) )
	    strength = channel_strength( mob, POWER_SPIRIT|POWER_AIR|POWER_EARTH );
	else
	    strength = mob->level;

	af.type			= gsn_haste;
	af.strength		= strength;
	af.duration		= -1;
	af.location		= APPLY_AGI;
	af.modifier		= 1 + (strength >= 25) + (strength >= 65) + (strength > 85);
	af.bitvector		= 0;
	af.bitvector_2		= 0;
	af.owner		= AFF_OWNER(mob);
	af.flags		= AFFECT_NOTCHANNEL;
	SET_SEX( af, mob );
	affect_to_char( mob, &af );
	af.location         = APPLY_DEX;
	affect_to_char( mob, &af );
    }
    mob->position = mob->start_pos;
    mob->guild = pMobIndex->guild;


    /* link the mob to the world list */
    mob->next		= char_list;
    char_list		= mob;
    pMobIndex->count++;
    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
	return;

    free_string( clone->name );
    free_string( clone->short_descr );
    free_string( clone->long_descr );
    free_string( clone->description );
    /* start fixing values */ 
    clone->name 	= str_dup(parent->name);
    clone->version	= parent->version;
    clone->short_descr	= str_dup(parent->short_descr);
    clone->long_descr	= str_dup(parent->long_descr);
    clone->description	= str_dup(parent->description);
    clone->sex		= parent->sex;
    clone->class	= parent->class;
    clone->race		= parent->race;
    clone->level	= parent->level;
    clone->trust	= 0;
    clone->timer	= parent->timer;
    clone->wait		= parent->wait;
    clone->hit		= parent->hit;
    clone->max_hit	= parent->max_hit;
    clone->stamina	= parent->stamina;
    clone->max_stamina	= parent->max_stamina;
    clone->gold		= parent->gold;
    clone->exp		= parent->exp;
    clone->act		= parent->act;
    clone->comm		= parent->comm;
    clone->imm_flags	= parent->imm_flags;
    clone->res_flags	= parent->res_flags;
    clone->vuln_flags	= parent->vuln_flags;
    clone->invis_level	= parent->invis_level;
    clone->affected_by	= parent->affected_by;
    clone->affected_by_2	= 0;
    clone->position	= parent->position;
    clone->practice	= parent->practice;
    clone->train	= parent->train;
    clone->hitroll	= parent->hitroll;
    clone->damroll	= parent->damroll;
    clone->wimpy	= parent->wimpy;
    clone->form		= parent->form;
    clone->parts	= parent->parts;
    clone->size		= parent->size;
    clone->material	= parent->material;
    clone->off_flags	= parent->off_flags;
    clone->dam_type	= parent->dam_type;
    clone->start_pos	= parent->start_pos;
    clone->default_pos	= parent->default_pos;
    clone->guild	= parent->guild;
    clone->prompt	= NULL;
    clone->on		= NULL;
    clone->rider	= NULL;
    clone->mount	= NULL;

    for ( i = 0; i < 5; i++ )
    {
	clone->channel_skill[i]	= parent->channel_skill[i];
	clone->channel_max[i]	= parent->channel_max[i];
    }
    
    for (i = 0; i < 4; i++)
    	clone->armor[i]	= parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
	clone->perm_stat[i]	= parent->perm_stat[i];
	clone->mod_stat[i]	= parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
	clone->damage[i]	= parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);

}




/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    object_count++;

    if ( pObjIndex == NULL )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	abort();
    }


    obj			= new_obj( );

    *obj		= obj_zero;
    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->enchanted	= FALSE;

    obj->level = item_level( obj );
    obj->wear_loc	= -1;

    obj->material	= pObjIndex->material;
    obj->name           = str_dup( pObjIndex->name );           /* OLC */
    obj->short_descr    = str_dup( pObjIndex->short_descr );    /* OLC */
    obj->description    = str_dup( pObjIndex->description );    /* OLC */

    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->value[5]	= pObjIndex->value[5];
    obj->value[6]	= pObjIndex->value[6];
    obj->value[7]	= pObjIndex->value[7];
    obj->weight		= pObjIndex->weight;
    if ( obj->weight < 1 )
	obj->weight = number_fuzzy(2);
    obj->condition	= pObjIndex->condition;
    obj->owner		= NULL;
    obj->on		= NULL;
    obj->match		= NULL;

    if (level == -1 || pObjIndex->new_format)
	obj->cost	= pObjIndex->cost;
    else
    	obj->cost	= number_fuzzy( 10 )
			* number_fuzzy( level ) * number_fuzzy( level );

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	break;

    case ITEM_LIGHT:
	if (obj->value[2] == 999)
	     obj->value[2] = -1;
	break;
    case ITEM_POTION:
	obj->value[2] = obj->value[1];
	break;
    case ITEM_FRUIT_TREE:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_FAQ:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_MAP:
    case ITEM_CLOTHING:
    case ITEM_ORE:
    case ITEM_INGREDIENT:
    case ITEM_VIAL:
    case ITEM_GATE:
	break;

    case ITEM_WEAPON:
	if (level != -1 && !pObjIndex->new_format)
	{
	    obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
	    obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	}

	if ( obj->value[1] * obj->value[2] > 120 )
	    obj->value[2] = 120 / obj->value[1];
	break;

    case ITEM_ARMOR:
	if (level != -1 && !pObjIndex->new_format)
	{
	    obj->value[0]	= number_fuzzy( level / 5 + 3 );
	    obj->value[1]	= number_fuzzy( level / 5 + 3 );
	    obj->value[2]	= number_fuzzy( level / 5 + 3 );
	}

	if ( !str_cmp("none", flag_string(armor_flags, obj->value[4])) )
	{
	    obj->value[4] = obj->wear_flags;
	    REMOVE_BIT(obj->value[4], ITEM_TAKE);
	    REMOVE_BIT(obj->value[4], ITEM_WEAR_FINGER);
	    REMOVE_BIT(obj->value[4], ITEM_WEAR_SHIELD);
	    REMOVE_BIT(obj->value[4], ITEM_WIELD);
	    REMOVE_BIT(obj->value[4], ITEM_HOLD);
	    REMOVE_BIT(obj->value[4], ITEM_TWO_HANDS);
	}
	break;

    case ITEM_MONEY:
	if (!pObjIndex->new_format)
	    obj->value[0]	= obj->cost;
	break;
    }

    obj->next		= object_list;
    object_list		= obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
/*
    EXTRA_DESCR_DATA *ed,*ed_new;
*/
    if (parent == NULL || clone == NULL)
	return;

    free_string( clone->name );
    free_string( clone->short_descr );
    free_string( clone->description );
    /* start fixing the object */
    clone->name 	= str_dup(parent->name);
    clone->short_descr 	= str_dup(parent->short_descr);
    clone->description	= str_dup(parent->description);
    clone->item_type	= parent->item_type;
    clone->extra_flags	= parent->extra_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight	= parent->weight;
    clone->cost		= parent->cost;
    clone->level	= parent->level;
    clone->condition	= parent->condition;
    clone->material	= parent->material;
    clone->timer	= parent->timer;
    clone->on		= NULL;

    for (i = 0;  i < 5; i ++)
	clone->value[i]	= parent->value[i];

    /* affects */
    clone->enchanted	= parent->enchanted;
  
    for (paf = parent->affected; paf != NULL; paf = paf->next) 
	affect_to_obj(clone,paf);

    /* extended desc */
/*
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next);
    {
        ed_new			= new_extra_descr();

        ed_new->keyword    	= str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next           	= clone->extra_descr;
        clone->extra_descr  	= ed_new;
    }
*/
}



/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    static CHAR_DATA ch_zero;
    int i;

    *ch				= ch_zero;
    ch->name			= &str_empty[0];
    ch->short_descr		= &str_empty[0];
    ch->long_descr		= &str_empty[0];
    ch->description		= &str_empty[0];
    ch->prompt			= &str_empty[0];

    ch->logon			= current_time;
    ch->lines			= PAGELEN;
    for (i = 0; i < 7; i++)
    	ch->armor[i]		= 0;
    ch->comm			= 0;
    ch->position		= POS_STANDING;
    ch->practice		= 0;
    ch->hit			= 50;
    ch->max_hit			= 50;
    ch->stamina			= 150;
    ch->max_stamina		= 150;
    ch->char_made		= FALSE;
    ch->bank			= 0;
    for (i = 0; i < MAX_STATS; i ++)
    {
	ch->perm_stat[i] = 13; 
	ch->mod_stat[i] = 0;
    }
    return;
}


/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name( (char *) name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex != NULL;
	  pMobIndex  = pMobIndex->next )
    {
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_mob_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex != NULL;
	  pObjIndex  = pObjIndex->next )
    {
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_obj_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next )
    {
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_room_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
TEXT_DATA *get_text_index( int id )
{
    TEXT_DATA *TextIndex;

    for ( TextIndex  = text_hash[id % MAX_KEY_HASH];
	  TextIndex != NULL;
	  TextIndex  = TextIndex->next )
    {
	if ( TextIndex->id == id )
	    return TextIndex;
    }

    return NULL;
}


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

long fread_flag( FILE *fp)
{
    int number;
    char c;

    do
    {
	c = getc(fp);
    }
    while ( isspace(c));

    number = 0;

    if (!isdigit(c) && c !='-' )
    {
	while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
	{
	    number += flag_convert(c);
	    c = getc(fp);
	}
    }

    if ( c == '-' )      /* ROM OLC */
    {
        number = fread_number( fp );
        return -number;
    }

    while (isdigit(c))
    {
	number = number * 10 + c - '0';
	c = getc(fp);
    }

    if (c == '|')
	number += fread_flag(fp);

    else if  ( c != ' ')
	ungetc(c,fp);

    return number;
}

long flag_convert(char letter )
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') 
    {
	bitsum = 1;
	for (i = letter; i > 'A'; i--)
	    bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
	bitsum = 67108864; /* 2^26 */
	for (i = letter; i > 'a'; i --)
	    bitsum *= 2;
    }

    return bitsum;
}




/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    char c;

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
	bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
	exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return &str_empty[0];

    for ( ;; )
    {
        /*
         * Back off the char type lookup,
         *   it was too dirty for portability.
         *   -- Furey
         */

	switch ( *plast = getc(fp) )
	{
        default:
            plast++;
            break;
 
        case EOF:
	/* temp fix */
            bug( "Fread_string: EOF", 0 );
	    return NULL;
            /* exit( 1 ); */
            break;
 
        case '\n':
            plast++;
            *plast++ = '\r';
            break;
 
        case '\r':
            break;
 
        case '~':
            plast++;
	    {
		union
		{
		    char *	pc;
		    char	rgc[sizeof(char *)];
		} u1;
		int ic;
		int iHash;
		char *pHash;
		char *pHashPrev;
		char *pString;

		plast[-1] = '\0';
		iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
		for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
		    pHash    += sizeof(char *);

		    if ( top_string[sizeof(char *)] == pHash[0]
		    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
			return pHash;
		}

		if ( fBootDb )
		{
		    pString		= top_string;
		    top_string		= plast;
		    u1.pc		= string_hash[iHash];
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			pString[ic] = u1.rgc[ic];
		    string_hash[iHash]	= pString;

		    nAllocString += 1;
		    sAllocString += top_string - pString;
		    return pString + sizeof(char *);
		}
		else
		{
		    return str_dup( top_string + sizeof(char *) );
		}
	    }
	}
    }
}

char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char *plast;
    char c;
 
    if ( char_special[EOF-EOF] != TRUE )
    {
        char_special[EOF -  EOF] = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }
 
    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];
 
    for ( ;; )
    {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;
 
        switch ( plast[-1] )
        {
        default:
            break;
 
        case EOF:
            bug( "Fread_string_eol  EOF", 0 );
            exit( 1 );
            break;
 
        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}



/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    int iList;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
	bug( "Alloc_mem: size %d too large.", sMem );
	exit( 1 );
    }

    if ( rgFreeList[iList] == NULL )
    {
	pMem		  = alloc_perm( rgSizeList[iList] );
    }
    else
    {
	pMem              = rgFreeList[iList];
	rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    return pMem;
}



/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    int iList;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
	bug( "Free_mem: size %d too large.", sMem );
	exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}



/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while ( sMem % sizeof(long) != 0 )
	sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
	bug( "Alloc_perm: %d too large.", sMem );
	exit( 1 );
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
	iMemPerm = 0;
	if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
	{
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;

    if ( str[0] == '\0' )
	return &str_empty[0];

    if ( str >= string_space && str < top_string )
	return (char *) str;

    str_new = alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
	return;

    free_mem( pstr, strlen(pstr) + 1 );
    return;
}



void do_areas( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea;

    if (argument[0] != '\0')
    {
	send_to_char("No argument is used with this command.\n\r",ch);
	return;
    }

    pArea	= area_first;

    send_to_char_new( ch, "%-39s %-39s\n\r", "Areas", "Builders" );
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	if ( IS_SET(pArea->area_flags, AREA_COMPLETE) )
	{
	    sprintf( buf, "%-39s %-39s\n\r",
		(pArea->name != NULL) ? pArea->name : "",
		(pArea->name != NULL) ? pArea->builders : "" );
	    send_to_char( buf, ch );
	}
    }
    return;
}



void do_memory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "Affects %5d\n\r", top_affect   );
    send_to_char( buf, ch );
    sprintf( buf, "Areas   %5d\n\r", top_area     ); 
    send_to_char( buf, ch );
    sprintf( buf, "ExDes   %5d\n\r", top_ed    ); 
    send_to_char( buf, ch );
    sprintf( buf, "Exits   %5d\n\r", top_exit     ); 
    send_to_char( buf, ch );
    sprintf( buf, "Helps   %5d\n\r", top_help     ); 
    send_to_char( buf, ch );
    sprintf( buf, "Guards  %5d\n\r", top_guard    );
    send_to_char( buf, ch );
    sprintf( buf, "Guilds  %5d\n\r", top_guild    );
    send_to_char( buf, ch );
    sprintf( buf, "Socials %5d\n\r", social_count  );
    send_to_char( buf, ch );
    sprintf( buf, "Mobs    %5d(%d new format)\n\r", top_mob_index,newmobs ); 
    send_to_char( buf, ch );
    sprintf( buf, "(in use)%5d\n\r", mobile_count  ); 
    send_to_char( buf, ch );
    sprintf( buf, "Objs    %5d(%d new format)\n\r", top_obj_index,newobjs ); 
    send_to_char( buf, ch );
    sprintf( buf, "(in use)%5d\r\n", object_count  );
    send_to_char( buf, ch );
    sprintf( buf, "Resets  %5d\n\r", top_reset     ); 
    send_to_char( buf, ch );
    sprintf( buf, "Rooms   %5d\n\r", top_room      ); 
    send_to_char( buf, ch );
    sprintf( buf, "Shops   %5d\n\r", top_shop      ); 
    send_to_char( buf, ch );

    send_to_char_new( ch, "Nodes   %5d\n\r", top_node	);
    send_to_char_new( ch, "Notes   %5d\n\r", top_note	);

    sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
	nAllocString, sAllocString, MAX_STRING );
    send_to_char( buf, ch );

    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\n\r",
	nAllocPerm, sAllocPerm );
    send_to_char( buf, ch );

    return;
}

void do_dump( CHAR_DATA *ch, char *argument )
{
    int count,count2,num_pcs,aff_count;
    CHAR_DATA *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    EXIT_DATA *exit;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA *af;
    FILE *fp;
    int vnum,nMatch = 0;

    /* open file */
    fclose(fpReserve);
    fp = fopen("mem.dmp","w");

    /* report use of data structures */
    
    num_pcs = 0;
    aff_count = 0;

    /* mobile prototypes */
    fprintf(fp,"MobProt	%4d (%8d bytes)\n",
	top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

    /* mobs */
    count = 0;  count2 = 0;
    for (fch = char_list; fch != NULL; fch = fch->next)
    {
	count++;
	if (fch->pcdata != NULL)
	    num_pcs++;
	for (af = fch->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (fch = char_free; fch != NULL; fch = fch->next)
	count2++;

    fprintf(fp,"Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

    /* pcdata */
    count = 0;
    for (pc = pcdata_free; pc != NULL; pc = pc->next)
	count++; 

    fprintf(fp,"Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
	num_pcs, num_pcs * (sizeof(*pc)), count, count * (sizeof(*pc)));

    /* descriptors */
    count = 0; count2 = 0;
    for (d = descriptor_list; d != NULL; d = d->next)
	count++;
    for (d= descriptor_free; d != NULL; d = d->next)
	count2++;

    fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

    /* object prototypes */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    for (af = pObjIndex->affected; af != NULL; af = af->next)
		aff_count++;
            nMatch++;
        }

    fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
	top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


    /* objects */
    count = 0;  count2 = 0;
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
	count++;
	for (af = obj->affected; af != NULL; af = af->next)
	    aff_count++;
    }
    for (obj = obj_free; obj != NULL; obj = obj->next)
	count2++;

    fprintf(fp,"Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
	count, count * (sizeof(*obj)), count2, count2 * (sizeof(*obj)));

    /* affects */
    count = 0;
    for (af = affect_free; af != NULL; af = af->next)
	count++;

    fprintf(fp,"Affects	%4d (%8d bytes), %2d free (%d bytes)\n",
	aff_count, aff_count * (sizeof(*af)), count, count * (sizeof(*af)));

    /* rooms */
    fprintf(fp,"Rooms	%4d (%8d bytes)\n",
	top_room, top_room * (sizeof(*room)));

     /* exits */
    fprintf(fp,"Exits	%4d (%8d bytes)\n",
	top_exit, top_exit * (sizeof(*exit)));

    fclose(fp);

    /* start printing out mobile data */
    fp = fopen("mob.dmp","w");

    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_mob_index; vnum++)
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
	{
	    nMatch++;
	    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
		pMobIndex->vnum,pMobIndex->count,
		pMobIndex->killed,pMobIndex->short_descr);
	}
    fclose(fp);

    /* start printing out object data */
    fp = fopen("obj.dmp","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
	if ((pObjIndex = get_obj_index(vnum)) != NULL)
	{
	    nMatch++;
	    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
		pObjIndex->vnum,pObjIndex->count,
		pObjIndex->reset_num,pObjIndex->short_descr);
	}

    /* close file */
    fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;

    if (from == 0 && to == 0)
	return 0;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm() & (power -1 ) ) >= to )
	;

    return from + number;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent;

    while ( (percent = number_mm() & (128-1) ) > 99 )
	;

    return 1 + percent;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm() & (8-1) ) > 5)
	;

    return door;
}

int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}




/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static  int     rgiState[2+55];
 
void init_mm( )
{
    int *piState;
    int iState;
 
    piState     = &rgiState[2];
 
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;
 
    piState[0]  = ((int) current_time) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = (piState[iState-1] + piState[iState-2])
                        & ((1 << 30) - 1);
    }
    return;
}
 
 
 
int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;
 
    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
                        & ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}

int mob_channel( int level, int max )
{
    return (level * max / 100 );
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}

char *right_case( const char *str, int fCase )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
    {
	if ( fCase == CASE_LOWER )
	    strcap[i] = LOWER(str[i]);
	else
	    strcap[i] = UPPER(str[i]);
    }
    strcap[i] = '\0';
    return strcap;
}

char *correct_name( const char *str )
{
    static char strname[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
    {
	if ( str[i] == '\'' )
	    strname[i] = '_';
	else
            strname[i] = LOWER(str[i]);
    }
    strname[i] = '\0';
    strname[0] = UPPER(strname[0]);
    return strname;
}

/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( !ch )
    {
	fclose( fpReserve );
	if ( ( fp = fopen( file, "a" ) ) == NULL )
	    perror( file );
	else
	{
	    fprintf( fp, "[ NULL ] %s\n", str );
	    fclose( fp );
	}
    
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );

	if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen(buf), str, param );
    log_string( buf );

/* RT removed due to bug-file spamming 
    fclose( fpReserve );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
*/

    return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
    char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';

    fprintf( stderr, "%s :: %s\n", strtime, str );
    
    if (!strcmp( str, "TICK!"))
    {
        fprintf( stderr, "Strings %5d strings of %7d bytes (max %d).\n",
	    nAllocString, sAllocString, MAX_STRING );
        fprintf( stderr, "Perms   %5d blocks  of %7d bytes.\n\n",
	    nAllocPerm, sAllocPerm );
    }	

    return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}


void place_tree( ROOM_INDEX_DATA *pRoom )
{
    int chance;
    OBJ_DATA *tree;

    for ( tree = pRoom->contents; tree; tree = tree->next_content )
	if ( tree->pIndexData->vnum == OBJ_VNUM_TREE )
	    return;

    if ( IS_SET(pRoom->room_flags, ROOM_NOTREE) )
	return;

    switch( pRoom->sector_type )
    {
	default:
	    return;
	case SECT_FIELD:
	    chance = number_range( 5, 8 );
	    break;
	case SECT_FOREST:
	    chance = number_range( 9, 15 );
	    break;
	case SECT_HILLS:
	    chance = number_range( 8, 10 );
	    break;
	case SECT_MOUNTAIN:
	    chance = number_range( 7, 9 );
	    break;
	case SECT_DESERT:
	    chance = number_range( 0, 1 );
	    break;
	case SECT_SWAMP:
	    chance = number_range( 4, 10 );
	    break;
    }
    tree = create_object( get_obj_index(OBJ_VNUM_TREE), 0 );
    free_string( tree->name );
    free_string( tree->short_descr );
    free_string( tree->description );

    if ( pRoom->area->climate == CLIMATE_TROPICAL )
    {
	switch( pRoom->area->terrain )
	{
	    default:
		extract_obj( tree );
		return;
	    case TERRAIN_GRASSLAND:
	    case TERRAIN_DESERT:
		break;
	    case TERRAIN_FOREST:
		chance += 35;
		break;
	    case TERRAIN_SWAMP:
		chance += 20;
		break;
	    case TERRAIN_MOUNTAIN:
		chance += 7;
		break;
	    case TERRAIN_COAST:
		chance += 10;
		break;
	}
	if ( number_percent() > chance )
	{
	    extract_obj( tree );
	    return;
	}
	switch( number_bits(2) )
	{
	    default:
	    case 0:
	    case 1:
		tree->name	  = str_dup( "tree palm" );
		tree->short_descr = str_dup( "a tall palm tree" );
		tree->description = str_dup( "A tall palm tree's shade covers a small area." );
		break;
	    case 2:
		tree->pIndexData  = get_obj_index( 4290 );
		tree->item_type   = ITEM_FRUIT_TREE;
		tree->value[0]	  = FRUIT_ORANGE;
		tree->value[1]	  = number_range(1, 10);
		tree->name	  = str_dup( "tree orange" );
		tree->short_descr = str_dup( "an orange tree" );
		tree->description = str_dup( "An orange tree stands near you, lending its shade and fruit." );
		break;
	    case 3:
		tree->pIndexData  = get_obj_index( 4290 );
		tree->item_type   = ITEM_FRUIT_TREE;
		tree->value[0]	  = FRUIT_PEACH;
		tree->value[1]	  = number_range(1, 12);
		tree->name	  = str_dup( "tree peach" );
		tree->short_descr = str_dup( "a peach tree" );
		tree->description = str_dup( "A peach tree stands near you, lending its shade and fruit." );
		break;
	}
	obj_to_room( tree, pRoom );
	return;
    }

    if ( pRoom->area->climate == CLIMATE_SUBTROPICAL )
    {
	switch( pRoom->area->terrain )
	{
	    default:
		extract_obj( tree );
		return;
	    case TERRAIN_GRASSLAND:
		chance += 4;
		break;
	    case TERRAIN_DESERT:
		chance += 2;
		break;
	    case TERRAIN_FOREST:
		chance += 36;
		break;
	    case TERRAIN_SWAMP:
		chance += 18;
		break;
	    case TERRAIN_MOUNTAIN:
		chance += 5;
		break;
	    case TERRAIN_COAST:
		chance += 4;
		break;
	}
	if ( number_percent() > chance )
	{
	    extract_obj( tree );
	    return;
	}
	switch( number_bits(2) )
	{
	    default:
	    case 0:
		tree->name	  = str_dup( "tree palm" );
		tree->short_descr = str_dup( "a tall palm tree" );
		tree->description = str_dup( "A tall palm tree's shade covers a small area." );
		break;
	    case 1:
		tree->name	  = str_dup( "tree willow" );
		tree->short_descr = str_dup( "a willow tree" );
		tree->description = str_dup( "A large willow tree stands near you." );
		break;
	    case 2:
		tree->pIndexData  = get_obj_index( 4290 );
		tree->item_type   = ITEM_FRUIT_TREE;
		tree->value[0]	  = FRUIT_APPLE;
		tree->value[1]	  = number_range(1, 10);
		tree->name	  = str_dup( "tree apple" );
		tree->short_descr = str_dup( "an apple tree" );
		tree->description = str_dup( "An apple tree stands near you, lending its shade and fruit." );
		break;
	    case 3:
		tree->item_type   = ITEM_FRUIT_TREE;
		tree->value[0]	  = FRUIT_CHERRY;
		tree->value[1]	  = number_range(1, 12);
		tree->name	  = str_dup( "tree cherry" );
		tree->short_descr = str_dup( "a cherry tree" );
		tree->description = str_dup( "A cherry tree stands near you, lending its shade and fruit." );
		break;
	}
	obj_to_room( tree, pRoom );
	return;
    }

    if ( pRoom->area->climate == CLIMATE_TEMPERATE )
    {
	switch( pRoom->area->terrain )
	{
	    default:
		extract_obj( tree );
		return;
	    case TERRAIN_GRASSLAND:
		chance += 10;
		break;
	    case TERRAIN_DESERT:
		chance -= 2;
		break;
	    case TERRAIN_FOREST:
		chance += 42;
		break;
	    case TERRAIN_SWAMP:
		chance += 8;
		break;
	    case TERRAIN_MOUNTAIN:
		chance += 10;
		break;
	    case TERRAIN_COAST:
		break;
	}
	if ( number_percent() > chance )
	{
	    extract_obj( tree );
	    return;
	}
	switch( number_bits(2) )
	{
	    default:
	    case 0:
		tree->name	  = str_dup( "tree pine" );
		tree->short_descr = str_dup( "a tall pine tree" );
		tree->description = str_dup( "A tall pine tree stands nearby." );
		break;
	    case 1:
		tree->name	  = str_dup( "tree willow" );
		tree->short_descr = str_dup( "a willow tree" );
		tree->description = str_dup( "A large willow tree stands near you." );
		break;
	    case 2:
		tree->item_type   = ITEM_FRUIT_TREE;
		tree->value[0]	  = FRUIT_APPLE;
		tree->value[1]	  = number_range(1, 10);
		tree->name	  = str_dup( "tree apple" );
		tree->short_descr = str_dup( "an apple tree" );
		tree->description = str_dup( "An apple tree stands near you, lending its shade and fruit." );
		break;
	    case 3:
		tree->name	  = str_dup( "tree fir" );
		tree->short_descr = str_dup( "a fir tree" );
		tree->description = str_dup( "A fir tree looms over you." );
		break;
	}
	obj_to_room( tree, pRoom );
	return;
    }
    extract_obj( tree );
    return;
}
