/* this is a listing of all the commands and command related data */

#define	LEVEL_ONLY	0
#define	LEVEL_LESS	1
#define	LEVEL_GREATER	2

#define	RANK_ONLY	A
#define	RANK_LESS	B
#define	RANK_GREATER	C

#define NO_INVIS	D
#define NO_HIDE		E
#define NO_ACTION	F

/*
#define	LOG_ALWAYS	G
#define	LOG_NORMAL	H
#define LOG_NEVER	I
*/

#define SHOW		J

#define COMMAND_ACTIVE	ee

/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    bool              show;
};

struct	guildcmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    char * const	guild;
    sh_int		log;
    int			rank;
    sh_int		restricted;
    bool		show;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];
extern	const	struct	guildcmd_type	guildcmd_table	[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */

DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN(	do_assist	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_autoall	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_bond		);
DECLARE_DO_FUN( do_unbond	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_color	);
DECLARE_DO_FUN( do_combat	);
DECLARE_DO_FUN( do_config	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_combine	);
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN(	do_guild	);
DECLARE_DO_FUN(	do_gdtalk	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_disguise	);
DECLARE_DO_FUN(	do_dismount	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drag		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_empty	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_exlist	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN(	do_lists	);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_mforce	);
DECLARE_DO_FUN( do_forge	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN( do_glist	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_ooc		);
DECLARE_DO_FUN( do_ocount	);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN( do_home		);
DECLARE_DO_FUN( do_hunt		);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_last		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_lore		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_marry	);
DECLARE_DO_FUN( do_mcount	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mlist	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_vstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN( do_mount	);
DECLARE_DO_FUN( do_mtalk	);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN( do_bard		);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN(	do_newname	);
DECLARE_DO_FUN( do_news		);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN( do_nonote	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_noshout	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_notify	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_olist	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_osay		);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN( do_permdelet	);
DECLARE_DO_FUN( do_permdelete	);
DECLARE_DO_FUN( do_pfind	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_push		);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_ic		);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_recline	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rename	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_rlist	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN(	do_second	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN( do_setcolor     );
DECLARE_DO_FUN( do_sethome	);
DECLARE_DO_FUN( do_sfind	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stalk	);
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_string	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_teach	);
DECLARE_DO_FUN(	do_giveqp	);
DECLARE_DO_FUN(	do_takeqp	);
DECLARE_DO_FUN(	do_givexp	);
DECLARE_DO_FUN(	do_takexp	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_think	);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN( do_toggle	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_ungroup	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN(	do_vlist	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_whisper	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN( do_ae_olc	);
DECLARE_DO_FUN( do_oe_olc	);
DECLARE_DO_FUN( do_me_olc	);
DECLARE_DO_FUN( do_he_olc	);
DECLARE_DO_FUN( do_ge_olc	);
DECLARE_DO_FUN( do_re_olc	);
DECLARE_DO_FUN( do_ol_olc	);
DECLARE_DO_FUN( do_ml_olc	);
DECLARE_DO_FUN( do_transin	);
DECLARE_DO_FUN( do_transout	);
DECLARE_DO_FUN( do_dirsay	);
DECLARE_DO_FUN( do_enter	);
DECLARE_DO_FUN( do_dftalk	);
DECLARE_DO_FUN( do_wdtalk	);
DECLARE_DO_FUN( do_guildadd	);
DECLARE_DO_FUN( do_guildremove	);
DECLARE_DO_FUN( do_guildrank	);
DECLARE_DO_FUN( do_guildeq	);
DECLARE_DO_FUN( do_shdrank	);
DECLARE_DO_FUN( do_shdname	);
DECLARE_DO_FUN( do_gstat	);
DECLARE_DO_FUN( do_knock	);
DECLARE_DO_FUN( do_fullreboot	);
DECLARE_DO_FUN( do_alias	);
DECLARE_DO_FUN( do_unalias	);
DECLARE_DO_FUN( do_resex	);
DECLARE_DO_FUN( do_use		);
DECLARE_DO_FUN( do_donate	);
DECLARE_DO_FUN( do_complaisance	);
DECLARE_DO_FUN( do_identify	);
DECLARE_DO_FUN( do_sweep	);
DECLARE_DO_FUN( do_tie		);
DECLARE_DO_FUN( do_untie	);
DECLARE_DO_FUN( do_cancel	);
DECLARE_DO_FUN( do_unread	);
DECLARE_DO_FUN( do_dtell	);
DECLARE_DO_FUN( do_grasp	);
DECLARE_DO_FUN( do_release	);
DECLARE_DO_FUN( do_treat	);
DECLARE_DO_FUN( do_weaves	);
DECLARE_DO_FUN( do_invert	);
DECLARE_DO_FUN( do_wiznet       );
DECLARE_DO_FUN( do_balance      );
DECLARE_DO_FUN( do_deposit      );
DECLARE_DO_FUN( do_withdraw     );
DECLARE_DO_FUN( do_armor	);
DECLARE_DO_FUN( do_repair	);
DECLARE_DO_FUN( do_regain	);
DECLARE_DO_FUN( do_appraise	);
DECLARE_DO_FUN( do_brew		);
DECLARE_DO_FUN( do_search	);
DECLARE_DO_FUN( do_potion	);
DECLARE_DO_FUN( do_tool		);
DECLARE_DO_FUN( do_create	);
DECLARE_DO_FUN( do_leash	);
DECLARE_DO_FUN( do_wolfsummon	);
DECLARE_DO_FUN( do_forget	);
DECLARE_DO_FUN( do_book		);
DECLARE_DO_FUN( do_permban	);
DECLARE_DO_FUN( do_emotion	);
DECLARE_DO_FUN( do_penalty	);
DECLARE_DO_FUN( do_capture	);
DECLARE_DO_FUN( do_replay	);
DECLARE_DO_FUN( do_isay		);
DECLARE_DO_FUN( do_channeling	);
DECLARE_DO_FUN( do_snoops	);
DECLARE_DO_FUN( do_mudstat	);
DECLARE_DO_FUN( do_fix		);
DECLARE_DO_FUN( do_for		);
DECLARE_DO_FUN( do_newbiehelper	);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN( do_doing	);
DECLARE_DO_FUN( do_wearing	);
DECLARE_DO_FUN( do_link		);
DECLARE_DO_FUN( do_snare	);
DECLARE_DO_FUN( do_import	);
DECLARE_DO_FUN( do_unlink	);
DECLARE_DO_FUN( do_toss		);
DECLARE_DO_FUN( do_loadplayer	);
DECLARE_DO_FUN( do_endplayer	);
DECLARE_DO_FUN( do_damname	);
DECLARE_DO_FUN( do_email	);
DECLARE_DO_FUN( do_command	);
DECLARE_DO_FUN( do_application	);
DECLARE_DO_FUN( do_ecast	);

