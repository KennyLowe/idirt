
#include <stdlib.h>
#include "kernel.h"
#include "oflags.h"
#include "eflags.h"
#include "pflags.h"
#include "lflags.h"
#include "sendsys.h"
#include "verbs.h"
#include "mobiles.h"
#include "objects.h"
#include "locations.h"
#include "objsys.h"
#include "spell.h"
#include "spells.h"
#include "bprintf.h"
#include "parse.h"
#include "fight.h"
#include "mobile.h"
#include "sflags.h"

void
spellscom (void)
{
  struct SPELL *spell;
  char dam[4], type[9];

  bprintf ("You know the following spells:\n");
  bprintf ("&+B---------------------------------------------------------------\n");
  bprintf ("&+WCommand    Spell                Mana  Damage  Chance  Type\n");
  bprintf ("&+B---------------------------------------------------------------\n");
  for (spell = spell_table; spell->verb >= 0; spell++) {
    if (etstflg (mynum, spell->flag)) {
      sprintf (dam, "%d", spell->damage);
      sprintf (type, "%s", spell->type == ATTACK ? "Attack" :
	       spell->type == HEAL ? "Heal" :
	       spell->duration ? "Duration" : "Special");
      bprintf ("&*%-10.10s %-20.20s %4d  %6.6s  %6d  %-9.9s\n",
	       spell->verbname, spell->name, spell->mana,
	       spell->damage ? dam : "---", spell->chance, type);
    }
  }
  bprintf ("&+B---------------------------------------------------------------\n");
}

void
spellcom (int spell)
{
  int victim = brkword () < 0 ? pfighting (mynum) : fpbn (wordbuf);

  if (!is_in_game (victim))
    bprintf ("Who?\n");
  else
    cast_spell (mynum, victim, spell);
}

