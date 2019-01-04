
#include <unistd.h>
#include <stdlib.h>
#include "kernel.h"
#include <time.h>
#include <string.h>
#include "levels.h"

#ifdef VARGS
#include <stdarg.h>
#endif

#include "timing.h"
#include "log.h"
#include "uaf.h"
#include "bprintf.h"
#include "verbs.h"
#include "wizard.h"
#include "parse.h"
#include "sendsys.h"
#include "mobile.h"
#include "objsys.h"
#include "rooms.h"
#include "flags.h"
#include "pflags.h"
#include "lflags.h"
#include "sflags.h"
#include "cflags.h"
#include "locations.h"
#include "fight.h"
#include "wizlist.h"
#include "commands.h"
#include "clone.h"
#include "mud.h"
#include "log.h"
#include "logcolors.h"

extern char *Oflags[];

/* The SYSTEM command
 */
void
systemcom ()
{
  char x[MAX_COM_LEN], y[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_RAW) && plev (mynum) < LVL_ARCHWIZARD) {
    erreval ();
    return;
  }
  getreinput (x);
  sprintf (y, "&+W[SYSTEM] &+C: &*%s&#&#\n", x);
  broad (y);

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_AVATAR, LVL_MAX, mynum,
	  NOBODY, "&+B[&+CSystem &*by &+W\001p%s\003&+B]\n", pname (mynum));
}

/* Call For Reset
 */
void
cfrcom ()
{
  char x[MAX_COM_LEN], y[MAX_COM_LEN];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  getreinput (x);
  sprintf (y, "&+W[Call For Reset by %s] &+C: &*%s&#&#\n", pname (mynum), x);
  broad (y);
}

/* The RAW command
 */
void
rawcom ()
{
  char x[MAX_COM_LEN], y[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_RAW)) {
    erreval ();
    return;
  }
  getreinput (x);
  sprintf (y, "&+W[\001p%s\003] &+C: &*%s&#&#\n", pname (mynum), x);
  broad (y);

}

/* The TXTRAW command
 */
void
textrawcom ()
{
  char x[600], y[1000], send[1600];

  if (!ptstflg (mynum, PFL_TEXTRAW)) {
    erreval ();
    return;
  }
  if (my_brkword () == -1) {
    bprintf ("Textraw what?");
    return;
  }
  sprintf (x, "%s", wordbuf);
  getreinput (y);

  sprintf (send, "&+W[%s&+W] &+C: &+w%s&#&#\n", x, y);
  broad (send);

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_GOD, LVL_MAX, mynum,
	 NOBODY, "&+B[&+CTextRaw &*by &+W\001p%s\003&+B]\n", pname (mynum));

}

/* The TOURNAMENT Command. Toggle Tournament-mode.
 */
void
tournamentcom ()
{
  if (plev (mynum) < LVL_ARCHWIZARD) {
    erreval ();
    return;
  }
  bprintf ("Tournament-mode is now &+W%s&*.\n",
       (the_world->w_tournament = !the_world->w_tournament) ? "ON" : "OFF");

  mudlog ("SYSTEM: Tournament mode turned %s by %s",
	(the_world->w_tournament = !the_world->w_tournament) ? "On" : "Off",
	  pname (mynum));

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	    "&+B[&*Tournament mode turned &+C%s &*by &+W\001p%s\003&+B]\n",
	(the_world->w_tournament = !the_world->w_tournament) ? "On" : "Off",
	    pname (mynum));

}

void
colorlog (char param[300], FILE * fp)
{
  int begin, end, x, i;

  if ((x = strtlookup (param, LogTable, &begin, &end)) < 0) {
    fprintf (fp, "%s", param);
    return;
  }
  for (i = 0; i < begin; i++)
    fprintf (fp, "%c", param[i]);
  fprintf (fp, "%s", LogColors[x]);
  for (; i < end; i++)
    fprintf (fp, "%c", param[i]);
  fprintf (fp, "&*");
  for (; i < strlen (param); i++)
    fprintf (fp, "%c", param[i]);
}

void
syslogcom (void)
{
  FILE *sysfp, *pagefp;
  char buffer[300], search[MAX_COM_LEN], tmpbuf[300];
  char *sptr, tmpname[100];
  int lines = 0;
  int sysloglen = fnumlines (LOG_FILE);

  if (!ptstflg (mynum, PFL_SYSLOG)) {
    erreval ();
    return;
  }
  sprintf (tmpname, "TMP/Syslog.%s", pname (mynum));

  if ((pagefp = fopen (tmpname, "wt")) == NULL) {
    bprintf ("Error: Cannot create temporary file.");
    mudlog ("ERROR: Cannot create temporary file for syslog pager");
    return;
  }
  if ((sysfp = fopen (LOG_FILE, "rt")) == NULL) {
    bprintf ("Error: Cannot read syslog file.");
    mudlog ("ERROR: Cannot read syslog file.");
    return;
  }
  if (EMPTY (txt1)) {
    sptr = ctime (&global_clock);
    sptr += 4;
    sptr[8] = '\0';
    sprintf (search, "%s", sptr);
  } else {
    getreinput (search);
    if ((lines = atoi (search)) > 0) {
      fileseek (sysfp, (sysloglen - lines));
      strcpy (search, " ");
    }
    if (search[0] == '*') {
      fileseek (sysfp, 1);
      strcpy (search, " ");
    }
  }

  while (!feof (sysfp)) {
    fgets (buffer, sizeof (buffer), sysfp);
    strcpy (tmpbuf, buffer);
    if (strstr (uppercase (tmpbuf), uppercase (search))) {
      if (!feof (sysfp)) {
	colorlog (buffer, pagefp);
      }
    }
  }
  fclose (sysfp);
  fclose (pagefp);
  file_pager (tmpname);
  unlink (tmpname);
}

/* (C) Rassilon (Brian Preble) */
void
echocom ()
{
  char x[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_ECHO)) {
    bprintf ("You hear echos.\n");
    return;
  }
  getreinput (x);
  if (EMPTY (x)) {
    bprintf ("ECHO what?\n");
    return;
  }
  send_msg (ploc (mynum), MODE_NPFLAG | MP (PFL_NOECHO), LVL_MIN, LVL_MAX, mynum,
	    NOBODY, "%s\n", x);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You echo: %s\n", x);
  else
    bprintf ("Ok\n");

  send_msg (ploc (mynum), MODE_SFLAG | MS (SFL_SEEEXT), LVL_ARCHWIZARD, LVL_MAX, mynum,
	    NOBODY, "&+B[&+CEcho &*by &+W\001p%s\003&+B]\n", pname (mynum));
}


void
levechocom (void)
{
  int lev;
  char x[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_ECHO)) {
    bprintf ("You hear echos.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Echo to what level?\n");
    return;
  }
  lev = atoi (wordbuf);
  if (lev < 0 || lev > plev (mynum)) {
    bprintf ("You cannot echo to level %d.\n", lev);
    return;
  }
  getreinput (x);

  if (EMPTY (x)) {
    bprintf ("Echo what to level %d?\n", lev);
    return;
  }
  send_msg (DEST_ALL, MODE_NPFLAG | MP (PFL_NOECHO), lev, LVL_MAX, mynum,
	    NOBODY, "%s\n", x);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You lev-echoed (Lvl %d): %s\n", lev, x);
  else
    bprintf ("Ok\n");

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_ARCHWIZARD, LVL_MAX, mynum,
	    NOBODY, "&+B[&+CLevecho (%d) &*by &+W\001p%s\003&+B]\n", lev, pname (mynum));

}

