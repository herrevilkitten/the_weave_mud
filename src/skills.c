/* Code specifically for the new skill system */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "mem.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

int	group_cost( CHAR_DATA *ch, int gn );
bool	has_flows( CHAR_DATA *ch, int sn );
int	weave_use( int sn );
int	number_avail( CHAR_DATA *ch, int gn );
int	count_group( int gn );
int	stat_bonus( CHAR_DATA *ch, sh_int stats );


/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0' || !str_prefix(arg,"list"))
    {
	int col;
	char buf2[MAX_STRING_LENGTH * 2];
	int count;
	
	col = 0;
	buf2[0] = '\0';

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
	    if (group_table[gn].name == NULL)
		break;

	    if (!ch->pcdata->group_known[gn]
	    &&  (group_table[gn].rating[ch->class] > 0
	    ||  (can_channel(ch,1) && group_table[gn].rating[ch->class] < -1)))
	    {
		sprintf(buf,"%-18.18s %-5d Trains\n\r",
		    group_table[gn].name,
		    group_cost(ch, gn) );
		strcat(buf2,buf);
	    }
	}
	
	strcat(buf2,"\n\r");
	page_to_char( buf2, ch );
	col = 0;
	buf2[0] = '\0';

        sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
                     "skill","cost","skill","cost","skill","cost");
        strcat(buf2, buf);

	count = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( ch->pcdata->learned[sn] > 0
	    &&   skill_table[sn].spell_fun == spell_null )
		count++;
	}
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;

	    if ( can_channel(ch, 1) && skill_table[sn].non_chan )
		continue;
	    if ( skill_table[sn].prerequisite != NULL )
	    {
		int pre;

		pre = skill_lookup( skill_table[sn].prerequisite );

		if ( pre != -1 && ch->pcdata->learned[pre] < 70 )
		    continue;
	    }

            if ( ch->pcdata->learned[sn] < 0
            &&   skill_table[sn].rating[ch->class] > 0
	    &&   skill_table[sn].spell_fun == spell_null
	    &&   ch->level >= skill_table[sn].skill_level[ch->class] )
            {
                sprintf(buf,"%-18.18s %-5d ",
                    skill_table[sn].name,
		    skill_table[sn].rating[ch->class] + count/10);
                strcat(buf2, buf);
                if (++col % 3 == 0)
                    strcat(buf2, "\n\r");
            }
        }
        if (col % 3 != 0)
            strcat(buf2, "\n\r");
	page_to_char( buf2, ch );
	return;
    }

    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 10)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your practice to training",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 10;
	ch->train +=1 ;
	return;
    }

    if (!str_prefix(arg,"learn"))
    {
	if (ch->train < 1)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N helps you apply your training to practice.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice += 10;
	ch->train -= 1 ;
	return;
    }

    if (!str_prefix(arg,"experience"))
    {
	if ( IS_NPC(ch) )
	    return;

	if (ch->pcdata->qp < 5)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N explains how your quests can give experience.",
		ch,NULL,trainer,TO_CHAR);
	gain_exp( ch, 1000 );
	ch->pcdata->qp -= 5;
	return;
    }

    if (!str_prefix(arg,"points"))
    {
	int xp_to_level;

	if (ch->train < 2)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->points <= 40)
	{
	    act("$N tells you 'There would be no point in that.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N trains you, and you feel more at ease with your skills.",
	    ch,NULL,trainer,TO_CHAR);

	xp_to_level = (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp;

	ch->train -= 2;
	ch->pcdata->points -= 1;
	ch->exp = exp_per_level(ch,ch->pcdata->points) * (ch->level + 1) - xp_to_level;
	return;
    }

    /* else add a group/skill */

    gn = group_lookup(argument);
    if (gn > 0)
    {
	if (ch->pcdata->group_known[gn])
	{
	    act("$N tells you 'You already know that group!'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if ((group_table[gn].rating[ch->class] <= 0 &&
	     group_table[gn].rating[ch->class] >= -1) ||
	    (!can_channel(ch,1) && group_table[gn].rating[ch->class] < -1))
	{
	    act("$N tells you 'That group is beyond your powers.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if ( group_table[gn].rating[ch->class] < -1 )
	{
	    if ( ch->train < group_cost(ch, gn) )
	    {
		act( "$N tells you, 'You are not ready for that group.'",
		    ch, NULL, trainer, TO_CHAR );
		return;
	    }
	}
	else
	{
	    if (ch->train < group_table[gn].rating[ch->class])
	    {
		act("$N tells you 'You are not yet ready for that group.'",
		    ch,NULL,trainer,TO_CHAR);
		return;
	    }
	}
	/* add the group */
	gn_add(ch,gn);
	act("$N trains you in the art of $t",
	    ch,group_table[gn].name,trainer,TO_CHAR);
	ch->train -= group_cost( ch, gn );
	return;
    }

    sn = skill_lookup(argument);
    if (sn > -1)
    {
	int count, gsn;

	if (skill_table[sn].spell_fun != spell_null)
	{
	    act("$N tells you 'You must learn the full group.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}
	 
	if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	{
	    act( "$N tells you 'You have no use for this.'",
		ch, NULL, trainer, TO_CHAR );
	    return;
	}
	if ( skill_table[sn].prerequisite != NULL )
	{
	    int pre;

	    pre = skill_lookup( skill_table[sn].prerequisite );
	    if ( pre != -1 && ch->pcdata->learned[pre] < 70 )
	    {
		act( "$N tells you 'You need to learn more about $t first.'",
		   ch, skill_table[pre].name, trainer, TO_CHAR );
		return;
	    }
	}

        if (ch->pcdata->learned[sn] >= 0)
        {
            act("$N tells you 'You already know that skill!'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }

        if (skill_table[sn].rating[ch->class] <= 0)
        {
            act("$N tells you 'That skill is beyond your powers.'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }

	count = 0;
	for ( gsn = 0; gsn < MAX_SKILL; gsn++ )
	{
	    if ( ch->pcdata->learned[gsn] > 0
	    &&   skill_table[gsn].spell_fun == spell_null )
		count++;
	} 
 
        if (ch->train < skill_table[sn].rating[ch->class] + count/10
	||  ch->level < skill_table[sn].skill_level[ch->class] )
        {
            act("$N tells you 'You are not yet ready for that skill.'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
        /* add the skill */
	ch->pcdata->learned[sn] = 0;
        act("$N trains you in the art of $t",
            ch,skill_table[sn].name,trainer,TO_CHAR);
        ch->train -= skill_table[sn].rating[ch->class] + count/15;
        return;
    }

    act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}


void do_forget( CHAR_DATA *ch, char *argument )
{
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( (sn = group_lookup(argument)) != -1 )
    {
	int cost;

	if ( !ch->pcdata->group_known[sn] )
	{
	    send_to_char( "You have not gained this group.\n\r", ch );
	    return;
	}

	cost = URANGE( 1, group_cost(ch, sn) * 2 / 3, group_cost(ch, sn) - 1 );
	send_to_char_new( ch, "You forget everything about %s, regaining %d training sessions.\n\r", group_table[sn].name, cost );
	gn_remove( ch, sn );
	ch->train += cost;
	
    }

    if ( (sn = skill_lookup(argument)) != -1 )
    {
	int cost;

	if ( ch->pcdata->learned[sn] < 0 )
	{
	    send_to_char( "You have not gained that skill.\n\r", ch );
	    return;
	}

	if ( skill_table[sn].spell_fun != spell_null )
	{
	    send_to_char( "You must forget the whole group.\n\r", ch );
	    return;
	}

	if ( skill_table[sn].rating[ch->class] < 0 )
	{
	    send_to_char( "You may not forget that skill.\n\r", ch );
	    return;
	}

	cost = URANGE( 1, skill_table[sn].rating[ch->class] * 2 / 3, skill_table[sn].rating[ch->class] - 1 );
	send_to_char_new( ch, "You forget everything about %s, regaining %d training sessions.\n\r", skill_table[sn].name, cost );
	ch->pcdata->learned[sn]	= -2;
	ch->pcdata->usage[sn]	= 0;
	ch->train += cost;
	return;
    }

    send_to_char( "That is not a skill\n\r", ch );
    return;
}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
  char spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
  char spell_columns[LEVEL_HERO];
  int sn, lev;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH ];
  CHAR_DATA *victim;
  BUFFER *buffer;
  char flow_str[20];
  int flows;

  victim = ch;

  if( IS_IMMORTAL( ch) && *argument)
    if( ( victim = get_char_world( ch, argument)) == NULL)
      {
	send_to_char( "They are not here.\r\n", ch);
	return;
      }

  if( IS_NPC(ch) || IS_NPC( victim))
    return;

  /* initilize data */
  for (lev = 0; lev < LEVEL_HERO; lev++)
    {
      spell_columns[lev] = 0;
      spell_list[lev][0] = '\0';
    }
 
  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if( skill_table[ sn].name == NULL)
        break;

      if ( skill_table[ sn].skill_level[ victim->class] < LEVEL_HERO 
      &&   skill_table[ sn].spell_fun != spell_null
      &&   victim->pcdata->learned[ sn] > -1 )
	{
	  found = TRUE;
	  flows = weave_use( sn );
	  flow_str[0] = '\0';
	  if ( flows & 1 )
	    strcat( flow_str, "E" );
	  if ( flows & 2 )
	    strcat( flow_str, "A" );
	  if ( flows & 4 )
	    strcat( flow_str, "F" );
	  if ( flows & 8 )
	    strcat( flow_str, "W" );
	  if ( flows & 16 )
	    strcat( flow_str, "S" );
	    
	  lev = skill_table[ sn].skill_level[ victim->class];
	  if( victim->level < lev)
	    sprintf( buf, "%-18.18s  %5s n/a ", skill_table[sn].name,
		flow_str);
	  else
	    {
	      sprintf( buf, "%-18.18s  %5s %3d ", skill_table[ sn].name,
		       flow_str, stamina_cost( victim, sn) );
	    }
	
	  if( spell_list[ lev][ 0] == '\0')
	    sprintf( spell_list[lev], "\n\rLevel %3d: %s", lev, buf);
	  else /* append */
	    {
	      if( ++spell_columns[ lev] % 2 == 0)
		strcat( spell_list[ lev], "\n\r           ");
	      strcat( spell_list [lev], buf);
	    }
	}
    }

  /* return results */

  if( victim == ch)
    strcpy( buf, "You know");
  else
    sprintf( buf, "%s knows", victim->name);

  if (!found)
    {
      sprintf( buf, "%s no spells at all.\r\n", buf);
      send_to_char( buf, ch);
      return;
    }
    
  buffer = new_buf();
  sprintf( buf, "%s the following spells:\r\n", buf);
  add_buf(buffer, buf);
  for( lev = 0; lev < LEVEL_HERO; lev++)
    if( spell_list[ lev][ 0] != '\0')
      add_buf( buffer, spell_list[ lev]);
  
  add_buf( buffer, "\n\r" );
  page_to_char( buf_string(buffer), ch);
  free_buf(buffer);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    victim = ch;
    if ( IS_NPC(ch) )
	return;

  if (IS_IMMORTAL(ch) && *argument)
    {
      if( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	}
    }

  if (IS_NPC(ch) || IS_NPC( victim))
    return;
 
  /* initilize data */
  for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }
 
  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL )
        break;
      
 
      if (skill_table[sn].skill_level[victim->class] < LEVEL_HERO &&
	  skill_table[sn].spell_fun == spell_null &&
	  skill_table[sn].skill_level[victim->class] >= 0 &&
	  victim->pcdata->learned[sn] > -1 )	/* Stuff you can eventually */
	{						/* Learn */
	  if ( is_channel_skill(sn)
	  &&   victim->pcdata->learned[sn] == 0 )
	    continue;

	  found = TRUE;
	  lev = skill_table[sn].skill_level[victim->class];

	  if (victim->level < lev)
	    sprintf(buf,"%-18.18s n/a      ", skill_table[sn].name);
	  else
	    sprintf(buf,"%-18.18s %3d%%      ",skill_table[sn].name,
		    SKILL(victim,sn) );
 
	  if (skill_list[lev][0] == '\0')
	    sprintf(skill_list[lev],"\n\rLevel %2d: %s",lev,buf);
	  else /* append */
	    {
	      if ( ++skill_columns[lev] % 2 == 0)
		strcat(skill_list[lev],"\n\r          ");
	      strcat(skill_list[lev],buf);
	    }
	}
    }
 
  /* return results */
 
  if( victim == ch)
    strcpy( buf, "You know");
  else
    sprintf( buf, "%s knows", victim->name);

  if (!found)
    {
      sprintf( buf, "%s no skills at all.\r\n", buf);
      send_to_char( buf, ch);
      return;
    }
 
  buffer = new_buf();
  sprintf( buf, "%s the following skill(s):\r\n", buf);
  add_buf( buffer, buf );
  for (lev = 0; lev < LEVEL_HERO; lev++)
    if (skill_list[lev][0] != '\0')
      add_buf(buffer, skill_list[lev]);
  add_buf(buffer, "\n\r");
  page_to_char( buf_string(buffer), ch);
  free_buf(buffer);
}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    char buf2[MAX_STRING_LENGTH *2];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;
    buf2[0] = '\0';

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","group","cp","group","cp","group","cp");
    strcat(buf2,buf);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	if (group_table[gn].name == NULL)
	    break;

	if ( !ch->gen_data->group_chosen[gn]
	&&   !ch->pcdata->group_known[gn]
	&&   group_cost(ch, gn) > 0
	&&   can_channel(ch, 1) )
	{
	    sprintf(buf,"%-18.18s %-5d CPs\n\r",
		group_table[gn].name,
		group_cost(ch, gn) );
	    strcat(buf2,buf);
	}
    }
    strcat(buf2,"\n\r");
    page_to_char( buf2, ch );
    col = 0;
    buf2[0] = '\0';
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

	if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	    continue;
	if ( skill_table[sn].prerequisite != NULL )
	{
	    int pre;

	    pre = skill_lookup( skill_table[sn].prerequisite );
	    if ( pre != -1 )
		continue;
	}

        if (!ch->gen_data->skill_chosen[sn] 
	&&  ch->pcdata->learned[sn] == -1
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].skill_level[ch->class] <= 20
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            strcat(buf2,buf);
            if (++col % 3 == 0)
                strcat(buf2,"\n\r");
        }
    }
    if ( col % 3 != 0 )
        strcat( buf2,"\n\r" );
    strcat(buf2,"\n\r");
    page_to_char( buf2, ch );
    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    char buf2[MAX_STRING_LENGTH * 2];
    int gn,sn,col;
 
    if (IS_NPC(ch))
        return;
 
    col = 0;
    buf2[0] = '\0';
 
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;
 
        if (ch->gen_data->group_chosen[gn] 
	&&  group_cost(ch, gn) > 0
	&&  can_channel(ch, 1) )
        {
	    sprintf(buf,"%-18.18s %-5d CPs\n\r",
		group_table[gn].name,
		group_cost(ch, gn) );
	    strcat(buf2,buf);
        }
    }
    strcat(buf2,"\n\r");
    page_to_char( buf2, ch );
    col = 0;
    buf2[0] = '\0';
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
    strcat(buf2,buf);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18.18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            strcat(buf2,buf);
            if (++col % 3 == 0)
                strcat(buf2,"\n\r");
        }
    }
    if ( col % 3 != 0 )
        strcat( buf2,"\n\r" );
    strcat(buf2,"\n\r");
    page_to_char( buf2, ch );
 
    sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

