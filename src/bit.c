/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



struct flag_stat_type
{
    const struct flag_type *structure;
    bool stat;
};



/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
/*  {	structure		stat	}, */
    {	area_flags,		FALSE	},
    {   sex_flags,		TRUE	},
    {   exit_flags,		FALSE	},
    {   door_resets,		TRUE	},
    {   room_flags,		FALSE	},
    {   sector_flags,		TRUE	},
    {   type_flags,		TRUE	},
    {   extra_flags,		FALSE	},
    {   wear_flags,		FALSE	},
    {   act_flags,		FALSE	},
    {   affect_flags,		FALSE	},
    {   affect_2_flags,		FALSE	},
    {   apply_flags,		TRUE	},
    {   wear_loc_flags,		TRUE	},
    {   wear_loc_strings,	TRUE	},
    {   weapon_flags,		TRUE	},
    {   container_flags,	FALSE	},
    {   liquid_flags,		TRUE	},

/* ROM specific flags: */

    {   material_type,          TRUE    },
    {   form_flags,             FALSE   },
    {   part_flags,             FALSE   },
    {   ac_type,                TRUE    },
    {   size_flags,             TRUE    },
    {   position_flags,         TRUE    },
    {   off_flags,              FALSE   },
    {   imm_flags,              FALSE   },
    {   res_flags,              FALSE   },
    {   vuln_flags,             FALSE   },
    {   weapon_class,           TRUE    },
    {   weapon_type,            FALSE   },
    {	furniture_flags,	FALSE	},
    {	ingredient_flags,	TRUE	},
    {	armor_flags,		FALSE	},
    {	fruit_flags,		TRUE	},
    {	climate_flags,		TRUE	},
    {	weather_flags,		TRUE	},
    {	terrain_flags,		TRUE	},
    {   0,			0	}
};
    