/* (C) Rassilon (Brian Preble) */
void
echoallcom ()
{
  char x[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_ECHO)) {
    bprintf ("You hear echos.\n");
    return;
  }
  getreinput (x);

  if (EMPTY (x)) {
    bprintf ("Echo what?\n");
    return;
  }
  send_msg (DEST_ALL, MODE_NPFLAG | MP (PFL_NOECHO), LVL_MIN, LVL_MAX, mynum,
	    NOBODY, "%s\n", x);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You echoall: %s\n", x);
  else
    bprintf ("Ok\n");

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_ARCHWIZARD, LVL_MAX,
  mynum, NOBODY, "&+B[&+CEchoall &*by &+W\001p%s\003&+B]\n", pname (mynum));

}

/* (C) Rassilon (Brian Preble) */
void
echotocom ()
{
  int b;

  if (!ptstflg (mynum, PFL_ECHO)) {
    bprintf ("You hear echos.\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Echo to who?\n");
    return;
  }
  if ((b = pl1) == -1) {
    bprintf ("No one with that name is playing.\n");
    return;
  }
  if (b == mynum) {
    bprintf ("What's the point?\n");
    return;
  }
  if (EMPTY (txt2)) {
    bprintf ("What do you want to echo to them?\n");
    return;
  }
  if (EQ (pname (mynum), "Oracle"))
    if (strstr (txt2, "tells you") && ststflg (b, SFL_NOORACLE))
      return;

  if (check_busy (b))
    return;

  send_msg (b, MODE_NPFLAG | MP (PFL_NOECHO), LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	    "%s\n", txt2);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You echoto: %s\n", txt2);
  else
    bprintf ("Ok\n");

  if (!EQ (pname (mynum), "Inego"))
    send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_GOD, LVL_MAX,
	      mynum, NOBODY, "&+B[&+CEchoto to &+W\001p%s\003 &*by "
	      "&+W\001p%s\003&+B]\n", pname (b), pname (mynum));
}

/* (C) Jim Finnis  (Yes he really did write one or two routines/A) */
void
emotecom ()
{
  char buff[MAX_COM_LEN + 10];

  if (!ptstflg (mynum, PFL_EMOTE) && !ltstflg (ploc (mynum), LFL_PARTY)) {
    bprintf ("Your emotions are strictly limited!\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("What do you want to emote?\n");
    return;
  }
  getreinput (buff);
  send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "\001p%s\003 %s\n", pname (mynum), buff);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You emote: '%s %s'\n", pname (mynum), buff);
  else
    bprintf ("Ok\n");
}

/* (C) Rassilon (Brian Preble) */
void
emotetocom ()
{
  int a;

  if (!ptstflg (mynum, PFL_EMOTE)
      && !ltstflg (ploc (mynum), LFL_PARTY)) {
    bprintf ("Your emotions are strictly limited!\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Emote to who?\n");
    return;
  }
  if ((a = pl1) == -1) {
    bprintf ("No one with that name is playing.\n");
    return;
  }
  if (a == mynum) {
    bprintf ("Good trick, that.\n");
    return;
  }
  if (EMPTY (txt2)) {
    bprintf ("Emote what?\n");
    return;
  }
  if (check_busy (a))
    return;

  sillytp (a, txt2);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("Your emoteto looked like: %s %s\n", pname (mynum), txt2);
  else
    bprintf ("Ok\n");
}

/* The SET command
 */
/* These properties require numbers:
 */
#define SET_BVALUE  0
#define SET_SIZE    1
#define SET_VIS     2
#define SET_DAMAGE  3
#define SET_ARMOR   4
#define SET_STATE   5

/* These require texts:
 */
#define SET_TEXT_MIN 5
#define SET_DESC0  6
#define SET_DESC1  7
#define SET_DESC2  8
#define SET_DESC3  9
#define SET_NAME   10
#define SET_ANAME  11
#define SET_TEXT_MAX 11

/* Properties
 */
static char *Props[] =
{"BaseValue", "Size", "Visibility", "Damage",
 "AC", "State", "Desc0", "Desc1",
 "Desc2", "Desc3", "Name", "AltName",
 TABLE_END
};

void
setcom ()
{
  int o, p, v;			/* Object, Property, Value */

  Boolean is_oflag;

  if (!ptstflg (mynum, PFL_OBJECT)) {
    erreval ();
    return;
  }
  if (brkword () == -1 || (o = fobn (wordbuf)) == -1) {
    bprintf ("Set what??\n");
    return;
  }
  if (brkword () == -1 || ((is_oflag = (p = tlookup (wordbuf, Props)) == -1)
			   && (p = tlookup (wordbuf, Oflags)) == -1)) {

    bprintf ("Set what property on the %s?\n", oname (o));
    return;
  }
  if ((is_oflag || p <= SET_TEXT_MIN || p > SET_TEXT_MAX)
      && brkword () == -1) {

    bprintf ("Set the %s property to what ??\n",
	     is_oflag ? Oflags[p] : Props[p]);
    return;
  }
  if (is_oflag) {

    if ((v = tlookup (wordbuf, OO)) == -1
	&& (v = tlookup (wordbuf, TF)) == -1) {

      bprintf ("Value must be On or Off (or True/False).\n");
      return;
    }
    if (v) {
      osetbit (o, p);
    } else {
      oclrbit (o, p);
    }
  } else if (p <= SET_TEXT_MIN || p > SET_TEXT_MAX) {

    int limit = -1;

    if (!isdigit (*wordbuf)) {
      bprintf ("Value must be a number >= 0.\n");
      return;
    } else
      v = atoi (wordbuf);

    switch (p) {
    case SET_BVALUE:
      if (v > O_BVALUE_MAX ||
	  (v > obaseval (o) && v > 400
	   && !ptstflg (mynum, PFL_FROB)))
	limit = O_BVALUE_MAX;
      else
	osetbaseval (o, v);
      break;
    case SET_SIZE:
      if (v > O_SIZE_MAX)
	limit = O_SIZE_MAX;
      else
	osetsize (o, v);
      break;
    case SET_VIS:
      if (v > O_VIS_MAX)
	limit = O_VIS_MAX;
      else
	osetvis (o, v);
      break;
    case SET_DAMAGE:
      if (v > O_DAM_MAX)
	limit = O_DAM_MAX;
      else
	osetdamage (o, v);
      break;
    case SET_ARMOR:
      if (v > O_ARMOR_MAX)
	limit = O_ARMOR_MAX;
      else
	osetarmor (o, v);
      break;
    case SET_STATE:
      if (v > omaxstate (o))
	limit = omaxstate (o);
      else
	setobjstate (o, v);
      break;
    default:
      bprintf ("Internal error\n");
      mudlog ("ERROR: Internal errror in setcom(): p = %d", p);
      return;
    }

    if (limit > -1) {
      bprintf ("Sorry, value must be <= %d.\n", limit);
      return;
    }
  } else {
    char **q;

    if (opermanent (o)) {
      bprintf ("You can only change %s on non-permanent"
	       " (wiz-created) objects.\n", Props[p]);
      return;
    }
    q = p == SET_DESC0 ? &olongt (o, 0)
      : p == SET_DESC1 ? &olongt (o, 1)
      : p == SET_DESC2 ? &olongt (o, 2)
      : p == SET_DESC3 ? &olongt (o, 3)
      : p == SET_NAME ? &oname (o)
      : p == SET_ANAME ? &oaltname (o)
      : NULL;

    if (q == NULL) {
      bprintf ("Internal Error\n");
      mudlog ("ERROR: Internal error in setcom(), p = %d", p);
      return;
    }
    if (strchr (getreinput (wordbuf), '^') != NULL) {
      bprintf ("Illegal character(s) (^) in text.\n");
      return;
    }
    if (p == SET_NAME || p == SET_ANAME) {
      char *s = wordbuf;

      if (strlen (wordbuf) > ONAME_LEN) {
	bprintf ("Name too long. Max = %d chars.\n",
		 ONAME_LEN);
	return;
      }
      while (*s != '\0' && isalpha (*s))
	s++;

      if (*s != '\0') {
	bprintf ("Name must only contain latters.\n");
	return;
      }
      if (is_classname (wordbuf)) {
	bprintf ("That's the name of a object-class\n");
	return;
      }
      if (EMPTY (wordbuf)) {
	if (p == SET_ANAME)
	  strcpy (wordbuf, "<null>");
	else {
	  bprintf ("Name is missing.\n");
	  return;
	}
      }
    }
    if (*q != NULL)
      FREE (*q);

    *q = COPY (wordbuf);
  }
  bprintf ("Ok.\n");
}


