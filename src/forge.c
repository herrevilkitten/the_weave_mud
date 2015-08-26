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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

CHAR_DATA *find_keeper( CHAR_DATA *ch );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_say           );
 
void do_forge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    OBJ_DATA *new_obj;
    int Make = -1;
    int forge = 0;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Forge what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
        return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        act( "$n tells you 'You don't have that item'.",
            keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    one_argument( argument, arg );
 
    for ( forge = 0; forge_table[forge].name != NULL; forge++)
    {
	if (LOWER(arg[0]) == LOWER(forge_table[forge].name[0])
	    &&  !str_prefix(arg, forge_table[forge].name))
	    Make = forge;
    }

    if ( Make < 0
         || (Make >= 0 && Make <= 6 && !IS_SET(keeper->act, ACT_FORGE_WEAPON) )
	 || (Make >= 7 && Make <=16 && !IS_SET(keeper->act, ACT_FORGE_ARMOR) ) )
    {
        act( "$n tells you 'I don't make that sort of thing'.",
            keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    sprintf( buf, "$n grabs $p and begins working on a %s %s.",
	material_name( obj->material ),
	forge_table[Make].long_name );
    act( buf, keeper, obj, NULL, TO_ROOM );

    new_obj = create_object( get_obj_index( OBJ_VNUM_FORGE_ITEM ), 0);

    if ( Make >=11 && Make <= 14 )
    {
	sprintf( buf, "some brand new %s %s",
	    material_name( obj->material ),
	    forge_table[Make].long_name );
	free_string( new_obj->short_descr );
	new_obj->short_descr = str_dup( buf );

	sprintf( buf, "Some brand new %s %s rests here.",
            material_name( obj->material ),
            forge_table[Make].long_name );
	free_string( new_obj->description );
	new_obj->description = str_dup( buf );
    }
    else
    {
	sprintf( buf, "a brand new %s %s",
	    material_name( obj->material ),
	    forge_table[Make].long_name );
	free_string( new_obj->short_descr );
	new_obj->short_descr = str_dup( buf );

	sprintf( buf, "A brand new %s %s rests here.",
            material_name( obj->material ),
            forge_table[Make].long_name );
	free_string( new_obj->description );
	new_obj->description = str_dup( buf );
    }

    sprintf( buf, "%s",
	forge_table[Make].name );
    free_string( new_obj->name );
    new_obj->name = str_dup( buf );

    new_obj->value[0] = forge_table[Make].v0;
    new_obj->value[1] = forge_table[Make].v1;
    new_obj->value[2] = forge_table[Make].v2;
    new_obj->value[3] = forge_table[Make].v3;

    SET_BIT(new_obj->wear_flags, ITEM_TAKE);
    if ( Make >=0 && Make <= 6)
    {
	new_obj->item_type = ITEM_WEAPON;
	SET_BIT(new_obj->wear_flags, ITEM_WIELD);
    }
    else
    {
	new_obj->item_type = ITEM_ARMOR;
	switch (Make)
	{
	    case 7:
		SET_BIT(new_obj->wear_flags, ITEM_WEAR_FINGER);
		break;
            case 8:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_NECK);
                break;
            case 9:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_BODY);
                break;
            case 10:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_HEAD);
                break;
            case 11:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_LEGS);
                break;
            case 12:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_FEET);
                break;
            case 13:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_HANDS);
                break;
            case 14:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_ARMS);
                break;
            case 15:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_WRIST);
                break;
            case 16:
                SET_BIT(new_obj->wear_flags, ITEM_WEAR_SHIELD);
                break;
	}
    }

    new_obj->material = obj->material;
    new_obj->cost = ITEM_FORGE_COST;
    new_obj->weight = forge_table[Make].weight;

    extract_obj( obj );
    new_obj->timer = number_range(50,100);
    obj_to_char( new_obj, keeper );
    return;
}