Boolean
cast_spell (int caster, int victim, int verb)
{
  int chance, damage = 0;
  int factor = 1;

  struct SPELL *spell;
  Boolean miss = False;

  /* Find spell */
  for (spell = spell_table; spell->verb != verb; spell++) {
    if (spell->verb < 0)
      return False;		/* No such spell */
  }

  if (victim == pfighting (caster) && spell->type != ATTACK) {
    bprintf ("Who?\n");
    return False;
  }
  /* Check to see if spell can be casted across a distance */
  if (plev (caster) < LVL_WIZARD && ploc (caster) != ploc (victim)) {
    if (spell->type != SPECIAL) {
      sendf (caster, "%s is not here.\n", pname (victim));
      return False;
    }
  }
  /* Check if player knows the spell */
  if (!etstflg (caster, spell->flag)) {
    bprintf ("You have not learned this spell yet.\n");
    return False;
  }
  /* Check if the spell can be cast */
  if (plev (caster) < LVL_WIZARD) {
    if (caster < max_players && pmagic (caster) < spell->mana) {
      sendf (caster, "You are to weak to cast this spell.\n");
      return False;
    }
    if (testpeace (caster) && spell->type == ATTACK) {
      sendf (caster, "No, that's violent!\n");
      return False;
    }
    if (ltstflg (ploc (victim), LFL_NO_MAGIC)) {
      sendf (caster, "Something about this location has drained your mana.\n");
      return False;
    }
    chance = (caster >= max_players) ? 60 + spell->chance : spell->chance;

    if (carries_obj_type (caster, OBJ_BLIZZARD_POWERSTONE) > -1)
      ++chance;
    if (carries_obj_type (caster, OBJ_BLIZZARD_POWERSTONE1) > -1)
      ++chance;
    if (carries_obj_type (caster, OBJ_BLIZZARD_POWERSTONE2) > -1)
      ++chance;
    if (carries_obj_type (caster, OBJ_FANTASY_MANA) > -1)
      chance += 2;

    if (caster < max_players && randperc () > chance + (4 * plev (caster))) {
      miss = True;
    }
    if (ptstflg (victim, PFL_NOHASSLE) != 0 && spell->type == ATTACK) {
      sendf (caster, "Something prevents you from casting your spell.\n");
      return False;
    }
  }
  /* Attack type spell */
  if (spell->type == ATTACK) {
    if (caster == victim) {
      sendf (victim, "You're supposed to be killing others, not yourself.\n");
      return False;
    }
    if (!do_okay (caster, victim, PFL_NOHASSLE)) {
      sendf (caster, "Something prevents you from casting your spell.\n");
      return False;
    }
    if (caster < max_players)
      setpmagic (caster, pmagic (caster) - spell->mana);

    send_magic_msg (caster, caster, victim, spell->spell_msg, CASTER);
    send_magic_msg (victim, caster, victim, spell->spell_msg_vic, VICTIM);
    send_magic_msg (ploc (caster), caster, victim, spell->to_room, ROOM);

    if (miss) {
      send_magic_msg (caster, caster, victim, spell->to_casterm, TOCAST);
      send_magic_msg (victim, caster, victim, spell->to_victimm, TOVIC);
      send_magic_msg (ploc (caster), caster, victim, spell->to_othersm, TOROOM);
      return False;
    }
    if (plev (caster) < LVL_WIZARD &&
#ifdef LOCMIN_TALON
	carries_obj_type (victim, OBJ_TALON_TALONSHIELD) > -1 &&
#endif
	carries_obj_type (victim, OBJ_CATACOMB_SHIELD) > -1) {

      sendf (caster, "The spell is absorbed by %s shield!\n", his_or_her (victim));

      send_msg (ploc (caster), 0, LVL_MIN, LVL_MAX, caster, victim,
		"\001p%s\003 casts a spell on \001p%s\003. It is absorbed by %s shield!\n",
		pname (caster), pname (victim), his_or_her (victim));

      sendf (victim, "%s casts a spell on you. It is absorbed by your shield!\n",
	     see_name (victim, caster));

      return False;
    }
    if (spell->immune_flag != NOFLAG && etstflg (victim, spell->immune_flag)) {
      send_magic_msg (caster, caster, victim, spell->to_casteri, TOCAST);
      send_magic_msg (victim, caster, victim, spell->to_victimi, TOVIC);
      send_magic_msg (ploc (caster), caster, victim, spell->to_othersi, TOROOM);
      return False;
    }
    if (spell->fear_flag != NOFLAG && etstflg (victim, spell->fear_flag))
      factor += 4;

    if (check_object (caster, spell->obj_flag))
      factor += 2;

    send_magic_msg (caster, caster, victim, spell->to_caster, TOCAST);
    send_magic_msg (victim, caster, victim, spell->to_victim, TOVIC);
    send_magic_msg (ploc (caster), caster, victim, spell->to_others, TOROOM);

    if (plev (caster) < LVL_WIZARD)
      damage = spell->damage * factor;
    else
      damage = pstr (victim) + 1;

    wound_player (caster, victim, damage, 0);
  }
  /* Heal type spell */
  if (spell->type == HEAL) {
    if (caster < max_players)
      setpmagic (caster, pmagic (caster) - spell->mana);

    send_magic_msg (caster, caster, victim, spell->spell_msg, CASTER);
    send_magic_msg (victim, caster, victim, spell->spell_msg_vic, VICTIM);
    send_magic_msg (ploc (caster), caster, victim, spell->to_room, ROOM);

    if (miss) {
      send_magic_msg (caster, caster, victim, spell->to_casterm, TOCAST);
      send_magic_msg (victim, caster, victim, spell->to_victimm, TOVIC);
      send_magic_msg (ploc (caster), caster, victim, spell->to_othersm, TOROOM);
      return False;
    }
    if (check_object (caster, spell->obj_flag))
      factor += 2;

    send_magic_msg (caster, caster, victim, spell->to_caster, TOCAST);
    send_magic_msg (victim, caster, victim, spell->to_victim, TOVIC);
    send_magic_msg (ploc (caster), caster, victim, spell->to_others, TOROOM);

    damage = spell->damage * factor;
    setpstr (victim, pstr (victim) + damage);
  }
  /* Special Spell */
  if (spell->type == SPECIAL) {

    /* Check all Special spells that are to the caster's room */
    if (spell->verb == VERB_VTOUCH || spell->verb == VERB_LIT ||
	spell->verb == VERB_DAMAGE || spell->verb == VERB_ARMOR ||
	spell->verb == VERB_BLUR) {
      if (ploc (caster) != ploc (victim)) {
	sendf (caster, "%s is not here.\n", pname (victim));
      }
    }
    if (!do_okay (caster, victim, PFL_NOHASSLE)) {
      sendf (caster, "Something prevents you from casting your spell.\n");
      return False;
    }
    if (spell->verb == VERB_VTOUCH) {
      if (caster == victim) {
	sendf (victim, "It's pointless to suck your own life away!\n");
	return False;
      }
    }
    if (check_duration (victim, spell->verb)) {
      sendf (victim, "%s are already under the effects of that spell.\n",
	     caster == victim ? "You" : "They");
      return False;
    }
    if (caster < max_players)
      setpmagic (caster, pmagic (caster) - spell->mana);

    send_magic_msg (caster, caster, victim, spell->spell_msg, CASTER);
    send_magic_msg (victim, caster, victim, spell->spell_msg_vic, VICTIM);
    send_magic_msg (ploc (caster), caster, victim, spell->to_room, ROOM);

    if (miss) {
      send_magic_msg (caster, caster, victim, spell->to_casterm, TOCAST);
      send_magic_msg (victim, caster, victim, spell->to_victimm, TOVIC);
      send_magic_msg (ploc (caster), caster, victim, spell->to_othersm, TOROOM);
      return False;
    }
    if (plev (mynum) < LVL_WIZARD) {
      if (spell->immune_flag != NOFLAG && etstflg (victim, spell->immune_flag)) {
	send_magic_msg (caster, caster, victim, spell->to_casteri, TOCAST);
	send_magic_msg (victim, caster, victim, spell->to_victimi, TOVIC);
	send_magic_msg (ploc (caster), caster, victim, spell->to_othersi, TOROOM);
	return False;
      }
      if (spell->fear_flag != NOFLAG && etstflg (victim, spell->fear_flag))
	factor += 4;

      if (check_object (caster, spell->obj_flag))
	factor += 2;

      send_magic_msg (caster, caster, victim, spell->to_caster, TOCAST);
      send_magic_msg (victim, caster, victim, spell->to_victim, TOVIC);
      send_magic_msg (ploc (caster), caster, victim, spell->to_others, TOROOM);

      damage = spell->damage * factor;
    }
    /* Vampiric Touch */
    if (spell->verb == VERB_VTOUCH) {
      pscore (caster) = pscore (caster) + 100 * damage;
      sendf (caster, "&*You gain &+W%d &*experience points!\n", damage * 100);
      wound_player (caster, victim, damage, 0);
    }
    /* Light */
    if (spell->verb == VERB_LIT) {
      if (add_duration (victim, VERB_LIT, spell->duration * factor, 0))
	ssetflg (victim, SFL_LIT);
    }
    /* Damage */
    if (spell->verb == VERB_DAMAGE) {
      add_duration (victim, VERB_DAMAGE, spell->duration * factor, 0);
    }
    /* Armor */
    if (spell->verb == VERB_ARMOR) {
      add_duration (victim, VERB_ARMOR, spell->duration * factor, 0);
    }
    /* Blur */
    if (spell->verb == VERB_BLUR) {
      add_duration (victim, VERB_BLUR, spell->duration, 0);
    }
  }
  return True;
}

