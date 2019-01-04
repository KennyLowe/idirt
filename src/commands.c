
#include <stdlib.h>
#include <unistd.h>
#include "kernel.h"
#include "locations.h"
#include "objects.h"
#include "mobiles.h"
#include "sflags.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "cflags.h"
#include "eflags.h"
#include "mflags.h"
#include "quests.h"
#include "sendsys.h"
#include "levels.h"
#include "commands.h"
#include "rooms.h"
#include "objsys.h"
#include "mobile.h"
#include "flags.h"
#include "bprintf.h"
#include "parse.h"
#include "uaf.h"
#include "clone.h"
#include "acct.h"
#include "mud.h"
#include "nflags.h"
#include "fight.h"
#include "zones.h"
#include "log.h"
#include "reboot.h"
#include "wizlist.h"
#include "climate.h"
#include "mailer.h"
#include "mud.h"
#include "timing.h"

extern char *WizLevels[];
extern char *MLevels[];
extern char *FLevels[];

void
lookcom (void)
{
  int a;

  if (brkword () == -1) {
    lookin (ploc (mynum), SHOW_LONG);
  } else {
    if (EQ (wordbuf, "at")) {
      strcpy (item1, item2);
      ob1 = ob2;
      examcom ();
    } else if (!EQ (wordbuf, "in") && !EQ (wordbuf, "into")) {
      bprintf ("I don't understand.  Are you trying to LOOK, LOOK AT, "
	       "or LOOK IN something?\n");
    } else if (EMPTY (item2)) {
      bprintf ("In what?\n");
    } else if ((a = ob2) == -1) {
      bprintf ("What?\n");
    } else if (a == OBJ_BLIZZARD_TUBE ||
	       a == OBJ_TREEHOUSE_BEDDING ||
	       a == OBJ_ORCHOLD_TOPCOVER) {
      strcpy (item1, item2);
      ob1 = ob2;
      examcom ();
    } else if (!otstbit (a, OFL_CONTAINER)) {
      bprintf ("That isn't a container.\n");
    } else if (otstbit (a, OFL_OPENABLE) && state (a) != 0) {
      bprintf ("It's closed!\n");
    } else {
      bprintf ("The %s contains:\n", oname (a));
      aobjsat (a, IN_CONTAINER, 8);
    }
  }
}

void
wherecom (void)
{
  int cha, rnd, num_obj_found = 0, num_chars_found = 0;

  if (plev (mynum) < LVL_WIZARD && pstr (mynum) < 10) {
    bprintf ("You're too weak to cast any spells.\n");
    return;
  }
  if (ltstflg (ploc (mynum), LFL_NO_MAGIC) && plev (mynum) < LVL_WIZARD) {
    bprintf ("Something about this location has drained your mana.\n");
    return;
  }
  if (plev (mynum) < LVL_WIZARD)
    setpmagic (mynum, pmagic (mynum) - 2);

  rnd = randperc ();
  cha = 6 * plev (mynum);

  if (carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE) > -1 ||
      carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE1) > -1 ||
      carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE2) > -1 ||
      carries_obj_type (mynum, OBJ_FANTASY_MANA) > -1 ||
      plev (mynum) >= LVL_LEGEND)
    cha = 100;

  if (rnd > cha) {
    bprintf ("Your spell fails.\n");
    return;
  }
  if (!item1[0]) {
    bprintf ("What's that?\n");
    return;
  }
  for (cha = 0; cha < numobs; cha++) {

    if (cha == num_const_obs && plev (mynum) < LVL_WIZARD)
      break;

    if (EQ (oname (cha), item1)
	|| (plev (mynum) >= LVL_WIZARD && EQ (oaltname (cha), item1))) {
      if (ovis (cha) <= plev (mynum)) {
	num_obj_found++;
	if (plev (mynum) >= LVL_WIZARD)
	  bprintf ("[%3d] ", cha);
	bprintf ("%16.16s - ", oname (cha));
	if (plev (mynum) < LVL_WIZARD && ospare (cha) == -1)
	  bprintf ("Nowhere.\n");
	else
	  desloc (oloc (cha), ocarrf (cha));
      }
    }
  }

  for (cha = 0; cha < numchars; cha++) {

    if (cha == num_const_chars && plev (mynum) < LVL_WIZARD)
      break;

    if (EQ (xname (pname (cha)), item1) && (pvis (cha) <= plev (mynum))) {

      num_chars_found++;

      if (plev (mynum) >= LVL_WIZARD)
	bprintf ("[%d]", GLOBAL_MAX_OBJS + cha);

      bprintf ("%16.16s - ", pname (cha));
      desloc (ploc (cha), 0);
    }
  }

  if (num_obj_found == 0 && num_chars_found == 0)
    bprintf ("I don't know what that is.\n");
}

static Boolean
find_stuff (int s, int o, char *t)
{
  if (odamage (s) == 0) {
    osetdamage (s, 1);
    bprintf (t);
    create (o);
    setoloc (o, ploc (mynum), IN_ROOM);
    return True;
  }
  return False;
}

