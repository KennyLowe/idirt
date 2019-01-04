
#include <stdlib.h>
#include "kernel.h"
#include "objects.h"
#include "mobiles.h"
#include "mobile.h"
#include "locations.h"
#include "sflags.h"
#include "pflags.h"
#include "mflags.h"
#include "oflags.h"
#include "lflags.h"
#include "cflags.h"
#include "eflags.h"
#include "quests.h"
#include "sendsys.h"
#include "levels.h"
#include "parse.h"
#include "objsys.h"
#include "oflagnames.h"
#include "rooms.h"
#include "objsys.h"
#include "bprintf.h"
#include "fight.h"
#include "commands.h"
#include "log.h"
#include "clone.h"

typedef struct {
  char *class_name;
  long class_mask;
  short class_state;
}

CLASS_DATA;

static int value_class (CLASS_DATA * cl, int plx, Boolean silent);
static CLASS_DATA *findclass (char *n);
static Boolean classmatch (int ob, CLASS_DATA * cl);
static void dropall (CLASS_DATA * cl);
static void getall (CLASS_DATA * cl);
static void getallfr (CLASS_DATA * cl);


static CLASS_DATA class_data[] =
{
  {"clothing", 1 << OFL_WEARABLE, -2},
  {"weapons", 1 << OFL_WEAPON, -1},
  {"containers", 1 << OFL_CONTAINER, -1},
  {"food", 1 << OFL_FOOD, -1},
  {"keys", 1 << OFL_KEY, -1},
  {"all", 0, -1},
  {NULL, 0, -1}};

int pits[] =
{
  OBJ_START_PIT, OBJ_START_CHURCH_PIT,
  OBJ_CATACOMB_PIT_NORTH, OBJ_CATACOMB_PIT_EAST,
  OBJ_CATACOMB_PIT_SOUTH, OBJ_CATACOMB_PIT_WEST,
  -1};

Boolean
ispit (register int o)
{
  register int i = 0;
  register int j;

  while ((j = pits[i++]) != o && j != -1) ;
  return (j >= 0);
}

void
givecom (void)
{
  int a, c;

  if (EMPTY (item1)) {
    bprintf ("Give what to who?\n");
    return;
  }
  if (pl1 != -1) {
    if ((a = pl1) == -1) {
      bprintf ("Who's that?\n");
      return;
    }
    if (mynum == a) {
      bprintf ("Cheap skate!\n");
      return;
    }
    if (EMPTY (item2)) {
      bprintf ("Give them what?\n");
      return;
    }
    if ((c = ob2) == -1) {
      bprintf ("You don't have it.\n");
      return;
    }
    dogive (c, a);
    return;
  }
  if ((a = ob1) == -1) {
    bprintf ("You don't have any %s.\n", item1);
    return;
  }
  if (EMPTY (item2)) {
    bprintf ("To who?\n");
    return;
  }
  if ((c = pl2) == -1) {
    bprintf ("Who's that?\n");
    return;
  }
  if (mynum == c) {
    bprintf ("Cheap skate!\n");
    return;
  }
  dogive (a, c);
}

void
dogive (int ob, int pl)
{
  int i, j, o = 0;

  if (plev (mynum) < LVL_WIZARD && ploc (pl) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return;
  }
  if (!iscarrby (ob, mynum)) {
    bprintf ("You don't have any %s.\n", oname (ob));
    return;
  }
  if (!cancarry (pl)) {
    bprintf ("They can't carry it.\n");
    return;
  }
  if (pl >= max_players && mtstflg (pl, MFL_QFOOD) && otstbit (ob, OFL_FOOD)) {
    bprintf ("%s thanks you.\n", pname (pl));
    sendf (ploc (mynum), "%s has left the game.\n", pname (pl));
    setpscore (pl, pscore (pl) + 50);
    dumpstuff (pl, ploc (pl));
    strcpy (pname (pl), "");
    eat (ob);
    return;
  }
#ifdef LOCMIN_RUINS
  if (pnum (pl) == MOB_RUINS_ZEPHERE && onum (ob) == OBJ_RUINS_HORN) {
    bprintf ("Zephere smiles at you and thanks you for the horn.\n");
    if (!etstflg (mynum, EFL_BLUR)) {
      bprintf ("In return for your kindness, he teaches you the BLUR spell!\n");
      esetflg (mynum, EFL_BLUR);
    }
  }
#endif

#ifdef LOCMIN_FANTASY
  if (pnum (pl) == MOB_FANTASY_SHOPKEEPERONE && onum (ob) == OBJ_FANTASY_GOLDNUGGET) {
    bprintf ("The shopkeeper smiles at you as he places the golden sword on the "
	     "counter\nfor you to take.\n\n");
    setobjstate (OBJ_FANTASY_GOLDSWORD, 0);
  }
  if (pnum (pl) == MOB_FANTASY_SHOPKEEPERTWO && onum (ob) == OBJ_FANTASY_SILVER) {
    bprintf ("The shopkeeper smiles at you as he places the silver sword and the "
	     "silver\narmor on the counter for you to take.\n\n");
    setobjstate (OBJ_FANTASY_SILVERSWORD, 0);
    setobjstate (OBJ_FANTASY_SILVERARMOR, 0);
  }
#endif

  if (pnum (pl) == MOB_CATACOMB_BEGGAR && otstbit (ob, OFL_FOOD)) {
    bprintf ("The Beggar thanks you and greedily devours the %s.\n",
	     oname (ob));
    setpscore (mynum, pscore (mynum) + 50);
    eat (ob);
    bprintf ("After finishing his meal, the beggar stares at you and says '");
    switch (my_random () % 4) {
    case 0:
      bprintf ("Charity");
      o = OBJ_CATACOMB_PIT_NORTH;
      break;
    case 1:
      bprintf ("Faith");
      o = OBJ_CATACOMB_PIT_EAST;
      break;
    case 2:
      bprintf ("Wisdom");
      o = OBJ_CATACOMB_PIT_SOUTH;
      break;
    case 3:
      bprintf ("Courage");
      o = OBJ_CATACOMB_PIT_WEST;
      break;
    }
    bprintf ("'\n");
    for (i = 0; (j = pits[i++]) != -1;)
      setobjstate (j, 0);
    setobjstate (o, 1);
    sendf (oloc (o), "A faint glow emanates from the pit.\n");
    return;
  }
  if (pnum (pl) == MOB_OAKTREE_VIOLA && onum (ob) == OBJ_OAKTREE_FAN) {
    bprintf ("Viola kisses you%s.\n", psex (mynum) == 0 ? "" : " on the cheek");
    bprintf ("Viola says 'Thank you, %s.  Won't you please come in?'\n",
	     psex (mynum) == 0 ? "kind sir" : "madame");
    setpscore (mynum, pscore (mynum) + 50);
  }
  if (onum (ob) == OBJ_VALLEY_ROSE && psex (mynum) != psex (pl)) {
    bprintf ("You give %s the %s.\n", him_or_her (pl), oname (ob));
    setpscore (mynum, pscore (mynum) + 60);
    setpscore (pl, pscore (pl) + 50);
    sendf (pl, "%s gives you the %s.\n", see_name (pl, mynum), oname (ob));
    setoloc (ob, pl, CARRIED_BY);
    if (++odamage (ob) >= 2) {
      sendf (ploc (pl), "The %s turns to dust.\n", oname (ob));
      destroy (ob);
    }
    return;
  }
  /* database.c */
  if ((onum (ob) == OBJ_START_UMBRELLA) && (pnum (pl) == MOB_VALLEY_CHICKEN)) {
    setoloc (ob, pl, CARRIED_BY);
    bprintf ("It looks confused.\n");
    bprintf ("It probably doesn't know how to use it, but it takes the "
	     "umbrella anyway.\n");
    setpscore (mynum, pscore (mynum) + 50);
    return;
  }
  if ((onum (ob) == OBJ_FOREST_PIPES) && (pnum (pl) == MOB_BLIZZARD_SEAMAS)) {
    setoloc (ob, pl, CARRIED_BY);
    setpscore (mynum, pscore (mynum) + 50);
    bprintf ("Seamas takes the pipes, thanks you, and begins to play a "
	     "haunting melody.\n");
    broad ("A haunting pipe melody echoes through the air.\n");
    return;
  }
  if ((onum (ob) == OBJ_CAVE_BAGPIPES) && (pnum (pl) == MOB_VALLEY_PIPER)) {
    setoloc (ob, pl, CARRIED_BY);
    bprintf ("The Piper thanks you and begins to play.\n");
    setpscore (mynum, pscore (mynum) + 50);
    broad ("In the distance you hear a stirring bagpipe rendition of "
	   "'Amazing Grace.'\n");
    return;
  }
  if (plev (mynum) < LVL_WIZARD) {
    if ((onum (ob) == OBJ_CASTLE_RUNESWORD)) {
      bprintf ("The Runesword does not wish to be given away.\n");
      return;
    }
  }
  setoloc (ob, pl, CARRIED_BY);
  sendf (pl, "%s gives you the %s.\n", see_name (pl, mynum), oname (ob));
  send_msg (ploc (pl), 0, LVL_MIN, LVL_MAX, pl, mynum,
	    "\001p%s\003 gives \001p%s\003 the %s.\n",
	    pname (mynum), pname (pl), oname (ob));
  return;
}

void
stealcom (void)
{
  int a, c, e, f;
  char x[128];

  if (EMPTY (item1)) {
    bprintf ("Steal what?\n");
    return;
  }
  strcpy (x, item1);
  if (EMPTY (item2)) {
    bprintf ("From who?\n");
    return;
  }
  if ((c = pl2) == -1) {
    bprintf ("Who is that?\n");
    return;
  }
  if (mynum == c) {
    bprintf ("A true kleptomaniac.\n");
    return;
  }
  if ((a = fobncb (x, c)) == -1) {
    bprintf ("They don't have it.\n");
    return;
  }
  if (plev (mynum) < LVL_WIZARD && ploc (c) != ploc (mynum)) {
    bprintf ("They're not here!\n");
    return;
  }
  if (ocarrf (a) == WORN_BY) {
    bprintf ("They're wearing it.\n");
    return;
  }
  if (pwpn (c) == a) {
    bprintf ("They have it firmly to hand ... for KILLING people with!\n");
    return;
  }
  if (pnum (c) == MOB_CATACOMB_DEFENDER || mtstflg (c, MFL_NOSTEAL)) {
    sendf (ploc (c), "%s says 'How dare you steal from me, %s!'\n",
	   pname (c), pname (mynum));
    hit_player (c, mynum, -1);
    return;
  }
  if (!do_okay (mynum, c, PFL_NOSTEAL)) {
    int i = randperc () % 3;

    switch (i) {
    case 0:
      bprintf ("%s is too watchful.\n", he_or_she (c));
      return;
    case 1:
      bprintf ("%s is too alert.\n", he_or_she (c));
      return;
    case 2:
      bprintf ("%s is too crafty.\n", he_or_she (c));
      return;
    }
  }
  if (!cancarry (mynum)) {
    bprintf ("You can't carry any more.\n");
    return;
  }
  f = randperc ();
  e = (10 + plev (mynum) - plev (c)) * 5;
  if (f < e || plev (mynum) >= LVL_WIZARD) {
    bprintf ("Got it!\n");
    sendf (c, "%s steals the %s from you!\n",
	   see_name (c, mynum), oname (a));
    setoloc (a, mynum, CARRIED_BY);
    if ((f & 1) && (c >= max_players))
      hit_player (c, mynum, -1);
    return;
  }
  bprintf ("Your attempt fails.\n");
}



Boolean
is_shield (int obj)
{
  return (!strncasecmp (oname (obj), "shield", strlen (oname (obj))));
}

/* Does player pl wear a shield ?
 */
Boolean
wears_shield (int pl)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iswornby (pobj_nr (i, pl), pl) && is_shield (pobj_nr (i, pl)))
      return True;
  }

  return False;
}



Boolean
is_armor (int obj)
{
  return (!strncasecmp (oname (obj), "armor", strlen (oname (obj))) ||
	  !strncasecmp (oname (obj), "mail", strlen (oname (obj))));
}


/* Does player pl wear armor ?
 */
Boolean
wears_armor (int pl)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iswornby (pobj_nr (i, pl), pl) && is_armor (pobj_nr (i, pl)))
      return True;
  }
  return False;
}

Boolean
is_mask (int obj)
{
  return (!strncasecmp (oname (obj), "mask", strlen (oname (obj))));
}


Boolean
wears_mask (int pl)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iswornby (pobj_nr (i, pl), pl) && is_mask (pobj_nr (i, pl)))
      return True;
  }

  return False;
}

Boolean
is_boat (int obj)
{
  return
    onum (obj) == OBJ_VILLAGE_BOAT
    || onum (obj) == OBJ_VILLAGE_RAFT
    || onum (obj) == OBJ_ANCIENT_CANOE;
}

Boolean
carries_boat (int pl)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iscarrby (pobj_nr (i, pl), pl) && is_boat (pobj_nr (i, pl)))
      return True;
  }

  return False;
}

/* Does pl carry object type or a clone of it ?
 */
int
carries_obj_type (int pl, int type)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iscarrby (pobj_nr (i, pl), pl) &&
	onum (pobj_nr (i, pl)) == type)
      return pobj_nr (i, pl);
  }
  return -1;
}


/* Does pl wear object type or a clone of it ?
 */
int
wears_obj_type (int pl, int type)
{
  int i;

  for (i = 0; i < pnumobs (pl); i++) {

    if (iswornby (pobj_nr (i, pl), pl) &&
	onum (pobj_nr (i, pl)) == type)
      return pobj_nr (i, pl);
  }
  return -1;
}

void
removecom (void)
{
  int a;
  int b;

  if ((a = ohereandget ()) == -1)
    return;
  if (!iswornby (a, mynum)) {
    bprintf ("You're not wearing it.\n");
    return;
  }
  b = (ocarrf (a) == BOTH_BY) ? WIELDED_BY : CARRIED_BY;
  setcarrf (a, b);

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "%s removes the %s.\n", pname (mynum), oname (a));

  bprintf ("Ok\n");
}