int exp_per_level(CHAR_DATA *ch, int points)
{
    int expl,inc;

    if (IS_NPC(ch))
	return 2000; 

    expl = 2000;
    inc = 500;

    if (points < 40)
	return 2000;

    /* processing */
    points -= 40;

    while (points > 9)
    {
	expl += inc;
        points -= 10;
        if (points > 9)
	{
	    expl += inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;  

    return expl;	
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn,sn,i;
 
    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_help(ch,"group help");
	    return TRUE;
	}

        do_help(ch,argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("You already know that group!\n\r",ch);
		return TRUE;
	    }

	    if ( ch->pcdata->points +
		 group_cost(ch, gn) > 80 )
	    {
		send_to_char( "You cannot have more 80 creation points.\n\r", ch );
		return TRUE;
	    }

	    if (group_cost(ch, gn) <= 0
	    ||  !can_channel(ch, 1) )
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }

	    sprintf(buf,"%s group added\n\r",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    ch->gen_data->points_chosen += group_cost( ch, gn );
	    gn_add(ch,gn);
	    ch->pcdata->points += group_cost( ch, gn );
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > -1)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	    {
		send_to_char( "Channelers may not gain this skill.\n\r", ch );
		return TRUE;
	    }
	    if ( skill_table[sn].prerequisite != NULL )
	    {
		int pre;

		pre = skill_lookup( skill_table[sn].prerequisite );
		if ( pre != -1 )
		{
		    send_to_char_new( ch, "You must adequately learn %s first.\n\r",
			skill_table[pre].name );
		    return TRUE;
		}
	    }

	    if (skill_table[sn].rating[ch->class] < 1
	    ||  skill_table[sn].skill_level[ch->class] > 20
	    ||  skill_table[sn].spell_fun != spell_null)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }
	    if ( ch->pcdata->points +
		 skill_table[sn].rating[ch->class] > 80 )
	    {
		send_to_char( "You cannot have more 80 creation points.\n\r", ch );
		return TRUE;
	    }
	    sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->points += skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("Group dropped.\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    ch->gen_data->points_chosen -= group_cost( ch, gn );
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    ch->pcdata->points -= group_cost( ch, gn );
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = -1;
	    ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_help(ch,"premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {

	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_groups(ch,argument);
	return TRUE;
    }

    return FALSE;
}
	    
	


        

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    if (argument[0] == '\0')
    {   /* show all groups */
	
	for (gn = 0; gn < MAX_GROUP; gn++)
        {
	    if (group_table[gn].name == NULL)
		break;
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"%-20.20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"Creation points: %d  ",ch->pcdata->points);
	send_to_char(buf,ch);
	send_to_char_new( ch, "Experience per level: %d\n\r",
	    exp_per_level(ch, ch->pcdata->points) );
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
        for (gn = 0; gn < MAX_GROUP; gn++)
        {
            if (group_table[gn].name == NULL)
                break;
	    sprintf(buf,"%-20.20s ",group_table[gn].name);
            send_to_char(buf,ch);
	    if (++col % 3 == 0)
            	send_to_char("\n\r",ch);
        }
        if ( col % 3 != 0 ) 
            send_to_char( "\n\r", ch );
	return;
     }
	
     
     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("No group of that name exist.\n\r",ch);
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	return;
     }

     for (sn = 0; sn < MAX_IN_GROUP; sn++)
     {
	if (group_table[gn].spells[sn] == NULL)
	    break;
	sprintf(buf,"%-20s ",group_table[gn].spells[sn]);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	    send_to_char("\n\r",ch);
     }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
}

