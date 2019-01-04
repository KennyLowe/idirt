
#include <stdlib.h>
#include "kernel.h"
#include "cflags.h"
#include "oflags.h"
#include "lflags.h"
#include "mflags.h"
#include "sflags.h"
#include "pflags.h"
#include "quests.h"
#include "sendsys.h"
#include "verbs.h"
#include "bprintf.h"
#include "commands.h"
#include "move.h"
#include "mud.h"
#include "rooms.h"
#include "log.h"
#include "clone.h"

#include "mobiles.h"
#include "objects.h"
#include "locations.h"

#include "spell.h"
#include "fight.h"
#include "mobile.h"
#include "objsys.h"

/* Calculate damage done by a player 'pl' carrying item 'it'.
 * If no item, return damage of player. If it is not a weapon, return -1
 * else return damage of player + damage of weapon.
 */
int
dambyitem (int pl, int it)
{
  if (it == -1)
    return pdam (pl);
  return !otstbit (it, OFL_WEAPON) ? -1 : odamage (it) + pdam (pl);
}

/*
 * Command function block to handle wielding of weapons.
 */
void
wieldcom (void)
{
  int a;

  if ((a = ob1) == -1) {
    bprintf ("What's that?\n");
    return;
  } else if (!iscarrby (a, mynum)) {
    bprintf ("You're not carrying the %s!\n", item1);
    set_weapon (mynum, -1);
    return;
  } else if (!otstbit (a, OFL_WEAPON)) {
    bprintf ("It's not a weapon.\n");
    set_weapon (mynum, -1);
    return;
  }
  set_weapon (mynum, a);

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "%s wields the %s.\n", pname (mynum), oname (a));

  bprintf ("You are now wielding the %s.\n", oname (a));
}



/*
 *  The killing and hitting command
 */
static void wreckdoor (void);

void
killcom (void)
{
  if (pfighting (mynum) >= 0) {
    bprintf ("You're already fighting!\n");
    return;
  }
  if (pl1 == -1 && ob1 == -1) {
    bprintf ("Kill who?\n");
    return;
  }
  if (EQ (item1, "door")) {
    if (!ptstflg (mynum, PFL_WRECK)) {
      bprintf ("What are you trying to do?  Wreck the door?\n");
    } else {
      wreckdoor ();
    }
    return;
  }
  if (ob1 != -1) {
    breakitem (ob1);
    return;
  }
  if (pl1 == -1) {
    bprintf ("You can't do that.\n");
    return;
  }
  if (pl1 == mynum) {
    bprintf ("Come on, it'll look better tomorrow...\n");
    return;
  }
  if (ploc (pl1) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return;
  }
  if (testpeace (mynum)) {
    bprintf ("Sorry, it's too peaceful for violence.\n");
    return;
  }
  if (ptstflg (pl1, PFL_NOHASSLE)) {
    bprintf ("Something interferes with your attack.\n");
    return;
  }
  if (pl1 < max_players && !the_world->w_tournament) {
    bprintf ("You cannot attack a player when tournament mode is off!\n");
    return;
  }
  setpfighting (mynum, pl1);

  hit_player (mynum, pl1, (ob2 == -1) ? pwpn (mynum) : ob2);

  /* Send message if mortal attacks mortal in a tournament
   */
  if (pl1 < max_players && plev (mynum) < LVL_WIZARD &&
      plev (pl1) < LVL_WIZARD && the_world->w_tournament)
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+r[&+YTournament: &+W%s &*attacked &+W%s&+r]\n",
	      pname (mynum), pname (pl1));
  return;
}


void
breakitem (int x)
{
  switch (x) {
  case OBJ_VILLAGE_PEBBLE:
    bprintf ("The pebble gets annoyed and goes to philosophize elsewhere.\n");
    setoloc (OBJ_VILLAGE_PEBBLE, LOC_DEAD_DESTROYED, IN_ROOM);
    return;
  case OBJ_BLIZZARD_RESET_STONE:
    sys_reset ();
    break;
  case OBJ_QUARRY_ROCK:
    bprintf ("You smash it apart to reveal a gem inside.\n");
    create (OBJ_QUARRY_GEM);
    setoloc (OBJ_QUARRY_GEM, oloc (OBJ_QUARRY_ROCK), ocarrf (OBJ_QUARRY_ROCK));
    destroy (OBJ_QUARRY_ROCK);
    break;
  case -1:
    bprintf ("What's that?\n");
    break;
  default:
    bprintf ("You can't do that.\n");
    break;
  }
}