static int
value_class (CLASS_DATA * cl, int plx, Boolean silent)
{
  int obj, sum = 0;

  for (obj = 0; obj < numobs; obj++) {

    if (in_inventory (obj, plx) && classmatch (obj, cl)) {
      sum += ovalue (obj);
      if (!silent) {
	if (plev (plx) >= LVL_WIZARD)
	  bprintf ("[%3d]", obj);
	bprintf ("%12.12s:%5d points\n",
		 oname (obj), ovalue (obj));
      }
    }
  }
  return sum;
}

void
valuecom (void)
{
  CLASS_DATA *c;
  int a;

  if (brkword () == -1) {
    bprintf ("Total value of all your possessions: %d points.\n",
	     value_class (findclass ("all"), mynum, True));
  } else {
    do {
      if ((c = findclass (wordbuf)) != NULL) {
	bprintf ("\nTotal value:%*d points.\n",
		 plev (mynum) >= LVL_WIZARD ? 11 : 6,
		 value_class (c, mynum, False));
      } else if ((a = fobn (wordbuf)) == -1) {
	bprintf ("%s: no such object\n", wordbuf);
      } else {
	if (plev (mynum) >= LVL_WIZARD)
	  bprintf ("[%3d]", a);
	bprintf ("%12.12s:%5d points\n", oname (a), ovalue (a));
      }
    }
    while (brkword () != -1);
  }
}

void
putcom (void)
{
  int a;
  char ar[128];
  int c;

  if ((a = ohereandget ()) == -1)
    return;
  if (EMPTY (item2)) {
    bprintf ("Where?\n");
    return;
  }
  if ((c = ob2) == -1) {
    bprintf ("I can't see any %s here.\n", item2);
    return;
  }
#ifdef LOCMIN_ZODIAC		/* Mancini */
  if (c == OBJ_ZODIAC_PENTAGRAM) {
    switch (a) {
    default:
      bprintf ("You try to place the %s on the pentagram, but it falls through \n"
	       "to the floor!\n",
	       oname (a));
      send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	 "%s try's to place the %s on the pentagram, but it falls through\n"
		"to the floor!\n", pname (mynum), oname (a));
      send_msg (ploc (mynum), 0, LVL_MIN, pvis (mynum), mynum, NOBODY,
		"Someone try's to place the %s on the pentagram, but it falls through\n"
		"to the floor!\n", oname (a));
      setoloc (a, ploc (mynum), IN_ROOM);
      return;
      break;
    case OBJ_ZODIAC_FIREBALL:
    case OBJ_ZODIAC_WATERBALL:
    case OBJ_ZODIAC_AIRBALL:
    case OBJ_ZODIAC_DIRTBALL:
    case OBJ_ZODIAC_WINE:
      setobjstate (a, 1);
      setoloc (a, ploc (mynum), IN_ROOM);
      if (((state (OBJ_ZODIAC_FIREBALL) == 1) &&
	   (state (OBJ_ZODIAC_WATERBALL) == 1) &&
	   (state (OBJ_ZODIAC_AIRBALL) == 1) &&
	   (state (OBJ_ZODIAC_DIRTBALL) == 1) &&
	   (state (OBJ_ZODIAC_WINE) == 1))) {
	bprintf ("As you place the %s upon the altar, the altar suddenly flares to an\n"
		 "intense blinding light!  When your vision clears, you are somewhere else!\n\n",
		 oname (a));
	send_msg (ploc (mynum), MODE_NSFLAG | MS (SFL_BLIND), pvis (mynum), LVL_MAX,
		  mynum, NOBODY, "When %s places the %s upon the altar, the altar suddenly flares to\n"
		  "an intense blinding flash!  When your vision clears, you see that %s is gone!",
		  pname (mynum), oname (a), psex (mynum) ? "she" : "he");
	trapch (LOC_ZODIAC_FRONTDOOR);
	send_msg (ploc (mynum), MODE_NSFLAG | MS (SFL_BLIND), pvis (mynum), LVL_MAX,
	 mynum, NOBODY, "%s suddenly appears beside you rubbing %s eyes!\n",
		  pname (mynum), his_or_her (mynum));
      } else {
	bprintf ("The pentagram lights briefly as you place the %s on the pentagram.\n",
		 oname (a));
	send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
		  "The pentagram lights briefly as %s places the %s on the pentagram.\n",
		  pname (mynum), oname (a));
	send_msg (ploc (mynum), 0, LVL_MIN, pvis (mynum), mynum, NOBODY,
		  "The pentagram lights briefly as someone places the %s on the pentagram.\n", oname (a));

      }
      return;
      break;
    }
  }
#endif /* LOCMIN_ZODIAC */

#ifdef LOCMIN_EFOREST

  if (onum (c) == OBJ_EFOREST_HOLE) {
    if (onum (a) != OBJ_EFOREST_HOPE) {
      bprintf ("Nothing happens.\n");
      return;
    }
    if (state (c) == 0) {
      bprintf ("You hear a 'click' sound but nothing seems to happen.\n");
      return;
    }
    bprintf ("The gem clicks into place...\n...and the door opens!\n");
    send_msg (LOC_EFOREST_HOT, 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "You hear a 'click' sound and the door opens!\n");
    setobjstate (OBJ_EFOREST_HOLE, 0);
    return;
  }
  if (c == OBJ_EFOREST_LAKE) {
    if (a != OBJ_EFOREST_SPONGE) {
      bprintf ("Nothing happens.\n");
      return;
    }
    if (state (OBJ_EFOREST_LAKE) == 0) {
      bprintf ("What lake ? It is dried up!\n");
      return;
    } else {
      setobjstate (OBJ_EFOREST_LAKE, 0);
      setobjstate (OBJ_EFOREST_SPONGE, 1);
      bprintf ("The sponge seems to miraculously suck up the"
	       " water in the lake!\n");
      bprintf ("It has dried the entire lake...wow!\n");
      send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
		"A sponge seems to suck up the water in the lake!\n");
      return;
    }
  }
#endif

#ifdef LOCMIN_FROBOZZ
  if (c == OBJ_FROBOZZ_WINDOW_OUTSIDE) {
    if (a != OBJ_FROBOZZ_LEAFLET_MAILBOX) {
      bprintf ("Nothing happens.\n");
      return;
    } else if (state (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE) == 2) {
      setobjstate (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE, 1);
      bprintf ("You hear a nearly inaudible click from "
	       "the southern wall.\n");
      return;
    } else {
      bprintf ("Nothing happens.\n");
      return;
    }
  }
#endif

#ifdef LOCMIN_TALON
  if (c == OBJ_TALON_STAFF) {
    if (a == OBJ_TALON_RUBY && otstbit (OBJ_TALON_FIRESTAFF, OFL_DESTROYED)) {
      oclrbit (OBJ_TALON_FIRESTAFF, OFL_DESTROYED);
      setoloc (OBJ_TALON_FIRESTAFF, oloc (OBJ_TALON_STAFF),
	       ocarrf (OBJ_TALON_STAFF));
      destroy (OBJ_TALON_STAFF);
      destroy (OBJ_TALON_RUBY);
      bprintf ("A blinding red light fills the room as the ruby clicks into "
	       "place.  When you\ncan see again, you find the staff is "
	       "glowing a fiery red.\n");
      send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
	     mynum, NOBODY, "%s affixes a small ruby on the end of a staff "
	     "and suddenly the room is\nfilled with a blinding red light.  "
		"When you can see again, you find the staff\nis glowing a "
		"fiery red.\n", pname (mynum));
      send_msg (ploc (mynum), MODE_NODEAF, LVL_MIN, pvis (mynum) - 1,
		mynum, NOBODY, "You hear a soft click and an 'ouch' "
		"somewhere close by.\n");
    } else {
      bprintf ("It doesn't fit, but it looks interesting.  Maybe you could "
	       "call it art.\n");
    }
    return;
  }
#endif


#ifdef LOCMIN_ANCIENT
  if (c == OBJ_ANCIENT_PEDESTAL) {
    if (a != OBJ_ANCIENT_SUNDISC) {
      bprintf ("Nothing happens.\n");
      return;
    } else {
      bprintf ("The sundisc fits perfectly on top of the pedestal with a loud "
	       "click!\n");
      send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
		"You hear a loud click as %s puts the sundisc on "
		"the pedestal.", pname (mynum));

      setoloc (OBJ_ANCIENT_SUNDISC, ploc (mynum), IN_ROOM);

      osetbit (OBJ_ANCIENT_SUNDISC, OFL_NOGET);
      setobjstate (OBJ_ANCIENT_SUNDISC, 2);
      setobjstate (OBJ_ANCIENT_PEDESTAL, 0);
      setpscore (mynum, pscore (mynum) + ovalue (OBJ_ANCIENT_SUNDISC) * 3);
      set_quest (mynum, Q_SUNDISC);
      return;
    }
  }
#endif

  /* database.c: Put chalice on altar */
  if ((c == OBJ_MOOR_ALTAR) && (a == OBJ_MOOR_CHALICE)) {
    setobjstate (OBJ_MOOR_ALTAR, 1);
    destroy (OBJ_MOOR_CHALICE);
    return;
  }
  if (onum (c) == OBJ_TOWER_CANDLESTICK) {
    if (onum (a) != OBJ_TOWER_RED_CANDLE && onum (a) != OBJ_TOWER_BLUE_CANDLE &&
	onum (a) != OBJ_TOWER_GREEN_CANDLE) {
      bprintf ("You can't do that.\n");
      return;
    }
    if (state (c) != 2) {
      bprintf ("There's already a candle in it!\n");
      return;
    }
    bprintf ("The candle fixes firmly into the candlestick.\n");
    setpscore (mynum, pscore (mynum) + 50);
    destroy (a);
    osetarmor (c, a);
    osetbit (c, OFL_LIGHTABLE);
    osetbit (c, OFL_EXTINGUISH);
    if (otstbit (a, OFL_LIT)) {
      osetbit (c, OFL_LIT);
      setobjstate (c, 0);
      return;
    }
    setobjstate (c, 1);
    oclrbit (c, OFL_LIT);
    return;
  }
  if (onum (c) == OBJ_TOWER_BALL) {
    if (onum (a) == OBJ_TOWER_WAND && oarmor (a) == 0) {
      bprintf ("The wand seems to soak up energy.\n");
      osetarmor (a, 4);
      return;
    }
    bprintf ("Nothing happens.\n");
    return;
  }
  if (c == OBJ_BLIZZARD_SLIME_PIT) {
    if (state (c) == 0) {
      setoloc (a, LOC_BLIZZARD_SLIME, IN_ROOM);
      bprintf ("Ok\n");
      return;
    }
    destroy (a);
    bprintf ("It dissappears with a fizzle into the slime.\n");
    if (onum (a) == OBJ_BLIZZARD_SOAP) {
      bprintf ("The soap dissolves the slime away!\n");
      setobjstate (OBJ_BLIZZARD_SLIME_PIT, 0);
    }
    return;
  }
  if (c == OBJ_TOWER_CHUTE_BOT) {
    bprintf ("You can't do that, the chute leads up from here!\n");
    return;
  }
  if (c == OBJ_TOWER_CHUTE_TOP) {
    if (onum (a) == OBJ_CASTLE_RUNESWORD) {
      bprintf ("You can't let go of it!\n");
      return;
    }
    bprintf ("It vanishes down the chute....\n");
    sendf (oloc (OBJ_TOWER_CHUTE_BOT),
	   "The %s comes out of the chute.\n", oname (a));
    setoloc (a, oloc (OBJ_TOWER_CHUTE_BOT), IN_ROOM);
    return;
  }
  if (c == OBJ_TOWER_HOLE) {
    if (onum (a) == OBJ_TOWER_SCEPTRE && state (OBJ_TOWER_DOOR_SHAZARETH) == 1) {
      setobjstate (OBJ_TOWER_DOOR_TREASURE, 0);
      strcpy (ar, "The door clicks open!\n");
      sendf (oloc (OBJ_TOWER_DOOR_TREASURE), ar);
      sendf (oloc (OBJ_TOWER_DOOR_SHAZARETH), ar);
      return;
    }
    bprintf ("Nothing happens.\n");
    return;
  }
  if (c == a) {
    bprintf ("What do you think this is, the goon show?\n");
    return;
  }
  if (otstbit (c, OFL_CONTAINER) == 0) {
    bprintf ("You can't do that.\n");
    return;
  }
  if (state (c) != 0) {
    bprintf ("It's not open.\n");
    return;
  }
  if (oflannel (a)) {
    bprintf ("You can't take that!\n");
    return;
  }
  if ((ishere (a)) && (dragget ()))
    return;
  if (onum (a) == OBJ_CASTLE_RUNESWORD) {
    bprintf ("You can't let go of it!\n");
    return;
  }
  if (onum (a) == OBJ_LIMBO_POUNCIE) {
    bprintf ("You can't let go of it!\n");
    return;
  }
  if (onum (a) == OBJ_START_UMBRELLA && state (a) == 1) {
    bprintf ("Close it first...\n");
    return;
  }
  if (otstbit (a, OFL_LIT)) {
    bprintf ("I'd try putting it out first!\n");
    return;
  }
  if (!willhold (c, a)) {
    bprintf ("It won't fit.\n");
    return;
  }
  setoloc (a, c, IN_CONTAINER);
  bprintf ("Ok\n");

  send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "\001D%s\003\001c puts the %s in the %s.\n\003",
	    pname (mynum), oname (a), oname (c));

  if (otstbit (a, OFL_GETFLIPS))
    setobjstate (a, 0);
  if (ploc (mynum) == LOC_TOWER_TREASURE && state (OBJ_TOWER_DOOR_TREASURE) == 0
      && ishere (a)) {
    setobjstate (OBJ_TOWER_DOOR_TREASURE, 1);
    strcpy (ar, "The door clicks shut....\n");
    sendf (LOC_TOWER_TREASURE, ar);
    sendf (oloc (OBJ_TOWER_DOOR_SHAZARETH), ar);
  }
}

