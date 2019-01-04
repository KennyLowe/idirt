#include <stdlib.h>
#include "kernel.h"
#include "sendsys.h"
#include "magic.h"
#include "pflags.h"
#include "sflags.h"
#include "cflags.h"
#include "lflags.h"
#include "oflags.h"
#include "objects.h"
#include "bprintf.h"
#include "parse.h"
#include "mobile.h"
#include "mud.h"
#include "uaf.h"
#include "objsys.h"
#include "rooms.h"


/* Brian Preble -- Heals a player (and mobiles -Nicknack) */
void
healcom (void)
{
  int a;
  Boolean f;
  PERSONA p;

  if (EMPTY (item1)) {
    bprintf ("Heal who?\n");
    return;
  }
  if (!ptstflg (mynum, PFL_HEAL)) {
    bprintf ("The spell fails.\n");
    return;
  }
  if (brkword () == -1 || (a = find_player (wordbuf, &p, &f)) == -1) {
    bprintf ("Heal who?\n");
    return;
  }
  if (a == mynum) {
    bprintf ("You feel much better.\n");
    setpstr (mynum, maxstrength (mynum));
    setpmagic (a, maxmagic (a));
    return;
  } else if (a >= max_players) {
    setpstr (a, pstr_reset (a));
  } else if (a >= 0) {
    if (plev (mynum) < LVL_AVATAR) {
      if (ststflg (a, SFL_NOHEAL)) {
	bprintf ("A magical force prevents you from healing that person.\n");
	return;
      }
      if ((pfighting (a) >= 0) && !ststflg (a, SFL_HEALFIGHT)) {
	bprintf ("You cannot heal a person in a fight.\n");
	return;
      }
    }
    setpstr (a, maxstrength (a));
    setpmagic (a, maxmagic (a));
  } else if (f) {
    bprintf ("%s will feel much better now!\n", p.p_name);
    p.p_strength = pmaxstrength (p.p_level);
    p.p_magic = pmaxmagic (p.p_level);
    putuaf (&p);
    return;
  }
  bprintf ("Power radiates from your hands to heal %s.\n", pname (a));
  sendf (a, "%s heals all your wounds.\n", see_name (a, mynum));

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	    "&+B[&+W\001p%s\003 &*has &+Chealed &+W\001p%s\003&+B]\n",
	    pname (mynum), pname (a));

}

void
healallcom (void)
{
  int i;
  int j = 0;

  if (!ptstflg (mynum, PFL_HEAL)) {
    bprintf ("The spell fails.\n");
    return;
  }
  if (plev (mynum) <= LVL_ARCHWIZARD && HEALALL_TIME != 0) {
    if (last_healall < global_clock && global_clock < last_healall + HEALALL_TIME) {
      bprintf ("The spell fails.\n");
      return;
    }
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && pfighting (i) < 0 && plev (i) < LVL_WIZARD &&
	((pstr (i) < maxstrength (i) || pmagic (i) < maxmagic (i)) ||
	 (pstr (i) < maxstrength (i) && pmagic (i) < maxmagic (i)))) {

      if (j == 0) {
	bprintf ("You have healed the following players:\n");
	j = 1;
      }
      setpstr (i, maxstrength (i));
      setpmagic (i, maxmagic (i));
      bprintf ("  %s\n", pname (i));
      sendf (i, "%s unleashes a healing spell over the world.\n",
	     see_name (i, mynum));
    }
  }

  if (j == 0) {
    bprintf ("No players needing healing.\n");
  } else {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	      "&+B[&+CHealall &*by &+W\001p%s\003&+B]\n", pname (mynum));
    last_healall = global_clock;
  }
}