static void
wreckdoor (void)
{
  int a;
  char ms[80];

  if ((a = fobna ("door")) == -1)
    bprintf ("Sorry, no doors here.\n");
  else if (state (a) == 0)
    bprintf ("It's already open.\n");
  else {
    setobjstate (a, 0);
    bprintf ("The door flies open!\n");
    sprintf (ms, "%s hits the door...  It flies open!\n", pname (mynum));
    sillycom (ms);
    sprintf (ms,
	 "\001dYou hear the distinctive crunch of %s meeting a door.\n\003",
	     pname (mynum));
    broad (ms);
  }
}

Boolean
testpeace (int player)
{
  return (the_world->w_peace || ltstflg (ploc (player), LFL_PEACEFUL));
}

void
fleecom (void)
{
  if (pfighting (mynum) < 0) {
    dogocom ();
  } else if (carries_obj_type (mynum, OBJ_CASTLE_RUNESWORD) > -1) {
    bprintf ("The sword won't let you!\n");
    return;
  } else {
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s drops things as %s make a frantic attempt to escape.\n",
	      pname (mynum), psex (mynum) ? "she" : "he");

    setpscore (mynum, pscore (mynum) - pscore (mynum) / 33);	/* lose 33% */
    setpfighting (mynum, -1);
    if (cur_player->i_follow >= 0) {
      bprintf ("You stopped following %s.\n", pname (cur_player->i_follow));
      cur_player->i_follow = -1;
    }
    drop_some_objects (mynum);
    dogocom ();
  }
}

