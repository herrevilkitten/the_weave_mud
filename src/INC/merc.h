/*************************************************************************
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



/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_USE_FUN( fun )		bool fun( )
#define DECLARE_SPEC_OBJ( fun )		bool fun( )
#define DECLARE_SPEC_ROOM( fun )	bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_USE_FUN( fun )		USE_FUN  fun
#define DECLARE_SPEC_OBJ( fun )		SPEC_OBJ  fun
#define DECLARE_SPEC_ROOM( fun )	SPEC_ROOM  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif

/* system calls */
int unlink();
int system();




/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif



/*
 * Structure types.
 */
typedef struct  mem_data                MEM_DATA;
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	buf_type		BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	node_data		NODE_DATA;
typedef struct	text_data		TEXT_DATA;
typedef struct	guard_data		GUARD_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	guild_data		GUILD_DATA;
typedef struct	guild			GUILD;
typedef struct	guild_skill_type	GUILD_SKILL_TYPE;


/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void USE_FUN	args( ( OBJ_DATA *obj, CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_OBJ	args( ( OBJ_DATA *obj ) );
typedef bool SPEC_ROOM	args( ( ROOM_INDEX_DATA *room ) );
typedef void SPELL_FUN	args( ( int sn, int multiplier, CHAR_DATA *ch, void *vo, int target_type ) );


/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4096
#define MAX_INPUT_LENGTH	  256
#define PAGELEN			   22



/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_ALIAS		   25
#define MAX_SOCIALS		  256
#define MAX_SPELL		  100
#define MAX_SKILL		  200
#define MAX_TALENT		   41
#define MAX_GROUP		   30
#define MAX_IN_GROUP		   30
#define MAX_BOND		    4
#define MAX_CLASS		    3 /* Warrior-type, Rogue-type,
					 Scholar-type */
#define WARRIOR_TYPE		    0
#define ROGUE_TYPE		    1
#define SCHOLAR_TYPE		    2

#define OBJ_VERSION			1

#define	MAX_HERB			200
#define	MAX_ORE				50
#define	MAX_LUMBER			150
#define	MAX_GEM				25
#define	MAX_FISH			100

#define MAX_LEVEL		   110
#define LEVEL_HERO		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 8)
#define MAX_COLORS                 13
#define MAX_DIR 6
#define NO_FLAG -99     /* Must not be used in flags or stats. */

#define PULSE_PER_SECOND	    4
/*
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
*/
#define PULSE_MIN		  (6   * PULSE_PER_SECOND)
#define PULSE_OBJ		  (60  * PULSE_PER_SECOND)
#define PULSE_HOUR		  (120 * PULSE_PER_SECOND)
#define PULSE_AREA		  (120 * PULSE_PER_SECOND)
#define PULSE_SECOND		  (1   * PULSE_PER_SECOND)
#define PULSE_WEATHER		  (120 * PULSE_PER_SECOND)
#define PULSE_TIMER		  (60  * PULSE_PER_SECOND)

#define IMPLEMENTOR		MAX_LEVEL
#define	CREATOR			(MAX_LEVEL - 1)
#define SUPREME			(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define ANGEL			(MAX_LEVEL - 7)
#define AVATAR			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO

/* for command types */
#define ML      MAX_LEVEL       /* implementor */
#define L1      MAX_LEVEL - 1   /* creator */
#define L2      MAX_LEVEL - 2   /* supreme being */
#define L3      MAX_LEVEL - 3   /* deity */
#define L4      MAX_LEVEL - 4   /* god */
#define L5      MAX_LEVEL - 5   /* immortal */
#define L6      MAX_LEVEL - 6   /* demigod */
#define L7      MAX_LEVEL - 7   /* angel */
#define L8      MAX_LEVEL - 8   /* avatar */
#define IM      LEVEL_IMMORTAL  /* angel */
#define HE      LEVEL_HERO      /* hero */


#define	NODE_EMPTY		-1
#define	NODE_PC			0
#define	NODE_FIGHT		1
#define	NODE_WEAVE_CHAR		2
#define	NODE_WEAVE_OBJ		3
#define	NODE_WEAVE_ROOM		4
#define NODE_WEAVE_CREATE	5
#define	NODE_SPEC_CHAR		7
#define	NODE_SPEC_OBJ		8
#define	NODE_SPEC_ROOM		9
#define NODE_WAIT		10
#define NODE_ROOM		11


/*
 * Node data.  For lists
 */
struct	node_data
{
    NODE_DATA	*	next;
    sh_int		data_type;
    void	*	data;
};

/*
 * Text data for 
 */
struct	text_data
{
    TEXT_DATA	*	next;
    char	*	keyword;
    char	*	text;
    int			id;
    int			timeout;
};

#define GUARD_NONE	0
#define	GUARD_NORTH	A
#define	GUARD_EAST	B
#define	GUARD_SOUTH	C
#define	GUARD_WEST	D
#define	GUARD_UP	E
#define	GUARD_DOWN	F

#define	GUARD_GUILD	A
#define	GUARD_RACE	B
#define	GUARD_CLASS	C
#define	GUARD_VNUM	D
#define GUARD_IMMORTAL	E
#define GUARD_NEWBIE	F

#define GUARD_DENY	A
#define GUARD_ATTACK	B
#define GUARD_MOVE	C
#define GUARD_MESSAGE	D
#define GUARD_ENTRANCE	E
#define GUARD_EXIT	F

struct	guard_data
{
    GUARD_DATA *	next;
    int			exit;
    int			allow;
    int			flag;
    int			message;
    int			room;
};

/*
 * Site ban structure.
 */
#define BAN_SUFFIX              A
#define BAN_PREFIX              B
#define BAN_NEWBIES             C
#define BAN_ALL                 D
#define BAN_PERMIT              E
#define BAN_PERMANENT           F
#define BAN_USER		G

struct  ban_data
{
    BAN_DATA *  next;
    sh_int      ban_flags;
    sh_int      level;
    char *      user;
    char *      host;
};



struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */ 
    sh_int      size;   /* size in k */
    char *      string; /* buffer's string */
};




/*
 * Time and weather stuff.
 */
#define SUN_DARK			0
#define SUN_RISE			1
#define SUN_LIGHT			2
#define SUN_SET				3

#define DAYS_PER_MONTH			28
#define MONTHS_PER_YEAR			13

#define	MOON_PERIOD			30

#define MOON_NEW			0
#define MOON_CRESCENT_WAXING		1
#define MOON_FIRST_QUARTER		2
#define MOON_GIBBUS_WAXING		3
#define MOON_FULL			4
#define MOON_GIBBUS_WANING		5
#define MOON_LAST_QUARTER		6
#define MOON_CRESCENT_WANING		7

#define SKY_CLOUDLESS			0
#define SKY_CLOUDY			1
#define SKY_RAINING			2
#define SKY_SNOWING			3
#define SKY_HAILING			4
#define SKY_LIGHTNING			5

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
    int		moonlight;
};



/*
 * Connected state for a channel.
 */
#define CON_IDENT_WAIT			 -1
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define	CON_GET_LAST_NAME		 6
#define CON_CONFIRM_LAST_NAME		 7
#define CON_GET_NEW_SEX			 8
#define CON_WEAPON_CHOICE		 9
#define CON_ADD_SKILLS			10
#define CON_GET_QUESTION		11
#define CON_CONFIRM_STATS		12
#define CON_PLACE_POINTS		13
#define CON_GEN_GROUPS                  14
#define CON_READ_IMOTD			15
#define CON_READ_MOTD			16
#define CON_BREAK_CONNECT		17
#define	CON_EMAIL			18
#define CON_GET_AGE			19
#define CON_GET_PASSWORD_2		20
#define CON_GET_RACE			21
#define CON_MAKE_CHOICE			22
#define CON_NAME_IS_FANTASY		23
#define	CON_MAKE_DESCRIPTION		24
#define	CON_DONE_DESCRIPTION		25

#define CON_TO_LIST			40
#define CON_SUBJECT			41


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    sh_int		descriptor;
    sh_int		connected;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    char *		showstr_head;
    char *		showstr_point;
    void *              pEdit;          /* OLC */
    char **             pString;        /* OLC */
    int                 editor;         /* OLC */
    int			ifd;
    pid_t		ipid;
    char *		ident;
    int			port;
    int			ip;
    char                delayed[MAX_INPUT_LENGTH];
    int			idle;
    bool		got_ident;
};

/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	dam_bonus;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	off_bonus;
    sh_int	def_bonus;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct	cha_app_type
{
    sh_int	price_mod;
};

struct	luk_app_type
{
    sh_int	roll_mod;
    sh_int	percent_mod;
};

struct	agi_app_type
{
    sh_int	aff;
};


/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4
#define TO_GROUP	    5
#define	TO_CHARVICT	    6

/*
 * TO types for broadcast
 */
#define	BC_ROOM			0
#define	BC_CHAR			1
#define	BC_ALL			2

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    sh_int	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */

#define MAX_RANK	16
#define MAX_SUB		4
#define MAX_STATS 	8

#define RACE_ANDOR	1
#define RACE_AIEL	2
#define RACE_BORDER	3
#define RACE_OGIER	4
#define RACE_SEANCHAN	5