void
eatcom (void)
{
  int b;
  char s[100];

  if (brkword () == -1) {
    bprintf ("Eat what?\n");
    return;
  }
  if (EQ (wordbuf, "water"))
    strcpy (wordbuf, "spring");
  if ((b = ob1 == -1 ? ob2 : ob1) == -1) {
    bprintf ("It isn't here.\n");
    return;
  }
  switch (onum (b)) {
  case OBJ_MOOR_CHALICE:
    bprintf ("However much blood you drink from the chalice it "
	     "stays just as stained!\n");
    break;
  case OBJ_OAKTREE_TART:
  case OBJ_OAKTREE_CAKES:
  case OBJ_OAKTREE_TOAST:
    bprintf ("That was delicious, but not very filling.\n");
    eat (b);
    setpstr (mynum, pstr (mynum) + 6);
    setpmagic (mynum, pmagic (mynum) + 6);
    break;
  case OBJ_OAKTREE_SOUP:
    bprintf ("As you finish off the last of the caterpillar consume\n");
    bprintf ("you notice a small diamond in the bottom of the cup.\n");
    eat (b);
    create (OBJ_OAKTREE_CUPDIAMOND);
    setoloc (OBJ_OAKTREE_CUPDIAMOND, mynum, CARRIED_BY);
    create (OBJ_OAKTREE_CUPCHINA);
    setoloc (OBJ_OAKTREE_CUPCHINA, mynum, CARRIED_BY);
    setpstr (mynum, pstr (mynum) + 6);
    break;
  case OBJ_TOWER_CAULDRON:
    bprintf ("You feel funny and pass out....\n");
    bprintf ("You wake up elsewhere....\n");
    teletrap (LOC_TOWER_MAGICAL);
    break;
  case OBJ_VALLEY_SPRING:
    bprintf ("Very refreshing.\n");
    break;
  case OBJ_TOWER_POTION:
    setpstr (mynum, maxstrength (mynum));
    setpmagic (mynum, maxmagic (mynum));
    bprintf ("You feel much much stronger!\n");
    setoloc (b, LOC_DEAD_EATEN, IN_ROOM);
    destroy (b);
    break;
  case OBJ_TREEHOUSE_WAYBREAD:
    if (plev (mynum) < LVL_WIZARD && cur_player->pretend < 0) {
      pl1 = (my_random () >> 3) % (numchars - 1);
      if (ststflg (pl1, SFL_OCCUPIED) || pl1 < max_players) {
	bprintf ("There is a sudden feeling of failure...\n");
	break;
      }
      polymorph (pl1, 25);	/* aliased for 25 moves */
    }
    setpstr (mynum, pstr (mynum) + 16);
    setpmagic (mynum, maxmagic (mynum) + 16);
    eat (b);
    break;
  case OBJ_ICECAVE_FOUNTAIN:
    if (plev (mynum) >= LVL_NOVICE && plev (mynum) < LVL_HERO) {
      setpscore (mynum, pscore (mynum) + 40);
      bprintf ("You feel a wave of energy sweeping through you.\n");
    } else {
      bprintf ("Faintly magical by the taste.\n");
      if (plev (mynum) >= LVL_HERO && pstr (mynum) < 10)
	setpstr (mynum, pstr (mynum) + 4);
    }
    break;

#ifdef LOCMIN_ANCIENT
  case OBJ_ANCIENT_FOUNTAIN_OF_YOUTH:
    if (pscore (mynum) >= 20) {
      setpscore (mynum, pscore (mynum) - 20);
      bprintf ("You feel younger and less experienced...\n");
    }
    break;
#endif

#ifdef LOCMIN_TALON
  case OBJ_TALON_SPRING:
    bprintf ("You feel a shifting of the ground under your feet and a "
	     "slight wind on your\nface, and looking about you you find "
	     "you have been transported.\n");
    send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
	      mynum, NOBODY, "%s bends to take a drink from the spring, and "
	    "no sooner than %s does\nso, %s is suddenly gone.  The whisper "
	      "of a cold breeze blows against your\ncheek.\n",
	      pname (mynum), psex (mynum) ? "she" : "he",
	      psex (mynum) ? "she" : "he");
    send_msg (ploc (mynum), 0, LVL_MIN, pvis (mynum) - 1, mynum, NOBODY,
	      "The whisper of a cold breeze blows against your cheek "
	      "suddenly, with no\nexplanation for its presence.\n");
    trapch (LOC_TALON_TALON25);
    send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
	      mynum, NOBODY, "%s suddenly appears beside you in a kneeling "
	      "position.  %s stands up\nand brushes %sself off, surveying "
	      "%s situation.\n", pname (mynum), he_or_she (mynum),
	      him_or_her (mynum), his_or_her (mynum));
    break;
  case OBJ_TALON_POTION:
    setpstr (mynum, maxstrength (mynum));
    bprintf ("You feel magical energies coursing through your veins like "
	     "fire, and when the\nsensations subside, you notice your "
	     "physical wounds have disappeared.\n");
    send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
	   mynum, NOBODY, "%s guzzles a potion, and shortly thereafter you "
	      "notice %s physical\nwounds disappearing.\n", pname (mynum),
	      his_or_her (mynum));
    eat (OBJ_TALON_POTION);
    break;
#endif

  default:
    if (otstbit (b, OFL_FOOD)) {
      eat (b);
      bprintf ("Delicious!\n");

      setpstr (mynum, pstr (mynum) + 12);
      setpmagic (mynum, pmagic (mynum) + 15);
      sprintf (s, "\001P%s\003 greedily devours the %s.\n",
	       pname (mynum), oname (b));
      sillycom (s);
    } else {
      bprintf ("I think I've lost my appetite.\n");
      return;
    }
    break;
  }
}

void
inventory (void)
{
  if (plev (mynum) < LVL_WIZARD) {
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s rummages through %s backpack.\n",
	      pname (mynum), his_or_her (mynum));
  }
  bprintf ("You are carrying:\n");
  aobjsat (mynum, CARRIED_BY, 0);
}


void
listobject (int loc, int mode)
{
  aobjsat (loc, mode, 0);
}

void
mlobjsat (int x, int m)
{
  aobjsat (x, CARRIED_BY, m);
}



/* All OBJectS AT - list all objects at the destination given.
 */
void
aobjsat (int loc, int mode, int marg)
{
  int ostack[64], ostackp = 0;

  char b[80], *s;

  int col;
  Boolean wwl;			/* worn, wielded or lit ? */
  Boolean d, show_contents, empty = True;
  int obj;
  int stp;
  int_set *inv = mode == IN_ROOM ? linv (loc) :
  mode == IN_CONTAINER ? oinv (loc) : pinv (loc);


  for (col = 0; col < marg; col++)
    bprintf (" ");

  for (obj = first_obj (inv); obj != SET_END; obj = next_obj (inv)) {

    if (ovis (obj) > plev (mynum))
      continue;

    show_contents = False;

    s = b;

    if ((mode == CARRIED_BY && iscarrby (obj, loc))
	|| (mode == IN_CONTAINER && iscontin (obj, loc))) {

      empty = False;
      show_contents = True;

      if ((d = otstbit (obj, OFL_DESTROYED))) {
	*s++ = '(';
      }
      strcpy (s, oname (obj));
      s += strlen (oname (obj));

      if (d) {
	*s++ = ')';
      }
      *s++ = ' ';

      wwl = False;

      if (mode == CARRIED_BY) {
	if (iswornby (obj, loc)) {
	  strcpy (s, "<worn");
	  s += strlen (s);
	  wwl = True;
	}
	if (pwpn (loc) == obj
	    && ocarrf (obj) == WIELDED_BY) {
	  if (wwl)
	    *s++ = ',';
	  else {
	    *s++ = '<';
	    wwl = True;
	  }
	  strcpy (s, "wielded");
	  s += strlen (s);
	}
	if (otstbit (obj, OFL_LIT)) {
	  if (wwl)
	    *s++ = ',';
	  else {
	    *s++ = '<';
	    wwl = True;
	  }
	  strcpy (s, "lit");
	  s += strlen (s);
	}
	if (wwl)
	  *s++ = '>';
      }
      *s = 0;

      if (s - b + 1 + col > 79) {
	bprintf ("\n");
	for (col = 0; col < marg; col++)
	  bprintf (" ");
      }
      bprintf ("%s ", b);
      col += strlen (b) + 1;

      if (otstbit (obj, OFL_CONTAINER) && show_contents &&
	  (!otstbit (obj, OFL_OPENABLE) || state (obj) == 0)) {
	ostack[ostackp++] = obj;
      }
    }
  }


  if (empty)
    bprintf ("Nothing");

  bprintf ("\n");

  for (stp = 0; stp < ostackp; stp++) {
    for (col = 0; col < marg; col++)
      bprintf (" ");

    obj = ostack[stp];

    bprintf ("    The %s contains:\n", oname (obj));
    /*                pbfr(); */
    aobjsat (obj, IN_CONTAINER, marg + 8);
  }
}




/* Is o1 contained in o2 ?
 */
Boolean
iscontin (int o1, int o2)
{
  if (ocarrf (o1) != IN_CONTAINER || oloc (o1) != o2)
    return False;

  if (plev (mynum) < LVL_WIZARD && otstbit (o1, OFL_DESTROYED))
    return False;

  return True;
}


/* The room where an object, or its container or its carrier, are at.
 */
int
obj_loc (int obj)
{
  for (; ocarrf (obj) == IN_CONTAINER; obj = oloc (obj)) ;
  return ocarrf (obj) >= CARRIED_BY ? ploc (oloc (obj)) : oloc (obj);
}



/* The 'Find Object By Name' system.
 *
 * Name can be either 1) <object-number>
 *                 or 2) <object-name>
 *                 or 3) <object-name><number-in-sequence-with-that-name>
 */
static int fobnsys (char *name, int ctrl, int ct_inf, int_set * inv);

int
fobn (char *word)
{
  int x;

  /* Look for all available objects (=in room or inventory) */
  if ((x = fobna (word)) != -1)
    return ovis (x) <= plev (mynum) ? x : -1;

  /* we didn't find any available object...look for *any* object */
  return fobnsys (word, 0, 0, NULL);
}

/* Look for available objects */
int
fobna (char *word)
{
  int i;

  return (i = fobnc (word)) >= 0 ? i : fobnh (word);
}

/* Look for objects contained in ct */
int
fobnin (char *word, int ct)
{
  return fobnsys (word, 5, ct, oinv (ct));
}

/* look for objects carried by me */
int
fobnc (char *word)
{
  return fobncb (word, mynum);
}

/* look for objects carried by 'by' */
int
fobncb (char *word, int by)
{
  return fobnsys (word, 3, by, pinv (by));
}

/* Look for objects that's here */
int
fobnh (char *word)
{
  return fobnsys (word, 4, ploc (mynum), linv (ploc (mynum)));
}

/* Look for a obj. that's here and not scenery (can be taken) */
int
fobn_can_take (char *word)
{
  return fobnsys (word, 6, ploc (mynum), linv (ploc (mynum)));
}


static int
fobnsys (char *name, int ctrl, int ct_inf, int_set * inv)
{
  char b[ONAME_LEN + 1], *p = b;
  int i, obj, num;
  char *n;

  if (name == NULL || strlen (name) > ONAME_LEN)
    return -1;

  while (*name != '\0' && isalpha (*name))
    *p++ = *name++;
  *p = '\0';

  if (isdigit (*name)) {
    num = atoi (name);

    while (isdigit (*++name)) ;
    if (*name != '\0')
      return -1;
  } else if (*name != '\0') {
    return -1;
  } else
    num = 1;

  if (num < 0 || num >= numobs)
    return -1;

  if (*b == '\0') {
    if (ovis (num) > plev (mynum))
      return -1;

    switch (ctrl) {
    case 0:
      return num;
    case 3:
      return iscarrby (num, ct_inf) ? num : -1;
    case 4:
      return ishere (num) ? num : -1;
    case 5:
      return iscontin (num, ct_inf) ? num : -1;
    case 6:
      return ishere (num) && !oflannel (num) ? num : -1;
    default:
      return -1;
    }
  }
  if (ctrl == 0) {		/* Look for first object with this name */

    for (obj = 0; obj < numobs; obj++) {
      n = EQ (b, oname (obj)) ? oname (obj) :
	EQ (b, oaltname (obj)) ? oaltname (obj) : NULL;

      if (n != NULL && --num == 0 && ovis (obj) <= plev (mynum)) {
	cur_player->wd_it = n;
	return obj;
      }
    }
    return -1;
  }
  for (i = 0; i < set_size (inv); i++) {

    obj = int_number (i, inv);

    n = EQ (b, oname (obj)) ? oname (obj) :
      EQ (b, oaltname (obj)) ? oaltname (obj) : NULL;

    if (n != NULL && ovis (obj) <= plev (mynum)) {
      cur_player->wd_it = n;

      switch (ctrl) {
      case 3:			/* Look for objects carried by ct_inf */
	if (iscarrby (obj, ct_inf) && --num == 0)
	  return obj;
	break;
      case 4:			/* Look for objects that's here */
	if (ishere (obj) && --num == 0)
	  return obj;
	break;
      case 5:			/* look for objects contained in ct_inf */
	if (iscontin (obj, ct_inf) && --num == 0)
	  return obj;
	break;
      case 6:			/* objects that are here and gettable */
	if (ishere (obj) && !oflannel (obj) && --num == 0)
	  return obj;
	break;
      default:
	return -1;
      }
    }
  }
  return -1;
}


/* Find an object's in-game index from its ID.
 * Return -1 if not found.
 */
int
find_object_by_id (long int id)
{
  long int x;

  if (id >= 0 && id < num_const_obs)
    return id;

  return (x = lookup_entry (id, &id_table)) == NOT_IN_TABLE
    || x < 0 || x >= numobs ? -1 : x;
}




