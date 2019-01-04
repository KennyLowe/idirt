#include <stdlib.h>
#include "kernel.h"
#include "locations.h"
#include "commands.h"
#include "wizlist.h"
#include "sendsys.h"
#include "objects.h"
#include "mobiles.h"
#include "bprintf.h"
#include "objsys.h"
#include "pflags.h"
#include "mflags.h"
#include "sflags.h"
#include "oflags.h"
#include "lflags.h"
#include "nflags.h"
#include "eflags.h"
#include "quests.h"
#include "cflags.h"
#include "levels.h"
#include "timing.h"
#include "spell.h"
#include "verbs.h"
#include "rooms.h"
#include "fight.h"
#include "parse.h"
#include "zones.h"
#include "flags.h"
#include "parse.h"
#include "puff.h"
#include "move.h"
#include "uaf.h"
#include "mud.h"
#include "log.h"
#include "jmp.h"
#include "mobile.h"

extern char *WizLevels[];

int
player_damage (int player)
{
  int w;
  int damage = pdam (player);

  if ((w = pwpn (player)) != -1 && iscarrby (w, player) && iswielded (w))
    damage += odamage (w);

  if (check_duration (player, VERB_DAMAGE))
    damage += 10;

  return damage;
}

int
player_armor (int player)
{
  int i, j;
  int armor = parmor (player);

  for (j = 0; j < pnumobs (player) && (i = pobj_nr (j, player), 1); j++) {
    if (iswornby (i, player) && otstbit (i, OFL_ARMOR))
      armor += oarmor (i);
  }
  if (check_duration (player, VERB_ARMOR))
    armor += 10;

  return armor;
}

/* List the people in a room as seen from myself
 */
void
list_people (void)
{
  char xx[SETIN_MAX + 200];
  int i, j;
  int room = ploc (mynum);

  for (j = 0; j < lnumchars (room) && (i = lmob_nr (j, room), 1); j++) {
    if (i != mynum && is_in_game (i) && seeplayer (i)) {
      if (plev (mynum) >= LVL_WIZARD && pvis (i) > 0)
	bprintf ("(");

      if (i >= max_players) {
	bprintf ("%s", pftxt (i));
      } else {
	bprintf ("%s%s", psitting (i) ?
		 build_setin (xx, players[i].setsit, pname (i), NULL, NULL) :
	       build_setin (xx, players[i].setstand, pname (i), NULL, NULL),
		 ststflg (i, SFL_LIT) ? " &+W[Lit]&*" : "");

	if (psex (i))
	  cur_player->wd_them = strcpy (cur_player->wd_her, pname (i));
	else
	  cur_player->wd_them = strcpy (cur_player->wd_him, pname (i));
      }

      if (plev (mynum) >= LVL_WIZARD && pvis (i) > 0)
	bprintf (")");

      bprintf ("\n");

      if (!ststflg (mynum, SFL_NOINV) && gotanything (i)) {
	bprintf ("%s is carrying:\n", pname (i));
	mlobjsat (i, 8);
      }
    }
  }
}

void
move_mobiles (void)
{
  int mon;

  for (mon = max_players; mon < numchars; mon++) {
    if (!ststflg (mon, SFL_OCCUPIED) && pfighting (mon) < 0 && pstr (mon) > 0)
      consid_move (mon);	/* Maybe move it */
  }
  onlook ();
}


/*  Fight control
 */
void
onlook (void)
{
  int i, j;

  for (i = numchars - 1; i >= 0; i--) {
    if (is_in_game (i) && (j = pfighting (i)) >= 0) {
      if (testpeace (i) || !is_in_game (j) || ploc (i) != ploc (j)) {
	pfighting (i) = pfighting (j) = -1;
      } else {
	hit_player (i, j, pwpn (i));

	/* "Wimpy" code, make victim flee if his
	 * strength is less then his "wimpy" val.
	 */
	if (j < max_players && pstr (j) >= 0 && pstr (j) < pwimpy (j)) {
	  int x = mynum;

	  setup_globals (j);
	  bprintf ("You run for your life as your health reaches %d.\n", pstr (mynum));
	  gamecom ("flee", False);
	  setup_globals (x);
	}
      }
    }
  }

  for (i = max_players; i < numchars; i++) {
    chkfight (i);
  }

  if (ocarrf (OBJ_CASTLE_RUNESWORD) >= CARRIED_BY) {
    dorune (oloc (OBJ_CASTLE_RUNESWORD));
  }
  if (ocarrf (OBJ_LIMBO_POUNCIE) >= WORN_BY) {
    dopouncie (oloc (OBJ_LIMBO_POUNCIE));
  }
  for (i = 0; i < max_players; i++) {
    if (is_in_game (i) && phelping (i) != -1) {
      helpchkr (i);
    }
  }
}


void
chkfight (int mon)
{
  int plx;

  if (mon < max_players || mon >= numchars || pagg (mon) == 0 ||
      ststflg (mon, SFL_OCCUPIED) || EMPTY (pname (mon)) ||
      pstr (mon) < 0 || testpeace (mon) || pfighting (mon) >= 0)	/* Already? */
    return;

  /* See if we can hit someone.... */

  for (plx = lfirst_mob (ploc (mon)); plx != SET_END; plx = lnext_mob (ploc (mon))) {

    if (plx < max_players && is_in_game (plx)
	&& !ptstflg (plx, PFL_NOHASSLE) && pvis (plx) == 0) {

      if (mtstflg (mon, MFL_CROSS) &&
	  carries_obj_type (plx, OBJ_CHURCH_CROSS) > -1)
	continue;

      if (randperc () >= pagg (mon))
	continue;

      if (mtstflg (mon, MFL_NOHEAT) && p_ohany (plx, (1 << OFL_LIT))) {
	sendf (plx,
	     "%s seems disturbed by naked flame, and keeps its distance.\n",
	       pname (mon));
	continue;
      }
      hit_player (mon, plx, pwpn (mon));	/* Start the fight */
    }
  }
}