#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4
#define STAT_CHR	5
#define STAT_LUK	6
#define STAT_AGI	7

#define SKILL_NONE	0
#define SKILL_STR	A
#define SKILL_INT	B
#define SKILL_WIS	C
#define SKILL_DEX	D
#define SKILL_CON	E
#define SKILL_CHR	F
#define SKILL_LUK	G
#define SKILL_AGI	H

struct	class_type
{
    char *	name;			/* the full name of the class */
    sh_int	off_bonus;
    sh_int	def_bonus;
    sh_int	hp;			/* Average hp gained on leveling */
    sh_int	stam;
};


struct weapons_type
{
    sh_int	type;
    sh_int	diff;
    sh_int	fumble;
    sh_int	critical;
    sh_int	range;
    sh_int	dice;
    sh_int	max_tohit;
    sh_int	max_dam;
    sh_int	speed;
};


struct wiznet_type
{
    char *      name;
    long        flag;
    int         level;
};


struct attack_type
{
    char *	name;			/* name and message */
    int   	damage;			/* damage class */
};

struct race_type
{
    char *	name;			/* call name of the race */
    bool        pc_race;                /* can be chosen by pcs */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long        res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    sh_int	dam_dice;		/* number of dice for damage */
    sh_int	dam_side;		/* number of sides for damage */
    sh_int	dam_bonus;		/* bonus for damage */
    sh_int	hit_mult;		/* hit multiplier */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
    int		size;
};

struct pc_race_type  /* additional data for pc races */
{
    char *      name;                   /* MUST be in race_type */
    sh_int      points;                 /* cost in points of the race */
    sh_int      class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */
    char *      skills[5];              /* bonus skills for the race */
    sh_int      stats[MAX_STATS];       /* stat modifier */
    sh_int      size;                   /* aff bits for the race */
    sh_int	max_age;		/* maximum age for race */
};




struct dice_type
{
    sh_int HitNum;
    sh_int HitSides;
    sh_int HitPlus;

    sh_int DamNum;
    sh_int DamSides;
    sh_int DamPlus;
};

/*
 * Data structure for notes.
 */
#define NOTE_NOTE       0
#define NOTE_IDEA       1
#define NOTE_PENALTY    2
#define NOTE_NEWS       3
#define NOTE_CHANGES    4
#define NOTE_APPLY	5

struct	note_data
{
    NOTE_DATA *	next;
    sh_int	type;
    char *	sender;
    char *	real_name;
    int		guild;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
};


/*
 * Affect flags
 */

#define	AFFECT_MALE		(A)
#define	AFFECT_FEMALE		(B)
#define	AFFECT_NOTCHANNEL	(C)
#define	AFFECT_INVERT		(D)
#define	AFFECT_TIED		(E)
#define AFFECT_LINKING		(F)

/*
 * Affect macros
 */
#define IS_LINKING(ch)		( IS_AFFECTED_2((ch),AFF_LINK) )
#define IS_GRASPING(ch)		( IS_SET((ch)->affected_by_2, AFF_GRASP) ? 1 : \
				  0 )
#define IS_INVERTED(af)		( IS_SET((af)->flags, AFFECT_INVERT) ? 1 : 0 )
#define IS_TIED(af)		( (af)->owner == NULL ? 1 : \
				  IS_SET((af)->flags, AFFECT_NOTCHANNEL) ? 1 : \
				  IS_SET((af)->flags, AFFECT_TIED) ? 1 : 0 )
#define SAME_SEX(af, ch)	( IS_SET((af)->flags, AFFECT_MALE) && \
				  TRUE_SEX(ch) == SEX_MALE ) ? 1 : \
				( IS_SET((af)->flags, AFFECT_FEMALE) && \
				  TRUE_SEX(ch) == SEX_FEMALE ) ? 1 : 0
#define	SET_SEX(af, ch)		( TRUE_SEX(ch) == SEX_MALE ? \
				  SET_BIT(af.flags, AFFECT_MALE) : \
				  SET_BIT(af.flags, AFFECT_FEMALE) )
#define AFF_OWNER(ch)		( IS_NPC(ch) ? NULL : (ch) )
#define SOURCE(ch)		( TRUE_SEX(ch) == SEX_MALE ? "saidin" : \
				  "saidar" )
#define SEVER(ch)		( TRUE_SEX(ch) == SEX_MALE ? "gentled" : \
				  "stilled" )

/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    sh_int		type;
    sh_int		strength;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
    int			bitvector_2;
    CHAR_DATA *		owner;		/* Caster of the spell - cackle */
    int			flags;		/* Affect flags */
};

/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    sh_int		number;
    sh_int		killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/


/*
 * Ansi colors and VT100 codes
 * Used in #PLAYER
 *
 * On most machines that use ANSI, namely the IBM PCs, the decimal value for
 * the escape char
acter is 27 (1B hex).     Change this value when needed.
 */

#define BLACK      "\x1b[30m"    /* These are foreground color codes */
#define RED        "\x1b[31m"
#define GREEN      "\x1b[32m"
#define YELLOW     "\x1b[33m"
#define BLUE       "\x1b[34m"
#define PURPLE     "\x1b[35m"
#define CYAN       "\x1b[36m"
#define GREY       "\x1b[37m"

#define B_BLACK    "\x1b[40m"    /* These are background color codes */
#define B_RED      "\x1b[41m" 
#define B_GREEN    "\x1b[42m"
#define B_YELLOW   "\x1b[43m"
#define B_BLUE     "\x1b[44m"
#define B_PURPLE   "\x1b[45m"
#define B_CYAN     "\x1b[46m"
#define B_GREY     "\x1b[47m"

/* Below are VT100 and ANSI codes (above are ANSI exclusively)       */

#define EEEE       "\x1b#8"                 /* Turns screen to EEEEs */
#define CLRSCR     "\x1b[2j"                /* Clear screen          */
#define CLREOL     "\x1b[K"                 /* Clear to end of line  */

#define UPARR      "\x1b[A"                 /* Up one line           */
#define DOWNARR    "\x1b[B"                 /* Down one line         */
#define RIGHTARR   "\x1b[C"                 /* Right one column      */
#define LEFTARR    "\x1b[D"                 /* Left one column       */
#define HOMEPOS    "\x1b[H"                 /* Home (upper left)     */

#define BOLD       "\x1b[1m"                /* High intensity        */
#define FLASH      "\x1b[5m"                /* Flashing text         */
#define INVERSE    "\x1b[7m"                /* XORed back and fore   */
#define NTEXT      "\x1b[0m"                /* Normal text  (grey)   */

/*
 * Color codes
 */
#define ANSI_NORMAL                     "^[[m"
#define ANSI_BOLD                       "^[[1m"
#define ANSI_BLINK                      "^[[5m"
#define ANSI_REVERSE                    "^[[7m"



/* 
 * Other codes of note for future ANSI development:
 */

/*
 * Color codings for
 * channels+says
 */
#define COLOR_AUCTION			0
#define COLOR_OOC			1
#define COLOR_IMMTALK			2
#define COLOR_BARD			3
#define COLOR_TELL			4
#define COLOR_SAY			5
#define COLOR_GUILD			6
#define COLOR_SPECIAL			7

#define COLOR_NAME			8
#define COLOR_DESC			9
#define COLOR_CHAR			10
#define COLOR_OBJ			11
#define COLOR_EXIT			12



/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO		   3090
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   3404

/* RT ASCII conversions -- used so we can have letters in this file */

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824

/* Powers - for finding strength, mainly */
#define	POWER_EARTH		(A)
#define	POWER_AIR		(B)
#define	POWER_FIRE		(C)
#define	POWER_WATER		(D)
#define	POWER_SPIRIT		(E)
#define	POWER_ALL		(A)|(B)|(C)|(D)|(E)

/* Hide type */
#define HIDE_NONE		0
#define HIDE_OBJECT		1
#define HIDE_CHAR		2

/*
 * Config bits for player config stuff
 */
#define	CONFIG_COMBHELP		(A)
#define	CONFIG_PEEK		(B)

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define	ACT_GUARDIAN		(C)		
#define ACT_TALKER		(D)		
#define ACT_NOEXP		(E)
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_MOUNT		(L)		/* Can be mounted */
#define ACT_TRACKER		(M)		/* Can track */
#define ACT_REMEMBER		(N)		/* Remembers last attacker */
#define ACT_STAY_OUTSIDE	(O)
#define ACT_STAY_INSIDE		(P)
#define ACT_CHANNELER		(Q)
#define ACT_SCHOLAR		(R)
#define ACT_ROGUE		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_BANKER		(X)
#define ACT_FIXER		(Y)

#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define	ACT_FORGE_WEAPON	(dd)
#define	ACT_FORGE_ARMOR		(ee)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define	DAM_SLAY		18
#define	DAM_BLEEDING		19

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH		(O)
#define ASSIST_ALL       	(P)

#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)
#define ASSIST_GUILD		(V)

#define OFF_POISON		(W)
#define OFF_TRACK		(X)
#define OFF_MEMORY		(Y)
#define OFF_ACROBATIC		(Z)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT		(S)
 
/* RES bits for mobs */
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT		(S)
 