int
get1objfrom (int ob, int container)
{
  int l;
  char *s;
  char bf[81];

  /* FANTASY */
#ifdef LOCMIN_FANTASY
  if (ob == OBJ_FANTASY_GOLDSWORD && state (ob) == 1) {
    bprintf ("As you go to take the sword the shopkeeper mentions he would "
	     "like a gold\nnugget in exchange for it. He then mentions that "
	"in the mountains behind\nhis shop there are many gold mines.\n\n");
    return 0;
  }
  if (ob == OBJ_FANTASY_SILVERSWORD && state (ob) == 1) {
    bprintf ("As you go to take the sword the shopkeeper mentions he would "
	     "like a bag\nof silver in exchange for it. He then speaks of "
	   "the dwarven cave to the west\nthat is said to have many hidden "
	     "treasures guarded by a monster.\n\n");
    return 0;
  }
  if (ob == OBJ_FANTASY_SILVERARMOR && state (ob) == 1) {
    bprintf ("As you go to take the armor the shopkeeper mentions he would "
	     "like a bag\nof silver in exchange for it. He then speaks of "
	   "the dwarven cave to the west\nthat is said to have many hidden "
	     "treasures guarded by a monster.\n\n");
    return 0;
  }
#endif

#ifdef LOCMIN_ANCIENT
  if (ob == OBJ_ANCIENT_SUNDISC && state (ob) == 1) {
    if (!iscarrby (OBJ_ANCIENT_ESTONE, mynum) ||
	!iscarrby (OBJ_ANCIENT_QFEATHER, mynum)) {
      bprintf ("You feel that you need more magical equipment "
	       "than just your hands for this job.\n");
      return 0;
    } else {
      setobjstate (OBJ_ANCIENT_SUNDISC, 0);
    }
  }
#endif

  if (ob == OBJ_BLIZZARD_SHIELD) {
    if (ishere (OBJ_BLIZZARD_SHIELD1))
      ob = OBJ_BLIZZARD_SHIELD1;
    else if (ishere (OBJ_BLIZZARD_SHIELD2))
      ob = OBJ_BLIZZARD_SHIELD2;
    else if (container == -1) {
      if (otstbit (OBJ_BLIZZARD_SHIELD1, OFL_DESTROYED))
	ob = OBJ_BLIZZARD_SHIELD1;
      else if (otstbit (OBJ_BLIZZARD_SHIELD2, OFL_DESTROYED))
	ob = OBJ_BLIZZARD_SHIELD2;
      if (ob == OBJ_BLIZZARD_SHIELD1 || ob == OBJ_BLIZZARD_SHIELD2)
	create (ob);
      else {
	bprintf ("The shields are too firmly secured to the walls.\n");
	return 0;
      }
    }
  }
  if (oflannel (ob)) {

    int i = fobn_can_take (oaltname (ob));

    if (i == -1)
      i = fobn_can_take (oname (ob));

    if (i == -1) {
      bprintf ("You can't take that!\n");
      return -1;

    } else
      ob = i;
  }
  if (container == -1 || !iscarrby (container, mynum)) {

    if (dragget ())
      return -1;
  }
  if (!cancarry (mynum)) {
    bprintf ("You can't carry any more.\n");
    return -1;
  }
  if (onum (ob) == OBJ_CASTLE_RUNESWORD && state (ob) == 1 &&
      ptothlp (mynum) == -1) {
    bprintf ("It's too well embedded to shift alone.\n");
    return 0;
  }
  if (ob == OBJ_CATACOMB_CUPSERAPH &&
      (l = alive ((max_players + MOB_CATACOMB_SERAPH))) != -1 &&
      ploc (l) == ploc (mynum)) {
    bprintf ("\001pThe Seraph\003 says 'Well done, my %s.  "
	     "Truly you are a %s of virtue.'\n",
	 psex (mynum) ? "daughter" : "son", psex (mynum) ? "woman" : "man");
  }
  setoloc (ob, mynum, CARRIED_BY);
  if (container == -1) {
    *bf = '\0';
  } else {
    sprintf (bf, " from the %s", oname (container));
  }
  send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "\001p%s\003 takes the %s%s.\n", pname (mynum), oname (ob), bf);

  if (otstbit (ob, OFL_GETFLIPS))
    setobjstate (ob, 0);
  if ((ploc (mynum) == LOC_TOWER_TREASURE) &&
      (state (OBJ_TOWER_DOOR_TREASURE) == 0)) {
    setobjstate (OBJ_TOWER_DOOR_TREASURE, 1);
    sendf (LOC_TOWER_TREASURE, s = "The door clicks shut...\n");
    sendf (obj_loc (olinked (OBJ_TOWER_DOOR_TREASURE)), s);	/*Other side of door */
  }
  if (ob == OBJ_CATACOMB_CUPSERAPH &&
      (l = alive ((max_players + MOB_CATACOMB_SERAPH))) != -1 &&
      ploc (l) == ploc (mynum)) {
    bprintf ("The Seraph gestures and you are transported to ...\n");
    send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "%s vanishes, taking \001p%s\003 with him!\n",
	      pname (l), pname (mynum));
    sendf (LOC_START_CHURCH,
	   "\001p%s\003 appears, accompanied by an angel!\n", pname (mynum));
    setploc ((max_players + MOB_CATACOMB_SERAPH), LOC_START_CHURCH);
    set_quest (mynum, Q_GRAIL);
    trapch (LOC_START_CHURCH);
    return 0;
  }
  bprintf ("Ok\n");
  return 0;
}

int
getcom (void)
{
  CLASS_DATA *cl;
  int ob;
  int i;
  int container = -1;
  char bf[1024];

  if (brkword () == -1) {
    bprintf ("Get what?\n");
    return -1;
  }
  if (isdark ()) {
    bprintf ("It's dark!\n");
    return -1;
  }
  if ((cl = findclass (wordbuf)) != NULL) {
    getall (cl);
    return 0;
  }
  ob = fobnh (wordbuf);

#ifdef LOCMIN_RUINS
  if (ob == OBJ_BLIZZARD_LITTABLET) {
    if (etstflg (mynum, EFL_LIGHT)) {
      bprintf ("Something won't let you take the tablet.\n");
      return -1;
    }
  }
  if (ob == OBJ_RUINS_CROWN && state (OBJ_RUINS_CROWN) == 1) {
    setobjstate (OBJ_RUINS_CROWN, 0);
    setobjstate (OBJ_RUINS_SCROLL, 1);
  }
  if (ob == OBJ_RUINS_SCROLL) {
    if (state (OBJ_RUINS_CROWN) == 1) {
      bprintf ("Try moving the crown first.\n");
      return -1;
    }
    if (state (OBJ_RUINS_SCROLL) == 1) {
      setobjstate (OBJ_RUINS_SCROLL, 0);
    }
  }
#endif

#ifdef LOCMIN_MITHDAN
  if ((ob == OBJ_MITHDAN_CRYSTAL) && (state (OBJ_MITHDAN_CRYSTAL) == 1)) {
    setploc (MOB_MITHDAN_DRUID + max_players, oloc (OBJ_MITHDAN_CRYSTAL));
    setploc (MOB_MITHDAN_DRUID2 + max_players, oloc (OBJ_MITHDAN_CRYSTAL));
    bprintf ("The druids, angered at you for defiling their sacred relic, shout an\n");
    bprintf ("ancient oath and leap towards you.\n");
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "A pair of druids appear on each side of %s and attack!\n", pname (mynum));
    setpagg (MOB_MITHDAN_DRUID + max_players, 100);
    setpagg (MOB_MITHDAN_DRUID2 + max_players, 100);
    setpspeed (MOB_MITHDAN_DRUID + max_players, 0);
    setpagg (MOB_MITHDAN_DRUID2 + max_players, 100);
    setpspeed (MOB_MITHDAN_DRUID + max_players, 0);
    setpspeed (MOB_MITHDAN_DRUID2 + max_players, 0);
    setobjstate (OBJ_MITHDAN_CRYSTAL, 0);
    hit_player (MOB_MITHDAN_DRUID + max_players, mynum, -1);
    hit_player (MOB_MITHDAN_DRUID2 + max_players, mynum, -1);
  }
#endif

#ifdef LOCMIN_ZODIAC		/* Mancini */
  switch (ob) {
  default:
    break;
  case OBJ_ZODIAC_FIREBALL:
  case OBJ_ZODIAC_WATERBALL:
  case OBJ_ZODIAC_AIRBALL:
  case OBJ_ZODIAC_DIRTBALL:
  case OBJ_ZODIAC_WINE:
    setobjstate (ob, 0);
    setoloc (ob, mynum, CARRIED_BY);
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s takes the %s from the pentagram.\n",
	      pname (mynum), oname (ob));
    send_msg (ploc (mynum), 0, LVL_MIN, pvis (mynum), mynum, NOBODY,
	      "Someone takes the %s from the pentagram.\n", oname (ob));
    bprintf ("Ok\n");
    return 0;
    break;
  }
#endif /* LOCMIN_ZODIAC */

  i = stp;
  strcpy (bf, wordbuf);
  if (brkword () != -1 && (EQ (wordbuf, "from") || EQ (wordbuf, "out"))) {
    if (brkword () == -1) {
      bprintf ("From what?\n");
      return -1;
    }
    if ((container = fobna (wordbuf)) == -1) {
      bprintf ("You can't take things from that!  It's not here!\n");
      return -1;
    }
    if (otstbit (container, OFL_LOCKABLE) &&
	state (container) == 2 && !ohany (1 << OFL_KEY)) {
      bprintf ("The %s is locked, and you have no key.\n", oname (container));
      return -1;
    }
    if ((otstbit (container, OFL_OPENABLE) || otstbit (container, OFL_LOCKABLE))
	&& state (container) > 0) {
      bprintf ("You open the %s.\n", oname (container));
      setobjstate (container, 0);
    }
    ob = fobnin (bf, container);
  }
  stp = i;
  if (ob == -1) {
    bprintf ("It's not here.\n");
    return -1;
  }
  return get1objfrom (ob, container);
}

static void
getall (CLASS_DATA * cl)
{
  int x;

  if (brkword () != -1) {
    getallfr (cl);
    return;
  }
  for (x = lfirst_obj (ploc (mynum)); x != SET_END; x = lnext_obj (ploc (mynum))) {

    if (ishere (x) && !oflannel (x) && classmatch (x, cl)) {
      strcpy (strbuf, oname (x));
      stp = 0;
      bprintf ("%s: ", oname (x));
      if (getcom () == -1)
	break;
    }
  }
}

static void
getallfr (CLASS_DATA * cl)
{
  int container;
  int ob;

  if (EQ (wordbuf, "from")) {
    if (brkword () == -1) {
      bprintf ("From what?\n");
      return;
    }
  }
  if ((container = fobna (wordbuf)) == -1) {
    bprintf ("That isn't here.\n");
    return;
  }
  /* Do items */

  for (ob = ofirst_obj (container); ob != SET_END; ob = onext_obj (container)) {

    if (iscontin (ob, container) && !oflannel (ob) && classmatch (ob, cl)) {
      sprintf (strbuf, "%s from %s", oname (ob), oname (container));
      stp = 0;
      bprintf ("%s: ", oname (ob));
      if (getcom () == -1)
	break;
    }
  }
}

static void
dropall (CLASS_DATA * cl)
{
  int ob;

  for (ob = pfirst_obj (mynum); ob != SET_END; ob = pnext_obj (mynum)) {

    if (iscarrby (ob, mynum) && classmatch (ob, cl)) {
      strcpy (strbuf, oname (ob));
      stp = 0;
      bprintf ("%s: ", oname (ob));
      if (dropobj () == -1)
	return;
    }
  }
}


/* Is the item in the same room as the player ?
 */
Boolean
p_ishere (int plr, int item)
{
  if (plev (plr) < LVL_WIZARD && otstbit (item, OFL_DESTROYED))
    return False;

  if (ocarrf (item) != IN_ROOM || oloc (item) != ploc (plr)
      || ploc (plr) == 0)
    return False;

  return True;
}


Boolean
ishere (int item)
{
  return p_ishere (mynum, item);
}



Boolean
iscarrby (int item, int user)
{
  if (plev (mynum) < LVL_WIZARD && otstbit (item, OFL_DESTROYED))
    return False;

  if (ocarrf (item) < CARRIED_BY)
    return False;

  if (oloc (item) != user)
    return False;

  return True;
}


/* Is the object in a players inventory ?
 * (also handles objects in a container in a container etc...)
 */
Boolean
in_inventory (int obj, int player)
{
  while (ocarrf (obj) == IN_CONTAINER)
    obj = oloc (obj);

  return iscarrby (obj, player);
}