void
consid_move (int mon)
{
  /* Speech for Mindy */
  static char *mindy[] =
  {
    "barks at you playfully.",
    "nibbles at her paw and then scratches her head.",
    "comes up to you and licks your face playfully.",
  "looks quite silly as she runs around in a circle trying to get her tail.",
    "looks at you playfully as she drops her chew toy at your side.",
    "rolls around on the ground playfully.",
    "curls up around your feet and closes her eyes.",
    "looks around for a tasty milk bone."
  };

  /* Speech for Farmer Willy */
  static char *willy[] =
  {
    "Buy Windows 95! Save the mortgage on my house!",
    "I brought you the 640k limit! Don't ever forget that!",
    "What do you mean the Stac Electronics court case?",
    "MacroShaft shall rule the earth!",
    "DoubleSpace is a hack of Stacker? I don't know what you are talking about..",
    "My policy is: if you can't write it, someone else can for you!",
  };

  /* Speech for Beavis */
  static char *beavis[] =
  {
    "Gwar! Gwar! Gwar!",
    "I am CORNHOLIO!",
    "Hmm hmm.. hehehe.. hehehe.. hmm hmm",
    "That was cool!",
    "Hey Butthead, you suck!",
    "Cram it assmunch!",
    "Hey, Butthead, I could kick his ass..",
    "Lets go find Puff, we can SCORE!",
    "Lets go find Baby, we'll score!",
    "Lets go find Imagen, we'll score!",
    "Lets go hang out at Moses's house, he rocks!",
    "We should go like, hang with Illusion, he's cool.. heh heh"
  };

  /* Speech for Butthead */
  static char *butthead[] =
  {
    "Shut up bunghole!",
    "Come to Butthead..",
    "Huh huh huh huh.. uhh.. huh huh",
    "Uhh.. that was pretty cool.. or something",
    "No Beavis! You suck!",
    "Who are you calling an assmunch you bunghole!",
    "Sure you can Beavis, you wuss!",
    "Go ahead dude, she'll burn your nards off.",
    "Go for it Beavis, Joad will zap your ass.. huh huh huh.",
    "You could score dude, but Illusion will get pissed off.",
    "Rock on Beavis!",
    "That's sounds pretty cool."
  };

  /* Speech for Emmy */
  static char *emmy[] =
  {
    "\001pEmmy\003 bounces around happily.",
    "\001pEmmy\003 giggles as she plays peek-a-boo with you!",
    "&+G\001pEmmy\003 &+wsays &+W'&+wKA SAWN!!&+W'"
  };

  int bb = (my_random () % arraysize (beavis));

  if (EMPTY (pname (mon)))
    return;

#if 0
  for (plx = 0; plx < lnumchars (ploc (mon)); plx++) {
    if (pfighting (lmob_nr (plx, ploc (mon))) == mon)
      return;
  }
#endif

  if (randperc () * 6 / (10 * TIMER_INTERRUPT) < pspeed (mon))
    movemob (mon);

  if (pnum (mon) == MOB_VALLEY_CHICKEN && randperc () < 8)
    sendf (ploc (mon), "&+G\001p%s\003 &+wsays &+W'&+wThe sky is about "
	   "to fall in.&+W'\n", pname (mon));

  if (pnum (mon) == MOB_CATACOMB_GHOST && randperc () < 12)
    sendf (ploc (mon), "The Ghost moans, sending chills down your spine.\n");

  if (pnum (mon) == MOB_START_PUFF && randperc () < 10)
    puffcom (mon);		/* Puff Speech/Action Handler */

  if (pnum (mon) == MOB_VALLEY_MINDY && randperc () < 10)
    sendf (ploc (mon), "&+w\001p%s\003 %s\n",
	   pname (mon), mindy[my_random () % arraysize (mindy)]);

  if (pnum (mon) == MOB_SHERWOOD_EMMY && randperc () < 10)
    sendf (ploc (mon), "%s\n", emmy[my_random () % arraysize (emmy)]);

  if (pnum (mon) == MOB_BLIZZARD_WILLY && randperc () < 10)
    sendf (ploc (mon), "&+G\001p%s\003 &+wsays &+W'&+w%s&+W'\n",
	   pname (mon), willy[my_random () % arraysize (willy)]);

  if (pnum (mon) == MOB_VILLAGE_BUTTHEAD && randperc () < 10) {
    sendf (ploc (mon), "&+G\001pBeavis\003 &+wsays &+W'&+w%s&+W'\n",
	   beavis[bb]);
    sendf (ploc (mon), "&+G\001pButthead\003 &+wsays &+W'&+w%s&+W'\n",
	   butthead[bb]);
  }
  if ((mtstflg (mon, MFL_THIEF) && randperc () < 20 && stealstuff (mon))
      || (mtstflg (mon, MFL_PICK) && randperc () < 40 && shiftstuff (mon)))
    return;
}

void
dopouncie (int plx)
{
  int ply, me = real_mynum;
  char pounce[MAX_COM_LEN];

  if (pfighting (plx) >= 0)
    return;

  if (randperc () <= 40) {
    for (ply = lfirst_mob (ploc (plx)); ply != SET_END; ply = lnext_mob (ploc (plx))) {

      if (ply != plx && !EMPTY (pname (ply)) &&
	  fpbns (pname (ply)) >= 0 && randperc () <= 10) {
	sprintf (pounce, "pounce %s", pname (ply));
	sendf (plx, "The pouncie takes control of your body!\n");
	send_msg (ploc (plx), 0, LVL_MIN, LVL_MAX, plx, NOBODY,
		  "\001p%s\003 pounces out of control!\n", pname (plx));
	setup_globals (plx);
	gamecom (pounce, True);
	setup_globals (me);
	return;
      }
    }
  }
}

/* Handle Runesword */
void
dorune (int plx)
{
  int ply;

  if (pfighting (plx) >= 0 || testpeace (plx))
    return;

  for (ply = lfirst_mob (ploc (plx)); ply != SET_END; ply = lnext_mob (ploc (plx))) {

    if (ply != plx && !EMPTY (pname (ply)) &&
	plev (ply) < LVL_WIZARD && fpbns (pname (ply)) >= 0 &&
	randperc () >= 9 * plev (plx)) {
      sendf (plx, "The Runesword twists in your hands, lashing out savagely!\n");
      hit_player (plx, ply, OBJ_CASTLE_RUNESWORD);
      return;
    }
  }
}

Boolean
dragget (void)
{
  int l;

  if (plev (mynum) >= LVL_WIZARD)
    return False;
  if ((l = alive ((max_players + MOB_CAVE_DRAGON))) != -1 &&
      ploc (l) == ploc (mynum)) {
    bprintf ("The dragon makes it quite clear that he doesn't want "
	     "his treasure borrowed!\n");
    return True;
  }
  if ((l = alive (max_players + MOB_OAKTREE_COSIMO)) != -1 &&
      ploc (l) == ploc (mynum)) {
    bprintf ("Cosimo guards his treasure jealously.\n");
    return True;
  }
  return False;
}

void
helpchkr (int plx)
{
  int x = phelping (plx);

  if (!is_in_game (x)) {
    sendf (plx, "You can no longer help.\n");
    setphelping (plx, -1);
  } else if (ploc (x) != ploc (plx)) {
    sendf (plx, "You can no longer help %s.\n", pname (x));
    sendf (x, "%s can no longer help you.\n", pname (plx));
    setphelping (plx, -1);
  }
}



void
movemob (int x)
{
  int n, r;
  Boolean butthead = False;
  int beavis = max_players + MOB_VILLAGE_BEAVIS;

  if (pnum (x) == MOB_VILLAGE_BUTTHEAD)
    butthead = True;

  if (the_world->w_mob_stop)
    return;

  r = randperc () % 6;		/* change this.... here chance is less if few exits */

  if ((n = getexit (ploc (x), r)) >= EX_SPECIAL)
    return;

  if (n >= DOOR) {
    if (state (n - DOOR) > 0)
      return;
    n = obj_loc (olinked (n - DOOR));
  }
  if (n >= 0 || ltstflg (n, LFL_NO_MOBILES) || ltstflg (n, LFL_DEATH))
    return;

  send_msg (ploc (x), 0, pvis (x), LVL_MAX, x, NOBODY,
	    "%s has gone %s.\n", pname (x), exits[r]);

  setploc (x, n);

  send_msg (ploc (x), 0, pvis (x), LVL_MAX, x, NOBODY,
	    "%s has arrived.\n", pname (x));

  if (butthead) {
    send_msg (ploc (beavis), 0, pvis (beavis), LVL_MAX, beavis, NOBODY,
	      "%s has gone %s.\n", pname (beavis), exits[r]);

    setploc (beavis, n);

    send_msg (ploc (beavis), 0, pvis (beavis), LVL_MAX, beavis, NOBODY,
	      "%s has arrived.\n", pname (beavis));
  }
}



void
stopcom (void)
{
  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  the_world->w_mob_stop = 1;

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	    "&+B[&+wMobiles Stopped&+B]\n");
}

void
startcom (void)
{
  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  the_world->w_mob_stop = 0;

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	    "&+B[&+wMobiles Started&+B]\n");
}



/*
 * Steal random object from random player in the same room
 * -Nicknack  Sep. 1990
 * improved by Alf Oct, 1991
 * improved by Nicknack May, 1993
 */
Boolean
stealstuff (int m)
{
  int p[50];			/* Room for mortals and objects */
  int i, j, k;
  int nr = 0;

  /* Count the number of mortals in the same room:
   */
  for (j = 0; j < lnumchars (ploc (m)) && nr < 50; j++) {
    i = lmob_nr (j, ploc (m));

    if (is_in_game (i) && i < max_players && plev (i) < LVL_WIZARD) {
      p[nr++] = i;
    }
  }
  if (nr == 0)
    return False;

  /* Select a random one of those in the same room: */
  i = p[randperc () % nr];

  /* Count the number of objects he carries that we can take: */
  for (nr = k = 0; k < pnumobs (i) && nr < 50; k++) {
    j = pobj_nr (k, i);

    if (iscarrby (j, i)) {
      if (((ocarrf (j) == WORN_BY || ocarrf (j) == BOTH_BY)
	   && !mtstflg (m, MFL_SWORN))
	  ||
	  ((ocarrf (j) == WIELDED_BY || ocarrf (j) == BOTH_BY)
	   && !mtstflg (m, MFL_SWPN)))
	continue;
      else
	p[nr++] = j;
    }
  }

  if (nr == 0)
    return False;

  /* Select random object from those he carries
   */
  j = p[randperc () % nr];

  sendf (i, "\001p%s\003 steals the %s from you!\n", pname (m), oname (j));

  send_msg (ploc (i), 0, LVL_MIN, LVL_MAX, i, NOBODY,
	    "\001p%s\003 steals the %s from \001p%s\003!\n",
	    pname (m), oname (j), pname (i));

  if (otstbit (j, OFL_WEARABLE)) {
    setoloc (j, m, WORN_BY);
  } else {
    setoloc (j, m, CARRIED_BY);
  }

  if (otstbit (j, OFL_WEAPON) &&
      (pwpn (m) == -1 || odamage (pwpn (m)) < odamage (j))) {
    set_weapon (m, j);
  }
  return True;
}