/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table )
{
    int flag;

    for (flag = 0; flag_stat_table[flag].structure; flag++)
    {
	if ( flag_stat_table[flag].structure == flag_table
	  && flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}



/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:		flag_lookup( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;
 
    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++)
    {
        if ( !str_cmp( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
int flag_value( const struct flag_type *flag_table, char *argument)
{
    char word[MAX_INPUT_LENGTH];
    int  bit;
    int  marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ) )
    {
	one_argument( argument, word );

	if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
	    return bit;
	else
	    return NO_FLAG;
    }

    /*
     * Accept multiple flags.
     */
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
	    break;

        if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
        {
            SET_BIT( marked, bit );
            found = TRUE;
        }
    }

    if ( found )
	return marked;
    else
	return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, int bits )
{
    static char buf[512];
    int  flag;

    buf[0] = '\0';

    for (flag = 0; flag_table[flag].name[0] != '\0'; flag++)
    {
	if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	}
	else
	if ( flag_table[flag].bit == bits )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }
    return (buf[0] != '\0') ? buf+1 : "none";
}



const struct flag_type area_flags[] =
{
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		TRUE	},
    {	"added",		AREA_ADDED,		TRUE	},
    {	"will-remove",		AREA_REMOVE,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {	"testing",		AREA_TESTING,		TRUE	},
    {	"completed",		AREA_COMPLETE,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               3,                      TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type exit_flags[] =
{
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {	"hidden",		EX_HIDDEN,		TRUE	},
    {	"open-exit",		EX_OPENEXIT,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	"",			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {   "exploration",		ROOM_EXPLORE,		FALSE	},
    {   "falling_room",		ROOM_FALL,		TRUE	},
    {	"no_tree",		ROOM_NOTREE,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {	"quest_shop",		ROOM_QUEST_SHOP,	TRUE	},
    {	"stedding",		ROOM_STEDDING,		TRUE	},
    {	"bank",			ROOM_BANK,		TRUE	},
    {	"warded",		ROOM_WARDED,		TRUE	},
    {	"snared",		ROOM_SNARED,		FALSE	},
    {	"fog",			ROOM_FOG,		TRUE	},
    {	"no_noise",		ROOM_NONOISE,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {	"underground",	SECT_UNDERGROUND,	TRUE	},
    {	"swamp",	SECT_SWAMP,		TRUE	},
    {	"",		0,			0	}
};



const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"tool",			ITEM_TOOL,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drink-container",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npc_corpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc_corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"clothing",		ITEM_CLOTHING,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {	"faq",			ITEM_FAQ,		TRUE	},
    {	"ore",			ITEM_ORE,		TRUE    },
    {	"vial",			ITEM_VIAL,		TRUE    },
    {	"gate",			ITEM_GATE,		TRUE    },
    {	"ingredient",		ITEM_INGREDIENT,	TRUE	},
    {	"fruit_tree",		ITEM_FRUIT_TREE,	TRUE	},
    {	"",			0,			0	}
};


const struct flag_type extra_flags[] =
{
    {	"small-shield",		ITEM_SMALL_SHIELD,	TRUE	},
    {	"terangreal",		ITEM_TERANGREAL,	TRUE	},
    {	"lock",			ITEM_LOCK,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"rot-death",		ITEM_ROT_DEATH,		TRUE	},
    {	"vis-death",		ITEM_VIS_DEATH,		TRUE	},
    {	"male-only",		ITEM_MALE_ONLY,		TRUE	},
    {	"female-only",		ITEM_FEMALE_ONLY,	TRUE	},
    {	"nobreak",		ITEM_NOBREAK,		TRUE	},
    {	"quest",		ITEM_QUEST,		TRUE	},
    {	"room-affect",		ITEM_ROOM_AFFECT,	FALSE	},
    {	"",			0,			0	}
};



const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "two-hands",            ITEM_TWO_HANDS,         TRUE    },
    {	"",			0,			0	}
};



const struct flag_type act_flags[] =
{
    {	"npc",			ACT_IS_NPC,		FALSE	},
    {	"sentinel",		ACT_SENTINEL,		TRUE	},
    {	"aggressive",		ACT_AGGRESSIVE,		TRUE	},
    {	"stay-area",		ACT_STAY_AREA,		TRUE	},
    {	"no-exp",		ACT_NOEXP,		TRUE	},
    {	"wimpy",		ACT_WIMPY,		TRUE	},
    {	"pet",			ACT_PET,		TRUE	},
    {	"train",		ACT_TRAIN,		TRUE	},
    {	"practice",		ACT_PRACTICE,		TRUE	},
    {	"channeler",		ACT_CHANNELER,		TRUE	},
    {	"scholar",		ACT_SCHOLAR,		TRUE	},
    {	"thief",		ACT_ROGUE,		TRUE	},
    {	"warrior",		ACT_WARRIOR,		TRUE	},
    {	"noalign",		ACT_NOALIGN,		TRUE	},
    {	"nopurge",		ACT_NOPURGE,		TRUE	},
    {	"healer",		ACT_IS_HEALER,		TRUE	},
    {	"gain",			ACT_GAIN,		TRUE	},
    {	"update-always",	ACT_UPDATE_ALWAYS,	TRUE	},
    {	"forge_armor",		ACT_FORGE_ARMOR,	TRUE	},
    {	"forge_weapon",		ACT_FORGE_WEAPON,	TRUE	},
    {	"mount",		ACT_MOUNT,		TRUE	},
    {	"banker",		ACT_BANKER,		TRUE	},
    {	"stay-outside",		ACT_STAY_OUTSIDE,	TRUE	},
    {	"stay-inside",		ACT_STAY_INSIDE,	TRUE	},
    {	"fixer",		ACT_FIXER,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type affect_flags[] =
{
    {	"blind",		AFF_BLIND,		TRUE	},
    {	"invisible",		AFF_INVISIBLE,		TRUE	},
    {	"detect-invis",		AFF_DETECT_INVIS,	TRUE	},
    {	"warded",		AFF_WARDED,		TRUE	},
    {	"awareness",		AFF_AWARENESS,		TRUE	},
    {	"air-armor",		AFF_AIR_ARMOR,		TRUE	},
    {	"poison",		AFF_POISON,		TRUE	},
    {	"stalk",		AFF_STALK,		FALSE	},
    {	"sneak",		AFF_SNEAK,		TRUE	},
    {	"hide",			AFF_HIDE,		TRUE	},
    {	"sleep",		AFF_SLEEP,		TRUE	},
    {	"charm",		AFF_CHARM,		TRUE	},
    {	"flying",		AFF_FLYING,		TRUE	},
    {	"haste",		AFF_HASTE,		TRUE	},  /* ROM: */
    {	"calm",			AFF_CALM,		TRUE	},
    {	"plague",		AFF_PLAGUE,		TRUE	},
    {	"weaken",		AFF_WEAKEN,		TRUE	},
    {	"dark-vision",		AFF_DARK_VISION,	TRUE	},
    {	"berserk",		AFF_BERSERK,		TRUE	},
    {	"wrap",			AFF_WRAP,		TRUE	},
    {	"gag",			AFF_GAG,		TRUE	},
    {	"slow",			AFF_SLOW,		TRUE	},
    {	"regeneration",		AFF_REGENERATION,	TRUE	},
    {	"",			0,			0	}
};

const struct flag_type affect_2_flags[] =
{
    {   "still",		AFF_STILL,		TRUE	},
    {   "link",			AFF_LINK,		TRUE	},
    {   "no-aggro",		AFF_NOAGGRO,		TRUE	},
    {   "leashed",		AFF_LEASHED,		FALSE	},
    {	"",			0,			0	}
};


/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"charisma",		APPLY_CHR,		TRUE	},
    {	"luck",			APPLY_LUK,		TRUE	},
    {	"agility",		APPLY_AGI,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",			APPLY_AGE,		TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"stamina",		APPLY_STAMINA,		TRUE	},
    {	"gold",			APPLY_GOLD,		TRUE	},
    {	"experience",		APPLY_EXP,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"addskill",		APPLY_SKILL,		TRUE	},
    {	"addspell",		APPLY_SPELL,		TRUE	},
    {	"subskill",		APPLY_SUBSKILL,		TRUE	},
    {	"",			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the body",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {	"on the legs",		WEAR_LEGS,	TRUE	},
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,	TRUE	},
    {	"held in the hands",	WEAR_HOLD,	TRUE	},
    {	"wielded secondary",	WEAR_SECONDARY,	TRUE	},
    {	"",			0			}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"hold",		WEAR_HOLD,	TRUE	},
    {	"secondary",	WEAR_SECONDARY,	TRUE	},
    {	"",		0,		0	}
};



const struct flag_type weapon_flags[] =
{
    {	"hit",		0,	TRUE	},
    {	"slice",	1,	TRUE	},
    {	"stab",		2,	TRUE	},
    {	"slash",	3,	TRUE	},
    {	"whip",		4,	TRUE	},
    {	"claw",		5,	TRUE	},
    {	"blast",	6,	TRUE	},
    {	"pound",	7,	TRUE	},
    {	"crush",	8,	TRUE	},
    {	"grep",		9,	TRUE	},
    {	"bite",		10,	TRUE	},
    {	"pierce",	11,	TRUE	},
    {	"suction",	12,	TRUE	},
    {	"beating",	13,	TRUE	},  /* ROM */
    {	"digestion",	14,	TRUE	},
    {	"charge",	15,	TRUE	},
    {	"slap",		16,	TRUE	},
    {	"punch",	17,     TRUE	},
    {	"wrath",	18,	TRUE	},
    {	"magic",	19,	TRUE	},
    {	"divine-power",	20,	TRUE	},
    {	"cleave",	21,	TRUE	},
    {	"scratch",	22,	TRUE	},
    {	"peck-pierce",  23,	TRUE	},
    {	"peck-bash",	24,	TRUE	},
    {	"chop",		25,	TRUE	},
    {	"sting",	26,	TRUE	},
    {	"smash",	27,	TRUE	},
    {	"shocking-bite", 28,	TRUE	},
    {	"flaming-bite",	 29,	TRUE	},
    {	"freezing-bite", 30,	TRUE    },
    {	"acidic-bite",	31,	TRUE	},
    {	"chomp",	32,	TRUE	},
    {   "explosion",	33,	TRUE	},
    {   "flaming-weapon",34,	TRUE	},
    {   "freezing-weapon",35,	TRUE	},
    {   "vampiric-weapon",36,	TRUE	},
    {	"",		0,	TRUE	}
};


const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"",			0,		0	}
};



const struct flag_type liquid_flags[] =
{
    {	"water",		0,	TRUE	},
    {	"beer",			1,	TRUE	},
    {	"wine",			2,	TRUE	},
    {	"ale",			3,	TRUE	},
    {	"dark-ale",		4,	TRUE	},

    {	"whiskey",		5,	TRUE	},
    {	"lemonade",		6,	TRUE	},
    {	"firebreather",		7,	TRUE	},
    {	"local-specialty",	8,	TRUE	},
    {	"slime-mold-juice",	9,	TRUE	},

    {	"milk",			10,	TRUE	},
    {	"tea",			11,	TRUE	},
    {	"coffee",		12,	TRUE	},
    {	"blood",		13,	TRUE	},
    {	"salt-water",		14,	TRUE	},

    {	"coke",			15,	TRUE	},
    {	"root beer",		16,	TRUE	},
    {	"elvish wine",		17,	TRUE	},
    {	"white wine",		18,	TRUE	},
    {	"champagne",		19,	TRUE	},

    {	"mead",			20,	TRUE	},
    {	"rose wine",		21,	TRUE	},
    {	"benedictine wine",	22,	TRUE	},
    {	"vodka",		23,	TRUE	},
    {	"cranberry juice",	24,	TRUE	},

    {	"orange juice",		25,	TRUE	},
    {	"absinthe",		26,	TRUE	},
    {	"brandy",		27,	TRUE	},
    {	"aquavit",		28,	TRUE	},
    {	"schnapps",		29,	TRUE	},

    {	"icewine",		30,	TRUE	},
    {	"amontillado",		31,	TRUE	},
    {	"sherry",		32,	TRUE	},
    {	"framboise",		33,	TRUE	},
    {	"rum",			34,	TRUE	},

    {	"cordial",		35,	TRUE	},
    {	"mountain dew",		36,	TRUE	},
    {	"oil",			37,	TRUE	},
    {	"none",			38,	TRUE	},
    {	"",			0,	0	}
};



/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/

const struct flag_type form_flags[] =
{
    {   "edible",        FORM_EDIBLE,          TRUE    },
    {   "poison",        FORM_POISON,          TRUE    },
    {   "magical",       FORM_MAGICAL,         TRUE    },
    {   "decay",         FORM_INSTANT_DECAY,   TRUE    },
    {   "other",         FORM_OTHER,           TRUE    },

    {   "animal",        FORM_ANIMAL,          TRUE    },
    {   "sentient",      FORM_SENTIENT,        TRUE    },
    {   "undead",        FORM_UNDEAD,          TRUE    },
    {   "construct",     FORM_CONSTRUCT,       TRUE    },
    {   "mist",          FORM_MIST,            TRUE    },
    {   "intangible",    FORM_INTANGIBLE,      TRUE    },

    {   "biped",         FORM_BIPED,           TRUE    },
    {   "centaur",       FORM_CENTAUR,         TRUE    },
    {   "insect",        FORM_INSECT,          TRUE    },
    {   "spider",        FORM_SPIDER,          TRUE    },
    {   "crustacean",    FORM_CRUSTACEAN,      TRUE    },
    {   "worm",          FORM_WORM,            TRUE    },
    {   "blob",          FORM_BLOB,            TRUE    },

    {   "mammal",        FORM_MAMMAL,          TRUE    },
    {   "bird",          FORM_BIRD,            TRUE    },
    {   "reptile",       FORM_REPTILE,         TRUE    },
    {   "snake",         FORM_SNAKE,           TRUE    },
    {   "dragon",        FORM_DRAGON,          TRUE    },
    {   "amphibian",     FORM_AMPHIBIAN,       TRUE    },
    {   "fish",          FORM_FISH,            TRUE    },
    {   "cold-blood",    FORM_COLD_BLOOD,      TRUE    },
    {   "",              0,                    0       }
};

const struct flag_type part_flags[] =
{
    {   "head",          PART_HEAD,            TRUE    },
    {   "arms",          PART_ARMS,            TRUE    },
    {   "legs",          PART_LEGS,            TRUE    },
    {   "heart",         PART_HEART,           TRUE    },
    {   "brains",        PART_BRAINS,          TRUE    },
    {   "guts",          PART_GUTS,            TRUE    },
    {   "hands",         PART_HANDS,           TRUE    },
    {   "feet",          PART_FEET,            TRUE    },
    {   "fingers",       PART_FINGERS,         TRUE    },
    {   "ear",           PART_EAR,             TRUE    },
    {   "eye",           PART_EYE,             TRUE    },
    {   "long-tongue",   PART_LONG_TONGUE,     TRUE    },
    {   "eyestalks",     PART_EYESTALKS,       TRUE    },
    {   "fins",          PART_TENTACLES,       TRUE    },
    {   "wings",         PART_FINS,            TRUE    },
    {   "tail",          PART_WINGS,           TRUE    },

    {   "claws",         PART_CLAWS,           TRUE    },
    {   "fangs",         PART_FANGS,           TRUE    },
    {   "horns",         PART_HORNS,           TRUE    },
    {   "scales",        PART_SCALES,          TRUE    },
    {   "tusks",         PART_TUSKS,           TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   "",              0,                    0       },
};


const struct flag_type weapon_class[] =
{
    {	"exotic",	WEAPON_EXOTIC,		TRUE	},
    {	"sword",	WEAPON_SWORD,		TRUE	},
    {	"dagger",	WEAPON_DAGGER,		TRUE	},
    {	"spear",	WEAPON_SPEAR,		TRUE	},
    {	"mace",		WEAPON_MACE,		TRUE	},
    {	"axe",		WEAPON_AXE,		TRUE	},
    {	"flail",	WEAPON_FLAIL,		TRUE	},
    {	"whip",		WEAPON_WHIP,		TRUE	},
    {	"polearm",	WEAPON_POLEARM,		TRUE	},
    {	"staff",	WEAPON_STAFF,		TRUE	},
    {	"hand-to-hand",	WEAPON_HAND_TO_HAND,	FALSE	},
    {	"bow",		WEAPON_BOW,		TRUE	},
    {	"crossbow",	WEAPON_CROSSBOW,	TRUE	},
    {	"sling",	WEAPON_SLING,		TRUE	},
    {	"javelin",	WEAPON_JAVELIN,		TRUE	},
    {	"warhammer",	WEAPON_WARHAMMER,	TRUE	},

    {	"",		0,			0	}
};


const struct flag_type weapon_type[] =
{
    {	"flaming",	WEAPON_FLAMING,		TRUE	},
    {	"frost",	WEAPON_FROST,		TRUE	},
    {	"sheathed",	WEAPON_SHEATHED,	FALSE	},
    {	"sharp",	WEAPON_SHARP,		TRUE	},
    {	"two-hands",	WEAPON_TWO_HANDS,	TRUE	},
    {	"poisoned",	WEAPON_POISON,		TRUE	},
    {	"no-fumble",	WEAPON_NOFUMBLE,	TRUE	},
    {	"",		0,			0	}
};


const struct flag_type off_flags[] =
{
    {   "area-attack",   OFF_AREA_ATTACK,      TRUE    },
    {   "backstab",      OFF_BACKSTAB,         TRUE    },
    {   "bash",          OFF_BASH,             TRUE    },
    {   "berserk",       OFF_BERSERK,          TRUE    },
    {   "disarm",        OFF_DISARM,           TRUE    },
    {   "dodge",         OFF_DODGE,            TRUE    },
    {   "fade",          OFF_FADE,             TRUE    },
    {   "fast",          OFF_FAST,             TRUE    },
    {   "kick",          OFF_KICK,             TRUE    },
    {   "kick-dirt",     OFF_KICK_DIRT,        TRUE    },
    {   "parry",         OFF_PARRY,            TRUE    },
    {   "rescue",        OFF_RESCUE,           TRUE    },
    {   "tail",          OFF_TAIL,             TRUE    },
    {   "trip",          OFF_TRIP,             TRUE    },
    {   "crush",         OFF_CRUSH,            TRUE    },
    {   "assist-all",    ASSIST_ALL,           TRUE    },
    {   "assist-race",   ASSIST_RACE,          TRUE    },
    {   "assist-player", ASSIST_PLAYERS,       TRUE    },
    {   "assist-guard",  ASSIST_GUARD,         TRUE    },
    {   "assist-vnum",   ASSIST_VNUM,          TRUE    },
    {   "assist-guild",  ASSIST_GUILD,         TRUE    },
    {	"poison",	 OFF_POISON,	       TRUE    },
    {	"track",	 OFF_TRACK,	       TRUE    },
    {	"memory",	 OFF_MEMORY,		TRUE	},
    {   "",              0,			0	}
};


const struct flag_type imm_flags[] =
{
    {   "summon",        IMM_SUMMON,           TRUE    },
    {   "charm",         IMM_CHARM,            TRUE    },
    {   "magic",         IMM_MAGIC,            TRUE    },
    {   "weapon",        IMM_WEAPON,           TRUE    },
    {   "bash",          IMM_BASH,             TRUE    },
    {   "pierce",        IMM_PIERCE,           TRUE    },
    {   "slash",         IMM_SLASH,            TRUE    },
    {   "fire",          IMM_FIRE,             TRUE    },
    {   "cold",          IMM_COLD,             TRUE    },
    {   "lightning",     IMM_LIGHTNING,        TRUE    },
    {   "acid",          IMM_ACID,             TRUE    },
    {   "poison",        IMM_POISON,           TRUE    },
    {   "negative",      IMM_NEGATIVE,         TRUE    },
    {   "holy",          IMM_HOLY,             TRUE    },
    {   "energy",        IMM_ENERGY,           TRUE    },
    {   "mental",        IMM_MENTAL,           TRUE    },
    {   "disease",       IMM_DISEASE,          TRUE    },
    {   "drowning",      IMM_DROWNING,         TRUE    },
    {   "light",         IMM_LIGHT,            TRUE    },
    {   "",          0,            0    }
};


const struct flag_type res_flags[] =
{
    {   "charm",         RES_CHARM,            TRUE    },
    {   "magic",         RES_MAGIC,            TRUE    },
    {   "weapon",        RES_WEAPON,           TRUE    },
    {   "bash",          RES_BASH,             TRUE    },
    {   "pierce",        RES_PIERCE,           TRUE    },
    {   "slash",         RES_SLASH,            TRUE    },
    {   "fire",          RES_FIRE,             TRUE    },
    {   "cold",          RES_COLD,             TRUE    },
    {   "lightning",     RES_LIGHTNING,        TRUE    },
    {   "acid",          RES_ACID,             TRUE    },
    {   "poison",        RES_POISON,           TRUE    },
    {   "negative",      RES_NEGATIVE,         TRUE    },
    {   "holy",          RES_HOLY,             TRUE    },
    {   "energy",        RES_ENERGY,           TRUE    },
    {   "mental",        RES_MENTAL,           TRUE    },
    {   "disease",       RES_DISEASE,          TRUE    },
    {   "drowning",      RES_DROWNING,         TRUE    },
    {   "light",         RES_LIGHT,            TRUE    },
    {   "",          0,            0    }
};


const struct flag_type vuln_flags[] =
{
    {   "magic",         VULN_MAGIC,           TRUE    },
    {   "weapon",        VULN_WEAPON,          TRUE    },
    {   "bash",          VULN_BASH,            TRUE    },
    {   "pierce",        VULN_PIERCE,          TRUE    },
    {   "slash",         VULN_SLASH,           TRUE    },
    {   "fire",          VULN_FIRE,            TRUE    },
    {   "cold",          VULN_COLD,            TRUE    },
    {   "lightning",     VULN_LIGHTNING,       TRUE    },
    {   "acid",          VULN_ACID,            TRUE    },
    {   "poison",        VULN_POISON,          TRUE    },
    {   "negative",      VULN_NEGATIVE,        TRUE    },
    {   "holy",          VULN_HOLY,            TRUE    },
    {   "energy",        VULN_ENERGY,          TRUE    },
    {   "mental",        VULN_MENTAL,          TRUE    },
    {   "disease",       VULN_DISEASE,         TRUE    },
    {   "drowning",      VULN_DROWNING,        TRUE    },
    {   "light",         VULN_LIGHT,           TRUE    },
    {   "wood",          VULN_WOOD,            TRUE    },
    {   "silver",        VULN_SILVER,          TRUE    },
    {   "iron",          VULN_IRON,            TRUE    },
    {   "",              0,                    0       }
};


const struct flag_type material_type[] = 
{
    {	"none",		0,		TRUE	},
    {	"wood",		MAT_WOOD,	TRUE	},
    {	"bone",		MAT_BONE,	TRUE	},
    {	"iron",		MAT_IRON,	TRUE	},
    {	"steel",	MAT_STEEL,	TRUE	},
    {	"silver",	MAT_SILVER,	TRUE	},
    {	"gold",		MAT_GOLD,	TRUE    },
    {	"darksilver",	MAT_DARKSILVER,	TRUE	},
    {	"heartstone",	MAT_HEARTSTONE,	TRUE	},
    {	"cloth",	MAT_CLOTH,	TRUE	},
    {	"leather",	MAT_LEATHER,	TRUE	},
    {	"silk",		MAT_SILK,	TRUE	},
    {	"stone",	MAT_STONE,	TRUE	},
    {	"obsidian",	MAT_OBSIDIAN,	TRUE	},
    {	"gem",		MAT_GEM_OTHER,	TRUE	},
    {	"diamond",	MAT_DIAMOND,	TRUE	},
    {	"ruby",		MAT_RUBY,	TRUE	},
    {	"sapphire",	MAT_SAPPHIRE,	TRUE	},
    {	"emerald",	MAT_EMERALD,	TRUE	},
    {	"opal",		MAT_OPAL,	TRUE	},
    {   "paper",	MAT_PAPER,	TRUE	},
    {   "glass",	MAT_GLASS,	TRUE	},
    {   "copper",	MAT_COPPER,	TRUE	},
    {   "soft-leather",	MAT_SOFT_LEATHER,TRUE	},
    {   "clay",		MAT_CLAY,	TRUE	},
    {   "special",	MAT_SPECIAL,	TRUE	},
    {	"satin",	MAT_SATIN,	TRUE	},
    {	"velvet",	MAT_VELVET,	TRUE	},
    {	"bamboo",	MAT_BAMBOO,	TRUE	},
    {	"electrum",	MAT_ELECTRUM,	TRUE	},
    {	"silvery-stuff",MAT_SILVERY_STUFF,TRUE	},
    {   "unknown",	MAT_UNKNOWN,	TRUE	},
    {   "",		0,		0	}
};


const struct flag_type position_flags[] =
{
    {	"dead",		POS_DEAD,		FALSE	},
    {	"mortal",	POS_MORTAL,		FALSE	},
    {	"incap",	POS_INCAP,		FALSE	},
    {	"stunned",	POS_STUNNED,		FALSE	},
    {	"sleeping",	POS_SLEEPING,		TRUE	},
    {	"resting",	POS_RESTING,		TRUE	},
    {	"sitting",	POS_SITTING,		TRUE	},
    {	"fighting",	POS_FIGHTING,		FALSE	},
    {	"mounted",	POS_MOUNTED,		FALSE	},
    {	"standing",	POS_STANDING,		TRUE	},
    {	"",		0,			0	}
};

const struct flag_type furniture_flags[] =
{
    {   "stand-at",	STAND_AT,		TRUE	},
    {   "stand-on",	STAND_ON,		TRUE	},
    {   "stand-in",	STAND_IN,		TRUE	},

    {   "sit-at",	SIT_AT,			TRUE	},
    {   "sit-on",	SIT_ON,			TRUE	},
    {   "sit-in",	SIT_IN,			TRUE	},

    {   "rest-at",	REST_AT,		TRUE	},
    {   "rest-on",	REST_ON,		TRUE	},
    {   "rest-in",	REST_IN,		TRUE	},

    {   "sleep-at",	SLEEP_AT,		TRUE	},
    {   "sleep-on",	SLEEP_ON,		TRUE	},
    {   "sleep-in",	SLEEP_IN,		TRUE	},

    {   "put-at",	PUT_AT,			TRUE	},
    {   "put-on",	PUT_ON,			TRUE	},
    {   "put-in",	PUT_IN,			TRUE	},
    {   "put-inside",	PUT_INSIDE,		TRUE	},

    {	"",		0,			0	}
};


const struct flag_type armor_flags[] =
{
    {	"neck",		ITEM_WEAR_NECK,		TRUE	},
    {	"body",		ITEM_WEAR_BODY,		TRUE	},
    {	"head",		ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",		ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",		ITEM_WEAR_FEET,		TRUE	},
    {	"hands",	ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",		ITEM_WEAR_ARMS,		TRUE	},
    {	"waist",	ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",	ITEM_WEAR_WRIST,	TRUE	},
    {	"about",	ITEM_WEAR_ABOUT,	TRUE	},

    {	"",		0,			0	}
};

const struct flag_type ingredient_flags[] =
{
    {	"aldaka",	HERB_ALDAKA,		TRUE	},
    {	"cureall",	HERB_CUREALL,		TRUE	},
    {	"belramba",	HERB_BELRAMBA,		TRUE	},
    {	"cats_tail",	HERB_CATS_TAIL,		TRUE	},
    {	"dagmather",	HERB_DAGMATHER,		TRUE	},
    {	"fetherfew",	HERB_FETHERFEW,		TRUE	},
    {	"goats_rue",	HERB_GOATS_RUE,		TRUE	},
    {	"hares_ears",	HERB_HARES_EARS,	TRUE	},
    {	"hore_hound",	HERB_HOREHOUND,		TRUE	},
    {	"masterwort",	HERB_MASTERWORT,	TRUE	},
    {	"pargen",	HERB_PARGEN,		TRUE	},
    {	"sweet_trefoile",HERB_SWEET_TREFOILE,	TRUE	},
    {	"orach",	HERB_ORACH,		TRUE	},
    {	"mugwort",	HERB_MUGWORT,		TRUE	},
    {	"willow_herb",	HERB_WILLOW_HERB,	TRUE	},

    {	"",		0,			0	}
};

const struct flag_type fruit_flags[] =
{
    {	"apple",	FRUIT_APPLE,		TRUE	},
    {	"banana",	FRUIT_BANANA,		TRUE	},
    {	"pear",		FRUIT_PEAR,		TRUE	},
    {	"cherry",	FRUIT_CHERRY,		TRUE	},
    {	"orange",	FRUIT_ORANGE,		TRUE	},
    {	"peach",	FRUIT_PEACH,		TRUE	},
    {	"plum",		FRUIT_PLUM,		TRUE	},

    {	"",		0,			0	}
};

const struct flag_type climate_flags[] =
{
    {	"tropical",	CLIMATE_TROPICAL,	TRUE	},
    {	"subtropical",	CLIMATE_SUBTROPICAL,	TRUE	},
    {	"temperate",	CLIMATE_TEMPERATE,	TRUE	},
    {	"arctic",	CLIMATE_ARCTIC,		TRUE	},

    {	"",		0,			0	}
};

const struct flag_type terrain_flags[] =
{
    {	"grassland",	TERRAIN_GRASSLAND,	TRUE	},
    {	"forest",	TERRAIN_FOREST,		TRUE	},
    {	"swamp",	TERRAIN_SWAMP,		TRUE	},
    {	"mountain",	TERRAIN_MOUNTAIN,	TRUE	},
    {	"coast",	TERRAIN_COAST,		TRUE	},
    {	"desert",	TERRAIN_DESERT,		TRUE	},
    {	"hill",		TERRAIN_HILL,		TRUE	},

    {	"",		0,			0	}
};

const struct flag_type weather_flags[] =
{
    {	"sunny",	WEATHER_SUNNY,		TRUE	},
    {	"cloudy",	WEATHER_CLOUDY,		TRUE	},
    {	"foggy",	WEATHER_FOGGY,		TRUE	},
    {	"rainy",	WEATHER_RAINY,		TRUE	},
    {	"snowy",	WEATHER_SNOWY,		TRUE	},
    {	"stormy",	WEATHER_STORMY,		TRUE	},

    {	"",		0,			0	}
};