bool is_maxed( CHAR_DATA *ch, int sn )
{
    int max;

    if ( sn == gsn_earth )
    {
	if ( ch->pcdata->learned[sn] >= ch->channel_max[0] )
	{
	    ch->pcdata->learned[sn] = ch->channel_max[0];
	    return TRUE;
	}
    }
    if ( sn == gsn_air )
    {
	if ( ch->pcdata->learned[sn] >= ch->channel_max[1] )
	{
	    ch->pcdata->learned[sn] = ch->channel_max[1];
	    return TRUE;
	}
    }
    if ( sn == gsn_fire )
    {
	if ( ch->pcdata->learned[sn] >= ch->channel_max[2] )
	{
	    ch->pcdata->learned[sn] = ch->channel_max[2];
	    return TRUE;
	}
    }
    if ( sn == gsn_water )
    {
	if ( ch->pcdata->learned[sn] >= ch->channel_max[3] )
	{
	    ch->pcdata->learned[sn] = ch->channel_max[3];
	    return TRUE;
	}
    }
    if ( sn == gsn_spirit )
    {
	if ( ch->pcdata->learned[sn] >= ch->channel_max[4] )
	{
	    ch->pcdata->learned[sn] = ch->channel_max[4];
	    return TRUE;
	}
    }

    max = UMIN( 100, 90 + stat_bonus(ch, skill_table[sn].stat) );
    if ( ch->pcdata->learned[sn] >= max )
	return TRUE;

    return FALSE;
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance, i;
    char buf[100];

    if (IS_NPC(ch))
	return;

    for ( i = 0; i < MAX_STATS; i++ )
    {
	if ( IS_SET(skill_table[sn].stat, 1 << i)
	&&   number_bits(2) == 0
	&&   success == TRUE
	&&   ch->pcdata->stat_use[i] < 100 )
	    ch->pcdata->stat_use[i]++;
    }

    if (ch->level < skill_table[sn].skill_level[ch->class]
    ||  skill_table[sn].rating[ch->class] == 0
    ||  ch->pcdata->learned[sn] < 1
    ||  ch->pcdata->learned[sn] >= 100
    ||  is_maxed(ch, sn) )
	return;  /* skill is not known or known too well */ 

    /* check to see if the character has a chance to learn */
    chance = 10 * ( int_app[get_curr_stat(ch,STAT_INT)].learn +
		    (ch->pcdata->learn_skill * 2) );
    chance /= (		multiplier
		*	abs(skill_table[sn].rating[ch->class]) 
		*	10);

    if (number_range(1,550) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	
    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
	    sprintf(buf,"You have become better at %s!\n\r",
		    skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn]++;
	    gain_exp(ch,2 * abs(skill_table[sn].rating[ch->class]));
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    sprintf(buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }
    if ( is_channel_skill(sn) )
    {
	if ( sn == gsn_earth )
	{
	    if ( ch->pcdata->learned[sn] >= ch->channel_max[0] )
	    {
		send_to_char( "You have reached your potential in earth.\n\r", ch );
		ch->pcdata->learned[sn] = ch->channel_max[0];
	    }
	}
	if ( sn == gsn_air )
	{
	    if ( ch->pcdata->learned[sn] >= ch->channel_max[1] )
	    {
		send_to_char( "You have reached your potential in air.\n\r", ch );
		ch->pcdata->learned[sn] = ch->channel_max[1];
	    }
	}
	if ( sn == gsn_fire )
	{
	    if ( ch->pcdata->learned[sn] >= ch->channel_max[2] )
	    {
		send_to_char( "You have reached your potential in fire.\n\r", ch );
		ch->pcdata->learned[sn] = ch->channel_max[2];
	    }
	}
	if ( sn == gsn_water )
	{
	    if ( ch->pcdata->learned[sn] >= ch->channel_max[3] )
	    {
		send_to_char( "You have reached your potential in water.\n\r", ch );
		ch->pcdata->learned[sn] = ch->channel_max[3];
	    }
	}
	if ( sn == gsn_spirit )
	{
	    if ( ch->pcdata->learned[sn] >= ch->channel_max[4] )
	    {
		send_to_char( "You have reached your potential in spirit.\n\r", ch );
		ch->pcdata->learned[sn] = ch->channel_max[4];
	    }
	}
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;
 
    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;
    
    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn, gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {

	if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	    return;

	if (ch->pcdata->learned[sn] == -1) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 0;
	    if (deduct)
	   	ch->pcdata->points += skill_table[sn].rating[ch->class]; 
	}
	return;
    }
	
    /* now check groups */
    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
		ch->pcdata->points += group_table[gn].rating[ch->class];
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = -1;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}

int group_cost( CHAR_DATA *ch, int gn )
{
    if ( group_table[gn].rating[ch->class] >= 0 )
	return( group_table[gn].rating[ch->class] );
    else if ( group_table[gn].rating[ch->class] <= -2 )
    {
	int cost;
	cost  = group_table[gn].rating[ch->class] * -1;
	cost -= 2;
	cost  = cost * 8 / 3;
	cost  = URANGE( 1, cost, 16 );
	return( cost );
    }
    return( 0 );
}

int count_group( int gn )
{
    int i;

    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
    }

    return i;
}