void
examcom (void)
{
  int a, foo, z;
  FILE *x;
  char ch;
  char *t;
  char text[80];

  if (!item1[0]) {
    bprintf ("Examine what?\n");
    return;
  }
  if ((a = fpbn (item1)) != -1 && ploc (a) == ploc (mynum)) {
    if (pstr (a) < 1) {
      bprintf ("You see the worm-infested corpse of %s.\n", pname (a));
      return;
    }
    if (a >= max_players && (t = pexam (a)) != NULL) {
      bprintf ("%s\n", t);
    } else {

      if (a != mynum) {
	sendf (a, "%s examines you closely.\n", pname (mynum));
      }
      sprintf (text, "%s/%s", DESC_DIR, pname (a));
      if ((x = fopen (text, "r")) == NULL) {	/*ACCESS! */
	bprintf ("A typical, run of the mill %s.\n", pname (a));
	return;
      }
      fclose (x);
      bprintf ("\001f%s\003", text);
    }
    return;
  }
  if ((a = ob1) == -1) {
    bprintf ("You see nothing special.\n");
    return;
  }
  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "%s examines the %s closely.\n", pname (mynum), oname (a));

  switch (onum (a)) {

#ifdef LOCMIN_ANCIENT

  case OBJ_ANCIENT_SKYMAP:
    if (ploc (mynum) == LOC_ANCIENT_ANC53) {
      bprintf ("You feel like you are flying....\n"
	       "     ....and land on a desolate planet,"
	       "obviously far from the sun.\n\n\n");
      trapch (LOC_ANCIENT_ANC56);
    } else {
      bprintf ("You ain't lost, Mac... yet!\n");
    }
    break;

  case OBJ_ANCIENT_LBOOK:
    if (oarmor (a) > 0) {
      bprintf ("The Book contains old writings. You can make out\n"
	       "the names of old mages like 'Elmo' and 'Infidel'.\n");
    } else {
      bprintf ("Ancient wisdom fills your head, you feel much more capable...\n");
      osetarmor (a, 1);
    }
    break;
#endif

#ifdef LOCMIN_EFOREST
  case OBJ_EFOREST_TREE:
    if (odamage (a) == 0) {
      osetdamage (a, 1);
      bprintf ("You find a door in the tree.\n");
      setobjstate (a, 0);
      return;
    }
    break;
  case OBJ_EFOREST_DESK:
    if (find_stuff (OBJ_EFOREST_DESK, OBJ_EFOREST_BUTTON,
		    "You find a button hidden in a recess of the desk.\n"))
      return;
    break;
  case OBJ_EFOREST_THRONE_WOOD:
    if (find_stuff (OBJ_EFOREST_THRONE_WOOD, OBJ_EFOREST_CROWN,
		    "You find a crown hidden under the throne.\n"))
      return;
    break;
  case OBJ_EFOREST_COAT:
    if (find_stuff (OBJ_EFOREST_COAT, OBJ_EFOREST_COIN,
		    "You find a coin in the pocket.\n"))
      return;
    break;
  case OBJ_EFOREST_TABLE:
    if (find_stuff (OBJ_EFOREST_TABLE, OBJ_EFOREST_GAUNTLETS,
		    "You find some gauntlets admidst the rubble "
		    "of this table.\n"))	/* ; */
      return;
    break;
  case OBJ_EFOREST_PAINTING:
    if (find_stuff (OBJ_EFOREST_PAINTING, OBJ_EFOREST_NOTE,
		    "There was a note behind the painting.\n"))
      return;
    break;
  case OBJ_EFOREST_CHAIR:
    if (find_stuff (OBJ_EFOREST_CHAIR, OBJ_EFOREST_RING,
		    "A ring was between the cushions of that chair!\n"))
      return;
    break;
  case OBJ_EFOREST_RACK:
    if (find_stuff (OBJ_EFOREST_RACK, OBJ_EFOREST_SCARAB,
		    "You found a scarab in the rack.\n"))
      return;
    break;
  case OBJ_EFOREST_PAPERS:
    if (find_stuff (OBJ_EFOREST_PAPERS, OBJ_EFOREST_TREATY,
		    "Among the papers, you find a treaty.\n"))
      return;
    break;
  case OBJ_EFOREST_DESK_LICH:
    if (find_stuff (OBJ_EFOREST_DESK_LICH, OBJ_EFOREST_EMERALD,
		    "Inside the desk is a beautiful emerald.\n"))
      return;
    break;
  case OBJ_EFOREST_ALTAR:
    if (find_stuff (OBJ_EFOREST_ALTAR, OBJ_EFOREST_STATUE,
		    "Inside the altar is a statue of a dark elven deity.\n"))
      return;
    break;
  case OBJ_EFOREST_MATTRESS:
    if (find_stuff (OBJ_EFOREST_MATTRESS, OBJ_EFOREST_PURSE,
		    "Hidden under the mattress is a purse.\n"))
      return;
    break;
  case OBJ_EFOREST_TRASH:
    if (find_stuff (OBJ_EFOREST_TRASH, OBJ_EFOREST_TRASH_COIN,
		    "In the trash is a silver coin.\n"))
      return;
    break;
#endif

  case OBJ_ISLAND_BONE:
    bprintf ("There is a flash and you are teleported...\n");
    setploc (mynum, LOC_LABYRINTH_K);
    lookin (ploc (mynum), 0);
    return;
    break;

  case OBJ_BLIZZARD_PLATFORM_ALTAR:
    if (state (OBJ_BLIZZARD_ALTAR) == 1) {
      bprintf ("You see nothing special.\n");
      return;
    } else {
      break;
    }

  case OBJ_BLIZZARD_BRICK:
    if (state (OBJ_BLIZZARD_ALTAR) == 1) {
      bprintf ("You see nothing special.\n");
      return;
    } else {
      break;
    }

  case OBJ_RUINS_SCROLL:
    if (state (OBJ_RUINS_SCROLL) == 2) {
      bprintf ("Try moving the crown first!\n");
      return;
    }
    bprintf ("The scroll speaks of how to cast the aid spell.\n");
    if (!etstflg (mynum, EFL_AID)) {
      bprintf ("\nYou learn the Aid spell!\n");
      bprintf ("The scroll crumbles to dust.\n");
      esetflg (mynum, EFL_AID);
      destroy (a);
      return;
    } else {
      return;
    }

  case OBJ_BLIZZARD_LITTABLET:
    bprintf ("The tablet speaks of how to cast the light spell.\n");
    if (!etstflg (mynum, EFL_LIGHT)) {
      bprintf ("\nYou learn the Light spell! Use LIT to use the spell.\n");
      bprintf ("The stone tablet crumbles to dust.\n");
      esetflg (mynum, EFL_LIGHT);
      destroy (a);
      return;
    } else {
      return;
    }

  case OBJ_RUINS_RUNES:
    if (!iswornby (OBJ_RUINS_TALISMAN, mynum)) {
      bprintf ("The runes make no sense to you.\n");
      return;
    } else {
      bprintf ("As you examine the inscription you come to understand "
	       "what they mean.\nThey speak of how to cast the Burning "
	       "Hands spell!\n");
      if (!etstflg (mynum, EFL_BHANDS)) {
	bprintf ("You learn the Burning Hands spell!\n");
	esetflg (mynum, EFL_BHANDS);
	return;
      } else {
	return;
      }
    }

  case OBJ_RUINS_DAMAGERUNES:
    if (!iswornby (OBJ_RUINS_AMULET, mynum)) {
      bprintf ("The runes make no sense to you.\n");
      return;
    } else {
      bprintf ("As you examine the inscription you come to understand what "
	       "they mean.\nThey speak of how to cast the Damage spell!\n");
      if (!etstflg (mynum, EFL_DAMAGE)) {
	bprintf ("You learn the Damage spell!\n");
	esetflg (mynum, EFL_DAMAGE);
	return;
      } else {
	return;
      }
    }

  case OBJ_PLAYHOUSE_TOILET:
    if (state (OBJ_PLAYHOUSE_TOILET) == 0)
      bprintf ("It has clean &+Bblue &*water inside of it.\n");
    else
      bprintf ("Woah! Look at the size of that! Just flush the thing!\n");
    break;

  case OBJ_BLIZZARD_TUBE:
    if (oarmor (a) == 0) {
      int obj = clone_object (OBJ_BLIZZARD_SCROLL, -1, NULL);

      if (obj >= 0) {
	osetarmor (a, 1);
	bprintf ("You take a scroll from the tube.\n");
	setoloc (obj, mynum, CARRIED_BY);
	return;
      }
    }
    break;

  case OBJ_BLIZZARD_SCROLL:
    if (iscarrby (OBJ_CATACOMB_CUPSERAPH, mynum)) {
      bprintf ("Funny, I thought this was a teleport scroll, but "
	       "nothing happened.\n");
      return;
    }
    bprintf ("As you read the scroll you are teleported!\n");
    destroy (a);
    teletrap (LOC_BLIZZARD_PENTACLE);
    return;

  case OBJ_BLIZZARD_BLACKROBE:
    if (oarmor (a) == 0) {
      int obj = clone_object (OBJ_BLIZZARD_KEY, -1, NULL);

      if (obj >= 0) {
	bprintf ("You take a key from one pocket.\n");
	osetarmor (a, 1);
	setoloc (obj, mynum, CARRIED_BY);
	return;
      }
    }
    break;

  case OBJ_TOWER_WAND:
    if (oarmor (a) != 0) {
      bprintf ("It seems to be charged.\n");
      return;
    }
    break;

  case OBJ_BLIZZARD_BALL:
    setobjstate (a, randperc () % 3 + 1);
    switch (state (a)) {
    case 1:
      bprintf ("It glows red.");
      break;
    case 2:
      bprintf ("It glows blue.");
      break;
    case 3:
      bprintf ("It glows green.");
      break;
    }
    bprintf ("\n");
    return;

  case OBJ_TOWER_SCROLL:
    foo = carries_obj_type (mynum, OBJ_BLIZZARD_BALL);
    if (foo < 0)
      foo = OBJ_BLIZZARD_BALL;

    if (((z = carries_obj_type (mynum, OBJ_TOWER_RED_CANDLE)) > -1 &&
	 state (foo) == 1 &&
	 otstbit (z, OFL_LIT)) ||

	((z = carries_obj_type (mynum, OBJ_TOWER_BLUE_CANDLE)) > -1 &&
	 state (foo) == 2 &&
	 otstbit (z, OFL_LIT)) ||

	((z = carries_obj_type (mynum, OBJ_TOWER_GREEN_CANDLE)) > -1 &&
	 state (foo) == 3 &&
	 otstbit (z, OFL_LIT))) {

      bprintf ("Everything shimmers and then solidifies into a different "
	       "view!\n");
      destroy (a);
      teletrap (LOC_TOWER_POTION);
      return;
    }
    break;

  case OBJ_VALLEY_BED:
    if (!odamage (a)) {
      int c = clone_object (OBJ_VALLEY_LOAF, -1, NULL);
      int b = clone_object (OBJ_VALLEY_PIE, -1, NULL);

      if (c >= 0 && b >= 0) {
	bprintf ("Aha!  Under the bed you find a loaf and a "
		 "rabbit pie.\n");
	setoloc (c, ploc (mynum), IN_ROOM);
	setoloc (b, ploc (mynum), IN_ROOM);
	osetdamage (a, 1);
	return;
      }
    }
    break;

#ifdef LOCMIN_ORCHOLD
  case OBJ_ORCHOLD_GARBAGE:
    if (state (a) == 0) {

      int x = clone_mobile (max_players + MOB_ORCHOLD_MAGGOT, -1, NULL);

      if (x >= 0) {

	bprintf ("In the garbage you find a gold plate... "
		 "with a maggot on it!\n");

	sillycom ("\001s%s\003A maggot leaps out of the garbage "
		  "onto %s!\n\003");

	setploc (x, ploc (mynum));

	/* Make sure maggot attacks the right person. */
	hit_player (x, mynum, -1);

	setobjstate (a, 1);
      } else {
	bprintf ("In the garbage you find a gold plate with some "
		 "slime on it.\n");
      }

      if ((x = clone_object (OBJ_ORCHOLD_ORCGOLD, -1, NULL)) > -1) {

	setoloc (x, ploc (mynum), IN_ROOM);
	setobjstate (a, 1);
      }
      return;
    }
    break;
#endif

#ifdef LOCMIN_TALON
  case OBJ_TALON_DESK:
    if (otstbit (OBJ_TALON_RUBY, OFL_DESTROYED) &&
	oloc (OBJ_TALON_RUBY) == LOC_TALON_TALON4) {
      create (OBJ_TALON_RUBY);
      bprintf ("You find a large, fiery ruby in one of the drawers.\n");
      send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
		mynum, NOBODY, "%s rummages around in the desk and turns up "
		"a large, fiery ruby.\n", pname (mynum));
      return;
    }
    break;
