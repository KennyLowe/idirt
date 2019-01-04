

#include <stdlib.h>
#include "kernel.h"
#include "levels.h"
#include "sendsys.h"
#include "pflags.h"
#include "sflags.h"
#include "frob.h"
#include "uaf.h"
#include "mobile.h"
#include "log.h"
#include "parse.h"
#include "bprintf.h"
#include "mud.h"
#include "flags.h"
#include "wizlist.h"

#define PFLAGS_FROB ((1<<PFL_FROB)|(1<<PFL_CH_SCORE)|(1<<PFL_CH_LEVEL))

#define cant_fr(x,l,w,a,d,g) ((x<a && l>=w) || (x<d && l>=a) || (x<g && l>=d))
#define cant_frob(lev) \
cant_fr(plev(mynum),lev,LVL_WIZARD,LVL_ARCHWIZARD,LVL_AVATAR,LVL_GOD)

#define cant_fro(lev) (lev < LVL_MIN || lev >= LVL_MAX || cant_frob(lev))

struct _f {
  int state;
  int oldwork;
  int oldlev;
  int level;
  int strength;
  int score;
  char *oldprompt;
  char name[PNAME_LEN];
};




static void
log (char *n, int lev, int sco, int str)
{
  mudlog ("FROB: %s by %s: Lev = %d, Sco = %d, Str = %d",
	  n, pname (mynum), lev, sco, str);
}

void
frobcom (char *line)
{
  PERSONA p;
  struct _f *f;
  int x;

  if (line == NULL) {		/* First (initial) time */
    if (!tstbits (pflags (mynum).l, PFLAGS_FROB)) {
      erreval ();
      return;
    }
    if (brkword () == -1
	|| ((x = fpbn (wordbuf)) == -1 && !ptstflg (mynum, PFL_UAF))) {

      bprintf ("Frob who?\n");
      return;
    }
    if (x == -1) {
      if (!getuaf (wordbuf, &p)) {
	bprintf ("No such persona in system.\n");
	return;
      }
      if (cant_frob (p.p_level)) {
	bprintf ("You can't frob %s!\n", wordbuf);
	return;
      }
    } else if (x >= max_players) {
      bprintf ("You can't frob mobiles!\n");
      return;
    } else if (cant_frob (plev (x))) {
      bprintf ("You can't frob %s!\n", wordbuf);
      return;
    } else {
      p.p_level = plev (x);
      p.p_strength = pstr (x);
      p.p_score = pscore (x);
      strcpy (p.p_name, pname (x));
    }
    f = NEW (struct _f, 1);

    strcpy (f->name, p.p_name);
    f->state = 0;
    f->level = p.p_level;
    f->oldlev = p.p_level;
    f->strength = p.p_strength;
    f->score = p.p_score;
    f->oldprompt = COPY (cur_player->cprompt);
    strcpy (cur_player->cprompt, "New Level: ");
    f->oldwork = cur_player->work;
    cur_player->work = (int) f;
    bprintf ("\001f" FROBCHT "\003");
    bprintf ("Level is: %d\n", f->level);
    push_input_handler (frobcom);
  } else {
    while (*line == ' ' || *line == '\t')
      ++line;
    f = (struct _f *) cur_player->work;
    switch (f->state) {
    case 0:
      if (*line == '\0') {
	x = f->level;
      } else {
	x = atoi (line);
      }
      if (x < LVL_NOVICE || x > LVL_MAX) {
	bprintf ("Level must be between %d and %d\n", LVL_NOVICE, LVL_MAX);
	f->state = 20;
      } else if (cant_fro (x)) {
	bprintf ("You can't do that.\n");
	f->state = 20;
      } else {
	f->level = x;
	f->state = 1;
	bprintf ("Score is: %d\n", f->score);
	strcpy (cur_player->cprompt, "New Score: ");
      }
      break;
    case 1:
      if (*line == '\0') {
	x = f->score;
      } else {
	x = atoi (line);
      }
      f->score = x;
      f->state = 2;
      bprintf ("Strength is: %d\n", f->strength);
      strcpy (cur_player->cprompt, "New Strength: ");
      break;
    case 2:
      if (*line == '\0') {
	x = f->strength;
      } else {
	x = atoi (line);
      }
      if (x <= 0) {
	bprintf ("Strength must be positive.\n");
	f->state = 20;
      } else {
	f->strength = x;
	if ((x = fpbn (f->name)) == -1) {
	  if (!ptstflg (mynum, PFL_UAF)) {
	    bprintf ("%s isn't here.\n", f->name);
	    f->state = 20;
	  } else if (!getuaf (f->name, &p)) {
	    bprintf ("No player named %s.\n", f->name);
	    f->state = 20;
	  }
	} else {
	  p.p_level = plev (x);
	  p.p_strength = pstr (x);
	  p.p_score = pscore (x);
	}
	if (f->state == 2) {
	  log (f->name, f->level, f->score, f->strength);
	  if (x >= 0) {
	    setpstr (x, f->strength);
	    setpscore (x, f->score);
	    setplev (x, f->level);

	    set_xpflags (f->level, &pflags (x), &pmask (x));
	    setptitle (x, std_title (f->level, psex (x)));

	  } else {
	    p.p_level = f->level;
	    p.p_strength = f->strength;
	    p.p_score = f->score;

	    set_xpflags (f->level, &p.p_pflags, &p.p_mask);
	    strcpy (p.p_title,
		    std_title (f->level, tst_bit (&p.p_sflags, SFL_FEMALE)));

	    putuaf (&p);
	  }
	  update_wizlist (f->name, wlevel (f->level));
	  bprintf ("Ok.\n");
	  f->state = 20;
	}
      }
      break;
    }
    if (f->state == 20) {
      strcpy (cur_player->cprompt, f->oldprompt);
      FREE (f->oldprompt);
      cur_player->work = f->oldwork;
      FREE (f);
      pop_input_handler ();
    }
  }
  bprintf ("%s", cur_player->cprompt);
}