/* The SAVESET command.
 */
void
saveset ()
{
  SETIN_REC s;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (cur_player->aliased || cur_player->polymorphed != -1) {
    bprintf ("Not while aliased.\n");
    return;
  }
  strcpy (s.name, pname (mynum));
  strcpy (s.prompt, cur_player->prompt);
  strcpy (s.setin, cur_player->setin);
  strcpy (s.setout, cur_player->setout);
  strcpy (s.setmin, cur_player->setmin);
  strcpy (s.setmout, cur_player->setmout);
  strcpy (s.setvin, cur_player->setvin);
  strcpy (s.setvout, cur_player->setvout);
  strcpy (s.setqin, cur_player->setqin);
  strcpy (s.setqout, cur_player->setqout);
  strcpy (s.setsit, cur_player->setsit);
  strcpy (s.setstand, cur_player->setstand);
  strcpy (s.setsum, cur_player->setsum);
  strcpy (s.setsumin, cur_player->setsumin);
  strcpy (s.setsumout, cur_player->setsumout);

  putsetins (pname (mynum), &s);
  bprintf ("Saving prompt and set*in/out messages.\n");
}



void
exorcom ()
{
  int x, q, y = real_mynum;

  if (!ptstflg (mynum, PFL_EXOR)) {
    bprintf ("You couldn't exorcise your way out of a paper bag.\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Exorcise who?\n");
    return;
  }
  if ((x = pl1) == -1) {
    bprintf ("They aren't playing.\n");
    return;
  }
  if (!do_okay (mynum, x, PFL_NOEXOR)) {
    bprintf ("They don't want to be exorcised.\n");
    return;
  }
  if (x < max_players)
    mudlog ("SYSTEM: %s exorcised %s", pname (mynum), pname (x));

  send_msg (DEST_ALL, MODE_QUIET, pvis (x), LVL_MAX, x, NOBODY,
	    "%s has been kicked off.\n", pname (x));

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX,
	mynum, x, "&+B[&+W\001p%s\003 &*has exorcised &+W\001p%s\003&+B]\n",
	    pname (mynum), pname (x));

  if ((q = find_pretender (x)) >= 0) {
    sendf (q, "You have been kicked off!\n");
    unalias (q);
    unpolymorph (q);
  }
  if (x >= max_players) {

    dumpstuff (x, ploc (x));
    setpfighting (x, -1);

    if (!ppermanent (x)) {
      destruct_mobile (x, NULL);
    } else {
      setpname (x, "");
    }

  } else {

    setup_globals (x);
    crapup ("\tYou have been kicked off!\n", CRAP_SAVE | CRAP_RETURN);
    setup_globals (y);
  }
}


void
setstart (void)
{
  PERSONA d;
  int loc, p;
  char buff[80];
  char *who;
  char *where;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  /* Parse the command, find *WHO*'s home we want to change and *WHERE*
   * * to change it to.
   */
  if (brkword () == -1) {

    who = pname (mynum);
    where = NULL;

  } else if (getuaf (strcpy (buff, wordbuf), &d)) {

    who = d.p_name;
    where = (brkword () == -1) ? NULL : wordbuf;

    if (!EQ (who, pname (mynum))) {
      if (!ptstflg (mynum, PFL_UAF)) {
	bprintf ("You can only set your own start-location.\n");
	return;
      }
      if (!do_okay_l (plev (mynum), d.p_level, False)) {
	bprintf ("That is beyond your powers.\n");
	return;
      }
    }
  } else if (brkword () == -1) {

    who = pname (mynum);
    where = buff;

  } else {

    bprintf ("No such player: %s\n", buff);
    return;
  }

  /* Got the arguments. If the operation is not 'erase home' (where=null),
   * * see if the argument corresponds to a real room.
   */
  if (where == NULL) {
    loc = 0;
  } else if ((loc = findroomnum (where)) == 0 || !exists (loc)) {
    bprintf ("No such location.\n");
    return;
  }
  if (exists (loc))
    loc = loc_id (loc);

  /*  Got the room number. Finally, set the home for the player.
   */
  if ((p = fpbns (who)) > -1) {
    setphome (p, loc);
  } else {
    d.p_home = loc;
    putuaf (&d);
  }
  bprintf ("Ok.\n");
}



void
noshoutcom ()
{
  int x;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if ((x = pl1) == -1 || x == mynum) {
    if (ststflg (mynum, SFL_NOSHOUT)) {
      bprintf ("You can hear shouts again.\n");
      sclrflg (mynum, SFL_NOSHOUT);
      send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX, mynum,
      NOBODY, "&+B[&+W%s &*is listening to &+CShouts&+B]\n", pname (mynum));
    } else {
      bprintf ("From now on you won't hear shouts.\n");
      ssetflg (mynum, SFL_NOSHOUT);
      send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX, mynum,
		NOBODY, "&+B[&+W%s &*is no longer listening to &+CShouts&+B]\n", pname (mynum));
    }
  } else if (plev (x) >= LVL_WIZARD) {
    if (ststflg (x, SFL_NOSHOUT)) {
      bprintf ("%s can hear shouts again.\n", pname (x));
      sclrflg (x, SFL_NOSHOUT);
    } else {
      bprintf ("From now on %s won't hear shouts.\n", pname (x));
      ssetflg (x, SFL_NOSHOUT);
    }
  } else if (ststflg (x, SFL_NOSHOUT)) {
    bprintf ("%s is allowed to shout again.\n", pname (x));
    sclrflg (x, SFL_NOSHOUT);
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum,
     NOBODY, "&+B[&+W\001p%s\003 &*has made &+W%s &*able to &+CShout&+B]\n",
	      pname (mynum), pname (x));
  } else {
    bprintf ("From now on %s cannot shout.\n", pname (x));
    ssetflg (x, SFL_NOSHOUT);
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum,
    NOBODY, "&+B[&+W\001p%s\003 &*has made &+W%s &*unable to &+CShout&+B]\n",
	      pname (mynum), pname (x));
  }
}




void
showlocation (int o)
{
  int uc, gotroom;

  uc = 1;
  gotroom = 0;
  while (!gotroom) {
    switch (ocarrf (o)) {
    case IN_ROOM:
      o = oloc (o);
      gotroom = 1;
      break;
    case IN_CONTAINER:
      bprintf (" %cnside the %s", (uc ? 'I' : 'i'), oname (oloc (o)));
      uc = 0;
      o = oloc (o);
      break;
    case CARRIED_BY:
      bprintf (" %carried by %s", (uc ? 'C' : 'c'), pname (oloc (o)));
      uc = 0;
      o = ploc (oloc (o));
      gotroom = 1;
      break;
    case WORN_BY:
      bprintf (" %corn by %s", (uc ? 'W' : 'w'), pname (oloc (o)));
      uc = 0;
      o = ploc (oloc (o));
      gotroom = 1;
      break;
    case WIELDED_BY:
      bprintf (" %cielded by %s", (uc ? 'W' : 'w'), pname (oloc (o)));
      uc = 0;
      o = ploc (oloc (o));
      gotroom = 1;
      break;
    case BOTH_BY:
      bprintf (" %corn and wielded by %s", (uc ? 'W' : 'w'), pname (oloc (o)));
      uc = 0;
      o = ploc (oloc (o));
      gotroom = 1;
      break;
    default:
      bprintf ("\n");
      return;
    }
  }
  bprintf (" %cn ", (uc ? 'I' : 'i'));
  bprintf ("%s\n", showname (o));
}