int
dropobj (void)
{
  CLASS_DATA *cl;
  int a, i, l, j;

  if (brkword () == -1) {
    bprintf ("Drop what?\n");
    return -1;
  }
  if ((cl = findclass (wordbuf)) != NULL) {
    dropall (cl);
    return 0;
  }
  if ((a = fobnc (wordbuf)) == -1) {
    bprintf ("You don't have it.\n");
    return -1;
  }
  if (plev (mynum) < LVL_WIZARD && onum (a) == OBJ_CASTLE_RUNESWORD) {
    bprintf ("You can't let go of it!\n");
    return 0;
  }
  if (plev (mynum) < LVL_WIZARD && onum (a) == OBJ_LIMBO_POUNCIE) {
    bprintf ("You can't let go of it!\n");
    return 0;
  }
  if (a == OBJ_CATACOMB_CUPSERAPH
      && ploc ((max_players + MOB_CATACOMB_SERAPH)) == LOC_START_CHURCH)
    setplev ((max_players + MOB_CATACOMB_SERAPH), -2);
  l = ploc (mynum);
  /* MINE LADDER */
  if (l == LOC_QUARRY_TUNNEL || l == LOC_QUARRY_LADDER_UU) {
    bprintf ("The %s falls down the ladder.\n", oname (a));
    l = LOC_QUARRY_LADDER_UD;
  }
  if ((l >= LOC_QUARRY_LADDER_D4 && l <= LOC_QUARRY_LADDER_D2) ||
      (l >= LOC_QUARRY_LADDER_D1 && l <= LOC_QUARRY_LADDER_UD)) {
    bprintf ("The %s falls down the ladder.\n", oname (a));
    l = LOC_QUARRY_MINE;
  }
  /* ALL AT SEA */
  if (ltstflg (l, LFL_ON_WATER) && onum (a) != OBJ_VILLAGE_BOAT &&
      onum (a) != OBJ_VILLAGE_RAFT && onum (a) != OBJ_ANCIENT_CANOE) {
    bprintf ("The %s sinks into the sea.\n", oname (a));
    l = LOC_SEA_7;
  }
  /* OAKTREE */
  if ((l >= LOC_OAKTREE_MAGNOLIA && l <= LOC_OAKTREE_TREE1)
      || l == LOC_OAKTREE_ILEX
      || (l >= LOC_OAKTREE_WALNUT && l <= LOC_OAKTREE_FIG)) {
    bprintf ("The %s falls through the leaves to the ground far below.\n",
	     oname (a));
    l = LOC_OAKTREE_GROVE;
    sendf (l, "Something falls to the ground.\n");
  }
  for (j = 0; (i = pits[j++]) != -1;)
    if (oloc (i) == l)
      break;
  if ((i >= 0 && state (i) == 0) || (oloc (OBJ_SEA_HOLE) == l)) {
    bprintf ("The %s disappears into the bottomless pit.....\n", oname (a));
    send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "\001p%s\003 drops the %s into the pit.\n",
	      pname (mynum), oname (a));
    dropinpit (a);
    return 0;
  } else if (i >= 0) {
    bprintf ("The %s disappears into the bottomless pit....."
	     "and hits bottom.\n", oname (a));
    send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "\001p%s\003 drops the %s into the pit.\n",
	      pname (mynum), oname (a));
    l = LOC_CATACOMB_CHAMBER;
    setoloc (a, l, IN_ROOM);	/* to spherical room in CATACOMB section */
    sendf (l, "Something falls to the ground.\n");
    return 0;
  }
  setoloc (a, l, IN_ROOM);
  send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "\001p%s\003 drops the %s.\n", pname (mynum), oname (a));
  if ((l = ploc (mynum)))
    bprintf ("Ok\n");
  return 0;
}

void
dropinpit (int o)
{
  int i;

  if (ozone (o) >= num_const_zon && plev (mynum) < LVL_WIZARD) {
    send_msg (DEST_ALL, 0, LVL_PROPHET, LVL_MAX, NOBODY, NOBODY,
	    "&+B[&+CCheat (Object)&*: Zone: &+W%s &*Pitted by: &+W%s&+B]\n",
	      zname (ozone (o)), pname (mynum));
    mudlog ("CHEAT: Object from Zone: %s, Pitted By: %s",
	    zname (ozone (o)), pname (mynum));
    bprintf ("&+WYou were just caught pitting a cloned item!\n");
    if (pscore (mynum) >= ovalue (o)) {
      bprintf ("&+WYou lose &+C%d &+Wpoints from your score!\n",
	       ovalue (o) / 2);
      setpscore (mynum, pscore (mynum) - (ovalue (o) / 2));
    }
  } else {
    setpscore (mynum, pscore (mynum) + ovalue (o));
  }
  osetbit (o, OFL_DESTROYED);
  setoloc (o, LOC_PIT_PIT, IN_ROOM);

#ifdef LOCMIN_MITHDAN
  if (o == OBJ_MITHDAN_CRYSTAL) {
    set_quest (mynum, Q_MITHDAN);
  }
#endif

#ifdef LOCMIN_TALON
  if (o == OBJ_TALON_CLAW) {
    if (alive (i = max_players + MOB_TALON_TALON) == -1 &&
	pscore (i) == mynum) {
      set_quest (mynum, Q_TALON);
    }
  }
#endif

  if (o == OBJ_WASTE_THRONE) {
    if (alive (i = max_players + MOB_WASTE_DJINNI) == -1 &&
	pscore (i) == mynum) {
      set_quest (mynum, Q_FIERY_KING);
    }
  } else if (o == OBJ_TOWER_CROWN) {
    if (alive (i = max_players + MOB_TOWER_SHAZARETH) == -1 &&
	pscore (i) == mynum) {
      set_quest (mynum, Q_TOWER);
    }
  }
  if (otstbit (o, OFL_CONTAINER)) {

    for (i = ofirst_obj (o); i != SET_END; i = onext_obj (o))
      if (iscontin (i, o)) {
	dropinpit (i);
      }
  }
}

#if (OFL_MAX == 96)
void 
list_objects (int n)
{
  int i, a;

  for (i = 0; i < lnumobs (ploc (mynum)); i++) {

    a = lobj_nr (i, ploc (mynum));

    if (ishere (a) && (n == OFL_MAX || ltstflg (ploc (mynum), n)) &&
	ovis (a) <= plev (mynum)) {
      if (state (a) > 3)
	continue;
      if (!EMPTY (olongt (a, state (a)))) {
	if (otstbit (a, OFL_DESTROYED))
	  bprintf ("--");
	oplong (a);
	cur_player->wd_it = oname (a);
      } else if (plev (mynum) >= LVL_ARCHWIZARD) {
	bprintf ("<marker>%s\n", oname (a));
      }
    }
  }
}

#else

/* List the objects at the current players location.
 */
void
list_objects (int n, Boolean f)
{
  int i, a;

  for (i = 0; i < lnumobs (ploc (mynum)); i++) {

    a = lobj_nr (i, ploc (mynum));

    if (ishere (a) && (n == 0 || otstmask (a, n) == f) &&
	ovis (a) <= plev (mynum)) {
      if (state (a) > 3)
	continue;
      if (!EMPTY (olongt (a, state (a)))) {
	if (otstbit (a, OFL_DESTROYED))
	  bprintf ("--");
	oplong (a);
	cur_player->wd_it = oname (a);
      } else if (plev (mynum) >= LVL_ARCHWIZARD) {
	bprintf ("<marker>%s\n", oname (a));
      }
    }
  }
}

#endif

void
dumpitems ()
{
  dumpstuff (mynum, ploc (mynum));
}

void
dumpstuff (int n, int loc)
{
  int b;

  for (b = pfirst_obj (n); b != SET_END; b = pnext_obj (n))
    if (iscarrby (b, n)) {
      if (loc == LOC_PIT_PIT)
	dropinpit (b);
      else
	setoloc (b, loc, IN_ROOM);
    }
}

/* Set a players weapon. Sets both the carry-flag of the weapon and the
 * 'pweapon' entry for that player in the world. A negative value removes
 * any current weapon. Return True if a new weapon got set for the player,
 * else False.
 */
Boolean
set_weapon (int plr, int wpn)
{
  int owpn, i;

  /* Erase any weapon we were allready wielding:
   */
  if ((owpn = pwpn (plr)) != -1 && oloc (owpn) == plr) {
    if (ocarrf (owpn) == BOTH_BY)
      setcarrf (owpn, WORN_BY);
    else if (ocarrf (owpn) == WIELDED_BY)
      setcarrf (owpn, CARRIED_BY);
  }
  if (wpn < 0 || odamage (wpn) == 0 || ocarrf (wpn) < CARRIED_BY
      || oloc (wpn) != plr) {

    setpwpn (plr, -1);
    return False;
  }
  i = WIELDED_BY;

  if (ocarrf (wpn) == WORN_BY)
    i = BOTH_BY;

  setpwpn (plr, wpn);
  setcarrf (wpn, i);

  return True;
}





void
oplong (int x)
{
  char *t = olongt (x, state (x));

  if (!EMPTY (t)) {
    bprintf ("%s\n", t);
  }
}


int
gotanything (int x)
{
  int ct;

  for (ct = 0; ct < pnumobs (x); ct++) {

    if (iscarrby (pobj_nr (ct, x), x))
      return 1;
  }

  return 0;
}


static CLASS_DATA *
findclass (char *n)
{
  CLASS_DATA *cl;

  for (cl = class_data; cl->class_name != NULL; cl++) {
    if (EQ (cl->class_name, n))
      return cl;
  }
  return NULL;
}

static Boolean
classmatch (int ob, CLASS_DATA * cl)
{
  register short st;

  return (cl == NULL ||
	  (((st = cl->class_state) < 0 || st == state (ob)) &&
	   otstmask (ob, cl->class_mask)));
}

Boolean
is_classname (char *name)
{
  return name != NULL && findclass (name) != NULL;
}


/* Can player 'plyr' carry any more objects now ?
 */
Boolean
cancarry (int plyr)
{
  int i, a;
  int num = 0;

  if (plev (plyr) >= LVL_WIZARD || plyr >= max_players)
    return True;

  for (i = 0; i < pnumobs (plyr); i++) {

    a = pobj_nr (i, plyr);

    if (iscarrby (a, plyr) && !iswornby (a, plyr))
      num++;
  }

  return num < plev (plyr) + 5;
}


Boolean
iswornby (int ob, int plr)
{
  return isworn (ob) && iscarrby (ob, plr);
}

/* Is object 'ob' available ?
 */
Boolean
isavl (int ob)
{
  return ishere (ob) || iscarrby (ob, mynum);
}


/* Try to reset an object, return True on success.
 */
Boolean
reset_object (int o)
{
  int loc = 0;

  osetbaseval (o, ovalue_reset (o));
  osetsize (o, osize_reset (o));
  osetvis (o, ovis_reset (o));
  osetdamage (o, odamage_reset (o));
  osetarmor (o, oarmor_reset (o));
  state (o) = state_reset (o);
  obits (o) = obits_reset (o);

  if (!opermanent (o)) {
    if (ocarrf_reset (o) == IN_ROOM) {
      if ((loc = find_loc_by_id (oloc_reset (o))) == 0) {
	destroy (o);
	return False;
      }
    } else if (ocarrf_reset (o) == IN_CONTAINER) {
      if ((loc = find_object_by_id (oloc_reset (o))) < 0) {
	destroy (o);
	return False;
      }
    } else if (ocarrf_reset (o) >= CARRIED_BY) {
      if ((loc = find_mobile_by_id (oloc_reset (o))) < 0) {
	destroy (o);
	return False;
      }
    }
  } else {
    loc = oloc_reset (o);
    if (ocarrf_reset (o) >= CARRIED_BY)
      loc += max_players;
  }

  setoloc (o, loc, ocarrf_reset (o));

  return True;
}



void
setobjstate (int obj, int state)
{
  if (state >= 0 && state <= omaxstate (obj)
      && (olinked (obj) == -1 || state <= omaxstate (olinked (obj)))) {

    state (obj) = state;

    if (olinked (obj) != -1) {
      state (olinked (obj)) = state;
    }
  } else {
    mudlog ("ERROR: Attempt to set object %s[%d] to state %d",
	    oname (obj), obj, state);
  }
}


void
destroy (int ob)
{
  osetbit (ob, OFL_DESTROYED);
  setoloc (ob, LOC_DEAD_DESTROYED, IN_ROOM);
}

void
eat (int ob)
{
  if (!opermanent (ob) && otemporary (ob)) {
    destruct_object (ob, NULL);
  } else {
    osetbit (ob, OFL_DESTROYED);
    setoloc (ob, LOC_DEAD_EATEN, IN_ROOM);
  }
}

void
create (int ob)
{
  oclrbit (ob, OFL_DESTROYED);
}


/* SET Object LOCation.
 */
void
setoloc (int obj, int loc, int c)
{
  /* The object is already in the right place/status
   */
  if (oloc (obj) == loc && ocarrf (obj) == c)
    return;

  /* First remove the object from wherever it is:
   */
  switch (ocarrf (obj)) {
  case IN_ROOM:
    if (exists (oloc (obj)))
      remove_int (obj, linv (oloc (obj)));
    break;
  case IN_CONTAINER:
    if (oloc (obj) >= 0 && oloc (obj) < numobs)
      remove_int (obj, oinv (oloc (obj)));
    break;
  case CARRIED_BY:
  case WORN_BY:
  case WIELDED_BY:
  case BOTH_BY:
    if (oloc (obj) >= 0 && oloc (obj) < numchars)
      remove_int (obj, pinv (oloc (obj)));
    break;
  }

  /* Then add it to the right place:
   */
  switch (c) {
  case IN_ROOM:
    if (exists (loc))
      add_int (obj, linv (loc));
    break;
  case IN_CONTAINER:
    if (loc >= 0 && loc < numobs)
      add_int (obj, oinv (loc));
    break;
  case CARRIED_BY:
  case WORN_BY:
  case WIELDED_BY:
  case BOTH_BY:
    if (loc >= 0 && loc < numchars)
      add_int (obj, pinv (loc));
    break;
  }

  oloc (obj) = loc;
  ocarrf (obj) = c;

  if (c >= WIELDED_BY)
    set_weapon (loc, obj);
}



/* is there an object, either carried by the player or in the same
 * room that satisfies certain criteria (determined by mask) ?
 */
Boolean
p_ohany (int plr, int mask)
{
  int i, a;

  mask &= 0xffff;

  for (i = 0; i < lnumobs (ploc (plr)); i++) {

    a = lobj_nr (i, ploc (plr));

    if (p_ishere (plr, a) && (obits (a).l & mask))
      return True;
  }

  for (i = 0; i < pnumobs (plr); i++) {

    a = pobj_nr (i, plr);

    if (iscarrby (a, plr) && (obits (a).l & mask))
      return True;
  }

  return False;
}


Boolean
ohany (int mask)
{
  return p_ohany (mynum, mask);
}

int
ovalue (int ob)
{
#ifdef USE_TSCALE
  return (tscale () * obaseval (ob) / 9);
#else
  return obaseval (ob);
#endif
}