#endif

  case OBJ_TREEHOUSE_BEDDING:
    if (a != OBJ_TREEHOUSE_BEDDING)
      break;

    if (!odamage (OBJ_TREEHOUSE_AMULET)) {
      create (OBJ_TREEHOUSE_AMULET);
      bprintf ("You pull an amulet from the bedding.\n");
      osetdamage (OBJ_TREEHOUSE_AMULET, 1);
      return;
    }
    break;

  case OBJ_OAKTREE_BOARSKIN:
    if (a != OBJ_OAKTREE_BOARSKIN)
      break;

    if (!odamage (OBJ_OAKTREE_WHISTLE)) {
      create (OBJ_OAKTREE_WHISTLE);
      bprintf ("Under the boarskin you find a silver whistle.\n");
      osetdamage (OBJ_OAKTREE_WHISTLE, 1);
      return;
    }
    break;
  }

  if (oexam_text (a) != NULL) {
    bprintf ("%s", oexam_text (a));
  } else if (oexamine (a) != 0 && (x = fopen (OBJECTS, "r")) != NULL) {

    fseek (x, oexamine (a), 0);
    while ((ch = fgetc (x)) != '^')
      bprintf ("%c", ch);
    fclose (x);
  } else {
    bprintf ("You see nothing special.\n");
    return;
  }
}

void
incom (Boolean inv)
{
  int x, y;
  char st[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_GOTO)) {
    /* Mortals who doesn't have goto will assume that in = inventory... */
    /* Thus hiding the fact that there IS a command called in... */
    /* Must fix it so that at this should only be the case when IN is used */
    /* and not when AT is used */

    /* old: erreval(); */

    if (inv)
      inventory ();
    else
      bprintf ("Pardon?\n");
    return;
  }
  if ((x = getroomnum ()) == 0 || !exists (x)) {
    bprintf ("Unknown Player Or Room\n");
    return;
  }
  getreinput (st);
  if (EQ (st, "!")) {
    bprintf ("What exactly do you want to do?\n");
    return;
  }
  if (ltstflg (x, LFL_NOAT) && !ptstflg (mynum, PFL_IGNORENOAT)) {
    bprintf ("Something won't let you do that in that location.\n");
    return;
  }
  y = ploc (mynum);
  setploc (mynum, x);
  gamecom (st, False);
  setploc (mynum, y);
}

/* Hit the reset-stone.
 */
void
sys_reset (void)
{
  int i;
  int time = RESET_IDLE;

  for (i = 0; i < max_players; i++) {
    if (is_in_game (i) && plev (i) < LVL_WIZARD && ploc (i) != ploc (mynum) &&
	global_clock > plast_cmd (i) - time) {
      bprintf ("There are other players on who are not at this location,"
	       " so it won't work.\n");
      return;
    }
  }

  if (last_reset < global_clock && global_clock < last_reset + 3600) {
    bprintf ("Sorry, at least an hour must pass between resets.\n");
    return;
  }
  resetcom (0);
}


/* The RESET command.
 */
void
resetcom (int flags)
{
  if ((flags & RES_TEST) != 0 && !ptstflg (mynum, PFL_RESET)) {
    erreval ();
    return;
  }
  if (brkword () != -1 && plev (mynum) >= LVL_WIZARD) {
    int zone, r_locs, r_mobs, r_objs, nlocs, nmobs, nobjs, d_locs, d_mobs,
      d_objs;

    if ((zone = get_zone_by_name (wordbuf)) == -1) {
      bprintf ("&*No such zone: &+W%s\n", wordbuf);
      return;
    }
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	      "&+C[&+W\001p%s\003 &+whas reset zone &+W%s&+C]\n",
	      pname (mynum), zname (zone));

    mudlog ("RESET: Reset by %s (Zone: %s)", pname (mynum), zname (zone));

    nlocs = znumloc (zone);
    nobjs = znumobs (zone);
    nmobs = znumchars (zone);

    reset_zone (zone, &global_clock, &d_locs, &d_mobs, &d_objs,
		&r_locs, &r_mobs, &r_objs);

    if (!ztemporary (zone)) {
      bprintf ("&*Zone &+W%s&*: Reset Completed.\n", zname (zone));

      bprintf ("\n&+C%3d &*(from &+C%d&*) "
	       "Room(s) were successfully reset.\n",
	       r_locs, nlocs - d_locs);

      bprintf ("&+C%3d &*(from &+C%d&*) "
	       "Object(s) were successfully reset.\n",
	       r_objs, nobjs - d_objs);

      bprintf ("&+C%3d &*(from &+C%d&*) "
	       "Mobile(s) were successfully reset.\n",
	       r_mobs, nmobs - d_mobs);
    } else {
      bprintf ("&*Zone &+W%s&*:\n\n"
	       "&+C%d &*Room(s) destroyed.\n"
	       "&+C%d &*Objects(s) destroyed.\n"
	       "&+C%d &*Mobiles(s) destroyed.\n",
	       zname (zone),
	       nlocs, nobjs, nmobs);
    }
  } else {
    int i;

    last_reset = global_clock;
    setqdflags (0);

    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+C[&+wReset Called by &+W\001p%s\003&+C]\n", pname (mynum));

    send_msg (DEST_ALL, 0, LVL_MIN, LVL_WIZARD - 1, NOBODY, NOBODY,
	      "&+C[&+wReset Called&+C]\n");

    if (breset) {
      broad ("&+C[&+wSystem has been marked for Reboot, rebooting&+C]\n");
      run_reboot (False, False);
    }
    mudlog ("RESET: Reset by %s", pname (mynum));

    for (i = 0; i < numzon; i++) {
      reset_zone (i, &global_clock, NULL, NULL, NULL, NULL, NULL, NULL);
    }

    ++numresets;
    move_pouncie ();
    broad ("&+C[&+wReset Completed&+C]\n");
  }
}

void
unveilcom (char *unv_pass)
{
  int lev;

  if (!cur_player->isawiz) {
    erreval ();
    return;
  }
  if (unv_pass == NULL) {
    if (brkword () == -1) {
      cur_player->work = OPERATOR (pname (mynum)) ? LVL_MASTER : LVL_GOD;
    } else {
      cur_player->work = atoi (wordbuf);
    }
    strcpy (cur_player->cprompt, "Magic Word: ");

    /* IAC WILL ECHO */
    bprintf ("\n\377\373\001\001Magic Word: ");

    cur_player->no_echo = True;
    push_input_handler (unveilcom);
  } else {
    bprintf ("\377\374\001\001");
    cur_player->no_echo = False;
    pop_input_handler ();
    if (!EQ (unv_pass, UNVEIL_PASS)) {
      bprintf ("Eek!  Go away!&#\n");
      mudlog ("UNVEIL: (Incorrect Password) %s", pname (mynum));
    } else {
      lev = cur_player->work;
      if ((OPERATOR (pname (mynum)) && lev >= LVL_MAX) ||
	  (!OPERATOR (pname (mynum)) && lev >= LVL_MASTER)) {
	mudlog ("UNVEIL: (Level too High) %s to %d", pname (mynum), plev (mynum));
	bprintf ("The maximum level is %d.\n", OPERATOR (pname (mynum)) ?
		 LVL_MAX - 1 : LVL_MASTER - 1);
      } else {
	set_xpflags (lev, &pflags (mynum), &pmask (mynum));
	setplev (mynum, lev);
	update_wizlist (pname (mynum), wlevel (lev));
	mudlog ("UNVEIL: %s to %d", pname (mynum), plev (mynum));
	bprintf ("Certainly, master!\n");
      }
    }
    get_command (NULL);
  }
}


void
posecom (void)
{
  char x[128];
  int n;
  int m = 0;

  char *POSE[] =
  {"gestures", "fireball", "hamster",
   "sizzle", "crackle", TABLE_END};

  if (plev (mynum) < LVL_MAGICIAN) {
    bprintf ("You're not up to this yet.\n");
    return;
  }
  if (brkword () != -1) {
    if (plev (mynum) < LVL_WIZARD) {
      bprintf ("Usage: POSE\n");
      return;
    }
    if (((n = atoi (wordbuf)) > 5 || n < 1) &&
	(m = tlookup (wordbuf, POSE)) == -1) {
      bprintf ("Usage: POSE <gestures/fireball/hamster/sizzle/crackle>\n");
      return;
    }
    if (m != -1)
      n = m;
    else
      --n;
  } else
    n = randperc () % 5;

  switch (n) {
  case 0:
    sprintf (x, "\001s%%s\003%%s raises %s arms in mighty magical "
	     "invocations.\n\003", his_or_her (mynum));
    sillycom (x);
    bprintf ("You make mighty magical gestures.\n");
    break;
  case 1:
    sillycom ("\001s%s\003%s throws out one arm and sends a huge bolt of "
	      "fire high into the sky.\n\003");
    bprintf ("You toss a fireball high into the sky.\n");
    broad ("\001cA massive ball of fire explodes high up in the sky.\n\003");
    break;
  case 2:
    sillycom ("\001s%s\003%s turns casually into a hamster before resuming "
	      "normal shape.\n\003");
    bprintf ("You casually turn into a hamster before resuming normal "
	     "shape.\n");
    break;
  case 3:
    sillycom ("\001s%s\003%s starts sizzling with magical energy.\n\003");
    bprintf ("You sizzle with magical energy.\n");
    break;
  case 4:
    sillycom ("\001s%s\003%s begins to crackle with magical fire.\n\003");
    bprintf ("You crackle with magical fire.\n");
    break;
  }
}