/* The SHOW command.
 */
void
showitem ()
{
  int x, i;
  OFLAGS *p;

  if (!ptstflg (mynum, PFL_STATS)) {
    erreval ();
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Show what?\n");
    return;
  }
  if ((x = fobn (item1)) == -1) {
    bprintf ("What's that?\n");
    return;
  }
  bprintf ("\nItem [%d]:  %s", x, oname (x));
  if (!EQ (oaltname (x), "<null>"))
    bprintf (" (%s)", oaltname (x));

  bprintf ("\nLocation:   ");
  showlocation (x);

  if (olinked (x) > -1) {
    bprintf ("Linked to:   ");
    if (olinked (x) >= numobs)
      bprintf ("non-existant object! (%d)", olinked (x));
    else {
      bprintf ("%s", oname (olinked (x)));
      if (olinked (olinked (x)) != x)
	bprintf (" ERROR: Not linked back! (%d)\n",
		 olinked (olinked (x)));
      else
	showlocation (olinked (x));
    }
  }
  bprintf ("Zone/Owner:  %s\n", zname (ozone (x)));

  bprintf ("\nState: %d\tMax State: %d\tVis Level: %d\n",
	   state (x), omaxstate (x), ovis (x));
  bprintf ("Damage: %d\tArmor Class: %d\t\tSize: %d\n",
	   odamage (x), oarmor (x), osize (x));
  bprintf ("Base Value: %-4d\t\tCurrent Value: %d\n\n",
	   obaseval (x), ovalue (x));
  bprintf ("Properties: ");
  p = &(obits (x));
  show_bits ((int *) p, sizeof (OFLAGS) / sizeof (int), Oflags);

  bprintf ("\nState   Description:\n");

  for (i = 0; i < 4; i++) {
    bprintf ("[%d]  %s\n", i, olongt (x, i) == NULL ? "" : olongt (x, i));
  }
}



void
wizlock ()
{
  extern char *WizLevels[];
  extern char *MLevels[];
  extern char *FLevels[];

  static int k[] =
  {0, LVL_WIZARD, LVL_DORQ, LVL_EMERITI, LVL_FULLWIZ,
   LVL_PROPHET, LVL_ARCHWIZARD, LVL_ADVISOR, LVL_AVATAR,
   LVL_GOD, LVL_MASTER};

  char s[80];
  char b[50];
  int l, v, n, x, y;

  l = the_world->w_lock;

  if (brkword () == -1) {
    bprintf ("The game is currently %slocked.\n", lev2s (b, l));
    return;
  }
  if (!ptstflg (mynum, PFL_LOCK) || (v = plev (mynum)) <= 0) {
    erreval ();
    return;
  }
  if (l > v || (v > LVL_WIZARD && l > k[wlevel (v)])) {
    bprintf ("Sorry, the game is already %slocked.\n", lev2s (b, l));
    return;
  }
  y = strlen (wordbuf);

  if (strncasecmp (wordbuf, "Off", y) == 0
      || strncasecmp (wordbuf, "Unlock", y) == 0) {
    n = 0;
  } else if ((x = tlookup (wordbuf, WizLevels)) > 0) {
    n = k[x];
  } else if ((x = tlookup (wordbuf, MLevels)) > 0) {
    n = x;
  } else if ((x = tlookup (wordbuf, FLevels)) > 0) {
    n = x;
  } else if (isdigit (*wordbuf)) {
    n = atoi (wordbuf);
  } else {
    bprintf ("Illegal argument to wizlock command.\n");
    return;
  }

  if (n > v || (v > LVL_WIZARD && n > k[wlevel (v)])) {
    bprintf ("You can't %slock the game!\n", lev2s (b, n));
    return;
  }
  the_world->w_lock = n;
  sprintf (s, "&+W[SYSTEM] &+C: &*The Game is now %slocked.&#&#\n", lev2s (b, n));
  broad (s);

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX,
	    mynum, NOBODY, "&+B[&+CWizlock &*by &+W\001p%s\003&+B]\n",
	    pname (mynum));
}

void
warcom ()
{
  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  the_world->w_peace = 0;
  broad ("The air of peace and friendship lifts.\n");
}

void
peacecom ()
{
  int m;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  the_world->w_peace = 1;
  for (m = 0; m < max_players; m++) {
    setpfighting (m, -1);
  }
  broad ("Peace and tranquility descend upon the world.\n");
}


void
tracecom (void)
{
  int i;

  if (!ptstflg (mynum, PFL_TRACE)) {
    erreval ();
    return;
  }
  if (cur_player->Trace.trace != -1) {
    bprintf ("Stopped tracing %s (%d).\n", cur_player->Trace.is_obj ?
	     oname (cur_player->Trace.trace) :
	     pname (cur_player->Trace.trace - GLOBAL_MAX_OBJS),
	     cur_player->Trace.trace);
    cur_player->Trace.trace = -2;
  }
  if (EMPTY (item1)) {
    if (cur_player->Trace.trace == -2)
      cur_player->Trace.trace = -1;
    else
      bprintf ("Trace what?\n");
    return;
  }
  if ((i = fpbn (item1)) != -1) {
    if (!do_okay (mynum, i, PFL_NOTRACE)) {
      bprintf ("They don't want to be traced.\n");
      return;
    }
    cur_player->Trace.is_obj = False;
    cur_player->Trace.loc = ploc (i);
    cur_player->Trace.trace = i;
    bprintf ("You are now tracing %s (%d).\n", pname (i),
	     cur_player->Trace.trace);
    return;
  }
  if ((i = fobn (item1)) != -1) {
    cur_player->Trace.is_obj = True;
    cur_player->Trace.loc = oloc (i);
    cur_player->Trace.oloc = getobjloc (i);
    cur_player->Trace.carry = ocarrf (i);
    cur_player->Trace.trace = i;
    bprintf ("You are now tracing %s (%d).\n", oname (i),
	     cur_player->Trace.trace);
    return;
  }
  bprintf ("What's that?\n");
}

void
zapcom (void)
{
  int vic, x;

  if (!ptstflg (mynum, PFL_ZAP)) {
    bprintf ("The spell fails.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Zap who?\n");
    return;
  }
  if ((vic = pl1) == -1) {
    bprintf ("There is no one on with that name.\n");
    return;
  }
  /* Message to everyone in the same room */
  send_msg (ploc (vic), 0, pvis (vic), LVL_MAX, vic, NOBODY,
	    "A massive lightning bolt strikes \001p%s\003!",
	    pname (vic));

  if (!do_okay (mynum, vic, PFL_NOZAP)) {
    sendf (vic, "%s casts a lightning bolt at you!\n",
	   see_player (vic, mynum) ? pname (mynum) : "Someone");
  } else {
    sendf (vic, "A massive lightning bolt arcs down out of "
	   "the sky to strike you between\nthe eyes!\n"
	   "You have been utterly destroyed by %s.\n",
	   see_player (vic, mynum) ? pname (mynum) : "Someone");

    if (vic < max_players) {
      mudlog ("ZAP: %s zapped %s", pname (mynum), pname (vic));

      if (plev (vic) >= LVL_WIZARD) {
	update_wizlist (pname (vic), LEV_MORTAL);
      }
      deluaf (pname (vic));

      send_msg (ploc (vic), 0, pvis (vic), LVL_MAX, vic, NOBODY,
		"\001s%s\003%s has just died.\n\003",
		pname (vic), pname (vic));

      /* Send info-msg. to wizards */
      send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, vic, NOBODY,
		"&+B[&+W\001p%s\003 &*has &+Czapped &+W\001p%s\003&+B]\n",
		pname (mynum), pname (vic));

      x = real_mynum;
      setup_globals (vic);
      crapup ("\t\tBye Bye.... Slain by a Thunderbolt\n", CRAP_RETURN);
      setup_globals (x);

    } else {
      wound_player (mynum, vic, pstr (vic) + 1, VERB_ZAP);
    }
  }
  broad ("\001dYou hear an ominous clap of thunder in the distance.\n\002");
}