char *
xdesloc (char *b, int loc, int cf)
{
  char k[256];
  char v[256];
  char buff[256];

  *buff = '\0';

  while (cf == IN_CONTAINER) {

    sprintf (v, "In the %s ", oname (loc));
    strcat (buff, v);
    cf = ocarrf (loc);
    loc = oloc (loc);
  }

  if (cf >= CARRIED_BY) {
    if (cf == CARRIED_BY)
      strcat (buff, "Carried");
    if (cf == WORN_BY)
      strcat (buff, "Worn");
    if (cf == WIELDED_BY)
      strcat (buff, "Wielded");
    if (cf == BOTH_BY)
      strcat (buff, "Worn & Wielded");
    sprintf (v, " by %s ", see_name (mynum, loc));
    strcat (buff, v);
    loc = ploc (loc);
  }
  if (!exists (loc)) {
    if (plev (mynum) < LVL_GOD)
      return strcpy (b, "Out in the void");
    else {
      sprintf (b, "NOT IN UNIVERSE[%d]", loc);
      return b;
    }
  }
  if (*buff != '\0')
    strcat (buff, "in ");

  if (plev (mynum) >= LVL_WIZARD)
    sprintf (v, "| %s", xshowname (k, loc));
  else
    *v = '\0';

  strcat (buff, sdesc (loc));

  sprintf (b, "%-40.40s%s", buff, v);

  return b;
}

void
desloc (int loc, int cf)
{
  char b[512];

  bprintf ("%s\n", xdesloc (b, loc, cf));
}

int
getobjloc (int obj)
{
  int loc = oloc (obj);

  while (ocarrf (obj) == IN_CONTAINER) {
    obj = loc;
    loc = oloc (obj);
  }

  if (ocarrf (obj) != IN_ROOM)
    return (ploc (loc));
  else
    return loc;
}

Boolean
otstmask (int ob, int v)
{
  return dtst_bit (&obits (ob), v);
}


/* Will the container x hold object y ?
 */
Boolean
willhold (int x, int y)
{
  int i, a, sum = 0;

  for (i = 0; i < onumobs (x); i++) {

    a = oobj_nr (i, x);

    if (iscontin (a, x))
      sum += osize (a);
  }

  sum += osize (y);

  return sum <= osize (x);
}

int
ohereandget (void)
{
  int obj;

  if (EMPTY (item1)) {
    bprintf ("Tell me more?\n");
    return -1;
  }
  if ((obj = ob1) == -1) {
    bprintf ("It isn't here.\n");
  }
  return obj;
}


/* plx drops the objects that he carries that are not worn or wielded,
 */
void
drop_some_objects (int plx)
{
  int obj;

  for (obj = pfirst_obj (plx); obj != SET_END; obj = pnext_obj (plx)) {
    if (ocarrf (obj) == CARRIED_BY && oloc (obj) == plx) {
      setoloc (obj, ploc (plx), IN_ROOM);
    }
  }
}


char *
xdesrm (char *b, int loc, int cf)
{
  char k[25];
  char v[30];

  if (plev (mynum) < LVL_WIZARD && cf == IN_ROOM && loc == LOC_LIMBO_LIMBO) {
    return strcpy (b, "[Somewhere]");
  }
  if (cf == IN_CONTAINER) {
    sprintf (b, "In the %s", oname (loc));
    return b;
  }
  if (cf >= CARRIED_BY) {
    if (!seeplayer (loc))
      return strcpy (b, "[Somewhere]");
    else {
      sprintf (b, "Carried by %s", pname (loc));
      return b;
    }
  }
  if (!exists (loc)) {
    if (plev (mynum) < LVL_ARCHWIZARD)
      return strcpy (b, "Out in the void");
    else {
      sprintf (b, "NOT IN UNIVERSE[%d]", loc);
      return b;
    }
  }
  if (plev (mynum) >= LVL_WIZARD)
    sprintf (v, "| %s", xshowname (k, loc));
  else
    *v = 0;

  sprintf (b, "%-30.30s %s", sdesc (loc), v);
  return b;
}


void
desrm (int loc, int cf)
{
  char b[80];

  bprintf ("%s\n", xdesrm (b, loc, cf));
}

char *
odescrm (int obj)
{
  static char b[300];
  char buff[300];

  if (ocarrf (obj) == IN_CONTAINER) {
    odescrm (oloc (obj));
  } else {
    if (ocarrf (obj) >= CARRIED_BY) {
      sprintf (b, "%s", xshowname (buff, ploc (oloc (obj))));
      return b;
    }
    if (!exists (oloc (obj))) {
      sprintf (b, "Unknown (%d)", oloc (obj));
      return b;
    }
    sprintf (b, "%s", xshowname (buff, oloc (obj)));
    return b;
  }
  return NULL;
}

void
move_pouncie (void)
{
  int ok = 1;
  int room;

  while (ok != 0) {
    room = my_random () % num_const_locs;	/* Get a random room    */
    room = room - (room * 2);	/* MUST be negative     */

    if (room == LOC_CASTLE_TORTURE || room == LOC_CASTLE_MAIDEN) {
      ok = 1;
    } else {
      if (plev (mynum) < LVL_AVATAR) {
	if (!ltstflg (room, LFL_DEATH) &&	/* Cannot be deathroom  */
	    !ltstflg (room, LFL_ON_WATER) &&	/* Cannot be on water   */
	    !ltstflg (room, LFL_ONE_PERSON) &&	/* Cannot be one-person */
	    !ltstflg (room, LFL_PRIVATE))	/* Cannot be private    */
	  ok = 0;
      } else {
	ok = 0;
      }
    }
  }

  setoloc (OBJ_LIMBO_POUNCIE, room, IN_ROOM);
}

/************************************************************************
 * Wear Commands							*
 ************************************************************************/

/* This function is where all the specials for wearcom() must be placed
 * so that wearcom and wearall will be able to handle wearcom specials.
 */
void
wearspecials (int a)
{
#ifdef LOCMIN_FANTASY
  if (onum (a) == OBJ_FANTASY_MANA &&
      ploc (mynum) == LOC_FANTASY_TREE2) {
    bprintf ("As you put the Pendant of Mana around your "
	     "neck the world begins to blur and\nshake around "
	     "as your eyes close. You feel yourself floating in "
	     "air and then\nthe moving stops. You open your eyes "
	     "in a different part of the world..\n\n");
    setploc (mynum, LOC_FANTASY_START);
    trapch (ploc (mynum));
    bprintf ("You have freed the mana tree from the grasp of the "
	     "evil Lord Glaive.\n\n");
    set_quest (mynum, Q_MANA);
  }
#endif

#ifdef LOCMIN_ANCIENT
  if (onum (a) == OBJ_ANCIENT_HEALBALSAM) {
    if (pstr (mynum) < maxstrength (mynum) - 20) {
      setpstr (mynum, pstr (mynum) + 20);
      bprintf ("You feel some of your wounds dissappear.\n");
    } else if (pstr (mynum) < maxstrength (mynum)) {
      setpstr (mynum, maxstrength (mynum));
      bprintf ("The balsam heals all your wounds!\n");
    } else {
      bprintf ("The balsam has a nice cooling effect.\n");
    }
    destroy (a);
    return;
  }
  if (onum (a) == OBJ_ANCIENT_EMBBALSAM) {
    bprintf ("You start applying the embalming balsam ...\n");
    bprintf ("You begin to feel sleepy, and after a while some mummies\n"
	     "turn up helping you with the embalming ...\n");
    destroy (a);
    crapup ("\tThe mummies carry you away to a safe restingplace."
	    " You are dead...\n", SAVE_ME);
    return;
  }
#endif
}

void
wearcom (void)
{
  int a, b;

  if ((a = ohereandget ()) == -1)
    return;
  if (!iscarrby (a, mynum)) {
    bprintf ("You don't have it.\n");
    return;
  }
  wearspecials (a);
  if (iswornby (a, mynum)) {
    bprintf ("You're already wearing it.\n");
    return;
  }
  if (check_armor (mynum, a)) {
    bprintf ("You have reached the maximum armor class of %d.\n", MAXARMOR);
    bprintf ("Wearing more armor will not increase your AC beyond that number.\n");
  }
  if (is_shield (a) && wears_shield (mynum)) {
    bprintf ("You can't use two shields at once.\n");
    return;
  }
  if (is_armor (a) && wears_armor (mynum)) {
    bprintf ("You can't wear two suits of armor at once.\n");
    return;
  }
  if (is_mask (a) && wears_mask (mynum)) {
    bprintf ("You can't wear two masks at once.\n");
    return;
  }
  if (!otstbit (a, OFL_WEARABLE)) {
    bprintf ("Is this a new fashion?\n");
    return;
  }
  b = WORN_BY;
  if (ocarrf (a) == WIELDED_BY)
    b = BOTH_BY;
  setcarrf (a, b);

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "\001p%s\003 wears the %s.\n", pname (mynum), oname (a));

  bprintf ("Ok\n");
}

void
wearall (void)
{
  int obj, objlst[100], objidx = 0;
  Boolean worn = False;

  bprintf ("You wear: ");
  for (obj = 0; obj < numobs; obj++) {
    if (iscarrby (obj, mynum) && otstbit (obj, OFL_WEARABLE)) {
      worn = True;
      objlst[objidx++] = obj;
      bprintf ("%s ", oname (obj));
      if (is_mask (obj) && wears_mask (mynum))
	continue;
      if (is_armor (obj) && wears_armor (mynum))
	continue;
      if (is_shield (obj) && wears_shield (mynum))
	continue;
      setcarrf (obj, WORN_BY);
    }
  }

  if (!worn) {
    bprintf ("Nothing.\n");
  } else {
    bprintf ("\n");
    for (obj = 0; obj < objidx; obj++) {
      wearspecials (objlst[obj]);
    }
  }

  /* Report if player has reached MaxArmor */
  if (player_armor (mynum) > MAXARMOR)
    bprintf ("You have reached the maximum armor class of %d.\n", MAXARMOR);
}

void
lockcom ()
{
  if (ob1 == -1) {
    bprintf ("Lock what?\n");
    return;
  }
  switch (ob1) {
  default:
    if (!otstbit (ob1, OFL_LOCKABLE))
      bprintf ("You can't lock that!\n");
    else if (state (ob1) > 2)
      bprintf ("It's already locked.\n");
    else if (!ohany (1 << OFL_KEY))
      bprintf ("You have no key.\n");
    else {
      setobjstate (ob1, 2);
      bprintf ("OK.\n");
    }
  }
}

void
unlockcom ()
{
  if (ob1 == -1) {
    bprintf ("What do you want to unlock?\n");
    return;
  }
  switch (ob1) {
  default:
    if (!otstbit (ob1, OFL_LOCKABLE))
      bprintf ("You can't unlock that!\n");
    else if (state (ob1) == 2)
      bprintf ("It's already unlocked.\n");
    else if (!ohany (1 << OFL_KEY))
      bprintf ("You have no key.\n");
    else
      setobjstate (ob1, 1);
  }
}

void
closecom ()
{
  if (ob1 == -1) {
    bprintf ("What would you like to close?\n");
    return;
  }
  switch (ob1) {
  case OBJ_START_UMBRELLA:
    bprintf ("You close the umbrella.\n");
    setobjstate (OBJ_START_UMBRELLA, 0);
    break;
  default:
    if (!otstbit (ob1, OFL_OPENABLE))
      bprintf ("You can't close that!\n");
    else if (state (ob1) > 0)
      bprintf ("It's already closed.\n");
    else {
      setobjstate (ob1, 1);
      bprintf ("Ok.\n");
    }
  }
}

void
opencom ()
{
  char *cant_open = "You can't open that!\n";

  if (ob1 == -1) {
    bprintf ("What would you like to open?\n");
    return;
  }
  switch (ob1) {
  case OBJ_FOREST_TREEEATING:
  case OBJ_FOREST_INSIDETREE:
    bprintf ("You can't shift the tree!\n");
    break;
  case OBJ_VILLAGE_BOT_BOARDS:
  case OBJ_VILLAGE_TOP_BOARDS:
    bprintf ("You shift the floorboards, with much heaving and tugging, to "
	     "reveal an exit\nbeyond.\n");
    setobjstate (OBJ_VILLAGE_TOP_BOARDS, 0);
    break;
  case OBJ_TOWER_DOOR_SHAZARETH:
    if (state (ob1) >= 1)
      bprintf ("It seems to be magically closed.\n");
    else
      bprintf ("It's already open.\n");
    break;
  case OBJ_TOWER_DOOR_TREASURE:
    bprintf ("You can't shift the door at all from this side.\n");
    break;
  case OBJ_START_UMBRELLA:
    bprintf ("You open the umbrella.\n");
    setobjstate (OBJ_START_UMBRELLA, 1);
    break;
  default:
    if (!otstbit (ob1, OFL_OPENABLE)) {
      bprintf (cant_open);
    } else { 
      if (state (ob1) == 0) {
        bprintf ("It's already open.\n");
      } else {
        if (state (ob1) == 1) {
          bprintf ("Ok.\n");
          setobjstate (ob1, 0);
        } else {
          if (state (ob1) == 2) {
            if (!ohany (1 << OFL_KEY)) {
	      bprintf ("It seems to be locked.\n");
            } else {
	      bprintf ("Ok.\n");
	      setobjstate (ob1, 0);
            }
          }
        }
      }
    }
    break;
  }
}

void
ringcom (void)
{
  switch (ob1) {
  case OBJ_VILLAGE_BELL:
    broad ("There is an almighty &+WBONG&*!\n");
    break;
  default:
    bprintf ("You can't ring that.\n");
    break;
  }
}

