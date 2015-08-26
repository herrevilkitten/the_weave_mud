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
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[]		=
{
    { 	"hit",		-1		},  /*  0 */
    {	"slice", 	DAM_SLASH	},	
    {   "stab",		DAM_PIERCE	},
    {	"slash",	DAM_SLASH	},
    {	"whip",		DAM_SLASH	},
    {   "claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	DAM_BASH	},
    {   "pound",	DAM_BASH	},
    {	"crush",	DAM_BASH	},
    {   "grep",		DAM_SLASH	},
    {	"bite",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	DAM_PIERCE	},
    {   "suction",	DAM_BASH	},
    {	"beating",	DAM_BASH	},
    {   "digestion",	DAM_ACID	},
    {	"charge",	DAM_BASH	},  /* 15 */
    { 	"slap",		DAM_BASH	},
    {	"punch",	DAM_BASH	},
    {	"wrath",	DAM_ENERGY	},
    {	"magic",	DAM_ENERGY	},
    {   "divine power",	DAM_HOLY	},  /* 20 */
    {	"cleave",	DAM_SLASH	},
    {	"scratch",	DAM_PIERCE	},
    {   "peck",		DAM_PIERCE	},
    {   "peck",		DAM_BASH	},
    {   "chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	DAM_PIERCE	},
    {   "smash",	DAM_BASH	},
    {   "shocking bite",DAM_LIGHTNING	},
    {	"flaming bite", DAM_FIRE	},
    {	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acidic bite", 	DAM_ACID	},
    {	"chomp",	DAM_PIERCE	},
    {	"explosion",	DAM_FIRE	},
    {	"flaming weapon",DAM_FIRE	},
    {	"freezing weapon",DAM_COLD	},
    {	"vampiric weapon",DAM_NEGATIVE	}
};


/* weapon selection table */
const   struct  weapons_type     weapon_table    []      =
{
/*
	Type-name,		diff,	fumble,	crit,	range,
	dice,	max TH bonus,max dam bonus,speed modifier
*/
    {	WEAPON_EXOTIC,		75,	10,	95,	0,
	10,	0,	0,	0				},
    {	WEAPON_SWORD,		56,	5,	93,	0,
	10,	8,	15,	2				},
    {	WEAPON_MACE,		66,	7,	95,	0,
	6,	5,	10,	3				},
    {	WEAPON_DAGGER,		47,	3,	91,	1,
	3,	8,	2,	1				},
    {	WEAPON_AXE,		61,	6,	94,	0,
	12,	4,	20,	3				},
    {	WEAPON_SPEAR,		56,	5,	93,	2,
	6,	12,	5,	2				},
    {	WEAPON_FLAIL,		75,	9,	97,	0,
	8,	8,	8,	2				},
    {	WEAPON_WHIP,		55,	4,	92,	0,
	4,	15,	2,	1				},
    {	WEAPON_POLEARM,		85,	11,	99,	0,
	16,	2,	30,	4				},
    {	WEAPON_STAFF,		80,	10,	98,	0,
	7,	6,	20,	3				},
    {	WEAPON_HAND_TO_HAND,	70,	5,	93,	0,
	0,	0,	0,	0				},
    {	WEAPON_BOW,		67,	13,	98,	4,
	6,	5,	20,	3				},
    {	WEAPON_CROSSBOW,	75,	15,	99,	6,
	8,	8,	35,	4				},
    {	WEAPON_SLING,		60,	10,	98,	3,
	4,	5,	10,	2				},
    {	WEAPON_JAVELIN,		60,	8,	95,	2,
	5,	5,	15,	2				},
    {	WEAPON_WARHAMMER,	65,	5,	95,	0,
	11,	10,	15,	3				},

    {	0,			0,	0,	0,	0,
	0,	0,	0,	0				}
};


/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
    {	"on",		WIZ_ON,		IM	},
    {	"prefix",	WIZ_PREFIX,	IM	},
    {	"ticks",	WIZ_TICKS,	IM	},
    {	"logins",	WIZ_LOGINS,	IM	},
    {	"sites",	WIZ_SITES,	IM	},
    {	"links",	WIZ_LINKS,	IM	},
    {	"newbies",	WIZ_NEWBIE,	IM	},
    {	"spam",		WIZ_SPAM,	L5	},
    {	"deaths",	WIZ_DEATHS,	IM	},
    {	"resets",	WIZ_RESETS,	L4	},
    {	"mobdeaths",	WIZ_MOBDEATHS,	L4	},
    {	"flags",	WIZ_FLAGS,	L5	},
    {	"penalties",	WIZ_PENALTIES,	L5	},
    {	"saccing",	WIZ_SACCING,	IM	},
    {	"levels",	WIZ_LEVELS,	IM	},
    {	"load",		WIZ_LOAD,	L2	},
    {	"restore",	WIZ_RESTORE,	L2	},
    {	"snoops",	WIZ_SNOOPS,	L4	},
    {	"switches",	WIZ_SWITCHES,	L4	},
    {	"secure",	WIZ_SECURE,	IM	},
    {	"insane",	WIZ_INSANE,	IM	},
    {	"stats",	WIZ_STAT,	IM	},
    {	"creation",	WIZ_CREATE,	IM	},
    {	"memory",	WIZ_MEMORY,	IM	},
    {	NULL,		0,		0	} 
};
   


/* Breakage table -- For the cool stuff :) */
const	struct	break_type	break_table	[]		=
{
/*  {	Breakage Multiplier,	Breakage Affect,
	Weight Multiplier,	Cost Multiplier,
	Armor min,		Armor Max,
	fire mod,	cold mod,	lightning mod,
	vnum,
	skill gsn					} */

	/* unknown */
    {	0,			AFF_NOTHING,
	100,			100,
	0,			0,		0,
	0,			0,		0,
	NULL						},
	/* wood */
    {	175,			AFF_SHATTER,
	65,			100,
	4,			9,		
	-4,			-2,		-3,
	OBJ_VNUM_LUMBER,
	&gsn_carpentry					},
	/* bone */
    {	150,			AFF_SHATTER,
	50,			100,
	3,			5,
	-2,			-3,		-3,
	0,
	NULL						},
	/* iron */
    {	250,			AFF_SHATTER,
	100,			100,
	6,			12,
	-2,			0,		-6,
	OBJ_VNUM_IRON,
	&gsn_smithing					},
	/* steel */
    {	500,			AFF_SHATTER,
	125,			100,
	10,			18,
	-1,			0,		-4,
	0,
	&gsn_smithing						},
	/* silver */
    {	75,			AFF_BEND,
	120,			125,
	3,			6,
	0,			0,		-3,
	OBJ_VNUM_SILVER,
	&gsn_smithing					},
	/* gold */
    {	100,			AFF_BEND,
	250,			150,
	2,			5,
	1,			-1,		-2,
	OBJ_VNUM_GOLD,
	&gsn_smithing					},
	/* darksilver */
    {	1000,			AFF_EXPLODE,
	70,			550,
	15,			18,
	1,			0,		0,
	0,
	NULL						},
	/* heartstone */
    {	0,			AFF_NOTHING,
	60,			800,
	20,			20,
	0,			0,		0,
	0,
	NULL						},
	/* cloth */
    {   0,                      AFF_TEAR,
        30,			100,
	1,			3,
	-6,			3,		0,
	OBJ_VNUM_CLOTH,
	&gsn_sewing						},
	/* leather */
    {   0,                      AFF_TEAR,
        75,			100,
	3,			7,
	2,			4,		1,
	OBJ_VNUM_HARD_LEATHER,
	&gsn_leatherworking				},
	/* silk */
    {	70,			AFF_TEAR,
	10,			110,
	1,			2,
	-8,			0,		-2,
	OBJ_VNUM_SILK,
	&gsn_sewing					},
	/* stone */
    {	200,			AFF_SHATTER,
	300,			100,
	5,			10,	
	5,			5,		4,
	0,
	NULL						},
	/* obsidian */
    {	275,			AFF_SHATTER,
	135,			100,
	4,			8,
	4,			5,		4,
	0,
	NULL						},
	/* other gems */
    {   400,			AFF_SHATTER,
        150,			135,
	6,			10,
	5,			5,		4,
	OBJ_VNUM_OTHER_GEM,
	&gsn_gemworking					},
	/* diamond */
    {   1200,			AFF_SHATTER,
        300,			180,
	12,			19,
	5,			5,		4,
	OBJ_VNUM_DIAMOND,
	&gsn_gemworking					},
	/* ruby */
    {   800,			AFF_SHATTER,
	250,			140,
	9,			15,
	5,			5,		4,
	OBJ_VNUM_RUBY,
	&gsn_gemworking					},
	/* sapphire */
    {   800,			AFF_SHATTER,
        250,			140,
	9,			15,
	5,			5,		4,
	OBJ_VNUM_SAPPHIRE,
	&gsn_gemworking					},
	/* emerald */
    {   900,			AFF_SHATTER,
	275,			150,
	11,			16,
	5,			5,		4,
	OBJ_VNUM_EMERALD,
	&gsn_gemworking					},
	/* opal */
    {   600,			AFF_SHATTER,
        200,			145,
	7,			12,
	5,			5,		4,
	OBJ_VNUM_OPAL,
	&gsn_gemworking					},
	/* paper */
    {   10,			AFF_TEAR,
       	1,			100,
	0,			1,
	-10,			0,		-2,
	0,
	NULL						},
	/* glass */
    {   10,			AFF_SHATTER,
        80,			100,
	1,			1,
	2,			-2,		4,
	0,
	NULL						},
	/* copper */
    {   90,			AFF_SHATTER,
	90,			100,
	3,			7,
	1,			3,		3,
	OBJ_VNUM_COPPER,
	&gsn_smithing					},
	/* soft leather */
    {	65,			AFF_TEAR,
	5,			105,
	2,			5,
	1,			1,		2,
	OBJ_VNUM_SOFT_LEATHER,
	&gsn_leatherworking				},
	/* clay */
    {	15,			AFF_SHATTER,
	125,			100,
	1,			3,
	2,			2,		2,
	0,
	NULL						},
	/* special */
    {	100,			AFF_NOTHING,
	100,			0,
	10,			20,
	0,			0,		0,
	0,
	NULL						},
	/* satin */
    {   0,			AFF_TEAR,
        25,			100,
	1,			4,
	-2,			1,		0,
	OBJ_VNUM_SATIN,
	&gsn_sewing					},
	/* velvet */
    {   0,			AFF_TEAR,
       	50,			100,
	1,			4,
	-2,			2,		0,
	OBJ_VNUM_VELVET,
	&gsn_sewing					},
	/* bamboo */
    {   0,			AFF_SHATTER,
        55,			100,
	9,			12,
	3,			3,		2,
	0,
	&gsn_carpentry					},
	/* electrum */
    {   87,			AFF_BEND,
        145,			110,
	3,			5,
	-1,			-2,		3,
	OBJ_VNUM_ELECTRUM,
	&gsn_smithing					},
	/* silvery stuff */
    {   87,			AFF_NOTHING,
        100,			0,
	1,			3,
	0,			0,		0,
	0,
	NULL						},

    {   -1,			AFF_NOTHING,
        0,			0,
	0,			0,
	0,			0,		0,
	0,
	NULL						}
};

const	struct	forge_type	forge_table	[]		=
{
/*	
    {
	name,	full name,	v0,	v1,	v2,	v3,
	wear flags
	Weapon -->		type	dice	sides	message	weight
	Armor -->		pierce	bash	slash	exotic	weight
    }
*/

/*	Weapons		*/

    {	"axe",		"axe",		5,	2,	5,	25, 7	},
    {	"sword",	"sword",	1,	2,	5,	3,  4	},
    {	"mace",		"mace",		4,	2,	4,	7,  5	},
    {	"flail",	"flail",	6,	1,	7,	7,  5	},
    {	"polearm",	"polearm",	8,	1,	7,	2,  10	},
    {	"dagger",	"dagger",	2,	1,	4,	2,  2	},
    {	"whip",		"chain whip",	7,	1,	6,	4,  2	},

/*	Armor		*/

    {	"ring",		"ring",		1,	1,	1,	1,  1	},
    {	"amulet",	"amulet",	3,	3,	3,	3,  1	},
    {	"armor",	"suit of armor",4,	4,	3,	1,  10	},
    {	"helm",		"helm",		4,	4,	3,	1,  3	},
    {	"leggings",	"leggings",	4,	4,	3,	1,  6	},
    {	"boots",	"plate boots",	4,	4,	3,	1,  4	},
    {	"gauntlets",	"gauntlets",	4,	4,	3,	1,  4	},
    {	"armplates",	"arm plates",	4,	4,	3,	1,  6	},
    {	"bracers",	"bracers",	2,	4,	4,	1,  3	},
    {	"shield",	"shield",	4,	4,	4,	2,  7	},
    {	NULL,		NULL,		0,	0,	0,	0	}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    { "unique",         FALSE, 0, 0, 0, 0, 0, 0, 100, 0, 0 },

    {
	"Aielman",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Amadician",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Andoran",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Borderlander",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Cairhienin",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Domani",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Illianer",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Seanchan",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"Tairen",		TRUE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
	"ogier",		FALSE,
	0,		0,		OFF_ACROBATIC,
	0,		0,		0,
	1,		4,		1,
	100,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K,
	SIZE_LARGE
    },

    {
        "human",                FALSE,
        0,              0,              OFF_ACROBATIC,
        0,              0,              0,
	1,		2,		0,
	100,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
        "bat",                  FALSE,
        0,              AFF_FLYING|AFF_DARK_VISION,     OFF_DODGE|OFF_FAST,
        0,              0,              VULN_LIGHT,
	1,		2,		0,
	20,
        A|G|V,          A|C|D|E|F|H|J|K|P,
	SIZE_SMALL
    },

    {
        "bear",                 FALSE,
        0,              0,              OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
        0,              RES_BASH|RES_COLD,      0,
	1,		4,		0,
	135,
        A|G|V,          A|B|C|D|E|F|H|J|K|U|V,
	SIZE_LARGE
    },

    {
        "cat",                  FALSE,
        0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
        0,              0,              0,
	1,		3,		0,
	40,
        A|G|V,          A|C|D|E|F|H|J|K|Q|U|V,
	SIZE_SMALL
    },

    {
        "centipede",            FALSE,
        0,              AFF_DARK_VISION,        0,
        0,              RES_PIERCE|RES_COLD,    VULN_BASH,
	1,		1,		0,
	10,
        A|B|G|O,                A|C|K,
	SIZE_TINY
    },

    {
        "dog",                  FALSE,
        0,              0,              OFF_FAST,
        0,              0,              0,
	1,		3,		0,
	45,
        A|G|V,          A|C|D|E|F|H|J|K|U|V,
	SIZE_SMALL
    },

    {
        "dragkhar",			FALSE,
        0,		AFF_FLYING,	OFF_ACROBATIC,
        0,		0,		0,
	1,		3,		0,
	100,
	B|G|H|M|W,	A|B|C|D|E|F|G|I|J|K|P|U,
	SIZE_MEDIUM
    },

    {
        "fox",				FALSE,
        0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
        0,              0,              0,
	1,		2,		0,
	35,
        A|G|V,          A|C|D|E|F|H|J|K|Q|V,
	SIZE_SMALL
    },

    {
        "horse",            FALSE,
        0,              0,   			OFF_FAST|OFF_KICK,
        0,              0,		0,
	1,		4,		0,
	145,
        A|G|V,        	A|C|D|E|F|J|K,
	SIZE_LARGE
    },

    {
        "lizard",               FALSE,
        0,              0,              0,
        0,              RES_POISON,     VULN_COLD,
	1,		2,		0,
	20,
        A|G|X|cc,       A|C|D|E|F|H|K|Q|V,
	SIZE_SMALL
    },

    {
	"myrddraal",		FALSE,
	0,		AFF_AWARENESS|AFF_DARK_VISION|AFF_SNEAK,
	OFF_KICK|OFF_KICK_DIRT|ASSIST_RACE|OFF_ACROBATIC,
	0,		RES_POISON,			0,
	1,		3,		0,
	225,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },	

    {
        "pig",                  FALSE,
        0,              0,              0,
        0,              0,              0,
	1,		2,		0,
	60,
        A|G|V,          A|C|D|E|F|H|J|K,
	SIZE_SMALL
    },

    {
        "rabbit",               FALSE,
        0,              0,              OFF_DODGE|OFF_FAST,
        0,              0,              0,
	1,		1,		0,
	30,
        A|G|V,          A|C|D|E|F|H|J|K,
	SIZE_SMALL
    },

    {
        "rat",                  FALSE,
        0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
        0,              0,              0,
	1,		2,		0,
	20,
        A|G|V,          A|C|D|E|F|H|J|K|Q|U|V,
	SIZE_SMALL
    },

    {
        "snake",                FALSE,
        0,              0,              0,
        0,              RES_POISON,     VULN_COLD,
	1,		2,		0,
	20,
        A|G|X|Y|cc,     A|D|E|F|K|L|Q|V|X,
	SIZE_SMALL
    },

    {
        "song bird",            FALSE,
        0,              AFF_FLYING,             OFF_FAST|OFF_DODGE,
        0,              0,              0,
	1,		2,		0,
	20,
        A|G|W,          A|C|D|E|F|H|K|P,
	SIZE_SMALL
    },

    {
	"trolloc",		FALSE,
	0,		AFF_DARK_VISION|AFF_AWARENESS,
	OFF_BERSERK|OFF_ACROBATIC,
	0,		0,		0,
	1,		2,		0,
	175,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	SIZE_MEDIUM
    },

    {
        "water fowl",           FALSE,
        0,              AFF_FLYING,		0,
        0,              RES_DROWNING,           0,
	1,		1,		0,
	20,
        A|G|W,          A|C|D|E|F|H|K|P,
	SIZE_SMALL
    },

    {
        "wolf",                 FALSE,
        0,              AFF_DARK_VISION,        OFF_FAST|OFF_DODGE,
        0,              0,              0,
	1,		5,		0,
	120,
        A|G|V,          A|C|D|E|F|J|K|Q|V,
	SIZE_MEDIUM
    },

    {
	"viper",		FALSE,
	0,		0,		OFF_POISON,
	0,		RES_POISON,	VULN_COLD,
	1,		2,		0,
	20,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X,
	SIZE_SMALL
    },

    {
        "unique",               FALSE,
        0,              0,              0,
        0,              0,              0,
	1,		4,		0,
	100,
        0,              0,
	SIZE_MEDIUM
    },

    {
        NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }

};

const   struct  pc_race_type    pc_race_table   []      =
{
    { "null race", 0, { 100, 100, 100 },
      { "" }, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0, 0 },

/*
    {
        "race name",    short name,     points, { class multipliers },
        { bonus skills },
        { base stats },         { max stats },          size
    },
*/
    {
	"Aielman",	0,	{ 100, 100, 100 },
	{ "" },
	{ 2, -2, -1, 1, 0, -2, 0, 2 },
	SIZE_MEDIUM, 80
    },	

    {
	"Amadician",	0,	{ 100, 100, 100 },
	{ "" },
	{ 2, 1, -1, 0, 0, -1, 0, 0 },
	SIZE_MEDIUM, 80
    },	

    {
        "Andoran",	0,      { 100, 100, 100 },
        { "" },
	{ 0, 2, 0, -2, -1, 0, 0, 1 },
	SIZE_MEDIUM, 80
    },

    {
	"Borderlander",	0,	{ 100, 100, 100 },
	{ "" },
	{ 2, -1, 0, 0, 1, 0, -1, 0 },
	SIZE_MEDIUM, 80
    },

    {
        "Cairhienin",	0,      { 100, 100, 100 },
        { "" },
	{ -1, 0, +2, -2, 0, 1, 0, 0 },
	SIZE_MEDIUM, 80
    },

    {
	"Domani",	0,	{ 100, 100, 100 },
	{ "" },
	{ -2, 0, 0, 1, -1, 2, 0, 0 },
	SIZE_MEDIUM, 80
    },

    {
	"Illianer",	0,	{ 100, 100, 100 },
	{ "" },
	{ 0, 1, 0, -1, -2, 0, 1, 1 },
	SIZE_MEDIUM, 80
    },

    {
	"Seanchan",	0,	{ 100, 100, 100 },
	{ "" },
	{ 2, 0, 0, 1, -1, -1, -1, 0 },
	SIZE_MEDIUM, 80
    },

    {
	"Tairen",	0,	{ 100, 100, 100 },
	{ "" },
	{ 1, 2, -2, 0, 0, -1, 0, 0 },
	SIZE_MEDIUM, 80
    }
};

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
        "warrior-type",	10,	5, 	11,	1
    },

    {
        "thief-type",	5,	10,	9,	2
    },

    {
        "scholar-type",	0,	5,	7,	3
    }

};

