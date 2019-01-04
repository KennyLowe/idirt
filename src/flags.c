
#include "kernel.h"
#include "pflags.h"
#include "lflags.h"
#include "mflagnames.h"
#include "pflagnames.h"
#include "sflagnames.h"
#include "lflagnames.h"
#include "nflagnames.h"
#include "eflagnames.h"
#include "levelnames.h"
#include "questnames.h"
#include "sflags.h"
#include "nflags.h"
#include "eflags.h"
#include "sendsys.h"
#include "bprintf.h"
#include "parse.h"
#include "rooms.h"
#include "mobile.h"
#include "uaf.h"
#include "log.h"

char *TF[3] =
{"False", "True", TABLE_END};
char *OO[3] =
{"Off", "On", TABLE_END};

/* Prototype */
void dump_lflags (int loc);

void
show_bits (int *bits, int n, char **table)
{
  int *b;
  int bno;
  int xno;
  int cno;
  int c2;
  char **t;
  char *p;
  Boolean t_end;
  char buff[16];


  cno = n;
  b = bits + n - 1;
  bno = 0;
  xno = 0;
  c2 = 0;
  t = table;
  t_end = False;
  for (; b >= bits; --b, ++cno) {
    for (bno = 0; bno < 32; bno++, xno++, t++) {
      if (((*b) & (1 << bno)) != 0) {
	if (t_end)
	  p = NULL;
	else if ((p = *t) == TABLE_END) {
	  t_end = True;
	  p = NULL;
	}
	if (p == NULL) {
	  sprintf (buff, "<Spare%d>", xno);
	  p = buff;
	}
	if (++c2 == 6) {
	  bprintf ("\n");
	  c2 = 1;
	}
	bprintf ("%-14s\t", p);
      }
    }
  }
  bprintf ("\n");
}

Boolean
dump_pflags (void)
{
  static char *t[] =
  {"Pflags", "Mask", TABLE_END};
  int k, k2;

  if (plev (mynum) < LVL_ARCHWIZARD)
    return False;
  if (brkword () == -1)
    return False;
  if ((k = tlookup (wordbuf, t)) < 0)
    return False;
  if (brkword () == -1)
    return False;
  if ((k2 = tlookup (wordbuf, WizLevels)) < LEV_APPRENTICE)
    return False;
  bprintf ("%s for %s:\n", t[k], WizLevels[k2]);
  if (k == 0) {
    show_bits ((int *) &the_world->w_pflags[k2 - 1],
	       sizeof (PFLAGS) / sizeof (int), Pflags);
  } else {
    show_bits ((int *) &the_world->w_mask[k2 - 1],
	       sizeof (PFLAGS) / sizeof (int), Pflags);
  }
  return True;
}

void
xshow_lflags (int loc)
{
  char k[80];
  LFLAGS *p;

  p = &(xlflags (loc));
  bprintf ("Room: %s\nLflags:\n", buildname (k, loc));
  bprintf ("Light: %s           Temp: %s\n",
	   Lflags[check_light (loc)], Lflags[check_temp (loc)]);
  show_bits ((int *) p, sizeof (LFLAGS) / sizeof (int), Lflags);

  /*dump_lflags(loc); */
}

void
dump_lflags (int loc)
{
  LFLAGS *p;

  p = &(xlflags (loc));

  show_bits ((int *) p, sizeof (LFLAGS) / sizeof (int), Lflags);
}