void
pzapcom (void)
{
  int vic, x;

  if (!ptstflg (mynum, PFL_PZAP)) {
    bprintf ("Pardon?\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("P-Zap who?\n");
    return;
  }
  if ((vic = pl1) == -1) {
    bprintf ("There is no one on with that name.\n");
    return;
  }
  /* Message to everyone in the same room */
  send_msg (ploc (vic), 0, pvis (vic), LVL_MAX, vic, NOBODY,
	    "\001A\033[1m\003\001cA massive lightning "
	    "bolt strikes \003\001D%s\003\001c!\n\003\001A\033[0m\003",
	    pname (vic));

  if (!do_okay (mynum, vic, PFL_NOZAP)) {

    sendf (vic, "%s casts a lightning bolt at you!\n",
	   see_player (vic, mynum) ? pname (mynum) : "Someone");

  } else {

    sendf (vic, "\001A\033[1m\003A massive lightning bolt arcs down out of "
	   "the sky to strike you between\nthe eyes!\001A\033[0m\003\n"
	   "You have been utterly destroyed by %s.\n",
	   see_player (vic, mynum) ? pname (mynum) : "Someone");

    if (vic < max_players) {

      mudlog ("PZAP: %s P-zapped %s", pname (mynum), pname (vic));

      send_msg (ploc (vic), 0, pvis (vic), LVL_MAX, vic, NOBODY,
		"\001s%s\003%s has just died.\n\003",
		pname (vic), pname (vic));

      /* Send info-msg. to wizards */
      send_msg (DEST_ALL, MODE_NPFLAG | MP (PFL_PZAP), LVL_WIZARD, LVL_MAX, vic, NOBODY,
		"&+B[&+W\001p%s\003 &*has &+Czapped &+W\001p%s\003&+B]\n",
		pname (mynum), pname (vic));

      send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_PZAP), LVL_WIZARD, LVL_MAX, vic, NOBODY,
		"&+B[&+W\001p%s\003 &*has &+CP-zapped &+W\001p%s\003&+B]\n",
		pname (mynum), pname (vic));

      x = real_mynum;
      setup_globals (vic);
      crapup ("\t\tBye Bye.... Slain by a Thunderbolt\n", CRAP_RETURN);
      setup_globals (x);

    } else {
      wound_player (mynum, vic, pstr (vic) + 1, VERB_ZAP);
    }
  }
  broad ("\001dYou hear an ominous clap of thunder in the distance.\n\002");
}

/* The TTY command. Displays hostnames.
 */
void
ttycom (void)
{
  char *z, header[100], time_online[25];
  int pos, i, a[256], a_len = 0;

  if (!ptstflg (mynum, PFL_SHUSER)) {
    erreval ();
    return;
  }
  if (brkword () != -1) {
    if ((i = fpbn (wordbuf)) >= 0) {
      if (i >= max_players) {
	bprintf ("A mobile doesn't have a host.\n");
	return;
      }
      z = ctime (&plogged_on (i));
      z[19] = '\0';
      for (pos = 0; pos < 15; pos++)
	time_online[pos] = z[pos + 4];
      time_online[pos] = '\0';

      bprintf ("&+WPlayer   : &+C%s\n", pname (i));
      bprintf ("&+WHostname : &+C%s\n",
	       !ptstflg (mynum, PFL_SEEUSER) ? players[i].hostname : players[i].usrname);
      bprintf ("&+WOn Since : &+C%s\n", time_online);
      return;
    } else {
      bprintf ("Cannot find player.\n");
      return;
    }
  }
  for (i = 0; i < max_players; i++)
    if (is_in_game (i) && (pvis (i) <= plev (mynum) || i == mynum))
      a[a_len++] = i;

  qsort (a, a_len, sizeof (int), cmp_player);

  sprintf (header, "%-*s %-38s  %15.15s  %8.8s",
	   PNAME_LEN,
	   "Player",
	   "Hostname",
	   "On Since",
	   "On For");

  bprintf ("&+C%s\n", header);
  bprintf ("&+c-------------------------------------------------------------------------------\n");

  for (i = 0; i < a_len; ++i) {

    z = ctime (&plogged_on (a[i]));
    z[19] = '\0';
    for (pos = 0; pos < 15; pos++)
      time_online[pos] = z[pos + 4];
    time_online[pos] = '\0';

    bprintf ("&+w%-*s %-38.38s  %15.15s  %8.8s\n",
	     PNAME_LEN,
	     pname (a[i]),
	     !ptstflg (mynum, PFL_SEEUSER) ? players[a[i]].hostname : players[a[i]].usrname,
	     time_online,
	     sec_to_hhmmss (global_clock - plogged_on (a[i])));
  }
  bprintf ("&+c-------------------------------------------------------------------------------\n");
}


void
toutcom (void)
{
  int a;

  if (!ptstflg (mynum, PFL_TIMEOUT)) {
    bprintf ("Pardon?\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Who?\n");
    return;
  }
  if ((a = fpbn (wordbuf)) == -1) {
    bprintf ("Who?\n");
    return;
  }
  if (a >= max_players) {
    bprintf ("Timeout a mobile? Why? What's the point?\n");
    return;
  }
  if (ptstflg (a, PFL_NOTIMEOUT) && a != mynum) {
    bprintf ("Something prevents you from doing this to that person.\n");
    return;
  }
  mudlog ("TOUT: %s has tout'd %s", pname (mynum), pname (a));

  send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_TIMEOUT), LVL_WIZARD, LVL_MAX, a, mynum,
	    "&+B[&+W\001p%s\003 &+whas tout'd &+W\001p%s\003&+B]\n",
	    pname (mynum), pname (a));

  send_msg (DEST_ALL, MODE_NPFLAG | MP (PFL_TIMEOUT), LVL_MIN, LVL_MAX, a, mynum,
	    "&+B[&+W\001p%s\003 &+whas timed-out&+B]\n", pname (a));

  p_crapup (a, "\tYou have timed-out.\n", CRAP_SAVE | CRAP_RETURN);

}

void
nowishcom (void)
{
  int a;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (brkword () != -1) {
    if ((a = fpbn (wordbuf)) == -1) {
      bprintf ("Cannot find that player.\n");
      return;
    }
    if (plev (a) < LVL_WIZARD) {
      if (ststflg (a, SFL_NOWISH)) {
	bprintf ("%s can now wish for things.\n", pname (a));
	sendf (a, "You are hit by a bolt of light and feel you've regained your link to the Gods.\n");
	sclrflg (a, SFL_NOWISH);
	send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
		  "&+B[&+W\001p%s\003 &*has made %s able to &+CWish&+B]\n",
		  pname (mynum), pname (a));
      } else {
	bprintf ("%s can no longer wish for things.\n", pname (a));
	sendf (a, "You are hit by a bolt of light and feel you've lost your link to the Gods.\n");
	ssetflg (a, SFL_NOWISH);
	send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
		  "&+B[&+W\001p%s\003 &*has made %s unable to &+CWish&+B]\n",
		  pname (mynum), pname (a));
      }
      return;
    } else {
      bprintf ("You can't seem to be able to make a Wizard not listen to wishes.\n");
      return;
    }
  }
  if (ststflg (mynum, SFL_NOWISH)) {
    bprintf ("You are once again listening to wishes.\n");
    sclrflg (mynum, SFL_NOWISH);
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	      "&+B[&+W\001p%s\003 &*is now listening to &+CWishes&+B]\n", pname (mynum));
    return;
  } else {
    bprintf ("You will no longer hear wishes.\n");
    ssetflg (mynum, SFL_NOWISH);
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	      "&+B[&+W\001p%s\003 &*is no longer listening to &+CWishes&+B]\n", pname (mynum));
    return;
  }
}