void
hit_player (int attacker, int victim, int weapon)
{
  int x, cth, d, w;
  int mob = victim - max_players;


  if ((x = attacker) >= max_players)
    x = victim;

  if (attacker >= numchars || victim >= numchars || attacker < 0 ||
      victim < 0 || testpeace (victim) || ploc (attacker) != ploc (victim)
      || !is_in_game (attacker) || !is_in_game (victim))
    return;

  /* if the attacker is played by a human: */
  if (attacker < max_players || ststflg (attacker, SFL_OCCUPIED) != 0) {
    /* if the attacker is a mortal: */

    if (plev (attacker) < LVL_WIZARD || ststflg (attacker, SFL_OCCUPIED) != 0) {


      /* if the victim is a mobile: */

      if (victim >= max_players) {

	if (mtstflg (victim, MFL_BLIND) != 0 && !ststflg (attacker, SFL_BLIND)) {
	  sendf (attacker, "%s scratches your eyes out! You are blind!\n",
		 pname (victim));
	  ssetflg (attacker, SFL_BLIND);
	}
	if (mob == MOB_CAVE_DRAGON) {
	  if (iswielded (OBJ_ICECAVE_DAGGER) && iscarrby (OBJ_ICECAVE_DAGGER, attacker)) {
	    bprintf ("The Dragon glares at you and prepares to blast you "
		     "out of existence with his\nfiery breath... but the "
		     "icy blade sinks deep into the dragon's hide.\nThe "
		     "Dragon shudders.");
	    setpscore (mynum, pscore (mynum) + 500);
	    wound_player (attacker, victim, pstr (victim) + 1, -1);
	    return;
	  }
	}
	if (mob == MOB_VALLEY_CHICKEN && !psex (victim)) {
	  sendf (attacker,
		 "As you look up you see the sky is somehow different.\n"
	      "The next second you're crushed to death as the sky hits you!"
		 "\n");
	  send_msg (ploc (attacker), 0,
		    pvis (attacker) > 0 ? pvis (attacker) : LVL_MIN, LVL_MAX,
		    attacker, NOBODY,
		    "%s is squashed as %she attacks Chicken Licken.\n",
		    pname (attacker), psex (attacker) ? "s" : "");
	  p_crapup (attacker,
		"\t\tChicken Licken was right!\n", CRAP_SAVE | CRAP_RETURN);
	  return;
	} else if (mob == MOB_CATACOMB_SERAPH) {
	  sendf (attacker,
		 "The Seraph is amused by your foolhardiness.  For your "
	      "impudence, he draws his\nflaming sword and slays you with a "
		 "single blow.\n");
	  send_msg (ploc (attacker), 0,
		    pvis (attacker) > 0 ? pvis (attacker) : LVL_MIN, LVL_MAX,
		    attacker, NOBODY,
		    "%s is sliced in half by the Seraph!\n\003",
		    pname (attacker));
	  p_crapup (attacker, "\t\tNever pick a fight with an angel..\n",
		    CRAP_SAVE | CRAP_RETURN);
	  return;
	} else if (mob == MOB_CATACOMB_BEGGAR) {
	  sendf (attacker,
		 "You drop everything as you are summoned by The Seraph.\n"
	       "The Seraph scowls at you contemptuously.  For your lack of "
		 "compassion, he\n"
		 "draws his flaming sword and slays you with a single "
		 "blow.\n");
	  send_msg (ploc (attacker), 0,
		    pvis (attacker) > 0 ? pvis (attacker) : LVL_MIN, LVL_MAX,
		    attacker, NOBODY,
		    "%s is sliced in half by the Seraph!\n\003",
		    pname (attacker));
	  p_crapup (attacker,
		    "\tRemember the ten commandments: Thou Shalt Not Kill\n",
		    CRAP_SAVE | CRAP_RETURN);
	  return;
	} else if (mob == MOB_SHERWOOD_EMMY) {
	  sendf (attacker, "Emmy cries out and you look around "
		 "nervously. Puff then appears with an\n"
		 "ear splitting bang, shaking her head at "
		 "you. The next thing you know, Puff\n"
		 "pounces on top of you!\n");
	  send_msg (ploc (attacker), 0, pvis (attacker) > 0 ?
		    pvis (attacker) : LVL_MIN, LVL_MAX, attacker,
		    NOBODY, "As %s goes to attack Emmy she cries "
		    "out! Puff then appears and pounces\non top of "
		    "%s!\n", pname (attacker), him_or_her (attacker));
	  p_crapup (attacker, "That's what you get for picking on "
		    "an innocent, sweet, little Emmy!\n",
		    CRAP_SAVE | CRAP_RETURN);
	  return;
	} else if (mob == MOB_SHERWOOD_LUSIE) {
	  sendf (attacker, "Li'l Lusie types something on his computer as "
		 "you go to attack him. After\na few seconds he looks up "
		 "from the screen and grins at you. The world opens\nup "
		 "beneath your feet, and you fall into a bottomless pit!\n");
	  send_msg (ploc (attacker), 0, max (pvis (attacker), LVL_MIN), LVL_MAX,
		    attacker, NOBODY, "As %s goes to attack Li'l Lusie, "
		    "Li'l Lusie goes to work on his\ncomputer. Moments "
		    "later Li'l Lusie looks up and grins at %s.\nThe world "
		    "then opens up and swallows up %s!\n", pname (attacker),
		    pname (attacker));
	  p_crapup (attacker, "\t\tDon't mess with the boy!\n",
		    CRAP_SAVE | CRAP_RETURN);
	  return;
	} else if (mob == MOB_RUINS_ZEPHERE) {
	  sendf (attacker, "The mage waves his arms and you don't feel "
		 "like attacking him anymore.\n");
	  return;
	} else if (mob == MOB_MITHDAN_RED_DRAGON) {
	  sendf (attacker, "The Dragon snarls as you wake him, turns around, "
		 "and swallows you in one gulp!\n");
	  send_msg (ploc (attacker), 0, pvis (attacker) > 0 ? pvis (attacker) :
		 LVL_MIN, LVL_MAX, attacker, NOBODY, "%s pokes the dragon. "
		    "The dragon wakes up, snarls, and eats %s!\n",
		    pname (attacker), him_or_her (attacker));
	  p_crapup (attacker, "\t\tFamous last words: Hey Bill, watch me goose this "
		    "sleeping dragon!", CRAP_SAVE | CRAP_RETURN);
	  return;
	}
      }				/* end if victim is a mobile */
    }				/* end, the attacker is a mortal */
    if (testpeace (attacker))
      return;
    if ((w = weapon) < 0)
      w = pwpn (attacker);
    if (w >= 0 && !iscarrby (w, attacker)) {
      sendf (attacker,
	     "You belatedly realize you don't have the %s,\nand are forced "
	     "to use your hands instead.\n", oname (w));
      w = -1;
    }
    set_weapon (attacker, w);
    if (w != -1 && ozone (w) >= num_const_zon) {
      send_msg (DEST_ALL, 0, LVL_PROPHET, LVL_MAX, NOBODY, NOBODY,
	      "&+B[&+CCheat (Weapon)&*: Zone: &+W%s &*Used by: &+W%s&+B]\n",
		zname (ozone (w)), pname (attacker));
      mudlog ("CHEAT: Weapon from Zone: %s, Used By: %s",
	      zname (ozone (w)), pname (attacker));
      sendf (attacker, "&+WThe weapon crumbles to dust in your hands.\n");
      sendf (attacker, "&+WNext time, don't use a cloned weapon!\n");
      set_weapon (attacker, -1);
      osetbit (w, OFL_DESTROYED);
      setoloc (w, LOC_PIT_PIT, IN_ROOM);
    }
    if (onum (w) == OBJ_CASTLE_RUNESWORD &&
	carries_obj_type (victim, OBJ_TOWER_STAFF) > -1) {
      sendf (attacker, "The Runesword flashes back from its target, "
	     "growling in anger!\n");
      return;
    }
    if (weapon >= 0 && dambyitem (attacker, weapon) < 0) {
      sendf (attacker, "You can't attack %s with a %s!\n",
	     pname (victim), oname (weapon));
      set_weapon (attacker, -1);
      return;
    }
    if (psitting (attacker)) {
      standcom (attacker);
    }
    /* end, attacker is played by a human */

  } else if (weapon < 0 &&
	     carries_obj_type (victim, OBJ_CATACOMB_SHIELD) == -1 &&
#ifdef LOCMIN_TALON
	     !iscarrby (OBJ_TALON_TALONSHIELD, victim) &&
#endif
	     (x = mob_cast_spell (attacker)) >= 0) {

    if (cast_spell (attacker, victim, x))
      return;

  }				/* end, the attacker was a mobile */
  x = randperc ();
  cth = 57 - parmor (victim);
  if (carries_obj_type (attacker, OBJ_CASTLE_HORSESHOE) > -1)
    cth += 5;

  if (victim < max_players) {
    if (check_duration (victim, VERB_BLUR))
      cth -= 5;
  }
  if (player_armor (victim) > MAXARMOR)
    cth -= MAXARMOR;
  else
    cth -= player_armor (victim);

  /* Add 1/2 damage of wielded weapon for mobiles..full for players.. */
  if ((w = weapon) < 0)
    w = pwpn (attacker);
  if (w >= 0 && iscarrby (w, attacker) && iswielded (w) &&
      otstbit (w, OFL_WEAPON)) {
    d = odamage (w);
    if (attacker >= max_players)
      d /= 2;
  } else {
    d = 0;
  }

  d += pdam (attacker);

#ifdef LOCMIN_TALON
  if (attacker == max_players + MOB_TALON_TALON &&
      iswornby (OBJ_TALON_TALONSHIELD, victim)) {
    if (d > 45)
      d -= 45;
    else
      d = 0;
  }
#endif

  d = (x < cth) ? my_random () % (d + 1) : -1;	/* damage done */

  combatmessage (victim, attacker, weapon, d);
  wound_player (attacker, victim, d, -1);
}