/* VULN bits for mobs */
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)
 
#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)	
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_WRAP		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_WARDED		(E)
#define AFF_AWARENESS		(F)
#define AFF_GAG			(G)
#define AFF_AIR_ARMOR		(H)

#define AFF_POISON		(M)
#define AFF_SHIELDED		(N)		/* Shielded from True Source */
#define AFF_STALK		(O)		/* For stalking, so you don't
						   Get grouped :)	*/
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SLOW		(bb)
#define AFF_REGENERATION        (cc)
#define AFF_SHADOWCLOAK		(dd)
#define AFF_SHAPE_CHANGE	(ee)

/* Stuff for affected_by_2 */
#define	AFF_STILL		(A)
#define AFF_LINK		(B)
#define AFF_NOAGGRO		(C)
#define AFF_GRASP		(D)
#define AFF_CAN_LINK		(E)
#define AFF_LEASHED		(F)
#define AFF_HIDE_CHANNEL	(G)
#define AFF_CAPTURED		(H)
#define AFF_STOP_CHANNEL	(I)
#define AFF_CONFUSED		(J)
#define AFF_PAIN		(K)
#define AFF_INCOGNITO		(L)

/*
 *   Body condition flags
 */

#define	BODY_RIGHT_LEG		(A)
#define	BODY_LEFT_LEG		(B)
#define	BODY_RIGHT_ARM		(C)
#define	BODY_LEFT_ARM		(D)
#define	BODY_BLEEDING		(E)
#define BODY_BLIND		(F)
#define BODY_DISEASE		(G)
#define BODY_POISON		(H)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5

/* material types */
#define MAT_UNKNOWN			0
#define MAT_WOOD			1
#define MAT_BONE			2
#define MAT_IRON			3
#define MAT_STEEL			4
#define MAT_SILVER			5
#define MAT_GOLD			6
#define MAT_DARKSILVER			7
#define MAT_HEARTSTONE			8
#define MAT_CLOTH			9
#define MAT_LEATHER			10
#define MAT_SILK			11
#define MAT_STONE			12
#define MAT_OBSIDIAN			13
#define MAT_GEM_OTHER			14
#define MAT_DIAMOND			15
#define MAT_RUBY			16
#define MAT_SAPPHIRE			17
#define MAT_EMERALD			18
#define MAT_OPAL			19
#define MAT_PAPER			20
#define MAT_GLASS			21
#define MAT_COPPER			22
#define MAT_SOFT_LEATHER		23
#define MAT_CLAY			24
#define MAT_SPECIAL			25
#define MAT_SATIN			26
#define MAT_VELVET			27
#define	MAT_BAMBOO			28
#define	MAT_ELECTRUM			29
#define	MAT_SILVERY_STUFF		30

/* Resources */
#define RES_NONE			0
#define	RES_LUMBER			A
#define RES_GEMS			B
#define RES_ORE				C
#define RES_FISH			D
#define RES_HERB			E
#define RES_WATER			F

/* Ingredient types */
#define	HERB_NONE			0
#define	HERB_ALDAKA			A
#define	HERB_CUREALL			B
#define	HERB_BELRAMBA			C
#define	HERB_CATS_TAIL			D
#define	HERB_DAGMATHER			E
#define	HERB_FETHERFEW			F
#define	HERB_GOATS_RUE			G
#define	HERB_HARES_EARS			H
#define	HERB_HOREHOUND			I
#define	HERB_MASTERWORT			J
#define	HERB_PARGEN			K
#define	HERB_SWEET_TREFOILE		L
#define	HERB_ORACH			M
#define	HERB_MUGWORT			N
#define HERB_WILLOW_HERB		O

#define	HERB_HEAL			A
#define	HERB_CURE_POISON		B
#define HERB_CURE_DISEASE		C
#define HERB_REFRESH			D
#define	HERB_SLEEP			E
#define	HERB_SPECIAL			F

#define	HERB_HURT			H
#define	HERB_POISON			I
#define HERB_DISEASE			J
#define	HERB_DRAIN			K
#define HERB_STOP_CHANNEL		L

#define POTION_DRINK			0
#define POTION_BALM			1
#define POTION_EAT			2

#define	GUILD_NONE			0
#define	GUILD_SUBRANK_2			A
#define	GUILD_SUBRANK_3			B


struct guild_skill_type
{   
    char        *name;
    int         rank;
    sh_int	sn;
};

struct guild_data
{
    char        *name;
    char        *savename;
    char        *imms;

    GUILD_SKILL_TYPE    skill[4];
    int                 equip[MAX_RANK];
    char                *rank[MAX_SUB][MAX_RANK];
 
    int		flags;
    char        *desc;
    char	*members;
    GUILD_DATA  *next; 
};

struct guild
{
    char *		warder;
    char *		damane_name;
    CHAR_DATA *		damane;
    unsigned short int	rank;
    GUILD *		next;
};


/* Breakage table struct */

struct	break_type
{
    sh_int chance;
    sh_int affect;
    sh_int weight_mult;
    sh_int cost_mult;
    sh_int armor_min;
    sh_int armor_max;
    sh_int fire_mod;
    sh_int cold_mod;
    sh_int lightning_mod;
    int vnum;
    sh_int *pgsn;    
};

/* Breakage affects */

#define AFF_NOTHING			0
#define AFF_SHATTER			1
#define AFF_BEND			2
#define AFF_EXPLODE			3
#define	AFF_TEAR			4

/* Forging stuff */

struct	forge_type
{
    char *name;
    char *long_name;
    int v0;
    int v1;
    int v2;
    int v3;
    int weight;
};

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define	OBJ_VNUM_PICK			4205
#define	OBJ_VNUM_FLAMESWORD		4223
#define	OBJ_VNUM_AIRSWORD		4224

/* Ore */
#define	OBJ_VNUM_ELECTRUM		4200
#define	OBJ_VNUM_IRON			4201
#define	OBJ_VNUM_COPPER			4202
#define	OBJ_VNUM_GOLD			4203
#define	OBJ_VNUM_SILVER			4204

/* Gems */
#define	OBJ_VNUM_DIAMOND		4267
#define	OBJ_VNUM_RUBY			4268
#define	OBJ_VNUM_OTHER_GEM		4269
#define	OBJ_VNUM_SAPPHIRE		4270
#define	OBJ_VNUM_EMERALD		4271
#define	OBJ_VNUM_OPAL			4272

/* Other */
#define	OBJ_VNUM_LUMBER			4260
#define	OBJ_VNUM_CLOTH			4261
#define	OBJ_VNUM_SATIN			4262
#define	OBJ_VNUM_VELVET			4263
#define	OBJ_VNUM_SILK			4264
#define	OBJ_VNUM_SOFT_LEATHER		4265
#define	OBJ_VNUM_HARD_LEATHER		4266

/* herbs */
#define	OBJ_VNUM_ALDAKA			4273
#define	OBJ_VNUM_CUREALL		4274
#define	OBJ_VNUM_BELRAMBA		4275
#define	OBJ_VNUM_CATS_TAIL		4276
#define	OBJ_VNUM_DAGMATHER		4277
#define	OBJ_VNUM_FETHERFEW		4278
#define	OBJ_VNUM_GOATS_RUE		4279
#define	OBJ_VNUM_HARES_EARS		4280
#define	OBJ_VNUM_HOREHOUND		4281
#define	OBJ_VNUM_MASTERWORT		4282
#define	OBJ_VNUM_PARGEN			4283
#define	OBJ_VNUM_SWEET_TREFOILE		4284
#define	OBJ_VNUM_ORACH			4285
#define	OBJ_VNUM_MUGWORT		4286
#define	OBJ_VNUM_WILLOW_HERB		4287

#define	OBJ_VNUM_BANDAGE		4225
#define	OBJ_VNUM_OINTMENT		4226
#define	OBJ_VNUM_VIAL			4227
#define	OBJ_VNUM_ANTIVENOM		4229
#define	OBJ_VNUM_POISON			4296
#define	OBJ_VNUM_TREE			4289

#define OBJ_VNUM_ADAM_BRACELET		4293
#define OBJ_VNUM_ADAM_COLLAR		4294

#define OBJ_VNUM_PORTAL			4251
#define OBJ_VNUM_DISC			4252
#define OBJ_VNUM_ROSE			4200
#define OBJ_VNUM_CORPSE_NPC		10
#define OBJ_VNUM_CORPSE_PC		11
#define OBJ_VNUM_SEVERED_HEAD		12
#define OBJ_VNUM_TORN_HEART		13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS		     17

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_PIT		   509
#define ROOM_VNUM_DONATE	   674

#define OBJ_VNUM_AXE		   4209
#define OBJ_VNUM_DAGGER	   	   4210
#define OBJ_VNUM_FLAIL		   4211
#define OBJ_VNUM_MACE	   	   4212
#define OBJ_VNUM_POLEARM	   4213
#define OBJ_VNUM_SPEAR		   4214
#define OBJ_VNUM_SWORD	   	   4215
#define OBJ_VNUM_WHIP		   4216
#define OBJ_VNUM_STAFF		   4228

#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP		   4250
#define OBJ_VNUM_FORGE_ITEM	   4207
#define	ITEM_FORGE_COST		   1000