void
noslaincom (void)
{
  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (ststflg (mynum, SFL_NOSLAIN)) {
    bprintf ("You will see slain messages again.\n");
    sclrflg (mynum, SFL_NOSLAIN);
    return;
  } else {
    bprintf ("You will no longer see slain messages.\n");
    ssetflg (mynum, SFL_NOSLAIN);
    return;
  }
}

void
puntcom (void)
{
  int b, i, puntroom = 0;
  int ok = 1;
  int me = real_mynum;

  char punt_to[50], buff[80];

  strcpy (punt_to, "");

  if (!ptstflg (mynum, PFL_PUNT)) {
    bprintf ("Yeah, you WISH you could punt!\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Who do you want to punt?\n");
    return;
  }
  if (!EMPTY (item2)) {
    if ((puntroom = findroomnum (item2)) == 0) {
      bprintf ("Location does not exist!\n");
      return;
    } else {
      ok = 0;
      sprintf (punt_to, " to %s (%s)",
	       sdesc (puntroom), xshowname (buff, puntroom));
    }
  }
  if ((b = pl1) == -1) {
    bprintf ("I can't find that person.\n");
    return;
  }
  if (b == mynum) {
    bprintf ("You really want to punt yourself around? Well, have a nice "
	     "flight!\n");
  }
  if (b != mynum) {
    if (ptstflg (b, PFL_NOPUNT) && plev (mynum) < LVL_GOD) {
      bprintf ("A magical force prevents you from punting that person.\n");
      return;
    }
  }
  if (ok == 0) {
    if (chkroom (puntroom, b)) {
      bprintf ("Something will not let you punt %s there.\n", b == mynum ?
	       "yourself" : pname (b));
      return;
    }
  }
  while (ok != 0) {
    puntroom = my_random () % num_const_locs;	/* Get a random room    */
    puntroom = puntroom - (puntroom * 2);	/* MUST be negative     */

    if (ltstflg (puntroom, LFL_DEATH) ||	/* Cannot be deathroom  */
	ltstflg (puntroom, LFL_ON_WATER) ||	/* Cannot be on water   */
	ltstflg (puntroom, LFL_ONE_PERSON) ||	/* Cannot be one-person */
	ltstflg (puntroom, LFL_PRIVATE) ||	/* Cannot be private    */
	r_isdark (puntroom, b) ||	/* Cannot be dark       */
	chkroom (puntroom, b))
      ok = 1;
    else {
      ok = 0;
    }
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && i != mynum && i != b && ploc (b) == ploc (i)) {

      if (b != mynum) {
	if (ploc (mynum) == ploc (b))
	  sendf (i, "%s grins evilly then picks up %s and boots %s across "
		 "the world..\n", see_name (i, mynum), see_name (i, b),
		 him_or_her (b));
	else
	  sendf (i, "%s appears, picks up %s and boots %s across the "
		 "world..\n", see_name (i, mynum), see_name (i, b),
		 him_or_her (b));
      } else {
	sendf (i, "\001p%s\003 boots %s across the world..\n", pname (mynum),
	       psex (mynum) ? "herself" : "himself");
      }
    }
    if (is_in_game (i) && i != mynum && i != b && ploc (b) != ploc (i))
      sendf (i, "%s flies overhead screaming for mercy..\n", see_name (i, b));
  }

  if (b != mynum) {
    bprintf ("You pick up %s and boot %s across the world..\n", pname (b),
	     him_or_her (b));
    sendf (b, "%s grins at you then picks you up and boots you across the "
	   "world..\n", see_name (b, mynum));
  } else {
    bprintf ("You pick yourself up and boot yourself around the world..\n");
  }

  if (b < max_players) {
    setup_globals (b);
    setploc (mynum, puntroom);
    setup_globals (me);
  } else
    setploc (b, puntroom);

  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && i != b && ploc (b) == ploc (i))
      sendf (i, "%s lands on the ground with a loud thud.\n", see_name (i, b));
  }

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX, b, mynum,
	    "&+B[&+W\001p%s\003 &+whas punted &+W\001p%s\003&+w%s&+B]\n",
	    pname (mynum), pname (b), punt_to);

}

void
puntallcom (void)
{
  int b, i, puntroom;
  int ok = 1;
  int me = real_mynum;

  b = i = puntroom = 0;

  if (!ptstflg (mynum, PFL_PUNTALL)) {
    bprintf ("Yeah, you WISH you could puntall!\n");
    return;
  }
  if (!EMPTY (item1)) {
    if ((puntroom = findroomnum (item1)) == 0) {
      bprintf ("Location does not exist!\n");
      return;
    } else {
      ok = 0;
    }
  }
  if (ok == 0) {
    for (i = 0; i < max_players; ++i) {
      if (is_in_game (i) && i != mynum &&
	  wlevel (plev (mynum)) < wlevel (plev (b))) {
	if (chkroom (puntroom, i)) {
	  bprintf ("Something will not let you puntall there.\n");
	  return;
	}
      }
    }
  }
  send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "\001p%s\003 gets ready to punt all of you around!\n",
	    pname (mynum));
  bprintf ("You punt everyone around the world!\n");

  if (ok == 0) {
    for (i = 0; i < max_players; ++i) {
      if (is_in_game (i) && i != mynum && plev (i) < LVL_MASTER) {
	sendf (i, "\001p%s\003 grins evilly and boots you across the world.\n",
	       pname (mynum));
	setup_globals (i);
	setploc (mynum, puntroom);
	setup_globals (me);
      }
    }
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && i != mynum && plev (i) < LVL_MASTER) {
      ok = 1;
      while (ok != 0) {
	puntroom = my_random () % num_const_locs;	/* Get a random room    */
	puntroom = puntroom - (puntroom * 2);	/* MUST be negative     */

	if (ltstflg (puntroom, LFL_DEATH) ||
	    ltstflg (puntroom, LFL_ON_WATER) ||
	    ltstflg (puntroom, LFL_ONE_PERSON) ||
	    ltstflg (puntroom, LFL_PRIVATE) ||
	    r_isdark (puntroom, i) ||
	    chkroom (puntroom, i))
	  ok = 1;
	else {
	  ok = 0;
	  sendf (i, "\001p%s\003 grins evilly and boots you across the "
		 "world.\n", pname (mynum));
	  setup_globals (i);
	  setploc (mynum, puntroom);
	  setup_globals (me);
	}
      }
    }
  }

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX, b, mynum,
	    "&+B[&+CPuntAll &*by &+W\001p%s\003&+B]\n", pname (mynum));

}

void
litcom ()
{
  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (!ststflg (mynum, SFL_LIT)) {
    ssetflg (mynum, SFL_LIT);
    bprintf ("You are now lit.\n");
  } else {
    sclrflg (mynum, SFL_LIT);
    bprintf ("You are no longer lit.\n");
  }
}

