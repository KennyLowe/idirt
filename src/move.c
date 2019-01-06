

/* Movement routines (north, south, go, etc.) */

#include "kernel.h"
#include "parse.h"
#include "objects.h"
#include "mobiles.h"
#include "locations.h"
#include "lflags.h"
#include "levels.h"
#include "mflags.h"
#include "sendsys.h"
#include "move.h"
#include "bprintf.h"
#include "objsys.h"
#include "mobile.h"
#include "rooms.h"
#include "fight.h"
#include "uaf.h"

static char *exittxt[] =
{
  "north", "east", "south", "west", "up", "down",
  "n", "e", "s", "w", "u", "d",
  0
};

static int exitnum[] =
{
  1, 2, 3, 4, 5, 6,
  1, 2, 3, 4, 5, 6
};

int
dodirn (int vb)
{
  char block[SETIN_MAX + 200];
  int i, pc, n;
  int newch, drnum, droff;

  if (pfighting (mynum) >= 0) {
    bprintf ("You can't just stroll out of a fight!\n");
    bprintf ("If you wish to leave, you must FLEE in a direction.\n");
    return -1;
  }
  if (iscarrby (OBJ_CATACOMB_CUPSERAPH, mynum)
      && (i = alive ((max_players + MOB_CATACOMB_SERAPH))) != -1
      && ploc (i) == ploc (mynum)) {
    bprintf ("The Seraph says 'I cannot allow you to leave this place "
	     "with the Holy Relic.'\n");
    return -1;
  }
  if (iscarrby (OBJ_CASTLE_RUNESWORD, mynum)
      && ploc ((max_players + MOB_CASTLE_GOLEM)) == ploc (mynum)
      && !EMPTY (pname ((max_players + MOB_CASTLE_GOLEM)))) {
    bprintf ("\001cThe Golem\003 bars the doorway!\n");
    return -1;
  }
  n = vb - 2;			/* Since VERB_NORTH = 2 etc....stupid */

  if (chkcrip () || chksitting ())
    return -1;

  switch (newch = getexit (ploc (mynum), n)) {
  case EX_DOWN_SLIME:
    if (state (OBJ_BLIZZARD_SLIME_PIT) != 0) {
      bprintf ("That doesn't look like a very good idea!\n");
      return -1;
    }
    newch = LOC_BLIZZARD_SLIME;
    break;
  case EX_DEFENDER:
    newch = LOC_CATACOMB_HALL;
    break;
  case EX_GET_STUFF:
    newch = LOC_CATACOMB_CHAMBER;
    break;

  default:

    if (newch >= DOOR && newch < EDOOR) {
      drnum = newch - DOOR;
      droff = /*drnum ^ 1 */ olinked (drnum);	/* other door side */
      if (state (drnum)) {
	if (!EQ (oname (drnum), "door") || isdark ()
	    || EMPTY (olongt (drnum, state (drnum))))
	  bprintf ("You can't go that way.\n");
	else
	  bprintf ("The door is closed.\n");
	return -1;
      }
      newch = obj_loc (droff);
    }
    break;
  }

  if (!exists (newch)) {
    bprintf ("You can't go that way.\n");
    return -1;
  }
  if (ltstflg (newch, LFL_PRIVATE) || ltstflg (newch, LFL_ONE_PERSON)) {
    pc = 0;
    for (i = 0; i < max_players; i++)
      if (!EMPTY (pname (i)) && ploc (i) == newch)
	pc++;
    if (pc > (ltstflg (newch, LFL_PRIVATE) ? 1 : 0)) {
      bprintf ("I'm sorry, that room is currently full.\n");
      return -1;
    }
  }
  /* database.c: North to trapdoor in ORCHOLD */
  if (n == 0) {
    if (ploc (mynum) == LOC_ORCHOLD_DAMP) {
      bprintf ("You fall through a trap door, plummeting to the ground below!\n");
      bprintf ("Dazed, you pick yourself up from the floor to find yourself...\n");
      setploc (mynum, LOC_ORCHOLD_TRAP);
      lookin (ploc (mynum), 0);
      return -1;
    }
  }
  /* Ruins Zone */
  if (newch == LOC_RUINS_SINKHOLE) {
    trapch (LOC_RUINS_SINKHOLE);
    bprintf ("You sink into the goo and find yourself in a lower level!\n");
    setploc (mynum, LOC_RUINS_GOOEY);
    trapch (ploc (mynum));
    return -1;
  }
  if (newch == LOC_RUINS_TELEPORT) {
    trapch (LOC_RUINS_TELEPORT);
    bprintf ("You open your eyes and find yourself in a different room..\n");
    setploc (mynum, LOC_RUINS_TELEPORTEND);
    trapch (ploc (mynum));
    return -1;
  }
  /* FANTASY */
#ifdef LOCMIN_FANTASY
  if (newch == LOC_FANTASY_GAIA1) {
    if ((!iswornby (OBJ_FANTASY_SILVERARMOR, mynum)
	 && !iscarrby (OBJ_FANTASY_SILVERSWORD, mynum))) {
      bprintf ("The Gaia Cave will not let you enter..\n");
      return -1;
    }
    bprintf ("The Gaia Cave seems to like the silver armor "
	     "and sword, you\nare allowed to pass.\n\n");
  }
  if (newch == LOC_FANTASY_AIRSHIP1) {
    bprintf ("The airship begins to take off as you fly over the world..\n");
    setploc (mynum, LOC_FANTASY_CASTLE1);
    trapch (ploc (mynum));
    return -1;
  }
  if (newch == LOC_FANTASY_AIRSHIP2) {
    bprintf ("The airship begins to take off as you fly over the world..\n");
    setploc (mynum, LOC_FANTASY_DFOREST18);
    trapch (ploc (mynum));
    return -1;
  }
#endif

  if (newch == LOC_BLIZZARD_LAVA_PATH2) {
    if (!iswornby (OBJ_BLIZZARD_SHIELD1, mynum)
	&& !iswornby (OBJ_BLIZZARD_SHIELD2, mynum)
	&& !iswornby (OBJ_TREEHOUSE_SHIELD, mynum)
	&& !iswornby (OBJ_CATACOMB_SHIELD, mynum)
	&& !iswornby (OBJ_EFOREST_SHIELD, mynum)) {
      bprintf ("The intense heat drives you back.\n");
      return -1;
    }
    bprintf ("The shield protects you from the worst of the lava's heat.\n");
  }
  if (n == EX_NORTH) {
    for (i = max_players; (!mtstflg (i, MFL_BAR_N) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_N) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 says 'None shall pass!'\n", pname (i));
      return -1;
    }
    if (iscarrby (OBJ_EFOREST_HOPE, mynum) && ploc (mynum) == LOC_EFOREST_STONE) {
      bprintf ("A mysterious force prevents you from going that way.\n");
      return -1;
    }
  }
  if (n == EX_WEST) {

    for (i = max_players; (!mtstflg (i, MFL_BAR_W) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_W) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 gives a warning growl.\n", pname (i));
      bprintf ("\001p%s\003 won't let you go West!\n", pname (i));
      return -1;
    }
  }
  if ((n == 0) || (n == 1) || (n == 2) || (n == 5)) {
    if ((i = alive ((max_players + MOB_EFOREST_ASMADEUS))) != -1
	&& plev (mynum) < LVL_WIZARD
	&& ploc (i) == ploc (mynum)) {
      bprintf ("\001pAsmadeus\003 refuses to let you enter his museum.\n");
      return -1;
    }
  }
  if (n == EX_DOWN) {		/* can't go down unless empty-handed */
    if ((ploc (mynum) == LOC_CATACOMB_BEGGAR || ploc (mynum) == LOC_VALLEY_FALLS)
	&& gotanything (mynum)) {
      if (ploc (mynum) == LOC_CATACOMB_BEGGAR) {
	bprintf ("A mysterious force blocks your passage.\n");
	if (ploc ((max_players + MOB_CATACOMB_BEGGAR)) == ploc (mynum)) {
	  sendf (ploc (mynum),
		 "%s says 'To continue on, you must forego all worldly "
	      "possessions.'\n", pname (max_players + MOB_CATACOMB_BEGGAR));
        }
	return -1;
      } else {
	bprintf ("The steep and slippery sides of the pool make it "
		 "impossible to climb down\nwithout dropping everything "
		 "first.\n");
	return -1;
      }
    }
    for (i = max_players; (!mtstflg (i, MFL_BAR_D) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_D) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 refuses to let you go Down!\n", pname (i));
      return -1;
    }
  }
  if (n == EX_UP) {
    for (i = max_players; (!mtstflg (i, MFL_BAR_U) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_U) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 blocks your way up!\n", pname (i));
      return -1;
    }
#ifdef LOCMIN_ANCIENT
    if ((i = ploc (mynum)) == ploc ((max_players + MOB_ANCIENT_RATTLESNAKE))
	&& alive ((max_players + MOB_ANCIENT_RATTLESNAKE)) != -1
	&& (!ishere (OBJ_ANCIENT_CHAIN) || !ishere (OBJ_ANCIENT_RBLOCK) ||
	    !ishere (OBJ_ANCIENT_RCOINS) || !ishere (OBJ_ANCIENT_RPLATE))) {
      hit_player (max_players + MOB_ANCIENT_RATTLESNAKE, mynum, -1);
      return -1;
    }
#endif
  }
  if (n == EX_SOUTH) {

    if ((i = alive ((max_players + MOB_BLIZZARD_FIGURE))) != mynum && i != -1
	&& ploc (i) == ploc (mynum) && !iswornby (OBJ_BLIZZARD_BLACKROBE, mynum)) {
      bprintf ("\001pThe Figure\003 holds you back!\n");
      bprintf ("\001pThe Figure\003 says 'Only true sorcerors may pass.'\n");
      return -1;
    }
    for (i = max_players; (!mtstflg (i, MFL_BAR_S) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_S) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 holds you back!\n", pname (i));
      return -1;
    }
  }
  if (n == EX_EAST) {
    if ((i = alive ((max_players + MOB_OAKTREE_VIOLA))) != mynum && i != -1
	&& ploc (i) == ploc (mynum) &&
	carries_obj_type (i, OBJ_OAKTREE_FAN) == -1) {
      bprintf ("\001pViola\003 says 'How dare you come to our land!  "
	       "Leave at once!'\n");
      return -1;
    }
    for (i = max_players; (!mtstflg (i, MFL_BAR_E) || ploc (i) != ploc (mynum)
			   || alive (i) == -1) && i < numchars; i++) ;
    if (mtstflg (i, MFL_BAR_E) && alive (i) != -1 && ploc (i) == ploc (mynum)
	&& plev (mynum) < LVL_WIZARD) {
      bprintf ("\001p%s\003 won't let you go East!\n", pname (i));
      return -1;
    }
  }
  if (ltstflg (newch, LFL_ON_WATER)) {
    if (plev (mynum) < LVL_WIZARD && !carries_boat (mynum)) {

      bprintf ("You need a boat to go to sea!\n");
      return -1;
    }
  }
  if (ltstflg (newch, LFL_IN_WATER)) {
    if (plev (mynum) < LVL_WIZARD) {
      bprintf ("You'd surely drown!\n");
      return -1;
    }
  }
  if (n == EX_DOWN && ploc (mynum) == LOC_OAKTREE_LANDING) {
    bprintf ("You slide down the banister.  Wheee!\n");

    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s slides down the banister shouting 'Yippeee...'\n",
	      pname (mynum));

    send_msg (newch, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s slides down the banister and lands at your feet.\n",
	      pname (mynum));

    if (oloc (OBJ_OAKTREE_MARBLEBUST) == newch) {
      bprintf ("On your way down, you smash a valuable bust.\n");

      send_msg (newch, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		"%s smashed a valuable bust on %s way down.\n",
		pname (mynum), his_or_her (mynum));

      destroy (OBJ_OAKTREE_MARBLEBUST);
      create (OBJ_OAKTREE_BUSTBROKEN);
    }
  } else if (mynum < max_players && cur_player->asmortal > 0) {
    send_msg (ploc (mynum), 0, max (LVL_WIZARD, pvis (mynum)), LVL_MAX,
	      mynum, NOBODY, "%s\n",
	      build_setin (block, cur_player->setout, pname (mynum), exittxt[n], NULL));
    send_msg (newch, 0, max (LVL_WIZARD, pvis (mynum)), LVL_MAX,
	      mynum, NOBODY, "%s\n",
	 build_setin (block, cur_player->setin, pname (mynum), NULL, NULL));
    if (pvis (mynum) < LVL_WIZARD) {
      send_msg (ploc (mynum), 0, pvis (mynum), LVL_WIZARD, NOBODY, NOBODY,
		"%s has gone %s.\n", pname (mynum), exittxt[n]);
      send_msg (newch, 0, pvis (mynum), LVL_WIZARD, NOBODY, NOBODY,
		"%s has arrived.\n", pname (mynum));
    }
  } else {

    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY, "%s\n",
	      build_setin (block, cur_player->setout, pname (mynum), exittxt[n], NULL));

    send_msg (newch, 0, pvis (mynum), LVL_MAX, mynum, NOBODY, "%s\n",
	 build_setin (block, cur_player->setin, pname (mynum), NULL, NULL));
  }
  setpfighting (mynum, -1);
  trapch (newch);
  do_follow ();
  return 0;
}

int
dogocom ()
{
  int a = (brkword () == -1) ? get_rand_exit_dir (ploc (mynum))
  : chklist (wordbuf, exittxt, exitnum) - 1;

  if (a < 0 || a >= NEXITS) {
    bprintf ("Go where?\n");
    return -1;
  }
  return dodirn (a + 2);
}