int number_avail( CHAR_DATA *ch, int gn )
{
    int i, count = 0;

    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	if ( has_flows(ch, skill_lookup(group_table[gn].spells[i])) )
	    count++;
    }
    return count;
}


void remove_skill( CHAR_DATA *ch )
{
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( IS_IMMORTAL(ch) )
	return;

    for ( sn = 0; skill_table[sn].name != NULL; sn++ )
    {
	int guild, rank;

	guild = get_guild_skill( sn );
	rank  = get_skill_rank( sn );

	if ( can_channel(ch, 1) && skill_table[sn].non_chan )
	{
	    ch->pcdata->learned[sn] = -1;
	    ch->pcdata->usage[sn]   = 0;
	}

	/* now do guild skills - this also GIVES skills */
	if ( guild > 0
	&&   (guild != ch->guild
	||    GET_RANK(ch,1) < rank) )
	{
	    ch->pcdata->learned[sn] = -1;
	    ch->pcdata->usage[sn]   = 0;
	}
	if ( guild > 0
	&&   guild == ch->guild
	&&   GET_RANK(ch,1) >= rank
	&&   ch->pcdata->learned[sn] < 70 )
	    ch->pcdata->learned[sn] = 70;
    }

    if ( ch->pcdata->learned[gsn_invert_weave]
    &&   !is_forsaken(ch) )
    {
	ch->pcdata->learned[gsn_invert_weave] = -1;
	ch->pcdata->usage[gsn_invert_weave]   = 0;
    }

    return;
}