Boolean
shiftstuff (int m)
{
  Boolean took = False;
  int a, b;
  int maxdam = 0, w = -1;
  int maxarm = 0, ww = -1;

  for (b = 0; b < lnumobs (ploc (m)); b++) {

    a = lobj_nr (b, ploc (m));

    if (!oflannel (a)) {
      took = True;

      sendf (ploc (m), "\001p%s\003 takes the %s.\n",
	     pname (m), oname (a));

      /* Wield the best weapon, wear the best armor: */
      if (otstbit (a, OFL_WEAPON) && odamage (a) > maxdam) {
	w = a;
	maxdam = odamage (a);
      }
      if (otstbit (a, OFL_ARMOR) && oarmor (a) > maxarm) {
	ww = a;
	maxarm = oarmor (a);
      }
      setoloc (a, m, CARRIED_BY);
    }
    if (w >= 0)
      set_weapon (m, w);
    if (ww >= 0) {
      setoloc (ww, m, (w == ww) ? BOTH_BY : WORN_BY);
    }
  }
  return took;
}


char *
xname (char *n)
{
  char *t;

  if (n != NULL && (t = strrchr (n, ' ')) != NULL) {
    return t + 1;
  }
  return n;
}



void
setname (int plx)
{
  register PLAYER_REC *p = cur_player;

  if (psex (plx))
    p->wd_them = strcpy (p->wd_her, pname (plx));
  else
    p->wd_them = strcpy (p->wd_him, pname (plx));
}

Boolean
see_player (int pla, int plb)
{
  if (pla < 0 || pla >= numchars)
    return False;
  if (plb == pla || plb < 0 || plb >= numchars)
    return True;
  if (pvis (plb) > 0 && plev (pla) < pvis (plb))
    return False;
  if (ststflg (pla, SFL_BLIND))
    return False;
  if (ploc (pla) == ploc (plb) && r_isdark (ploc (pla), pla))
    return False;
  return True;
}


char *
see_name (int pla, int plb)
{
  return see_player (pla, plb) ? pname (plb) : "Someone";
}

Boolean
seeplayer (int plx)
{
  if (plx == mynum || plx < 0 || plx >= numchars)
    return True;
  if (!see_player (mynum, plx))
    return False;
  setname (plx);
  return True;
}




/* Return a player index given a target name on one of the forms:
 * 1) <player-number>
 * 2) <player-name>
 * 3) <player-name><number-in-sequence-with-that-name>
 *
 * Return -1 if not found.
 */
int
find_player_by_name (char *name)
{
  char b[MNAME_LEN + 1], *p = b;
  int n, i;

  if (name == NULL || strlen (name) > MNAME_LEN)
    return -1;

  name = xname (name);		/* Skip the "The " if there. */

  while (*name != '\0' && isalpha (*name))
    *p++ = *name++;

  *p = '\0';

  if (isdigit (*name)) {
    n = atoi (name);

    while (isdigit (*++name)) ;
    if (*name != '\0')
      return -1;
  } else if (*name != '\0') {
    return -1;
  } else
    n = 1;

  if (*b == '\0') {
    if (n >= GLOBAL_MAX_OBJS && n < GLOBAL_MAX_OBJS + numchars &&
	is_in_game (n - GLOBAL_MAX_OBJS))
      return n - GLOBAL_MAX_OBJS;
    else
      return -1;
  } else {
    /* Try the mobiles in the players location first: */
    if (is_in_game (mynum)) {
      int m = n;
      int loc = ploc (mynum);

      for (i = 0; i < lnumchars (loc); i++) {
	if (EQ (b, xname (pname (lmob_nr (i, loc)))) &&
	    is_in_game (lmob_nr (i, loc)) && --m == 0)
	  return lmob_nr (i, loc);
      }
      if (m < n)
	return -1;
    }
    /* Now try anyone. */
    for (i = 0; i < numchars; i++) {
      if (EQ (b, xname (pname (i))) && is_in_game (i) && --n == 0)
	return i;
    }

    /* Abbreviation */
    for (i = 0; i < numchars; ++i) {
      if (!strncasecmp (b, xname (pname (i)), strlen (b)) &&
	  is_in_game (i) && --n == 0)
	return i;
    }
  }
  return -1;
}

/* Like find_player_by_name() but doesn't check for abbreviations.
 */
int
check_if_player_exists (char *name)
{
  char b[MNAME_LEN + 1], *p = b;
  int n, i;

  if (name == NULL || strlen (name) > MNAME_LEN)
    return -1;

  name = xname (name);		/* Skip the "The " if there. */

  while (*name != '\0' && isalpha (*name))
    *p++ = *name++;

  *p = '\0';

  if (isdigit (*name)) {
    n = atoi (name);

    while (isdigit (*++name)) ;
    if (*name != '\0')
      return -1;
  } else if (*name != '\0') {
    return -1;
  } else
    n = 1;

  if (*b == '\0') {
    if (n >= GLOBAL_MAX_OBJS && n < GLOBAL_MAX_OBJS + numchars &&
	is_in_game (n - GLOBAL_MAX_OBJS))
      return n - GLOBAL_MAX_OBJS;
    else
      return -1;
  } else {
    /* Try the mobiles in the players location first: */
    if (is_in_game (mynum)) {
      int m = n;
      int loc = ploc (mynum);

      for (i = 0; i < lnumchars (loc); i++) {
	if (EQ (b, xname (pname (lmob_nr (i, loc)))) &&
	    is_in_game (lmob_nr (i, loc)) && --m == 0)
	  return lmob_nr (i, loc);
      }
      if (m < n)
	return -1;
    }
    /* Now try anyone. */
    for (i = 0; i < numchars; i++) {
      if (EQ (b, xname (pname (i))) && is_in_game (i) && --n == 0)
	return i;
    }
  }
  return -1;
}


/* Find player by name, if visible to mynum:
 */
int
fpbn (char *name)
{
  int n = find_player_by_name (name);

  return n < 0 || seeplayer (n) ? n : -1;
}


/* Find mobile's in-game index from its ID. Return -1 if not found.
 */
int
find_mobile_by_id (long int id)
{
  long int x;

  if (id >= max_players && id < num_const_chars)
    return id;

  return (x = lookup_entry (id, &id_table)) == NOT_IN_TABLE
    || x < 0 || x >= numchars ? -1 : x;
}



/* Find player or mobile, return number if in game.
 * Set f to true if in file, False if in game. If exists, put
 * the data in the object pointed to by p.
 */
int
find_player (char *name, PERSONA * p, Boolean * f)
{
  int plr;

  *f = False;
  if ((plr = fpbns (name)) >= 0) {
    if (!seeplayer (plr))
      return -1;
    player2pers (p, NULL, plr);
    return plr;
  } else if (ptstflg (mynum, PFL_UAF) != 0 && getuaf (name, p)) {
    *f = True;
    return -2;
  }
  return -1;
}

int
alive (int i)
{
  if (pstr (i) < 0 || EMPTY (pname (i)))
    return -1;
  else
    return i;
}

int
wlevel (int lev)
{
  if (lev < LVL_GUEST)
    return LEV_NEG;		/* Negative level */
  if (lev < LVL_WIZARD)
    return LEV_MORTAL;		/* Mortal */
  if (lev == LVL_WIZARD)
    return LEV_APPRENTICE;	/* Apprentice */
  if (lev < LVL_EMERITI)
    return LEV_DORQ;		/* Dorq */
  if (lev < LVL_FULLWIZ)
    return LEV_EMERITI;		/* Emeriti (Honorary) */
  if (lev < LVL_PROPHET)
    return LEV_WIZARD;		/* Wizard */
  if (lev < LVL_ARCHWIZARD)
    return LEV_PROPHET;		/* Prophet */
  if (lev < LVL_ADVISOR)
    return LEV_ARCHWIZARD;	/* ArchWizard */
  if (lev < LVL_AVATAR)
    return LEV_ADVISOR;		/* Advisor */
  if (lev < LVL_GOD)
    return LEV_AVATAR;		/* Avatar */
  if (lev < LVL_MASTER)
    return LEV_GOD;		/* God */

  return LEV_MASTER;
}