char *	const	wear_name[MAX_WEAR] =
{
    "",			/* 0 */
    "",
    "",
    " in $S neck",
    " in $S neck",
    " in $S torso",	/* 6 */
    " on $S head",
    " on $S legs",
    " on $S foot",
    " on $S hand",
    " on $S arm",
    "",			/* 11 */
    "",
    " along $S waist",
    " on $S left wrist",
    " on $S right wrist",
    "",			/* 16 */
    "",
    ""			/* 18 */
};

/*
 * A single set of colors for use with the COLOR command. :)
 */
   
const   struct  color_data color_table[] =
{
   { "\x1b[30m",     "",   "black",         0 },
   { "\x1b[34m",     "",   "blue",          1 },
   { "\x1b[32m",     "",   "green",         2 },
   { "\x1b[36m",     "",   "cyan",          3 },
   { "\x1b[31m",     "",   "red",           4 },
   { "\x1b[35m",     "",   "purple",        5 },
   { "\x1b[33m",     "",   "brown",         6 },
   { "\x1b[37m",     "",   "grey",          7 },
   { "\x1b[30;1m",   "",   "dark_grey",     8 },
   { "\x1b[34;1m",   "",   "light_blue",    9 },
   { "\x1b[32;1m",   "",   "light_green",  10 },
   { "\x1b[36;1m",   "",   "light_cyan",   11 },
   { "\x1b[31;1m",   "",   "light_red",    12 },
   { "\x1b[35;1m",   "",   "magenta",      13 },
   { "\x1b[33;1m",   "",   "yellow",       14 },
   { "\x1b[37;1m",   "",   "white",        15 },
   { "\x1b[1m",      "",   "bold",         16 },
   { "\x1b[7m",      "",   "inverse",      18 },
   { "\x1b[0m",      "",   "normal",       19 } 
};
   