/* Brian Preble -- shows current set*in/set*out settings.
 * (Improved by Alf. Extended to include other players setins by Nicknack.)
 */
void
reviewcom (void)
{
  char xx[SETIN_MAX + 200];
  SETIN_REC s;
  PERSONA p;
  int x;
  Boolean me = False;
  PLAYER_REC *v = NULL;
  char *in_ms, *out_ms, *min_ms, *mout_ms;
  char *vin_ms, *vout_ms, *qin_ms, *qout_ms;
  char *sit_ms, *sta_ms, *sum_ms, *sin_ms, *sout_ms;
  char *pro;
  char *name;

  if (brkword () == -1 || EQ (wordbuf, pname (mynum))) {
    me = True;
    v = cur_player;
    x = mynum;
  } else if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  } else if ((x = fpbns (wordbuf)) >= 0 && x < max_players &&
	     plev (x) >= LVL_WIZARD) {
    v = players + x;
  } else if (!getuaf (wordbuf, &p) || p.p_level < LVL_WIZARD) {
    bprintf ("I know no wizard with the name '%s'.\n", wordbuf);
    return;
  } else if (!getsetins (wordbuf, &s)) {
    bprintf ("%s has no non-standard travel-messages.\n", p.p_name);
    return;
  }
  if (v != NULL) {
    in_ms = v->setin;
    out_ms = v->setout;
    min_ms = v->setmin;
    mout_ms = v->setmout;
    vin_ms = v->setvin;
    vout_ms = v->setvout;
    qin_ms = v->setqin;
    qout_ms = v->setqout;
    pro = v->prompt;
    sit_ms = v->setsit;
    sta_ms = v->setstand;
    sum_ms = v->setsum;
    sin_ms = v->setsumin;
    sout_ms = v->setsumout;
    name = pname (x);
  } else {
    in_ms = s.setin;
    out_ms = s.setout;
    min_ms = s.setmin;
    mout_ms = s.setmout;
    vin_ms = s.setvin;
    vout_ms = s.setvout;
    qin_ms = s.setqin;
    qout_ms = s.setqout;
    pro = s.prompt;
    sit_ms = s.setsit;
    sta_ms = s.setstand;
    sum_ms = s.setsum;
    sin_ms = s.setsumin;
    sout_ms = s.setsumout;
    name = p.p_name;
  }

  bprintf ("Current travel messages");
  if (!me)
    bprintf (" for %s", name);
  bprintf (":\n------------------------\n");
  bprintf ("SETIN     : %s\n", build_setin (xx, in_ms, name, NULL, NULL));
  bprintf ("SETOUT    : %s\n", build_setin (xx, out_ms, name, "<dir>", NULL));
  bprintf ("SETMIN    : %s\n", build_setin (xx, min_ms, name, NULL, NULL));
  bprintf ("SETMOUT   : %s\n", build_setin (xx, mout_ms, name, NULL, NULL));
  bprintf ("SETVIN    : %s\n", build_setin (xx, vin_ms, name, NULL, NULL));
  bprintf ("SETVOUT   : %s\n", build_setin (xx, vout_ms, name, NULL, NULL));
  bprintf ("SETQIN    : %s\n", build_setin (xx, qin_ms, name, NULL, NULL));
  bprintf ("SETQOUT   : %s\n", build_setin (xx, qout_ms, name, NULL, NULL));
  bprintf ("SETSIT    : %s\n", build_setin (xx, sit_ms, name, NULL, NULL));
  bprintf ("SETSTND   : %s\n", build_setin (xx, sta_ms, name, NULL, NULL));
  bprintf ("SETSUM    : %s\n", build_setin (xx, sum_ms, name, NULL, NULL));
  bprintf ("SETSIN    : %s\n", build_setin (xx, sin_ms, name, NULL, "<victim>"));
  bprintf ("SETSOUT   : %s\n", build_setin (xx, sout_ms, name, NULL, "<victim>"));
  bprintf ("PROMPT    : %s\n", pro);
}

void
scorecom (void)
{
  int plx = mynum;

  if (brkword () != -1) {
    if ((plx = fpbn (wordbuf)) < 0) {
      bprintf ("Who?\n");
      return;
    }
    if (plev (mynum) < LVL_WIZARD) {
      bprintf ("That is beyond your powers.\n");
      return;
    }
  }
  bprintf ("%s%s Statistics:\n", (plx == mynum) ? "Your" : pname (plx),
	   (plx == mynum) ? "" : "'s");

  if (plx < max_players) {
    bprintf ("&+w---------------------------------------------------------\n");
    bprintf ("&+wScore:    &+W%d\n", pscore (plx));

    if (plev (plx) < LVL_WIZARD) {
      if (pscore (plx) < levels[LVL_WIZARD]) {
	bprintf ("&+wNext Lvl: &+W%d\n", (levels[plev (plx) + 1] - pscore (plx)));
      } else {
	bprintf ("&+wNext Lvl: &+WNo Points Needed\n");
      }
    }

    bprintf ("&+wHealth:   %s%d&+w/&+W%d\n", str_color (plx), pstr (plx), maxstrength (plx));
    bprintf ("&+wMana:     %s%d&+w/&+W%d\n", mag_color (plx), pmagic (plx), maxmagic (plx));
    bprintf ("&+wDamage:   &+W%d\n", player_damage (plx));
    bprintf ("&+wArmor:    &+W%d&+w/&+W%d\n", player_armor (plx), MAXARMOR);
    bprintf ("&+wKills:    &+W%d\n", pkilled (plx));
    bprintf ("&+wDeaths:   &+W%d\n", pdied (plx));
    bprintf ("&+wRank:     &+W%s\n", make_rank (plx));
    bprintf ("&+w---------------------------------------------------------\n");
  } else {
    bprintf ("&+w---------------------------------------------------------\n");
    if (pstr (plx) <= 0)
      bprintf ("&+wHealth: &+RDead\n");
    else
      bprintf ("&+wHealth: %s%d&+w/&+W%d\n", str_color (plx), pstr (plx), pstr_reset (plx));
    bprintf ("&+wDamage: &+W%d\n", player_damage (plx));
    bprintf ("&+wArmor:  &+W%d\n", player_armor (plx));
    bprintf ("&+w---------------------------------------------------------\n");
  }
}

void
sitcom (void)
{
  if (psitting (mynum)) {
    bprintf ("You're already sitting.\n");
    return;
  } else if (pfighting (mynum) >= 0) {
    bprintf ("You want to sit down while fighting?  Do you have a death "
	     "wish or something?\n");
    return;
  }
#ifdef LOCMIN_PLAYHOUSE
  if (ploc (mynum) == oloc (OBJ_PLAYHOUSE_TOILET)) {
    bprintf ("You sit down upon the Porcelain Throne.\n");
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s sits upon the Porcelain Throne.\n", pname (mynum));
    setpsitting (mynum, 1);
    return;
  }
#endif
  if (ploc (mynum) == LOC_ICECAVE_THRONE) {
    if (oloc (OBJ_ICECAVE_WESTICETHRONE)) {
      bprintf ("The throne collapses under your weight, revealing a "
	       "passage east.\n");
      setobjstate (OBJ_ICECAVE_WESTICETHRONE, 0);
      return;
    }
  }
  bprintf ("You assume the lotus position.\n");
  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "%s sits down.\n", pname (mynum));
  setpsitting (mynum, 1);
}

void
standcom (int player)
{
  if (!psitting (player)) {
    sendf (player, "You're already standing.\n");
    return;
  } else {
    sendf (player, "You clamber to your feet.\n");
    send_msg (ploc (player), 0, pvis (player), LVL_MAX, player, NOBODY,
	  "%s clambers to %s feet.\n", pname (player), his_or_her (player));
    setpsitting (player, 0);
  }
}