Boolean
do_okay_l (int p, int v, Boolean c)
{
  int lev_p = wlevel (p);
  int lev_v = wlevel (v);

  if (lev_v > lev_p && lev_p > LEV_NEG) {
    return False;
  }
  return (c || (lev_v < lev_p && lev_p > LEV_WIZARD) || lev_p >= LEV_GOD);
}


/* Can p(layer) do XX to v(ictim) ?
 * ** prot_flag protects against it.
 */

Boolean
do_okay (int p, int v, int prot_flag)
{
  return do_okay_l (plev (p), plev (v),
		    (prot_flag < PFL_MAX && !ptstflg (v, prot_flag)));
}


void
setpsex (int chr, Boolean v)
{
  if (v)
    ssetflg (chr, SFL_FEMALE);
  else
    sclrflg (chr, SFL_FEMALE);
}


/* SET Player LOCation.
 */
void
setploc (int plr, int room)
{
  /* First remove plr from his current location:
   */
  if (exists (ploc (plr)))
    remove_int (plr, lmobs (ploc (plr)));

  /* Then add him to the new room:
   */
  if (exists (room))
    add_int (plr, lmobs (room));

  ploc (plr) = room;
}

int
ptothlp (int pl)
{
  int ct;

  for (ct = 0; ct < numchars; ct++) {
    if (ploc (ct) == ploc (pl) && phelping (ct) == pl)
      return ct;
  }
  return -1;
}

int
maxstrength (int p)
{
  return pmaxstrength (plev (p));
}

int
maxmagic (int p)
{
  return pmaxmagic (plev (p));
}

char *
make_rank (int player)
{
  static char rank[80];

  if (plev(player) < 0) {
    sprintf(rank, "%s the Mobile (Level %d)", pname(player), plev(player));
  } else {
  if (plev (player) <= LVL_WIZARD)
    sprintf (rank, "%s the %s (Level %d)", pname (player),
	     psex (player) ? FLevels[plev (player)] : MLevels[plev (player)],
	     plev (player));
  else
    sprintf (rank, "%s the %s (Level %d)", pname (player),
	     WizLevels[wlevel (plev (player))], plev (player));
  }
  return rank;
}

char *
make_title (char *title, char *name)
{
  static char buffer[TITLE_LEN + 100];

  char *p, *q, *r;

  for (p = buffer, q = title; *q != 0;) {
    if (*q != '%')
      *p++ = *q++;
    else {
      switch (*++q) {
      case 's':
	for (r = name; *r != 0;)
	  *p++ = *r++;
	break;
      case 0:
	--q;
	break;
      default:
	*p++ = *--q;
	*p++ = *++q;
	break;
      }
      ++q;
    }
  }
  if (p[-1] == '\n')
    --p;
  *p = 0;
  return buffer;

#if 0
  sprintf (buff, (EMPTY (title) ? "%s the Unknown" : title), name);
  return buff;
#endif
}

char *
std_title (int level, Boolean sex)
{
  extern char *MLevels[];
  extern char *FLevels[];

  static char buff[TITLE_LEN + 10];
  int wl = wlevel (level);

  strcpy (buff, "%s the ");

  switch (wl) {
  case LEV_NEG:
    strcat (buff, "Mobile");
    break;
  case LEV_MORTAL:
  case LEV_APPRENTICE:
    strcat (buff, (sex ? FLevels : MLevels)[level]);
    break;
  default:
    strcat (buff, WizLevels[wl]);
  }
  return buff;
}


/* Try to reset a mobile.
 */
Boolean
reset_mobile (int m)
{
  int loc;

  /* Quick Language Patch */
  setplang (m, NFL_ENGLISH);

  setpstr (m, pstr_reset (m));
  setpvis (m, pvis_reset (m));
  setpflags (m, pflags_reset (m));
  setmflags (m, mflags_reset (m));
  setsflags (m, sflags_reset (m));
  setnflags (m, nflags_reset (m));
  seteflags (m, eflags_reset (m));
  setpmskh (m, 0);
  setpmskl (m, 0);
  setpsitting (m, 0);
  setpfighting (m, -1);
  setphelping (m, -1);
  setpwpn (m, -1);
  setplev (m, plev_reset (m));
  setpagg (m, pagg_reset (m));
  setpspeed (m, pspeed_reset (m));
  setpdam (m, pdam_reset (m));
  setparmor (m, parmor_reset (m));
  setpwimpy (m, pwimpy_reset (m));

  if (EMPTY (pname (m)))
    setpname (m, pname_reset (m));

  if ((loc = find_loc_by_id (ploc_reset (m))) == 0) {
    setpstr (m, -1);
    setploc (m, LOC_DEAD_DEAD);
    return False;
  } else {
    setploc (m, loc);
    return True;
  }
}

void
p_crapup (int player, char *str, int flags)
{
  int m = real_mynum;

  if (player < max_players)
    setup_globals (player);
  else
    setup_globals (m);

  crapup (str, flags | CRAP_RETURN);
  setup_globals (m);
}

void
crapup (char *str, int flags)
{
  if ((flags & CRAP_UNALIAS) != 0) {
    unalias (real_mynum);
    unpolymorph (real_mynum);
  }
  xcrapup (str, (flags & CRAP_SAVE) != 0);

  if ((flags & CRAP_RETURN) == 0) {
    longjmp (to_main_loop, JMP_QUITTING);
  }
}

void
xcrapup (char *str, Boolean save_flag)
{
  static char *dashes =
  "-------------------------------------------------------------------------------";

  if (cur_player->aliased) {
    bprintf ("%s\n", str);
    unalias (real_mynum);
    return;
  } else if (cur_player->polymorphed >= 0) {
    bprintf ("%s\n", str);
    unpolymorph (real_mynum);
    return;
  }
  quit_player ();		/* So we don't get a prompt after exit */

  loseme (save_flag);
  if (str != NULL) {
    bprintf ("\n%s\n\n%s\n%s\n", dashes, str, dashes);
    bflush ();
  }
  setpname (mynum, "");
  setploc (mynum, 0);

  remove_entry (mob_id (mynum), &id_table);

  cur_player->iamon = False;
}

void
death_msg (int plr)
{

  if (ploc (plr) == LOC_CASTLE_MAIDEN && state (OBJ_CASTLE_IN_MAIDEN) == 1) {
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (plr), LVL_WIZARD), LVL_MAX,
	      plr, NOBODY, "&+B[&+RSquished to Death&+B]\n");
    return;
  }
  if (ploc (plr) == LOC_RUINS_DEATHROOM) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, plr, NOBODY,
	      "&+B[&+RTook A Wrong Turn, Fell in a Pit, Got Eaten Alive "
	      "&+g(&+GK&+Ca&+GK&+g)&+B]\n");
    return;
  }
  if (plev (plr) >= LVL_WIZARD)
    return;

  if (ltstflg (ploc (plr), LFL_DEATH)) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, plr, NOBODY,
	      "&+B[&+RDeathroom&+B]\n");
    return;
  }
  if (ltstflg (ploc (plr), LFL_NEGREGEN)) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+B[&+RDied of Natural Causes&+B]\n");
    return;
  }
  if (ltstflg (ploc (plr), LFL_ON_WATER)) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, plr, NOBODY,
	      "&+B[&+RForgot a Boat (Again)&+B]\n");
    return;
  }
  if (ploc (plr) == LOC_FOREST_INTREE) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, plr, NOBODY,
	      "&+B[&+RTree Wins Again&+B]\n");
    return;
  }
  if (ploc (plr) == LOC_PIT_PIT) {
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, plr, NOBODY,
	      "&+B[&+RSuicide&+B]\n");
    return;
  }
}

/* Remove me from the game. Dump objects, send messages, save etc..
 * May only be used after has been called.
 */