void
forcecom (void)
{
  int rme = real_mynum;
  int me = mynum;
  int a;
  char z[MAX_COM_LEN];

  if ((a = vicf2 (SPELL_REFLECTS, 4)) < 0)
    return;
  if (a >= max_players) {
    bprintf ("You can only force players to do things.\n");
    return;
  }
  if (plev (mynum) < LVL_WIZARD && ploc (mynum) != ploc (a)) {
    bprintf ("There is no %s here.\n", pname (a));
    return;
  }
  getreinput (z);

  if (do_okay (me, a, PFL_NOFORCE)) {

    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "[\001p%s\003 forced \001p%s\003]\n[Force:%s]\n",
	      pname (rme), pname (a), z);

    setup_globals (a);
    bprintf ("%s has forced you to %s\n", see_name (a, me), z);
    cur_player->isforce = True;
    gamecom (z, True);
    cur_player->isforce = False;
    setup_globals (rme);

  } else {

    sendf (a, "%s tried to force you to %s\n", see_name (a, me), z);
  }
}

void
forceallcom (void)
{
  int i, me = real_mynum;
  char com[MAX_COM_LEN];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  getreinput (com);

  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && see_player (me, i) && do_okay (me, i, PFL_NOFORCE)
	&& i != me) {
      setup_globals (i);
      bprintf ("\001p%s\003 has forced you to %s.\n", pname (me), com);
      cur_player->isforce = True;
      gamecom (com, True);
      cur_player->isforce = False;
    }
  }

  setup_globals (me);

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
    "&+B[&+CForceall &*by &+W\001p%s\003 &*(%s)&+B]\n", pname (mynum), com);

}

void
sumcom (void)
{
  int a, me = mynum, rme = real_mynum;
  int c, d, x;
  char xx[SETIN_MAX + 200];

  if (brkword () == -1) {
    bprintf ("Summon who?\n");
    return;
  }
  do {
    if ((a = fpbn (wordbuf)) != -1) {

      if (plev (mynum) < LVL_WIZARD) {
	if (a == mynum) {
	  bprintf ("You're already here.\n");
	  return;
	}
	if (pmagic (mynum) < 1) {
	  bprintf ("You're too weak.\n");
	  return;
	}
	setpmagic (mynum, pmagic (mynum) - 1);

	c = plev (mynum) * 2;

	if (carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE) > -1 ||
	    carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE1) > -1 ||
	    carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE2) > -1 ||
	    carries_obj_type (mynum, OBJ_FANTASY_MANA) > -1)
	  c += plev (mynum);

	d = randperc ();

	if (ltstflg (ploc (mynum), LFL_ONE_PERSON)) {
	  bprintf ("It's too restricted in here.\n");
	  return;
	}
	if (ltstflg (ploc (mynum), LFL_NO_SUMMON)) {
	  bprintf ("Something prevents your summoning "
		   "from succeeding.\n");
	  return;
	}
	if (ltstflg (ploc (mynum), LFL_ON_WATER)) {
	  bprintf ("The boat is rolling too much.\n");
	  return;
	}
	if (a >= max_players) {
	  bprintf ("You can't summon mobiles.\n");
	  return;
	} else if (c < d || players[a].asmortal > 0) {
	  bprintf ("The spell fails.\n");
	  return;
	}
	if (wears_obj_type (a, OBJ_CATACOMB_SHIELD) > -1
#ifdef LOCMIN_TALON
	    || wears_obj_type (a, OBJ_TALON_TALONSHIELD) > -1
#endif
	    || wears_obj_type (a, OBJ_TREEHOUSE_AMULET) > -1
	    || carries_obj_type (a, OBJ_CASTLE_RUNESWORD) > -1
	    || wears_obj_type (a, OBJ_BLIZZARD_TALISMAN) > -1
	    || wears_obj_type (a, OBJ_ICECAVE_PENDANT) > -1) {
	  bprintf ("Something prevents your summoning "
		   "from succeeding.\n");
	  return;
	}
      }				/* end; if not wiz */
      if (!do_okay (mynum, a, PFL_NOSUMMON)) {
	bprintf ("%s doesn't want to be summoned.\n", pname (a));
	sendf (a, "%s tried to summon you!\n", see_name (a, mynum));
	continue;
      }
      bprintf ("You cast the summoning...\n");

      if (plev (mynum) < LVL_WIZARD && !the_world->w_tournament) {
	sendf (a, "You drop everything as you are summoned by %s.\n",
	       see_name (a, mynum));

	/* let a drop everything except items worn/wielded */

      } else {
	sendf (a, "%s\n", build_setin (xx, players[mynum].setsum, ipname (mynum), NULL, NULL));
      }

      send_msg (ploc (a), 0, pvis (a), LVL_MAX, a, NOBODY,
		"%s\n", build_setin (xx, players[mynum].setsumout, ipname (mynum), NULL, pname (a)));

      if (a < max_players) {

	send_msg (ploc (mynum), 0, pvis (a), LVL_MAX, a, NOBODY,
		  "%s\n", build_setin (xx, players[mynum].setsumin, ipname (mynum), NULL, pname (a)));

	setup_globals (a);
	trapch (ploc (me));	/*summon into death rooms might cause prob. */
	/*because of the crapup() call in trapch() */


	setup_globals (rme);
      } else {

	send_msg (ploc (mynum), 0, pvis (a), LVL_MAX, a, NOBODY,
		  "%s\n", build_setin (xx, players[mynum].setsumin, ipname (mynum), NULL, pname (a)));

	setploc (a, ploc (mynum));
      }

      continue;

    } else if ((a = fobn (wordbuf)) != -1) {

      /* Summon object... also by number now.
       */

      if (!ptstflg (mynum, PFL_SUMOBJ)) {
	bprintf ("You can only summon people.\n");
	continue;
      }
      x = a;

      if (ovis (x) > plev (mynum)) {
	bprintf ("Who or what is %s?\n", wordbuf);
      }
      while (ocarrf (x) == IN_CONTAINER)
	x = oloc (x);

      if (ocarrf (x) >= CARRIED_BY)
	x = ploc (oloc (x));
      else
	x = oloc (x);

      send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	   "%s fetches something from another dimension.\n", pname (mynum));

      sendf (x, "The %s vanishes!\n", oname (a));

      bprintf ("The %s flies into your hand.\nIt was:", oname (a));
      desrm (oloc (a), ocarrf (a));
      setoloc (a, mynum, CARRIED_BY);
      continue;

    } else {
      bprintf ("Who or what is %s?\n", wordbuf);
    }
  }
  while (plev (mynum) >= LVL_WIZARD && brkword () != -1);
}


