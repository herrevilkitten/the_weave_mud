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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"


bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );



/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;




/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_MOUNTED,     0,  LOG_NEVER, 0 },
    { "east",		do_east,	POS_MOUNTED, 	 0,  LOG_NEVER, 0 },
    { "south",		do_south,	POS_MOUNTED, 	 0,  LOG_NEVER, 0 },
    { "west",		do_west,	POS_MOUNTED,	 0,  LOG_NEVER, 0 },
    { "up",		do_up,		POS_MOUNTED,	 0,  LOG_NEVER, 0 },
    { "down",		do_down,	POS_MOUNTED,	 0,  LOG_NEVER, 0 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "group",		do_group,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "book",		do_book,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "ooc",		do_ooc,		POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "for",		do_for,		POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "at",             do_at,          POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "auction",        do_auction,     POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "balance",	do_balance,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "deposit",	do_deposit,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "withdraw",	do_withdraw,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "channel",	do_cast,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "cast",		do_cast,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "echannel",	do_ecast,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "ecast",		do_ecast,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "grasp",		do_grasp,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "release",	do_release,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "weaves",		do_weaves,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "invert",		do_invert,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "bard",           do_bard,   	POS_SLEEPING,    0,  LOG_NORMAL, 1 }, 
    { "practice",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "recline",	do_recline,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "sockets",        do_sockets,	POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "whisper",	do_whisper,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "dirsay",		do_dirsay,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "-",		do_dirsay,	POS_RECLINING,	 0,  LOG_NORMAL, 0 },
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "second",		do_second,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "drag",		do_drag,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "enter",		do_enter,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "donate",		do_donate,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "tie",		do_tie,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "untie",		do_untie,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "cancel",		do_cancel,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "treat",		do_treat,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "armor",		do_armor,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    /*
     * Informational commands.
     */
    { "link",		do_link,	POS_STANDING,	 0, LOG_NORMAL,1 },
    { "unlink",		do_unlink,	POS_STANDING,	 0, LOG_NORMAL,1 },
    { "alias",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "unalias",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "compare",	do_compare,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "penalty",	do_penalty,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "info",           do_groups,      POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "read",		do_read,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "worth",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "scan",		do_scan,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "channeling",	do_channeling,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "track",		do_hunt,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "wolfsummon",	do_wolfsummon,	POS_STANDING,	 0,  LOG_NORMAL, 1 },

    /*
     * Configuration commands.
     */
    { "autolist",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "autoall",	do_autoall,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "color",		do_color,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autogold",	do_autogold,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosac",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "brief",		do_brief,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "combine",	do_combine,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "compact",	do_compact,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, 0 },
    { "delete",		do_delete,	POS_DEAD,	 0,  LOG_ALWAYS, 1 },
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER,  1 },
    { "prompt",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "teach",		do_teach,	POS_STANDING,  101,  LOG_NORMAL, 1 },
    { "last",		do_last,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "config",		do_config,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "brew",		do_brew,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "create",		do_create,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "search",		do_search,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "toss",		do_toss,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },


    /*
     * Communication commands.
     */
    { "afk",		do_afk,		POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "deaf",		do_deaf,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { ":",		do_emote,	POS_RECLINING,	 0,  LOG_NORMAL, 0 },
    { "emote",		do_emote,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "order",		do_order,	POS_RECLINING,	 0,  LOG_ALWAYS, 1 },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "gsay",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "guild",		do_guild,	POS_SLEEPING,	 0,  LOG_NORMAL, 0 },
    { "dftalk",		do_dftalk,	POS_SLEEPING,	 0,  LOG_NORMAL, 0 },
    { "gdtalk",		do_gdtalk,	POS_SLEEPING,	 0,  LOG_NORMAL, 0 },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "unread",		do_unread,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "newnote",	do_nonote,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "combat",		do_combat,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1 },
    { "osay",		do_osay,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "isay",		do_isay,	POS_RECLINING,	L8,  LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "'",		do_say,		POS_RECLINING,	 0,  LOG_NORMAL, 0 },
    { "yell",		do_yell,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "mtalk",		do_mtalk,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "think",		do_think,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },

    /*
     * Object manipulation commands.
     */
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "drink",		do_drink,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "empty",		do_empty,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL, 1 }, 
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "knock",		do_knock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "put",		do_put,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "apply",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "swallow",	do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "sacrifice",	do_sacrifice,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "junk",           do_sacrifice,   POS_RECLINING,	 0,  LOG_NORMAL, 0 },
    { "tap",      	do_sacrifice,   POS_RECLINING,	 0,  LOG_NORMAL, 0 },   
    { "unlock",		do_unlock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "doing",		do_doing,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wearing",	do_wearing,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "use",		do_use,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "forge",		do_forge,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "repair",		do_repair,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "fix",		do_fix,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "mount",		do_mount,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "dismount",	do_dismount,	POS_MOUNTED,	 0,  LOG_NORMAL, 1 },
    { "lore",		do_lore,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "appraise",	do_appraise,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "snare",		do_snare,	POS_STANDING,	 0,  LOG_NORMAL, 1 },

    /*
     * Combat commands.
     */
    { "assist",		do_assist,	POS_MOUNTED,	 0,  LOG_NORMAL, 1 },
    { "backstab",	do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, 0 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "murde",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "murder",		do_murder,	POS_FIGHTING,	 0,  LOG_ALWAYS, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "trip",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL, 1 },

    /*
     * Miscellaneous commands.
     */
    { "follow",		do_follow,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "stalk",		do_stalk,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "gain",		do_gain,	POS_MOUNTED,	 0,  LOG_NORMAL, 1 },
    { "forget",		do_forget,	POS_MOUNTED,	 0,  LOG_NORMAL, 1 },
    { "ungroup",	do_ungroup,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "groups",		do_groups,	POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "hide",		do_hide,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, 0 },
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RECLINING,	 0,  LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "disguise",	do_disguise,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "newbie",		do_newbiehelper,POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "quest",		do_quest,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "email",		do_email,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "application",	do_application, POS_DEAD,	 0,  LOG_NORMAL, 1 },

    /*
     * Immortal commands.
     */
    { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "trust",		do_trust,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "permdelet",	do_permdelet,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "permdelete",	do_permdelete,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "import",		do_import,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },

    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "fullreboot",	do_fullreboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "ban",            do_ban,         POS_DEAD,       L2,  LOG_ALWAYS, 1 },
    { "permban",        do_permban,     POS_DEAD,       L1,  LOG_ALWAYS, 1 },
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "freeze",		do_freeze,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "marry",		do_marry,	POS_STANDING,	L3,  LOG_NORMAL, 1 },
    { "sla",		do_sla,		POS_DEAD,	L3,  LOG_NORMAL, 0 },
    { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "gecho",		do_echo,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L8,  LOG_ALWAYS, 1 }, 
    { "recho",		do_recho,	POS_DEAD,	L8,  LOG_ALWAYS, 1 }, 
    { "newname",	do_newname,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "set",		do_set,		POS_DEAD,	L5,  LOG_ALWAYS, 1 },

    { "load",		do_load,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "peace",		do_peace,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "purge",		do_purge,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "snoop",		do_snoop,	POS_DEAD,	L5,  LOG_ALWAYS, 1 },
    { "snoops",		do_snoops,	POS_DEAD,	L1,  LOG_NORMAL, 1 },
    { "string",		do_string,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "tool",		do_tool,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "transfer",	do_transfer,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "clone",		do_clone,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },

    { "switch",		do_switch,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "return",		do_return,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "giveqp",		do_giveqp,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "takeqp",		do_takeqp,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "givexp",		do_givexp,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "takexp",		do_takexp,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "guildadd",	do_guildadd,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "guildremove",	do_guildremove,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "guildrank",	do_guildrank,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "shdrank",	do_shdrank,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "shdname",	do_shdname,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "damname",	do_damname,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "guildeq",	do_guildeq,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "gstat",		do_gstat,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "glist",		do_glist,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "potion",		do_potion,	POS_DEAD,	L7,  LOG_NORMAL, 1 },

    { "force",		do_force,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "mforce",		do_mforce,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "mfind",		do_mfind,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "ofind",		do_ofind,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "poofin",		do_bamfin,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	L8,  LOG_NORMAL, 0 },
    { "mudstat",	do_mudstat,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "mwhere",		do_mwhere,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "owhere",		do_owhere,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "mcount",		do_mcount,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "ocount",		do_ocount,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "sfind",		do_sfind,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "stat",		do_stat,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "vstat",		do_vstat,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "wizinvis",	do_invis,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "home",		do_home,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "sethome", 	do_sethome,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "pfind",		do_pfind,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "exlist",		do_exlist,	POS_DEAD,	L7,  LOG_NORMAL, 1 },
    { "vlist",		do_vlist,	POS_DEAD,	L7,  LOG_NORMAL, 1 },
    { "olist",		do_olist,	POS_DEAD,	L7,  LOG_NORMAL, 1 },
    { "mlist",		do_mlist,	POS_DEAD,	L7,  LOG_NORMAL, 1 },
    { "rlist",		do_rlist,	POS_DEAD,	L7,  LOG_NORMAL, 1 },
    { "toggle",		do_toggle,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "transin",	do_transin,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "transout",	do_transout,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "rename",		do_rename,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "resex",		do_resex,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "newsex",		do_resex,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "wiznet",         do_wiznet,      POS_DEAD,       L8,  LOG_NORMAL, 1 },
    { "immtalk",	do_immtalk,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
    { "immsay",		do_isay,	POS_RECLINING,	L8,  LOG_NORMAL, 1 },
    { "/",		do_immtalk,	POS_DEAD,	L8,  LOG_NORMAL, 0 },
    { "imotd",          do_imotd,       POS_DEAD,       L8,  LOG_NORMAL, 1 },

    /*
     * OLC
     */
    { "edit",           do_olc,         POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "asave",          do_asave,       POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "alist",          do_alist,       POS_DEAD,    L8,  LOG_NORMAL, 1 },
    { "resets",         do_resets,      POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "aedit",		do_ae_olc,	POS_DEAD,    L4,  LOG_NORMAL, 1 },
    { "oedit",		do_oe_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "hedit",		do_he_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "gedit",		do_ge_olc,	POS_DEAD,    L4,  LOG_NORMAL, 1 },
    { "medit",		do_me_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "redit",		do_re_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "oload",		do_ol_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 },
    { "mload",		do_ml_olc,	POS_DEAD,    L7,  LOG_NORMAL, 1 }, 
    { "lists",		do_lists,	POS_DEAD,    ML,  LOG_NORMAL, 1 }, 

    { "setcolor",	do_setcolor,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "loadplayer",	do_loadplayer,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "endplayer",	do_endplayer,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "commandedit",	do_command,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0 }
};