void
pncom (void)
{
  int plx = real_mynum;
  char *p;

  if (brkword () == -1 || (plx = fpbn (wordbuf)) == -1 ||
      plx >= max_players || (plx != mynum && plev (mynum) < LVL_WIZARD)) {
    plx = real_mynum;
  }
  if (plx == real_mynum) {
    bprintf ("Current pronouns are:\n");
  } else {
    bprintf ("Current pronouns for %s are:\n", pname (plx));
  }
  bprintf ("Me    : %s\n", pname (plx));
  bprintf ("It    : %s\n", (p = players[plx].wd_it) == NULL ? "<none>" : p);
  bprintf ("Him   : %s\n", (p = players[plx].wd_him) == NULL ? "<none>" : p);
  bprintf ("Her   : %s\n", (p = players[plx].wd_her) == NULL ? "<none>" : p);
  bprintf ("Them  : %s\n", (p = players[plx].wd_them) == NULL ? "<none>" : p);
}

void
bugcom (void)
{
  char x[MAX_COM_LEN];

  getreinput (x);
  if (EMPTY (txt1)) {
    bprintf ("What do you want to bug me about?\n");
  } else {
    mudlog ("BUG: %s - %s", pname (mynum), x);
    bprintf ("Ok.\n");
  }
}

void
typocom (void)
{
  int l = ploc (mynum);
  char x[MAX_COM_LEN];

  getreinput (x);
  if (EMPTY (x)) {
    bprintf ("What typo do you wish to inform me of?\n");
  } else {
    mudlog ("TYPO: %s (%s): %s", pname (mynum), showname (l), x);
    bprintf ("Ok.\n");
  }
}

void
helpcom (void)
{
  int a, b;

  if (item1[0] != 0) {
    if (showhelp (item1) < 0)
      return;
    if ((a = vichere ()) < 0)
      return;
    if ((b = phelping (mynum)) >= 0) {
      sillytp (b, "stopped helping you.");
      bprintf ("Stopped helping \001p%s\003.\n", pname (b));
    }
    if (a == mynum) {
      bprintf ("You are beyond help.\n");
      return;
    }
    setphelping (mynum, a);
    sillytp (a, "has offered to help you.");
    bprintf ("Started helping \001p%s\003.\n", pname (phelping (mynum)));
    return;
  }
  bprintf ("\001f%s/%s\003", HELP_DIR, HELP1);
  if (plev (mynum) < LVL_WIZARD) {
    return;
  }
  bprintf (qwait);
  replace_input_handler (help2);
}

void
help2 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP2);
    bprintf ("\n");
    if (plev (mynum) < LVL_PROPHET) {
      get_command (NULL);
    } else {
      bprintf (qwait);
      replace_input_handler (help3);
    }
  }
}

void
help3 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP3);
    bprintf ("\n");
    if (plev (mynum) < LVL_ARCHWIZARD) {
      get_command (NULL);
    } else {
      bprintf (qwait);
      replace_input_handler (help4);
    }
  }
}

void
help4 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP4);
    bprintf ("\n");
    if (plev (mynum) < LVL_ADVISOR) {
      get_command (NULL);
    } else {
      bprintf (qwait);
      replace_input_handler (help5);
    }
  }
}

void
help5 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP5);
    bprintf ("\n");
    if (plev (mynum) < LVL_AVATAR) {
      get_command (NULL);
    } else {
      bprintf (qwait);
      replace_input_handler (help6);
    }
  }
}

void
help6 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP6);
    bprintf ("\n");
    if (plev (mynum) < LVL_GOD) {
      get_command (NULL);
    } else {
      bprintf (qwait);
      replace_input_handler (help7);
    }
  }
}

void
help7 (char *cont)
{
  if (cont[0] == 'q') {
    get_command (NULL);
  } else {
    bprintf ("\001f%s/%s\003", HELP_DIR, HELP7);
    bprintf ("\n");
    get_command (NULL);
  }
}

int
showhelp (char *verb)
{
  char file[130];
  char line[80];
  int scanreturn = EOF;
  int v;
  FILE *fp;

  sprintf (file, "../bin/pfilter %d 0x%08lx:0x%08lx:0x%08lx 0x%08lx:0x%08lx:0x%08lx "
	   HELP_DIR "/" FULLHELP, plev (mynum), pflags (mynum).u,
	   pflags (mynum).h, pflags (mynum).l, pmask (mynum).u,
	   pmask (mynum).h, pmask (mynum).l);

  if ((fp = popen (file, "r")) == NULL) {
    bprintf ("Someone's editing the help file.\n");
    return -1;
  }
  ssetflg (mynum, SFL_BUSY);
  pbfr ();
  v = strlen (verb);
  while (True) {
    if (fgets (line, sizeof (line), fp) == NULL) {
      scanreturn = EOF;
      break;
    }
    if ((scanreturn = strncasecmp (line, verb, v)) == 0)
      break;
    do {
      if (fgets (line, sizeof (line), fp) == NULL) {
	scanreturn = EOF;
	break;
      }
    }
    while (strcmp (line, "^\n") != 0);
  }

  if (scanreturn != 0) {	/* command not found in extern list */
    pclose (fp);
    sclrflg (mynum, SFL_BUSY);
    return 0;
  }
  bprintf ("\nUsage: %s\n", line);
  while (fgets (line, sizeof (line), fp) && strcmp (line, "^\n") != 0) {
    bprintf ("%s", line);
  }
  bprintf ("\n");

  pclose (fp);
  sclrflg (mynum, SFL_BUSY);
  return -1;
}




/* The QUESTS-Command. Mortals may find out which quests they have
 * comleted and which are left. Arch-Wizards may in addition set or
 * clear quests for a mortal, Wizards only for themselves.
 * Usage: QUESTS <player> <questname> <true/false>
 */