static void
vis_proc (int new)
{
  char xx[SETIN_MAX + 200];

  int oldvis = pvis (mynum);

  setpvis (mynum, new);

  bprintf ("Ok\n");

  if (new < oldvis) {
    send_msg (ploc (mynum), 0, new, oldvis, mynum, NOBODY,
    "%s\n", build_setin (xx, cur_player->setvin, pname (mynum), NULL, NULL));
  } else {
    send_msg (ploc (mynum), 0, oldvis, new, mynum, NOBODY,
	      "%s\n", build_setin (xx, cur_player->setvout, pname (mynum), NULL, NULL));
  }
}

void
viscom (void)
{
  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("You can't do that sort of thing at will, you know.\n");
    return;
  }
  if (pvis (mynum) == 0) {
    bprintf ("You're already visible.\n");
    return;
  }
  vis_proc (0);
}

void
inviscom ()
{
  int x, y;

  switch (wlevel (plev (mynum))) {
  case LEV_MASTER:
    x = LVL_MAX;
    break;
  case LEV_GOD:
    x = LVL_MASTER;
    break;
  case LEV_AVATAR:
    x = LVL_GOD;
    break;
  case LEV_ADVISOR:
    x = LVL_AVATAR;
    break;
  case LEV_ARCHWIZARD:
    x = LVL_ADVISOR;
    break;
  case LEV_PROPHET:
    x = LVL_ARCHWIZARD;
    break;
  case LEV_WIZARD:
  case LEV_EMERITI:
  case LEV_DORQ:
  case LEV_APPRENTICE:
    x = LVL_PROPHET;
    break;

  default:
    bprintf ("You can't turn invisible at will, you know.\n");
    return;
  }

  if (brkword () == -1 || (y = atoi (wordbuf)) < 0 || y > x)
    y = x;

  if (pvis (mynum) == y) {
    if (pvis (mynum) == 0)
      bprintf ("You're already visible.\n");
    else
      bprintf ("You're already invisible (Level %d).\n", pvis (mynum));
    return;
  }
  vis_proc (y);
}