const   struct  guildcmd_type        guildcmd_table       [] =
{
    /* Aes Sedai commands */
    {	"emotion",	do_emotion,		POS_RESTING,
	"aes sedai",	LOG_NORMAL,		1,
	LEVEL_GREATER,	1					},
    { 	"bond",		do_bond,		POS_STANDING,
	"aes sedai",	LOG_NORMAL,		1,
	LEVEL_GREATER,	1					},
    { 	"unbond",	do_unbond,		POS_STANDING,
	"aes sedai",	LOG_NORMAL,		1,
	LEVEL_GREATER,	1					},
    {	"wdtalk",	do_wdtalk,		POS_SLEEPING,
	"aes sedai",	LOG_NORMAL,		0,
	LEVEL_GREATER,	1					},

    /* Ajah commands */
    {	"complaisance",	do_complaisance,	POS_STANDING,  
	"aes sedai",  	LOG_NORMAL,		2,
	LEVEL_GREATER,	1					},
    {	"identify",	do_identify,		POS_STANDING,
	"aes sedai",	LOG_NORMAL,		2,
	LEVEL_GREATER,	1					},

    /* Warder commands */
    {	"emotion",	do_emotion,		POS_RESTING,
	"warder",	LOG_NORMAL,		1,
	LEVEL_GREATER,	1					},
    {	"sweep",	do_sweep,		POS_STANDING,
	"warder",	LOG_NORMAL,		4,
	LEVEL_GREATER,	1					},
    {	"wdtalk",	do_wdtalk,		POS_SLEEPING,
	"warder",	LOG_NORMAL,		0,
	LEVEL_GREATER,	1					},

    /* Darkfriend commands */
    {	"rename",	do_rename,		POS_STANDING,
	"darkfriend",	LOG_ALWAYS,		5,
	LEVEL_GREATER,	1					},
    {	"resex",	do_resex,		POS_STANDING,
	"darkfriend",	LOG_ALWAYS,		5,
	LEVEL_GREATER,	1					},

    /* Seanchan commands */
    {	"leash",	do_leash,		POS_STANDING,
	"seanchan",	LOG_ALWAYS,		7,
	LEVEL_ONLY,	1					},

    /* Whitecloak commands */
    {	"capture",	do_capture,		POS_STANDING,
	"Children of the Light",LOG_ALWAYS,	1,
	LEVEL_GREATER,	1					},

    { 	"",		0,			POS_DEAD,
	"",  		LOG_NORMAL,		0,
	LEVEL_GREATER,	1					}
};