/*
 * Attribute bonus tables.
 */
const   struct  str_app_type    str_app         [28]            =
{
    { -10,  0,  0 },  /* 0  */
    { -9,   3,  1 },  /* 1  */
    { -8,   3,  2 },
    { -7,  10,  3 },  /* 3  */
    { -6,  25,  4 },
    { -5,  55,  5 },  /* 5  */
    { -4,  80,  6 },  
    { -3,  90,  7 },
    { -2, 100,  8 },
    { -1, 100,  9 },
    { -1, 115, 10 }, /* 10  */
    {  0, 115, 11 },
    {  0, 130, 12 },
    {  0, 130, 13 }, /* 13  */
    {  1, 140, 14 },
    {  1, 150, 15 }, /* 15  */
    {  2, 165, 16 },
    {  3, 180, 22 },
    {  4, 200, 25 }, /* 18  */
    {  5, 225, 30 },
    {  6, 250, 35 }, /* 20  */
    {  7, 300, 40 },
    {  8, 350, 45 },
    {  9, 400, 50 },  
    {  10, 450, 55 },
    {  12, 500, 60 },  /* 25   */
    {  13, 550, 70 },
    {  14, 700, 95 }
};
    
    
    
const   struct  int_app_type    int_app         [28]            =
{
    {  3 },     /*  0 */
    {  4 },     /*  1 */
    {  4 },
    {  6 },     /*  3 */
    {  6 },
    {  7 },     /*  5 */
    {  7 },
    {  7 },
    {  8 },
    { 10 },
    { 11 },     /* 10 */
    { 13 },
    { 15 },
    { 17 },
    { 20 },
    { 21 },     /* 15 */
    { 21 },
    { 24 },
    { 26 },     /* 18 */
    { 29 },
    { 32 },     /* 20 */
    { 36 },
    { 40 },
    { 46 },
    { 53 },
    { 56 },      /* 25 */
    { 60 },
    { 65 }      /* 25 */
};
    
    
    
const   struct  wis_app_type    wis_app         [28]            =
{
    { 0 },      /*  0 */
    { 0 },      /*  1 */
    { 1 }, 
    { 1 },      /*  3 */
    { 1 }, 
    { 1 },      /*  5 */
    { 2 }, 
    { 2 }, 
    { 2 }, 
    { 2 }, 
    { 3 },      /* 10 */
    { 3 },
    { 3 },
    { 3 },
    { 3 },
    { 4 },      /* 15 */
    { 4 },
    { 4 },
    { 4 },      /* 18 */
    { 4 }, 
    { 4 },      /* 20 */
    { 5 }, 
    { 5 },
    { 6 }, 
    { 6 }, 
    { 6 },      /* 25 */
    { 7 }, 
    { 7 }       /* 27 */
};    

const   struct  dex_app_type    dex_app         [28]            =
{
    {   -40, -45 },   /* 0 */ 
    {   -35, -35 },   /* 1 */
    {   -30, -25 },
    {   -25, -20 },
    {   -20, -15 },
    {   -15, -10 },   /* 5 */ 
    {   -10,  -8 },
    {    -8,  -6 },
    {    -6,  -4 },
    {    -4,  -2 },
    {    -2,   0 },   /* 10 */
    {     0,   0 },
    {     0,   0 },
    {     0,   0 },
    {     2,   2 },
    {     4,   4 },   /* 15 */
    {     6,   6 },
    {     8,   8 },
    {    10,  10 },
    {    15,  15 },
    {    20,  25 },   /* 20 */
    {    25,  30 },
    {    30,  35 },
    {    35,  40 },
    {    40,  50 },
    {    45,  55 },    /* 25 */
    {    50,  60 },
    {    60,  70 }    /* 25 */
};
    
    
const   struct  con_app_type    con_app         [28]            =
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -3, 30 },
    { -2, 35 },   /*  3 */
    { -2, 40 },
    { -2, 45 },   /*  5 */
    { -1, 50 },
    { -1, 55 },
    { -1, 60 },
    { -1, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  0, 90 },   /* 15 */
    {  1, 95 },
    {  1, 97 },
    {  1, 99 },   /* 18 */
    {  1, 99 },
    {  2, 99 },   /* 20 */
    {  2, 99 },
    {  2, 99 },
    {  3, 99 },
    {  3, 99 },
    {  4, 99 },    /* 25 */
    {  5, 99 },
    {  6, 99 }     /* 27 */
};


const   struct  cha_app_type    cha_app         [28]            =
{
    {  -20 },   /* 0 */ 
    {  -17 },   /* 1 */
    {  -15 },
    {  -13 },
    {  -10 },
    {   -5 },   /* 5 */ 
    {   -2 },
    {   -1 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    1 },
    {    2 },   /* 15 */
    {    5 },
    {    7 },
    {   10 },
    {   13 },
    {   15 },   /* 20 */
    {   17 },
    {   20 },
    {   23 },
    {   25 },
    {   27 },   /* 25 */
    {   33 },
    {   35 }    /* 27 */
};


const	struct	luk_app_type	luk_app		[28]		=
{
    { -8, -40 },   /*  0 */
    { -7, -35 },   /*  1 */
    { -6, -30 },
    { -5, -25 },   /*  3 */
    { -4, -20 },
    { -3, -15 },   /*  5 */
    { -2, -10 },
    { -1, -5 },
    {  0, -2 },
    {  0, -1 },
    {  0,  0 },   /* 10 */
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },
    {  0,  0 },   /* 15 */
    {  0,  0 },
    {  0,  1 },
    {  0,  2 },   /* 18 */
    {  1,  5 },
    {  2, 10 },   /* 20 */
    {  3, 15 },
    {  4, 20 },
    {  5, 25 },
    {  6, 30 },
    {  7, 35 },    /* 25 */
    {  8, 40 },
    {  9, 45 }    /* 25 */
};


const   struct  agi_app_type    agi_app         [28]            =
{
    {  300 },   /* 0 */ 
    {  280 },   /* 1 */
    {  260 },
    {  240 },
    {  220 },
    {  200 },   /* 5 */ 
    {  180 },
    {  160 },
    {  140 },
    {  120 },
    {  100 },   /* 10 */
    {  100 },
    {  100 },
    {  100 },
    {  100 },
    {  100 },   /* 15 */
    {   95 },
    {   90 },
    {   85 },
    {   80 },
    {   75 },   /* 20 */
    {   70 },
    {   65 },
    {   60 },
    {   55 },
    {   50 },    /* 25 */
    {   45 },
    {   40 }    /* 25 */
};
    
    