#define	FRUIT_APPLE			1
#define	FRUIT_BANANA			2
#define	FRUIT_PEAR			3
#define	FRUIT_CHERRY			4
#define	FRUIT_ORANGE			5
#define	FRUIT_PEACH			6
#define	FRUIT_PLUM			7


/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		     	 1
#define ITEM_SCROLL			 2
#define ITEM_TOOL			 3
#define ITEM_INGREDIENT			 4
#define ITEM_WEAPON		     	 5
#define ITEM_FRUIT_TREE			 6
#define ITEM_TREASURE		     	 8
#define ITEM_ARMOR		     	 9
#define ITEM_POTION		     	10
#define ITEM_CLOTHING		     	11
#define ITEM_FURNITURE		     	12
#define ITEM_TRASH		     	13
#define ITEM_CONTAINER		     	15
#define ITEM_DRINK_CON		     	17
#define ITEM_KEY		     	18
#define ITEM_FOOD		     	19
#define ITEM_MONEY		     	20
#define ITEM_BOAT		     	22
#define ITEM_CORPSE_NPC		     	23
#define ITEM_CORPSE_PC		     	24
#define ITEM_FOUNTAIN		     	25
#define	ITEM_JEWELRY		     	26
#define ITEM_PROTECT		     	27
#define ITEM_MAP		     	28
#define ITEM_FAQ		     	29
#define ITEM_ORE		     	30
#define ITEM_GATE		     	31
#define ITEM_VIAL			32
#define ITEM_ROOM_AFFECT		33

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_SMALL_SHIELD	(B)
#define ITEM_TERANGREAL		(C)
#define ITEM_LOCK		(D)
#define ITEM_NODROP		(H)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define	ITEM_BENT		(S)
#define	ITEM_RUINED		(U)
#define	ITEM_MALE_ONLY		(V)
#define	ITEM_FEMALE_ONLY	(W)
#define	ITEM_NOBREAK		(X)
#define ITEM_NOUNCURSE		(Y)
#define ITEM_QUEST		(Z)
#define ITEM_DONATE		(aa)
#define ITEM_CHANNELED		(bb)
#define ITEM_TIED		(cc)
#define ITEM_DONATE_ROT		(dd)
#define ITEM_NEW_VERSION	(ee)
/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_TWO_HANDS		(P)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7	
#define WEAPON_POLEARM		8
#define WEAPON_STAFF		9
#define WEAPON_HAND_TO_HAND	10
#define WEAPON_BOW		11
#define WEAPON_CROSSBOW		12
#define WEAPON_SLING		13
#define	WEAPON_JAVELIN		14
#define	WEAPON_WARHAMMER	15

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_SHEATHED		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_POISON		(G)
#define WEAPON_NOFUMBLE		(H)


/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_HIT		     13
#define APPLY_STAMINA		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SKILL		     25
#define APPLY_SPELL		     26
#define APPLY_CHR		     27
#define APPLY_LUK		     28
#define APPLY_SUBSKILL		     29
#define APPLY_AGI		     30



/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	   504
#define ROOM_VNUM_ALTAR		   675
#define ROOM_VNUM_SCHOOL	   3700
#define ROOM_VNUM_RECALL	   504



/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)

#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_EXPLORE		(E)
#define ROOM_FALL		(F)
#define ROOM_NOTREE		(G)

#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_QUEST_SHOP		(T)
#define ROOM_BANK		(U)
#define	ROOM_STEDDING		(Z)
#define ROOM_WARDED		(bb)
#define ROOM_SNARED		(cc)
#define ROOM_FOG		(dd)
#define ROOM_NONOISE		(ee)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR			(A)
#define EX_CLOSED			(B)
#define EX_LOCKED			(C)
#define EX_PICKPROOF			(D)
#define EX_HIDDEN			(E)
#define EX_OPENEXIT			(F)

/*
 * Gate flags.
 * Used in act_enter
 */

#define	GATE_NOUNCURSE			(A)
#define	GATE_RANDOM			(B)
#define	GATE_BUGGY			(C)
#define	GATE_NORMAL_EXIT		(D)
#define	GATE_GOWITH			(E)
#define	GATE_NOCURSE			(F)
#define	GATE_DESTROY			(G)

/* furniture flags */
#define STAND_AT                (A)
#define STAND_ON                (B)
#define STAND_IN                (C)
#define SIT_AT                  (D)
#define SIT_ON                  (E)
#define SIT_IN                  (F)
#define REST_AT                 (G)
#define REST_ON                 (H)
#define REST_IN                 (I)
#define SLEEP_AT                (J)
#define SLEEP_ON                (K)
#define SLEEP_IN                (L)
#define PUT_AT                  (M)
#define PUT_ON                  (N)
#define PUT_IN                  (O)
#define PUT_INSIDE              (P)


/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1       /* Area has been modified. */
#define         AREA_ADDED      2       /* Area has been added to. */
#define         AREA_LOADING    4       /* Used for counting in db.c */
#define         AREA_REMOVE     8       /* Used for counting in db.c */
#define		AREA_TESTING	16
#define		AREA_COMPLETE	32

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_UNDERGROUND	     11
#define SECT_SWAMP		     12
#define SECT_MAX		     13



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_SECONDARY		     18
#define MAX_WEAR		     19



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
    char *      spec_name;
    SPEC_FUN *  spec_fun;
};

struct spec_obj_type
{
    char *      spec_name;
    SPEC_OBJ *  spec_fun;
};

struct use_fun_type
{
    char *	use_name;
    USE_FUN *	use_fun;
};


/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char * name;
    int  bit;
    bool settable;
};



/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY  4298

/*
 * Interp.c
 */
DECLARE_DO_FUN( do_olc          );
DECLARE_DO_FUN( do_asave        );
DECLARE_DO_FUN( do_alist        );
DECLARE_DO_FUN( do_resets       );


/*
 * Global Constants
 */
extern  char *  const   dir_name        [];
extern  char *  const	wear_name	[MAX_WEAR];
extern  const   sh_int  rev_dir         [];          /* sh_int - ROM OLC */
extern  const   struct  spec_type	spec_table	[];
extern  const   struct  use_fun_type	use_fun_table	[];
extern  const   struct  spec_obj_type	spec_obj_table	[];


/*
 * Global variables
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern          SHOP_DATA *             shop_last;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_guild;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern          int                     top_guild;

extern		int			top_text;
extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;
extern		GUILD_DATA	*	guild_first;
extern		GUILD_DATA	*	guild_last;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];
extern	TEXT_DATA *		text_hash	[MAX_KEY_HASH];

extern	int			max_on;


/* act_wiz.c */
/*
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
*/
/* db.c */
void    reset_area      args( ( AREA_DATA * pArea ) );
void    reset_room      args( ( ROOM_INDEX_DATA *pRoom ) );