void
resurcom (void)
{
  int a;

  if (!ptstflg (mynum, PFL_RES)) {
    erreval ();
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Resurrect what?\n");
    return;
  }
  if ((a = fobn (item1)) == -1) {
    if ((a = fpbn (item1)) == -1 || a < max_players) {
      bprintf ("You can only resurrect objects and mobiles.\n");
      return;
    }
    if (!EMPTY (pname (a))) {
      /* if alive we should only give them strength and level back :-) */

      if (pfighting (a) != -1) {
	bprintf ("%s is fighting at the moment.\n", pname (a));
	return;
      } else if (pstr (a) >= 0) {
	bprintf ("%s is resurrected.\n", pname (a));
	setpstr (a, pstr_reset (a));
	return;
      }
    }
    if (ltstflg (ploc (mynum), LFL_ONE_PERSON)) {
      bprintf ("This room is too small.\n");
      return;
    }
    reset_mobile (a);

    setploc (a, ploc (mynum));

    sendf (ploc (mynum), "%s suddenly appears.\n", pname (a));
    return;
  }
  if (ospare (a) != -1) {
    bprintf ("It already exists.\n");
    return;
  }
  oclrbit (a, OFL_DESTROYED);
  setoloc (a, ploc (mynum), IN_ROOM);

  sendf (ploc (mynum), "The %s suddenly appears.\n", oname (a));
}



void
deafcom (void)
{
  int a;

  if ((a = vicf2 (SPELL_REFLECTS, 4)) != -1) {

    if (do_okay (mynum, a, PFL_NOMAGIC)) {
      sendf (a, "You have been magically deafened!\n");
      ssetflg (a, SFL_DEAF);
    } else
      sendf (a, "%s tried to deafen you!\n", see_name (a, mynum));
  }
}

void
blindcom (void)
{
  int a;

  if ((a = vicf2 (SPELL_REFLECTS, 4)) != -1) {

    if (do_okay (mynum, a, PFL_NOMAGIC)) {
      sendf (a, "You have been struck magically blind!\n");
      ssetflg (a, SFL_BLIND);
    } else
      sendf (a, "%s tried to blind you!\n", see_name (a, mynum));
  }
}


void
dumbcom (void)
{
  int a;

  if ((a = vicf2 (SPELL_REFLECTS, 4)) >= 0) {
    if (do_okay (mynum, a, PFL_NOMAGIC)) {
      sendf (a, "You have been struck magically dumb!\n");
      ssetflg (a, SFL_DUMB);
    } else
      sendf (a, "%s tried to mute you!\n", see_name (a, mynum));
  }
}


void
cripplecom (void)
{
  int a;

  if ((a = vicf2 (SPELL_REFLECTS, 4)) >= 0) {
    if (do_okay (mynum, a, PFL_NOMAGIC)) {
      sendf (a, "You have been magically crippled!\n");
      ssetflg (a, SFL_CRIPPLED);
    } else
      sendf (a, "%s tried to cripple you!\n", see_name (a, mynum));
  }
}


void
curecom (void)
{
  int a;

  if (EMPTY (item1)) {
    bprintf ("Cure who?\n");
    return;
  }
  if ((a = vicf2 (SPELL_PEACEFUL, 8)) >= 0) {
    sclrflg (a, SFL_DUMB);
    sclrflg (a, SFL_CRIPPLED);
    sclrflg (a, SFL_BLIND);
    sclrflg (a, SFL_DEAF);
    sendf (a, "All your ailments have been cured.\n");
    if (a != mynum)
      bprintf ("With a laying on of hands, you miraculously cure %s.\n",
	       pname (a));
  }
}
