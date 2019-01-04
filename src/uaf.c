


#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include "pflags.h"
#include "kernel.h"
#include "sflags.h"
#include "mflags.h"
#include "nflags.h"
#include "eflags.h"
#include "uaf.h"
#include "mobile.h"
#include "flags.h"
#include "exit.h"
#include "sendsys.h"
#include "mud.h"
#include "bprintf.h"
#include "wizlist.h"
#include "bootstrap.h"
#include "log.h"
#include "parse.h"

extern int errno;
extern char *WizLevels[];
time_t time (time_t * v);

void
pers2player (PERSONA * d, int plx)
{
  setpstr (plx, d->p_strength);
  setpdam (plx, d->p_damage);
  setphome (plx, d->p_home);
  setpscore (plx, d->p_score);
  setparmor (plx, d->p_armor);
  setsflags (plx, d->p_sflags);
  setpflags (plx, d->p_pflags);
  setpmask (plx, d->p_mask);
  setqflags (plx, d->p_quests);
  setpvis (plx, ptstflg (plx, PFL_STARTINVIS) ? d->p_vlevel : 0);
  setplev (plx, d->p_level);
  setpwimpy (plx, d->p_wimpy);
  mob_id (plx) = d->p_id;
  if (plx < max_players) {
    setptitle (plx, d->p_title);
    (void) strcpy (players[plx].passwd, d->p_passwd);

    setpmagic (plx, d->p_magic);
    setpchannel (plx, d->p_channel);
    setpkilled (plx, d->p_killed);
    setpdied (plx, d->p_died);
    setppager (plx, d->p_pager);
    setnflags (plx, d->p_nflags);
    seteflags (plx, d->p_eflags);
    setplang (plx, d->p_lang);
  }
}

void
player2pers (PERSONA * d, time_t * last_on, int plx)
{
  d->p_strength = pstr (plx);
  d->p_damage = pdam (plx);
  d->p_home = phome (plx);
  d->p_score = pscore (plx);
  d->p_armor = parmor (plx);
  d->p_sflags = sflags (plx);
  d->p_pflags = pflags (plx);
  d->p_mask = pmask (plx);
  d->p_quests = qflags (plx);
  d->p_vlevel = pvis (plx);
  d->p_level = plev (plx);
  d->p_damage = pdam (plx);
  d->p_armor = parmor (plx);
  d->p_wimpy = pwimpy (plx);
  d->p_id = mob_id (plx);
  if (plx < max_players) {
    (void) strcpy (d->p_title, ptitle (plx));
    (void) strcpy (d->p_passwd, players[plx].passwd);
    d->p_magic = pmagic (plx);
    d->p_channel = pchannel (plx);
    d->p_killed = pkilled (plx);
    d->p_died = pdied (plx);
    d->p_pager = ppager (plx);
    d->p_nflags = nflags (plx);
    d->p_eflags = eflags (plx);
    d->p_lang = plang (plx);

    (void) strcpy (d->p_last_host, players[plx].hostname);
    (void) strcpy (d->p_usrname, players[plx].usrname);
  }
  (void) strcpy (d->p_name, pname (plx));
  if (last_on != NULL)
    d->p_last_on = *last_on;
}