/*
 * procedure to wound a player.
 * attacker = player index of the attacker.
 * victim   = player index of the victim.
 * damage   = damage caused by the attacker on the victim.
 * hit_type >= 0  verb that caused the magical hit.
 *          <  0  non-magical hit.
 *
 * Return true if victim died.
 */
Boolean
wound_player (int attacker, int victim, int damage, int hit_type)
{
  int x, q;
  char prompt[100];

  if (victim < 0 || victim >= numchars || attacker < 0 ||
      attacker >= numchars) {
    return False;
  }
  pfighting (victim) = attacker;

  if (damage <= 0)
    return False;

  if (victim >= max_players) {
    setpstr (victim, pstr (victim) - damage);
  } else if (plev (victim) < LVL_WIZARD) {
    setpstr (victim, pstr (victim) - damage);
    if (attacker >= max_players) {
      if (mtstflg (attacker, MFL_DSCORE) != 0) {
	sendf (victim, "You feel weaker as %s's icy touch drains your "
	       "very life force!\n", pname (attacker));
	setpscore (victim, pscore (victim) - 100 * damage);
	if (pscore (victim) <= 0) {
	  setpstr (victim, -1);
	}
      }
    }
    strcpy (prompt, players[victim].prompt);
    if ((strstr (prompt, "%h")) != NULL) {
      strcpy (players[victim].cprompt, build_prompt (victim));
      if (ststflg (victim, SFL_NEWSTYLE))
	sendf (victim, "\r%s", players[victim].cprompt);
    } else {
      sendf (victim, "&+W[&*Your strength is now &+C%d&+W]\n", pstr (victim));
    }

  }
  if (pstr (victim) >= 0) {
    if (attacker < max_players && (plev (victim) > LVL_HERO ||
		  (victim >= max_players && zpermanent (pzone (victim))))) {
      setpscore (attacker, pscore (attacker) + damage * 2);
    }
    return False;
  }
  /* Victim has died */

  pfighting (victim) = -1;

  if (hit_type >= 0 && hit_type != VERB_ZAP) {
    sendf (attacker, "Your last spell did the trick.\n");
  }
  setpstr (victim, -1);
  setpfighting (victim, -1);
  sendf (ploc (victim), "%s has died.\n", pname (victim));
  send_msg (DEST_ALL, MODE_NSFLAG | MS (SFL_NOSLAIN), LVL_WIZARD, LVL_MAX,
	    NOBODY, NOBODY, "&+r[&+W%s &*has been &+Yslain &*by &+W%s&+r]\n",
	    pname (victim), pname (attacker));

  if (the_world->w_tournament) {
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_WIZARD, NOBODY, NOBODY,
	      "&+r[&+YTournament: &+W%s &*has died&+r]", pname (victim));
  }
  dumpstuff (victim, ploc (victim));

  if (victim >= max_players) {
    /* Kill mobile */
    if (attacker < max_players) {
      setpscore (attacker, pscore (attacker) + 10 * pdam (victim));
      setpkilled (attacker, pkilled (attacker) + 1);
    }
    if ((q = find_pretender (victim)) >= 0) {
      sendf (victim, "*** You have died. ***\n");
      unalias (q);
      unpolymorph (q);
    }
    setploc (victim, LOC_DEAD_DEAD);
    if (attacker < max_players) {
      setpscore (victim, attacker);	/* remember who killed the mobile */
      if (ststflg (attacker, SFL_AUTOHEAL)) {
	sendf (attacker, "You feel your health being restored from the dead soul..\n");
	setpstr (attacker, maxstrength (attacker));
	setpmagic (attacker, maxmagic (attacker));
      }
    } else {
      setpscore (victim, -1);	/* Mobile killing mobile */
    }

    if (pnum (victim) == MOB_RUINS_MARGOYLE) {
      sendf (attacker, "As the monster collapses over, his horn falls off.\n");
      setoloc (OBJ_RUINS_HORN, ploc (attacker), IN_ROOM);
    }
    /* FANTASY */
#ifdef LOCMIN_FANTASY
    if (pnum (victim) == MOB_FANTASY_SILVERMONSTER) {
      sendf (attacker, "The wall to the west suddenly collapses as the "
	     "monster lets out its\nlast breath..\n");
      setobjstate (OBJ_FANTASY_CAVEWALLOUT, 0);
    }
    if (pnum (victim) == MOB_FANTASY_JULIUS) {
      sendf (attacker, "You grab Julius and order him to tell you where "
	     "Lord Glaive is as he\nlets out his last breath.. "
	     "Julius waves his hands and you start to\nfeel faint.. you "
	     "awaken finding yourself in a different place..\n");
      setploc (attacker, LOC_FANTASY_TREE1);

      x = real_mynum;
      setup_globals (attacker);
      trapch (ploc (mynum));
      setup_globals (x);
    }
#endif

    if ((q = victim - max_players) == MOB_EFOREST_LICH)
      x = max_players + MOB_EFOREST_ASMODEUS;
    else if (q == MOB_EFOREST_ASMODEUS)
      x = max_players + MOB_EFOREST_LICH;
    else
      x = -1;
    if (x >= 0) {
      if (alive (x) == -1) {
	send_msg (DEST_ALL, MODE_NODEAF, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
		  "A Great Evil has departed from the land.\n");
	if (pscore (x) == attacker) {
	  sendf (attacker, "You have avenged the Elven Forest!\n");
	  set_quest (attacker, Q_EFOREST);
	}
      }
    }
    if (ptemporary (victim))
      destruct_mobile (victim, NULL);

  } else if (plev (victim) >= LVL_WIZARD)
    return False;		/* Wizards don't die */
  else {
    /* Kill mortal */
    mudlog ("SYSTEM: %s slain %sby %s", pname (victim),
	    (hit_type < 0 ? "" : "magically "),
	    pname (attacker));
    if (attacker < max_players &&
	(the_world->w_tournament || plev (victim) > LVL_HERO)) {
      x = plev (victim);
      setpscore (attacker, pscore (attacker) + x * x * x * 10);
    }
    setpstr (victim, 40);
    setpscore (victim, pscore (victim) / 2);

    x = real_mynum;
    setup_globals (victim);

    /* Count of number of deaths */
    if (victim < max_players && !the_world->w_tournament)
      setpdied (victim, pdied (victim) + 1);

    send_msg (DEST_ALL, 0, pvis (victim), LVL_MAX, victim, NOBODY,
	      "&+B%s &*shouts &+W'&*Huh huh what a wus, %s couldn't even kill me!&+W'\n",
	      pname (attacker), pname (victim));

    crapup ("\t\tOh dear... you seem to be slightly dead.\n",
	    CRAP_SAVE | CRAP_RETURN);
    setup_globals (x);

  }
  return True;
}