void
loseme (Boolean save_flag)
{
  char b[80];
  int x, y;
  Boolean emp = EMPTY (pname (mynum));

  if (cur_player->aliased || cur_player->polymorphed >= 0)
    return;

  if (cur_player->iamon) {
    if (!emp) {

      wipe_duration (mynum);
      calibme ();
      if (save_flag)
	saveme ();

      if ((x = the_world->w_lock) > 0
	  && (y = plev (mynum)) >= x && y >= LVL_WIZARD)
	bprintf ("\nDon't forget the game is locked....\n");

      death_msg (mynum);

      send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
		LVL_MAX, mynum, NOBODY,
	 "&+B[&+W%s &*has departed from the MUD in &+C%s &*(&+C%s&*)&+B]\n",
	  pname (mynum), sdesc (ploc (mynum)), xshowname (b, ploc (mynum)));
    }
    dumpitems ();


    if (!emp) {
      if (!ststflg (mynum, SFL_SILENT))
	mudlog ("EXIT: %s [Level: %d, Score: %d]",
		pname (mynum), plev (mynum), pscore (mynum));

      close_plr_log ();
      setpname (mynum, "");
      setploc (mynum, 0);
      wipe_forget (mynum);
    }
    cur_player->iamon = False;
    snoop_off (mynum);
  }
}

char *
lev2s (char *b, int lvl)
{
  switch (lvl) {
  case 0:
    strcpy (b, "Un");
    return b;
  case LVL_MASTER:
    strcpy (b, "Master User ");
    return b;
  case LVL_GOD:
    strcpy (b, "God ");
    return b;
  case LVL_AVATAR:
    strcpy (b, "Avatar ");
    return b;
  case LVL_ADVISOR:
    strcpy (b, "Advisor ");
    return b;
  case LVL_ARCHWIZARD:
    strcpy (b, "ArchWizard ");
    return b;
  case LVL_PROPHET:
    strcpy (b, "Prophet ");
    return b;
  case LVL_FULLWIZ:
    strcpy (b, "Wizard ");
    return b;
  case LVL_EMERITI:
    strcpy (b, "Emeriti ");
    return b;
  case LVL_DORQ:
    strcpy (b, "Dorq ");
    return b;
  case LVL_WIZARD:
    strcpy (b, "Apprentice Wizard ");
    return b;

  case LVL_LEGEND:
    strcpy (b, "Legend ");
    return b;
  case LVL_WARLOCK:
    strcpy (b, "Warlock ");
    return b;
  case LVL_SORCERER:
    strcpy (b, "Sorcerer ");
    return b;
  case LVL_ENCHANTER:
    strcpy (b, "Enchanter ");
    return b;
  case LVL_MAGICIAN:
    strcpy (b, "Magician ");
    return b;
  case LVL_CONJURER:
    strcpy (b, "Conjurer ");
    return b;
  case LVL_CHAMPION:
    strcpy (b, "Champion ");
    return b;
  case LVL_WARRIOR:
    strcpy (b, "Warrior ");
    return b;
  case LVL_HERO:
    strcpy (b, "Hero ");
    return b;
  case LVL_ADVENTURE:
    strcpy (b, "Adventurer ");
    return b;
  case LVL_NOVICE:
    strcpy (b, "Novice ");
    return b;
  }

  switch (wlevel (lvl)) {
  case LEV_MASTER:
    sprintf (b, "Master User (Level %d) ", lvl);
    return b;
  case LEV_GOD:
    sprintf (b, "God (Level %d) ", lvl);
    return b;
  case LEV_AVATAR:
    sprintf (b, "Avatar (Level %d) ", lvl);
    return b;
  case LEV_ADVISOR:
    sprintf (b, "Advisor (Level %d) ", lvl);
    return b;
  case LEV_ARCHWIZARD:
    sprintf (b, "ArchWizard (Level %d) ", lvl);
    return b;
  case LEV_PROPHET:
    sprintf (b, "Prophet (Level %d) ", lvl);
    return b;
  case LEV_WIZARD:
    sprintf (b, "Wizard (Level %d) ", lvl);
    return b;
  case LEV_EMERITI:
    sprintf (b, "Emeriti (Level %d) ", lvl);
    return b;
  case LEV_DORQ:
    sprintf (b, "Dorq (Level %d) ", lvl);
    return b;
  }
  sprintf (b, "Level %d ", lvl);
  return b;
}

int
tscale (void)
{
  int a = 0;
  int b;

  for (b = 0; b < max_players; b++)
    if (is_in_game (b) && plev (b) < LVL_WIZARD)
      a++;
  if (a < 2)
    return 1;
  else
    return (a > 9 ? 9 : a);
}

Boolean
chkdumb (void)
{
  if (!ststflg (mynum, SFL_DUMB))
    return False;
  bprintf ("You are mute.\n");
  return True;
}

Boolean
chkcrip (void)
{
  if (!ststflg (mynum, SFL_CRIPPLED))
    return False;
  bprintf ("You are crippled.\n");
  return True;
}

Boolean
chksitting (void)
{
  if (!psitting (mynum))
    return False;
  bprintf ("You'll have to stand up, first.\n");
  return True;
}


void
calib_player (int pl)
{
  extern char *MLevels[];
  extern char *FLevels[];
  int b;

  if (pl >= max_players || !players[pl].iamon || players[pl].aliased ||
      players[pl].polymorphed >= 0)
    return;
  if ((b = levelof (pscore (pl), plev (pl))) != plev (pl) &&
      plev (pl) > 0) {

#ifdef USE_QUESTS
    if (b == LVL_WIZARD && (!tstbits (qflags (pl), Q_ALL))) {
      return;
    }
#endif

    setplev (pl, b);

    setptitle (pl, std_title (b, psex (pl)));

    sendf (pl, "You are now %s\n", make_title (ptitle (pl), pname (pl)));

    mudlog ("SYSTEM: %s to level %d", pname (pl), b);

    send_msg (DEST_ALL, MODE_QUIET, max (pvis (pl), LVL_WIZARD), LVL_MAX,
	      NOBODY, NOBODY, "&+B[&+W%s &+wis now level &+C%d&+B]\n",
	      pname (pl), plev (pl));

    switch (b) {
    case LVL_MAGICIAN:
      sendf (pl, "\nWelcome, %s.  You may now use the &+WPOSE &+wcommand.\n",
	     (psex (pl) ? FLevels : MLevels)[LVL_MAGICIAN]);
      break;
    case LVL_SORCERER:
      sendf (pl, "\nNice work, %s.  You may now use the &+RBANG &+wcommand.\n",
	     (psex (pl) ? FLevels : MLevels)[LVL_SORCERER]);
      break;
    case LVL_WIZARD:
      set_xpflags (LVL_WIZARD, &pflags (pl), &pmask (pl));

      sendf (DEST_ALL, "Trumpets sound to praise &+W%s&+w, the new Wizard.\n",
	     pname (pl));

      sendf (pl, "\001f%s\003", GWIZ);

      update_wizlist (pname (pl), LEV_APPRENTICE);

      break;
    }
  }
  if (pstr (pl) > (b = maxstrength (pl)))
    setpstr (pl, b);

  if (pmagic (pl) > (b = maxmagic (pl)))
    setpmagic (pl, b);
}

void
calibme ()
{
  calib_player (mynum);
}

void
destroy_mobile (int mob)
{
  setploc (mob, LOC_DEAD_DEAD);
}

int
levelof (int score, int lev)
{
  int i;

  if (lev > LVL_WIZARD || lev < LVL_NOVICE)
    return lev;
  for (i = LVL_WIZARD; i > LVL_GUEST; i--)
    if (score >= levels[i])
      return i;
  return 0;
}

Boolean
check_setin (char *s, Boolean d, Boolean v)
{
  char *p;
  int nn = 0, dd = 0, vv = 0;

  for (p = s; (p = strchr (p, '%')) != NULL;) {
    switch (*++p) {
    case 'n':
    case 'N':
      ++nn;
      break;
    case 'd':
      ++dd;
      break;
    case 'v':
      ++vv;
      break;
    }
  }

  if (v)
    return (vv > 0 && strlen (s) < SETIN_MAX);
  else
    return (nn > 0 && (dd == 0 || d) && strlen (s) < SETIN_MAX);
}

Boolean
check_nooracle (int plx)
{
  if (ststflg (plx, SFL_NOORACLE) && EQ (pname (mynum), "Oracle"))
    return True;
  else
    return False;
}