/* string.c */
void    string_edit     args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *  string_replace  args( ( char * orig, char * old, char * new ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *  string_unpad    args( ( char * argument ) );
char *  string_proper   args( ( char * argument ) );

/* olc.c */
bool    run_olc_editor  args( ( DESCRIPTOR_DATA *d, char *argument ) );
char    *olc_ed_name    args( ( CHAR_DATA *ch ) );
char    *olc_ed_vnum    args( ( CHAR_DATA *ch ) );
/* special.c */
char *	spec_string	args( ( SPEC_FUN *fun ) );	/* OLC */
char *	use_fun_string	args( ( USE_FUN *fun ) );	/* OLC */
char *  spec_obj_string     args( ( SPEC_OBJ *fun ) );      /* OLC */

/* bit.c */
extern const struct flag_type   area_flags[];
extern const struct flag_type   sex_flags[];
extern const struct flag_type   exit_flags[];
extern const struct flag_type   door_resets[];
extern const struct flag_type   room_flags[];
extern const struct flag_type   sector_flags[];
extern const struct flag_type   type_flags[];
extern const struct flag_type   extra_flags[];
extern const struct flag_type   wear_flags[];
extern const struct flag_type   act_flags[];
extern const struct flag_type   affect_flags[];
extern const struct flag_type   affect_2_flags[];
extern const struct flag_type   apply_flags[];
extern const struct flag_type   wear_loc_strings[];
extern const struct flag_type   wear_loc_flags[];
extern const struct flag_type   weapon_flags[];
extern const struct flag_type   container_flags[];
extern const struct flag_type   liquid_flags[];
extern const struct flag_type   furniture_flags[];
extern const struct flag_type   armor_flags[];
extern const struct flag_type	ingredient_flags[];
extern const struct flag_type	fruit_flags[];
extern const struct flag_type	weather_flags[];
extern const struct flag_type	climate_flags[];
extern const struct flag_type	terrain_flags[];
/* ROM OLC: */

extern const struct flag_type   material_type[];
extern const struct flag_type   form_flags[];
extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   off_flags[];
extern const struct flag_type   imm_flags[];
extern const struct flag_type   res_flags[];
extern const struct flag_type   vuln_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   weapon_type[];
extern const struct flag_type   guild_type[];



/*****************************************************************************
 *                                 OLC END                                   *
 *****************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RECLINING		      5
#define POS_RESTING		      6
#define POS_SITTING		      7
#define POS_FIGHTING		      8
#define POS_MOUNTED		      9
#define POS_STANDING		      10



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		(B)
/* RT auto flags */
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_SAFE		(I)
#define	PLR_NEWBIEHELPER	(J)
#define PLR_QUESTING		(K)
#define PLR_BOOK		(L)
/* 5 bits reserved, I-M */

/* RT personal flags */
#define PLR_HOLYLIGHT		(N)
#define PLR_WIZINVIS		(O)
#define PLR_CANLOOT		(P)
#define PLR_FOECOND		(Q)
#define PLR_NOFOLLOW		(R)
/* 4 bits reserved, S-V */


/* penalty flags */
#define PLR_PERMIT		(V)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_WOLFKIN		(Z)
#define PLR_DREAMWALKER		(aa)

/* New flags */
#define PLR_ANSI                (bb)
#define PLR_MARRIED		(cc)
#define PLR_NOQUIT		(dd)
#define PLR_SUBDUE		(ee)

/* Book flags */
#define	BOOK_1			(A)
#define	BOOK_2			(B)
#define	BOOK_3			(C)
#define	BOOK_4			(D)
#define	BOOK_5			(E)
#define	BOOK_6			(F)
#define	BOOK_7			(G)



/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOOOC              (E)
#define COMM_NOBARD             (G)
#define	COMM_NOGDT		(H)
#define	COMM_NOWDT		(I)
#define	COMM_NODFT		(J)
/* 4 channels reserved, H-K */

/* display flags */
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
/* 3 flags reserved, Q-S */

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W) 

/* new flags */
#define COMM_NOTES		(Y)
#define COMM_AFK		(Z)
#define COMM_NOSPAM		(aa)
#define COMM_TANKCOND		(bb)
#define COMM_AUTOHEAL		(cc)
#define COMM_NOTICK		(dd)

/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_TICKS               (B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)
#define WIZ_INSANE		(U)
#define WIZ_STAT		(V)
#define WIZ_CREATE		(W)
#define WIZ_MEMORY		(X)

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    sh_int		vnum;
    bool		new_format;
    sh_int		count;
    sh_int		killed;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		act;
    long		affected_by;
    sh_int		level;
    sh_int		hitroll;
    sh_int		hit[3];
    sh_int		damage[3];
    sh_int		ac[4];
    sh_int 		dam_type;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		gold;
    long		form;
    long		parts;
    sh_int		size;
    sh_int		material;
    sh_int		guild;
    sh_int		max;
    char *		text;
    GUARD_DATA *	guard;
    AREA_DATA *         area;           /* OLC */
};

/* memory settings */
#define MEM_CUSTOMER    A
#define MEM_SELLER      B
#define MEM_HOSTILE     C
#define MEM_AFRAID      D

/* memory for mobs */
struct mem_data
{
    MEM_DATA    *next;
    bool        valid;
    int         id;
    int         reaction;
    time_t      when;
};



/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		pet;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    MEM_DATA *          memory;
    PC_DATA *		pcdata;
    GEN_DATA *		gen_data;
    DO_FUN *		action;
    int			action_timer;
    char *		action_args;
    CHAR_DATA *		action_target;
    char *		name;
    sh_int		version;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		prompt;
    sh_int		sex;
    sh_int		class;
    sh_int		level;
    sh_int		trust;
    int			played;
    int			lines;  /* for the pager */
    long		id;
    time_t		logon;
    sh_int		timer;
    sh_int		wait;
    sh_int		hit;
    sh_int		max_hit;
    sh_int		stamina;
    sh_int		max_stamina;
    sh_int		race;
    sh_int		killed;
    long		gold;
    long		bank;
    int			exp;
    long		act;
    long		comm;   /* RT added to pad the vector */
    long		book;   /* Book channels */
    long                wiznet; /* wiz stuff */
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    int			affected_by;
    int			affected_by_2;
    sh_int		position;
    sh_int		practice;
    sh_int		train;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor[7];
    sh_int		wimpy;
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    sh_int		size;
    sh_int		material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		colors [MAX_COLORS];
    int			home;		/* Homes of people */
    sh_int		guild;		/* guild you belong to */
    CHAR_DATA *         hunting;        /* Used by hunting routine */
    OBJ_DATA *		on;		/* Used to sit on objects */
    CHAR_DATA *		mount;		/* Used for mounts */
    CHAR_DATA *		rider;		/* Used for mounts */
    sh_int		start_age;	/* Starting age of player */
    sh_int		still;		/* Stilling value :) */
    int			body;		/* Body condition */
    sh_int		fight_timer;	/* Timer for fighting */
    sh_int		channel_max	[5];
    sh_int		channel_skill	[5];
    bool		char_made;
    sh_int		hide_type;
    void *		hide;
};


/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    char *		last_name;
    char *		pwd;
    char *		transin;
    char *		transout;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		spouse;
    char *		afk_message;
    char *		new_name;
    char *		new_last;
    char *		new_title;
    char *		new_desc;
    char *		sedai;
    char *		shadow_name;
    char *		doing;
    char *		wearing;
    char *		email;		/* email address */
    sh_int		perm_hit;
    sh_int		perm_stamina;
    sh_int		true_sex;
    int			last_level;
    int			max_level;
    sh_int		condition	[3];
    sh_int		talent		[MAX_TALENT];
    sh_int		known		[MAX_TALENT];
    sh_int		learned		[MAX_SKILL];
    sh_int		skill_mod	[MAX_SKILL];
    sh_int		usage		[MAX_SKILL];
    bool		group_known	[MAX_GROUP];
    sh_int		points;
    bool              	confirm_delete;
    int                 security;       /* OLC */ /* Builder security */
    int			weapon;		/* starting weapon */
    int			qp;		/* quest points */
    sh_int		teach_skill;
    sh_int		learn_skill;    
    sh_int		stat_count;	/* count for rolling stats */
    sh_int		stat_point;	/* points for rolling stats */
    sh_int		shadow_rank;	/* Rank as a Shadow Friend */
    char *              alias[MAX_ALIAS];     /* From 2.4 */
    char *              alias_sub[MAX_ALIAS]; /* too */
    sh_int		hits;
    sh_int		misses;
    sh_int		count;
    sh_int		hand;
    int			insane;
    time_t		last_tax;
    time_t		last_note;
    time_t		last_idea;
    time_t		last_penalty;
    time_t		last_news;
    time_t		last_changes;
    time_t		last_apply;
    BUFFER	*	buffer;
    sh_int		stat_use[MAX_STATS];
    GUILD	*	guild;
    int			config;
};

/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    bool	skill_chosen[MAX_SKILL];
    bool	group_chosen[MAX_GROUP];
    int		points_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		39

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    SPEC_OBJ *          spec_fun;
    USE_FUN *		use_fun;
    bool		new_format;
    char *		name;
    char *		short_descr;
    char *		description;
    int			version;
    sh_int		vnum;
    sh_int		reset_num;
    sh_int		material;
    sh_int		item_type;
    int			extra_flags;
    sh_int		wear_flags;
    sh_int		level;
    sh_int 		condition;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			qp;
    int			value[8];
    sh_int		max;
    AREA_DATA *         area;           /* OLC */
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    bool		enchanted;
    char *	        owned; 
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		item_type;
    int			extra_flags;
    sh_int		wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int 		condition;
    sh_int		material;
    sh_int		timer;
    int			value	[8];
    CHAR_DATA *		owner;
    OBJ_DATA *		on;
    OBJ_DATA *		match;	/* Matching portal */
};



/*
 * Exit data.
 */
struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	sh_int			vnum;
    } u1;
    sh_int		exit_info;
    sh_int		key;
    char *		keyword;
    char *		description;
    EXIT_DATA *         next;           /* OLC */
    int                 rs_flags;       /* OLC */
    int                 orig_door;      /* OLC */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
};



/*
 * Area definition.
 */
struct area_data
{
    AREA_DATA *         next;           /*
    RESET_DATA *        reset_first;     *  Removed for OLC reset system.
    RESET_DATA *        reset_last;      */
    char *              name;
    int                 age;
    int                 nplayer;
    bool                empty;          /* ROM OLC */
    char *              filename;       /* OLC */
    char *              builders;       /* OLC */ /* Listing of */
    char *		moddate;
    int                 security;       /* OLC */ /* Value 1-9  */
    int                 lvnum;          /* OLC */ /* Lower vnum */
    int                 uvnum;          /* OLC */ /* Upper vnum */
    int                 vnum;           /* OLC */ /* Area vnum  */
    int                 area_flags;     /* OLC */
    sh_int		timer;		/* timer for area auto save */

    sh_int		climate;	/* Climate */
    sh_int		terrain;	/* dominant terrain */
    sh_int		weather;	/* dominant weather patterns */
    sh_int		wind_dir;	/* Wind direction */
    sh_int		wind_str;	/* Wind strength */
    sh_int		temp;		/* Area Temperature */
    int			mmhg;		/* Air pressure */
};


#define	CLIMATE_TROPICAL	0
#define	CLIMATE_SUBTROPICAL	1
#define	CLIMATE_TEMPERATE	2
#define	CLIMATE_ARCTIC		3