void
digcom (void)
{
  if (ploc (mynum) == oloc (OBJ_ICECAVE_HOLE_SNOW_CAVE) ||
      ploc (mynum) == oloc (OBJ_ICECAVE_HOLE_G_HALL)) {
    if (state (OBJ_ICECAVE_HOLE_G_HALL) == 1) {
      if (iscarrby (ob1, mynum) && otstbit (ob1, OFL_WEAPON)) {
	bprintf ("You rapidly dig through to another passage.\n");
	setobjstate (OBJ_ICECAVE_HOLE_G_HALL, 0);
	return;
      } else {
	bprintf ("The ice and snow are thick, you'll need something "
		 "to hack through it with.\n");
	return;
      }
    } else {
      bprintf ("You widen the hole but with little effect.");
      return;
    }
  }
  if ((ploc (mynum) == oloc (OBJ_LEDGE_TUNNWEST) ||
       ploc (mynum) == oloc (OBJ_LEDGE_TUNNEAST)) &&
      state (OBJ_LEDGE_TUNNEAST) == 1) {
    bprintf ("You dig your way through the rockfall, and soon clear "
	     "the passage.\n");
    setobjstate (OBJ_LEDGE_TUNNEAST, 0);
    return;
  }
  if (ploc (mynum) == oloc (OBJ_CHURCH_SLAB_TOP) &&
      otstbit (OBJ_CHURCH_SLAB_TOP, OFL_DESTROYED)) {
    bprintf ("You uncover a stone slab!\n");
    oclrbit (OBJ_CHURCH_SLAB_TOP, OFL_DESTROYED);
    return;
  }
  if (ploc (mynum) == oloc (OBJ_OAKTREE_TOPFOXHOLE) &&
      state (OBJ_OAKTREE_TOPFOXHOLE) == 1) {
    bprintf ("Although there are fox tracks in the grass, you can't "
	     "find the fox hole.\n");
    return;
  }
  bprintf ("You find nothing.\n");
  return;
}

void
tiecom (void)
{
  switch (ob1) {
  case OBJ_LIMBO_ROPE:
    switch (ploc (mynum)) {
    case LOC_TREEHOUSE_PORCH:
    case LOC_VALLEY_ESIDE:
      if (state (OBJ_VALLEY_LADDER2ELF) == 0) {
	bprintf ("Why?   There is already a rope there!\n");
	break;
      }
      setobjstate (OBJ_TREEHOUSE_LADDER, 0);
      destroy (OBJ_LIMBO_ROPE);
      bprintf ("You tie the rope to the tree.\n");
      break;
    case LOC_LEDGE_PIT:
    case LOC_MOOR_PIT:
      if (state (OBJ_LEDGE_ROPEBOTPIT) == 0) {
	bprintf ("Why? There is already a rope there!\n");
	break;
      }
      setobjstate (OBJ_LEDGE_ROPEBOTPIT, 0);
      destroy (OBJ_LIMBO_ROPE);
      bprintf ("You tie the rope to the pit.\n");
      break;
    default:
      bprintf ("You have nothing to tie the rope to.\n");
      break;
    }
    break;
  default:
    bprintf ("You cannot tie that!\n");
    break;
  }
  return;
}

void
untiecom (void)
{
  if (ploc (mynum) != oloc (ob1)) {
    bprintf ("It isn't here.\n");
    return;
  }
  switch (ob1) {
  case OBJ_VALLEY_LADDER2ELF:
  case OBJ_TREEHOUSE_LADDER:
    if (state (OBJ_VALLEY_LADDER2ELF) == 1) {
      sendf (mynum, "You can't untie that.\n");
      break;
    } else
      setobjstate (OBJ_VALLEY_LADDER2ELF, 1);
    oclrbit (OBJ_VALLEY_LADDER2ELF, OFL_NOGET);
    setoloc (OBJ_VALLEY_LADDER2ELF, mynum, CARRIED_BY);
    sendf (mynum, "You untie the rope.\n");
    break;
  }


  switch (ob1) {
  case OBJ_VALLEY_LADDER2ELF:
  case OBJ_TREEHOUSE_LADDER:
    if (state (OBJ_VALLEY_LADDER2ELF) == 1) {
      sendf (mynum, "You cannot untie that!\n");
      break;
    }
    create (OBJ_LIMBO_ROPE);
    setoloc (OBJ_LIMBO_ROPE, ploc (mynum), IN_ROOM);
    setobjstate (OBJ_VALLEY_LADDER2ELF, 1);
    bprintf ("You untie the rope.\n");
    break;
  case OBJ_LEDGE_ROPEWEST:
  case OBJ_VALLEY_ROPEEAST:
    if (state (OBJ_VALLEY_ROPEEAST) == 1) {
      sendf (mynum, "You cannot untie that!\n");
      break;
    }
    if (oloc (OBJ_LIMBO_ROPE) == LOC_DEAD_DESTROYED ||
	oloc (OBJ_LIMBO_ROPE) == LOC_LIMBO_LIMBO) {
      create (OBJ_LIMBO_ROPE);
      setoloc (OBJ_LIMBO_ROPE, ploc (mynum), IN_ROOM);
    }
    setobjstate (OBJ_VALLEY_ROPEEAST, 1);
    bprintf ("You untie the rope.\n");
    break;
  case OBJ_MOOR_ROPETOPPIT:
  case OBJ_LEDGE_ROPEBOTPIT:
    if (state (OBJ_MOOR_ROPETOPPIT) == 1) {
      sendf (mynum, "You cannot untie that!\n");
      break;
    }
    create (OBJ_LIMBO_ROPE);
    setoloc (OBJ_LIMBO_ROPE, ploc (mynum), IN_ROOM);
    setobjstate (OBJ_MOOR_ROPETOPPIT, 1);
    bprintf ("You untie the rope.\n");
    break;
  default:
    sendf (mynum, "You cannot untie that!\n");
    break;
  }
  return;
}

void
playcom (void)
{
  if (ob1 == OBJ_FOREST_PIPES) {
    bprintf ("Much as you try, the pipes only make nasty squeaking noises.\n");
    return;
  }
  if (ob1 == OBJ_BLIZZARD_HARP) {
    bprintf ("A beautiful harp melody floats to you on a gentle breeze.\n");
    return;
  }
}

void
rollcom (void)
{
  int a;

  if ((a = ohereandget ()) == -1)
    return;
  switch (a) {
  case OBJ_BLIZZARD_PILLAR_WEST:
  case OBJ_BLIZZARD_PILLAR_EAST:
    gamecom ("push pillar", True);
    break;
  case OBJ_BLIZZARD_BOULDER:
    gamecom ("push boulder", True);
    break;
  default:
    bprintf ("You can't roll that.\n");
  }
}

void
emptycom (void)
{
  int a, b;
  char x[128];

  if ((a = ohereandget ()) == -1)
    return;

  if (otstbit (a, OFL_LOCKABLE) && state (a) == 2 && !ohany (1 << OFL_KEY)) {
    bprintf ("The %s is locked, and you have no key.\n", oname (a));
    return;
  } else if ((otstbit (a, OFL_OPENABLE) || otstbit (a, OFL_LOCKABLE))
	     && state (a) > 0) {
    bprintf ("You open the %s.\n", oname (a));
    setobjstate (a, 0);
  }
  for (b = ofirst_obj (a); b != SET_END; b = onext_obj (a)) {
    if (iscontin (b, a)) {
      setoloc (b, mynum, CARRIED_BY);
      bprintf ("You empty the %s from the %s.\n", oname (b), oname (a));
      sprintf (x, "drop %s", oname (b));
      gamecom (x, False);
    }
  }
}

void
blowcom (void)
{
  int a;
  char s[100];

  if ((a = ohereandget ()) == -1)
    return;
  if (onum (a) == OBJ_CAVE_BAGPIPES) {
    broad ("\001dA hideous wailing sounds echos all around.\n\003");
    return;
  }
  if (onum (a) == OBJ_OAKTREE_WHISTLE) {
    broad ("\001dA strange ringing fills your head.\n\003");
    if (alive (max_players + MOB_OAKTREE_OTTIMO) != -1) {
      bprintf ("A small dachshund bounds into the room "
	       "and leaps on you playfully.\n");
      sprintf (s, "A small dachshund bounds into the room "
	       "and leaps on %s playfully.\n", pname (mynum));
      sillycom (s);
      setploc (max_players + MOB_OAKTREE_OTTIMO, ploc (mynum));
    }
    return;
  }
  if (onum (a) == OBJ_LABYRINTH_HORN) {
    broad ("\001dA mighty horn blast echoes around you.\n\003");
    if (ploc (mynum) >= LOC_SEA_TREASURE && ploc (mynum) <= LOC_SEA_1 &&
	oarmor (OBJ_SEA_EXCALIBUR) == 0) {
      setoloc (OBJ_SEA_EXCALIBUR, ploc (mynum), IN_ROOM);
      setobjstate (OBJ_SEA_EXCALIBUR, 1);
      bprintf ("A hand breaks through the water holding up "
	       "the sword Excalibur!\n");
      osetarmor (OBJ_SEA_EXCALIBUR, 1);
      set_quest (mynum, Q_EXCALIBUR);
    }
    return;
  }
  bprintf ("You can't blow that.\n");
}


void
lightcom (void)
{
  int a;
  char s[100];

  if ((a = ohereandget ()) == -1)
    return;
  if (!ohany (1 << OFL_LIT) && !ststflg (mynum, SFL_LIT)) {
    bprintf ("You have nothing to light things from.\n");
    return;
  }
#ifdef LOCMIN_EFOREST
  if (a == OBJ_EFOREST_THORNS || a == OBJ_EFOREST_THORNSEAST) {
    if (state (OBJ_EFOREST_THORNS) == 0) {
      bprintf ("The thorns have already been burned away.\n");
      return;
    }
    setobjstate (OBJ_EFOREST_THORNS, 0);
    bprintf ("You burn the wall of thorns away!\n");
    bprintf ("Behind them you can see the entrance to a cave.\n");
    sprintf (s, "%s burns the wall of thorns away!\n", pname (mynum));

    send_msg (LOC_EFOREST_THORNY, 0, LVL_MIN, LVL_MAX, mynum, NOBODY, s);
    send_msg (LOC_EFOREST_CAVE, 0, LVL_MIN, LVL_MAX, mynum, NOBODY, s);

    return;
  }
#endif

  if (a == OBJ_FOREST_INSIDETREE) {
    bprintf ("The tree screams and thrashes around.  In its dying throes "
	     "you dash free!\n");
    trapch (LOC_FOREST_F4);
    destroy (OBJ_FOREST_TREEEATING);
    setpscore (mynum, pscore (mynum) + 100);
    return;
  }
  if (!otstbit (a, OFL_LIGHTABLE))
    bprintf ("You can\'t light that!\n");
  else if (otstbit (a, OFL_LIT))
    bprintf ("It\'s already lit.\n");
  else {
    setobjstate (a, 0);
    osetbit (a, OFL_LIT);
    bprintf ("Ok\n");
  }
}

void
extinguishcom (void)
{
  int a;

  if ((a = ohereandget ()) == -1)
    return;
  if (!otstbit (a, OFL_LIT))
    bprintf ("It\'s not lit!\n");
  else if (!otstbit (a, OFL_EXTINGUISH))
    bprintf ("You can\'t extinguish that!\n");
  else {
    setobjstate (a, 1);
    oclrbit (a, OFL_LIT);
    bprintf ("Ok\n");
  }
}