void
questcom (void)
{
  int a, b, c, l;
  char *n;
  Boolean f, all;
  QFLAGS q, *p;
  PERSONA d;

  f = False;

  if (brkword () == -1) {
    a = mynum;
    p = &(qflags (a));
    n = pname (a);
    l = plev (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    if (a != mynum && plev (mynum) < LVL_WIZARD) {
      bprintf ("You can only check your own Quest-stats.\n");
      return;
    }
    p = &(qflags (a));
    n = pname (a);
    l = plev (a);
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else if (plev (mynum) < LVL_WIZARD) {
    bprintf ("You can only check your own Quest-stats.\n");
    return;
  } else {
    f = True;
    p = &(d.p_quests);
    n = d.p_name;
    l = d.p_level;
  }

  if (brkword () == -1) {
    bprintf ("\nPlayer: %s\n\n", n);

    all = False;
    bprintf ("Completed Quests:\n");
    if ((*p & Q_ALL) == Q_ALL) {
      bprintf ("All!\n");
      all = True;
    } else if ((*p & Q_ALL) != 0) {
      show_bits ((int *) p, sizeof (QFLAGS) / sizeof (int), Quests);
    } else {
      bprintf ("None!\n");
    }
    if (!all) {
      bprintf ("\nStill to do:\n");
      q = (~*p & Q_ALL);
      show_bits ((int *) &q, sizeof (QFLAGS) / sizeof (int), Quests);
    }
    return;
  } else if ((b = tlookup (wordbuf, Quests)) == -1) {
    bprintf ("%s: No such Quest.\n", wordbuf);
    return;
  } else if (brkword () == -1 || plev (mynum) < LVL_WIZARD) {
    c = xtstbit (*p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Quests[b], TF[c]);
    return;
  } else if (plev (mynum) < LVL_ARCHWIZARD && !EQ (n, pname (mynum))) {
    bprintf ("You can't change other players Quest-stats.\n");
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  mudlog ("QUEST: %s by %s, %s = %s", n, pname (mynum), Quests[b], TF[c]);

  if (c == 0) {
    xclrbit (*p, b);
  } else {
    xsetbit (*p, b);
  }
  if (f) {
    putuaf (&d);
  }
}

/* The INFO command.
 */
void
infocom (void)
{
  char file[100];

  if (brkword () == -1) {
    strcpy (file, INFO_DIR "/" INFO);
  } else {
    if (strlen (wordbuf) >= (sizeof (file) - 10)) {
      bprintf ("No info available on that topic.\n");
      return;
    }
    sprintf (file, "%s/%s.i", INFO_DIR, lowercase (wordbuf));
    if (access (file, R_OK) < 0) {
      bprintf ("No info available on that topic.\n");
      return;
    }
  }

  file_pager (file);
}

/* The POLICY command.
 */
void
policycom (void)
{
  char file[100];

  if (brkword () == -1) {
    strcpy (file, POLICY_DIR "/" POLICY);
  } else {
    if (strlen (wordbuf) >= (sizeof (file) - 10)) {
      bprintf ("No info available on that topic.\n");
      return;
    }
    sprintf (file, "%s/%s.p", POLICY_DIR, lowercase (wordbuf));
    if (access (file, R_OK) < 0) {
      bprintf ("No such policy exists.\n");
      return;
    }
  }
  file_pager (file);
}

/* The GLOBAL command.
 */
void
globalcom (void)
{
  char buff[80];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  } else if (plev (mynum) >= LVL_GOD && dump_pflags ())
    return;

  bprintf ("&+CGlobal Settings:\n");
  bprintf ("&+B---------------------------------------------------------------------------\n");
  bprintf ("&+WWeather (Season)  : &+C%s (%s)\n",
	   weather_desc (the_climate->weather),
	   season_name (the_climate->season));
  bprintf ("&+WLock-status       : &+C%slocked\n",
	   lev2s (buff, the_world->w_lock));
  bprintf ("&+WNoRun Status      : &+C%s\n", norun ? "Enabled" : "Disabled");
  bprintf ("&+WMobiles           : &+C%s\n",
	   (the_world->w_mob_stop) ? "Stopped" : "Started");
  bprintf ("&+WWar/Peace         : &+C%s\n",
	   (the_world->w_peace) ? "Peace" : "War");
  bprintf ("&+WTournament-Mode   : &+C%s\n",
	   (the_world->w_tournament) ? "On" : "Off");
  bprintf ("&+WReboot on Reset   : &+C%s\n", breset ? "Yes" : "No");
  bprintf ("&+WMax. Users        : &+C%d\n", max_players);
  bprintf ("&+WNumber of Rooms   : "
     "&+C%4d &*permanent +&+C%4d &*wiz-made = &+C%4d &*(&+WMax: &+C%d&*)\n",
	   num_const_locs, numloc - num_const_locs, numloc, GLOBAL_MAX_LOCS);
  bprintf ("&+WNumber of Mobiles : "
     "&+C%4d &*permanent +&+C%4d &*wiz-made = &+C%4d &*(&+WMax: &+C%d&*)\n",
	   num_const_chars - max_players, numchars - num_const_chars,
	   numchars - max_players, GLOBAL_MAX_MOBS);
  bprintf ("&+WNumber of Objects : "
     "&+C%4d &*permanent +&+C%4d &*wiz-made = &+C%4d &*(&+WMax: &+C%d&*)\n",
	   num_const_obs, numobs - num_const_obs, numobs, GLOBAL_MAX_OBJS);
  bprintf ("&+B---------------------------------------------------------------------------\n");

}

static void
do_pretend (int plx)
{
  int p;

  if ((p = cur_player->pretend) >= 0) {
    /* We are someone else, unalias him */
    sclrflg (p, SFL_OCCUPIED);
    mynum = real_mynum;
    cur_player->pretend = -1;
  }
  if (plx < 0) {
    /* Back to ourselves */
    if (p < 0)
      return;			/* we already are ourselves */

    strcpy (cur_player->setin, cur_player->o_setin);
    strcpy (cur_player->setout, cur_player->o_setout);
  } else {
    /* We will pretend to be plx from now */
    mynum = plx;
    cur_player->pretend = plx;
    ssetflg (plx, SFL_OCCUPIED);
    if (p < 0) {
      strcpy (cur_player->o_setin, cur_player->setin);
      strcpy (cur_player->o_setout, cur_player->setout);
    }
    if (plx >= max_players) {
      strcpy (cur_player->setin, "%n has arrived.");
      strcpy (cur_player->setout, "%n has gone %d.");
    } else {
      strcpy (cur_player->setin, players[plx].setin);
      strcpy (cur_player->setout, players[plx].setout);
    }
  }
}

int
find_pretender (int plx)
{
  int p;

  if (ststflg (plx, SFL_OCCUPIED)) {
    for (p = 0; p < max_players; p++) {
      if (players[p].pretend == plx && is_in_game (p)) {
	return p;
      }
    }
  }
  return -1;
}

void
aliascom (void)
{
  if (pl1 == -1) {
    if (ptstflg (mynum, PFL_ALIAS) || ptstflg (mynum, PFL_ALIASP)) {
      bprintf ("Who?\n");
    } else {
      erreval ();
    }
    return;
  }
  if (cur_player->polymorphed >= 0) {
    bprintf ("A mysterious force stops you.\n");
    return;
  }
  if (ststflg (pl1, SFL_OCCUPIED)) {
    bprintf ("Already occupied!\n");
    return;
  }
  if (pl1 < max_players) {
    if (!ptstflg (mynum, PFL_ALIASP)) {
      bprintf ("You can't become another player!\n");
      return;
    }
    if (!do_okay (mynum, pl1, PFL_NOALIAS)) {
      if (players[pl1].asmortal > 0 && plev (mynum) <= LVL_WIZARD)
	bprintf ("Already occupied!\n");
      else
	bprintf ("They don't want to be aliased!\n");
      return;
    }
  } else if (!ptstflg (mynum, PFL_ALIAS)) {
    bprintf ("You can't become a mobile!\n");
    return;
  }
  cur_player->aliased = True;
  cur_player->aliasto = pl1;
  do_pretend (pl1);
  bprintf ("Aliased to %s.\n", pname (pl1));
}

void
polymorph (int plx, int turns)
{
  /* Polymorph to PLX for TURNS turns. */
  if (plx < 0 || turns < 0) {
    unpolymorph (real_mynum);
    return;
  }
  do_pretend (plx);
  cur_player->polymorphed = turns;
  bprintf ("You pass out.....\n..... and wake up.\n");
}


void
unalias (int pl)
{
  int me = real_mynum;

  setup_globals (pl);
  if (cur_player->aliased) {
    do_pretend (-1);
    cur_player->aliased = False;
    bprintf ("Back to good old %s....\n", pname (mynum));
  }
  setup_globals (me);
}

void
unpolymorph (int pl)
{
  int me = real_mynum;

  setup_globals (pl);
  if (cur_player->polymorphed >= 0) {
    do_pretend (-1);
    cur_player->polymorphed = -1;
    setpfighting (mynum, -1);
    bprintf ("Suddenly you awake... were you dreaming or what?\n");
  }
  setup_globals (me);
}



Boolean
disp_file (char *fname, FILE * f)
{
  char buff[BUFSIZ];

  if (f == NULL && (f = fopen (fname, "r")) == NULL) {
    return False;
  }
  while (fgets (buff, BUFSIZ, f) != NULL) {
    bprintf ("%s", buff);
  }
  fclose (f);
  return True;
}




void
becom (char *passwd)
{
  char x[128];
  PERSONA p;

  if (passwd == NULL) {

    if (cur_player->polymorphed != -1 || cur_player->aliased) {
      bprintf ("Not when aliased.");
      return;
    }
    if (cur_player->writer != NULL) {
      bprintf ("Finish whatever you're writing first!\n");
      return;
    }
    if (pfighting (mynum) != -1) {
      bprintf ("Not while fighting!\n");
      return;
    }
    if (brkword () == -1) {
      bprintf ("Become what?  Inebriated?\n");
      return;
    }
    strcpy (cur_player->work2, wordbuf);
    strcpy (cur_player->cprompt, "Password: ");
    cur_player->no_echo = True;

    bprintf ("\n\377\373\001\001Password: ");
    push_input_handler (becom);
  } else {
    bprintf ("\377\374\001\001");
    cur_player->no_echo = False;
    pop_input_handler ();

    if (!getuaf (cur_player->work2, &p)) {
      bprintf ("No such player.\n");
    } else if (!EQ (p.p_passwd, my_crypt (x, passwd, strlen (p.p_passwd) + 1))) {
      bprintf ("Incorrect password!\n");
    } else if (fpbns (cur_player->work2) >= 0) {
      bprintf ("That player is allready on the game.\n");
    } else {
      close_plr_log ();
      saveme ();
      remove_entry (mob_id (mynum), &id_table);

      pers2player (&p, mynum);
      strcpy (cur_player->cprompt, cur_player->prompt);
      mudlog ("SYSTEM: %s has BECOME %s", pname (mynum), p.p_name);

      send_msg (DEST_ALL, MODE_QUIET, max (LVL_WIZARD, pvis (mynum)), LVL_MAX,
		mynum, NOBODY, "&+B[&+W%s &*has &+CBECOME &+W%s&+B]\n", pname (mynum), p.p_name);
      send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		"%s disintegrates, and reappears as %s.\n",
		pname (mynum), p.p_name);

      setpname (mynum, p.p_name);
      fetchprmpt (mynum);
      insert_entry (mob_id (mynum), mynum, &id_table);

      bprintf ("You are now %s.\n", pname (mynum));
      check_files ();

      if (privileged_user (pname (mynum)))
	cur_player->isawiz = True;

      if (is_monitored (pname (mynum))) {
	open_plr_log ();
      }
    }
    get_command (NULL);
  }
}

void
dircom (void)
{
  int a, c, oc = 0;
  char b[40], d[40];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  for (a = 0; a < numobs; a++) {
    oc++;
    c = findzone (oloc (a), b);
    sprintf (d, "%s%d", b, c);
    if (ocarrf (a) >= CARRIED_BY)
      strcpy (d, "Carried");
    else if (ocarrf (a) == IN_CONTAINER)
      strcpy (d, "In item");
    bprintf ("%-13s%-13s", oname (a), d);
    if (a % 3 == 2)
      bprintf ("\n");
  }
  bprintf ("\nTotal of %d objects.\n", oc);
}


void
treasurecom (void)
{
  int a, c, tc = 0;
  char b[40], d[40];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  for (a = 0; a < numobs; a++) {
    if (!oflannel (a) && !otstbit (a, OFL_DESTROYED) && ocarrf (a) == IN_ROOM) {
      tc++;
      c = findzone (oloc (a), b);
      sprintf (d, "%s%d", b, c);
      bprintf ("%-13s%-13s", oname (a), d);
      if (tc % 3 == 0)
	bprintf ("\n");
      if (tc % 18 == 17)
	pbfr ();
    }
  }
  bprintf ("\nApproximately %d treasures remain.\n", tc);
}



/* The LEVELS command. Display the number of points required to reach each
 * new level together with the corresponding male and female titles.
 */
void
levelscom (void)
{
  int i;

  bprintf ("Level   Points        Male                    Female\n");
  bprintf ("=====   ======        ====                    ======\n");

  for (i = 1; i <= LVL_WIZARD; i++) {

    bprintf ("%-5d   %6d        the %-17s   the %-20s\n",
	     i, levels[i], MLevels[i], FLevels[i]);
  }
  bprintf ("\n");
}


void
promptcom (void)
{
  char buff[MAX_COM_LEN];

  getreinput (buff);
  if (strlen (buff) > PROMPT_LEN) {
    bprintf ("Max. prompt length is %d.\n", PROMPT_LEN);
    return;
  }
  strcpy (cur_player->prompt, buff);
  bprintf ("Ok\n");

}

void
followcom (void)
{
  if (cur_player->i_follow >= 0) {
    bprintf ("Stopped following \001P%s\003.\n", pname (cur_player->i_follow));
    sendf (cur_player->i_follow, "\001p%s\003 has stopped following you.\n", pname (mynum));
    cur_player->i_follow = -1;
    return;
  }
  if (pfighting (mynum) != -1) {
    bprintf ("Not in a fight!\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Who?\n");
    return;
  }
  if (pl1 == -1) {
    bprintf ("Who?\n");
    return;
  }
  if (ploc (pl1) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return;
  }
  if (mynum == pl1) {
    bprintf ("You run round in circles!\n");
    return;
  }
  sendf (pl1, "%s is following you.\n", see_name (pl1, mynum));
  cur_player->i_follow = pl1;
  bprintf ("Ok\n");
}

void
losecom (void)
{
  if (cur_player->i_follow == -1) {
    bprintf ("You aren't following anyone.\n");
    return;
  }
  sendf (cur_player->i_follow, "\001p%s\003 has stopped following you.\n", pname (mynum));
  cur_player->i_follow = -1;
  bprintf ("Ok\n");
}

void
togglecom (int flg, char on[80], char off[80])
{
  Boolean away = (flg == SFL_AWAY);
  char buffer[MAX_COM_LEN];
  int size = sizeof (cur_player->awaymsg);

  if ((flg == SFL_QUIET || flg == SFL_ALOOF) && (plev (mynum) < LVL_WIZARD)) {
    erreval ();
    return;
  }
  if (!ststflg (mynum, flg)) {
    ssetflg (mynum, flg);
    bprintf ("%s\n", on);

    if (away) {
      if (EMPTY (txt1))
	sprintf (buffer, "No particular reason.");
      else
	getreinput (buffer);

      sprintf (cur_player->awaymsg, "%.*s", size, buffer);
      send_msg (DEST_ALL, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		"&+G[&+W%s &*has walked away from %s keyboard&+G]\n",
		pname (mynum), his_or_her (mynum));
      send_msg (DEST_ALL, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		"&+G[&+WReason: &*%s&+G]\n", cur_player->awaymsg);
      bprintf ("Reason: %s\n", cur_player->awaymsg);
    }
  } else {
    sclrflg (mynum, flg);
    bprintf ("%s\n", off);

    if (away) {
      send_msg (DEST_ALL, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		"&+G[&+W%s &*has returned to %s keyboard&+G]\n",
		pname (mynum), his_or_her (mynum));
    }
  }
}

/* QDONE: Shows quests done so far during the current reset.
 * 1995 by Illusion
 */
void
qdonecom ()
{
  int i;

  bprintf ("&+WQuest Status For This Reset:\n");
  bprintf ("&+B---------------------------------------------------\n");
  for (i = 0; i <= Q_LAST_QUEST; ++i)
    bprintf ("&+w%-30s &+W%20s\n", Quests[i],
	     qdtstflg (i) ? "Done" : "Not Completed");
  bprintf ("&+B---------------------------------------------------\n");
}

void
beepcom (void)
{
  int a;

  if (brkword () == -1) {
    bprintf ("Who do you want to beep?\n");
    return;
  }
  if ((a = fpbn (wordbuf)) == -1) {
    bprintf ("I can't seem to find that player.\n");
    return;
  }
  if (a >= max_players) {
    bprintf ("Beep a mobile? Why?\n");
    return;
  }
  if (a == mynum) {
    bprintf ("You want to beep yourself? Ok then!&#&#\n");
    return;
  }
  if (check_forget (a, mynum)) {
    bprintf ("&+W%s &*has forgotten you and will not receive your beeps.\n", pname (a));
    return;
  }
  if (ltstflg (ploc (a), LFL_SOUNDPROOF)) {
    bprintf ("%s seems to be in a soundproof room.\n", pname (a));
    return;
  }
  bprintf ("Beeping %s.\n", pname (a));
  sendf (a, "\001p%s\003 is trying to get your attention!&#&#\n", pname (mynum));
}

void
togglefinger (void)
{
  if (!ststflg (mynum, SFL_NOFINGER)) {
    ssetflg (mynum, SFL_NOFINGER);
    bprintf ("You can no longer be checked on by other users.\n");
  } else {
    sclrflg (mynum, SFL_NOFINGER);
    bprintf ("You can now be checked on by other users.\n");
  }
}

void
fingercom (void)
{
  int c, b;
  PERSONA p;
  SFLAGS *s;

  if (brkword () == -1) {
    togglefinger ();
    return;
  }
  if ((b = fpbns (wordbuf)) != -1 && seeplayer (b)) {
    if (b >= max_players)
      bprintf ("Why would you want to check the last time a mobile logged in?\n");
    else
      bprintf ("They are currently online.\n");

    return;
  }
  if (!getuaf (wordbuf, &p)) {
    bprintf ("Person not found in system.\n");
    return;
  }
  s = &(p.p_sflags);
  c = tst_bit (s, SFL_NOFINGER) ? 1 : 0;

  if (c && plev (mynum) < LVL_AVATAR) {
    bprintf ("That player cannot be finger'd.\n");
    return;
  }
  bprintf ("&+wPlayer: &+C%s\n", p.p_name);
  bprintf ("&+wLast Login: &+C%s\n", time2ascii (p.p_last_on));

  if (ptstflg (mynum, PFL_SHUSER))
    bprintf ("&+wFrom Host: &+C%s\n", !ptstflg (mynum, PFL_SEEUSER) ?
	     p.p_last_host : p.p_usrname);

  bprintf ("\n");
  check_mail (p.p_name);
}

void
setpager (void)
{
  int new;

  if (brkword () == -1) {
    bprintf ("&+wPager is currently set to &+C%d &+wlines.\n", ppager (mynum));
    return;
  }
  new = atoi (wordbuf);

  if (new > 200) {
    bprintf ("Do you really need the pager set that high?\n");
    return;
  }
  if (new < 0) {
    bprintf ("Do you really need the pager set that low?\n");
    return;
  }
  setppager (mynum, new);

  if (new) {
    bprintf ("&+wSetting pager to &+C%d &+wlines.\n", new);
  } else {
    bprintf ("&+wTurning pager &+COff&+w.\n");
  }
}

void
optionscom (void)
{
  char pager[20], chan[20], wimpy[20];

  sprintf (pager, "&+C%d Lines", ppager (mynum));
  sprintf (chan, "&+C%d", pchannel (mynum));
  sprintf (wimpy, "&+C%d", pwimpy (mynum));

  bprintf ("&+WYour current game options:\n");
  bprintf ("&+B---------------------------------------------------------\n");
  bprintf ("&+wColor          %-11s       &+wNewStyle       %-11s\n",
	   ststflg (mynum, SFL_COLOR) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NEWSTYLE) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wBrief          %-11s       &+wAway           %-11s\n",
	   ststflg (mynum, SFL_BRIEF) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_AWAY) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wBusy           %-11s       &+wNoFight        %-11s\n",
	   ststflg (mynum, SFL_BUSY) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NOFIGHT) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wAutoExit       %-11s       &+wNoFinger       %-11s\n",
	   ststflg (mynum, SFL_AUTOEXIT) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NOFINGER) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wNoInventory    %-11s       &+wNoBeep         %-11s\n",
	   ststflg (mynum, SFL_NOINV) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NOBEEP) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wHearBack       %-11s       &+wNoBlink        %-11s\n",
	   ststflg (mynum, SFL_HEARBACK) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NOBLINK) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wNoFlag         %-11s       &+wNoOracle       %-11s\n",
	   ststflg (mynum, SFL_NOFLAG) ? "&+CEnabled" : "&+cDisabled",
	   ststflg (mynum, SFL_NOORACLE) ? "&+CEnabled" : "&+cDisabled");
  bprintf ("&+wNoWeather      %-11s       &+wWimpy          %-11s\n",
	   ststflg (mynum, SFL_NOWET) ? "&+CEnabled" : "&+cDisabled",
	   pwimpy (mynum) == 0 ? "&+cDisabled" : wimpy);
  bprintf ("&+wChat Channel   %-11s       &+wPager          %-11s\n",
	   chan, ppager (mynum) == 0 ? "&+cDisabled" : pager);

  if (plev (mynum) >= LVL_WIZARD) {
    bprintf ("&+wQuiet          %-11s       &+wAloof          %-11s\n",
	     ststflg (mynum, SFL_QUIET) ? "&+CEnabled" : "&+cDisabled",
	     ststflg (mynum, SFL_ALOOF) ? "&+CEnabled" : "&+cDisabled");
    bprintf ("&+wNoWish         %-11s       &+wSeeExtended    %-11s\n",
	     ststflg (mynum, SFL_NOWISH) ? "&+CEnabled" : "&+cDisabled",
	     ststflg (mynum, SFL_SEEEXT) ? "&+CEnabled" : "&+cDisabled");
    bprintf ("&+wNoSlain        %-11s       &+wLit            %-11s\n",
	     ststflg (mynum, SFL_NOSLAIN) ? "&+CEnabled" : "&+cDisabled",
	     ststflg (mynum, SFL_LIT) ? "&+CEnabled" : "&+cDisabled");
  }
  if (plev (mynum) >= LVL_GOD) {
    bprintf ("&+wSilent Entry   %-11s       &+wCoding         %-11s\n",
	     ststflg (mynum, SFL_SILENT) ? "&+CEnabled" : "&+cDisabled",
	     ststflg (mynum, SFL_CODING) ? "&+CEnabled" : "&+cDisabled");
    bprintf ("&+wIdle           %-11s       &+wSeeIdle        %-11s\n",
	     ptstflg (mynum, PFL_IDLE) ? "&+CEnabled" : "&+cDisabled",
	     ptstflg (mynum, PFL_SEEIDLE) ? "&+CEnabled" : "&+cDisabled");
    bprintf ("&+wSeeSocket      %-11s\n",
	     ptstflg (mynum, PFL_SEESOCKET) ? "&+CEnabled" : "&+cDisabled");
  }
  bprintf ("&+B---------------------------------------------------------\n");
}