#define TERRAIN_GRASSLAND	0
#define TERRAIN_FOREST		1
#define TERRAIN_SWAMP		2
#define TERRAIN_MOUNTAIN	3
#define TERRAIN_COAST		4
#define TERRAIN_DESERT		5
#define TERRAIN_HILL		6

#define WEATHER_SUNNY		0
#define WEATHER_CLOUDY		1
#define WEATHER_FOGGY		2	
#define WEATHER_RAINY		3
#define WEATHER_SNOWY		4
#define WEATHER_HAIL		5
#define WEATHER_STORMY		6


/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    AFFECT_DATA *	affected;
    SPEC_ROOM *		spec_fun;
    char *		name;
    char *		description;
    sh_int		vnum;
    int			room_flags;
    int			resources;
    sh_int		light;
    sh_int		sector_type;
    RESET_DATA *        reset_first;    /* OLC */
    RESET_DATA *        reset_last;     /* OLC */
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_CHAR_OTHER		    5
#define TAR_OBJ_HERE		    6
#define TAR_CHAR_OBJ		    7
#define TAR_CHAR_WORLD		    8
#define TAR_ROOM		    9
#define TAR_IN_ROOM		    10

#define	TARGET_ROOM		0
#define TARGET_OBJ		1
#define TARGET_CHAR		2

/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/	
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	stat;			/* What stat it's based on	*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	prerequisite;		/* Prerequisite for skill	*/
    bool	non_chan;		/* non-channeler only		*/
    bool	cascade;		/* is a cascade skill		*/
    bool	teachable;		/* is teachable			*/
    int		power[5];		/* Power usage for spells:
					   earth, air, fire, water, spirit */
};

struct talent_type
{
    char * 	name;
    int		diff;
    bool	non_chan;		/* If non-channelers can gain this */
    sh_int *	pgsn;			/* Pointer to associated tn	*/
};

struct herb_type
{
    int		herb;
    int		terrain;
    int		diff;
    int		vnum;
};

struct potion_type
{
    char *	name;
    int		ingredients;
    sh_int	diff;
    sh_int	type;
    int		affect;    
};

/*
 * Un colour type.
 */
struct color_data
{
   char         code[10];
   char         act_code[5];
   char         name[15];
   int          number;
};

struct  group_type
{
    char *	name;
    sh_int	rating[MAX_CLASS];
    char *	spells[MAX_IN_GROUP];
};


/*
 * Talents sns so talent_lookup doesn't go to hell and back from calls :)
 */

extern	sh_int                  tn_str_talent;   
extern	sh_int                  tn_int_talent;
extern	sh_int                  tn_wis_talent;
extern	sh_int                  tn_dex_talent;  
extern	sh_int                  tn_con_talent; 
extern	sh_int                  tn_cha_talent;
extern	sh_int                  tn_luc_talent;
extern	sh_int                  tn_agi_talent;

extern	sh_int			tn_herbalism;
extern	sh_int			tn_geology;
extern	sh_int			tn_teaching;

extern	sh_int                  tn_blademastery;
extern	sh_int                  tn_master_thief;
extern	sh_int			tn_craftsmanship;

extern	sh_int                  tn_earth_talent;
extern	sh_int                  tn_air_talent;
extern	sh_int                  tn_fire_talent;
extern	sh_int                  tn_water_talent;
extern	sh_int                  tn_spirit_talent;

extern	sh_int                  tn_sense_taveren;
extern	sh_int                  tn_sense_terangreal;
extern	sh_int                  tn_create_terangreal;
extern	sh_int                  tn_dreamwalking;
extern	sh_int                  tn_foretelling;
extern	sh_int                  tn_shielding; 
extern	sh_int                  tn_charm;
extern	sh_int                  tn_combat;
extern	sh_int                  tn_creation;
extern	sh_int                  tn_detection;   
extern	sh_int                  tn_enchantment;
extern	sh_int                  tn_healing;
extern	sh_int                  tn_maladictions;
extern	sh_int                  tn_protective;
extern	sh_int                  tn_transportation;
extern	sh_int                  tn_weather;
extern	sh_int			tn_channeler;
extern	sh_int			tn_powerful_channeler;

extern	sh_int			tn_weak_taveren;
extern	sh_int			tn_taveren;
extern	sh_int			tn_strong_taveren;


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	sh_int	gsn_backstab;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_shield;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;

extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern  sh_int  gsn_plague;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;

/* new gsns */
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_spear;
extern sh_int  gsn_staff;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;
 
extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;
extern sh_int  gsn_weapon_prof;
extern sh_int  gsn_martial_arts;
extern sh_int  gsn_shield_bash;
 
extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_concentration;
 
/* newer gsns */

extern sh_int	gsn_shape_change;
extern sh_int	gsn_earth;
extern sh_int	gsn_air;
extern sh_int	gsn_fire;
extern sh_int	gsn_water;
extern sh_int	gsn_spirit;
extern sh_int	gsn_fourth_attack;
extern sh_int	gsn_stalk;
extern sh_int	gsn_teaching;
extern sh_int	gsn_gambling;
extern sh_int	gsn_disguise;
extern sh_int	gsn_haste;
extern sh_int	gsn_air_armor;
extern sh_int	gsn_ward_person;
extern sh_int	gsn_ambidexterity;
extern sh_int	gsn_dual_wield;
extern sh_int	gsn_set_snare;
extern sh_int	gsn_hunt;
extern sh_int	gsn_swimming;
extern sh_int	gsn_riding;
extern sh_int	gsn_medicine;
extern sh_int	gsn_tie_weave;
extern sh_int	gsn_invert_weave;
extern sh_int	gsn_summon_wolf;
extern sh_int	gsn_envenom;
extern sh_int	gsn_feint;
extern sh_int	gsn_forms;
extern sh_int	gsn_riposte;
extern sh_int	gsn_switch_opponent;
extern sh_int	gsn_heroic_rescue;
extern sh_int	gsn_pilfer;
extern sh_int	gsn_blindfighting;
extern sh_int	gsn_hail_storm;
extern sh_int	gsn_lightning_storm;
extern sh_int	gsn_old_tongue;
extern sh_int	gsn_acrobatics;
extern sh_int	gsn_create_flame_sword;
extern sh_int	gsn_create_air_sword;
extern sh_int	gsn_create_spring;
extern sh_int	gsn_travel;
extern sh_int	gsn_skim;
extern sh_int	gsn_brew;
extern sh_int	gsn_herbalism;
extern sh_int	gsn_forestry;
extern sh_int	gsn_foraging;
extern sh_int	gsn_mining;
extern sh_int	gsn_wind_barrier;
extern sh_int	gsn_earth_barrier;
extern sh_int	gsn_fire_wall;
extern sh_int	gsn_ice_wall;
extern sh_int	gsn_incognito;

/* Aes Sedai gsns */
extern sh_int	gsn_detect_shadowspawn;
extern sh_int	gsn_detect_channeler;
extern sh_int	gsn_teach_channeling;
extern sh_int	gsn_identify_fauna;
extern sh_int	gsn_complaisance;
extern sh_int	gsn_geography;

/* Warder gsns */
extern sh_int	gsn_flash_strike;
extern sh_int	gsn_sweep;
extern sh_int	gsn_endurance;

/* Whitecloak gsns */
extern sh_int	gsn_group_fighting;
extern sh_int	gsn_capture;
extern sh_int	gsn_intimidation;
extern sh_int	gsn_questioning;

/* Seanchan gsns */
extern sh_int	gsn_leashing;

/* Aiel gsns */
extern sh_int	gsn_hardiness;
extern sh_int	gsn_spear_dancing;

/* creation gsns */
extern sh_int	gsn_gemworking;
extern sh_int	gsn_smithing;
extern sh_int	gsn_carpentry;
extern sh_int	gsn_sewing;
extern sh_int	gsn_leatherworking;
extern sh_int	gsn_repairing;
extern sh_int	gsn_balefire;
extern sh_int	gsn_grasping;
extern sh_int	gsn_linking;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define replace_string( pstr, nstr ) \
	{ free_string( (pstr) ); pstr=str_dup( (nstr) ); }
#define IS_NULLSTR(str)	((str)==NULL || (str)[0]=='\0' || (str) == &str_empty[0])
#define CH(d)		((d)->original ? (d)->original : (d)->character )
#define IS_WRITING(ch)		( ch && ch->desc && ch->desc->pString )
#define WEATHER(ch)		( weather_info.sky )
#define	CLIMATE(ch)		( (ch->in_room == NULL) ? CLIMATE_TEMPERATE : \
				   ch->in_room->area->climae )
#define	TERRAIN(ch)		( (ch->in_room == NULL) ? TERRAIN_GRASSLAND : \
				   ch->in_room->area->terrain )
#define	EXP_TO_LEVEL(ch)	( IS_NPC(ch) ? 0 :			\
				  (ch->level+1) *			\
				  exp_per_level(ch,ch->pcdata->points) - \
				  ch->exp )
/*
 *  Guild macros
 */