Boolean
check_busy (int plx)
{
  if (ststflg (plx, SFL_BUSY) && (plev (plx) > plev (mynum))) {
    bprintf ("%s is busy, try later!\n", pname (plx));
    return True;
  }
  return False;
}

Boolean
check_coding (int plx)
{
  if (ststflg (plx, SFL_CODING)) {
    return True;
  }
  return False;
}

Boolean
check_away (int plx)
{
  if (ststflg (plx, SFL_AWAY))
    return True;
  else
    return False;
}

Boolean
check_forget (int p1, int p2)
{
  int i;
  Boolean found = False;

  if (p1 >= max_players)
    return found;
  if (p2 >= max_players)
    return found;

  for (i = 0; i < 10; ++i) {
    if (players[p1].forget[i] != -1) {
      if (!is_in_game (players[p1].forget[i]))
	players[p1].forget[i] = -1;
      else if (players[p1].forget[i] == p2)
	found = True;
    }
  }
  return found;
}

void
wipe_forget (int plr)
{
  int i, j;

  for (i = 0; i < max_players; ++i) {
    for (j = 0; j < 10; ++j) {
      if (players[i].forget[j] == plr)
	players[i].forget[j] = -1;
    }
  }
  for (i = 0; i < 10; ++i)
    players[plr].forget[i] = -1;
}

Boolean
check_armor (int plx, int obj)
{
  if ((player_armor (plx) + oarmor (obj)) > MAXARMOR)
    return True;
  else
    return False;
}

char *
make_prompt (char *b, char *s, char *h, char *c, char *l, char *n, char *m)
{
  char *p, *q, *r;

  for (p = b, q = s; *q != 0;) {
    if (*q != '%')
      *p++ = *q++;
    else {
      switch (*++q) {
      case 'h':		/* Health */
	if (h == NULL)
	  return NULL;
	for (r = h; *r != 0;)
	  *p++ = *r++;
	break;
      case 'c':		/* Score */
	if (c == NULL)
	  return NULL;
	for (r = c; *r != 0;)
	  *p++ = *r++;
	break;
      case 'm':		/* Mana  */
	if (m == NULL)
	  return NULL;
	for (r = m; *r != 0;)
	  *p++ = *r++;
	break;
      case 'l':		/* Level */
	if (l == NULL)
	  return NULL;
	for (r = l; *r != 0;)
	  *p++ = *r++;
	break;
      case 'n':		/* Name  */
	if (n == NULL)
	  return NULL;
	for (r = n; *r != 0;)
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

char *
build_prompt (int plx)
{
  static char b[PROMPT_LEN + 30];
  char xx[PROMPT_LEN + 40];
  char h[40];
  char c[40];
  char l[40];
  char n[40];
  char m[40];

  *b = 0;

  if (!players[plx].inmailer && !players[plx].inpager) {
    if (pconv (plx) != -1) {
      if (is_in_game (pconv (plx))) {
	sprintf (b, "&+B[&+WCONV: &+C%s&+B] &+w", pname (pconv (plx)));
	return b;
      }
    }
  }
  sprintf (h, "%d/%d", pstr (plx), maxstrength (plx));	/* Health */
  sprintf (c, "%d", pscore (plx));	/* Score  */
  sprintf (l, "%d", plev (plx));/* Level  */
  sprintf (n, "%s", pname (plx));	/* Name   */
  sprintf (m, "%d/%d", pmagic (plx), maxmagic (plx));	/* Mana   */

  sprintf (b, "%s%s%s%s",
	   pvis (plx) ? "(" : "",
	   players[plx].aliased ? "@" : "",
	   make_prompt (xx, players[plx].prompt, h, c, l, n, m),
	   pvis (plx) ? ")" : "");

  return b;
}

int
doorthru (int x)
{
  if (x < DOOR || x > EDOOR)
    return x;
  if (state (x - DOOR))
    return 0;
  return oloc ((x - DOOR) ^ 1);
}

void
do_follow ()
{
  int ct;
  int i;
  int me = mynum;
  int c = 0;

  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && i != mynum && players[i].i_follow == mynum) {

      if (pfighting (i) >= 0) {
	sendf (i, "You can not follow %s out of a fight!\n", pname (mynum));
	sendf (i, "You can no longer follow %s.\n", pname (mynum));
	players[i].i_follow = -1;
	continue;
      }
      if (ltstflg (ploc (mynum), LFL_ON_WATER) && !carries_boat (i)) {
	sendf (i, "You can not follow %s into water!\n", pname (mynum));
	sendf (i, "You can no longer follow %s.\n", pname (mynum));
	players[i].i_follow = -1;
	continue;
      }
      if (ploc (i) == ploc (mynum))
	continue;

      for (ct = 0; ct < 6; ++ct) {
	if (getexit (ploc (i), ct) == ploc (mynum) ||
	    doorthru (getexit (ploc (i), ct)) == ploc (mynum)) {
	  ++c;
	  setup_globals (i);
	  dodirn (ct + 2);
	  setup_globals (me);
	  continue;
	}
      }
      if (!c) {
	sendf (i, "You can no longer follow %s.\n", pname (mynum));
	players[i].i_follow = -1;
      }
    }
  }
}

void
chk_fol (int plx)
{
  int ct;

  for (ct = 0; ct < 6; ++ct) {
    if (!(getexit (ploc (plx), ct) == ploc (mynum) ||
	  doorthru (getexit (ploc (plx), ct)) == ploc (mynum))) {
      players[plx].i_follow = -1;
      sendf (plx, "You can no longer follow %s.\n", pname (mynum));
      return;
    }
  }
}

void
check_follow (void)
{
  int i;

  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && i != mynum && players[i].i_follow == mynum) {
      chk_fol (i);
    }
  }
}

int
vicf2 (int fl, int i)
{
  int plr;

  if (ltstflg (ploc (mynum), LFL_NO_MAGIC) != 0 && plev (mynum) < LVL_WIZARD) {
    bprintf ("Something about this location has drained your mana.\n");
    return -1;
  }
  if (fl >= SPELL_VIOLENT && plev (mynum) < LVL_WIZARD && testpeace (mynum)) {
    bprintf ("No, that's violent!\n");
    return -1;
  }
  if ((plr = vicbase ()) < 0)
    return -1;

  if (pmagic (mynum) < 1) {
    bprintf ("You are too weak to cast magic.\n");
    return -1;
  }
  if (plev (mynum) < LVL_WIZARD)
    setpstr (mynum, pstr (mynum) - 1);

  if (carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE) > -1)
    i++;
  if (carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE1) > -1)
    i++;
  if (carries_obj_type (mynum, OBJ_BLIZZARD_POWERSTONE2) > -1)
    i++;
  if (carries_obj_type (mynum, OBJ_FANTASY_MANA) > -1)
    i = i + 2;

  if (plev (mynum) < LVL_WIZARD && randperc () > i * plev (mynum)) {
    bprintf ("You fumble the magic.\n");
    if (fl == SPELL_REFLECTS) {
      bprintf ("The spell reflects back.\n");
      return mynum;
    }
    return -1;
  }
  if (plev (mynum) < LVL_WIZARD) {
    if ((fl >= SPELL_VIOLENT) && wears_obj_type (plr, OBJ_CATACOMB_SHIELD) > -1) {
      bprintf ("The spell is absorbed by %s shield!\n", his_or_her (plr));
      return -1;
    } else
      bprintf ("The spell succeeds!\n");
  }
  return plr;
}

int
vichfb (int cth)
{
  int a;

  if ((a = vicf2 (SPELL_VIOLENT, cth)) < 0)
    return a;
  if (ploc (a) != ploc (mynum)) {
    bprintf ("%s isnt here.\n", psex (a) ? "She" : "He");
    return -1;
  }
  return a;
}

int
vichere (void)
{
  int a;

  if ((a = vicbase ()) == -1)
    return -1;
  if (ploc (a) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return -1;
  }
  return a;
}

int
vicbase (void)
{
  int a;

  do {
    if (brkword () == -1) {
      bprintf ("Who?\n");
      return -1;
    }
  }
  while (EQ (wordbuf, "at"));
  if ((a = fpbn (wordbuf)) < 0) {
    bprintf ("That person isn't playing now.\n");
    return -1;
  }
  return a;
}