/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
/*    name			color		 drunk, full, thirst */
    { "water",			"clear",	{ 0, 1, 10 }	},
    { "beer",			"amber",	{ 2, 1,  8 }	},
    { "red wine",		"burgundy",	{ 5, 1,  8 }	},
    { "ale",			"brown",	{ 3, 1,  8 }	},
    { "dark ale",		"dark",		{ 3, 1,  8 }	}, /*4*/

    { "whiskey",		"golden",	{ 20, 1, 5 }	},
    { "lemonade",		"pink",		{ 0, 1,  9 }	},
    { "firebreather",		"boiling",	{ 31, 0, 4 }	},
    { "local specialty",	"clear",	{ 25, 1, 3 }	},
    { "slime mold juice",	"green",	{ 0, 2, -8 }	},/*9*/

    { "milk",			"white",	{ 0, 2,  9 }	},
    { "tea",			"tan",		{ 0, 1,  8 }	},
    { "coffee",			"black",	{ 0, 1,  8 }	},
    { "blood",			"red",		{ 0, 2, -1 }	},
    { "salt water",		"clear",	{ 0, 1, -2 }	},/*14*/

    { "coke",			"brown",	{ 0, 2,  9 }	}, 
    { "root beer",		"brown",	{ 0, 2,  9 }	},
    { "elvish wine",		"green",	{ 6, 2,  8 }	},
    { "white wine",		"golden",	{ 5, 1,  8 }	},
    { "champagne",		"golden",	{ 5, 1,  8 }	},/*19*/

    { "mead",			"honey-colored",{ 6, 2,  8 }	},
    { "rose wine",		"pink",		{ 4, 1,  8 }	},
    { "benedictine wine",	"burgundy",	{ 7, 1,  8 }	},
    { "vodka",			"clear",	{ 21, 1, 5 }	},
    { "cranberry juice",	"red",		{ 0, 1,  9 }	},/*24*/

    { "orange juice",		"orange",	{ 0, 2,  9 }	}, 
    { "absinthe",		"green",	{ 33, 1, 4 }	},
    { "brandy",			"golden",	{ 13, 1, 5 }	},
    { "aquavit",		"clear",	{ 23, 1, 5 }	},
    { "schnapps",		"clear",	{ 15, 1, 5 }	},/*29*/

    { "icewine",		"purple",	{ 8, 2,  6 }	},
    { "amontillado",		"burgundy",	{ 6, 2,  8 }	},
    { "sherry",			"red",		{ 6, 2,  7 }	},
    { "framboise",		"red",		{ 8, 1,  7 }	},
    { "rum",			"amber",	{ 25, 1, 4 }	},/*34*/

    { "cordial",		"clear",	{ 16, 1, 5 }	},
    { "mountain dew",		"pale green",	{ 0, 1,  4 }	},
    { "oil",			"clear",	{ 0, 0, 0  }	},/*37*/
    { "none",			"very clear",	{ 0, 0, 0 },	}

};

const	struct	herb_type	herb_table[]			=
{
    {	HERB_ALDAKA,		SECT_MOUNTAIN,	95,	OBJ_VNUM_ALDAKA	},
    {	HERB_CUREALL,		SECT_FOREST,	75,	OBJ_VNUM_CUREALL},
    {	HERB_BELRAMBA,		SECT_SWAMP,	90,	OBJ_VNUM_BELRAMBA},
    {	HERB_CATS_TAIL,		SECT_WATER_SWIM,95,	OBJ_VNUM_CATS_TAIL},
    {	HERB_DAGMATHER,		SECT_FIELD,	95,	OBJ_VNUM_DAGMATHER},
    {	HERB_FETHERFEW,		SECT_FOREST,	85,	OBJ_VNUM_FETHERFEW},
    {	HERB_GOATS_RUE,		SECT_HILLS,	90,	OBJ_VNUM_GOATS_RUE},
    {	HERB_HARES_EARS,	SECT_MOUNTAIN,	85,	OBJ_VNUM_HARES_EARS},
    {	HERB_HOREHOUND,		SECT_WATER_SWIM,80,	OBJ_VNUM_HOREHOUND},
    {	HERB_MASTERWORT,	SECT_HILLS,	85,	OBJ_VNUM_MASTERWORT},
    {	HERB_PARGEN,		SECT_SWAMP,	99,	OBJ_VNUM_PARGEN	},
    {	HERB_SWEET_TREFOILE,	SECT_FOREST,	75,	OBJ_VNUM_SWEET_TREFOILE},
    {	HERB_ORACH,		SECT_DESERT,	95,	OBJ_VNUM_ORACH	},
    {	HERB_MUGWORT,		SECT_FIELD,	75,	OBJ_VNUM_MUGWORT},
    {	HERB_WILLOW_HERB,	SECT_FOREST,	70,	OBJ_VNUM_WILLOW_HERB},

    {	HERB_NONE,		-1,		100,	0	}
};

const	struct	potion_type	potion_table[]			=
{
    {	"nothing",	0,	0,	0,	0	},

    {
	"linmus balm",
	HERB_PARGEN|HERB_ALDAKA,
	25,	POTION_BALM,
	HERB_HEAL|HERB_DRAIN
    },

    {
	"alean salve",
	HERB_ORACH|HERB_SWEET_TREFOILE,
	65,	POTION_BALM,
	HERB_SLEEP|HERB_REFRESH
    },

    {
	"syrup of cat's tail",
	HERB_CATS_TAIL,
	45,	POTION_DRINK,
	HERB_POISON|HERB_HEAL|HERB_REFRESH
    },

    {
	"sarium pills",
	HERB_PARGEN|HERB_HOREHOUND,
	95,	POTION_EAT,
	HERB_STOP_CHANNEL|HERB_SLEEP
    },

    {
	"willow weed extract",
	HERB_WILLOW_HERB,
	85,	POTION_DRINK,
	HERB_SLEEP|HERB_POISON|HERB_HEAL|HERB_REFRESH|HERB_CURE_DISEASE
    },

    {	NULL,	0,	0,	0,	0	}
};