void
get_gender (char *gen)
{
  Boolean ok = False;
  Boolean female = False;
  PERSONA d;

  if (gen == NULL) {
    replace_input_handler (get_gender);
  } else if (*gen == 'M' || *gen == 'm') {
    sclrflg (mynum, SFL_FEMALE);
    ok = True;
  } else if (*gen == 'F' || *gen == 'f') {
    ssetflg (mynum, SFL_FEMALE);
    ok = female = True;
  } else {
    bprintf ("M or F");
  }
  if (ok) {
    /* initialize a very new user */
    if (OPERATOR (pname (mynum))) {
      /* We make him a god */
      sprintf (ptitle (mynum), "%%s the %s", WizLevels[LEV_MASTER]);
      setplev (mynum, LVL_MASTER);
      setpstr (mynum, pmaxstrength (LVL_MASTER));
      //setpscore (mynum, levels[LVL_MASTER]);
      update_wizlist (pname (mynum), LEV_MASTER);
    } else {
      sprintf (ptitle (mynum), "%%s the %s",
	       (female ? FLevels : MLevels)[LVL_NOVICE]);
      setplev (mynum, LVL_NOVICE);
      setpstr (mynum, 40);
      setpscore (mynum, 0);
    }
    setpwimpy (mynum, 0);
    mob_id (mynum) = id_counter++;
    setphome (mynum, 0);
    setpdam (mynum, 8);
    setparmor (mynum, 0);
    setpvis (mynum, 0);
    setsflgh (mynum, 0);
    setsflgl (mynum, 0);
    if (female)
      ssetflg (mynum, SFL_FEMALE);
    setqflags (mynum, 0);
    setpflgh (mynum, 0);
    setpflgl (mynum, 0);
    setpmskh (mynum, 0);
    setpmskl (mynum, 0);
    set_xpflags (plev (mynum), &(pflags (mynum)), &(pmask (mynum)));

    setpmagic (mynum, 0);
    setpchannel (mynum, 0);
    setpkilled (mynum, 0);
    setpdied (mynum, 0);
    setppager (mynum, 24);
    setnflags (mynum, 0);
    nsetflg (mynum, NFL_ENGLISH);
    setplang (mynum, NFL_ENGLISH);

    seteflags (mynum, 0);
    esetflg (mynum, EFL_FIREBALL);
    esetflg (mynum, EFL_MISSILE);
    esetflg (mynum, EFL_FROST);
    esetflg (mynum, EFL_SHOCK);

    strcpy (d.p_last_host, "Unknown, First time logged in.");

    if (female)
      xsetbit (d.p_sflags.l, SFL_FEMALE);

    player2pers (&d, &global_clock, mynum);
    putuaf (&d);
    save_id_counter ();
    do_motd (NULL);
  } else {
    bprintf ("\n");
    bprintf (strcpy (cur_player->cprompt, "Sex (M/F) : >"));
  }
}

void
saveother (void)
{
  int p;
  PERSONA d;

  if (EMPTY (item1))
    p = mynum;
  else {
    if ((p = pl1) == -1) {
      bprintf ("Cannot find player.\n");
      return;
    }
    if (plev (mynum) < LVL_WIZARD && p != mynum) {
      bprintf ("You cannot save another player!\n");
      return;
    }
  }

  if (players[p].aliased || players[p].polymorphed >= 0) {
    bprintf ("Not while aliased.\n");
    return;
  }
  player2pers (&d, &global_clock, mynum);

  if (p != mynum)
    sendf (mynum, "&+W[Saving %s]\n", pname (p));
  else
    bprintf ("\nSaving %s\n", pname (mynum));

  bflush ();
  putuaf (&d);
}

void
saveme (void)
{
  PERSONA d;

  if (cur_player->aliased || cur_player->polymorphed >= 0) {
    bprintf ("Not while aliased.\n");
    return;
  }
  player2pers (&d, &global_clock, mynum);
  bprintf ("\nSaving %s\n", pname (mynum));
  bflush ();
  putuaf (&d);
}

/* Saves all players */
void
saveallcom (void)
{
  PERSONA d;
  int i;
  int j = 0;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i)) {
      if (players[i].aliased || players[i].polymorphed >= 0) ;
      else {
	if (j == 0) {
	  bprintf ("Saving all players.\n");
	  j = 1;
	}
	player2pers (&d, &global_clock, i);
	bflush ();
	putuaf (&d);
      }
    }
  }

  if (j == 0)
    bprintf ("No players needed healing.\n");
  else {
    bprintf ("Done.\n");
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	      "&+B[&+CSaveall &*by &+W\001p%s\003&+B]\n", pname (mynum));
  }
}