void
klockcom (char *passwd)
{
  char pass[sizeof (cur_player->passwd)];

  if (passwd == NULL) {
    if (phandler (mynum) != get_command) {
      bprintf ("You can only lock the keyboard from the command prompt.\n");
      return;
    }
    bprintf ("Locking Keyboard With Login Password.\n");
    bprintf ("\377\373\001\001Enter Keyboard Password: ");
    cur_player->no_echo = True;
    replace_input_handler (klockcom);
  } else {
    my_crypt (pass, passwd, sizeof (pass));
    if (strcmp (cur_player->passwd, pass) == 0) {
      bprintf ("\377\374\001\001\nPassword Correct, Keyboard Unlocked.\n");
      cur_player->no_echo = False;
      bprintf ("%s", cur_player->cprompt);
      replace_input_handler (get_command);
      return;
    } else {
      bprintf ("\377\373\001\001\nEnter Keyboard Password: ");
      return;
    }
  }
}

/* Compares two objects that are weapon or armor and tells which object
 * has the better DP/AC.
 */
void
comparecom ()
{
  int a, b;			/* Objects */
  int adata, bdata, data;	/* DP/AC and compared data for object */

  /* If there is an empty parameter, quit */
  if (EMPTY (item1) || EMPTY (item2)) {
    bprintf ("Compare what?\n");
    return;
  }
  if (plev (mynum) < LVL_WIZARD) {
    if ((a = ob1) == -1 && (b = ob2) == -1) {
      bprintf ("You don't have those objects!\n");
      return;
    }
    if ((a = ob1) == -1) {
      bprintf ("You don't have the first object!\n");
      return;
    }
    if ((b = ob2) == -1) {
      bprintf ("You don't have the second object!\n");
      return;
    }
  } else {
    /* Check to make sure that objects exist */
    if ((a = fobn (item1)) == -1 && (a = fobn (item2)) == -1) {
      bprintf ("Neither object exists!\n");
      return;
    }
    if ((a = fobn (item1)) == -1) {
      bprintf ("Object does not exist! (First one)\n");
      return;
    }
    if ((b = fobn (item2)) == -1) {
      bprintf ("Object does not exist! (Second one)\n");
      return;
    }
  }
  /* Check to see if objects are weapons, if so compare them */
  if (otstbit (a, OFL_WEAPON) && otstbit (b, OFL_WEAPON)) {
    adata = odamage (a);
    bdata = odamage (b);
    if (adata > bdata) {
      data = adata - bdata;
      bprintf ("The %s (%d) is stronger than the %s (%d) by %d DP.\n",
	       oname (a), a, oname (b), b, data);
    }
    if (adata < bdata) {
      data = bdata - adata;
      bprintf ("The %s (%d) is stronger than the %s (%d) by %d DP.\n",
	       oname (b), b, oname (a), a, data);
    }
    if (adata == bdata) {
      bprintf ("The %s (%d) has the same DP has the %s (%d).\n",
	       oname (a), a, oname (b), b);
    }
  } else
    /* Check to see if objects are armor, if so compare them */
  if (otstbit (a, OFL_ARMOR) && otstbit (b, OFL_ARMOR)) {
    adata = oarmor (a);
    bdata = oarmor (b);
    if (adata > bdata) {
      data = adata - bdata;
      bprintf ("The %s (%d) is stronger than the %s (%d) by %d AC.\n",
	       oname (a), a, oname (b), b, data);
    }
    if (adata < bdata) {
      data = bdata - adata;
      bprintf ("The %s (%d) is stronger than the %s (%d) by %d AC.\n",
	       oname (b), b, oname (a), a, data);
    }
    if (adata == bdata) {
      bprintf ("The %s (%d) has the same AC has the %s (%d).\n",
	       oname (a), a, oname (b), b);
    }
  } else {
    /* Objects are not the same, or are not weapons/armor */
    bprintf ("Both objects must be either weapons, armor, shields, or masks.\n");
  }
}