void
combatmessage (int victim, int attacker, int wpn, int ddn)
{
  static char *form = "\001p%s\003";

  char vic_name[64];
  char att_name[64];
  char weap[64];
  char his[20];
  char hard[64];

  if (victim < 0 || attacker < 0 || victim >= numchars ||
      attacker >= numchars)
    return;
  sprintf (vic_name, form, pname (victim));
  sprintf (att_name, form, pname (attacker));

  strcpy (his, his_or_her (attacker));

  if (wpn >= 0) {
    strcpy (weap, oname (wpn));
  } else {
    strcpy (weap, "bare hands");
  }

  if (ddn < 0) {		/* Miss */
    sendf (attacker, "You missed %s.\n", vic_name);
    sendf (victim, "%s missed you.\n", att_name);
    send_msg (ploc (victim), MODE_NSFLAG | MS (SFL_NOFIGHT), LVL_MIN, LVL_MAX, victim, attacker,
	      "%s missed %s.\n", att_name, vic_name);
  } else if (ddn < 4) {
    sendf (attacker, "You hit %s very lightly.\n", vic_name);
    sendf (victim, "%s hit you very lightly.\n", att_name);
    send_msg (ploc (victim), MODE_NSFLAG | MS (SFL_NOFIGHT), LVL_MIN, LVL_MAX, victim, attacker,
	      "%s hit %s very lightly.\n", att_name, vic_name);
  } else if (ddn < 7) {
    sendf (attacker, "You hit %s lightly.\n", vic_name);
    sendf (victim, "%s hit you lightly.\n", att_name);
    send_msg (ploc (victim), MODE_NSFLAG | MS (SFL_NOFIGHT), LVL_MIN, LVL_MAX, victim, attacker,
	      "%s hit %s lightly.\n", att_name, vic_name);
  } else if (ddn < 13) {
    sendf (attacker, "You hit %s.\n", vic_name);
    sendf (victim, "%s hit you.\n", att_name);
    send_msg (ploc (victim), MODE_NSFLAG | MS (SFL_NOFIGHT), LVL_MIN, LVL_MAX, victim, attacker,
	      "%s hit %s.\n", att_name, vic_name);
  } else {
    if (ddn < 17)
      strcpy (hard, " hard");
    else if (ddn < 21)
      strcpy (hard, " very hard");
    else
      strcpy (hard, " extremely hard");
    sendf (attacker, "You hit %s%s with your %s.\n", vic_name, hard, weap);
    sendf (victim, "%s hit you%s with %s %s.\n", att_name, hard, his, weap);
    send_msg (ploc (victim), MODE_NSFLAG | MS (SFL_NOFIGHT), LVL_MIN, LVL_MAX, victim, attacker,
	      "%s hit %s%s with %s %s.\n",
	      att_name, vic_name, hard, his, weap);
  }

}

void
setpfighting (int x, int y)
{
  int ct, loc = ploc (x);

  pfighting (x) = y;

  if (y == -1) {
    if (exists (loc)) {
      for (ct = 0; ct < lnumchars (loc); ct++) {
	if (pfighting (lmob_nr (ct, loc)) == x) {
	  pfighting (lmob_nr (ct, loc)) = -1;
	}
      }
    }
  } else {
    pfighting (y) = x;
  }
}