/* The JUMP command
 */
void
jumpcom ()
{
  int a, b, i, j, x;
  char ms[128];
  extern int pits[];

  /* Where is it possible to jump from, and where do
   * * we land...
   */
  static int jumtb[] =
  {
  /* FROM,                    TO */
    LOC_CASTLE_LEAP, LOC_VALLEY_NPATH,
    LOC_TOWER_LEDGE, LOC_CASTLE_WALLS,
    LOC_TREEHOUSE_PORCH, LOC_VALLEY_ESIDE,
    LOC_MOOR_PIT, LOC_LEDGE_PIT,
    LOC_BLIZZARD_WINDOW, LOC_BLIZZARD_CHAMBER,
    LOC_BLIZZARD_CHAMBER, LOC_BLIZZARD_WINDOW,
    LOC_CATACOMB_CHASM1, LOC_CATACOMB_CAVERN,
    LOC_CATACOMB_CHASM2, LOC_CATACOMB_CAVERN,
    LOC_CATACOMB_SLEDGE2, LOC_CATACOMB_CAVERN,
    LOC_CATACOMB_NLEDGE2, LOC_CATACOMB_CAVERN,
    LOC_CATACOMB_SLEDGE1, LOC_CATACOMB_CHASM1,
    LOC_CATACOMB_NLEDGE1, LOC_CATACOMB_CHASM2,
    0, 0};

  if (psitting (mynum)) {
    bprintf ("You have to stand up first.\n");
    return;
  }
#ifdef LOCMIN_MITHDAN
  if (ploc (mynum) == oloc (OBJ_MITHDAN_FOUNTAIN)) {
    bprintf ("You leap over the sleeping dragon, and plunge into the fountain...\n");
    bprintf ("As you touch the water, you find yourself teleported!\n");
    send_msg (ploc (mynum), 0,
	      pvis (mynum) > 0 ? pvis (mynum) : LVL_MIN, LVL_MAX,
	      mynum, NOBODY,
	    "%s leaps over the dragon into the fountain, and dissapears!\n",
	      pname (mynum));
    teletrap (LOC_MITHDAN_MITHDAN51);
    return;
  }
#endif

#ifdef LOCMIN_FROBOZZ
  if (ploc (mynum) == LOC_FROBOZZ_ENDING) {
    sprintf (ms, "%s jumps into the hole and disappears!\n", pname (mynum));
    sillycom (ms);
    setploc (mynum, LOC_BLIZZARD_GLITTERING);
    bprintf ("You jump into the hole...\n\n\n\nAnd fall....."
	     "Until the tunnel stops abruptly\n"
	     "and ends as a hole in a ceiling, "
	     "through which you fall and land\n");
    sprintf (ms, "%s comes falling through a hole in the ceiling and lands\n",
	     pname (mynum));
    sillycom (ms);
    if (ploc (mynum) == ploc (max_players + MOB_BLIZZARD_GIANT)) {
      bprintf ("on the stomach of a snoring giant!\n");
      sillycom ("on the stomach of the the snoring giant!\n");
      hit_player (max_players + MOB_BLIZZARD_GIANT, mynum, -1);
    } else {
      bprintf ("on the ground with a thud, hurting yourself badly!\n");
      sillycom ("on the ground with a loud thud, being badly hurt!\n");
      if (plev (mynum) < LVL_WIZARD)
	setpstr (mynum, pstr (mynum) / 2);
    }
    return;
  }
#endif

#ifdef LOCMIN_TALON
  if (ploc (mynum) == LOC_TALON_TALON25) {
    send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX, mynum, NOBODY,
	   "%s makes a small jump off the edge of the cliff and disappears "
	      "with a\nsmall popping noise.\n", pname (mynum));
    send_msg (ploc (mynum), MODE_NODEAF, LVL_MIN, pvis (mynum) - 1, mynum, NOBODY,
	      "You hear a small popping noise.\n");
    bprintf ("You step off the edge of the cliff and feel a sudden change in "
	     "pressure at\nabout the same time you realize you're back in "
	     "the study.\n");
    trapch (LOC_TALON_TALON4);
    send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s appears from nowhere, accompanied by a small popping "
	      "noise.\n", pname (mynum));
    send_msg (ploc (mynum), MODE_NODEAF, LVL_MIN, pvis (mynum) - 1, mynum, NOBODY,
	      "You hear a small popping noise.\n");
    return;
  }
#endif

  /* Search the jump-table for special locations..
   */
  for (a = 0, b = 0; jumtb[a]; a += 2) {
    if (jumtb[a] == ploc (mynum)) {
      b = jumtb[a + 1];
      break;
    }
  }

  if (ploc (mynum) == LOC_CATACOMB_SLEDGE2 ||
      ploc (mynum) == LOC_CATACOMB_NLEDGE2) {
    if (oloc (OBJ_START_UMBRELLA) == mynum &&
	ocarrf (OBJ_START_UMBRELLA) == CARRIED_BY) {
      bprintf ("You grab hold of the umbrella and step off the ledge.\n\n"
	       "You are drifting slowly through the darkness of the "
	       "chasm, falling towards an\nuncertain fate. The rocky "
	       "walls slide past you, as does another ledge, just\n"
	       "beyond your reach.  After several minutes, your descent "
	       "comes to an end and\nyou alight in ...\n");
      trapch (b);
      return;
    } else {
      bprintf ("You step off the ledge, but instead of plummeting into the "
	       "darkness below,\nyour motion is stopped and you find "
	       "yourself ...\n");
      trapch ((ploc (mynum) == LOC_CATACOMB_SLEDGE2) ?
	      LOC_CATACOMB_CHASM1 : LOC_CATACOMB_CHASM2);
      return;
    }
  }
  if (ploc (mynum) == LOC_CATACOMB_SLEDGE1 ||
      ploc (mynum) == LOC_CATACOMB_NLEDGE1) {
    bprintf ("You make a leap of Faith, throwing yourself off the ledge. "
	     "You are hurtling\nrapidly through the darkness of the chasm, "
	     "falling to almost certain doom.\nThe rocky walls rush past "
	     "you ...\n");
    trapch (b);
    return;
  }
  /* Are we by a pit ? If so we'll jump into it.
   */
  for (i = 0; (j = pits[i++]) != -1 && oloc (j) != ploc (mynum);) ;

  if (j >= OBJ_CATACOMB_PIT_NORTH && j <= OBJ_CATACOMB_PIT_WEST
      && state (i) == 0) {
    b = LOC_CATACOMB_RIPS;
  } else if ((j >= OBJ_START_PIT && j <= OBJ_START_CHURCH_PIT)
	     || (oloc (OBJ_ORCHOLD_HOLEORCS) == ploc (mynum))) {
    b = LOC_PIT_PIT;
  }
  if (b == 0) {
    bprintf ("Wheeeeee....\n");
    return;
  }
  if ((x = carries_obj_type (mynum, OBJ_START_UMBRELLA)) > -1
      && state (x) != 0) {
    sprintf (ms, "%s jumps off the ledge.\n", pname (mynum));
    bprintf ("You grab hold of the %s and fly down like "
	     "Mary Poppins.\n", oname (x));
  } else if (plev (mynum) < LVL_WIZARD) {
    sprintf (ms, "%s makes a perfect swan dive off the ledge.\n",
	     pname (mynum));
    if (b != LOC_PIT_PIT) {
      send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY, ms);
      setploc (mynum, b);
      bprintf ("Wheeeeeeeeeeeeeeeee       <<<<SPLAT>>>>\n");
      bprintf ("You seem to be splattered all over the place.\n");
      crapup ("\t\tI suppose you could be scraped up with a spatula.\n",
	      SAVE_ME);

    }
  } else
    sprintf (ms, "%s dives off the ledge and floats down.\n", pname (mynum));


  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY, ms);

  setploc (mynum, b);

  if (iscarrby (OBJ_START_UMBRELLA, mynum))
    sprintf (ms, "%s flys down, clutching an umbrella.\n", pname (mynum));
  else
    sprintf (ms, "%s has just dropped in.\n", pname (mynum));

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY, ms);

  trapch (b);
}

/* Stuff that should be done both after every command and at every
 * i/o-interrupt at the latest.
 */