const	struct	talent_type	talent_table[MAX_TALENT + 10]	=
{
    {
	"heightened muscles",			75,	TRUE,
	&tn_str_talent
    },
    {
	"superior intellect",			75,	TRUE,
	&tn_int_talent
    },
    {
	"wisdom beyond years",			75,	TRUE,
	&tn_wis_talent
    },
    {
	"quick muscles",			75,	TRUE,
	&tn_dex_talent
    },
    {
	"hardy body and endurance",		75,	TRUE,	/* 4 */
	&tn_con_talent
    },
    {
	"personality and looks",		75,	TRUE,
	&tn_cha_talent
    },
    {
	"strange luck",				75,	TRUE,
	&tn_luc_talent
    },
    {
	"lightning reflexes",			75,	TRUE,
	&tn_agi_talent
    },
    {
	"blademastery",				98,	TRUE,
	&tn_blademastery
    },
    {
	"master thief",				98,	TRUE,
	&tn_master_thief
    },
    {
	"craftsmanship",			78,	TRUE,
	&tn_craftsmanship
    },
    {
	"earth",				7,	FALSE,	/* 9 */
	&tn_earth_talent
    },
    {
	"air",					7,	FALSE,
	&tn_air_talent
    },
    {
	"fire",					7,	FALSE,
	&tn_fire_talent
    },
    {
	"water",				7,	FALSE,
	&tn_water_talent
    },
    {
	"spirit",				12,	FALSE,
	&tn_spirit_talent
    },
    {
	"herbalism",				65,	TRUE,
	&tn_herbalism
    },
    {
	"geology",				85,	TRUE,
	&tn_geology
    },   
    {
	"teaching",				80,	TRUE,
	&tn_teaching
    },   
    {
	"sense taveren",			20,	FALSE,	/* 14 */
	&tn_sense_taveren
    },
    {
	"sense terangreal",			25,	FALSE,
	&tn_sense_terangreal
    },
    {
	"create terangreal",			99,	FALSE,
	&tn_create_terangreal
    },
    {
	"dreamwalking",				95,	TRUE,
	&tn_dreamwalking
    },
    {
	"foretelling",				99,	FALSE,
	&tn_foretelling
    },
    {
	"shielding",				90,	FALSE,	/* 19 */
	&tn_shielding
    },
    {
	"charm/illusion",			75,	FALSE,
	&tn_charm
    },
    {
	"combat",				90,	FALSE,
	&tn_combat
    },
    {
	"creation",				65,	FALSE,
	&tn_creation
    },
    {
	"detection",				15,	FALSE,
	&tn_detection
    },
    {
	"enchantment",				45,	FALSE,	/* 24 */
	&tn_enchantment
    },
    {
	"healing",				90,	FALSE,
	&tn_healing
    },
    {
	"maladictions",				60,	FALSE,
	&tn_maladictions
    },
    {
	"protective",				25,	FALSE,
	&tn_protective
    },
    {
	"transportation",			93,	FALSE,
	&tn_transportation
    },
    {
	"weather",				95,	FALSE,	/* 29 */
	&tn_weather
    },
    {
	"strong channeler",			98,	FALSE,
	&tn_channeler
    },
    {
	"powerful channeler",			99,	FALSE,
	&tn_powerful_channeler
    },
    {
	"weak taveren",				89,	TRUE,	/* 29 */
	&tn_weak_taveren
    },
    {
	"taveren",				94,	TRUE,	/* 29 */
	&tn_taveren
    },
    {
	"strong taveren",			99,	TRUE,	/* 29 */
	&tn_strong_taveren
    }

};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

    {
	"reserved",		{ 199, 199, 199 },	{ 199, 199, 199 },
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	0,	0,
	"",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"flame dart",		{ 1, 1, 1 },		{ 2, 2, 2 },
	spell_flame_dart,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 1),	0,	2,
	"flaming dart",		"!flame dart!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 5, 0, 0 }
    },

    {
	"shield from true source",{ 60, 60, 60 },	{ 7, 7, 7 },
	spell_shield_from_true_source,TAR_CHAR_OTHER,	POS_STANDING,
	&gsn_shield,		SLOT( 2),	0,	8,
	"",			"You can touch the True Source again.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 60 }
    },

    {
	"gag",			{ 45, 45, 45 },		{ 5, 5, 5 },
	spell_gag,		TAR_CHAR_OTHER,	POS_STANDING,
	NULL,			SLOT( 3),	0,	5,
	"",			"You can speak again.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 40, 0, 0, 0 }
    },

    {
	"wrap",			{ 45, 45, 45 },		{ 5, 5, 5 },
	spell_wrap,		TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT( 4),	0,	5,
	"",			"You can move again.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 35, 0, 0, 0 }
    },

    {
	"cut weave",		{ 55, 55, 55 },		{ 6, 6, 6 },
	spell_cut_weave,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 5),	0,	4,
	"",			"!cut weave!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 30, 0, 25 }
    },

    {
	"minor healing",	{ 25, 25, 25 },		{ 1, 1, 1 },
	spell_minor_heal,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT( 6),	0,	2,
	"",			"!minor healing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 10, 0, 20, 15 }
    },

    {
	"healing",		{ 45, 45, 45 },		{ 3, 3, 3 },
	spell_heal,		TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT( 7),	0,	3,
	"",			"!healing!",
	"minor healing",	0,
	FALSE,			FALSE,
	{ 0, 20, 0, 40, 30 }
    },

    {
	"major healing",	{ 75, 75, 75 },		{ 7, 7, 7 },
	spell_major_heal,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT( 8),	0,	5,
	"",			"!major healing!",
	"healing",		0,
	FALSE,			FALSE,
	{ 0, 30, 0, 60, 45 }
    },

    {
	"refreshment",		{ 10, 10, 10 },		{ 2, 2, 2 },
	spell_refreshment,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT( 9),	0,	2,
	"",			"!refreshment!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 20, 35 }
    },

    {
	"cure blindness",	{ 12, 12, 12 },		{ 2, 2, 2 },
	spell_cure_blindness,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(10),	0,	3,
	"",			"!cure blindness!",
	"probe",		0,
	FALSE,			FALSE,
	{ 0, 0, 0, 15, 20 }
    },

    {
	"cure poison",		{ 12, 12, 12 },		{ 2, 2, 2 },
	spell_cure_poison,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(11),	0,	3,
	"",			"!cure poison!",
	"probe",		0,
	FALSE,			FALSE,
	{ 0, 10, 0, 10, 15 }
    },

    {
	"cure disease",		{ 15, 15, 15 },		{ 2, 2, 2 },
	spell_cure_disease,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(12),	0,	5,
	"",			"!cure disease!",
	"probe",		0,
	FALSE,			FALSE,
	{ 0, 15, 0, 15, 20 }
    },

    {
	"bone knitting",	{ 17, 17, 17 },		{ 2, 2, 2 },
	spell_bone_knitting,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(13),	0,	2,
	"",			"!bone knitting!",
	"probe",		0,
	FALSE,			FALSE,
	{ 10, 0, 0, 0, 20 }
    },

    {
	"clot",			{ 10, 10, 10 },		{ 2, 2, 2 },
	spell_clot,		TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(14),	0,	2,
	"",			"!clot!",
	"probe",		0,
	FALSE,			FALSE,
	{ 5, 0, 5, 0, 10 }
    },

    {
	"probe",		{ 5, 5, 5 },		{ 1, 1, 1 },
	spell_probe,		TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(15),	0,	2,
	"",			"!probe!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 5 }
    },

    {
	"stone strike",		{ 8, 8, 8 },		{ 2, 2, 2 },
	spell_stone_strike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	0,	3,
	"rock",			"!stone strike!",
	NULL,			0,
	FALSE,			FALSE,
	{ 6, 0, 0, 0, 0 }
    },

    {
	"wind mace",		{ 12, 12, 12 },		{ 2, 2, 2 },
	spell_wind_mace,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(17),	0,	4,
	"club of air",		"!wind mace!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 15, 0, 0, 0 }
    },

    {
	"flame column",		{ 100, 100, 100 },	{ 7, 7, 7 },
	spell_flame_column,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(18),	0,	8,
	"column of fire",	"!flame column!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 50, 80, 0, 0 }
    },

    {
	"net of pain",		{ 39, 39, 39 },		{ 7, 7, 7 },
	spell_net_of_pain,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	NULL,			SLOT(19),	0,	7,
	"",			"The pain in your body ebbs away.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 39 }
    },

    {
	"skim",			{ 55, 55, 55 },		{ 6, 6, 6 },
	spell_skim,		TAR_ROOM,		POS_STANDING,
	&gsn_skim,		SLOT(20),	0,	5,
	"",			"$p snapes out of existence",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 35, 0, 0, 60 }
    },

    {
	"travel",		{ 75, 75, 75 },		{ 7, 7, 7 },
	spell_travel,		TAR_ROOM,		POS_STANDING,
	&gsn_travel,		SLOT(21),	0,	7,
	"",			"$p snaps out of existence.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 50, 0, 0, 75 }
    },

    {
	"reveal invisible",	{ 25, 25, 25 },		{ 3, 3, 3 },
	spell_reveal_invisible,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(22),	0,	2,
	"",			"!reveal invisible!",
	"detect invisible",	0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 0 }
    },

    {
	"ward room",		{ 40, 40, 40 },		{ 3, 3, 3 },
	spell_ward_room,	TAR_IN_ROOM,		POS_STANDING,
	NULL,			SLOT(23),	0,	4,
	"",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 35 }
    },

    {
	"ward person",		{ 27, 27, 27 },		{ 2, 2, 2 },
	spell_ward_person,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(24),	0,	3,
	"",			"!ward person!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 27 }
    },

    {
	"create flame sword",	{ 40, 40, 40 },		{ 4, 4, 4 },
	spell_create_flame_sword,TAR_IGNORE,		POS_STANDING,
	&gsn_create_flame_sword,SLOT(25),	0,	4,
	"",			"$p disappears in a flash of light.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 40, 0, 0 }
    },

    {
	"create air sword",	{ 40, 40, 40 },		{ 4, 4, 4 },
	spell_create_air_sword,	TAR_IGNORE,		POS_STANDING,
	&gsn_create_air_sword,	SLOT(26),	0,	4,
	"",			"$p fades away into nothingness.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 40, 0, 0, 0 }
    },

    {
	"flame wave",		{ 79, 79, 79 },		{ 8, 8, 8 },
	spell_flame_wave,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(27),	0,	6,
	"wave of flame",	"!flame wave!",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 30, 68, 0, 0 }
    },

    {
	"earth wave",		{ 58, 58, 58 },		{ 6, 6, 6 },
	spell_earth_wave,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(28),	0,	5,
	"wave of earth",	"!earth wave!",
	NULL,			0,
	FALSE,			TRUE,
	{ 68, 0, 0, 0, 0 }
    },

    {
	"windstorm",		{ 67, 67, 67 },		{ 6, 6, 6 },
	spell_windstorm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(29),	0,	7,
	"terrible wind",	"!windstorm!",
	"wind push",		0,
	FALSE,			TRUE,
	{ 0, 76, 0, 0, 0 }
    },

    {
	"fireball",		{ 36, 36, 36 },		{ 4, 4, 4 },
	spell_fireball,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(30),	0,	5,
	"fireball",		"!fireball!",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 0, 36, 0, 0 }
    },

    {
	"air shield",		{ 10, 10, 10 },		{ 1, 1, 1 },
	spell_air_shield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(31),	0,	3,
	"",			"The air in front of you seems less solid.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 10, 0, 0, 0 }
    },

    {
	"air armor",		{ 35, 35, 35 },		{ 4, 4, 4 },
	spell_air_armor,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(32),	0,	5,
	"",			"The air around you seems less solid.",
	"air shield",		0,
	FALSE,			FALSE,
	{ 0, 25, 0, 0, 0 }
    },

    {
	"wind spear",		{ 30, 30, 30 },		{ 2, 2, 2 },
	spell_wind_spear,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(33),	0,	4,
	"spear of wind",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 30, 0, 0, 0 }
    },

    {
	"create spring",	{ 11, 11, 11 },		{ 2, 2, 2 },
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	&gsn_create_spring,	SLOT(34),	0,	3,
	"",			"$p stops flowing.",
	"create water",		0,
	FALSE,			FALSE,
	{ 10, 0, 0, 15, 0 }
    },

    {
	"create water",		{ 8, 8, 8 },		{ 1, 1, 1 },
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(35),	0,	2,
	"",			"!create water!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 10, 0 }
    },

    {
	"lightning bolt",	{ 40, 40, 40 },		{ 4, 4, 4 },
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(36),	0,	5,
	"bolt of lightning",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 20, 0, 0 }
    },

    {
	"invisibility",		{ 30, 30, 30 },		{ 5, 5, 5 },
	spell_invisibility,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_invis,		SLOT(37),	0,	3,
	"",			"You snap back into existence.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 0 }
    },

    {
	"mass invisibility",	{ 45, 45, 45 },		{ 7, 7, 7 },
	spell_mass_invisibility,TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(38),	0,	5,
	"",			"",
	"invisibility",		0,
	FALSE,			TRUE,
	{ 0, 45, 0, 0, 0 }
    },

    {
	"ice ball",		{ 10, 10, 10 },		{ 2, 2, 2 },
	spell_ice_ball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(39),	0,	4,
	"ball of ice",		"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 5, 0, 5, 0 }
    },

    {
	"sleep",		{ 35, 35, 35 },		{ 5, 5, 5 },
	spell_sleep,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(40),	0,	4,
	"",			"You can wake again.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 40 }
    },

    {
	"ward object",		{ 20, 20, 20 },		{ 5, 5, 5 },
	spell_ward_object,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(41),	0,	3,
	"explosion of fire",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 20, 0, 10 }
    },

    {
	"earthquake",		{ 31, 31, 31 },		{ 5, 5, 5 },
	spell_earthquake,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(42),	0,	7,
	"earthquake",		"",
	NULL,			0,
	FALSE,			TRUE,
	{ 35, 0, 0, 0, 0 }
    },

    {
	"shape change",		{ 55, 55, 55 },		{ 6, 6, 6 },
	spell_shape_change,	TAR_IGNORE,		POS_STANDING,
	&gsn_shape_change,	SLOT(43),	0,	5,
	"",			"Your form returns to normal.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 40, 0, 35, 55 }
    },

    {
	"wind push",		{ 13, 13, 13 },		{ 3, 3, 3 },
	spell_wind_push,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(44),	0,	4,
	"",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 0 }
    },

    {
	"blindness",		{ 20, 20, 20 },		{ 3, 3, 3 },
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT(45),	0,	4,
	"",			"You can see again.",
	NULL,			0,
	FALSE,			FALSE,
	{ 35, 0, 0, 0, 0 }
    },

    {
	"poison",		{ 25, 25, 25 },		{ 3, 3, 3 },
	spell_poison,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_poison,		SLOT(46),	0,	5,
	"",			"The burning in your veins goes away.",
	NULL,			0,
	FALSE,			FALSE,
	{ 20, 0, 0, 15, 0 }
    },

    {
	"disease",		{ 28, 28, 28 },		{ 3, 3, 3 },
	spell_disease,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(47),	0,	5,
	"",			"You don't feel sick anymore.",
	NULL,			0,
	FALSE,			FALSE,
	{ 20, 0, 15, 10, 0 }
    },

    {
	"illusion",		{ 28, 28, 28 },		{ 3, 3, 3 },
	spell_illusion,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(48),	0,	4,
	NULL,			"",
	"",			0,
	FALSE,			FALSE,
	{ 0, 40, 0, 0, 35 }
    },

    {
	"ventriloquate",	{ 28, 28, 28 },		{ 3, 3, 3 },
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(49),	0,	4,
	"",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 45, 0, 0, 0 }
    },

    {
	"charm",		{ 50, 50, 50 },		{ 6, 6, 6 },
	spell_charm,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(50),	0,	6,
	"",			"You feel more free willed.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 45 }
    },

    {
	"lash",			{ 5, 5, 5 },		{ 1, 1, 1 },
	spell_lash,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(51),	0,	4,
	"whip of air",		"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 6, 0, 0, 0 }
    },

    {
	"rock shower",		{ 16, 16, 16 },		{ 3, 3, 3 },
	spell_rock_shower,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(52),	0,	5,
	"shower of rocks",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 13, 3, 0, 0, 0 }
    },

    {
	"air blade",		{ 20, 20, 20 },		{ 2, 2, 2 },
	spell_air_blade,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(53),	0,	4,
	"blade of air",		"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 0 }
    },

    {
	"steam",		{ 26, 26, 26 },		{ 4, 4, 4 },
	spell_steam,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(54),	0,	6,
	"billow of steam",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 10, 6, 10, 0 }
    },

    {
	"control weather",	{ 44, 44, 44 },		{ 9, 9, 9 },
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(55),	0,	3,
	"",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 25, 0, 30, 0 }
    },

    {
	"awareness",		{ 36, 36, 36 },		{ 2, 2, 2 },
	spell_awareness,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	0,	1,
	"",			"You're not as aware of your surroundings.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 20, 0, 0, 16 }
    },

    {
	"detect invisible",	{ 26, 26, 26 },		{ 2, 2, 2 },
	spell_detect_invisible,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(57),	0,	1,
	"",			"The tingling in your eyes goes away.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 15, 0, 0, 10 }
    },

    {
	"strengthen",		{ 18, 18, 18 },		{ 3, 3, 3 },
	spell_strengthen,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(58),	0,	2,
	"",			"You feel weaker.",
	NULL,			0,
	FALSE,			FALSE,
	{ 15, 0, 0, 0, 5 }
    },

    {
	"haste",		{ 45, 45, 45 },		{ 4, 4, 4 },
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_haste,		SLOT(59),	0,	1,
	"",			"You feel yourself slowing down.",
	NULL,			0,
	FALSE,			FALSE,
	{ 15, 30, 0, 0, 25 }
    },

    {
	"hail storm",		{ 46, 46, 46 },		{ 7, 7, 7 },
	spell_hail_storm,	TAR_IN_ROOM,		POS_STANDING,
	&gsn_hail_storm,	SLOT(60),	0,	6,
	"",			"The hail stops.",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 20, 0, 36, 0 }
    },

    {
	"lightning storm",	{ 50, 50, 50 },		{ 9, 9, 9 },
	spell_lightning_storm,	TAR_IN_ROOM,		POS_STANDING,
	&gsn_lightning_storm,	SLOT(61),	0,	8,
	"",			"The lightning stops.",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 50, 10, 0, 0 }
    },

    {
	"slow",			{ 45, 45, 45 },		{ 4, 4, 4 },
	spell_slow,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(62),	0,	1,
	"",			"You feel yourself moving faster.",
	NULL,			0,
	FALSE,			FALSE,
	{ 15, 30, 0, 0, 25 }
    },

    {
	"weaken",		{ 18, 18, 18 },		{ 3, 3, 3 },
	spell_weaken,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(63),	0,	2,
	"",			"Your strength returns.",
	NULL,			0,
	FALSE,			FALSE,
	{ 15, 0, 0, 0, 5 }
    },

    {
	"earth shield",		{ 10, 10, 10 },		{ 1, 1, 1 },
	spell_earth_shield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(64),	0,	3,
	"",			"The shield of earth crumbles away.",
	NULL,			0,
	FALSE,			FALSE,
	{ 10, 15, 0, 0, 0 }
    },

    {
	"frost wave",		{ 70, 70, 70 },		{ 8, 8, 8 },
	spell_frost_wave,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(65),	0,	6,
	"wave of snow and ice",	"!frost wave!",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 30, 0, 50, 0 }
    },

    {
	"detect ability",	{ 1, 1, 1 },		{ -1, -1, -1 },
	spell_detect_ability,	TAR_CHAR_OTHER,		POS_STANDING,
	NULL,			SLOT(66),	0,	1,
	"",			"!detect ability!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 45 }
    },

    {
	"cover channeling",	{ 10, 10, 10 },		{ 5, 5, 5 },
	spell_hide_channeling,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(67),	0,	3,
	"",			"Your channeling ability is no longer hidden.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 30 }
    },

    {
	"flame burst",		{ 1, 1, 1 },		{ -1, -1, -1 },
	spell_flame_burst,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	0,	4,
	"burst of flame",		"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 50, 0, 0 }
    },

    {
	"light ball",		{ 15, 15, 15 },		{ 1, 1, 1 },
	spell_light_ball,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(69),	0,	2,
	"",			"$p darkens and snaps out.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 12 }
    },

    {
	"light storm",		{ 33, 33, 33 },		{ 2, 2, 2 },
	spell_light_storm,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(70),	0,	4,
	"ray of light",		"",
	NULL,			0,
	FALSE,			TRUE,
	{ 0, 0, 0, 0, 30 }
    },

    {
	"air hammer",		{ 1, 1, 1 },		{ -1, -1, -1 },
	spell_air_hammer,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(71),	0,	4,
	"air of hammer",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 50, 0, 0, 0 }
    },

    {
	"frostbite",		{ 1, 1, 1 },		{ -1, -1, -1 },
	spell_frostbite,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	0,	4,
	"freezing atmosphere",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 50, 0 }
    },

    {
	"earth rending",	{ 1, 1, 1 },		{ -1, -1, -1 },
	spell_earth_rending,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(73),	0,	4,
	"rumbling earth",	"",
	NULL,			0,
	FALSE,			FALSE,
	{ 50, 0, 0, 0, 0 }
    },

    {
	"cause pain",		{ 34, 34, 34 },		{ 1, 1, 1 },
	spell_cause_pain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(74),	0,	3,
	"burning pain",		"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 30 }
    },

    {
	"delve",		{ 25, 25, 25 },		{ 3, 3, 3 },
	spell_delve,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(75),	0,	3,
	"",			"",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 40, 0, 0, 0, 0 }
    },

    {
	"wind barrier",		{ 45, 45, 45 }, 	{ 5, 5, 5 },
	spell_wind_barrier,	TAR_IGNORE,		POS_STANDING,
	&gsn_wind_barrier,	SLOT(76),	0,	5,
	"",			"The winds die down.",
	"air shield",		FALSE,
	FALSE,			FALSE,
	{ 0, 55, 0, 0, 0 }
    },

    {
	"earth barrier",	{ 50, 50, 50 }, 	{ 7, 7, 7 },
	spell_earth_barrier,	TAR_IGNORE,		POS_STANDING,
	&gsn_earth_barrier,	SLOT(77),	0,	6,
	"",			"The piles of earth collapse.",
	"earth shield",		FALSE,
	FALSE,			FALSE,
	{ 60, 0, 0, 0, 0 }
    },

    {
	"fire wall",		{ 43, 43, 43 }, 	{ 4, 4, 4 },
	spell_fire_wall,	TAR_IGNORE,		POS_STANDING,
	&gsn_fire_wall,		SLOT(78),	0,	4,
	"wall of flame",	"The flames die down.",
	"",			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 40, 0, 0 }
    },

    {
	"ice wall",		{ 40, 40, 40 }, 	{ 4, 4, 4 },
	spell_ice_wall,		TAR_IGNORE,		POS_STANDING,
	&gsn_ice_wall,		SLOT(79),	0,	4,
	"",			"The ice wall melts down to nothing.",
	"",			FALSE,
	FALSE,			FALSE,
	{ 0, 10, 0, 30, 0 }
    },

    {
	"balefire",		{ 60, 60, 60 },		{ 8, 8, 8 },
	spell_balefire,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_balefire,		SLOT(80),	0,	6,
	"bar of liquid fire",	"",
	"",			FALSE,
	FALSE,			FALSE,
	{ 60, 60, 60, 60, 60 }
    },

    {
	"confusion",		{ 30, 30, 30 },		{ 5, 5, 5 },
	spell_confusion,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	0,	2,
	"",			"You can think clearly again.",
	"",			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 35 }
    },

    {
	"sever from true source",{ 101, 101, 101 },	{ 9, 9, 9 },
	spell_sever_from_true_source,TAR_CHAR_OTHER,	POS_STANDING,
	NULL,			SLOT(82),	0,	8,
	"",			"!Sever from True Source!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 90, 90, 90, 90, 90 }
    },