void
judgecom (void)
{
  int plr, maxstr;

  if ((plr = pl1) == -1) {
    bprintf ("Judge who?\n");
    return;
  }
  if (ploc (plr) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return;
  }
  if (plr < max_players && !the_world->w_tournament) {
    bprintf ("Let's not plan on starting any fights, ok?\n");
    return;
  }
  maxstr = plr >= max_players ? pstr_reset (plr) : maxstrength (plr);
  if (pstr (plr) >= maxstr)
    bprintf ("They look to be in perfect health.\n");
  else if (pstr (plr) >= (maxstr * 0.90))
    bprintf ("They look to have taken a little damage.\n");
  else if (pstr (plr) >= (maxstr * 0.70))
    bprintf ("They look to have taken more then a little damage.\n");
  else if (pstr (plr) >= (maxstr * 0.50))
    bprintf ("They look like they have taken quite a bit of damage.\n");
  else if (pstr (plr) >= (maxstr * 0.30))
    bprintf ("They look like they may not be around too much longer.\n");
  else if (pstr (plr) >= (maxstr * 0.10))
    bprintf ("They look like they have been beaten almost to death.\n");
  else
    bprintf ("Put them out of their misery..\n");
}

/************************************************************************
 * The Bulletin Commands                                                *
 * 1995 by Illusion                                                     *
 ************************************************************************/

/* Mortal Bulletins */
void
mbullcom (void)
{
  bprintf ("\001f%s\003", BULL_DIR "/" BULLETIN1);
}

/* Wizard Bulletins */
void
wbullcom (void)
{
  if (plev (mynum) < LVL_WIZARD)
    erreval ();
  else
    bprintf ("\001f%s\003", BULL_DIR "/" BULLETIN2);
}

/* ArchWizard/Advisor Bulletins */
void
abullcom (void)
{
  if (plev (mynum) < LVL_ARCHWIZARD)
    erreval ();
  else
    bprintf ("\001f%s\003", BULL_DIR "/" BULLETIN3);
}

/*  Avatar/God/Master User Bulletins */
void
ubullcom (void)
{
  if (plev (mynum) < LVL_AVATAR)
    erreval ();
  else
    bprintf ("\001f%s\003", BULL_DIR "/" BULLETIN4);
}