void
pflagscom (void)
{
  int a, b, c, l;
  char *n;
  Boolean f;
  PFLAGS *p;
  PERSONA d;

  f = False;
  if (brkword () == -1) {
    a = mynum;
    p = &(pflags (a));
    n = pname (a);
    l = plev (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    if (a != mynum && !ptstflg (mynum, PFL_PFLAGS) && a < max_players) {
      bprintf ("You can only change/see your own P-flags.\n");
      return;
    }
    p = &(pflags (a));
    n = pname (a);
    l = plev (a);
  } else if (!ptstflg (mynum, PFL_UAF)) {
    bprintf ("Who's that?\n");
    return;
  } else if (!ptstflg (mynum, PFL_PFLAGS)) {
    bprintf ("You can only change/see your own P-flags.\n");
    return;
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else {
    f = True;
    p = &(d.p_pflags);
    n = d.p_name;
    l = d.p_level;
  }

  if ((a != mynum && wlevel (plev (mynum)) <= wlevel (l) && plev (mynum) < LVL_GOD)
      || (a >= max_players && plev (mynum) < LVL_WIZARD)) {
    bprintf ("That is beyond your powers.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Player: %s\n", n);
    bprintf ("Privileges:\n");
    show_bits ((int *) p, sizeof (PFLAGS) / sizeof (int), Pflags);

    return;
  } else if ((b = tlookup (wordbuf, Pflags)) == -1
	     || (!ptstmsk (mynum, b) && !OPERATOR (pname (mynum)))) {
    bprintf ("%s: No such P-Flag.\n", wordbuf);
    return;
  } else if (brkword () == -1) {
    c = dtst_bit (p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Pflags[b], TF[c]);
    return;
  } else if (a != mynum && !ptstflg (mynum, PFL_PFLAGS)) {
    bprintf ("You can only change your own P-flags.\n");
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  if (!EQ (n, pname (mynum))) {
    mudlog ("PFLAG: %s by %s, %s := %s", n, pname (mynum), Pflags[b], TF[c]);
  }
  if (c == 0) {
    dclr_bit (p, b);
  } else {
    dset_bit (p, b);
  }
  if (f) {
    putuaf (&d);
  }
}

void
maskcom (void)
{
  int a, b, c;
  char *n;
  Boolean f;
  PFLAGS *p;
  PERSONA d;

  f = False;
  if (brkword () == -1 || !ptstflg (mynum, PFL_MASK)) {
    a = mynum;
    p = &(pmask (a));
    n = pname (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    p = &(pmask (a));
    n = pname (a);
  } else if (!ptstflg (mynum, PFL_UAF)) {
    bprintf ("Who's that?\n");
    return;
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else {
    f = True;
    p = &(d.p_mask);
    n = d.p_name;
  }

  if (brkword () == -1) {
    bprintf ("Player: %s\nMask:\n", n);
    show_bits ((int *) p, sizeof (PFLAGS) / sizeof (int), Pflags);

    return;
  } else if ((b = tlookup (wordbuf, Pflags)) == -1
	     || (!ptstmsk (mynum, b) && !ptstflg (mynum, PFL_MASK))) {
    bprintf ("%s: No such Mask-bit.\n", wordbuf);
    return;
  } else if (!ptstflg (mynum, PFL_MASK) || brkword () == -1) {
    c = dtst_bit (p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Pflags[b], TF[c]);
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  mudlog ("MASK: %s by %s, %s := %s", n, pname (mynum), Pflags[b], TF[c]);

  if (c == 0) {
    dclr_bit (p, b);
  } else {
    dset_bit (p, b);
  }
  if (f) {
    putuaf (&d);
  }
}

void
mflagscom (void)
{
  int a, b, c;

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("Pardon?\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Whose MFlags?\n");
    return;
  }
  if ((a = fpbn (wordbuf)) < max_players) {
    bprintf ("No mobile with this name!\n");
    return;
  }
  if (brkword () == -1 || (b = tlookup (wordbuf, Mflags)) == -1) {
    bprintf ("Mobile: %s\n", pname (a));
    bprintf ("Mflags:\n");
    show_bits ((int *) (&mflags (a)), sizeof (MFLAGS) / sizeof (int), Mflags);

    return;
  }
  if (brkword () == -1) {
    c = mtstflg (a, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Mflags[b], TF[c]);
    return;
  }
  if (!ptstflg (mynum, PFL_MFLAGS)) {
    bprintf ("You are not authorised to do this...\n");
    return;
  }
  if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("\001A\033[31m\003Set flag to True or False.\001A\033[0m\003\n");
    return;
  }
#ifdef LOG_MFLAG
  mudlog ("MFLAG: %s by %s, %s := %s", pname (a), pname (mynum), Mflags[b], TF[c]);
#endif

  if (c == 0) {
    mclrflg (a, b);
  } else {
    msetflg (a, b);
  }
}

void
lflagscom (void)
{
  int a, b, c, d = 0;
  char k[80];

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("Pardon?\n");
    return;
  }
  if ((a = getroomnum ()) == 0)
    a = ploc (mynum);

  if (brkword () == -1) {
    xshow_lflags (a);
    return;
  }
  buildname (k, a);
  if ((b = tlookup (wordbuf, Lflags)) == -1) {
    if ((b = tlookup (wordbuf, L_Light)) > -0)
      d = 1;
    else if ((b = tlookup (wordbuf, L_Temp)) >= 0)
      d = 2;
    else {
      bprintf ("%s: No such Location-Flag.\n", wordbuf);
      return;
    }
  }
  if (brkword () == -1) {
    if (d == 1) {
      bprintf ("Value of Light in %s is %s\n", k,
	       Lflags[check_light (a)]);
    } else if (d == 2) {
      bprintf ("Value of Temp in %s is %s\n", k,
	       Lflags[check_temp (a)]);
    } else {
      c = ltstflg (a, b);
      c = (c != 0);
      bprintf ("Value of %s in %s is %s\n", Lflags[b], k, TF[c]);
    }
    return;
  }
  if (!ptstflg (mynum, PFL_ROOM)) {
    bprintf ("You are not authorised to do this.\n");
    return;
  }
  if (!ltstflg (a, LFL_PUBLIC)) {
    if (!EQ (pname (mynum), lowner (a)) && plev (mynum) < LVL_AVATAR) {
      bprintf ("You can only change the lflags of your own rooms.\n");
      return;
    }
  }
  if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Set flag to True or False.n");
    return;
  }
#ifdef LOG_LFLAG
  mudlog ("LFLAG: %s in %s, %s := %s", pname (mynum), k, Lflags[b], TF[c]);
#endif

  if (c)
    lsetflg (a, b);
  else
    lclrflg (a, b);
}

void
set_xpflags (int y, PFLAGS * p, PFLAGS * m)
{

  if (y >= LVL_MASTER) {
    *p = the_world->w_pflags[7];
    *m = the_world->w_mask[7];
  } else if (y >= LVL_GOD) {
    *p = the_world->w_pflags[6];
    *m = the_world->w_mask[6];
  } else if (y >= LVL_AVATAR) {
    *p = the_world->w_pflags[5];
    *m = the_world->w_mask[5];
  } else if (y >= LVL_ADVISOR) {
    *p = the_world->w_pflags[4];
    *m = the_world->w_mask[4];
  } else if (y >= LVL_ARCHWIZARD) {
    *p = the_world->w_pflags[4];
    *m = the_world->w_mask[3];
  } else if (y >= LVL_PROPHET) {
    *p = the_world->w_pflags[2];
    *m = the_world->w_mask[2];
  } else if (y > LVL_WIZARD) {
    *p = the_world->w_pflags[1];
    *m = the_world->w_mask[1];
  } else if (y == LVL_WIZARD) {
    *p = the_world->w_pflags[0];
    *m = the_world->w_mask[0];
  } else if (y < LVL_WIZARD) {
    p->u = p->h = p->l = 0;
    *m = *p;
    dset_bit (p, PFL_TITLES);
    dset_bit (p, PFL_EMOTE);
  } else {
    p->u = p->h = p->l = 0;
    *m = *p;
  }
}

/*
 * ** Sflagscom() written by Feldene for SharkMUD.
 * ** Just add this function to the end of flags.c and add the appropriate
 * ** verbs.
 */

void
sflagscom (void)
{
  int a, b, c, l;
  char *n;
  Boolean f;
  SFLAGS *p;
  PERSONA d;

  if (!ptstflg (mynum, PFL_SFLAGEDIT)) {
    bprintf ("Pardon?\n");
    return;
  }
  f = False;
  if (brkword () == -1) {
    a = mynum;
    p = &(sflags (a));
    n = pname (a);
    l = plev (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    if (plev (mynum) < LVL_ARCHWIZARD) {
      erreval ();
      return;
    }
    p = &(sflags (a));
    n = pname (a);
    l = plev (a);
  } else if (!ptstflg (mynum, PFL_UAF)) {
    bprintf ("Who's that?\n");
    return;
  } else if (plev (mynum) < LVL_ARCHWIZARD) {
    bprintf ("You can't do that now.\n");
    return;
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else {
    f = True;
    p = &(d.p_sflags);
    n = d.p_name;
    l = d.p_level;
  }

  if (a != mynum && wlevel (plev (mynum)) <= wlevel (l) && plev (mynum) < LVL_GOD) {
    bprintf ("That is beyond your powers.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Player: %s\n", n);
    bprintf ("Sflags:\n");
    show_bits ((int *) p, sizeof (SFLAGS) / sizeof (int), Sflags);

    return;
  } else if ((b = tlookup (wordbuf, Sflags)) == -1) {
    bprintf ("%s: No such Sflag.\n", wordbuf);
    return;
  } else if (brkword () == -1) {
    c = tst_bit (p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Sflags[b], TF[c]);
    return;
  } else if (a != mynum && plev (mynum) < LVL_ARCHWIZARD && plev (a) >= plev (mynum)) {
    bprintf ("That is beyond your powers.\n");
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  if (!EQ (n, pname (mynum))) {
    mudlog ("SFLAG: %s by %s, %s = %s", n, pname (mynum), Sflags[b], TF[c]);
  }
  if (c == 0) {
    clr_bit (p, b);
  } else {
    set_bit (p, b);
  }
  if (f) {
    putuaf (&d);
  }
}

/*
 * Nflagscom()
 * 1995 by Illusion
 * [Derived from Sflagscom() by Feldene]
 */

void
nflagscom (void)
{
  int a, b, c, l;
  char *n;
  Boolean f;
  NFLAGS *p;
  PERSONA d;

  if (!ptstflg (mynum, PFL_NFLAGEDIT)) {
    bprintf ("Pardon?\n");
    return;
  }
  f = False;
  if (brkword () == -1) {
    a = mynum;
    p = &(nflags (a));
    n = pname (a);
    l = plev (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    if (plev (mynum) < LVL_ARCHWIZARD) {
      erreval ();
      return;
    }
    p = &(nflags (a));
    n = pname (a);
    l = plev (a);
  } else if (!ptstflg (mynum, PFL_UAF)) {
    bprintf ("Who's that?\n");
    return;
  } else if (plev (mynum) < LVL_ARCHWIZARD) {
    bprintf ("You can't do that now.\n");
    return;
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else {
    f = True;
    p = &(d.p_nflags);
    n = d.p_name;
    l = d.p_level;
  }

  if (a != mynum && wlevel (plev (mynum)) <= wlevel (l) && plev (mynum) < LVL_GOD) {
    bprintf ("That is beyond your powers.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Player: %s\n", n);
    bprintf ("Nflags:\n");
    show_bits ((int *) p, sizeof (NFLAGS) / sizeof (int), Nflags);

    return;
  } else if ((b = tlookup (wordbuf, Nflags)) == -1) {
    bprintf ("%s: No such Nflag.\n", wordbuf);
    return;
  } else if (brkword () == -1) {
    c = xtstbit (*p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Nflags[b], TF[c]);
    return;
  } else if (a != mynum && plev (mynum) < LVL_ARCHWIZARD && plev (a) >= plev (mynum)) {
    bprintf ("That is beyond your powers.\n");
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  if (!EQ (n, pname (mynum))) {
    mudlog ("NFLAG: %s by %s, %s = %s", n, pname (mynum), Nflags[b], TF[c]);
  }
  if (c == 0) {
    xclrbit (*p, b);
  } else {
    xsetbit (*p, b);
  }
  if (f) {
    putuaf (&d);
  }
}

/*
 * Eflagscom()
 * 1996 by Illusion
 * [Derived from Sflagscom() by Feldene]
 */

void
eflagscom (void)
{
  int a, b, c, l;
  char *n;
  Boolean f;
  EFLAGS *p;
  PERSONA d;

  if (!ptstflg (mynum, PFL_EFLAGEDIT)) {
    bprintf ("Pardon?\n");
    return;
  }
  f = False;
  if (brkword () == -1) {
    a = mynum;
    p = &(eflags (a));
    n = pname (a);
    l = plev (a);
  } else if ((a = fpbn (wordbuf)) != -1) {
    if (plev (mynum) < LVL_ARCHWIZARD) {
      erreval ();
      return;
    }
    p = &(eflags (a));
    n = pname (a);
    l = plev (a);
  } else if (!ptstflg (mynum, PFL_UAF)) {
    bprintf ("Who's that?\n");
    return;
  } else if (plev (mynum) < LVL_ARCHWIZARD) {
    bprintf ("You can't do that now.\n");
    return;
  } else if (!getuaf (wordbuf, &d)) {
    bprintf ("No such persona in system.\n");
    return;
  } else {
    f = True;
    p = &(d.p_eflags);
    n = d.p_name;
    l = d.p_level;
  }

  if (a != mynum && wlevel (plev (mynum)) <= wlevel (l) && plev (mynum) < LVL_GOD) {
    bprintf ("That is beyond your powers.\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("Player: %s\n", n);
    bprintf ("Eflags:\n");
    show_bits ((int *) p, sizeof (EFLAGS) / sizeof (int), Eflags);

    return;
  } else if ((b = tlookup (wordbuf, Eflags)) == -1) {
    bprintf ("%s: No such Eflag.\n", wordbuf);
    return;
  } else if (brkword () == -1) {
    c = xtstbit (*p, b) ? 1 : 0;
    bprintf ("Value of %s is %s\n", Eflags[b], TF[c]);
    return;
  } else if (a != mynum && plev (mynum) < LVL_ARCHWIZARD && plev (a) >= plev (mynum)) {
    bprintf ("That is beyond your powers.\n");
    return;
  } else if ((c = tlookup (wordbuf, TF)) == -1) {
    bprintf ("Value must be True or False.\n");
    return;
  }
  if (!EQ (n, pname (mynum))) {
    mudlog ("EFLAG: %s by %s, %s = %s", n, pname (mynum), Eflags[b], TF[c]);
  }
  if (c == 0) {
    xclrbit (*p, b);
  } else {
    xsetbit (*p, b);
  }
  if (f) {
    putuaf (&d);
  }
}