void do_fix( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    int cost;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Fix what?\n\r", ch );
        return;
    }

    if ( (keeper = find_keeper( ch )) == NULL )
	return;

    if ( !IS_SET(keeper->act, ACT_FIXER) )
    {
	send_to_char( "No one here to repair your item.\n\r", ch );
        return;
    }

    if ( (obj = get_obj_carry( ch, arg )) == NULL )
    {
        act( "$n tells you 'You don't have that item'.",
            keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( obj->condition > 100 )
    {
	act( "$n tells you 'Nothing wrong with this.'", keeper, NULL, ch,
	    TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( obj->condition <= 0 )
    {
	act( "$n tells you 'Damages like this are hard.  This will cost more.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	cost = (100 - obj->condition) * 95;
    }
    else
	cost = (100 - obj->condition) * 65;

    if ( cost < 0 )
    {
	sprintf( buf, "$n tells you 'Something isn't right here.'" );
	act( buf, keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( ch->gold < cost )
    {
	sprintf( buf, "$n tells you 'You need %d gold to pay for this'",
	    cost );
	act( buf, keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    sprintf( buf, "$n tells you, 'This will cost %d gold coins.'",
	cost );
    act( buf, keeper, NULL, ch, TO_VICT );
    ch->reply = keeper;
    act( "$n quickly and efficiently repair$% $p.", keeper, obj, NULL,
	TO_ALL );
    ch->gold -= cost;

    obj->condition = 100;
    return;
}

void create_armor( CHAR_DATA *ch, OBJ_DATA *mat1, OBJ_DATA *mat2,
		   int sn, char *argument )
{
    int skill, make, i, material, min, max, value;
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *armor;
    int chance;
    const char  *       create_type[] =
    {
	"body", "head", "arms", "legs", "hands", "wrists", "neck", "feet",
	"shield",
	NULL
    };

    for ( i = 0; create_type[i] != NULL; i++ )
    {
	if ( UPPER(create_type[i][0]) == UPPER(argument[0])
	&&   !str_prefix(argument, create_type[i]) )
	    break;
    }
    if ( create_type[i] == NULL )
    {
	send_to_char( "You cannot make armor for this location.\n\r", ch );
	return;
    }
    make = i;
    material = mat1->material;
    min = break_table[material].armor_min;
    max = break_table[material].armor_max;

    act( "$n begin$% working with some raw materials.", ch, NULL, NULL,
	TO_ALL );
    WAIT_STATE(ch, 6);

    chance = 33;
    if ( IS_TALENTED(ch, tn_craftsmanship) )
	chance *= 2;

    extract_obj( mat1 );
    if ( !check_skill(ch, sn, chance, TRUE) )
    {
	send_to_char( "You fail to create anything, but stop before all of your material is used up.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    extract_obj( mat2 );
    if ( !check_skill(ch, sn, chance + 17, TRUE) )
    {
	send_to_char( "You fail to create anything, but use up all of your material.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    skill = get_skill(ch, sn);
    max   = UMAX( min + 1, number_range(min, max) * skill / 100 );
    skill = skill * 5 / 8;
    value = min + UMAX( 1, (max - min) * skill / 75 );
    armor = create_object( get_obj_index(4298), 1 );

    armor->item_type = ITEM_ARMOR;
    armor->material  = material;
    free_string( armor->name		);
    free_string( armor->short_descr	);
    free_string( armor->description	);

    for ( i = 0; i < 7; i++ )
	armor->value[i] = value;
    armor->value[3] += break_table[material].fire_mod;
    armor->value[4] += break_table[material].cold_mod;
    armor->value[5] += break_table[material].lightning_mod;
    armor->value[6] /= 2;
    switch( make )
    {
	default:
	    bug( "create_armor: invalid armor type.", 0 );
	    extract_obj( armor );
	    return;
	case 0:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_BODY;
	    armor->value[7]	= ITEM_WEAR_BODY;
	    sprintf( buf, "armor %s", material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a suit of armor made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A suit of armor made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 1:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_HEAD;
	    armor->value[7]	= ITEM_WEAR_HEAD;
	    sprintf( buf, "helmet %s", material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a helmet made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A helmet made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 2:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_ARMS;
	    armor->value[7]	= ITEM_WEAR_ARMS;
	    sprintf( buf, "arms guards armguards %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a set of arm guards made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A set of arm guards made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 3:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_LEGS;
	    armor->value[7]	= ITEM_WEAR_LEGS;
	    sprintf( buf, "legs leggings %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a set of leggings made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A set of leggings made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 4:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_HANDS;
	    armor->value[7]	= ITEM_WEAR_HANDS;
	    if ( material == MAT_CLOTH
	    ||   material == MAT_SILK
	    ||   material == MAT_VELVET
	    ||   material == MAT_SATIN )
	    {
		sprintf( buf, "gloves %s",
		    material_name(material) );
		armor->name		= str_dup( buf );
		sprintf( buf, "a set of gloves made of %s",
		    material_name(material) );
		armor->short_descr	= str_dup( buf );
		sprintf( buf, "A set of gloves made of %s sits here.",
		    material_name(material) );
		armor->description	= str_dup( buf );
	    }
	    else
	    {
		sprintf( buf, "gauntlets %s",
		    material_name(material) );
		armor->name		= str_dup( buf );
		sprintf( buf, "a set of gauntlets made of %s",
		    material_name(material) );
		armor->short_descr	= str_dup( buf );
		sprintf( buf, "A set of gauntlets made of %s sits here.",
		    material_name(material) );
		armor->description	= str_dup( buf );
	    }
	    break;
	case 5:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_WRIST;
	    armor->value[7]	= ITEM_WEAR_WRIST;
	    sprintf( buf, "bracers %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a bracer made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A bracer made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 6:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_NECK;
	    armor->value[7]	= ITEM_WEAR_NECK;
	    sprintf( buf, "neck neckguard %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a neckguard made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A neckguard made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 7:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_FEET;
	    armor->value[7]	= ITEM_WEAR_FEET;
	    sprintf( buf, "boots %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "some boots made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "Some boots made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 8:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_SHIELD;
	    armor->value[7]	= 0;
	    sprintf( buf, "shield %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a shield made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A shield made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
    }
    if ( (skill = get_skill(ch, sn)) >= 75 )
    {
	AFFECT_DATA af;
	affect_from_index( armor );

	af.type			= 0;
	af.strength		= skill;
	af.duration		= -1;
	af.location		= APPLY_AC;
	af.modifier		= number_range( 1, skill - 75 );
	af.bitvector		= 0;
	af.bitvector_2		= 0;
	af.owner		= NULL;
	af.flags		= AFFECT_NOTCHANNEL;
	affect_to_obj( armor, &af );
    }
    act( "$n hold$% up $p, smiling.", ch, armor, NULL, TO_ALL );
    obj_to_char( armor, ch );
    return;
}


void create_weapons( CHAR_DATA *ch, OBJ_DATA *mat1, OBJ_DATA *mat2,
		   int sn, char *argument )
{
    int skill, make, i, material, min, max;
    char buf[MAX_STRING_LENGTH];
    int chance;
    OBJ_DATA *obj;
    const char  *       create_type[] =
    {
	"sword", "dagger", "spear", "mace", "axe", "flail",
	"whip", "polearm", "staff",
	NULL
    };

    for ( i = 0; create_type[i] != NULL; i++ )
    {
	if ( UPPER(create_type[i][0]) == UPPER(argument[0])
	&&   !str_prefix(argument, create_type[i]) )
	    break;
    }
    if ( create_type[i] == NULL )
    {
	send_to_char( "You've never heard of that kind of weapon.\n\r", ch );
	return;
    }
    make = i;
    material = mat1->material;
    min = 1;
    max = 3;

    if ( (material == MAT_LEATHER
    ||    material == MAT_SOFT_LEATHER
    ||    material == MAT_SILK
    ||    material == MAT_SATIN
    ||    material == MAT_VELVET)
    &&   make != 6 )
    {
	send_to_char( "You cannot make that weapon out of that material.\n\r", ch );
	return;
    }

    act( "$n begin$% working with some raw materials.", ch, NULL, NULL,
	TO_ALL );
    WAIT_STATE(ch, 6);

    chance = 33;
    if ( IS_TALENTED(ch, tn_craftsmanship) )
	chance *= 2;

    extract_obj( mat1 );
    if ( !check_skill(ch, sn, chance, TRUE) )
    {
	send_to_char( "You fail to create anything, but stop before all of your material is used up.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    extract_obj( mat2 );
    if ( !check_skill(ch, sn, chance + 17, TRUE) )
    {
	send_to_char( "You fail to create anything, but use up all of your material.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    obj = create_object( get_obj_index(4298), 1 );

    obj->item_type = ITEM_WEAPON;
    obj->material  = material;
    free_string( obj->name		);
    free_string( obj->short_descr	);
    free_string( obj->description	);

    switch( make )
    {
	default:
	    bug( "create_weapons: invalid weapon type.", 0 );
	    extract_obj( obj );
	    return;
	case 0:
	    obj->value[0]	= WEAPON_SWORD;

	    obj->value[2]	= 10;
	    obj->value[3]	= 3;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "sword %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a sword made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A sword made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 1:
	    obj->value[0]	= WEAPON_DAGGER;

	    obj->value[2]	= 3;
	    obj->value[3]	= 11;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "dagger %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a dagger made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A dagger made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 2:
	    obj->value[0]	= WEAPON_SPEAR;

	    obj->value[2]	= 6;
	    obj->value[3]	= 11;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "spear %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a spear with a tip made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A spear with a tip made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 3:
	    obj->value[0]	= WEAPON_MACE;

	    obj->value[2]	= 6;
	    obj->value[3]	= 8;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "mace %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a mace made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A mace made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 4:
	    obj->value[0]	= WEAPON_AXE;

	    obj->value[2]	= 12;
	    obj->value[3]	= 25;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "axe %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "an axe made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "An axe made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 5:
	    obj->value[0]	= WEAPON_FLAIL;

	    obj->value[2]	= 8;
	    obj->value[3]	= 8;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "flail %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a flail made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A flail made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 6:
	    obj->value[0]	= WEAPON_WHIP;

	    obj->value[2]	= 4;
	    obj->value[3]	= 4;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    if ( material == MAT_LEATHER
	    ||   material == MAT_SOFT_LEATHER
	    ||   material == MAT_SILK
	    ||   material == MAT_CLOTH
	    ||   material == MAT_VELVET
	    ||   material == MAT_SATIN )
	    {
		sprintf( buf, "whip %s", material_name(material) );
		obj->name		= str_dup( buf );
		sprintf( buf, "a whip made of %s",
		    material_name(material) );
		obj->short_descr	= str_dup( buf );
		sprintf( buf, "A whip made of %s sits here.",
		    material_name(material) );
		obj->description	= str_dup( buf );
	    }
	    else
	    {
		sprintf( buf, "whip %s", material_name(material) );
		obj->name		= str_dup( buf );
		sprintf( buf, "a whip made of %s chain links",
		    material_name(material) );
		obj->short_descr	= str_dup( buf );
		sprintf( buf, "A whip made of %s chain links sits here.",
		    material_name(material) );
		obj->description	= str_dup( buf );
	    }
	    break;
	case 7:
	    obj->value[0]	= WEAPON_POLEARM;

	    obj->value[2]	= 16;
	    obj->value[3]	= 8;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "polearm %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a polearm made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A polearm made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
	case 8:
	    obj->value[0]	= WEAPON_STAFF;

	    obj->value[2]	= 7;
	    obj->value[3]	= 7;
	    obj->value[4]	= 0;
	    obj->wear_flags	= ITEM_TAKE|ITEM_WIELD;
	    sprintf( buf, "staff %s", material_name(material) );
	    obj->name		= str_dup( buf );
	    sprintf( buf, "a staff made of %s",
		material_name(material) );
	    obj->short_descr	= str_dup( buf );
	    sprintf( buf, "A staff made of %s sits here.",
		material_name(material) );
	    obj->description	= str_dup( buf );
	    break;
    }
    skill = get_skill(ch, sn);
    max   = UMAX(min, max * skill / 100 );
    skill = skill * 5 / 8;
    obj->value[1] = UMAX( 1, (max - min) * skill / 75 ); 

    if ( (skill = get_skill(ch, sn)) >= 75 )
    {
	obj->value[5] = number_range(1, skill - 75 ) * 4 / 7;
	obj->value[6] = number_range(1, skill - 75 ) * 4 / 7;
    }
    act( "$n hold$% up $p, smiling.", ch, obj, NULL, TO_ALL );
    obj_to_char( obj, ch );
    return;
}


void create_jewelry( CHAR_DATA *ch, OBJ_DATA *mat1, OBJ_DATA *mat2,
		   int sn, char *argument )
{
    int skill, make, i, material;
    char buf[MAX_STRING_LENGTH];
    int chance;
    OBJ_DATA *armor;
    const char  *       create_type[] =
    {
	"head", "fingers", "wrists", "neck",
	NULL
    };


    for ( i = 0; create_type[i] != NULL; i++ )
    {
	if ( UPPER(create_type[i][0]) == UPPER(argument[0])
	&&   !str_prefix(argument, create_type[i]) )
	    break;
    }
    if ( create_type[i] == NULL )
    {
	send_to_char( "You cannot make jewelry for this location.\n\r", ch );
	return;
    }
    make = i;
    material = mat1->material;

    act( "$n begin$% working with some raw materials.", ch, NULL, NULL,
	TO_ALL );
    WAIT_STATE(ch, 6);

    chance = 33;
    if ( IS_TALENTED(ch, tn_craftsmanship) )
	chance *= 2;

    extract_obj( mat1 );
    if ( !check_skill(ch, sn, chance, TRUE) )
    {
	send_to_char( "You fail to create anything, but stop before all of your material is used up.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    extract_obj( mat2 );
    if ( !check_skill(ch, sn, chance + 17, TRUE) )
    {
	send_to_char( "You fail to create anything, but use up all of your material.\n\r",ch );
	act( "$n fails in $s work and curses.", ch, NULL, NULL,
	    TO_ROOM );
	return;
    }
    skill = get_skill(ch, sn);
    skill = skill * 5 / 8;
    armor = create_object( get_obj_index(4298), 1 );

    armor->item_type = ITEM_JEWELRY;
    armor->material  = material;
    free_string( armor->name		);
    free_string( armor->short_descr	);
    free_string( armor->description	);

    switch( make )
    {
	default:
	    bug( "create_jewelry: invalid jewelry type.", 0 );
	    extract_obj( armor );
	    return;
	case 0:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_HEAD;
	    sprintf( buf, "tiara %s", material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a tiara made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A tiara made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 1:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_FINGER;
	    armor->value[7]	= ITEM_WEAR_FINGER;
	    sprintf( buf, "ring %s", material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a ring made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A ring made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 2:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_WRIST;
	    armor->value[7]	= ITEM_WEAR_WRIST;
	    sprintf( buf, "bracelet %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a bracelet made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A bracelet made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
	case 3:
	    armor->wear_flags	= ITEM_TAKE|ITEM_WEAR_NECK;
	    armor->value[7]	= ITEM_WEAR_NECK;
	    sprintf( buf, "necklace %s",
		material_name(material) );
	    armor->name		= str_dup( buf );
	    sprintf( buf, "a necklace made of %s",
		material_name(material) );
	    armor->short_descr	= str_dup( buf );
	    sprintf( buf, "A necklace made of %s sits here.",
		material_name(material) );
	    armor->description	= str_dup( buf );
	    break;
    }
    act( "$n hold$% up $p, smiling.", ch, armor, NULL, TO_ALL );
    obj_to_char( armor, ch );
    return;
}



void do_create( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *mat1, *mat2;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int sn, material, vnum, count, i;
    const char  *       create_type[] =
    {
	"armor", "weapons", "jewelry", NULL
    };

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "What do you want to create with what?\n\r", ch );
	return;
    }

    material = material_lookup( arg1 );
    vnum = break_table[material].vnum;
    if ( vnum == 0 )
    {
	send_to_char( "You cannot make anything out of this.\n\r", ch );
	return;
    }
    count = 0;

    mat1 = NULL;
    mat2 = NULL;

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == vnum 
	&&   obj->material == material     )
	{
	    if ( count == 0 )
	    {
		mat1 = obj;
		count++;
	    }
	    else if ( count == 1 )
	    {
		mat2 = obj;
		count++;
		break;
	    }
	}
    }

    if ( count < 2
    ||   mat1 == NULL
    ||   mat2 == NULL )
    {
	send_to_char( "You need at least two units of the resource in order to make something.\n\r", ch );
	return;
    }

    sn = *break_table[material].pgsn;
    if ( sn < 1 || get_skill(ch, sn) < 1 )
    {
	send_to_char( "You don't have this skill practiced.\n\r", ch );
	return;
    }

    for ( i = 0; create_type[i] != NULL; i++ )
    {
	if ( UPPER(create_type[i][0]) == UPPER(arg2[0])
	&&   !str_prefix(create_type[i], arg2) )
	    break;
    }
    if ( create_type[i] == NULL )
    {
	send_to_char( "You cannot make this.\n\r", ch );
	return;
    }
    switch( i )
    {
	default:
	    bug( "Invalid creation type.", 0 );
	    break;
	case 0:
	    create_armor( ch, mat1, mat2, sn, argument );
	    break;
	case 1:
	    create_weapons( ch, mat1, mat2, sn, argument );
	    break;
	case 2:
	    create_jewelry( ch, mat1, mat2, sn, argument );
	    break;
    }
    return;
}


void do_repair( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *mat;
    char arg[MAX_INPUT_LENGTH];
    int sn, material, vnum, chance;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you want to repair?\n\r", ch );
	return;
    }

    if ( (obj = get_obj_carry(ch, arg)) == NULL )
    {
	send_to_char( "You are not carrying that.\n\r", ch );
	return;
    }

    if ( obj->condition >= 100 )
    {
	send_to_char( "It is not broken.\n\r", ch );
	return;
    }

    material = obj->material;
    chance = 100;

    if ( IS_SET(obj->extra_flags, ITEM_RUINED) )
    {
	mat = NULL;
	vnum = break_table[material].vnum;
    	sn = *break_table[material].pgsn;

	if ( sn < 1 || break_table[material].pgsn == NULL )
	{
	    send_to_char( "This material cannot be repaired.\n\r", ch );
	    return;
	}
    
	if ( get_skill(ch, sn) < 1 )
	{
	    send_to_char_new( ch, "You need to learn about %s first.\n\r",
		skill_table[sn].name );
	    return;
	}

	chance = get_skill(ch, sn);
	if ( vnum == 0 )
	{
	    send_to_char( "It is beyond repair.\n\r", ch );
	    return;
	}
	for ( obj = ch->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->pIndexData->vnum == vnum 
	    &&   obj->material == material     )
	    {
		mat = obj;
		break;
	    }
	}

	if ( mat == NULL )
	{
	    send_to_char_new( ch, "You need %s to repair this.",
		get_obj_index(vnum)->short_descr );
	    return;
	}
	act( "You use $p in order to fix $P.", ch, mat, obj, TO_CHAR );
	extract_obj( mat );
	chance = chance / 2;
    }
    if ( IS_SET(obj->extra_flags, ITEM_BENT) )
	chance = chance * 2 / 3;

    if ( check_skill(ch, gsn_repairing, chance, TRUE) )
    {
	int repair;
	repair = get_skill(ch, gsn_repairing) / 8
	       + number_range(1, chance);
	obj->condition += repair;
	if ( obj->condition >= 0 )
	{
	    REMOVE_BIT(obj->extra_flags, ITEM_BENT);
	    REMOVE_BIT(obj->extra_flags, ITEM_RUINED);
	}
	act( "$p seems to be in better shape.", ch, obj, NULL, TO_CHAR );
	return;
    }
    send_to_char( "You could not repair it.\n\r", ch );
    return;
}