void
burncom (void)
{
  int vic, x = real_mynum;

  if (!ptstflg (mynum, PFL_BURN)) {
    bprintf ("The spell fails.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Burn who?\n");
    return;
  }
  if ((vic = pl1) == -1) {
    bprintf ("There is no one on with that name.\n");
    return;
  }
  if (!do_okay (mynum, vic, PFL_NOBURN)) {
    bprintf ("You can't send that person to hell!\n");
    return;
  }
  send_msg (DEST_ALL, 0, pvis (vic), LVL_MAX, vic, NOBODY,
	    "Screams are heard in the distance as %s is sent to hell..\n",
	    pname (vic));

  sendf (vic, "You are ripped up and thrown into hell by \001p%s\003..\n",
	 pname (mynum));

  if (vic < max_players)
    mudlog ("BURN: %s has burned %s", pname (mynum), pname (vic));

  send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, vic, NOBODY,
	    "&+W\001p%s\003 &*has &+Rpissed off &*the &+WGods&*, and has been sent to burn in &+rhell&*!\n",
	    pname (vic));

  send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, vic, NOBODY,
	    "&+B[&+W\001p%s\003 &+whas burned &+W\001p%s\003&+B]\n",
	    pname (mynum), pname (vic));


  if (vic < max_players) {
    setplev (vic, 1);
    setpscore (vic, 0);
    set_xpflags (1, &pflags (vic), &pmask (vic));
    setploc (vic, LOC_LIMBO_HELL);

    setup_globals (vic);
    trapch (ploc (mynum));
    setup_globals (x);

  }
}

void
follist (void)
{
  int i, j;
  int a[256], a_len = 0;
  int me = real_mynum;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  for (i = 0; i < max_players; ++i)
    if (is_in_game (i) && (pvis (i) <= plev (me) || i == me) && pfollow (i) != -1)
      a[a_len++] = i;

  if (a_len == 0) {
    bprintf ("&+WThere are currently no players using follow.\n");
    return;
  }
  qsort (a, a_len, sizeof (int), cmp_player);

  bprintf ("&+WName            Str/Max DP AC           Following        Str/Max DP AC\n");
  bprintf ("&+B-----------------------------------------------------------------------\n");
  for (i = 0; i < a_len; ++i) {
    j = pfollow (a[i]);
    bprintf ("&+w%-15s %3d/%-3d %2d %2d           %-15s %3d/%-3d %2d %2d\n",
	pname (a[i]), pstr (a[i]), maxstrength (a[i]), player_damage (a[i]),
	     player_armor (a[i]), pname (j), pstr (j), maxstrength (j),
	     player_damage (j), player_armor (j));
  }

  bprintf ("&+B-----------------------------------------------------------------------\n");
}

void
toggleseeext (void)
{

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (!ststflg (mynum, SFL_SEEEXT)) {
    ssetflg (mynum, SFL_SEEEXT);
    bprintf ("Extended Messages Enabled\n");
  } else {
    sclrflg (mynum, SFL_SEEEXT);
    bprintf ("Extended Messages Disabled\n");
  }

}

/* findloccom()
 * 1995 by Sithel
 *
 * Changed to findcom() by Illusion.
 * findcom() not only finds locations, it will now look for objects,
 * players, and mobiles.
 */
void
findcom (void)
{
  int i, j;
  int plr = 0, mob = 0, obj = 0, loc = 0;
  char pln[256];
  char sst[MAX_COM_LEN];
  char tmp[256];

  getreinput (sst);

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("Pardon?\n");
    return;
  }
  if (EMPTY (txt1)) {
    bprintf ("What text are you looking for?\n");
    return;
  }
  bprintf ("&+WType  %-40.40s   %-18.18s  Number\n", "Name", "Location");
  bprintf ("&+B-------------------------------------------------------------------------------\n");

  for (i = 0; i < max_players; i++) {
    strcpy (tmp, pname (i));
    if (strstr (uppercase (tmp), uppercase (sst)) && see_player (mynum, i)) {
      ++plr;
      bprintf ("&+W(Plr) &+w%-40.40s   %-18.18s  %d\n",
	       pname (i), xshowname (pln, ploc (i)), GLOBAL_MAX_OBJS + i);
    }
  }

  for (i = max_players; i < numchars; i++) {
    strcpy (tmp, pname (i));
    if (strstr (uppercase (tmp), uppercase (sst)) && see_player (mynum, i)) {
      ++mob;
      bprintf ("&+W(Mob) &+w%-40.40s   %-18.18s  %d\n",
	       pname (i), xshowname (pln, ploc (i)), GLOBAL_MAX_OBJS + i);
    }
  }

  for (i = 0; i < numobs; ++i) {
    strcpy (tmp, oname (i));
    if (strstr (uppercase (tmp), uppercase (sst)) && (ovis (i) <= plev (mynum))) {
      ++obj;
      bprintf ("&+W(Obj) &+w%-40.40s   %-18.18s  %d\n", oname (i), odescrm (i), i);
    }
  }

  for (i = 0; i < numzon; i++)
    for (j = zfirst_loc (i); j != SET_END; j = znext_loc (i)) {
      strcpy (tmp, sdesc (j));
      if (strstr (uppercase (tmp), uppercase (sst))) {
	++loc;
	bprintf ("&+W(Loc) &+w%-40.40s   %-18.18s  %d\n", sdesc (j), xshowname (pln, j), j);
      }
    }

  bprintf ("&+B-------------------------------------------------------------------------------\n");

  if (plr)
    bprintf ("&+wPlayers: &+W%d  ", plr);
  if (mob)
    bprintf ("&+wMobiles: &+W%d  ", mob);
  if (obj)
    bprintf ("&+wObjects: &+W%d  ", obj);
  if (loc)
    bprintf ("&+wLocations: &+W%d  ", loc);

  if (!plr && !mob && !obj && !loc)
    bprintf ("&+wNothing was found with that text.");

  bprintf ("\n");
}

void
nopuntcom (void)
{
  if (!ptstmsk (mynum, PFL_NOPUNT)) {
    erreval ();
    return;
  }
  if (ptstflg (mynum, PFL_NOPUNT)) {
    bprintf ("NoPunt Disabled\n");
    pclrflg (mynum, PFL_NOPUNT);
  } else {
    bprintf ("NoPunt Enabled\n");
    psetflg (mynum, PFL_NOPUNT);
  }
}