void
pushcom (void)
{
  int x;
  char s[128];

  if (brkword () == -1) {
    bprintf ("Push what?\n");
    return;
  }
  if ((x = fobna (wordbuf)) == -1) {
    bprintf ("That is not here.\n");
    return;
  }
  /* If it's the original object OR a copy:
   */
  switch (onum (x)) {

  case OBJ_BLIZZARD_TRIPWIRE:
    bprintf ("The tripwire moves and a huge stone crashes down from above!\n");
    broad ("\001dYou hear a thud and a squelch in the distance.\n\003");
    crapup ("             S   P    L      A         T           !\n", SAVE_ME);
    break;
  case OBJ_BLIZZARD_BOOKCASE:
    bprintf ("A trapdoor opens at your feet and you plumment downwards!\n");
    sillycom ("\001p%s\003 disappears through a trapdoor!\n");
    teletrap (LOC_BLIZZARD_FISSURE);
    return;
  }


  /* If it's the original object (only), not a copy:
   */
  switch (x) {

#ifdef LOCMIN_EFOREST

  case OBJ_EFOREST_BUTTON:
    setobjstate (OBJ_EFOREST_BUTTON, 0);
    setobjstate (OBJ_EFOREST_THRONE_CATHEDRAL,
		 1 - state (OBJ_EFOREST_THRONE_CATHEDRAL));

    sendf (ploc (mynum),
	"You hear a grinding sound from near the entrance of the caves.\n");

    sprintf (s, "You hear a grinding sound as a mysterious force moves "
	     "the throne!\n");

    sendf (LOC_EFOREST_EASTEND, s);
    sendf (LOC_EFOREST_BOTTOM, s);
    break;
#endif

#ifdef LOCMIN_FROBOZZ
  case OBJ_FROBOZZ_BUTTON_OUTSIDE:
    if (state (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE) == 1) {
      setobjstate (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE, 0);

      sendf (LOC_FROBOZZ_OUTSIDE, "Without a sound the whole southern "
	     "wall moves some feet westwards!\n");

      sendf (LOC_FROBOZZ_VAULT, "Without a sound the whole northern wall "
	     "moves some feet westwards!\n");
      break;
    }
    if (state (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE) == 0) {
      setobjstate (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE, 2);
      sendf (LOC_FROBOZZ_OUTSIDE, "The southern wall glides some feet "
	     "to the east closing the vault!\n");

      sendf (LOC_FROBOZZ_VAULT, "The northern wall glides some feet to "
	     "the east closing the vault!\n");
      break;
    }
    if (state (OBJ_FROBOZZ_VAULTDOOR_OUTSIDE) == 2) {
      bprintf ("Nothing happens.\n");
      break;
    }
  case OBJ_FROBOZZ_RUG_LIVING:
    if (state (OBJ_FROBOZZ_RUG_LIVING) == 1) {
      setobjstate (OBJ_FROBOZZ_RUG_LIVING, 0);
      setobjstate (OBJ_FROBOZZ_TRAPDOOR_LIVING, 2);
      bprintf ("You move the rug and uncover a trapdoor!\n");
      sprintf (s, "%s moves the heavy rug and uncovers a trapdoor!\n",
	       pname (mynum));
      sillycom (s);
    } else {
      bprintf ("You move the rug, but find nothing.\n");
      sprintf (s, "%s moves the rug.\n", pname (mynum));
      sillycom (s);
    }
    break;
#endif
#ifdef LOCMIN_ANCIENT
  case OBJ_ANCIENT_MOONCROSS:
    if (state (OBJ_ANCIENT_MOONCROSS) > 0) {
      setobjstate (OBJ_ANCIENT_MOONCROSS, 0);
      if (state (OBJ_ANCIENT_BIGCROSS) > 0) {
	bprintf ("You move the cross to one side, but nothing happens.\n");
      } else {
	bprintf ("You hear a loud hollow sound from a room nearby!\n");
	setobjstate (OBJ_ANCIENT_HOLE_ANC31, 0);
      }
    } else {
      setobjstate (OBJ_ANCIENT_MOONCROSS, 1);
      setobjstate (OBJ_ANCIENT_HOLE_ANC31, 1);
      bprintf ("You move the cross back into place.\n");
    }
    break;
  case OBJ_ANCIENT_BIGCROSS:
    if (state (OBJ_ANCIENT_BIGCROSS) > 0) {
      setobjstate (OBJ_ANCIENT_BIGCROSS, 0);
      if (state (OBJ_ANCIENT_MOONCROSS) > 0) {
	bprintf ("You move the cross to one side, but nothing happens.\n");
      } else {
	bprintf ("You hear a loud hollow sound from a room nearby!\n");
	setobjstate (OBJ_ANCIENT_HOLE_ANC31, 0);
      }
    } else {
      setobjstate (OBJ_ANCIENT_BIGCROSS, 1);
      setobjstate (OBJ_ANCIENT_HOLE_ANC31, 1);
      bprintf ("You move the cross back into place.\n");
    }
    break;
#endif

#ifdef LOCMIN_RUINS
  case OBJ_BLIZZARD_ALTAR:
    if (state (OBJ_BLIZZARD_PLATFORM_ALTAR) == 2) {
      setobjstate (OBJ_BLIZZARD_PLATFORM_ALTAR, 1);
      setobjstate (OBJ_BLIZZARD_ALTAR, 0);
      bprintf ("You move the altar to the side, uncovering the platform.\n");
      break;
    }
  case OBJ_BLIZZARD_BRICK:
    if (state (OBJ_BLIZZARD_ALTAR) == 1) {
      bprintf ("That is not here.\n");
      break;
    } else {
      setobjstate (OBJ_BLIZZARD_PLATFORM_ALTAR, 0);
      setobjstate (OBJ_BLIZZARD_BRICK, 0);
      bprintf ("You push the brick, and the platform opens!\n");
      break;
    }

  case OBJ_RUINS_OUTSIDETORCH:
    if (state (OBJ_RUINS_OUTSIDETORCH) == 1) {
      setobjstate (OBJ_RUINS_OUTSIDETORCH, 0);
      bprintf ("You pull the torch down and the west wall collapses!\n");
      break;
    } else {
      bprintf ("The torch has already been moved.\n");
      break;
    }
#endif

#ifdef LOCMIN_PLAYHOUSE
  case OBJ_PLAYHOUSE_SINKHANDLE:
    if (state (OBJ_PLAYHOUSE_SINKHANDLE) == 1) {
      setobjstate (OBJ_PLAYHOUSE_SINKHANDLE, 0);
      bprintf ("You turn the &+Wfaucet &*on.\n");
      broad ("You hear running &+Bwater &*in the distance.\n");
      break;
    } else {
      setobjstate (OBJ_PLAYHOUSE_SINKHANDLE, 1);
      bprintf ("You turn the &+Wfaucet &*off.\n");
      broad ("The running &+Bwater &*in the distance stops.\n");
      break;
    }
#endif

  case OBJ_BLIZZARD_IRONBAR:
    if (state (OBJ_BLIZZARD_PASS_STONE) == 1) {
      setobjstate (OBJ_BLIZZARD_PASS_FISSURE, 0);
      bprintf ("A secret panel opens in the east wall!\n");
      break;
    }
    bprintf ("Nothing happens.\n");
    break;
  case OBJ_BLIZZARD_BOULDER:
    bprintf ("With a mighty heave you manage to move the boulder a few feet\n");
    if (state (OBJ_BLIZZARD_HOLE_CRACK) == 1) {
      bprintf ("uncovering a hole behind it.\n");
      setobjstate (OBJ_BLIZZARD_HOLE_CRACK, 0);
    } else {
      bprintf ("covering a hole behind it.\n");
      setobjstate (OBJ_BLIZZARD_HOLE_CRACK, 1);
    }
    break;
  case OBJ_BLIZZARD_LEVER:
    if (ptothlp (mynum) == -1) {
      bprintf ("It's too stiff.  Maybe you need help.\n");
      return;
    }
    bprintf ("Ok\n");
    if (state (OBJ_BLIZZARD_SLIME_PIT) != 0) {
      sillycom ("\001s%s\002%s pulls the lever.\n\003");
      sendf (oloc (OBJ_BLIZZARD_LEVER),
	     "\001dYou hear a gurgling noise and then silence.\n\003");
      setobjstate (OBJ_BLIZZARD_SLIME_PIT, 0);
      sendf (oloc (OBJ_BLIZZARD_SLIME_PIT),
	  "\001cThere is a muffled click and the slime drains away.\n\003");
    }
    break;
  case OBJ_BLIZZARD_THRONE_CURTAINS:
  case OBJ_BLIZZARD_ROUGH_CURTAINS:
    setobjstate (OBJ_BLIZZARD_THRONE_CURTAINS,
		 1 - state (OBJ_BLIZZARD_THRONE_CURTAINS));
    bprintf ("Ok\n");
    break;
  case OBJ_CASTLE_LEVER:
    setobjstate (OBJ_CASTLE_PORT_INSIDE, 1 - state (OBJ_CASTLE_PORT_INSIDE));
    if (state (OBJ_CASTLE_PORT_INSIDE)) {
      sendf (oloc (OBJ_CASTLE_PORT_INSIDE), "\001cThe portcullis falls.\n\003");
      sendf (oloc (OBJ_CASTLE_PORT_OUTSIDE), "\001cThe portcullis falls.\n\003");
    } else {
      sendf (oloc (OBJ_CASTLE_PORT_INSIDE), "\001cThe portcullis rises.\n\003");
      sendf (oloc (OBJ_CASTLE_PORT_OUTSIDE), "\001cThe portcullis rises.\n\003");
    }
    break;
  case OBJ_BLIZZARD_BRIDGE_LEVER:
    setobjstate (OBJ_BLIZZARD_BRIDGE_HALL, 1 - state (OBJ_BLIZZARD_BRIDGE_HALL));
    if (state (OBJ_BLIZZARD_BRIDGE_HALL)) {
      sendf (oloc (OBJ_BLIZZARD_BRIDGE_HALL), "\001cThe drawbridge rises.\n\003");
      sendf (oloc (OBJ_BLIZZARD_BRIDGE_FIRE), "\001cThe drawbridge rises.\n\003");
    } else {
      sendf (oloc (OBJ_BLIZZARD_BRIDGE_HALL), "\001cThe drawbridge is lowered.\n\003");
      sendf (oloc (OBJ_BLIZZARD_BRIDGE_FIRE), "\001cThe drawbridge is lowered.\n\003");
    }
    break;
  case OBJ_CASTLE_TORCH:
    if (state (OBJ_CASTLE_DOOR_GOLEM) == 1) {
      setobjstate (OBJ_CASTLE_DOOR_GOLEM, 0);
      sendf (oloc (OBJ_CASTLE_DOOR_GOLEM),
	     "A secret door slides quietly open in the south wall!\n");
    } else
      bprintf ("It moves but nothing seems to happen.\n");
    return;
  case OBJ_CHURCH_ROPE:
    if (oarmor (OBJ_CHURCH_ROPE) >= 12)
      bprintf ("\001dChurch bells ring out around you.\n\003");
    else {
      broad ("\001dChurch bells ring out around you.\n\003");
      if (++oarmor (OBJ_CHURCH_ROPE) == 12) {
	bprintf ("A strange ghostly guitarist shimmers briefly before you.\n");
	setpscore (mynum, pscore (mynum) + 300);
	broad ("\001dA faint ghostly guitar solo "
	       "floats through the air.\n\003");
      }
    }
    break;
  case OBJ_CATACOMB_DUST:
    bprintf ("Great clouds of dust billow up, causing you to sneeze "
	     "horribly.\nWhen you're finished sneezing, you notice "
	     "a message carved into one wall.\n");

    broad ("\001dA loud sneeze echoes through the land.\n\003");
    destroy (OBJ_CATACOMB_DUST);
    create (OBJ_CATACOMB_KOAN);
    break;
  case OBJ_ORCHOLD_BOTCOVER:
    bprintf ("You can't seem to get enough leverage to move it.\n");
    return;
  case OBJ_ORCHOLD_TOPCOVER:
    if (ptothlp (mynum) == -1) {
      bprintf ("You try to shift it, but it's too heavy.\n");
      break;
    }
    sillytp (ptothlp (mynum), "pushes the cover aside with your help.");
    setobjstate (x, 1 - state (x));
    oplong (x);
    return;
  case OBJ_ORCHOLD_SWITCH:
    if (state (x)) {
      bprintf ("A hole slides open in the north wall!\n");
      setobjstate (x, 0);
    } else
      bprintf ("You hear a little 'click' sound.\n");
    return;
  case OBJ_BLIZZARD_STATUE_DOWN:
    if (ptothlp (mynum) == -1) {
      bprintf ("You can't shift it alone, maybe you need help.\n");
      break;
    }
    sillytp (ptothlp (mynum), "pushes the statue with your help.");

#ifdef LOCMIN_TALON
  case OBJ_TALON_CHINA_FIG:
    if (state (x)) {
      bprintf ("A large stone slab slides away in the floor, revealing a "
	       "passage leading down.\n");
      send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
		mynum, NOBODY, "As %s pushes the china figurine, a large "
		"stone slab slides away in the\nfloor, revealing a passage "
		"leading down.\n", pname (mynum));
      send_msg (ploc (mynum), MODE_NOBLIND, LVL_MIN, pvis (mynum) - 1,
		mynum, NOBODY, "The china figurine suddenly leans over and "
		"a large stone slab slides away in\nthe floor, revealing a "
		"passage leading down.\n");
      setobjstate (x, 0);
      setobjstate (OBJ_TALON_DOOR, 0);
      return;
    } else {
      bprintf ("The slab of stone slides over the hole in the floor, "
	       "blocking the passage.\n");
      send_msg (ploc (mynum), MODE_NOBLIND, pvis (mynum), LVL_MAX,
		mynum, NOBODY, "%s returns the china figurine to an upright "
		"position, and the slab of\nstone slides over the hole, "
		"blocking the passage.\n", pname (mynum));
      send_msg (ploc (mynum), MODE_NOBLIND, LVL_MIN, pvis (mynum) - 1,
		mynum, NOBODY, "The china figurine suddenly returns to an "
		"upright position and the slab of\nstone slides over the "
		"hole, blocking the passage.\n");
      setobjstate (x, 1);
      setobjstate (OBJ_TALON_DOOR, 1);
      return;
    }
    break;
#endif

    /* FALLTHROUGH */
  default:
    if (otstbit (x, OFL_PUSHABLE)) {
      setobjstate (x, 0);
      oplong (x);
      return;
    }
    if (otstbit (x, OFL_PUSHTOGGLE)) {
      setobjstate (x, 1 - state (x));
      oplong (x);
      return;
    }
    bprintf ("Nothing happens.\n");
  }
}

void
usecom (void)
{
  int obj;

  if ((obj = ob1) == -1) {
    bprintf ("That isn't here.\n");
    return;
  }
#ifdef LOCMIN_PLAYHOUSE
  if (obj == OBJ_PLAYHOUSE_TOILET) {
    if (state (OBJ_PLAYHOUSE_TOILETLID) == 1) {
      bprintf ("You may want to lift the lid first.\n");
      return;
    }
    if (state (OBJ_PLAYHOUSE_TOILET) == 1) {
      bprintf ("&+GEwww!! &*Someone forgot to flush last time "
	       "they used it!!\n");
      bprintf ("You decide &+Wnot &*to use the toilet. If only "
	       "they had &+Cflushed&*.\n");
      return;
    } else {
      if (psex (mynum) && !psitting (mynum)) {
	bprintf ("Shouldn't you sit down, ma'am?\n");
	return;
      }
      setobjstate (OBJ_PLAYHOUSE_TOILET, 1);
      bprintf ("Isn't that &+Cr&+Be&+Cf&+Br&+Ce&+Bs&+Ch&+Bi&+Cn&+Bg&+W? "
	       "&+WDon't &*forget to &+Wflush!&*\n");
      send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
		"\001p%s\003 uses the toilet and looks more refreshed.\n",
		pname (mynum));
      broad ("A &+Wloud, &+Cr&+Be&+Cf&+Br&+Ce&+Bs&+Ch&+Bi&+Cn&+Bg "
	     "&+Ytinkle &*is heard throughout the land.\n");
      return;
    }
  }
#endif
  bprintf ("You can't use that!\n");
}

void
flushcom (void)
{
  int obj;

  if ((obj = ob1) == -1) {
    bprintf ("That isn't here.\n");
    return;
  }
#ifdef LOCMIN_PLAYHOUSE
  if (obj == OBJ_PLAYHOUSE_TOILET) {
    if (plev (mynum) < LVL_ENCHANTER)
      setpscore (mynum, pscore (mynum) + 10);

    setobjstate (OBJ_PLAYHOUSE_TOILET, 0);
    bprintf ("You flush the toilet. Wow! Look at the pretty &+Bblue &*water!\n");
    broad ("You hear a &+WPorcelain God &*gargling in the distance.\n");
    return;
  }
#endif
  bprintf ("You can't flush that!\n");
}