#define SET_RANK(ch, set, value)                                        \
        { int i;for (i = 0; i < 4; i++) if ( IS_SET(value, 1<<i) )      \
          SET_BIT(ch->pcdata->guild->rank, (1<<( (set - 1) * 4 )) << i);\
          else                                                          \
          REMOVE_BIT(ch->pcdata->guild->rank, (1<<( (set - 1) * 4 )) << i); }
#define GET_RANK(ch, set)                                               \
        ( (ch->pcdata->guild->rank >> ( (set - 1) * 4 )) & ~(~0 << 4) )
#define IS_GUILDED(ch)                                                  \
        ( IS_NPC(ch) ? FALSE : ch->guild == 0 ? FALSE :                 \
          ch->pcdata->guild == NULL ? FALSE : TRUE )
#define IS_IN_GUILD(ch, value)						\
	( IS_NPC(ch) ? FALSE : ch->guild != value ? FALSE :		\
	  ch->pcdata->guild == NULL ? FALSE : TRUE )


/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED_2(ch, sn)	(IS_SET((ch)->affected_by_2, (sn)))
#define CHARM_SET(ch)		(IS_AFFECTED((ch), AFF_CHARM) ||	\
				 IS_AFFECTED_2((ch), AFF_LEASHED))
#define	IS_BRIEF(ch)		( IS_SET(ch->comm, COMM_BRIEF) )

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_SENTIENT(ch)		(IS_SET((ch)->form, FORM_SENTIENT))
#define HAS_GROUP(ch, gsn)	(IS_NPC(ch) ? FALSE : (ch)->pcdata->group_known[(gsn)] )
#define	IS_SKILLED(ch, sn)	(IS_NPC(ch) ? get_skill((ch), (sn)) : \
				(ch->pcdata->learned[(sn)] ? TRUE : FALSE))
#define	IS_TALENTED(ch, tn)	(IS_NPC(ch) ? FALSE : (ch)->pcdata->talent[(tn)] )
#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_HITROLL(ch)	\
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_DEX)].off_mod)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)
#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))
#define WAIT_STATE(ch, npulse)	\
	{ add_wait_list(ch);ch->wait = UMAX((ch)->wait, (npulse)); }
#define IS_ANSI(ch)             (!IS_NPC(ch) && IS_SET((ch)->act, PLR_ANSI))
#define	IS_IDLE(ch)		(ch->desc && ch->desc->idle >= 300)
#define SKILL(ch,sn)		(ch->pcdata->learned[sn] + ch->pcdata->skill_mod[sn])

#define IS_CONFIG(ch,sn)	( IS_NPC(ch) ? FALSE :			\
				  IS_SET((ch)->pcdata->config,(sn)) )

#define IS_ROGUE(ch)		( IS_NPC(ch) ? IS_SET(ch->act, ACT_ROGUE) : \
				  (ch->class == 1) )
#define IS_WARRIOR(ch)		( IS_NPC(ch) ? IS_SET(ch->act, ACT_WARRIOR) : \
				  (ch->class == 0) )
#define IS_SCHOLAR(ch)		( IS_NPC(ch) ? IS_SET(ch->act, ACT_SCHOLAR) : \
				  (ch->class == 2) )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define CONDITION(obj)		( UMAX(0, obj->condition) )

#define	CASE_LOWER		0
#define CASE_UPPER		1


/*
 * Description macros.
 */
#define IS_DISGUISED(ch)						\
		( IS_NPC(ch) ? FALSE :					\
		  IS_AFFECTED((ch),AFF_SHAPE_CHANGE) ? TRUE : FALSE )
#define PERS(ch, looker)	( personal((ch), (looker)) )
#define	SHADOWNAME(ch)		( IS_NPC(ch) ? (ch)->short_descr : 	\
				  IS_NULLSTR(ch->pcdata->shadow_name) ? \
				  (ch)->name : ch->pcdata->shadow_name )
#define FIRSTNAME(ch)		( IS_NPC(ch) ? (ch)->short_descr : \
				  IS_DISGUISED(ch) ? \
				( IS_NULLSTR(ch->pcdata->new_name) ? \
				  (ch)->name : (ch)->pcdata->new_name) : ch->name )
#define	LASTNAME(ch)		( IS_NPC(ch) ? "" : IS_DISGUISED(ch) ? \
				  (IS_NULLSTR(ch->pcdata->new_last) ? "" : \
				   (ch)->pcdata->new_last) : \
				  (IS_NULLSTR(ch->pcdata->last_name) ? \
				   "" : (ch)->pcdata->last_name) )
#define	TITLE(ch)		( IS_NPC(ch) ? "" : IS_DISGUISED(ch) ? \
				  (IS_NULLSTR(ch->pcdata->new_title) ? "" : \
				   (ch)->pcdata->new_title) : \
				  (IS_NULLSTR(ch->pcdata->title) ? \
				   "" : (ch)->pcdata->title) )
#define	TRUE_SEX(ch)		( IS_NPC(ch) ? (ch)->sex :		\
				  (ch)->pcdata->true_sex )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *    char_auto;
    char *    others_auto;
};



/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app		[28];
extern	const	struct	int_app_type	int_app		[28];
extern	const	struct	wis_app_type	wis_app		[28];
extern	const	struct	dex_app_type	dex_app		[28];
extern	const	struct	con_app_type	con_app		[28];
extern	const	struct	cha_app_type	cha_app		[28];
extern	const	struct	luk_app_type	luk_app		[28];
extern	const	struct	agi_app_type	agi_app		[28];

extern  const   struct  weapons_type    weapon_table    [];
extern  const   struct  wiznet_type     wiznet_table    [];
extern	const	struct	class_type	class_table	[MAX_CLASS];
extern  const   struct  color_data      color_table     [];
extern	const	struct	attack_type	attack_table	[];
extern  const	struct  race_type	race_table	[];
extern  const   struct  pc_race_type    pc_race_table   [];
extern	const	struct	dice_type	dice_table	[];
extern	const	struct	break_type	break_table	[];
extern	const   struct  forge_type      forge_table     [];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern	const	struct	talent_type	talent_table	[MAX_TALENT+10];
extern	const	struct	potion_type	potion_table	[];
extern	const	struct	herb_type	herb_table	[];
extern  const   struct  group_type      group_table	[MAX_GROUP];
extern          struct	social_type	social_table[MAX_SOCIALS];
extern	const	struct	guilds_type	guild_table[];
extern	const	int			channel_table[5][101];

#define MAX_COLOR_LIST             16

extern  char *  const                   color_list      [MAX_COLOR_LIST];




/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		HELP_DATA	  *	help_last;
extern		SHOP_DATA	  *	shop_first;
extern		SHOP_DATA	  *	shop_last;
extern		GUILD_DATA	  *	guild_first;
extern		GUILD_DATA	  *	guild_last;

extern		BAN_DATA	  *	ban_list;
extern		CHAR_DATA	  *	char_list;
extern		NODE_DATA	  *	weave_list;
extern		NODE_DATA	  *	spec_list;
extern		NODE_DATA	  *	fight_list;
extern		NODE_DATA	  *	pc_list;
extern		NODE_DATA	  *	room_list;
extern		NODE_DATA	  *	wait_list;
extern		GEN_DATA	  *	gen_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		NOTE_DATA	  *	note_list;
extern		OBJ_DATA	  *	object_list;

extern		AFFECT_DATA	  *	affect_free;
extern		BAN_DATA	  *	ban_free;
extern		CHAR_DATA	  *	char_free;
extern		DESCRIPTOR_DATA	  *	descriptor_free;
extern		EXTRA_DESCR_DATA  *	extra_descr_free;
extern		NOTE_DATA	  *	note_free;
extern		OBJ_DATA	  *	obj_free;
extern		PC_DATA		  *	pcdata_free;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		time_t			boot_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;

extern		int			num_herb;
extern		int			num_lumber;
extern		int			num_ore;
extern		int			num_fish;
extern		int			num_gem;
extern		int			num_explore;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ));
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
siz_t	fread		args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#else
/*
int	fread		args( ( void *ptr, int size, int n, FILE *stream ));
*/
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""		/* Player files			*/
#define PLAYER_TEMP	"temp"
#define NULL_FILE	"proto.are"		/* To reserve one stream	*/
#endif

#if defined(MSDOS)
#define NULL_FILE	"nul"		/* To reserve one stream	*/
#endif

#if defined(unix)
#define HELP_FILE	"weave.hlp"	/* Help file */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define PLAYER_TEMP	"../player/temp"
#define GOD_DIR		"../gods/"	/* list of gods			*/
#define BAN_FILE	"ban.txt"	/* ban file			*/
#define GUILD_FILE	"guild.txt"	/* guild file */
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define	COMMAND_FILE	"../log/command_log"	/* Command log #1 */
#define COMMAND_LIST	"commands.txt"	/* List of commands		*/
#endif

#define AREA_LIST	"area.lst"	/* List of areas		*/

#define BUG_FILE	"bugs.txt"      /* For 'bug' and bug( )		*/
#define IDEA_FILE	"ideas.txt"	/* For 'idea'			*/
#define TYPO_FILE	"typos.txt"     /* For 'typo'			*/
#define NOTE_FILE	"notes.txt"	/* For 'notes'			*/
#define APPLY_FILE	"apply.txt"	/* application file		*/
#define NOTE_IDEA_FILE	"ideas.txt"	/* For 'ideas'			*/
#define NOTE_PENALTY_FILE	"penalty.txt"	/* For 'penalties'	*/
#define NOTE_NEWS_FILE	"news.txt"	/* For 'news'			*/
#define NOTE_CHANGES_FILE	"changes.txt"	/* For 'changes'	*/
#define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/