int
mob_cast_spell (int caster)
{
  struct SPELL *spell;

  for (spell = spell_table; spell->verb >= 0; spell++) {
    if (etstflg (caster, spell->flag) == 0)
      continue;
    if (randperc () < spell->chance)
      return spell->verb;
  }
  return -1;
}

Boolean
check_object (int plr, int flag)
{
  int i;

  if (flag == NOFLAG)
    return False;

  for (i = 0; i < pnumobs (plr); i++)
    if (iscarrby (pobj_nr (i, plr), flag) && otstbit (i, flag))
      return True;

  return False;
}

void
send_magic_msg (int dest, int caster, int victim, char *msg, int type)
{
  char xx[120];
  char c[30], v[30];

  if (msg == NULL)
    return;

  sprintf (c, "\001p%s\003", pname (caster));
  sprintf (v, "\001p%s\003", pname (victim));

  if (type == CASTER && caster == victim)
    sprintf (v, "yourself");
  if (type == ROOM && caster == victim)
    sprintf (v, "%sself", psex (caster) ? "her" : "him");
  if (type == VICTIM && caster == victim)
    return;
  if (type == TOCAST && caster == victim)
    return;

  if (type == ROOM || type == TOROOM)
    send_msg (dest, 0, LVL_MIN, LVL_MAX, caster, victim,
	      "%s\n", make_magic_msg (xx, msg, c, v));
  else
    send_msg (dest, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "%s\n", make_magic_msg (xx, msg, c, v));
}