/*
 * This procedure tries to locate an uaf record in the uaf file.
 * If we find the record, we return and the file position is just
 * past the record we found.
 */
Boolean
finduaf (char *name, PERSONA * d, int fd)
{
  unsigned long int x = 0;

  while (read (fd, (char *) d, sizeof (PERSONA)) == sizeof (PERSONA)) {
    if (x == 0 && EMPTY (d->p_name))
      x = lseek (fd, 0L, SEEK_CUR);
    if (EQ (d->p_name, name)) {
      lseek (fd, (long) -sizeof (PERSONA), SEEK_CUR);
      return True;
    }
  }
  if (x != 0) {
    lseek (fd, (long) (x - sizeof (PERSONA)), SEEK_SET);
  }
  return False;
}

/*
 * This procedure gets one uaf record
 * from the system
 * return 'True' if a record with the specified name is found and
 * d is filled with the specified info.
 * return 'False' if record could not be found and d do not contain.
 * any meaningful data.
 *
 */
Boolean
getuaf (char *name, PERSONA * d)
{
  int fd;
  Boolean b = False;

  if (name == NULL)
    return False;

  if ((fd = open (UAF_RAND, O_RDONLY, 0)) >= 0) {
    b = finduaf (name, d, fd);
    close (fd);
  } else if (errno != ENOENT) {	/* UAF_RAND file doesn't exist */
    mudlog ("ERROR: Error in getuaf for open");
    progerror ("getuaf/open");
  }
  return b;
}