/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define GD	GUILD_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define	ND	NODE_DATA

/* act_comm.c */
bool    is_note_to      args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	add_stalk	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_stalk	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
void    ansi_color      args( ( const char *txt, CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
int	stat_avg	args( ( CHAR_DATA *ch, int stat ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );

/* act_obj.c */
bool 	can_loot	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );

/* act_wiz.c */
void wiznet             args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
                               long flag, long flag_skip, int min_level ) );

/* comm.c */
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void	send_to_char_new args( ( CHAR_DATA *ch, char *format, ... ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );
void	act_fight	args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type ) );
void	act_channel	args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type ) );
void	act_immortal	args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type ) );
void	broadcast	args( ( const char *str, CHAR_DATA *ch, int type ) );

/* db.c */
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
TEXT_DATA *get_text_index args( ( int id ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	right_case	args( ( const char *str, int fCase ) );
char *	capitalize	args( ( const char *str ) );
char *	correct_name	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );

/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, int class, bool second ) );
bool	spell_damage	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, int class ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );

/* handler.c */
void	drop_link	args( ( CHAR_DATA *ch ) );
void	link_stamina	args( ( CHAR_DATA *leader, CHAR_DATA *ch, bool fGain ) );
int	link_count	args( ( CHAR_DATA *ch, int sex ) );
bool	can_join_link	args( ( CHAR_DATA *ch, CHAR_DATA *join ) );
void	break_con	args( ( CHAR_DATA *ch ) );
bool	eq_is_free	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
int	weapon_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
bool	has_color	args( (const char *str) );
int	channel_strength	args( (CHAR_DATA *ch, int power) );
void	die_weave	args( (CHAR_DATA *ch) );
bool	is_metal	args( (OBJ_DATA *obj ) );
bool	can_channel	args( (CHAR_DATA *ch, int skill ) );
char *  material_name   args( ( sh_int num ) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int 	material_lookup args( ( char *name) );
int 	guild_lookup args( ( const char *name) );
GD * 	guild_struct args( ( int number ) );
char* 	guild_name args( ( int number ) );
char* 	guild_savename args( ( int number ) );
char* 	guild_rank args( ( int guild, sh_int rank, int type, bool fDup )
);
int	race_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_secondary_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int	get_secondary_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
bool	check_skill	args( (CHAR_DATA *ch, int sn, int skill_mod, bool pLuck) );
bool	check_stat	args( (CHAR_DATA *ch, int stat, int mod) );
int	get_skill	args( (CHAR_DATA *ch, int sn) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool	is_full_name	args( ( char *str, char *namelist ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
RID *	get_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	random_room_char	args( ( ROOM_INDEX_DATA *in_room ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_sedai	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int amount ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	room_flag_name args( ( int room_flag ) );
char *  resource_string	args( ( int resource ) );
char *  ingredient_string	args( ( int ingredient ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( int vector ) );
char *	affect_bit_name_2	args( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char * 	wear_bit_name	args( ( int wear_flags ) );
char *	act_bit_name	args( ( int act_flags ) );
char *	off_bit_name	args( ( int off_flags ) );
char *  imm_bit_name	args( ( int imm_flags ) );
char * 	form_bit_name	args( ( int form_flags ) );
char *	part_bit_name	args( ( int part_flags ) );
char *	weapon_bit_name	args( ( int weapon_flags ) );
char *  comm_bit_name	args( ( int comm_flags ) );
char *  color_value_string args( ( int color, bool bold, bool flash ) );
int     strlen_color    args( ( char *argument ) );
bool	is_room_affect	args( ( ROOM_INDEX_DATA *vRoom, sh_int type ) );
bool	is_guild_imm	args( ( CHAR_DATA *ch ) );
int	health_status	args( ( CHAR_DATA *ch ) );
int	health_percent	args( ( CHAR_DATA *ch ) );
int	stamina_status	args( ( CHAR_DATA *ch ) );
int	stamina_percent	args( ( CHAR_DATA *ch ) );
bool	lose_stamina	args( ( CHAR_DATA *ch, int loss, bool use_weight, bool force_loss ) );
bool	gain_stamina	args( ( CHAR_DATA *ch, int gain, bool force_gain ) );
bool	lose_health	args( ( CHAR_DATA *ch, int loss, bool force_loss ) );
bool	gain_health	args( ( CHAR_DATA *ch, int gain, bool force_gain ) );
bool	cure_condition	args( ( CHAR_DATA *ch, int condition, int chance ) );
bool	is_warder	args( ( CHAR_DATA *ch ) );
bool	is_darkfriend	args( ( CHAR_DATA *ch ) );
bool	is_protected	args( ( CHAR_DATA *ch ) );
bool	is_forsaken	args( ( CHAR_DATA *ch ) );
bool	is_channel_skill	args( ( int sn ) );
char	*personal	args( ( CHAR_DATA *ch, CHAR_DATA *looker ) );
int	potion_lookup	args( ( const char *name ) );
char	*potion_name	args( ( sh_int num ) );
OD *	get_bracelet	args( ( CHAR_DATA *ch ) );
OD *	get_collar	args( ( CHAR_DATA *ch ) );
CD *    is_damane	args( ( CHAR_DATA *ch ) );
CD *	get_suldam	args( ( CHAR_DATA *ch ) );
int	hand_count	args( ( CHAR_DATA *ch ) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );

/* newmagic.c */
int 	stamina_cost 	args( ( CHAR_DATA *ch, int sn ) );
int	skill_lookup	args( ( const char *name ) );
int	talent_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );
void	remove_shape	args( ( CHAR_DATA *ch ) );
void	mob_cast	args( ( CHAR_DATA *ch, void *vo, int sn, int multiplier, int target_type ) );
bool	check_power	args( ( CHAR_DATA *ch, int power ) );

/* save.c */
void	*load_value( char *argument, int type );
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
char *  print_flags	args( ( int flag ) );
/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );
void	remove_skill	args( ( CHAR_DATA *ch ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
SPEC_OBJ *	spec_obj_lookup	args( ( const char *name ) );
USE_FUN *	use_fun_lookup		args( ( const char *name ) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool display ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );

/* hunt.c */
void    hunt_victim     args( ( CHAR_DATA *ch ) );

/* alias.c */
void    substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );
char *  one_command      args( (char *argument, char *arg_first) );

/* quest.c */
int	item_level	args( (OBJ_DATA *obj) );
RID	*get_random_room(CHAR_DATA *ch);
bool	is_guild_eq	args( ( int vnum ) );
int	quest_cost( OBJ_DATA *obj );
void	save_helps( );

/* guild.c */
int	get_guild_skill	args( ( int sn ) );
int	get_skill_rank	args( ( int sn ) );


/* list.c */
void destroy_list	args( ( NODE_DATA *list ) );
void add_fight_list	args( ( CHAR_DATA *ch ) );
void rem_fight_list	args( ( CHAR_DATA *ch ) );
void add_pc_list	args( ( CHAR_DATA *ch ) );
void rem_pc_list	args( ( CHAR_DATA *ch ) );
void add_weave_list	args( ( void *data, sh_int data_type ) );
void rem_weave_list	args( ( void *data, sh_int data_type ) );
void add_wait_list	args( ( CHAR_DATA *ch ) );
void rem_wait_list	args( ( CHAR_DATA *ch ) );
void add_room_list	args( ( ROOM_INDEX_DATA *pRoom ) );
void rem_room_list	args( ( ROOM_INDEX_DATA *pRoom ) );
void add_spec_list	args( ( void *data, sh_int data_type ) );
void rem_spec_list	args( ( void *data, sh_int data_type ) );

/* affects.c */
void	affect_to_char		args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj		args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_to_room		args( (ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_remove		args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_obj_remove	args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_room_remove	args( (ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_strip		args( ( CHAR_DATA *ch, int sn ) );
void	affect_obj_strip	args( ( OBJ_DATA *obj, int sn ) );
void	affect_room_strip	args( (ROOM_INDEX_DATA *room, int sn ) );
bool	is_affected		args( ( CHAR_DATA *ch, int sn ) );
bool	is_obj_affected		args( ( OBJ_DATA *obj, int sn ) );
bool	is_room_affected	args( (ROOM_INDEX_DATA *room, int sn ) );
void	affect_join		args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
int	affect_lookup		args( ( int vnum ) );
void	affect_from_index	args( ( OBJ_DATA *obj ) );

/* critical.c */
void	fumble	args( (CHAR_DATA *ch, CHAR_DATA *victim, int roll, int dam_type, int sn) );
bool	critical	args( (CHAR_DATA *ch, CHAR_DATA *victim, int roll, int dam_type, int sn, bool secondary) );

/* ban.c */
bool	check_ban	args( (DESCRIPTOR_DATA *d, int type) );

/* talent.c */
void	check_taveren	args( (CHAR_DATA *ch) );


#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	GD
#undef	ND