void
special_events (int player)
{
  int start = (player == SP_ALL) ? 0 : player;
  int stop = (player == SP_ALL) ? max_players - 1 : player;
  int i;

  if (player >= max_players)
    return;

  for (i = start; i <= stop; ++i)
    if (is_in_game (i)) {

      if (ploc (i) == LOC_RUINS_DEATHROOM) {
	broad ("A muffled cry is heard before the sound of shattering bones "
	       "drowns it out.\n");
	p_crapup (i, "\tYou should really avoid those bottomless pits.\n",
		  CRAP_SAVE);
	continue;
      }
      if (ploc (i) == LOC_CASTLE_MAIDEN && state (OBJ_CASTLE_IN_MAIDEN) == 1) {
	broad ("There is a long drawn out scream in the distance..\n");
	p_crapup (i, "\t\tThe iron maiden closes....... S Q U I S H !\n",
		  CRAP_SAVE);
	continue;
      }
      if (plev (i) < LVL_WIZARD && ltstflg (ploc (i), LFL_ON_WATER)) {
	if (!carries_boat (i)) {
	  broad ("A loud gurgling is heard, then a crashing of waves..\n");
	  p_crapup (i, "\t\tYou plunge beneath the waves....\n", CRAP_SAVE);
	  continue;
	}
      }
      /* 15% of losing health in NEGREGEN room */
      if (plev (i) < LVL_WIZARD && ltstflg (ploc (i), LFL_NEGREGEN) &&
	  randperc () < 15) {
	sendf (i, "You feel weaker.\n");
	if ((pstr (i) - 1) == 0) {
	  setpstr (i, 50);
	  p_crapup (i, "You should have been more careful when your life "
		    "started slipping away..\n", CRAP_SAVE);
	}
	setpstr (i, (pstr (i) - 1));
      }
      /* 10% chance of the tree swallowing you
       */
      if (ploc (i) == LOC_FOREST_INTREE
	  && plev (i) < LVL_WIZARD && randperc () < 10) {

	p_crapup (i, "You have been absorbed, and crushed to death by "
		  "the tree....\n", CRAP_SAVE);
	broad ("There is a hideous scream and a grinding of bone...\n");
	continue;
      }
      /* 40% chance that tree will suck you in
       */
      if (ploc (i) == oloc (OBJ_FOREST_TREEEATING) && randperc () < 40) {
	int me = real_mynum;

	sendf (i, "You are suddenly grabbed and taken into the tree!\n");
	setup_globals (i);
	teletrap (LOC_FOREST_INTREE);
	setup_globals (me);
      }
      /* 10% chance that the floorboards will squeak */
      if (ploc (i) == oloc (OBJ_VILLAGE_TOP_BOARDS) && randperc () < 10) {
	sendf (i, "The floorboards creak alarmingly as you move around.\n");
      }
      /* 10% chance of the fox barking */
      if (ploc (i) == oloc (OBJ_OAKTREE_TOPFOXHOLE) && randperc () < 10) {
	sendf (i, "The sharp bark of a fox reverberates off the oaks.\n");
      }
      if (ploc (i) == oloc (OBJ_MOOR_ALTAR) && plev (i) < LVL_WIZARD) {
	if (oloc (OBJ_CHURCH_CROSS) == i) {
	  sendf (i, "The altar and cross suddenly start sparking and "
		 "flashing, as if in some kind of\nconflict.  With "
		 "a massive bang, the altar cracks.\n");
	  destroy (OBJ_MOOR_ALTAR);
	}
      }
      if (ploc (i) == oloc (OBJ_OAKTREE_TOPFOXHOLE) &&
	  ploc (i) == ploc (pnum (MOB_OAKTREE_OTTIMO)) &&
	  state (OBJ_OAKTREE_TOPFOXHOLE) == 1) {
	sendf (ploc (i), "Ottimo digs up the fox hole.\n");
	setobjstate (OBJ_OAKTREE_TOPFOXHOLE, 0);
      }
      /* If the snoop-victim has disappeard or the snooper is (only) a wizard
       * * and the target has gone into a private room, then stop the snoop.
       */
      if (players[i].snooptarget != -1) {

	if (!is_in_game (players[i].snooptarget)) {
	  sendf (i, "You can no longer snoop.\n");
	  snoop_off (i);
	} else if (ltstflg (ploc (players[i].snooptarget), LFL_PRIVATE)
		   && plev (i) < LVL_ARCHWIZARD) {

	  sendf (i, "%s went into a PRIVATE room, you can no longer snoop.\n",
		 pname (players[i].snooptarget));

	  snoop_off (i);
	}
      }
      /* count down if polymorphed
       */
      if (players[i].polymorphed == 0)
	unpolymorph (i);
      if (players[i].polymorphed > -1)
	players[i].polymorphed--;

      /* if invis, count down
       */
      if (players[i].me_ivct > 0 && --players[i].me_ivct == 0) {
	setpvis (i, 0);
      }
      if (pstr (i) < 0 && (players[i].aliased
			   || players[i].polymorphed != -1)) {
	sendf (i, "You've just died.\n");
	if (players[i].polymorphed != -1)
	  unpolymorph (i);
	else
	  unalias (i);
      }
    }				/* end for each player */
}

void
regenerate (int plr)
{
  int obj, chance, mchance;

  chance = 10;
  mchance = 20;

  if ((ltstflg (ploc (plr), LFL_NOREGEN) || ltstflg (ploc (plr), LFL_NEGREGEN))
      && plev (plr) < LVL_WIZARD && randperc () < 30) {
    sendf (plr, "You cannot rest here.\n");
    return;
  }
  for (obj = pfirst_obj (plr); obj != SET_END; obj = pnext_obj (plr)) {
    if (otstbit (obj, OFL_REGENHEALTH))
      chance += (chance == 10) ? 50 : 10;
    if (otstbit (obj, OFL_REGENMANA))
      mchance += (mchance == 20) ? 50 : 10;
  }

  chance += psitting (plr) ? 15 : 0;
  mchance += psitting (plr) ? 15 : 0;
  chance += ltstflg (ploc (plr), LFL_FASTHEAL) ? 15 : 0;
  mchance += ltstflg (ploc (plr), LFL_FASTMANA) ? 15 : 0;
  chance = (chance >= 100) ? 90 : chance;
  mchance = (mchance >= 100) ? 90 : mchance;

  if (pfighting (plr) < 0 && pstr (plr) < maxstrength (plr)) {
    if (randperc () < chance) {
      if (maxstrength (plr) == pstr (plr) + 1)
	sendf (plr, "Your feel fully healed.\n");
      setpstr (plr, pstr (plr) + 1);
    }
  }
  if (pfighting (plr) < 0 && pmagic (plr) < maxmagic (plr)) {
    if (randperc () < mchance) {
      if (maxmagic (plr) == pmagic (plr) + 1)
	sendf (plr, "Your mana has been replenished.\n");
      setpmagic (plr, pmagic (plr) + 1);
    }
  }
}

/* Sets quest flags
 * 1995 by Illusion
 */
void
set_quest (int plx, int quest)
{
  mudlog ("SYSTEM: %s got quest %s (%d)", pname (plx), Quests[quest], quest);

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX, NOBODY,
	NOBODY, "&+B[&+WQuest: &+C%s &+wcompleted the &+C%s &+wquest&+B]\n",
	    pname (plx), Quests[quest]);

  qdsetflg (quest);
  qsetflg (plx, quest);

  /* Give the language flag as well for completing the quest */
  nsetflg (plx, quest + 1);
}

void
set_msg (char *b, Boolean dir_ok, Boolean sum)
{
  char k[MAX_COM_LEN];

  getreinput (k);
  if (check_setin (k, dir_ok, sum))
    strcpy (b, k);
  else
    bprintf ("Not changed, wrong format.\n");
}

char *
str_color (int plr)
{
  if (plr < max_players)
    return (pstr (plr) == maxstrength (plr) ? "&+W" :
	    pstr (plr) < 20 ? "&+R" : "&+Y");
  else
    return (pstr (plr) == pstr_reset (plr) ? "&+W" :
	    pstr (plr) < 20 ? "&+R" : "&+Y");
}

char *
mag_color (int plr)
{
  if (plr < max_players)
    return (pmagic (plr) == maxmagic (plr) ? "&+W" :
	    pmagic (plr) < 20 ? "&+R" : "&+Y");
  else
    return NULL;
}