void
fakequitcom (void)
{
  static char *QuitTable[] =
  {
    "list", "quit", "cut", "net1", "net2", "net3", "net4",
    "net5", "net6", "net7", TABLE_END
  };

  static int vislev[] =
  {
    0, LVL_PROPHET, LVL_PROPHET, LVL_PROPHET,
    LVL_PROPHET, LVL_ARCHWIZARD, LVL_ADVISOR,
    LVL_AVATAR, LVL_GOD, LVL_MASTER, LVL_MAX
  };


  int x, maxlev = vislev[wlevel (plev (mynum))];
  int wlev = wlevel (plev (mynum));
  char b[100], xx[SETIN_MAX + 100];

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("Pardon?\n");
    return;
  }
  if (brkword () == -1) {
    x = 1;
  } else {
    if ((x = tlookup (wordbuf, QuitTable)) < 0) {
      bprintf ("Use which message? (LIST for listing)\n");
      return;
    }
    if (wlev == LEV_WIZARD && x > 1) {
      bprintf ("Use which message? (LIST for listing)\n");
      return;
    }
    if (wlev == LEV_PROPHET && x > 2) {
      bprintf ("Use which message? (LIST for listing)\n");
      return;
    }
    if (wlev == LEV_ARCHWIZARD && x > 4) {
      bprintf ("Use which message? (LIST for listing)\n");
      return;
    }
  }

  switch (x) {
  case 0:
    bprintf ("FakeQuit Listing\n");
    bprintf ("--------------------------------------\n");
    bprintf ("QUIT : Regular quit message\n");

    if (wlev < LEV_PROPHET)
      return;
    bprintf ("CUT  : Connection lost (cut) message\n");

    if (wlev < LEV_ARCHWIZARD)
      return;
    bprintf ("NET1 : Connection Reset by Peer (ECONNRESET)\n");
    bprintf ("NET2 : No Route to Host (EHOSTUNREACH)\n");
    bprintf ("NET3 : Connection Timed Out (ETIMEDOUT)\n");

    if (wlev < LEV_ADVISOR)
      return;
    bprintf ("NET4 : Network Unreachable (ENETUNREACH)\n");
    bprintf ("NET5 : Network Dropped Connection on Reset (ENETRESET)\n");
    bprintf ("NET6 : Network is Down (ENETDOWN)\n");
    bprintf ("NET7 : Empty Packets\n");
    return;

  case 1:
    send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "%s\n", build_setin (xx, cur_player->setqout, pname (mynum),
				   NULL, NULL));

    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
	      LVL_MAX, mynum, NOBODY, "&+B[&+CQuitting Game: &+W%s&+B]\n", pname (mynum));

    break;
  case 2:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY,
	      "&+W[&+CSocket: &+C%s &+whas lost (cut) connection&+W]\n",
	      pname (mynum));
    break;
  case 3:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
    "&+B(&+WConnection Reset by Peer (ECONNRESET)&+B)&+W]\n", pname (mynum));
    break;
  case 4:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
	 "&+B(&+WNo Route to Host (EHOSTUNREACH)&+B)&+W]\n", pname (mynum));
    break;
  case 5:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
	"&+B(&+WConnection Timed Out (ETIMEDOUT)&+B)&+W]\n", pname (mynum));
    break;
  case 6:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
       "&+B(&+WNetwork Unreachable (ENETUNREACH)&+B)&+W]\n", pname (mynum));
    break;
  case 7:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
	      "&+B(&+WNetwork Dropped Connection on Reset (ENETRESET)&+B)&+W]\n", pname (mynum));
    break;
  case 8:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
	      "&+B(&+WNetwork is Down (ENETDOWN)&+B)&+W]\n", pname (mynum));
    break;
  case 9:
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD), LVL_MAX,
	      mynum, NOBODY, "&+W[&+CSocket: &+C%s &+whas lost link "
	      "&+B(&+WEmpty Packets&+B)&+W]\n", pname (mynum));
    break;
  default:
    bprintf ("What exactly are you trying to do?\n");
    return;
  }

  send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
	    LVL_MAX, mynum, NOBODY,
	 "&+B[&+W%s &*has departed from the MUD in &+C%s &*(&+C%s&*)&+B]\n",
	  pname (mynum), sdesc (ploc (mynum)), xshowname (b, ploc (mynum)));

  setpvis (mynum, maxlev);

  bprintf ("You have faked quitting the game.\n");

  send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
  LVL_MAX, mynum, NOBODY, "&+B[&+CFakequit &*by &+W%s&+B]\n", pname (mynum));
}

void
siccom (void)
{
  int mob, plr;

  if (!ptstflg (mynum, PFL_SIC)) {
    erreval ();
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Sic what mobile on what player?\n");
    return;
  }
  if ((mob = pl1) == -1) {
    bprintf ("That mobile can't be found.\n");
    return;
  }
  if (mob < max_players) {
    bprintf ("You can't sic a player on a player!\n");
    return;
  }
  if (EMPTY (item2)) {
    bprintf ("Sic %s on what player?\n", pname (mob));
    return;
  }
  if ((plr = pl2) == -1) {
    bprintf ("That player can't be found.\n");
    return;
  }
  if (plr >= max_players) {
    bprintf ("You can't sic a mobile on a mobile!\n");
    return;
  }
  if (!do_okay (mynum, plr, PFL_NOSIC)) {
    bprintf ("You can't sic a mobile on that person.\n");
    return;
  }
  if (ltstflg (ploc (plr), LFL_PEACEFUL)) {
    bprintf ("It's too peaceful where %s is to use sic.\n", pname (plr));
    return;
  }
  bprintf ("You sic %s on %s.\n", pname (mob), pname (plr));

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	    "&+B[&+CSic &*by &+W\001p%s\003&*: &+W\001p%s\003 &*on &+W\001p%s\003&+B]\n",
	    pname (mynum), pname (mob), pname (plr));

  setploc (mob, ploc (plr));
  sendf (plr, "\001p%s\003 appears and begins to attack you!\n", pname (mob));
  setpfighting (mob, plr);
  hit_player (mob, plr, -1);
}

void
atviscom (void)
{
  static int vislev[] =
  {
    0, LVL_PROPHET, LVL_PROPHET, LVL_PROPHET,
    LVL_PROPHET, LVL_ARCHWIZARD, LVL_ADVISOR,
    LVL_AVATAR, LVL_GOD, LVL_MASTER, LVL_MAX
  };

  int vis, oldvis = pvis (mynum);
  int maxvis = vislev[wlevel (plev (mynum))];
  char com[MAX_COM_LEN];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("What visibility level do you want to use?\n");
    return;
  }
  if ((vis = atoi (wordbuf)) == 0) {
    bprintf ("Invalid Visibility Value!\n");
    return;
  }
  getreinput (com);

  if (vis > maxvis) {
    bprintf ("You can't go invisible to that level! Setting you to %d.\n", maxvis);
    vis = maxvis;
  }
  setpvis (mynum, vis);
  gamecom (com, False);
  setpvis (mynum, oldvis);
}

void
freaqcom (void)
{
  int b;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Who is a Frea&+GQ&*?\n");
    return;
  }
  if ((b = pl1) == -1) {
    bprintf ("Who is that?\n");
    return;
  }
  if (b >= max_players) {
    bprintf ("You can't make a mobile a Frea&+GQ&*!\n");
    return;
  }
  if (wlevel (plev (mynum)) < wlevel (plev (b))) {
    bprintf ("You can't make that player a Frea&+GQ&*!\n");
    return;
  }
  if (b == mynum) {
    if (ststflg (mynum, SFL_FREAQ)) {
      bprintf ("You don't feel like a Frea&+GQ&* anymore.\n");
      sclrflg (mynum, SFL_FREAQ);
      psetflg (mynum, PFL_TITLES);
    } else {
      bprintf ("You start to Frea&+GQ&*out!\n");
      ssetflg (mynum, SFL_FREAQ);
      pclrflg (mynum, PFL_TITLES);
    }
    return;
  }
  if (ststflg (b, SFL_FREAQ)) {
    bprintf ("You return %s to normal.\n", pname (b));
    sendf (b, "You don't feel like a Frea&+GQ&* anymore.\n");
    send_msg (DEST_ALL, MODE_QUIET, max (LVL_WIZARD, pvis (b)), LVL_MAX,
	      mynum, b, "&+B[&+CFreaQ: &+W\001p%s\003 &*has made &+W%s "
	      "&*normal&+B]\n", pname (mynum), pname (b));
    mudlog ("FREAQ: %s made %s normal", pname (mynum), pname (b));
    psetflg (b, PFL_TITLES);
    sclrflg (b, SFL_FREAQ);
  } else {
    bprintf ("You turn %s into a FreaQ.\n", pname (b));
    sendf (b, "You begin to feel like a Frea&+GQ&*.\n");
    send_msg (DEST_ALL, MODE_QUIET, max (LVL_WIZARD, pvis (b)), LVL_MAX,
	      mynum, b, "&+B[&+CFreaQ: &+W\001p%s\003 &*has made &+W%s "
	      "&*a FreaQ&+B]\n", pname (mynum), pname (b));
    mudlog ("FREAQ: %s made %s a FreaQ", pname (mynum), pname (b));
    pclrflg (b, PFL_TITLES);
    ssetflg (b, SFL_FREAQ);
  }
}