/* combat and weapons skills */

    {
	"axe",			{ 1, 1, 1 },	{ 4, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_axe,		SLOT(800),
	SKILL_STR|SKILL_DEX,	0,
        "",			"!Axe!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "dagger",		{ 1, 1, 1 },	{ 1, 1, 2 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_dagger,		SLOT(801),
	SKILL_DEX,	0,
        "",			"!Dagger!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },
 
    {
	"flail",		{ 1, 1, 1 },	{ 2, 3, 3 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_flail,		SLOT(802),
	SKILL_STR|SKILL_DEX,	0,
        "",			"!Flail!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"mace",			{ 1, 1, 1 },	{ 1, 2, 4 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_mace,		SLOT(803),
	SKILL_STR,	0,
        "",			"!Mace!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"polearm",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_polearm,		SLOT(804),
	SKILL_STR,	0,
        "",			"!Polearm!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },
    
    {
	"shield block",		{ 1, 1, 1 },	{ 3, 4, 4 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_shield_block,	SLOT(805),
	SKILL_DEX|SKILL_AGI,	0,
	"",			"!Shield!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"spear",		{ 1, 1, 1 },	{ 3, 3, 3 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_spear,		SLOT(806),	
	SKILL_DEX,	0,
        "",			"!Spear!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"staff",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_staff,		SLOT(876),
	SKILL_STR,	0,
        "",			"!Staff!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"sword",		{ 1, 1, 1 },	{ 4, 4, 4 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_sword,		SLOT(807),
	SKILL_DEX|SKILL_AGI,	0,
	"",			"!sword!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"whip",			{ 1, 1, 1 },	{ 3, 3, 3 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_whip,		SLOT(808),
	SKILL_DEX|SKILL_AGI,	0,
        "",			"!whip!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "backstab",		{ 101, 7, 101 },{ -1, 3, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_backstab,		SLOT(809),
	SKILL_DEX|SKILL_INT,	6,
        "backstab",		"!Backstab!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"bash",			{ 4, 101, 101 },{ 2, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_bash,		SLOT(810),
	SKILL_STR|SKILL_AGI,	3,
        "bash",			"!Bash!",
	NULL,			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"berserk",		{ 10, 101, 101 },{ 3, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_berserk,		SLOT(811),	0,	4,
        "",			"You feel your pulse slow down.",
	NULL,			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"dirt kicking",		{ 10, 10, 25 },	{ 2, 2, 4 }, 
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_dirt,		SLOT(812),
	SKILL_DEX,	4,
	"kicked dirt",		"You rub the dirt out of your eyes.",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "disarm",		{ 12, 28, 40 },	{ 2, 3, 4 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_disarm,		SLOT(813),
	SKILL_DEX,	3,
        "",			"!Disarm!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },
 
    {
        "enhanced damage",	{ 15, 48, 60 },	{ 2, 4, 5 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_enhanced_damage,	SLOT(815),	0,	0,
        "",			"!Enhanced Damage!",
	NULL,			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"hand to hand",		{ 2, 1, 4 },	{ 8, 5, 6 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT(816),
	SKILL_STR|SKILL_DEX|SKILL_INT,	0,
	"",			"!Hand to Hand!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "kick",			{ 5, 9, 13 },	{ 2, 3, 5 },
        spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        &gsn_kick,		SLOT(817),
	SKILL_STR|SKILL_DEX,	2,
        "kick",			"!Kick!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "parry",		{ 4, 7, 15 },	{ 4, 4, 6 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_parry,		SLOT(818),
	SKILL_DEX,	0,
        "",			"!Parry!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
        "rescue",		{ 1, 101, 101 },	{ 1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_FIGHTING,
        &gsn_rescue,		SLOT(819),
	SKILL_DEX|SKILL_INT,	3,
        "",			"!Rescue!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"trip",			{ 21, 10, 30 },	{ 3, 2, 4 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT(820),
	SKILL_DEX,	4,
	"trip",			"!Trip!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"set snare",		{ 17, 12, 30 },	{ 3, 3, 5 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_axe,            	SLOT(824),
	SKILL_INT|SKILL_DEX,	5,
        "devious trap",		"A trap in the room falls apart.",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"dual wield",		{ 20, 30, 45 },	{ 4, 5, 5 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dual_wield,	SLOT(825),
	SKILL_STR|SKILL_AGI,	0,
        "",                     "!Dual Wield!",
	NULL,			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"ambidexterity",	{ 40, 25, 60 },	{ 5, 4, 5 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_ambidexterity,    	SLOT(826),
	SKILL_DEX,	0,
        "",                     "!Ambidexterity!",
	NULL,			FALSE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"weapon proficiency",	{ 10, 15, 20 },	{ 4, 5, 6 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
        &gsn_weapon_prof,	SLOT(827),	0,	0,
        "",			"!weapon proficiency!",
	NULL,			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"martial arts",		{ 15, 25, 40 },	{ 6, 5, 7 },
        spell_null,             TAR_IGNORE,		POS_FIGHTING,
        &gsn_martial_arts,    	SLOT(828),
	SKILL_STR|SKILL_DEX,	0,
        "",                     "!martial arts!",
	"hand to hand",		TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"shield bash",		{ 10, 101, 101 },{ 4, -1, -1 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
        &gsn_shield_bash,	SLOT(829),
	STAT_STR,	0,
        "shield bash",		"!shield bash!",
	"bash",			TRUE,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/* One power skills */

    {
	"channel earth",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_earth,		SLOT(850), 	0,	0,
	"",			"!earth!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"channel air",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_air,		SLOT(851), 	0,	0,
	"",			"!air!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"channel fire",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_fire,		SLOT(852), 	0,	0,
	"",			"!fire!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"channel water",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_water,		SLOT(853), 	0,	0,
	"",			"!water!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"channel spirit",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_spirit,		SLOT(854), 	0,	0,
	"",			"!spirit!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"tie weave",		{ 1, 1, 1 },	{ -4, -4, -4 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_tie_weave,		SLOT(855),	0,	4,
	"",			"!Tie Weave!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"invert weave",		{ 1, 1, 1 },	{ -4, -4, -4 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_invert_weave,	SLOT(856),	0,	3,
	"",			"!Invert Weave!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"grasping",		{ 1, 1, 1 },	{ -2, -2, -2 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_grasping,		SLOT(857),	0,	3,
	"",			"!Grasping!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"linking",		{ 1, 1, 1 },	{ -5, -5, -5 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_linking,		SLOT(858),	0,	3,
	"",			"!Linking!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },


/* non-combat skills */

    { 
	"fast healing",		{ 17, 28, 31 },	{ 4, 4, 5 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT(860),
	SKILL_CON,	0,
	"",			"!Fast Healing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"haggle",		{ 30, 12, 18 },	{ 6, 3, 4 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT(861),
	SKILL_INT|SKILL_CHR,	0,
	"",			"!Haggle!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"hide",			{ 15, 8, 23 },	{ 3, 2, 4 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT(862),
	SKILL_INT|SKILL_AGI,	2,
	"",			"!Hide!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"lore",			{ 101, 101, 25 },{ -1, -1, 4 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_lore,		SLOT(863),
	SKILL_INT|SKILL_WIS,	6,
	"",			"!Lore!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"concentration",	{ 101, 101, 16 },{ -1, -1, 3 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_concentration,	SLOT(864),
	SKILL_INT,	0,
	"",			"!Concentration!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"peek",			{ 15, 6, 17 },	{ 5, 3, 4 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT(865),
	SKILL_INT|SKILL_DEX,	 0,
	"",			"!Peek!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"pick lock",		{ 20, 15, 17 },	{ 6, 3, 4 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT(866),
	SKILL_DEX|SKILL_AGI,	2,
	"",			"!Pick!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"sneak",		{ 10, 7, 12 },	{ 4, 2, 5 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT(867),
	STAT_DEX|SKILL_WIS,	2,
	"",			"You no longer feel stealthy.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"steal",		{ 16, 11, 22 },	{ 5, 3, 5 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT(868),
	SKILL_DEX|SKILL_AGI,	4,
	"",			"!Steal!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"gambling",		{ 1, 1, 1 },	{ 5, 3, 4 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_gambling,		SLOT(869),
	SKILL_INT|SKILL_CHR,	0,
        "",                     "!Gambling!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"brewing",		{ 101, 101, 15 },{ -1, -1, 3 },
        spell_null,		TAR_IGNORE,             POS_STANDING,
        &gsn_brew,		SLOT(870),
	SKILL_INT|SKILL_WIS,	0,
        "",                     "!Brewing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"appraising",		{ 10, 7, 10 },	{ 3, 2, 3 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        NULL,            	SLOT(871),
	SKILL_INT,	0,
        "",                     "!Appraising!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"tracking",		{ 17, 25, 40 },	{ 3, 3, 5 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_hunt,            	SLOT(872),
	SKILL_WIS,	2,
        "",                     "!Tracking!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"herbalism",		{ 101, 101, 10 },{ -1, -1, 3 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_herbalism,		SLOT(873),
	SKILL_INT,	0,
        "",                     "!Herbalism!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"swimming",		{ 1, 1, 1 },	{ 3, 3, 3 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_swimming,		SLOT(874),
	SKILL_STR|SKILL_CON,      0,
        "",                     "!Swimming!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"fishing",		{ 1, 1, 1 },	{ 2, 2, 2 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        NULL,            	SLOT(875),       0,      0,
        "",                     "!Fishing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"disguise",		{ 101, 25, 101 },{ -1, 5, -1 },
        spell_null,             TAR_IGNORE,	POS_STANDING,
        &gsn_disguise,         	SLOT(876),
	SKILL_INT|SKILL_CHR,	0,
        "",                     "!Disguise!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"riding",		{ 5, 7, 7 },	{ 3, 4, 4 },
        spell_null,             TAR_IGNORE,	POS_STANDING,
        &gsn_riding,            SLOT(877),	0,	0,
        "",                     "!Riding!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"seamanship",		{ 10, 10, 10 },	{ 5, 5, 5 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        NULL,            	SLOT(878),       0,      0,
        "",                     "!Seamanship!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"teaching",		{ 101, 101, 30 },	{ -1, -1, 6 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_teaching,		SLOT(879),
	SKILL_INT,      0,
        "",                     "!Teaching!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"medicine",		{ 30, 40, 15 },		{ 4, 5, 3 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_medicine,		SLOT(880),	0,	0,
        "",			"!Medicine!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"forestry",		{ 10, 10, 10 },	{ 3, 4, 4 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        NULL,            	SLOT(881),
	SKILL_WIS,	0,
        "",                     "!Forestry!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"stalking",		{ 101, 45, 101 },{ -1, 4, -1 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_stalk,            	SLOT(882),
	SKILL_DEX,	0,
        "",                     "!Stalking!",
	"sneak",		0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"envenom",		{ 101, 17, 101 },{ -1, 6, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_envenom,		SLOT(883),	0,	0,
	"",			"!envenom!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"pilfer",		{ 101, 12, 101 },{ -1, 5, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_pilfer,		SLOT(884),
	SKILL_DEX,	0,
	"",			"!pilfer!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"old tongue",		{ 20, 20, 20 },	{ 6, 6, 4 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_old_tongue,	SLOT(885),
	SKILL_INT,	0,
	"",			"!old tongue!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"acrobatics",		{ 20, 15, 30 },	{ 3, 2, 4 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_acrobatics,	SLOT(886),
	SKILL_AGI,	0,
	"",			"!acrobatics!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"foraging",		{ 10, 12, 15 },	{ 3, 3, 3 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_foraging,		SLOT(887),
	SKILL_WIS,	3,
	"",			"!foraging!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/* Career Skills */

    {
	"mining",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_mining,		SLOT(888),	0,      3,
        "",			"!Mining!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"carpentry",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_carpentry,		SLOT(889),	0,      3,
        "",			"!Carpentry!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"gemworking",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_gemworking,	SLOT(890),	0,      3,
        "",			"!gemworking!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"sewing",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_sewing,		SLOT(891),	0,      3,
        "",			"!Sewing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"leatherworking",	{ 1, 1, 1 },	{ 5, 5, 5 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_leatherworking,	SLOT(892),	0,      3,
	"",			"!Leatherworking!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"smithing",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_smithing,		SLOT(893),	0,	3,
        "",                     "!Smithing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"repairing",		{ 1, 1, 1 },	{ 5, 5, 5 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_repairing,		SLOT(894),	0,	2,
        "",			"!Repairing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/*
 *   Wolfkin skill
 */

    {
	"summon wolf",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_summon_wolf,	SLOT(895),	0,	4,
	"",			"!summon wolf!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/*
 *   Blademaster skills
 */

    {
	"feint",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_feint,		SLOT(920),
	SKILL_DEX,	0,
	"",			"!feint!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"forms",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_forms,		SLOT(921),	0,	0,
	"",			"!forms!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"riposte",		{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_riposte,		SLOT(922),
	SKILL_DEX,	0,
	"",			"!riposte!",
	"parry",		0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"switch opponent",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_switch_opponent,	SLOT(923),	0,	0,
	"",			"!switch opponents!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"heroic rescue",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_FIGHTING,
	&gsn_heroic_rescue,	SLOT(924),	0,	0,
	"",			"!heroic rescue!",
	"rescue",		0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"blindfighting",	{ 1, 1, 1 },	{ -1, -1, -1 },
	spell_null,		TAR_IGNORE,	POS_STANDING,
	&gsn_blindfighting,	SLOT(925),
	SKILL_INT,	0,
	"",			"!blindfighting!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/* Guild Skills */

    {
	"detect shadowspawn",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,             TAR_IGNORE,	POS_STANDING,
	&gsn_detect_shadowspawn,SLOT(950),
	SKILL_NONE,	0,
        "",                     "!detect shadowspawn!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"detect channeler",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,             TAR_IGNORE,            POS_STANDING,
        &gsn_detect_channeler,  SLOT(951),       0,      0,
        "",                     "!detect channeler!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"teach channeling",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_teach_channeling,	SLOT(952),	SKILL_INT,	0,
        "",			"!teach channeling!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"flash strike",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_flash_strike,	SLOT(953),
	SKILL_AGI,	0,
        "",			"!flash strike!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"sweep strike",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_sweep,		SLOT(954),
	SKILL_DEX,	0,
        "",			"!sweep strike!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"endurance",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_endurance,		SLOT(955),
	SKILL_CON,	0,
        "",			"!endurance!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/* Ajah Skills */

    /* Brown */
    {
	"identify fauna",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_identify_fauna,	SLOT(956),
	SKILL_INT,	4,
        "",			"!identify fauna!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"complaisance",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_complaisance,	SLOT(957),
	SKILL_CHR,	3,
        "",			"!complaisance!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"geography",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_geography,		SLOT(958),
	SKILL_INT,	3,
        "",			"!geography!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/*
 *  Seanchan skills
 */
    {
	"leashing",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_leashing,		SLOT(959),
	SKILL_AGI,	3,
        "",			"Your leash is removed.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

/*
 *  Aiel skills
 */
    {
	"hardiness",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_hardiness,		SLOT(960),
	SKILL_CON,	0,
        "",			"!hardiness!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"spear dancing",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_spear_dancing,	SLOT(961),
	SKILL_DEX,	0,
        "",			"!spear dancing!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },
/*
 *  Whitecloak skills
 */
    {
	"capture",		{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_capture,		SLOT(970),
	SKILL_DEX,	3,
        "",			"Your ropes come undone.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"group fighting",	{ 1, 1, 1 },	{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        &gsn_group_fighting,	SLOT(970),	0,	3,
        "",			"",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },


/*
 *  Non-spell affects
 */
    {
	"stop channeling",	{ 199, 199, 199 },{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        NULL,			SLOT(1200),	0,	3,
        "",			"!stop channeling!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"electrical shock",	{ 199, 199, 199 },{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        NULL,			SLOT(500),	0,	3,
        "surge of the One Power","!electrical shock!",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    },

    {
	"set limb",		{ 199, 199, 199 },{ -1, -1, -1 },
        spell_null,		TAR_IGNORE,	POS_STANDING,
        NULL,			SLOT(501),	0,	3,
        "",			"Your limb feels straightened.",
	NULL,			0,
	FALSE,			FALSE,
	{ 0, 0, 0, 0, 0 }
    }

};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"warrior-type",		{ 0, -1, -1 },
	{
	    "shield block", "berserk", "trip", "hand to hand",
	    "parry", "disarm", "enhanced damage", "kick"
	}
    },

    {
        "rogue-type",		{ -1, 0, -1 },
        {
	    "dagger", "backstab", "peek", "pick lock"
	    "dodge", "hide", "sneak", "steal"
	}
    },

    {
        "scholar-type",		{ -1, -1, 0 },
        {
	    "lore", "concentration", "haggle", "writing",
	    "teaching", "appraising", "herbalism", "gambling"
	}
    },

    {
        "wisdom-type",		{ -1, -1, 0 },
        {
	    "lore", "concentration", "writing", "foraging",
	    "brewing", "herbalism", "medicine", "forestry"
	}
    },

    {
        "hunter-type",		{ 0, -1, -1 },
        {
	    "shield block", "hand to hand", "tracking",
	    "parry", "dirt kick", "kick", "enhanced damage", "forestry"
	}
    },

    {
        "merchant-type",	{ -1, 0, -1},
        {
	    "haggle", "gambling", "steal", "staves", "wands",
	    "appraising", "pilfer", "dodge"
	}
    },

    {
	"blademaster",		{ -1, -1, -1 },
	{
	    "feint", "riposte", "switch opponent", "heroic rescue",
	    "blindfighting", "forms"
	}
    },

    {
	"channeling",		{ -1, -1, -1 },
	{
	    "tie weave", "grasping", "linking"
	}
    },

    {
	"general weaves",	{ -1, -1, -1 },
	{
	    "shield from true source", "sever from true source",
	    "cut weave", "wrap", "gag", "channeling"
	}
    },

    {
	"charm/illusion",	{ -6, -6, -6 },
	{
	    "reveal invisible", "sleep", "illusion", "ventriloquate",
	    "charm", "shape change", "mass invisibility",
	    "invisibility", "compulsion", "light storm"
	}
    },

    {
	"combat",		{ -6, -6, -6 },
	{
	    "flame dart", "flame wave", "earth wave", "stone strike",
	    "wind mace", "flame column", "fireball", "ice ball",
	    "wind spear", "rock shower", "steam", "lash", "air blade"
	}
    },

    {
	"creation",		{ -5, -5, -5 },
	{
	    "create air sword", "create flame sword", "create spring",
	    "create water", "light ball", "fire wall"
	}
    },

    {
	"detection",		{ -2, -2, -2 },
 	{
	    "awareness", "detect invisible", "delve"
	}
    },

    {
	"enchantment",		{ -4, -4, -4 },
	{
	    "ward object", "strengthen", "haste", "enhance weapon",
	    "enhance armor"
	}
    },

    {   
	"healing",		{ -3, -3, -3 },
 	{
	    "minor healing", "healing", "major healing", "refreshment",
	    "cure blindness", "cure poison", "cure disease",
	    "bone knitting", "clot", "probe"
	}
    },

    {
	"maladictions",		{ -3, -3, -3 },
	{
	    "blindness", "poison", "disease", "slow", "weaken",
	    "cause pain", "confusion", "net of pain"
	}
    },

    { 
	"protective",		{ -2, -2, -2 },
	{
	    "ward room", "ward person", "air shield", "air armor",
	    "earth shield", "earth barrier"
	}
    },

    {
	"transportation",	{ -8, -8, -8 },
	{
	    "skim", "travel"
	}
    },
   
    {
	"weather",		{ -8, -8, -8 },
	{
	    "windstorm", "lightning bolt", "earthquake", "wind push",
	    "control weather", "hail storm", "lightning storm",
	    "frost wave", "wind barrier", "ice wall"
	}
    },

    {
	"legendary",		{ -1, -1, -1 },
	{
	    "invisibility", "shape change", "mass invisibility",
	    "travel", "skim", "compulsion"
	}
    },
    {
	"unteachable",		{ -1, -1, -1 },
	{
	    "balefire", "flame burst", "air hammer", "frostbite",
	    "earth rending", "summon wolf", "detect shadowspawn",
	    "detect channeler", "teach channeling", "flash strike",
	    "sweep strike", "endurance", "identify fauna", "complaisance",
	    "geography", "leashing", "hardiness", "spear dancing",
	    "capture", "group fighting", "stop channeling", "electrical shock",
	    "set limb", "detect ability"
	}
    }
};