bool is_invis_okay( char *argument )
{
    const char *invis_commands[] =
    {
	"afk", "who", "look", "scan", "tie", "untie", "invert", "cancel",
	"score", NULL
    };
    int i;

    for ( i = 0; invis_commands[i] != NULL; i++ )
    {
	if ( LOWER(argument[0]) == LOWER(invis_commands[i][0])
	&&   !str_prefix(argument, invis_commands[i]) )
	    return TRUE;
    }
    return FALSE;
}

bool is_afk_okay( char *argument )
{
    const char *afk_commands[] =
    {
	"afk", "who", "look", "scan", "score", NULL
    };
    int i;

    for ( i = 0; afk_commands[i] != NULL; i++ )
    {
	if ( LOWER(argument[0]) == LOWER(afk_commands[i][0])
	&&   !str_prefix(argument, afk_commands[i]) )
	    return TRUE;
    }
    return FALSE;
}

bool is_action_okay( char *argument )
{
    const char *action_commands[] =
    {
	"north", "east", "south", "west", "up", "down", "buy", "channel",
	"cast", "invert", "get", "hit", "kill", "practice", "rest",
	"sit", "recline", "stand", "wield", "second", "push", "drag",
	"enter", "donate", "treat", "compare", "consider", "examine",
	"track", "wolfsummon", "teach", "brew", "create", "search",
	"close", "drop", "fill", "empty", "give", "heal", "hold", "list",
	"lock", "open", "knock", "pick", "put", "quaff", "apply",
	"swallow", "remove", "sell", "take", "unlock", "wear", "use",
	"forge", "repair", "fix", "mount", "dismount", "lore",
	"appraise", "assist", "backstab", "bash", "bs", "berserk",
	"dirt", "disarm", "flee", "kick", "murde", "murder", "rescue",
	"trip", "stalk", "gain", "forget", "hide", "sleep", "sneak",
	"steal", "train", "wake", "disguise", "bond", "unbond",
	"complaisance", "identify", "sweep", "leash", "capture",
	NULL
    };
    int i;

    for ( i = 0; action_commands[i] != NULL; i++ )
    {
	if ( LOWER(argument[0]) == LOWER(action_commands[i][0])
	&&   !str_prefix(argument, action_commands[i]) )
	    return TRUE;
    }
    return FALSE;
}


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;
    bool guild_cmd;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    if ( ch->desc )
	ch->desc->idle = 0;

    append_file(ch, COMMAND_FILE, argument );

    /*
     * No hiding.
     */
    if ( (IS_AFFECTED( ch, AFF_INVISIBLE )
    ||	  is_affected( ch, gsn_invis )
    ||	  is_affected( ch, gsn_mass_invis ))
    &&    !is_invis_okay(argument) )
    {
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
    }

    if ( IS_AFFECTED( ch, AFF_HIDE )
    &&   !is_invis_okay(argument) )
    {
	REMOVE_BIT( ch->affected_by, AFF_HIDE );
	ch->hide = NULL;
	ch->hide_type = HIDE_NONE;
    }

    if ( IS_SET(ch->comm, COMM_AFK)
    &&   !is_afk_okay(argument) )
	do_afk( ch, "" );

    if ( ch->action_timer > 0
    &&   is_action_okay(argument) )
	break_con( ch );	

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    guild_cmd = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   cmd_table[cmd].level <= trust )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( found )
    {
    	/*
    	 * Log and snoop.
    	 */
	if ( cmd_table[cmd].log == LOG_NEVER )
	    strcpy( logline, "" );

    	if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    	||   fLogAll
    	||   cmd_table[cmd].log == LOG_ALWAYS )
    	{
	    sprintf( log_buf, "Log %s: %s", ch->name, logline );
	    wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	    log_string( log_buf );
    	}

    	if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    	{
	    write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	    write_to_buffer( ch->desc->snoop_by, logline, 0 );
	    write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    	}
    }
    else
    {
    	for ( cmd = 0; guildcmd_table[cmd].name[0] != '\0'; cmd++ )
    	{
	    if ( command[0] == guildcmd_table[cmd].name[0]
	    &&   !str_prefix(command, guildcmd_table[cmd].name)
	    &&   guild_lookup(guildcmd_table[cmd].guild) == ch->guild )
	    {
		if ( IS_NPC(ch) )
		{
		    guild_cmd = TRUE;
		    found = TRUE;
		    break;
		}

		if ( guildcmd_table[cmd].restrict == LEVEL_ONLY 
		&&   GET_RANK(ch,1) != guildcmd_table[cmd].rank )
		    continue;

		if ( guildcmd_table[cmd].restrict == LEVEL_LESS
		&&   GET_RANK(ch,1) > guildcmd_table[cmd].rank )
		    continue;

		if ( guildcmd_table[cmd].restrict == LEVEL_GREATER 
		&&   GET_RANK(ch,1) < guildcmd_table[cmd].rank )
		    continue;

		guild_cmd = TRUE;
		found = TRUE;
		break;
	    }
    	}

	if ( found )
	{
	    if ( guildcmd_table[cmd].log == LOG_NEVER )
	    	strcpy( logline, "" );

    	    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    	    ||   fLogAll
    	    ||   guildcmd_table[cmd].log == LOG_ALWAYS )
    	    {
	    	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	    	log_string( log_buf );
    	    }

    	    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    	    {
	    	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	    	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	    	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    	    }
	}
    }
	
    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < (!guild_cmd ? cmd_table[cmd].position : guildcmd_table[cmd].position) )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RECLINING:
	    send_to_char( "Not while lying down.\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	case POS_MOUNTED:
	    send_to_char( "From the back of a mount?\n\r", ch );
	    return;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    if ( !guild_cmd )
	(*cmd_table[cmd].do_fun) ( ch, argument );
    else
	(*guildcmd_table[cmd].do_fun) ( ch, argument );
    tail_chain( );
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL
	&&   IS_SENTIENT(victim) )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
        &&   cmd_table[cmd].show)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