char *
make_magic_msg (char *b, char *s, char *c, char *v)
{
  char *p, *q, *r;

  for (p = b, q = s; *q != 0;) {
    if (*q != '%')
      *p++ = *q++;
    else {
      switch (*++q) {
      case 'c':		/* Caster */
	if (c == NULL)
	  return NULL;
	for (r = c; *r != 0;)
	  *p++ = *r++;
	break;
      case 'v':		/* Victim */
	if (v == NULL)
	  return NULL;
	for (r = v; *r != 0;)
	  *p++ = *r++;
	break;
      case 0:
	--q;
	break;
      default:
	*p++ = *q;
      }
      ++q;
    }
  }
  if (p[-1] == '\n')
    --p;
  *p = 0;
  return b;
}

/************************************************************************
 * Spell Duration Handler Functions					*
 ************************************************************************/

/* This function wipes out all of the duration data.
 */
void
wipe_duration (int plr)
{
  SPELL_DURATION *check = plr < max_players ? players[plr].duration : NULL;
  SPELL_DURATION *nextptr = NULL;

  while (check != NULL) {
    if (nextptr != NULL)
      check = nextptr;
    if (check->spell == VERB_LIT)
      sclrflg (plr, SFL_LIT);
    nextptr = check->next;
    FREE (check);
  }
}

/* This function sends text to a user that his/her spell has ended.
 */
void
duration_end (int plr, int spell, int tmp)
{
  if (spell == VERB_LIT) {
    sendf (plr, "Your lit spell expires.\n");
    sclrflg (plr, SFL_LIT);
  }
  if (spell == VERB_DAMAGE) {
    sendf (plr, "You feel your strength spell wearing off.\n");
    setpdam (plr, tmp);
  }
  if (spell == VERB_ARMOR) {
    sendf (plr, "You feel your protection spell wearing off.\n");
    setparmor (plr, tmp);
  }
  if (spell == VERB_BLUR) {
    sendf (plr, "You seem to be more visible.\n");
  }
}

/* This function adds a spell to a player's duration linklist.
 */
Boolean
add_duration (int plr, int spell, int duration, int tmp)
{
  /* Convert minutes to seconds. Since the MUD interrupts
   * once every 2 seconds though, we multiply by 30.
   */
  duration = duration * 30;

  /* Check to see if the spell is already being used */
  if (!check_duration (plr, spell)) {
    push_duration (plr, spell, duration, tmp);
    return True;
  } else {
    return False;
  }
}

/* This function checks if a spell is currently being used.
 */
Boolean
check_duration (int plr, int spell)
{
  SPELL_DURATION *check = plr < max_players ? players[plr].duration : NULL;

  if (check == NULL)
    return False;

  /* Check to see if the spell is already being used */
  while (check != NULL) {
    if (check->spell == spell)
      return True;
    check = check->next;
  }
  return False;
}

/* This function handles going through the linklist to decrement the
 * duration and check the spells.
 */
void
handle_duration (int plr)
{
  SPELL_DURATION *check = plr < max_players ? players[plr].duration : NULL;
  SPELL_DURATION *prev = NULL;

  /* Nothing is in the linklist, do not process. */
  if (check == NULL)
    return;

  /* Decrement all spells, disable if it ran out, and free memory */
  while (check != NULL) {
    if (!check->duration--) {
      duration_end (plr, check->spell, check->tmp);
      if (prev == NULL) {
	players[plr].duration = check->next;
	FREE (check);
      } else {
	prev->next = check->next;
	FREE (check);
	check = prev;
      }
    }
    prev = check;
    check = check->next;
  }
}

/* This function pushes the linklist to add another duration pointer
 * into it.
 */
void
push_duration (int plr, int spell, int duration, int tmp)
{
  SPELL_DURATION *new;
  SPELL_DURATION *curr = players[plr].duration;

  new = NEW (SPELL_DURATION, 1);
  new->spell = spell;
  new->duration = duration;
  new->tmp = tmp;

  if (curr != NULL) {
    new->next = curr->next;
    curr->next = new;
  } else {
    new->next = curr;
    curr = new;
  }
}