void
putuaf (PERSONA * d)
{
  int fd;
  PERSONA x;

  /* Find if he is there already */

  if ((fd = open (UAF_RAND, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
    mudlog ("ERROR: putuaf: Error in open");
    progerror ("putuaf/open");
    __exit (1);
  }
  lseek (fd, 0L, SEEK_SET);
  (void) finduaf (d->p_name, &x, fd);
  if (write (fd, (char *) d, sizeof (PERSONA)) != sizeof (PERSONA)) {
    mudlog ("ERROR: Error in putuaf for write, UAF_RAND = " UAF_RAND);
    progerror ("putuaf/write");
    __exit (1);
  }
  lseek (fd, 0L, SEEK_END);
  close (fd);
}

void
deluaf (char *name)
{
  int fd;
  PERSONA p;

  if ((fd = open (UAF_RAND, O_RDWR, 0)) < 0) {
    if (errno == ENOENT)
      return;
    mudlog ("ERROR: Error for fopen(\"" UAF_RAND "\",\"r+\")");
    progerror ("deluaf/open");
    __exit (1);
  }
  if (finduaf (name, &p, fd)) {
    p.p_name[0] = '\0';
    if (write (fd, (char *) &p, sizeof (PERSONA)) != sizeof (PERSONA)) {
      progerror ("deluaf/write");
      __exit (1);
    }
  }
  lseek (fd, 0L, SEEK_END);
  close (fd);
}


Boolean
getuafinfo (char *name)
{
  PERSONA d;
  Boolean b;

  b = getuaf (name, &d);
  if (b) {
    pers2player (&d, mynum);
    setpname (mynum, d.p_name);
  }
  return b;
}

Boolean
findsetins (char *name, SETIN_REC * s, int fd)
{
  while (read (fd, s, sizeof (SETIN_REC)) == sizeof (SETIN_REC))
    if (EQ (s->name, name)) {
      return True;
    }
  return False;
}

Boolean
getsetins (char *name, SETIN_REC * s)
{
  int fd;
  Boolean b;

  if ((fd = open (SETIN_FILE, O_RDONLY, 0)) < 0)
    return False;

  b = findsetins (name, s, fd);
  close (fd);
  return b;
}

void
putsetins (char *name, SETIN_REC * s)
{
  SETIN_REC v;
  int fd;
  Boolean b;

  if ((fd = open (SETIN_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
    mudlog ("ERROR: putsetins: Error in open for " SETIN_FILE);
    progerror ("open");
    __exit (1);
  }
  if ((b = findsetins (name, &v, fd))) {
    lseek (fd, (long) -sizeof (SETIN_REC), SEEK_CUR);
  } else {
    lseek (fd, 0L, SEEK_END);
  }
  if (write (fd, s, sizeof (SETIN_REC)) != sizeof (SETIN_REC)) {
    mudlog ("ERROR: putsetins: Error in write for " SETIN_FILE);
    progerror ("write");
  }
  lseek (fd, 0L, SEEK_END);
  close (fd);
}

void
fetchprmpt (int plr)
{
  SETIN_REC s;

  if (plr >= max_players || plr < 0)
    return;
  if (plev (plr) >= LVL_WIZARD && getsetins (pname (plr), &s)) {
    strcpy (players[plr].prompt, s.prompt);
    strcpy (players[plr].setin, s.setin);
    strcpy (players[plr].setout, s.setout);
    strcpy (players[plr].setmin, s.setmin);
    strcpy (players[plr].setmout, s.setmout);
    strcpy (players[plr].setvin, s.setvin);
    strcpy (players[plr].setvout, s.setvout);
    strcpy (players[plr].setqin, s.setqin);
    strcpy (players[plr].setqout, s.setqout);
    strcpy (players[plr].setsit, s.setsit);
    strcpy (players[plr].setstand, s.setstand);
    strcpy (players[plr].setsum, s.setsum);
    strcpy (players[plr].setsumin, s.setsumin);
    strcpy (players[plr].setsumout, s.setsumout);
  } else {
    sprintf (players[plr].prompt, "%s", DFL_PROMPT);
    strcpy (players[plr].setin, "%n has arrived.");
    strcpy (players[plr].setout, "%n has gone %d.");
    strcpy (players[plr].setmin, "%n appears with an ear-splitting bang.");
    strcpy (players[plr].setmout, "%n vanishes in a puff of smoke.");
    strcpy (players[plr].setvin, "%n suddenly appears!");
    strcpy (players[plr].setvout, "%n has vanished!");
    strcpy (players[plr].setqin, "%n has entered the game.");
    strcpy (players[plr].setqout, "%n has left the game.");
    strcpy (players[plr].setsit, "%n is sitting here.");
    strcpy (players[plr].setstand, "%n is standing here.");
    strcpy (players[plr].setsum, "You are summoned by %n.");
    strcpy (players[plr].setsumin, "%v appears with an ear-splitting bang.");
    strcpy (players[plr].setsumout, "%v vanishes in a puff of smoke.");
  }
}

char *
build_setin (char *b, char *s, char *n, char *d, char *v)
{
  char *p, *q, *r;

  for (p = b, q = s; *q != 0;) {
    if (*q != '%')
      *p++ = *q++;
    else {
      switch (*++q) {
      case 'n':
	for (r = n; *r != 0;)
	  *p++ = *r++;
	break;
      case 'v':
	for (r = v; *r != 0;)
	  *p++ = *r++;
	break;
      case 'd':
	if (d == NULL)
	  return NULL;
	for (r = d; *r != 0;)
	  *p++ = *r++;
	break;
      case 'N':
	for (r = xname (n); *r != 0;)
	  *p++ = *r++;
	break;
      case 'f':
	for (r = (psex (mynum) ? "her" : "his"); *r != 0;)
	  *p++ = *r++;
	break;
      case 'F':
	for (r = (psex (mynum) ? "her" : "him"); *r != 0;)
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

/* Returns a players name, checking for invisibility. */
char *
ipname (int plr)
{
  static char name[30];

  sprintf (name, "\001p%s\003", pname (plr));
  return name;
}

int
make_kd_ratio (int kill, int death)
{
  int proc;

  if (death > kill) {
    return 0;
  } else {
    if (!(kill))
      proc = 0;
    else
      proc = (death * 100) / kill;
  }
  return 100 - proc;
}
